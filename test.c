#include <stdio.h>
#include <string.h>

void *heap = NULL;

int main(void)
{

    char region[] = {-1, -1, -1, 1};
    printf("%d\n", *(int *)region);

    return 0;
}