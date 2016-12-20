// -*- c++ -*-
#if !defined(_HASHTABLE_H_)
#define _HASHTABLE_H_

class HashTable  
{
public:
	enum
	{
		HashTableSize = 256
	};

	typedef unsigned short HashValue;
	static HashValue Hash (const char *string);
};

#endif
