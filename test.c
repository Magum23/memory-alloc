#include <stdio.h>
#include <string.h>

void *heap = NULL;

int main(void)
{

    char region[] = {-84, 0, 0, 0};
    printf("%d\n", *(int *)region);

    return 0;
}