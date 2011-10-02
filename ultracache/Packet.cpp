#include "Packet.h"
#include "JAllocator.h"
#include "Spinlock.h"

JAllocator<Packet, 512, true> s_alloc;

//FIXME: Packet shouldn't be agnostic of header size, it makes no sense


Spinlock packetSL;


void *Packet::operator new (size_t _size)
{

	packetSL.enter();
	void *ret = s_alloc.Alloc ();
	packetSL.leave();
	return ret;
}

void Packet::operator delete (void *_p)
{
	packetSL.enter();
	s_alloc.Free ( (Packet *) _p);
	packetSL.leave();
}

protocol::Header *Packet::getHeader()
{
	return (protocol::Header *) m_buffer;
}

UINT8 *Packet::getPayload()
{
	return (UINT8 *)(((protocol::Header *) m_buffer) + 1);
}

size_t Packet::getBufferSize()
{
	return sizeof(m_buffer);
}

void Packet::setupBuffer(size_t cbTotal)
{
	m_cbTotal = cbTotal;
}

size_t Packet::getPayloadSize()
{
	return m_cbTotal - sizeof(protocol::Header);
}

size_t Packet::getTotalSize()
{
	return m_cbTotal;
}

struct sockaddr_in *Packet::getRemoteAddr()
{
	return &m_remoteAddr;
}

