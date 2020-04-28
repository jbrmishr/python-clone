import ast
import re
from typing import Any, cast, Dict, IO, Optional, List, Text, Tuple, Set

from pegen.grammar import (
    Cut,
    GrammarVisitor,
    Rhs,
    Alt,
    NamedItem,
    NameLeaf,
    StringLeaf,
    Lookahead,
    PositiveLookahead,
    NegativeLookahead,
    Opt,
    Repeat0,
    Repeat1,
    Gather,
    Group,
    Rule,
)
from pegen import grammar
from pegen.parser_generator import dedupe, ParserGenerator

EXTENSION_PREFIX = """\
#include "pegen.h"

"""

EXTENSION_SUFFIX = """
void *
_PyPegen_parse(Parser *p)
{
    // Initialize keywords
    p->keywords = reserved_keywords;
    p->n_keyword_lists = n_keyword_lists;

    return start_rule(p);
}
"""


class CCallMakerVisitor(GrammarVisitor):
    def __init__(
        self,
        parser_generator: ParserGenerator,
        exact_tokens: Dict[str, int],
        non_exact_tokens: Set[str],
    ):
        self.gen = parser_generator
        self.exact_tokens = exact_tokens
        self.non_exact_tokens = non_exact_tokens
        self.cache: Dict[Any, Any] = {}
        self.keyword_cache: Dict[str, int] = {}

    def keyword_helper(self, keyword: str) -> Tuple[str, str]:
        if keyword not in self.keyword_cache:
            self.keyword_cache[keyword] = self.gen.keyword_type()
        return "keyword", f"_PyPegen_expect_token(p, {self.keyword_cache[keyword]})"

    def visit_NameLeaf(self, node: NameLeaf) -> Tuple[str, str]:
        name = node.value
        if name in self.non_exact_tokens:
            name = name.lower()
            return f"{name}_var", f"_PyPegen_{name}_token(p)"
        return f"{name}_var", f"{name}_rule(p)"

    def visit_StringLeaf(self, node: StringLeaf) -> Tuple[str, str]:
        val = ast.literal_eval(node.value)
        if re.match(r"[a-zA-Z_]\w*\Z", val):  # This is a keyword
            return self.keyword_helper(val)
        else:
            assert val in self.exact_tokens, f"{node.value} is not a known literal"
            type = self.exact_tokens[val]
            return "literal", f"_PyPegen_expect_token(p, {type})"

    def visit_Rhs(self, node: Rhs) -> Tuple[Optional[str], str]:
        def can_we_inline(node: Rhs) -> int:
            if len(node.alts) != 1 or len(node.alts[0].items) != 1:
                return False
            # If the alternative has an action we cannot inline
            if getattr(node.alts[0], "action", None) is not None:
                return False
            return True

        if node in self.cache:
            return self.cache[node]
        if can_we_inline(node):
            self.cache[node] = self.visit(node.alts[0].items[0])
        else:
            name = self.gen.name_node(node)
            self.cache[node] = f"{name}_var", f"{name}_rule(p)"
        return self.cache[node]

    def visit_NamedItem(self, node: NamedItem) -> Tuple[Optional[str], str]:
        name, call = self.visit(node.item)
        if node.name:
            name = node.name
        return name, call

    def lookahead_call_helper(self, node: Lookahead, positive: int) -> Tuple[None, str]:
        name, call = self.visit(node.node)
        func, args = call.split("(", 1)
        assert args[-1] == ")"
        args = args[:-1]
        if "name_token" in call:
            return None, f"_PyPegen_lookahead_with_name({positive}, {func}, {args})"
        elif not args.startswith("p,"):
            return None, f"_PyPegen_lookahead({positive}, {func}, {args})"
        elif args[2:].strip().isalnum():
            return None, f"_PyPegen_lookahead_with_int({positive}, {func}, {args})"
        else:
            return None, f"_PyPegen_lookahead_with_string({positive}, {func}, {args})"

    def visit_PositiveLookahead(self, node: PositiveLookahead) -> Tuple[None, str]:
        return self.lookahead_call_helper(node, 1)

    def visit_NegativeLookahead(self, node: NegativeLookahead) -> Tuple[None, str]:
        return self.lookahead_call_helper(node, 0)

    def visit_Opt(self, node: Opt) -> Tuple[str, str]:
        name, call = self.visit(node.node)
        return "opt_var", f"{call}, 1"  # Using comma operator!

    def visit_Repeat0(self, node: Repeat0) -> Tuple[str, str]:
        if node in self.cache:
            return self.cache[node]
        name = self.gen.name_loop(node.node, False)
        self.cache[node] = f"{name}_var", f"{name}_rule(p)"
        return self.cache[node]

    def visit_Repeat1(self, node: Repeat1) -> Tuple[str, str]:
        if node in self.cache:
            return self.cache[node]
        name = self.gen.name_loop(node.node, True)
        self.cache[node] = f"{name}_var", f"{name}_rule(p)"
        return self.cache[node]

    def visit_Gather(self, node: Gather) -> Tuple[str, str]:
        if node in self.cache:
            return self.cache[node]
        name = self.gen.name_gather(node)
        self.cache[node] = f"{name}_var", f"{name}_rule(p)"
        return self.cache[node]

    def visit_Group(self, node: Group) -> Tuple[Optional[str], str]:
        return self.visit(node.rhs)

    def visit_Cut(self, node: Cut) -> Tuple[str, str]:
        return "cut_var", "1"


class CParserGenerator(ParserGenerator, GrammarVisitor):
    def __init__(
        self,
        grammar: grammar.Grammar,
        exac_tokens: Dict[str, int],
        non_exac_tokens: Set[str],
        file: Optional[IO[Text]],
        debug: bool = False,
        skip_actions: bool = False,
    ):
        super().__init__(grammar, file)
        self.callmakervisitor: CCallMakerVisitor = CCallMakerVisitor(
            self, exac_tokens, non_exac_tokens
        )
        self._varname_counter = 0
        self.debug = debug
        self.skip_actions = skip_actions

    def unique_varname(self, name: str = "tmpvar") -> str:
        new_var = name + "_" + str(self._varname_counter)
        self._varname_counter += 1
        return new_var

    def call_with_errorcheck_return(self, call_text: str, returnval: str) -> None:
        error_var = self.unique_varname()
        self.print(f"int {error_var} = {call_text};")
        self.print(f"if ({error_var}) {{")
        with self.indent():
            self.print(f"return {returnval};")
        self.print(f"}}")

    def call_with_errorcheck_goto(self, call_text: str, goto_target: str) -> None:
        error_var = self.unique_varname()
        self.print(f"int {error_var} = {call_text};")
        self.print(f"if ({error_var}) {{")
        with self.indent():
            self.print(f"goto {goto_target};")
        self.print(f"}}")

    def out_of_memory_return(
        self,
        expr: str,
        returnval: str,
        message: str = "Parser out of memory",
        cleanup_code: Optional[str] = None,
    ) -> None:
        self.print(f"if ({expr}) {{")
        with self.indent():
            self.print(f'PyErr_Format(PyExc_MemoryError, "{message}");')
            if cleanup_code is not None:
                self.print(cleanup_code)
            self.print(f"return {returnval};")
        self.print(f"}}")

    def out_of_memory_goto(
        self, expr: str, goto_target: str, message: str = "Parser out of memory"
    ) -> None:
        self.print(f"if ({expr}) {{")
        with self.indent():
            self.print(f'PyErr_Format(PyExc_MemoryError, "{message}");')
            self.print(f"goto {goto_target};")
        self.print(f"}}")

    def generate(self, filename: str) -> None:
        self.collect_todo()
        self.print(f"// @generated by pegen.py from {filename}")
        header = self.grammar.metas.get("header", EXTENSION_PREFIX)
        if header:
            self.print(header.rstrip("\n"))
        subheader = self.grammar.metas.get("subheader", "")
        if subheader:
            self.print(subheader)
        self._setup_keywords()
        for i, (rulename, rule) in enumerate(self.todo.items(), 1000):
            comment = "  // Left-recursive" if rule.left_recursive else ""
            self.print(f"#define {rulename}_type {i}{comment}")
        self.print()
        for rulename, rule in self.todo.items():
            if rule.is_loop() or rule.is_gather():
                type = "asdl_seq *"
            elif rule.type:
                type = rule.type + " "
            else:
                type = "void *"
            self.print(f"static {type}{rulename}_rule(Parser *p);")
        self.print()
        while self.todo:
            for rulename, rule in list(self.todo.items()):
                del self.todo[rulename]
                self.print()
                if rule.left_recursive:
                    self.print("// Left-recursive")
                self.visit(rule)
        if self.skip_actions:
            mode = 0
        else:
            mode = int(self.rules["start"].type == "mod_ty") if "start" in self.rules else 1
            if mode == 1 and self.grammar.metas.get("bytecode"):
                mode += 1
        modulename = self.grammar.metas.get("modulename", "parse")
        trailer = self.grammar.metas.get("trailer", EXTENSION_SUFFIX)
        keyword_cache = self.callmakervisitor.keyword_cache
        if trailer:
            self.print(trailer.rstrip("\n") % dict(mode=mode, modulename=modulename))

    def _group_keywords_by_length(self) -> Dict[int, List[Tuple[str, int]]]:
        groups: Dict[int, List[Tuple[str, int]]] = {}
        for keyword_str, keyword_type in self.callmakervisitor.keyword_cache.items():
            length = len(keyword_str)
            if length in groups:
                groups[length].append((keyword_str, keyword_type))
            else:
                groups[length] = [(keyword_str, keyword_type)]
        return groups

    def _setup_keywords(self) -> None:
        keyword_cache = self.callmakervisitor.keyword_cache
        n_keyword_lists = (
            len(max(keyword_cache.keys(), key=len)) + 1 if len(keyword_cache) > 0 else 0
        )
        self.print(f"static const int n_keyword_lists = {n_keyword_lists};")
        groups = self._group_keywords_by_length()
        self.print("static KeywordToken *reserved_keywords[] = {")
        with self.indent():
            num_groups = max(groups) + 1 if groups else 1
            for keywords_length in range(num_groups):
                if keywords_length not in groups.keys():
                    self.print("NULL,")
                else:
                    self.print("(KeywordToken[]) {")
                    with self.indent():
                        for keyword_str, keyword_type in groups[keywords_length]:
                            self.print(f'{{"{keyword_str}", {keyword_type}}},')
                        self.print("{NULL, -1},")
                    self.print("},")
        self.print("};")

    def _set_up_token_start_metadata_extraction(self) -> None:
        self.print("if (p->mark == p->fill && _PyPegen_fill_token(p) < 0) {")
        with self.indent():
            self.print("p->error_indicator = 1;")
            self.print("return NULL;")
        self.print("}")
        self.print("int start_lineno = p->tokens[mark]->lineno;")
        self.print("UNUSED(start_lineno); // Only used by EXTRA macro")
        self.print("int start_col_offset = p->tokens[mark]->col_offset;")
        self.print("UNUSED(start_col_offset); // Only used by EXTRA macro")

    def _set_up_token_end_metadata_extraction(self) -> None:
        self.print("Token *token = _PyPegen_get_last_nonnwhitespace_token(p);")
        self.print("if (token == NULL) {")
        with self.indent():
            self.print("return NULL;")
        self.print("}")
        self.print(f"int end_lineno = token->end_lineno;")
        self.print("UNUSED(end_lineno); // Only used by EXTRA macro")
        self.print(f"int end_col_offset = token->end_col_offset;")
        self.print("UNUSED(end_col_offset); // Only used by EXTRA macro")

    def _set_up_rule_memoization(self, node: Rule, result_type: str) -> None:
        self.print("{")
        with self.indent():
            self.print(f"{result_type} res = NULL;")
            self.print(f"if (_PyPegen_is_memoized(p, {node.name}_type, &res))")
            with self.indent():
                self.print("return res;")
            self.print("int mark = p->mark;")
            self.print("int resmark = p->mark;")
            self.print("while (1) {")
            with self.indent():
                self.call_with_errorcheck_return(
                    f"_PyPegen_update_memo(p, mark, {node.name}_type, res)", "res"
                )
                self.print("p->mark = mark;")
                self.print(f"void *raw = {node.name}_raw(p);")
                self.print("if (raw == NULL || p->mark <= resmark)")
                with self.indent():
                    self.print("break;")
                self.print("resmark = p->mark;")
                self.print("res = raw;")
            self.print("}")
            self.print("p->mark = resmark;")
            self.print("return res;")
        self.print("}")
        self.print(f"static {result_type}")
        self.print(f"{node.name}_raw(Parser *p)")

    def _should_memoize(self, node: Rule) -> bool:
        return node.memo and not node.left_recursive

    def _handle_default_rule_body(self, node: Rule, rhs: Rhs, result_type: str) -> None:
        memoize = self._should_memoize(node)

        with self.indent():
            self.print("if (p->error_indicator) {")
            with self.indent():
                self.print("return NULL;")
            self.print("}")
            self.print(f"{result_type} res = NULL;")
            if memoize:
                self.print(f"if (_PyPegen_is_memoized(p, {node.name}_type, &res))")
                with self.indent():
                    self.print("return res;")
            self.print("int mark = p->mark;")
            if any(alt.action and "EXTRA" in alt.action for alt in rhs.alts):
                self._set_up_token_start_metadata_extraction()
            self.visit(
                rhs,
                is_loop=False,
                is_gather=node.is_gather(),
                rulename=node.name if memoize else None,
            )
            if self.debug:
                self.print(f'fprintf(stderr, "Fail at %d: {node.name}\\n", p->mark);')
            self.print("res = NULL;")
        self.print("  done:")
        with self.indent():
            if memoize:
                self.print(f"_PyPegen_insert_memo(p, mark, {node.name}_type, res);")
            self.print("return res;")

    def _handle_loop_rule_body(self, node: Rule, rhs: Rhs) -> None:
        memoize = self._should_memoize(node)
        is_repeat1 = node.name.startswith("_loop1")

        with self.indent():
            self.print("if (p->error_indicator) {")
            with self.indent():
                self.print("return NULL;")
            self.print("}")
            self.print(f"void *res = NULL;")
            if memoize:
                self.print(f"if (_PyPegen_is_memoized(p, {node.name}_type, &res))")
                with self.indent():
                    self.print("return res;")
            self.print("int mark = p->mark;")
            self.print("int start_mark = p->mark;")
            self.print("void **children = PyMem_Malloc(sizeof(void *));")
            self.out_of_memory_return(f"!children", "NULL")
            self.print("ssize_t children_capacity = 1;")
            self.print("ssize_t n = 0;")
            if any(alt.action and "EXTRA" in alt.action for alt in rhs.alts):
                self._set_up_token_start_metadata_extraction()
            self.visit(
                rhs,
                is_loop=True,
                is_gather=node.is_gather(),
                rulename=node.name if memoize else None,
            )
            if is_repeat1:
                self.print("if (n == 0) {")
                with self.indent():
                    self.print("PyMem_Free(children);")
                    self.print("return NULL;")
                self.print("}")
            self.print("asdl_seq *seq = _Py_asdl_seq_new(n, p->arena);")
            self.out_of_memory_return(
                f"!seq",
                "NULL",
                message=f"asdl_seq_new {node.name}",
                cleanup_code="PyMem_Free(children);",
            )
            self.print("for (int i = 0; i < n; i++) asdl_seq_SET(seq, i, children[i]);")
            self.print("PyMem_Free(children);")
            if node.name:
                self.print(f"_PyPegen_insert_memo(p, start_mark, {node.name}_type, seq);")
            self.print("return seq;")

    def visit_Rule(self, node: Rule) -> None:
        is_loop = node.is_loop()
        is_gather = node.is_gather()
        rhs = node.flatten()
        if is_loop or is_gather:
            result_type = "asdl_seq *"
        elif node.type:
            result_type = node.type
        else:
            result_type = "void *"

        for line in str(node).splitlines():
            self.print(f"// {line}")
        if node.left_recursive and node.leader:
            self.print(f"static {result_type} {node.name}_raw(Parser *);")

        self.print(f"static {result_type}")
        self.print(f"{node.name}_rule(Parser *p)")

        if node.left_recursive and node.leader:
            self._set_up_rule_memoization(node, result_type)

        self.print("{")
        if is_loop:
            self._handle_loop_rule_body(node, rhs)
        else:
            self._handle_default_rule_body(node, rhs, result_type)
        self.print("}")

    def visit_NamedItem(self, node: NamedItem, names: List[str]) -> None:
        name, call = self.callmakervisitor.visit(node)
        if not name:
            self.print(call)
        else:
            name = dedupe(name, names)
            self.print(f"({name} = {call})")

    def visit_Rhs(
        self, node: Rhs, is_loop: bool, is_gather: bool, rulename: Optional[str]
    ) -> None:
        if is_loop:
            assert len(node.alts) == 1
        for alt in node.alts:
            self.visit(alt, is_loop=is_loop, is_gather=is_gather, rulename=rulename)

    def join_conditions(self, keyword: str, node: Any, names: List[str]) -> None:
        self.print(f"{keyword} (")
        with self.indent():
            first = True
            for item in node.items:
                if first:
                    first = False
                else:
                    self.print("&&")
                self.visit(item, names=names)
        self.print(")")

    def emit_action(self, node: Alt, cleanup_code: Optional[str] = None) -> None:
        self.print(f"res = {node.action};")

        self.print("if (res == NULL && PyErr_Occurred()) {")
        with self.indent():
            self.print("p->error_indicator = 1;")
            if cleanup_code:
                self.print(cleanup_code)
            self.print("return NULL;")
        self.print("}")

        if self.debug:
            self.print(
                f'fprintf(stderr, "Hit with action [%d-%d]: %s\\n", mark, p->mark, "{node}");'
            )

    def emit_default_action(self, is_gather: bool, names: List[str], node: Alt) -> None:
        if len(names) > 1:
            if is_gather:
                assert len(names) == 2
                self.print(f"res = _PyPegen_seq_insert_in_front(p, {names[0]}, {names[1]});")
            else:
                if self.debug:
                    self.print(
                        f'fprintf(stderr, "Hit without action [%d:%d]: %s\\n", mark, p->mark, "{node}");'
                    )
                self.print(f"res = _PyPegen_dummy_name(p, {', '.join(names)});")
        else:
            if self.debug:
                self.print(
                    f'fprintf(stderr, "Hit with default action [%d:%d]: %s\\n", mark, p->mark, "{node}");'
                )
            self.print(f"res = {names[0]};")

    def emit_dummy_action(self) -> None:
        self.print(f"res = _PyPegen_dummy_name(p);")

    def handle_alt_normal(self, node: Alt, is_gather: bool, names: List[str]) -> None:
        self.join_conditions(keyword="if", node=node, names=names)
        self.print("{")
        # We have parsed successfully all the conditions for the option.
        with self.indent():
            # Prepare to emmit the rule action and do so
            if node.action and "EXTRA" in node.action:
                self._set_up_token_end_metadata_extraction()
            if self.skip_actions:
                self.emit_dummy_action()
            elif node.action:
                self.emit_action(node)
            else:
                self.emit_default_action(is_gather, names, node)

            # As the current option has parsed correctly, do not continue with the rest.
            self.print(f"goto done;")
        self.print("}")

    def handle_alt_loop(
        self, node: Alt, is_gather: bool, rulename: Optional[str], names: List[str]
    ) -> None:
        # Condition of the main body of the alternative
        self.join_conditions(keyword="while", node=node, names=names)
        self.print("{")
        # We have parsed successfully one item!
        with self.indent():
            # Prepare to emit the rule action and do so
            if node.action and "EXTRA" in node.action:
                self._set_up_token_end_metadata_extraction()
            if self.skip_actions:
                self.emit_dummy_action()
            elif node.action:
                self.emit_action(node, cleanup_code="PyMem_Free(children);")
            else:
                self.emit_default_action(is_gather, names, node)

            # Add the result of rule to the temporary buffer of children. This buffer
            # will populate later an asdl_seq with all elements to return.
            self.print("if (n == children_capacity) {")
            with self.indent():
                self.print("children_capacity *= 2;")
                self.print("children = PyMem_Realloc(children, children_capacity*sizeof(void *));")
                self.out_of_memory_return(f"!children", "NULL", message=f"realloc {rulename}")
            self.print("}")
            self.print(f"children[n++] = res;")
            self.print("mark = p->mark;")
        self.print("}")

    def visit_Alt(
        self, node: Alt, is_loop: bool, is_gather: bool, rulename: Optional[str]
    ) -> None:
        self.print(f"{{ // {node}")
        with self.indent():
            # Prepare variable declarations for the alternative
            vars = self.collect_vars(node)
            for v, var_type in sorted(item for item in vars.items() if item[0] is not None):
                if not var_type:
                    var_type = "void *"
                else:
                    var_type += " "
                if v == "cut_var":
                    v += " = 0"  # cut_var must be initialized
                self.print(f"{var_type}{v};")
                if v == "opt_var":
                    self.print("UNUSED(opt_var); // Silence compiler warnings")

            names: List[str] = []
            if is_loop:
                self.handle_alt_loop(node, is_gather, rulename, names)
            else:
                self.handle_alt_normal(node, is_gather, names)

            self.print("p->mark = mark;")
            if "cut_var" in names:
                self.print("if (cut_var) return NULL;")
        self.print("}")

    def collect_vars(self, node: Alt) -> Dict[str, Optional[str]]:
        names: List[str] = []
        types = {}
        for item in node.items:
            name, type = self.add_var(item, names)
            types[name] = type
        return types

    def add_var(self, node: NamedItem, names: List[str]) -> Tuple[str, Optional[str]]:
        name: str
        call: str
        name, call = self.callmakervisitor.visit(node.item)
        type = None
        if not name:
            return name, type
        if name.startswith("cut"):
            return name, "int"
        if name.endswith("_var"):
            rulename = name[:-4]
            rule = self.rules.get(rulename)
            if rule is not None:
                if rule.is_loop() or rule.is_gather():
                    type = "asdl_seq *"
                else:
                    type = rule.type
            elif name.startswith("_loop") or name.startswith("_gather"):
                type = "asdl_seq *"
            elif name in ("name_var", "string_var", "number_var"):
                type = "expr_ty"
        if node.name:
            name = node.name
        name = dedupe(name, names)
        return name, type
