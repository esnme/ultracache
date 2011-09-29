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
	int main(int argc, char **argv);
	
private:
	Cache *m_cache;
	SOCKET m_sockfd;

private:
	SOCKET createSocket(int port);
	void decodeRequest(Request *request);
	void sendResponse(Request *request, protocol::Commands cmd, ByteStream *data);


};