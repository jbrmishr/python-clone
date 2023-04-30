/* Python interpreter main program */

#include "Python.h"
#include "pycore_call.h"          // _PyObject_CallNoArgs()
#include "pycore_initconfig.h"    // _PyArgv
#include "pycore_interp.h"        // _PyInterpreterState.sysdict
#include "pycore_pathconfig.h"    // _PyPathConfig_ComputeSysPath0()
#include "pycore_pylifecycle.h"   // _Py_PreInitializeFromPyArgv()
#include "pycore_pystate.h"       // _PyInterpreterState_GET()

/* Includes for exit_sigint() */
#include <stdio.h>                // perror()
#ifdef HAVE_SIGNAL_H
#  include <signal.h>             // SIGINT
#endif
#if defined(HAVE_GETPID) && defined(HAVE_UNISTD_H)
#  include <unistd.h>             // getpid()
#endif
#ifdef MS_WINDOWS
#  include <windows.h>            // STATUS_CONTROL_C_EXIT
#endif
/* End of includes for exit_sigint() */

#define COPYRIGHT \
    "Type \"help\", \"copyright\", \"credits\" or \"license\" " \
    "for more information."

#ifdef __cplusplus
extern "C" {
#endif

/* --- pymain_init() ---------------------------------------------- */

static PyStatus
pymain_init(const _PyArgv *args)
{
    PyStatus status;

    status = _PyRuntime_Initialize();
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    PyPreConfig preconfig;
    PyPreConfig_InitPythonConfig(&preconfig);

    status = _Py_PreInitializeFromPyArgv(&preconfig, args);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }

    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    /* pass NULL as the config: config is read from command line arguments,
       environment variables, configuration files */
    if (args->use_bytes_argv) {
        status = PyConfig_SetBytesArgv(&config, args->argc, args->bytes_argv);
    }
    else {
        status = PyConfig_SetArgv(&config, args->argc, args->wchar_argv);
    }
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }

    status = Py_InitializeFromConfig(&config);
    if (_PyStatus_EXCEPTION(status)) {
        goto done;
    }
    status = _PyStatus_OK();

done:
    PyConfig_Clear(&config);
    return status;
}


/* --- pymain_run_python() ---------------------------------------- */

/* Non-zero if filename, command (-c) or module (-m) is set
   on the command line */
static inline int config_run_code(const PyConfig *config)
{
    return (config->run_command != NULL
            || config->run_filename != NULL
            || config->run_module != NULL);
}


/* Return non-zero if stdin is a TTY or if -i command line option is used */
static int
stdin_is_interactive(const PyConfig *config)
{
    return (isatty(fileno(stdin)) || config->interactive);
}


/* Display the current Python exception and return an exitcode */
static int
pymain_err_print(int *exitcode_p)
{
    int exitcode;
    if (_Py_HandleSystemExit(&exitcode)) {
        *exitcode_p = exitcode;
        return 1;
    }

    PyErr_Print();
    return 0;
}


static int
pymain_exit_err_print(void)
{
    int exitcode = 1;
    pymain_err_print(&exitcode);
    return exitcode;
}


/* Write an exitcode into *exitcode and return 1 if we have to exit Python.
   Return 0 otherwise. */
static int
pymain_get_importer(const wchar_t *filename, PyObject **importer_p, int *exitcode)
{
    PyObject *sys_path0 = NULL, *importer;

    sys_path0 = PyUnicode_FromWideChar(filename, wcslen(filename));
    if (sys_path0 == NULL) {
        goto error;
    }

    importer = PyImport_GetImporter(sys_path0);
    if (importer == NULL) {
        goto error;
    }

    if (importer == Py_None) {
        Py_DECREF(sys_path0);
        Py_DECREF(importer);
        return 0;
    }

    Py_DECREF(importer);
    *importer_p = sys_path0;
    return 0;

error:
    Py_XDECREF(sys_path0);

    PySys_WriteStderr("Failed checking if argv[0] is an import path entry\n");
    return pymain_err_print(exitcode);
}


static int
pymain_sys_path_add_path0(PyInterpreterState *interp, PyObject *path0)
{
    PyObject *sys_path;
    PyObject *sysdict = interp->sysdict;
    if (sysdict != NULL) {
        sys_path = PyDict_GetItemWithError(sysdict, &_Py_ID(path));
        if (sys_path == NULL && PyErr_Occurred()) {
            return -1;
        }
    }
    else {
        sys_path = NULL;
    }
    if (sys_path == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "unable to get sys.path");
        return -1;
    }

    if (PyList_Insert(sys_path, 0, path0)) {
        return -1;
    }
    return 0;
}


static void
pymain_header(const PyConfig *config)
{
    if (config->quiet) {
        return;
    }

    if (!config->verbose && (config_run_code(config) || !stdin_is_interactive(config))) {
        return;
    }

    fprintf(stderr, "Python %s on %s\n", Py_GetVersion(), Py_GetPlatform());
    if (config->site_import) {
        fprintf(stderr, "%s\n", COPYRIGHT);
    }
}


static void
pymain_import_readline(const PyConfig *config)
{
    if (config->isolated) {
        return;
    }
    if (!config->inspect && config_run_code(config)) {
        return;
    }
    if (!isatty(fileno(stdin))) {
        return;
    }

    PyObject *mod = PyImport_ImportModule("readline");
    if (mod == NULL) {
        PyErr_Clear();
    }
    else {
        Py_DECREF(mod);
    }
    mod = PyImport_ImportModule("rlcompleter");
    if (mod == NULL) {
        PyErr_Clear();
    }
    else {
        Py_DECREF(mod);
    }
}

#ifdef MS_WINDOWS
#  define WCSTOK wcstok_s
#else
#  define WCSTOK wcstok
#endif

//#define DEBUG_DEDENT

/* Strip common leading whitespace from an input command
 * Sort of works?
 * */
wchar_t* _wcs_dedent(wchar_t *command)
{
    // Security problem, what is the right way to do this?
    size_t nchars = wcslen(command);

#ifdef DEBUG_DEDENT
    fprintf(stderr, "\nSTART WCS_DEDENT\n");
    fprintf(stderr, "command: '%ls'\n", command);
    fprintf(stderr, "nchars: %d\n", nchars);
#endif

    wchar_t *command_copy = (wchar_t *)PyMem_Malloc(nchars * sizeof(wchar_t));
    // wcscpy has security problems, what is the workaround?
    wcscpy(command_copy, command);
    // fprintf(stderr, "command_copy: '%ls'\n", command_copy);

    size_t num_common_leading_spaces = nchars + 1;

    // I wcstok has a problem because ignores multiple instances of the
    // delimiter which may make programs behave differently if that newline
    // belongs to a multiline string. However, it is ok if we just want to do a
    // first pass over the data to find the common indentation on non-empty
    // lines.
    wchar_t *buffer;
    wchar_t* line = WCSTOK(command_copy, L"\n", &buffer);
    while (line) {

        // Move the pointer up to the first non-space character
        wchar_t *first_nonspace = line;
        while (wcsncmp(first_nonspace, L" ", 1) == 0){
            first_nonspace++;
        }

        // Only check lines that contain non-whitespace characters
        if (wcsncmp(first_nonspace, L"\0", 1)) {

            size_t num_leading_spaces = first_nonspace - line;
            if (num_leading_spaces < num_common_leading_spaces) {
                num_common_leading_spaces = num_leading_spaces;
            }
#ifdef DEBUG_DEDENT
         fprintf(stderr, "==========\n");
         fprintf(stderr, "line: '%ls'\n", line);
         fprintf(stderr, "first_nonspace: '%ls'\n", first_nonspace);
         fprintf(stderr, "num_common_leading_spaces: '%d'\n", num_common_leading_spaces);
         fprintf(stderr, "num_leading_spaces: '%d'\n", num_leading_spaces);
#endif
        }
        line = WCSTOK(NULL, L"\n", &buffer);
    }
    PyMem_Free(command_copy);

    wchar_t *end_ptr = command + nchars;
    wchar_t *curr_line_ptr = command;
    wchar_t *next_line_ptr;
    wchar_t *new_start_loc;
    size_t new_line_len;

    // What is the correct way to ensure this is null terminated
    wchar_t *new_command = (wchar_t *)PyMem_Malloc((nchars + 1) * sizeof(wchar_t));
    wmemset(new_command, NULL, nchars + 1);
    wchar_t *curr_dst = new_command;

    while (curr_line_ptr != end_ptr) {
        // Find the end of the current line.
        next_line_ptr = wcsstr(curr_line_ptr, L"\n");
        if (next_line_ptr == NULL) {
            next_line_ptr = end_ptr;
        }
        else {
            next_line_ptr++;
        }

        size_t line_len = next_line_ptr - curr_line_ptr;

        if (line_len > num_common_leading_spaces){
            new_start_loc = curr_line_ptr + num_common_leading_spaces;
            new_line_len = line_len - num_common_leading_spaces;
        }
        else {
            new_start_loc = curr_line_ptr;
            new_line_len = line_len;
        }

        int offset = curr_line_ptr - command;

#ifdef DEBUG_DEDENT
        fprintf(stderr, "line_len: '%d'\n", line_len);
        fprintf(stderr, "offset: '%d'\n", offset);
#endif

        // Copy the part of the line we want to keep to the new location
        wcsncpy(curr_dst, new_start_loc, new_line_len);
        curr_dst += new_line_len;

        curr_line_ptr = next_line_ptr;
    }

    // FIXME: I'm sure this is not the memory safe way to do this, but I dont
    // know what is.
    command = new_command;

#ifdef DEBUG_DEDENT
    fprintf(stderr, "new_command: '%ls'\n", new_command);
    fprintf(stderr, "\nEND WCS_DEDENT\n");
#endif
    return command;

}


/* Strip common leading whitespace from an input command */
PyObject* _unicode_dedent(PyObject *unicode)
{
    /*fprintf(stderr, "\nSTART unicode dedent\n");      */
    /*PyObject_Print(PyObject_Repr(unicode), stderr, 0);*/
    /*fprintf(stderr, "\n");                            */

    PyObject* space = PyUnicode_FromWideChar(L" ", -1);
    PyObject* emptystr = PyUnicode_FromWideChar(L"", -1);
    PyObject* new_unicode;

    // Break up the input into lines
    PyObject *lines = PyUnicode_Splitlines(unicode, 1);

    // Init leading space to a large value to indicate
    // that it is uninitialized
    Py_ssize_t effective_inf = PyObject_Length(unicode) + 1;
    Py_ssize_t common_leading_spaces = effective_inf;

    Py_ssize_t num_lines = PyObject_Length(lines);
    for (Py_ssize_t line_idx = 0; line_idx < num_lines; line_idx ++) {
        PyObject* index = PyLong_FromSsize_t(line_idx);
        PyObject* line = PyObject_GetItem(lines, index);
        PyObject* striped_line = _PyUnicode_XStrip(line, 0, space);

        // Determine the number of leading whitespace on this line.
        Py_ssize_t line_len = PyObject_Length(line);
        Py_ssize_t stripline_len = PyObject_Length(striped_line);
        Py_ssize_t leading_spaces = line_len - stripline_len;

        // On non-empty lines, see if the amount of leading whitespace is less
        // than current value. If so, update it.
        if (line_len > 1 && leading_spaces < common_leading_spaces) {
            common_leading_spaces = leading_spaces;
        }

        Py_DECREF(striped_line);
        Py_DECREF(line);
        Py_DECREF(index);
    }

    if (common_leading_spaces > 0 && common_leading_spaces < effective_inf) {

        // We found common leading whitespace, strip if off.
        PyObject* new_lines = PyList_New(num_lines);
        for (Py_ssize_t line_idx = 0; line_idx < num_lines; line_idx ++) {
            PyObject* index = PyLong_FromSsize_t(line_idx);
            PyObject* line = PyObject_GetItem(lines, index);
            Py_ssize_t end = PyObject_Length(line);
            Py_ssize_t start = common_leading_spaces;
            if (end <= 1) {
                start = 0;
            }
            PyObject* new_line = PyUnicode_Substring(line, start, end);
            PyList_SetItem(new_lines, line_idx, new_line);

            //Py_DECREF(new_line);  // is it correct that we dont need to DECREF here?
            Py_DECREF(line);
            Py_DECREF(index);
        }
        new_unicode = PyUnicode_Join(emptystr, new_lines);

        Py_DECREF(new_lines);

        // We are going to return an updated version of "unicode" that the
        // caller will decref, so need to decref the version we are replacing
        // here.  This feels fragile and like the wrong way to do this.
        // Guidance here would be appreciated.
        Py_DECREF(unicode);
    }
    else {
       new_unicode = unicode;
    }

    Py_DECREF(lines);
    Py_DECREF(space);
    Py_DECREF(emptystr);

    /*PyObject_Print(PyObject_Repr(new_unicode), stderr, 0);*/
    /*fprintf(stderr, "\nEND unicode dedent\n");            */
    /*fprintf(stderr, "\n");                                */

    return new_unicode;
}


static int
pymain_run_command(wchar_t *command)
{
    PyObject *unicode, *bytes;
    int ret;

    // Should the input be modified here with pure C?
    if (wcsncmp(command, L"\n", 1) == 0) {
        command = _wcs_dedent(command);
        if (command == NULL) {
            goto error;
        }
    }

    unicode = PyUnicode_FromWideChar(command, -1);
    if (unicode == NULL) {
        goto error;
    }

    if (PySys_Audit("cpython.run_command", "O", unicode) < 0) {
        return pymain_exit_err_print();
    }

    // Only perform auto-dedent if the string starts with a newline
    /*if (wcsncmp(command, L"\n", 1) == 0) {                         */
    /*    // Should the input be modified here with the Python C-API?*/
    /*    unicode = _unicode_dedent(unicode);                        */
    /*}                                                              */

    bytes = PyUnicode_AsUTF8String(unicode);
    Py_DECREF(unicode);
    if (bytes == NULL) {
        goto error;
    }

    PyCompilerFlags cf = _PyCompilerFlags_INIT;
    cf.cf_flags |= PyCF_IGNORE_COOKIE;
    ret = PyRun_SimpleStringFlags(PyBytes_AsString(bytes), &cf);
    Py_DECREF(bytes);
    return (ret != 0);

error:
    PySys_WriteStderr("Unable to decode the command from the command line:\n");
    return pymain_exit_err_print();
}


static int
pymain_run_module(const wchar_t *modname, int set_argv0)
{
    PyObject *module, *runpy, *runmodule, *runargs, *result;
    if (PySys_Audit("cpython.run_module", "u", modname) < 0) {
        return pymain_exit_err_print();
    }
    runpy = PyImport_ImportModule("runpy");
    if (runpy == NULL) {
        fprintf(stderr, "Could not import runpy module\n");
        return pymain_exit_err_print();
    }
    runmodule = PyObject_GetAttrString(runpy, "_run_module_as_main");
    if (runmodule == NULL) {
        fprintf(stderr, "Could not access runpy._run_module_as_main\n");
        Py_DECREF(runpy);
        return pymain_exit_err_print();
    }
    module = PyUnicode_FromWideChar(modname, wcslen(modname));
    if (module == NULL) {
        fprintf(stderr, "Could not convert module name to unicode\n");
        Py_DECREF(runpy);
        Py_DECREF(runmodule);
        return pymain_exit_err_print();
    }
    runargs = PyTuple_Pack(2, module, set_argv0 ? Py_True : Py_False);
    if (runargs == NULL) {
        fprintf(stderr,
            "Could not create arguments for runpy._run_module_as_main\n");
        Py_DECREF(runpy);
        Py_DECREF(runmodule);
        Py_DECREF(module);
        return pymain_exit_err_print();
    }
    _PyRuntime.signals.unhandled_keyboard_interrupt = 0;
    result = PyObject_Call(runmodule, runargs, NULL);
    if (!result && PyErr_Occurred() == PyExc_KeyboardInterrupt) {
        _PyRuntime.signals.unhandled_keyboard_interrupt = 1;
    }
    Py_DECREF(runpy);
    Py_DECREF(runmodule);
    Py_DECREF(module);
    Py_DECREF(runargs);
    if (result == NULL) {
        return pymain_exit_err_print();
    }
    Py_DECREF(result);
    return 0;
}


static int
pymain_run_file_obj(PyObject *program_name, PyObject *filename,
                    int skip_source_first_line)
{
    if (PySys_Audit("cpython.run_file", "O", filename) < 0) {
        return pymain_exit_err_print();
    }

    FILE *fp = _Py_fopen_obj(filename, "rb");
    if (fp == NULL) {
        // Ignore the OSError
        PyErr_Clear();
        PySys_FormatStderr("%S: can't open file %R: [Errno %d] %s\n",
                           program_name, filename, errno, strerror(errno));
        return 2;
    }

    if (skip_source_first_line) {
        int ch;
        /* Push back first newline so line numbers remain the same */
        while ((ch = getc(fp)) != EOF) {
            if (ch == '\n') {
                (void)ungetc(ch, fp);
                break;
            }
        }
    }

    struct _Py_stat_struct sb;
    if (_Py_fstat_noraise(fileno(fp), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        PySys_FormatStderr("%S: %R is a directory, cannot continue\n",
                           program_name, filename);
        fclose(fp);
        return 1;
    }

    // Call pending calls like signal handlers (SIGINT)
    if (Py_MakePendingCalls() == -1) {
        fclose(fp);
        return pymain_exit_err_print();
    }

    /* PyRun_AnyFileExFlags(closeit=1) calls fclose(fp) before running code */
    PyCompilerFlags cf = _PyCompilerFlags_INIT;
    int run = _PyRun_AnyFileObject(fp, filename, 1, &cf);
    return (run != 0);
}

static int
pymain_run_file(const PyConfig *config)
{
    PyObject *filename = PyUnicode_FromWideChar(config->run_filename, -1);
    if (filename == NULL) {
        PyErr_Print();
        return -1;
    }
    PyObject *program_name = PyUnicode_FromWideChar(config->program_name, -1);
    if (program_name == NULL) {
        Py_DECREF(filename);
        PyErr_Print();
        return -1;
    }

    int res = pymain_run_file_obj(program_name, filename,
                                  config->skip_source_first_line);
    Py_DECREF(filename);
    Py_DECREF(program_name);
    return res;
}


static int
pymain_run_startup(PyConfig *config, int *exitcode)
{
    int ret;
    if (!config->use_environment) {
        return 0;
    }
    PyObject *startup = NULL;
#ifdef MS_WINDOWS
    const wchar_t *env = _wgetenv(L"PYTHONSTARTUP");
    if (env == NULL || env[0] == L'\0') {
        return 0;
    }
    startup = PyUnicode_FromWideChar(env, wcslen(env));
    if (startup == NULL) {
        goto error;
    }
#else
    const char *env = _Py_GetEnv(config->use_environment, "PYTHONSTARTUP");
    if (env == NULL) {
        return 0;
    }
    startup = PyUnicode_DecodeFSDefault(env);
    if (startup == NULL) {
        goto error;
    }
#endif
    if (PySys_Audit("cpython.run_startup", "O", startup) < 0) {
        goto error;
    }

    FILE *fp = _Py_fopen_obj(startup, "r");
    if (fp == NULL) {
        int save_errno = errno;
        PyErr_Clear();
        PySys_WriteStderr("Could not open PYTHONSTARTUP\n");

        errno = save_errno;
        PyErr_SetFromErrnoWithFilenameObjects(PyExc_OSError, startup, NULL);
        goto error;
    }

    PyCompilerFlags cf = _PyCompilerFlags_INIT;
    (void) _PyRun_SimpleFileObject(fp, startup, 0, &cf);
    PyErr_Clear();
    fclose(fp);
    ret = 0;

done:
    Py_XDECREF(startup);
    return ret;

error:
    ret = pymain_err_print(exitcode);
    goto done;
}


/* Write an exitcode into *exitcode and return 1 if we have to exit Python.
   Return 0 otherwise. */
static int
pymain_run_interactive_hook(int *exitcode)
{
    PyObject *sys, *hook, *result;
    sys = PyImport_ImportModule("sys");
    if (sys == NULL) {
        goto error;
    }

    hook = PyObject_GetAttrString(sys, "__interactivehook__");
    Py_DECREF(sys);
    if (hook == NULL) {
        PyErr_Clear();
        return 0;
    }

    if (PySys_Audit("cpython.run_interactivehook", "O", hook) < 0) {
        goto error;
    }

    result = _PyObject_CallNoArgs(hook);
    Py_DECREF(hook);
    if (result == NULL) {
        goto error;
    }
    Py_DECREF(result);

    return 0;

error:
    PySys_WriteStderr("Failed calling sys.__interactivehook__\n");
    return pymain_err_print(exitcode);
}


static void
pymain_set_inspect(PyConfig *config, int inspect)
{
    config->inspect = inspect;
_Py_COMP_DIAG_PUSH
_Py_COMP_DIAG_IGNORE_DEPR_DECLS
    Py_InspectFlag = inspect;
_Py_COMP_DIAG_POP
}


static int
pymain_run_stdin(PyConfig *config)
{
    if (stdin_is_interactive(config)) {
        // do exit on SystemExit
        pymain_set_inspect(config, 0);

        int exitcode;
        if (pymain_run_startup(config, &exitcode)) {
            return exitcode;
        }

        if (pymain_run_interactive_hook(&exitcode)) {
            return exitcode;
        }
    }

    /* call pending calls like signal handlers (SIGINT) */
    if (Py_MakePendingCalls() == -1) {
        return pymain_exit_err_print();
    }

    if (PySys_Audit("cpython.run_stdin", NULL) < 0) {
        return pymain_exit_err_print();
    }

    PyCompilerFlags cf = _PyCompilerFlags_INIT;
    int run = PyRun_AnyFileExFlags(stdin, "<stdin>", 0, &cf);
    return (run != 0);
}


static void
pymain_repl(PyConfig *config, int *exitcode)
{
    /* Check this environment variable at the end, to give programs the
       opportunity to set it from Python. */
    if (!config->inspect && _Py_GetEnv(config->use_environment, "PYTHONINSPECT")) {
        pymain_set_inspect(config, 1);
    }

    if (!(config->inspect && stdin_is_interactive(config) && config_run_code(config))) {
        return;
    }

    pymain_set_inspect(config, 0);
    if (pymain_run_interactive_hook(exitcode)) {
        return;
    }

    PyCompilerFlags cf = _PyCompilerFlags_INIT;
    int res = PyRun_AnyFileFlags(stdin, "<stdin>", &cf);
    *exitcode = (res != 0);
}


static void
pymain_run_python(int *exitcode)
{
    PyObject *main_importer_path = NULL;
    PyInterpreterState *interp = _PyInterpreterState_GET();
    /* pymain_run_stdin() modify the config */
    PyConfig *config = (PyConfig*)_PyInterpreterState_GetConfig(interp);

    /* ensure path config is written into global variables */
    if (_PyStatus_EXCEPTION(_PyPathConfig_UpdateGlobal(config))) {
        goto error;
    }

    if (config->run_filename != NULL) {
        /* If filename is a package (ex: directory or ZIP file) which contains
           __main__.py, main_importer_path is set to filename and will be
           prepended to sys.path.

           Otherwise, main_importer_path is left unchanged. */
        if (pymain_get_importer(config->run_filename, &main_importer_path,
                                exitcode)) {
            return;
        }
    }

    // import readline and rlcompleter before script dir is added to sys.path
    pymain_import_readline(config);

    if (main_importer_path != NULL) {
        if (pymain_sys_path_add_path0(interp, main_importer_path) < 0) {
            goto error;
        }
    }
    else if (!config->safe_path) {
        PyObject *path0 = NULL;
        int res = _PyPathConfig_ComputeSysPath0(&config->argv, &path0);
        if (res < 0) {
            goto error;
        }

        if (res > 0) {
            if (pymain_sys_path_add_path0(interp, path0) < 0) {
                Py_DECREF(path0);
                goto error;
            }
            Py_DECREF(path0);
        }
    }

    pymain_header(config);

    if (config->run_command) {
        *exitcode = pymain_run_command(config->run_command);
    }
    else if (config->run_module) {
        *exitcode = pymain_run_module(config->run_module, 1);
    }
    else if (main_importer_path != NULL) {
        *exitcode = pymain_run_module(L"__main__", 0);
    }
    else if (config->run_filename != NULL) {
        *exitcode = pymain_run_file(config);
    }
    else {
        *exitcode = pymain_run_stdin(config);
    }

    pymain_repl(config, exitcode);
    goto done;

error:
    *exitcode = pymain_exit_err_print();

done:
    Py_XDECREF(main_importer_path);
}


/* --- pymain_main() ---------------------------------------------- */

static void
pymain_free(void)
{
    _PyImport_Fini2();

    /* Free global variables which cannot be freed in Py_Finalize():
       configuration options set before Py_Initialize() which should
       remain valid after Py_Finalize(), since
       Py_Initialize()-Py_Finalize() can be called multiple times. */
    _PyPathConfig_ClearGlobal();
    _Py_ClearStandardStreamEncoding();
    _Py_ClearArgcArgv();
    _PyRuntime_Finalize();
}


static int
exit_sigint(void)
{
    /* bpo-1054041: We need to exit via the
     * SIG_DFL handler for SIGINT if KeyboardInterrupt went unhandled.
     * If we don't, a calling process such as a shell may not know
     * about the user's ^C.  https://www.cons.org/cracauer/sigint.html */
#if defined(HAVE_GETPID) && defined(HAVE_KILL) && !defined(MS_WINDOWS)
    if (PyOS_setsig(SIGINT, SIG_DFL) == SIG_ERR) {
        perror("signal");  /* Impossible in normal environments. */
    } else {
        kill(getpid(), SIGINT);
    }
    /* If setting SIG_DFL failed, or kill failed to terminate us,
     * there isn't much else we can do aside from an error code. */
#endif  /* HAVE_GETPID && !MS_WINDOWS */
#ifdef MS_WINDOWS
    /* cmd.exe detects this, prints ^C, and offers to terminate. */
    /* https://msdn.microsoft.com/en-us/library/cc704588.aspx */
    return STATUS_CONTROL_C_EXIT;
#else
    return SIGINT + 128;
#endif  /* !MS_WINDOWS */
}


static void _Py_NO_RETURN
pymain_exit_error(PyStatus status)
{
    if (_PyStatus_IS_EXIT(status)) {
        /* If it's an error rather than a regular exit, leave Python runtime
           alive: Py_ExitStatusException() uses the current exception and use
           sys.stdout in this case. */
        pymain_free();
    }
    Py_ExitStatusException(status);
}


int
Py_RunMain(void)
{
    int exitcode = 0;

    pymain_run_python(&exitcode);

    if (Py_FinalizeEx() < 0) {
        /* Value unlikely to be confused with a non-error exit status or
           other special meaning */
        exitcode = 120;
    }

    pymain_free();

    if (_PyRuntime.signals.unhandled_keyboard_interrupt) {
        exitcode = exit_sigint();
    }

    return exitcode;
}


static int
pymain_main(_PyArgv *args)
{
    PyStatus status = pymain_init(args);
    if (_PyStatus_IS_EXIT(status)) {
        pymain_free();
        return status.exitcode;
    }
    if (_PyStatus_EXCEPTION(status)) {
        pymain_exit_error(status);
    }

    return Py_RunMain();
}


int
Py_Main(int argc, wchar_t **argv)
{
    _PyArgv args = {
        .argc = argc,
        .use_bytes_argv = 0,
        .bytes_argv = NULL,
        .wchar_argv = argv};
    return pymain_main(&args);
}


int
Py_BytesMain(int argc, char **argv)
{
    _PyArgv args = {
        .argc = argc,
        .use_bytes_argv = 1,
        .bytes_argv = argv,
        .wchar_argv = NULL};
    return pymain_main(&args);
}

#ifdef __cplusplus
}
#endif
