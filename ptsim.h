#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384  // MUST equal PAGE_SIZE * PAGE_COUNT
#define PAGE_SIZE 256  // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define PAGE_SHIFT 8  // Shift page number this much
#define MAX_ASSIGNABLE_VALUE 255 // This is the highest number we can store in our memory slots.

int getPhysicalAddress(unsigned char, unsigned char);
unsigned char getValue(int);
unsigned char freeBit(int);
unsigned char pageTableAddress(unsigned char);
int pageAddress(unsigned char page);
unsigned char processPageTable(unsigned char procNum);
void assignPhysicalMemory(int, int);
void loadValue(int, int);
void storeValue(int, int, int);
void killProcess(int);
int verifyProcAndPage(int, int);
unsigned char get_page_table(int);
void initializeMem(void);
unsigned char getVirtualPage(void);
void newProcess(int, int);
void view_page_table(int);
void print_page_free_map(void);
