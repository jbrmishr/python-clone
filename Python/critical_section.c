#include "Python.h"

#include "pycore_lock.h"
#include "pycore_critical_section.h"

#ifdef Py_GIL_DISABLED
static_assert(_Alignof(PyCriticalSection) >= 4,
              "critical section must be aligned to at least 4 bytes");
#endif

void
_PyCriticalSection_BeginSlow(PyCriticalSection *c, PyMutex *m)
{
#ifdef Py_GIL_DISABLED
    PyThreadState *tstate = _PyThreadState_GET();
    c->_mutex = NULL;
    c->_prev = (uintptr_t)tstate->critical_section;
    tstate->critical_section = (uintptr_t)c;

    PyMutex_Lock(m);
    c->_mutex = m;
#endif
}

void
_PyCriticalSection2_BeginSlow(PyCriticalSection2 *c, PyMutex *m1, PyMutex *m2,
                              int is_m1_locked)
{
#ifdef Py_GIL_DISABLED
    PyThreadState *tstate = _PyThreadState_GET();
    c->_base._mutex = NULL;
    c->_mutex2 = NULL;
    c->_base._prev = tstate->critical_section;
    tstate->critical_section = (uintptr_t)c | _Py_CRITICAL_SECTION_TWO_MUTEXES;

    if (!is_m1_locked) {
        PyMutex_Lock(m1);
    }
    PyMutex_Lock(m2);
    c->_base._mutex = m1;
    c->_mutex2 = m2;
#endif
}

#ifdef Py_GIL_DISABLED
static PyCriticalSection *
untag_critical_section(uintptr_t tag)
{
    return (PyCriticalSection *)(tag & ~_Py_CRITICAL_SECTION_MASK);
}
#endif

// Release all locks held by critical sections. This is called by
// _PyThreadState_Detach.
void
_PyCriticalSection_SuspendAll(PyThreadState *tstate)
{
#ifdef Py_GIL_DISABLED
    uintptr_t *tagptr = &tstate->critical_section;
    while (_PyCriticalSection_IsActive(*tagptr)) {
        PyCriticalSection *c = untag_critical_section(*tagptr);

        if (c->_mutex) {
            PyMutex_Unlock(c->_mutex);
            if ((*tagptr & _Py_CRITICAL_SECTION_TWO_MUTEXES)) {
                PyCriticalSection2 *c2 = (PyCriticalSection2 *)c;
                if (c2->_mutex2) {
                    PyMutex_Unlock(c2->_mutex2);
                }
            }
        }

        *tagptr |= _Py_CRITICAL_SECTION_INACTIVE;
        tagptr = &c->_prev;
    }
#endif
}

void
_PyCriticalSection_Resume(PyThreadState *tstate)
{
#ifdef Py_GIL_DISABLED
    uintptr_t p = tstate->critical_section;
    PyCriticalSection *c = untag_critical_section(p);
    assert(!_PyCriticalSection_IsActive(p));

    PyMutex *m1 = c->_mutex;
    c->_mutex = NULL;

    PyMutex *m2 = NULL;
    PyCriticalSection2 *c2 = NULL;
    if ((p & _Py_CRITICAL_SECTION_TWO_MUTEXES)) {
        c2 = (PyCriticalSection2 *)c;
        m2 = c2->_mutex2;
        c2->_mutex2 = NULL;
    }

    if (m1) {
        PyMutex_Lock(m1);
    }
    if (m2) {
        PyMutex_Lock(m2);
    }

    c->_mutex = m1;
    if (m2) {
        c2->_mutex2 = m2;
    }

    tstate->critical_section &= ~_Py_CRITICAL_SECTION_INACTIVE;
#endif
}

#undef PyCriticalSection_Begin
void
PyCriticalSection_Begin(PyCriticalSection *c, PyObject *op)
{
#ifdef Py_GIL_DISABLED
    _PyCriticalSection_Begin(c, op);
#endif
}

#undef PyCriticalSection_End
void
PyCriticalSection_End(PyCriticalSection *c)
{
#ifdef Py_GIL_DISABLED
    _PyCriticalSection_End(c);
#endif
}

#undef PyCriticalSection2_Begin
void
PyCriticalSection2_Begin(PyCriticalSection2 *c, PyObject *a, PyObject *b)
{
#ifdef Py_GIL_DISABLED
    _PyCriticalSection2_Begin(c, a, b);
#endif
}

#undef PyCriticalSection2_End
void
PyCriticalSection2_End(PyCriticalSection2 *c)
{
#ifdef Py_GIL_DISABLED
    _PyCriticalSection2_End(c);
#endif
}
