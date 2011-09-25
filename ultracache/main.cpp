#include "Heap.h"
#include "Hash.h"
#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <time.h>

Heap *g_heap;
Hash *g_hash;

extern void dumpMem(const char *desc, void *_ptr, size_t cbBytes);

extern size_t logh(size_t val);

char g_valueBuffer[256000] = { 'a' };
char g_keyBuffer[256000];

char *randomString (char *base, size_t length)
{
	char *ptr = base;

	size_t numBase = rand();


	memset (ptr, 'a' + rand () % 25, length);

	ptr += length;

/*
	for (size_t index = 0; index < length; index ++)
	{
		(*ptr++) = 'a' + ((numBase + index) % 25);//'a' + rand () % 25;
	}
*/

	(*ptr++) = '\0';

	return base;
}

int main (int argc, char **argv)
{
	assert (logh(1024 * 1024) <= HEAP_ALLOC_BITS);

	g_heap = new Heap(1024ULL * 1024ULL * 1024ULL * 1ULL);
	g_hash = new Hash(g_heap->getHeapSize() / 0x400ULL);

	assert (Heap::align(8, 1) == 8);
	assert (Heap::align(8, 9) == 16);
	assert (Heap::align(8, 8) == 8);

	Hash::HashItem *item;
	Hash::HashItem *oldItem;
	Hash::HashItem *newItem;
	Hash::HashItem *delItem;
	/*

	g_hash->put("key", "value", &item);
	assert(item == NULL);

	item = g_hash->get("key");

	assert (item != NULL);

	std::string str = item->getValueStr ();
		
	assert (str == "value");

	g_hash->put("key", "value2", &oldItem);
	assert (oldItem == item);

	g_hash->free(oldItem);

	newItem = g_hash->get("key");
	assert (newItem->getValueStr() == "value2");

	delItem = g_hash->remove("key");
	assert (delItem == newItem);

	g_hash->free(delItem);
	*/

	size_t keyCount = 0;

	while (true)
	{
		char *key = g_keyBuffer;

		sprintf(key, "%08x", keyCount, keyCount, keyCount, keyCount);
		
		size_t keyLen = strlen(key);
		size_t alignLen = Heap::align(8, keyLen);
		
		for (size_t offset = keyLen; offset < alignLen; offset ++)
		{
			key[offset] = 'X';
		}

		size_t valueLen = 32;//rand () % 65536;

		keyLen = Heap::align(8, keyLen);
		valueLen = Heap::align(8, valueLen);

		char *value = g_valueBuffer;
		
		if (!g_hash->put(key, keyLen, value, valueLen, &item))
		{
			break;
		}
		
		assert (item == NULL);

		keyCount ++;

		if (keyCount % 1000000 == 0)
		{
			fprintf (stderr, ".");
		}
	}

	fprintf (stderr, "%s: Managed to store %u keys\n", __FUNCTION__, keyCount);

	for (size_t keyIndex = 0; keyIndex < keyCount; keyIndex ++)
	{
		char *key = g_keyBuffer;
		sprintf(key, "%08x", keyIndex, keyIndex, keyIndex, keyIndex);
		size_t keyLen = strlen(key);
		size_t alignLen = Heap::align(8, keyLen);

		for (size_t offset = keyLen; offset < alignLen; offset ++)
		{
			key[offset] = 'X';
		}


		keyLen = Heap::align(8, keyLen);

		item = g_hash->remove(key, keyLen);
		g_hash->free(item);

	}



	//keyIndex *= 2;

	time_t tsStart = time (0);

	size_t keysPerSec = 0;

	for (int index = 0; index < 100000000; index ++)
	{
		time_t tsNow = time(0);

		if (tsNow != tsStart)
		{
			fprintf (stderr, "%u writes/sec\n", keysPerSec);
			keysPerSec = 0;
			tsStart = tsNow;
		}


		size_t iKey = rand () % keyCount;

		char *key = g_keyBuffer;
		sprintf(key, "%08x", iKey, iKey, iKey, iKey);
		size_t keyLen = strlen(key);
		size_t valueLen = 32;//rand () % 65536;
		char *value = g_valueBuffer;

		keyLen = Heap::align(8, keyLen);
		valueLen = Heap::align(8, valueLen);

		keysPerSec ++;

		if (!g_hash->put(key, keyLen, value, valueLen, &item))
		{
			item = g_hash->remove(key, keyLen);

			if (item)
			{
				g_hash->free(item);
			}
			continue;
		}

		if (item != NULL)
		{
			g_hash->free(item);
		}
	}
	
	fprintf (stderr, "%s: Time elapsed %d\n", __FUNCTION__, time(0) - tsStart);

	tsStart = time (0);

	keysPerSec = 0;

	for (int index = 0; index < 100000000; index ++)
	{
		time_t tsNow = time(0);

		if (tsNow != tsStart)
		{
			fprintf (stderr, "%u reads/sec\n", keysPerSec);
			keysPerSec = 0;
			tsStart = tsNow;
		}


		size_t iKey = rand () % keyCount;

		char *key = g_keyBuffer;
		sprintf(key, "%08x", iKey, iKey, iKey, iKey);
		size_t keyLen = strlen(key);
		size_t valueLen = 32;//rand () % 65536;
		char *value = g_valueBuffer;

		keyLen = Heap::align(8, keyLen);
		valueLen = Heap::align(8, valueLen);

		keysPerSec ++;

		item = g_hash->get(key, keyLen);
	}
	
	fprintf (stderr, "%s: Time elapsed %d\n", __FUNCTION__, time(0) - tsStart);


	delete g_hash;
	delete g_heap;

	getchar();

	return 0;
}