#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include "Cache.h"
#include "config.h"

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
	(*ptr++) = '\0';

	return base;
}

int main (int argc, char **argv)
{
	Cache *cache = new Cache(412);
	int flags;
	UINT64 cas;
	void *value;
	size_t cbValue;

	cache->set("key", 3, "value", 5, 0, 0);
	cache->set("key", 3, "bigdeal", 7, 0, 0);
	assert (cache->get("key", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 7);
	assert (memcmp(value, "bigdeal", 7) == 0);

	cache->set("key", 3, "valuelonger", 11, 0, 0);
	assert (cache->get("key", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 11);
	assert (memcmp(value, "valuelonger", 11) == 0);


	assert (!cache->add("key", 3, "value", 5, 0, 0));
	assert (cache->del("key", 3, NULL));
	assert (cache->add("key", 3, "value", 5, 0, 0));

	assert (cache->replace("key", 3, "newvalue5", 9, 0, 0));
	assert (cache->get("key", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 9);
	assert (memcmp(value, "newvalue5", 9) == 0);

	assert (!cache->add("key", 3, "value", 5, 0, 0));

	assert (cache->append("key", 3, "1337", 4, 0, 0));
	assert (cache->prepend("key", 3, "4590",4, 0, 0));

	assert (cache->get("key", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 17);

	char *str = (char *) value;

	assert (memcmp(value, "4590newvalue51337", 17) == 0);

	assert (!cache->incr("key2", 4, 1));
	assert (cache->set("key2", 4, "0", 1, 0, 0));
	assert (cache->incr("key2", 4, 31337));
	assert (cache->get("key2", 4, &value, &cbValue, &flags, &cas));
	assert (cbValue == 5);
	assert (memcmp(value, "31337", 5) == 0);

	assert (cache->decr("key2", 4, 31338));
	assert (cache->get("key2", 4, &value, &cbValue, &flags, &cas));
	assert (cbValue == CONFIG_UINT64_STRING_LENGTH);
	assert (memcmp(value, "18446744073709551615", CONFIG_UINT64_STRING_LENGTH) == 0);
	

	assert (cache->set("key", 3, "test", 4, 0, 0));
	assert (cache->get("key", 3, &value, &cbValue, &flags, &cas));
	assert (cache->cas("key", 3, cas, "right cas", 9, 0, 0));
	assert (!cache->cas("key", 3, cas, "wrong cas", 9, 0, 0));

	assert (cache->get("key", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 9);
	assert (memcmp(value, "right cas", 9) == 0);





	delete cache;


	getchar();

	return 0;
}