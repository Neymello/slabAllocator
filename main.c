#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <kmem.h>

#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#define OBJ_SIZE 1000

struct testBuf{
	int id;
	char * value;
	int pad[OBJ_SIZE];
};
typedef struct testBuf *testBuf_t;

void constr(void *buf, int size){
	int s = size;
	s++;
	testBuf_t buffer = (testBuf_t) buf;
	buffer->id=0;
	buffer->value = NULL;
}

void destruct(void *buf, int size){
	int s = size;
	s++;
	testBuf_t buffer = (testBuf_t) buf;
	buffer->id=0;
	buffer->value = NULL;
}

int main(void) {
	int count;
	int num_loop = 100;
	void *items[num_loop];
	int num_buf;
	unsigned long long start, end;
	printf("##########################\nThe result will be showed in number of cycles\n##########################\n");
	printf("------------\n\tObjects Size:%d\n\tIterations %d\n------------\n",OBJ_SIZE, num_loop);

	rdtscll(start);
	kmem_cache_t cache_test = kmem_cache_create("first", sizeof(struct testBuf), 0, &constr, &destruct );
	rdtscll(end);
	printf("Overhead for CACHE_CREATE %lld\n", (end-start));
	assert(cache_test->slab_free_count==0 && cache_test->slab_free_count==0 && cache_test->slab_free_count==0);

	rdtscll(start);
	for(count=0;count<num_loop;count++){
		items[count] = kmem_cache_alloc(cache_test,KM_SLEEP);
	}
	rdtscll(end);
	printf("Overhead for CACHE_ALLOC %lld\n", (end-start)/num_loop);

	rdtscll(start);
	for(count=0;count<num_loop/2;count++){
		kmem_cache_free(cache_test,items[count]);
	}
	rdtscll(end);
	printf("Overhead for CACHE_FREE %lld\n", (end-start)/(num_loop/2));

	rdtscll(start);
	for(count=num_loop/2;count<num_loop;count++){
		items[count] = kmem_cache_alloc(cache_test,KM_SLEEP);
	}
	rdtscll(end);
	printf("Overhead for CACHE_ALLOC (pre loaded) %lld\n", (end-start)/(num_loop/2));

	for(count=0;count<num_loop;count++){
		kmem_cache_free(cache_test,items[count]);
	}
	rdtscll(start);
	kmem_cache_reap(cache_test);
	rdtscll(end);
	printf("Overhead for CACHE_REAP %lld\n", (end-start));

	rdtscll(start);
	kmem_cache_destroy(cache_test);
	rdtscll(end);
	printf("Overhead for CACHE_DESTROY %lld\n", (end-start));

	return 0;

}
