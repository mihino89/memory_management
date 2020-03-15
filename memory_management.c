#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* padding(int size)
{
    return (INIT_MEMORY_ADDRESS + size);
}

int make_memory_block(int padding_left_address, int size, int is_free)
{

    if (is_free) {
        // HEADER
        *(int*)padding(padding_left_address) = size;

        // FOOTER
        *(int*)padding(padding_left_address + size - HEADER_SIZE) = size;
    }
    else {
        // HEADER
        *(int*)padding(padding_left_address) = -size;

        // FOOTER
        *(int*)padding(padding_left_address + size - HEADER_SIZE) = -size;
    }

    return padding_left_address;
}

int find_previous_free_memory_block(int padding_left_address)
{
    padding_left_address -= FOOTER_SIZE;

    while (*(int*)padding(padding_left_address) < 0) {
        padding_left_address -= abs(*(int*)padding(padding_left_address));
    }

    // dany blok je prvy volny a teda padding_left_address je adresa super_pointra
    if (padding_left_address == HEADER_SIZE) {
        return padding_left_address;
    }

    // blok nie je prvy a aktualne je padding_left_address je pozicia footer-a - dostan sa na header
    padding_left_address = padding_left_address - abs(*(int*)padding(padding_left_address)) + FOOTER_SIZE;
    return padding_left_address;
}

int find_next_free_memory_block(int padding_address)
{
    int padding_value = *(int*)padding(padding_address);

    // som posledny
    if (padding_value + padding_address >= *(int*)INIT_MEMORY_ADDRESS) {
        return 0;
    }

    while (*(int*)padding(padding_address + padding_value) < 0) {
        padding_address += abs(padding_value);
        padding_value = abs(*(int*)padding(padding_address));
        if (padding_value + padding_address >= *(int*)INIT_MEMORY_ADDRESS) {
            return 0;
        }
    }

    padding_address += abs(padding_value);
    if (padding_address >= *(int*)INIT_MEMORY_ADDRESS)
        return 0;
    return padding_address;
}

void* memory_alloc(unsigned int size)
{

    if (!INIT_MEMORY_ADDRESS) {
        return NULL;
    }

    unsigned int INIT_MEMORY_SIZE = *(unsigned int*)INIT_MEMORY_ADDRESS;
    int best_match_value = -1, *best_match_address, best_match_padding, is_last_free_block = 0, block_size_1, block_size_2;
    int *super_pointer, *current_free_block, current_free_block_padding;

    super_pointer = (unsigned int*)padding(HEADER_SIZE);

    if (*super_pointer == 0) {
        printf("V initovanej pamati nie je uz dostatok miesta! Prosim realokujte pamat\n");
        return 0;
    }

    current_free_block = (unsigned int*)padding(*super_pointer);
    current_free_block_padding = *super_pointer;

    while (!is_last_free_block) {
        /**
         * current_free_block je presne taky velky ako potrebujem
        */
        if (*current_free_block == size + FOOTER_HEADER_SIZE) {
            // ======= BLOKY =======
            int the_nearest_address_left = find_previous_free_memory_block(current_free_block_padding);
            int the_nereast_address_right = find_next_free_memory_block(current_free_block_padding);

            block_size_1 = size + FOOTER_HEADER_SIZE;

            int padding_addres_block = make_memory_block(current_free_block_padding, block_size_1, 0);

            // ======= POINTRE =======
            /**
             * Som prvy a nie posledny - povedz super_pointru adresu na nasledujuci
            */
            if (the_nearest_address_left <= 4 && the_nereast_address_right)
                *super_pointer = the_nereast_address_right;

            /**
             * Som posledny a nie prvy - povedz predchadzajucemu ze je posledny
            */
            else if (the_nearest_address_left > 4 && !the_nereast_address_right)
                *(unsigned int*)padding(the_nearest_address_left + FOOTER_SIZE) = 0;

            /**
             * som prvy aj posledny - nastav super pointer na 0
            */
            else if (the_nearest_address_left <= 4 && !the_nereast_address_right)
                *super_pointer = 0;

            /**
             * Som v strede - povedz predchadzajucemu nech ukazuje na nasledujuceho 
            */
            else if (the_nearest_address_left > 4 && the_nereast_address_right)
                *(int*)(padding(the_nearest_address_left + HEADER_SIZE)) = the_nereast_address_right;

            return (char*)padding(padding_addres_block + sizeof(int));
        }

        /**
         * Dany blok je vacsi ako potrebujem
        */
        else if (*current_free_block > size + FOOTER_HEADER_SIZE) {
            /**
             * Ak som na poslednom moznom bloku
            */
            if (!find_next_free_memory_block(current_free_block_padding)) {
                // ======= BLOKY =======
                /**
                 * som na poslednom bloku a zaroven prvy vhodny kandidat
                */
                if (best_match_value == -1 || best_match_value > (*current_free_block - FOOTER_HEADER_SIZE - size)) {
                    best_match_value = *current_free_block - FOOTER_HEADER_SIZE - FOOTER_HEADER_SIZE;
                    best_match_address = current_free_block;
                    best_match_padding = (char*)current_free_block - (char*)INIT_MEMORY_ADDRESS;
                }
                int the_nearest_address_left = find_previous_free_memory_block(best_match_padding);
                int the_nereast_address_right = find_next_free_memory_block(best_match_padding);

                block_size_1 = size + FOOTER_HEADER_SIZE;
                block_size_2 = best_match_value - size + FOOTER_HEADER_SIZE;

                int padding_addres_block_1 = (char*)best_match_address - (char*)INIT_MEMORY_ADDRESS;

                /**
                 * Ak v bloku 2 nie je priestor ani na footer/header alebo ani na najmensiu pamat tak spoj s blokom 1
                */
                if (block_size_2 <= FOOTER_HEADER_SIZE) {
                    block_size_1 = size + FOOTER_HEADER_SIZE + block_size_2;
                    int padding_addres_block_left = make_memory_block(padding_addres_block_1, block_size_1, 0);

                    if (the_nearest_address_left <= 4 && the_nereast_address_right)
                        *super_pointer = the_nereast_address_right;

                    if (the_nearest_address_left <= 4 && !the_nereast_address_right)
                        *super_pointer = 0;

                    else if (the_nearest_address_left > 4 && the_nereast_address_right)
                        *(int*)(padding(the_nearest_address_left + HEADER_SIZE)) = the_nereast_address_right;

                    return (char*)padding(padding_addres_block_left + HEADER_SIZE);
                }

                int padding_addres_block_left = make_memory_block(padding_addres_block_1, block_size_1, 0);
                int padding_addres_block_right = make_memory_block(padding_addres_block_1 + block_size_1, block_size_2, 1);

                // ======= POINTRE =======
                /**
                 * Som prvy a nie posledny - povedz super_pointru adresu na nasledujuci
                */
                if (the_nearest_address_left <= 4 && the_nereast_address_right) {
                    *super_pointer = padding_addres_block_right;
                    *(int*)padding(*super_pointer + HEADER_SIZE) = the_nereast_address_right;
                }

                /**
                 * som prvy aj posledny
                */
                else if (the_nearest_address_left <= 4 && !the_nereast_address_right) {
                    *super_pointer = padding_addres_block_right;
                    *(unsigned int*)padding(*super_pointer + HEADER_SIZE) = 0;
                }

                /**
                 * Som posledny a nie prvy - pozri sa na predchadzajuci a posun pointer na blok 2, blok dva_header + HEADER = 0;
                */
                else if (the_nearest_address_left > 4 && !the_nereast_address_right) {
                    *(unsigned int*)padding(padding_addres_block_right + HEADER_SIZE) = 0;
                    *(int*)(padding(the_nearest_address_left + HEADER_SIZE)) = padding_addres_block_right;
                }

                /**
                 * Som v strede - povedz predchadzajucemu nech ukazuje na nasledujuceho 
                */
                else if (the_nearest_address_left > 4 && the_nereast_address_right) {
                    *(int*)(padding(the_nearest_address_left + HEADER_SIZE)) = padding_addres_block_right;
                    *(int*)padding(padding_addres_block_right + HEADER_SIZE) = the_nereast_address_right;
                }

                return (char*)padding(padding_addres_block_left + HEADER_SIZE);
            }

            /**
             * este nie som na poslednom moznom bloku - urob porovnanie s best_match
            */
            else if (best_match_value == -1 || best_match_value > (*current_free_block - FOOTER_HEADER_SIZE - size)) {
                /**
                 * vypocitaj rodiel a uloz adressu do best_match_address
                */
                best_match_value = *current_free_block - FOOTER_HEADER_SIZE - size;
                best_match_address = (int*)current_free_block;
                best_match_padding = (char*)current_free_block - (char*)INIT_MEMORY_ADDRESS;
            }

            /**
             * posun sa na dalsi blok
            */
            current_free_block_padding = *(int*)padding((char*)current_free_block - (char*)INIT_MEMORY_ADDRESS + HEADER_SIZE);
            current_free_block = (int*)padding(current_free_block_padding);
        }

        /**
         * Ak som na poslednom bloku a nenasiel som vhodny (vacsi) blok
        */
        else if (!find_next_free_memory_block(current_free_block_padding) && best_match_value == -1) {
            printf("MEMORY ALLOC ERROR --> In region is not enough memory for this size of block\n");
            return 0;
        }
    }
}

int check_previous_memory_block(int padding_left_address)
{
    int* before_memory_block = (unsigned int*)padding(padding_left_address - FOOTER_SIZE);

    if (padding_left_address <= 8)
        return 0;

    else if (*before_memory_block > 0)
        return abs(*before_memory_block);

    return 0;
}

int check_next_memory_block(int padding_address)
{
    int padding_value = abs(*(int*)padding(padding_address));
    int* next_memory_block = (unsigned int*)padding(padding_address + FOOTER_SIZE);

    if (padding_address + padding_value >= *(int*)INIT_MEMORY_ADDRESS)
        return 0;

    else if (*next_memory_block > 0)
        return abs(*next_memory_block);

    return 0;
}

int memory_free(void* valid_ptr)
{
    int size_of_left_free_memory_block, size_of_right_free_memory_block, padding_left, padding_right, find_the_nearest_left, find_the_nearest_right, padding_addres_block_1;
    void* block_to_free_header_address = (char*)valid_ptr - HEADER_SIZE;
    *(int*)block_to_free_header_address = abs(*(int*)block_to_free_header_address);

    if ((int*)valid_ptr < (int*)INIT_MEMORY_ADDRESS || (int*)valid_ptr > ((int*)INIT_MEMORY_ADDRESS + *(int*)INIT_MEMORY_ADDRESS)) {
        printf("pamat mimo rozsahu\n");
        return 1;
    }

    padding_left = block_to_free_header_address - INIT_MEMORY_ADDRESS;
    padding_right = (block_to_free_header_address + *(int*)block_to_free_header_address - FOOTER_SIZE) - INIT_MEMORY_ADDRESS;

    size_of_left_free_memory_block = check_previous_memory_block(padding_left);
    size_of_right_free_memory_block = check_next_memory_block(padding_right);

    // ======= BLOKY =======
    if (size_of_left_free_memory_block && size_of_right_free_memory_block)
        padding_addres_block_1 = make_memory_block(padding_left - size_of_left_free_memory_block, *(int*)block_to_free_header_address + size_of_left_free_memory_block + size_of_right_free_memory_block, 1);

    else if (size_of_left_free_memory_block && !size_of_right_free_memory_block)
        padding_addres_block_1 = make_memory_block(padding_left - size_of_left_free_memory_block, *(int*)block_to_free_header_address + size_of_left_free_memory_block, 1);

    else if (!size_of_left_free_memory_block && size_of_right_free_memory_block)
        padding_addres_block_1 = make_memory_block(padding_left, *(int*)block_to_free_header_address + size_of_right_free_memory_block, 1);

    else {
        padding_addres_block_1 = make_memory_block(padding_left, *(int*)block_to_free_header_address, 1);
    }

    // ======= POINTRE =======

    // najdi najblizsi predchadzajuci
    find_the_nearest_left = find_previous_free_memory_block(padding_addres_block_1);
    find_the_nearest_right = find_next_free_memory_block(padding_addres_block_1);

    if (find_the_nearest_left <= HEADER_SIZE)
        *(int*)padding(find_the_nearest_left) = padding_addres_block_1;
    else
        *(int*)padding(find_the_nearest_left + HEADER_SIZE) = padding_addres_block_1;

    if (find_the_nearest_right)
        *(int*)padding(padding_addres_block_1 + HEADER_SIZE) = find_the_nearest_right;
    else
        *(int*)padding(padding_addres_block_1 + HEADER_SIZE) = 0;

    return 0;
}

void memory_init(void* ptr, unsigned int size)
{
    INIT_MEMORY_ADDRESS = (char*)ptr;

    if (size < 4 * (sizeof(unsigned int)))
        return;

    *(unsigned int*)INIT_MEMORY_ADDRESS = size;
    printf("adresa MB: %d, velkost MB: %d \n", (int*)INIT_MEMORY_ADDRESS, *((int*)INIT_MEMORY_ADDRESS));

    // pocet bytov k prvemu volnemu bloku
    *(unsigned int*)padding(HEADER_SIZE) = 2 * HEADER_SIZE;

    // inicializacia headera bloku pamate
    *(unsigned int*)padding(FOOTER_HEADER_SIZE) = size - FOOTER_HEADER_SIZE;

    // najblizsi volny priestor pre data
    *(unsigned int*)padding(FOOTER_HEADER_SIZE + HEADER_SIZE) = 0;

    // inicializacia footera bloku pamate
    *(unsigned int*)padding(size - FOOTER_SIZE) = size - FOOTER_HEADER_SIZE;
}

int memory_check(void* ptr)
{
    int akt_padding_address = FOOTER_HEADER_SIZE;

    while (akt_padding_address > HEADER_SIZE && akt_padding_address + abs(*(int*)padding(akt_padding_address)) <= *(int*)INIT_MEMORY_ADDRESS) {
        if ((int*)padding(akt_padding_address) <= (int*)ptr && padding(akt_padding_address) + abs(*(int*)padding(akt_padding_address)) >= ptr) {
            if (*(int*)padding(akt_padding_address) < 0) {
                return 1;
            }
            else {
                return 0;
            }
        }
        akt_padding_address = akt_padding_address + abs(*(int*)padding(akt_padding_address));
    }

    return 0;
}

void print_memory_blocks_in_region(char region[])
{
    int akt_address = FOOTER_HEADER_SIZE;

    printf("\nPRINT REGION --> super pointer: %d ", abs(*(unsigned int*)padding(akt_address - HEADER_SIZE)));

    while (abs(*(int*)padding(akt_address)) > HEADER_SIZE && akt_address < *(int*)INIT_MEMORY_ADDRESS) {
        printf("| %d xx%dxx %d ", *(int*)padding(akt_address), akt_address, *(int*)padding(akt_address));
        akt_address = akt_address + abs(*(int*)padding(akt_address));
    }
    printf("\n");
}

void memory_alloc_check(char* pointer)
{
    int padding_left, padding_left_value;

    padding_left = (void*)pointer - INIT_MEMORY_ADDRESS - 4;
    padding_left_value = *(int*)padding(padding_left);

    printf("MEMORY ALLOC --> super pointer %d address header: %d address footer: %d value header: %d value footer: %d next 4B: %d\n", *((int*)INIT_MEMORY_ADDRESS + 1), padding_left, padding_left - FOOTER_SIZE + abs(*(int*)padding(padding_left)), *(int*)padding(padding_left), *(int*)padding(padding_left + abs(padding_left_value) - FOOTER_SIZE), *(int*)padding((void*)pointer - INIT_MEMORY_ADDRESS));
}

int generate_numbers_for_testing(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void test_medium_memory_checks(char* pointer1, char* pointer2, char* pointer3, char* pointer4, char* pointer5, char* pointer6)
{
    printf("MEMORY CHECK --> pointer1 :%d | pointer2 :%d | pointer3 :%d | pointer4 :%d | pointer5 :%d | pointer6 :%d\n", memory_check(pointer1), memory_check(pointer2), memory_check(pointer3), memory_check(pointer4), memory_check(pointer5), memory_check(pointer6));
}

void test_small_memory_checks(char* pointer1, char* pointer2, char* pointer3)
{
    printf("MEMORY CHECK --> pointer1 :%d | pointer2 :%d | pointer3 :%d\n", memory_check(pointer1), memory_check(pointer2), memory_check(pointer3));
}

void test_maly()
{
    int random_region_number = 200;
    char arr[random_region_number], *pointer, *pointer2, *pointer3;

    memory_init(arr, random_region_number);

    pointer = (char*)memory_alloc(8);
    memory_alloc_check(pointer);

    pointer2 = (char*)memory_alloc(30);
    memory_alloc_check(pointer2);

    pointer3 = (char*)memory_alloc(130);
    memory_alloc_check(pointer3);

    print_memory_blocks_in_region(arr);
    test_small_memory_checks(pointer, pointer2, pointer3);

    if (pointer) {
        memory_free(pointer);
    }
    print_memory_blocks_in_region(arr);
    test_small_memory_checks(pointer, pointer2, pointer3);

    if (pointer3) {
        memory_free(pointer3);
    }
    print_memory_blocks_in_region(arr);
    test_small_memory_checks(pointer, pointer2, pointer3);

    if (pointer2) {
        memory_free(pointer2);
    }
    print_memory_blocks_in_region(arr);
    test_small_memory_checks(pointer, pointer2, pointer3);
}

void test_stredny()
{
    int random_region_number = generate_numbers_for_testing(24, 50000);
    char arr[random_region_number], *pointer1, *pointer2, *pointer3, *pointer4, *pointer5, *pointer6;

    memory_init(arr, random_region_number);

    pointer1 = (char*)memory_alloc(8);
    memory_alloc_check(pointer1);

    pointer2 = (char*)memory_alloc(30);
    memory_alloc_check(pointer2);

    pointer3 = (char*)memory_alloc(130);
    memory_alloc_check(pointer3);

    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    if (pointer1) {
        memory_free(pointer1);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    pointer1 = (char*)memory_alloc(8);
    memory_alloc_check(pointer1);
    print_memory_blocks_in_region(arr);

    if (pointer3) {
        memory_free(pointer3);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    pointer3 = (char*)memory_alloc(8);
    memory_alloc_check(pointer3);
    print_memory_blocks_in_region(arr);

    if (pointer2) {
        memory_free(pointer2);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    pointer2 = (char*)memory_alloc(8);
    memory_alloc_check(pointer2);
    print_memory_blocks_in_region(arr);

    pointer4 = (char*)memory_alloc(8);
    memory_alloc_check(pointer4);
    print_memory_blocks_in_region(arr);

    pointer5 = (char*)memory_alloc(8);
    memory_alloc_check(pointer5);

    pointer6 = (char*)memory_alloc(15000);
    memory_alloc_check(pointer6);
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);
}

void test_velky()
{
    int blok_1, blok_2, blok_3, blok_4, random_region_number = generate_numbers_for_testing(24, 50000);
    char arr[random_region_number];
    char *pointer1, *pointer2, *pointer3, *pointer4, *pointer5, *pointer6;

    memory_init(arr, random_region_number);

    blok_1 = generate_numbers_for_testing(8, 2000);
    pointer1 = (char*)memory_alloc(blok_1);
    memory_alloc_check(pointer1);

    blok_2 = generate_numbers_for_testing(8, 2000);
    pointer2 = (char*)memory_alloc(blok_2);
    memory_alloc_check(pointer2);

    blok_3 = generate_numbers_for_testing(8, 2000);
    pointer3 = (char*)memory_alloc(blok_3);
    memory_alloc_check(pointer3);
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    if (pointer1) {
        memory_free(pointer1);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    pointer4 = (char*)memory_alloc(8);
    memory_alloc_check(pointer4);
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    pointer5 = (char*)memory_alloc(8);
    memory_alloc_check(pointer5);
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    if (pointer4) {
        memory_free(pointer4);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    if (pointer5) {
        memory_free(pointer5);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    pointer6 = (char*)memory_alloc(blok_1);
    memory_alloc_check(pointer6);
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    pointer4 = (char*)memory_alloc(random_region_number - FOOTER_HEADER_SIZE);
    if (pointer4) {
        memory_alloc_check(pointer4);
        print_memory_blocks_in_region(arr);
        test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);
    }

    if (pointer3) {
        memory_free(pointer3);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    if (pointer2) {
        memory_free(pointer2);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    if (pointer6) {
        memory_free(pointer6);
    }
    print_memory_blocks_in_region(arr);
    test_medium_memory_checks(pointer1, pointer2, pointer3, pointer4, pointer5, pointer6);

    if (pointer4) {
        memory_free(pointer4);
    }
    print_memory_blocks_in_region(arr);
}

void testing_enviroment()
{
    int choice;

    printf("======= Vitajte v testovacom prostredi pamatoveho manazmentu! =======\n");
    printf("Prosim vyberte si test (cislo 0-6):\n");
    printf("\t- 0. Maly test\n");
    printf("\t- 1. Stredny test\n");
    printf("\t- 2. Velky test\n");
    printf("\t- 3. Maly a stredny test\n");
    printf("\t- 4. Stredny a velky test\n");
    printf("\t- 5. Vsetky testy -> maly + stredny + velky\n");
    printf("\t- 6. Nechcem testovat\n");

    scanf("%d", &choice);

    switch (choice) {
    case 0:
        printf("======= TEST MALY =======\n");
        test_maly();
        break;
    case 1:
        printf("======= TEST STREDNY =======\n");
        test_stredny();
        break;
    case 2:
        printf("======= NEXT VELKY =======\n");
        test_velky();
        break;
    case 3:
        printf("======= TEST MALY =======\n");
        test_maly();
        printf("======= TEST STREDNY =======\n");
        test_stredny();
        break;
    case 4:
        printf("======= TEST STREDNY =======\n");
        test_stredny();
        printf("======= NEXT VELKY =======\n");
        test_velky();
        break;
    case 5:
        printf("======= TEST MALY =======\n");
        test_maly();
        printf("======= TEST STREDNY =======\n");
        test_stredny();
        printf("======= NEXT VELKY =======\n");
        test_velky();
        break;
    default:
        break;
    }
}

int main()
{
    testing_enviroment();

    return 0;
}