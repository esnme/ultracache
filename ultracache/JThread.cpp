#include "JThread.h"

JThread::JThread (HANDLE _handle)
{
	m_handle = _handle;
}

JThread::JThread ()
{
	m_handle = NULL;
}


JThread::~JThread (void)
{
}

JThread JThread::createThread(THREADPROC proc, void *arg)
{
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) proc, arg, 0, NULL);
	JThread ret(hThread);
	return ret;
}

void JThread::join()
{
	WaitForSingleObject(m_handle, INFINITE);
}