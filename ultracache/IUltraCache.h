#pragma once

#include "types.h"
#include <time.h>

class IUltraCache
{
public:
	typedef void * HANDLE;

public:
	bool set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool del(const char *key, size_t cbKey, time_t *expiration, bool bAsync);
	bool add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);

	bool replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool append(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync);
	bool prepend(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync);
	
	bool cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool incr(const char *key, size_t cbKey, UINT64 increment, bool bAsync);
	bool decr(const char *key, size_t cbKey, UINT64 decrement, bool bAsync);
	bool version(char **version, size_t *cbVersion);
	
	HANDLE get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas);
	void release(HANDLE handle);

};