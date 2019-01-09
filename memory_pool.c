#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#define align_ptr(d,a) (uint8_t*)(((uintptr_t)(d) + (uintptr_t)(a-1)) &(~((uintptr_t)a-1)))
#include "memory_pool.h"

memonry_pool_t *memonry_pool_create(size_t size)
{
	memonry_pool_t *p = NULL;
	p = (memonry_pool_t*)malloc(size + sizeof(memonry_pool_t));
	assert(p != NULL);
	p->max = size;
	p->d.failed = 0;
	p->d.last = (uint8_t *)p + sizeof(memonry_pool_t);
	p->d.end = (uint8_t*)p + size + sizeof(memonry_pool_t);
	p->d.next = NULL;
	p->large = NULL;
	p->current = p;
	return p;
}
void memonry_pool_destroy(memonry_pool_t *pool)
{
	assert(pool != NULL);	
	memonry_pool_t *p = pool;
	memonry_pool_t *n = NULL;
	memonry_pool_large_data_t *pl,*nl = NULL;
	for(pl = pool->large;pl;pl = nl,nl = nl->next)
	{
		nl = pl->next;
		free(pl->alloc);
		free(pl);
		if(nl == NULL)
			break;
	}

	for(p = pool,n = pool->d.next;;p = n,n = n->d.next)
	{
		free(p);
		if(n == NULL)
			break;
	}
	
	return;
}
void *memonry_pool_alloc_block(memonry_pool_t *pool,size_t size)
{
	assert(pool != NULL);
	printf("\n========\nallocate a block memonry in memonry pool\n=======\n");
	uint8_t *m = NULL;
	memonry_pool_t *current = pool->current;
	memonry_pool_t *p = NULL;
	memonry_pool_t *newPool = (memonry_pool_t*)malloc(size + sizeof(memonry_pool_t));
	assert(newPool);
	newPool->d.last = (uint8_t *)newPool + sizeof(memonry_pool_t); 
	newPool->d.end = (uint8_t *)newPool + size + sizeof(memonry_pool_t);
	newPool->max = size;
	newPool->large = NULL;
	newPool->d.failed = 0;
	for(p = current;p->d.next;p = p->d.next)
	{
		if(p->d.failed++ > 4)
			current = p->d.next;
	}
	newPool->d.next = pool->d.next;
	pool->d.next = newPool;
	pool->current = current ? current : newPool;
	m = align_ptr(newPool->d.last,4);
	return m;
}
void *memonry_pool_alloc_large(memonry_pool_t *pool,size_t size)
{
	assert(pool != NULL);
	memonry_pool_large_data_t *pl = NULL; 
	int n = 0;
	void *p = malloc(size);
	assert(p != NULL);
	printf("\n========\nallocate a large memonry in memonry pool\n=======\n");
	for(pl = pool->large;pl;pl = pl->next)
	{
		if(pl->alloc == NULL)
		{
			pl->alloc = p;
			return p;
		}
		if(++n > 3)
			break;
	}
	pl = (memonry_pool_large_data_t *)malloc(sizeof(memonry_pool_large_data_t));
	pl->alloc = p;
	pl->next = pool->large;
	pool->large = pl;
	return p;
}
void *memonry_pool_alloc(memonry_pool_t *pool,size_t size)
{
	assert(pool != NULL);
	uint8_t *m;
	memonry_pool_t *p = NULL;
	if(size <= pool->max)
	{
		p = pool->current;
		do{
			m = align_ptr(p->d.last,4);
			if((size_t)(p->d.end - m) >= size)
			{
				p->d.last = m + size;	
				return m;
			}
			p = p->d.next;
		}while(p);
		return memonry_pool_alloc_block(pool,size);
	}
	return memonry_pool_alloc_large(pool,size);
}

int memonry_pool_free(memonry_pool_t *pool,void *p)
{
	memonry_pool_large_data_t *pl = NULL;
	assert(pool != NULL);
	for(pl = pool->large;pl;pl = pl->next)
	{
		if(pl->alloc == p)
		{
			free(p);
			pl->alloc = NULL;
			return 0;
		}
	}
	return -1;
}
