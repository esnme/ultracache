#pragma once

#include "Hash.h"
#include "Heap.h"
#include <time.h>

class Cache
{

public:
	Cache(size_t mbytes);
	~Cache(void);

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
	bool stats(const char *arg, size_t cbArg);
	
private:

	static UINT64 *alignKey(const char *key, size_t cbKey, char *buffer, size_t &cbKeyAligned);
	Hash::HashItem *alloc(size_t cbKey, size_t cbValue, size_t &cbOutSize);
private:
	Hash *m_hash;
	Heap *m_heap;
		

};