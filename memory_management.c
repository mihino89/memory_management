#include "header.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>


void *padding(int size){
    return (INIT_MEMORY_ADDRESS + size);
}


int make_memory_block(int padding_left_address, int size, int is_free){

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

    return padding_left_address;
}


int find_previous_free_memory_block(int padding_left_address){
    padding_left_address -= FOOTER_SIZE;
    // 58

    while (*(int *)padding(padding_left_address) < 0){
        padding_left_address -= abs(*(int*)padding(padding_left_address));
    }

    // dany blok je prvy volny a teda padding_left_address je adresa super_pointra
    if(padding_left_address == HEADER_SIZE){
        return padding_left_address;
    }
    
    // blok nie je prvy a aktualne je padding_left_address je pozicia footer-a - dostan sa na header
    padding_left_address = padding_left_address - abs(*(int *)padding(padding_left_address)) + FOOTER_SIZE;
    return padding_left_address;
}


int find_next_free_memory_block(int padding_address){
    int padding_value = *(int *)padding(padding_address);
    // printf("padding_value %d\n",padding_value);

    if(padding_value + padding_address >= *(int *)INIT_MEMORY_ADDRESS){
        // printf("som posledny!\n");
        return 0;
    }

    // printf("padding value is: %d padding address: %d next hop: %d\n", padding_value, padding_address, *(int *)padding(padding_address + padding_value));
    while (*(int *)padding(padding_address + padding_value) < 0){
        padding_address += abs(padding_value);
        padding_value = abs(*(int *)padding(padding_address));
        if(padding_value + padding_address >= *(int *)INIT_MEMORY_ADDRESS){
            // printf("som posledny!\n");
            return 0;
        }
    }

    padding_address += abs(padding_value);
    // printf("return padding address: %d\n", padding_address);
    if(padding_address >= *(int *)INIT_MEMORY_ADDRESS)
        return 0;
    return padding_address;
}


void *memory_alloc(unsigned int size){

    if(!INIT_MEMORY_ADDRESS){
        return NULL;
    }

    unsigned int INIT_MEMORY_SIZE = *(unsigned int *)INIT_MEMORY_ADDRESS;
    int best_match_value = -1, *best_match_address, best_match_padding, is_last_free_block = 0, block_size_1, block_size_2;
    int *super_pointer, *current_free_block, current_free_block_padding;

    super_pointer = (unsigned int *) padding(HEADER_SIZE);

    if(*super_pointer == 0){
        printf("V initovanej pamati nie je uz dostatok miesta! Prosim realokujte pamat\n");
        return 0;
    }

    current_free_block = (unsigned int *) padding(*super_pointer);
    current_free_block_padding = *super_pointer;

    while (!is_last_free_block){
        /**
         * current_free_block je presne taky velky ako potrebujem
        */
        if(*current_free_block == size + FOOTER_HEADER_SIZE){
            // ======= BLOKY =======
            int the_nearest_address_left = find_previous_free_memory_block(current_free_block_padding);
            int the_nereast_address_right = find_next_free_memory_block(current_free_block_padding);
            // printf("the_nereast_address_right: %d\n",the_nereast_address_right);

            block_size_1 = size + FOOTER_HEADER_SIZE;
            
            int padding_addres_block = make_memory_block(current_free_block_padding, block_size_1, 0);

            // ======= POINTRE =======
            /**
             * Som prvy a nie posledny - povedz super_pointru adresu na nasledujuci
            */
            if(the_nearest_address_left <= 4 && the_nereast_address_right){
                // printf("Som prvy - povedz super_pointru ze nasledujuci je prvy, superpointer before: %d\n", *super_pointer);
                *super_pointer = the_nereast_address_right;
            }

            /**
             * Som posledny a nie prvy - povedz predchadzajucemu ze je posledny
            */
            else if(the_nearest_address_left > 4 && !the_nereast_address_right){
                // printf("som na konci!\n!");
                *(unsigned int *)padding(the_nearest_address_left + FOOTER_SIZE) = 0;
            }

            /**
             * som prvy aj posledny - nastav super pointer na 0
            */
            if(the_nearest_address_left <= 4 && !the_nereast_address_right)
                *super_pointer = 0;
            
            /**
             * Som v strede - povedz predchadzajucemu nech ukazuje na nasledujuceho 
            */
            else if(the_nearest_address_left > 4&& the_nereast_address_right){
                *(int *)(padding(the_nearest_address_left + HEADER_SIZE)) = the_nereast_address_right;
                // printf("z lava nereast one: %d z prava nearest one: %d pointer z lava adresa: %d\n", the_nearest_address_left, the_nereast_address_right, *(int *)(padding(the_nearest_address_left + HEADER_SIZE)));
            }

            return (char *)padding(padding_addres_block + sizeof(int));
        }

        /**
         * Dany blok je vacsi ako potrebujem
        */
        else if(*current_free_block > size + FOOTER_HEADER_SIZE){
            /**
             * Ak som na poslednom moznom bloku
            */
            // printf("!!find_next_free_memory_block(): %d %d %d\n",current_free_block, check_next_memory_block(current_free_block_padding), *(int *)padding((char *)current_free_block - (char *)INIT_MEMORY_ADDRESS + HEADER_SIZE));
            if(!find_next_free_memory_block(current_free_block_padding)){
                // ======= BLOKY =======
                /**
                 * som na poslednom a skontrolujem ci posledny nie je lepsi ako predchadzajuce
                */
                if(best_match_value == -1 || best_match_value > (*current_free_block - FOOTER_HEADER_SIZE - size)){
                    best_match_value = *current_free_block - FOOTER_HEADER_SIZE;
                    best_match_address = current_free_block;
                    best_match_padding = (char *)current_free_block - (char *)INIT_MEMORY_ADDRESS;
                    // printf("current free block: %d %d, current free block value HAHAH: %d\n", best_match_address, (int *)INIT_MEMORY_ADDRESS, best_match_value);
                } 
                int the_nearest_address_left = find_previous_free_memory_block(best_match_padding);
                int the_nereast_address_right = find_next_free_memory_block(best_match_padding);
                // printf("this %d\n", the_nereast_address_right);
                // printf("best_match_value: %d address: %d\n", best_match_padding, best_match_address);

                block_size_1 = size + FOOTER_HEADER_SIZE;
                block_size_2 = best_match_value - size + FOOTER_HEADER_SIZE;
                // printf("size of block 1: %d and size of block 2: %d \n", block_size_1, block_size_2);
                
                int padding_addres_block_1 = (char *)best_match_address - (char *)INIT_MEMORY_ADDRESS;
                // printf("padding address 1: %d\n", padding_addres_block_1);

                int padding_addres_block_left = make_memory_block(padding_addres_block_1, block_size_1, 0);
                int padding_addres_block_right = make_memory_block(padding_addres_block_1 + block_size_1, block_size_2, 1);
                
                // ======= POINTRE =======
                /**
                 * Som prvy a nie posledny - povedz super_pointru adresu na nasledujuci
                */
                if(the_nearest_address_left <= 4 && the_nereast_address_right){
                    *super_pointer = padding_addres_block_right;
                    // nastav bloku 2. pointer na dalsi blok
                    *(int *)padding(*super_pointer + HEADER_SIZE) = the_nereast_address_right;
                    // printf("super pointer: %d\n", the_nereast_address_right);
                }

                /**
                 * som prvy aj posledny
                */
                else if(the_nearest_address_left <= 4 && !the_nereast_address_right){
                    *super_pointer = padding_addres_block_right;
                    *(unsigned int *)padding(*super_pointer + HEADER_SIZE) = 0;
                }

                /**
                 * Som posledny a nie prvy - pozri sa na predchadzajuci a posun pointer na blok 2, blok dva_header + HEADER = 0;
                */
                else if(the_nearest_address_left > 4 && !the_nereast_address_right){
                    *(unsigned int *)padding(padding_addres_block_right + HEADER_SIZE) = 0;                  
                    *(int *)(padding(the_nearest_address_left + HEADER_SIZE)) = padding_addres_block_right;
                }

                /**
                 * Som v strede - povedz predchadzajucemu nech ukazuje na nasledujuceho 
                */
                else if(the_nearest_address_left > 4 && the_nereast_address_right){
                    *(int *)(padding(the_nearest_address_left + HEADER_SIZE)) = padding_addres_block_right;
                    *(int *)padding(padding_addres_block_right + HEADER_SIZE) = the_nereast_address_right;
                }

                return (char *)padding(padding_addres_block_left + sizeof(int));
            }
            /**
             * este nie som na poslednom moznom bloku - urob porovnanie s best_match
            */
            else if(best_match_value == -1 || best_match_value > (*current_free_block - FOOTER_HEADER_SIZE - size)){
                // printf("I am not on the last block! and this block is better for best_match\n");

                /**
                 * vypocitaj rodiel a uloz adressu do best_match_address
                */
                best_match_value = *current_free_block - FOOTER_HEADER_SIZE - size;
                best_match_address = (int *)current_free_block;
                best_match_padding = (char *)current_free_block - (char *)INIT_MEMORY_ADDRESS;
            } 
            /**
             * posun sa na dalsi blok
            */
            current_free_block_padding = *(int *)padding((char *)current_free_block - (char *)INIT_MEMORY_ADDRESS + HEADER_SIZE);
            current_free_block = (int *)padding(current_free_block_padding);
            // printf("current free block: %d, current free block value: %d, next is: %d\n", current_free_block, *current_free_block, current_free_block_padding);
        }
    }
}


int check_previous_memory_block(int padding_left_address){
    int *before_memory_block = (unsigned int*)padding(padding_left_address - FOOTER_SIZE);
    // printf("arg footer_previous address: %d\n", *before_memory_block );

    if(padding_left_address <= 8){
        // printf("Dany blok je prvy alebo mimo laveho rozsahu!\n");
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
        // printf("Dany blok je posledny alebo mimo laveho rozsahu!\n");
        return 0;
    }
    else if(*next_memory_block > 0){
        return abs(*next_memory_block);
        // printf("header_next address: %d header_nect value: %d\n", next_memory_block, *next_memory_block);
    }

    return 0;
}


int memory_free(void *valid_ptr){
    int size_of_left_free_memory_block, size_of_right_free_memory_block, padding_left, padding_right, find_the_nearest_left, find_the_nearest_right, padding_addres_block_1;
    void *block_to_free_header_address = (char *)valid_ptr - HEADER_SIZE;
    *(int *)block_to_free_header_address = abs(*(int *)block_to_free_header_address);

    padding_left = block_to_free_header_address - INIT_MEMORY_ADDRESS;
    padding_right = (block_to_free_header_address + *(int *)block_to_free_header_address - FOOTER_SIZE) - INIT_MEMORY_ADDRESS;

    size_of_left_free_memory_block = check_previous_memory_block(padding_left);
    size_of_right_free_memory_block = check_next_memory_block(padding_right);
    
    // printf("value there to delete: %d, padding_left is: %d padding_right is: %d size_of_left_free_memory_block is: %d size_of_right_free_memory_block is: %d\n", *(int *)block_to_free_header_address, padding_left, padding_right, size_of_left_free_memory_block, size_of_right_free_memory_block);

    // ======= BLOKY =======
    if(size_of_left_free_memory_block && size_of_right_free_memory_block){
        // printf("vedla mna na pravo aj na lavo sa nachadza volny blok, spoj sa s nim!\n");
        padding_addres_block_1 = make_memory_block(padding_left - size_of_left_free_memory_block, *(int *)block_to_free_header_address + size_of_left_free_memory_block + size_of_right_free_memory_block, 1);
    }

    else if(size_of_left_free_memory_block && !size_of_right_free_memory_block){
        // printf("vedla mna na lavo sa nachadza volny blok, spoj sa s nim! %d\n", padding_left - size_of_left_free_memory_block);
        padding_addres_block_1 = make_memory_block(padding_left - size_of_left_free_memory_block, *(int *)block_to_free_header_address + size_of_left_free_memory_block, 1);
    }

    else if(!size_of_left_free_memory_block && size_of_right_free_memory_block){
        // printf("vedla mna na pravo sa nachadza volny blok, spoj sa s nim!\n");
        padding_addres_block_1 = make_memory_block(padding_left, *(int *)block_to_free_header_address + size_of_right_free_memory_block, 1);
    }

    else{
        padding_addres_block_1 = make_memory_block(padding_left, *(int *)block_to_free_header_address, 1);
        // printf("vedla mna nieje volny blok!! %d %d\n", (int *)help_address_1, (int *)block_to_free_header_address);
    }

    // ======= POINTRE =======
    // najdi najblizsi predchadzajuci 

    find_the_nearest_left = find_previous_free_memory_block(padding_addres_block_1);
    find_the_nearest_right = find_next_free_memory_block(padding_addres_block_1);
    // printf("find_the_nearest_left: %d find_the_nearest_right: %d padding_after_fragmentation  %d\n", find_the_nearest_left, find_the_nearest_right, padding_after_fragmentation);

    if(find_the_nearest_left <= HEADER_SIZE)
        *(int *)padding(find_the_nearest_left) = padding_addres_block_1;
    else
        *(int *)padding(find_the_nearest_left + HEADER_SIZE) = padding_addres_block_1;
    

    if(find_the_nearest_right)
        *(int *)padding(padding_addres_block_1 + HEADER_SIZE) = find_the_nearest_right;
    else 
        *(int *)padding(padding_addres_block_1 + HEADER_SIZE) = 0;

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


void print_memory_blocks_in_region(char region[]){
    int akt_address = FOOTER_HEADER_SIZE;

    printf("\nregion super pointer: %d ", abs(*(unsigned int *)padding(akt_address - HEADER_SIZE)));

    while(abs(*(int *)padding(akt_address)) > HEADER_SIZE && akt_address < *(int *)INIT_MEMORY_ADDRESS){
        printf("| %d xx%dxx %d ", *(int *)padding(akt_address), akt_address, *(int *)padding(akt_address));
        akt_address = akt_address + abs(*(int *)padding(akt_address));
    }
    // printf("? %d xx%dxx %d ", *(int *)padding(akt_address), akt_address, *(int *)padding(akt_address));
    printf("\n");
}


int generate_numbers_for_testing(int min, int max){
    return (rand() % (max - min + 1)) + min;
}


void small_test(){
    int random_region_number = generate_numbers_for_testing(24, 50000);
    char arr[random_region_number];

    memory_init(arr, random_region_number);
    
    char* pointer = (char*) memory_alloc(8);
    int padding_left = (void *)pointer - INIT_MEMORY_ADDRESS - 4;
    int padding_left_value = *(int *)padding(padding_left);
    printf("super pointer: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer - INIT_MEMORY_ADDRESS));
    
    char* pointer2 = (char *) memory_alloc(30);
    padding_left = (void *)pointer2 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer2 - INIT_MEMORY_ADDRESS));

    char* pointer3 = (char *) memory_alloc(130);
    padding_left = (void *)pointer3 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer3 - INIT_MEMORY_ADDRESS));
    print_memory_blocks_in_region(arr);

    if (pointer){
        memory_free(pointer);
    }
    print_memory_blocks_in_region(arr);

    if (pointer3){
        memory_free(pointer3);
    }
    print_memory_blocks_in_region(arr);

    if (pointer2){
        memory_free(pointer2);
    }
    print_memory_blocks_in_region(arr);
}


void medium_test(){
    int blok_1, blok_2, blok_3, blok_4, random_region_number = generate_numbers_for_testing(24, 50000);
    char arr[random_region_number];
    char *pointer1, *pointer2, *pointer3, *pointer4, *pointer5, *pointer6;

    memory_init(arr, random_region_number);
    
    blok_1 = generate_numbers_for_testing(8, 2000);
    pointer1 = (char*) memory_alloc(blok_1);
    int padding_left = (void *)pointer1 - INIT_MEMORY_ADDRESS - 4;
    int padding_left_value = *(int *)padding(padding_left);
    printf("super pointer1: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer1 - INIT_MEMORY_ADDRESS));
    
    blok_2 = generate_numbers_for_testing(8, 2000);
    pointer2 = (char *) memory_alloc(blok_2);
    padding_left = (void *)pointer2 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer2: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer2 - INIT_MEMORY_ADDRESS));

    blok_3 = generate_numbers_for_testing(8, 2000);
    pointer3 = (char *) memory_alloc(blok_3);
    padding_left = (void *)pointer3 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer3: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer3 - INIT_MEMORY_ADDRESS));
    print_memory_blocks_in_region(arr);

    if (pointer1){
        memory_free(pointer1);
    }
    print_memory_blocks_in_region(arr);
    
    pointer4 = (char*) memory_alloc(8);
    padding_left = (void *)pointer4 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer4: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer4 - INIT_MEMORY_ADDRESS));
    print_memory_blocks_in_region(arr);

    pointer5 = (char*) memory_alloc(8);
    padding_left = (void *)pointer5 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer5: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer5 - INIT_MEMORY_ADDRESS));
    print_memory_blocks_in_region(arr);

    if (pointer4){
        memory_free(pointer4);
    }
    print_memory_blocks_in_region(arr);

    if (pointer5){
        memory_free(pointer5);
    }
    print_memory_blocks_in_region(arr);

    pointer6 = (char*) memory_alloc(blok_1);
    padding_left = (void *)pointer6 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer6: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer6 - INIT_MEMORY_ADDRESS));
    print_memory_blocks_in_region(arr);

    pointer4 = (char *) memory_alloc(random_region_number - FOOTER_HEADER_SIZE);
    padding_left = (void *)pointer4 - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int *)padding(padding_left);
    printf("super pointer4: %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int *)arr + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int *)padding(padding_left)), *(int *)padding(padding_left), *(int *)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int *)padding((void *)pointer4 - INIT_MEMORY_ADDRESS));
    print_memory_blocks_in_region(arr);

    if (pointer3){
        memory_free(pointer3);
    }
    print_memory_blocks_in_region(arr);

    if (pointer2){
        memory_free(pointer2);
    }
    print_memory_blocks_in_region(arr);

    if (pointer6){
        memory_free(pointer6);
    }
    print_memory_blocks_in_region(arr);

    if (pointer4){
        memory_free(pointer4);
    }
    print_memory_blocks_in_region(arr);
}


int main(){
    small_test();
    printf("======= NEXT TEST =======\n");
    medium_test();

    return 0;
}