
#include "Python.h"
#include <sys/resource.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

/* On some systems, these aren't in any header file.
   On others they are, with inconsistent prototypes.
   We declare the (default) return type, to shut up gcc -Wall;
   but we can't declare the prototype, to avoid errors
   when the header files declare it different.
   Worse, on some Linuxes, getpagesize() returns a size_t... */

#define doubletime(TV) ((double)(TV).tv_sec + (TV).tv_usec * 0.000001)

static PyObject *ResourceError;

static PyObject *
resource_getrusage(PyObject *self, PyObject *args)
{
	int who;
	struct rusage ru;

	if (!PyArg_ParseTuple(args, "i:getrusage", &who))
		return NULL;

	if (getrusage(who, &ru) == -1) {
		if (errno == EINVAL) {
			PyErr_SetString(PyExc_ValueError,
					"invalid who parameter");
			return NULL;
		} 
		PyErr_SetFromErrno(ResourceError);
		return NULL;
	}

	/* Yeah, this 16-tuple is way ugly. It's probably a lot less
	   ugly than a dictionary with keys (or object attributes)
	   named things like 'ixrss'. 
	   */
	return Py_BuildValue(
		"ddiiiiiiiiiiiiii",
		doubletime(ru.ru_utime),     /* user time used */
		doubletime(ru.ru_stime),     /* system time used */
		ru.ru_maxrss,		     /* max. resident set size */
		ru.ru_ixrss,		     /* shared memory size */
		ru.ru_idrss,		     /* unshared memory size */
		ru.ru_isrss,		     /* unshared stack size */
		ru.ru_minflt,		     /* page faults not requiring I/O*/
		ru.ru_majflt,		     /* page faults requiring I/O */
		ru.ru_nswap,		     /* number of swap outs */
		ru.ru_inblock,		     /* block input operations */
		ru.ru_oublock,		     /* block output operations */
		ru.ru_msgsnd,		     /* messages sent */
		ru.ru_msgrcv,		     /* messages received */
		ru.ru_nsignals,		     /* signals received */
		ru.ru_nvcsw,		     /* voluntary context switches */
		ru.ru_nivcsw		     /* involuntary context switches */
		);
}


static PyObject *
resource_getrlimit(PyObject *self, PyObject *args)
{
	struct rlimit rl;
	int resource;

	if (!PyArg_ParseTuple(args, "i:getrlimit", &resource)) 
		return NULL;

	if (resource < 0 || resource >= RLIM_NLIMITS) {
		PyErr_SetString(PyExc_ValueError,
				"invalid resource specified");
		return NULL;
	}

	if (getrlimit(resource, &rl) == -1) {
		PyErr_SetFromErrno(ResourceError);
		return NULL;
	}

#if defined(HAVE_LONG_LONG)
	if (sizeof(rl.rlim_cur) > sizeof(long)) {
		return Py_BuildValue("LL",
				     (LONG_LONG) rl.rlim_cur,
				     (LONG_LONG) rl.rlim_max);
	}
#endif
	return Py_BuildValue("ii", (long) rl.rlim_cur, (long) rl.rlim_max);
}

static PyObject *
resource_setrlimit(PyObject *self, PyObject *args)
{
	struct rlimit rl;
	int resource;
	PyObject *curobj, *maxobj;

	if (!PyArg_ParseTuple(args, "i(OO):setrlimit", &resource, &curobj, &maxobj))
		return NULL;

	if (resource < 0 || resource >= RLIM_NLIMITS) {
		PyErr_SetString(PyExc_ValueError,
				"invalid resource specified");
		return NULL;
	}

#if !defined(HAVE_LARGEFILE_SUPPORT)
	rl.rlim_cur = PyInt_AsLong(curobj);
	rl.rlim_max = PyInt_AsLong(maxobj);
#else
	/* The limits are probably bigger than a long */
	rl.rlim_cur = PyLong_Check(curobj) ?
		PyLong_AsLongLong(curobj) : PyInt_AsLong(curobj);
	rl.rlim_max = PyLong_Check(maxobj) ?
		PyLong_AsLongLong(maxobj) : PyInt_AsLong(maxobj);
#endif

	rl.rlim_cur = rl.rlim_cur & RLIM_INFINITY;
	rl.rlim_max = rl.rlim_max & RLIM_INFINITY;
	if (setrlimit(resource, &rl) == -1) {
		if (errno == EINVAL) 
			PyErr_SetString(PyExc_ValueError,
					"current limit exceeds maximum limit");
		else if (errno == EPERM)
			PyErr_SetString(PyExc_ValueError,
					"not allowed to raise maximum limit");
		else
			PyErr_SetFromErrno(ResourceError);
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
resource_getpagesize(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":getpagesize"))
		return NULL;
	return Py_BuildValue("i", getpagesize());
}

/* List of functions */

static struct PyMethodDef
resource_methods[] = {
	{"getrusage",    resource_getrusage,   METH_VARARGS},
	{"getrlimit",    resource_getrlimit,   METH_VARARGS},
	{"setrlimit",    resource_setrlimit,   METH_VARARGS},
	{"getpagesize",  resource_getpagesize, METH_VARARGS},
	{NULL, NULL}			     /* sentinel */
};


/* Module initialization */

DL_EXPORT(void)
initresource(void)
{
	PyObject *m;

	/* Create the module and add the functions */
	m = Py_InitModule("resource", resource_methods);

	/* Add some symbolic constants to the module */
	if (ResourceError == NULL) {
		ResourceError = PyErr_NewException("resource.error",
						   NULL, NULL);
	}
	Py_INCREF(ResourceError);
	PyModule_AddObject(m, "error", ResourceError);

	/* insert constants */
#ifdef RLIMIT_CPU
	PyModule_AddIntConstant(m, "RLIMIT_CPU", RLIMIT_CPU);
#endif

#ifdef RLIMIT_FSIZE
	PyModule_AddIntConstant(m, "RLIMIT_FSIZE", RLIMIT_FSIZE);
#endif

#ifdef RLIMIT_DATA
	PyModule_AddIntConstant(m, "RLIMIT_DATA", RLIMIT_DATA);
#endif

#ifdef RLIMIT_STACK
	PyModule_AddIntConstant(m, "RLIMIT_STACK", RLIMIT_STACK);
#endif

#ifdef RLIMIT_CORE
	PyModule_AddIntConstant(m, "RLIMIT_CORE", RLIMIT_CORE);
#endif

#ifdef RLIMIT_NOFILE
	PyModule_AddIntConstant(m, "RLIMIT_NOFILE", RLIMIT_NOFILE);
#endif

#ifdef RLIMIT_OFILE
	PyModule_AddIntConstant(m, "RLIMIT_OFILE", RLIMIT_OFILE);
#endif

#ifdef RLIMIT_VMEM
	PyModule_AddIntConstant(m, "RLIMIT_VMEM", RLIMIT_VMEM);
#endif

#ifdef RLIMIT_AS
	PyModule_AddIntConstant(m, "RLIMIT_AS", RLIMIT_AS);
#endif

#ifdef RLIMIT_RSS
	PyModule_AddIntConstant(m, "RLIMIT_RSS", RLIMIT_RSS);
#endif

#ifdef RLIMIT_NPROC
	PyModule_AddIntConstant(m, "RLIMIT_NPROC", RLIMIT_NPROC);
#endif

#ifdef RLIMIT_MEMLOCK
	PyModule_AddIntConstant(m, "RLIMIT_MEMLOCK", RLIMIT_MEMLOCK);
#endif

#ifdef RUSAGE_SELF
	PyModule_AddIntConstant(m, "RUSAGE_SELF", RUSAGE_SELF);
#endif

#ifdef RUSAGE_CHILDREN
	PyModule_AddIntConstant(m, "RUSAGE_CHILDREN", RUSAGE_CHILDREN);
#endif

#ifdef RUSAGE_BOTH
	PyModule_AddIntConstant(m, "RUSAGE_BOTH", RUSAGE_BOTH);
#endif
}
