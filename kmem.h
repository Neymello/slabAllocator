#define DEBUG 0
#define PRINT_DEBUG(fmt, args...)    if(DEBUG)printf(fmt, ## args)

struct kmem_bufctl;

//kmem_slab
struct kmem_slab{
	int buf_free_count;
	int buf_used_count;
	void *buffer;
	struct kmem_slab *next;
	struct kmem_slab *previous;
	struct kmem_bufctl *buf_free;
};
typedef struct kmem_slab * kmem_slab_t;

//kmem_bufctl
struct kmem_bufctl {
	void *buffer;
	struct kmem_bufctl *next_free;
	struct kmem_slab *slab;
};
typedef struct kmem_bufctl * kmem_bufctrl_t;

typedef void (*kmem_fn_t)(void*, int);

//kmem_cache
struct kmem_cache{
	char *name;
	int size;
	int align;
	kmem_fn_t constructor;
	kmem_fn_t destructor;

	kmem_slab_t slab_free;
	kmem_slab_t slab_used;
	kmem_slab_t slab_full;

	int slab_free_count;
	int slab_used_count;
	int slab_full_count;
};
typedef struct kmem_cache * kmem_cache_t;

typedef enum{
	KM_NOSLEEP,
	KM_SLEEP
} km_wait_flag;

typedef enum{
	KM_FREE_LIST,
	KM_USED_LIST,
	KM_FULL_LIST
} km_slab_list;

/*
 * Functions
 */

kmem_cache_t kmem_cache_create(char *name, int size, int align, kmem_fn_t construtor, kmem_fn_t destructor);

void *kmem_cache_alloc(kmem_cache_t cache, int flags);

void kmem_cache_free(kmem_cache_t cache, void *buf);

void kmem_cache_destroy(kmem_cache_t cache);

void kmem_cache_reap(kmem_cache_t cache);

/*
 * Internal functions
 *
 */

kmem_slab_t __kmem_cache_grow(kmem_cache_t cache, int size, int flag);

kmem_bufctrl_t __kmem_create_buffer(kmem_slab_t slab, void *pointer);

kmem_bufctrl_t __kmem_pull_buffer(kmem_slab_t *slab_p, int size);

void __kmem_push_buffer(kmem_slab_t *slab_p, kmem_bufctrl_t *bufctrl_p, int size);

/*
 * Convenient
 *
 */
void __insert_into_slab_list(kmem_cache_t *cache, kmem_slab_t *item_p, int type);

void __remove_from_slab_list(kmem_cache_t *cache, int type);

