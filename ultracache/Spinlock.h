#pragma once

#ifdef _WIN32
#else
#include <pthread.h>
#endif

class Spinlock
{
public:
	Spinlock();
	~Spinlock();

	void enter(void);
	bool tryEnter(void);
	void leave(void);
private:

#ifdef _WIN32
	volatile long m_s;
	
	static long compareExchange(volatile long *ptr, long exchange, long comparand);
	static long exchange(volatile long *ptr, long exchange);
	enum LOCKSTATE
	{
		LOCK_IS_FREE = 0, 
		LOCK_IS_TAKEN = 1
	};
#else
	pthread_spinlock_t m_s;
#endif

	static void yield(void);

};
