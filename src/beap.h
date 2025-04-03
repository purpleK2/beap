#ifndef BEAP_HEAP_H
#define BEAP_HEAP_H

#include <stddef.h>

#define ALIGN8(size) (((size) + 7) & ~7)
#define FUNC_PREFIX(x) k##x // so malloc -> kmalloc

// SLUB allocator constants
#define MAX_SLUB_CACHES 10
#define MIN_SLUB_SIZE 8
#define MAX_SLUB_SIZE 2048

typedef void *(*beap_page_alloc_func)();
typedef void (*beap_page_free_func)(void *ptr);
typedef void (*beap_lock_func)();
typedef void (*beap_unlock_func)();

void beap_init(beap_page_alloc_func alloc, beap_page_free_func free,
               beap_lock_func lock_func, beap_unlock_func unlock_func);

void *FUNC_PREFIX(malloc)(size_t size);
void FUNC_PREFIX(free)(void *ptr);
void *FUNC_PREFIX(calloc)(size_t num, size_t size);
void *FUNC_PREFIX(realloc)(void *ptr, size_t size);

#endif // BEAP_HEAP_H