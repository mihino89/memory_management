#include <stdio.h>


#define TRUE	(1)
#define FALSE	(0)
#define HEADER_SIZE 2*sizeof(int)


void *memory_alloc(unsigned int size);
int memory_free(void *valid_ptr);
int memory_check(void *ptr);
void memory_init(void *ptr, unsigned int size);
