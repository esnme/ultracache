#pragma once

#include "types.h"

#define HEAP_ALLOC_BITS 20
#define HEAP_ALLOC_SLOTS 128
#define HEAP_ALIGNMENT 8

class Heap
{
public:
	Heap(size_t _cbSize);
	~Heap(void);

	void *alloc(size_t _cbSize);
	void free(void *ptr);
	size_t getAllocSize();
	size_t getHeapSize();


private:
	struct HeapEntry
	{
		UINT64 cbSize:32;
		UINT64 next:32;
	};

	HeapEntry *allocFromFree(size_t _cbSize);
	HeapEntry *allocFromBase(size_t _cbSize);
	HeapEntry *allocByCompact(size_t _cbSize);

public:
	static size_t align(size_t base, size_t value);
	static size_t compressPtr(HeapEntry *ptr);
	static HeapEntry *decompressPtr(size_t value);
	
	static size_t getSizeOfAlloc(void *ptr);

private:
	void *m_pBase;

	UINT8 *m_pStart;
	UINT8 *m_pOffset;
	UINT8 *m_pEnd;

	void unlinkEntry(size_t index, HeapEntry *entry, HeapEntry *prevEntry);

	size_t m_cbAllocSize;
	HeapEntry *m_free[HEAP_ALLOC_BITS][HEAP_ALLOC_SLOTS];


	void debugPrintFree(size_t index);

};