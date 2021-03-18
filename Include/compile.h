#ifndef Py_COMPILE_H
#define Py_COMPILE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_LIMITED_API
#  include  "cpython/compile.h"
#endif

#ifdef __cplusplus
}
#endif

/* These definitions must match corresponding definitions in graminit.h. */
#define Py_single_input 256
#define Py_file_input 257
#define Py_eval_input 258
#define Py_func_type_input 345

/* This doesn't need to match anything */
#define Py_fstring_input 800

#endif /* !Py_COMPILE_H */
