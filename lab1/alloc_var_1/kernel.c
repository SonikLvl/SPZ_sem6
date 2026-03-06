#include "kernel.h"
#include <windows.h>

void* os_alloc(size_t size) {
    void* ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return ptr;
}

void os_free(void* ptr, size_t size) {
    if (ptr != NULL) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}
