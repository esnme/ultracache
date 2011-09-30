#pragma once
#include "Packet.h"
#include "Protocol.h"
#include "socketdefs.h"

class PacketReader
{

public:
	enum Result
	{
		NEXTPACKET,
		FAILED,
		COMPLETE
	};
	
public:
	PacketReader();
	~PacketReader();
	Result put(struct sockaddr_in *remoteAddr, Packet *packet);

	Packet *getPackets();
	protocol::Commands getCommand();
	bool isAsync(void);

	const struct sockaddr_in &getRemoteAddr();

	int copyToBuffer(UINT8 *buffer, size_t cbBuffer);

	unsigned int getRid();

private:
	Packet *m_head;
	Packet *m_tail;

	Packet *m_curr;
	UINT8 *m_offset;
	UINT8 *m_end;

	size_t m_cPackets;
	struct sockaddr_in m_remoteAddr;

	UINT16 m_nextSeq;
	size_t m_packetsLeft;
	size_t m_cbSize;
	bool m_bAsync;
	protocol::Commands m_cmd;
	unsigned int m_rid;

};
