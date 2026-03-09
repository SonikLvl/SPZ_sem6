#include "allocator.h"
#include "block.h"
#include "kernel.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

#define SHRINK_THRESHOLD 0.5

static block_tree free_tree;
static block_node tree_null_node;
static block_header_t* arena_start = NULL;

void* mem_alloc(size_t size) {
    if (size == 0 || size > SIZE_MAX - sizeof(block_header_t) - ALIGNMENT) return NULL;

    size_t required_size = ALIGN(size);
    if (required_size < MIN_PAYLOAD_SIZE) {
        required_size = MIN_PAYLOAD_SIZE;
    }
    size_t total_required = required_size + sizeof(block_header_t);

    // Ініціалізація арени при першому виклику
    if (!arena_start) {
        block_tree_init(&free_tree, &tree_null_node);

        size_t initial_arena = DEFAULT_ARENA_SIZE > required_size ? DEFAULT_ARENA_SIZE : required_size;
        arena_start = (block_header_t*)os_alloc(initial_arena);
        if (!arena_start) return NULL;

        arena_start->size = 0;
        block_set_size(arena_start, initial_arena);
        arena_start->prev_size = 0;
        block_set_first(arena_start, true);
        block_set_last(arena_start, true);
        block_set_busy(arena_start, false);

        // Додаємо перший великий блок у дерево
        block_node* n = block_to_node(arena_start);
        init_node(n, initial_arena);
        block_tree_insert(&free_tree, n);
    }

    // BEST FIT: Шукаємо найменший блок, який підходить
    block_node* best = block_tree_find_best(&free_tree, total_required);
    if (!best) {
        size_t arena_size = DEFAULT_ARENA_SIZE;

        if (total_required > DEFAULT_ARENA_SIZE) {
            arena_size = ALIGN(total_required);
        }

        block_header_t* new_arena = (block_header_t*)os_alloc(arena_size);
        if (!new_arena) return NULL; // закінчилася пам'ять ОС

        new_arena->size = 0;
        block_set_size(new_arena, arena_size);
        new_arena->prev_size = 0;
#if USE_PAGED_MEMORY
        new_arena->offset = sizeof(block_header_t);
#endif
        block_set_first(new_arena, true);
        block_set_last(new_arena, true);
        block_set_busy(new_arena, false);

        // Вставляємо нову арену в дерево як звичайний великий вільний блок
        block_node* n = block_to_node(new_arena);
        init_node(n, arena_size);
        block_tree_insert(&free_tree, n);

        best = block_tree_find_best(&free_tree, total_required);
    }

    block_header_t* block = node_to_block(best);

    // Видаляємо знайдений блок з дерева ПЕРЕД змінами
    block_tree_remove(&free_tree, best);

    // Відрізаємо зайве (запам'ятовуємо старий розмір)
    size_t old_size = block_size(block);
    block_split(block, total_required);
    block_set_busy(block, true);

    // Якщо блок поділився, додаємо вільну частину назад у дерево
    if (block_size(block) < old_size) {
        block_header_t* next = block_next(block);
        if (next && !block_is_busy(next)) {
            block_node* next_node = block_to_node(next);
            init_node(next_node, block_size(next));
            block_tree_insert(&free_tree, next_node);
        }
    }

    return block_to_payload(block);
}

void mem_free(void* ptr) {
    if (!ptr) return;

    block_header_t* block = payload_to_block(ptr);
    block_set_busy(block, false);

    // Спроба злиття з правим сусідом
    block_header_t* next = block_next(block);
    if (next && !block_is_busy(next)) {
        block_tree_remove(&free_tree, block_to_node(next));
        block_merge(block); // Зливаємо (block залишається вказівником на початок)
    }

    // Спроба злиття з лівим сусідом
    block_header_t* prev = block_prev(block);
    if (prev && !block_is_busy(prev)) {
        block_tree_remove(&free_tree, block_to_node(prev));
        block = block_merge(prev); // Тепер prev поглинає block, вказівник зсувається ліворуч
    }

    // Якщо блок одночасно є і першим, і останнім - це означає, що арена повністю порожня
    if (block_is_first(block) && block_is_last(block)) {
        os_free((void*)block, 0);
        return;
    }

    // Додаємо підсумковий великий блок у дерево
    block_node* n = block_to_node(block);
    init_node(n, block_size(block));
    block_tree_insert(&free_tree, n);

#if USE_PAGED_MEMORY
    // Захищаємо структуру дерева від отруєння сторінок
    size_t payload_size = block_size(block) - sizeof(block_header_t);
    if (payload_size > sizeof(block_node)) {
        void* safe_ptr = (uint8_t*)block_to_payload(block) + sizeof(block_node);
        size_t safe_size = payload_size - sizeof(block_node);
        size_t safe_offset = block->offset + sizeof(block_node);

        os_discard_pages(safe_ptr, safe_size, safe_offset);
    }
#endif
}

void* mem_realloc(void* ptr, size_t size) {
    if (!ptr) return mem_alloc(size);
    if (size == 0) { mem_free(ptr); return NULL; }

    block_header_t* block = payload_to_block(ptr);
    size_t current_size = block_size(block);

    size_t required_size = ALIGN(size);
    if (required_size < MIN_PAYLOAD_SIZE) {
        required_size = MIN_PAYLOAD_SIZE;
    }
    size_t total_required = required_size + sizeof(block_header_t);

    // Сценарій 1: Зменшення блоку (Shrinking) in-place
    if (total_required <= current_size) {

        if (current_size >= DEFAULT_ARENA_SIZE) {
            if ((double)total_required > (double)current_size * SHRINK_THRESHOLD) {
                return ptr;
            }
        }

        if (current_size - total_required >= sizeof(block_header_t) + MIN_PAYLOAD_SIZE) {
            block_split(block, total_required);

            block_header_t* next = block_next(block);
            if (next && !block_is_busy(next)) {
                block_node* next_node = block_to_node(next);
                init_node(next_node, block_size(next)); // Ключ - це розмір нового уламка
                block_tree_insert(&free_tree, next_node);
#if USE_PAGED_MEMORY
                size_t next_payload_size = block_size(next) - sizeof(block_header_t);
                if (next_payload_size > sizeof(block_node)) {
                    void* safe_ptr = (uint8_t*)block_to_payload(next) + sizeof(block_node);
                    size_t safe_size = next_payload_size - sizeof(block_node);
                    size_t safe_offset = next->offset + sizeof(block_node);

                    os_discard_pages(safe_ptr, safe_size, safe_offset);
                }
#endif
            }
        }
        return ptr;
    }

    // Спроба розширення in-place вправо
    block_header_t* next = block_next(block);
    if (next && !block_is_busy(next) && (current_size + block_size(next) >= total_required)) {

        // Видаляємо правого сусіда з дерева ПЕРЕД тим як його чіпати
        block_tree_remove(&free_tree, block_to_node(next));

        // Тимчасово робимо блок вільним, щоб block_merge спрацював
        block_set_busy(block, false);
        block_merge(block);
        block_set_busy(block, true);

        // Якщо утворений блок завеликий, відрізаємо зайве
        if (block_size(block) - total_required >= sizeof(block_header_t) + MIN_PAYLOAD_SIZE) {
            block_split(block, total_required);

            // Повертаємо залишок (новий правий блок) у дерево
            block_header_t* new_next = block_next(block);
            if (new_next && !block_is_busy(new_next)) {
                block_node* new_next_node = block_to_node(new_next);
                init_node(new_next_node, block_size(new_next));
                block_tree_insert(&free_tree, new_next_node);
            }
        }
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

void print_block_header(block_node* node) {
    block_header_t* block = node_to_block(node);
    printf("Free block | Adress: %p | Size: %zu | Busy: %d | First: %d | Last: %d\n",
           (void*)block,
           block_size(block),
           block_is_busy(block),
           block_is_first(block),
           block_is_last(block));
}

void mem_show() {
    printf("--- Arena state ---\n");
    if (!arena_start) {
        printf("Arena isn`t initialized.\n");
        return;
    }

    // Обходимо лише вільні блоки, що знаходяться в дереві, від найменшого до найбільшого
    block_tree_traverse(&free_tree, print_block_header);
    printf("-------------------------------------------\n\n");
}
