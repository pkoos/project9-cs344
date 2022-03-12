#include "ptsim.h"

#define RUN_PROGRAM 1

// Simulated RAM
unsigned char mem[MEM_SIZE];


/* 
    Input:
    page: The virtual page number.
    offset: The virtual address on said page number.

    Out:
    A physical address calculated from page and offset.
*/
int getPhysicalAddress(unsigned char page, unsigned char offset)
{
    // printf("page: %d offset: %d getPhysicalAddress: %d\n",page, offset, (page << PAGE_SHIFT) | offset);
    return (page << PAGE_SHIFT) | offset;
}

/*
	Input: A physical address.
	Output: The value assigned to the physical address.
*/
unsigned char getValue(int address) {
		assert(address < MEM_SIZE && address > 0);
		return mem[address];
}

/*
    Input: A physical address between 0-63.
    Output: The physical address passed in.
*/
unsigned char freeBit(int address) {
    
    assert(address < PAGE_COUNT && address >= 0);
    return getPhysicalAddress(0, address);
}
/*
    Input: A process number, validated to be between 0 - 63.
    Output: The page table address 
*/
unsigned char pageTableAddress(unsigned char procNum) {
    
    assert(procNum < PAGE_COUNT);
    return getPhysicalAddress(0, PAGE_COUNT + procNum);
}

int pageAddress(unsigned char page) {
    
    assert(page < PAGE_COUNT);
    return getPhysicalAddress(page, 0);
}

/*
    Input:
    A process number for the page table address you're looking up.

    Output: The address of the process number's page table.
*/
unsigned char processPageTable(unsigned char procNum) {
    
    assert(procNum < PAGE_COUNT);
    return mem[pageTableAddress(procNum)];
}

/*
    Input: 
    Address: a physical memory address.
    Value: A value you want to assign to memory.

    Validates memory address and value, and assigns the value
    in the memory address.
*/
void assignPhysicalMemory(int address, int value) {
    assert(address < MEM_SIZE && address >= 0);
    assert(value < MAX_ASSIGNABLE_VALUE && value >= 0);
    mem[address] = value;
}

void loadValue(int procNum, int virt_addr) {
    (void)procNum; 
    (void)virt_addr;
}

void storeValue(int procNum, int virt_addr, int value) {
    (void)procNum;
    (void)virt_addr;
    (void)value;
}

void killProcess(int procNum) {

    int ptAddress = processPageTable(procNum);
    printf("ptAddress: %d\n", ptAddress);
    // int pageTableAddr = get_page_table(procNum);
    // int procPageTable = processPageTable(procNum);
    // mem[freeBit(pageTableAddr)] = 0;
    // int procPageAddr;
    // while ((procPageAddr = mem[procPageTable++]) != 0) {
    //     mem[procPageAddr] = 0;
    // }
    // mem[pageTableAddress(procNum)] = 0;
}

int verifyProcAndPage(int procNum, int pageCount) {
    int error = 1;
    if(!(procNum < PAGE_COUNT && procNum >= 0)) {
        printf("Requested process %d is not valid. [0-63]\n", procNum);
        error = 0;
    }
    if(!(pageCount < PAGE_COUNT && pageCount >= 0)) {
        printf("Requested %d pages is not valid. [0-63]\n", pageCount);
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

// int pageTableAddress(int procNum) {
//     return getPhysicalAddress(0, pageCount + procNum);
// }

unsigned char get_page_table(int procNum) {
    printf("procNum: %d address: %d value: %d\n", procNum, pageTableAddress(procNum), mem[pageTableAddress(procNum)]);
    return mem[pageTableAddress(procNum)];
}

/*
    Initialize RAM
*/
void initializeMem(void)
{
    for(int i = 0;i<MEM_SIZE;i++) {
        assignPhysicalMemory(i, 0);
    }
    assignPhysicalMemory(freeBit(0), 1);
}

/*
    Allocate a virtual page

    Output: Virtual page number, or 0xff if no more virtual pages available
*/
unsigned char getVirtualPage(void)
{
    int page = 0xff;
    for(int i=1;i<PAGE_COUNT;i++) {
        if(!mem[i]) {
            assignPhysicalMemory(i, 1); page = i; break;
        }
    }
    return page;
}

/*
    Allocate pages for a new process

    This includes the new process page table and pageCount data pages.
*/
void newProcess(int procNum, int pageCount)
{
    if (!verifyProcAndPage(procNum, pageCount)) {
        return;
    }

    int pageAddr = getVirtualPage();
    printf("proc: %d page table: %d\n", procNum, pageTableAddress(procNum));
    if (isPageTableFull(pageAddr, procNum)) return;
    assignPhysicalMemory(pageTableAddress(procNum), pageAddr);
    
    for(int i=0;i<pageCount;i++) {
        int newPage = getVirtualPage();
        if (isPageTableFull(newPage, procNum)) return;
        assignPhysicalMemory(pageAddress(pageAddr)+i ,newPage);
        printf("%d: pageAddr: %d value: %d\n",i, pageAddress(pageAddr)+ i, mem[pageAddress(pageAddr)+i]);
    }
}

/*
    Print the address map from virtual pages to physical
*/
void print_page_table(int procNum)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", procNum);
    // Get the page table for this process
    int page_table = get_page_table(procNum);
    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++) {
        int addr = getPhysicalAddress(page_table, i);
        
        int page = mem[addr];
        printf("addr: %d page: %d\n", addr, page);

        if (page != 0) {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

/*
    Print the free page map
*/
void print_page_free_map(void)
{
    for (int i = 0; i < 64; i++) {
        int addr = getPhysicalAddress(0, i);

        printf("%c", mem[addr] == 0? '.': '#');
        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

/*
    Drives the program
*/
int main(int argc, char *argv[])
{
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);
#if RUN_PROGRAM
    if (argc == 1) {
        fprintf(stderr, "usage: ptsim commands\n -> [np] procNum req_pages\n -> [pfm]\n -> [ppt]\n -> [kp] procNum\n -> [st] procNum virt_addr value\n -> [lb] procNum virt_addr\n");
        return 1;
    }
    
    initializeMem();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "np") == 0) {
            int procNum = atoi(argv[++i]);
            int pages = atoi(argv[++i]);
            newProcess(procNum, pages);
        }
        else if (strcmp(argv[i], "pfm") == 0) {
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0) {
            int procNum = atoi(argv[++i]);
            print_page_table(procNum);
        }
        else if (strcmp(argv[i], "kp") == 0) {
            int procNum = atoi(argv[++i]);
            killProcess(procNum);
        }
        else if (strcmp(argv[i], "st") == 0) {
            int procNum = atoi(argv[++i]);
            int virt_addr = atoi(argv[++i]);
            int value = atoi(argv[++i]);
            storeValue(procNum, virt_addr, value);
        }
        else if (strcmp(argv[i], "lb") == 0) {
            int procNum = atoi(argv[++i]);
            int virt_addr = atoi(argv[++i]);
            loadValue(procNum, virt_addr);
        }
    }
#else

    /* 
        Unit testing in C ?!?!? Inconceivable! 
    */

    assert(freeBit(17) == 17);
    printf("freeBit(17) passed! Value: %d\n", freeBit(17));
    assert(pageTableAddress(17) == PAGE_COUNT + 17);
    printf("pageTableAddress(17) passed! Value: %d\n", pageTableAddress(17));
    assert(pageAddress(17) == PAGE_SIZE * 17);
    printf("pageAddress(17) passed! Value: %d\n", pageAddress(17));
    int procNum = 17;
    int address = 2;
    mem[PAGE_COUNT + procNum] = address;
    mem[address] = 1;
    
    assert(processPageTable(procNum) == 2);
    printf("ProcNum: %d Address: %d\n", procNum, address);
    printf("processPageTable(17) passed! Value: %d\n", processPageTable(17));

    (void)argv;
    (void)argc;

#endif
}
