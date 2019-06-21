#include <stdio.h>
#include <pthread.h>

#define VIRTUAL_SIZE 128
#define PHYSICAL_SIZE 8
#define SECONDARY_SIZE 120

// Global variables:
int virtual_memory[VIRTUAL_SIZE][2]; // 1 MB > 128 pages
int physical_memory[PHYSICAL_SIZE][2]; // 64k > 8 pgs
int secondary_memory[SECONDARY_SIZE][2];

// Functions:
void mmu(int pid, int pages, int selector){
    // selector is used to define if the MMU will create pages or find them.
    // selector: 0 > Create pages// selector: 1 > Load page.
    if(selector == 0){
        // Virtual Memory
        for (int i = 0; i < pages; ++i) {
            for (int j = 0; j < VIRTUAL_SIZE; ++j) {
                if(virtual_memory[j] != NULL){
                    virtual_memory[j][0] = pid; // Process ID
                    virtual_memory[j][1] = i; // Page ID
                }
            }
        }

        // Secondary Memory
        for (int i = 0; i < pages; ++i) {
            for (int j = 0; j < SECONDARY_SIZE; ++j) {
                if(secondary_memory[j] != NULL){
                    secondary_memory[j][0] = pid; // Process ID
                    secondary_memory[j][1] = i; // Page ID
                }
            }
        }
    } else if(selector == 1){

    } else{
        printf("Wrong selector, use 0 to create page or 1 to load page into mem.\n");
    }

}

void *process(void *id){
    // Creates the process and uses the secondary_memory to store the data, each position of the array it's a page.
    // Randomly the process will request a page from the MMU so it can execute from memory.
    int pages = 12;
    int pid = (int *)id;

    mmu(pid, pages, 0);

}

// Main:
int main() {
    pthread_t processes[2];
    int pid[2];

    pid[0] = 1; // Setting pid of the first process to 1.

    (void) pthread_create(&processes[0], NULL, process, NULL);

    (void) pthread_join(processes[0], NULL);

    return 0;
}