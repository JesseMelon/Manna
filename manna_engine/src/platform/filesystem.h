#pragma once

#include "defines.h"

typedef struct file_handle {
    void* handle;
    b8 is_valid;
} file_handle;

typedef enum file_modes {
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
} file_modes;

MANNA_API b8 file_exists(const char* path);

MANNA_API b8 open_file(const char* path, file_modes mode, b8 binary, file_handle* out_handle);

MANNA_API void close_file(file_handle* handle);

MANNA_API b8 read_line(file_handle* handle, char** line_buf);

MANNA_API b8 write_line(file_handle* handle, const char* text);

MANNA_API b8 read_from_file(file_handle* handle, u64 data_size, void* out_data, u64* out_bytes_read);

MANNA_API b8 read_all_bytes(file_handle* handle, u8** out_bytes, u64* out_bytes_read);

MANNA_API b8 write_to_file(file_handle* handle, u64 data_size, const void* data, u64* out_bytes_written);
