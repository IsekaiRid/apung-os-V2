#include "paging.h"
#include "../libc/mem.h"
#include "../libc/string.h"
#include "../driver/screen.h"

uint32_t *page_directory;

// Ganti baris ini: void page_fault_handler(registers_t regs)
void page_fault_handler(registers_t *regs)
{
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    int present = !(regs->err_code & 0x1);
    int rw = regs->err_code & 0x2;
    int us = regs->err_code & 0x4;
    int reserved = regs->err_code & 0x8;
    int id = regs->err_code & 0x10;

    kprint("\n--- PAGE FAULT DETECTED ---\n");
    kprint("Faulting address: ");
    char addr_str[16];
    hex_to_ascii(faulting_address, addr_str);
    kprint(addr_str);

    kprint("\nReason: ");
    if (present)
        kprint("[Not Present] ");
    if (rw)
        kprint("[Write Violation] ");
    if (us)
        kprint("[User Mode] ");
    if (reserved)
        kprint("[Reserved Bit Overwritten] ");
    if (id)
        kprint("[Instruction Fetch] ");

    kprint("\n--- ApungOS Halted ---");

    for (;;)
        ;
}

void init_paging()
{
    // 1. Alokasikan Page Directory (harus rata 4KB!)
    page_directory = (uint32_t *)kmalloc(4096, 1);

    // 2. Isi Page Directory dengan "Not Present" (atribut 0x2)
    for (int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0x00000002;
    }

    // 3. Buat Page Table pertama untuk memetakan 4MB pertama (termasuk Kernel)
    uint32_t *first_page_table = (uint32_t *)kmalloc(4096, 1);

    // 4. Identity Map: Petakan 0x0 hingga 0x3FFFFF (4MB)
    uint32_t address = 0;
    for (int i = 0; i < 1024; i++)
    {
        // Atribut: Present (0x1), Read/Write (0x2) -> 0x3
        first_page_table[i] = address | 3;
        address += 4096;
    }

    // 5. Masukkan Page Table ke Page Directory
    page_directory[0] = ((uint32_t)first_page_table) | 3;

    register_interrupt_handler(14, page_fault_handler);

    // 7. Daftarkan Page Directory ke CPU dan aktifkan Paging
    load_page_directory(page_directory);
    enable_paging();
}