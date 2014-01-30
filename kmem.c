#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <kmem.h>

#define PAGE_SIZE 4096

kmem_slab_t _kmem_create_slab(kmem_fn_t construtor, int size){
	printf("Creating Slab... \n");


	kmem_slab_t slab = (kmem_slab_t) malloc(sizeof(struct kmem_slab));
	int num_buf, var;

	assert(size < PAGE_SIZE-1);

	num_buf = (PAGE_SIZE-1) / size;

	slab->buf_free = NULL;

	printf("%d buffers of size %d will be created\n", num_buf, size	);

	for (var = 0; var <= num_buf; ++var) {
		_kmem_create_buffer(slab, construtor, size);
	}

	return slab;
}

kmem_bufctrl_t _kmem_create_buffer(kmem_slab_t slab, kmem_fn_t construtor, int size){
//	printf("Creating Buffers...\n");

	kmem_bufctrl_t bufctrl = (kmem_bufctrl_t) malloc(sizeof(struct kmem_bufctl));

	bufctrl->buffer = malloc(size);

	bufctrl->slab = slab;

	if(slab->buf_free == NULL){
		slab->buf_free = bufctrl;
	}else{
		bufctrl->next_free = slab->buf_free;
		slab->buf_free = bufctrl;
	}

	return bufctrl;
}

kmem_cache_t kmem_cache_create(char *name, int size, int align, kmem_fn_t constructor, kmem_fn_t destructor){

	kmem_cache_t cache = (kmem_cache_t) malloc(sizeof(struct kmem_cache));

	cache->name = name;
	cache->size = size;
	cache->constructor = constructor;
	cache->destructor = destructor;

	cache->slab_free = _kmem_create_slab(constructor, size);

	return cache;
}

void *kmem_cache_alloc(kmem_cache_t cache, int flags){

	/*
	 * TODO: Check
	 */
	kmem_bufctrl_t bufctrl = cache->slab_free->buf_free;

	void *buffer = bufctrl->buffer;

	//Execute the constructor function
	cache->constructor(buffer,cache->size);

	//Remove the current buffer from the free list
	cache->slab_free->buf_free = bufctrl->next_free;

	//Add the current buffer to used list
	bufctrl->next_free = cache->slab_free->buf_used;
	cache->slab_free->buf_used = bufctrl;

	cache->slab_free->free_count--;

	return buffer;
}

void kmem_cache_free(kmem_cache_t cache, void *buf){

	kmem_bufctrl_t bufctrl = cache->slab_free->buf_used;

	bufctrl->buffer = buf;

	//Remove the current buffer from the used list
	cache->slab_free->buf_used = bufctrl->next_free;

	//Add the current buffer into the free list
	bufctrl->next_free = cache->slab_free->buf_free;
	cache->slab_free->buf_free = bufctrl;

	cache->slab_free->free_count++;

}
