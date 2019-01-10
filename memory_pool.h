#ifndef __memory_POOL__
#define __memory_POOL__
#include <stdint.h>
#include <sys/types.h>
/*
	内存储内存申请分配方式:
		首先判断申请的内存是否大于内存块的容量,若是，则直接通过malloc申请内存，并挂载到当前内存块的large链表中
		若申请的内存小于内存块容量，则有一下两种情况:
		1. 申请内存大小小于等于当前内存块空闲容量，则直接在当前内存块中分配内存，返回空闲空间首地址
		2. 申请内存大小大于当前内存块空闲容量，则创建一个新的内存块，并在新的内存块中分配空间.
 
 */
typedef struct _memory_pool_large_data_t{
	struct _memory_pool_large_data_t *next;
	void *alloc;
}memory_pool_large_data_t;

typedef struct _memory_pool_t memory_pool_t;
typedef struct _memory_pool_data_t{
	uint8_t *last;
	uint8_t *end;
	memory_pool_t *next;
	uint32_t failed;
}memory_pool_data_t;

struct _memory_pool_t{
	memory_pool_data_t d;
	uint32_t max;
	struct _memory_pool_t *current;
	memory_pool_large_data_t *large;
};

memory_pool_t *memory_pool_create(size_t size);
void memory_pool_destroy(memory_pool_t *pool);
void *memory_pool_alloc(memory_pool_t *pool,size_t size);
void *memory_pool_alloc_block(memory_pool_t *pool,size_t size);
void *memory_pool_alloc_large(memory_pool_t *pool,size_t size);
int memory_pool_free(memory_pool_t *pool,void *p);
#endif
