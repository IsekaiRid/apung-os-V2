#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../cpu/paging.h"
#include "../driver/screen.h"
#include "../driver/keyboard.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "kernel.h"
#include <stdint.h>

/* Kita simpan alamat terakhir untuk tes kfree */
uint32_t last_allocated_ptr = 0;

void kernel_main()
{
    /* 1. Setup Interrupts */
    isr_install();
    irq_install();

    /* 2. Setup Physical Memory Management (Heap) */
    init_mem();

    /* 3. Setup Virtual Memory (Paging)
       Paging butuh kmalloc, jadi harus dipanggil SETELAH init_mem */
    init_paging();

    /* 4. Tampilan Awal OS */
    clear_screen();
    kprint("--- ApungOS Kernel Loaded with Paging ---\n");
    kprint("Commands: END, PAGE, FREE, CLEAR\n");
    kprint("> ");
}

void user_input(char *input)
{
    if (strcmp(input, "END") == 0)
    {
        kprint("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }
    else if (strcmp(input, "PAGE") == 0)
    {
        /* Panggil kmalloc dengan 2 argumen: (size, align)
           Kita pakai align=1 agar alamatnya rapi (kelipatan 4KB) */
        void *addr = kmalloc(1000, 1);

        if (addr == NULL)
        {
            kprint("OOM: Out of memory!\n");
        }
        else
        {
            last_allocated_ptr = (uint32_t)addr;
            char addr_str[16] = "";
            hex_to_ascii((uint32_t)addr, addr_str);
            kprint("Allocated 1000 bytes at: ");
            kprint(addr_str);
            kprint("\n");
        }
    }
    else if (strcmp(input, "FREE") == 0)
    {
        if (last_allocated_ptr != 0)
        {
            kfree((void *)last_allocated_ptr);
            kprint("Memory at ");
            char addr_str[16] = "";
            hex_to_ascii(last_allocated_ptr, addr_str);
            kprint(addr_str);
            kprint(" has been freed!\n");
            last_allocated_ptr = 0;
        }
        else
        {
            kprint("Nothing to free. Use PAGE first.\n");
        }
    }
    else if (strcmp(input, "CLEAR") == 0)
    {
        clear_screen();
    }
    else if (strcmp(input, "CRASH") == 0)
    {
        uint32_t *ptr = (uint32_t *)0xA00000; 
        uint32_t bad_data = *ptr;            
    }
    else if (strlen(input) > 0)
    {
        kprint("You said: ");
        kprint(input);
        kprint("\n");
    }

    kprint("> ");
}