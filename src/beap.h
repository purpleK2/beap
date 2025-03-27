#ifndef BEAP_HEAP_H
#define BEAP_HEAP_H

#include <stddef.h>

#define ALIGN8(size) (((size) + 7) & ~7)
#define FUNC_PREFIX(x) k##x // so malloc -> kmalloc

typedef void *(*beap_page_alloc_func)();
typedef void (*beap_page_free_func)(void *ptr);

void beap_init(beap_page_alloc_func alloc, beap_page_free_func free);

void *FUNC_PREFIX(malloc)(size_t size);
void FUNC_PREFIX(free)(void *ptr);
void *FUNC_PREFIX(calloc)(size_t num, size_t size);
void *FUNC_PREFIX(realloc)(void *ptr, size_t size);

#endif // BEAP_HEAP_H