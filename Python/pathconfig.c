/* Path configuration like module_search_path (sys.path) */

#include "Python.h"
#include "osdefs.h"
#include "pycore_coreconfig.h"
#include "pycore_fileutils.h"
#include "pycore_pathconfig.h"
#include "pycore_pymem.h"
#include "pycore_pystate.h"
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif


_PyPathConfig _Py_path_config = _PyPathConfig_INIT;


static int
copy_wstr(wchar_t **dst, const wchar_t *src)
{
    if (src != NULL) {
        *dst = _PyMem_RawWcsdup(src);
        if (*dst == NULL) {
            return -1;
        }
    }
    else {
        *dst = NULL;
    }
    return 0;
}


static void
_PyPathConfig_Clear(_PyPathConfig *config)
{
    /* _PyMem_SetDefaultAllocator() is needed to get a known memory allocator,
       since Py_SetPath(), Py_SetPythonHome() and Py_SetProgramName() can be
       called before Py_Initialize() which can changes the memory allocator. */
    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

#define CLEAR(ATTR) \
    do { \
        PyMem_RawFree(ATTR); \
        ATTR = NULL; \
    } while (0)

    CLEAR(config->prefix);
    CLEAR(config->program_full_path);
    CLEAR(config->exec_prefix);
#ifdef MS_WINDOWS
    CLEAR(config->dll_path);
#endif
    CLEAR(config->module_search_path);
    CLEAR(config->home);
    CLEAR(config->program_name);
#undef CLEAR

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
}


/* Calculate the path configuration: initialize path_config from core_config */
static _PyInitError
_PyPathConfig_Calculate(_PyPathConfig *path_config,
                        const _PyCoreConfig *core_config)
{
    _PyInitError err;
    _PyPathConfig new_config = _PyPathConfig_INIT;

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    /* Calculate program_full_path, prefix, exec_prefix,
       dll_path (Windows), and module_search_path */
    err = _PyPathConfig_Calculate_impl(&new_config, core_config);
    if (_Py_INIT_FAILED(err)) {
        goto err;
    }

    /* Copy home and program_name from core_config */
    if (copy_wstr(&new_config.home, core_config->home) < 0) {
        err = _Py_INIT_NO_MEMORY();
        goto err;
    }
    if (copy_wstr(&new_config.program_name, core_config->program_name) < 0) {
        err = _Py_INIT_NO_MEMORY();
        goto err;
    }

    _PyPathConfig_Clear(path_config);
    *path_config = new_config;

    err = _Py_INIT_OK();
    goto done;

err:
    _PyPathConfig_Clear(&new_config);

done:
    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
    return err;
}


_PyInitError
_PyPathConfig_SetGlobal(const _PyPathConfig *config)
{
    _PyInitError err;
    _PyPathConfig new_config = _PyPathConfig_INIT;

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

#define COPY_ATTR(ATTR) \
    do { \
        if (copy_wstr(&new_config.ATTR, config->ATTR) < 0) { \
            _PyPathConfig_Clear(&new_config); \
            err = _Py_INIT_NO_MEMORY(); \
            goto done; \
        } \
    } while (0)

    COPY_ATTR(program_full_path);
    COPY_ATTR(prefix);
    COPY_ATTR(exec_prefix);
#ifdef MS_WINDOWS
    COPY_ATTR(dll_path);
#endif
    COPY_ATTR(module_search_path);
    COPY_ATTR(program_name);
    COPY_ATTR(home);

    _PyPathConfig_Clear(&_Py_path_config);
    /* Steal new_config strings; don't clear new_config */
    _Py_path_config = new_config;

    err = _Py_INIT_OK();

done:
    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
    return err;
}


void
_PyPathConfig_ClearGlobal(void)
{
    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    _PyPathConfig_Clear(&_Py_path_config);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
}


static wchar_t*
_PyWstrList_Join(const _PyWstrList *list, wchar_t sep)
{
    size_t len = 1;   /* NUL terminator */
    for (Py_ssize_t i=0; i < list->length; i++) {
        if (i != 0) {
            len++;
        }
        len += wcslen(list->items[i]);
    }

    wchar_t *text = PyMem_RawMalloc(len * sizeof(wchar_t));
    if (text == NULL) {
        return NULL;
    }
    wchar_t *str = text;
    for (Py_ssize_t i=0; i < list->length; i++) {
        wchar_t *path = list->items[i];
        if (i != 0) {
            *str++ = SEP;
        }
        len = wcslen(path);
        memcpy(str, path, len * sizeof(wchar_t));
        str += len;
    }
    *str = L'\0';

    return text;
}


/* Set the global path configuration from core_config. */
_PyInitError
_PyCoreConfig_SetPathConfig(const _PyCoreConfig *core_config)
{
    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    _PyInitError err;
    _PyPathConfig path_config = _PyPathConfig_INIT;

    path_config.module_search_path = _PyWstrList_Join(&core_config->module_search_paths, DELIM);
    if (path_config.module_search_path == NULL) {
        goto no_memory;
    }

    if (copy_wstr(&path_config.program_full_path, core_config->executable) < 0) {
        goto no_memory;
    }
    if (copy_wstr(&path_config.prefix, core_config->prefix) < 0) {
        goto no_memory;
    }
    if (copy_wstr(&path_config.exec_prefix, core_config->exec_prefix) < 0) {
        goto no_memory;
    }
#ifdef MS_WINDOWS
    if (copy_wstr(&path_config.dll_path, core_config->dll_path) < 0) {
        goto no_memory;
    }
#endif
    if (copy_wstr(&path_config.program_name, core_config->program_name) < 0) {
        goto no_memory;
    }
    if (copy_wstr(&path_config.home, core_config->home) < 0) {
        goto no_memory;
    }

    err = _PyPathConfig_SetGlobal(&path_config);
    if (_Py_INIT_FAILED(err)) {
        goto done;
    }

    err = _Py_INIT_OK();
    goto done;

no_memory:
    err = _Py_INIT_NO_MEMORY();

done:
    _PyPathConfig_Clear(&path_config);
    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);
    return err;
}


static _PyInitError
core_config_init_module_search_paths(_PyCoreConfig *config,
                                     _PyPathConfig *path_config)
{
    assert(!config->use_module_search_paths);

    _PyWstrList_Clear(&config->module_search_paths);

    const wchar_t *sys_path = path_config->module_search_path;
    const wchar_t delim = DELIM;
    const wchar_t *p = sys_path;
    while (1) {
        p = wcschr(sys_path, delim);
        if (p == NULL) {
            p = sys_path + wcslen(sys_path); /* End of string */
        }

        size_t path_len = (p - sys_path);
        wchar_t *path = PyMem_RawMalloc((path_len + 1) * sizeof(wchar_t));
        if (path == NULL) {
            return _Py_INIT_NO_MEMORY();
        }
        memcpy(path, sys_path, path_len * sizeof(wchar_t));
        path[path_len] = L'\0';

        int res = _PyWstrList_Append(&config->module_search_paths, path);
        PyMem_RawFree(path);
        if (res < 0) {
            return _Py_INIT_NO_MEMORY();
        }

        if (*p == '\0') {
            break;
        }
        sys_path = p + 1;
    }
    config->use_module_search_paths = 1;
    return _Py_INIT_OK();
}


static _PyInitError
_PyCoreConfig_CalculatePathConfig(_PyCoreConfig *config)
{
    _PyPathConfig path_config = _PyPathConfig_INIT;
    _PyInitError err;

    err = _PyPathConfig_Calculate(&path_config, config);
    if (_Py_INIT_FAILED(err)) {
        goto error;
    }

    if (!config->use_module_search_paths) {
        err = core_config_init_module_search_paths(config, &path_config);
        if (_Py_INIT_FAILED(err)) {
            goto error;
        }
    }

    if (config->executable == NULL) {
        if (copy_wstr(&config->executable,
                      path_config.program_full_path) < 0) {
            goto no_memory;
        }
    }

    if (config->prefix == NULL) {
        if (copy_wstr(&config->prefix, path_config.prefix) < 0) {
            goto no_memory;
        }
    }

    if (config->exec_prefix == NULL) {
        if (copy_wstr(&config->exec_prefix,
                      path_config.exec_prefix) < 0) {
            goto no_memory;
        }
    }

#ifdef MS_WINDOWS
    if (config->dll_path == NULL) {
        if (copy_wstr(&config->dll_path, path_config.dll_path) < 0) {
            goto no_memory;
        }
    }
#endif

    if (path_config.isolated != -1) {
        config->preconfig.isolated = path_config.isolated;
    }
    if (path_config.site_import != -1) {
        config->site_import = path_config.site_import;
    }

    _PyPathConfig_Clear(&path_config);
    return _Py_INIT_OK();

no_memory:
    err = _Py_INIT_NO_MEMORY();

error:
    _PyPathConfig_Clear(&path_config);
    return err;
}


_PyInitError
_PyCoreConfig_InitPathConfig(_PyCoreConfig *config)
{
    /* Do we need to calculate the path? */
    if (!config->use_module_search_paths
        || (config->executable == NULL)
        || (config->prefix == NULL)
#ifdef MS_WINDOWS
        || (config->dll_path == NULL)
#endif
        || (config->exec_prefix == NULL))
    {
        _PyInitError err = _PyCoreConfig_CalculatePathConfig(config);
        if (_Py_INIT_FAILED(err)) {
            return err;
        }
    }

    if (config->base_prefix == NULL) {
        if (copy_wstr(&config->base_prefix, config->prefix) < 0) {
            return _Py_INIT_NO_MEMORY();
        }
    }

    if (config->base_exec_prefix == NULL) {
        if (copy_wstr(&config->base_exec_prefix,
                      config->exec_prefix) < 0) {
            return _Py_INIT_NO_MEMORY();
        }
    }
    return _Py_INIT_OK();
}


static void
pathconfig_global_init(void)
{
    if (_Py_path_config.module_search_path != NULL) {
        /* Already initialized */
        return;
    }

    _PyInitError err;
    _PyCoreConfig config = _PyCoreConfig_INIT;

    err = _PyCoreConfig_Read(&config, NULL);
    if (_Py_INIT_FAILED(err)) {
        goto error;
    }

    err = _PyCoreConfig_SetPathConfig(&config);
    if (_Py_INIT_FAILED(err)) {
        goto error;
    }

    _PyCoreConfig_Clear(&config);
    return;

error:
    _PyCoreConfig_Clear(&config);
    _Py_ExitInitError(err);
}


/* External interface */

void
Py_SetPath(const wchar_t *path)
{
    if (path == NULL) {
        _PyPathConfig_Clear(&_Py_path_config);
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    _PyPathConfig new_config;
    new_config.program_full_path = _PyMem_RawWcsdup(Py_GetProgramName());
    int alloc_error = (new_config.program_full_path == NULL);
    new_config.prefix = _PyMem_RawWcsdup(L"");
    alloc_error |= (new_config.prefix == NULL);
    new_config.exec_prefix = _PyMem_RawWcsdup(L"");
    alloc_error |= (new_config.exec_prefix == NULL);
#ifdef MS_WINDOWS
    new_config.dll_path = _PyMem_RawWcsdup(L"");
    alloc_error |= (new_config.dll_path == NULL);
#endif
    new_config.module_search_path = _PyMem_RawWcsdup(path);
    alloc_error |= (new_config.module_search_path == NULL);

    /* steal the home and program_name values (to leave them unchanged) */
    new_config.home = _Py_path_config.home;
    _Py_path_config.home = NULL;
    new_config.program_name = _Py_path_config.program_name;
    _Py_path_config.program_name = NULL;

    _PyPathConfig_Clear(&_Py_path_config);
    _Py_path_config = new_config;

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (alloc_error) {
        Py_FatalError("Py_SetPath() failed: out of memory");
    }
}


void
Py_SetPythonHome(const wchar_t *home)
{
    if (home == NULL) {
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    PyMem_RawFree(_Py_path_config.home);
    _Py_path_config.home = _PyMem_RawWcsdup(home);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (_Py_path_config.home == NULL) {
        Py_FatalError("Py_SetPythonHome() failed: out of memory");
    }
}


void
Py_SetProgramName(const wchar_t *program_name)
{
    if (program_name == NULL || program_name[0] == L'\0') {
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    PyMem_RawFree(_Py_path_config.program_name);
    _Py_path_config.program_name = _PyMem_RawWcsdup(program_name);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (_Py_path_config.program_name == NULL) {
        Py_FatalError("Py_SetProgramName() failed: out of memory");
    }
}

void
_Py_SetProgramFullPath(const wchar_t *program_full_path)
{
    if (program_full_path == NULL || program_full_path[0] == L'\0') {
        return;
    }

    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    PyMem_RawFree(_Py_path_config.program_full_path);
    _Py_path_config.program_full_path = _PyMem_RawWcsdup(program_full_path);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (_Py_path_config.program_full_path == NULL) {
        Py_FatalError("_Py_SetProgramFullPath() failed: out of memory");
    }
}


wchar_t *
Py_GetPath(void)
{
    pathconfig_global_init();
    return _Py_path_config.module_search_path;
}


wchar_t *
Py_GetPrefix(void)
{
    pathconfig_global_init();
    return _Py_path_config.prefix;
}


wchar_t *
Py_GetExecPrefix(void)
{
    pathconfig_global_init();
    return _Py_path_config.exec_prefix;
}


wchar_t *
Py_GetProgramFullPath(void)
{
    pathconfig_global_init();
    return _Py_path_config.program_full_path;
}


wchar_t*
Py_GetPythonHome(void)
{
    pathconfig_global_init();
    return _Py_path_config.home;
}


wchar_t *
Py_GetProgramName(void)
{
    pathconfig_global_init();
    return _Py_path_config.program_name;
}

/* Compute module search path from argv[0] or the current working
   directory ("-m module" case) which will be prepended to sys.argv:
   sys.path[0].

   Return 1 if the path is correctly resolved, but *path0_p can be NULL
   if the Unicode object fail to be created.

   Return 0 if it fails to resolve the full path (and *path0_p will be NULL).
   For example, return 0 if the current working directory has been removed
   (bpo-36236) or if argv is empty.
   */
int
_PyPathConfig_ComputeSysPath0(const _PyWstrList *argv, PyObject **path0_p)
{
    assert(_PyWstrList_CheckConsistency(argv));
    assert(*path0_p == NULL);

    if (argv->length == 0) {
        /* Leave sys.path unchanged if sys.argv is empty */
        return 0;
    }

    wchar_t *argv0 = argv->items[0];
    int have_module_arg = (wcscmp(argv0, L"-m") == 0);
    int have_script_arg = (!have_module_arg && (wcscmp(argv0, L"-c") != 0));

    wchar_t *path0 = argv0;
    Py_ssize_t n = 0;

#ifdef HAVE_REALPATH
    wchar_t fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS)
    wchar_t fullpath[MAX_PATH];
#endif

    if (have_module_arg) {
#if defined(HAVE_REALPATH) || defined(MS_WINDOWS)
        if (!_Py_wgetcwd(fullpath, Py_ARRAY_LENGTH(fullpath))) {
            return 0;
        }
        path0 = fullpath;
#else
        path0 = L".";
#endif
        n = wcslen(path0);
    }

#ifdef HAVE_READLINK
    wchar_t link[MAXPATHLEN + 1];
    int nr = 0;

    if (have_script_arg) {
        nr = _Py_wreadlink(path0, link, Py_ARRAY_LENGTH(link));
    }
    if (nr > 0) {
        /* It's a symlink */
        link[nr] = '\0';
        if (link[0] == SEP) {
            path0 = link; /* Link to absolute path */
        }
        else if (wcschr(link, SEP) == NULL) {
            /* Link without path */
        }
        else {
            /* Must join(dirname(path0), link) */
            wchar_t *q = wcsrchr(path0, SEP);
            if (q == NULL) {
                /* path0 without path */
                path0 = link;
            }
            else {
                /* Must make a copy, path0copy has room for 2 * MAXPATHLEN */
                wchar_t path0copy[2 * MAXPATHLEN + 1];
                wcsncpy(path0copy, path0, MAXPATHLEN);
                q = wcsrchr(path0copy, SEP);
                wcsncpy(q+1, link, MAXPATHLEN);
                q[MAXPATHLEN + 1] = L'\0';
                path0 = path0copy;
            }
        }
    }
#endif /* HAVE_READLINK */

    wchar_t *p = NULL;

#if SEP == '\\'
    /* Special case for Microsoft filename syntax */
    if (have_script_arg) {
        wchar_t *q;
#if defined(MS_WINDOWS)
        /* Replace the first element in argv with the full path. */
        wchar_t *ptemp;
        if (GetFullPathNameW(path0,
                           Py_ARRAY_LENGTH(fullpath),
                           fullpath,
                           &ptemp)) {
            path0 = fullpath;
        }
#endif
        p = wcsrchr(path0, SEP);
        /* Test for alternate separator */
        q = wcsrchr(p ? p : path0, '/');
        if (q != NULL)
            p = q;
        if (p != NULL) {
            n = p + 1 - path0;
            if (n > 1 && p[-1] != ':')
                n--; /* Drop trailing separator */
        }
    }
#else
    /* All other filename syntaxes */
    if (have_script_arg) {
#if defined(HAVE_REALPATH)
        if (_Py_wrealpath(path0, fullpath, Py_ARRAY_LENGTH(fullpath))) {
            path0 = fullpath;
        }
#endif
        p = wcsrchr(path0, SEP);
    }
    if (p != NULL) {
        n = p + 1 - path0;
#if SEP == '/' /* Special case for Unix filename syntax */
        if (n > 1) {
            /* Drop trailing separator */
            n--;
        }
#endif /* Unix */
    }
#endif /* All others */

    *path0_p = PyUnicode_FromWideChar(path0, n);
    return 1;
}


#ifdef MS_WINDOWS
#define WCSTOK wcstok_s
#else
#define WCSTOK wcstok
#endif

/* Search for a prefix value in an environment file (pyvenv.cfg).
   If found, copy it into the provided buffer. */
int
_Py_FindEnvConfigValue(FILE *env_file, const wchar_t *key,
                       wchar_t *value, size_t value_size)
{
    int result = 0; /* meaning not found */
    char buffer[MAXPATHLEN * 2 + 1];  /* allow extra for key, '=', etc. */
    buffer[Py_ARRAY_LENGTH(buffer)-1] = '\0';

    fseek(env_file, 0, SEEK_SET);
    while (!feof(env_file)) {
        char * p = fgets(buffer, Py_ARRAY_LENGTH(buffer) - 1, env_file);

        if (p == NULL) {
            break;
        }

        size_t n = strlen(p);
        if (p[n - 1] != '\n') {
            /* line has overflowed - bail */
            break;
        }
        if (p[0] == '#') {
            /* Comment - skip */
            continue;
        }

        wchar_t *tmpbuffer = _Py_DecodeUTF8_surrogateescape(buffer, n, NULL);
        if (tmpbuffer) {
            wchar_t * state;
            wchar_t * tok = WCSTOK(tmpbuffer, L" \t\r\n", &state);
            if ((tok != NULL) && !wcscmp(tok, key)) {
                tok = WCSTOK(NULL, L" \t", &state);
                if ((tok != NULL) && !wcscmp(tok, L"=")) {
                    tok = WCSTOK(NULL, L"\r\n", &state);
                    if (tok != NULL) {
                        wcsncpy(value, tok, value_size - 1);
                        value[value_size - 1] = L'\0';
                        result = 1;
                        PyMem_RawFree(tmpbuffer);
                        break;
                    }
                }
            }
            PyMem_RawFree(tmpbuffer);
        }
    }
    return result;
}

#ifdef __cplusplus
}
#endif
