#include "PacketReader.h"
#include <assert.h>
#include <stdio.h>

PacketReader::PacketReader()
{
	this->m_cPackets = 0;
	this->m_head = NULL;
	this->m_tail = NULL;
	m_cbSize = 0;
}

PacketReader::~PacketReader()
{
	Packet *packet = m_head;

	while (packet)
	{
		Packet *free = packet;
		delete free;
		packet = packet->next;
	}
}

PacketReader::Result PacketReader::put(struct sockaddr_in *remoteAddr, Packet *packet)
{
	protocol::Header *header = (protocol::Header *) packet->getHeader();

	if (m_cPackets == 0)
	{
		m_remoteAddr = *remoteAddr;
		m_cPackets ++;

		if (!header->first)
		{
			return PacketReader::FAILED;
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
			return PacketReader::FAILED;
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
		return PacketReader::FAILED;
	}


	if (header->last)
	{
		if (m_packetsLeft != 0)
		{
			return PacketReader::FAILED;
		}

		return PacketReader::COMPLETE;
	}
	else
	{
		if (m_packetsLeft == 0)
		{
			return PacketReader::FAILED;
		}
	}

	return PacketReader::NEXTPACKET;
}

Packet *PacketReader::getPackets()
{
	return m_head;
}

protocol::Commands PacketReader::getCommand()
{
	return m_cmd;
}

bool PacketReader::isAsync(void)
{
	return m_bAsync;
}


const struct sockaddr_in &PacketReader::getRemoteAddr()
{
	return m_remoteAddr;
}

int PacketReader::copyToBuffer(UINT8 *buffer, size_t cbBuffer)
{
	Packet *packet = m_head;
	size_t cbSize = 0;
	
	assert(packet);

	while (packet)
	{
		size_t cbPayload = packet->getPayloadSize();

		if (cbSize + cbPayload > cbBuffer)
		{
			return -1;
		}

		memcpy (buffer, packet->getPayload(), cbPayload);
		buffer += cbPayload;
		cbSize += cbPayload;

		Packet *free = packet;
		packet = packet->next;
		delete free;
	}

	m_head = m_tail = NULL;

	return (int) cbSize;
}
