#include "src/beap.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *mmap_alloc_page() {
  void *page = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  printf("Allocating page at %p\n", page);
  return (page == MAP_FAILED) ? NULL : page;
}

void test_lock() {
  printf("Locking...\n");
  pthread_mutex_lock(&mutex);
}
void test_unlock() {
  printf("Unlocking...\n");
  pthread_mutex_unlock(&mutex);
}

void mmap_free_page(void *page) {
  printf("Freeing page at %p\n", page);
  munmap(page, 4096);
}

int main() {
  beap_init(mmap_alloc_page, mmap_free_page, test_lock, test_unlock);

  char *str = (char *)kmalloc(16);
  if (str) {
    snprintf(str, 16, "Hello, Heap!");
    printf("%s\n", str);
  }

  str = (char *)krealloc(str, 32);
  if (str) {
    snprintf(str, 32, "Heap Realloc Works!");
    printf("%s\n", str);
  }

  int *arr = (int *)kcalloc(4, sizeof(int));
  printf("arr[0] = %d\n", arr[0]);

  kfree(str);
  kfree(arr);

  return 0;
}
