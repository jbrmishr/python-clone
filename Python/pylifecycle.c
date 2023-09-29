/* Python interpreter top-level routines, including init/exit */

#include "Python.h"

#include "pycore_call.h"          // _PyObject_CallMethod()
#include "pycore_ceval.h"         // _PyEval_FiniGIL()
#include "pycore_codecs.h"        // _PyCodec_Lookup()
#include "pycore_context.h"       // _PyContext_Init()
#include "pycore_dict.h"          // _PyDict_Fini()
#include "pycore_exceptions.h"    // _PyExc_InitTypes()
#include "pycore_fileutils.h"     // _Py_ResetForceASCII()
#include "pycore_floatobject.h"   // _PyFloat_InitTypes()
#include "pycore_genobject.h"     // _PyAsyncGen_Fini()
#include "pycore_global_objects_fini_generated.h"  // "_PyStaticObjects_CheckRefcnt()
#include "pycore_import.h"        // _PyImport_BootstrapImp()
#include "pycore_initconfig.h"    // _PyStatus_OK()
#include "pycore_list.h"          // _PyList_Fini()
#include "pycore_long.h"          // _PyLong_InitTypes()
#include "pycore_object.h"        // _PyDebug_PrintTotalRefs()
#include "pycore_pathconfig.h"    // _PyPathConfig_UpdateGlobal()
#include "pycore_pyerrors.h"      // _PyErr_Occurred()
#include "pycore_pylifecycle.h"   // _PyErr_Print()
#include "pycore_pymem.h"         // _PyObject_DebugMallocStats()
#include "pycore_pystate.h"       // _PyThreadState_GET()
#include "pycore_runtime.h"       // _Py_ID()
#include "pycore_runtime_init.h"  // _PyRuntimeState_INIT
#include "pycore_setobject.h"     // _PySet_NextEntry()
#include "pycore_sliceobject.h"   // _PySlice_Fini()
#include "pycore_sysmodule.h"     // _PySys_ClearAuditHooks()
#include "pycore_traceback.h"     // _Py_DumpTracebackThreads()
#include "pycore_typeobject.h"    // _PyTypes_InitTypes()
#include "pycore_typevarobject.h" // _Py_clear_generic_types()
#include "pycore_unicodeobject.h" // _PyUnicode_InitTypes()
#include "pycore_weakref.h"       // _PyWeakref_GET_REF()

#include "opcode.h"

#include <locale.h>               // setlocale()
#include <stdlib.h>               // getenv()

#if defined(__APPLE__)
#  include <mach-o/loader.h>
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h>             // SIG_IGN
#endif

#ifdef HAVE_LANGINFO_H
#  include <langinfo.h>           // nl_langinfo(CODESET)
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>              // F_GETFD
#endif

#ifdef MS_WINDOWS
#  undef BYTE
#endif

#define PUTS(fd, str) (void)_Py_write_noraise(fd, str, (int)strlen(str))


/* Forward declarations */
static PyStatus add_main_module(PyInterpreterState *interp);
static PyStatus init_import_site(void);
static PyStatus init_set_builtins_open(void);
static PyStatus init_sys_streams(PyThreadState *tstate);
static void wait_for_thread_shutdown(PyThreadState *tstate);
static void call_ll_exitfuncs(_PyRuntimeState *runtime);

/* The following places the `_PyRuntime` structure in a location that can be
 * found without any external information. This is meant to ease access to the
 * interpreter state for various runtime debugging tools, but is *not* an
 * officially supported feature */

/* Suppress deprecation warning for PyBytesObject.ob_shash */
_Py_COMP_DIAG_PUSH
_Py_COMP_DIAG_IGNORE_DEPR_DECLS

#if defined(MS_WINDOWS)

#pragma section("PyRuntime", read, write)
__declspec(allocate("PyRuntime"))

#elif defined(__APPLE__)

__attribute__((
    section(SEG_DATA ",PyRuntime")
))

#endif

_PyRuntimeState _PyRuntime
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__ ((section (".PyRuntime")))
#endif
= _PyRuntimeState_INIT(_PyRuntime);
_Py_COMP_DIAG_POP

static int runtime_initialized = 0;

PyStatus
_PyRuntime_Initialize(void)
{
    /* XXX We only initialize once in the process, which aligns with
       the static initialization of the former globals now found in
       _PyRuntime.  However, _PyRuntime *should* be initialized with
       every Py_Initialize() call, but doing so breaks the runtime.
       This is because the runtime state is not properly finalized
       currently. */
    if (runtime_initialized) {
        return _PyStatus_OK();
    }
    runtime_initialized = 1;

    return _PyRuntimeState_Init(&_PyRuntime);
}

void
_PyRuntime_Finalize(void)
{
    _PyRuntimeState_Fini(&_PyRuntime);
    runtime_initialized = 0;
}

int
Py_IsFinalizing(void)
{
    return _PyRuntimeState_GetFinalizing(&_PyRuntime) != NULL;
}

/* Hack to force loading of object files */
int (*_PyOS_mystrnicmp_hack)(const char *, const char *, Py_ssize_t) = \
    PyOS_mystrnicmp; /* Python/pystrcmp.o */


/* APIs to access the initialization flags
 *
 * Can be called prior to Py_Initialize.
 */

int
_Py_IsCoreInitialized(void)
{
    return _PyRuntime.core_initialized;
}

int
Py_IsInitialized(void)
{
    return _PyRuntime.initialized;
}


/* Helper functions to better handle the legacy C locale
 *
 * The legacy C locale assumes ASCII as the default text encoding, which
 * causes problems not only for the CPython runtime, but also other
 * components like GNU readline.
 *
 * Accordingly, when the CLI detects it, it attempts to coerce it to a
 * more capable UTF-8 based alternative as follows:
 *
 *     if (_Py_LegacyLocaleDetected()) {
 *         _Py_CoerceLegacyLocale();
 *     }
 *
 * See the documentation of the PYTHONCOERCECLOCALE setting for more details.
 *
 * Locale coercion also impacts the default error handler for the standard
 * streams: while the usual default is "strict", the default for the legacy
 * C locale and for any of the coercion target locales is "surrogateescape".
 */

int
_Py_LegacyLocaleDetected(int warn)
{
#ifndef MS_WINDOWS
    if (!warn) {
        const char *locale_override = getenv("LC_ALL");
        if (locale_override != NULL && *locale_override != '\0') {
            /* Don't coerce C locale if the LC_ALL environment variable
               is set */
            return 0;
        }
    }

    /* On non-Windows systems, the C locale is considered a legacy locale */
    /* XXX (ncoghlan): some platforms (notably Mac OS X) don't appear to treat
     *                 the POSIX locale as a simple alias for the C locale, so
     *                 we may also want to check for that explicitly.
     */
    const char *ctype_loc = setlocale(LC_CTYPE, NULL);
    return ctype_loc != NULL && strcmp(ctype_loc, "C") == 0;
#else
    /* Windows uses code pages instead of locales, so no locale is legacy */
    return 0;
#endif
}

#ifndef MS_WINDOWS
static const char *_C_LOCALE_WARNING =
    "Python runtime initialized with LC_CTYPE=C (a locale with default ASCII "
    "encoding), which may cause Unicode compatibility problems. Using C.UTF-8, "
    "C.utf8, or UTF-8 (if available) as alternative Unicode-compatible "
    "locales is recommended.\n";

static void
emit_stderr_warning_for_legacy_locale(_PyRuntimeState *runtime)
{
    const PyPreConfig *preconfig = &runtime->preconfig;
    if (preconfig->coerce_c_locale_warn && _Py_LegacyLocaleDetected(1)) {
        PySys_FormatStderr("%s", _C_LOCALE_WARNING);
    }
}
#endif   /* !defined(MS_WINDOWS) */

typedef struct _CandidateLocale {
    const char *locale_name; /* The locale to try as a coercion target */
} _LocaleCoercionTarget;

static _LocaleCoercionTarget _TARGET_LOCALES[] = {
    {"C.UTF-8"},
    {"C.utf8"},
    {"UTF-8"},
    {NULL}
};


int
_Py_IsLocaleCoercionTarget(const char *ctype_loc)
{
    const _LocaleCoercionTarget *target = NULL;
    for (target = _TARGET_LOCALES; target->locale_name; target++) {
        if (strcmp(ctype_loc, target->locale_name) == 0) {
            return 1;
        }
    }
    return 0;
}


#ifdef PY_COERCE_C_LOCALE
static const char C_LOCALE_COERCION_WARNING[] =
    "Python detected LC_CTYPE=C: LC_CTYPE coerced to %.20s (set another locale "
    "or PYTHONCOERCECLOCALE=0 to disable this locale coercion behavior).\n";

static int
_coerce_default_locale_settings(int warn, const _LocaleCoercionTarget *target)
{
    const char *newloc = target->locale_name;

    /* Reset locale back to currently configured defaults */
    _Py_SetLocaleFromEnv(LC_ALL);

    /* Set the relevant locale environment variable */
    if (setenv("LC_CTYPE", newloc, 1)) {
        fprintf(stderr,
                "Error setting LC_CTYPE, skipping C locale coercion\n");
        return 0;
    }
    if (warn) {
        fprintf(stderr, C_LOCALE_COERCION_WARNING, newloc);
    }

    /* Reconfigure with the overridden environment variables */
    _Py_SetLocaleFromEnv(LC_ALL);
    return 1;
}
#endif

int
_Py_CoerceLegacyLocale(int warn)
{
    int coerced = 0;
#ifdef PY_COERCE_C_LOCALE
    char *oldloc = NULL;

    oldloc = _PyMem_RawStrdup(setlocale(LC_CTYPE, NULL));
    if (oldloc == NULL) {
        return coerced;
    }

    const char *locale_override = getenv("LC_ALL");
    if (locale_override == NULL || *locale_override == '\0') {
        /* LC_ALL is also not set (or is set to an empty string) */
        const _LocaleCoercionTarget *target = NULL;
        for (target = _TARGET_LOCALES; target->locale_name; target++) {
            const char *new_locale = setlocale(LC_CTYPE,
                                               target->locale_name);
            if (new_locale != NULL) {
#if !defined(_Py_FORCE_UTF8_LOCALE) && defined(HAVE_LANGINFO_H) && defined(CODESET)
                /* Also ensure that nl_langinfo works in this locale */
                char *codeset = nl_langinfo(CODESET);
                if (!codeset || *codeset == '\0') {
                    /* CODESET is not set or empty, so skip coercion */
                    new_locale = NULL;
                    _Py_SetLocaleFromEnv(LC_CTYPE);
                    continue;
                }
#endif
                /* Successfully configured locale, so make it the default */
                coerced = _coerce_default_locale_settings(warn, target);
                goto done;
            }
        }
    }
    /* No C locale warning here, as Py_Initialize will emit one later */

    setlocale(LC_CTYPE, oldloc);

done:
    PyMem_RawFree(oldloc);
#endif
    return coerced;
}

/* _Py_SetLocaleFromEnv() is a wrapper around setlocale(category, "") to
 * isolate the idiosyncrasies of different libc implementations. It reads the
 * appropriate environment variable and uses its value to select the locale for
 * 'category'. */
char *
_Py_SetLocaleFromEnv(int category)
{
    char *res;
#ifdef __ANDROID__
    const char *locale;
    const char **pvar;
#ifdef PY_COERCE_C_LOCALE
    const char *coerce_c_locale;
#endif
    const char *utf8_locale = "C.UTF-8";
    const char *env_var_set[] = {
        "LC_ALL",
        "LC_CTYPE",
        "LANG",
        NULL,
    };

    /* Android setlocale(category, "") doesn't check the environment variables
     * and incorrectly sets the "C" locale at API 24 and older APIs. We only
     * check the environment variables listed in env_var_set. */
    for (pvar=env_var_set; *pvar; pvar++) {
        locale = getenv(*pvar);
        if (locale != NULL && *locale != '\0') {
            if (strcmp(locale, utf8_locale) == 0 ||
                    strcmp(locale, "en_US.UTF-8") == 0) {
                return setlocale(category, utf8_locale);
            }
            return setlocale(category, "C");
        }
    }

    /* Android uses UTF-8, so explicitly set the locale to C.UTF-8 if none of
     * LC_ALL, LC_CTYPE, or LANG is set to a non-empty string.
     * Quote from POSIX section "8.2 Internationalization Variables":
     * "4. If the LANG environment variable is not set or is set to the empty
     * string, the implementation-defined default locale shall be used." */

#ifdef PY_COERCE_C_LOCALE
    coerce_c_locale = getenv("PYTHONCOERCECLOCALE");
    if (coerce_c_locale == NULL || strcmp(coerce_c_locale, "0") != 0) {
        /* Some other ported code may check the environment variables (e.g. in
         * extension modules), so we make sure that they match the locale
         * configuration */
        if (setenv("LC_CTYPE", utf8_locale, 1)) {
            fprintf(stderr, "Warning: failed setting the LC_CTYPE "
                            "environment variable to %s\n", utf8_locale);
        }
    }
#endif
    res = setlocale(category, utf8_locale);
#else /* !defined(__ANDROID__) */
    res = setlocale(category, "");
#endif
    _Py_ResetForceASCII();
    return res;
}


static int
interpreter_update_config(PyThreadState *tstate, int only_update_path_config)
{
    const PyConfig *config = &tstate->interp->config;

    if (!only_update_path_config) {
        PyStatus status = _PyConfig_Write(config, tstate->interp->runtime);
        if (_PyStatus_EXCEPTION(status)) {
            _PyErr_SetFromPyStatus(status);
            return -1;
        }
    }

    if (_Py_IsMainInterpreter(tstate->interp)) {
        PyStatus status = _PyPathConfig_UpdateGlobal(config);
        if (_PyStatus_EXCEPTION(status)) {
            _PyErr_SetFromPyStatus(status);
            return -1;
        }
    }

    tstate->interp->long_state.max_str_digits = config->int_max_str_digits;

    // Update the sys module for the new configuration
    if (_PySys_UpdateConfig(tstate) < 0) {
        return -1;
    }
    return 0;
}


int
_PyInterpreterState_SetConfig(const PyConfig *src_config)
{
    PyThreadState *tstate = _PyThreadState_GET();
    int res = -1;

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    PyStatus status = _PyConfig_Copy(&config, src_config);
    if (_PyStatus_EXCEPTION(status)) {
        _PyErr_SetFromPyStatus(status);
        goto done;
    }

    status = _PyConfig_Read(&config, 1);
    if (_PyStatus_EXCEPTION(status)) {
        _PyErr_SetFromPyStatus(status);
        goto done;
    }

    status = _PyConfig_Copy(&tstate->interp->config, &config);
    if (_PyStatus_EXCEPTION(status)) {
        _PyErr_SetFromPyStatus(status);
        goto done;
    }

    res = interpreter_update_config(tstate, 0);

done:
    PyConfig_Clear(&config);
    return res;
}


/* Global initializations.  Can be undone by Py_Finalize().  Don't
   call this twice without an intervening Py_Finalize() call.

   Every call to Py_InitializeFromConfig, Py_Initialize or Py_InitializeEx
   must have a corresponding call to Py_Finalize.

   Locking: you must hold the interpreter lock while calling these APIs.
   (If the lock has not yet been initialized, that's equivalent to
   having the lock, but you cannot use multiple threads.)

*/

static PyStatus
pyinit_core_reconfigure(_PyRuntimeState *runtime,
                        PyThreadState **tstate_p,
                        const PyConfig *config)
{
    PyStatus status;
    PyThreadState *tstate = _PyThreadState_GET();
    if (!tstate) {
        return _PyStatus_ERR("failed to read thread state");
    }
    *tstate_p = tstate;

    PyInterpreterState *interp = tstate->interp;
    if (interp == NULL) {
        return _PyStatus_ERR("can't make main interpreter");
    }

    status = _PyConfig_Write(config, runtime);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyConfig_Copy(&interp->config, config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    config = _PyInterpreterState_GetConfig(interp);

    if (config->_install_importlib) {
        status = _PyPathConfig_UpdateGlobal(config);
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }
    return _PyStatus_OK();
}


static PyStatus
pycore_init_runtime(_PyRuntimeState *runtime,
                    const PyConfig *config)
{
    if (runtime->initialized) {
        return _PyStatus_ERR("main interpreter already initialized");
    }

    PyStatus status = _PyConfig_Write(config, runtime);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    /* Py_Finalize leaves _Py_Finalizing set in order to help daemon
     * threads behave a little more gracefully at interpreter shutdown.
     * We clobber it here so the new interpreter can start with a clean
     * slate.
     *
     * However, this may still lead to misbehaviour if there are daemon
     * threads still hanging around from a previous Py_Initialize/Finalize
     * pair :(
     */
    _PyRuntimeState_SetFinalizing(runtime, NULL);

    _Py_InitVersion();

    status = _Py_HashRandomization_Init(config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyTime_Init();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyImport_Init();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyInterpreterState_Enable(runtime);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    return _PyStatus_OK();
}


static PyStatus
init_interp_settings(PyInterpreterState *interp,
                     const PyInterpreterConfig *config)
{
    assert(interp->feature_flags == 0);

    if (config->use_main_obmalloc) {
        interp->feature_flags |= Py_RTFLAGS_USE_MAIN_OBMALLOC;
    }
    else if (!config->check_multi_interp_extensions) {
        /* The reason: PyModuleDef.m_base.m_copy leaks objects between
           interpreters. */
        return _PyStatus_ERR("per-interpreter obmalloc does not support "
                             "single-phase init extension modules");
    }

    if (config->allow_fork) {
        interp->feature_flags |= Py_RTFLAGS_FORK;
    }
    if (config->allow_exec) {
        interp->feature_flags |= Py_RTFLAGS_EXEC;
    }
    // Note that fork+exec is always allowed.

    if (config->allow_threads) {
        interp->feature_flags |= Py_RTFLAGS_THREADS;
    }
    if (config->allow_daemon_threads) {
        interp->feature_flags |= Py_RTFLAGS_DAEMON_THREADS;
    }

    if (config->check_multi_interp_extensions) {
        interp->feature_flags |= Py_RTFLAGS_MULTI_INTERP_EXTENSIONS;
    }

    /* We check "gil" in init_interp_create_gil(). */

    return _PyStatus_OK();
}


static PyStatus
init_interp_create_gil(PyThreadState *tstate, int gil)
{
    PyStatus status;

    /* finalize_interp_delete() comment explains why _PyEval_FiniGIL() is
       only called here. */
    // XXX This is broken with a per-interpreter GIL.
    _PyEval_FiniGIL(tstate->interp);

    /* Auto-thread-state API */
    status = _PyGILState_SetTstate(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    int own_gil;
    switch (gil) {
    case PyInterpreterConfig_DEFAULT_GIL: own_gil = 0; break;
    case PyInterpreterConfig_SHARED_GIL: own_gil = 0; break;
    case PyInterpreterConfig_OWN_GIL: own_gil = 1; break;
    default:
        return _PyStatus_ERR("invalid interpreter config 'gil' value");
    }

    /* Create the GIL and take it */
    status = _PyEval_InitGIL(tstate, own_gil);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    return _PyStatus_OK();
}


static PyStatus
pycore_create_interpreter(_PyRuntimeState *runtime,
                          const PyConfig *src_config,
                          PyThreadState **tstate_p)
{
    PyStatus status;
    PyInterpreterState *interp;
    status = _PyInterpreterState_New(NULL, &interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    assert(interp != NULL);
    assert(_Py_IsMainInterpreter(interp));

    status = _PyConfig_Copy(&interp->config, src_config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    /* Auto-thread-state API */
    status = _PyGILState_Init(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    PyInterpreterConfig config = _PyInterpreterConfig_LEGACY_INIT;
    // The main interpreter always has its own GIL.
    config.gil = PyInterpreterConfig_OWN_GIL;
    status = init_interp_settings(interp, &config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    PyThreadState *tstate = _PyThreadState_New(interp);
    if (tstate == NULL) {
        return _PyStatus_ERR("can't make first thread");
    }
    _PyThreadState_Bind(tstate);
    // XXX For now we do this before the GIL is created.
    (void) _PyThreadState_SwapNoGIL(tstate);

    status = init_interp_create_gil(tstate, config.gil);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    *tstate_p = tstate;
    return _PyStatus_OK();
}


static PyStatus
pycore_init_global_objects(PyInterpreterState *interp)
{
    PyStatus status;

    _PyFloat_InitState(interp);

    status = _PyUnicode_InitGlobalObjects(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    _PyUnicode_InitState(interp);

    return _PyStatus_OK();
}


static PyStatus
pycore_init_types(PyInterpreterState *interp)
{
    PyStatus status;

    status = _PyTypes_InitTypes(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyLong_InitTypes(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyUnicode_InitTypes(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyFloat_InitTypes(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    if (_PyExc_InitTypes(interp) < 0) {
        return _PyStatus_ERR("failed to initialize an exception type");
    }

    status = _PyExc_InitGlobalObjects(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyExc_InitState(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyErr_InitTypes(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyContext_Init(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    return _PyStatus_OK();
}

static PyStatus
pycore_init_builtins(PyThreadState *tstate)
{
    PyInterpreterState *interp = tstate->interp;

    PyObject *bimod = _PyBuiltin_Init(interp);
    if (bimod == NULL) {
        goto error;
    }

    PyObject *modules = _PyImport_GetModules(interp);
    if (_PyImport_FixupBuiltin(bimod, "builtins", modules) < 0) {
        goto error;
    }

    PyObject *builtins_dict = PyModule_GetDict(bimod);
    if (builtins_dict == NULL) {
        goto error;
    }
    interp->builtins = Py_NewRef(builtins_dict);

    PyObject *isinstance = PyDict_GetItemWithError(builtins_dict, &_Py_ID(isinstance));
    if (!isinstance) {
        goto error;
    }
    interp->callable_cache.isinstance = isinstance;

    PyObject *len = PyDict_GetItemWithError(builtins_dict, &_Py_ID(len));
    if (!len) {
        goto error;
    }
    interp->callable_cache.len = len;

    PyObject *list_append = _PyType_Lookup(&PyList_Type, &_Py_ID(append));
    if (list_append == NULL) {
        goto error;
    }
    interp->callable_cache.list_append = list_append;

    PyObject *object__getattribute__ = _PyType_Lookup(&PyBaseObject_Type, &_Py_ID(__getattribute__));
    if (object__getattribute__ == NULL) {
        goto error;
    }
    interp->callable_cache.object__getattribute__ = object__getattribute__;

    if (_PyBuiltins_AddExceptions(bimod) < 0) {
        return _PyStatus_ERR("failed to add exceptions to builtins");
    }

    interp->builtins_copy = PyDict_Copy(interp->builtins);
    if (interp->builtins_copy == NULL) {
        goto error;
    }
    Py_DECREF(bimod);

    if (_PyImport_InitDefaultImportFunc(interp) < 0) {
        goto error;
    }

    assert(!_PyErr_Occurred(tstate));
    return _PyStatus_OK();

error:
    Py_XDECREF(bimod);
    return _PyStatus_ERR("can't initialize builtins module");
}


static PyStatus
pycore_interp_init(PyThreadState *tstate)
{
    PyInterpreterState *interp = tstate->interp;
    PyStatus status;
    PyObject *sysmod = NULL;

    // Create singletons before the first PyType_Ready() call, since
    // PyType_Ready() uses singletons like the Unicode empty string (tp_doc)
    // and the empty tuple singletons (tp_bases).
    status = pycore_init_global_objects(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    // The GC must be initialized before the first GC collection.
    status = _PyGC_Init(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = pycore_init_types(interp);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

    if (_PyWarnings_InitState(interp) < 0) {
        return _PyStatus_ERR("can't initialize warnings");
    }

    status = _PyAtExit_Init(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PySys_Create(tstate, &sysmod);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

    status = pycore_init_builtins(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

    const PyConfig *config = _PyInterpreterState_GetConfig(interp);

    status = _PyImport_InitCore(tstate, sysmod, config->_install_importlib);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

done:
    /* sys.modules['sys'] contains a strong reference to the module */
    Py_XDECREF(sysmod);
    return status;
}


static PyStatus
pyinit_config(_PyRuntimeState *runtime,
              PyThreadState **tstate_p,
              const PyConfig *config)
{
    PyStatus status = pycore_init_runtime(runtime, config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    PyThreadState *tstate;
    status = pycore_create_interpreter(runtime, config, &tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    *tstate_p = tstate;

    status = pycore_interp_init(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    /* Only when we get here is the runtime core fully initialized */
    runtime->core_initialized = 1;
    return _PyStatus_OK();
}


PyStatus
_Py_PreInitializeFromPyArgv(const PyPreConfig *src_config, const _PyArgv *args)
{
    PyStatus status;

    if (src_config == NULL) {
        return _PyStatus_ERR("preinitialization config is NULL");
    }

    status = _PyRuntime_Initialize();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    _PyRuntimeState *runtime = &_PyRuntime;

    if (runtime->preinitialized) {
        /* If it's already configured: ignored the new configuration */
        return _PyStatus_OK();
    }

    /* Note: preinitialized remains 1 on error, it is only set to 0
       at exit on success. */
    runtime->preinitializing = 1;

    PyPreConfig config;

    status = _PyPreConfig_InitFromPreConfig(&config, src_config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyPreConfig_Read(&config, args);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = _PyPreConfig_Write(&config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    runtime->preinitializing = 0;
    runtime->preinitialized = 1;
    return _PyStatus_OK();
}


PyStatus
Py_PreInitializeFromBytesArgs(const PyPreConfig *src_config, Py_ssize_t argc, char **argv)
{
    _PyArgv args = {.use_bytes_argv = 1, .argc = argc, .bytes_argv = argv};
    return _Py_PreInitializeFromPyArgv(src_config, &args);
}


PyStatus
Py_PreInitializeFromArgs(const PyPreConfig *src_config, Py_ssize_t argc, wchar_t **argv)
{
    _PyArgv args = {.use_bytes_argv = 0, .argc = argc, .wchar_argv = argv};
    return _Py_PreInitializeFromPyArgv(src_config, &args);
}


PyStatus
Py_PreInitialize(const PyPreConfig *src_config)
{
    return _Py_PreInitializeFromPyArgv(src_config, NULL);
}


PyStatus
_Py_PreInitializeFromConfig(const PyConfig *config,
                            const _PyArgv *args)
{
    assert(config != NULL);

    PyStatus status = _PyRuntime_Initialize();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    _PyRuntimeState *runtime = &_PyRuntime;

    if (runtime->preinitialized) {
        /* Already initialized: do nothing */
        return _PyStatus_OK();
    }

    PyPreConfig preconfig;

    _PyPreConfig_InitFromConfig(&preconfig, config);

    if (!config->parse_argv) {
        return Py_PreInitialize(&preconfig);
    }
    else if (args == NULL) {
        _PyArgv config_args = {
            .use_bytes_argv = 0,
            .argc = config->argv.length,
            .wchar_argv = config->argv.items};
        return _Py_PreInitializeFromPyArgv(&preconfig, &config_args);
    }
    else {
        return _Py_PreInitializeFromPyArgv(&preconfig, args);
    }
}


/* Begin interpreter initialization
 *
 * On return, the first thread and interpreter state have been created,
 * but the compiler, signal handling, multithreading and
 * multiple interpreter support, and codec infrastructure are not yet
 * available.
 *
 * The import system will support builtin and frozen modules only.
 * The only supported io is writing to sys.stderr
 *
 * If any operation invoked by this function fails, a fatal error is
 * issued and the function does not return.
 *
 * Any code invoked from this function should *not* assume it has access
 * to the Python C API (unless the API is explicitly listed as being
 * safe to call without calling Py_Initialize first)
 */
static PyStatus
pyinit_core(_PyRuntimeState *runtime,
            const PyConfig *src_config,
            PyThreadState **tstate_p)
{
    PyStatus status;

    status = _Py_PreInitializeFromConfig(src_config, NULL);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    status = _PyConfig_Copy(&config, src_config);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

    // Read the configuration, but don't compute the path configuration
    // (it is computed in the main init).
    status = _PyConfig_Read(&config, 0);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

    if (!runtime->core_initialized) {
        status = pyinit_config(runtime, tstate_p, &config);
    }
    else {
        status = pyinit_core_reconfigure(runtime, tstate_p, &config);
    }
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

done:
    PyConfig_Clear(&config);
    return status;
}


/* Py_Initialize() has already been called: update the main interpreter
   configuration. Example of bpo-34008: Py_Main() called after
   Py_Initialize(). */
static PyStatus
pyinit_main_reconfigure(PyThreadState *tstate)
{
    if (interpreter_update_config(tstate, 0) < 0) {
        return _PyStatus_ERR("fail to reconfigure Python");
    }
    return _PyStatus_OK();
}


static PyStatus
init_interp_main(PyThreadState *tstate)
{
    assert(!_PyErr_Occurred(tstate));

    PyStatus status;
    int is_main_interp = _Py_IsMainInterpreter(tstate->interp);
    PyInterpreterState *interp = tstate->interp;
    const PyConfig *config = _PyInterpreterState_GetConfig(interp);

    if (!config->_install_importlib) {
        /* Special mode for freeze_importlib: run with no import system
         *
         * This means anything which needs support from extension modules
         * or pure Python code in the standard library won't work.
         */
        if (is_main_interp) {
            interp->runtime->initialized = 1;
        }
        return _PyStatus_OK();
    }

    // Initialize the import-related configuration.
    status = _PyConfig_InitImportConfig(&interp->config);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    if (interpreter_update_config(tstate, 1) < 0) {
        return _PyStatus_ERR("failed to update the Python config");
    }

    status = _PyImport_InitExternal(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    if (is_main_interp) {
        /* initialize the faulthandler module */
        status = _PyFaulthandler_Init(config->faulthandler);
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }

    status = _PyUnicode_InitEncodings(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    if (is_main_interp) {
        if (_PySignal_Init(config->install_signal_handlers) < 0) {
            return _PyStatus_ERR("can't initialize signals");
        }

        if (config->tracemalloc) {
           if (_PyTraceMalloc_Start(config->tracemalloc) < 0) {
                return _PyStatus_ERR("can't start tracemalloc");
            }
        }

#ifdef PY_HAVE_PERF_TRAMPOLINE
        if (config->perf_profiling) {
            if (_PyPerfTrampoline_SetCallbacks(&_Py_perfmap_callbacks) < 0 ||
                    _PyPerfTrampoline_Init(config->perf_profiling) < 0) {
                return _PyStatus_ERR("can't initialize the perf trampoline");
            }
        }
#endif
    }

    status = init_sys_streams(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = init_set_builtins_open();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    status = add_main_module(interp);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    if (is_main_interp) {
        /* Initialize warnings. */
        PyObject *warnoptions = PySys_GetObject("warnoptions");
        if (warnoptions != NULL && PyList_Size(warnoptions) > 0)
        {
            PyObject *warnings_module = PyImport_ImportModule("warnings");
            if (warnings_module == NULL) {
                fprintf(stderr, "'import warnings' failed; traceback:\n");
                _PyErr_Print(tstate);
            }
            Py_XDECREF(warnings_module);
        }

        interp->runtime->initialized = 1;
    }

    if (config->site_import) {
        status = init_import_site();
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }

    if (is_main_interp) {
#ifndef MS_WINDOWS
        emit_stderr_warning_for_legacy_locale(interp->runtime);
#endif
    }

    // Turn on experimental tier 2 (uops-based) optimizer
    if (is_main_interp) {
        char *envvar = Py_GETENV("PYTHONUOPS");
        int enabled = envvar != NULL && *envvar > '0';
        if (_Py_get_xoption(&config->xoptions, L"uops") != NULL) {
            enabled = 1;
        }
        if (true || enabled) {  // XXX
            PyObject *opt = PyUnstable_Optimizer_NewUOpOptimizer();
            if (opt == NULL) {
                return _PyStatus_ERR("can't initialize optimizer");
            }
            PyUnstable_SetOptimizer((_PyOptimizerObject *)opt);
            Py_DECREF(opt);
        }
    }

    assert(!_PyErr_Occurred(tstate));

    return _PyStatus_OK();
}


/* Update interpreter state based on supplied configuration settings
 *
 * After calling this function, most of the restrictions on the interpreter
 * are lifted. The only remaining incomplete settings are those related
 * to the main module (sys.argv[0], __main__ metadata)
 *
 * Calling this when the interpreter is not initializing, is already
 * initialized or without a valid current thread state is a fatal error.
 * Other errors should be reported as normal Python exceptions with a
 * non-zero return code.
 */
static PyStatus
pyinit_main(PyThreadState *tstate)
{
    PyInterpreterState *interp = tstate->interp;
    if (!interp->runtime->core_initialized) {
        return _PyStatus_ERR("runtime core not initialized");
    }

    if (interp->runtime->initialized) {
        return pyinit_main_reconfigure(tstate);
    }

    PyStatus status = init_interp_main(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    return _PyStatus_OK();
}


PyStatus
Py_InitializeFromConfig(const PyConfig *config)
{
    if (config == NULL) {
        return _PyStatus_ERR("initialization config is NULL");
    }

    PyStatus status;

    status = _PyRuntime_Initialize();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    _PyRuntimeState *runtime = &_PyRuntime;

    PyThreadState *tstate = NULL;
    status = pyinit_core(runtime, config, &tstate);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    config = _PyInterpreterState_GetConfig(tstate->interp);

    if (config->_init_main) {
        status = pyinit_main(tstate);
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }

    return _PyStatus_OK();
}


void
Py_InitializeEx(int install_sigs)
{
    PyStatus status;

    status = _PyRuntime_Initialize();
    if (_PyStatus_EXCEPTION(status)) {
        Py_ExitStatusException(status);
    }
    _PyRuntimeState *runtime = &_PyRuntime;

    if (runtime->initialized) {
        /* bpo-33932: Calling Py_Initialize() twice does nothing. */
        return;
    }

    PyConfig config;
    _PyConfig_InitCompatConfig(&config);

    config.install_signal_handlers = install_sigs;

    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);
    if (_PyStatus_EXCEPTION(status)) {
        Py_ExitStatusException(status);
    }
}

void
Py_Initialize(void)
{
    Py_InitializeEx(1);
}


PyStatus
_Py_InitializeMain(void)
{
    PyStatus status = _PyRuntime_Initialize();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    PyThreadState *tstate = _PyThreadState_GET();
    return pyinit_main(tstate);
}


static void
finalize_modules_delete_special(PyThreadState *tstate, int verbose)
{
    // List of names to clear in sys
    static const char * const sys_deletes[] = {
        "path", "argv", "ps1", "ps2", "last_exc",
        "last_type", "last_value", "last_traceback",
        "__interactivehook__",
        // path_hooks and path_importer_cache are cleared
        // by _PyImport_FiniExternal().
        // XXX Clear meta_path in _PyImport_FiniCore().
        "meta_path",
        NULL
    };

    static const char * const sys_files[] = {
        "stdin", "__stdin__",
        "stdout", "__stdout__",
        "stderr", "__stderr__",
        NULL
    };

    PyInterpreterState *interp = tstate->interp;
    if (verbose) {
        PySys_WriteStderr("# clear builtins._\n");
    }
    if (PyDict_SetItemString(interp->builtins, "_", Py_None) < 0) {
        PyErr_WriteUnraisable(NULL);
    }

    const char * const *p;
    for (p = sys_deletes; *p != NULL; p++) {
        if (_PySys_ClearAttrString(interp, *p, verbose) < 0) {
            PyErr_WriteUnraisable(NULL);
        }
    }
    for (p = sys_files; *p != NULL; p+=2) {
        const char *name = p[0];
        const char *orig_name = p[1];
        if (verbose) {
            PySys_WriteStderr("# restore sys.%s\n", name);
        }
        PyObject *value;
        if (PyDict_GetItemStringRef(interp->sysdict, orig_name, &value) < 0) {
            PyErr_WriteUnraisable(NULL);
        }
        if (value == NULL) {
            value = Py_NewRef(Py_None);
        }
        if (PyDict_SetItemString(interp->sysdict, name, value) < 0) {
            PyErr_WriteUnraisable(NULL);
        }
        Py_DECREF(value);
    }
}


static PyObject*
finalize_remove_modules(PyObject *modules, int verbose)
{
    PyObject *weaklist = PyList_New(0);
    if (weaklist == NULL) {
        PyErr_WriteUnraisable(NULL);
    }

#define STORE_MODULE_WEAKREF(name, mod) \
        if (weaklist != NULL) { \
            PyObject *wr = PyWeakref_NewRef(mod, NULL); \
            if (wr) { \
                PyObject *tup = PyTuple_Pack(2, name, wr); \
                if (!tup || PyList_Append(weaklist, tup) < 0) { \
                    PyErr_WriteUnraisable(NULL); \
                } \
                Py_XDECREF(tup); \
                Py_DECREF(wr); \
            } \
            else { \
                PyErr_WriteUnraisable(NULL); \
            } \
        }

#define CLEAR_MODULE(name, mod) \
        if (PyModule_Check(mod)) { \
            if (verbose && PyUnicode_Check(name)) { \
                PySys_FormatStderr("# cleanup[2] removing %U\n", name); \
            } \
            STORE_MODULE_WEAKREF(name, mod); \
            if (PyObject_SetItem(modules, name, Py_None) < 0) { \
                PyErr_WriteUnraisable(NULL); \
            } \
        }

    if (PyDict_CheckExact(modules)) {
        Py_ssize_t pos = 0;
        PyObject *key, *value;
        while (PyDict_Next(modules, &pos, &key, &value)) {
            CLEAR_MODULE(key, value);
        }
    }
    else {
        PyObject *iterator = PyObject_GetIter(modules);
        if (iterator == NULL) {
            PyErr_WriteUnraisable(NULL);
        }
        else {
            PyObject *key;
            while ((key = PyIter_Next(iterator))) {
                PyObject *value = PyObject_GetItem(modules, key);
                if (value == NULL) {
                    PyErr_WriteUnraisable(NULL);
                    continue;
                }
                CLEAR_MODULE(key, value);
                Py_DECREF(value);
                Py_DECREF(key);
            }
            if (PyErr_Occurred()) {
                PyErr_WriteUnraisable(NULL);
            }
            Py_DECREF(iterator);
        }
    }
#undef CLEAR_MODULE
#undef STORE_MODULE_WEAKREF

    return weaklist;
}


static void
finalize_clear_modules_dict(PyObject *modules)
{
    if (PyDict_CheckExact(modules)) {
        PyDict_Clear(modules);
    }
    else {
        if (PyObject_CallMethodNoArgs(modules, &_Py_ID(clear)) == NULL) {
            PyErr_WriteUnraisable(NULL);
        }
    }
}


static void
finalize_restore_builtins(PyThreadState *tstate)
{
    PyInterpreterState *interp = tstate->interp;
    PyObject *dict = PyDict_Copy(interp->builtins);
    if (dict == NULL) {
        PyErr_WriteUnraisable(NULL);
    }
    PyDict_Clear(interp->builtins);
    if (PyDict_Update(interp->builtins, interp->builtins_copy)) {
        PyErr_WriteUnraisable(NULL);
    }
    Py_XDECREF(dict);
}


static void
finalize_modules_clear_weaklist(PyInterpreterState *interp,
                                PyObject *weaklist, int verbose)
{
    // First clear modules imported later
    for (Py_ssize_t i = PyList_GET_SIZE(weaklist) - 1; i >= 0; i--) {
        PyObject *tup = PyList_GET_ITEM(weaklist, i);
        PyObject *name = PyTuple_GET_ITEM(tup, 0);
        PyObject *mod = _PyWeakref_GET_REF(PyTuple_GET_ITEM(tup, 1));
        if (mod == NULL) {
            continue;
        }
        assert(PyModule_Check(mod));
        PyObject *dict = _PyModule_GetDict(mod);  // borrowed reference
        if (dict == interp->builtins || dict == interp->sysdict) {
            Py_DECREF(mod);
            continue;
        }
        if (verbose && PyUnicode_Check(name)) {
            PySys_FormatStderr("# cleanup[3] wiping %U\n", name);
        }
        _PyModule_Clear(mod);
        Py_DECREF(mod);
    }
}


static void
finalize_clear_sys_builtins_dict(PyInterpreterState *interp, int verbose)
{
    // Clear sys dict
    if (verbose) {
        PySys_FormatStderr("# cleanup[3] wiping sys\n");
    }
    _PyModule_ClearDict(interp->sysdict);

    // Clear builtins dict
    if (verbose) {
        PySys_FormatStderr("# cleanup[3] wiping builtins\n");
    }
    _PyModule_ClearDict(interp->builtins);
}


/* Clear modules, as good as we can */
// XXX Move most of this to import.c.
static void
finalize_modules(PyThreadState *tstate)
{
    PyInterpreterState *interp = tstate->interp;
    PyObject *modules = _PyImport_GetModules(interp);
    if (modules == NULL) {
        // Already done
        return;
    }
    int verbose = _PyInterpreterState_GetConfig(interp)->verbose;

    // Delete some special builtins._ and sys attributes first.  These are
    // common places where user values hide and people complain when their
    // destructors fail.  Since the modules containing them are
    // deleted *last* of all, they would come too late in the normal
    // destruction order.  Sigh.
    //
    // XXX Perhaps these precautions are obsolete. Who knows?
    finalize_modules_delete_special(tstate, verbose);

    // Remove all modules from sys.modules, hoping that garbage collection
    // can reclaim most of them: set all sys.modules values to None.
    //
    // We prepare a list which will receive (name, weakref) tuples of
    // modules when they are removed from sys.modules.  The name is used
    // for diagnosis messages (in verbose mode), while the weakref helps
    // detect those modules which have been held alive.
    PyObject *weaklist = finalize_remove_modules(modules, verbose);

    // Clear the modules dict
    finalize_clear_modules_dict(modules);

    // Restore the original builtins dict, to ensure that any
    // user data gets cleared.
    finalize_restore_builtins(tstate);

    // Collect garbage
    _PyGC_CollectNoFail(tstate);

    // Dump GC stats before it's too late, since it uses the warnings
    // machinery.
    _PyGC_DumpShutdownStats(interp);

    if (weaklist != NULL) {
        // Now, if there are any modules left alive, clear their globals to
        // minimize potential leaks.  All C extension modules actually end
        // up here, since they are kept alive in the interpreter state.
        //
        // The special treatment of "builtins" here is because even
        // when it's not referenced as a module, its dictionary is
        // referenced by almost every module's __builtins__.  Since
        // deleting a module clears its dictionary (even if there are
        // references left to it), we need to delete the "builtins"
        // module last.  Likewise, we don't delete sys until the very
        // end because it is implicitly referenced (e.g. by print).
        //
        // Since dict is ordered in CPython 3.6+, modules are saved in
        // importing order.  First clear modules imported later.
        finalize_modules_clear_weaklist(interp, weaklist, verbose);
        Py_DECREF(weaklist);
    }

    // Clear sys and builtins modules dict
    finalize_clear_sys_builtins_dict(interp, verbose);

    // Clear module dict copies stored in the interpreter state:
    // clear PyInterpreterState.modules_by_index and
    // clear PyModuleDef.m_base.m_copy (of extensions not using the multi-phase
    // initialization API)
    _PyImport_ClearModulesByIndex(interp);

    // Clear and delete the modules directory.  Actual modules will
    // still be there only if imported during the execution of some
    // destructor.
    _PyImport_ClearModules(interp);

    // Collect garbage once more
    _PyGC_CollectNoFail(tstate);
}


/* Flush stdout and stderr */

static int
file_is_closed(PyObject *fobj)
{
    int r;
    PyObject *tmp = PyObject_GetAttrString(fobj, "closed");
    if (tmp == NULL) {
        PyErr_Clear();
        return 0;
    }
    r = PyObject_IsTrue(tmp);
    Py_DECREF(tmp);
    if (r < 0)
        PyErr_Clear();
    return r > 0;
}


static int
flush_std_files(void)
{
    PyThreadState *tstate = _PyThreadState_GET();
    PyObject *fout = _PySys_GetAttr(tstate, &_Py_ID(stdout));
    PyObject *ferr = _PySys_GetAttr(tstate, &_Py_ID(stderr));
    int status = 0;

    if (fout != NULL && fout != Py_None && !file_is_closed(fout)) {
        if (_PyFile_Flush(fout) < 0) {
            PyErr_WriteUnraisable(fout);
            status = -1;
        }
    }

    if (ferr != NULL && ferr != Py_None && !file_is_closed(ferr)) {
        if (_PyFile_Flush(ferr) < 0) {
            PyErr_Clear();
            status = -1;
        }
    }

    return status;
}

/* Undo the effect of Py_Initialize().

   Beware: if multiple interpreter and/or thread states exist, these
   are not wiped out; only the current thread and interpreter state
   are deleted.  But since everything else is deleted, those other
   interpreter and thread states should no longer be used.

   (XXX We should do better, e.g. wipe out all interpreters and
   threads.)

   Locking: as above.

*/


static void
finalize_interp_types(PyInterpreterState *interp)
{
    _PyUnicode_FiniTypes(interp);
    _PySys_FiniTypes(interp);
    _PyExc_Fini(interp);
    _PyAsyncGen_Fini(interp);
    _PyContext_Fini(interp);
    _PyFloat_FiniType(interp);
    _PyLong_FiniTypes(interp);
    _PyThread_FiniType(interp);
    // XXX fini collections module static types (_PyStaticType_Dealloc())
    // XXX fini IO module static types (_PyStaticType_Dealloc())
    _PyErr_FiniTypes(interp);
    _PyTypes_FiniTypes(interp);

    _PyTypes_Fini(interp);

    // Call _PyUnicode_ClearInterned() before _PyDict_Fini() since it uses
    // a dict internally.
    _PyUnicode_ClearInterned(interp);

    _PyDict_Fini(interp);
    _PyList_Fini(interp);
    _PyTuple_Fini(interp);

    _PySlice_Fini(interp);

    _PyUnicode_Fini(interp);
    _PyFloat_Fini(interp);
#ifdef Py_DEBUG
    _PyStaticObjects_CheckRefcnt(interp);
#endif
}


static void
finalize_interp_clear(PyThreadState *tstate)
{
    int is_main_interp = _Py_IsMainInterpreter(tstate->interp);

    _PyExc_ClearExceptionGroupType(tstate->interp);
    _Py_clear_generic_types(tstate->interp);

    /* Clear interpreter state and all thread states */
    _PyInterpreterState_Clear(tstate);

    /* Clear all loghooks */
    /* Both _PySys_Audit function and users still need PyObject, such as tuple.
       Call _PySys_ClearAuditHooks when PyObject available. */
    if (is_main_interp) {
        _PySys_ClearAuditHooks(tstate);
    }

    if (is_main_interp) {
        _Py_HashRandomization_Fini();
        _PyArg_Fini();
        _Py_ClearFileSystemEncoding();
        _PyPerfTrampoline_Fini();
    }

    finalize_interp_types(tstate->interp);
}


static void
finalize_interp_delete(PyInterpreterState *interp)
{
    /* Cleanup auto-thread-state */
    _PyGILState_Fini(interp);

    /* We can't call _PyEval_FiniGIL() here because destroying the GIL lock can
       fail when it is being awaited by another running daemon thread (see
       bpo-9901). Instead pycore_create_interpreter() destroys the previously
       created GIL, which ensures that Py_Initialize / Py_FinalizeEx can be
       called multiple times. */

    PyInterpreterState_Delete(interp);
}


int
Py_FinalizeEx(void)
{
    int status = 0;

    _PyRuntimeState *runtime = &_PyRuntime;
    if (!runtime->initialized) {
        return status;
    }

    /* Get current thread state and interpreter pointer */
    PyThreadState *tstate = _PyThreadState_GET();
    // XXX assert(_Py_IsMainInterpreter(tstate->interp));
    // XXX assert(_Py_IsMainThread());

    // Block some operations.
    tstate->interp->finalizing = 1;

    // Wrap up existing "threading"-module-created, non-daemon threads.
    wait_for_thread_shutdown(tstate);

    // Make any remaining pending calls.
    _Py_FinishPendingCalls(tstate);

    /* The interpreter is still entirely intact at this point, and the
     * exit funcs may be relying on that.  In particular, if some thread
     * or exit func is still waiting to do an import, the import machinery
     * expects Py_IsInitialized() to return true.  So don't say the
     * runtime is uninitialized until after the exit funcs have run.
     * Note that Threading.py uses an exit func to do a join on all the
     * threads created thru it, so this also protects pending imports in
     * the threads created via Threading.
     */

    _PyAtExit_Call(tstate->interp);
    PyUnstable_PerfMapState_Fini();

    /* Copy the core config, PyInterpreterState_Delete() free
       the core config memory */
#ifdef Py_REF_DEBUG
    int show_ref_count = tstate->interp->config.show_ref_count;
#endif
#ifdef Py_TRACE_REFS
    int dump_refs = tstate->interp->config.dump_refs;
    wchar_t *dump_refs_file = tstate->interp->config.dump_refs_file;
#endif
#ifdef WITH_PYMALLOC
    int malloc_stats = tstate->interp->config.malloc_stats;
#endif

    /* Remaining daemon threads will automatically exit
       when they attempt to take the GIL (ex: PyEval_RestoreThread()). */
    _PyInterpreterState_SetFinalizing(tstate->interp, tstate);
    _PyRuntimeState_SetFinalizing(runtime, tstate);
    runtime->initialized = 0;
    runtime->core_initialized = 0;

    // XXX Call something like _PyImport_Disable() here?

    /* Destroy the state of all threads of the interpreter, except of the
       current thread. In practice, only daemon threads should still be alive,
       except if wait_for_thread_shutdown() has been cancelled by CTRL+C.
       Clear frames of other threads to call objects destructors. Destructors
       will be called in the current Python thread. Since
       _PyRuntimeState_SetFinalizing() has been called, no other Python thread
       can take the GIL at this point: if they try, they will exit
       immediately. */
    _PyThreadState_DeleteExcept(tstate);

    /* At this point no Python code should be running at all.
       The only thread state left should be the main thread of the main
       interpreter (AKA tstate), in which this code is running right now.
       There may be other OS threads running but none of them will have
       thread states associated with them, nor will be able to create
       new thread states.

       Thus tstate is the only possible thread state from here on out.
       It may still be used during finalization to run Python code as
       needed or provide runtime state (e.g. sys.modules) but that will
       happen sparingly.  Furthermore, the order of finalization aims
       to not need a thread (or interpreter) state as soon as possible.
     */
    // XXX Make sure we are preventing the creating of any new thread states
    // (or interpreters).

    /* Flush sys.stdout and sys.stderr */
    if (flush_std_files() < 0) {
        status = -1;
    }

    /* Disable signal handling */
    _PySignal_Fini();

    /* Collect garbage.  This may call finalizers; it's nice to call these
     * before all modules are destroyed.
     * XXX If a __del__ or weakref callback is triggered here, and tries to
     * XXX import a module, bad things can happen, because Python no
     * XXX longer believes it's initialized.
     * XXX     Fatal Python error: Interpreter not initialized (version mismatch?)
     * XXX is easy to provoke that way.  I've also seen, e.g.,
     * XXX     Exception exceptions.ImportError: 'No module named sha'
     * XXX         in <function callback at 0x008F5718> ignored
     * XXX but I'm unclear on exactly how that one happens.  In any case,
     * XXX I haven't seen a real-life report of either of these.
     */
    PyGC_Collect();

    /* Destroy all modules */
    _PyImport_FiniExternal(tstate->interp);
    finalize_modules(tstate);

    /* Print debug stats if any */
    _PyEval_Fini();

    /* Flush sys.stdout and sys.stderr (again, in case more was printed) */
    if (flush_std_files() < 0) {
        status = -1;
    }

    /* Collect final garbage.  This disposes of cycles created by
     * class definitions, for example.
     * XXX This is disabled because it caused too many problems.  If
     * XXX a __del__ or weakref callback triggers here, Python code has
     * XXX a hard time running, because even the sys module has been
     * XXX cleared out (sys.stdout is gone, sys.excepthook is gone, etc).
     * XXX One symptom is a sequence of information-free messages
     * XXX coming from threads (if a __del__ or callback is invoked,
     * XXX other threads can execute too, and any exception they encounter
     * XXX triggers a comedy of errors as subsystem after subsystem
     * XXX fails to find what it *expects* to find in sys to help report
     * XXX the exception and consequent unexpected failures).  I've also
     * XXX seen segfaults then, after adding print statements to the
     * XXX Python code getting called.
     */
#if 0
    _PyGC_CollectIfEnabled();
#endif

    /* Disable tracemalloc after all Python objects have been destroyed,
       so it is possible to use tracemalloc in objects destructor. */
    _PyTraceMalloc_Fini();

    /* Finalize any remaining import state */
    // XXX Move these up to where finalize_modules() is currently.
    _PyImport_FiniCore(tstate->interp);
    _PyImport_Fini();

    /* unload faulthandler module */
    _PyFaulthandler_Fini();

    /* dump hash stats */
    _PyHash_Fini();

#ifdef Py_TRACE_REFS
    /* Display all objects still alive -- this can invoke arbitrary
     * __repr__ overrides, so requires a mostly-intact interpreter.
     * Alas, a lot of stuff may still be alive now that will be cleaned
     * up later.
     */

    FILE *dump_refs_fp = NULL;
    if (dump_refs_file != NULL) {
        dump_refs_fp = _Py_wfopen(dump_refs_file, L"w");
        if (dump_refs_fp == NULL) {
            fprintf(stderr, "PYTHONDUMPREFSFILE: cannot create file: %ls\n", dump_refs_file);
        }
    }

    if (dump_refs) {
        _Py_PrintReferences(tstate->interp, stderr);
    }

    if (dump_refs_fp != NULL) {
        _Py_PrintReferences(tstate->interp, dump_refs_fp);
    }
#endif /* Py_TRACE_REFS */

    /* At this point there's almost no other Python code that will run,
       nor interpreter state needed.  The only possibility is the
       finalizers of the objects stored on tstate (and tstate->interp),
       which are triggered via finalize_interp_clear().

       For now we operate as though none of those finalizers actually
       need an operational thread state or interpreter.  In reality,
       those finalizers may rely on some part of tstate or
       tstate->interp, and/or may raise exceptions
       or otherwise fail.
     */
    // XXX Do this sooner during finalization.
    // XXX Ensure finalizer errors are handled properly.

    finalize_interp_clear(tstate);

#ifdef Py_TRACE_REFS
    /* Display addresses (& refcnts) of all objects still alive.
     * An address can be used to find the repr of the object, printed
     * above by _Py_PrintReferences. */
    if (dump_refs) {
        _Py_PrintReferenceAddresses(tstate->interp, stderr);
    }
    if (dump_refs_fp != NULL) {
        _Py_PrintReferenceAddresses(tstate->interp, dump_refs_fp);
        fclose(dump_refs_fp);
    }
#endif /* Py_TRACE_REFS */

    finalize_interp_delete(tstate->interp);

#ifdef Py_REF_DEBUG
    if (show_ref_count) {
        _PyDebug_PrintTotalRefs();
    }
    _Py_FinalizeRefTotal(runtime);
#endif
    _Py_FinalizeAllocatedBlocks(runtime);

#ifdef WITH_PYMALLOC
    if (malloc_stats) {
        _PyObject_DebugMallocStats(stderr);
    }
#endif

    call_ll_exitfuncs(runtime);

    _PyRuntime_Finalize();
    return status;
}

void
Py_Finalize(void)
{
    Py_FinalizeEx();
}


/* Create and initialize a new interpreter and thread, and return the
   new thread.  This requires that Py_Initialize() has been called
   first.

   Unsuccessful initialization yields a NULL pointer.  Note that *no*
   exception information is available even in this case -- the
   exception information is held in the thread, and there is no
   thread.

   Locking: as above.

*/

static PyStatus
new_interpreter(PyThreadState **tstate_p, const PyInterpreterConfig *config)
{
    PyStatus status;

    status = _PyRuntime_Initialize();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
    _PyRuntimeState *runtime = &_PyRuntime;

    if (!runtime->initialized) {
        return _PyStatus_ERR("Py_Initialize must be called first");
    }

    /* Issue #10915, #15751: The GIL API doesn't work with multiple
       interpreters: disable PyGILState_Check(). */
    runtime->gilstate.check_enabled = 0;

    PyInterpreterState *interp = PyInterpreterState_New();
    if (interp == NULL) {
        *tstate_p = NULL;
        return _PyStatus_OK();
    }

    PyThreadState *tstate = _PyThreadState_New(interp);
    if (tstate == NULL) {
        PyInterpreterState_Delete(interp);
        *tstate_p = NULL;
        return _PyStatus_OK();
    }
    _PyThreadState_Bind(tstate);

    // XXX For now we do this before the GIL is created.
    PyThreadState *save_tstate = _PyThreadState_SwapNoGIL(tstate);
    int has_gil = 0;

    /* From this point until the init_interp_create_gil() call,
       we must not do anything that requires that the GIL be held
       (or otherwise exist).  That applies whether or not the new
       interpreter has its own GIL (e.g. the main interpreter). */

    /* Copy the current interpreter config into the new interpreter */
    const PyConfig *src_config;
    if (save_tstate != NULL) {
        // XXX Might new_interpreter() have been called without the GIL held?
        _PyEval_ReleaseLock(save_tstate->interp, save_tstate);
        src_config = _PyInterpreterState_GetConfig(save_tstate->interp);
    }
    else
    {
        /* No current thread state, copy from the main interpreter */
        PyInterpreterState *main_interp = _PyInterpreterState_Main();
        src_config = _PyInterpreterState_GetConfig(main_interp);
    }

    /* This does not require that the GIL be held. */
    status = _PyConfig_Copy(&interp->config, src_config);
    if (_PyStatus_EXCEPTION(status)) {
        goto error;
    }

    /* This does not require that the GIL be held. */
    status = init_interp_settings(interp, config);
    if (_PyStatus_EXCEPTION(status)) {
        goto error;
    }

    status = init_interp_create_gil(tstate, config->gil);
    if (_PyStatus_EXCEPTION(status)) {
        goto error;
    }
    has_gil = 1;

    /* No objects have been created yet. */

    status = pycore_interp_init(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        goto error;
    }

    status = init_interp_main(tstate);
    if (_PyStatus_EXCEPTION(status)) {
        goto error;
    }

    *tstate_p = tstate;
    return _PyStatus_OK();

error:
    *tstate_p = NULL;

    /* Oops, it didn't work.  Undo it all. */
    PyErr_PrintEx(0);
    if (has_gil) {
        PyThreadState_Swap(save_tstate);
    }
    else {
        _PyThreadState_SwapNoGIL(save_tstate);
    }
    PyThreadState_Clear(tstate);
    PyThreadState_Delete(tstate);
    PyInterpreterState_Delete(interp);

    return status;
}

PyStatus
Py_NewInterpreterFromConfig(PyThreadState **tstate_p,
                            const PyInterpreterConfig *config)
{
    return new_interpreter(tstate_p, config);
}

PyThreadState *
Py_NewInterpreter(void)
{
    PyThreadState *tstate = NULL;
    const PyInterpreterConfig config = _PyInterpreterConfig_LEGACY_INIT;
    PyStatus status = new_interpreter(&tstate, &config);
    if (_PyStatus_EXCEPTION(status)) {
        Py_ExitStatusException(status);
    }
    return tstate;
}

/* Delete an interpreter and its last thread.  This requires that the
   given thread state is current, that the thread has no remaining
   frames, and that it is its interpreter's only remaining thread.
   It is a fatal error to violate these constraints.

   (Py_FinalizeEx() doesn't have these constraints -- it zaps
   everything, regardless.)

   Locking: as above.

*/

void
Py_EndInterpreter(PyThreadState *tstate)
{
    PyInterpreterState *interp = tstate->interp;

    if (tstate != _PyThreadState_GET()) {
        Py_FatalError("thread is not current");
    }
    if (tstate->current_frame != NULL) {
        Py_FatalError("thread still has a frame");
    }
    interp->finalizing = 1;

    // Wrap up existing "threading"-module-created, non-daemon threads.
    wait_for_thread_shutdown(tstate);

    // Make any remaining pending calls.
    _Py_FinishPendingCalls(tstate);

    _PyAtExit_Call(tstate->interp);

    if (tstate != interp->threads.head || tstate->next != NULL) {
        Py_FatalError("not the last thread");
    }

    /* Remaining daemon threads will automatically exit
       when they attempt to take the GIL (ex: PyEval_RestoreThread()). */
    _PyInterpreterState_SetFinalizing(interp, tstate);

    // XXX Call something like _PyImport_Disable() here?

    _PyImport_FiniExternal(tstate->interp);
    finalize_modules(tstate);
    _PyImport_FiniCore(tstate->interp);

    finalize_interp_clear(tstate);
    finalize_interp_delete(tstate->interp);
}

int
_Py_IsInterpreterFinalizing(PyInterpreterState *interp)
{
    /* We check the runtime first since, in a daemon thread,
       interp might be dangling pointer. */
    PyThreadState *finalizing = _PyRuntimeState_GetFinalizing(&_PyRuntime);
    if (finalizing == NULL) {
        finalizing = _PyInterpreterState_GetFinalizing(interp);
    }
    return finalizing != NULL;
}

/* Add the __main__ module */

static PyStatus
add_main_module(PyInterpreterState *interp)
{
    PyObject *m, *d, *ann_dict;
    m = PyImport_AddModule("__main__");
    if (m == NULL)
        return _PyStatus_ERR("can't create __main__ module");

    d = PyModule_GetDict(m);
    ann_dict = PyDict_New();
    if ((ann_dict == NULL) ||
        (PyDict_SetItemString(d, "__annotations__", ann_dict) < 0)) {
        return _PyStatus_ERR("Failed to initialize __main__.__annotations__");
    }
    Py_DECREF(ann_dict);

    int has_builtins = PyDict_ContainsString(d, "__builtins__");
    if (has_builtins < 0) {
        return _PyStatus_ERR("Failed to test __main__.__builtins__");
    }
    if (!has_builtins) {
        PyObject *bimod = PyImport_ImportModule("builtins");
        if (bimod == NULL) {
            return _PyStatus_ERR("Failed to retrieve builtins module");
        }
        if (PyDict_SetItemString(d, "__builtins__", bimod) < 0) {
            return _PyStatus_ERR("Failed to initialize __main__.__builtins__");
        }
        Py_DECREF(bimod);
    }

    /* Main is a little special - BuiltinImporter is the most appropriate
     * initial setting for its __loader__ attribute. A more suitable value
     * will be set if __main__ gets further initialized later in the startup
     * process.
     */
    PyObject *loader;
    if (PyDict_GetItemStringRef(d, "__loader__", &loader) < 0) {
        return _PyStatus_ERR("Failed to test __main__.__loader__");
    }
    int has_loader = !(loader == NULL || loader == Py_None);
    Py_XDECREF(loader);
    if (!has_loader) {
        PyObject *loader = _PyImport_GetImportlibLoader(interp,
                                                        "BuiltinImporter");
        if (loader == NULL) {
            return _PyStatus_ERR("Failed to retrieve BuiltinImporter");
        }
        if (PyDict_SetItemString(d, "__loader__", loader) < 0) {
            return _PyStatus_ERR("Failed to initialize __main__.__loader__");
        }
        Py_DECREF(loader);
    }
    return _PyStatus_OK();
}

/* Import the site module (not into __main__ though) */

static PyStatus
init_import_site(void)
{
    PyObject *m;
    m = PyImport_ImportModule("site");
    if (m == NULL) {
        return _PyStatus_ERR("Failed to import the site module");
    }
    Py_DECREF(m);
    return _PyStatus_OK();
}

/* Check if a file descriptor is valid or not.
   Return 0 if the file descriptor is invalid, return non-zero otherwise. */
static int
is_valid_fd(int fd)
{
/* dup() is faster than fstat(): fstat() can require input/output operations,
   whereas dup() doesn't. There is a low risk of EMFILE/ENFILE at Python
   startup. Problem: dup() doesn't check if the file descriptor is valid on
   some platforms.

   fcntl(fd, F_GETFD) is even faster, because it only checks the process table.
   It is preferred over dup() when available, since it cannot fail with the
   "too many open files" error (EMFILE).

   bpo-30225: On macOS Tiger, when stdout is redirected to a pipe and the other
   side of the pipe is closed, dup(1) succeed, whereas fstat(1, &st) fails with
   EBADF. FreeBSD has similar issue (bpo-32849).

   Only use dup() on Linux where dup() is enough to detect invalid FD
   (bpo-32849).
*/
    if (fd < 0) {
        return 0;
    }
#if defined(F_GETFD) && ( \
        defined(__linux__) || \
        defined(__APPLE__) || \
        defined(__wasm__))
    return fcntl(fd, F_GETFD) >= 0;
#elif defined(__linux__)
    int fd2 = dup(fd);
    if (fd2 >= 0) {
        close(fd2);
    }
    return (fd2 >= 0);
#elif defined(MS_WINDOWS)
    HANDLE hfile;
    _Py_BEGIN_SUPPRESS_IPH
    hfile = (HANDLE)_get_osfhandle(fd);
    _Py_END_SUPPRESS_IPH
    return (hfile != INVALID_HANDLE_VALUE
            && GetFileType(hfile) != FILE_TYPE_UNKNOWN);
#else
    struct stat st;
    return (fstat(fd, &st) == 0);
#endif
}

/* returns Py_None if the fd is not valid */
static PyObject*
create_stdio(const PyConfig *config, PyObject* io,
    int fd, int write_mode, const char* name,
    const wchar_t* encoding, const wchar_t* errors)
{
    PyObject *buf = NULL, *stream = NULL, *text = NULL, *raw = NULL, *res;
    const char* mode;
    const char* newline;
    PyObject *line_buffering, *write_through;
    int buffering, isatty;
    const int buffered_stdio = config->buffered_stdio;

    if (!is_valid_fd(fd))
        Py_RETURN_NONE;

    /* stdin is always opened in buffered mode, first because it shouldn't
       make a difference in common use cases, second because TextIOWrapper
       depends on the presence of a read1() method which only exists on
       buffered streams.
    */
    if (!buffered_stdio && write_mode)
        buffering = 0;
    else
        buffering = -1;
    if (write_mode)
        mode = "wb";
    else
        mode = "rb";
    buf = _PyObject_CallMethod(io, &_Py_ID(open), "isiOOOO",
                               fd, mode, buffering,
                               Py_None, Py_None, /* encoding, errors */
                               Py_None, Py_False); /* newline, closefd */
    if (buf == NULL)
        goto error;

    if (buffering) {
        raw = PyObject_GetAttr(buf, &_Py_ID(raw));
        if (raw == NULL)
            goto error;
    }
    else {
        raw = Py_NewRef(buf);
    }

#ifdef HAVE_WINDOWS_CONSOLE_IO
    /* Windows console IO is always UTF-8 encoded */
    PyTypeObject *winconsoleio_type = (PyTypeObject *)_PyImport_GetModuleAttr(
            &_Py_ID(_io), &_Py_ID(_WindowsConsoleIO));
    if (winconsoleio_type == NULL) {
        goto error;
    }
    int is_subclass = PyObject_TypeCheck(raw, winconsoleio_type);
    Py_DECREF(winconsoleio_type);
    if (is_subclass) {
        encoding = L"utf-8";
    }
#endif

    text = PyUnicode_FromString(name);
    if (text == NULL || PyObject_SetAttr(raw, &_Py_ID(name), text) < 0)
        goto error;
    res = PyObject_CallMethodNoArgs(raw, &_Py_ID(isatty));
    if (res == NULL)
        goto error;
    isatty = PyObject_IsTrue(res);
    Py_DECREF(res);
    if (isatty == -1)
        goto error;
    if (!buffered_stdio)
        write_through = Py_True;
    else
        write_through = Py_False;
    if (buffered_stdio && (isatty || fd == fileno(stderr)))
        line_buffering = Py_True;
    else
        line_buffering = Py_False;

    Py_CLEAR(raw);
    Py_CLEAR(text);

#ifdef MS_WINDOWS
    /* sys.stdin: enable universal newline mode, translate "\r\n" and "\r"
       newlines to "\n".
       sys.stdout and sys.stderr: translate "\n" to "\r\n". */
    newline = NULL;
#else
    /* sys.stdin: split lines at "\n".
       sys.stdout and sys.stderr: don't translate newlines (use "\n"). */
    newline = "\n";
#endif

    PyObject *encoding_str = PyUnicode_FromWideChar(encoding, -1);
    if (encoding_str == NULL) {
        Py_CLEAR(buf);
        goto error;
    }

    PyObject *errors_str = PyUnicode_FromWideChar(errors, -1);
    if (errors_str == NULL) {
        Py_CLEAR(buf);
        Py_CLEAR(encoding_str);
        goto error;
    }

    stream = _PyObject_CallMethod(io, &_Py_ID(TextIOWrapper), "OOOsOO",
                                  buf, encoding_str, errors_str,
                                  newline, line_buffering, write_through);
    Py_CLEAR(buf);
    Py_CLEAR(encoding_str);
    Py_CLEAR(errors_str);
    if (stream == NULL)
        goto error;

    if (write_mode)
        mode = "w";
    else
        mode = "r";
    text = PyUnicode_FromString(mode);
    if (!text || PyObject_SetAttr(stream, &_Py_ID(mode), text) < 0)
        goto error;
    Py_CLEAR(text);
    return stream;

error:
    Py_XDECREF(buf);
    Py_XDECREF(stream);
    Py_XDECREF(text);
    Py_XDECREF(raw);

    if (PyErr_ExceptionMatches(PyExc_OSError) && !is_valid_fd(fd)) {
        /* Issue #24891: the file descriptor was closed after the first
           is_valid_fd() check was called. Ignore the OSError and set the
           stream to None. */
        PyErr_Clear();
        Py_RETURN_NONE;
    }
    return NULL;
}

/* Set builtins.open to io.open */
static PyStatus
init_set_builtins_open(void)
{
    PyObject *wrapper;
    PyObject *bimod = NULL;
    PyStatus res = _PyStatus_OK();

    if (!(bimod = PyImport_ImportModule("builtins"))) {
        goto error;
    }

    if (!(wrapper = _PyImport_GetModuleAttrString("io", "open"))) {
        goto error;
    }

    /* Set builtins.open */
    if (PyObject_SetAttrString(bimod, "open", wrapper) == -1) {
        Py_DECREF(wrapper);
        goto error;
    }
    Py_DECREF(wrapper);
    goto done;

error:
    res = _PyStatus_ERR("can't initialize io.open");

done:
    Py_XDECREF(bimod);
    return res;
}


/* Create sys.stdin, sys.stdout and sys.stderr */
static PyStatus
init_sys_streams(PyThreadState *tstate)
{
    PyObject *iomod = NULL;
    PyObject *std = NULL;
    int fd;
    PyObject * encoding_attr;
    PyStatus res = _PyStatus_OK();
    const PyConfig *config = _PyInterpreterState_GetConfig(tstate->interp);

    /* Check that stdin is not a directory
       Using shell redirection, you can redirect stdin to a directory,
       crashing the Python interpreter. Catch this common mistake here
       and output a useful error message. Note that under MS Windows,
       the shell already prevents that. */
#ifndef MS_WINDOWS
    struct _Py_stat_struct sb;
    if (_Py_fstat_noraise(fileno(stdin), &sb) == 0 &&
        S_ISDIR(sb.st_mode)) {
        return _PyStatus_ERR("<stdin> is a directory, cannot continue");
    }
#endif

    if (!(iomod = PyImport_ImportModule("io"))) {
        goto error;
    }

    /* Set sys.stdin */
    fd = fileno(stdin);
    /* Under some conditions stdin, stdout and stderr may not be connected
     * and fileno() may point to an invalid file descriptor. For example
     * GUI apps don't have valid standard streams by default.
     */
    std = create_stdio(config, iomod, fd, 0, "<stdin>",
                       config->stdio_encoding,
                       config->stdio_errors);
    if (std == NULL)
        goto error;
    PySys_SetObject("__stdin__", std);
    _PySys_SetAttr(&_Py_ID(stdin), std);
    Py_DECREF(std);

    /* Set sys.stdout */
    fd = fileno(stdout);
    std = create_stdio(config, iomod, fd, 1, "<stdout>",
                       config->stdio_encoding,
                       config->stdio_errors);
    if (std == NULL)
        goto error;
    PySys_SetObject("__stdout__", std);
    _PySys_SetAttr(&_Py_ID(stdout), std);
    Py_DECREF(std);

#if 1 /* Disable this if you have trouble debugging bootstrap stuff */
    /* Set sys.stderr, replaces the preliminary stderr */
    fd = fileno(stderr);
    std = create_stdio(config, iomod, fd, 1, "<stderr>",
                       config->stdio_encoding,
                       L"backslashreplace");
    if (std == NULL)
        goto error;

    /* Same as hack above, pre-import stderr's codec to avoid recursion
       when import.c tries to write to stderr in verbose mode. */
    encoding_attr = PyObject_GetAttrString(std, "encoding");
    if (encoding_attr != NULL) {
        const char *std_encoding = PyUnicode_AsUTF8(encoding_attr);
        if (std_encoding != NULL) {
            PyObject *codec_info = _PyCodec_Lookup(std_encoding);
            Py_XDECREF(codec_info);
        }
        Py_DECREF(encoding_attr);
    }
    _PyErr_Clear(tstate);  /* Not a fatal error if codec isn't available */

    if (PySys_SetObject("__stderr__", std) < 0) {
        Py_DECREF(std);
        goto error;
    }
    if (_PySys_SetAttr(&_Py_ID(stderr), std) < 0) {
        Py_DECREF(std);
        goto error;
    }
    Py_DECREF(std);
#endif

    goto done;

error:
    res = _PyStatus_ERR("can't initialize sys standard streams");

done:
    Py_XDECREF(iomod);
    return res;
}


static void
_Py_FatalError_DumpTracebacks(int fd, PyInterpreterState *interp,
                              PyThreadState *tstate)
{
    PUTS(fd, "\n");

    /* display the current Python stack */
    _Py_DumpTracebackThreads(fd, interp, tstate);
}

/* Print the current exception (if an exception is set) with its traceback,
   or display the current Python stack.

   Don't call PyErr_PrintEx() and the except hook, because Py_FatalError() is
   called on catastrophic cases.

   Return 1 if the traceback was displayed, 0 otherwise. */

static int
_Py_FatalError_PrintExc(PyThreadState *tstate)
{
    PyObject *exc = _PyErr_GetRaisedException(tstate);
    if (exc == NULL) {
        /* No current exception */
        return 0;
    }

    PyObject *ferr = _PySys_GetAttr(tstate, &_Py_ID(stderr));
    if (ferr == NULL || ferr == Py_None) {
        /* sys.stderr is not set yet or set to None,
           no need to try to display the exception */
        Py_DECREF(exc);
        return 0;
    }

    PyErr_DisplayException(exc);

    PyObject *tb = PyException_GetTraceback(exc);
    int has_tb = (tb != NULL) && (tb != Py_None);
    Py_XDECREF(tb);
    Py_DECREF(exc);

    /* sys.stderr may be buffered: call sys.stderr.flush() */
    if (_PyFile_Flush(ferr) < 0) {
        _PyErr_Clear(tstate);
    }

    return has_tb;
}

/* Print fatal error message and abort */

#ifdef MS_WINDOWS
static void
fatal_output_debug(const char *msg)
{
    /* buffer of 256 bytes allocated on the stack */
    WCHAR buffer[256 / sizeof(WCHAR)];
    size_t buflen = Py_ARRAY_LENGTH(buffer) - 1;
    size_t msglen;

    OutputDebugStringW(L"Fatal Python error: ");

    msglen = strlen(msg);
    while (msglen) {
        size_t i;

        if (buflen > msglen) {
            buflen = msglen;
        }

        /* Convert the message to wchar_t. This uses a simple one-to-one
           conversion, assuming that the this error message actually uses
           ASCII only. If this ceases to be true, we will have to convert. */
        for (i=0; i < buflen; ++i) {
            buffer[i] = msg[i];
        }
        buffer[i] = L'\0';
        OutputDebugStringW(buffer);

        msg += buflen;
        msglen -= buflen;
    }
    OutputDebugStringW(L"\n");
}
#endif


static void
fatal_error_dump_runtime(int fd, _PyRuntimeState *runtime)
{
    PUTS(fd, "Python runtime state: ");
    PyThreadState *finalizing = _PyRuntimeState_GetFinalizing(runtime);
    if (finalizing) {
        PUTS(fd, "finalizing (tstate=0x");
        _Py_DumpHexadecimal(fd, (uintptr_t)finalizing, sizeof(finalizing) * 2);
        PUTS(fd, ")");
    }
    else if (runtime->initialized) {
        PUTS(fd, "initialized");
    }
    else if (runtime->core_initialized) {
        PUTS(fd, "core initialized");
    }
    else if (runtime->preinitialized) {
        PUTS(fd, "preinitialized");
    }
    else if (runtime->preinitializing) {
        PUTS(fd, "preinitializing");
    }
    else {
        PUTS(fd, "unknown");
    }
    PUTS(fd, "\n");
}


static inline void _Py_NO_RETURN
fatal_error_exit(int status)
{
    if (status < 0) {
#if defined(MS_WINDOWS) && defined(_DEBUG)
        DebugBreak();
#endif
        abort();
    }
    else {
        exit(status);
    }
}


// Dump the list of extension modules of sys.modules, excluding stdlib modules
// (sys.stdlib_module_names), into fd file descriptor.
//
// This function is called by a signal handler in faulthandler: avoid memory
// allocations and keep the implementation simple. For example, the list is not
// sorted on purpose.
void
_Py_DumpExtensionModules(int fd, PyInterpreterState *interp)
{
    if (interp == NULL) {
        return;
    }
    PyObject *modules = _PyImport_GetModules(interp);
    if (modules == NULL || !PyDict_Check(modules)) {
        return;
    }

    Py_ssize_t pos;
    PyObject *key, *value;

    // Avoid PyDict_GetItemString() which calls PyUnicode_FromString(),
    // memory cannot be allocated on the heap in a signal handler.
    // Iterate on the dict instead.
    PyObject *stdlib_module_names = NULL;
    if (interp->sysdict != NULL) {
        pos = 0;
        while (PyDict_Next(interp->sysdict, &pos, &key, &value)) {
            if (PyUnicode_Check(key)
               && PyUnicode_CompareWithASCIIString(key, "stdlib_module_names") == 0) {
                stdlib_module_names = value;
                break;
            }
        }
    }
    // If we failed to get sys.stdlib_module_names or it's not a frozenset,
    // don't exclude stdlib modules.
    if (stdlib_module_names != NULL && !PyFrozenSet_Check(stdlib_module_names)) {
        stdlib_module_names = NULL;
    }

    // List extensions
    int header = 1;
    Py_ssize_t count = 0;
    pos = 0;
    while (PyDict_Next(modules, &pos, &key, &value)) {
        if (!PyUnicode_Check(key)) {
            continue;
        }
        if (!_PyModule_IsExtension(value)) {
            continue;
        }
        // Use the module name from the sys.modules key,
        // don't attempt to get the module object name.
        if (stdlib_module_names != NULL) {
            int is_stdlib_ext = 0;

            Py_ssize_t i = 0;
            PyObject *item;
            Py_hash_t hash;
            while (_PySet_NextEntry(stdlib_module_names, &i, &item, &hash)) {
                if (PyUnicode_Check(item)
                    && PyUnicode_Compare(key, item) == 0)
                {
                    is_stdlib_ext = 1;
                    break;
                }
            }
            if (is_stdlib_ext) {
                // Ignore stdlib extension
                continue;
            }
        }

        if (header) {
            PUTS(fd, "\nExtension modules: ");
            header = 0;
        }
        else {
            PUTS(fd, ", ");
        }

        _Py_DumpASCII(fd, key);
        count++;
    }

    if (count) {
        PUTS(fd, " (total: ");
        _Py_DumpDecimal(fd, count);
        PUTS(fd, ")");
        PUTS(fd, "\n");
    }
}


static void _Py_NO_RETURN
fatal_error(int fd, int header, const char *prefix, const char *msg,
            int status)
{
    static int reentrant = 0;

    if (reentrant) {
        /* Py_FatalError() caused a second fatal error.
           Example: flush_std_files() raises a recursion error. */
        fatal_error_exit(status);
    }
    reentrant = 1;

    if (header) {
        PUTS(fd, "Fatal Python error: ");
        if (prefix) {
            PUTS(fd, prefix);
            PUTS(fd, ": ");
        }
        if (msg) {
            PUTS(fd, msg);
        }
        else {
            PUTS(fd, "<message not set>");
        }
        PUTS(fd, "\n");
    }

    _PyRuntimeState *runtime = &_PyRuntime;
    fatal_error_dump_runtime(fd, runtime);

    /* Check if the current thread has a Python thread state
       and holds the GIL.

       tss_tstate is NULL if Py_FatalError() is called from a C thread which
       has no Python thread state.

       tss_tstate != tstate if the current Python thread does not hold the GIL.
       */
    PyThreadState *tstate = _PyThreadState_GET();
    PyInterpreterState *interp = NULL;
    PyThreadState *tss_tstate = PyGILState_GetThisThreadState();
    if (tstate != NULL) {
        interp = tstate->interp;
    }
    else if (tss_tstate != NULL) {
        interp = tss_tstate->interp;
    }
    int has_tstate_and_gil = (tss_tstate != NULL && tss_tstate == tstate);

    if (has_tstate_and_gil) {
        /* If an exception is set, print the exception with its traceback */
        if (!_Py_FatalError_PrintExc(tss_tstate)) {
            /* No exception is set, or an exception is set without traceback */
            _Py_FatalError_DumpTracebacks(fd, interp, tss_tstate);
        }
    }
    else {
        _Py_FatalError_DumpTracebacks(fd, interp, tss_tstate);
    }

    _Py_DumpExtensionModules(fd, interp);

    /* The main purpose of faulthandler is to display the traceback.
       This function already did its best to display a traceback.
       Disable faulthandler to prevent writing a second traceback
       on abort(). */
    _PyFaulthandler_Fini();

    /* Check if the current Python thread hold the GIL */
    if (has_tstate_and_gil) {
        /* Flush sys.stdout and sys.stderr */
        flush_std_files();
    }

#ifdef MS_WINDOWS
    fatal_output_debug(msg);
#endif /* MS_WINDOWS */

    fatal_error_exit(status);
}


#undef Py_FatalError

void _Py_NO_RETURN
Py_FatalError(const char *msg)
{
    fatal_error(fileno(stderr), 1, NULL, msg, -1);
}


void _Py_NO_RETURN
_Py_FatalErrorFunc(const char *func, const char *msg)
{
    fatal_error(fileno(stderr), 1, func, msg, -1);
}


void _Py_NO_RETURN
_Py_FatalErrorFormat(const char *func, const char *format, ...)
{
    static int reentrant = 0;
    if (reentrant) {
        /* _Py_FatalErrorFormat() caused a second fatal error */
        fatal_error_exit(-1);
    }
    reentrant = 1;

    FILE *stream = stderr;
    const int fd = fileno(stream);
    PUTS(fd, "Fatal Python error: ");
    if (func) {
        PUTS(fd, func);
        PUTS(fd, ": ");
    }

    va_list vargs;
    va_start(vargs, format);
    vfprintf(stream, format, vargs);
    va_end(vargs);

    fputs("\n", stream);
    fflush(stream);

    fatal_error(fd, 0, NULL, NULL, -1);
}


void _Py_NO_RETURN
_Py_FatalRefcountErrorFunc(const char *func, const char *msg)
{
    _Py_FatalErrorFormat(func,
                         "%s: bug likely caused by a refcount error "
                         "in a C extension",
                         msg);
}


void _Py_NO_RETURN
Py_ExitStatusException(PyStatus status)
{
    if (_PyStatus_IS_EXIT(status)) {
        exit(status.exitcode);
    }
    else if (_PyStatus_IS_ERROR(status)) {
        fatal_error(fileno(stderr), 1, status.func, status.err_msg, 1);
    }
    else {
        Py_FatalError("Py_ExitStatusException() must not be called on success");
    }
}


/* Wait until threading._shutdown completes, provided
   the threading module was imported in the first place.
   The shutdown routine will wait until all non-daemon
   "threading" threads have completed. */
static void
wait_for_thread_shutdown(PyThreadState *tstate)
{
    PyObject *result;
    PyObject *threading = PyImport_GetModule(&_Py_ID(threading));
    if (threading == NULL) {
        if (_PyErr_Occurred(tstate)) {
            PyErr_WriteUnraisable(NULL);
        }
        /* else: threading not imported */
        return;
    }
    result = PyObject_CallMethodNoArgs(threading, &_Py_ID(_shutdown));
    if (result == NULL) {
        PyErr_WriteUnraisable(threading);
    }
    else {
        Py_DECREF(result);
    }
    Py_DECREF(threading);
}

int Py_AtExit(void (*func)(void))
{
    struct _atexit_runtime_state *state = &_PyRuntime.atexit;
    PyThread_acquire_lock(state->mutex, WAIT_LOCK);
    if (state->ncallbacks >= NEXITFUNCS) {
        PyThread_release_lock(state->mutex);
        return -1;
    }
    state->callbacks[state->ncallbacks++] = func;
    PyThread_release_lock(state->mutex);
    return 0;
}

static void
call_ll_exitfuncs(_PyRuntimeState *runtime)
{
    atexit_callbackfunc exitfunc;
    struct _atexit_runtime_state *state = &runtime->atexit;

    PyThread_acquire_lock(state->mutex, WAIT_LOCK);
    while (state->ncallbacks > 0) {
        /* pop last function from the list */
        state->ncallbacks--;
        exitfunc = state->callbacks[state->ncallbacks];
        state->callbacks[state->ncallbacks] = NULL;

        PyThread_release_lock(state->mutex);
        exitfunc();
        PyThread_acquire_lock(state->mutex, WAIT_LOCK);
    }
    PyThread_release_lock(state->mutex);

    fflush(stdout);
    fflush(stderr);
}

void _Py_NO_RETURN
Py_Exit(int sts)
{
    if (Py_FinalizeEx() < 0) {
        sts = 120;
    }

    exit(sts);
}


/*
 * The file descriptor fd is considered ``interactive'' if either
 *   a) isatty(fd) is TRUE, or
 *   b) the -i flag was given, and the filename associated with
 *      the descriptor is NULL or "<stdin>" or "???".
 */
int
Py_FdIsInteractive(FILE *fp, const char *filename)
{
    if (isatty(fileno(fp))) {
        return 1;
    }
    if (!_Py_GetConfig()->interactive) {
        return 0;
    }
    return ((filename == NULL)
            || (strcmp(filename, "<stdin>") == 0)
            || (strcmp(filename, "???") == 0));
}


int
_Py_FdIsInteractive(FILE *fp, PyObject *filename)
{
    if (isatty(fileno(fp))) {
        return 1;
    }
    if (!_Py_GetConfig()->interactive) {
        return 0;
    }
    return ((filename == NULL)
            || (PyUnicode_CompareWithASCIIString(filename, "<stdin>") == 0)
            || (PyUnicode_CompareWithASCIIString(filename, "???") == 0));
}


/* Wrappers around sigaction() or signal(). */

PyOS_sighandler_t
PyOS_getsig(int sig)
{
#ifdef HAVE_SIGACTION
    struct sigaction context;
    if (sigaction(sig, NULL, &context) == -1)
        return SIG_ERR;
    return context.sa_handler;
#else
    PyOS_sighandler_t handler;
/* Special signal handling for the secure CRT in Visual Studio 2005 */
#if defined(_MSC_VER) && _MSC_VER >= 1400
    switch (sig) {
    /* Only these signals are valid */
    case SIGINT:
    case SIGILL:
    case SIGFPE:
    case SIGSEGV:
    case SIGTERM:
    case SIGBREAK:
    case SIGABRT:
        break;
    /* Don't call signal() with other values or it will assert */
    default:
        return SIG_ERR;
    }
#endif /* _MSC_VER && _MSC_VER >= 1400 */
    handler = signal(sig, SIG_IGN);
    if (handler != SIG_ERR)
        signal(sig, handler);
    return handler;
#endif
}

/*
 * All of the code in this function must only use async-signal-safe functions,
 * listed at `man 7 signal` or
 * http://www.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_04.html.
 */
PyOS_sighandler_t
PyOS_setsig(int sig, PyOS_sighandler_t handler)
{
#ifdef HAVE_SIGACTION
    /* Some code in Modules/signalmodule.c depends on sigaction() being
     * used here if HAVE_SIGACTION is defined.  Fix that if this code
     * changes to invalidate that assumption.
     */
    struct sigaction context, ocontext;
    context.sa_handler = handler;
    sigemptyset(&context.sa_mask);
    /* Using SA_ONSTACK is friendlier to other C/C++/Golang-VM code that
     * extension module or embedding code may use where tiny thread stacks
     * are used.  https://bugs.python.org/issue43390 */
    context.sa_flags = SA_ONSTACK;
    if (sigaction(sig, &context, &ocontext) == -1)
        return SIG_ERR;
    return ocontext.sa_handler;
#else
    PyOS_sighandler_t oldhandler;
    oldhandler = signal(sig, handler);
#ifdef HAVE_SIGINTERRUPT
    siginterrupt(sig, 1);
#endif
    return oldhandler;
#endif
}
