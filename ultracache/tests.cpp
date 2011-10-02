#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include "Cache.h"
#include "config.h"
#include "Server.h"
#include "Client.h"
#include "JThread.h"

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

void *ServerThread(void *arg)
{
	Server *server = (Server *) arg;
	server->main(0, NULL);
	return NULL;
}



int tests (int argc, char **argv)
{
	Hash::HashItem *inptr = (Hash::HashItem *) 0xfafa3130fafa3130ULL;
	size_t cptr = Hash::compressPtr(inptr);
	assert (Hash::decompressPtr(cptr) == inptr);
	
	Server server;

	JThread thread = JThread::createThread(ServerThread, &server);
	MSECSleep(1000);

	Client client(1);

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
	
	// Connect to unknown address (?)

	// Set sync
	assert (client.set("KEY", 3, (void *) "VALUE", 5, 0, 0, false));
	
	// Set async
	assert (client.set("KEY", 3, (void *) "VALUE2", 6, 0, 0, true));
	MSECSleep(500);

	// Set expire relative
	// Set expire absolute
	
	// Get 
	assert (client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 6);
	assert (memcmp(value, "VALUE2", 6) == 0);
	

/*
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
	*/


	server.shutdown();


#ifdef _WIN32
	thread.join();

#endif

	return 0;
}
