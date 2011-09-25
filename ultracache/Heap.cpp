#include "Heap.h"
#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#error "Need virtual memory functions here"
#endif


Heap::Heap(size_t _cbSize)
{
	m_pBase = VirtualAlloc(NULL, _cbSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	m_pStart = (UINT8 *) m_pBase;
	m_pEnd = m_pStart + _cbSize;
	m_pOffset = m_pStart;

	m_cbAllocSize = 0;

	memset (m_free, 0, sizeof (m_free));
}

Heap::~Heap(void)
{
	VirtualFree(m_pBase, 0, MEM_RELEASE); 
}

size_t logh(size_t value)
{
	size_t ret = -1;

	while (value != 0) 
	{
		value >>= 1;
		ret++;
	}
	return ret;
}

Heap::HeapEntry *Heap::allocFromBase(size_t _cbSize)
{
	size_t allocSize = _cbSize + sizeof(HeapEntry);

	HeapEntry *entry;

	if (m_pOffset + allocSize >= m_pEnd)
	{
		return NULL;

	}

	entry = (HeapEntry *) m_pOffset;
	//FIXME: Perhaps align here?
	m_pOffset += allocSize;
	entry->cbSize = _cbSize;

	return entry;
}

void Heap::unlinkEntry(size_t index, HeapEntry *entry, HeapEntry *prevEntry)
{
}


Heap::HeapEntry *Heap::allocFromFree(size_t _cbSize)
{
	size_t lg = logh(_cbSize);
	size_t div = ((1 << lg) / HEAP_ALLOC_SLOTS);
	size_t slot = 0;
	if (div == 0)
	{
		slot = 0;
	}
	else
	{
		slot = (_cbSize - (1 << lg)) / div;
	}



	for (; lg < HEAP_ALLOC_BITS; lg ++)
	{

		for (; slot < HEAP_ALLOC_SLOTS; slot ++)
		{
			HeapEntry *entry  = m_free[lg][slot];
			HeapEntry *prev = NULL;
			while (entry)
			{
				if (entry->cbSize >= _cbSize)
				{
					if (prev)
					{
						prev->next = entry->next;
					}
					else
					{
						m_free[lg][slot] = decompressPtr(entry->next);
					}

					assert (entry->cbSize >= _cbSize);
					return entry;
				}
				prev = entry;
				entry = decompressPtr(entry->next);
			}
		}
		slot = 0;
	}

	return NULL;
}

size_t Heap::align(size_t base, size_t value)
{
	//ptr += (ALIGN-(reinterpret_cast<ptrdiff_t>(ptr)&(ALIGN-1)))&(ALIGN-1);
	return value += (base-(value & (base-1))) & (base - 1);
}


void *Heap::alloc(size_t _cbSize)
{
	HeapEntry *entry;

	_cbSize = align(8, _cbSize);

	entry = allocFromBase(_cbSize);

	if (entry == NULL)
	{
		entry = allocFromFree(_cbSize);

		if (entry == NULL)
		{
			//FIXME: What do we do here?
			return NULL;
		}
 }

	this->m_cbAllocSize += _cbSize;
	return (void *) (entry+1);
}


void Heap::debugPrintFree(size_t index)
{
	//getchar();

}


void Heap::free(void *ptr)
{
	HeapEntry *freeEntry = (((HeapEntry *) ptr) - 1);

	size_t index = logh(freeEntry->cbSize);
	assert (index < HEAP_ALLOC_BITS);

	size_t div = (1 << index) / HEAP_ALLOC_SLOTS;
	size_t slot = 0;
	if (div == 0)
	{
		slot = 0;
	}
	else
	{
		slot = (freeEntry->cbSize - (1 << index)) / div;
	}

	freeEntry->next = compressPtr(m_free[index][slot]);
	m_free[index][slot] = freeEntry;

	this->m_cbAllocSize -= freeEntry->cbSize;
}

size_t Heap::getAllocSize()
{
	return m_cbAllocSize;
}

Heap::HeapEntry *Heap::allocByCompact(size_t _cbSize)
{
	return NULL;
}

size_t Heap::compressPtr(HeapEntry *ptr)
{
	return (((size_t)(ptr)) >> 3);
}

Heap::HeapEntry *Heap::decompressPtr(size_t value)
{
	return ((Heap::HeapEntry*)(((size_t)(value)) << 3));
}
