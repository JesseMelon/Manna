#include "containers/darray.h"
#include "core/logger.h"
#include "memory/memory.h"

//TODO: some of these can be optimized to not regenerate the pointer to darray

void* _darray_create(u64 capacity, u64 stride) {
    u64 header_size = DARRAY_FIELD_COUNT * sizeof(u64);
    u64 darray_size = capacity * stride;
    u64* new_darray = m_allocate(header_size + darray_size, MEMORY_TAG_DARRAY);
    m_set_memory(new_darray, 0, header_size + darray_size);
    new_darray[DARRAY_CAPACITY] = capacity;
    new_darray[DARRAY_LENGTH] = 0;
    new_darray[DARRAY_STRIDE] = stride;
    return (void*)(new_darray + DARRAY_FIELD_COUNT);
}

void _darray_destroy(void* darray) {
    //array meta data is stored before it in memory, therefore we subtract the pointer here
    u64* header = (u64*)darray - DARRAY_FIELD_COUNT; 
    u64 header_size = DARRAY_FIELD_COUNT * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    m_free(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_get_field(void* darray, u64 field) {
     u64* header = (u64*)darray - DARRAY_FIELD_COUNT;
    return header[field];
}

void _darray_set_field(void* darray, u64 field, u64 value) {
    u64* header = (u64*)darray - DARRAY_FIELD_COUNT;
    header[field] = value;
}

void* _darray_resize(void* darray) {
    u64 length = darray_get_length(darray);
    u64 stride = darray_get_stride(darray);
    void* temp = _darray_create(DARRAY_RESIZE_FACTOR * darray_get_capacity(darray), stride);
    m_copy_memory(temp, darray, length * stride);

    _darray_set_field(temp, DARRAY_LENGTH, length);
    _darray_destroy(darray);
    return temp;
}

void* _darray_push(void* darray, const void* value_ptr) {
    u64 length = darray_get_length(darray);
    u64 stride = darray_get_stride(darray);
    if (length >= darray_get_capacity(darray)) {
        darray = _darray_resize(darray);
    }
    u64 addr = (u64)darray;
    addr += (length * stride);
    m_copy_memory((void*)addr, value_ptr, stride);
    _darray_set_field(darray, DARRAY_LENGTH, length + 1); //increment length to reflect the addition of an element
    return darray;
}

void _darray_pop(void* darray, void* dest) {
    u64 length = darray_get_length(darray);
    u64 stride = darray_get_stride(darray);
    u64 addr = (u64)darray; //cast pointer to number to avoid remain defined behaviour and avoid any weirdness with pointer arithematic
    addr += ((length - 1) * stride);
    m_copy_memory(dest, (void*)addr, stride); //copy the last value into dest
    _darray_set_field(darray, DARRAY_LENGTH, length - 1); //decrement length to remove value
}

//FIXME: memcpy is being given overlapping memory, which can break depending on the memcpy implementation. should use memove
void* _darray_pop_at(void* darray, u64 index, void* dest) {
    u64 length = darray_get_length(darray);
    u64 stride = darray_get_stride(darray);
    if (index >= length) {
        LOG_ERROR("Index out of bounds of dynamic array. Length: %i, Index: %i", length, index);
        return darray;
    }
    u64 addr = (u64)darray;
    m_copy_memory(dest, (void*)(addr + (index * stride)), stride);
    if (index != length-1) {
        m_copy_memory((void*)(addr + (index * stride)), (void*)(addr + ((index + 1) * stride)), stride * (length - index + 1));
    }
    _darray_set_field(darray, DARRAY_LENGTH, length - 1);
    return darray;
}

//FIXME: memcpy is being given overlapping memory, which can break depending on the memcpy implementation. should use memmove
void* _darray_insert_at(void* darray, u64 index, void* value_ptr) {
    u64 length = darray_get_length(darray);
    u64 stride = darray_get_stride(darray);
    if (index >= length) {
        LOG_ERROR("Index outside of bounds of dynamic array. Length: %i, Index: %i", length, index);
        return darray;
    }
    u64 addr = (u64)darray;
    if (index != length - 1) {
        m_copy_memory((void*)(addr + ((index + 1) * stride)), (void*)(index * stride), stride * (length - index));
    }
    m_copy_memory((void*)(addr + (index * stride)), value_ptr, stride);
    _darray_set_field(darray, DARRAY_LENGTH, length + 1);
    return darray;
}
