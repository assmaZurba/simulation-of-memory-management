#include <iostream>
#include "sim_mem.h"
int main() {
    char exe1 [20]="exec_file1";
    char exe2 [20]="exec_file2";
    char swapFile [20]="swap_file";
    sim_mem mem_sm(exe1,exe2,swapFile,25,25,25,
                   25,20,5,1);
    printf("The char is: %c\n",mem_sm.load(1,74));
    printf("The char is: %c\n",mem_sm.load(1,2));
    mem_sm.store(1,30,'A');
    mem_sm.store(1,80,'B');
    mem_sm.print_memory();
    mem_sm.print_swap();
    mem_sm.print_page_table();

    return 0;
}
