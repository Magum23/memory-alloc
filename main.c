#include <stdio.h>
#include <stdint.h> //uint32_t
#include <stdlib.h> //exit()
#include <math.h>   //log()
#include <string.h> //memset()

//Constants
#define MEMORY_HEADER_SIZE 0x4
#define OFFSET_SIZE 0x4

//Macros
#define THROW_ERROR(message) (fprintf(stdout, "%s\n", message), exit(1))
#define LIST_COUNT(size) (log(/))

//Type definitions
typedef uint32_t u_int;

//Global variables
void *heap = NULL;

//Function definitions
void *memory_alloc(u_int size);
int memory_free(void *valid_ptr);
int memory_check(void *ptr);
void memory_init(void *ptr, u_int size);

//Main program body
int main(void)
{
    //Different memory sizes
    static const int small_memory_sizes[] = {50, 100, 200};
    static const int large_memory_sizes[] = {1000, 5000, 10000, 25000, 50000};

    char region[large_memory_sizes[0]];
    memory_init(region, large_memory_sizes[0]);
    return 0;
}

//Functions declarations
void memory_init(void *ptr, u_int size)
{
    heap = NULL; //set heap to null in case it was used before

    if (ptr == NULL)
    {
        THROW_ERROR("Pointer argument to memory_init is NULL");
    }
    else if (size <= 0)
    {
        THROW_ERROR("You can't initialize space which size is zero or less.");
    }

    heap = ptr; //when there is no error assign ptr address to heap global variable

    //Clear array a.k.a "heap"
    memset(ptr, 0, size);

    //Memory header
    *((int *)ptr) = size - MEMORY_HEADER_SIZE;
}