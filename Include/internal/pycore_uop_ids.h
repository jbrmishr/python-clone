// This file is generated by Tools/cases_generator/uop_id_generator.py
// from:
//   ['./Python/bytecodes.c']
// Do not edit!
#ifndef Py_CORE_UOP_IDS_H
#define Py_CORE_UOP_IDS_H
#ifdef __cplusplus
extern "C" {
#endif
#define _EXIT_TRACE 300
#define _SET_IP 301
#define _NOP NOP
#define _RESUME RESUME
#define _RESUME_CHECK RESUME_CHECK
#define _INSTRUMENTED_RESUME INSTRUMENTED_RESUME
#define _LOAD_FAST_CHECK LOAD_FAST_CHECK
#define _LOAD_FAST LOAD_FAST
#define _LOAD_FAST_AND_CLEAR LOAD_FAST_AND_CLEAR
#define _LOAD_FAST_LOAD_FAST LOAD_FAST_LOAD_FAST
#define _LOAD_CONST LOAD_CONST
#define _STORE_FAST STORE_FAST
#define _STORE_FAST_LOAD_FAST STORE_FAST_LOAD_FAST
#define _STORE_FAST_STORE_FAST STORE_FAST_STORE_FAST
#define _POP_TOP POP_TOP
#define _PUSH_NULL PUSH_NULL
#define _INSTRUMENTED_END_FOR INSTRUMENTED_END_FOR
#define _END_SEND END_SEND
#define _INSTRUMENTED_END_SEND INSTRUMENTED_END_SEND
#define _UNARY_NEGATIVE UNARY_NEGATIVE
#define _UNARY_NOT UNARY_NOT
#define _SPECIALIZE_TO_BOOL 302
#define _TO_BOOL 303
#define _TO_BOOL_BOOL TO_BOOL_BOOL
#define _TO_BOOL_INT TO_BOOL_INT
#define _TO_BOOL_LIST TO_BOOL_LIST
#define _TO_BOOL_NONE TO_BOOL_NONE
#define _TO_BOOL_STR TO_BOOL_STR
#define _TO_BOOL_ALWAYS_TRUE TO_BOOL_ALWAYS_TRUE
#define _UNARY_INVERT UNARY_INVERT
#define _GUARD_BOTH_INT 304
#define _BINARY_OP_MULTIPLY_INT 305
#define _BINARY_OP_ADD_INT 306
#define _BINARY_OP_SUBTRACT_INT 307
#define _GUARD_BOTH_FLOAT 308
#define _BINARY_OP_MULTIPLY_FLOAT 309
#define _BINARY_OP_ADD_FLOAT 310
#define _BINARY_OP_SUBTRACT_FLOAT 311
#define _GUARD_BOTH_UNICODE 312
#define _BINARY_OP_ADD_UNICODE 313
#define _BINARY_OP_INPLACE_ADD_UNICODE 314
#define _SPECIALIZE_BINARY_SUBSCR 315
#define _BINARY_SUBSCR 316
#define _BINARY_SLICE BINARY_SLICE
#define _STORE_SLICE STORE_SLICE
#define _BINARY_SUBSCR_LIST_INT BINARY_SUBSCR_LIST_INT
#define _BINARY_SUBSCR_STR_INT BINARY_SUBSCR_STR_INT
#define _BINARY_SUBSCR_TUPLE_INT BINARY_SUBSCR_TUPLE_INT
#define _BINARY_SUBSCR_DICT BINARY_SUBSCR_DICT
#define _BINARY_SUBSCR_GETITEM BINARY_SUBSCR_GETITEM
#define _LIST_APPEND LIST_APPEND
#define _SET_ADD SET_ADD
#define _SPECIALIZE_STORE_SUBSCR 317
#define _STORE_SUBSCR 318
#define _STORE_SUBSCR_LIST_INT STORE_SUBSCR_LIST_INT
#define _STORE_SUBSCR_DICT STORE_SUBSCR_DICT
#define _DELETE_SUBSCR DELETE_SUBSCR
#define _CALL_INTRINSIC_1 CALL_INTRINSIC_1
#define _CALL_INTRINSIC_2 CALL_INTRINSIC_2
#define _RAISE_VARARGS RAISE_VARARGS
#define _INTERPRETER_EXIT INTERPRETER_EXIT
#define _POP_FRAME 319
#define _INSTRUMENTED_RETURN_VALUE INSTRUMENTED_RETURN_VALUE
#define _INSTRUMENTED_RETURN_CONST INSTRUMENTED_RETURN_CONST
#define _GET_AITER GET_AITER
#define _GET_ANEXT GET_ANEXT
#define _GET_AWAITABLE GET_AWAITABLE
#define _SPECIALIZE_SEND 320
#define _SEND 321
#define _SEND_GEN SEND_GEN
#define _INSTRUMENTED_YIELD_VALUE INSTRUMENTED_YIELD_VALUE
#define _YIELD_VALUE YIELD_VALUE
#define _POP_EXCEPT POP_EXCEPT
#define _RERAISE RERAISE
#define _END_ASYNC_FOR END_ASYNC_FOR
#define _CLEANUP_THROW CLEANUP_THROW
#define _LOAD_ASSERTION_ERROR LOAD_ASSERTION_ERROR
#define _LOAD_BUILD_CLASS LOAD_BUILD_CLASS
#define _STORE_NAME STORE_NAME
#define _DELETE_NAME DELETE_NAME
#define _SPECIALIZE_UNPACK_SEQUENCE 322
#define _UNPACK_SEQUENCE 323
#define _UNPACK_SEQUENCE_TWO_TUPLE UNPACK_SEQUENCE_TWO_TUPLE
#define _UNPACK_SEQUENCE_TUPLE UNPACK_SEQUENCE_TUPLE
#define _UNPACK_SEQUENCE_LIST UNPACK_SEQUENCE_LIST
#define _UNPACK_EX UNPACK_EX
#define _SPECIALIZE_STORE_ATTR 324
#define _STORE_ATTR 325
#define _DELETE_ATTR DELETE_ATTR
#define _STORE_GLOBAL STORE_GLOBAL
#define _DELETE_GLOBAL DELETE_GLOBAL
#define _LOAD_LOCALS LOAD_LOCALS
#define _LOAD_FROM_DICT_OR_GLOBALS LOAD_FROM_DICT_OR_GLOBALS
#define _LOAD_NAME LOAD_NAME
#define _SPECIALIZE_LOAD_GLOBAL 326
#define _LOAD_GLOBAL 327
#define _GUARD_GLOBALS_VERSION 328
#define _GUARD_BUILTINS_VERSION 329
#define _LOAD_GLOBAL_MODULE 330
#define _LOAD_GLOBAL_BUILTINS 331
#define _DELETE_FAST DELETE_FAST
#define _MAKE_CELL MAKE_CELL
#define _DELETE_DEREF DELETE_DEREF
#define _LOAD_FROM_DICT_OR_DEREF LOAD_FROM_DICT_OR_DEREF
#define _LOAD_DEREF LOAD_DEREF
#define _STORE_DEREF STORE_DEREF
#define _COPY_FREE_VARS COPY_FREE_VARS
#define _BUILD_STRING BUILD_STRING
#define _BUILD_TUPLE BUILD_TUPLE
#define _BUILD_LIST BUILD_LIST
#define _LIST_EXTEND LIST_EXTEND
#define _SET_UPDATE SET_UPDATE
#define _BUILD_SET BUILD_SET
#define _BUILD_MAP BUILD_MAP
#define _SETUP_ANNOTATIONS SETUP_ANNOTATIONS
#define _BUILD_CONST_KEY_MAP BUILD_CONST_KEY_MAP
#define _DICT_UPDATE DICT_UPDATE
#define _DICT_MERGE DICT_MERGE
#define _MAP_ADD MAP_ADD
#define _INSTRUMENTED_LOAD_SUPER_ATTR INSTRUMENTED_LOAD_SUPER_ATTR
#define _SPECIALIZE_LOAD_SUPER_ATTR 332
#define _LOAD_SUPER_ATTR 333
#define _LOAD_SUPER_ATTR_ATTR LOAD_SUPER_ATTR_ATTR
#define _LOAD_SUPER_ATTR_METHOD LOAD_SUPER_ATTR_METHOD
#define _SPECIALIZE_LOAD_ATTR 334
#define _LOAD_ATTR 335
#define _GUARD_TYPE_VERSION 336
#define _CHECK_MANAGED_OBJECT_HAS_VALUES 337
#define _LOAD_ATTR_INSTANCE_VALUE 338
#define _CHECK_ATTR_MODULE 339
#define _LOAD_ATTR_MODULE 340
#define _CHECK_ATTR_WITH_HINT 341
#define _LOAD_ATTR_WITH_HINT 342
#define _LOAD_ATTR_SLOT 343
#define _CHECK_ATTR_CLASS 344
#define _LOAD_ATTR_CLASS 345
#define _LOAD_ATTR_PROPERTY LOAD_ATTR_PROPERTY
#define _LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN LOAD_ATTR_GETATTRIBUTE_OVERRIDDEN
#define _GUARD_DORV_VALUES 346
#define _STORE_ATTR_INSTANCE_VALUE 347
#define _STORE_ATTR_WITH_HINT STORE_ATTR_WITH_HINT
#define _STORE_ATTR_SLOT 348
#define _SPECIALIZE_COMPARE_OP 349
#define _COMPARE_OP 350
#define _COMPARE_OP_FLOAT COMPARE_OP_FLOAT
#define _COMPARE_OP_INT COMPARE_OP_INT
#define _COMPARE_OP_STR COMPARE_OP_STR
#define _IS_OP IS_OP
#define _CONTAINS_OP CONTAINS_OP
#define _CHECK_EG_MATCH CHECK_EG_MATCH
#define _CHECK_EXC_MATCH CHECK_EXC_MATCH
#define _IMPORT_NAME IMPORT_NAME
#define _IMPORT_FROM IMPORT_FROM
#define _JUMP_FORWARD JUMP_FORWARD
#define _JUMP_BACKWARD JUMP_BACKWARD
#define _ENTER_EXECUTOR ENTER_EXECUTOR
#define _POP_JUMP_IF_FALSE 351
#define _POP_JUMP_IF_TRUE 352
#define _IS_NONE 353
#define _JUMP_BACKWARD_NO_INTERRUPT JUMP_BACKWARD_NO_INTERRUPT
#define _GET_LEN GET_LEN
#define _MATCH_CLASS MATCH_CLASS
#define _MATCH_MAPPING MATCH_MAPPING
#define _MATCH_SEQUENCE MATCH_SEQUENCE
#define _MATCH_KEYS MATCH_KEYS
#define _GET_ITER GET_ITER
#define _GET_YIELD_FROM_ITER GET_YIELD_FROM_ITER
#define _SPECIALIZE_FOR_ITER 354
#define _FOR_ITER 355
#define _FOR_ITER_TIER_TWO 356
#define _INSTRUMENTED_FOR_ITER INSTRUMENTED_FOR_ITER
#define _ITER_CHECK_LIST 357
#define _ITER_JUMP_LIST 358
#define _GUARD_NOT_EXHAUSTED_LIST 359
#define _ITER_NEXT_LIST 360
#define _ITER_CHECK_TUPLE 361
#define _ITER_JUMP_TUPLE 362
#define _GUARD_NOT_EXHAUSTED_TUPLE 363
#define _ITER_NEXT_TUPLE 364
#define _ITER_CHECK_RANGE 365
#define _ITER_JUMP_RANGE 366
#define _GUARD_NOT_EXHAUSTED_RANGE 367
#define _ITER_NEXT_RANGE 368
#define _FOR_ITER_GEN FOR_ITER_GEN
#define _BEFORE_ASYNC_WITH BEFORE_ASYNC_WITH
#define _BEFORE_WITH BEFORE_WITH
#define _WITH_EXCEPT_START WITH_EXCEPT_START
#define _PUSH_EXC_INFO PUSH_EXC_INFO
#define _GUARD_DORV_VALUES_INST_ATTR_FROM_DICT 369
#define _GUARD_KEYS_VERSION 370
#define _LOAD_ATTR_METHOD_WITH_VALUES 371
#define _LOAD_ATTR_METHOD_NO_DICT 372
#define _LOAD_ATTR_NONDESCRIPTOR_WITH_VALUES 373
#define _LOAD_ATTR_NONDESCRIPTOR_NO_DICT 374
#define _CHECK_ATTR_METHOD_LAZY_DICT 375
#define _LOAD_ATTR_METHOD_LAZY_DICT 376
#define _INSTRUMENTED_CALL INSTRUMENTED_CALL
#define _SPECIALIZE_CALL 377
#define _CALL 378
#define _CHECK_CALL_BOUND_METHOD_EXACT_ARGS 379
#define _INIT_CALL_BOUND_METHOD_EXACT_ARGS 380
#define _CHECK_PEP_523 381
#define _CHECK_FUNCTION_EXACT_ARGS 382
#define _CHECK_STACK_SPACE 383
#define _INIT_CALL_PY_EXACT_ARGS 384
#define _PUSH_FRAME 385
#define _CALL_PY_WITH_DEFAULTS CALL_PY_WITH_DEFAULTS
#define _CALL_TYPE_1 CALL_TYPE_1
#define _CALL_STR_1 CALL_STR_1
#define _CALL_TUPLE_1 CALL_TUPLE_1
#define _CALL_ALLOC_AND_ENTER_INIT CALL_ALLOC_AND_ENTER_INIT
#define _EXIT_INIT_CHECK EXIT_INIT_CHECK
#define _CALL_BUILTIN_CLASS CALL_BUILTIN_CLASS
#define _CALL_BUILTIN_O CALL_BUILTIN_O
#define _CALL_BUILTIN_FAST CALL_BUILTIN_FAST
#define _CALL_BUILTIN_FAST_WITH_KEYWORDS CALL_BUILTIN_FAST_WITH_KEYWORDS
#define _CALL_LEN CALL_LEN
#define _CALL_ISINSTANCE CALL_ISINSTANCE
#define _CALL_LIST_APPEND CALL_LIST_APPEND
#define _CALL_METHOD_DESCRIPTOR_O CALL_METHOD_DESCRIPTOR_O
#define _CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS CALL_METHOD_DESCRIPTOR_FAST_WITH_KEYWORDS
#define _CALL_METHOD_DESCRIPTOR_NOARGS CALL_METHOD_DESCRIPTOR_NOARGS
#define _CALL_METHOD_DESCRIPTOR_FAST CALL_METHOD_DESCRIPTOR_FAST
#define _INSTRUMENTED_CALL_KW INSTRUMENTED_CALL_KW
#define _CALL_KW CALL_KW
#define _INSTRUMENTED_CALL_FUNCTION_EX INSTRUMENTED_CALL_FUNCTION_EX
#define _CALL_FUNCTION_EX CALL_FUNCTION_EX
#define _MAKE_FUNCTION MAKE_FUNCTION
#define _SET_FUNCTION_ATTRIBUTE SET_FUNCTION_ATTRIBUTE
#define _RETURN_GENERATOR RETURN_GENERATOR
#define _BUILD_SLICE BUILD_SLICE
#define _CONVERT_VALUE CONVERT_VALUE
#define _FORMAT_SIMPLE FORMAT_SIMPLE
#define _FORMAT_WITH_SPEC FORMAT_WITH_SPEC
#define _COPY COPY
#define _SPECIALIZE_BINARY_OP 386
#define _BINARY_OP 387
#define _SWAP SWAP
#define _INSTRUMENTED_INSTRUCTION INSTRUMENTED_INSTRUCTION
#define _INSTRUMENTED_JUMP_FORWARD INSTRUMENTED_JUMP_FORWARD
#define _INSTRUMENTED_JUMP_BACKWARD INSTRUMENTED_JUMP_BACKWARD
#define _INSTRUMENTED_POP_JUMP_IF_TRUE INSTRUMENTED_POP_JUMP_IF_TRUE
#define _INSTRUMENTED_POP_JUMP_IF_FALSE INSTRUMENTED_POP_JUMP_IF_FALSE
#define _INSTRUMENTED_POP_JUMP_IF_NONE INSTRUMENTED_POP_JUMP_IF_NONE
#define _INSTRUMENTED_POP_JUMP_IF_NOT_NONE INSTRUMENTED_POP_JUMP_IF_NOT_NONE
#define _GUARD_IS_TRUE_POP 388
#define _GUARD_IS_FALSE_POP 389
#define _GUARD_IS_NONE_POP 390
#define _GUARD_IS_NOT_NONE_POP 391
#define _JUMP_TO_TOP 392
#define _SAVE_RETURN_OFFSET 393
#define _INSERT 394
#define _CHECK_VALIDITY 395

#ifdef __cplusplus
}
#endif
#endif /* !Py_OPCODE_IDS_H */
