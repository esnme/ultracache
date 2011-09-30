#pragma once
#include "PacketReader.h"

class Request : public PacketReader
{
public:
	static void *operator new (size_t _size);
	static void operator delete (void *_p);

};
