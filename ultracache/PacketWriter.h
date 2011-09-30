#pragma once

#include "Protocol.h"
#include "types.h"
#include "socketdefs.h"
#include "Packet.h"

class PacketWriter
{
public:
	PacketWriter(protocol::Commands cmd, const struct sockaddr_in &_remoteAddr, unsigned int _rid);
	~PacketWriter(void);

public:
	void write(UINT8 *data, size_t cbData);
	void write(UINT8 value);
	void write(UINT16 value);
	void write(UINT32 value);
	void write(UINT64 value);
	void send(SOCKET sockfd);

private:
	void ensureSmallSpace(size_t sz);

	void preparePacket();

private:
	Packet *m_head;
	Packet *m_tail;
	struct sockaddr_in m_remoteAddr;
	
	UINT8 *m_offset;
	UINT8 *m_start;
	UINT8 *m_end;

	size_t m_packets;
	UINT16 m_rid;
	UINT8 m_cmd;

	UINT16 m_seq;
};