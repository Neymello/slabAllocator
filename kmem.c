#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <kmem.h>

#define PAGE_SIZE 4096

kmem_slab_t __kmem_cache_grow(kmem_cache_t cache, int size, int flag){
	PRINT_DEBUG("Creating Slab... \n");
	int num_buf, var;
	kmem_slab_t slab = (kmem_slab_t) malloc(sizeof(struct kmem_slab));

	if(flag){
		while(slab == NULL){
			slab = (kmem_slab_t) malloc(sizeof(struct kmem_slab));
		}
	}

	assert(size < PAGE_SIZE-1);
	num_buf = (PAGE_SIZE-1) / size;

	slab->buf_free = NULL;
	slab->buf_free_count = num_buf;
	slab->buf_used_count = 0;
	slab->next = NULL;
	slab->previous = NULL;
	slab->buffer = malloc(PAGE_SIZE-1);

	PRINT_DEBUG("%d buffers of size %d will be created\n", num_buf, size	);

	for (var = 0; var <= num_buf; ++var) {
		__kmem_create_buffer(slab, slab->buffer + (size*var));
	}

	__insert_into_slab_list(&cache, &slab, KM_FREE_LIST);

	cache->slab_free = slab;

	return slab;
}

kmem_bufctrl_t __kmem_create_buffer(kmem_slab_t slab, void *pointer){
	kmem_bufctrl_t bufctrl = (kmem_bufctrl_t ) pointer;

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
	cache->slab_used = NULL;
	cache->slab_full = NULL;

	cache->slab_free_count=0;
	cache->slab_used_count=0;
	cache->slab_full_count=0;

	cache->align = align;

	return cache;
}

void kmem_cache_destroy(kmem_cache_t cache){
	PRINT_DEBUG("===>Destroying cache\n");
	kmem_slab_t slab = cache->slab_free;
	kmem_slab_t aux;
	int count;

	assert(cache->slab_used_count == 0);
	assert(cache->slab_full_count == 0);

	for (count = 0; count < cache->slab_free_count; ++count) {
		aux = slab;
		slab = slab->next;

		free(aux);
	}
}

void kmem_cache_reap(kmem_cache_t cache){
	PRINT_DEBUG("===>Reaping the cache");
	kmem_slab_t slab;

	if(cache->slab_free_count > 0){
		slab = cache->slab_free;
		__remove_from_slab_list(&cache,KM_FREE_LIST);
		free(slab);
	}
}

void *kmem_cache_alloc(kmem_cache_t cache, int flag){
	PRINT_DEBUG("===>Allocating cache: ");
	km_slab_list type;
	kmem_slab_t slab;
	kmem_bufctrl_t bufctrl;

	if(	cache->slab_used_count != 0 && cache->slab_used->buf_free_count !=0){
		PRINT_DEBUG("One used found");
		type = KM_USED_LIST;
		slab = cache->slab_used;

	}else if(cache->slab_free_count != 0){
		PRINT_DEBUG("One free found");
		type = KM_FREE_LIST;
		slab = cache->slab_free;
	}
	else{
		PRINT_DEBUG("No more free slabs.....");
		__kmem_cache_grow(cache, cache->size, flag);
		type = KM_FREE_LIST;
		slab = cache->slab_free;
	}

	bufctrl = __kmem_pull_buffer(&slab, cache->size);

	PRINT_DEBUG("\n");

	//Add the slab into the list of used slabs
	if(type == KM_FREE_LIST){
		__remove_from_slab_list(&cache,KM_FREE_LIST);
		if(slab->buf_free_count == 0){
			__insert_into_slab_list(&cache, &slab, KM_FULL_LIST);
		}else{
			__insert_into_slab_list(&cache, &slab, KM_USED_LIST);
		}
	}
	else if(type== KM_USED_LIST && slab){
		if(slab->buf_free_count == 0){
			__remove_from_slab_list(&cache,KM_USED_LIST);
			__insert_into_slab_list(&cache, &slab, KM_FULL_LIST);
		}
	}

	void *buffer = bufctrl;

	//Execute the constructor function
	cache->constructor(buffer,cache->size);

	return buffer;
}

void kmem_cache_free(kmem_cache_t cache, void *buf){
	kmem_bufctrl_t bufctrl = buf;
	kmem_slab_t slab;

	cache->destructor(buf, cache->size);

	if(cache->slab_used_count != 0){
		slab = cache->slab_used;
		__kmem_push_buffer(&cache->slab_used, &bufctrl, cache->size);

		if(cache->slab_used->buf_used_count == 0){
			__remove_from_slab_list(&cache, KM_USED_LIST);
			__insert_into_slab_list(&cache,&slab, KM_FREE_LIST);
		}

	}else{
		slab = cache->slab_full;
		__kmem_push_buffer(&cache->slab_full, &bufctrl, cache->size);

		if(cache->slab_full->buf_used_count == 0){
			__remove_from_slab_list(&cache, KM_FULL_LIST);
			__insert_into_slab_list(&cache,&slab, KM_FREE_LIST);
		}else{

			__remove_from_slab_list(&cache, KM_FULL_LIST);
			__insert_into_slab_list(&cache,&slab, KM_USED_LIST);
		}
	}

}

kmem_bufctrl_t __kmem_pull_buffer(kmem_slab_t *slab_p, int size){
	kmem_slab_t slab = *slab_p;
	kmem_bufctrl_t bufctrl = (kmem_bufctrl_t) slab->buffer + (size * slab->buf_used_count-1);

	slab->buf_free = bufctrl;
	slab->buf_free_count--;
	slab->buf_used_count++;

	return bufctrl;
}

void __kmem_push_buffer(kmem_slab_t *slab_p, kmem_bufctrl_t *bufctrl_p, int size){
	kmem_slab_t slab = *slab_p;
	kmem_bufctrl_t bufctrl = *bufctrl_p;

	if (slab->buf_free_count == 1){
		bufctrl->next_free = NULL;
	}else{
		bufctrl->next_free = slab->buffer + (size * slab->buf_used_count);
	}

	slab->buf_free = bufctrl;
	slab->buf_free_count++;
	slab->buf_used_count--;
}

void __insert_into_slab_list(kmem_cache_t *cache_p, kmem_slab_t *item_p, int type){
	PRINT_DEBUG("===>Inserting into the Slab List %d\n",type);
	kmem_cache_t  cache = (*cache_p);
	kmem_slab_t item = (*item_p);

	if(type == KM_FREE_LIST){
		if(cache->slab_free == NULL){
			cache->slab_free = item;
		}else{
			item->previous = cache->slab_free->previous;
			item->next = cache->slab_free;
			cache->slab_free->previous = item;
			cache->slab_free = item;
		}
		cache->slab_free_count++;
	}
	else if(type == KM_USED_LIST){
		if(cache->slab_used == NULL){
			cache->slab_used = item;
		}else{
			item->previous = cache->slab_used->previous;
			item->next = cache->slab_used;
			cache->slab_used->previous = item;
			cache->slab_used = item;
		}
		cache->slab_used_count++;
	}
	else if(type == KM_FULL_LIST){
		if(cache->slab_full == NULL){
			cache->slab_full = item;
		}else{
			item->previous = cache->slab_full->previous;
			item->next = cache->slab_full;
			cache->slab_full->previous = item;
			cache->slab_full = item;
		}
		cache->slab_full_count++;
	}

}

void __remove_from_slab_list(kmem_cache_t *cache_p, int type){
	PRINT_DEBUG("===>Removing item from list %d\n",type);
	kmem_cache_t cache = (*cache_p);
	kmem_slab_t previous;
	kmem_slab_t next;
	kmem_slab_t current;

	if(type == KM_FREE_LIST){
		current = cache->slab_free;
		previous = current->previous;
		next = current->next;

		if(previous){
			previous->next = next;
		}
		if(next){
			next->previous = previous;
		}
		cache->slab_free = next;
		cache->slab_free_count--;
	}
	else if(type == KM_USED_LIST){
		current = cache->slab_used;
		previous = current->previous;
		next = current->next;

		if(previous){
			previous->next = next;
		}
		if(next){
			next->previous = previous;
		}

		cache->slab_used = next;
		cache->slab_used_count--;
	}
	else if(type == KM_FULL_LIST){
		current = cache->slab_full;
		previous = current->previous;
		next = current->next;

		if(previous){
			previous->next = next;
		}
		if(next){
			next->previous = previous;
		}

		cache->slab_full = next;
		cache->slab_full_count--;
	}
}
