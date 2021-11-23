#ifndef Py_INTERNAL_TYPEOBJECT_H
#define Py_INTERNAL_TYPEOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif

extern PyStatus _PyType_InitState(PyInterpreterState *interp);
extern void _PyType_FiniState(PyInterpreterState *interp);

#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_TYPEOBJECT_H */
