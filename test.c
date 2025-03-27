#include "src/beap.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

// Allocate a 4KB page using mmap
void *mmap_alloc_page() {
  void *page = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return (page == MAP_FAILED) ? NULL : page;
}

// Free a 4KB page using munmap
void mmap_free_page(void *page) { munmap(page, 4096); }

int main() {
  // Initialize heap with mmap-based page allocator
  beap_init(mmap_alloc_page, mmap_free_page);

  // Allocate memory
  char *str = (char *)kmalloc(16);
  if (str) {
    snprintf(str, 16, "Hello, Heap!");
    printf("%s\n", str);
  }

  // Reallocate memory
  str = (char *)krealloc(str, 32);
  if (str) {
    snprintf(str, 32, "Heap Realloc Works!");
    printf("%s\n", str);
  }

  // Allocate zeroed memory
  int *arr = (int *)kcalloc(4, sizeof(int));
  printf("arr[0] = %d\n", arr[0]);

  // Free memory
  kfree(str);
  kfree(arr);

  return 0;
}
