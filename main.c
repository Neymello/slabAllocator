#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <kmem.h>


int main(void) {

	_kmem_create_slab(400);

	return 0;
}
