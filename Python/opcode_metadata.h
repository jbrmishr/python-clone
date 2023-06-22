// This file is generated by Tools/cases_generator/generate_cases.py
// from:
//   Python/bytecodes.c
// Do not edit!


#define IS_PSEUDO_INSTR(OP)  \
    ((OP) == STORE_FAST_MAYBE_NULL) || \
    ((OP) == LOAD_SUPER_METHOD) || \
    ((OP) == LOAD_ZERO_SUPER_METHOD) || \
    ((OP) == LOAD_ZERO_SUPER_ATTR) || \
    ((OP) == LOAD_METHOD) || \
    ((OP) == JUMP) || \
    ((OP) == JUMP_NO_INTERRUPT) || \
    ((OP) == SETUP_FINALLY) || \
    ((OP) == SETUP_CLEANUP) || \
    ((OP) == SETUP_WITH) || \
    ((OP) == POP_BLOCK) || \
    0

#ifndef NEED_OPCODE_METADATA
extern int _PyOpcode_num_popped(int opcode, int oparg, bool jump);
#else
int
_PyOpcode_num_popped(int opcode, int oparg, bool jump) {
    switch(opcode) {
        case NOP:
            return 0;
        case RESUME:
            return 0;
        case INSTRUMENTED_RESUME:
            return 0;
        case LOAD_CLOSURE:
            return 0;
        case LOAD_FAST_CHECK:
            return 0;
        case LOAD_FAST:
            return 0;
        case LOAD_FAST_AND_CLEAR:
            return 0;
        case LOAD_FAST_LOAD_FAST:
            return 0;
        case LOAD_CONST:
            return 0;
        case STORE_FAST:
            return 1;
        case STORE_FAST_MAYBE_NULL:
            return 1;
        case STORE_FAST_LOAD_FAST:
            return 1;
        case STORE_FAST_STORE_FAST:
            return 2;
        case POP_TOP:
            return 1;
        case PUSH_NULL:
            return 0;
        case END_FOR:
            return 2;
        case INSTRUMENTED_END_FOR:
            return 2;
        case END_SEND:
            return 2;
        case INSTRUMENTED_END_SEND:
            return 2;
        case UNARY_NEGATIVE:
            return 1;
        case UNARY_NOT:
            return 1;
        case UNARY_INVERT:
            return 1;
        case BINARY_OP_MULTIPLY_INT:
            return 2;
        case BINARY_OP_ADD_INT:
            return 2;
        case BINARY_OP_SUBTRACT_INT:
            return 2;
        case BINARY_OP_MULTIPLY_FLOAT:
            return 2;
        case BINARY_OP_ADD_FLOAT:
            return 2;
        case BINARY_OP_SUBTRACT_FLOAT:
            return 2;
        case BINARY_OP_ADD_UNICODE:
            return 2;
        case BINARY_OP_INPLACE_ADD_UNICODE:
            return 2;
        case BINARY_SUBSCR:
            return 2;
        case BINARY_SLICE:
            return 3;
        case STORE_SLICE:
            return 4;
        case BINARY_SUBSCR_LIST_INT:
            return 2;
        case BINARY_SUBSCR_TUPLE_INT:
            return 2;
        case BINARY_SUBSCR_DICT:
            return 2;
        case BINARY_SUBSCR_GETITEM:
            return 2;
        case LIST_APPEND:
            return (oparg-1) + 2;
        case SET_ADD:
            return (oparg-1) + 2;
        case STORE_SUBSCR:
            return 3;
        case STORE_SUBSCR_LIST_INT:
            return 3;
        case STORE_SUBSCR_DICT:
            return 3;
        case DELETE_SUBSCR:
            return 2;
        case CALL_INTRINSIC_1:
            return 1;
        case CALL_INTRINSIC_2:
            return 2;
        case RAISE_VARARGS:
            return oparg;
        case INTERPRETER_EXIT:
            return 1;
        case RETURN_VALUE:
            return 1;
        case INSTRUMENTED_RETURN_VALUE:
            return 1;
        case RETURN_CONST:
            return 0;
        case INSTRUMENTED_RETURN_CONST:
            return 0;
        case GET_AITER:
            return 1;
        case GET_ANEXT:
            return 1;
        case GET_AWAITABLE:
            return 1;
        case SEND:
            return 2;
        case SEND_GEN:
            return 2;
        case INSTRUMENTED_YIELD_VALUE:
            return 1;
        case YIELD_VALUE:
            return 1;
        case POP_EXCEPT:
            return 1;
        case RERAISE:
            return oparg + 1;
        case END_ASYNC_FOR:
            return 2;
        case CLEANUP_THROW:
            return 3;
        case LOAD_ASSERTION_ERROR:
            return 0;
        case LOAD_BUILD_CLASS:
            return 0;
        case STORE_NAME:
            return 1;
        case DELETE_NAME:
            return 0;
        case UNPACK_SEQUENCE:
            return 1;
        case UNPACK_SEQUENCE_TWO_TUPLE:
            return 1;
        case UNPACK_SEQUENCE_TUPLE:
            return 1;
        case UNPACK_SEQUENCE_LIST:
            return 1;
        case UNPACK_EX:
            return 1;
        case STORE_ATTR:
            return 2;
        case DELETE_ATTR:
            return 1;
        case STORE_GLOBAL:
            return 1;
        case DELETE_GLOBAL:
            return 0;
        case LOAD_LOCALS:
            return 0;
        case LOAD_NAME:
            return 0;
        case LOAD_FROM_DICT_OR_GLOBALS:
            return 1;
        case LOAD_GLOBAL:
            return 0;
        case LOAD_GLOBAL_MODULE:
            return 0;
        case LOAD_GLOBAL_BUILTIN:
            return 0;
        case DELETE_FAST:
            return 0;
        case MAKE_CELL:
            return 0;
        case DELETE_DEREF:
            return 0;
        case LOAD_FROM_DICT_OR_DEREF:
            return 1;
        case LOAD_DEREF:
            return 0;
        case STORE_DEREF:
            return 1;
        case COPY_FREE_VARS:
            return 0;
        case BUILD_STRING:
            return oparg;
        case BUILD_TUPLE:
            return oparg;
        case BUILD_LIST:
            return oparg;
        case LIST_EXTEND:
            return (oparg-1) + 2;
        case SET_UPDATE:
            return (oparg-1) + 2;
        case BUILD_SET:
            return oparg;
        case BUILD_MAP:
            return oparg*2;
        case SETUP_ANNOTATIONS:
            return 0;
        case BUILD_CONST_KEY_MAP:
            return oparg + 1;
        case DICT_UPDATE:
            return 1;
        case DICT_MERGE:
            return 1;
        case MAP_ADD:
            return 2;
        case INSTRUMENTED_LOAD_SUPER_ATTR:
            return 3;
        case LOAD_SUPER_ATTR:
            return 3;
        case LOAD_SUPER_METHOD:
            return 3;
        case LOAD_ZERO_SUPER_METHOD:
            return 3;
        case LOAD_ZERO_SUPER_ATTR:
            return 3;
        case LOAD_SUPER_ATTR_ATTR:
            return 3;
        case LOAD_SUPER_ATTR_METHOD:
            return 3;
        case LOAD_ATTR:
            return 1;
        case LOAD_METHOD:
            return 1;
        case LOAD_ATTR_INSTANCE_VALUE:
            return 1;
        case LOAD_ATTR_MODULE:
            return 1;
        case LOAD_ATTR_WITH_HINT:
            return 1;
        case LOAD_ATTR_SLOT:
            return 1;
        case LOAD_ATTR_CLASS:
            return 1;
        case LOAD_ATTR_PROPERTY:
            return 1;
        case LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN:
            return 1;
        case STORE_ATTR_INSTANCE_VALUE:
            return 2;
        case STORE_ATTR_WITH_HINT:
            return 2;
        case STORE_ATTR_SLOT:
            return 2;
        case COMPARE_OP:
            return 2;
        case COMPARE_OP_FLOAT:
            return 2;
        case COMPARE_OP_INT:
            return 2;
        case COMPARE_OP_STR:
            return 2;
        case IS_OP:
            return 2;
        case CONTAINS_OP:
            return 2;
        case CHECK_EG_MATCH:
            return 2;
        case CHECK_EXC_MATCH:
            return 2;
        case IMPORT_NAME:
            return 2;
        case IMPORT_FROM:
            return 1;
        case JUMP_FORWARD:
            return 0;
        case JUMP_BACKWARD:
            return 0;
        case JUMP:
            return 0;
        case JUMP_NO_INTERRUPT:
            return 0;
        case ENTER_EXECUTOR:
            return 0;
        case POP_JUMP_IF_FALSE:
            return 1;
        case POP_JUMP_IF_TRUE:
            return 1;
        case POP_JUMP_IF_NOT_NONE:
            return 1;
        case POP_JUMP_IF_NONE:
            return 1;
        case JUMP_BACKWARD_NO_INTERRUPT:
            return 0;
        case GET_LEN:
            return 1;
        case MATCH_CLASS:
            return 3;
        case MATCH_MAPPING:
            return 1;
        case MATCH_SEQUENCE:
            return 1;
        case MATCH_KEYS:
            return 2;
        case GET_ITER:
            return 1;
        case GET_YIELD_FROM_ITER:
            return 1;
        case FOR_ITER:
            return 1;
        case INSTRUMENTED_FOR_ITER:
            return 0;
        case FOR_ITER_LIST:
            return 1;
        case FOR_ITER_TUPLE:
            return 1;
        case FOR_ITER_RANGE:
            return 1;
        case FOR_ITER_GEN:
            return 1;
        case BEFORE_ASYNC_WITH:
            return 1;
        case BEFORE_WITH:
            return 1;
        case WITH_EXCEPT_START:
            return 4;
        case SETUP_FINALLY:
            return 0;
        case SETUP_CLEANUP:
            return 0;
        case SETUP_WITH:
            return 0;
        case POP_BLOCK:
            return 0;
        case PUSH_EXC_INFO:
            return 1;
        case LOAD_ATTR_METHOD_WITH_VALUES:
            return 1;
        case LOAD_ATTR_METHOD_NO_DICT:
            return 1;
        case LOAD_ATTR_METHOD_LAZY_DICT:
            return 1;
        case KW_NAMES:
            return 0;
        case INSTRUMENTED_CALL:
            return 0;
        case CALL:
            return oparg + 2;
        case CALL_BOUND_METHOD_EXACT_ARGS:
            return oparg + 2;
        case CALL_PY_EXACT_ARGS:
            return oparg + 2;
        case CALL_PY_WITH_DEFAULTS:
            return oparg + 2;
        case CALL_NO_KW_TYPE_1:
            return oparg + 2;
        case CALL_NO_KW_STR_1:
            return oparg + 2;
        case CALL_NO_KW_TUPLE_1:
            return oparg + 2;
        case CALL_NO_KW_ALLOC_AND_ENTER_INIT:
            return oparg + 2;
        case EXIT_INIT_CHECK:
            return 1;
        case CALL_BUILTIN_CLASS:
            return oparg + 2;
        case CALL_NO_KW_BUILTIN_O:
            return oparg + 2;
        case CALL_NO_KW_BUILTIN_FAST:
            return oparg + 2;
        case CALL_BUILTIN_FAST_WITH_KEYWORDS:
            return oparg + 2;
        case CALL_NO_KW_LEN:
            return oparg + 2;
        case CALL_NO_KW_ISINSTANCE:
            return oparg + 2;
        case CALL_NO_KW_LIST_APPEND:
            return oparg + 2;
        case CALL_NO_KW_METHOD_DESCRIPTOR_O:
            return oparg + 2;
        case CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS:
            return oparg + 2;
        case CALL_NO_KW_METHOD_DESCRIPTOR_NOARGS:
            return oparg + 2;
        case CALL_NO_KW_METHOD_DESCRIPTOR_FAST:
            return oparg + 2;
        case INSTRUMENTED_CALL_FUNCTION_EX:
            return 0;
        case CALL_FUNCTION_EX:
            return ((oparg & 1) ? 1 : 0) + 3;
        case MAKE_FUNCTION:
            return 1;
        case SET_FUNCTION_ATTRIBUTE:
            return 2;
        case RETURN_GENERATOR:
            return 0;
        case BUILD_SLICE:
            return ((oparg == 3) ? 1 : 0) + 2;
        case CONVERT_VALUE:
            return 1;
        case FORMAT_SIMPLE:
            return 1;
        case FORMAT_WITH_SPEC:
            return 2;
        case COPY:
            return (oparg-1) + 1;
        case BINARY_OP:
            return 2;
        case SWAP:
            return (oparg-2) + 2;
        case INSTRUMENTED_INSTRUCTION:
            return 0;
        case INSTRUMENTED_JUMP_FORWARD:
            return 0;
        case INSTRUMENTED_JUMP_BACKWARD:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_TRUE:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_FALSE:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_NONE:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_NOT_NONE:
            return 0;
        case EXTENDED_ARG:
            return 0;
        case CACHE:
            return 0;
        case RESERVED:
            return 0;
        default:
            return -1;
    }
}
#endif

#ifndef NEED_OPCODE_METADATA
extern int _PyOpcode_num_pushed(int opcode, int oparg, bool jump);
#else
int
_PyOpcode_num_pushed(int opcode, int oparg, bool jump) {
    switch(opcode) {
        case NOP:
            return 0;
        case RESUME:
            return 0;
        case INSTRUMENTED_RESUME:
            return 0;
        case LOAD_CLOSURE:
            return 1;
        case LOAD_FAST_CHECK:
            return 1;
        case LOAD_FAST:
            return 1;
        case LOAD_FAST_AND_CLEAR:
            return 1;
        case LOAD_FAST_LOAD_FAST:
            return 2;
        case LOAD_CONST:
            return 1;
        case STORE_FAST:
            return 0;
        case STORE_FAST_MAYBE_NULL:
            return 0;
        case STORE_FAST_LOAD_FAST:
            return 1;
        case STORE_FAST_STORE_FAST:
            return 0;
        case POP_TOP:
            return 0;
        case PUSH_NULL:
            return 1;
        case END_FOR:
            return 0;
        case INSTRUMENTED_END_FOR:
            return 0;
        case END_SEND:
            return 1;
        case INSTRUMENTED_END_SEND:
            return 1;
        case UNARY_NEGATIVE:
            return 1;
        case UNARY_NOT:
            return 1;
        case UNARY_INVERT:
            return 1;
        case BINARY_OP_MULTIPLY_INT:
            return 1;
        case BINARY_OP_ADD_INT:
            return 1;
        case BINARY_OP_SUBTRACT_INT:
            return 1;
        case BINARY_OP_MULTIPLY_FLOAT:
            return 1;
        case BINARY_OP_ADD_FLOAT:
            return 1;
        case BINARY_OP_SUBTRACT_FLOAT:
            return 1;
        case BINARY_OP_ADD_UNICODE:
            return 1;
        case BINARY_OP_INPLACE_ADD_UNICODE:
            return 0;
        case BINARY_SUBSCR:
            return 1;
        case BINARY_SLICE:
            return 1;
        case STORE_SLICE:
            return 0;
        case BINARY_SUBSCR_LIST_INT:
            return 1;
        case BINARY_SUBSCR_TUPLE_INT:
            return 1;
        case BINARY_SUBSCR_DICT:
            return 1;
        case BINARY_SUBSCR_GETITEM:
            return 1;
        case LIST_APPEND:
            return (oparg-1) + 1;
        case SET_ADD:
            return (oparg-1) + 1;
        case STORE_SUBSCR:
            return 0;
        case STORE_SUBSCR_LIST_INT:
            return 0;
        case STORE_SUBSCR_DICT:
            return 0;
        case DELETE_SUBSCR:
            return 0;
        case CALL_INTRINSIC_1:
            return 1;
        case CALL_INTRINSIC_2:
            return 1;
        case RAISE_VARARGS:
            return 0;
        case INTERPRETER_EXIT:
            return 0;
        case RETURN_VALUE:
            return 0;
        case INSTRUMENTED_RETURN_VALUE:
            return 0;
        case RETURN_CONST:
            return 0;
        case INSTRUMENTED_RETURN_CONST:
            return 0;
        case GET_AITER:
            return 1;
        case GET_ANEXT:
            return 2;
        case GET_AWAITABLE:
            return 1;
        case SEND:
            return 2;
        case SEND_GEN:
            return 2;
        case INSTRUMENTED_YIELD_VALUE:
            return 1;
        case YIELD_VALUE:
            return 1;
        case POP_EXCEPT:
            return 0;
        case RERAISE:
            return oparg;
        case END_ASYNC_FOR:
            return 0;
        case CLEANUP_THROW:
            return 2;
        case LOAD_ASSERTION_ERROR:
            return 1;
        case LOAD_BUILD_CLASS:
            return 1;
        case STORE_NAME:
            return 0;
        case DELETE_NAME:
            return 0;
        case UNPACK_SEQUENCE:
            return oparg;
        case UNPACK_SEQUENCE_TWO_TUPLE:
            return oparg;
        case UNPACK_SEQUENCE_TUPLE:
            return oparg;
        case UNPACK_SEQUENCE_LIST:
            return oparg;
        case UNPACK_EX:
            return (oparg & 0xFF) + (oparg >> 8) + 1;
        case STORE_ATTR:
            return 0;
        case DELETE_ATTR:
            return 0;
        case STORE_GLOBAL:
            return 0;
        case DELETE_GLOBAL:
            return 0;
        case LOAD_LOCALS:
            return 1;
        case LOAD_NAME:
            return 1;
        case LOAD_FROM_DICT_OR_GLOBALS:
            return 1;
        case LOAD_GLOBAL:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_GLOBAL_MODULE:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_GLOBAL_BUILTIN:
            return ((oparg & 1) ? 1 : 0) + 1;
        case DELETE_FAST:
            return 0;
        case MAKE_CELL:
            return 0;
        case DELETE_DEREF:
            return 0;
        case LOAD_FROM_DICT_OR_DEREF:
            return 1;
        case LOAD_DEREF:
            return 1;
        case STORE_DEREF:
            return 0;
        case COPY_FREE_VARS:
            return 0;
        case BUILD_STRING:
            return 1;
        case BUILD_TUPLE:
            return 1;
        case BUILD_LIST:
            return 1;
        case LIST_EXTEND:
            return (oparg-1) + 1;
        case SET_UPDATE:
            return (oparg-1) + 1;
        case BUILD_SET:
            return 1;
        case BUILD_MAP:
            return 1;
        case SETUP_ANNOTATIONS:
            return 0;
        case BUILD_CONST_KEY_MAP:
            return 1;
        case DICT_UPDATE:
            return 0;
        case DICT_MERGE:
            return 0;
        case MAP_ADD:
            return 0;
        case INSTRUMENTED_LOAD_SUPER_ATTR:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_SUPER_ATTR:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_SUPER_METHOD:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ZERO_SUPER_METHOD:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ZERO_SUPER_ATTR:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_SUPER_ATTR_ATTR:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_SUPER_ATTR_METHOD:
            return 2;
        case LOAD_ATTR:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_METHOD:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_INSTANCE_VALUE:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_MODULE:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_WITH_HINT:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_SLOT:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_CLASS:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_PROPERTY:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN:
            return ((oparg & 1) ? 1 : 0) + 1;
        case STORE_ATTR_INSTANCE_VALUE:
            return 0;
        case STORE_ATTR_WITH_HINT:
            return 0;
        case STORE_ATTR_SLOT:
            return 0;
        case COMPARE_OP:
            return 1;
        case COMPARE_OP_FLOAT:
            return 1;
        case COMPARE_OP_INT:
            return 1;
        case COMPARE_OP_STR:
            return 1;
        case IS_OP:
            return 1;
        case CONTAINS_OP:
            return 1;
        case CHECK_EG_MATCH:
            return 2;
        case CHECK_EXC_MATCH:
            return 2;
        case IMPORT_NAME:
            return 1;
        case IMPORT_FROM:
            return 2;
        case JUMP_FORWARD:
            return 0;
        case JUMP_BACKWARD:
            return 0;
        case JUMP:
            return 0;
        case JUMP_NO_INTERRUPT:
            return 0;
        case ENTER_EXECUTOR:
            return 0;
        case POP_JUMP_IF_FALSE:
            return 0;
        case POP_JUMP_IF_TRUE:
            return 0;
        case POP_JUMP_IF_NOT_NONE:
            return 0;
        case POP_JUMP_IF_NONE:
            return 0;
        case JUMP_BACKWARD_NO_INTERRUPT:
            return 0;
        case GET_LEN:
            return 2;
        case MATCH_CLASS:
            return 1;
        case MATCH_MAPPING:
            return 2;
        case MATCH_SEQUENCE:
            return 2;
        case MATCH_KEYS:
            return 3;
        case GET_ITER:
            return 1;
        case GET_YIELD_FROM_ITER:
            return 1;
        case FOR_ITER:
            return 2;
        case INSTRUMENTED_FOR_ITER:
            return 0;
        case FOR_ITER_LIST:
            return 2;
        case FOR_ITER_TUPLE:
            return 2;
        case FOR_ITER_RANGE:
            return 2;
        case FOR_ITER_GEN:
            return 2;
        case BEFORE_ASYNC_WITH:
            return 2;
        case BEFORE_WITH:
            return 2;
        case WITH_EXCEPT_START:
            return 5;
        case SETUP_FINALLY:
            return 0;
        case SETUP_CLEANUP:
            return 0;
        case SETUP_WITH:
            return 0;
        case POP_BLOCK:
            return 0;
        case PUSH_EXC_INFO:
            return 2;
        case LOAD_ATTR_METHOD_WITH_VALUES:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_METHOD_NO_DICT:
            return ((oparg & 1) ? 1 : 0) + 1;
        case LOAD_ATTR_METHOD_LAZY_DICT:
            return ((oparg & 1) ? 1 : 0) + 1;
        case KW_NAMES:
            return 0;
        case INSTRUMENTED_CALL:
            return 0;
        case CALL:
            return 1;
        case CALL_BOUND_METHOD_EXACT_ARGS:
            return 1;
        case CALL_PY_EXACT_ARGS:
            return 1;
        case CALL_PY_WITH_DEFAULTS:
            return 1;
        case CALL_NO_KW_TYPE_1:
            return 1;
        case CALL_NO_KW_STR_1:
            return 1;
        case CALL_NO_KW_TUPLE_1:
            return 1;
        case CALL_NO_KW_ALLOC_AND_ENTER_INIT:
            return 1;
        case EXIT_INIT_CHECK:
            return 0;
        case CALL_BUILTIN_CLASS:
            return 1;
        case CALL_NO_KW_BUILTIN_O:
            return 1;
        case CALL_NO_KW_BUILTIN_FAST:
            return 1;
        case CALL_BUILTIN_FAST_WITH_KEYWORDS:
            return 1;
        case CALL_NO_KW_LEN:
            return 1;
        case CALL_NO_KW_ISINSTANCE:
            return 1;
        case CALL_NO_KW_LIST_APPEND:
            return 1;
        case CALL_NO_KW_METHOD_DESCRIPTOR_O:
            return 1;
        case CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS:
            return 1;
        case CALL_NO_KW_METHOD_DESCRIPTOR_NOARGS:
            return 1;
        case CALL_NO_KW_METHOD_DESCRIPTOR_FAST:
            return 1;
        case INSTRUMENTED_CALL_FUNCTION_EX:
            return 0;
        case CALL_FUNCTION_EX:
            return 1;
        case MAKE_FUNCTION:
            return 1;
        case SET_FUNCTION_ATTRIBUTE:
            return 1;
        case RETURN_GENERATOR:
            return 0;
        case BUILD_SLICE:
            return 1;
        case CONVERT_VALUE:
            return 1;
        case FORMAT_SIMPLE:
            return 1;
        case FORMAT_WITH_SPEC:
            return 1;
        case COPY:
            return (oparg-1) + 2;
        case BINARY_OP:
            return 1;
        case SWAP:
            return (oparg-2) + 2;
        case INSTRUMENTED_INSTRUCTION:
            return 0;
        case INSTRUMENTED_JUMP_FORWARD:
            return 0;
        case INSTRUMENTED_JUMP_BACKWARD:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_TRUE:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_FALSE:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_NONE:
            return 0;
        case INSTRUMENTED_POP_JUMP_IF_NOT_NONE:
            return 0;
        case EXTENDED_ARG:
            return 0;
        case CACHE:
            return 0;
        case RESERVED:
            return 0;
        default:
            return -1;
    }
}
#endif

enum InstructionFormat { INSTR_FMT_IB, INSTR_FMT_IBC, INSTR_FMT_IBC00, INSTR_FMT_IBC000, INSTR_FMT_IBC00000000, INSTR_FMT_IX, INSTR_FMT_IXC, INSTR_FMT_IXC000 };
#define HAS_ARG_FLAG (1)
#define HAS_CONST_FLAG (2)
#define HAS_NAME_FLAG (4)
#define HAS_JUMP_FLAG (8)
#define IS_UOP_FLAG (16)
#define OPCODE_HAS_ARG(OP) (_PyOpcode_opcode_metadata[(OP)].flags & (HAS_ARG_FLAG))
#define OPCODE_HAS_CONST(OP) (_PyOpcode_opcode_metadata[(OP)].flags & (HAS_CONST_FLAG))
#define OPCODE_HAS_NAME(OP) (_PyOpcode_opcode_metadata[(OP)].flags & (HAS_NAME_FLAG))
#define OPCODE_HAS_JUMP(OP) (_PyOpcode_opcode_metadata[(OP)].flags & (HAS_JUMP_FLAG))
#define OPCODE_IS_UOP(OP) (_PyOpcode_opcode_metadata[(OP)].flags & (IS_UOP_FLAG))
struct opcode_metadata {
    bool valid_entry;
    enum InstructionFormat instr_format;
    int flags;
};

#define OPCODE_METADATA_FMT(OP) (_PyOpcode_opcode_metadata[(OP)].instr_format)
#define SAME_OPCODE_METADATA(OP1, OP2) \
        (OPCODE_METADATA_FMT(OP1) == OPCODE_METADATA_FMT(OP2))

#ifndef NEED_OPCODE_METADATA
extern const struct opcode_metadata _PyOpcode_opcode_metadata[512];
#else
const struct opcode_metadata _PyOpcode_opcode_metadata[512] = {
    [NOP] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [RESUME] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [INSTRUMENTED_RESUME] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [LOAD_CLOSURE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [LOAD_FAST_CHECK] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [LOAD_FAST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [LOAD_FAST_AND_CLEAR] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [LOAD_FAST_LOAD_FAST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [LOAD_CONST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG | IS_UOP_FLAG },
    [STORE_FAST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [STORE_FAST_MAYBE_NULL] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [STORE_FAST_LOAD_FAST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [STORE_FAST_STORE_FAST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [POP_TOP] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [PUSH_NULL] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [END_FOR] = { true, INSTR_FMT_IB, 0 },
    [INSTRUMENTED_END_FOR] = { true, INSTR_FMT_IX, 0 },
    [END_SEND] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [INSTRUMENTED_END_SEND] = { true, INSTR_FMT_IX, 0 },
    [UNARY_NEGATIVE] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [UNARY_NOT] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [UNARY_INVERT] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [BINARY_OP_MULTIPLY_INT] = { true, INSTR_FMT_IBC, 0 },
    [BINARY_OP_ADD_INT] = { true, INSTR_FMT_IBC, 0 },
    [BINARY_OP_SUBTRACT_INT] = { true, INSTR_FMT_IBC, 0 },
    [BINARY_OP_MULTIPLY_FLOAT] = { true, INSTR_FMT_IBC, 0 },
    [BINARY_OP_ADD_FLOAT] = { true, INSTR_FMT_IBC, 0 },
    [BINARY_OP_SUBTRACT_FLOAT] = { true, INSTR_FMT_IBC, 0 },
    [BINARY_OP_ADD_UNICODE] = { true, INSTR_FMT_IBC, 0 },
    [BINARY_OP_INPLACE_ADD_UNICODE] = { true, INSTR_FMT_IB, 0 },
    [BINARY_SUBSCR] = { true, INSTR_FMT_IXC, 0 },
    [BINARY_SLICE] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [STORE_SLICE] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [BINARY_SUBSCR_LIST_INT] = { true, INSTR_FMT_IXC, IS_UOP_FLAG },
    [BINARY_SUBSCR_TUPLE_INT] = { true, INSTR_FMT_IXC, IS_UOP_FLAG },
    [BINARY_SUBSCR_DICT] = { true, INSTR_FMT_IXC, IS_UOP_FLAG },
    [BINARY_SUBSCR_GETITEM] = { true, INSTR_FMT_IXC, 0 },
    [LIST_APPEND] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [SET_ADD] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [STORE_SUBSCR] = { true, INSTR_FMT_IXC, 0 },
    [STORE_SUBSCR_LIST_INT] = { true, INSTR_FMT_IXC, IS_UOP_FLAG },
    [STORE_SUBSCR_DICT] = { true, INSTR_FMT_IXC, IS_UOP_FLAG },
    [DELETE_SUBSCR] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [CALL_INTRINSIC_1] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [CALL_INTRINSIC_2] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [RAISE_VARARGS] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [INTERPRETER_EXIT] = { true, INSTR_FMT_IX, 0 },
    [RETURN_VALUE] = { true, INSTR_FMT_IX, 0 },
    [INSTRUMENTED_RETURN_VALUE] = { true, INSTR_FMT_IX, 0 },
    [RETURN_CONST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG },
    [INSTRUMENTED_RETURN_CONST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG },
    [GET_AITER] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [GET_ANEXT] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [GET_AWAITABLE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [SEND] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [SEND_GEN] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG },
    [INSTRUMENTED_YIELD_VALUE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [YIELD_VALUE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [POP_EXCEPT] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [RERAISE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [END_ASYNC_FOR] = { true, INSTR_FMT_IX, 0 },
    [CLEANUP_THROW] = { true, INSTR_FMT_IX, 0 },
    [LOAD_ASSERTION_ERROR] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [LOAD_BUILD_CLASS] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [STORE_NAME] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | IS_UOP_FLAG },
    [DELETE_NAME] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | IS_UOP_FLAG },
    [UNPACK_SEQUENCE] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG },
    [UNPACK_SEQUENCE_TWO_TUPLE] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | IS_UOP_FLAG },
    [UNPACK_SEQUENCE_TUPLE] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | IS_UOP_FLAG },
    [UNPACK_SEQUENCE_LIST] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | IS_UOP_FLAG },
    [UNPACK_EX] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [STORE_ATTR] = { true, INSTR_FMT_IBC000, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [DELETE_ATTR] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | IS_UOP_FLAG },
    [STORE_GLOBAL] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | IS_UOP_FLAG },
    [DELETE_GLOBAL] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG | IS_UOP_FLAG },
    [LOAD_LOCALS] = { true, INSTR_FMT_IB, 0 },
    [LOAD_NAME] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_FROM_DICT_OR_GLOBALS] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_GLOBAL] = { true, INSTR_FMT_IBC000, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_GLOBAL_MODULE] = { true, INSTR_FMT_IBC000, HAS_ARG_FLAG },
    [LOAD_GLOBAL_BUILTIN] = { true, INSTR_FMT_IBC000, HAS_ARG_FLAG },
    [DELETE_FAST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [MAKE_CELL] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [DELETE_DEREF] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [LOAD_FROM_DICT_OR_DEREF] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [LOAD_DEREF] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [STORE_DEREF] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [COPY_FREE_VARS] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [BUILD_STRING] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [BUILD_TUPLE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [BUILD_LIST] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [LIST_EXTEND] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [SET_UPDATE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [BUILD_SET] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [BUILD_MAP] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [SETUP_ANNOTATIONS] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [BUILD_CONST_KEY_MAP] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [DICT_UPDATE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [DICT_MERGE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [MAP_ADD] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [INSTRUMENTED_LOAD_SUPER_ATTR] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_SUPER_ATTR] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_SUPER_METHOD] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_ZERO_SUPER_METHOD] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_ZERO_SUPER_ATTR] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_SUPER_ATTR_ATTR] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG | IS_UOP_FLAG },
    [LOAD_SUPER_ATTR_METHOD] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_NAME_FLAG | IS_UOP_FLAG },
    [LOAD_ATTR] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_METHOD] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_ATTR_INSTANCE_VALUE] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_ATTR_MODULE] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_ATTR_WITH_HINT] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [LOAD_ATTR_SLOT] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_ATTR_CLASS] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_ATTR_PROPERTY] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [STORE_ATTR_INSTANCE_VALUE] = { true, INSTR_FMT_IXC000, 0 },
    [STORE_ATTR_WITH_HINT] = { true, INSTR_FMT_IBC000, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [STORE_ATTR_SLOT] = { true, INSTR_FMT_IXC000, 0 },
    [COMPARE_OP] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG },
    [COMPARE_OP_FLOAT] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | IS_UOP_FLAG },
    [COMPARE_OP_INT] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | IS_UOP_FLAG },
    [COMPARE_OP_STR] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | IS_UOP_FLAG },
    [IS_OP] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [CONTAINS_OP] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [CHECK_EG_MATCH] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [CHECK_EXC_MATCH] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [IMPORT_NAME] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [IMPORT_FROM] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_NAME_FLAG },
    [JUMP_FORWARD] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [JUMP_BACKWARD] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [JUMP] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [JUMP_NO_INTERRUPT] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [ENTER_EXECUTOR] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [POP_JUMP_IF_FALSE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [POP_JUMP_IF_TRUE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [POP_JUMP_IF_NOT_NONE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [POP_JUMP_IF_NONE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [JUMP_BACKWARD_NO_INTERRUPT] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [GET_LEN] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [MATCH_CLASS] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [MATCH_MAPPING] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [MATCH_SEQUENCE] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [MATCH_KEYS] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [GET_ITER] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [GET_YIELD_FROM_ITER] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [FOR_ITER] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [INSTRUMENTED_FOR_ITER] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [FOR_ITER_LIST] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [FOR_ITER_TUPLE] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [FOR_ITER_RANGE] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG | HAS_JUMP_FLAG },
    [FOR_ITER_GEN] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG },
    [BEFORE_ASYNC_WITH] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [BEFORE_WITH] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [WITH_EXCEPT_START] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [SETUP_FINALLY] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [SETUP_CLEANUP] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [SETUP_WITH] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [POP_BLOCK] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [PUSH_EXC_INFO] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [LOAD_ATTR_METHOD_WITH_VALUES] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_ATTR_METHOD_NO_DICT] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [LOAD_ATTR_METHOD_LAZY_DICT] = { true, INSTR_FMT_IBC00000000, HAS_ARG_FLAG },
    [KW_NAMES] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG },
    [INSTRUMENTED_CALL] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [CALL] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_BOUND_METHOD_EXACT_ARGS] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_PY_EXACT_ARGS] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_PY_WITH_DEFAULTS] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_TYPE_1] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_STR_1] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_TUPLE_1] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_ALLOC_AND_ENTER_INIT] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [EXIT_INIT_CHECK] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [CALL_BUILTIN_CLASS] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_BUILTIN_O] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_BUILTIN_FAST] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_BUILTIN_FAST_WITH_KEYWORDS] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_LEN] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_ISINSTANCE] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_LIST_APPEND] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_METHOD_DESCRIPTOR_O] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_METHOD_DESCRIPTOR_NOARGS] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [CALL_NO_KW_METHOD_DESCRIPTOR_FAST] = { true, INSTR_FMT_IBC00, HAS_ARG_FLAG },
    [INSTRUMENTED_CALL_FUNCTION_EX] = { true, INSTR_FMT_IX, 0 },
    [CALL_FUNCTION_EX] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [MAKE_FUNCTION] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [SET_FUNCTION_ATTRIBUTE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [RETURN_GENERATOR] = { true, INSTR_FMT_IX, 0 },
    [BUILD_SLICE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [CONVERT_VALUE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [FORMAT_SIMPLE] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [FORMAT_WITH_SPEC] = { true, INSTR_FMT_IX, IS_UOP_FLAG },
    [COPY] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [BINARY_OP] = { true, INSTR_FMT_IBC, HAS_ARG_FLAG },
    [SWAP] = { true, INSTR_FMT_IB, HAS_ARG_FLAG | IS_UOP_FLAG },
    [INSTRUMENTED_INSTRUCTION] = { true, INSTR_FMT_IX, 0 },
    [INSTRUMENTED_JUMP_FORWARD] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [INSTRUMENTED_JUMP_BACKWARD] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [INSTRUMENTED_POP_JUMP_IF_TRUE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [INSTRUMENTED_POP_JUMP_IF_FALSE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [INSTRUMENTED_POP_JUMP_IF_NONE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [INSTRUMENTED_POP_JUMP_IF_NOT_NONE] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [EXTENDED_ARG] = { true, INSTR_FMT_IB, HAS_ARG_FLAG },
    [CACHE] = { true, INSTR_FMT_IX, 0 },
    [RESERVED] = { true, INSTR_FMT_IX, 0 },
};
#endif
