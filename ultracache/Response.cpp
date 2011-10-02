#include "Response.h"
#include <assert.h>
#include "JAllocator.h"
#include <stdlib.h>
#include "Spinlock.h"

JAllocator<Response, 512, true> s_alloc;

static Spinlock s_respSL;

void *Response::operator new (size_t _size)
{
	s_respSL.enter();
	void *ret = (void *) s_alloc.Alloc ();
	s_respSL.leave();

	return ret;
}

void Response::operator delete (void *_p)
{
	s_respSL.enter();
	s_alloc.Free ( (Response *) _p);
	s_respSL.leave();
}

Response::Response(protocol::Commands cmd, const struct sockaddr_in &remoteAddr, unsigned int _rid) 
	: PacketWriter(cmd, remoteAddr, _rid, false)
{
}

