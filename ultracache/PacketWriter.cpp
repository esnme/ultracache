#include "PacketWriter.h"
#include <assert.h>
#include <stdlib.h>


PacketWriter::PacketWriter(protocol::Commands cmd, const struct sockaddr_in &_remoteAddr, unsigned int _rid)
{
	m_seq = rand ();
	m_cmd = cmd;
	m_rid = _rid;
	m_remoteAddr = _remoteAddr;
	
	m_offset = m_end = NULL;

	preparePacket();
	((protocol::Header *)m_head->getHeader())->first = 1;

}



PacketWriter::~PacketWriter(void)
{
	assert (m_head == NULL);
	assert (m_tail == NULL);
}


void PacketWriter::ensureSmallSpace(size_t sz)
{
	if (m_end < m_offset >= sz)
	{
		return;
	}

	preparePacket();
}


void PacketWriter::preparePacket()
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
	m_seq ++;

	packet->next = NULL;

	if (m_tail == NULL)
	{
		assert (m_head == NULL);

		m_head = packet;
		m_tail = packet;
		m_tail->next = NULL;
	}
	else
	{
		m_tail->setupBuffer(CONFIG_SIZEOF_PACKET_HEADER, CONFIG_SIZEOF_PACKET_HEADER + (m_end - m_offset));
		m_tail->next = packet;
		m_tail = packet;
	}

	m_offset = packet->getPayload();
	m_end = m_offset + packet->getBufferSize();


}


void PacketWriter::write(UINT8 *data, size_t cbData)
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

void PacketWriter::write(UINT8 value)
{
	ensureSmallSpace(1);

	*((UINT8 *) m_offset) = value;
	m_offset += 1;
}

void PacketWriter::write(UINT32 value)
{
	ensureSmallSpace(4);

	*((UINT32 *) m_offset) = value;
	m_offset += 4;
}

void PacketWriter::write(UINT64 value)
{
	ensureSmallSpace(8);

	*((UINT64 *) m_offset) = value;
	m_offset += 8;
}
	

void PacketWriter::send(SOCKET sockfd)
{
	protocol::Header *tail = (protocol::Header *) m_tail->getHeader();
	protocol::Header *head = (protocol::Header *) m_head->getHeader();

	tail->last = 1;
	head->packets = m_packets;

	Packet *packet = m_head;

	while (packet)
	{
		sendto(sockfd, (char *) packet->getHeader(), packet->getPayloadSize(), MSG_NOSIGNAL, (sockaddr *) &m_remoteAddr, sizeof (struct sockaddr_in));
		Packet *free = packet;
		packet = packet->next;
		delete free;
	}


}

