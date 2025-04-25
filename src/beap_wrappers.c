#include <stdio.h>

#include <sys/mman.h>
#include <pthread.h>

#include <beap.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *beap_alloc_page() {
  void *page = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  printf("Allocating page at %p\n", page);
  return (page == MAP_FAILED) ? NULL : page;
}

void beap_lock() {
  // printf("Locking...\n");
  pthread_mutex_lock(&mutex);
}
void beap_unlock() {
  // printf("Unlocking...\n");
  pthread_mutex_unlock(&mutex);
}

void beap_free_page(void *page) {
  printf("Freeing page at %p\n", page);
  munmap(page, 4096);
}
