#include "Server.h"
#include "ByteStream.h"
#include <assert.h>
#include "Response.h"

Server::Server()
{
	m_buffer = new UINT8[CONFIG_MAX_REQUEST_SIZE];
	m_bIsRunning = true;
}

Server::~Server()
{
	delete m_buffer;
}

SOCKET Server::createSocket(int port)
{
	SOCKET sockfd;

	/* Setup initial UDP socket */
  sockfd = socket (AF_INET, SOCK_DGRAM, 0);

	if (sockfd == -1)
	{
		fprintf (stderr, "%s: Failed to create AF_INET socket\n", __FUNCTION__);
		return -1;
	}

/**/

	int size = (1024 * 1024 * 64);
	
	while (true)
	{
		if (setsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, (char*) &size, sizeof(size)) == -1 ||
				setsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, (char*) &size, sizeof(size)) == -1)
		{
			size >>= 1;
		}

		break;
	}

	fprintf (stderr, "SO_SNDBUF: %u\n", size);
	fprintf (stderr, "SO_RCVBUF: %u\n", size);

	int flag = 0;
#ifdef _LINUX
	if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &flag, sizeof(flag));
#endif

#ifdef _WIN32
	setsockopt (sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &flag, sizeof (flag));
#endif
	flag=0;
	setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&flag, sizeof(flag)); 



	struct sockaddr_in bindAddr;

	memset(&bindAddr, 0, sizeof (sockaddr_in));

	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	bindAddr.sin_port = htons(port);

	if (bind(sockfd, (sockaddr *) &bindAddr, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf (stderr, "%s: Failed to bind port %d\n", __FUNCTION__, port);
		return -1;
	}
		
	return sockfd;
}

void Server::decodeRequest(Request *request)
{
	UINT8 *offset = m_buffer;

	int cbBuffer = request->copyToBuffer(m_buffer, CONFIG_MAX_REQUEST_SIZE);

	if (cbBuffer == -1)
	{
		return;
	}


	try
	{
		ByteStream bs(m_buffer, (size_t) cbBuffer);
		/*
		bool set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
		bool del(const char *key, size_t cbKey, time_t *expiration);
		bool add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);

		bool replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
		bool append(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
		bool prepend(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);

		bool cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags);
		bool incr(const char *key, size_t cbKey, UINT64 increment);
		bool decr(const char *key, size_t cbKey, UINT64 decrement);
		bool version(char **version, size_t *cbVersion);

		KwHandle get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas);
		*/

		switch (request->getCommand())
		{
		case protocol::SET:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				size_t valueLen = bs.readUINT32();
				void *value = bs.read(valueLen);
				time_t expiration = bs.readUINT32();
				int flags = bs.readUINT32();
				bool bResult = m_cache->set( (char *) key, keyLen, value, valueLen, expiration, flags, false);
				
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_ERROR_OOM, request->getRemoteAddr(), request->getRid());
					response->send(m_sockfd);
					delete response;
				}
				break;
			}

		case protocol::ADD:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				size_t valueLen = bs.readUINT32();
				void *value = bs.read(valueLen);
				time_t expiration = bs.readUINT32();
				int flags = bs.readUINT32();
				bool bResult = m_cache->add( (char *) key, keyLen, value, valueLen, expiration, flags, false);
				
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_NOT_STORED, request->getRemoteAddr(), request->getRid());
					response->send(m_sockfd);
					delete response;
				}
				break;
			}
		case protocol::REPLACE:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				size_t valueLen = bs.readUINT32();
				void *value = bs.read(valueLen);
				time_t expiration = bs.readUINT32();
				int flags = bs.readUINT32();
				bool bResult = m_cache->replace( (char *) key, keyLen, value, valueLen, expiration, flags, false);
				
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_NOT_STORED, request->getRemoteAddr(), request->getRid());
					response->send(m_sockfd);
					delete response;
				}
				break;
			}
		case protocol::APPEND:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				size_t valueLen = bs.readUINT32();
				void *value = bs.read(valueLen);
				bool bResult = m_cache->append( (char *) key, keyLen, value, valueLen, false);
				
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_ERROR_OOM, request->getRemoteAddr(), request->getRid());
					response->send(m_sockfd);
					delete response;
				}

				break;
			}
		case protocol::PREPEND:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				size_t valueLen = bs.readUINT32();
				void *value = bs.read(valueLen);
				bool bResult = m_cache->prepend( (char *) key, keyLen, value, valueLen, false);
				
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_ERROR_OOM, request->getRemoteAddr(), request->getRid());
					response->send(m_sockfd);
					delete response;
				}
				break;

			}
		case protocol::CAS:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				size_t valueLen = bs.readUINT32();
				void *value = bs.read(valueLen);
				UINT64 casUnique = bs.readUINT64();
				time_t expiration = bs.readUINT32();
				int flags = bs.readUINT32();
				bool bResult = m_cache->cas( (char *) key, keyLen, casUnique, value, valueLen, expiration, flags, false);
				
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_EXISTS, request->getRemoteAddr(), request->getRid());
					response->send(m_sockfd);
					delete response;
				}
				break;
			}
		case protocol::INCR:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				UINT64 value = bs.readUINT64();
				bool bResult = m_cache->incr( (char *) key, keyLen, value, false);
				
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_NOT_FOUND, request->getRemoteAddr(), request->getRid());
					response->write(value);
					response->send(m_sockfd);
					delete response;
				}

				break;
			}
		case protocol::DECR:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				UINT64 value = bs.readUINT64();
				bool bResult = m_cache->decr( (char *) key, keyLen, value, false);
			
				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_STORED : protocol::RESULT_NOT_FOUND, request->getRemoteAddr(), request->getRid());
					response->write(value);
					response->send(m_sockfd);
					delete response;
				}
				break;
			}

		case protocol::DEL:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				time_t expiration;

				bool bResult = m_cache->del( (char *) key, keyLen, &expiration, false);
				UINT32 exp = (UINT32) expiration;

				if (!request->isAsync())
				{
					Response *response = new Response(bResult ? protocol::RESULT_DELETED: protocol::RESULT_NOT_FOUND, request->getRemoteAddr(), request->getRid());
					response->write(exp);
					response->send(m_sockfd);
					delete response;
				}
				break;
			}

		case protocol::VERSION:
			{
				char *version;
				size_t cbVersion;

				m_cache->version(&version, &cbVersion);
				Response *response = new Response(protocol::RESULT_VERSION, request->getRemoteAddr(), request->getRid());

				response->write( (UINT8) cbVersion);
				response->write( (UINT8*) version, cbVersion);
				break;
			}

		case protocol::GET:
			{
				size_t keyLen = bs.readUINT8();
				void *key = bs.read(keyLen);
				void *value;
				size_t cbValue;
				int flags;
				UINT64 cas;

				//VALUE <key> <flags> <bytes> [<cas unique>]\r\n
				Cache::HANDLE handle = m_cache->get( (char *) key, keyLen, &value, &cbValue, &flags, &cas);

				Response *response;

				if (handle)
				{
					response = new Response(protocol::RESULT_GET, request->getRemoteAddr(), request->getRid());
					response->write( (UINT8) keyLen);
					response->write( (UINT8 *) key, keyLen);
					response->write ( (UINT32) flags);
					response->write ( (UINT64) cas);
					response->write( (UINT32) cbValue);
					response->write( (UINT8*) value, cbValue);

					m_cache->release(handle);
				}
				else
				{
					response = new Response(protocol::RESULT_NOT_FOUND, request->getRemoteAddr(), request->getRid());
					response->write( (UINT8) keyLen);
					response->write( (UINT8 *) key, keyLen);
				}

				response->send(m_sockfd);
				delete response;
				break;
			}

		default:
			{
				break;
			}
		}
	}
	catch (ByteStream::Exception &e)
	{
		return;
	}

}

int Server::main(int argc, char **argv)
{
	REQUESTMAP rmap;

	SOCKET sockfd = Server::createSocket(11211);

	if (sockfd == -1)
	{
		return -1;
	}

	Server::m_sockfd = sockfd;

	Server::m_cache = new Cache(1024);

	while (m_bIsRunning)
	{
		struct sockaddr_in remoteAddr;

		Packet *packet = new Packet();
		socklen_t addrLen = sizeof(struct sockaddr_in);

		int recvResult = recvfrom (sockfd, (char *) packet->getHeader(), (int) packet->getBufferSize(), MSG_NOSIGNAL, (sockaddr *) &remoteAddr, &addrLen);

		if (recvResult == -1)
		{
			delete packet;
			continue;
		}

		if (recvResult < sizeof (protocol::Header))
		{
			delete packet;
			continue;
		}

		packet->setupBuffer(recvResult);

		protocol::Header *header = packet->getHeader();

		UINT64 nodeId = 0;

		nodeId |= ( ((UINT64) remoteAddr.sin_addr.s_addr) << 32ULL);
		nodeId |= ( ((UINT64) remoteAddr.sin_port) << 16ULL);
		nodeId |= ( ((UINT64) header->rid) << 0ULL);

		REQUESTMAP::iterator iter = rmap.find(nodeId);

		Request *request;

		if (iter == rmap.end())
		{
			request = new Request();
			rmap[nodeId] = request;
			iter = rmap.find(nodeId);
		}
		else
		{
			request = iter->second;
		}

		Request::Result result = request->put(&remoteAddr, packet);

		switch (result)
		{
		case Request::NEXTPACKET:
			break;

		case Request::COMPLETE:
			decodeRequest(request);

			// Fall through
		case Request::FAILED:
			rmap.erase(iter);

			delete request;
			break;
		}


	}

	SocketClose(sockfd);
	return 0;
}


void Server::shutdown()
{
	m_bIsRunning = false;
	SocketClose(m_sockfd);
}