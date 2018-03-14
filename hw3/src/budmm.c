/*
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "budmm.h"

/*
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in budmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
extern bud_free_block free_list_heads[NUM_FREE_LIST];

extern char *sbrk_value; // MAKE GLOBAL?

int isEmptyList(int i) {
    if (i >= 0 && i <= 9) {
        if (free_list_heads[i].next == free_list_heads[i].prev)
            return 1;
    }
    return 0;
}


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

void *bud_malloc(uint32_t rsize) {
    // base - error cases
    if (rsize == 0 || rsize > MAX_BLOCK_SIZE - sizeof(bud_header)) {
        errno = EINVAL;
        return NULL;
    }

    // Allocate to the closest power of 2 possible
    int order = 0;
    uint32_t size_alloc = 1;
    while (size_alloc < rsize) {
        size_alloc *= 2;
        order++;
    }

    // Determine payload and header size
    uint32_t size_header = sizeof(bud_header); // Is this right?
    uint32_t size_payload = size_alloc - (rsize - size_header); // Is this right?
    uint32_t size_padding; // IMPLEMENT?

    // Determine if we need to allocate more memory - edit IF condition
    if ((char*)bud_heap_end - (char*)bud_heap_start < size_alloc) {
        void *sbrk_char_value = bud_sbrk();
        sbrk_value = (char*)sbrk_char_value;
    }

    // Case of failure - request CANNOT be satisfied (not enough memory)

    if (sbrk_value == (void*)-1) {
        errno = ENOMEM;
        return NULL;
    }

    // Case of success - request CAN be satisfied (enough memory)

    // First, insert our newly allocated sbrk block, if any, to the free_head_list
    if (free_list_heads[9].next == free_list_heads[9].prev) {
        bud_header newAllocHeader ={0, 14, 0, 16384, 0, 0};
        bud_free_block newFreeBlock = {newAllocHeader, &free_list_heads[9], NULL};
        free_list_heads[9].next = &newFreeBlock;
    }
    else {
        bud_free_block cursor = free_list_heads[9];
        while (cursor.next != NULL) {
            cursor = *(cursor.next);
        }
        bud_header newAllocHeader ={0, 14, 0, 16384, 0, 0};
        bud_free_block newFreeBlock = {newAllocHeader, &free_list_heads[9], NULL};
        cursor.next = &newFreeBlock;
    }



    return NULL;
}

void *bud_realloc(void *ptr, uint32_t rsize) {
    return NULL;
}

void bud_free(void *ptr) {
    return;
}

/**
* Helper function for splitting a large memory block
**/

void *split_block() {
    //
    return NULL;
}

