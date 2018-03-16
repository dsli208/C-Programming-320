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

extern void *sbrk_value; // MAKE GLOBAL

//////// BUD_MALLOC HELPER FUNCTIONS //////////////////////////////////////////

int isEmptyList(uint32_t i) {
    if (i >= 0 && i <= 9) {
        if (free_list_heads[i].next == free_list_heads[i].prev)
            return 1;
    }
    return 0;
}

uint32_t getOrder(uint32_t n) {
    uint32_t order = ORDER_MIN;
    while (n > 1) {
        n /= 2;
        order++;
    }
    return order;
}

void free_list_heads_insert(bud_free_block *block, int order) {
    int index = order - 5;

    bud_free_block cursor = free_list_heads[index];
    // Empty list condition
    if (isEmptyList(index)) {
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

/**
* Helper function for splitting a large memory block
*
* Returns start of block intended to be allocated, places others on the free_list_heads[]
**/
// *** NEED TO FIX THIS FUNCTION ***
void *split_block(uint32_t rsize, void *block) {
    // Casting to allow use of header and other parts of the free_block struct
    bud_free_block *cast_block  = (bud_free_block*)block;
    //char *char_block = (char*)block;
    uint64_t order = cast_block -> header.order;
    uint32_t size_block = ORDER_TO_BLOCK_SIZE(order); // YOU MAY NEED TO FIX THIS
    uint32_t new_size_block = size_block / 2;
    if (new_size_block < rsize)
        return block;

    // Reduce order
    cast_block -> header.order -= 1;

    // Should we split the block further?
    // This is the "rightmost" block, that should be put in the free list for later use
    // SEGFAULT ON THIS LINE, ARE YOU SPLITTING THE RIGHT WAY?
    bud_free_block *new_free_block = (bud_free_block*)(cast_block + (unsigned long)new_size_block);
    // Initialize the struct value
    bud_header new_block_header = {};
    new_free_block -> header = new_block_header;
    // First, set bits and block properties accordingly before adding to list
    new_block_header.allocated = 0; new_block_header.padded = 0;
    uint32_t new_block_order = getOrder(new_size_block); // or just use the decremented order
    new_block_header.order = (uint64_t)new_block_order;

    // Store to free list
    free_list_heads_insert(new_free_block, (int)new_block_order);

    return split_block(rsize, block);
}

void *get_free_block(uint32_t rsize, uint32_t order) {
    uint32_t index = order - 5;
    int block_split_required = 0;
    while (isEmptyList(index)) {
        block_split_required = 1;
        index++;
    }
    if (block_split_required) {
        bud_free_block free_block = *free_list_heads[index].next;
        free_list_heads[index].next = free_block.next;
        return split_block(rsize, &free_block);
    }
    else {
        bud_free_block *free_block = free_list_heads[index].next;
        free_list_heads[index].next = free_block -> next;
        return free_block;
    }
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
    int order = ORDER_MIN;
    uint32_t size_alloc = MIN_BLOCK_SIZE;
    while (size_alloc < rsize) {
        size_alloc *= 2;
        order++;
    }

    // Determine payload and header size
    uint32_t size_header = sizeof(bud_header); // Is this right?
    uint32_t size_payload = rsize; // Is this right?
    uint32_t size_padding; // IMPLEMENT?

    // Determine if we need to allocate more memory - edit IF condition
    if ((char*)bud_heap_end - (char*)bud_heap_start < size_alloc) {
        sbrk_value = bud_sbrk();
        sbrk_call = 1;
        //sbrk_value = (char*)sbrk_char_value;
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

    // Now, take the BIG block, and split it until we get a "almost perfect fit" --> START HERE NEXT TIME
    //bud_free_block *alloc_block = (bud_free_block*)(split_block(rsize, (bud_free_block*)sbrk_value)); // FIX
    bud_free_block *alloc_block = (bud_free_block*)(get_free_block(rsize, order));

    // After we get our allocated block, let's set the bits accordingly
    bud_header alloc_header = alloc_block -> header;
    (void)alloc_header;
    // Allocated bit
    alloc_header.allocated = 1;
    // Set the order bits
    alloc_header.order = order; // Correct way?

    // Padded bit - if padding is needed as described below
    if (size_alloc - rsize - sizeof(bud_header) > 0) {
        alloc_header.padded = 1;
    }

    // HOW DO WE SET THE UNUSED FIELDS?
    alloc_header.unused2 = rsize;
    alloc_header.rsize = rsize;

    return alloc_block + sizeof(bud_header);
}

void *bud_realloc(void *ptr, uint32_t rsize) {
    return NULL;
}

void bud_free(void *ptr) {
    // Base cases for bud_free
    bud_free_block *block_ptr = (bud_free_block*)ptr;
    int order = (int)block_ptr -> header.order;
    if (ptr < bud_heap_start() || ptr > bud_heap_end())
        abort();
    else if ((unsigned long)ptr % MIN_BLOCK_SIZE == 0) {
        abort();
    }
    // Free block base cases
    else if (order < ORDER_MIN || order >= ORDER_MAX) {
        abort();
    }
    else if (block_ptr->header.allocated == 0) {
        abort();
    }
    // FINISH BASE CASES
    return;
}