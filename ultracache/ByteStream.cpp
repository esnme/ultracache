#include "ByteStream.h"
#include <stdio.h>
#include <string.h>

ByteStream::ByteStream(void *buffer, size_t cbBuffer)
{
	m_offset = (UINT8 *) buffer;
	m_end = m_offset + cbBuffer;
	m_start = m_offset;
}

ByteStream::ByteStream(void)
{
	m_offset = (UINT8 *) 0;
	m_end = m_offset + 0;
	m_start = m_offset;
}

UINT8 *ByteStream::read(size_t cbBytes)
{
	if ((m_end - m_offset) < cbBytes)
	{
		throw Exception();
	}

	UINT8 *ret = m_offset;
	m_offset += cbBytes;

	return ret;
}

UINT8 ByteStream::readUINT8()
{
	if ((m_end - m_offset) < 1)
	{
		throw Exception();
	}

	UINT8 ret = *((UINT8 *) m_offset);
	m_offset += 1;
	return ret;
}

UINT32 ByteStream::readUINT32()
{
	if ((m_end - m_offset) < 4)
	{
		throw Exception();
	}

	UINT32 ret = *((UINT32 *) m_offset);
	m_offset += 4;
	return ret;
}

UINT64 ByteStream::readUINT64()
{
	if ((m_end - m_offset) < 8)
	{
		throw Exception();
	}

	UINT64 ret = *((UINT64 *) m_offset);
	m_offset += 8;
	return ret;
}

void ByteStream::write(void *data, size_t cbData)
{
	if ((m_end - m_offset) < cbData)
	{
		throw Exception();
	}

	memcpy (m_offset, data, cbData);
	m_offset += cbData;
}

void ByteStream::writeUINT8(UINT8 value)
{
	if ((m_end - m_offset) < 1)
	{
		throw Exception();
	}

	*((UINT8 *) m_offset) = value;
	m_offset += 1;
}

void ByteStream::writeUINT32(UINT32 value)
{
	if ((m_end - m_offset) < 4)
	{
		throw Exception();
	}

	*((UINT32 *) m_offset) = value;
	m_offset += 4;
}

void ByteStream::writeUINT64(UINT64 value)
{
	if ((m_end - m_offset) < 8)
	{
		throw Exception();
	}

	*((UINT64 *) m_offset) = value;
	m_offset += 8;
}

