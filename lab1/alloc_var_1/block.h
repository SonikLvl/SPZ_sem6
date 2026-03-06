#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "allocator_impl.h"

#define FLAG_BUSY  0x1 //0001
#define FLAG_FIRST 0x2 //0010
#define FLAG_LAST  0x4 //0100
#define FLAGS_MASK 0x7 //0111

typedef struct block_header {
    size_t size;      // Включає розмір блоку (з заголовком) + прапорці
    size_t prev_size; // Розмір попереднього лівого сусіда (для O(1) merge)
} block_header_t;

// --- Getters ---
static inline size_t block_size(block_header_t* b) { return b->size & ~FLAGS_MASK; }
static inline bool block_is_busy(block_header_t* b) { return b->size & FLAG_BUSY; }
static inline bool block_is_first(block_header_t* b) { return b->size & FLAG_FIRST; }
static inline bool block_is_last(block_header_t* b) { return b->size & FLAG_LAST; }

// --- Setters ---
static inline void block_set_size(block_header_t* b, size_t size) {
    b->size = size | (b->size & FLAGS_MASK); // Зберігаємо старі прапорці
}
static inline void block_set_busy(block_header_t* b, bool busy) {
    if (busy) b->size |= FLAG_BUSY; else b->size &= ~FLAG_BUSY;
}
static inline void block_set_first(block_header_t* b, bool first) {
    if (first) b->size |= FLAG_FIRST; else b->size &= ~FLAG_FIRST;
}
static inline void block_set_last(block_header_t* b, bool last) {
    if (last) b->size |= FLAG_LAST; else b->size &= ~FLAG_LAST;
}

// --- Navigation ---
static inline void* block_to_payload(block_header_t* b) {
    return (void*)((uint8_t*)b + sizeof(block_header_t));
}
static inline block_header_t* payload_to_block(void* payload) {
    return (block_header_t*)((uint8_t*)payload - sizeof(block_header_t));
}
static inline block_header_t* block_next(block_header_t* b) {
    if (block_is_last(b)) return NULL;
    return (block_header_t*)((uint8_t*)b + block_size(b));
}
static inline block_header_t* block_prev(block_header_t* b) {
    if (block_is_first(b)) return NULL;
    return (block_header_t*)((uint8_t*)b - b->prev_size);
}

void block_split(block_header_t* b, size_t new_size);
block_header_t* block_merge(block_header_t* b);

#endif
