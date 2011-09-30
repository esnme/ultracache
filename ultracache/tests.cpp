#include <assert.h>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include "Cache.h"
#include "config.h"
#include "Server.h"
#include "Client.h"

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
	Server server;

#ifdef _WIN32
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ServerThread, &server, 0, NULL);
#endif
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
	assert (client.set("KEY", 3, "VALUE", 5, 0, 0, false));
	
	// Set async
	assert (client.set("KEY", 3, "VALUE2", 6, 0, 0, true));
	MSECSleep(500);

	// Get 
	assert (client.get("KEY", 3, &value, &cbValue, &flags, &cas));
	assert (cbValue == 6);
	assert (memcmp(value, "VALUE", 6) == 0);

	


#ifdef _WIN32
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

#endif

	return 0;
}