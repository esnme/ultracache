#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif


class JCondition
{
private:

#ifdef _WIN32
	CRITICAL_SECTION m_mutex;
	CONDITION_VARIABLE m_condition;
#else
  pthread_mutex_t m_mutex;
  pthread_cond_t m_condition;
#endif
public:

  JCondition ()
  {
#ifdef _WIN32
		InitializeConditionVariable(&m_condition);
		InitializeCriticalSection(&m_mutex);
#else
		pthread_mutex_init (&m_mutex, NULL);
		pthread_cond_init(&m_condition, NULL);
#endif
  }

  ~JCondition (void)
  {
#ifdef _WIN32
		//FIXME: No delete for conditions?
		DeleteCriticalSection(&m_mutex);
#else
		pthread_mutex_destroy(&m_mutex);
		pthread_cond_destroy (&m_condition);
#endif
  }

	void Signal (void)
  {
#ifdef _WIN32
		WakeConditionVariable(&m_condition);
#else
    pthread_cond_signal (&m_condition);
#endif
  }

	void Broadcast (void)
  {
#ifdef _WIN32
		WakeAllConditionVariable(&m_condition);
#else
    pthread_cond_broadcast (&m_condition);
#endif
  }

  void WaitForCondition (void)
  {
#ifdef _WIN32
		SleepConditionVariableCS(&m_condition, &m_mutex, INFINITE);
#else
    pthread_cond_wait (&m_condition, &m_mutex);
#endif
	}

	void WaitAndLock (void)
	{
#ifdef _WIN32
		EnterCriticalSection(&m_mutex);
#else
    pthread_mutex_lock (&m_mutex);
#endif
	}

	void Unlock (void)
	{
#ifdef _WIN32
		LeaveCriticalSection(&m_mutex);
#else
    pthread_mutex_unlock (&m_mutex);
#endif

	}
};
