from pathlib import Path
from typing import TextIO

from analyzer import (
    Instruction,
    Uop,
    Properties,
    StackItem,
    analysis_error,
)
from cwriter import CWriter
from typing import Callable, Mapping, TextIO, Iterator, Iterable
from lexer import Token
from stack import Stack, Local, Storage, StackError

# Set this to true for voluminous output showing state of stack and locals
PRINT_STACKS = False

class TokenIterator:

    look_ahead: Token | None
    iterator: Iterator[Token]

    def __init__(self, tkns: Iterable[Token]):
        self.iterator = iter(tkns)
        self.look_ahead = None

    def __iter__(self) -> "TokenIterator":
        return self

    def __next__(self) -> Token:
        if self.look_ahead is None:
            return next(self.iterator)
        else:
            res = self.look_ahead
            self.look_ahead = None
            return res

    def peek(self) -> Token | None:
        if self.look_ahead is None:
            for tkn in self.iterator:
                self.look_ahead = tkn
                break
        return self.look_ahead

ROOT = Path(__file__).parent.parent.parent.resolve()
DEFAULT_INPUT = (ROOT / "Python/bytecodes.c").as_posix()


def root_relative_path(filename: str) -> str:
    try:
        return Path(filename).resolve().relative_to(ROOT).as_posix()
    except ValueError:
        # Not relative to root, just return original path.
        return filename


def type_and_null(var: StackItem) -> tuple[str, str]:
    if var.type:
        return var.type, "NULL"
    elif var.is_array():
        return "_PyStackRef *", "NULL"
    else:
        return "_PyStackRef", "PyStackRef_NULL"


def write_header(
    generator: str, sources: list[str], outfile: TextIO, comment: str = "//"
) -> None:
    outfile.write(
        f"""{comment} This file is generated by {root_relative_path(generator)}
{comment} from:
{comment}   {", ".join(root_relative_path(src) for src in sources)}
{comment} Do not edit!
"""
    )


def emit_to(out: CWriter, tkn_iter: TokenIterator, end: str) -> Token:
    parens = 0
    for tkn in tkn_iter:
        if tkn.kind == end and parens == 0:
            return tkn
        if tkn.kind == "LPAREN":
            parens += 1
        if tkn.kind == "RPAREN":
            parens -= 1
        out.emit(tkn)
    raise analysis_error(f"Expecting {end}. Reached end of file", tkn)


ReplacementFunctionType = Callable[
    [Token, TokenIterator, Uop, Storage, Instruction | None], bool
]

def always_true(tkn: Token | None) -> bool:
    if tkn is None:
        return False
    return tkn.text in {"true", "1"}


class Emitter:
    out: CWriter
    _replacers: dict[str, ReplacementFunctionType]

    def __init__(self, out: CWriter):
        self._replacers = {
            "EXIT_IF": self.exit_if,
            "DEOPT_IF": self.deopt_if,
            "ERROR_IF": self.error_if,
            "ERROR_NO_POP": self.error_no_pop,
            "DECREF_INPUTS": self.decref_inputs,
            "DEAD": self.kill,
            "INPUTS_DEAD": self.kill_inputs,
            "SYNC_SP": self.sync_sp,
            "SAVE_STACK": self.save_stack,
            "RELOAD_STACK": self.reload_stack,
            "PyStackRef_CLOSE": self.stackref_close,
            "PyStackRef_CLOSE_SPECIALIZED": self.stackref_close,
            "PyStackRef_AsPyObjectSteal": self.stackref_steal,
            "DISPATCH": self.dispatch
        }
        self.out = out

    def dispatch(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        self.emit(tkn)
        return False

    def deopt_if(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        self.out.emit_at("DEOPT_IF", tkn)
        lparen = next(tkn_iter)
        self.emit(lparen)
        assert lparen.kind == "LPAREN"
        first_tkn = tkn_iter.peek()
        emit_to(self.out, tkn_iter, "RPAREN")
        next(tkn_iter)  # Semi colon
        self.out.emit(", ")
        assert inst is not None
        assert inst.family is not None
        self.out.emit(inst.family.name)
        self.out.emit(");\n")
        return not always_true(first_tkn)

    exit_if = deopt_if

    def error_if(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        self.out.emit_at("if ", tkn)
        lparen = next(tkn_iter)
        self.emit(lparen)
        assert lparen.kind == "LPAREN"
        first_tkn = tkn_iter.peek()
        emit_to(self.out, tkn_iter, "COMMA")
        label = next(tkn_iter).text
        next(tkn_iter)  # RPAREN
        next(tkn_iter)  # Semi colon
        self.out.emit(") ")
        storage.clear_inputs("at ERROR_IF")
        c_offset = storage.stack.peek_offset()
        try:
            offset = -int(c_offset)
        except ValueError:
            offset = -1
        if offset > 0:
            self.out.emit(f"goto pop_{offset}_")
            self.out.emit(label)
            self.out.emit(";\n")
        elif offset == 0:
            self.out.emit("goto ")
            self.out.emit(label)
            self.out.emit(";\n")
        else:
            self.out.emit("{\n")
            storage.copy().flush(self.out)
            self.out.emit("goto ")
            self.out.emit(label)
            self.out.emit(";\n")
            self.out.emit("}\n")
        return not always_true(first_tkn)

    def error_no_pop(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        next(tkn_iter)  # LPAREN
        next(tkn_iter)  # RPAREN
        next(tkn_iter)  # Semi colon
        self.out.emit_at("goto error;", tkn)
        return False

    def decref_inputs(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        next(tkn_iter)
        next(tkn_iter)
        next(tkn_iter)
        self.out.emit_at("", tkn)
        for var in uop.stack.inputs:
            if var.name == "unused" or var.name == "null" or var.peek:
                continue
            if var.size:
                if var.size == "1":
                    self.out.emit(f"PyStackRef_CLOSE({var.name}[0]);\n")
                else:
                    self.out.emit(f"for (int _i = {var.size}; --_i >= 0;) {{\n")
                    self.out.emit(f"PyStackRef_CLOSE({var.name}[_i]);\n")
                    self.out.emit("}\n")
            elif var.condition:
                if var.condition == "1":
                    self.out.emit(f"PyStackRef_CLOSE({var.name});\n")
                elif var.condition != "0":
                    self.out.emit(f"PyStackRef_XCLOSE({var.name});\n")
            else:
                self.out.emit(f"PyStackRef_CLOSE({var.name});\n")
        for input in storage.inputs:
            input.defined = False
        return True

    def kill_inputs(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        next(tkn_iter)
        next(tkn_iter)
        next(tkn_iter)
        for var in storage.inputs:
            var.defined = False
        return True

    def kill(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        next(tkn_iter)
        name_tkn = next(tkn_iter)
        name = name_tkn.text
        next(tkn_iter)
        next(tkn_iter)
        for var in storage.inputs:
            if var.name == name:
                var.defined = False
                break
        else:
            raise analysis_error(f"'{name}' is not a live input-only variable", name_tkn)
        return True

    def stackref_close(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        self.out.emit(tkn)
        tkn = next(tkn_iter)
        assert tkn.kind == "LPAREN"
        self.out.emit(tkn)
        name = next(tkn_iter)
        self.out.emit(name)
        if name.kind == "IDENTIFIER":
            for var in storage.inputs:
                if var.name == name.text:
                    var.defined = False
        rparen = emit_to(self.out, tkn_iter, "RPAREN")
        self.emit(rparen)
        return True

    stackref_steal = stackref_close

    def sync_sp(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        next(tkn_iter)
        next(tkn_iter)
        next(tkn_iter)
        storage.clear_inputs("when syncing stack")
        storage.flush(self.out)
        self._print_storage(storage)
        return True

    def emit_save(self, storage: Storage) -> None:
        storage.save(self.out)
        self._print_storage(storage)

    def save_stack(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        next(tkn_iter)
        next(tkn_iter)
        next(tkn_iter)
        self.emit_save(storage)
        return True

    def emit_reload(self, storage: Storage) -> None:
        storage.reload(self.out)
        self._print_storage(storage)

    def reload_stack(
        self,
        tkn: Token,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> bool:
        next(tkn_iter)
        next(tkn_iter)
        next(tkn_iter)
        self.emit_reload(storage)
        return True

    def _print_storage(self, storage: Storage) -> None:
        if PRINT_STACKS:
            self.out.start_line()
            self.emit(storage.as_comment())
            self.out.start_line()

    def _emit_if(
        self,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> tuple[bool, Token, Storage]:
        """Returns (reachable?, closing '}', stack)."""
        tkn = next(tkn_iter)
        assert tkn.kind == "LPAREN"
        self.out.emit(tkn)
        rparen = emit_to(self.out, tkn_iter, "RPAREN")
        self.emit(rparen)
        if_storage = storage.copy()
        reachable, rbrace, if_storage = self._emit_block(tkn_iter, uop, if_storage, inst, True)
        try:
            maybe_else = tkn_iter.peek()
            if maybe_else and maybe_else.kind == "ELSE":
                self._print_storage(storage)
                self.emit(rbrace)
                self.emit(next(tkn_iter))
                maybe_if = tkn_iter.peek()
                if maybe_if and maybe_if.kind == "IF":
                    #Emit extra braces around the if to get scoping right
                    self.emit(" {\n")
                    self.emit(next(tkn_iter))
                    else_reachable, rbrace, else_storage = self._emit_if(tkn_iter, uop, storage, inst)
                    self.out.start_line()
                    self.emit("}\n")
                else:
                    else_reachable, rbrace, else_storage = self._emit_block(tkn_iter, uop, storage, inst, True)
                if not reachable:
                    # Discard the if storage
                    reachable = else_reachable
                    storage = else_storage
                elif not else_reachable:
                    # Discard the else storage
                    storage = if_storage
                    reachable = True
                else:
                    if PRINT_STACKS:
                        self.emit("/* Merge */\n")
                    else_storage.merge(if_storage, self.out)
                    storage = else_storage
                    self._print_storage(storage)
            else:
                if reachable:
                    if PRINT_STACKS:
                        self.emit("/* Merge */\n")
                    if_storage.merge(storage, self.out)
                    storage = if_storage
                    self._print_storage(storage)
                else:
                    # Discard the if storage
                    reachable = True
        except StackError as ex:
            self._print_storage(if_storage)
            raise analysis_error(ex.args[0], rbrace) # from None
        return reachable, rbrace, storage

    def _emit_block(
        self,
        tkn_iter: TokenIterator,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
        emit_first_brace: bool
    ) -> tuple[bool, Token, Storage]:
        """ Returns (reachable?, closing '}', stack)."""
        braces = 1
        out_stores = set(uop.output_stores)
        tkn = next(tkn_iter)
        reload: Token | None = None
        try:
            reachable = True
            line : int = -1
            if tkn.kind != "LBRACE":
                raise analysis_error(f"PEP 7: expected '{{', found: {tkn.text}", tkn)
            escaping_calls = uop.properties.escaping_calls
            if emit_first_brace:
                self.emit(tkn)
            self._print_storage(storage)
            for tkn in tkn_iter:
                if PRINT_STACKS and tkn.line != line:
                    self.out.start_line()
                    self.emit(storage.as_comment())
                    self.out.start_line()
                    line = tkn.line
                if tkn in escaping_calls:
                    if tkn != reload:
                        self.emit_save(storage)
                    _, reload = escaping_calls[tkn]
                elif tkn == reload:
                    self.emit_reload(storage)
                if tkn.kind == "LBRACE":
                    self.out.emit(tkn)
                    braces += 1
                elif tkn.kind == "RBRACE":
                    self._print_storage(storage)
                    braces -= 1
                    if braces == 0:
                        return reachable, tkn, storage
                    self.out.emit(tkn)
                elif tkn.kind == "GOTO":
                    reachable = False;
                    self.out.emit(tkn)
                elif tkn.kind == "IDENTIFIER":
                    if tkn.text in self._replacers:
                        if not self._replacers[tkn.text](tkn, tkn_iter, uop, storage, inst):
                            reachable = False
                    else:
                        if tkn in out_stores:
                            for out in storage.outputs:
                                if out.name == tkn.text:
                                    out.defined = True
                                    out.in_memory = False
                                    break
                        if tkn.text.startswith("DISPATCH"):
                            self._print_storage(storage)
                            reachable = False
                        self.out.emit(tkn)
                elif tkn.kind == "IF":
                    self.out.emit(tkn)
                    if_reachable, rbrace, storage = self._emit_if(tkn_iter, uop, storage, inst)
                    if reachable:
                        reachable = if_reachable
                    self.out.emit(rbrace)
                else:
                    self.out.emit(tkn)
        except StackError as ex:
            raise analysis_error(ex.args[0], tkn) from None
        raise analysis_error("Expecting closing brace. Reached end of file", tkn)


    def emit_tokens(
        self,
        uop: Uop,
        storage: Storage,
        inst: Instruction | None,
    ) -> Storage:
        tkn_iter = TokenIterator(uop.body)
        self.out.start_line()
        _, rbrace, storage = self._emit_block(tkn_iter, uop, storage, inst, False)
        try:
            self._print_storage(storage)
            storage.push_outputs()
            self._print_storage(storage)
        except StackError as ex:
            raise analysis_error(ex.args[0], rbrace)
        return storage

    def emit(self, txt: str | Token) -> None:
        self.out.emit(txt)


def cflags(p: Properties) -> str:
    flags: list[str] = []
    if p.oparg:
        flags.append("HAS_ARG_FLAG")
    if p.uses_co_consts:
        flags.append("HAS_CONST_FLAG")
    if p.uses_co_names:
        flags.append("HAS_NAME_FLAG")
    if p.jumps:
        flags.append("HAS_JUMP_FLAG")
    if p.has_free:
        flags.append("HAS_FREE_FLAG")
    if p.uses_locals:
        flags.append("HAS_LOCAL_FLAG")
    if p.eval_breaker:
        flags.append("HAS_EVAL_BREAK_FLAG")
    if p.deopts:
        flags.append("HAS_DEOPT_FLAG")
    if p.side_exit:
        flags.append("HAS_EXIT_FLAG")
    if not p.infallible:
        flags.append("HAS_ERROR_FLAG")
    if p.error_without_pop:
        flags.append("HAS_ERROR_NO_POP_FLAG")
    if p.escapes:
        flags.append("HAS_ESCAPES_FLAG")
    if p.pure:
        flags.append("HAS_PURE_FLAG")
    if p.oparg_and_1:
        flags.append("HAS_OPARG_AND_1_FLAG")
    if flags:
        return " | ".join(flags)
    else:
        return "0"
