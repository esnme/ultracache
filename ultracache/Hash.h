#pragma once 

#include "Heap.h"
#include <string>

#ifdef _WIN32
#include <BaseTsd.h>
#else
#error "Type definitions are needed here"
#endif

class Hash
{
public:

	typedef size_t HASHCODE;

	

	struct HashItem
	{
		UINT64 keyLength:8;
		UINT64 valueLength:20;
		UINT64 reserved:4;
		UINT64 next:32;

		void setup(size_t cbSize, void *key, size_t cbkey, void *value, size_t cbvalue, HASHCODE _hash);

		void *getKeyPtr();
		void *getValuePtr();
		size_t getKeyLen();
		size_t getValueLen();

		bool compareKey(void *key, size_t cbKey, HASHCODE hash);

		std::string getValueStr();
		std::string getKeyStr();

	};

public:
	Hash (size_t binSize);
	~Hash (void);

	HashItem *get(void *key, size_t cbKey);

	bool put(void *key, size_t cbKey, void *value, size_t cbValue, HashItem **previous);

	HashItem *get(const std::string &key);
	bool put(const std::string &key, const std::string &value, HashItem **previous);

	HashItem *remove(const std::string &key);
	HashItem *remove(void *key, size_t cbKey);

	void free(HashItem *item);

	static size_t compressPtr(HashItem *ptr);
	static HashItem *decompressPtr(size_t value);

private:
	size_t m_binSize;
	HashItem **m_bin;




};