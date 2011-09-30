#pragma once

#include "types.h"
#include "IUltraCache.h"
#include "socketdefs.h"
#include "Packet.h"
#include "PacketReader.h"
#include "ByteStream.h"

#include <time.h>

class Client : public IUltraCache
{
public:
	Client(int timeoutSEC);
	virtual ~Client(void);

	//IUltraCache
	bool set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
	bool del(const char *key, size_t cbKey, time_t *expiration);
	bool add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);

	bool replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags);
	bool append(const char *key, size_t cbKey, void *data, size_t cbData);
	bool prepend(const char *key, size_t cbKey, void *data, size_t cbData);
	
	bool cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags);
	bool incr(const char *key, size_t cbKey, UINT64 increment);
	bool decr(const char *key, size_t cbKey, UINT64 decrement);
	bool version(char **version, size_t *cbVersion);
	
	HANDLE get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas);
	void release(HANDLE handle);
	//=======

	bool isConnected();
	void connect(const sockaddr_in &remoteAddr);
	void disconnect(void);
	
	virtual void wouldSleep(int msec);
	virtual int wouldBlock(SOCKET fd, int op, const timeval *tv);


public:
	enum Errors
	{
		SUCCESS,
		NOT_CONNECTED,
		KEY_TOO_LONG,
		VALUE_TO_LONG,
		PROTOCOL_ERROR,
		CONNECTION_TIMEDOUT,
	};

	Errors getError();

private:
	unsigned int getNextRid();
	void setError(Errors _error);
	Packet *waitForPacket(struct sockaddr_in *outRemoteAddr);
	
	bool readResponse(PacketReader &reader, ByteStream &bs);

private:
	SOCKET m_sockfd;
	UINT16 m_rid;

	struct sockaddr_in m_remoteAddr;
	Errors m_error;

	int m_timeout;

	UINT8 m_buffer[CONFIG_MAX_REQUEST_SIZE];
	size_t m_cbBuffer;
};