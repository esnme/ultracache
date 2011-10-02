#ifndef __JQUEUE_H__
#define __JQUEUE_H__

#include <string.h>
#include <stdio.h>
#include <list>

template <typename _Ty> class JQueue
{
	volatile unsigned int m_iReadCursor;
	volatile unsigned int m_iWriteCursor;
	volatile unsigned int m_cItems;

	_Ty *m_pQueueList;
	unsigned int m_endIndex;
	unsigned int m_startIndex;

	unsigned int m_cQueueLength;

public:

	bool PushEnd (const _Ty& _inItem)
	{
		if (m_cItems == m_cQueueLength)
		{
			return false;
		}

		m_pQueueList[m_iWriteCursor % m_cQueueLength] = _inItem;
		m_iWriteCursor ++;
		m_cItems ++;
		return true;
	}

	_Ty &Begin (void)
	{
		return m_pQueueList[m_iReadCursor % m_cQueueLength];
	}

	_Ty &End (void)
	{
		return m_pQueueList[ (m_iWriteCursor - 1) % m_cQueueLength];
	}


	bool PushBegin (const _Ty& _inItem)
	{
		if (m_cItems == m_cQueueLength)
		{
			return false;
		}

		//FIXME: Check so that we are not coliding with writeCursor
		m_iReadCursor --;
		//memcpy ( (void *) &m_pQueueList[m_iReadCursor % m_cQueueLength], (void *) &_inItem, sizeof (_Ty));
		m_pQueueList[m_iReadCursor % m_cQueueLength] = _inItem;
		m_cItems ++;
		return true;
	}

	bool PopBegin (_Ty& _outItem)
	{
		if (m_cItems == 0)
		{
			return false;
		}


		_outItem = m_pQueueList[m_iReadCursor % m_cQueueLength];
		m_pQueueList[m_iReadCursor % m_cQueueLength] = NULL;

		m_iReadCursor ++;
		m_cItems --;

		return true;
	}


	bool PopEnd (_Ty& _outItem)
	{
		if (m_cItems == 0)
		{
			return false;
		}

		m_iWriteCursor --;
		_outItem = m_pQueueList[m_iWriteCursor % m_cQueueLength];
		m_pQueueList[m_iWriteCursor % m_cQueueLength] = NULL;

		m_cItems --;
		return true;
	}

	int GetItemCount (void)
	{
		return (int) m_cItems;
	}

	JQueue (int _cLength)
	{
		m_pQueueList = new _Ty[_cLength];
		m_cQueueLength = _cLength;
		m_iReadCursor = 0;
		m_iWriteCursor = 0;
		m_cItems = 0;
		
		m_endIndex = 0;
		m_startIndex = 0;

	}

	~JQueue (void)
	{
		if (m_pQueueList) delete m_pQueueList;
	}


};

#endif
