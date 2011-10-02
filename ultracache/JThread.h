#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

class JThread
{
public:
	typedef void *(*THREADPROC)(void *arg);

private:

#ifdef _WIN32
	JThread (HANDLE _handle);
#else
	JThread (pthread_t _handle);
#endif


public:
	JThread ();
	~JThread (void);

	static JThread createThread(THREADPROC proc, void *arg);
	void join();

private:
#ifdef _WIN32
	HANDLE m_handle;
#else
	pthread_t m_handle;
#endif

};