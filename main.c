#include <stdio.h>
#include <stdint.h> //uint32_t
#include <stdlib.h> //exit()
#include <math.h>   //log(), pow()
#include <string.h> //memset(), strcmp()
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

void scenario0(int block_size, int mem_size);
void scenario2(int mem_size);
void scenario3();
void scenario4();

//Main program body
// int main(void)
// {
//     char memory[170];
//     heap = memory;
//     memory_init(memory, 170);
//     void *p0 = memory_alloc(15);
//     void *p1 = memory_alloc(15);
//     void *p2 = memory_alloc(15);
//     void *p3 = memory_alloc(15);
//     void *p4 = memory_alloc(15);
//     memory_free(p2);

//     for (int i = 0; i < 170; i++)
//     {
//         int k = memory_check((char *)heap + i);
//         if (k)
//         {
//             printf("%d\n", k);
//         }
//     }
//     return 0;
// }
int main(void)
{
    //Different memory sizes
    static const int small_memory_sizes[] = {50, 100, 200};
    static const int large_memory_sizes[] = {10000, 100000};

    char buff[3];
    printf("Which scenario do you want to run ?\n");
    printf("0a / 0b / 0c / 1a / 1b / 1c / 2 / 3 / 4\n");
    fscanf(stdin, "%s", buff);

    if (strcmp(buff, "0a") == 0)
    {
        scenario0(8, small_memory_sizes[0]);
        scenario0(8, small_memory_sizes[1]);
        scenario0(8, small_memory_sizes[2]);
    }
    else if (strcmp(buff, "0b") == 0)
    {
        scenario0(15, small_memory_sizes[0]);
        scenario0(15, small_memory_sizes[1]);
        scenario0(15, small_memory_sizes[2]);
    }
    else if (strcmp(buff, "0c") == 0)
    {
        scenario0(24, small_memory_sizes[0]);
        scenario0(24, small_memory_sizes[1]);
        scenario0(24, small_memory_sizes[2]);
    }
    else if (strcmp(buff, "1a") == 0)
    {
        char region1[50];
        memory_init(region1, 50);
        char *p0 = memory_alloc(8);
        char *p1 = memory_alloc(8);
        memory_free(p0); //freeing block scenario notMemory/allocated/allocated
        if (!memory_check(p0))
        {
            printf("Not allocated block\n");
        }
        char *p2 = memory_alloc(8); //this will allocate on the same spot as p0 used to be
        char *p3 = memory_alloc(8);
        if (p3 == NULL)
        {
            printf("Memory is full.\n");
        }

        char region2[100];
        memory_init(region2, 100);
        char *p4 = memory_alloc(8);
        char *p5 = memory_alloc(8);
        char *p6 = memory_alloc(8);
        char *p7 = memory_alloc(8);
        memory_free(p4);
        memory_free(p6);
        memory_free(p5); //freeing block where the block is in the middle free/allocated/free
        char *p8 = memory_alloc(8);
        char *p9 = memory_alloc(8);
        char *p10 = memory_alloc(8);

        char region3[200];
        memory_init(region3, 200);
        char *p11 = memory_alloc(8);
        char *p12 = memory_alloc(8);
        char *p13 = memory_alloc(8);
        memory_free(p13); //freeing block scenario allocated/allocated/free
        char *p14 = memory_alloc(8);
        memory_free(p12); //freeing block scenario allocated/allocated/allocated
    }
    else if (strcmp(buff, "1b") == 0)
    {
        char region1[50];
        memory_init(region1, 50);
        char *p0 = memory_alloc(15);
        char *p1 = memory_alloc(15); //cant allocate
        char *p2 = memory_alloc(15); //cant allocate
        memory_free(p0);             //freeing block scenario notMemory/allocated/notMemory

        char region2[100];
        memory_init(region2, 100);
        char *p3 = memory_alloc(15);
        char *p4 = memory_alloc(15);
        char *p5 = memory_alloc(15);
        char *p6 = memory_alloc(15);
        if (memory_check(p5))
        {
            printf("Allocated block\n");
        }
        char *p7 = memory_alloc(15);
        memory_free(p4);
        memory_free(p5); //freeing block scenario free/allocated/notMemory
        char *p8 = memory_alloc(15);

        char region3[200];
        memory_init(region3, 200);
        char *p9 = memory_alloc(15);
        char *p10 = memory_alloc(15);
        char *p11 = memory_alloc(15);
        char *p12 = memory_alloc(15);
        char *p13 = memory_alloc(15);
        char *p14 = memory_alloc(15);
        memory_free(p11);
        memory_free(p12); //freeing block scenario free/allocated/allocated
    }
    else if (strcmp(buff, "1c") == 0)
    {
        char region1[50];
        memory_init(region1, 50);
        char *p0 = memory_alloc(24);
        char *p1 = memory_alloc(24); //cant allocate
        char *p2 = memory_alloc(24); //cant allocate
        if (p2 == NULL)
        {
            printf("Couldnt allocate p2\n");
        }
        memory_free(p0); //freeing block scenario notMemory/allocated/notMemory

        char region2[100];
        memory_init(region2, 100);
        char *p3 = memory_alloc(24);
        char *p4 = memory_alloc(24);
        char *p5 = memory_alloc(24); //this wont be allocated baecause of lack of space
        memory_free(p3);
        memory_free(p4); //just free everything from the memory

        char region3[200];
        memory_init(region3, 200);
        char *p6 = memory_alloc(24);
        char *p7 = memory_alloc(24);
        char *p8 = memory_alloc(24);
        memory_free(p8);
        memory_free(p6);
        char *p9 = memory_alloc(24);
        char *p10 = memory_alloc(24);
        char *p11 = memory_alloc(24);
        memory_free(p11);
    }
    else if (strcmp(buff, "2") == 0)
    {
        scenario2(small_memory_sizes[0]);
        scenario2(small_memory_sizes[1]);
        scenario2(small_memory_sizes[2]);
    }
    else if (strcmp(buff, "3") == 0)
    {
        scenario3();
    }
    else if (strcmp(buff, "4") == 0)
    {
        scenario4();
    }

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

    if ((size % 2) != 0) //make the size devisor of 2
    {
        size += 1;
    }

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
    //check if the input pointer is pointer for sth in the array a.k.a memory
    if (check_memory_range(ptr))
    {
        void *p = ((char *)heap + FIRST_BLOCK_OFFSET);
        void *temp = NULL;
        while (((char *)p) != ((char *)heap + LAST_BLOCK_OFFSSET))
        {
            if ((char *)p == (char *)ptr)
            {
                if (*(int *)((char *)p - HEADER_SIZE) < 0)
                {
                    return 1;
                }
            }
            temp = ((char *)p + (ABS(*(int *)((char *)p - HEADER_SIZE)) + FOOTER_SIZE + HEADER_SIZE));
            p = temp;
        }
        if ((char *)p == (char *)ptr)
        {
            if (*(int *)((char *)p - HEADER_SIZE) < 0)
            {
                return 1;
            }
        }
        return 0;
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
    if (ptr == (char *)heap + FIRST_BLOCK_OFFSET && ptr == (char *)heap + LAST_BLOCK_OFFSSET)
    {
        case1(ptr);
    }
    else if (ptr == (char *)heap + FIRST_BLOCK_OFFSET)
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

void scenario0(int block_size, int mem_size)
{
    int random = 0;
    void *p = NULL;
    float usable_memory_size = FIRST_FREE_BLOCK_SIZE(mem_size) + HEADER_SIZE + FOOTER_SIZE;
    float percentage = 0;
    printf("\n==================================================\n\n");
    printf("Memory allocation scenario 0:\nMemory size: %dB\nActual usable memory size: %dB\nPayload size: %dB", mem_size, (int)usable_memory_size, block_size);

    char region[mem_size];
    memory_init(region, mem_size);
    float counter = 0;
    int full = 0;
    int costs = 0;
    int frag = 0;
    while (!full)
    {
        p = memory_alloc(block_size);
        if (p == NULL)
        {
            full = 1;
        }
        else
        {
            counter += block_size;
            costs += HEADER_SIZE + FOOTER_SIZE;
            frag += ABS(*(int *)((char *)p - HEADER_SIZE)) - block_size;
        }
    }
    percentage = counter / usable_memory_size * 100;
    printf("Memory header and list size: %dB\n", MEMORY_HEADER_SIZE + LIST_SIZE(mem_size));
    printf("Costs: %dB\n", costs);
    printf("Internal fragmentation: %dB\n", frag);
    printf("Intended sum of memory: %dB\n", (int)counter);
    printf("Memory allocated(): %.2f %%\n", percentage);
    printf("\n==================================================\n\n");
}
void scenario2(int mem_size)
{
    srand(time(0));
    int random = 0;
    void *p = NULL;
    float usable_memory_size = FIRST_FREE_BLOCK_SIZE(mem_size) + HEADER_SIZE + FOOTER_SIZE;
    float percentage = 0;
    printf("\n==================================================\n\n");
    printf("Memory allocation scenario 2:\nMemory size: %d\n", mem_size);
    printf("Memory header and list size: %dB\n", MEMORY_HEADER_SIZE + LIST_SIZE(mem_size));

    for (int i = 0; i < 5; i++)
    {
        char region[mem_size];
        memory_init(region, mem_size);
        random = rand() % 16 + 8;
        float counter = 0;
        int full = 0;
        int costs = 0;
        int frag = 0;
        while (!full)
        {
            p = memory_alloc(random);
            if (p == NULL)
            {
                full = 1;
            }
            else
            {
                counter += random;
                costs += HEADER_SIZE + FOOTER_SIZE;
                frag += ABS(*(int *)((char *)p - HEADER_SIZE)) - random;
            }
        }
        percentage = counter / usable_memory_size * 100;
        printf("\nPayload size: %dB\n", random);
        printf("Costs: %dB\n", costs);
        printf("Internal fragmentation: %dB\n", frag);
        printf("Intended sum of memory: %dB\n", (int)counter);
        printf("Memory allocated: %.2f %%\n", percentage);
    }
    printf("\n==================================================\n\n");
}
void scenario3()
{
    const int mem_size = 10000;
    srand(time(0));
    int random = 0;
    void *p = NULL;
    float usable_memory_size = FIRST_FREE_BLOCK_SIZE(mem_size) + HEADER_SIZE + FOOTER_SIZE;
    float percentage = 0;
    printf("\n==================================================\n\n");
    printf("Memory allocation scenario 3:\nMemory size: %d\n", mem_size);
    printf("Memory header and list size: %dB\n", MEMORY_HEADER_SIZE + LIST_SIZE(mem_size));

    for (int i = 0; i < 5; i++)
    {
        char region[mem_size];
        memory_init(region, mem_size);
        random = rand() % 4500 + 500;
        float counter = 0;
        int full = 0;
        int costs = 0;
        int frag = 0;
        while (!full)
        {
            p = memory_alloc(random);
            if (p == NULL)
            {
                full = 1;
            }
            else
            {
                counter += random;
                costs += HEADER_SIZE + FOOTER_SIZE;
                frag += ABS(*(int *)((char *)p - HEADER_SIZE)) - random;
            }
        }
        percentage = counter / usable_memory_size * 100;
        printf("\nPayload size: %dB\n", random);
        printf("Costs: %dB\n", costs);
        printf("Internal fragmentation: %dB\n", frag);
        printf("Intended sum of memory: %dB\n", (int)counter);
        printf("Memory allocated: %.2f %%\n", percentage);
    }
    printf("\n==================================================\n\n");
}
void scenario4()
{
    const int mem_size = 100000;
    srand(time(0));
    int random = 0;
    void *p = NULL;
    float usable_memory_size = FIRST_FREE_BLOCK_SIZE(mem_size) + HEADER_SIZE + FOOTER_SIZE;
    float percentage = 0;
    printf("\n==================================================\n\n");
    printf("Memory allocation scenario 4:\nMemory size: %d\n", mem_size);
    printf("Memory header and list size: %dB\n", MEMORY_HEADER_SIZE + LIST_SIZE(mem_size));

    for (int i = 0; i < 5; i++)
    {
        char region[mem_size];
        memory_init(region, mem_size);
        random = rand() % 49992 + 8;
        float counter = 0;
        int full = 0;
        int costs = 0;
        int frag = 0;
        while (!full)
        {
            p = memory_alloc(random);
            if (p == NULL)
            {
                full = 1;
            }
            else
            {
                counter += random;
                costs += HEADER_SIZE + FOOTER_SIZE;
                frag += ABS(*(int *)((char *)p - HEADER_SIZE)) - random;
            }
        }
        percentage = counter / usable_memory_size * 100;
        printf("\nPayload size: %dB\n", random);
        printf("Costs: %dB\n", costs);
        printf("Internal fragmentation: %dB\n", frag);
        printf("Intended sum of memory: %dB\n", (int)counter);
        printf("Memory allocated: %.2f %%\n", percentage);
    }
    printf("\n==================================================\n\n");
}