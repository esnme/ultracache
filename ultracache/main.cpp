#include "Server.h"
#include "socketdefs.h"

int tests (int argc, char **argv);

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
	
	Server server;

	return server.main(argc, argv);
}