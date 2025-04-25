#include "beap.h"
#include "tlsf.h"

#include <stdint.h>

tlsf_t tlsf_pool = NULL;

uint64_t memory_used  = 0;
uint64_t memory_total = 0;
uint64_t memory_free  = 0;

// memfuncs
void beap_memset(void *ptr, int value, size_t size) {
    unsigned char *p = (unsigned char *)ptr;
    while (size--) {
        *p++ = (unsigned char)value;
    }
}

void beap_memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d       = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
}

void tlsf_beap_init(void) {
    beap_lock();
    if (tlsf_pool == NULL) {
        void *mem = beap_alloc_pages(BEAP_INITIAL_PAGES);
        tlsf_pool =
            tlsf_create_with_pool(mem, BEAP_INITIAL_PAGES * BEAP_PAGE_SIZE);

        if (tlsf_pool == NULL) {
            beap_free_pages(mem, BEAP_INITIAL_PAGES);
            return;
        }
        memory_total = BEAP_INITIAL_PAGES * BEAP_PAGE_SIZE;
        memory_free  = BEAP_INITIAL_PAGES * BEAP_PAGE_SIZE;
        memory_used  = 0;
    }
    beap_unlock();
}

void *PREFIX(malloc)(size_t size) {
    beap_lock();
    void *ptr = tlsf_malloc(tlsf_pool, size);

    memory_used += size;
    memory_free -= size;

    if (memory_free < size) {
        // If memory is low, try to allocate more pages
        size_t pages_needed =
            (size - memory_free + BEAP_PAGE_SIZE - 1) / BEAP_PAGE_SIZE;
        void *new_mem =
            beap_alloc_pages(pages_needed); // Allocate additional pages
        if (new_mem != NULL) {
            tlsf_add_pool(tlsf_pool, new_mem, pages_needed * BEAP_PAGE_SIZE);
            memory_total += pages_needed * BEAP_PAGE_SIZE;
            memory_free  += pages_needed * BEAP_PAGE_SIZE;
        }
    }

    beap_unlock();
    return ptr;
}

void *PREFIX(calloc)(size_t count, size_t size) {
    // Check for multiplication overflow and zero-size requests
    if (count == 0 || size == 0) {
        return PREFIX(malloc)(
            0); // Standard behavior for calloc with zero values
    }

    // Check for multiplication overflow
    // If nmemb * size overflows, the division won't equal nmemb
    if (size > 4096 / count) {
        return NULL; // Overflow detected
    }

    // Calculate total size needed
    size_t total_size = size * size;

    // Allocate memory using malloc
    void *ptr = PREFIX(malloc)(total_size);

    // Initialize all bytes to zero if allocation succeeded
    if (ptr != NULL) {
        beap_memset(ptr, 0, total_size);
    }

    return ptr;
    ;
}

void *PREFIX(realloc)(void *ptr, size_t size) {
    // If ptr is NULL, realloc behaves like malloc
    if (ptr == NULL) {
        return PREFIX(malloc)(size);
    }

    // If size is 0 and ptr is not NULL, free the memory and return NULL
    if (size == 0) {
        PREFIX(free)(ptr);
        return NULL;
    }

    // Allocate new memory block
    void *new_ptr = PREFIX(malloc)(size);

    // If allocation failed, return NULL without freeing original block
    if (new_ptr == NULL) {
        return NULL;
    }

    beap_memcpy(new_ptr, ptr, size); // This is an oversimplification

    // Free old memory block
    PREFIX(free)(ptr);

    return new_ptr;
}

void PREFIX(free)(void *ptr) {
    beap_lock();
    tlsf_free(tlsf_pool, ptr);

    memory_used -= tlsf_block_size(ptr);
    memory_free += tlsf_block_size(ptr);
    if (memory_free > memory_total) {
        // If memory free exceeds total, free the pages
        size_t pages_to_free =
            (memory_free - memory_total + BEAP_PAGE_SIZE - 1) /
            BEAP_PAGE_SIZE; // Calculate pages to free
        beap_free_pages(ptr, pages_to_free);
        memory_total -= pages_to_free * BEAP_PAGE_SIZE;
        memory_free  -= pages_to_free * BEAP_PAGE_SIZE;
    }

    beap_unlock();
}