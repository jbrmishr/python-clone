// This is the implementation of Python atomic operations for MSVC if the
// compiler does not support C11 or C++11 atomics.
//
// MSVC intrinsics are defined on char, short, long, __int64, and pointer
// types. Note that long and int are both 32-bits even on 64-bit Windows,
// so operations on int are cast to long.
//
// The volatile keyword has additional memory ordering semantics on MSVC. On
// x86 and x86-64, volatile accesses have acquire-release semantics. On ARM64,
// volatile accesses behave like C11's memory_order_relaxed.

#ifndef Py_ATOMIC_MSC_H
#  error "this header file must not be included directly"
#endif

#include <intrin.h>

static inline int
_Py_atomic_add_int(int *ptr, int value)
{
    Py_BUILD_ASSERT(sizeof(int) == sizeof(long));
    return (int)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
}

static inline int8_t
_Py_atomic_add_int8(int8_t *ptr, int8_t value)
{
    Py_BUILD_ASSERT(sizeof(int8_t) == sizeof(char));
    return (int8_t)_InterlockedExchangeAdd8((volatile char*)ptr, (char)value);
}

static inline int16_t
_Py_atomic_add_int16(int16_t *ptr, int16_t value)
{
    Py_BUILD_ASSERT(sizeof(int16_t) == sizeof(short));
    return (int16_t)_InterlockedExchangeAdd16((volatile short*)ptr, (short)value);
}

static inline int32_t
_Py_atomic_add_int32(int32_t *ptr, int32_t value)
{
    Py_BUILD_ASSERT(sizeof(int32_t) == sizeof(long));
    return (int32_t)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
}

static inline int64_t
_Py_atomic_add_int64(int64_t *ptr, int64_t value)
{
#if defined(_M_X64) || defined(_M_ARM64)
    return (int64_t)_InterlockedExchangeAdd64((volatile __int64*)ptr, (__int64)value);
#else
    int64_t old_value = *(volatile int64_t*)ptr;
    for (;;) {
        int64_t new_value = old_value + value;
        if (_Py_atomic_compare_exchange_int64(ptr, &old_value, new_value)) {
            return old_value;
        }
    }
#endif
}

static inline intptr_t
_Py_atomic_add_intptr(intptr_t *ptr, intptr_t value)
{
#if SIZEOF_VOID_P == 8
    return (intptr_t)_InterlockedExchangeAdd64((volatile __int64*)ptr, (__int64)value);
#else
    return (intptr_t)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
#endif
}

static inline unsigned int
_Py_atomic_add_uint(unsigned int *ptr, unsigned int value)
{
    return (unsigned int)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
}

static inline uint8_t
_Py_atomic_add_uint8(uint8_t *ptr, uint8_t value)
{
    return (uint8_t)_InterlockedExchangeAdd8((volatile char*)ptr, (char)value);
}

static inline uint16_t
_Py_atomic_add_uint16(uint16_t *ptr, uint16_t value)
{
    return (uint16_t)_InterlockedExchangeAdd16((volatile short*)ptr, (short)value);
}

static inline uint32_t
_Py_atomic_add_uint32(uint32_t *ptr, uint32_t value)
{
    return (uint32_t)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
}

static inline uint64_t
_Py_atomic_add_uint64(uint64_t *ptr, uint64_t value)
{
    return (uint64_t)_Py_atomic_add_int64((int64_t*)ptr, (int64_t)value);
}

static inline uintptr_t
_Py_atomic_add_uintptr(uintptr_t *ptr, uintptr_t value)
{
#if SIZEOF_VOID_P == 8
    return (uintptr_t)_InterlockedExchangeAdd64((volatile __int64*)ptr, (__int64)value);
#else
    return (uintptr_t)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
#endif
}

static inline Py_ssize_t
_Py_atomic_add_ssize(Py_ssize_t *ptr, Py_ssize_t value)
{
#if SIZEOF_SIZE_T == 8
    return (Py_ssize_t)_InterlockedExchangeAdd64((volatile __int64*)ptr, (__int64)value);
#else
    return (Py_ssize_t)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
#endif
}


static inline int
_Py_atomic_compare_exchange_int(int *ptr, int *expected, int value)
{
    int initial = (int)_InterlockedCompareExchange((volatile long*)ptr, (long)value, (long)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_int8(int8_t *ptr, int8_t *expected, int8_t value)
{
    int8_t initial = (int8_t)_InterlockedCompareExchange8((volatile char*)ptr, (char)value, (char)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_int16(int16_t *ptr, int16_t *expected, int16_t value)
{
    int16_t initial = (int16_t)_InterlockedCompareExchange16((volatile short*)ptr, (short)value, (short)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_int32(int32_t *ptr, int32_t *expected, int32_t value)
{
    int32_t initial = (int32_t)_InterlockedCompareExchange((volatile long*)ptr, (long)value, (long)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_int64(int64_t *ptr, int64_t *expected, int64_t value)
{
    int64_t initial = (int64_t)_InterlockedCompareExchange64((volatile __int64*)ptr, (__int64)value, (__int64)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_intptr(intptr_t *ptr, intptr_t *expected, intptr_t value)
{
    intptr_t initial = (intptr_t)_InterlockedCompareExchangePointer((void * volatile *)ptr, (void *)value, (void *)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_uint(unsigned int *ptr, unsigned int *expected, unsigned int value)
{
    unsigned int initial = (unsigned int)_InterlockedCompareExchange((volatile long*)ptr, (long)value, (long)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_uint8(uint8_t *ptr, uint8_t *expected, uint8_t value)
{
    uint8_t initial = (uint8_t)_InterlockedCompareExchange8((volatile char*)ptr, (char)value, (char)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_uint16(uint16_t *ptr, uint16_t *expected, uint16_t value)
{
    uint16_t initial = (uint16_t)_InterlockedCompareExchange16((volatile short*)ptr, (short)value, (short)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_uint32(uint32_t *ptr, uint32_t *expected, uint32_t value)
{
    uint32_t initial = (uint32_t)_InterlockedCompareExchange((volatile long*)ptr, (long)value, (long)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_uint64(uint64_t *ptr, uint64_t *expected, uint64_t value)
{
    uint64_t initial = (uint64_t)_InterlockedCompareExchange64((volatile __int64*)ptr, (__int64)value, (__int64)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_uintptr(uintptr_t *ptr, uintptr_t *expected, uintptr_t value)
{
    uintptr_t initial = (uintptr_t)_InterlockedCompareExchangePointer((void * volatile *)ptr, (void *)value, (void *)*expected);
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_ssize(Py_ssize_t *ptr, Py_ssize_t *expected, Py_ssize_t value)
{
    Py_ssize_t initial =
#if SIZEOF_SIZE_T == 8
        _InterlockedCompareExchange64((volatile __int64*)ptr, (__int64)value, (__int64)*expected);
#else
        _InterlockedCompareExchange((volatile long*)ptr, (long)value, (long)*expected);
#endif
    if (initial == *expected) {
        return 1;
    }
    *expected = initial;
    return 0;
}

static inline int
_Py_atomic_compare_exchange_ptr(void *ptr, void *expected, void *value)
{
    void *initial = _InterlockedCompareExchangePointer((void **)ptr, value, *(void **)expected);
    if (initial == *(void **)expected) {
        return 1;
    }
    *(void **)expected = initial;
    return 0;
}

static inline int
_Py_atomic_exchange_int(int *ptr, int value)
{
    return (int)_InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline int8_t
_Py_atomic_exchange_int8(int8_t *ptr, int8_t value)
{
    return (int8_t)_InterlockedExchange8((volatile char*)ptr, (char)value);
}

static inline int16_t
_Py_atomic_exchange_int16(int16_t *ptr, int16_t value)
{
    return (int16_t)_InterlockedExchange16((volatile short*)ptr, (short)value);
}

static inline int32_t
_Py_atomic_exchange_int32(int32_t *ptr, int32_t value)
{
    return (int32_t)_InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline int64_t
_Py_atomic_exchange_int64(int64_t *ptr, int64_t value)
{
#if defined(_M_X64) || defined(_M_ARM64)
    return (int64_t)_InterlockedExchange64((volatile __int64*)ptr, (__int64)value);
#else
    int64_t old_value = *(volatile int64_t*)ptr;
    for (;;) {
        if (_Py_atomic_compare_exchange_int64(ptr, &old_value, value)) {
            return old_value;
        }
    }
#endif
}

static inline intptr_t
_Py_atomic_exchange_intptr(intptr_t *ptr, intptr_t value)
{
    return (intptr_t)_InterlockedExchangePointer((void * volatile *)ptr, (void *)value);
}

static inline unsigned int
_Py_atomic_exchange_uint(unsigned int *ptr, unsigned int value)
{
    return (unsigned int)_InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline uint8_t
_Py_atomic_exchange_uint8(uint8_t *ptr, uint8_t value)
{
    return (uint8_t)_InterlockedExchange8((volatile char*)ptr, (char)value);
}

static inline uint16_t
_Py_atomic_exchange_uint16(uint16_t *ptr, uint16_t value)
{
    return (uint16_t)_InterlockedExchange16((volatile short*)ptr, (short)value);
}

static inline uint32_t
_Py_atomic_exchange_uint32(uint32_t *ptr, uint32_t value)
{
    return (uint32_t)_InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline uint64_t
_Py_atomic_exchange_uint64(uint64_t *ptr, uint64_t value)
{
    return (uint64_t)_Py_atomic_exchange_int64((int64_t *)ptr, (int64_t)value);
}

static inline uintptr_t
_Py_atomic_exchange_uintptr(uintptr_t *ptr, uintptr_t value)
{
    return (uintptr_t)_InterlockedExchangePointer((void * volatile *)ptr, (void *)value);
}

static inline Py_ssize_t
_Py_atomic_exchange_ssize(Py_ssize_t *ptr, Py_ssize_t value)
{
#if SIZEOF_SIZE_T == 8
    return (Py_ssize_t)_InterlockedExchange64((volatile __int64*)ptr, (__int64)value);
#else
    return (Py_ssize_t)_InterlockedExchange((volatile long*)ptr, (long)value);
#endif
}

static inline void *
_Py_atomic_exchange_ptr(void *ptr, void *value)
{
    return (void *)_InterlockedExchangePointer((void * volatile *)ptr, (void *)value);
}

static inline uint8_t
_Py_atomic_and_uint8(uint8_t *ptr, uint8_t value)
{
    return (uint8_t)_InterlockedAnd8((volatile char*)ptr, (char)value);
}

static inline uint16_t
_Py_atomic_and_uint16(uint16_t *ptr, uint16_t value)
{
    return (uint16_t)_InterlockedAnd16((volatile short*)ptr, (short)value);
}

static inline uint32_t
_Py_atomic_and_uint32(uint32_t *ptr, uint32_t value)
{
    return (uint32_t)_InterlockedAnd((volatile long*)ptr, (long)value);
}

static inline uint64_t
_Py_atomic_and_uint64(uint64_t *ptr, uint64_t value)
{
#if defined(_M_X64) || defined(_M_ARM64)
    return (uint64_t)_InterlockedAnd64((volatile __int64*)ptr, (__int64)value);
#else
    uint64_t old_value = *(volatile uint64_t*)ptr;
    for (;;) {
        uint64_t new_value = old_value & value;
        if (_Py_atomic_compare_exchange_uint64(ptr, &old_value, new_value)) {
            return old_value;
        }
    }
#endif
}

static inline uintptr_t
_Py_atomic_and_uintptr(uintptr_t *ptr, uintptr_t value)
{
#if SIZEOF_VOID_P == 8
    return (uintptr_t)_InterlockedAnd64((volatile __int64*)ptr, (__int64)value);
#else
    return (uintptr_t)_InterlockedAnd((volatile long*)ptr, (long)value);
#endif
}

static inline uint8_t
_Py_atomic_or_uint8(uint8_t *ptr, uint8_t value)
{
    return (uint8_t)_InterlockedOr8((volatile char*)ptr, (char)value);
}

static inline uint16_t
_Py_atomic_or_uint16(uint16_t *ptr, uint16_t value)
{
    return (uint16_t)_InterlockedOr16((volatile short*)ptr, (short)value);
}

static inline uint32_t
_Py_atomic_or_uint32(uint32_t *ptr, uint32_t value)
{
    return (uint32_t)_InterlockedOr((volatile long*)ptr, (long)value);
}

static inline uint64_t
_Py_atomic_or_uint64(uint64_t *ptr, uint64_t value)
{
#if defined(_M_X64) || defined(_M_ARM64)
    return (uint64_t)_InterlockedOr64((volatile __int64*)ptr, (__int64)value);
#else
    uint64_t old_value = *(volatile uint64_t *)ptr;
    for (;;) {
        uint64_t new_value = old_value | value;
        if (_Py_atomic_compare_exchange_uint64(ptr, &old_value, new_value)) {
            return old_value;
        }
    }
#endif
}

static inline uintptr_t
_Py_atomic_or_uintptr(uintptr_t *ptr, uintptr_t value)
{
#if SIZEOF_VOID_P == 8
    return (uintptr_t)_InterlockedOr64((volatile __int64*)ptr, (__int64)value);
#else
    return (uintptr_t)_InterlockedOr((volatile long*)ptr, (long)value);
#endif
}

static inline int
_Py_atomic_load_int(const int *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile int *)ptr;
#elif defined(_M_ARM64)
    return (int)__ldar32((unsigned __int32 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_int
#endif
}

static inline int8_t
_Py_atomic_load_int8(const int8_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile int8_t *)ptr;
#elif defined(_M_ARM64)
    return (int8_t)__ldar8((unsigned __int8 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_int8
#endif
}

static inline int16_t
_Py_atomic_load_int16(const int16_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile int16_t *)ptr;
#elif defined(_M_ARM64)
    return (int16_t)__ldar16((unsigned __int16 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_int16
#endif
}

static inline int32_t
_Py_atomic_load_int32(const int32_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile int32_t *)ptr;
#elif defined(_M_ARM64)
    return (int32_t)__ldar32((unsigned __int32 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_int32
#endif
}

static inline int64_t
_Py_atomic_load_int64(const int64_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile int64_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar64((unsigned __int64 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_int64
#endif
}

static inline intptr_t
_Py_atomic_load_intptr(const intptr_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile intptr_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar64((unsigned __int64 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_intptr
#endif
}

static inline uint8_t
_Py_atomic_load_uint8(const uint8_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile uint8_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar8((unsigned __int8 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_uint8
#endif
}

static inline uint16_t
_Py_atomic_load_uint16(const uint16_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile uint16_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar16((unsigned __int16 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_uint16
#endif
}

static inline uint32_t
_Py_atomic_load_uint32(const uint32_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile uint32_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar32((unsigned __int32 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_uint32
#endif
}

static inline uint64_t
_Py_atomic_load_uint64(const uint64_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile uint64_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar64((unsigned __int64 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_uint64
#endif
}

static inline uintptr_t
_Py_atomic_load_uintptr(const uintptr_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile uintptr_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar64((unsigned __int64 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_uintptr
#endif
}

static inline unsigned int
_Py_atomic_load_uint(const unsigned int *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile unsigned int *)ptr;
#elif defined(_M_ARM64)
    return __ldar32((unsigned __int32 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_uint
#endif
}

static inline Py_ssize_t
_Py_atomic_load_ssize(const Py_ssize_t *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(volatile Py_ssize_t *)ptr;
#elif defined(_M_ARM64)
    return __ldar64((unsigned __int64 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_ssize
#endif
}

static inline void *
_Py_atomic_load_ptr(const void *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(void * volatile *)ptr;
#elif defined(_M_ARM64)
    return (void *)__ldar64((unsigned __int64 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_ptr
#endif
}

static inline int
_Py_atomic_load_int_relaxed(const int *ptr)
{
    return *(volatile int *)ptr;
}

static inline int8_t
_Py_atomic_load_int8_relaxed(const int8_t *ptr)
{
    return *(volatile int8_t *)ptr;
}

static inline int16_t
_Py_atomic_load_int16_relaxed(const int16_t *ptr)
{
    return *(volatile int16_t *)ptr;
}

static inline int32_t
_Py_atomic_load_int32_relaxed(const int32_t *ptr)
{
    return *(volatile int32_t *)ptr;
}

static inline int64_t
_Py_atomic_load_int64_relaxed(const int64_t *ptr)
{
    return *(volatile int64_t *)ptr;
}

static inline intptr_t
_Py_atomic_load_intptr_relaxed(const intptr_t *ptr)
{
    return *(volatile intptr_t *)ptr;
}

static inline uint8_t
_Py_atomic_load_uint8_relaxed(const uint8_t *ptr)
{
    return *(volatile uint8_t *)ptr;
}

static inline uint16_t
_Py_atomic_load_uint16_relaxed(const uint16_t *ptr)
{
    return *(volatile uint16_t *)ptr;
}

static inline uint32_t
_Py_atomic_load_uint32_relaxed(const uint32_t *ptr)
{
    return *(volatile uint32_t *)ptr;
}

static inline uint64_t
_Py_atomic_load_uint64_relaxed(const uint64_t *ptr)
{
    return *(volatile uint64_t *)ptr;
}

static inline uintptr_t
_Py_atomic_load_uintptr_relaxed(const uintptr_t *ptr)
{
    return *(volatile uintptr_t *)ptr;
}

static inline unsigned int
_Py_atomic_load_uint_relaxed(const unsigned int *ptr)
{
    return *(volatile unsigned int *)ptr;
}

static inline Py_ssize_t
_Py_atomic_load_ssize_relaxed(const Py_ssize_t *ptr)
{
    return *(volatile Py_ssize_t *)ptr;
}

static inline void*
_Py_atomic_load_ptr_relaxed(const void *ptr)
{
    return *(void * volatile *)ptr;
}


static inline void
_Py_atomic_store_int(int *ptr, int value)
{
    _InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline void
_Py_atomic_store_int8(int8_t *ptr, int8_t value)
{
    _InterlockedExchange8((volatile char*)ptr, (char)value);
}

static inline void
_Py_atomic_store_int16(int16_t *ptr, int16_t value)
{
    _InterlockedExchange16((volatile short*)ptr, (short)value);
}

static inline void
_Py_atomic_store_int32(int32_t *ptr, int32_t value)
{
    _InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline void
_Py_atomic_store_int64(int64_t *ptr, int64_t value)
{
    _Py_atomic_exchange_int64(ptr, value);
}

static inline void
_Py_atomic_store_intptr(intptr_t *ptr, intptr_t value)
{
    _InterlockedExchangePointer((void * volatile *)ptr, (void *)value);
}

static inline void
_Py_atomic_store_uint8(uint8_t *ptr, uint8_t value)
{
    _InterlockedExchange8((volatile char*)ptr, (char)value);
}

static inline void
_Py_atomic_store_uint16(uint16_t *ptr, uint16_t value)
{
    _InterlockedExchange16((volatile short*)ptr, (short)value);
}

static inline void
_Py_atomic_store_uint32(uint32_t *ptr, uint32_t value)
{
    _InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline void
_Py_atomic_store_uint64(uint64_t *ptr, uint64_t value)
{
    _Py_atomic_exchange_int64((int64_t *)ptr, (int64_t)value);
}

static inline void
_Py_atomic_store_uintptr(uintptr_t *ptr, uintptr_t value)
{
    _InterlockedExchangePointer((void * volatile *)ptr, (void *)value);
}

static inline void
_Py_atomic_store_uint(unsigned int *ptr, unsigned int value)
{
    _InterlockedExchange((volatile long*)ptr, (long)value);
}

static inline void
_Py_atomic_store_ptr(void *ptr, void *value)
{
    _InterlockedExchangePointer((void * volatile *)ptr, (void *)value);
}

static inline void
_Py_atomic_store_ssize(Py_ssize_t *ptr, Py_ssize_t value)
{
#if SIZEOF_SIZE_T == 8
    _InterlockedExchange64((volatile __int64 *)ptr, (__int64)value);
#else
    _InterlockedExchange((volatile long*)ptr, (long)value);
#endif
}


static inline void
_Py_atomic_store_int_relaxed(int *ptr, int value)
{
    *(volatile int *)ptr = value;
}

static inline void
_Py_atomic_store_int8_relaxed(int8_t *ptr, int8_t value)
{
    *(volatile int8_t *)ptr = value;
}

static inline void
_Py_atomic_store_int16_relaxed(int16_t *ptr, int16_t value)
{
    *(volatile int16_t *)ptr = value;
}

static inline void
_Py_atomic_store_int32_relaxed(int32_t *ptr, int32_t value)
{
    *(volatile int32_t *)ptr = value;
}

static inline void
_Py_atomic_store_int64_relaxed(int64_t *ptr, int64_t value)
{
    *(volatile int64_t *)ptr = value;
}

static inline void
_Py_atomic_store_intptr_relaxed(intptr_t *ptr, intptr_t value)
{
    *(volatile intptr_t *)ptr = value;
}

static inline void
_Py_atomic_store_uint8_relaxed(uint8_t *ptr, uint8_t value)
{
    *(volatile uint8_t *)ptr = value;
}

static inline void
_Py_atomic_store_uint16_relaxed(uint16_t *ptr, uint16_t value)
{
    *(volatile uint16_t *)ptr = value;
}

static inline void
_Py_atomic_store_uint32_relaxed(uint32_t *ptr, uint32_t value)
{
    *(volatile uint32_t *)ptr = value;
}

static inline void
_Py_atomic_store_uint64_relaxed(uint64_t *ptr, uint64_t value)
{
    *(volatile uint64_t *)ptr = value;
}

static inline void
_Py_atomic_store_uintptr_relaxed(uintptr_t *ptr, uintptr_t value)
{
    *(volatile uintptr_t *)ptr = value;
}

static inline void
_Py_atomic_store_uint_relaxed(unsigned int *ptr, unsigned int value)
{
    *(volatile unsigned int *)ptr = value;
}

static inline void
_Py_atomic_store_ptr_relaxed(void *ptr, void* value)
{
    *(void * volatile *)ptr = value;
}

static inline void
_Py_atomic_store_ssize_relaxed(Py_ssize_t *ptr, Py_ssize_t value)
{
    *(volatile Py_ssize_t *)ptr = value;
}

static inline void *
_Py_atomic_load_ptr_acquire(const void *ptr)
{
#if defined(_M_X64) || defined(_M_IX86)
    return *(void * volatile *)ptr;
#elif defined(_M_ARM64)
    return (void *)__ldar64((unsigned __int64 volatile*)ptr);
#else
#error no implementation of _Py_atomic_load_ptr_acquire
#endif
}

static inline void
_Py_atomic_store_ptr_release(void *ptr, void *value)
{
#if defined(_M_X64) || defined(_M_IX86)
    *(void * volatile *)ptr = value;
#elif defined(_M_ARM64)
    __stlr64(ptr, (uintptr_t)value);
#else
#error no implementation of _Py_atomic_store_ptr_release
#endif
}

 static inline void
_Py_atomic_fence_seq_cst(void)
{
#if defined(_M_ARM64)
    __dmb(_ARM64_BARRIER_ISH);
#elif defined(_M_X64)
    __faststorefence();
#elif defined(_M_IX86)
    _mm_mfence();
#else
#error no implementation of _Py_atomic_fence_seq_cst
#endif
}

 static inline void
_Py_atomic_fence_release(void)
{
#if defined(_M_ARM64)
    __dmb(_ARM64_BARRIER_ISH);
#elif defined(_M_X64) || defined(_M_IX86)
    _ReadWriteBarrier();
#else
#error no implementation of _Py_atomic_fence_release
#endif
}
