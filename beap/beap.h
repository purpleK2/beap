#ifndef BEAP_H
#define BEAP_H

#include "tlsf.h"
#include <stddef.h>
#include <stdint.h>

#define BEAP_PAGE_SIZE     4096
#define BEAP_INITIAL_PAGES 1 // 1 * 4KiB = 4 KiB

#define PREFIX(x) k##x

extern tlsf_t tlsf_pool;

extern uint64_t memory_used;
extern uint64_t memory_total;
extern uint64_t memory_free;

// implement the following yourself
extern void *beap_alloc_pages(size_t pages);
extern void beap_free_pages(void *page, size_t pages);
extern void beap_lock(void);
extern void beap_unlock(void);
#ifdef BEAP_DEBUG
extern void beap_debug(const char *fmt, ...);
#endif

// functions
void tlsf_beap_init(void);

void *PREFIX(malloc)(size_t size);
void *PREFIX(calloc)(size_t count, size_t size);
void *PREFIX(realloc)(void *ptr, size_t size);
void PREFIX(free)(void *ptr);

#endif // BEAP_H