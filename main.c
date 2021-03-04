#include <stdio.h>
#include <stdint.h> //uint32_t
#include <stdlib.h> //exit()
#include <math.h>   //log(), pow()
#include <string.h> //memset()
#include <time.h>

//Constants
#define MEMORY_HEADER_SIZE 0x4
#define OFFSET_SIZE 0x4
#define HEADER_SIZE 0x4
#define FOOTER_SIZE 0x4
#define MIN_PAYLOAD_SIZE 0x8
#define MIN_BLOCK_SIZE 0x10

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
#define FIRST_BLOCK_OFFSET (MEMORY_HEADER_SIZE + LIST_SIZE((*(int *)heap) + MEMORY_HEADER_SIZE) + HEADER_SIZE)
#define LAST_BLOCK_OFFSSET ((*(int *)heap) + MEMORY_HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)heap + (*(int *)heap) + MEMORY_HEADER_SIZE - FOOTER_SIZE)))

//Type definitions
typedef uint32_t u_int;

//Global variables
void *heap = NULL;

//Function definitions
void *memory_alloc(u_int size);
int get_list_offset(int size);   //get offset of list for certain numbers
int find_free_block(u_int size); //returns offset for appropriate block, uses best fit

int memory_free(void *ptr);
int get_memory_offset(void *ptr);
void case1(void *ptr);
void case2(void *ptr);
void case3(void *ptr);
void case4(void *ptr);

int memory_check(void *ptr);
int check_memory_range(void *ptr);

void memory_init(void *ptr, u_int size);

void z1_testovac(char *region, char **pointer, int minBlock, int maxBlock, int minMemory, int maxMemory, int testFragDefrag)
{
    srand(time(0));
    unsigned int allocated = 0;
    unsigned int mallocated = 0;
    unsigned int allocated_count = 0;
    unsigned int mallocated_count = 0;
    unsigned int i = 0;
    int random_memory = 0;
    int random = 0;
    memset(region, 0, 100000);
    random_memory = (rand() % (maxMemory - minMemory + 1)) + minMemory;
    memory_init(region + 500, random_memory);
    if (testFragDefrag)
    {
        do
        {
            pointer[i] = memory_alloc(8);
            if (pointer[i])
                i++;
        } while (pointer[i]);
        for (int j = 0; j < i; j++)
        {
            if (memory_check(pointer[j]))
            {
                memory_free(pointer[j]);
            }
            else
            {
                printf("Error: Wrong memory check.\n");
            }
        }
    }
    i = 0;
    while (allocated <= random_memory - minBlock)
    {
        random = (rand() % (maxBlock - minBlock + 1)) + minBlock;
        if (allocated + random > random_memory)
            continue;
        allocated += random;
        allocated_count++;
        pointer[i] = memory_alloc(random);
        if (pointer[i])
        {
            i++;
            mallocated_count++;
            mallocated += random;
        }
    }
    for (int j = 0; j < i; j++)
    {
        if (memory_check(pointer[j]))
        {
            memory_free(pointer[j]);
        }
        else
        {
            printf("Error: Wrong memory check.\n");
        }
    }
    memset(region + 500, 0, random_memory);
    for (int j = 0; j < 100000; j++)
    {
        if (region[j] != 0)
        {
            region[j] = 0;
            printf("Error: Modified memory outside the managed region. index: %d\n", j - 500);
        }
    }
    float result = ((float)mallocated_count / allocated_count) * 100;
    float result_bytes = ((float)mallocated / allocated) * 100;
    printf("Memory size of %d bytes: allocated %.2f%% blocks (%.2f%% bytes).\n", random_memory, result, result_bytes);
}

//Main program body
int main(void)
{
    //Different memory sizes
    static const int small_memory_sizes[] = {50, 100, 200};
    static const int large_memory_sizes[] = {1000, 5000, 10000, 25000, 60000};

    // char region[large_memory_sizes[4]];
    // memory_init(region, large_memory_sizes[4]);

    char region[100000];
    char *pointer[13000];
    z1_testovac(region, pointer, 8, 24, 50, 100, 1);
    z1_testovac(region, pointer, 8, 1000, 10000, 20000, 0);
    z1_testovac(region, pointer, 8, 35000, 50000, 99000, 0);

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
        if ((((int)pow(2, n + 1)) - 1 >= size)) //if it is in correct sector or on end of list
        {
            //check if list with that sizes exists
            if ((*((int *)((char *)heap + get_list_offset((int)pow(2, n))))) > 0)
            {
                int offset = (*((int *)((char *)heap + get_list_offset((int)pow(2, n)))));
                do
                {
                    if (*(int *)((char *)(heap) + offset - HEADER_SIZE) >= size)
                    {
                        return offset;
                    }
                    else if (HAS_NEXT((char *)heap + offset))
                    {
                        offset = *(int *)((char *)heap + offset);
                    }
                    else
                    {
                        offset = 0;
                    }
                } while (offset != 0);
                if (offset == 0)
                {
                    continue;
                }
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

        if ((*(int *)((char *)ptr - HEADER_SIZE) - size) < MIN_BLOCK_SIZE) //case if the block after malloc would be so small that it couldnt be used anymore - solution -> merge
        {
            if (HAS_NEXT(ptr) && HAS_PREV(ptr))
            {
                //set prev->next to curr->next
                *(int *)((char *)heap + *(int *)((char *)ptr + OFFSET_SIZE)) = *(int *)ptr;
                //set next->prev to curr->prev
                *(int *)((char *)heap + *(int *)((char *)ptr) + OFFSET_SIZE) = *(int *)((char *)ptr + OFFSET_SIZE);
            }
            else if (HAS_NEXT(ptr)) //if it is first free block of the list
            {
                //get next offset and write it to the free list
                (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)))) = (*(int *)ptr);
                memset(ptr, 1, *(int *)((char *)ptr - HEADER_SIZE));
                //set block header and footer to allocated
                *(int *)((char *)ptr - HEADER_SIZE) = TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE));
                *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE))) = TOGGLE_FULL_FREE(*(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE))));
            }
            else if (HAS_PREV(ptr))
            {
                //set prev->next to 0
                *(int *)((char *)heap + *(int *)((char *)ptr + OFFSET_SIZE)) = 0;
            }
            else //if it is only free block of the list
            {
                memset(ptr, 1, *(int *)((char *)ptr - HEADER_SIZE));
                //remove offset of the free block from the free list
                (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)))) = 0;
                //set block header and footer to allocated
                *(int *)((char *)ptr - HEADER_SIZE) = TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE));
                *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE))) = TOGGLE_FULL_FREE(*(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE))));
            }
            return ptr;
        }

        else //if the free block would be splitted to smaller
        {
            if (HAS_NEXT(ptr) && HAS_PREV(ptr))
            {
                //set prev->next to curr->next
                *(int *)((char *)heap + *(int *)((char *)ptr + OFFSET_SIZE)) = *(int *)ptr;
                //set next->prev to curr->prev
                *(int *)((char *)heap + *(int *)((char *)ptr) + OFFSET_SIZE) = *(int *)((char *)ptr + OFFSET_SIZE);
            }

            else if (HAS_NEXT(ptr))
            {
                //set next->prev to 0
                *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = 0;
                //put next block to the start of the list
                *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(((char *)heap + (*(int *)ptr)));
            }
            else if (HAS_PREV(ptr))
            {
                //set prev->next to 0
                *(int *)((char *)heap + (*(int *)((char *)ptr + OFFSET_SIZE))) = 0;
            }
            else //if the free block was alone in the list
            {

                //remove old offset from list
                *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = 0;
            }
            if (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE)) == 0) //if the list is empty
            {
                //set new list offset
                *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE)) = get_memory_offset((char *)ptr + size + HEADER_SIZE + FOOTER_SIZE);
            }
            else //if there is already free block put it in the begining of the list and set curr-next and next->prev
            {
                //set new free block -> next to another free block of that size
                *(int *)((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE) = *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE));
                //set original free block -> prev to memory offset of new free block
                *(int *)((char *)heap + (*(int *)((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE))) = get_memory_offset((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE);
                //set list offset to new free block
                *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE)) = get_memory_offset((char *)ptr + size + FOOTER_SIZE + HEADER_SIZE);
            }

            //new allocated block footer
            *(int *)((char *)ptr + size) = TOGGLE_FULL_FREE(size);
            //new free block header
            *(int *)((char *)ptr + size + FOOTER_SIZE) = *(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE;
            //new footer size of free block
            *(int *)((char *)ptr + *(int *)((char *)ptr - HEADER_SIZE)) = *(int *)((char *)ptr - HEADER_SIZE) - size - HEADER_SIZE - FOOTER_SIZE;
            //set allcated bytes to 1
            memset(ptr, 1, size);
            //new allocated block header
            *(int *)((char *)ptr - HEADER_SIZE) = TOGGLE_FULL_FREE(size);
        }
    }

    return ptr;
}

int check_memory_range(void *ptr)
{
    for (int i = 0; i <= (*(int *)heap) + MEMORY_HEADER_SIZE; i++)
    {
        if ((char *)ptr == (char *)heap + i)
        {
            return 1;
        }
    }
    return 0;
}

int memory_check(void *ptr)
{
    //check if the input pointer if pointer for sth in the array a.k.a memory
    if (check_memory_range(ptr))
    {
        if (*(int *)((char *)ptr - HEADER_SIZE) < 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

int get_memory_offset(void *ptr)
{
    for (int i = 0; i <= (*(int *)heap) + MEMORY_HEADER_SIZE; i++)
    {
        if ((char *)ptr == (char *)heap + i)
        {
            return i;
        }
    }
    return 0;
}

void case1(void *ptr)
{
    *(int *)((char *)ptr - HEADER_SIZE) = TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE));
    *(int *)((char *)ptr + (*(int *)((char *)ptr - HEADER_SIZE))) = TOGGLE_FULL_FREE(*(int *)((char *)ptr + (*(int *)((char *)ptr - HEADER_SIZE))));
    memset(ptr, 0, *(int *)((char *)ptr - HEADER_SIZE));

    //if the block is free the just save the offset to the list
    if (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) == 0)
    {
        *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
    }
    else
    {
        //save the offset of free block to the our new free block
        *(int *)ptr = *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)));
        //save offset of our free block as prev to the original free block
        *(int *)((char *)heap + *(int *)ptr + OFFSET_SIZE) = get_memory_offset(ptr);
        //put our new free block to the start of the list
        *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
    }
}
void case2(void *ptr)
{
    //copy prev and next from next block
    *(int *)ptr = *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE);
    *(int *)((char *)ptr + OFFSET_SIZE) = *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE + OFFSET_SIZE);
    //set new block header
    *(int *)((char *)ptr - HEADER_SIZE) = ABS(*(int *)((char *)ptr - HEADER_SIZE)) + HEADER_SIZE + FOOTER_SIZE + *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE);
    //check if new free block will stay in the same free list
    if (get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)) == get_list_offset(*(int *)((char *)ptr + *(int *)((char *)ptr - HEADER_SIZE))))
    {
        //if it stays in the same list
        if (HAS_NEXT(ptr) && HAS_PREV(ptr))
        { //if the block is in the middle of free blocks
            //set next->prev to the new offset
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = get_memory_offset(ptr);
            //set prev->next to the new offset
            *(int *)((char *)heap + *(int *)((char *)ptr + OFFSET_SIZE)) = get_memory_offset(ptr);
        }
        else if (HAS_NEXT(ptr))
        {
            //set next->prev to the new offset
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = get_memory_offset(ptr);
            //update offset of the list in the begining
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
        }
        else if (HAS_PREV(ptr))
        {
            //set prev->next to the new offset
            *(int *)((char *)heap + *(int *)((char *)ptr + OFFSET_SIZE) + OFFSET_SIZE) = get_memory_offset(ptr);
        }
        else
        {
            //if it was alone in the list, just update the offset of the list
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
        }
    }
    else //if it doesnt stay in the same list
    {
        if (HAS_NEXT(ptr) && HAS_PREV(ptr))
        { //if the block is in the middle of free blocks
            //set next->prev to the curr->prev
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = *(int *)((char *)ptr + OFFSET_SIZE);
            //set prev->next to the curr->next
            *(int *)((char *)heap + (*(int *)((char *)ptr + OFFSET_SIZE))) = *(int *)ptr;
        }
        else if (HAS_NEXT(ptr))
        {
            //set next->prev to 0
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = 0;
            //set next block to begining of the list
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr + (*(int *)((char *)ptr - HEADER_SIZE))))) = (*(int *)ptr);
        }
        else if (HAS_PREV(ptr))
        {
            //set prev->next to 0
            *(int *)((char *)heap + (*(int *)((char *)ptr + OFFSET_SIZE))) = 0;
        }
        else
        {
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr + *(int *)((char *)ptr - HEADER_SIZE)))) = 0;
        }

        memset(ptr, 0, 2 * OFFSET_SIZE); //remove old offsets

        if (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) == 0)
        {
            //if there is nothing just make it first free block of the list
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
        }
        else
        {
            //set new block next to what was before in the begining of the list
            *(int *)ptr = *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)));
            //set original block -> prev to the new free block
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = get_memory_offset(ptr);
            //save to the begining of the list
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
        }
    }
    //set new block footer
    *(int *)((char *)ptr + *(int *)((char *)ptr - HEADER_SIZE)) = *(int *)((char *)ptr - HEADER_SIZE);
    memset((char *)ptr + 2 * OFFSET_SIZE, 0, ABS(*(int *)((char *)ptr - HEADER_SIZE)) - 2 * OFFSET_SIZE);
}
void case3(void *ptr)
{
    //set new header
    (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) - HEADER_SIZE)) = (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + HEADER_SIZE + FOOTER_SIZE;
    //set footer to original free block size (for future manipulation)
    *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE))) = (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE));

    //set ptr to start of the free block
    ptr = ((char *)ptr - HEADER_SIZE - FOOTER_SIZE - (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)));
    //set memory to 0 except first offset 8 bytes
    memset((char *)ptr + 2 * OFFSET_SIZE, 0, ABS(*(int *)((char *)ptr - HEADER_SIZE)) - 2 * OFFSET_SIZE);

    //check if new free block will not stay in the same free list
    if (!(get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)) == get_list_offset(*(int *)((char *)ptr + *(int *)((char *)ptr - HEADER_SIZE)))))
    {
        //if it doesnt stay in the same list

        if (HAS_NEXT(ptr) && HAS_PREV(ptr))
        { //if the block is in the middle of free blocks
            //set next->prev to the curr->prev
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = *(int *)((char *)ptr + OFFSET_SIZE);
            //set prev->next to the curr->next
            *(int *)((char *)heap + (*(int *)((char *)ptr + OFFSET_SIZE))) = *(int *)ptr;
        }
        else if (HAS_NEXT(ptr))
        {
            //set next->prev to 0
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = 0;
            //set next block to begining of the list
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr + (*(int *)((char *)ptr - HEADER_SIZE))))) = (*(int *)ptr);
        }
        else if (HAS_PREV(ptr))
        {
            //set prev->next to 0
            *(int *)((char *)heap + (*(int *)((char *)ptr + OFFSET_SIZE))) = 0;
        }

        memset(ptr, 0, 2 * OFFSET_SIZE); //remove old offsets

        if (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) == 0)
        {
            //if there is nothing just make it first free block of the list
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
            //set old list block to 0
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr + (*(int *)((char *)ptr - HEADER_SIZE))))) = 0;
        }
        else
        {
            //set new block next to what was before in the begining of the list
            *(int *)ptr = *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)));
            //set original block -> prev to the new free block
            *(int *)((char *)heap + (*(int *)ptr) + OFFSET_SIZE) = get_memory_offset(ptr);
            //save to the begining of the list
            *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
        }
    }

    //set new block footer
    *(int *)((char *)ptr + *(int *)((char *)ptr - HEADER_SIZE)) = *(int *)((char *)ptr - HEADER_SIZE);
}
void case4(void *ptr)
{
    //working with left free block

    if (HAS_NEXT((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE))) && HAS_PREV((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE))))
    {
        //set next->prev to curr->prev
        *(int *)((char *)heap + (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)))) + OFFSET_SIZE) = *(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) + OFFSET_SIZE);
        //set prev->next to curr->next
        *(int *)((char *)heap + (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) + OFFSET_SIZE))) = *(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)));
    }
    else if (HAS_NEXT((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE))))
    {
        //set next->prev to 0
        *(int *)((char *)heap + *(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE))) + OFFSET_SIZE) = 0;
        //set list offset to next block offset
        *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE))) = *(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)));
    }
    else if (HAS_PREV((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE))))
    {
        //set prev->next to 0
        *(int *)((char *)heap + *(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) + OFFSET_SIZE)) = 0;
    }
    else
    {
        //if the free block is alone in the list just remove it
        *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE))) = 0;
    }

    //working with right free block
    if (HAS_NEXT((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE) && HAS_PREV((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE))
    {
        //set next->prev to curr->prev
        *(int *)((char *)heap + (*(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE)) + OFFSET_SIZE) = *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE + OFFSET_SIZE);
        //set prev->next to curr->next
        *(int *)((char *)heap + (*(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE + OFFSET_SIZE))) = *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE);
    }
    else if (HAS_NEXT((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE))
    {
        //set next->prev to 0
        *(int *)((char *)heap + *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE) + OFFSET_SIZE) = 0;
        //set list offset to next block offset
        *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE))) = *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE);
    }
    else if (HAS_PREV((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE))
    {
        //set prev->next to 0
        *(int *)((char *)heap + *(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE + OFFSET_SIZE)) = 0;
    }
    else
    {
        //if the free block is alone in the list just remove it
        *(int *)((char *)heap + get_list_offset(*(int *)(char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE)) = 0;
    }

    //fuse the left side and the right side together

    //set new free block header
    *(int *)((char *)ptr - HEADER_SIZE - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) - FOOTER_SIZE - HEADER_SIZE) = ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) + FOOTER_SIZE + HEADER_SIZE + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE + ABS(*(int *)((char *)ptr + ABS(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE));
    //set new pointer for ease of use
    ptr = ((char *)ptr - ABS(*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE)) - FOOTER_SIZE - HEADER_SIZE);
    //set the free memory to 0
    memset(ptr, 0, *(int *)((char *)ptr - HEADER_SIZE));
    //set new free block footer
    *(int *)((char *)ptr + (*(int *)((char *)ptr - HEADER_SIZE))) = *(int *)((char *)ptr - HEADER_SIZE);
    //add new offset to the list
    if (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) == 0)
    {
        //if the list was empty before, set the list offset to ptr
        *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
    }
    else
    {
        //if there is already free block in the list, just put it in the begining

        *(int *)ptr = *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)));
        *(int *)((char *)heap + (*(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE)))) + OFFSET_SIZE) = get_memory_offset(ptr);
        //put it in the begining of the list
        *(int *)((char *)heap + get_list_offset(*(int *)((char *)ptr - HEADER_SIZE))) = get_memory_offset(ptr);
    }
}

int memory_free(void *ptr)
{
    if (ptr == (char *)heap + FIRST_BLOCK_OFFSET)
    {
        if (*(int *)((char *)ptr + TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE) < 0)
        {
            //if the right side is allocated
            case1(ptr);
        }
        else
        {
            case2(ptr);
        }
    }
    else if (ptr == (char *)heap + LAST_BLOCK_OFFSSET)
    {
        if (*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE) < 0)
        {
            //if the left side id allocated
            case1(ptr);
        }
        else
        {
            case3(ptr);
        }
    }
    else
    {
        /*

        +---------------------+
        |      Allocated      |
        |                     |
        +---------------------+
        |                     |
        |       Current       |
        |                     |
        +---------------------+
        |      Allocated      |
        |                     |
        +---------------------+

    */
        if ((*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE) < 0) && (*(int *)((char *)ptr + TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE) < 0))
        {
            case1(ptr);
        }

        /*

        +---------------------+
        |      Allocated      |
        |                     |
        +---------------------+
        |                     |
        |       Current       |
        |                     |
        +---------------------+
        |         Free        |
        |                     |
        +---------------------+

    */

        else if ((*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE) < 0) && (*(int *)((char *)ptr + TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE) > 0))
        {
            case2(ptr);
        }

        /*
   
        +---------------------+
        |        Free         |
        |                     |
        +---------------------+
        |                     |
        |       Current       |
        |                     |
        +---------------------+
        |      Allocated      |
        |                     |
        +---------------------+

   */
        else if ((*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE) > 0) && (*(int *)((char *)ptr + TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE) < 0))
        {
            case3(ptr);
        }
        /*
  
        +---------------------+
        |        Free         |
        |                     |
        +---------------------+
        |                     |
        |       Current       |
        |                     |
        +---------------------+
        |        Free         |
        |                     |
        +---------------------+

  */
        else if ((*(int *)((char *)ptr - HEADER_SIZE - FOOTER_SIZE) > 0) && (*(int *)((char *)ptr + TOGGLE_FULL_FREE(*(int *)((char *)ptr - HEADER_SIZE)) + FOOTER_SIZE) > 0))
        {
            case4(ptr);
        }
    }
    return 1;
}