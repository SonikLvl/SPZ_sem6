#ifndef ALLOCATOR_IMPL_H
#define ALLOCATOR_IMPL_H

#include <stddef.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#endif
