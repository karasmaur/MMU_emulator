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
    printf("MMU function\n");
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
        if(physical_memory[i][0] == pid && physical_memory[i][1]){
            //Found page on physical memory
            printf("Found page on physical memory: %d\n", physical_memory[i][1]);
            return physical_memory[i][1];
        }
    }

    // If page isn't in the physical memory, then it has to load it on it.
    for (int i = 0; i < SECONDARY_SIZE; ++i) {
        if(secondary_memory[i][0] == pid && secondary_memory[i][1] == page){
            //Checks if the physical memory has space to accomodate the page.
            for (int j = 0; j < PHYSICAL_SIZE; ++j) {
                if(physical_memory[i][0] == NULL){
                    return  move_page(j, i);
                } else {
                    int empty_space = find_victim();
                    return move_page(empty_space, i);
                }
            }
        } else {
            printf("Page %d wasn't found in secondary memory!\n", page);
            return 0;
        }
    }
}

int move_page(int primary, int secondary){
    physical_memory[primary][0] = secondary_memory[secondary][0];
    physical_memory[primary][1] = secondary_memory[secondary][2];
    secondary_memory[secondary][0] = 0;
    secondary_memory[secondary][1] = 0;

    printf("Page moved to physical memory!\n");

    return  physical_memory[primary][1];
}

int find_victim(){
    return 0;
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
        printf(" %d -  %d\n", secondary_memory[i][0], secondary_memory[i][1]);
    }

    (void) pthread_join(processes[0], NULL);

    return 0;
}