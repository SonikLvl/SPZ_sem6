#include "kernel.h"
#include <windows.h>
#include <stdint.h>
#include "config.h"

void* os_alloc(size_t size) {
    void* ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return ptr;
}

void os_free(void* ptr, size_t size) {
    if (ptr != NULL) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}

void os_discard_pages(void* payload_ptr, size_t payload_size, size_t offset) {
    if (payload_size == 0) return;

    uintptr_t payload_addr = (uintptr_t)payload_ptr;

    // VirtualAlloc завжди повертає пам'ять, вирівняну по сторінці
    uintptr_t arena_start = payload_addr - offset;

    size_t first_page_offset = (offset + PAGE_SIZE - 1) & ~((size_t)PAGE_SIZE - 1);
    size_t end_offset = offset + payload_size;
    size_t last_page_offset = end_offset & ~((size_t)PAGE_SIZE - 1);

    if (last_page_offset > first_page_offset) {
        size_t pages_size = last_page_offset - first_page_offset;
        uintptr_t page_start_addr = arena_start + first_page_offset;

        // Заповнюємо сторінки
        uint8_t* p = (uint8_t*)page_start_addr;
        for (size_t i = 0; i < pages_size; i++) {
            p[i] = rand() % 256;
        }

        VirtualAlloc((void*)page_start_addr, pages_size, MEM_RESET, PAGE_READWRITE);
    }
}
