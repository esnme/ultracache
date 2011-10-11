#include "Server.h"
#include "socketdefs.h"
#include "TimeProvider.h"
int tests (int argc, char **argv);
int benchmark (int argc, char **argv);

TimeProvider g_tp;


int main (int argc, char **argv)
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

	if (argc > 1 && strcmp(argv[1], "--tests") == 0)
	{
		return tests(argc, argv);
	}

	if (argc > 1 && strcmp(argv[1], "--benchmark") == 0)
	{
		return benchmark(argc, argv);
	}

	Server server;

	TimeProvider::setup(time(0), false);
	
	return server.main(argc, argv);
}