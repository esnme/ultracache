#pragma once

#include <Windows.h>

class JThread
{
public:
	typedef void *(*THREADPROC)(void *arg);

private:
	JThread (HANDLE _handle);

public:
	JThread ();
	~JThread (void);

	static JThread createThread(THREADPROC proc, void *arg);
	void join();

private:
	HANDLE m_handle;


};