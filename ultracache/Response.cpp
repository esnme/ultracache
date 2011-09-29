#include "Response.h"
#include <assert.h>
#include "JAllocator.h"
#include <stdlib.h>

JAllocator<Response, 512, true> s_alloc;

void *Response::operator new (size_t _size)
{
	return (void *) s_alloc.Alloc ();
}

void Response::operator delete (void *_p)
{
	s_alloc.Free ( (Response *) _p);
}

Response::Response(protocol::Commands cmd, Request *request)
{
	m_seq = rand ();
	m_cmd = cmd;
	m_rid = ((protocol::Header *) request->getPackets()->getHeader())->rid;
	
	m_offset = m_end = NULL;

	preparePacket();
}



Response::~Response(void)
{
	assert(false);
}


void Response::ensureSmallSpace(size_t sz)
{
	if (m_end < m_offset >= sz)
	{
		return;
	}

	preparePacket();
}


void Response::preparePacket()
{
	//FIXME: Must update existing packet's size here

	assert (m_offset == m_end);

	Packet *packet = new Packet();
	packet->setupBuffer(CONFIG_SIZEOF_PACKET_HEADER, CONFIG_SIZEOF_PACKET_HEADER);
	
	protocol::Header *header = (protocol::Header *) packet->getHeader();

	header->async = 0;
	header->cmd = m_cmd;
	header->first = 0;
	header->last = 0;
	header->packets = 0;
	header->rid = m_rid;
	header->seq = m_seq;

	m_offset = m_head->getPayload();
	m_end = m_offset + m_head->getBufferSize();
	
	packet->next = m_head;
	m_head = packet->next;
}


void Response::write(UINT8 *data, size_t cbData)
{
	size_t bytesLeft = cbData;
	
	while (bytesLeft)
	{
		size_t bytesToCopy = bytesLeft;

		if (m_end - m_offset == 0)
		{
			preparePacket();
		}

		if (bytesToCopy > (m_end - m_offset))
		{
			bytesToCopy = m_end - m_offset;
		}
		
		memcpy(m_offset, data, bytesToCopy);

		data += bytesToCopy;
		bytesLeft -= bytesToCopy;
	}
}

void Response::write(UINT8 value)
{
	ensureSmallSpace(1);

	*((UINT8 *) m_offset) = value;
	m_offset += 1;
}

void Response::write(UINT32 value)
{
	ensureSmallSpace(4);

	*((UINT32 *) m_offset) = value;
	m_offset += 4;
}

void Response::write(UINT64 value)
{
	ensureSmallSpace(8);

	*((UINT64 *) m_offset) = value;
	m_offset += 8;
}
	

void Response::send(SOCKET sockfd)
{
}

