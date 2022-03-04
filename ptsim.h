#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384  // MUST equal PAGE_SIZE * PAGE_COUNT
#define PAGE_SIZE 256  // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define PAGE_SHIFT 8  // Shift page number this much

unsigned char get_page_table(int);
int get_address(int, int);
void initialize_mem(void);
unsigned char get_page(void);
void new_process(int, int);
void print_page_free_map(void);
void print_page_table(int);