#ifndef HEAP_H
#define HEAP_H 1

#ifndef BEAP_PAGE
#error "BEAP_PAGE is not defined. Please define it and assign to it the size of your page frames."
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ROUND_DOWN(n, a) ((n) & ~((a) - 1))
#define ROUND_UP(n, a)   (((n) + (a) - 1) & ~((a) - 1))

#define MIN(a, b) (a < b ? a : b)

#define PREFIX(fun) k##fun

#define HEAPMAGIC_AVAIL 0xC00DC00B // GOOD GOOB
#define HEAPMAGIC_UNAV  0xDEADD00D // DEAD DOOD

#define HEAPVER_MAJOR 2
#define HEAPVER_MINOR 1

typedef struct beap_memnode_t {
    uint32_t magic;

    size_t len;     // usable data length (in bytes)
    bool allocated; // to be used with magic bytes

    struct beap_memnode_t *next;
} beap_memnode_t;

typedef struct beap_maj_t {
    beap_memnode_t* root;

    beap_memnode_t* last_checked;
} beap_maj_t;

#define HEAP_ALIGN(p)   p + sizeof(beap_memnode_t)
#define HEAP_UNALIGN(p) p - sizeof(beap_memnode_t)

#define BEAP_PAGES 1 // default pages to allocate for root node

// NO WAY your fancy malloc stuff
void *PREFIX(malloc)(size_t);
void PREFIX(free)(void *);
void *PREFIX(calloc)(size_t times, size_t size);
void *PREFIX(realloc)(void *p_old, size_t size);

/*   the following needs to be implemented by the kernel    */

// According to your build system, remember to define BEAP_PAGE which indicates
// the size of a page frame (eg 4KiB, 2MiB)

// asks the bottom-level MM <size> bytes (not page-aligned)
// save the actually allocated size to <size_out> (might be NULL, and you
// shouldn't save it in that case)
void *beap_alloc(size_t size, size_t *size_out);
// releases <pages> pages from a pointer to the same bottom-level MM
void beap_dealloc(void *p, size_t pages);

// Locks/releases a spinlock/mutex/whatever you want
void beap_lock();
void beap_unlock();

// prints debug info (if HEAP_DEBUG is defined)
#ifdef BEAP_DEBUG
void beap_debug(const char *, ...);
#endif

// your usual mem*** functions, if you're a sane person you should have these
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

#endif // HEAP_H
