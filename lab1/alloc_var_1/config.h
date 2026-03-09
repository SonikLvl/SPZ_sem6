#ifndef CONFIG_H
#define CONFIG_H

#define PAGE_SIZE 4096
#define DEFAULT_ARENA_SIZE (PAGE_SIZE * 1024) // 4 MB

// 1 - Сторінкова пам'ять, 0 - Несторінкова пам'ять
#define USE_PAGED_MEMORY 1

#endif
