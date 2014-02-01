#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <kmem.h>

#define PAGE_SIZE 4096

kmem_slab_t __kmem_cache_grow(kmem_cache_t cache, int size, int flag){
	printf("Creating Slab... \n");

	kmem_slab_t slab = (kmem_slab_t) malloc(sizeof(struct kmem_slab));

	if(flag){
		while(slab == NULL){
			slab = (kmem_slab_t) malloc(sizeof(struct kmem_slab));
		}
	}

	int num_buf, var;

	assert(size < PAGE_SIZE-1);

	num_buf = (PAGE_SIZE-1) / size;

	slab->buf_free = NULL;
	slab->free_count = num_buf;

	printf("%d buffers of size %d will be created\n", num_buf, size	);

	for (var = 0; var <= num_buf; ++var) {
		__kmem_create_buffer(slab, size);
	}

	slab->next = NULL;

	if(cache->slab_free != NULL){
		slab->previous = NULL;
	}else{
		slab->previous = cache->slab_free;
	}

	cache->slab_free = slab;

	return slab;
}

kmem_bufctrl_t __kmem_create_buffer(kmem_slab_t slab, int size){
	kmem_bufctrl_t bufctrl = (kmem_bufctrl_t ) malloc(size);

	bufctrl->slab = slab;

	if(slab->buf_free != NULL){
		bufctrl->next_free = slab->buf_free;
		slab->buf_free = bufctrl;
	}

	slab->buf_free = bufctrl;

	return bufctrl;
}

kmem_cache_t kmem_cache_create(char *name, int size, int align, kmem_fn_t constructor, kmem_fn_t destructor){

	kmem_cache_t cache = (kmem_cache_t) malloc(sizeof(struct kmem_cache));

	cache->name = name;
	cache->size = size;
	cache->constructor = constructor;
	cache->destructor = destructor;
	cache->slab_free = NULL;
	cache->align = align;

	__kmem_cache_grow(cache, size, KM_SLEEP);

	return cache;
}

void *kmem_cache_alloc(kmem_cache_t cache, int flag){
	if(cache->slab_free->free_count == 0){
		printf("No more free slabs.....");
		__kmem_cache_grow(cache, cache->size, flag);
	}

	kmem_bufctrl_t bufctrl = cache->slab_free->buf_free;

	//Remove the current buffer from the free list
	cache->slab_free->buf_free = bufctrl->next_free;
	cache->slab_free->free_count--;

	void *buffer = bufctrl;

	//Execute the constructor function
	cache->constructor(buffer,cache->size);

	return buffer;
}

void kmem_cache_free(kmem_cache_t cache, void *buf){

	cache->destructor(buf, cache->size);

	kmem_bufctrl_t bufctrl = buf;

	//Add the current buffer into the free list
	bufctrl->next_free = cache->slab_free->buf_free;
	cache->slab_free->buf_free = bufctrl;

	cache->slab_free->free_count++;

}
