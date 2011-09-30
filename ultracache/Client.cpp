#include "Client.h"
#include "PacketWriter.h"
#include "PacketReader.h"
#include <stdlib.h>
#include "Protocol.h"
#include "ByteStream.h"

#include <assert.h>

//FIXME: "Implement async handling"
//FIXME: "Implement multi-get"

Client::Client(int timeoutSEC) 
{
	m_sockfd = -1;
	m_rid = rand();
	m_error = Client::SUCCESS;
	m_timeout = timeoutSEC;

	m_buffer = new UINT8[CONFIG_MAX_REQUEST_SIZE];
	m_cbBuffer = CONFIG_MAX_REQUEST_SIZE;
}

Client::~Client(void)
{
	delete m_buffer;
	disconnect();
}

unsigned int Client::getNextRid()
{
	return m_rid ++;
}

void Client::setError(Errors _error)
{
	m_error = _error;
}

Client::Errors Client::getError()
{
	return m_error;
}

void Client::wouldSleep(int msec)
{
	Sleep(msec);
}

int Client::wouldBlock(SOCKET fd, int op, const timeval *tv)
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(fd, &set);

	switch (op)
	{
	case FD_READ: 
		return select (fd + 1, &set, NULL, NULL, tv);

	case FD_WRITE:
		return select (fd + 1, NULL, &set, NULL, tv);

	default:
		assert (false);
		return -1;
	}
}

Packet *Client::waitForPacket(struct sockaddr_in *outRemoteAddr, unsigned int expRid)
{
	Packet *packet = new Packet();

	struct timeval tv;
	tv.tv_sec = m_timeout;
	tv.tv_usec = 0;

	while (true)
	{
		socklen_t len = sizeof (sockaddr_in);
		int result = recvfrom(m_sockfd, (char *) packet->getHeader(), packet->getBufferSize(), MSG_NOSIGNAL, (sockaddr *) outRemoteAddr, &len);

		switch (result)
		{
		case -1:
			if (SocketWouldBlock(m_sockfd))
			{
				int result = wouldBlock(m_sockfd, FD_READ, &tv);

				if (result < 1)
				{
					delete packet;
					return NULL;
				}

				continue;
			}
			else
			{
				delete packet;
				return NULL;
			}
		case 0:
			delete packet;
			return NULL;

		default:
			if (result < sizeof(protocol::Header))
			{
				delete packet;
				return NULL;
			}

			packet->setupBuffer(result);

			if (packet->getHeader()->rid != expRid)
			{
				delete packet;
				return NULL;
			}

			return packet;
		}
	}
		
	return NULL;
}

bool Client::readResponse(PacketReader &reader, ByteStream &bs, unsigned int expRid)
{
	Packet *packet;
	struct sockaddr_in remoteAddr;

	while (true)
	{
		packet = waitForPacket(&remoteAddr, expRid);

		if (packet == NULL)
		{
			setError(CONNECTION_TIMEDOUT);
			return false;
		}

		PacketReader::Result result = reader.put(&remoteAddr, packet);

		switch (result)
		{
		case PacketReader::FAILED: return false;
		case PacketReader::NEXTPACKET: break;

		case PacketReader::COMPLETE: 
			{
				int sz = reader.copyToBuffer(m_buffer, m_cbBuffer);

				if (sz == -1)
				{
					setError(PROTOCOL_ERROR);
					return false;
				}

				bs = ByteStream(m_buffer, (size_t) sz);

				return true;
			}
		}
	}

	return false;
}



bool Client::set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT32 data
	[data]
	UINT32 expiration
	UINT32 flags

	Reply:

	*/

	PacketWriter writer(protocol::SET, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	if (cbData > CONFIG_VALUE_LENGTH)
	{
		setError(VALUE_TO_LONG);
		return false;
	}
	
	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT32) cbData);
	writer.write((UINT8 *) data, cbData);
	writer.write((UINT32) expiration);
	writer.write((UINT32) flags);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_ERROR_OOM: return false;
		default: 
			assert (false);
			return false;
	}

	return true;
}

bool Client::del(const char *key, size_t cbKey, time_t *expiration, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT32 data
	[data]

	Reply:
	UINT32 expiration

	*/

	PacketWriter writer(protocol::SET, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}


	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_DELETED: break; 
		case protocol::RESULT_NOT_FOUND: return false;
		default: assert(false); return false;
	}

	*expiration = bs.readUINT32();
	return true;
}

bool Client::add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT32 data
	[data]
	UINT32 expiration
	UINT32 flags

	Reply:

	*/

	PacketWriter writer(protocol::ADD, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	if (cbData > CONFIG_VALUE_LENGTH)
	{
		setError(VALUE_TO_LONG);
		return false;
	}
	
	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT32) cbData);
	writer.write((UINT8 *) data, cbData);
	writer.write((UINT32) expiration);
	writer.write((UINT32) flags);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_NOT_STORED: return false;
		default: 
			assert (false);
			return false;
	}

	return true;
}

bool Client::replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT32 data
	[data]
	UINT32 expiration
	UINT32 flags

	Reply:

	*/

	PacketWriter writer(protocol::REPLACE, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	if (cbData > CONFIG_VALUE_LENGTH)
	{
		setError(VALUE_TO_LONG);
		return false;
	}
	
	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT32) cbData);
	writer.write((UINT8 *) data, cbData);
	writer.write((UINT32) expiration);
	writer.write((UINT32) flags);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_NOT_STORED: return false;
		default: 
			assert (false);
			return false;
	}

	return true;
}

bool Client::append(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT32 data
	[data]

	Reply:

	*/

	PacketWriter writer(protocol::APPEND, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	if (cbData > CONFIG_VALUE_LENGTH)
	{
		setError(VALUE_TO_LONG);
		return false;
	}
	
	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT32) cbData);
	writer.write((UINT8 *) data, cbData);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_ERROR_OOM: return false;
		default: 
			assert (false);
			return false;
	}

	return true;
}

bool Client::prepend(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT32 data
	[data]

	Reply:

	*/

	PacketWriter writer(protocol::PREPEND, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	if (cbData > CONFIG_VALUE_LENGTH)
	{
		setError(VALUE_TO_LONG);
		return false;
	}
	
	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT32) cbData);
	writer.write((UINT8 *) data, cbData);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_ERROR_OOM: return false;
		default: 
			assert (false);
			return false;
	}

	return true;
}
	
bool Client::cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT32 data
	[data]
	UINT64 cas
	UINT32 expiration
	UINT32 flags

	Reply:

	*/

	PacketWriter writer(protocol::SET, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	if (cbData > CONFIG_VALUE_LENGTH)
	{
		setError(VALUE_TO_LONG);
		return false;
	}
	
	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT32) cbData);
	writer.write((UINT8 *) data, cbData);
	writer.write((UINT64) casUnique);
	writer.write((UINT32) expiration);
	writer.write((UINT32) flags);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_EXISTS: return false;
		default: 
			assert (false);
			return false;
	}

	return true;
}

bool Client::incr(const char *key, size_t cbKey, UINT64 increment, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT64 value

	Reply:

	*/

	PacketWriter writer(protocol::INCR, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT64) increment);
	writer.send(m_sockfd);

	if (bAsync)
	{
		return true;
	}
	
	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_NOT_FOUND: return false;
		default: 
			assert (false);
			return false;
	}

	//FIXME: Server's gonna send us new value back, do we really want to ignore it?

	return true;
}

bool Client::decr(const char *key, size_t cbKey, UINT64 decrement, bool bAsync)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT64 value

	Reply:

	*/

	PacketWriter writer(protocol::DECR, m_remoteAddr, getNextRid(), bAsync);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return false;
	}

	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.write((UINT64) decrement);
	writer.send(m_sockfd);
	
	if (bAsync)
	{
		return true;
	}

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_STORED: return true;
		case protocol::RESULT_NOT_FOUND: return false;
		default: 
			assert (false);
			return false;
	}

	//FIXME: Server's gonna send us new value back, do we really want to ignore it?

	return true;
}

bool Client::version(char **version, size_t *cbVersion)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Reply:
	UINT8 versionLen
	[version]

	*/

	PacketWriter writer(protocol::VERSION, m_remoteAddr, getNextRid(), false);
	writer.send(m_sockfd);

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return false;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_VERSION: 
			break;

		default: 
			assert (false);
			return false;
	}

	*cbVersion = bs.readUINT8();
	*version = (char *) bs.read(*cbVersion);
	return true;
}
	
HANDLE Client::get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	Request:
	UINT8 key
	[key]
	UINT64 value

	Reply:

	*/

	PacketWriter writer(protocol::GET, m_remoteAddr, getNextRid(), false);

	if (cbKey > CONFIG_KEY_LENGTH)
	{
		setError(KEY_TOO_LONG);
		return NULL;
	}

	writer.write((UINT8) cbKey);
	writer.write((UINT8 *) key, cbKey);
	writer.send(m_sockfd);

	ByteStream bs;
	PacketReader reader;
	
	if (!readResponse(reader, bs, writer.getRid()))
	{
		return NULL;
	}

	switch (reader.getCommand())
	{
		case protocol::RESULT_GET:
			break;
		case protocol::RESULT_NOT_FOUND: 
			//FIXME: Server sends key back here, do we really want it?
			return NULL;
		default: 
			assert (false);
			return NULL;
	}

	size_t outKeyLen = bs.readUINT8();
	UINT8 *outKey = bs.read(outKeyLen);

	*_outFlags = bs.readUINT32();
	*_outCas = bs.readUINT64();

	*_cbOutValue = bs.readUINT32();
	*outValue = bs.read(*_cbOutValue);

	return (Client::HANDLE) m_buffer;

}

void Client::release(HANDLE handle)
{
}

bool Client::isConnected()
{
	return (m_sockfd > 0);
}

bool Client::connect(const sockaddr_in &remoteAddr)
{
	disconnect();

	m_remoteAddr = remoteAddr;

	SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (sockfd == -1)
	{
		return false;
	}

	SocketSetNonBlock(sockfd, true);
	
	::connect(sockfd, (sockaddr *) &m_remoteAddr, sizeof(struct sockaddr_in));

	m_sockfd = sockfd;

	return true;
}

void Client::disconnect(void)
{
	if (m_sockfd == -1)
	{
		return;
	}

	SocketClose(m_sockfd);
	m_sockfd = -1;
}

