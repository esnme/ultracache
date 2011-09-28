#include "Hash.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Heap.h"
#include <string>
#include <stdint.h> 

/*
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
*/

size_t Hash::compressPtr(HashItem *ptr)
{
	return (((size_t)(ptr)) >> 3ULL);
}

Hash::HashItem *Hash::decompressPtr(size_t value)
{
	return ((Hash::HashItem*)(((size_t)(value)) << 3ULL));
}


UINT64 *Hash::HashItem::getKeyPtr()
{
	return (UINT64 *) (this+1);
}
UINT64 *Hash::HashItem::getValuePtr()
{
	return (UINT64 *) (((UINT8 *)(this+1)) + Heap::align(8, this->cbKeyLength));
}

size_t Hash::HashItem::getKeyLen()
{
	return this->cbKeyLength;
}

size_t Hash::HashItem::getValueLen()
{
	return this->cbValueLength;
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

void Hash::HashItem::setup(size_t cwSize, void *key, size_t cbKey, void *value, size_t cbValue, UINT32 flags, UINT64 cas, UINT32 expire)
{
	//FIXME: alignment is calculated twice here

	size_t cbKeyAlign = Heap::align(8, cbKey);
	this->next = compressPtr(NULL);
	this->cbKeyLength = cbKey;
	this->cbValueLength = cbValue;
	this->cas = cas;
	this->flags = flags;
	this->expire = expire;

	UINT8 *ptr = (UINT8 *) (this+1);
	memcpy(ptr, key, cbKey);
	ptr += cbKey;

	for (size_t i = cbKey; i < cbKeyAlign; i ++)
	{
		(*ptr++) = 0x00;
	}
	
	memcpy(ptr,  value, cbValue);
	ptr += cbValue;
}

Hash::HashItem *Hash::HashItem::appendValue(void *value, size_t cbValue)
{
	return NULL;
}


UINT64 Hash::HashItem::getCas()
{
	return this->cas;
}

UINT32 Hash::HashItem::getFlags()
{
	return this->flags;
}

UINT32 Hash::HashItem::getExpire()
{
	return this->expire;
}

size_t Hash::HashItem::getSize()
{
	return Heap::align(8, this->cbKeyLength) + this->cbValueLength + sizeof(HashItem);
}


bool Hash::HashItem::compareKey(UINT64 *key, size_t cbKey)
{
	size_t cwKey = Heap::align(8, cbKey) >> 3;

	if (cbKey != this->cbKeyLength)
	{
		return false;
	}
	
	if (memcmp ( (UINT64 *)key, (UINT64 *)this->getKeyPtr(), cwKey << 3))
	{
		return false;
	}

	return true;
}

static Hash::HASHCODE calcHash(UINT64 *data, size_t cwKey)
{
	Hash::HASHCODE h = 0;

	UINT64 *start = (UINT64 *) data;
	UINT64 *end = start + cwKey;

	while (start != end)
	{
		UINT64 value = *start;

		h = 31ULL * h + ((value & 0xff00000000000000ULL) >> 56ULL);
		h = 31ULL * h + ((value & 0x00ff000000000000ULL) >> 48ULL);
		h = 31ULL * h + ((value & 0x0000ff0000000000ULL) >> 40ULL);
		h = 31ULL * h + ((value & 0x000000ff00000000ULL) >> 32ULL);
		h = 31ULL * h + ((value & 0x00000000ff000000ULL) >> 24ULL);
		h = 31ULL * h + ((value & 0x0000000000ff0000ULL) >> 16ULL);
		h = 31ULL * h + ((value & 0x000000000000ff00ULL) >> 8ULL);
		h = 31ULL * h + ((value & 0x00000000000000ffULL) >> 0ULL);

		start ++;
	}

	return h;


	//return hashword( (uint32_t *) data, cbKey >> 2, 0);
}


Hash::Hash (size_t binSize)
{
	m_binSize = binSize;
	m_bin = (UINT32 *) malloc(binSize * sizeof(UINT32));
	memset (m_bin, 0, sizeof (UINT32) * binSize);
}

Hash::~Hash (void)
{
	free (m_bin);
}
Hash::HashItem *Hash::get(UINT64 *key, size_t cbKey, HashItem **previous, HASHCODE *outHash)
{
	*previous = NULL;
	size_t cwKey = Heap::align(8, cbKey) >> 3;

	HASHCODE hash = calcHash(key, cwKey);
	*outHash = hash;
	size_t index = hash % m_binSize;

	HashItem *item = decompressPtr(m_bin[index]);
	HashItem *prev = NULL;

	while (item)
	{
		if (item->compareKey(key, cbKey))
		{
			*previous = prev;
			return item;
		}

		prev = item;
		item = decompressPtr(item->next);
	}

	*previous = prev;
	return NULL;
}

void Hash::link(HASHCODE hash, HashItem *item, HashItem *previous)
{
	size_t index = hash % m_binSize;

	if (previous == NULL)
	{
		m_bin[index] = compressPtr(item);
	}
	else
	{
		previous->next = compressPtr(item);
	}


}

void Hash::unlink(HASHCODE hash, HashItem *item, HashItem *previous)
{
	size_t index = hash % m_binSize;

	if (previous == NULL)
	{
		m_bin[index] = item->next;
	}
	else
	{
		previous->next = item->next;
	}

}


bool Hash::set(HashItem *newItem, HashItem **previous)
{
	(*previous) = NULL;

	size_t cwKey = Heap::align(8, newItem->getKeyLen()) >> 3;

	HASHCODE hash = calcHash(newItem->getKeyPtr(), cwKey);
	size_t index = hash % m_binSize;

	HashItem *item = decompressPtr(m_bin[index]);
	HashItem *prevItem = NULL;
	
	while (item)
	{
		if (item->compareKey(newItem->getKeyPtr(), newItem->getKeyLen()))
		{
			if (prevItem)
			{
				prevItem->next = compressPtr(newItem);
				newItem->next = item->next;
			}
			else
			{
				m_bin[index] = compressPtr(newItem);
				newItem->next = item->next;
			}

			(*previous) = item;

			return true;
		}

		prevItem = item;
		item = decompressPtr(item->next);
	}

	newItem->next = m_bin[index];
	m_bin[index] = compressPtr(newItem);
	
	return true;
}

Hash::HashItem *Hash::remove(UINT64 *key, size_t cbKey)
{
	size_t cwKey = Heap::align(8, cbKey) >> 3;

	HASHCODE hash = calcHash(key, cwKey);

	size_t index = hash % m_binSize;

	HashItem *item = decompressPtr(m_bin[index]); 
	HashItem *prevItem = NULL;
	
	while (item)
	{
		if (item->compareKey(key, cbKey))
		{
			if (prevItem)
			{
				prevItem->next = item->next;
			}
			else
			{
				m_bin[index] = item->next;
			}

			return item;
		}

		prevItem = item;
		item = decompressPtr(item->next);
	}

	return NULL;
}

