#ifndef BEAP_HEAP_H
#define BEAP_HEAP_H

#include <stddef.h>

#define BEAP_ALIGN8(size) (((size) + 7) & ~7)
#define BEAP_PREFIX(x) k##x // so malloc -> kmalloc

// SLUB allocator constants
#define MAX_SLUB_CACHES 10
#define MIN_SLUB_SIZE 8
#define MAX_SLUB_SIZE 2048

void beap_init();

void *BEAP_PREFIX(malloc)(size_t size);
void BEAP_PREFIX(free)(void *ptr);
void *BEAP_PREFIX(calloc)(size_t num, size_t size);
void *BEAP_PREFIX(realloc)(void *ptr, size_t size);

/*
    The following needs to be implemented by the user
*/

// allocates a page to the lower-level MM
extern void *beap_alloc_page();
extern void beap_free_page(void *);

// locks and releases a spinlock/mutex/ecc.
extern void beap_lock();
extern void beap_unlock();

#endif // BEAP_HEAP_H