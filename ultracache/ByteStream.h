#pragma once

#include "types.h"


class ByteStream
{
public:
	class Exception
	{
	};

public:
	ByteStream(void *buffer, size_t cbBuffer);

	UINT8 *read(size_t cbBytes);
	UINT8 readUINT8();
	UINT32 readUINT32();
	UINT64 readUINT64();

private:
	UINT8 *m_offset;
	UINT8 *m_end;

};