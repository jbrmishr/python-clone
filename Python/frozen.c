
/* Frozen modules initializer
 *
 * Frozen modules are written to header files by Programs/_freeze_module.
 * These files are typically put in Python/frozen_modules/.  Each holds
 * an array of bytes named "_Py_M__<module>", which is used below.
 *
 * These files must be regenerated any time the corresponding .pyc
 * file would change (including with changes to the compiler, bytecode
 * format, marshal format).  This can be done with "make regen-frozen".
 * That make target just runs Tools/scripts/freeze_modules.py.
 *
 * The freeze_modules.py script also determines which modules get
 * frozen.  Update the list at the top of the script to add, remove,
 * or modify the target modules.  Then run the script
 * (or run "make regen-frozen").
 *
 * The script does the following:
 *
 * 1. run Programs/_freeze_module on the target modules
 * 2. update the includes and _PyImport_FrozenModules[] in this file
 * 3. update the FROZEN_FILES variable in Makefile.pre.in
 * 4. update the per-module targets in Makefile.pre.in
 * 5. update the lists of modules in PCbuild/_freeze_module.vcxproj and
 *    PCbuild/_freeze_module.vcxproj.filters
 *
 * (Note that most of the data in this file is auto-generated by the script.)
 *
 * Those steps can also be done manually, though this is not recommended.
 * Expect such manual changes to be removed the next time
 * freeze_modules.py runs.
 * */

/* In order to test the support for frozen modules, by default we
   define some simple frozen modules: __hello__, __phello__ (a package),
   and __phello__.spam.  Loading any will print some famous words... */

#include "Python.h"
#include "pycore_import.h"

/* Includes for frozen modules: */
#include "frozen_modules/importlib._bootstrap.h"
#include "frozen_modules/importlib._bootstrap_external.h"
#include "frozen_modules/zipimport.h"
#include "frozen_modules/getpath.h"
#include "frozen_modules/abc.h"
#include "frozen_modules/codecs.h"
#include "frozen_modules/io.h"
#include "frozen_modules/_collections_abc.h"
#include "frozen_modules/_sitebuiltins.h"
#include "frozen_modules/genericpath.h"
#include "frozen_modules/ntpath.h"
#include "frozen_modules/posixpath.h"
#include "frozen_modules/os.h"
#include "frozen_modules/site.h"
#include "frozen_modules/stat.h"
#include "frozen_modules/__hello__.h"
#include "frozen_modules/__phello__.h"
#include "frozen_modules/__phello__.ham.h"
#include "frozen_modules/__phello__.ham.eggs.h"
#include "frozen_modules/__phello__.spam.h"
#include "frozen_modules/frozen_only.h"
/* End includes */

#ifdef MS_WINDOWS
/* Deepfreeze isn't supported on Windows yet. */
#define GET_CODE(name) NULL
#else
#define GET_CODE(name) _Py_get_##name##_toplevel
#endif

/* Start extern declarations */
extern PyObject *_Py_get_importlib__bootstrap_toplevel(void);
extern PyObject *_Py_get_importlib__bootstrap_external_toplevel(void);
extern PyObject *_Py_get_zipimport_toplevel(void);
extern PyObject *_Py_get_abc_toplevel(void);
extern PyObject *_Py_get_codecs_toplevel(void);
extern PyObject *_Py_get_io_toplevel(void);
extern PyObject *_Py_get__collections_abc_toplevel(void);
extern PyObject *_Py_get__sitebuiltins_toplevel(void);
extern PyObject *_Py_get_genericpath_toplevel(void);
extern PyObject *_Py_get_ntpath_toplevel(void);
extern PyObject *_Py_get_posixpath_toplevel(void);
extern PyObject *_Py_get_posixpath_toplevel(void);
extern PyObject *_Py_get_os_toplevel(void);
extern PyObject *_Py_get_site_toplevel(void);
extern PyObject *_Py_get_stat_toplevel(void);
extern PyObject *_Py_get___hello___toplevel(void);
extern PyObject *_Py_get___hello___toplevel(void);
extern PyObject *_Py_get___hello___toplevel(void);
extern PyObject *_Py_get___hello___toplevel(void);
extern PyObject *_Py_get___phello___toplevel(void);
extern PyObject *_Py_get___phello___toplevel(void);
extern PyObject *_Py_get___phello___ham_toplevel(void);
extern PyObject *_Py_get___phello___ham_toplevel(void);
extern PyObject *_Py_get___phello___ham_eggs_toplevel(void);
extern PyObject *_Py_get___phello___spam_toplevel(void);
extern PyObject *_Py_get_frozen_only_toplevel(void);
/* End extern declarations */

/* Note that a negative size indicates a package. */

static const struct _frozen bootstrap_modules[] = {
    {"_frozen_importlib", _Py_M__importlib__bootstrap, (int)sizeof(_Py_M__importlib__bootstrap), GET_CODE(importlib__bootstrap)},
    {"_frozen_importlib_external", _Py_M__importlib__bootstrap_external, (int)sizeof(_Py_M__importlib__bootstrap_external), GET_CODE(importlib__bootstrap_external)},
    {"zipimport", _Py_M__zipimport, (int)sizeof(_Py_M__zipimport), GET_CODE(zipimport)},
    {0, 0, 0} /* bootstrap sentinel */
};
static const struct _frozen stdlib_modules[] = {
    /* stdlib - startup, without site (python -S) */
    {"abc", _Py_M__abc, (int)sizeof(_Py_M__abc), GET_CODE(abc)},
    {"codecs", _Py_M__codecs, (int)sizeof(_Py_M__codecs), GET_CODE(codecs)},
    {"io", _Py_M__io, (int)sizeof(_Py_M__io), GET_CODE(io)},

    /* stdlib - startup, with site */
    {"_collections_abc", _Py_M___collections_abc, (int)sizeof(_Py_M___collections_abc), GET_CODE(_collections_abc)},
    {"_sitebuiltins", _Py_M___sitebuiltins, (int)sizeof(_Py_M___sitebuiltins), GET_CODE(_sitebuiltins)},
    {"genericpath", _Py_M__genericpath, (int)sizeof(_Py_M__genericpath), GET_CODE(genericpath)},
    {"ntpath", _Py_M__ntpath, (int)sizeof(_Py_M__ntpath), GET_CODE(ntpath)},
    {"posixpath", _Py_M__posixpath, (int)sizeof(_Py_M__posixpath), GET_CODE(posixpath)},
    {"os.path", _Py_M__posixpath, (int)sizeof(_Py_M__posixpath), GET_CODE(posixpath)},
    {"os", _Py_M__os, (int)sizeof(_Py_M__os), GET_CODE(os)},
    {"site", _Py_M__site, (int)sizeof(_Py_M__site), GET_CODE(site)},
    {"stat", _Py_M__stat, (int)sizeof(_Py_M__stat), GET_CODE(stat)},
    {0, 0, 0} /* stdlib sentinel */
};
static const struct _frozen test_modules[] = {
    {"__hello__", _Py_M____hello__, (int)sizeof(_Py_M____hello__), GET_CODE(__hello__)},
    {"__hello_alias__", _Py_M____hello__, (int)sizeof(_Py_M____hello__), GET_CODE(__hello__)},
    {"__phello_alias__", _Py_M____hello__, -(int)sizeof(_Py_M____hello__), GET_CODE(__hello__)},
    {"__phello_alias__.spam", _Py_M____hello__, (int)sizeof(_Py_M____hello__), GET_CODE(__hello__)},
    {"__phello__", _Py_M____phello__, -(int)sizeof(_Py_M____phello__), GET_CODE(__phello__)},
    {"__phello__.__init__", _Py_M____phello__, (int)sizeof(_Py_M____phello__), GET_CODE(__phello__)},
    {"__phello__.ham", _Py_M____phello___ham, -(int)sizeof(_Py_M____phello___ham), GET_CODE(__phello___ham)},
    {"__phello__.ham.__init__", _Py_M____phello___ham, (int)sizeof(_Py_M____phello___ham), GET_CODE(__phello___ham)},
    {"__phello__.ham.eggs", _Py_M____phello___ham_eggs, (int)sizeof(_Py_M____phello___ham_eggs), GET_CODE(__phello___ham_eggs)},
    {"__phello__.spam", _Py_M____phello___spam, (int)sizeof(_Py_M____phello___spam), GET_CODE(__phello___spam)},
    {"__hello_only__", _Py_M__frozen_only, (int)sizeof(_Py_M__frozen_only), GET_CODE(frozen_only)},
    {0, 0, 0} /* test sentinel */
};
const struct _frozen *_PyImport_FrozenBootstrap = bootstrap_modules;
const struct _frozen *_PyImport_FrozenStdlib = stdlib_modules;
const struct _frozen *_PyImport_FrozenTest = test_modules;

static const struct _module_alias aliases[] = {
    {"_frozen_importlib", "importlib._bootstrap"},
    {"_frozen_importlib_external", "importlib._bootstrap_external"},
    {"os.path", "posixpath"},
    {"__hello_alias__", "__hello__"},
    {"__phello_alias__", "__hello__"},
    {"__phello_alias__.spam", "__hello__"},
    {"__phello__.__init__", "<__phello__"},
    {"__phello__.ham.__init__", "<__phello__.ham"},
    {"__hello_only__", NULL},
    {0, 0} /* aliases sentinel */
};
const struct _module_alias *_PyImport_FrozenAliases = aliases;


/* Embedding apps may change this pointer to point to their favorite
   collection of frozen modules: */

const struct _frozen *PyImport_FrozenModules = NULL;
