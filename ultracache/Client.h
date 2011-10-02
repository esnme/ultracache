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

	typedef unsigned int MGETHANDLE;

public:
	Client(int timeoutSEC);
	virtual ~Client(void);

	//IUltraCache
	bool set(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool del(const char *key, size_t cbKey, time_t *expiration, bool bAsync);
	bool add(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);

	bool replace(const char *key, size_t cbKey, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool append(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync);
	bool prepend(const char *key, size_t cbKey, void *data, size_t cbData, bool bAsync);
	
	bool cas(const char *key, size_t cbKey, UINT64 casUnique, void *data, size_t cbData, time_t expiration, int flags, bool bAsync);
	bool incr(const char *key, size_t cbKey, UINT64 increment, bool bAsync);
	bool decr(const char *key, size_t cbKey, UINT64 decrement, bool bAsync);
	bool version(char **version, size_t *cbVersion);
	
	HANDLE get(const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas);
	void release(HANDLE handle);

	MGETHANDLE getMulti(const char *key, size_t cbKey);
	bool readMulti(MGETHANDLE *handles, size_t cHandles, int &offset, const char *key, size_t cbKey, void **outValue, size_t *_cbOutValue, int *_outFlags, UINT64 *_outCas);

	//=======

	bool isConnected();
	bool connect(const sockaddr_in &remoteAddr);
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
		SERVER_ERROR_OOM,
		SERVER_ERROR_UNKNOWN,
		PROTOCOL_ERROR_SEQUENCE,
		PROTOCOL_ERROR_SIZE,
	};

	Errors getError();

private:
	unsigned int getNextRid();
	void setError(Errors _error);
	Packet *waitForPacket(struct sockaddr_in *outRemoteAddr, unsigned int expRid);
	bool readResponse(PacketReader &reader, ByteStream &bs, unsigned int expRid);

private:
	SOCKET m_sockfd;
	UINT16 m_rid;

	struct sockaddr_in m_remoteAddr;
	Errors m_error;

	int m_timeout;

	UINT8 *m_buffer;
	size_t m_cbBuffer;
};