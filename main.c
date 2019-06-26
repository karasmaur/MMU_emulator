/*
 *  Author: Mauricio Ricardo Karas
 *  Class: Operating Systems Lab.
 *  Title: MMU Simulator.
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define TOTAL_PROCESSES 2
#define VIRTUAL_SIZE 128
#define PHYSICAL_SIZE 8
#define SECONDARY_SIZE 120
#define RELOCATION_REGISTER 14000

// Global variables:
int virtual_memory[VIRTUAL_SIZE][2]; // 1 MB > 128 pages
int physical_memory[PHYSICAL_SIZE][3]; // 64k > 8 pgs
int secondary_memory[SECONDARY_SIZE][2];
int indexing_table[VIRTUAL_SIZE][3]; // Contains the translation of addresses from virtual to physical memory.

pthread_mutex_t mutex;
pthread_cond_t control;

// Functions:
void mmu_create_pages(int pid, int pages){
    for (int i = 0; i < pages; i++) {
        // Virtual Memory
        for (int j = 0; j < VIRTUAL_SIZE; j++) {
            if(virtual_memory[j][0] == 0){
                virtual_memory[j][0] = pid; // Process ID
                virtual_memory[j][1] = i; // Page address in virtual mem.
                break;
            }
        }
        // Secondary Memory
        for (int j = 0; j < SECONDARY_SIZE; j++) {
            if(secondary_memory[j][0] == 0){
                secondary_memory[j][0] = pid; // Process ID
                secondary_memory[j][1] = i + RELOCATION_REGISTER; //Page address in physical mem.
                break;
            }
        }
        // Creating index address table
        for (int j = 0; j < VIRTUAL_SIZE; j++) {
            if(indexing_table[j][0] == 0){
                indexing_table[j][0] = pid; // Process ID
                indexing_table[j][1] = i; // Page address in virtual mem.
                indexing_table[j][2] = i + RELOCATION_REGISTER; // Page address in physical mem.
                break;
            }
        }
    }
}

int translate_page_address(int pid, int page){
    for (int i = 0; i < VIRTUAL_SIZE; ++i) {
        if(indexing_table[i][0] == pid && indexing_table[i][1] == page){
            return indexing_table[i][2];
        }
    }
    return 0;
}

int mmu_load(int pid, int page){

    page = translate_page_address(pid, page);

    // Search physical memory for the page
    for (int i = 0; i < PHYSICAL_SIZE; ++i) {
        if(physical_memory[i][0] == pid && physical_memory[i][1] == page){
            //Found page on physical memory
            printf("PID: %d, Found page on physical memory: %d\n", pid, physical_memory[i][1]);
            physical_memory[i][2]++; // Increments the use count.
            return 1;
        }
    }

    printf("PID: %d, Page %d is not in physical memory!\n", pid, page);
    // If page isn't in the physical memory, then it has to load it on it.
    for (int i = 0; i < SECONDARY_SIZE; ++i) {
        if(secondary_memory[i][0] == pid && secondary_memory[i][1] == page){
            //Checks if the physical memory has space to accommodate the page.
            for (int j = 0; j < PHYSICAL_SIZE; j++) {
                if(physical_memory[j][0] == 0){
                    swap_in_mem(j, i);
                    return 1;
                }
            }
            // With no space in the physical memory, we have to find a victim to remove.
            int empty_space = find_victim();
            swap_in_mem(empty_space, i);
            return 1;
        }
    }

    printf("PID: %d, Page %d wasn't found in secondary memory!\n", pid, page);
    return 0;
}

int swap_in_mem(int prim_position, int secon_position){
    // Swaps in pages from secondary memory into the physical primary memory.
    physical_memory[prim_position][0] = secondary_memory[secon_position][0]; // PID
    physical_memory[prim_position][1] = secondary_memory[secon_position][1]; // Page
    physical_memory[prim_position][2] = 0; // Uses counter

    secondary_memory[secon_position][0] = 0; // PID
    secondary_memory[secon_position][1] = 0; // Page

    printf("PID: %d, Page %d moved to physical memory!\n", physical_memory[prim_position][0],
           physical_memory[prim_position][1]);

    return  physical_memory[prim_position][1];
}

int swap_out_mem(int prim_position, int secon_position){
    // Swaps out pages from physical memory into secondary memory.
    secondary_memory[secon_position][0] = physical_memory[prim_position][0]; // PID
    secondary_memory[secon_position][1] = physical_memory[prim_position][1]; // Page
    physical_memory[prim_position][0] = 0; // PID
    physical_memory[prim_position][1] = 0; // Page
    physical_memory[prim_position][2] = 0;  // Uses counter

    return secondary_memory[secon_position][1];
}

int find_victim(){
    pthread_mutex_lock(&mutex);

    int most_unused_page[2];
    most_unused_page[0] = physical_memory[0][0];
    most_unused_page[1] = physical_memory[0][2];

    for (int i = 0; i < PHYSICAL_SIZE; ++i) {
        if(physical_memory[i][2] < most_unused_page[1]){
            most_unused_page[0] = i;
            most_unused_page[1] = physical_memory[i][2];
        }
    }

    printf("Found Victim to move out of memory: PID: %d, Page: %d, Uses: %d\n",
           physical_memory[most_unused_page[0]][0],  physical_memory[most_unused_page[0]][1],
           most_unused_page[1]);

    for (int i = 0; i < SECONDARY_SIZE; i++) {
        if(secondary_memory[i][0] == 0){
            swap_out_mem(most_unused_page[0], i);
        }
    }

    pthread_cond_signal(&control);
    pthread_mutex_unlock(&mutex);

    return most_unused_page[0];
}

int *process(void *id) {
    // Creates the process and uses the secondary_memory to store the data, each position of
    // the array it's a page.
    // Randomly the process will request a page from the MMU so it can execute from memory.
    int pages = 12;
    int pid = *((int *) id);

    mmu_create_pages(pid, pages);

    srand(time(NULL));

    // Requests the pages.
    while(1){
        sleep(rand() % 5);

        int random_page = rand() % pages;

        printf("PID: %d, Requesting page %d\n", pid, random_page);
        mmu_load(pid, random_page);
    }

    return 0;
}

// Main:
int main() {
    pthread_t processes[TOTAL_PROCESSES];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&control, NULL);

    int *pid = malloc(sizeof(int)*TOTAL_PROCESSES);
    for (int i = 0; i < TOTAL_PROCESSES; ++i) {
        pid[i] = i+1;
    }

    for (int i = 0; i < TOTAL_PROCESSES; ++i) {
        (void) pthread_create(&processes[i], NULL, process, (void *) &pid[i]);
    }

    // Requests the pages.
    while(1) {
        /*printf("Virtual Memory:\n");
        printf("PID - Page:\n");
        for (int i = 0; i < VIRTUAL_SIZE; i++) {
            printf(" %d - %d\n", virtual_memory[i][0], virtual_memory[i][1]);
        }*/

        printf("Physical Memory:\n");
        printf("PID - Page:\n");
        for (int i = 0; i < PHYSICAL_SIZE; i++) {
            printf(" %d - %d\n", physical_memory[i][0], physical_memory[i][1]);
        }

        printf("Secondary Memory(HD):\n");
        printf("PID - Page:\n");
        for (int i = 0; i < 30; i++) {
            printf(" %d - %d\n", secondary_memory[i][0], secondary_memory[i][1]);
        }

        sleep(3);
    }
    //(void) pthread_join(processes[0], NULL);

    return 0;
}