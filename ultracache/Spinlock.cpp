#include "Spinlock.h"

#ifdef _WIN32
#include <Windows.h>
#include <intrin.h>

#pragma intrinsic(_InterlockedCompareExchange, _InterlockedExchange)

#else
#include <pthread.h>
#include <sched.h>
#endif

#define SPINS_COUNT 4096

Spinlock::Spinlock()
{
#ifdef _WIN32
	m_s = LOCK_IS_FREE;
#else
	pthread_spin_init(&m_s, PTHREAD_PROCESS_PRIVATE);
#endif
}

Spinlock::~Spinlock()
{
#ifdef _WIN32
#else
	pthread_spin_destroy(&m_s);
#endif
}

void Spinlock::enter(void)
{
#ifdef _WIN32
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
#else
	pthread_spin_lock(&m_s);
#endif
}

bool Spinlock::tryEnter(void)
{
#ifdef _WIN32
	if (compareExchange(&m_s, LOCK_IS_TAKEN, LOCK_IS_FREE) == LOCK_IS_FREE)
	{
		return true;
	}

	return false;
#else
	pthread_spin_trylock(&m_s);
#endif
}

void Spinlock::leave(void)
{
#ifdef _WIN32
	exchange(&m_s, LOCK_IS_FREE);
#else
	pthread_spin_unlock(&m_s);
#endif
}

#ifdef _WIN32
long Spinlock::compareExchange(volatile long *ptr, long exchange, long comparand)
{
	return _InterlockedCompareExchange(ptr, exchange, comparand);
}

long Spinlock::exchange(volatile long *ptr, long exchange)
{
	return _InterlockedExchange(ptr, exchange);
}
#endif
void Spinlock::yield()
{
#ifdef _WIN32
	SwitchToThread();
#else
	sched_yield();
#endif
}

