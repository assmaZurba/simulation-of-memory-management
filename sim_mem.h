//
// Created by Assma zurba 323139782
//
#ifndef OPEX5_SIM_MEM_H
#define OPEX5_SIM_MEM_H
#define MEMORY_SIZE 200
#include <iostream>
#include <malloc.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <queue>
using namespace std;

extern char main_memory[MEMORY_SIZE];
extern queue<int> q;//a queue for the frames (fifo)
extern int *pageOfFrame;//an array, the frame of the i-th page
extern int *pageInSwap;//an array, the swap frame of the i-th page
extern int swap_size;//the num of chars in the swap file

typedef struct page_descriptor{
    int V; //valid
    int D; //dirty
    int P; //permission
    int frame; // the number of frame if in case it is page_mapped
    int swap_index; //where the page is located in the swap file
}page_descriptor;

class sim_mem{
    int swapfile_fd; //swap file fd
    int program_fd[2]; //array that contains the paths of the executable files
    int text_size;//the num of chars that are allocated for text-type
    int data_size;//the num of chars that are allocated for data-type
    int bss_size;//the num of chars that are allocated for bss-type
    int heap_stack_size;//the num of chars that are allocated for heap and stack type
    int num_of_pages;
    int page_size;//the num of chars in each pge
    int num_of_proc;//the num of process
    page_descriptor ** page_table; //pointer to the page table

    public:

        sim_mem(char exe_file_name1[],char exe_file_name2[],char swap_file_name[],
                int text_size,int data_size,int bss_size,int heap_stack_size,
                int num_of_pages,int page_size,int num_of_process);
        //the constructor
        ~sim_mem();
        //the destructor
        char load(int process_id, int address);
        //load a char from the address
        void store(int process_id, int address, char value);
        //store a char in the address
        void print_memory();
        void print_swap ();
        void print_page_table();
        void copyPageFromExe(int process_id, int address);
        //copy a page from the executable file
        void initialNewPage(int pageNum,int process_id);
        //initial a new page and put it the main memory
        void bringFromSwap(int pageNum,int process_id);
        //bring a page from the swap file and write it in the main memory
        void savePageInSwap(int process_id, int memoryIndex, int pageNum);
        //save a page in the swap file
};




#endif //OPEX5_SIM_MEM_H
