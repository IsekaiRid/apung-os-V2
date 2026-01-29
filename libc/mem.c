#include "mem.h"

#define HEAP_START 0x10000
#define HEAP_MAX 0x40000

mem_header_t *first_mem_block;

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes)
{
    int i;
    for (i = 0; i < nbytes; i++)
    {
        *(dest + i) = *(source + i);
    }
}

void memory_set(uint8_t *dest, uint8_t val, uint32_t len)
{
    uint8_t *temp = (uint8_t *)dest;
    for (; len != 0; len--)
        *temp++ = val;
}

void init_mem()
{
    first_mem_block = (mem_header_t *)HEAP_START;
    first_mem_block->size = HEAP_MAX - HEAP_START - sizeof(mem_header_t);
    first_mem_block->is_free = 1;
    first_mem_block->next = NULL;
}

// Di libc/mem.c
void *kmalloc(size_t size, int align)
{
    mem_header_t *curr = first_mem_block;

    while (curr)
    {
        uint32_t addr = (uint32_t)curr + sizeof(mem_header_t);

        /* Jika minta align, hitung adjustment */
        uint32_t offset = 0;
        if (align && (addr & 0xFFF))
        {
            offset = 4096 - (addr & 0xFFF);
        }

        if (curr->is_free && curr->size >= (size + offset))
        {
            /* Jika ada offset karena alignment, kita sesuaikan sizenya */
            if (offset > 0)
            {
                // Sederhananya, kita korbankan sedikit space di depan blok
                // untuk memastikan alamat return rata 4KB
                curr->size -= offset;
                curr = (mem_header_t *)((uint32_t)curr + offset);
                curr->is_free = 0; // Sebenarnya ini butuh penanganan header yang lebih rumit,
                                   // tapi untuk paging yang dipanggil di awal, ini cukup.
            }

            if (curr->size > size + sizeof(mem_header_t) + 4)
            {
                mem_header_t *new_block = (mem_header_t *)((uint32_t)curr + sizeof(mem_header_t) + size);
                new_block->size = curr->size - size - sizeof(mem_header_t);
                new_block->is_free = 1;
                new_block->next = curr->next;
                curr->size = size;
                curr->next = new_block;
            }

            curr->is_free = 0;
            return (void *)((uint32_t)curr + sizeof(mem_header_t));
        }
        curr = curr->next;
    }
    return NULL;
}

void kfree(void *ptr)
{
    if (!ptr)
        return;

    mem_header_t *header = (mem_header_t *)((uint32_t)ptr - sizeof(mem_header_t));
    header->is_free = 1;

    // Coalescing (Penggabungan blok yang bebas bersebelahan)
    mem_header_t *curr = first_mem_block;
    while (curr && curr->next)
    {
        if (curr->is_free && curr->next->is_free)
        {
            curr->size += curr->next->size + sizeof(mem_header_t);
            curr->next = curr->next->next;
        }
        else
        {
            curr = curr->next;
        }
    }
}