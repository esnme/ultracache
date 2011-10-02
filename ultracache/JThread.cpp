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
#ifdef _WIN32
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) proc, arg, 0, NULL);
#else
	HANDLE hThread;
	pthread_create(&m_handle, NULL, proc, arg);
#endif

	JThread ret(hThread);
	return ret;
}

void JThread::join()
{
#ifdef _WIN32
	WaitForSingleObject(m_handle, INFINITE);
#else
	pthread_join(&m_handle, NULL),
#endif
}