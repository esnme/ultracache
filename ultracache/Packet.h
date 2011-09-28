#pragma once

#include "config.h"
#include "types.h"


class Packet
{
public:
	UINT8 *getHeader();
	UINT8 *getPayload();
	void setupBuffer(size_t cbHeader, size_t cbTotal);

	size_t getBufferSize();
	size_t getPayloadSize();

	static void *operator new (size_t _size);
	static void operator delete (void *_p);
private:
	UINT8 m_buffer[CONFIG_PACKET_SIZE];
	UINT8 *m_payload;
	size_t m_cbHeader;

public:
	Packet *next;

};