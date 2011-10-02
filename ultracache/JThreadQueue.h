#ifndef __JTHREADQUEUE_H__
#define __JTHREADQUEUE_H__

#include <assert.h>

#include "JQueue.h"
#include "JCondition.h"

template <typename _Ty>
class JThreadQueue
{
private:
	JCondition m_cond;
	bool m_bStopped;


	JQueue<_Ty> m_queue;

public:
	JThreadQueue (int _cMaxItems) :
			m_queue (_cMaxItems)
	{
		m_bStopped = false;

	}

	void LockQueue (void)
	{
		m_cond.WaitAndLock ();
	}

	void UnlockQueue (void)
	{
		m_cond.Unlock ();
	}

	void StopQueue (void)
	{
		m_cond.WaitAndLock ();
		m_bStopped = true;
	}

	void StartQueue (void)
	{
		m_cond.Signal ();
		m_cond.Unlock ();
		m_bStopped = false;
	}

	/*
	LOCK MUST BE AQUIRED BEFORE THIS! */
	JQueue<_Ty> &GetQueue (void)
	{
		return m_queue;
	}

	void WaitForItem (_Ty &_outItem, bool _bLock = true)
	{
		bool bResult;

		RETRY_WAIT:

		if (_bLock)
		{
			m_cond.WaitAndLock ();
		}

		bResult = m_queue.PopEnd (_outItem);

		if (!bResult)
		{
			m_cond.WaitForCondition ();
			bResult = m_queue.PopEnd (_outItem);
		}

		if (!bResult)
		{
			m_cond.Unlock ();
			goto RETRY_WAIT;
		}

		assert (bResult);
		m_cond.Unlock ();

	}

	bool PollForItem (_Ty &_outItem)
	{
		bool bResult;

		m_cond.WaitAndLock ();

		bResult = m_queue.PopEnd (_outItem);

		m_cond.Unlock ();

		return bResult;
	}

	void PostItem (_Ty _outItem)
	{
		bool bResult; 

		m_cond.WaitAndLock ();
		bResult = m_queue.PushBegin (_outItem);

		if (!m_bStopped)
		{
			m_cond.Signal ();
		}
		m_cond.Unlock ();

		assert (bResult);
	}

	int GetItemCount (void)
	{
		return m_queue.GetItemCount ();
	}

	int GetMaxSize (void)
	{
		return m_queue.GetMaxSize ();
	}
};

#endif
