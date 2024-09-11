/*[clinic input]
preserve
[clinic start generated code]*/

#include "pycore_modsupport.h"    // _PyArg_CheckPositional()

PyDoc_STRVAR(_contextvars_Context___enter____doc__,
"__enter__($self, /)\n"
"--\n"
"\n"
"Context manager enter.\n"
"\n"
"Automatically called by the \'with\' statement.  Using the Context object as a\n"
"context manager is an alternative to calling the Context.run() method.\n"
"\n"
"Example:\n"
"\n"
"    var = contextvars.ContextVar(\'var\')\n"
"    var.set(\'initial\')\n"
"\n"
"    with contextvars.copy_context():\n"
"        var.set(\'updated\')\n"
"        print(var.get())  # \'updated\'\n"
"\n"
"    print(var.get())  # \'initial\'");

#define _CONTEXTVARS_CONTEXT___ENTER___METHODDEF    \
    {"__enter__", (PyCFunction)_contextvars_Context___enter__, METH_NOARGS, _contextvars_Context___enter____doc__},

static PyObject *
_contextvars_Context___enter___impl(PyContext *self);

static PyObject *
_contextvars_Context___enter__(PyContext *self, PyObject *Py_UNUSED(ignored))
{
    return _contextvars_Context___enter___impl(self);
}

PyDoc_STRVAR(_contextvars_Context___exit____doc__,
"__exit__($self, exc_type, exc_val, exc_tb, /)\n"
"--\n"
"\n"
"Context manager exit.\n"
"\n"
"Automatically called at the conclusion of a \'with\' statement when the Context\n"
"is used as a context manager.  See the Context.__enter__() method for more\n"
"details.");

#define _CONTEXTVARS_CONTEXT___EXIT___METHODDEF    \
    {"__exit__", _PyCFunction_CAST(_contextvars_Context___exit__), METH_FASTCALL, _contextvars_Context___exit____doc__},

static PyObject *
_contextvars_Context___exit___impl(PyContext *self, PyObject *exc_type,
                                   PyObject *exc_val, PyObject *exc_tb);

static PyObject *
_contextvars_Context___exit__(PyContext *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *exc_type;
    PyObject *exc_val;
    PyObject *exc_tb;

    if (!_PyArg_CheckPositional("__exit__", nargs, 3, 3)) {
        goto exit;
    }
    exc_type = args[0];
    exc_val = args[1];
    exc_tb = args[2];
    return_value = _contextvars_Context___exit___impl(self, exc_type, exc_val, exc_tb);

exit:
    return return_value;
}

PyDoc_STRVAR(_contextvars_Context_get__doc__,
"get($self, key, default=None, /)\n"
"--\n"
"\n"
"Return the value for `key` if `key` has the value in the context object.\n"
"\n"
"If `key` does not exist, return `default`. If `default` is not given,\n"
"return None.");

#define _CONTEXTVARS_CONTEXT_GET_METHODDEF    \
    {"get", _PyCFunction_CAST(_contextvars_Context_get), METH_FASTCALL, _contextvars_Context_get__doc__},

static PyObject *
_contextvars_Context_get_impl(PyContext *self, PyObject *key,
                              PyObject *default_value);

static PyObject *
_contextvars_Context_get(PyContext *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *key;
    PyObject *default_value = Py_None;

    if (!_PyArg_CheckPositional("get", nargs, 1, 2)) {
        goto exit;
    }
    key = args[0];
    if (nargs < 2) {
        goto skip_optional;
    }
    default_value = args[1];
skip_optional:
    return_value = _contextvars_Context_get_impl(self, key, default_value);

exit:
    return return_value;
}

PyDoc_STRVAR(_contextvars_Context_items__doc__,
"items($self, /)\n"
"--\n"
"\n"
"Return all variables and their values in the context object.\n"
"\n"
"The result is returned as a list of 2-tuples (variable, value).");

#define _CONTEXTVARS_CONTEXT_ITEMS_METHODDEF    \
    {"items", (PyCFunction)_contextvars_Context_items, METH_NOARGS, _contextvars_Context_items__doc__},

static PyObject *
_contextvars_Context_items_impl(PyContext *self);

static PyObject *
_contextvars_Context_items(PyContext *self, PyObject *Py_UNUSED(ignored))
{
    return _contextvars_Context_items_impl(self);
}

PyDoc_STRVAR(_contextvars_Context_keys__doc__,
"keys($self, /)\n"
"--\n"
"\n"
"Return a list of all variables in the context object.");

#define _CONTEXTVARS_CONTEXT_KEYS_METHODDEF    \
    {"keys", (PyCFunction)_contextvars_Context_keys, METH_NOARGS, _contextvars_Context_keys__doc__},

static PyObject *
_contextvars_Context_keys_impl(PyContext *self);

static PyObject *
_contextvars_Context_keys(PyContext *self, PyObject *Py_UNUSED(ignored))
{
    return _contextvars_Context_keys_impl(self);
}

PyDoc_STRVAR(_contextvars_Context_values__doc__,
"values($self, /)\n"
"--\n"
"\n"
"Return a list of all variables\' values in the context object.");

#define _CONTEXTVARS_CONTEXT_VALUES_METHODDEF    \
    {"values", (PyCFunction)_contextvars_Context_values, METH_NOARGS, _contextvars_Context_values__doc__},

static PyObject *
_contextvars_Context_values_impl(PyContext *self);

static PyObject *
_contextvars_Context_values(PyContext *self, PyObject *Py_UNUSED(ignored))
{
    return _contextvars_Context_values_impl(self);
}

PyDoc_STRVAR(_contextvars_Context_copy__doc__,
"copy($self, /)\n"
"--\n"
"\n"
"Return a shallow copy of the context object.");

#define _CONTEXTVARS_CONTEXT_COPY_METHODDEF    \
    {"copy", (PyCFunction)_contextvars_Context_copy, METH_NOARGS, _contextvars_Context_copy__doc__},

static PyObject *
_contextvars_Context_copy_impl(PyContext *self);

static PyObject *
_contextvars_Context_copy(PyContext *self, PyObject *Py_UNUSED(ignored))
{
    return _contextvars_Context_copy_impl(self);
}

PyDoc_STRVAR(_contextvars_ContextVar_get__doc__,
"get($self, default=<unrepresentable>, /)\n"
"--\n"
"\n"
"Return a value for the context variable for the current context.\n"
"\n"
"If there is no value for the variable in the current context, the method will:\n"
" * return the value of the default argument of the method, if provided; or\n"
" * return the default value for the context variable, if it was created\n"
"   with one; or\n"
" * raise a LookupError.");

#define _CONTEXTVARS_CONTEXTVAR_GET_METHODDEF    \
    {"get", _PyCFunction_CAST(_contextvars_ContextVar_get), METH_FASTCALL, _contextvars_ContextVar_get__doc__},

static PyObject *
_contextvars_ContextVar_get_impl(PyContextVar *self, PyObject *default_value);

static PyObject *
_contextvars_ContextVar_get(PyContextVar *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *default_value = NULL;

    if (!_PyArg_CheckPositional("get", nargs, 0, 1)) {
        goto exit;
    }
    if (nargs < 1) {
        goto skip_optional;
    }
    default_value = args[0];
skip_optional:
    return_value = _contextvars_ContextVar_get_impl(self, default_value);

exit:
    return return_value;
}

PyDoc_STRVAR(_contextvars_ContextVar_set__doc__,
"set($self, value, /)\n"
"--\n"
"\n"
"Call to set a new value for the context variable in the current context.\n"
"\n"
"The required value argument is the new value for the context variable.\n"
"\n"
"Returns a Token object that can be used to restore the variable to its previous\n"
"value via the `ContextVar.reset()` method.");

#define _CONTEXTVARS_CONTEXTVAR_SET_METHODDEF    \
    {"set", (PyCFunction)_contextvars_ContextVar_set, METH_O, _contextvars_ContextVar_set__doc__},

PyDoc_STRVAR(_contextvars_ContextVar_reset__doc__,
"reset($self, token, /)\n"
"--\n"
"\n"
"Reset the context variable.\n"
"\n"
"The variable is reset to the value it had before the `ContextVar.set()` that\n"
"created the token was used.");

#define _CONTEXTVARS_CONTEXTVAR_RESET_METHODDEF    \
    {"reset", (PyCFunction)_contextvars_ContextVar_reset, METH_O, _contextvars_ContextVar_reset__doc__},
/*[clinic end generated code: output=68e3b8eb96ff5dc8 input=a9049054013a1b77]*/
