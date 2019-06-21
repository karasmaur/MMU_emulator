#include <stdio.h>
#include <pthread.h>

void *process(){
    // Creates the process and uses the secondary_memory to store the data, each position of the array it's a page.
    // Randomly the process will request a page from the MMU so it can execute from memory.
}

int main() {
    char virtual_memory[128]; // 1 MB > 128 pages
    char physical_memory[8]; // 64k > 8 pgs
    char secondary_memory[120];




    return 0;
}