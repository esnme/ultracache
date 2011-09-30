#pragma once
#include "socketdefs.h"
#include "Packet.h"
#include "Protocol.h"
#include "Request.h"
#include "PacketWriter.h"
#include "socketdefs.h"

class Response : public PacketWriter
{
public:
	Response(protocol::Commands cmd, const struct sockaddr_in &remoteAddr, unsigned int _rid);


public:
	static void *operator new (size_t _size);
	static void operator delete (void *_p);

private:
	void ensureSmallSpace(size_t sz);
};