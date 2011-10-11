#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include "Cache.h"
#include "config.h"
#include "Server.h"
#include "Client.h"
#include "JThread.h"
#include "TimeProvider.h"

void dumpMem(const char *desc, void *_ptr, size_t cbBytes);
size_t logh(size_t val);

extern TimeProvider g_tp;

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

void *ServerThread(void *arg)
{
	Server *server = (Server *) arg;
	server->main(0, NULL);
	return NULL;
}



int tests (int argc, char **argv)
{
	Server server;

	TimeProvider::setup(0, true);

	JThread thread = JThread::createThread(ServerThread, &server);
	MSECSleep(1000);

	Client client(60);

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof (struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(11211);

	int flags;
	UINT64 cas;
	void *value;
	size_t cbValue;
	
	// Connect
	assert(client.connect(serverAddr));
	time_t now;
	time_t exp;

	// Connect to unknown address (?)

	/*
	//===========================================================================
	// Set sync
	//===========================================================================
	assert (client.set("KEY", 3, (void *) "VALUE", 5, 0, 0, false));
	assert (client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 5);
	assert (memcmp(value, "VALUE", 5) == 0);
	//===========================================================================

	//===========================================================================
	// Set async
	//===========================================================================
	assert (client.set("KEY", 3, (void *) "VALUE2", 6, 0, 0, true));
	MSECSleep(500);
	assert (client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 6);
	assert (memcmp(value, "VALUE2", 6) == 0);
	//===========================================================================

	//===========================================================================
	// Set expire relative
	//===========================================================================
	assert (client.set("KEY", 3, (void *) "VALUE2", 6, 10, 0, false));
	TimeProvider::jumpTime(5);
	assert (client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	TimeProvider::jumpTime(6);
	assert (!client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	//===========================================================================

	//===========================================================================
	// Set expire absolute
	//===========================================================================
	now = TimeProvider::getTime() + 10;

	assert (client.set("KEY", 3, (void *) "VALUE2", 6, now, 0, false));
	TimeProvider::jumpTime(5);
	assert (client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	TimeProvider::jumpTime(6);
	assert (!client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	//===========================================================================

	//===========================================================================
	// Del sync found
	//===========================================================================

	now = TimeProvider::getTime() + 10;
	assert (client.set("KEYDEL", 6, (void *) "VALUE", 5, now, 0, false));
	assert (client.del("KEYDEL", 6, &exp, false));
	assert (exp == now);
	//===========================================================================

	//===========================================================================
	// Del sync not found
	//===========================================================================
	assert (!client.del("KEYDEL NOTFOUND", 15, &exp, false));
	//===========================================================================
	*/

	//===========================================================================
	// Del sync expired
	//===========================================================================
	assert (client.set("KEYDEL EXPIRED", 14, (void *) "VALUE", 5, 5, 0, false));
	TimeProvider::jumpTime(6);
	assert (!client.del("KEYDEL EXPIRED", 14, &exp, false));
	//===========================================================================

	//===========================================================================
	// Del async found
	//===========================================================================
	assert (client.set("KEYDEL FOUND", 12, (void *) "VALUE", 5, 0, 0, true));
	MSECSleep(500);
	assert (client.del("KEYDEL FOUND", 12, &exp, true));
	MSECSleep(500);
	assert (!client.del("KEYDEL FOUND", 12, &exp, false));
	//===========================================================================

	//===========================================================================
	// Add found
	//===========================================================================
	assert (client.set("KEYADD FOUND", 12, (void *) "VALUE", 5, 5, 0, false));
	assert (!client.add("KEYADD FOUND", 12, (void *) "VALUE2", 6, 0, 0, false));
	//===========================================================================

	//===========================================================================
	// Add not found
	//===========================================================================
	assert (client.add("KEYADD NOTFOUND", 15, (void *) "VALUE", 5, 0, 0, false));
	//===========================================================================

	//===========================================================================
	// Add expired found
	//===========================================================================
	assert (client.set("KEYADD EXPIRE", 13, (void *) "VALUE", 5, 5, 0, false));
	TimeProvider::jumpTime(6);
	assert (client.add("KEYADD EXPIRE", 13, (void *) "VALUE2", 6, 0, 0, false));
	//===========================================================================

	//FIXME: No async tests for ADD?
	//FIXME: No async tests for REPLACE?

	//===========================================================================
	// Replace found
	//===========================================================================
	assert (client.set("KEYREP FOUND", 12, (void *) "VALUE", 5, 5, 0, false));
	assert (client.replace("KEYREP FOUND", 12, (void *) "VALUE2", 6, 0, 0, false));
	//===========================================================================

	//===========================================================================
	// Replace not found
	//===========================================================================
	assert (!client.replace("KEYREP NOTFOUND", 15, (void *) "VALUE", 5, 0, 0, false));
	//===========================================================================

	//===========================================================================
	// Replace expired found
	//===========================================================================
	assert (client.set("KEYREP EXPIRE", 13, (void *) "VALUE", 5, 5, 0, false));
	TimeProvider::jumpTime(6);
	assert (!client.replace("KEYREP EXPIRE", 13, (void *) "VALUE2", 6, 0, 0, false));
	//===========================================================================



/*
	bool replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool append(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync);
	bool prepend(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync);
	
	bool cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool incr(const char *key, size_t cbKey, UINT64 increment, bool bAsync);
	bool decr(const char *key, size_t cbKey, UINT64 decrement, bool bAsync);
	bool version(char **version, size_t *cbVersion);
	
	HANDLE get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas);
	*/


	server.shutdown();


#ifdef _WIN32
	thread.join();

#endif

	return 0;
}
