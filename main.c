#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <kmem.h>

#define NUM_BUFFERS 10

void * BUFFERS[NUM_BUFFERS];

struct testBuf{
	int id;
	char * value;
	int pad[100];
};
typedef struct testBuf *testBuf_t;

void constr(void *buf, int size){
	int s = size;
	s++;

	printf("===>Inside construtor\t");
	testBuf_t buffer = (testBuf_t) buf;


	buffer->id=0;
	buffer->value = NULL;

	printf(":::::Leaving constructor\n");
}

void destruct(void *buf, int size){
	int s = size;
	s++;
	printf("===>Inside destructor\t");
	testBuf_t buffer = (testBuf_t) buf;

	buffer->id=0;
	buffer->value = NULL;

	printf(":::::Leaving destructor\n");
}

void testAllocation(kmem_cache_t cache, int numLoop){
	printf("===>Test Allocation\n");
	int index;

	for(index = 0; index < numLoop; index++){
		testBuf_t b = (testBuf_t) kmem_cache_alloc(cache,KM_SLEEP);
		assert(b->id == 0);

		b->id = 20;
		b->value = "alo";

		BUFFERS[index] = b;
	}
}

void testFreeing(kmem_cache_t cache, int numLoop){
	printf("===>Test Freeing\n");
	int index;

	for(index = 0; index < numLoop; index++){
		kmem_cache_free(cache,BUFFERS[index]);
	}
}



int main(void) {

	kmem_cache_t cache_test = kmem_cache_create("first", sizeof(struct testBuf), 0, &constr, &destruct );

	testAllocation(cache_test,NUM_BUFFERS);

	testFreeing(cache_test, NUM_BUFFERS/2 );

	testAllocation(cache_test,NUM_BUFFERS);

	return 0;

}
