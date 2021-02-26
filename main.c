#include <stdio.h>
#include <stdint.h> //uint32_t
#include <stdlib.h> //exit()
#include <math.h>   //log(), pow()
#include <string.h> //memset()

//Constants
#define MEMORY_HEADER_SIZE 0x4
#define OFFSET_SIZE 0x4
#define HEADER_SIZE 0x4
#define FOOTER_SIZE 0x4
#define MIN_PAYLOAD_SIZE 0x8

//Macros
#define THROW_ERROR(message) (fprintf(stdout, "%s\n", message), exit(1))
#define LIST_COUNT(size) ((int)(floor((log(size) / log(2))) - 2))
#define LIST_SIZE(size) (LIST_COUNT(size) * OFFSET_SIZE)
#define FIRST_FREE_BLOCK_SIZE(size) (size - MEMORY_HEADER_SIZE - LIST_SIZE(size) - HEADER_SIZE - FOOTER_SIZE)
#define GET_MEMORY_SIZE(heap) ((*(int *)heap) + MEMORY_HEADER_SIZE) //size of full memory
#define HAS_NEXT(ptr) ((*(int *)ptr) ? 1 : 0)
#define HAS_PREV(ptr) ((*(int *)((char *)ptr + OFFSET_SIZE)) ? 1 : 0)
#define GET_NEXT_OFFSET(ptr) (*(int *)ptr)
#define GET_PREV_OFFSET(ptr) (*(int *)((char *)ptr + OFFSET_SIZE))
#define TOGGLE_FULL_FREE(size) (size * (-1))
#define ABS(x) (((x) < 0) ? -(x) : (x))

//Type definitions
typedef uint32_t u_int;

//Global variables
void *heap = NULL;

//Function definitions
void *memory_alloc(u_int size);
int get_list_offset(int size);   //get offset of list for certain numbers
int find_free_block(u_int size); //returns offset for appropriate block, uses best fit

int memory_free(void *valid_ptr);

int memory_check(void *ptr);

void memory_init(void *ptr, u_int size);

//Main program body
int main(void)
{
    //Different memory sizes
    static const int small_memory_sizes[] = {50, 100, 200};
    static const int large_memory_sizes[] = {1000, 5000, 10000, 25000, 50000};

    char region[small_memory_sizes[2]];
    memory_init(region, small_memory_sizes[2]);
    memory_alloc(10);
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

    //Clear the array a.k.a "heap"
    memset(ptr, 0, size);

    //Memory header
    *((int *)ptr) = size - MEMORY_HEADER_SIZE; //removed  - LIST_SIZE(size)
    //Create free block
    //First casting to char (because of size 1B), then to int so I can write the offset to 4B as integer
    *((int *)(((char *)ptr) + MEMORY_HEADER_SIZE + LIST_SIZE(size))) = FIRST_FREE_BLOCK_SIZE(size); //Header of first block
    *((int *)(((char *)ptr) + size - FOOTER_SIZE)) = FIRST_FREE_BLOCK_SIZE(size);                   //Footer of first block

    for (int n = 3; n <= LIST_COUNT(size) + 2; n++) //starting from 2^3, there is LIST_COUNT+2 because I dont use super small blocks however I have to use the power
    {
        if (pow(2, n + 1) - 1 >= FIRST_FREE_BLOCK_SIZE(size) || n == LIST_COUNT(size) + 2) //if it is in correct sector or on end of list
        {
            *((int *)(((char *)ptr) + MEMORY_HEADER_SIZE + (n - 3) * OFFSET_SIZE)) = MEMORY_HEADER_SIZE + LIST_SIZE(size) + HEADER_SIZE; //save offset to first block
            break;
        }
    }
}

int get_list_offset(int size) // returns
{
    for (int n = 1; pow(2, n) <= size; n++)
    {
        if (size < MIN_PAYLOAD_SIZE)
        {
            return (MEMORY_HEADER_SIZE + 0 * OFFSET_SIZE);
        }
        if (size >= pow(2, n) && size < pow(2, n + 1))
        {
            return (MEMORY_HEADER_SIZE + (n - 3) * OFFSET_SIZE);
        }
    }
}

int find_free_block(u_int size)
{
    for (int n = 3; n <= (LIST_COUNT(GET_MEMORY_SIZE(heap)) + 2); n++) //starting from 2^3, there is LIST_COUNT+2 because I dont use super small blocks however I have to use the power
    {
        if ((((int)pow(2, n + 1)) - 1 >= size + HEADER_SIZE + FOOTER_SIZE)) //if it is in correct sector or on end of list
        {
            if ((*((int *)((char *)heap + get_list_offset((int)pow(2, n))))) > 0)
            {
                // printf("this %d\n", *((int *)((char *)heap + get_list_offset((int)pow(2, n)))));
                return (*((int *)((char *)heap + get_list_offset((int)pow(2, n)))));
            }
        }
    }
    return 0;
}

void *memory_alloc(u_int size)
{
    void *ptr = NULL;

    if (find_free_block(size) != 0) //if free block is avalaible
    {
        ptr = (char *)heap + find_free_block(size);

        if ((*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE) < MIN_PAYLOAD_SIZE) //case if the block after malloc would be so small that it couldnt be used anymore - solution -> merge
        {

            if (HAS_NEXT(ptr)) //if it is first free block of the list
            {
                memset((char *)heap + find_free_block(size), 1, *(int *)((char *)ptr - HEADER_SIZE));
                (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)))) = (*(int *)ptr);
            }
            else //if it is only free block of the list
            {
                memset((char *)heap + find_free_block(size), 1, *(int *)((char *)ptr - HEADER_SIZE));
                (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)))) = 0;
            }
        }

        else //if the free block would be splitted to smaller
        {

            if (HAS_NEXT(ptr))
            {
                //write next offset to new block
                *(int *)((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE) = *(int *)ptr;
                *(int *)((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE + OFFSET_SIZE) = *(int *)((char *)ptr + OFFSET_SIZE);
            }
            else //if the free block was alone in the list
            {
                if (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE)) == 0) //if the list is empty
                {
                    //set new list offset
                    *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE)) = (*(int *)((char *)heap + get_list_offset((*(int *)((char *)ptr - HEADER_SIZE))))) + size + FOOTER_SIZE + HEADER_SIZE;
                }
                else //if there is already free block
                {
                    //TODO if it stays in same block I dont have to do this

                    if (!(get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)) == get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE)))
                    {

                        //add new free block to list start by setting next for another list
                        *(int *)((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE) = *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE));
                        //set prev for existing free block
                        *(int *)((char *)heap + (*(int *)((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE))) = (*(int *)((char *)heap + get_list_offset((*(int *)((char *)ptr - HEADER_SIZE))))) + size + FOOTER_SIZE + HEADER_SIZE;
                    }
                    //add to start of list
                    *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE)) = (*(int *)((char *)heap + get_list_offset((*(int *)((char *)ptr - HEADER_SIZE))))) + size + FOOTER_SIZE + HEADER_SIZE;
                }

                //check weather allocation of new block changes size that much that the free block has to be moved to smaller list
                if (!(get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE) == get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))))
                {
                    //remove old offset from list
                    *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = 0;
                }

                //new allocated block footer
                *(int *)((char *)ptr + size) = size;
                //new free block header
                *(int *)((char *)ptr + size + FOOTER_SIZE) = *(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE;
                //new footer size of free block
                *(int *)((char *)ptr + *(int *)((char *)ptr - HEADER_SIZE)) = *(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE;
                //set allcated bytes to 1
                memset(ptr, 1, size);
                //new allocated block header
                *(int *)((char *)ptr - HEADER_SIZE) = size;
            }

            //new free block ((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE)
        }
    }

    return ptr;
}