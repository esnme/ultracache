#pragma once
#include "Packet.h"
#include "Protocol.h"
#include "socketdefs.h"

class Request
{

public:
	enum Result
	{
		NEXTPACKET,
		FAILED,
		COMPLETE
	};
	
public:
	Request();
	~Request();
	Result put(struct sockaddr_in *remoteAddr, Packet *packet);
		
	static void *operator new (size_t _size);
	static void operator delete (void *_p);

	Packet *getPackets();
	protocol::Commands getCommand();
	bool isAsync(void);

private:
	Packet *m_head;
	Packet *m_tail;

	size_t m_cPackets;
	struct sockaddr_in m_remoteAddr;

	UINT16 m_nextSeq;
	size_t m_packetsLeft;
	size_t m_cbSize;
	bool m_bAsync;
	protocol::Commands m_cmd;

};
