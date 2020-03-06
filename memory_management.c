#include "header.h"
#include <stdio.h> 
#include <stdlib.h> 


void *memory_alloc(unsigned int size){
    

}

void *padding(int size){
    return (MEMORY_BEGIN + size);
}

void memory_init(void *ptr, unsigned int size){
    MEMORY_BEGIN = (char *) ptr;

    if(size < 4*(sizeof(unsigned int))) return;

	*(unsigned int*)ptr = size;
    printf("adresa MB: %d, velkost MB: %d \n", (int *) MEMORY_BEGIN, *((int *) MEMORY_BEGIN));

    // pocet bytov k prvemu volnemu bloku
    *(unsigned int*)padding(HEADER_SIZE) = 2*HEADER_SIZE;
    printf("pocet bytov k prvemu volnemu bloku %d, adresa: %d \n", 2*HEADER_SIZE, (unsigned int*)padding(HEADER_SIZE));

    // inicializacia headera bloku pamate
    *(unsigned int*)padding(FOOTER_HEADER_SIZE) = size - FOOTER_HEADER_SIZE;
    printf("pocet volnych bytov: %d v pamati %d \n", size - FOOTER_HEADER_SIZE, (unsigned int*)padding(FOOTER_HEADER_SIZE));

    // inicializacia footera bloku pamate
    *(unsigned int*)padding(size - FOOTER_SIZE) = size - FOOTER_HEADER_SIZE;
    printf("pocet volnych bytov: %d v pamati %d \n", size - FOOTER_HEADER_SIZE, (unsigned int*)padding(size - FOOTER_SIZE));

    // najblizsi volny priestor pre data
    *(unsigned int*)padding(FOOTER_HEADER_SIZE + HEADER_SIZE) = 0;
    printf("pocet volnych bytov: %d v pamati %d \n", *(int *) ((char*)ptr + FOOTER_HEADER_SIZE + HEADER_SIZE), (unsigned int*)padding(FOOTER_HEADER_SIZE + HEADER_SIZE));
}


int main(){
    char arr[100];
    char arr1[200];

    memory_init(arr, 100);
    memory_init(arr1, 200);
}