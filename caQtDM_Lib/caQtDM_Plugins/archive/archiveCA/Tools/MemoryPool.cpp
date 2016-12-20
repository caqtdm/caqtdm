#include <stdio.h>
#include "MsgLogger.h"
#include "MemoryPool.h"

enum { MAGIC = 0xEFFACED };

inline MemoryPoolGuard *mem2guard(void *mem)
{
	MemoryPoolGuard *guard = (MemoryPoolGuard *)
        ((char *)mem - sizeof (MemoryPoolGuard));
	LOG_ASSERT(guard->_magic == MAGIC);
	return guard;
}

inline void *guard2mem(MemoryPoolGuard *guard)
{
	return ((char *)guard) + sizeof(MemoryPoolGuard);
}

MemoryPool::MemoryPool()
{
	for (size_t i=0; i<MAX; ++i)
		_pool[i] = 0;
	_hits      = 0;
	_misses    = 0;
	_allocated = 0;
}

MemoryPool::~MemoryPool()
{
	for (size_t i=0; i<MAX; ++i)
	{
		if (_pool[i])
			free (_pool[i]);
	}

    printf("MemoryPool stats:\n");
	printf("* %u hits, %u misses\n",
           (unsigned int)_hits, (unsigned int)_misses);
	printf("* %u chunks left allocated\n", (unsigned int)_allocated);
}

void *MemoryPool::alloc (size_t bytes)
{
	++_allocated;
	// look for appropriate chunk in pool
	for (size_t i=0; i<MAX; ++i)
	{
		if (_pool[i])
		{
			if (_pool[i]->_size >= bytes)
			{
				// found, remove from pool
				void *mem = guard2mem (_pool[i]);
				_pool[i] = 0;
				++_hits;
				return mem;
			}
		}
	}
	// get new memory from OS
	bytes += bytes % GRANULARITY;
	MemoryPoolGuard *guard = (MemoryPoolGuard *) malloc (sizeof (MemoryPoolGuard) + bytes);
	guard->_magic = MAGIC;
	guard->_size  = bytes;
	++_misses;

	return guard2mem (guard);
}

void MemoryPool::release (void *mem)
{
	LOG_ASSERT (_allocated > 0);
	--_allocated;
	MemoryPoolGuard *guard = mem2guard (mem);

	// try to keep in pool
	for (size_t i=0; i<MAX; ++i)
	{
		if (_pool[i] == 0)
		{
			_pool[i] = guard;
			return;
		}
	}

	LOG_MSG("MemoryPool::free gives %d bytes\n", guard->_size);
	// no place in pool: give back to OS
	free(guard);
}

