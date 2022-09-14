//
// Created by Assma zurba 323139782
//
#include "sim_mem.h"
char main_memory[MEMORY_SIZE];
queue<int> q;
int *pageOfFrame;
int*pageInSwap;
int swap_size;

sim_mem::sim_mem( char exe_file_name1[], char exe_file_name2[],char swap_file_name[],
                 int text_size,int data_size, int bss_size, int heap_stack_size,
                 int num_of_pages, int page_size, int num_of_process) {
    this->text_size=text_size;
    this->data_size=data_size;
    this->bss_size=bss_size;
    this->heap_stack_size=heap_stack_size;
    this->num_of_pages=num_of_pages;
    this->page_size=page_size;
    this->num_of_proc=num_of_process;
    this->program_fd[0]=open(exe_file_name1, O_RDONLY, 0);
    if(this->num_of_proc==2)
        this->program_fd[1]=open(exe_file_name2, O_RDONLY, 0);
    this->page_table=(page_descriptor**)malloc (sizeof(page_descriptor *)* this->num_of_proc);//the table of pointers
    this->swapfile_fd=open(swap_file_name, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
    int text_pages=this->text_size/this->page_size;
    for(int j=0;j<this->num_of_proc;j++){
        this->page_table[j]=(page_descriptor*)malloc(sizeof(page_descriptor)* this->num_of_pages);
        for(int i=0;i<this->num_of_pages;i++){
            page_table[j][i].V=0;
            page_table[j][i].frame=-1;
            if(i<text_pages)
                page_table[j][i].P=0;
            else
                page_table[j][i].P=1;
            page_table[j][i].D=0;
            page_table[j][i].swap_index=-1;
        }
    }
    for(int i=0;i<MEMORY_SIZE;i++)
        main_memory[i]='0';
    swap_size=this->num_of_proc*this->page_size*(this->num_of_pages-text_pages);
    for(int i=0;i<swap_size;i++){
        write(this->swapfile_fd,"0",1);
    }
    for(int i=0;i<MEMORY_SIZE/this->page_size;i++){
        q.push(i);
    }
    pageOfFrame=(int*) malloc(sizeof (int )*(MEMORY_SIZE/this->page_size));
    for(int i=0;i<MEMORY_SIZE/this->page_size;i++){
        pageOfFrame[i]=-1;
    }
    pageInSwap=(int*) malloc(sizeof (int )*(swap_size/this->page_size));
    for(int i=0;i<swap_size/this->page_size;i++){
        pageInSwap[i]=-1;
    }
}
/**************************************************************************************/
void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}
/************************************************************************************/
void sim_mem::print_swap() {
    char* str = (char*)malloc(this->page_size *sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) {
        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}
/***************************************************************************************/
void sim_mem::print_page_table() {
    int i;
    for (int j = 0; j < num_of_proc; j++) {
        printf("\n page table of process: %d \n", j+1);
        printf("Valid Dirty Permission Frame SwapIndex\n");
        for(i = 0; i < num_of_pages; i++) {
            printf("[%d]\t\t[%d]\t\t[%d]\t\t[%d]\t[%d]\n",
                   page_table[j][i].V,
                   page_table[j][i].D,
                   page_table[j][i].P,
                   page_table[j][i].frame ,
                   page_table[j][i].swap_index);
        }
    }
}
sim_mem::~sim_mem(){
    //close all the files and free all the allocated memory
    close(swapfile_fd);
    for(int i=0;i<this->num_of_proc;i++){
        free(this->page_table[i]);
        close(this->program_fd[i]);
    }
    free(this->page_table);
    free(pageOfFrame);
    free(pageInSwap);
}
char sim_mem:: load(int process_id, int address){
    int offset=address % this->page_size;
    int pageNum=address /this->page_size;
    if(this->page_table[process_id-1][pageNum].V==0){
        //Iam not in the memory!
        if(this->page_table[process_id-1][pageNum].P==0){
            //text position: copy page from exe
            copyPageFromExe(process_id,address);
            return main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset];
        }
        else{//p=1
            if(this->page_table[process_id-1][pageNum].D==0) {//d=0
                if(pageNum>=(text_size+data_size+bss_size)/page_size){
                    // heapStack page position: ERROR
                    fprintf(stderr,"you should store first!\n");
                    return '\0';
                }
                else if(pageNum>=(text_size+data_size)/page_size && pageNum<(text_size+data_size+bss_size)/page_size ){
                    //bss position: initial new page
                    initialNewPage(pageNum,process_id);
                    return main_memory[(this->page_table[process_id - 1][pageNum].frame * this->page_size) + offset];
                }
                else{
                    //text, data page position: copy page from exe
                    copyPageFromExe(process_id,address);
                    return main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset];
                }
            }
            else{//d=1
                //bring page from swap
                bringFromSwap(pageNum,process_id);
                return main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset];
            }
        }
    }
    else{//V=1
        return main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset];
    }
}
void sim_mem::store(int process_id, int address, char value){
    int offset=address % this->page_size;
    int pageNum=address /this->page_size;
    if(this->page_table[process_id-1][pageNum].V==0){
        //Iam not in the memory!
        if(this->page_table[process_id-1][pageNum].P==0){
            //you can't write in a text page!
            fprintf(stderr,"you can't write in a text page!\n");
        }
        else{//p=1
            if(this->page_table[process_id-1][pageNum].D==0) {//d=0
                if(pageNum>=(text_size+data_size)/page_size){
                    //bss,heapStack page position: initial new page
                    initialNewPage(pageNum,process_id);
                    main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset]=value;
                    this->page_table[process_id - 1][pageNum].D=1;
                }
                else{
                    //data page position: copy page from exe
                    copyPageFromExe(process_id,address);
                    main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset]=value;
                    this->page_table[process_id - 1][pageNum].D=1;
                }
            }
            else{//d=1
                //bring page from swap
                bringFromSwap(pageNum,process_id);
                main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset]=value;
                this->page_table[process_id - 1][pageNum].D=1;
            }
        }
    }
    else{//V=1
        //I am in the memory
        main_memory[(this->page_table[process_id - 1][pageNum].frame *this->page_size)+offset]=value;
        this->page_table[process_id - 1][pageNum].D=1;
    }
}
void sim_mem::copyPageFromExe(int process_id, int address){
    int pageNum=address /this->page_size;
    int emptyFrame=q.front();
    q.pop();
    q.push(emptyFrame);
    int memoryIndex=emptyFrame* this->page_size;
    char *copyString=(char *) malloc(sizeof (char) * this->page_size);
    if(pageOfFrame[emptyFrame]!=-1){
        //full memory
        int oldPageNum=pageOfFrame[emptyFrame];
        int oldPageProcId;
        for(int i=0;i<num_of_proc;i++){
            if(page_table[i][oldPageNum].frame==emptyFrame){
                oldPageProcId=i+1;
                break;
            }
        }
        this->page_table[oldPageProcId-1][oldPageNum].frame=-1;
        this->page_table[oldPageProcId-1][oldPageNum].V=0;
        if(page_table[oldPageProcId-1][oldPageNum].D==1){
            //if the old page is dirty save it in the swap file
            savePageInSwap(oldPageProcId,memoryIndex,oldPageNum);
        }
    }
    //write the newPage in the main memory
    strncpy(copyString,"",1);
    lseek(this->program_fd[process_id-1], pageNum*page_size, SEEK_SET); // go to the address in the exe file
    read(this->program_fd[process_id-1],copyString,page_size);//read from exe file
    this->page_table[process_id - 1][pageNum].frame = (memoryIndex ) / this->page_size;//update frame
    this->page_table[process_id - 1][pageNum].V=1;//update valid
    for(int i=0,j=memoryIndex;i<page_size;i++,j++){ //fill main memory
        main_memory[j]=copyString[i];
    }
    pageOfFrame[emptyFrame]=pageNum;
    free(copyString);
}
void sim_mem::initialNewPage(int pageNum,int process_id){
    int emptyFrame=q.front();
    q.push(emptyFrame);
    q.pop();
    int memoryIndex=emptyFrame* this->page_size;
    char *copyString=(char *) malloc(sizeof (char) * this->page_size);
    if(pageOfFrame[emptyFrame]!=-1) {
        //full memory
        int oldPageNum = pageOfFrame[emptyFrame];
        int oldPageProcId;
        for (int i = 0; i < num_of_proc; i++) {
            if (page_table[i][oldPageNum].frame == emptyFrame)
                oldPageProcId = i + 1;
        }
        this->page_table[oldPageProcId - 1][oldPageNum].frame = -1;
        this->page_table[oldPageProcId - 1][oldPageNum].V = 0;
        if (page_table[oldPageProcId - 1][oldPageNum].D == 1) {
            //if the old page is dirty save it in the swap file
            savePageInSwap(oldPageProcId,memoryIndex,oldPageNum);
        }
    }
    this->page_table[process_id - 1][pageNum].frame = memoryIndex / this->page_size;//update frame
    this->page_table[process_id - 1][pageNum].V=1;//update valid
    pageOfFrame[emptyFrame]=pageNum;
    for(int i=0,j=memoryIndex;i<page_size;i++,j++){ //fill the memory
        main_memory[j]='0';
    }
    free(copyString);
}
void sim_mem:: bringFromSwap(int pageNum,int process_id){
    char *copyString=(char *) malloc(sizeof (char) * this->page_size);
    int emptyFrame=q.front();
    q.pop();
    q.push(emptyFrame);
    int memoryIndex=emptyFrame* this->page_size;
    if(pageOfFrame[emptyFrame]!=-1){
        //full memory
        int oldPageNum=pageOfFrame[emptyFrame];
        int oldPageProcId;
        for(int i=0;i<num_of_proc;i++){
            if(page_table[i][oldPageNum].frame==emptyFrame){
                oldPageProcId=i+1;
                break;
            }
        }
        this->page_table[oldPageProcId-1][oldPageNum].frame=-1;
        this->page_table[oldPageProcId-1][oldPageNum].V=0;
        if(this->page_table[oldPageProcId-1][oldPageNum].D==1){
            //if the old page is dirty save it in the swap file
            savePageInSwap(oldPageProcId,memoryIndex,oldPageNum);
        }
    }
    //write the newPage in the main memory
    int swapIndex=this->page_table[process_id-1][pageNum].swap_index;
    lseek(swapfile_fd, swapIndex, SEEK_SET); // go to the address in the swap file
    read(swapfile_fd,copyString,page_size);//read from swap file
    for(int i=0;i<page_size;i++){
        lseek(swapfile_fd, swapIndex+i, SEEK_SET); // go to the address in the swap file
        write(swapfile_fd,"0",1);//write 0 in the place
    }
    page_table[process_id-1][pageNum].swap_index=-1;
    pageInSwap[swapIndex/page_size]=-1;
    this->page_table[process_id - 1][pageNum].frame = (memoryIndex) / this->page_size;//update frame
    this->page_table[process_id - 1][pageNum].V=1;//update valid
    pageOfFrame[emptyFrame]=pageNum;
    for(int i=0,j=memoryIndex;i<page_size;i++,j++){ //fill main memory
        main_memory[j]=copyString[i];
    }
    free(copyString);
}
void sim_mem:: savePageInSwap(int process_id, int memoryIndex, int pageNum){
    int swapIndex;
    char *copyString=(char *) malloc(sizeof (char )*page_size);
    for (int i = 0, j = memoryIndex; i < page_size; i++, j++) {
        //copy the old page from the memory
        copyString[i] = main_memory[j];
    }
    for (int i = 0; i < (swap_size / this->page_size); i++) {
        if (pageInSwap[i] == -1) {
            swapIndex = i * page_size;
            break;
        }
    }
    lseek(swapfile_fd, swapIndex, SEEK_SET);
    write(swapfile_fd, copyString, page_size);
    this->page_table[process_id - 1][pageNum].swap_index = swapIndex;
    pageInSwap[swapIndex / page_size] = pageNum;
    free(copyString);
}
