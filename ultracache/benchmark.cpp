#include "Server.h"
#include "Client.h"

#define CLIENT_COUNT 8
#define MULTI_GETS 64

void *ServerProc(void *arg)
{
	Server *server = (Server *) arg;
	server->main(0, NULL);
	return NULL;
}

volatile UINT64 g_writeCount = 0;
volatile UINT64 g_readCount = 0;

void *ClientProc(void *arg)
{
	srand(PortableGetCurrentThreadId());

	struct sockaddr_in *remoteAddr = (sockaddr_in *) arg;

	Client client(1);
	client.connect(*remoteAddr);

	char value[64] = { 'Z' };
	size_t valueLen = sizeof(value);

	size_t cCount = 0;

	while (cCount < 100000)
	{
		char strKey[16];
		sprintf(strKey, "%08x%08x", PortableGetCurrentThreadId(), cCount);

		bool bResult = client.set(strKey, 16, value, valueLen, 0, 0, false);
				
		g_writeCount ++;

		if (!bResult && client.getError() == Client::SERVER_ERROR_OOM)
		{
			break;
		}

		cCount ++;
	}

	fprintf (stderr, "Switching to reads...\n");

	while (true)
	{
		Client::MGETHANDLE handles[MULTI_GETS];


		for (int index = 0; index < MULTI_GETS; index ++)
		{

			int key = rand () % cCount;
			char strKey[16];
			sprintf(strKey, "%08x%08x", PortableGetCurrentThreadId(), key);

			handles[index] = client.getMulti(strKey, 16);
		}

		int offset = 0;

		void *pValue;
		size_t valueLen;
		int flags;
		UINT64 cas;

		while (client.readMulti(handles, MULTI_GETS, offset, NULL, 0, &pValue, &valueLen, &flags, &cas))
		{
			g_readCount ++;
		}

		if (offset != MULTI_GETS)
		{
			fprintf (stderr, "Request lost at offset %d\n", offset);
		}

	}



	return NULL;
}

int benchmark (int argc, char **argv)
{
	char *serverAddress = "192.168.1.174";
	char *noserver = NULL;

	for (int index = 1; index < argc-1; index ++)
	{
		if (strcmp(argv[index], "-address") == 0)
		{
			serverAddress = argv[index + 1];
		}
		else
		if (strcmp(argv[index], "-noserver") == 0)
		{
			noserver = argv[index + 1];
		}

	}

	Server *server;

	JThread serverThread;

	if (!noserver)
	{
		server = new Server();
		serverThread = JThread::createThread(ServerProc, server);
		MSECSleep(5000);
	}

	fprintf (stderr, "%s: Using server address %s\n", __FUNCTION__, serverAddress);

	JThread clients[CLIENT_COUNT];

	struct sockaddr_in remoteAddr;
	memset (&remoteAddr, 0, sizeof(sockaddr_in));
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(11211);
	remoteAddr.sin_addr.s_addr = inet_addr(serverAddress);

	for (int index = 0; index < CLIENT_COUNT; index ++)
	{
		clients[index] = JThread::createThread(ClientProc, &remoteAddr);
	}

	UINT64 wlast = 0;
	UINT64 rlast = 0;

	while (true)
	{
		fprintf (stderr, "%llu writes %llu reads/sec\n", g_writeCount - wlast,  g_readCount - rlast);
		wlast = g_writeCount;
		rlast = g_readCount;

		MSECSleep(1000);
	}



}
