#include "beap.h"

typedef struct block_header {
  size_t size;
  struct block_header *next;
} block_header;

static block_header *free_list = NULL;
static beap_page_alloc_func alloc_page = NULL;
static beap_page_free_func free_page = NULL;

void beap_init(beap_page_alloc_func alloc, beap_page_free_func free) {
  alloc_page = alloc;
  free_page = free;
}

void coalesce() {
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
  unsigned char *p = ptr; // Cast the pointer to an unsigned char pointer
  for (size_t i = 0; i < num; i++) {
    p[i] = (unsigned char)value; // Set each byte to the specified value
  }
  return ptr; // Return the original pointer
}

void *beap_memcpy(void *dest, const void *src, size_t n) {
  unsigned char *d = dest;
  const unsigned char *s = src;
  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
}

void *FUNC_PREFIX(malloc)(size_t size) {
  size = ALIGN8(size);
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
      return (void *)(curr + 1);
    }
    prev = curr;
    curr = curr->next;
  }

  if (!alloc_page)
    return NULL;
  block_header *new_page = (block_header *)alloc_page();
  if (!new_page)
    return NULL;

  new_page->size = 4096 - sizeof(block_header);
  new_page->next = free_list;
  free_list = new_page;

  return FUNC_PREFIX(malloc)(size);
}

void FUNC_PREFIX(free)(void *ptr) {
  if (!ptr)
    return;

  block_header *block = (block_header *)ptr - 1;
  block->next = free_list;
  free_list = block;

  coalesce();
}

void *FUNC_PREFIX(calloc)(size_t num, size_t size) {
  size_t total_size = num * size;
  void *ptr = FUNC_PREFIX(malloc)(total_size);
  if (ptr) {
    beap_memset(ptr, 0, total_size);
  }
  return ptr;
}

void *FUNC_PREFIX(realloc)(void *ptr, size_t size) {
  if (!ptr)
    return FUNC_PREFIX(malloc)(size);
  if (size == 0) {
    FUNC_PREFIX(free)(ptr);
    return NULL;
  }

  block_header *old_block = (block_header *)ptr - 1;
  if (old_block->size >= size) {
    return ptr; // Already fits
  }

  void *new_ptr = FUNC_PREFIX(malloc)(size);
  if (new_ptr) {
    beap_memcpy(new_ptr, ptr, old_block->size);
    FUNC_PREFIX(free)(ptr);
  }
  return new_ptr;
}
