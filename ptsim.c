#include "ptsim.h"

// Simulated RAM
unsigned char mem[MEM_SIZE];
unsigned char pt[PAGE_SIZE];

/* 
    Input:
    page: The virtual page number.
    offset: The virtual address on said page number.

    Out:
    A physical address calculated from page and offset.
*/
int getPhysicalAddress(unsigned char page, unsigned char offset)
{
    return (page << PAGE_SHIFT) | offset;
}


void deallocatePage(int page) {
    assert(page < PAGE_COUNT && page >= 0);
    assignPhysicalMemory(page, 0);

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
    // get the process page table address
    unsigned char pages[PAGE_COUNT] = {0};
    unsigned char numPages = 0;
    unsigned char ppt = processPageTable(procNum);

    pages[numPages++] = ppt; // page table for the listed process
    int pptAddr = pageAddress(ppt); // physical address for process page table (proc_num * 256)
    unsigned char addrValue;
    while ((addrValue = getValue(pptAddr++)) != 0) {
        // printf("addrValue: %d\n", addrValue);
        pages[numPages++] = addrValue;
    }
    for(int i= 0;i< numPages;i++) {
        assignPhysicalMemory(freeBit(pages[i]), 0);
        // printf("pages[%d]: %d\n", i, pages[i]);
    }
    (void)pages;
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
    if (isPageTableFull(pageAddr, procNum)) return;
    assignPhysicalMemory(pageTableAddress(procNum), pageAddr);
    
    for(int i=0;i<pageCount;i++) {
        int newPage = getVirtualPage();
        if (isPageTableFull(newPage, procNum)) return;
        assignPhysicalMemory(pageAddress(pageAddr)+i ,newPage);
    }
}

/*
    Print the address map from virtual pages to physical
*/
void printPageTable(int procNum)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", procNum);
    // Get the page table for this process
    unsigned char page_table = processPageTable(procNum);
    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++) {
        int addr = getPhysicalAddress(page_table, i);
        
        int page = getValue(addr);

        if (page != 0) {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

/*
    Print the free page map
*/
void printPageFreeMap(void)
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
#ifndef TEST
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
            printPageFreeMap();
        }
        else if (strcmp(argv[i], "ppt") == 0) {
            int procNum = atoi(argv[++i]);
            printPageTable(procNum);
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
