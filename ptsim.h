#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384  // MUST equal PAGE_SIZE * PAGE_COUNT
#define PAGE_SIZE 256  // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define PAGE_SHIFT 8  // Shift page number this much
#define MAX_ASSIGNABLE_VALUE 255 // This is the highest number we can store in our memory slots.

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