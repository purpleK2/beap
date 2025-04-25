#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include <beap.h>

static pthread_mutex_t allocator_lock = PTHREAD_MUTEX_INITIALIZER;

void *beap_alloc_pages(size_t pages) {
    // Using mmap to allocate memory, aligned to the page boundary
    size_t page_size = sysconf(_SC_PAGE_SIZE);
    void *addr       = mmap(NULL, pages * page_size, PROT_READ | PROT_WRITE,
                            MAP_ANON | MAP_PRIVATE, -1, 0);
    return addr == MAP_FAILED ? NULL : addr;
}

void beap_free_pages(void *ptr, size_t pages) {
    size_t page_size = sysconf(_SC_PAGE_SIZE);
    munmap(ptr, pages * page_size);
}

void beap_lock() {
    pthread_mutex_lock(&allocator_lock);
}

void beap_unlock() {
    pthread_mutex_unlock(&allocator_lock);
}

void test_malloc_free() {
    printf("Testing malloc and free...\n");
    void *ptr = kmalloc(128);
    if (ptr == NULL) {
        printf("malloc failed!\n");
    } else {
        printf("malloc succeeded!\n");
        kfree(ptr);
        printf("Memory freed.\n");
    }
}

void test_realloc() {
    printf("Testing realloc...\n");
    void *ptr = kmalloc(128);
    ptr       = krealloc(ptr, 256);
    if (ptr == NULL) {
        printf("realloc failed!\n");
    } else {
        printf("realloc succeeded!\n");
        kfree(ptr);
    }
}

void test_calloc() {
    printf("Testing calloc...\n");
    void *ptr = kcalloc(8, 16);
    if (ptr == NULL) {
        printf("calloc failed!\n");
    } else {
        printf("calloc succeeded!\n");
        kfree(ptr);
    }
}

/*int main(int argc, char **argv) {

    tlsf_beap_init();

    test_malloc_free();
    test_realloc();
    test_realloc();

    return 0;
}*/

#define ALLOCS 10000000ull

int main(int argc, char **argv) {

    tlsf_beap_init();

    if (argc > 2 || argc < 1) {
        printf("u dumb?\n");
        return -1;
    }

    unsigned long long allocations = (argc < 2 ? ALLOCS : atoi(argv[1]));

    srand(time(NULL));

    void **pool = malloc(sizeof(void *) *
                         allocations); // at least this one won't use beap just
                                       // to be sure it's reliable

    double start = (double)clock() / CLOCKS_PER_SEC;

    for (int i = 0; i < allocations; i++) {
        pool[i] = kmalloc(rand() % 0x1000);
    }

    double end = (double)clock() / CLOCKS_PER_SEC;
    printf("- %llu allocations took %lf seconds\n", allocations, end - start);

    for (int i = 0; i < allocations; i++) {
        kfree(pool[i]);
    }

    double end2 = (double)clock() / CLOCKS_PER_SEC;
    printf("- %llu deallocations took %lf seconds\n", allocations, end2 - end);

    char *str = kcalloc(80, sizeof(char));
    sprintf(str, "This string was allocated with beap :^)\n");
    printf("%s", str);

    kfree(str);
    free(pool);
}
