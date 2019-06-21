#include <stdio.h>
#include <pthread.h>

// Global variables:
char virtual_memory[128]; // 1 MB > 128 pages
char physical_memory[8]; // 64k > 8 pgs
char secondary_memory[120];

// Functions:
void mmu(int pid, int pages){

}

void *process(void *id){
    // Creates the process and uses the secondary_memory to store the data, each position of the array it's a page.
    // Randomly the process will request a page from the MMU so it can execute from memory.
    int pages = 12;
    int pid = (int *)id;

    mmu(pid, pages);

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