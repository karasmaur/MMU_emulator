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

#define VIRTUAL_SIZE 128
#define PHYSICAL_SIZE 8
#define SECONDARY_SIZE 120
#define RELOCATION_REGISTER 14000

// Global variables:
int virtual_memory[VIRTUAL_SIZE][2]; // 1 MB > 128 pages
int physical_memory[PHYSICAL_SIZE][3]; // 64k > 8 pgs
int secondary_memory[SECONDARY_SIZE][2];

int indexing_table[VIRTUAL_SIZE][VIRTUAL_SIZE];

// Functions:
void mmu_create(int pid, int pages){
    // Virtual Memory
    for (int i = 0; i < pages; i++) {
        for (int j = 0; j < VIRTUAL_SIZE; j++) {
            if(virtual_memory[j][0] == NULL){
                virtual_memory[j][0] = pid; // Process ID
                virtual_memory[j][1] = i; // Page ID
                break;
            }
        }
    }
    // Secondary Memory
    for (int i = 0; i < pages; i++) {
        for (int j = 0; j < SECONDARY_SIZE; j++) {
            if(secondary_memory[j][0] == NULL){
                secondary_memory[j][0] = pid; // Process ID
                secondary_memory[j][1] = i; // Page ID  -- + RELOCATION_REGISTER
                break;
            }
        }
    }
}

int mmu_load(int pid, int page){
    // Search physical memory for the page
    for (int i = 0; i < PHYSICAL_SIZE; ++i) {
        if(physical_memory[i][0] == pid && physical_memory[i][1] == page){
            //Found page on physical memory
            printf("Found page on physical memory: %d\n", physical_memory[i][1]);
            physical_memory[i][2]++ // Increments the use count.
            return physical_memory[i][1];
        }
    }

    // If page isn't in the physical memory, then it has to load it on it.
    for (int i = 0; i < SECONDARY_SIZE; ++i) {
        if(secondary_memory[i][0] == pid && secondary_memory[i][1] == page){
            //Checks if the physical memory has space to accomodate the page.
            for (int j = 0; j < PHYSICAL_SIZE; j++) {
                if(physical_memory[i][0] == NULL){
                    return swap_in_mem(j, i);
                }
            }
            // With no space in the physical memory, we have to find a victim to remove.
            int empty_space = find_victim();
            return swap_in_mem(empty_space, i);
        } else {
            printf("Page %d wasn't found in secondary memory!\n", page);
            return 0;
        }
    }
}

int swap_in_mem(int prim_position, int secon_position){
    // Swaps in pages from secondary memory into the physical primary memory.
    physical_memory[prim_position][0] = secondary_memory[secon_position][0]; // PID
    physical_memory[prim_position][1] = secondary_memory[secon_position][2]; // Page
    physical_memory[prim_position][2] = 0; // Last used counter

    secondary_memory[secon_position][0] = 0; // PID
    secondary_memory[secon_position][1] = 0; // Page

    printf("Page moved to physical memory!\n");

    return  physical_memory[prim_position][1];
}

int swap_out_mem(int prim_position, int secon_position){
    // Swaps out pages from physical memory into secondary memory.
    secondary_memory[secon_position][0] = physical_memory[prim_position][0]; // PID
    secondary_memory[secon_position][1] = physical_memory[prim_position][1]; // Page
    physical_memory[prim_position][0] = 0; // PID
    physical_memory[prim_position][1] = 0; // Page
    physical_memory[prim_position][2] = 0;  // Last used counter

    return secondary_memory[secon_position][1];
}

int find_victim(){
    int last_used_page[2];
    last_used_page[0] = 0;
    last_used_page[1] = 0;

    for (int i = 0; i < PHYSICAL_SIZE; ++i) {
        if(physical_memory[i][2] > last_used_page[1]){
            last_used_page[0] = i;
            last_used_page[1] = physical_memory[i][2];
        }
    }

    for (int i = 0; i < SECONDARY_SIZE; i++) {
        if(secondary_memory[i][0] == NULL){
            swap_out_mem(last_used_page[0], i)
        }
    }

    return last_used_page[0];
}

int *process(void *id) {
    // Creates the process and uses the secondary_memory to store the data, each position of
    // the array it's a page.
    // Randomly the process will request a page from the MMU so it can execute from memory.
    int pages = 12;
    //int pid = *((int *) id); // TODO: this is causing segmentation fault, SOLVE THIS!
    //printf("%d\n", pid);

    mmu_create(1, pages);

    srand(time(NULL));

    // Requests the pages.
    while(1){
        sleep(rand() % 5);

        int random_page = rand() % pages;

        printf("Requesting page %d\n", random_page);
        mmu_load(1, random_page);
    }

    return 0;
}

// Main:
int main() {
    pthread_t processes[2];
    int pid[2];

    pid[0] = 1; // Setting pid of the first process to 1.

    (void) pthread_create(&processes[0], NULL, process, (void*) pid[0]);

    // Requests the pages.
    while(1) {
        sleep(3);

        printf("Virtual Memory:\n");
        printf("PID - Page:\n");
        for (int i = 0; i < VIRTUAL_SIZE; i++) {
            printf(" %d -  %d\n", virtual_memory[i][0], virtual_memory[i][1]);
        }

        printf("Physical Memory:\n");
        printf("PID - Page:\n");
        for (int i = 0; i < PHYSICAL_SIZE; i++) {
            printf(" %d -  %d\n", physical_memory[i][0], physical_memory[i][1]);
        }

        printf("Secondary Memory(HD):\n");
        printf("PID - Page:\n");
        for (int i = 0; i < SECONDARY_SIZE; i++) {
            printf(" %d - %d\n", secondary_memory[i][0], secondary_memory[i][1]);
        }
    }
    (void) pthread_join(processes[0], NULL);

    return 0;
}