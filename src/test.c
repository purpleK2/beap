#include <beap.h>

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define ALLOCS 1000ull

int decStringToInt(char *str);

int main(int argc, char **argv)
{
    if (argc > 2 || argc < 1)
    {
        printf("u dumb?\n");
        return -1;
    }

    int allocations = (argc < 2 ? ALLOCS : decStringToInt(argv[1]));

    srand(time(NULL));

    void **pool = malloc(8 * allocations);  // at least this one won't use beap just to be sure it's reliable

    double start = (double)clock() / CLOCKS_PER_SEC;

    beap_init();

    for (int i = 0; i < allocations; i++)
    {
        pool[i] = kmalloc(rand() % 0x1000);
    }

    double end = (double)clock() / CLOCKS_PER_SEC;
    printf("- %llu allocations took %lf seconds\n", allocations, end - start);

    for (int i = 0; i < allocations; i++)
    {
        kfree(pool[i]);
    }

    double end2 = (double)clock() / CLOCKS_PER_SEC;
    printf("- %llu deallocations took %lf seconds\n", allocations, end2 - end);

    char *str = (char *)kmalloc(16);
    if (str) {
      snprintf(str, 16, "Hello, Beap!");
      printf("%s\n", str);
    }
  
    str = (char *)krealloc(str, 32);
    if (str) {
      snprintf(str, 32, "Beap Realloc Works!");
      printf("%s\n", str);
    }  

    kfree(str);
    free(pool);
}

int decStringToInt(char *str)
{
    if (!str)
        return -1;

    size_t len = strlen(str);

    int dec = 0;

    for (int i = 0; i < strlen(str); i++) {
        dec *= 10;
        dec += str[i] - 48;
    }

    return dec;
}