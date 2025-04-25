#include "beap.h"

typedef struct block_header {
  size_t size;
  struct block_header *next;
} block_header;

// Slub allocator structure
typedef struct slub_object {
  struct slub_object *next;
} slub_object;

typedef struct slub_page {
  struct slub_page *next;
  void *page_addr;
  size_t free_objs;
  size_t total_objs;
} slub_page;

typedef struct slub_cache {
  size_t obj_size;
  slub_object *free_objs;
  slub_page *pages;
  size_t objs_per_page;
} slub_cache;

static block_header *free_list = NULL;
static int lock_held = 0; // Track if we're already holding the lock

// SLUB cache array
static slub_cache caches[MAX_SLUB_CACHES];
static int num_caches = 0;

static void safe_lock() {
  if (!lock_held) {
    beap_lock();
    lock_held = 1;
  }
}

static void safe_unlock() {
  if (lock_held) {
    beap_unlock();
    lock_held = 0;
  }
}

void beap_init() {
  beap_lock(); // Initial lock is safe as we're initializing

  lock_held = 1; // Track that we now hold the lock

  num_caches = 0;

  // Create caches for common sizes (powers of 2 work well)
  size_t size = MIN_SLUB_SIZE;
  while (size <= MAX_SLUB_SIZE && num_caches < MAX_SLUB_CACHES) {
    caches[num_caches].obj_size = size;
    caches[num_caches].free_objs = NULL;
    caches[num_caches].pages = NULL;
    caches[num_caches].objs_per_page = (4096 - sizeof(slub_page)) / size;
    num_caches++;
    size *= 2;
  }
  beap_unlock();
  lock_held = 0; // Track that we released the lock
}

// Find appropriate cache for requested size
static slub_cache *find_cache(size_t size) {
  // Don't lock here as the caller should already hold the lock
  for (int i = 0; i < num_caches; i++) {
    if (size <= caches[i].obj_size) {
      return &caches[i];
    }
  }
  return NULL; // No suitable cache found
}

// Allocate a new page for the cache and divide it into objects
static int refill_cache(slub_cache *cache) {
  // Don't lock here as the caller should already hold the lock

  void *page_mem = beap_alloc_page();
  if (!page_mem) {
    return 0;
  }

  slub_page *page = (slub_page *)page_mem;
  page->page_addr = page_mem;
  page->total_objs = cache->objs_per_page;
  page->free_objs = cache->objs_per_page;

  // Link page into cache
  page->next = cache->pages;
  cache->pages = page;

  // Create free object list
  char *obj_start = (char *)page_mem + sizeof(slub_page);
  for (size_t i = 0; i < page->total_objs; i++) {
    slub_object *obj = (slub_object *)(obj_start + i * cache->obj_size);
    obj->next = cache->free_objs;
    cache->free_objs = obj;
  }

  return 1;
}

void coalesce() {
  // Don't lock here as the caller should already hold the lock
  block_header *curr = free_list;
  while (curr && curr->next) {
    if ((char *)curr + curr->size + sizeof(block_header) ==
        (char *)curr->next) {
      curr->size += curr->next->size + sizeof(block_header);
      curr->next = curr->next->next;
    } else {
      curr = curr->next;
    }
  }
}

void *beap_memset(void *ptr, int value, size_t num) {
  // Don't acquire lock as the caller should already hold it
  unsigned char *p = ptr; // Cast the pointer to an unsigned char pointer
  for (size_t i = 0; i < num; i++) {
    p[i] = (unsigned char)value; // Set each byte to the specified value
  }
  return ptr; // Return the original pointer
}

void *beap_memcpy(void *dest, const void *src, size_t n) {
  // Don't acquire lock as the caller should already hold it
  unsigned char *d = dest;
  const unsigned char *s = src;
  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
}

void *BEAP_PREFIX(malloc)(size_t size) {
  safe_lock();
  // Align the size to 8 bytes
  size = BEAP_ALIGN8(size);

  // For small allocations, use SLUB cache
  slub_cache *cache = find_cache(size);
  if (cache) {
    // If no free objects, refill the cache
    if (!cache->free_objs) {
      if (!refill_cache(cache)) {
        safe_unlock();
        return NULL; // Failed to allocate new page
      }
    }

    // Remove object from free list
    slub_object *obj = cache->free_objs;
    cache->free_objs = obj->next;

    // Clear object memory
    beap_memset(obj, 0, cache->obj_size);

    safe_unlock();

    return obj;
  }

  // For larger allocations, use the existing buddy system
  block_header *prev = NULL;
  block_header *curr = free_list;

  while (curr) {
    if (curr->size >= size) {
      if (curr->size > size + sizeof(block_header)) {
        block_header *new_block =
            (block_header *)((char *)curr + sizeof(block_header) + size);
        new_block->size = curr->size - size - sizeof(block_header);
        new_block->next = curr->next;

        curr->size = size;
        curr->next = NULL;

        if (prev) {
          prev->next = new_block;
        } else {
          free_list = new_block;
        }
      } else {
        if (prev) {
          prev->next = curr->next;
        } else {
          free_list = curr->next;
        }
      }
      safe_unlock();
      return (void *)(curr + 1);
    }
    prev = curr;
    curr = curr->next;
  }

  block_header *new_page = (block_header *)beap_alloc_page();
  if (!new_page) {
    safe_unlock();
    return NULL;
  }

  new_page->size = 4096 - sizeof(block_header);
  new_page->next = free_list;
  free_list = new_page;

  safe_unlock();
  return BEAP_PREFIX(malloc)(size);
}

// Helper function to check if a pointer belongs to a SLUB cache
static slub_cache *get_cache_for_ptr(void *ptr) {
  // Don't acquire the lock here as the caller should already hold it
  for (int i = 0; i < num_caches; i++) {
    slub_page *page = caches[i].pages;
    while (page) {
      // Check if ptr is within this page's objects
      char *page_start = (char *)page + sizeof(slub_page);
      char *page_end =
          page_start + (caches[i].obj_size * caches[i].objs_per_page);

      if ((char *)ptr >= page_start && (char *)ptr < page_end) {
        return &caches[i];
      }

      page = page->next;
    }
  }
  return NULL;
}

void BEAP_PREFIX(free)(void *ptr) {
  safe_lock();
  if (!ptr) {
    safe_unlock();
    return;
  }

  // Check if ptr belongs to SLUB cache
  slub_cache *cache = get_cache_for_ptr(ptr);
  if (cache) {
    // Return object to cache's free list
    slub_object *obj = (slub_object *)ptr;
    obj->next = cache->free_objs;
    cache->free_objs = obj;
    safe_unlock();
    return;
  }

  // Otherwise use existing buddy system
  block_header *block = (block_header *)ptr - 1;
  block->next = free_list;
  free_list = block;

  coalesce();
  safe_unlock();
}

void *BEAP_PREFIX(calloc)(size_t num, size_t size) {
  // Don't acquire lock here since malloc will handle locking
  size_t total_size = num * size;
  void *ptr = BEAP_PREFIX(malloc)(total_size);
  if (ptr) {
    beap_memset(ptr, 0, total_size);
  }
  return ptr;
}

void *BEAP_PREFIX(realloc)(void *ptr, size_t size) {
  if (!ptr)
    return BEAP_PREFIX(malloc)(size);
  if (size == 0) {
    BEAP_PREFIX(free)(ptr);
    return NULL;
  }

  safe_lock();

  // Check if ptr is from SLUB cache
  slub_cache *cache = get_cache_for_ptr(ptr);
  if (cache) {
    if (size <= cache->obj_size) {
      // Object can fit in the same cache
      safe_unlock();
      return ptr;
    } else {
      safe_unlock();
      // Allocate new space and copy data
      void *new_ptr = BEAP_PREFIX(malloc)(size);
      if (new_ptr) {
        safe_lock();
        beap_memcpy(new_ptr, ptr, cache->obj_size);
        safe_unlock();
        BEAP_PREFIX(free)(ptr);
      }
      return new_ptr;
    }
  }

  // Handle buddy system allocated memory
  block_header *old_block = (block_header *)ptr - 1;
  if (old_block->size >= size) {
    safe_unlock();
    return ptr; // Already fits
  }

  safe_unlock();

  void *new_ptr = BEAP_PREFIX(malloc)(size);
  if (new_ptr) {
    safe_lock();
    beap_memcpy(new_ptr, ptr, old_block->size);
    safe_unlock();
    BEAP_PREFIX(free)(ptr);
  }
  return new_ptr;
}
