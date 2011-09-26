#pragma once

#ifdef _WIN32

typedef unsigned __int64 UINT64;
typedef unsigned __int32 UINT32;
typedef unsigned __int8 UINT8;

#else

#include <sys/types.h>
typedef u_int64_t UINT64;
typedef u_int32_t UINT32;
typedef u_int8_t UINT8;

#endif
