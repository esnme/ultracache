#pragma once
#include "socketdefs.h"
#include "Packet.h"
#include "Protocol.h"
#include "Request.h"
#include "PacketWriter.h"

class Response : public PacketWriter
{
public:
	Response(protocol::Commands cmd, Request *request);


public:
	static void *operator new (size_t _size);
	static void operator delete (void *_p);

private:
	void ensureSmallSpace(size_t sz);
};