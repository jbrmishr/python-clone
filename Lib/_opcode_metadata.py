# This file is generated by Tools/cases_generator/py_metadata_generator.py
# from:
#   Python/bytecodes.c
# Do not edit!
_specializations = {
    "RESUME": [
        "RESUME_CHECK",
    ],
    "LOAD_CONST": [
        "LOAD_CONST_IMMORTAL",
    ],
    "TO_BOOL": [
        "TO_BOOL_ALWAYS_TRUE",
        "TO_BOOL_BOOL",
        "TO_BOOL_INT",
        "TO_BOOL_LIST",
        "TO_BOOL_NONE",
        "TO_BOOL_STR",
    ],
    "BINARY_OP": [
        "BINARY_OP_MULTIPLY_INT",
        "BINARY_OP_ADD_INT",
        "BINARY_OP_SUBTRACT_INT",
        "BINARY_OP_MULTIPLY_FLOAT",
        "BINARY_OP_ADD_FLOAT",
        "BINARY_OP_SUBTRACT_FLOAT",
        "BINARY_OP_ADD_UNICODE",
        "BINARY_OP_INPLACE_ADD_UNICODE",
    ],
    "BINARY_SUBSCR": [
        "BINARY_SUBSCR_DICT",
        "BINARY_SUBSCR_GETITEM",
        "BINARY_SUBSCR_LIST_INT",
        "BINARY_SUBSCR_STR_INT",
        "BINARY_SUBSCR_TUPLE_INT",
    ],
    "STORE_SUBSCR": [
        "STORE_SUBSCR_DICT",
        "STORE_SUBSCR_LIST_INT",
    ],
    "SEND": [
        "SEND_GEN",
    ],
    "UNPACK_SEQUENCE": [
        "UNPACK_SEQUENCE_TWO_TUPLE",
        "UNPACK_SEQUENCE_TUPLE",
        "UNPACK_SEQUENCE_LIST",
    ],
    "STORE_ATTR": [
        "STORE_ATTR_INSTANCE_VALUE",
        "STORE_ATTR_SLOT",
        "STORE_ATTR_WITH_HINT",
    ],
    "LOAD_GLOBAL": [
        "LOAD_GLOBAL_MODULE",
        "LOAD_GLOBAL_BUILTIN",
    ],
    "LOAD_SUPER_ATTR": [
        "LOAD_SUPER_ATTR_ATTR",
        "LOAD_SUPER_ATTR_METHOD",
    ],
    "LOAD_ATTR": [
        "LOAD_ATTR_INSTANCE_VALUE",
        "LOAD_ATTR_MODULE",
        "LOAD_ATTR_WITH_HINT",
        "LOAD_ATTR_SLOT",
        "LOAD_ATTR_CLASS",
        "LOAD_ATTR_CLASS_WITH_METACLASS_CHECK",
        "LOAD_ATTR_PROPERTY",
        "LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN",
        "LOAD_ATTR_METHOD_WITH_VALUES",
        "LOAD_ATTR_METHOD_NO_DICT",
        "LOAD_ATTR_METHOD_LAZY_DICT",
        "LOAD_ATTR_NONDESCRIPTOR_WITH_VALUES",
        "LOAD_ATTR_NONDESCRIPTOR_NO_DICT",
    ],
    "COMPARE_OP": [
        "COMPARE_OP_FLOAT",
        "COMPARE_OP_INT",
        "COMPARE_OP_STR",
    ],
    "CONTAINS_OP": [
        "CONTAINS_OP_SET",
        "CONTAINS_OP_DICT",
    ],
    "FOR_ITER": [
        "FOR_ITER_LIST",
        "FOR_ITER_TUPLE",
        "FOR_ITER_RANGE",
        "FOR_ITER_GEN",
    ],
    "CALL": [
        "CALL_BOUND_METHOD_EXACT_ARGS",
        "CALL_PY_EXACT_ARGS",
        "CALL_TYPE_1",
        "CALL_STR_1",
        "CALL_TUPLE_1",
        "CALL_BUILTIN_CLASS",
        "CALL_BUILTIN_O",
        "CALL_BUILTIN_FAST",
        "CALL_BUILTIN_FAST_WITH_KEYWORDS",
        "CALL_LEN",
        "CALL_ISINSTANCE",
        "CALL_LIST_APPEND",
        "CALL_METHOD_DESCRIPTOR_O",
        "CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS",
        "CALL_METHOD_DESCRIPTOR_NOARGS",
        "CALL_METHOD_DESCRIPTOR_FAST",
        "CALL_ALLOC_AND_ENTER_INIT",
        "CALL_PY_GENERAL",
        "CALL_BOUND_METHOD_GENERAL",
        "CALL_NON_PY_GENERAL",
    ],
    "CALL_KW": [
        "CALL_KW_BOUND_METHOD",
        "CALL_KW_PY",
        "CALL_KW_NON_PY",
    ],
}

_specialized_opmap = {
    'BINARY_OP_ADD_FLOAT': 150,
    'BINARY_OP_ADD_INT': 151,
    'BINARY_OP_ADD_UNICODE': 152,
    'BINARY_OP_INPLACE_ADD_UNICODE': 3,
    'BINARY_OP_MULTIPLY_FLOAT': 153,
    'BINARY_OP_MULTIPLY_INT': 154,
    'BINARY_OP_SUBTRACT_FLOAT': 155,
    'BINARY_OP_SUBTRACT_INT': 156,
    'BINARY_SUBSCR_DICT': 157,
    'BINARY_SUBSCR_GETITEM': 158,
    'BINARY_SUBSCR_LIST_INT': 159,
    'BINARY_SUBSCR_STR_INT': 160,
    'BINARY_SUBSCR_TUPLE_INT': 161,
    'CALL_ALLOC_AND_ENTER_INIT': 162,
    'CALL_BOUND_METHOD_EXACT_ARGS': 163,
    'CALL_BOUND_METHOD_GENERAL': 164,
    'CALL_BUILTIN_CLASS': 165,
    'CALL_BUILTIN_FAST': 166,
    'CALL_BUILTIN_FAST_WITH_KEYWORDS': 167,
    'CALL_BUILTIN_O': 168,
    'CALL_ISINSTANCE': 169,
    'CALL_KW_BOUND_METHOD': 170,
    'CALL_KW_NON_PY': 171,
    'CALL_KW_PY': 172,
    'CALL_LEN': 173,
    'CALL_LIST_APPEND': 174,
    'CALL_METHOD_DESCRIPTOR_FAST': 175,
    'CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS': 176,
    'CALL_METHOD_DESCRIPTOR_NOARGS': 177,
    'CALL_METHOD_DESCRIPTOR_O': 178,
    'CALL_NON_PY_GENERAL': 179,
    'CALL_PY_EXACT_ARGS': 180,
    'CALL_PY_GENERAL': 181,
    'CALL_STR_1': 182,
    'CALL_TUPLE_1': 183,
    'CALL_TYPE_1': 184,
    'COMPARE_OP_FLOAT': 185,
    'COMPARE_OP_INT': 186,
    'COMPARE_OP_STR': 187,
    'CONTAINS_OP_DICT': 188,
    'CONTAINS_OP_SET': 189,
    'FOR_ITER_GEN': 190,
    'FOR_ITER_LIST': 191,
    'FOR_ITER_RANGE': 192,
    'FOR_ITER_TUPLE': 193,
    'LOAD_ATTR_CLASS': 194,
    'LOAD_ATTR_CLASS_WITH_METACLASS_CHECK': 195,
    'LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN': 196,
    'LOAD_ATTR_INSTANCE_VALUE': 197,
    'LOAD_ATTR_METHOD_LAZY_DICT': 198,
    'LOAD_ATTR_METHOD_NO_DICT': 199,
    'LOAD_ATTR_METHOD_WITH_VALUES': 200,
    'LOAD_ATTR_MODULE': 201,
    'LOAD_ATTR_NONDESCRIPTOR_NO_DICT': 202,
    'LOAD_ATTR_NONDESCRIPTOR_WITH_VALUES': 203,
    'LOAD_ATTR_PROPERTY': 204,
    'LOAD_ATTR_SLOT': 205,
    'LOAD_ATTR_WITH_HINT': 206,
    'LOAD_CONST_IMMORTAL': 207,
    'LOAD_GLOBAL_BUILTIN': 208,
    'LOAD_GLOBAL_MODULE': 209,
    'LOAD_SUPER_ATTR_ATTR': 210,
    'LOAD_SUPER_ATTR_METHOD': 211,
    'RESUME_CHECK': 212,
    'SEND_GEN': 213,
    'STORE_ATTR_INSTANCE_VALUE': 214,
    'STORE_ATTR_SLOT': 215,
    'STORE_ATTR_WITH_HINT': 216,
    'STORE_SUBSCR_DICT': 217,
    'STORE_SUBSCR_LIST_INT': 218,
    'TO_BOOL_ALWAYS_TRUE': 219,
    'TO_BOOL_BOOL': 220,
    'TO_BOOL_INT': 221,
    'TO_BOOL_LIST': 222,
    'TO_BOOL_NONE': 223,
    'TO_BOOL_STR': 224,
    'UNPACK_SEQUENCE_LIST': 225,
    'UNPACK_SEQUENCE_TUPLE': 226,
    'UNPACK_SEQUENCE_TWO_TUPLE': 227,
}

opmap = {
    'CACHE': 0,
    'RESERVED': 17,
    'RESUME': 149,
    'INSTRUMENTED_LINE': 254,
    'ENTER_EXECUTOR': 255,
    'BINARY_SLICE': 1,
    'BINARY_SUBSCR': 2,
    'CHECK_EG_MATCH': 4,
    'CHECK_EXC_MATCH': 5,
    'CLEANUP_THROW': 6,
    'DELETE_SUBSCR': 7,
    'END_ASYNC_FOR': 8,
    'END_FOR': 9,
    'END_SEND': 10,
    'EXIT_INIT_CHECK': 11,
    'FORMAT_SIMPLE': 12,
    'FORMAT_WITH_SPEC': 13,
    'GET_AITER': 14,
    'GET_ANEXT': 15,
    'GET_ITER': 16,
    'GET_LEN': 18,
    'GET_YIELD_FROM_ITER': 19,
    'INTERPRETER_EXIT': 20,
    'LOAD_BUILD_CLASS': 21,
    'LOAD_LOCALS': 22,
    'MAKE_FUNCTION': 23,
    'MATCH_KEYS': 24,
    'MATCH_MAPPING': 25,
    'MATCH_SEQUENCE': 26,
    'NOP': 27,
    'POP_EXCEPT': 28,
    'POP_TOP': 29,
    'PUSH_EXC_INFO': 30,
    'PUSH_NULL': 31,
    'RETURN_GENERATOR': 32,
    'RETURN_VALUE': 33,
    'SETUP_ANNOTATIONS': 34,
    'STORE_SLICE': 35,
    'STORE_SUBSCR': 36,
    'TO_BOOL': 37,
    'UNARY_INVERT': 38,
    'UNARY_NEGATIVE': 39,
    'UNARY_NOT': 40,
    'WITH_EXCEPT_START': 41,
    'BINARY_OP': 42,
    'BUILD_LIST': 43,
    'BUILD_MAP': 44,
    'BUILD_SET': 45,
    'BUILD_SLICE': 46,
    'BUILD_STRING': 47,
    'BUILD_TUPLE': 48,
    'CALL': 49,
    'CALL_FUNCTION_EX': 50,
    'CALL_INTRINSIC_1': 51,
    'CALL_INTRINSIC_2': 52,
    'CALL_KW': 53,
    'CHECK_ITERABLE': 54,
    'COMPARE_OP': 55,
    'CONTAINS_OP': 56,
    'CONVERT_VALUE': 57,
    'COPY': 58,
    'COPY_FREE_VARS': 59,
    'DELETE_ATTR': 60,
    'DELETE_DEREF': 61,
    'DELETE_FAST': 62,
    'DELETE_GLOBAL': 63,
    'DELETE_NAME': 64,
    'DICT_MERGE': 65,
    'DICT_UPDATE': 66,
    'EXTENDED_ARG': 67,
    'FOR_ITER': 68,
    'GET_AWAITABLE': 69,
    'IMPORT_FROM': 70,
    'IMPORT_NAME': 71,
    'IS_OP': 72,
    'JUMP_BACKWARD': 73,
    'JUMP_BACKWARD_NO_INTERRUPT': 74,
    'JUMP_FORWARD': 75,
    'LIST_APPEND': 76,
    'LIST_EXTEND': 77,
    'LOAD_ATTR': 78,
    'LOAD_COMMON_CONSTANT': 79,
    'LOAD_CONST': 80,
    'LOAD_DEREF': 81,
    'LOAD_FAST': 82,
    'LOAD_FAST_AND_CLEAR': 83,
    'LOAD_FAST_CHECK': 84,
    'LOAD_FAST_LOAD_FAST': 85,
    'LOAD_FROM_DICT_OR_DEREF': 86,
    'LOAD_FROM_DICT_OR_GLOBALS': 87,
    'LOAD_GLOBAL': 88,
    'LOAD_NAME': 89,
    'LOAD_SMALL_INT': 90,
    'LOAD_SPECIAL': 91,
    'LOAD_SUPER_ATTR': 92,
    'MAKE_CELL': 93,
    'MAP_ADD': 94,
    'MATCH_CLASS': 95,
    'POP_JUMP_IF_FALSE': 96,
    'POP_JUMP_IF_NONE': 97,
    'POP_JUMP_IF_NOT_NONE': 98,
    'POP_JUMP_IF_TRUE': 99,
    'RAISE_VARARGS': 100,
    'RERAISE': 101,
    'SEND': 102,
    'SET_ADD': 103,
    'SET_FUNCTION_ATTRIBUTE': 104,
    'SET_UPDATE': 105,
    'STORE_ATTR': 106,
    'STORE_DEREF': 107,
    'STORE_FAST': 108,
    'STORE_FAST_LOAD_FAST': 109,
    'STORE_FAST_STORE_FAST': 110,
    'STORE_GLOBAL': 111,
    'STORE_NAME': 112,
    'SWAP': 113,
    'UNPACK_EX': 114,
    'UNPACK_SEQUENCE': 115,
    'YIELD_VALUE': 116,
    'INSTRUMENTED_END_FOR': 237,
    'INSTRUMENTED_END_SEND': 238,
    'INSTRUMENTED_LOAD_SUPER_ATTR': 239,
    'INSTRUMENTED_FOR_ITER': 240,
    'INSTRUMENTED_CALL_KW': 241,
    'INSTRUMENTED_CALL_FUNCTION_EX': 242,
    'INSTRUMENTED_INSTRUCTION': 243,
    'INSTRUMENTED_JUMP_FORWARD': 244,
    'INSTRUMENTED_POP_JUMP_IF_TRUE': 245,
    'INSTRUMENTED_POP_JUMP_IF_FALSE': 246,
    'INSTRUMENTED_POP_JUMP_IF_NONE': 247,
    'INSTRUMENTED_POP_JUMP_IF_NOT_NONE': 248,
    'INSTRUMENTED_RESUME': 249,
    'INSTRUMENTED_RETURN_VALUE': 250,
    'INSTRUMENTED_YIELD_VALUE': 251,
    'INSTRUMENTED_CALL': 252,
    'INSTRUMENTED_JUMP_BACKWARD': 253,
    'JUMP': 256,
    'JUMP_IF_FALSE': 257,
    'JUMP_IF_TRUE': 258,
    'JUMP_NO_INTERRUPT': 259,
    'LOAD_CLOSURE': 260,
    'POP_BLOCK': 261,
    'SETUP_CLEANUP': 262,
    'SETUP_FINALLY': 263,
    'SETUP_WITH': 264,
    'STORE_FAST_MAYBE_NULL': 265,
}

HAVE_ARGUMENT = 41
MIN_INSTRUMENTED_OPCODE = 237
