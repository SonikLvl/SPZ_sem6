#include "allocator.h"
#include "block.h"
#include "kernel.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

static block_header_t* arena_start = NULL;

void* mem_alloc(size_t size) {
    if (size == 0 || size > SIZE_MAX - sizeof(block_header_t) - ALIGNMENT) return NULL;

    size_t required_size = ALIGN(size + sizeof(block_header_t));

    // Ініціалізація арени при першому виклику
    if (!arena_start) {
        size_t initial_arena = DEFAULT_ARENA_SIZE > required_size ? DEFAULT_ARENA_SIZE : required_size;
        arena_start = (block_header_t*)os_alloc(initial_arena);
        if (!arena_start) return NULL;

        arena_start->size = 0;
        block_set_size(arena_start, initial_arena);
        arena_start->prev_size = 0;
        block_set_first(arena_start, true);
        block_set_last(arena_start, true);
        block_set_busy(arena_start, false);
    }

    // Лінійне сканування (First Fit)
    block_header_t* current = arena_start;
    while (current) {
        if (!block_is_busy(current) && block_size(current) >= required_size) {
            block_split(current, required_size);
            block_set_busy(current, true);
            return block_to_payload(current);
        }
        current = block_next(current);
    }

    // Якщо не знайшли місця, в ідеалі тут треба розширити арену
    return NULL;
}

void mem_free(void* ptr) {
    if (!ptr) return;

    block_header_t* block = payload_to_block(ptr);
    block_set_busy(block, false);

    // Спочатку пробуємо злити з правим сусідом
    block_merge(block);

    // Потім пробуємо злити з лівим сусідом
    block_header_t* prev = block_prev(block);
    if (prev && !block_is_busy(prev)) {
        block_merge(prev);
    }
}

void* mem_realloc(void* ptr, size_t size) {
    if (!ptr) return mem_alloc(size);
    if (size == 0) { mem_free(ptr); return NULL; }

    block_header_t* block = payload_to_block(ptr);
    size_t required_size = ALIGN(size + sizeof(block_header_t));
    size_t current_size = block_size(block);

    if (required_size <= current_size) {
        // Зменшення in-place
        block_split(block, required_size);
        return ptr;
    }

    // Спроба розширення in-place вправо
    block_header_t* next = block_next(block);
    if (next && !block_is_busy(next) && (current_size + block_size(next) >= required_size)) {
        block_merge(block);
        block_split(block, required_size);
        return ptr;
    }

    // Fallback: новий mem_alloc -> memcpy -> mem_free
    void* new_ptr = mem_alloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, current_size - sizeof(block_header_t));
        mem_free(ptr);
    }
    return new_ptr;
}

void mem_show() {
    printf("--- Arena state ---\n");
    if (!arena_start) {
        printf("Arena isn`t initialized.\n");
        return;
    }

    block_header_t* current = arena_start;
    int index = 0;

    while (current) {
        printf("Block %d | Adress: %p | Size: %zu | Busy: %d | First: %d | Last: %d\n",
               index++,
               (void*)current,
               block_size(current),
               block_is_busy(current),
               block_is_first(current),
               block_is_last(current));

        current = block_next(current);
    }
    printf("------------------\n\n");
}
