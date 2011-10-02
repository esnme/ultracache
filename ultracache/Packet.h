#pragma once

#include "config.h"
#include "types.h"
#include "Protocol.h"
#include "socketdefs.h"

class Packet
{
public:
	protocol::Header *getHeader();
	UINT8 *getPayload();
	void setup(size_t cbTotal);

	size_t getBufferSize();
	size_t getPayloadSize();
	size_t getTotalSize();

	static void *operator new (size_t _size);
	static void operator delete (void *_p);

	void setupBuffer(size_t cbTotal);
	
	struct sockaddr_in *getRemoteAddr();

private:
	UINT8 m_buffer[CONFIG_PACKET_SIZE];
	size_t m_cbTotal;
	struct sockaddr_in m_remoteAddr;

public:
	Packet *next;


};