#pragma once
#include "socketdefs.h"
#include "Packet.h"
#include "Protocol.h"
#include "Request.h"

class Response
{
public:
	Response(protocol::Commands cmd, Request *request);
	~Response(void);

public:
	void write(UINT8 *data, size_t cbData);
	void write(UINT8 value);
	void write(UINT16 value);
	void write(UINT32 value);
	void write(UINT64 value);
	
	void send(SOCKET sockfd);

	static void *operator new (size_t _size);
	static void operator delete (void *_p);

private:
	void ensureSmallSpace(size_t sz);

	void preparePacket();

private:
	Packet *m_head;
	Packet *m_tail;
	Request *m_request;

	UINT8 *m_offset;
	UINT8 *m_end;

	size_t m_packets;
	UINT16 m_rid;
	UINT8 m_cmd;

	UINT16 m_seq;
};