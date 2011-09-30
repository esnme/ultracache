#include "Client.h"
#include "PacketWriter.h"
#include "PacketReader.h"
#include <stdlib.h>
#include "Protocol.h"

#include <assert.h>

Client::Client(void)
{
	m_sockfd = -1;
	m_rid = rand();
	m_error = Client::SUCCESS;
}

Client::~Client(void)
{
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



bool Client::set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	if (!isConnected())
	{
		setError(NOT_CONNECTED);
		return false;
	}

	/*
	UINT8 key
	[key]
	UINT32 data
	[data]
	UINT32 expiration
	UINT32 flags
	*/

	PacketWriter writer(protocol::SET, m_remoteAddr, getNextRid());

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

	PacketReader reader;


	return false;
}

bool Client::del(const char *key, size_t cbKey, time_t *expiration)
{
	return false;
}

bool Client::add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	return false;
}

bool Client::replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags)
{
	return false;
}

bool Client::append(const char *key, size_t cbKey, void *data, size_t cbData)
{
	return false;
}

bool Client::prepend(const char *key, size_t cbKey, void *data, size_t cbData)
{
	return false;
}
	
bool Client::cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags)
{
	return false;
}

bool Client::incr(const char *key, size_t cbKey, UINT64 increment)
{
	return false;
}

bool Client::decr(const char *key, size_t cbKey, UINT64 decrement)
{
	return false;
}

bool Client::version(char **version, size_t *cbVersion)
{
	return false;
}
	
HANDLE Client::get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas)
{
	return NULL;
}

void Client::release(HANDLE handle)
{
}

bool Client::isConnected()
{
	return false;
}

void Client::connect(const sockaddr_in &remoteAddr)
{
	m_remoteAddr = remoteAddr;
}

void Client::disconnect(void)
{
}

