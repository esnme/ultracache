#include "Spinlock.h"

#ifdef _WIN32
#include <Windows.h>
#include <intrin.h>

#pragma intrinsic(_InterlockedCompareExchange, _InterlockedExchange)

#endif

#define SPINS_COUNT 4096

Spinlock::Spinlock()
{
	m_s = LOCK_IS_FREE;
}

void Spinlock::enter(void)
{
	int spins = 0;

	while (compareExchange(&m_s, LOCK_IS_TAKEN, LOCK_IS_FREE) == LOCK_IS_TAKEN)
	{
		spins ++;

		if (SPINS_COUNT >= SPINS_COUNT && (spins % SPINS_COUNT) == 0)
		{
			yield();
		}
			
		// Spin or yield
	}

}

bool Spinlock::tryEnter(void)
{
	if (compareExchange(&m_s, LOCK_IS_TAKEN, LOCK_IS_FREE) == LOCK_IS_FREE)
	{
		return true;
	}

	return false;
}

void Spinlock::leave(void)
{
	exchange(&m_s, LOCK_IS_FREE);
}

long Spinlock::compareExchange(volatile long *ptr, long exchange, long comparand)
{
	return _InterlockedCompareExchange(ptr, exchange, comparand);
}

long Spinlock::exchange(volatile long *ptr, long exchange)
{
	return _InterlockedExchange(ptr, exchange);
}

void Spinlock::yield()
{
	SwitchToThread();
}