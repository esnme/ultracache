#pragma once

#include "types.h"

namespace protocol
{
/*
	bool set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
	bool del(const char *key, size_t cbKey, time_t *expiration);
	bool add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);

	bool replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
	bool append(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
	bool prepend(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
	
	bool cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags);
	bool incr(const char *key, size_t cbKey, UINT64 increment);
	bool decr(const char *key, size_t cbKey, UINT64 decrement);
	bool version(char **version, size_t *cbVersion);
	
	KwHandle get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas);
*/

enum Commands
{
	RESERVED = 0,
	SET,
	DEL,
	ADD,
	REPLACE,
	APPEND,
	PREPEND,
	CAS,
	INCR,
	DECR,
	GET,
	VERSION,

	RESULT_STORED,
	RESULT_NOT_STORED,
	RESULT_EXISTS,
	RESULT_NOT_FOUND,
	RESULT_DELETED,
	RESULT_VERSION,
	RESULT_GET,
	RESULT_ERROR_OOM,


};

#ifdef _WIN32
#pragma pack(push)
#pragma pack(1)
#define __GCC_ALIGN__
#else
#define __GCC_ALIGN__ __attribute__ ((packed))
#endif

struct Header
{
	UINT16 rid;
	UINT16 seq;

	UINT32 cmd:5;
	UINT32 packets:20;
	UINT32 async:1;
	UINT32 first:1;
	UINT32 last:1;
} __GCC_ALIGN__;


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma align 0
#endif

}