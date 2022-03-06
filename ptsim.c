#include "ptsim.h"

// Simulated RAM
unsigned char mem[MEM_SIZE];
int printControl = 1;

int isPageTableFull(int addr, int proc) {
    int full = 0;
    if(addr == 0xff) {
        printf("OOM: proc %d: page table\n", proc);
        full = 1;
    }
    return full;
}

unsigned char get_page_table(int proc_num) {
    (void)proc_num;
    char x = 'a';
    return x;
}

int getFreeBit(int address) {
    return get_address(0, address);
}

int getPageTableAddress(int proc_num) {
    return get_address(0, PAGE_COUNT + proc_num);
}

int getProcessPageTable(int proc_num) {
    printf("getProcessPageTable: %d\n", mem[getPageTableAddress(proc_num)]);
    return mem[getPageTableAddress(proc_num)];
}

/*
    Convert a page, offset into an address
*/
int get_address(int page, int offset)
{
    if(printControl) printf("Address: %d page: %d offset: %d\n", (page << PAGE_SHIFT) | offset, page, offset);

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
    mem[getFreeBit(0)] = 1;
    mem[getPageTableAddress(0)] = 0;
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
            mem[i] = 1; page = i; break;
        }
    }
    printf("get_page: %d\n", page);
    return page;
}

/*
    Allocate pages for a new process

    This includes the new process page table and page_count data pages.
*/
void new_process(int proc_num, int page_count)
{
    int page_addr = get_page();
    if (isPageTableFull(page_addr, proc_num)) return;

    mem[getPageTableAddress(proc_num)] = page_addr;
    printf("proc%d page table address: %d\n", proc_num, mem[getPageTableAddress(proc_num)]);
    for(int i=0;i<page_count;i++) {
        page_addr = get_page();
        if (isPageTableFull(page_addr, proc_num)) return;
        mem[get_address(mem[getPageTableAddress(proc_num)], i)] = page_addr;
        printf("proc%d page table address: %d\n", proc_num, mem[getPageTableAddress(proc_num)]);

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
        fprintf(stderr, "usage: ptsim commands\n[np] proc_num page_count\n[pfm]\n[ppt]\n");
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
    }
}
