#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>

void* mem_alloc(size_t size);
void mem_free(void* ptr);
void* mem_realloc(void* ptr, size_t size);
void mem_show();

#endif
