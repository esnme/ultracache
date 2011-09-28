#include "Packet.h"
#include "JAllocator.h"

JAllocator<Packet, 512, true> s_alloc;

void *Packet::operator new (size_t _size)
{
	return (void *) s_alloc.Alloc ();
}

void Packet::operator delete (void *_p)
{
	s_alloc.Free ( (Packet *) _p);
}

UINT8 *Packet::getHeader()
{
	return m_buffer;
}

UINT8 *Packet::getPayload()
{
	return m_payload;
}

size_t Packet::getBufferSize()
{
	return sizeof(m_buffer);
}

void Packet::setupBuffer(size_t cbHeader, size_t cbTotal)
{
	m_cbHeader = cbHeader;
	m_payload = m_buffer + cbHeader;

}

size_t Packet::getPayloadSize()
{
	return getBufferSize() - m_cbHeader;
}