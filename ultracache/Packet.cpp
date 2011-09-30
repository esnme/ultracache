#include "Packet.h"
#include "JAllocator.h"

JAllocator<Packet, 512, true> s_alloc;

//FIXME: Packet shouldn't be agnostic of header size, it makes no sense

void *Packet::operator new (size_t _size)
{
	return (void *) s_alloc.Alloc ();
}

void Packet::operator delete (void *_p)
{
	s_alloc.Free ( (Packet *) _p);
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
