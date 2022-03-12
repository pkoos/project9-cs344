#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384
#define PAGE_SIZE 256
#define PAGE_COUNT 64
#define PAGE_SHIFT 8
#define MAX_ASSIGNABLE_VALUE 255

int pptPage(int);
int getVirtualOffsetAddress(int);
int getVirtualPageNumber(int);
int translateToPhysicalPage(int, int);
int translateToPhysicalAddress(int, int);
int getPhysicalAddress(int, int);
void deallocatePage(int);
int getValue(int);
int freeBit(int);
int pageTableAddress(int);
int pageAddress(int);
void assignMemory(int, int);
void loadValue(int, int);
void storeValue(int, int, int);
void killProcess(int);
int verifyProcAndPage(int, int);
int isPageTableFull(int, int);
void initializeMem(void);
int getVirtualPage(void);
void newProcess(int, int);
void printPageTable(int);
void printPageFreeMap(void);