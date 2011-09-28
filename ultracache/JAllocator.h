#pragma once
#include <assert.h>

#define JALLOCATOR_GUARD 0xaaaaaaaa 

template <typename _T, size_t _PREALLOC, bool _DETECTLEAKS>
class JAllocator
{
public:
	JAllocator (void)
	{
		m_pFreeList = (ITEM *) 0;
		m_cItems = 0;

		for (size_t index = 0; index < _PREALLOC; index ++)
		{
			(m_preAlloc + index)->pNext = m_pFreeList;
#ifdef _DEBUG
			(m_preAlloc + index)->guard = JALLOCATOR_GUARD;
#endif
			m_pFreeList = m_preAlloc + index;
			m_cItems ++;
		}
	}

	~JAllocator (void)
	{
		ITEM *ptrStart = m_preAlloc + 0;
		ITEM *ptrEnd = m_preAlloc + _PREALLOC;

		size_t cFree = 0;

		while (m_pFreeList)
		{
#ifdef _DEBUG
			assert (m_pFreeList->guard == JALLOCATOR_GUARD);
#endif
			if (m_pFreeList < ptrStart && m_pFreeList > ptrEnd)
			{
				ITEM *pItem = m_pFreeList;
				m_pFreeList = m_pFreeList->pNext;
				delete pItem;
			}
			else
			{
				m_pFreeList = m_pFreeList->pNext;
			}

			cFree ++;
		}

		if (_DETECTLEAKS)
		{	
			assert (cFree == m_cItems);
		}
	}

	_T *Alloc (void)
	{
		ITEM *pItem;

		if (!m_pFreeList)
		{
			m_pFreeList = new ITEM;
			m_cItems ++;
			m_pFreeList->pNext = (ITEM *) 0;
#ifdef _DEBUG
			m_pFreeList->guard = JALLOCATOR_GUARD;
#endif
		}

		pItem = m_pFreeList;
		m_pFreeList = m_pFreeList->pNext;

#ifdef _DEBUG
		assert (pItem->guard == JALLOCATOR_GUARD);
#endif
		return (_T *) pItem->data;
	}

	void Free (const _T *_pItem)
	{
		ITEM *pPtr = (ITEM *) _pItem;

#ifdef _DEBUG
		assert (pPtr->guard == JALLOCATOR_GUARD);
#endif

		ITEM *pItem = m_pFreeList;
		m_pFreeList = (ITEM *) pPtr;
		m_pFreeList->pNext = pItem;
	}

private:

	struct ITEM
	{
		char data[sizeof(_T)];
#ifdef _DEBUG
		unsigned int guard;
#endif
		ITEM *pNext;
	};

	ITEM *m_pFreeList;
	ITEM m_preAlloc[_PREALLOC];

	size_t m_cItems;

};

