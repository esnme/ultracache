#pragma once

class Spinlock
{
public:
	Spinlock();
	void enter(void);
	bool tryEnter(void);
	void leave(void);
private:
	volatile long m_s;
	
	static long compareExchange(volatile long *ptr, long exchange, long comparand);
	static long exchange(volatile long *ptr, long exchange);
	static void yield(void);

	enum LOCKSTATE
	{
		LOCK_IS_FREE = 0, 
		LOCK_IS_TAKEN = 1
	};
};
