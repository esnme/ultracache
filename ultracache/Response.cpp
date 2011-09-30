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

Response::Response(protocol::Commands cmd, const struct sockaddr_in &remoteAddr, unsigned int _rid) 
	: PacketWriter(cmd, remoteAddr, _rid, false)
{
}

