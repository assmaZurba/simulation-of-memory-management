# simulation-of-memory-managment
written by Assma zurba

in general:
A simulation of memory managing in the computer.

the program contains a class that called sim_mem, the fields of the class are:
    int swapfile_fd: swap file fd.
    int program_fd[2]: array that contains the paths of the executable files.
    int text_size: the num of chars that are allocated for text-type.
    int data_size: the num of chars that are allocated for data-type.
    int bss_size: the num of chars that are allocated for bss-type.
    int heap_stack_size: the num of chars that are allocated for heap and stack type.
    int num_of_pages.
    int page_size: the num of chars in each page.
    int num_of_proc: the num of process.
    page_descriptor ** page_table: pointer to the page table.

the program contains struct that called page_descriptor, the fields of the struct are:
    int V: valid; if the page in the memory V=1, else V=0.
    int D: dirty; if the user make store before D=1, else D=0.
    int P: permission; if the page is text-type P=1, else P=0.
    int frame: the number of frame if in case it is page_mapped.
    int swap_index: where the page is located in the swap file.

the constructor of the class: sim_mem(char exe_file_name1[],char exe_file_name2[],char swap_file_name[],
                int text_size,int data_size,int bss_size,int heap_stack_size,
                int num_of_pages,int page_size,int num_of_process):
    fill all fields of the class.
    fill the main memory by 0.
    fill the swap file by 0.
    V=0 of all the pages.
    D=0 of all the pages.
    swap index=-1 of all the pages.
    frame =-1 of all the pages.
    P=0 if the page is text_type, else P=1.

the destructor of the class: ~sim_mem():
    close all the files and free all the allocated memory.

functions:

char load(int process_id, int address): load a char from the address:
    parameters:-
    process_id: the process id that contains the wanted char.
    address: the address of the wanted char:
        address/pages number=the num of the page of the wanted char.
        address%pages number=the offset of the wanted char in the page.

void store(int process_id, int address, char value):store a char in the address
    parameters:-
    process_id: the process id that contains the wanted char.
    address: the address of the wanted char:
        address/pages number=the num of the page of the wanted char
        address%pages number=the offset of the wanted char in the page

void print_memory().

void print_swap ().

void print_page_table().

void copyPageFromExe(int process_id, int address): copy a page from the executable file
    parameters:-
    process_id: the process id that contains the wanted char.
    address: the address of the wanted char:
        address/pages number=the num of the page of the wanted char.
        address%pages number=the offset of the wanted char in the page.

void initialNewPage(int pageNum,int process_id): initial a new page and put it the main memory
    parameters:-
        pageNum: the num of page in the page table.
        process_id: the process id that contains the wanted char.

void bringFromSwap(int pageNum,int process_id): bring a page from the swap file and write it in the main memory
    parameters:-
    pageNum: the num of page in the page table.
    process_id: the process id that contains the wanted char.


void savePageInSwap(int process_id, int memoryIndex, int pageNum): save a page in the swap file
    parameters:
    process_id: the process id that contains the wanted char.
    memoryIndex: the place of the page that we want to save it in the swap file.
    pageNum: the num of page in the page table.

to compile:
    g++ main.cpp sim_mem.cpp -o main
to run:
    ./main
