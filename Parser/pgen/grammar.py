from lib2to3.pgen2 import grammar

from . import token


class Grammar(grammar.Grammar):

    def produce_graminit_h(self, writer):
        writer("/* Generated by Parser/pgen */\n\n")
        for number, symbol in self.number2symbol.items():
            writer("#define {} {}\n".format(symbol, number))

    def produce_graminit_c(self, writer):
        writer("/* Generated by Parser/pgen */\n\n")

        writer('#include "pgenheaders.h"\n')
        writer('#include "grammar.h"\n')
        writer("grammar _PyParser_Grammar;\n")

        self.print_dfas(writer)
        self.print_labels(writer)

        writer("grammar _PyParser_Grammar = {\n")
        writer("    {n_dfas},\n".format(n_dfas=len(self.dfas)))
        writer("    dfas,\n")
        writer("    {{{n_labels}, labels}},\n".format(n_labels=len(self.labels)))
        writer("    {start_number}\n".format(start_number=self.start))
        writer("};\n")

    def print_labels(self, writer):
        writer(
            "static label labels[{n_labels}] = {{\n".format(n_labels=len(self.labels))
        )
        for label, name in self.labels:
            if name is None:
                writer("    {{{label}, 0}},\n".format(label=label))
            else:
                writer(
                    '    {{{label}, "{label_name}"}},\n'.format(
                        label=label, label_name=name
                    )
                )
        writer("};\n")

    def print_dfas(self, writer):
        self.print_states(writer)
        writer("static dfa dfas[{}] = {{\n".format(len(self.dfas)))
        for dfaindex, dfa_elem in enumerate(self.dfas.items()):
            symbol, (dfa, first_sets) = dfa_elem
            writer(
                '    {{{dfa_symbol}, "{symbol_name}", '.format(
                    dfa_symbol=symbol, symbol_name=self.number2symbol[symbol]
                )
                + "0, {n_states}, states_{dfa_index},\n".format(
                    n_states=len(dfa), dfa_index=dfaindex
                )
            )
            writer('     "')

            k = [name for label, name in self.labels if label in first_sets]
            bitset = bytearray((len(self.labels) >> 3) + 1)
            for token in first_sets:
                bitset[token >> 3] |= 1 << (token & 7)
            for byte in bitset:
                writer("\\%03o" % (byte & 0xFF))
            writer('"},\n')
        writer("};\n")

    def print_states(self, write):
        for dfaindex, dfa in enumerate(self.states):
            self.print_arcs(write, dfaindex, dfa)
            write(
                "static state states_{dfa_index}[{n_states}] = {{\n".format(
                    dfa_index=dfaindex, n_states=len(dfa)
                )
            )
            for stateindex, state in enumerate(dfa):
                narcs = len(state)
                write(
                    "    {{{n_arcs}, arcs_{dfa_index}_{state_index}}},\n".format(
                        n_arcs=narcs, dfa_index=dfaindex, state_index=stateindex
                    )
                )
            write("};\n")

    def print_arcs(self, write, dfaindex, states):
        for stateindex, state in enumerate(states):
            narcs = len(state)
            write(
                "static arc arcs_{dfa_index}_{state_index}[{n_arcs}] = {{\n".format(
                    dfa_index=dfaindex, state_index=stateindex, n_arcs=narcs
                )
            )
            for a, b in state:
                write(
                    "    {{{from_label}, {to_state}}},\n".format(
                        from_label=a, to_state=b
                    )
                )
            write("};\n")
