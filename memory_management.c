#include "header.h"


void memory_init(void *ptr, unisigned int size){
	*(unisigned int*)ptr = size;
	*(char*)((char*)ptr + sizeof(unisigned int)) = 0;

	return
}


void *memory_alloc(unsigned int size){

}


void addblock(ptr p, int len) {
    int newsize = ((len + 1) >> 1) << 1;
    int oldsize = *p & ~0x1;
    *p = newsize | 0x1;
    if (newsize < oldsize)
    *(p+newsize) = oldsize - newsize;
}