#pragma once
#include "socketdefs.h"
#include "Request.h"
#include "Cache.h"
#include "ByteStream.h"
#include <map>

class Server
{
private:
	typedef std::map<UINT64, Request *> REQUESTMAP;

public:
	Server();
	~Server();
	int main(int argc, char **argv);
	void shutdown();

private:
	Cache *m_cache;
	SOCKET m_sockfd;

private:
	SOCKET createSocket(int port);
	void decodeRequest(Request *request);
	
	UINT8 *m_buffer;

	bool m_bIsRunning;

};