#include "Cache.h"
#include "config.h"
#include <assert.h>
#include <string.h>

Cache::Cache(size_t mbytes)
{
	size_t heapSize = mbytes * 1024ULL;

	m_heap = new Heap(heapSize);
	m_hash = new Hash(heapSize / 0x400ULL);
}

Cache::~Cache(void)
{
	delete m_hash;
	delete m_heap;
}

Hash::HashItem *Cache::alloc(size_t cbKey, size_t cbValue, size_t &cbOutSize)
{
	size_t cbAlignKey = Heap::align(8, cbKey);
	cbOutSize = cbAlignKey + cbValue + sizeof (Hash::HashItem);
	
	return  (Hash::HashItem *) m_heap->alloc(cbOutSize);
}

UINT64 *Cache::alignKey(const char *key, size_t cbKey, char *buffer, size_t &cbKeyAligned)
{
	cbKeyAligned = Heap::align(8, cbKey);
	memcpy(buffer, key, cbKey);

	for (size_t i=cbKey; i < cbKeyAligned; i ++)
	{
		buffer[i] = 0x00;
	}

	return (UINT64 *) buffer;


}

bool Cache::set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	size_t cbSize;

	Hash::HashItem *item = alloc(cbKey, cbData, cbSize);
	Hash::HashItem *previous = NULL;

	if (!item)
	{
		//FIXME: What do we do here?
		return false;
	}
	
	item->setup(cbSize, (void *) key, cbKey, data, cbData);

	if (!m_hash->set(item, &previous))
	{
		m_heap->free(item);
		return false;
	}

	if (previous)
	{
		m_heap->free(previous);
	}

	return true;
}

bool Cache::del(const char *key, size_t cbKey, time_t *expiration)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *item = m_hash->remove(alignedKey, cbKey);

	if (item == NULL)
	{
		return false;
	}

	m_heap->free(item);
	return true;
}

bool Cache::add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *item = m_hash->get(alignedKey, cbKey);
	if (item != NULL)
	{
		return false;
	}

	size_t cbTotal;
	Hash::HashItem *previous = NULL;
	Hash::HashItem *newItem = alloc(cbKey, cbData, cbTotal);
	
	newItem->setup(cbTotal, (void *) key, cbKey, data, cbData);
	m_hash->set(newItem, &previous);

	assert (previous == NULL);
	
	return true;
}

bool Cache::replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *item = m_hash->get(alignedKey, cbKey);
	
	if (item == NULL)
	{
		return false;
	}
	

	
	return true;
}

bool Cache::append(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);

	Hash::HashItem *item = m_hash->get(alignedKey, cbKeyAligned >> 3);

	if (item == NULL)
	{
		return false;
	}

	size_t cbAvailable = Heap::getSizeOfAlloc(item);
	size_t cbCurrSize = Heap::align(8, item->getKeyLen()) + sizeof(Hash::HashItem) + item->getValueLen();

	if (cbCurrSize + cbData > cbAvailable)
	{
		Hash::HashItem *oldItem = item;
		Hash::HashItem *item = (Hash::HashItem *) m_heap->alloc(cbCurrSize + cbData);
		memcpy (item, oldItem, cbCurrSize);
	}
	
	memcpy ( ((UINT8 *)item->getValuePtr()) + item->getValueLen(), data, cbData);
	item->cbValueLength += cbData;

	return true;
}

bool Cache::prepend(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	return true;
}
	
bool Cache::cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags)
{
	return true;
}

bool Cache::incr(const char *key, size_t cbKey, UINT64 increment)
{
	return true;
}

bool Cache::decr(const char *key, size_t cbKey, UINT64 decrement)
{
	return true;
}

bool Cache::version(char **version, size_t *cbVersion)
{
	return true;
}

bool Cache::stats(const char *arg, size_t cbArg)
{
	return true;
}
