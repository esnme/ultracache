#include "Cache.h"
#include "Packet.h"
#include "Protocol.h"
#include "Request.h"
#include "Packet.h"
#include "socketdefs.h"
#include <stdio.h>
#include <map>
#include <assert.h>

//FIXME: We probably don't want to use STL here since it sucks
typedef std::map<UINT64, Request *> REQUESTMAP;

void decodeRequest(Request *request);


SOCKET setupSocket(int _port)
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
	if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &flag, sizeof(flag)) == -1)
	{
	}
#endif

#ifdef _WIN32
	if (setsockopt (sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &flag, sizeof (flag)) == -1)
	{
	}
#endif
	flag=0;
	setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&flag, sizeof(flag)); 



	struct sockaddr_in bindAddr;

	memset(&bindAddr, 0, sizeof (sockaddr_in));

	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	bindAddr.sin_port = htons(_port);

	if (bind(sockfd, (sockaddr *) &bindAddr, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf (stderr, "%s: Failed to bind port %d\n", __FUNCTION__, _port);
		return -1;
	}
		
	return sockfd;
}
int main (int argc, char **argv)
{
	REQUESTMAP rmap;

#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif


	SOCKET sockfd = setupSocket(11211);

	if (sockfd == -1)
	{
		return -1;
	}




	while (true)
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

		packet->setupBuffer(CONFIG_SIZEOF_PACKET_HEADER, recvResult);

		protocol::Header *header = (protocol::Header *) packet->getHeader();

		UINT64 nodeId = 0;

		nodeId |= ( ((UINT64) remoteAddr.sin_addr.S_un.S_addr) << 32ULL);
		nodeId |= ( ((UINT64) remoteAddr.sin_port) << 16ULL);
		nodeId |= ( ((UINT64) header->rid) << 0ULL);

		REQUESTMAP::iterator iter = rmap.find(nodeId);

		Request *request;

		if (iter == rmap.end())
		{
			request = new Request();
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

	getchar();

	SocketClose(sockfd);

	return 0;
}