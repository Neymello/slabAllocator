struct kmem_bufctl;

//kmem_slab
struct kmem_slab{
	struct kmem_slab *next;
	struct kmem_slab *previous;
	struct kmem_bufctl *buf_free;
	struct kmem_bufctl *buf_used;
};
typedef struct kmem_slab * kmem_slab_t;

//kmem_bufctl
struct kmem_bufctl {
	unsigned int *buffer;
	struct kmem_bufctl *next_free;
	struct kmem_slab *slab;
};
typedef struct kmem_bufctl * kmem_bufctrl_t;

//kmem_cache
struct kmem_cache{
	char *name;
	int size;
	int align;
	void *constructor;
	void *destructor;
};
typedef struct kmem_chache * kmem_cache_t;

/*
 * Functions
 */

kmem_cache_t kmem_cache_create(char *name, int size, int align, void *constructor, void *destructor);

void *kmem_cache_alloc(kmem_cache_t cache, int flags);

void kmem_cache_free(kmem_cache_t cache, void *buf);

void kmem_cache_destroy(kmem_cache_t cache);

void kmem_cache_grow(kmem_cache_t cache);

void kmem_cache_reap(kmem_cache_t cache);
