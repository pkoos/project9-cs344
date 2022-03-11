#include "ptsim.h"

// Simulated RAM
unsigned char mem[MEM_SIZE];
int printControl = 1;

void loadValue(int proc_num, int virt_addr) {
    (void)proc_num; 
    (void)virt_addr;
}

void storeValue(int proc_num, int virt_addr, int value) {
    (void)proc_num;
    (void)virt_addr;
    (void)value;
}

void killProcess(int proc_num) {
    (void)proc_num;
}

int verifyProcAndPage(int proc_num, int page_count) {
    int error = 1;
    if(!(proc_num < PAGE_COUNT && proc_num > 0)) {
        printf("Requested process %d is not valid. [1-64]\n", proc_num);
        error = 0;
    }
    if(!(page_count < PAGE_COUNT && page_count > 0)) {
        printf("Requested %d pages is not valid. [1-64]\n", page_count);
        error = 0;
    }
    return error;
}

int isPageTableFull(int addr, int proc) {
    int full = 0;
    if(addr == 0xff) {
        printf("OOM: proc %d: page table\n", proc);
        full = 1;
    }
    return full;
}

int pageTableAddress(int proc_num) {
    return get_address(0, PAGE_COUNT + proc_num);
}

unsigned char get_page_table(int proc_num) {
    return mem[pageTableAddress(proc_num)];
}

int freeBit(int address) {
    return get_address(0, address);
}

/*
    Convert a page, offset into an address
*/
int get_address(int page, int offset)
{
    return (page << PAGE_SHIFT) | offset;
}

/*
    Initialize RAM
*/
void initialize_mem(void)
{
    for(int i = 0;i<MEM_SIZE;i++) {
        mem[i] = 0;
    }
    mem[freeBit(0)] = 1;
    mem[pageTableAddress(0)] = 0;
}

/*
    Allocate a physical page

    Returns the number of the page, or 0xff if no more pages available
*/
unsigned char get_page(void)
{
    int page = 0xff;
    for(int i=1;i<PAGE_COUNT;i++) {
        if(!mem[i]) {
            mem[freeBit(i)] = 1; page = i; break;
        }
    }
    return page;
}

/*
    Allocate pages for a new process

    This includes the new process page table and page_count data pages.
*/
void new_process(int proc_num, int page_count)
{
    if (!verifyProcAndPage(proc_num, page_count)) {
        return;
    }

    int page_addr = get_page();
    if (isPageTableFull(page_addr, proc_num)) return;

    mem[pageTableAddress(proc_num)] = page_addr;
    
    for(int i=0;i<page_count;i++) {
        page_addr = get_page();
        if (isPageTableFull(page_addr, proc_num)) return;
        mem[get_address(mem[pageTableAddress(proc_num)], i)] = page_addr;

    }
}

/*
    Print the free page map
*/
void print_page_free_map(void)
{
    printControl = 0;

    for (int i = 0; i < 64; i++) {
        int addr = get_address(0, i);

        printf("%c", mem[addr] == 0? '.': '#');

        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
    printControl = 1;
}

/*
    Print the address map from virtual pages to physical
*/
void print_page_table(int proc_num)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", proc_num);
    printControl = 0;
    // Get the page table for this process
    int page_table = get_page_table(proc_num);

    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++) {
        int addr = get_address(page_table, i);

        int page = mem[addr];

        if (page != 0) {
            printf("%02x -> %02x\n", i, page);
        }
    }
    printControl = 1;
}

int main(int argc, char *argv[])
{
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);

    if (argc == 1) {
        fprintf(stderr, "usage: ptsim commands\n -> [np] proc_num req_pages\n -> [pfm]\n -> [ppt]\n -> [kp] proc_num\n -> [st] proc_num virt_addr value\n -> [lb] proc_num virt_addr\n");
        return 1;
    }
    
    initialize_mem();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "np") == 0) {
            int proc_num = atoi(argv[++i]);
            int pages = atoi(argv[++i]);
            new_process(proc_num, pages);
        }
        else if (strcmp(argv[i], "pfm") == 0) {
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0) {
            int proc_num = atoi(argv[++i]);
            print_page_table(proc_num);
        }
        else if (strcmp(argv[i], "kp") == 0) {
            int proc_num = atoi(argv[++i]);
            killProcess(proc_num);
        }
        else if (strcmp(argv[i], "st") == 0) {
            int proc_num = atoi(argv[++i]);
            int virt_addr = atoi(argv[++i]);
            int value = atoi(argv[++i]);
            storeValue(proc_num, virt_addr, value);
        }
        else if (strcmp(argv[i], "lb") == 0) {
            int proc_num = atoi(argv[++i]);
            int virt_addr = atoi(argv[++i]);
            loadValue(proc_num, virt_addr);
        }
    }
}
