#pragma once 

#include "Heap.h"
#include "types.h"


class Hash
{
public:

	typedef size_t HASHCODE;
	

	struct HashItem
	{
		UINT64 cbKeyLength:8;
		UINT64 cbValueLength:20;
		UINT64 reserved:10;
		UINT64 next:32;

		void setup(size_t cbSize, void *key, size_t cbKey, void *value, size_t cbValue);

		UINT64 *getKeyPtr();
		UINT64 *getValuePtr();
		size_t getKeyLen();
		size_t getValueLen();

		bool compareKey(UINT64 *key, size_t cbKey);
		HashItem *appendValue(void *value, size_t cbValue);
	};

public:
	Hash (size_t binSize);
	~Hash (void);

	HashItem *get(UINT64 *key, size_t cbKey);
	
	bool set(HashItem *newItem, HashItem **previousItem);

	HashItem *remove(UINT64 *key, size_t cbKey);
	
	static size_t compressPtr(HashItem *ptr);
	static HashItem *decompressPtr(size_t value);

private:
	size_t m_binSize;
	UINT32 *m_bin;




};