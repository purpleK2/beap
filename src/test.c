#include <beap.h>

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
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

    void **pool = malloc(sizeof(void*) * allocations);  // at least this one won't use beap just to be sure it's reliable

    char *str = kcalloc(80, sizeof(char));
    sprintf(str, "This string was allocated with beap :^)\n");

    double start = (double)clock() / CLOCKS_PER_SEC;


    for (int i = 0; i < allocations; i++)
    {
        pool[i] = kmalloc(rand() % 0x1000);
    }

    double end = (double)clock() / CLOCKS_PER_SEC;
    printf("- %llu allocations took %lf seconds\n", allocations, end - start);

    for (int i = 0; i < ALLOCS; i++)
    {
        kfree(pool[i]);
    }

    double end2 = (double)clock() / CLOCKS_PER_SEC;
    printf("- %llu deallocations took %lf seconds\n", allocations, end2 - end);

    printf("%s", str);

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