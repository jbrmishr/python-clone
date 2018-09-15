#ifndef Py_INTERNAL_CEVAL_H
#define Py_INTERNAL_CEVAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "pyatomic.h"
#include "pythread.h"

struct _is;  // See PyInterpreterState in pystate.h.

PyAPI_FUNC(int) _Py_AddPendingCall(struct _is*, unsigned long, int (*)(void *), void *);
PyAPI_FUNC(int) _Py_MakePendingCalls(struct _is*);

struct _pending_calls {
    PyThread_type_lock lock;
    /* Request for running pending calls. */
    _Py_atomic_int calls_to_do;
    /* Request for looking at the `async_exc` field of the current
       thread state.
       Guarded by the GIL. */
    int async_exc;
#define NPENDINGCALLS 32
    struct {
        unsigned long thread_id;
        int (*func)(void *);
        void *arg;
    } calls[NPENDINGCALLS];
    int first;
    int last;
};

#include "internal/gil.h"

struct _ceval_runtime_state {
    int recursion_limit;
    /* Records whether tracing is on for any thread.  Counts the number
       of threads for which tstate->c_tracefunc is non-NULL, so if the
       value is 0, we know we don't have to check this thread's
       c_tracefunc.  This speeds up the if statement in
       PyEval_EvalFrameEx() after fast_next_opcode. */
    int tracing_possible;
    /* Request for dropping the GIL */
    _Py_atomic_int gil_drop_request;
    struct _gil_runtime_state gil;
};

PyAPI_FUNC(void) _PyEval_Initialize(struct _ceval_runtime_state *);

#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_CEVAL_H */
