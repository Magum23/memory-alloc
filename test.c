#include <stdio.h>
#include <string.h>

void *heap = NULL;

int main(void)
{

    char region[100];
    heap = region;

    memset(heap, 1, 100);
    void *ptr = heap;

    int a = 5;

    *((void **)(ptr)) = &a;

    return 0;
}