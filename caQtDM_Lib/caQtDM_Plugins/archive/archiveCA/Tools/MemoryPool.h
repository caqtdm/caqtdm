#ifndef __MEMORYPOOL_H__
#define __MEMORYPOOL_H__

#include <ToolsConfig.h>
#include <stdlib.h>

typedef struct
{
public:
	int		_magic;
	size_t	_size;
}	MemoryPoolGuard;

class MemoryPool
{
public:
	MemoryPool ();
	~MemoryPool ();

	void *alloc (size_t bytes);
	void release (void *mem);

private:
	enum { MAX = 100, GRANULARITY = 10 };
	MemoryPoolGuard *_pool[MAX];
	size_t _hits;
	size_t _misses;
	size_t _allocated;
};

#endif //__MEMORYPOOL_H__
