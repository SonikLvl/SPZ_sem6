#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "allocator.h"

int main() {
    printf("1. Alignment overflow test\n");
    void* overflow_ptr1 = mem_alloc(SIZE_MAX);
    void* overflow_ptr2 = mem_alloc(SIZE_MAX - 1);
    printf("SIZE_MAX allocation result: %p\n", overflow_ptr1);
    printf("SIZE_MAX - 1 allocation result: %p\n\n", overflow_ptr2);

    printf("2. Basic allocation and arena initialization\n");
    // Алокуємо 3 блоки для створення різних комбінацій сусідів
    void* ptr1 = mem_alloc(100);
    void* ptr2 = mem_alloc(200);
    void* ptr3 = mem_alloc(300);
    //void* ptr4 = mem_alloc(400); // Блок-бар'єр, щоб ptr3 не злився з кінцем арени

    // Заповнюємо випадковими числами
    if (ptr1) {
        uint8_t* p = (uint8_t*)ptr1;
        for (int i = 0; i < 100; i++) p[i] = rand() % 256;
    }

    mem_show();

    printf("3. mem_free test: Case 1 (busy - CURR - busy)\n");
    // ptr1 (зайнятий) - ptr2 (звільняємо) - ptr3 (зайнятий)
    mem_free(ptr2);
    mem_show();

    printf("4. mem_free test: Case 2 (free - CURR - busy)\n");
    // ptr2 (вже вільний) - ptr1 (звільняємо) -> вони мають злитися
    mem_free(ptr1);
    mem_show();

    printf("5. Restoring state for the following tests\n");
    ptr1 = mem_alloc(100);
    ptr2 = mem_alloc(200);
    mem_show();

    printf("6. mem_free test: Case 3 (busy - CURR - free)\n");
    // Звільняємо правий блок, потім поточний
    mem_free(ptr3);
    mem_free(ptr2); // ptr2 має злитися з ptr3
    mem_show();

    printf("7. mem_free test: Case 4 (free - CURR - free)\n");
    // ptr2, ptr3 вже вільні (об'єднані). Звільняємо ptr1, який має ліворуч нічого (він перший),але праворуч має великий вільний блок.
    mem_free(ptr1);
    mem_show();

    return 0;
}
