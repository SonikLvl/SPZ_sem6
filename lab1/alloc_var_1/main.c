#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "allocator.h"

#define MAX_RECORDS 100         // Кількість блоків, які ми одночасно запам'ятовуємо
#define NUM_OPERATIONS 10000    // Загальна кількість операцій
#define MAX_ALLOC_SIZE 4096     // Максимальний розмір одного блоку для виділення

// Структура для відстеження виділеної пам'яті
typedef struct {
    void* ptr;          // Покажчик на виділений блок (payload)
    size_t size;        // Запитаний розмір
    uint32_t checksum;  // Контрольна сума даних
    bool active;        // Чи зайнятий цей слот у масиві
} AllocationRecord;

// Масив для збереження стану
AllocationRecord records[MAX_RECORDS] = {0};

// --- Допоміжні функції ---

// Обчислення простої контрольної суми (сума всіх байтів)
uint32_t calculate_checksum(void* ptr, size_t size) {
    uint32_t sum = 0;
    uint8_t* data = (uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

// Заповнення блоку випадковими даними
void fill_random_data(void* ptr, size_t size) {
    uint8_t* data = (uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        data[i] = rand() % 256;
    }
}

// --- Головна функція ---

int main() {
    // Ініціалізуємо генератор випадкових чисел
    srand((unsigned int)time(NULL));

    int alloc_count = 0;
    int free_count = 0;
    int realloc_count = 0;
    int failed_allocs = 0;

    printf("Start auto testing...\n");
    printf("Number of operations: %d\n", NUM_OPERATIONS);
    printf("Size of follow array: %d\n\n", MAX_RECORDS);

    for (int i = 0; i < NUM_OPERATIONS; i++) {
        // Вибираємо випадковий слот у масиві відстеження
        int slot = rand() % MAX_RECORDS;

        // Визначаємо, що будемо робити (ймовірність)
        int action = rand() % 100;

        if (!records[slot].active) {
            // СЛОТ ПОРОЖНІЙ: Робимо mem_alloc()
            size_t size = (rand() % MAX_ALLOC_SIZE) + 1; // Розмір від 1 до MAX_ALLOC_SIZE
            void* ptr = mem_alloc(size);

            if (ptr != NULL) {
                fill_random_data(ptr, size);

                records[slot].ptr = ptr;
                records[slot].size = size;
                records[slot].checksum = calculate_checksum(ptr, size);
                records[slot].active = true;

                alloc_count++;
            } else {
                failed_allocs++; // Арена може бути переповнена, це нормально
            }
        } else {
            // СЛОТ ЗАЙНЯТИЙ: Перевіряємо цілісність перед діями!
            uint32_t current_check = calculate_checksum(records[slot].ptr, records[slot].size);
            if (current_check != records[slot].checksum) {
                printf("\n[!!! ERROR !!!]\n");
                printf("Memory corruption detected in slot %d (Iteration %d)\n", slot, i);
                printf("Expected sum: %u, Got: %u\n", records[slot].checksum, current_check);
                return 1; // Негайно зупиняємо тест
            }

            // Якщо пам'ять ціла, вибираємо між mem_free і mem_realloc
            if (action < 50) {
                // Виконуємо mem_free()
                mem_free(records[slot].ptr);
                records[slot].active = false;
                free_count++;
            } else {
                // Виконуємо mem_realloc()
                size_t new_size = (rand() % MAX_ALLOC_SIZE) + 1;
                void* new_ptr = mem_realloc(records[slot].ptr, new_size);

                if (new_ptr != NULL) {
                    // Знову заповнюємо весь новий розмір випадковими даними
                    fill_random_data(new_ptr, new_size);

                    records[slot].ptr = new_ptr;
                    records[slot].size = new_size;
                    records[slot].checksum = calculate_checksum(new_ptr, new_size);

                    realloc_count++;
                } else {
                    failed_allocs++;
                    // Якщо realloc повертає NULL, старий блок залишається валідним (нічого не робимо)
                }
            }
        }

        // Вивід прогресу кожні 2000 ітерацій
        if ((i + 1) % 2000 == 0) {
            printf("Done %d / %d oparations...\n", i + 1, NUM_OPERATIONS);
        }
    }

    printf("\n=== Testing finnished successfully, checking remaining memory... ===\n");

    // Фінальна перевірка та очищення
    int active_blocks = 0;
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (records[i].active) {
            active_blocks++;

            // Остання перевірка цілісності
            uint32_t current_check = calculate_checksum(records[i].ptr, records[i].size);
            if (current_check != records[i].checksum) {
                printf("\n[!!! ERROR !!!]\n");
                printf("Corruption in slot %d\n", i);
                return 1;
            }

            // Звільняємо блок
            mem_free(records[i].ptr);
            records[i].active = false;
        }
    }

    printf("\n--- Statistics ---\n");
    printf("Successful mem_alloc:   %d\n", alloc_count);
    printf("Successful mem_realloc: %d\n", realloc_count);
    printf("Successful mem_free:    %d\n", free_count);
    printf("Unsuccessful tries (not enough memory): %d\n", failed_allocs);
    printf("Freed slots: %d\n", active_blocks);

    // Виклик mem_show() в кінці (якщо все працює правильно, арена повинна мати лише 1-2 великих вільних блоки)
    mem_show();

    return 0;
}
