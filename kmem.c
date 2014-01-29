#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <kmem.h>

#define PAGE_SIZE 4096

kmem_slab_t _kmem_create_slab(int size){

	kmem_slab_t slab = (kmem_slab_t) malloc(sizeof(struct kmem_slab));
	int num_buf, var;

	assert(size < PAGE_SIZE-1);

	num_buf = (PAGE_SIZE-1) / size;

	slab->buf_free = NULL;

	printf("%d buffers will be created\n", num_buf);

	for (var = 0; var <= num_buf; ++var) {
		_kmem_create_buffer(slab, size);
	}

	return slab;
}

kmem_bufctrl_t _kmem_create_buffer(kmem_slab_t slab, int size){

	kmem_bufctrl_t buffer = (kmem_bufctrl_t) malloc(sizeof(struct kmem_bufctl));

	buffer->buffer = (void *) malloc(size);

	buffer->slab = slab;

	if(slab->buf_free == NULL){
		slab->buf_free = buffer;
	}else{
		buffer->next_free = slab->buf_free;
		slab->buf_free = buffer;
	}

	return buffer;
}
