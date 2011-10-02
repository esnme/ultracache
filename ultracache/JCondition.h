#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#error "Not portable yet"
#endif


class JCondition
{
private:

	CRITICAL_SECTION m_mutex;
	CONDITION_VARIABLE m_condition;

public:

  JCondition ()
  {
		InitializeConditionVariable(&m_condition);
		InitializeCriticalSection(&m_mutex);
  }

  ~JCondition (void)
  {
		//FIXME: No delete for conditions?
		DeleteCriticalSection(&m_mutex);
  }

	void Signal (void)
  {
		WakeConditionVariable(&m_condition);
  }

	void Broadcast (void)
  {
		WakeAllConditionVariable(&m_condition);
  }

  void WaitForCondition (void)
  {
		SleepConditionVariableCS(&m_condition, &m_mutex, INFINITE);
  }

	void WaitAndLock (void)
	{
		EnterCriticalSection(&m_mutex);
	}

	void Unlock (void)
	{
		LeaveCriticalSection(&m_mutex);
	}
};
