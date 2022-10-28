"""Generate the main interpreter switch."""

# Write the cases to cases.h, which can be #included in ceval.c.

# TODO: Reuse C generation framework from deepfreeze.py?

import argparse
from dataclasses import dataclass
import re
import sys

import eparser
import sparser
import parser

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument("-i", "--input", type=str, default="Python/bytecodes.c")
arg_parser.add_argument("-o", "--output", type=str, default="Python/cases.h")
arg_parser.add_argument("-c", "--compare", action="store_true")
arg_parser.add_argument("-q", "--quiet", action="store_true")


def eopen(filename, mode="r"):
    if filename == "-":
        if "r" in mode:
            return sys.stdin
        else:
            return sys.stdout
    return open(filename, mode)


def leading_whitespace(line):
    return len(line) - len(line.lstrip())


@dataclass
class Instruction:
    opcode_name: str
    inputs: list[str]
    outputs: list[str]
    block: sparser.Block


def parse_cases(src: str, filename: str|None = None) -> tuple[list[Instruction], list[parser.Family]]:
    psr = parser.Parser(src, filename=filename)
    instrs: list[Instruction] = []
    families: list[parser.Family] = []
    while not psr.eof():
        if inst := psr.inst_def():
            assert inst.block
            instrs.append(Instruction(inst.name, inst.inputs, inst.outputs, inst.block))
        elif fam := psr.family_def():
            families.append(fam)
        else:
            raise psr.make_syntax_error(f"Unexpected token")
    return instrs, families


def always_exits(node: eparser.Node):
    match node:
        case sparser.Block(stmts):
            if stmts:
                i = len(stmts) - 1
                while i >= 0 and isinstance(stmts[i], sparser.NullStmt):
                    i -= 1
                if i >= 0:
                    return always_exits(stmts[i])
        case sparser.IfStmt(_, body, orelse):
            return always_exits(body) and always_exits(orelse)
        case sparser.GotoStmt():
            return True
        case sparser.ReturnStmt():
            return True
        case eparser.Call(term):
            if isinstance(term, eparser.Name):
                text = term.tok.text
                return (text.startswith("JUMP_TO_") or
                        text.startswith("DISPATCH") or
                        text == "Py_UNREACHABLE")

    return False


def write_cases(f, instrs):
    indent = "        "
    f.write("// This file is generated by Tools/scripts/generate_cases.py\n")
    f.write("// Do not edit!\n")
    for instr in instrs:
        assert isinstance(instr, Instruction)
        f.write(f"\n{indent}TARGET({instr.opcode_name}) {{\n")
        # input = ", ".join(instr.inputs)
        # output = ", ".join(instr.outputs)
        # f.write(f"{indent}    // {input} -- {output}\n")
        blocklines = instr.block.text.splitlines(True)
        # Remove blank lines from ends
        while blocklines and not blocklines[0].strip():
            blocklines.pop(0)
        while blocklines and not blocklines[-1].strip():
            blocklines.pop()
        # Remove leading '{' and trailing '}'
        assert blocklines and blocklines[0].strip() == "{"
        assert blocklines and blocklines[-1].strip() == "}"
        blocklines.pop()
        blocklines.pop(0)
        # Remove trailing blank lines
        while blocklines and not blocklines[-1].strip():
            blocklines.pop()
        # Write the body
        for line in blocklines:
            f.write(line)
        # Add a DISPATCH() unless the block always exits
        if not always_exits(instr.block):
            f.write(f"{indent}    DISPATCH();\n")
        # Write trailing '}'
        f.write(f"{indent}}}\n")


def main():
    args = arg_parser.parse_args()
    with eopen(args.input) as f:
        srclines = f.read().splitlines()
    begin = srclines.index("// BEGIN BYTECODES //")
    end = srclines.index("// END BYTECODES //")
    src = "\n".join(srclines[begin+1 : end])
    instrs, families = parse_cases(src, filename=args.input)
    if not args.quiet:
        ninstrs = len(instrs)
        nfamilies = len(families)
        print(
            f"Read {ninstrs} instructions "
            f"and {nfamilies} families from {args.input}",
            file=sys.stderr,
        )
    with eopen(args.output, "w") as f:
        write_cases(f, instrs)
    if not args.quiet:
        print(
            f"Wrote {ninstrs} instructions to {args.output}",
            file=sys.stderr,
        )


if __name__ == "__main__":
    main()
