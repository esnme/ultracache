#include "Cache.h"
#include "config.h"
#include <assert.h>
#include <string.h>


//FIXME: Handle when growAndReplace returns NULL
//FIXME: Items prone to appending might be advisable to increase in big chunks

Cache::Cache(size_t mbytes)
{
	size_t heapSize = mbytes * (1048576ULL);

	m_heap = new Heap(heapSize);
	m_hash = new Hash(heapSize / 0x400ULL);
	m_cas = 0x1;
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

Cache::HANDLE Cache::get(const char *key, size_t cbKey, void **_outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *previous;
	Hash::HASHCODE hash;
	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);

	if (item == NULL)
	{
		return NULL;
	}

	*_outValue = item->getValuePtr();
	*_cbOutValue = item->getValueLen();

	*_outFlags = item->getFlags();
	*_outCas = item->getCas();
	
	return (Cache::HANDLE) item;
}

Hash::HashItem *Cache::growAndReplace(Hash::HASHCODE hash, Hash::HashItem *item, Hash::HashItem *previous, size_t cbValue)
{
	size_t cbAvail = Heap::getSizeOfAlloc(item) - (item->getSize() - item->getValueLen());
	size_t cbTotal;

	Hash::HashItem *newItem;

	if (cbValue <= cbAvail)
	{
		newItem = item;
	}
	else
	{
		newItem = alloc(item->getKeyLen(), cbValue, cbTotal);

		if (newItem == NULL)
		{
			//FIXME: What do we do here?
			return false;
		}

		memcpy (newItem, item, Heap::getSizeOfAlloc(item));
	}

	newItem->cbValueLength = cbValue;
	newItem->cas = getNextCas();

	if (newItem == item)
	{
		return item;;
	}

	m_hash->unlink(hash, item, previous);
	m_hash->link(hash, newItem, previous);

	return newItem;
}


bool Cache::set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *previous;
	Hash::HASHCODE hash;

	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);

	Hash::HashItem *newItem;
	size_t cbTotal;
	
	if (item)
	{
		newItem = growAndReplace(hash, item, previous, cbData);
		newItem->expire = (UINT32) expiration;
		newItem->flags = flags;

	}
	else
	{
		newItem = alloc(cbKey, cbData, cbTotal);

		if (newItem == NULL)
		{
			return false;
		}

		UINT64 cas = getNextCas();
		newItem->setup(cbTotal, (void *) key, cbKey, data, cbData, flags, cas, expiration);
		m_hash->link(hash, newItem, previous);
	}

	memcpy (newItem->getValuePtr(), data, cbData);

	assert (newItem->getValueLen() == cbData);

	if (item && newItem != item)
	{
		m_heap->free(item);
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

	if (expiration)
		*expiration = item->getExpire();

	m_heap->free(item);
	return true;
}

bool Cache::add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *previous;
	Hash::HASHCODE hash;

	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);
	if (item != NULL)
	{
		return false;
	}

	size_t cbTotal;
	Hash::HashItem *newItem = alloc(cbKey, cbData, cbTotal);
	
	if (newItem == NULL)
	{
		//FIXME: What do we do here?
		return false;
	}

	UINT64 cas = getNextCas();
	newItem->setup(cbTotal, (void *) key, cbKey, data, cbData, flags, cas, expiration);
	m_hash->link(hash, newItem, previous);
	
	return true;
}

bool Cache::replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *previous;
	Hash::HASHCODE hash;

	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);
	
	if (item == NULL)
	{
		return false;
	}

	Hash::HashItem *newItem = growAndReplace(hash, item, previous, cbData);
	memcpy(newItem->getValuePtr(), data, cbData);
	assert (newItem->getValueLen() == cbData);

	newItem->expire = expiration;
	newItem->flags = flags;

	if (newItem != item)
	{
		m_heap->free(item);
	}

	return true;
}

bool Cache::append(const char *key, size_t cbKey, void *data, size_t cbData)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);

	Hash::HashItem *previous;
	Hash::HASHCODE hash;

	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);

	if (item == NULL)
	{
		return false;
	}

	Hash::HashItem *newItem = growAndReplace(hash, item, previous, cbData + item->getValueLen());

	if (newItem != item)
	{
		memcpy(newItem->getValuePtr(), item->getValuePtr(), item->getValueLen());
	}

	memcpy(((UINT8 *)newItem->getValuePtr()) + item->getValueLen() - cbData, data, cbData);
	
	if (newItem != item)
	{
		m_heap->free(item);
	}


	return true;
}

bool Cache::prepend(const char *key, size_t cbKey, void *data, size_t cbData)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);

	Hash::HashItem *previous;
	Hash::HASHCODE hash;

	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);

	if (item == NULL)
	{
		return false;
	}

	Hash::HashItem *newItem = growAndReplace(hash, item, previous, cbData + item->getValueLen());

	if (newItem == item)
	{
		memmove ( ((UINT8 *)newItem->getValuePtr()) + cbData, ((UINT8 *)newItem->getValuePtr()), newItem->getValueLen() - cbData); 
	}
	else
	{
		memcpy (((UINT8 *)newItem->getValuePtr()) + cbData, item->getValuePtr(), item->getValueLen());
	}

	memcpy(newItem->getValuePtr(), data, cbData);
	
	if (newItem != item)
	{
		m_heap->free(item);
	}

	return true;
}
	
bool Cache::cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);

	Hash::HashItem *previous;
	Hash::HASHCODE hash;

	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);

	if (item && item->getCas() != casUnique)
	{
		return false;
	}

	Hash::HashItem *newItem;
	if (item)
	{
		newItem = growAndReplace(hash, item, previous, cbData);
		memcpy (newItem->getValuePtr(), data, cbData);

		newItem->expire = expiration;
		newItem->flags = flags;

		if (newItem != item)
		{
			m_heap->free(item);
		}
	}
	else
	{
		size_t cbTotal;
		newItem = alloc(cbKey, cbData, cbTotal);
		newItem->setup(cbTotal, (void *) key, cbKey, data, cbData, flags, getNextCas(), expiration);
	}

	
	return true;
}

static UINT64 StringToInteger(char *value, size_t cbValue)
{
	UINT64 ret = 0;
	char *end = value + cbValue;

	while (value < end)
	{
		UINT8 chr = (*value++);

		switch (chr)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ret = ret * 10ULL + (UINT64) (chr - 48);
			break;

		default:
			return 0ULL;
		}
	}

	return ret;
}

static void strreverse(char* begin, char* end)
{
	char aux;
	while (end > begin)
	aux = *end, *end-- = *begin, *begin++ = aux;
}


static size_t IntegerToString(UINT64 value, char *buffer)
{
	char* wstr;
	wstr = buffer;
	// Conversion. Number is reversed.
	
	do *wstr++ = (char)(48 + (value % 10ULL)); while(value /= 10ULL);

	// Reverse string
	strreverse(buffer,wstr - 1);
	return (wstr - buffer);
}

bool Cache::incrementDecrement(const char *key, size_t cbKey, UINT64 number, bool bIncr)
{
	char buffer[CONFIG_MAX_KEY_LENGTH];
	size_t cbKeyAligned;
	UINT64 *alignedKey = alignKey(key, cbKey, buffer, cbKeyAligned);
	
	Hash::HashItem *previous;
	Hash::HASHCODE hash;
	Hash::HashItem *item = m_hash->get(alignedKey, cbKey, &previous, &hash);

	if (item == NULL)
	{
		//FIXME: Is this really right?
		return false;
	}

	UINT64 value = StringToInteger( (char *) item->getValuePtr(), item->getValueLen());
	if (bIncr)
	{
		value += number;
	}
	else
	{
		value -= number;
	}

	size_t capacity = m_heap->getSizeOfAlloc(item) - item->getSize();
	
	Hash::HashItem *newItem = growAndReplace(hash, item, previous, CONFIG_UINT64_STRING_LENGTH);
	newItem->cbValueLength = IntegerToString(value, (char *) newItem->getValuePtr());
	return true;
}

bool Cache::incr(const char *key, size_t cbKey, UINT64 increment)
{
	return incrementDecrement(key, cbKey, increment, true);
}

bool Cache::decr(const char *key, size_t cbKey, UINT64 decrement)
{
	return incrementDecrement(key, cbKey, decrement, false);
}


bool Cache::version(char **version, size_t *cbVersion)
{
	*version = CONFIG_VERSION_STRING;
	*cbVersion = strlen(CONFIG_VERSION_STRING);
	return true;
}

UINT64 Cache::getNextCas()
{
	return m_cas++;
}

void Cache::release(HANDLE handle)
{
	//DUMMY
}