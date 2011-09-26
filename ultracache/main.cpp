#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include "Cache.h"

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
	Cache *cache = new Cache(1024);


	cache->set("key", 3, "value", 5, 0, 0);
	assert (!cache->add("key", 3, "value", 5, 0, 0));
	assert (cache->del("key", 3, NULL));
	assert (cache->add("key", 3, "value", 5, 0, 0));
	assert (cache->replace("key", 3, "newvalue5", 9, 0, 0));
	assert (!cache->add("key", 3, "value", 5, 0, 0));



	delete cache;

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

	getchar();

	return 0;
}