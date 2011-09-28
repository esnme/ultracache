#include "Request.h"
#include <assert.h>


void decodeRequest(Request *request)
{
	UINT8 buffer[CONFIG_MAX_REQUEST_SIZE];

	UINT8 *offset = buffer;

	Packet *packet = request->getPackets();
	
	assert(packet);

	while (packet)
	{
		memcpy (offset, packet->getPayload(), packet->getPayloadSize());
		offset += packet->getPayloadSize();
		packet = packet->next;
	}


	try
	{

	switch (request->getCommand())
	{
	case protocol::SET:
		break;
	case protocol::DEL:
		break;
	case protocol::ADD:
		break;
	case protocol::REPLACE:
		break;
	case protocol::APPEND:
		break;
	case protocol::PREPEND:
		break;
	case protocol::CAS:
		break;
	case protocol::INCR:
		break;
	case protocol::DECR:
		break;
	case protocol::GET:
		break;
	case protocol::VERSION:
		break;
	default:
		break;
	}
	}




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



}