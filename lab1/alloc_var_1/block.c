#include "block.h"

// Відділяє частину блоку, якщо залишок достатній для нового блоку
void block_split(block_header_t* b, size_t new_size) {
    size_t current_size = block_size(b);
    // Якщо залишок менший за розмір заголовка + мінімальне корисне навантаження, не ділимо
    if (current_size - new_size < sizeof(block_header_t) + MIN_PAYLOAD_SIZE) {
        return;
    }

    bool was_last = block_is_last(b);

    // Оновлюємо поточний блок
    block_set_size(b, new_size);
    block_set_last(b, false);

    // Створюємо новий правий блок
    block_header_t* next = block_next(b);
    next->size = 0; // Очищуємо
    block_set_size(next, current_size - new_size);
    next->prev_size = new_size;

    block_set_busy(next, false);
    block_set_first(next, false);
    block_set_last(next, was_last);

    // Оновлюємо prev_size для наступного за новим блоком (якщо він є)
    block_header_t* next_next = block_next(next);
    if (next_next) {
        next_next->prev_size = block_size(next);
    }
}

// Об'єднує блок з його правим сусідом, якщо обидва вільні
block_header_t* block_merge(block_header_t* b) {
    if (!b || block_is_busy(b)) return b;

    block_header_t* next = block_next(b);
    if (next && !block_is_busy(next)) {
        size_t merged_size = block_size(b) + block_size(next);
        bool next_was_last = block_is_last(next);

        block_set_size(b, merged_size);
        block_set_last(b, next_was_last);

        block_header_t* next_next = block_next(b); // Вже оновлений сусід
        if (next_next) {
            next_next->prev_size = merged_size;
        }
    }
    return b;
}
