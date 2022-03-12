#include "ptsim.h"

int mem[MEM_SIZE];

int pptPage(int procNum) {
    assert(procNum < PAGE_COUNT);
    
    return mem[pageTableAddress(procNum)];
}

int getVirtualOffsetAddress(int virtAddr) {
    assert(virtAddr < MEM_SIZE);
    
    return virtAddr & 255;
}

int getVirtualPageNumber(int virtAddr) {
    return virtAddr >> 8;
}

int translateToPhysicalPage(int procNum, int vPage) {
    assert(procNum < PAGE_COUNT && vPage < PAGE_COUNT);

    int ppt = pptPage(procNum);
    int vPageAddr = pageAddress(ppt) + vPage;
    int ppg = getValue(vPageAddr);

    return ppg;
}

int translateToPhysicalAddress(int procNum, int virtAddr) {
    int vPage = getVirtualPageNumber(virtAddr);
    int vOffset = getVirtualOffsetAddress(virtAddr);
    int physicalPage = translateToPhysicalPage(procNum, vPage);
    int physicalAddress = getPhysicalAddress(physicalPage, vOffset);

    return physicalAddress;
}

int getPhysicalAddress(int page, int offset)
{
    return (page << PAGE_SHIFT) | offset;
}

void deallocatePage(int page) {
    assert(page < PAGE_COUNT && page >= 0);
    assignMemory(page, 0);

}

int getValue(int address) {
		assert(address < MEM_SIZE);
        assert(address > 0);
		return mem[address];
}

int freeBit(int address) {
    assert(address < PAGE_COUNT);

    return getPhysicalAddress(0, address);
}

int pageTableAddress(int procNum) {
    
    assert(procNum < PAGE_COUNT);
    return getPhysicalAddress(0, PAGE_COUNT + procNum);
}

int pageAddress(int page) {
    
    assert(page < PAGE_COUNT);
    return getPhysicalAddress(page, 0);
}

void assignMemory(int address, int value) {
    assert(address < MEM_SIZE && address >= 0);
    assert(value < MAX_ASSIGNABLE_VALUE && value >= 0);
    mem[address] = value;
}

void loadValue(int procNum, int virtAddr) {
    int physicalAddress = translateToPhysicalAddress(procNum, virtAddr);

    printf("Load proc %d: %d => %d, value=%d\n", procNum, virtAddr, physicalAddress, getValue(physicalAddress));
}

void storeValue(int procNum, int virtAddr, int value) {
    int physicalAddress = translateToPhysicalAddress(procNum, virtAddr);
    assignMemory(physicalAddress, value);

    printf("Store proc %d: %d => %d, value=%d\n", procNum, virtAddr, physicalAddress, value);
}

void killProcess(int procNum) {
    // get the process page table address
    int pages[PAGE_COUNT] = {0};
    int numPages = 0;
    int ppt = pptPage(procNum);

    pages[numPages++] = ppt;
    int pptAddr = pageAddress(ppt);
    int addrValue;
    while ((addrValue = getValue(pptAddr++)) != 0) {
        pages[numPages++] = addrValue;
    }
    for(int i= 0;i< numPages;i++) {
        deallocatePage(freeBit(pages[i]));
        if (i+1 < numPages) {// this will deallocate the process page table
            assignMemory(pageAddress(ppt) + i, 0);
        }
    }
    (void)pages;
}

int verifyProcAndPage(int procNum, int pageCount) {
    int error = 1;
    if(!(procNum < PAGE_COUNT)) {
        printf("Requested process %d is not valid. [0-63]\n", procNum);
        error = 0;
    }
    if(!(pageCount < PAGE_COUNT)) {
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

void initializeMem(void)
{
    for(int i = 0;i<MEM_SIZE;i++) {
        assignMemory(i, 0);
    }
    assignMemory(freeBit(0), 1);
}

int getFreePage(void)
{
    int page = 0xff;
    for(int i=1;i<PAGE_COUNT;i++) {
        if(!mem[i]) {
            assignMemory(i, 1); page = i; break;
        }
    }
    return page;
}

void newProcess(int procNum, int pageCount)
{
    if (!verifyProcAndPage(procNum, pageCount)) return;

    int pageAddr = getFreePage();
    if (isPageTableFull(pageAddr, procNum)) return;
    assignMemory(pageTableAddress(procNum), pageAddr);
    
    for(int i=0;i<pageCount;i++) {
        int newPage = getFreePage();
        if (isPageTableFull(newPage, procNum)) return;
        assignMemory(pageAddress(pageAddr)+i ,newPage);
    }
}

void printPageTable(int procNum) {
    printf("--- PROCESS %d PAGE TABLE ---\n", procNum);
    int page_table = pptPage(procNum);
    for (int i = 0; i < PAGE_COUNT; i++) {
        int addr = getPhysicalAddress(page_table, i);
        
        int page = getValue(addr);

        if (page != 0) {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

void printPageFreeMap(void) {
    printf("--- PAGE FREE MAP ---\n");
    for (int i = 0; i < PAGE_COUNT; i++) {
        int addr = getPhysicalAddress(0, i);

        printf("%c", mem[addr] == 0? '.': '#');
        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

int main(int argc, char *argv[]) {
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);
#ifndef TEST
    if (argc == 1) {
        fprintf(stderr, "usage: ptsim commands\n -> [np] procNum req_pages\n -> [pfm]\n -> [ppt]\n -> [kp] procNum\n -> [sb] procNum virt_addr value\n -> [lb] procNum virt_addr\n");
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
        else if (strcmp(argv[i], "sb") == 0) {
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

    (void)argc;
    (void)argv;
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
    
    assert(pptPage(procNum) == 2);
    printf("ProcNum: %d Address: %d\n", procNum, address);
    printf("pptPage(17) passed! Value: %d\n", pptPage(17));

    assert(getVirtualPageNumber(755) == 755 >> 8);
    printf("getVirtualPageNumber(755) passed! Value: %d\n", getVirtualPageNumber(17));

    mem[0] = 1;
    mem[81] = 1;
    mem[256] = 2;
    mem[257] = 3;

    assert(translateToPhysicalAddress(17, 272) == 784);
    printf("translateToPhysicalAddress(17, 272) passed! Value: %d\n", translateToPhysicalAddress(17, 272));
    printf("np 17 2\n mem[81] = 1\nmem[256] = 2\nmem[257] = 3\nphysical address - 528\n");
#endif
}
