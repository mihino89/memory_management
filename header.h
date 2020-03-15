#include <stdio.h>

#define TRUE	(1)
#define FALSE	(0)
#define HEADER_SIZE sizeof(unsigned int)
#define FOOTER_SIZE sizeof(unsigned int)
#define FOOTER_HEADER_SIZE (2*sizeof(unsigned int))

void* INIT_MEMORY_ADDRESS = NULL;

void *memory_alloc(unsigned int size);
int memory_free(void *valid_ptr);
int memory_check(void *ptr);
void memory_init(void *ptr, unsigned int size);

int check_next_memory_block(int padding_address);
int check_previous_memory_block(int padding_left_address);
int find_next_free_memory_block(int padding_address);
int find_previous_free_memory_block(int padding_left_address);

int make_memory_block(int padding_left_address, int size, int is_free);
void* padding(int size);

void testing_enviroment();
void test_velky();
void test_stredny();
void test_maly();

void test_small_memory_checks(char* pointer1, char* pointer2, char* pointer3);
void test_medium_memory_checks(char* pointer1, char* pointer2, char* pointer3, char* pointer4, char* pointer5, char* pointer6);
int generate_numbers_for_testing(int min, int max);
void memory_alloc_check(char* pointer);
void print_memory_blocks_in_region(char region[]);
int perc_usage(int akt_memory_usage);