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

void free_list_heads_insert(bud_free_block *block, int order) {
    int index = order - 5;

    bud_free_block cursor = free_list_heads[index];
    // Empty list condition
    if (free_list_heads[index].next == free_list_heads[index].prev) {
        block -> prev = &free_list_heads[index];
        free_list_heads[index].next = block;
        return;
    }

    while (cursor.next != NULL) {
        cursor = *(cursor.next);
    }
    block -> prev = &cursor;
    free_list_heads[index].next = block;
}

int isEmptyList(int i) {
    if (i >= 0 && i <= 9) {
        if (free_list_heads[i].next == free_list_heads[i].prev)
            return 1;
    }
    return 0;
}

uint32_t getOrder(uint32_t n) {
    uint32_t order = 0;
    while (n > 1) {
        n /= 2;
        order++;
    }
    return order;
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
    int sbrk_call = 0;
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
    if (sbrk_call) {
        bud_header newAllocHeader = {};
        // CHECK TO SEE THAT THESE ARE THE VALUES YOU ARE SUPPOSED TO SET
        newAllocHeader.allocated = 0;
        newAllocHeader.order = 14;
        newAllocHeader.padded = 0;

        bud_free_block newFreeBlock = {newAllocHeader, NULL, NULL};
        free_list_heads_insert(&newFreeBlock, (int)newAllocHeader.order);
    }

    // Now, take the BIG block, and split it until we get a "almost perfect fit"

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
*
* Returns start of block intended to be allocated, places others on the free_list_heads[]
**/
// *** MAY NEED TO FIX THIS FUNCTION ***
void *split_block(uint32_t rsize, bud_free_block *block) {
    uint32_t size_block = sizeof(*block); // YOU MAY NEED TO FIX THIS
    uint32_t new_size_block = size_block / 2;
    if (new_size_block < rsize)
        return block;

    // Should we split the block further?
    // This is the "rightmost" block, that should be put in the free list for later use
    bud_free_block *new_free_block = block + new_size_block;
    // First, set bits and block properties accordingly before adding to list
    new_free_block -> header.allocated = 0; new_free_block -> header.padded = 0;
    uint32_t new_block_order = getOrder(new_size_block);
    new_free_block -> header.order = (uint64_t)new_block_order;

    // Store to free list
    free_list_heads_insert(new_free_block, (int)new_block_order);

    return NULL;
}

