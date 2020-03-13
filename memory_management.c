#include "header.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>


void *padding(int size){
    return (INIT_MEMORY_ADDRESS + size);
}


void *make_memory_block(int padding_left_address, int size, int is_free){

    if(is_free){
        // HEADER 
        *(int *)padding(padding_left_address) = size;

        // FOOTER
        *(int *)padding(padding_left_address + size - HEADER_SIZE) = size;
    } else {
        // HEADER 
        *(int *)padding(padding_left_address) = -size;

        // FOOTER
        *(int *)padding(padding_left_address + size - HEADER_SIZE) = -size;
    }

    // printf("header new block: %d pamat %d is free: %d\n", *(int *)padding(padding_left_address), (int *)padding(padding_left_address), is_free);
    // printf("footer new block: %d pamat %d is free: %d\n", *(int *)padding(padding_left_address + size - sizeof(int)), (int *)padding(padding_left_address + size - sizeof(int)), is_free);

    return padding(padding_left_address);
}


int find_previous_free_memory_block(int padding_left_address){
    padding_left_address -= FOOTER_SIZE;

    while (*(int*)padding(padding_left_address) < 0){
        padding_left_address -= abs(*(int*)padding(padding_left_address));
    }
    
    return padding_left_address;
}


int find_next_free_memory_block(int padding_address){
    int padding_value = *(int *)padding(padding_address);
    // printf("padding value is: %d padding address: %d next hop: %d\n", padding_value, padding_address, *(int *)padding(padding_address + padding_value));

    if(padding_value + padding_address >= *(int *)INIT_MEMORY_ADDRESS){
        printf("som posledny!\n");
        return 0;
    }

    // not tested (not implemented)
    while (*(int *)padding(padding_address + padding_value) < 0){
        padding_address += abs(padding_value);
        padding_value = abs(*(int *)padding(padding_address));
    }

    padding_address += abs(padding_value);
    if(padding_address == *(int *)INIT_MEMORY_ADDRESS)
        return 0;

    // printf("return padding address: %d\n", padding_address);
    return padding_address;
}


void *memory_alloc(unsigned int size){

    if(!INIT_MEMORY_ADDRESS){
        return NULL;
    }

    unsigned int INIT_MEMORY_SIZE = *(unsigned int *)INIT_MEMORY_ADDRESS;
    int best_match_value = -1, *best_match_address, is_last_free_block = 0, block_size_1, block_size_2, counter = 0;
    int *akt_pointer, *super_pointer, *current_free_block;

    akt_pointer = (unsigned int *) padding(HEADER_SIZE);
    super_pointer = (unsigned int *) padding(HEADER_SIZE);

    if(*super_pointer == 0){
        printf("V initovanej pamati nie je uz dostatok miesta! Prosim realokujte pamat\n");
        return 0;
    }

    while (!is_last_free_block){
        counter++;

        current_free_block = (unsigned int *) padding(*akt_pointer);
        printf("current free block: %d, current free block value: %d\n", current_free_block, *current_free_block);


        /**
         * current_free_block je presne taky velky ako potrebujem
        */
        if(*current_free_block == size + FOOTER_HEADER_SIZE){
            // ======= BLOKY =======

            block_size_1 = size + FOOTER_HEADER_SIZE;
            printf("Great, this is exactly what I needed, starting pointer is: %d, block size: %d\n", *akt_pointer, block_size_1);
            
            void *help_address_1 = (char *)make_memory_block((char *)current_free_block - (char *)INIT_MEMORY_ADDRESS, block_size_1, 0);
            // printf("address of block is: %d and value there is: %d checking footer: %d AND ADDRESS: %d\n", (char *)help_address_1, *(int *)help_address_1, *(int *)padding(block_size_1 + *akt_pointer - HEADER_SIZE), (char *)padding(size+ *akt_pointer - HEADER_SIZE));
            
            // ======= POINTRE =======
            /**
             * Som prvy a nie posledny - povedz super_pointru adresu na nasledujuci
            */
            if(counter == 1 && *((char *)current_free_block + HEADER_SIZE) != 0){
                printf("Som prvy - povedz super_pointru ze nasledujuci je prvy, superpointer before: %d\n", *super_pointer);
                *super_pointer = find_next_free_memory_block((char *)current_free_block - (char *)INIT_MEMORY_ADDRESS);
                printf("Som prvy - povedz super_pointru ze nasledujuci je prvy, superpointer after: %d\n", *super_pointer);
            }

            /**
             * Som posledny a nie prvy - povedz predchadzajucemu ze je posledny
            */
            else if(counter != 1 && *((char *)current_free_block + HEADER_SIZE) == 0){
                printf("som na konci!\n!");
                int the_nearest_address_left = find_previous_free_memory_block((char *)current_free_block - (char *)INIT_MEMORY_ADDRESS);
                *(unsigned int *)padding(the_nearest_address_left + FOOTER_SIZE) = 0;
            }

            /**
             * som prvy aj posledny - nastav super pointer na 0
            */
            if(counter == 1 && *((char *)current_free_block + HEADER_SIZE) == 0){
                printf("Som prvy aj posledny pointer!\n");
                *super_pointer = 0;
            }

            /**
             * Som v strede - povedz predchadzajucemu nech ukazuje na nasledujuceho 
            */
            else if(counter != 1 && *((char *)current_free_block + HEADER_SIZE) != 0){
                int the_nearest_address_left = find_previous_free_memory_block((char *)current_free_block - (char *)INIT_MEMORY_ADDRESS);
                // zapise do najblizsieho z lava pointer na najblizsi z prava
                int the_nereast_address_right = find_next_free_memory_block((char *)current_free_block - (char *)INIT_MEMORY_ADDRESS);
                *(int *)(padding(the_nearest_address_left + HEADER_SIZE)) = the_nereast_address_right;
                printf("z lava nereast one: %d z prava nearest one: %d pointer z lava adresa: %d\n", the_nearest_address_left, the_nereast_address_right, *(int *)(padding(the_nearest_address_left + HEADER_SIZE)));
            }

            return (char *)help_address_1 + sizeof(int);
        }

        /**
         * Dany blok je vacsi ako potrebujem
        */
        else if(*current_free_block > size + FOOTER_HEADER_SIZE){
            /**
             * Ak som na poslednom moznom bloku
            */
            if(*((char *)current_free_block + HEADER_SIZE) == 0 ){
                // ======= BLOKY =======
                /**
                 * som na poslednom a skontrolujem ci posledny nie je lepsi ako predchadzajuce
                */
                if(best_match_value == -1 || best_match_value > (*current_free_block - FOOTER_HEADER_SIZE - size)){
                    best_match_value = *current_free_block - FOOTER_HEADER_SIZE;
                    best_match_address = current_free_block;
                    printf("current free block: %d %d, current free block value HAHAH: %d\n", best_match_address, (int *)INIT_MEMORY_ADDRESS, best_match_value);
                }

                block_size_1 = size + FOOTER_HEADER_SIZE;
                block_size_2 = best_match_value - size;
                printf("size of block 1: %d and size of block 2: %d \n", block_size_1, block_size_2);
                
                int padding_addres_block_1 = (char *)best_match_address - (char *)INIT_MEMORY_ADDRESS;
                printf("padding address 1: %d\n", padding_addres_block_1);
                void *help_address_1 = (char *)make_memory_block(padding_addres_block_1, block_size_1, 0);

                void *help_address_2 = (char *)make_memory_block(padding_addres_block_1 + block_size_1, block_size_2, 1);
                
                // ======= POINTRE =======
                /**
                 * Som prvy a nie posledny - povedz super_pointru adresu na nasledujuci
                */
                if(counter == 1 && *((char *)best_match_address + HEADER_SIZE) != 0){
                    *super_pointer = (char *)help_address_2 - (char *)INIT_MEMORY_ADDRESS;
                    int the_nereast_address_right = find_next_free_memory_block((char *)help_address_2 - (char *)INIT_MEMORY_ADDRESS);
                    // nastav bloku 2. pointer na dalsi blok
                    *(int *)padding(*super_pointer + HEADER_SIZE) = the_nereast_address_right;
                }

                /**
                 * som prvy aj posledny
                */
                else if(counter == 1 && *((char *)best_match_address + HEADER_SIZE) == 0){
                    *super_pointer = (char *)help_address_2 - (char *)INIT_MEMORY_ADDRESS;
                    *(unsigned int *)padding(*super_pointer + HEADER_SIZE) = 0;
                }

                /**
                 * Som posledny a nie prvy - pozri sa na predchadzajuci a posun pointer na blok 2, blok dva_header + HEADER = 0;
                */
                else if(counter != 1 && *((char *)best_match_address + HEADER_SIZE) == 0){
                    *(unsigned int *)padding(help_address_2 - INIT_MEMORY_ADDRESS + HEADER_SIZE) = 0;
                    int the_nearest_address_left = find_previous_free_memory_block(help_address_2 - INIT_MEMORY_ADDRESS);
                    
                    *(int *)(padding(the_nearest_address_left + HEADER_SIZE)) = help_address_2 - INIT_MEMORY_ADDRESS;
                }

                /**
                 * Som v strede - povedz predchadzajucemu nech ukazuje na nasledujuceho 
                */
                else if(counter != 1 && *((char *)best_match_address + HEADER_SIZE) != 0){
                    int the_nearest_address_left = find_previous_free_memory_block(help_address_2 - INIT_MEMORY_ADDRESS);
                    int the_nereast_address_right = find_next_free_memory_block(help_address_2 - INIT_MEMORY_ADDRESS);

                    *(int *)(padding(the_nearest_address_left + HEADER_SIZE)) = help_address_2 - INIT_MEMORY_ADDRESS;
                    *(int *)padding(help_address_2 - INIT_MEMORY_ADDRESS + HEADER_SIZE) = the_nereast_address_right;
                }

                return (char *)help_address_1 + HEADER_SIZE;
            }
            /**
             * este nie som na poslednom moznom bloku - urob porovnanie s best_match
            */
            else if(best_match_value == -1 || best_match_value > (*current_free_block - FOOTER_HEADER_SIZE - size)){
                printf("I am not on the last block! and this block is better for best_match\n");

                /**
                 * vypocitaj rodiel a uloz adressu do best_match_address
                */
                best_match_value = *current_free_block - FOOTER_HEADER_SIZE - size;
                best_match_address = (int *)current_free_block;
            } 
            /**
             * posun sa na dalsi blok
            */
            current_free_block = current_free_block + *current_free_block;
        }
    }
}


int check_previous_memory_block(int padding_left_address){
    int *before_memory_block = (unsigned int*)padding(padding_left_address - FOOTER_SIZE);
    
    if(padding_left_address <= 8){
        printf("Dany block je prvy alebo mimo laveho rozsahu!\n");
        return 0;
    }

    else if(*before_memory_block > 0){
        // printf("footer_previous address: %d footer_previous value: %d\n", before_memory_block, *before_memory_block);
        return abs(*before_memory_block);
    }

    return 0;
}


int check_next_memory_block(int padding_address){
    int padding_value = abs(*(int *)padding(padding_address));
    int *next_memory_block = (unsigned int*)padding(padding_address + FOOTER_SIZE);

    if(padding_address + padding_value >= *(int *)INIT_MEMORY_ADDRESS){
        printf("Dany block je posledny alebo mimo laveho rozsahu!\n");
        return 0;
    }
    else if(*next_memory_block > 0){
        return abs(*next_memory_block);
        // printf("header_next address: %d header_nect value: %d\n", next_memory_block, *next_memory_block);
    }

    return 0;
}


int memory_free(void *valid_ptr){
    int size_of_left_free_memory_block, size_of_right_free_memory_block, padding_left, padding_right, padding_after_fragmentation, find_the_nearest_left, find_the_nearest_right;
    void *help_address_1, *block_to_free_header_address = (char *)valid_ptr - HEADER_SIZE;
    *(int *)block_to_free_header_address = abs(*(int *)block_to_free_header_address);

    padding_left = block_to_free_header_address - INIT_MEMORY_ADDRESS;
    padding_right = (block_to_free_header_address + *(int *)block_to_free_header_address - FOOTER_SIZE) - INIT_MEMORY_ADDRESS;

    size_of_left_free_memory_block = check_previous_memory_block(padding_left);
    size_of_right_free_memory_block = check_next_memory_block(padding_right);
    
    printf("value there to delete: %d, padding_left is: %d padding_right is: %d size_of_left_free_memory_block is: %d AND! %d size_of_right_free_memory_block is: %d\n", *(int *)block_to_free_header_address, padding_left, padding_right, size_of_left_free_memory_block, *(int *)padding(padding_left - size_of_left_free_memory_block), size_of_right_free_memory_block);

    // ======= BLOKY =======
    if(size_of_left_free_memory_block && size_of_right_free_memory_block){
        printf("vedla mna na pravo aj na lavo sa nachadza volny blok, spoj sa s nim!\n");
        help_address_1 = (char *)make_memory_block(padding_left - size_of_left_free_memory_block, *(int *)block_to_free_header_address + size_of_left_free_memory_block + size_of_right_free_memory_block, 1);
    }

    else if(size_of_left_free_memory_block && !size_of_right_free_memory_block){
        printf("vedla mna na lavo sa nachadza volny blok, spoj sa s nim! %d\n", padding_left - size_of_left_free_memory_block);
        help_address_1 = (char *)make_memory_block(padding_left - size_of_left_free_memory_block, *(int *)block_to_free_header_address + size_of_left_free_memory_block, 1);
    }

    else if(!size_of_left_free_memory_block && size_of_right_free_memory_block){
        printf("vedla mna na pravo sa nachadza volny blok, spoj sa s nim!\n");
        help_address_1 = (char *)make_memory_block(padding_left, *(int *)block_to_free_header_address + size_of_right_free_memory_block, 1);
    }

    else {
        help_address_1 = make_memory_block(padding_left, *(int *)block_to_free_header_address, 1);
        printf("vedla mna nieje volny blok!! %d %d\n", (int *)help_address_1, (int *)block_to_free_header_address);
    }

    padding_after_fragmentation = help_address_1 - INIT_MEMORY_ADDRESS;
    printf("help address %d help value: %d padding_after_fragmentation  %d\n", (int *)help_address_1, *(int *)help_address_1, padding_after_fragmentation);

    // ======= POINTRE =======
    // najdi najblizsi predchadzajuci 

    find_the_nearest_left = find_previous_free_memory_block(padding_after_fragmentation);
    find_the_nearest_right = find_next_free_memory_block(padding_after_fragmentation);
    printf("find_the_nearest_left: %d find_the_nearest_right: %d padding_after_fragmentation  %d\n", find_the_nearest_left, find_the_nearest_right, padding_after_fragmentation);

    // bug!!!
    *(int *)padding(find_the_nearest_left) = padding_after_fragmentation;
    if(find_the_nearest_right)
        *(int *)padding(padding_after_fragmentation + HEADER_SIZE) = find_the_nearest_right;
    else 
        *(int *)padding(padding_after_fragmentation + HEADER_SIZE) = 0;

    printf("super pointer block: %d, nearest right pointer: %d\n", *(int *)padding(find_the_nearest_left + HEADER_SIZE), *(int *)padding(padding_after_fragmentation + HEADER_SIZE));
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


void small_test(){
    char arr[200];

    memory_init(arr, 200);
    
    char* pointer = (char*) memory_alloc(8);
    printf("pointer 1: %d\n", pointer);
    
    char* pointer2 = (char *) memory_alloc(30);
    printf("pointer 2: %d\n", pointer2);

    char* pointer3 = (char *) memory_alloc(130);
    printf("pointer 3: %d\n", pointer3);

    if (pointer){
        memory_free(pointer);
    }

    if (pointer2){
        memory_free(pointer2);
    }

    if (pointer3){
        memory_free(pointer3);
    }
}


int main(){
    small_test();
}