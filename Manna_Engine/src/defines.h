#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;

typedef int b32;
typedef char b8;

#if defined(__clang__) || defined(__gcc__)
	#define STATIC_ASSERT _Static_assert
#else
	#define STATIC_ASSERT static_assert
#endif

//ensure types are the correct size
STATIC_ASSERT(sizeof(u8) == 1, "u8 must be 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "u16 must be 2 bytes");
STATIC_ASSERT(sizeof(u32) == 4, "u32 must be 4 bytes");
STATIC_ASSERT(sizeof(u64) == 8, "u64 must be 8 bytes");
STATIC_ASSERT(sizeof(i8) == 1, "i8 must be 1 byte");
STATIC_ASSERT(sizeof(i16) == 2, "i16 must be 2 bytes");
STATIC_ASSERT(sizeof(i32) == 4, "i32 must be 4 bytes");
STATIC_ASSERT(sizeof(i64) == 8, "i64 must be 8 bytes");
STATIC_ASSERT(sizeof(f32) == 4, "f32 must be 4 bytes");
STATIC_ASSERT(sizeof(f64) == 8, "f64 must be 8 bytes");
