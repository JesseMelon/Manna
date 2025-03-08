#pragma once

#include "defines.h"

/*
layout:
u64 capacity
u64 length
u64 stride
void* elements
*/

enum {
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_COUNT
};

//expose internal functions? yes. Because macros become these
MANNA_API void* _darray_create(u64 length, u64 stride);
MANNA_API void _darray_destroy(void* darray);

MANNA_API u64 _darray_get_field(void* darray, u64 field);
MANNA_API void _darray_set_field(void* darray, u64 field, u64 value);

MANNA_API void* _darray_resize(void* darray);

MANNA_API void* _darray_push(void* darray, const void* value_ptr);
MANNA_API void _darray_pop(void* darray, void* dest);

MANNA_API void* _darray_pop_at(void* darray, u64 index, void* dest);
MANNA_API void* _darray_insert_at(void* darray, u64 index, void* value_ptr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

#define darray_create(type) _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define darray_reserve(type, capacity) _darray_create(capacity, sizeof(type));

#define darray_destroy(darray) _darray_destroy(darray);

//this is doing some magic with stack allocated value types here. making a temp var of type typeof(value) allows it to be passed by reference
//note: could also use __auto_type. Both are GNU extensions
#define darray_push(darray, value)           \
{                                           \
    typeof(value) temp = value;             \
    darray = _darray_push(darray, &temp);   \
}

#define darray_pop(darray, value_ptr) _darray_pop(darray, value_ptr);

#define darray_insert_at(darray, index, value)          \
{                                                       \
    typeof(value) temp = value;                         \
    darray = _darray_insert_at(darray, index, &temp);   \
}

#define darray_pop_at(darray, index, value_ptr) _darray_pop_at(darray, index, value_ptr)

#define darray_clear(darray) _darray_set_field(darray, DARRAY_LENGTH, 0)

#define darray_get_capacity(darray) _darray_get_field(darray, DARRAY_CAPACITY)

#define darray_get_length(darray) _darray_get_field(darray, DARRAY_LENGTH)

#define darray_get_stride(darray) _darray_get_field(darray, DARRAY_STRIDE)

#define darray_set_length(darray) _darray_set_field(darray, DARRAY_LENGTH)
