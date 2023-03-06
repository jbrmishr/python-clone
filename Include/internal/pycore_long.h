#ifndef Py_INTERNAL_LONG_H
#define Py_INTERNAL_LONG_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif

#include "pycore_global_objects.h"  // _PY_NSMALLNEGINTS
#include "pycore_runtime.h"       // _PyRuntime

/*
 * Default int base conversion size limitation: Denial of Service prevention.
 *
 * Chosen such that this isn't wildly slow on modern hardware and so that
 * everyone's existing deployed numpy test suite passes before
 * https://github.com/numpy/numpy/issues/22098 is widely available.
 *
 * $ python -m timeit -s 's = "1"*4300' 'int(s)'
 * 2000 loops, best of 5: 125 usec per loop
 * $ python -m timeit -s 's = "1"*4300; v = int(s)' 'str(v)'
 * 1000 loops, best of 5: 311 usec per loop
 * (zen2 cloud VM)
 *
 * 4300 decimal digits fits a ~14284 bit number.
 */
#define _PY_LONG_DEFAULT_MAX_STR_DIGITS 4300
/*
 * Threshold for max digits check.  For performance reasons int() and
 * int.__str__() don't checks values that are smaller than this
 * threshold.  Acts as a guaranteed minimum size limit for bignums that
 * applications can expect from CPython.
 *
 * % python -m timeit -s 's = "1"*640; v = int(s)' 'str(int(s))'
 * 20000 loops, best of 5: 12 usec per loop
 *
 * "640 digits should be enough for anyone." - gps
 * fits a ~2126 bit decimal number.
 */
#define _PY_LONG_MAX_STR_DIGITS_THRESHOLD 640

#if ((_PY_LONG_DEFAULT_MAX_STR_DIGITS != 0) && \
   (_PY_LONG_DEFAULT_MAX_STR_DIGITS < _PY_LONG_MAX_STR_DIGITS_THRESHOLD))
# error "_PY_LONG_DEFAULT_MAX_STR_DIGITS smaller than threshold."
#endif


/* runtime lifecycle */

extern PyStatus _PyLong_InitTypes(PyInterpreterState *);
extern void _PyLong_FiniTypes(PyInterpreterState *interp);


/* other API */

#define _PyLong_SMALL_INTS _Py_SINGLETON(small_ints)

// _PyLong_GetZero() and _PyLong_GetOne() must always be available
// _PyLong_FromUnsignedChar must always be available
#if _PY_NSMALLPOSINTS < 257
#  error "_PY_NSMALLPOSINTS must be greater than or equal to 257"
#endif

// Return a borrowed reference to the zero singleton.
// The function cannot return NULL.
static inline PyObject* _PyLong_GetZero(void)
{ return (PyObject *)&_PyLong_SMALL_INTS[_PY_NSMALLNEGINTS]; }

// Return a borrowed reference to the one singleton.
// The function cannot return NULL.
static inline PyObject* _PyLong_GetOne(void)
{ return (PyObject *)&_PyLong_SMALL_INTS[_PY_NSMALLNEGINTS+1]; }

static inline PyObject* _PyLong_FromUnsignedChar(unsigned char i)
{
    return Py_NewRef((PyObject *)&_PyLong_SMALL_INTS[_PY_NSMALLNEGINTS+i]);
}

PyObject *_PyLong_Add(PyLongObject *left, PyLongObject *right);
PyObject *_PyLong_Multiply(PyLongObject *left, PyLongObject *right);
PyObject *_PyLong_Subtract(PyLongObject *left, PyLongObject *right);

/* Used by Python/mystrtoul.c, _PyBytes_FromHex(),
   _PyBytes_DecodeEscape(), etc. */
PyAPI_DATA(unsigned char) _PyLong_DigitValue[256];

/* Format the object based on the format_spec, as defined in PEP 3101
   (Advanced String Formatting). */
PyAPI_FUNC(int) _PyLong_FormatAdvancedWriter(
    _PyUnicodeWriter *writer,
    PyObject *obj,
    PyObject *format_spec,
    Py_ssize_t start,
    Py_ssize_t end);

PyAPI_FUNC(int) _PyLong_FormatWriter(
    _PyUnicodeWriter *writer,
    PyObject *obj,
    int base,
    int alternate);

PyAPI_FUNC(char*) _PyLong_FormatBytesWriter(
    _PyBytesWriter *writer,
    char *str,
    PyObject *obj,
    int base,
    int alternate);

/* Long value tag bits:
 * 0-1: Sign bits value = (1-sign), ie. negative=2, positive=0, zero=1.
 * 2: Reserved for immortality bit
 * 3+ Unsigned digit count
 */
#define SIGN_MASK 3
#define SIGN_ZERO 1
#define SIGN_NEGATIVE 2
#define NON_SIZE_BITS 3

static int
unused_bits_are_zero(const PyLongObject* op) {
    return (op->long_value.lv_tag & 4) == 0;
}

static int
inconsistent_zero(const PyLongObject* op) {
    return
    ((op->long_value.lv_tag & SIGN_MASK) == SIGN_ZERO
    && (op->long_value.lv_tag >> NON_SIZE_BITS) != 0)
    ||
    ((op->long_value.lv_tag & SIGN_MASK) != SIGN_ZERO
    && (op->long_value.lv_tag >> NON_SIZE_BITS) == 0);
}

/* Return 1 if the argument is positive single digit int */
static inline int
_PyLong_IsNonNegativeSingleDigit(const PyLongObject* op) {
    assert(PyLong_Check(op));
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    return op->long_value.lv_tag <= (1 << NON_SIZE_BITS);
}

static inline int
_PyLong_IsSingleDigit(const PyLongObject* op) {
    assert(PyLong_Check(op));
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    return op->long_value.lv_tag < (2 << NON_SIZE_BITS);
}

static inline int
_PyLong_BothAreSingleDigit(const PyLongObject* a, PyLongObject* b) {
    assert(PyLong_Check(a));
    assert(PyLong_Check(b));
    return (a->long_value.lv_tag | b->long_value.lv_tag) < (2 << NON_SIZE_BITS);
}

static inline Py_ssize_t
_PyLong_SingleDigitValue(const PyLongObject *op)
{
    assert(PyLong_Check(op));
    assert(_PyLong_IsSingleDigit(op));
    assert(unused_bits_are_zero(op));
    Py_ssize_t sign = 1 - (op->long_value.lv_tag & SIGN_MASK);
    return sign * (Py_ssize_t)op->long_value.ob_digit[0];
}

static inline bool
_PyLong_IsZero(const PyLongObject *op)
{
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    return (op->long_value.lv_tag & SIGN_MASK) == SIGN_ZERO;
}

static inline bool
_PyLong_IsNegative(const PyLongObject *op)
{
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    return (op->long_value.lv_tag & SIGN_MASK) == SIGN_NEGATIVE;
}

static inline bool
_PyLong_IsPositive(const PyLongObject *op)
{
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    return (op->long_value.lv_tag & SIGN_MASK) == 0;
}

static inline Py_ssize_t
_PyLong_DigitCount(const PyLongObject *op)
{
    assert(PyLong_Check(op));
    assert(unused_bits_are_zero(op));
    return op->long_value.lv_tag >> NON_SIZE_BITS;
}

/* Equivalent to _PyLong_DigitCount(op) * _PyLong_NonZeroSign(op) */
static inline Py_ssize_t
_PyLong_SignedDigitCount(const PyLongObject *op)
{
    assert(PyLong_Check(op));
    assert(unused_bits_are_zero(op));
    Py_ssize_t sign = 1 - (op->long_value.lv_tag & SIGN_MASK);
    assert(!inconsistent_zero(op));
    return sign * (Py_ssize_t)(op->long_value.lv_tag >> NON_SIZE_BITS);
}

/* Like _PyLong_DigitCount but asserts that op is non-negative */
static inline Py_ssize_t
_PyLong_UnsignedDigitCount(const PyLongObject *op)
{
    assert(PyLong_Check(op));
    assert(!_PyLong_IsNegative(op));
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    return op->long_value.lv_tag >> NON_SIZE_BITS;
}

static inline int
_PyLong_NonZeroSign(const PyLongObject *op)
{
    assert(PyLong_Check(op));
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    return 1 - (op->long_value.lv_tag & SIGN_MASK);
}

/* Do a and b have the same sign? */
static inline int
_PyLong_SameSign(const PyLongObject *a, const PyLongObject *b)
{
    return (a->long_value.lv_tag & SIGN_MASK) == (b->long_value.lv_tag & SIGN_MASK);
}

#define TAG_FROM_SIGN_AND_SIZE(sign, size) ((1 - (sign)) | ((size) << NON_SIZE_BITS))

static inline void
_PyLong_SetSignAndSize(PyLongObject *op, int sign, Py_ssize_t size)
{
    assert(size >= 0);
    assert(-1 <= sign && sign <= 1);
    assert(sign != 0 || size == 0);
    op->long_value.lv_tag = TAG_FROM_SIGN_AND_SIZE(sign, size);
    assert(!inconsistent_zero(op));
}

static inline void
_PyLong_SetSize(PyLongObject *op, Py_ssize_t size)
{
    assert(size >= 0);
    assert(!inconsistent_zero(op));
    op->long_value.lv_tag = (size << NON_SIZE_BITS) | (op->long_value.lv_tag & SIGN_MASK);
    assert(!inconsistent_zero(op));
}

static inline void
_PyLong_FlipSign(PyLongObject *op) {
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
    int flipped_sign = 2 - (op->long_value.lv_tag & SIGN_MASK);
    op->long_value.lv_tag &= ~7;
    op->long_value.lv_tag |= flipped_sign;
    assert(unused_bits_are_zero(op));
    assert(!inconsistent_zero(op));
}

#define _PyLong_DIGIT_INIT(val) \
    { \
        .ob_base = _PyObject_IMMORTAL_INIT(&PyLong_Type), \
        .long_value  = { \
            .lv_tag = TAG_FROM_SIGN_AND_SIZE( \
                (val) == 0 ? 0 : ((val) < 0 ? -1 : 1), \
                (val) == 0 ? 0 : 1), \
            { ((val) >= 0 ? (val) : -(val)) }, \
        } \
    }

#define _PyLong_FALSE_TAG TAG_FROM_SIGN_AND_SIZE(0, 0)
#define _PyLong_TRUE_TAG TAG_FROM_SIGN_AND_SIZE(1, 1)

static_assert(TAG_FROM_SIGN_AND_SIZE(0, 0) == 1);
static_assert(TAG_FROM_SIGN_AND_SIZE(1, 1) == 8);
static_assert(TAG_FROM_SIGN_AND_SIZE(-1, 1) == 10);

#ifdef __cplusplus
}
#endif
#endif /* !Py_INTERNAL_LONG_H */
