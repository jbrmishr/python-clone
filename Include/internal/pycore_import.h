#ifndef Py_LIMITED_API
#ifndef Py_INTERNAL_IMPORT_H
#define Py_INTERNAL_IMPORT_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_FORK
extern PyStatus _PyImport_ReInitLock(void);
#endif
extern PyObject* _PyImport_BootstrapImp(PyThreadState *tstate);

struct _module_alias {
    const char *name;                 /* ASCII encoded string */
    const char *orig;                 /* ASCII encoded string */
};

extern const struct _frozen * _PyImport_FrozenStdlib;
extern const struct _module_alias * _PyImport_FrozenAliases;

#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_IMPORT_H */
#endif /* !Py_LIMITED_API */
