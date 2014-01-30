#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <kmem.h>


struct testBuf{
	int id;
	char * value;
	int pad[100];
};
typedef struct testBuf *testBuf_t;

void constr(void *buf, int size){
	printf("Inside construtor\n");
	testBuf_t buffer = (testBuf_t) buf;

	buffer->id=0;
	buffer->value = NULL;
//	buffer->pad = malloc(400);

}

int main(void) {


//	_kmem_create_slab(&constr, sizeof(struct testBuf));
	kmem_cache_t a = kmem_cache_create("first", sizeof(struct testBuf), 0, &constr, &constr );

	assert(a->size == sizeof(struct testBuf));

	testBuf_t b = (testBuf_t) kmem_cache_alloc(a,0);

	assert(b->id == 0);

	b->id = 20;
	b->value = "alo";

	kmem_cache_free(a, b);

	testBuf_t c = (testBuf_t) kmem_cache_alloc(a,0);

	assert(c->id == 0);



	return 0;

}
