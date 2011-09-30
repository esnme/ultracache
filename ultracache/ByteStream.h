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
	ByteStream(void);

	UINT8 *read(size_t cbBytes);
	UINT8 readUINT8();
	UINT32 readUINT32();
	UINT64 readUINT64();

	void write(void *data, size_t cbData);
	void writeUINT8(UINT8 value);
	void writeUINT32(UINT32 value);
	void writeUINT64(UINT64 value);

private:
	UINT8 *m_start;
	UINT8 *m_offset;
	UINT8 *m_end;

};