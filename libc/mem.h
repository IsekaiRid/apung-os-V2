#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

/* Struktur Header dengan Self-Reference yang benar */
typedef struct mem_header {
    uint32_t size;
    int is_free;
    struct mem_header *next; /* Sekarang compiler tahu ini merujuk ke struct ini sendiri */
} mem_header_t;

void memory_copy(uint8_t *source, uint8_t *dest, int nbytes);
void memory_set(uint8_t *dest, uint8_t val, uint32_t len);

void init_mem();
void* kmalloc(size_t size, int align);
void kfree(void* ptr);

#endif