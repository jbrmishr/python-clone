/*
 * Written in 2013 by Dmitry Chestnykh <dmitry@codingrobots.com>
 * Modified for CPython by Christian Heimes <christian@python.org>
 *
 * To the extent possible under law, the author have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty. http://creativecommons.org/publicdomain/zero/1.0/
 */

/* WARNING: autogenerated file!
 *
 * The blake2s_impl.c is autogenerated from blake2b_impl.c.
 */

#include "Python.h"
#include "pystrhex.h"

#include "../hashlib.h"
#include "blake2ns.h"

#define HAVE_BLAKE2B 1
#define BLAKE2_LOCAL_INLINE(type) Py_LOCAL_INLINE(type)

#include "impl/blake2.h"
#include "impl/blake2-impl.h" /* for secure_zero_memory() and store48() */

/* pure SSE2 implementation is very slow, so only use the more optimized SSSE3+
 * https://bugs.python.org/issue31834 */
#if defined(__SSSE3__) || defined(__SSE4_1__) || defined(__AVX__) || defined(__XOP__)
#include "impl/blake2b.c"
#else
#include "impl/blake2b-ref.c"
#endif


extern PyType_Spec blake2b_type_spec;

typedef struct {
    PyObject_HEAD
    blake2b_param    param;
    blake2b_state    state;
    PyThread_type_lock lock;
} BLAKE2bObject;

#include "clinic/blake2b_impl.c.h"

/*[clinic input]
module _blake2
class _blake2.blake2b "BLAKE2bObject *" "&PyBlake2_BLAKE2bType"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=d47b0527b39c673f]*/


static BLAKE2bObject *
new_BLAKE2bObject(PyTypeObject *type)
{
    BLAKE2bObject *self;
    self = (BLAKE2bObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->lock = NULL;
    }
    return self;
}

/*[clinic input]
@classmethod
_blake2.blake2b.__new__ as py_blake2b_new
    data: object(c_default="NULL") = b''
    /
    *
    digest_size: int(c_default="BLAKE2B_OUTBYTES") = _blake2.blake2b.MAX_DIGEST_SIZE
    key: Py_buffer(c_default="NULL", py_default="b''") = None
    salt: Py_buffer(c_default="NULL", py_default="b''") = None
    person: Py_buffer(c_default="NULL", py_default="b''") = None
    fanout: int = 1
    depth: int = 1
    leaf_size: unsigned_long = 0
    node_offset: unsigned_long_long = 0
    node_depth: int = 0
    inner_size: int = 0
    last_node: bool = False
    usedforsecurity: bool = True

Return a new BLAKE2b hash object.
[clinic start generated code]*/

static PyObject *
py_blake2b_new_impl(PyTypeObject *type, PyObject *data, int digest_size,
                    Py_buffer *key, Py_buffer *salt, Py_buffer *person,
                    int fanout, int depth, unsigned long leaf_size,
                    unsigned long long node_offset, int node_depth,
                    int inner_size, int last_node, int usedforsecurity)
/*[clinic end generated code: output=32bfd8f043c6896f input=b947312abff46977]*/
{
    BLAKE2bObject *self = NULL;
    Py_buffer buf;

    self = new_BLAKE2bObject(type);
    if (self == NULL) {
        goto error;
    }

    /* Zero parameter block. */
    memset(&self->param, 0, sizeof(self->param));

    /* Set digest size. */
    if (digest_size <= 0 || digest_size > BLAKE2B_OUTBYTES) {
        PyErr_Format(PyExc_ValueError,
                "digest_size must be between 1 and %d bytes",
                BLAKE2B_OUTBYTES);
        goto error;
    }
    self->param.digest_length = digest_size;

    /* Set salt parameter. */
    if ((salt->obj != NULL) && salt->len) {
        if (salt->len > BLAKE2B_SALTBYTES) {
            PyErr_Format(PyExc_ValueError,
                "maximum salt length is %d bytes",
                BLAKE2B_SALTBYTES);
            goto error;
        }
        memcpy(self->param.salt, salt->buf, salt->len);
    }

    /* Set personalization parameter. */
    if ((person->obj != NULL) && person->len) {
        if (person->len > BLAKE2B_PERSONALBYTES) {
            PyErr_Format(PyExc_ValueError,
                "maximum person length is %d bytes",
                BLAKE2B_PERSONALBYTES);
            goto error;
        }
        memcpy(self->param.personal, person->buf, person->len);
    }

    /* Set tree parameters. */
    if (fanout < 0 || fanout > 255) {
        PyErr_SetString(PyExc_ValueError,
                "fanout must be between 0 and 255");
        goto error;
    }
    self->param.fanout = (uint8_t)fanout;

    if (depth <= 0 || depth > 255) {
        PyErr_SetString(PyExc_ValueError,
                "depth must be between 1 and 255");
        goto error;
    }
    self->param.depth = (uint8_t)depth;

    if (leaf_size > 0xFFFFFFFFU) {
        PyErr_SetString(PyExc_OverflowError, "leaf_size is too large");
        goto error;
    }
    // NB: Simple assignment here would be incorrect on big endian platforms.
    store32(&(self->param.leaf_length), leaf_size);

#ifdef HAVE_BLAKE2S
    if (node_offset > 0xFFFFFFFFFFFFULL) {
        /* maximum 2**48 - 1 */
         PyErr_SetString(PyExc_OverflowError, "node_offset is too large");
         goto error;
     }
    store48(&(self->param.node_offset), node_offset);
#else
    // NB: Simple assignment here would be incorrect on big endian platforms.
    store64(&(self->param.node_offset), node_offset);
#endif

    if (node_depth < 0 || node_depth > 255) {
        PyErr_SetString(PyExc_ValueError,
                "node_depth must be between 0 and 255");
        goto error;
    }
    self->param.node_depth = node_depth;

    if (inner_size < 0 || inner_size > BLAKE2B_OUTBYTES) {
        PyErr_Format(PyExc_ValueError,
                "inner_size must be between 0 and is %d",
                BLAKE2B_OUTBYTES);
        goto error;
    }
    self->param.inner_length = inner_size;

    /* Set key length. */
    if ((key->obj != NULL) && key->len) {
        if (key->len > BLAKE2B_KEYBYTES) {
            PyErr_Format(PyExc_ValueError,
                "maximum key length is %d bytes",
                BLAKE2B_KEYBYTES);
            goto error;
        }
        self->param.key_length = (uint8_t)key->len;
    }

    /* Initialize hash state. */
    if (blake2b_init_param(&self->state, &self->param) < 0) {
        PyErr_SetString(PyExc_RuntimeError,
                "error initializing hash state");
        goto error;
    }

    /* Set last node flag (must come after initialization). */
    self->state.last_node = last_node;

    /* Process key block if any. */
    if (self->param.key_length) {
        uint8_t block[BLAKE2B_BLOCKBYTES];
        memset(block, 0, sizeof(block));
        memcpy(block, key->buf, key->len);
        blake2b_update(&self->state, block, sizeof(block));
        secure_zero_memory(block, sizeof(block));
    }

    /* Process initial data if any. */
    if (data != NULL) {
        GET_BUFFER_VIEW_OR_ERROR(data, &buf, goto error);

        if (buf.len >= HASHLIB_GIL_MINSIZE) {
            Py_BEGIN_ALLOW_THREADS
            blake2b_update(&self->state, buf.buf, buf.len);
            Py_END_ALLOW_THREADS
        } else {
            blake2b_update(&self->state, buf.buf, buf.len);
        }
        PyBuffer_Release(&buf);
    }

    return (PyObject *)self;

  error:
    if (self != NULL) {
        Py_DECREF(self);
    }
    return NULL;
}

/*[clinic input]
_blake2.blake2b.copy

Return a copy of the hash object.
[clinic start generated code]*/

static PyObject *
_blake2_blake2b_copy_impl(BLAKE2bObject *self)
/*[clinic end generated code: output=ff6acee5f93656ae input=e383c2d199fd8a2e]*/
{
    BLAKE2bObject *cpy;

    if ((cpy = new_BLAKE2bObject(Py_TYPE(self))) == NULL)
        return NULL;

    ENTER_HASHLIB(self);
    cpy->param = self->param;
    cpy->state = self->state;
    LEAVE_HASHLIB(self);
    return (PyObject *)cpy;
}

/*[clinic input]
_blake2.blake2b.update

    data: object
    /

Update this hash object's state with the provided bytes-like object.
[clinic start generated code]*/

static PyObject *
_blake2_blake2b_update(BLAKE2bObject *self, PyObject *data)
/*[clinic end generated code: output=010dfcbe22654359 input=ffc4aa6a6a225d31]*/
{
    Py_buffer buf;

    GET_BUFFER_VIEW_OR_ERROUT(data, &buf);

    if (self->lock == NULL && buf.len >= HASHLIB_GIL_MINSIZE)
        self->lock = PyThread_allocate_lock();

    if (self->lock != NULL) {
       Py_BEGIN_ALLOW_THREADS
       PyThread_acquire_lock(self->lock, 1);
       blake2b_update(&self->state, buf.buf, buf.len);
       PyThread_release_lock(self->lock);
       Py_END_ALLOW_THREADS
    } else {
        blake2b_update(&self->state, buf.buf, buf.len);
    }
    PyBuffer_Release(&buf);

    Py_RETURN_NONE;
}

/*[clinic input]
_blake2.blake2b.digest

Return the digest value as a bytes object.
[clinic start generated code]*/

static PyObject *
_blake2_blake2b_digest_impl(BLAKE2bObject *self)
/*[clinic end generated code: output=a5864660f4bfc61a input=7d21659e9c5fff02]*/
{
    uint8_t digest[BLAKE2B_OUTBYTES];
    blake2b_state state_cpy;

    ENTER_HASHLIB(self);
    state_cpy = self->state;
    blake2b_final(&state_cpy, digest, self->param.digest_length);
    LEAVE_HASHLIB(self);
    return PyBytes_FromStringAndSize((const char *)digest,
            self->param.digest_length);
}

/*[clinic input]
_blake2.blake2b.hexdigest

Return the digest value as a string of hexadecimal digits.
[clinic start generated code]*/

static PyObject *
_blake2_blake2b_hexdigest_impl(BLAKE2bObject *self)
/*[clinic end generated code: output=b5598a87d8794a60 input=76930f6946351f56]*/
{
    uint8_t digest[BLAKE2B_OUTBYTES];
    blake2b_state state_cpy;

    ENTER_HASHLIB(self);
    state_cpy = self->state;
    blake2b_final(&state_cpy, digest, self->param.digest_length);
    LEAVE_HASHLIB(self);
    return _Py_strhex((const char *)digest, self->param.digest_length);
}


static PyMethodDef py_blake2b_methods[] = {
    _BLAKE2_BLAKE2B_COPY_METHODDEF
    _BLAKE2_BLAKE2B_DIGEST_METHODDEF
    _BLAKE2_BLAKE2B_HEXDIGEST_METHODDEF
    _BLAKE2_BLAKE2B_UPDATE_METHODDEF
    {NULL, NULL}
};



static PyObject *
py_blake2b_get_name(BLAKE2bObject *self, void *closure)
{
    return PyUnicode_FromString("blake2b");
}



static PyObject *
py_blake2b_get_block_size(BLAKE2bObject *self, void *closure)
{
    return PyLong_FromLong(BLAKE2B_BLOCKBYTES);
}



static PyObject *
py_blake2b_get_digest_size(BLAKE2bObject *self, void *closure)
{
    return PyLong_FromLong(self->param.digest_length);
}


static PyGetSetDef py_blake2b_getsetters[] = {
    {"name", (getter)py_blake2b_get_name,
        NULL, NULL, NULL},
    {"block_size", (getter)py_blake2b_get_block_size,
        NULL, NULL, NULL},
    {"digest_size", (getter)py_blake2b_get_digest_size,
        NULL, NULL, NULL},
    {NULL}
};


static void
py_blake2b_dealloc(PyObject *self)
{
    BLAKE2bObject *obj = (BLAKE2bObject *)self;

    /* Try not to leave state in memory. */
    secure_zero_memory(&obj->param, sizeof(obj->param));
    secure_zero_memory(&obj->state, sizeof(obj->state));
    if (obj->lock) {
        PyThread_free_lock(obj->lock);
        obj->lock = NULL;
    }
    PyObject_Del(self);
}

static PyType_Slot blake2b_type_slots[] = {
    {Py_tp_dealloc, py_blake2b_dealloc},
    {Py_tp_doc, (char *)py_blake2b_new__doc__},
    {Py_tp_methods, py_blake2b_methods},
    {Py_tp_getset, py_blake2b_getsetters},
    {Py_tp_new, py_blake2b_new},
    {0,0}
};

PyType_Spec blake2b_type_spec = {
    .name = "_blake2.blake2b",
    .basicsize =  sizeof(BLAKE2bObject),
    .flags = Py_TPFLAGS_DEFAULT,
    .slots = blake2b_type_slots
};
