#include "Request.h"
#include "JAllocator.h"
#include <assert.h>
#include <stdio.h>

JAllocator<Request, 512, true> s_alloc;

void *Request::operator new (size_t _size)
{
	return (void *) s_alloc.Alloc ();
}

void Request::operator delete (void *_p)
{
	s_alloc.Free ( (Request *) _p);
}


Request::Request()
{
	this->m_cPackets = 0;
	this->m_head = NULL;
	this->m_tail = NULL;
	m_cbSize = 0;
}

Request::~Request()
{
	Packet *packet = m_head;

	while (packet)
	{
		Packet *free = packet;
		delete free;
		packet = packet->next;
	}
}

Request::Result Request::put(struct sockaddr_in *remoteAddr, Packet *packet)
{
	protocol::Header *header = (protocol::Header *) packet->getHeader();

	if (m_cPackets == 0)
	{
		m_remoteAddr = *remoteAddr;
		m_cPackets ++;

		if (!header->first)
		{
			return Request::FAILED;
		}

		// Fall through for packets being 1 packets large
		m_nextSeq = header->seq;
		m_nextSeq ++;
		m_packetsLeft = header->packets;
		m_packetsLeft --;

		m_bAsync = header->async;
		m_cmd = (protocol::Commands) header->cmd;
	}
	else
	{
		// Validate sequence;
		if (m_nextSeq != header->seq)
		{
			return Request::FAILED;
		}

		m_nextSeq ++;
		m_packetsLeft --;
	}

	if (!m_tail)
	{
		m_tail = packet;
		m_head = packet;
	}
	else
	{
		m_tail->next = packet;
		m_tail = packet;
	}

	m_cbSize += packet->getPayloadSize();

	if (m_cbSize > CONFIG_MAX_REQUEST_SIZE)
	{
		return Request::FAILED;
	}


	if (header->last)
	{
		if (m_packetsLeft != 0)
		{
			return Request::FAILED;
		}

		return Request::COMPLETE;
	}
	else
	{
		if (m_packetsLeft == 0)
		{
			return Request::FAILED;
		}
	}

	return Request::NEXTPACKET;
}

Packet *Request::getPackets()
{
	return m_head;
}

protocol::Commands Request::getCommand()
{
	return m_cmd;
}

bool Request::isAsync(void)
{
	return m_bAsync;
}
