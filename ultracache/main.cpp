#include "Server.h"
#include "socketdefs.h"


int main (int argc, char **argv)
{

#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

	Server server;

	return server.main(argc, argv);
}