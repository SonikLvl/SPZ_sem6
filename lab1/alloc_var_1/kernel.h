#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>

void* os_alloc(size_t size);
void os_free(void* ptr, size_t size);

#endif
