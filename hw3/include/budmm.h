/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef BUDMM_H
#define BUDMM_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ORDER_MIN 5  /* Min block size 32 */
#define ORDER_MAX 15
#define ORDER_TO_BLOCK_SIZE(ord) (1 << (ord))
#define MIN_BLOCK_SIZE (ORDER_TO_BLOCK_SIZE(ORDER_MIN))
#define MAX_BLOCK_SIZE (ORDER_TO_BLOCK_SIZE(ORDER_MAX-1))
#define MAX_HEAP_SIZE (4 * ORDER_TO_BLOCK_SIZE(ORDER_MAX-1))
#define NUM_FREE_LIST (ORDER_MAX - ORDER_MIN)

/*
                                 Format of an allocated memory block
    +---------------------------------------------------------------------------------------+
    |                                      64-bits wide                                     |
    +---------------------------------------------------------------------------------------+

    +-------------------------------------------+----------------+-------+------+-----+-----+ <- header
    |                unused                     | requested_size |unused |padded|order|alloc|
    |                                           |                |       |  1   |  4  |  1  |
    |                32 bits                    |    16 bits     |10 bits| bits | bits| bit |
    +-------------------------------------------+----------------+-------+------+-----+-----+
    |                                                                                       |
    |                                   Payload and Padding                                 |
    |                                                                                       |
    +---------------------------------------------------------------------------------------+
*/

/*
                                      Format of a free block
    +---------------------------------------------------------------------------------------+
    |                                      64-bits wide                                     |
    +---------------------------------------------------------------------------------------+

    +-------------------------------------------+----------------+-------+------+-----+-----+ <- header
    |                unused                     | requested_size |unused |padded|order|alloc|
    |                                           |                |       |  1   |  4  |  1  |
    |                32 bits                    |    16 bits     |10 bits| bits | bits| bit |
    +-------------------------------------------+----------------+-------+------+-----+-----+
    |                                      next pointer                                     |
    |                                                                                       |
    |                                         64 bits                                       |
    +---------------------------------------------------------------------------------------+
    |                                      prev pointer                                     |
    |                                                                                       |
    |                                         64 bits                                       |
    +---------------------------------------------------------------------------------------+
    |                                                                                       |
    |                                        FREE AREA                                      |
    |                                                                                       |
    +---------------------------------------------------------------------------------------+
*/

/* Struct for a block header (64 bits) */
typedef struct {
    uint64_t allocated : 1;
    uint64_t     order : 4;
    uint64_t    padded : 1;
    uint64_t   unused1 : 10;
    uint64_t     rsize : 16;
    uint64_t   unused2 : 32;
} __attribute__((packed)) bud_header;

/* Struct for a free block */
typedef struct bud_free_block {
    bud_header header;
    struct bud_free_block *next;
    struct bud_free_block *prev;
} bud_free_block;

/*
 * Header nodes of the segregated free list.
 * These serve as sentinels for circular, doubly linked lists of free blocks.
 */
bud_free_block free_list_heads[NUM_FREE_LIST];

/*
 * This is your implementation of bud_malloc. It acquires uninitialized memory that
 * is aligned and padded properly for the underlying system.
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, a pointer to a valid region of memory of the
 * requested size is returned, else NULL is returned and errno as follows:
 *
 * If the size passed is invalid, errno is set to EINVAL.
 * If the request cannot be satisfied, errno is set to ENOMEM.
 */
void *bud_malloc(uint32_t size);

/*
 * Resizes the memory pointed to by ptr to size bytes.
 *
 * @param ptr Address of the memory region to resize.
 * @param size The minimum size to resize the memory to.
 *
 * @return If successful, a pointer to a valid region of memory is
 * returned, else NULL is returned and errno is set appropriately.
 *
 * A call to bud_realloc with a ptr == NULL should be equivalent to a
 * call to bud_malloc(size).
 * A call to bud_realloc with size == 0 should be equivalent to a
 * call to bud_free(ptr).
 * If bud_realloc is called with an invalid ptr it should call abort()
 * to exit the program.
 * If there is no memory available bud_realloc should set errno to ENOMEM.
 */
void *bud_realloc(void *ptr, uint32_t size);

/*
 * Marks a dynamically allocated region as no longer in use.
 * Adds the newly freed block to the free list.
 *
 * @param ptr Address of memory returned by the function bud_malloc.
 *
 * If bud_free is called with an invalid ptr, it should call abort()
 * to exit the program.
 */
void bud_free(void *ptr);

/* budutil.c: Helper functions already created for this assignment. */

/*
 * Any program using the budmm library must call this function ONCE
 * before issuing any allocation requests. This function DOES NOT
 * allocate any space to your allocator.
 */
void bud_mem_init();

/*
 * Any program using the budmm library must call this function ONCE
 * after all allocation requests are complete. If implemented cleanly,
 * your program should have no memory leaks in valgrind after this function
 * is called.
 */
void bud_mem_fini();

/*
 * This function changes the position of your program's break.
 * Each time it is called, the break pointer is increased by BLOCK_SIZE_MAX.
 *
 * @return On success, this function returns the previous program break.
 * If the break was increased, this value is a pointer to the start of the newly
 * allocated memory. On error, (void *)-1 is returned and errno is set to ENOMEM.
 */
void *bud_sbrk();

/*
 * @return The starting address of the heap for your allocator.
 */
void *bud_heap_start();

/*
 * @return The ending address of the heap for your allocator.
 */
void *bud_heap_end();

#endif
