#include "Request.h"
#include "JAllocator.h"
#include <assert.h>
#include <stdio.h>

JAllocator<Request, 512, true> s_alloc;

void *Request::operator new (size_t _size)
{
	return (void *) s_alloc.Alloc ();
}

void Request::operator delete (void *_p)
{
	s_alloc.Free ( (Request *) _p);
}
