#include "header.h"
#include <stdio.h> 
#include <stdlib.h> 


void *padding(int size){
    return (INIT_MEMORY_ADDRESS + size);
}


void *make_memory_block(int padding_address, int size, int is_free){

    if(is_free){
        // HEADER 
        *(int *)padding(padding_address) = size;

        // FOOTER
        *(int *)padding(padding_address + size - HEADER_SIZE) = size;
    } else {
        // HEADER 
        *(int *)padding(padding_address) = -size;

        // FOOTER
        *(int *)padding(padding_address + size - HEADER_SIZE) = -size;
    }


    printf("header new block: %d pamat %d is free: %d\n", *(int *)padding(padding_address), (int *)padding(padding_address), is_free);
    printf("footer new block: %d pamat %d is free: %d\n", *(int *)padding(padding_address + size - sizeof(int)), (int *)padding(padding_address + size - sizeof(int)), is_free);

    return padding(padding_address);
}


void *memory_alloc(unsigned int size){

    if(!INIT_MEMORY_ADDRESS){
        return NULL;
    }

    unsigned int INIT_MEMORY_SIZE = *(unsigned int *)INIT_MEMORY_ADDRESS;
    // printf("init memory adrress: %d, init memory size: %d \n", INIT_MEMORY_ADDRESS, *(unsigned int *)INIT_MEMORY_ADDRESS);
    int best_match_address = -1, is_last_free_block = 0, block_size_1, block_size_2;
    int *starting_pointer, *current_free_block;

    starting_pointer = (unsigned int *) padding(HEADER_SIZE);
    // printf("starting pointer address: %d , value: %d \n", starting_pointer, *starting_pointer);

   
    /**
     * Implementation BEST FIT algorithm
     * Kym nie som na poslednom volnom bloku 
    */
    while (!is_last_free_block){
        current_free_block = (unsigned int *) padding(*starting_pointer);
        printf("current free block: %d, current free block value: %d and next one: %d\n", current_free_block, *current_free_block, *((char *)current_free_block + sizeof(int)));


        /**
         * current_free_block je presne taky velky ako potrebujem
        */
        if(*current_free_block == size + FOOTER_HEADER_SIZE){
            block_size_1 = size + FOOTER_HEADER_SIZE;
            printf("Great, this is exactly what I needed, starting pointer is: %d, block size: %d\n", *starting_pointer, block_size_1);

            void *help_address_1 = (char *)make_memory_block(*starting_pointer, block_size_1, 0);
            // printf("address of block is: %d and value there is: %d checking footer: %d AND ADDRESS: %d", (char *)help_address_1, *(int *)help_address_1, *(int *)padding(block_size_1 + *starting_pointer - HEADER_SIZE), (char *)padding(size+ *starting_pointer - HEADER_SIZE));

            return (char *)help_address_1 + sizeof(int);
        }

        /**
         * current_free_block je mensi ako pozadovany blok
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
            /**
             * Ak som na poslednom moznom bloku
            */
            if(*((char *)current_free_block + sizeof(int)) == 0){
                printf("I am on the last memorry block, please decide!\n");

                /**
                 * je prvy aj posledny zaroven vyhovujuci volny blok alebo dany blok pamata je najvhodnejsi
                 * 
                 * Pozri sa na best_match_address 
                 * Vytvor hlavicku, data a paticku 1. bloku
                 * Vytvor hlavicku data a paticku pre zostavajucu pamat po orezani 1.bloku >> 2. blok a za hlavicku volneho bloku daj 0 ak je posledny
                 * Vrat pointer na 1. blok
                 * Ak je to pociatocny pointer tak zmen pociatocny padding 
                */

                if(best_match_address == -1 || best_match_address > (*current_free_block - FOOTER_HEADER_SIZE)){
                    block_size_1 = size + FOOTER_HEADER_SIZE;
                    block_size_2 = *current_free_block - size - FOOTER_HEADER_SIZE;

                    printf("size of block 1: %d and size of block 2: %d \n", block_size_1, block_size_2);

                    void *help_address_1 = (char *)make_memory_block(*starting_pointer, block_size_1, 0);
                    printf("returned value_1: %d\n", (char *)help_address_1 + sizeof(int));

                    char *help_address_2 = (char *)make_memory_block(*starting_pointer + block_size_1, block_size_2, 1);

                    *starting_pointer = help_address_2 - (char *)INIT_MEMORY_ADDRESS;

                    printf("starting pointer: %d %d\n",*starting_pointer, *starting_pointer + sizeof(int));
                    *(unsigned int *)padding(*starting_pointer + sizeof(int)) = 0;
                    printf("starting pointer: %d %d\n",*(unsigned int *)padding(*starting_pointer + sizeof(int)), (unsigned int *)padding(*starting_pointer + sizeof(int)));

                    return (char *)help_address_1 + sizeof(int);
                }
                is_last_free_block = 1;
            }
            /**
             * este nie som na poslednom moznom bloku
            */
            else{
                printf("I am not on the last block!\n");
                /**
                 * vypocitaj rodiel a uloz adressu do best_match_address
                */
            } 
            is_last_free_block = 1;
        }
    }
}


int memory_free_before_block(int padding_address){
    int *before_memory_block = (unsigned int*)padding(padding_address - FOOTER_SIZE);
    
    printf("footer_previous address: %d footer_previous value: %d\n", before_memory_block, *before_memory_block);

    if(*before_memory_block > 0){
        return abs(*before_memory_block);
    }

    return 0;
}


int memory_free_behind_block(int padding_address){
    int *before_memory_block = (unsigned int*)padding(padding_address + FOOTER_SIZE);
    
    printf("header_next address: %d header_nect value: %d\n", before_memory_block, *before_memory_block);

    if(*before_memory_block > 0){
        return abs(*before_memory_block);
    }
    
    return 0;
}


int memory_free(void *valid_ptr){
    int helper_left, helper_right;
    void *block_to_free_header_address = (char *)valid_ptr - HEADER_SIZE;
    int padding_left = block_to_free_header_address - INIT_MEMORY_ADDRESS;
    int padding_right = (block_to_free_header_address + abs(*(int *)block_to_free_header_address) - FOOTER_SIZE) - INIT_MEMORY_ADDRESS;

    helper_left = memory_free_before_block(padding_left);
    helper_right = memory_free_behind_block(padding_right);
    printf("this address want delete: %d and value there: %d, padding_left is: %d padding_right is: %d helper_left is: %d helper_right is: %d\n", block_to_free_header_address, *(int *)block_to_free_header_address, padding_left, padding_right, helper_left, helper_right);

    if(helper_left > 0){
        /**
         * urob fragmentaciu s predchadzajucim blokom 
         * helper + abs(*(int *)block_to_free_header_address)
        */
    }


    return 0;
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
    // printf("pocet bytov k prvemu volnemu bloku %d, adresa: %d \n", 2*HEADER_SIZE, (unsigned int*)padding(HEADER_SIZE));

    // inicializacia headera bloku pamate
    *(unsigned int*)padding(FOOTER_HEADER_SIZE) = size - FOOTER_HEADER_SIZE;
    // printf("pocet volnych bytov: %d v pamati %d \n", size - FOOTER_HEADER_SIZE, (unsigned int*)padding(FOOTER_HEADER_SIZE));

    // najblizsi volny priestor pre data
    *(unsigned int*)padding(FOOTER_HEADER_SIZE + HEADER_SIZE) = 0;
    // printf("pocet volnych bytov: %d v pamati %d \n", *(unsigned int *)padding(FOOTER_HEADER_SIZE + HEADER_SIZE), (unsigned int*)padding(FOOTER_HEADER_SIZE + HEADER_SIZE));

    // inicializacia footera bloku pamate
    *(unsigned int *)padding(size - FOOTER_SIZE) = size - FOOTER_HEADER_SIZE;
    // printf("pocet volnych bytov: %d v pamati %d \n", size - FOOTER_HEADER_SIZE, (unsigned int*)padding(size - FOOTER_SIZE));
}


int main(){
    char arr[200];

    memory_init(arr, 200);
    
    char* pointer = (char*) memory_alloc(8);
    printf("pointer 1: %d\n", pointer);
    
    char* pointer2 = (char *) memory_alloc(30);
    printf("pointer 2: %d\n", pointer2);

    char* pointer3 = (char *) memory_alloc(130);
    printf("pointer 3: %d\n", pointer3);

    if (pointer2)
        memory_free(pointer2);
}