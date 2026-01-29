#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "isr.h"

void init_paging();
void page_fault_handler(registers_t *regs); 

extern void load_page_directory(uint32_t* directory);
extern void enable_paging();

#endif