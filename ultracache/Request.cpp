#include "Request.h"
#include "JAllocator.h"
#include <assert.h>
#include <stdio.h>
#include "Spinlock.h"

static JAllocator<Request, 512, true> s_alloc;

static Spinlock s_requestSL;

void *Request::operator new (size_t _size)
{
	s_requestSL.enter();
	void *ret = (void *) s_alloc.Alloc ();
	s_requestSL.leave();

	return ret;
}

void Request::operator delete (void *_p)
{
	s_requestSL.enter();
	s_alloc.Free ( (Request *) _p);
	s_requestSL.leave();
}
