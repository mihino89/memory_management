#include "header.h"
#include <stdio.h> 
#include <stdlib.h> 


void *padding(int size){
    return (INIT_MEMORY_ADDRESS + size);
}


void *memory_alloc(unsigned int size){

    if(!INIT_MEMORY_ADDRESS){
        return NULL;
    }

    unsigned int INIT_MEMORY_SIZE = *(unsigned int *)INIT_MEMORY_ADDRESS;
    // printf("init memory adrress: %d, init memory size: %d \n", INIT_MEMORY_ADDRESS, *(unsigned int *)INIT_MEMORY_ADDRESS);
    int best_match_address = -1, is_last_free_block = 0;
    int *starting_pointer, *current_free_block;

    starting_pointer = (unsigned int *) padding(HEADER_SIZE);
    printf("starting pointer address: %d , value: %d \n", starting_pointer, *starting_pointer);

   
    /**
     * Implementation BEST FIT algorithm
     * Kym nie som na poslednom volnom bloku 
    */
    while (!is_last_free_block){
        current_free_block = (unsigned int *) padding(*starting_pointer);
        printf("current free block: %d, current free block value: %d\n", current_free_block, *current_free_block);


        /**
         * current_free_block je presne taky velky ako potrebujem
        */
        if(*current_free_block == size + FOOTER_HEADER_SIZE){
            printf("Great, this is exactly what I needed\n");
            /**
             * Vytvor blok pamate 
            */
        }

        /**
         * current_free_block je mensi ako pozadovany  blok
        */
        else if(*current_free_block < size + FOOTER_HEADER_SIZE){
            printf("Current memorry block is too small, sorry, go ahead!\n");
            /**
             * Dany blok je prilis maly posun sa na pointer >> skoc na dalsi volny blok pamate
            */
        }

        /**
         * Dany blok je vacsi ako potrebujem
        */
        else {
            if(!*((char *)current_free_block + sizeof(int))){
                printf("Som na poslednom volnom bloku!\n");
                /**
                 * Pozri sa na best_match_address 
                 * Vytvor hlavicku, data a paticku 1. bloku
                 * Vytvor hlavicku data a paticku pre zostavajucu pamat po orezani 1.bloku >> 2. blok
                 * Vrat pointer na 1. blok
                */
                is_last_free_block = 1;
            }
            else{
                /**
                 * ci je blok vacsi + hlavicka + paticka == size
                 * ak nie vypocitaj rodiel a uloz adressu do best_match_address
                */
            } 
        }       
    }
   

}


void memory_init(void *ptr, unsigned int size){
    INIT_MEMORY_ADDRESS = (char *) ptr;

    /** 
     * Ak ma menej ako 16b tak neviem vytvorit ani jeden blok 
     * TODO - Najmensia hodnota na init bude potrebna v mojej strukture - 8+16 = 24b
    */
   
    if(size < 4*(sizeof(unsigned int))) return;

	*(unsigned int *)INIT_MEMORY_ADDRESS = size;
    printf("adresa MB: %d, velkost MB: %d \n", (int *) INIT_MEMORY_ADDRESS, *((int *) INIT_MEMORY_ADDRESS));

    // pocet bytov k prvemu volnemu bloku
    *(unsigned int*)padding(HEADER_SIZE) = 2*HEADER_SIZE;
    printf("pocet bytov k prvemu volnemu bloku %d, adresa: %d \n", 2*HEADER_SIZE, (unsigned int*)padding(HEADER_SIZE));

    // inicializacia headera bloku pamate
    *(unsigned int*)padding(FOOTER_HEADER_SIZE) = size - FOOTER_HEADER_SIZE;
    printf("pocet volnych bytov: %d v pamati %d \n", size - FOOTER_HEADER_SIZE, (unsigned int*)padding(FOOTER_HEADER_SIZE));

    // najblizsi volny priestor pre data
    *(unsigned int*)padding(FOOTER_HEADER_SIZE + HEADER_SIZE) = 0;
    printf("pocet volnych bytov: %d v pamati %d \n", *(unsigned int *)padding(FOOTER_HEADER_SIZE + HEADER_SIZE), (unsigned int*)padding(FOOTER_HEADER_SIZE + HEADER_SIZE));

    // inicializacia footera bloku pamate
    *(unsigned int *)padding(size - FOOTER_SIZE) = size - FOOTER_HEADER_SIZE;
    printf("pocet volnych bytov: %d v pamati %d \n", size - FOOTER_HEADER_SIZE, (unsigned int*)padding(size - FOOTER_SIZE));
}


int main(){
    char arr[200];

    memory_init(arr, 200);
    
    char* pointer = (char*) memory_alloc(8);
}