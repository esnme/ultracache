#include "ByteStream.h"
#include <stdio.h>
ByteStream::ByteStream(void *buffer, size_t cbBuffer)
{
	m_offset = (UINT8 *) buffer;
	m_end = m_offset + cbBuffer;
}

UINT8 *ByteStream::read(size_t cbBytes)
{
	if ((m_end - m_offset) < cbBytes)
	{
		return NULL;
	}

	m_offset += cbBytes;
}

UINT8 ByteStream::readUINT8()
{
	if ((m_end - m_offset) < 1)
	{
		return false;
	}

	UINT8 ret = *((UINT8 *) m_offset);
	m_offset += 1;
	return ret;
}

UINT32 ByteStream::readUINT32()
{
	if ((m_end - m_offset) < 4)
	{
		return false;
	}

	UINT32 ret = *((UINT32 *) m_offset);
	m_offset += 4;
	return ret;
}

UINT64 ByteStream::readUINT64()
{
	if ((m_end - m_offset) < 8)
	{
		return false;
	}

	UINT64 ret = *((UINT64 *) m_offset);
	m_offset += 8;
	return ret;
}
