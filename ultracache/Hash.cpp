#include "Hash.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Heap.h"
#include <string>

#define MEMCPY(_dst, _src, _size) \
	memcpy64(_dst, _src, _size)

#define MEMCMP(_dst, _src, _size) \
	memcmp64(_dst, _src, _size)

#ifdef _WIN64
#define X64_BUILD
#endif


extern Heap *g_heap;


static void memcpy64(UINT64 *dst, UINT64 *src, size_t count)
{
	UINT64 *end = src + (count >> 3);

	while (src != end)
	{
		(*dst++) = (*src++);
	}
}

static int memcmp64(UINT64 *dst, UINT64 *src, size_t count)
{
	UINT64 *end = src + (count >> 3);

	while (src != end)
	{
		if (*dst != *src)
		{
			return -1;
		}

		dst ++;
		src ++;
	}

	return 0;
}
size_t Hash::compressPtr(HashItem *ptr)
{
	return (((size_t)(ptr)) >> 3);
}

Hash::HashItem *Hash::decompressPtr(size_t value)
{
	return ((Hash::HashItem*)(((size_t)(value)) << 3));
}


void *Hash::HashItem::getKeyPtr()
{
	return (this+1);
}
void *Hash::HashItem::getValuePtr()
{
	return ((UINT8 *)(this+1)) + this->keyLength;
}

size_t Hash::HashItem::getKeyLen()
{
	return this->keyLength;
}

size_t Hash::HashItem::getValueLen()
{
	return this->valueLength;
}

std::string Hash::HashItem::getValueStr()
{
	std::string ret((char *)getValuePtr(), getValueLen());
	return ret;
}

std::string Hash::HashItem::getKeyStr()
{
	std::string ret((char *)getKeyPtr(), getKeyLen());
	return ret;
}

void dumpMem(const char *desc, void *_ptr, size_t cbBytes)
{
	size_t index = 0;
	UINT8 *ptr = (UINT8 *) _ptr;

	fprintf (stderr, "- %s (%p) ------------------------------\n", desc, ptr);

	while (index < cbBytes)
	{
		for (size_t row = 0; row < 16; row ++)
		{
			fprintf (stderr, "%02X ", ptr[index + row]);

			if (index + row >= cbBytes)
			{
				break;
			}
		}

		fprintf (stderr, "| ");

		for (size_t row = 0; row < 16; row ++)
		{
			int chr = (unsigned char) ptr[index + row];
			fprintf (stderr, "%c", (isprint(chr) > 0) ? chr : '.');

			if (index + row >= cbBytes)
			{
				break;
			}
		}

		fprintf (stderr, "\n");

		index += 16;
	}
}


void Hash::HashItem::setup(size_t cbSize, void *key, size_t cbKey, void *value, size_t cbValue, HASHCODE _hash)
{
	this->next = compressPtr(NULL);
	this->keyLength = cbKey;
	this->valueLength = cbValue;

	UINT8 *ptr = (UINT8 *) (this+1);

	assert (sizeof (HashItem) + cbKey + cbValue <= cbSize);

	MEMCPY( (UINT64 *)ptr,  (UINT64 *)key, cbKey);
	ptr += cbKey;

	MEMCPY( (UINT64 *)ptr,  (UINT64 *)value, cbValue);
	ptr += cbValue;

}

bool Hash::HashItem::compareKey(void *key, size_t cbKey, Hash::HASHCODE hash)
{
	assert (Heap::align(8, cbKey) == cbKey);

	if (cbKey != this->keyLength)
	{
		return false;
	}
	
	if (MEMCMP ( (UINT64 *)key, (UINT64 *)this->getKeyPtr(), cbKey))
	{
		return false;
	}

	return true;
}


static Hash::HASHCODE calcHash(void *data, size_t cbKey)
{
	Hash::HASHCODE hash = 0ULL;
	Hash::HASHCODE *offset = (Hash::HASHCODE *) data;
	Hash::HASHCODE *end = (Hash::HASHCODE*)(((UINT8 *)data)+cbKey);
	
	while (offset != end)
	{
		hash = 0x1f * hash + (*offset++);
	}


	return hash;

	/*
	unsigned long hash = 0;
	UINT8 *offset = (UINT8 *) data;
	UINT8 *end = offset + cbKey;
	
	while (offset != end)
	{
		hash = 31 * hash + (*offset++);
	}

	return hash;
	*/
}

Hash::Hash (size_t binSize)
{
	m_binSize = binSize;
	m_bin = (HashItem **) g_heap->alloc(binSize * sizeof(HashItem *));
	memset (m_bin, 0, sizeof (HashItem *) * binSize);
}

Hash::~Hash (void)
{
}

Hash::HashItem *Hash::get(void *key, size_t cbKey)
{
	HASHCODE hash = calcHash(key, cbKey);

	for (HashItem *item = m_bin[hash % m_binSize]; item != NULL; item = decompressPtr(item->next))
	{
		if (item->compareKey(key, cbKey, hash))
		{
			return item;
		}
	}

	return NULL;
}

bool Hash::put(void *key, size_t cbKey, void *value, size_t cbValue, HashItem **previous)
{
	(*previous) = NULL;

	size_t itemSize = cbKey + cbValue + sizeof(HashItem);
	HashItem *newItem = (HashItem*) g_heap->alloc(itemSize);
	
	if (newItem == NULL)
	{
		//FIXME: Evict or what the hell do we do?
		return false;
	}
	
	HASHCODE hash = calcHash(key, cbKey);
	size_t index = hash % m_binSize;

	HashItem *item = m_bin[index];

	HashItem *prevItem = NULL;
	newItem->setup(itemSize, key, cbKey, value, cbValue, hash);
	
	while (item)
	{
		if (item->compareKey(key, cbKey, hash))
		{
			if (prevItem)
			{
				prevItem->next = compressPtr(newItem);
				newItem->next = item->next;
			}
			else
			{
				m_bin[index] = newItem;
				newItem->next = item->next;
			}

			(*previous) = item;

			return true;
		}

		prevItem = item;
		item = decompressPtr(item->next);
	}

	newItem->next = compressPtr(m_bin[index]);
	m_bin[index] = newItem;
	
	return true;
}

Hash::HashItem *Hash::remove(void *key, size_t cbKey)
{
	HASHCODE hash = calcHash(key, cbKey);

	size_t index = hash % m_binSize;

	HashItem *item = m_bin[index]; 
	HashItem *prevItem = NULL;
	
	while (item)
	{
		if (item->compareKey(key, cbKey, hash))
		{
			if (prevItem)
			{
				prevItem->next = item->next;
			}
			else
			{
				m_bin[index] = decompressPtr(item->next);
			}

			return item;
		}

		prevItem = item;
		item = decompressPtr(item->next);
	}

	return NULL;
}

void Hash::free(Hash::HashItem *item)
{
	g_heap->free(item);
}


Hash::HashItem *Hash::remove(const std::string &key)
{
	return remove ( (void *) key.c_str(), key.length());
}


Hash::HashItem *Hash::get(const std::string &key)
{
	return get( (void *) key.c_str(), key.length());
}

bool Hash::put(const std::string &key, const std::string &value, HashItem **previous)
{
	return put( (void *) key.c_str(), key.length(), (void *) value.c_str(), value.size(), previous);
}
