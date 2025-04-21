/*
    This is an example implementation of beap that should work on a non-freestanding system.

    DO NOT USE THIS IN YOUR PROJECT UNLESS YOU KNOW WHAT YOU'RE DOING
*/

#include <beap.h>

#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>

#include <stdio.h>
#include <stdarg.h>

static pthread_mutex_t BEAP_LOCK = PTHREAD_MUTEX_INITIALIZER;

unsigned long long page_size = 0x1000;

void *beap_alloc(size_t size, size_t *size_out)
{
    size_t aligned = ROUND_UP(size, page_size);
    if (size_out)
        size_out[0] = aligned;

    void *page = mmap(NULL, aligned, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (page == MAP_FAILED) ? NULL : page;
}

void beap_dealloc(void *p, size_t pages)
{
    munmap(p, pages * page_size);
}

// Locks/releases a spinlock/mutex/whatever you want
void beap_lock()
{
    pthread_mutex_lock(&BEAP_LOCK);
}
void beap_unlock()
{
    pthread_mutex_unlock(&BEAP_LOCK);
}

void beap_debug(const char *fmt, ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    int length = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (length < 0 || length >= (int)sizeof(buffer))
    {
        return;
    }

    printf("[ heap::DEBUG ] %s", buffer);
}
