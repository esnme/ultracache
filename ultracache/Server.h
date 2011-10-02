#pragma once
#include "socketdefs.h"
#include "Request.h"
#include "Cache.h"
#include "ByteStream.h"
#include <map>
#ifdef _WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif
#include "JThreadQueue.h"
#include "Response.h"
#include "JThread.h"
#include "Spinlock.h"
#include "types.h"

class Server
{
private:
#ifdef _WIN32
	typedef std::hash_map<UINT64, Request *> REQUESTMAP;
#else
	typedef __gnu_cxx::hash_map<UINT64, Request *> REQUESTMAP;
#endif

public:
	Server();
	~Server();
	int main(int argc, char **argv);
	void shutdown();
	void rxThread();
	void txThread();

private:
	Cache *m_cache;
	SOCKET m_sockfd;

private:
	SOCKET createSocket(int port);
	void decodeRequest(Request *request);
	

	UINT8 *m_buffer;

	volatile bool m_bIsRunning;

	JThreadQueue<Request *> m_rxQueue;
	JThreadQueue<Response *> m_txQueue;

	JThread m_rxThread[2];
	JThread m_txThread[2];

	Spinlock m_rmapSL;
	REQUESTMAP m_rmap;
};
