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
extern uint64_t heap_mem_used;

//////////////////// HEAP FUNCTIONS //////////////////////////////////////
void heap_counter_init() {
    heap_mem_used = 0;
}

void increment_heap_counter(uint32_t size_alloc) {
    heap_mem_used += size_alloc;
}

void *get_current_heap_address() {
    return bud_heap_start() + heap_mem_used;
}

//////// BUD_MALLOC HELPER FUNCTIONS //////////////////////////////////////////

int isEmptyList(uint32_t i) {
    if (i >= 0 && i < ORDER_MAX - ORDER_MIN) {
        if (free_list_heads[i].next == free_list_heads[i].prev) {
            if (free_list_heads[i].next != &free_list_heads[i])
                return 0;
            else
                return 1;
        }
    }
    return 0;
}

// Determine if we need to sbrk again
int sbrk_again() {
    int i = 0;
    while (i < ORDER_MAX - ORDER_MIN) {
        if (!isEmptyList(i))
            return 0;
        i++;
    }
    return 1;
}

uint32_t getOrder(uint32_t n) {
    uint32_t order = ORDER_MIN;
    while (n > 32) {
        n /= 2;
        order++;
    }
    return order;
}

void free_list_heads_insert(bud_header *header, int order) {
    int index = order - ORDER_MIN;

    //bud_free_block block = {*header, NULL, NULL};

    bud_free_block cursor = free_list_heads[index];
    // Empty list condition
    if (isEmptyList(index)) {
        // Set the sentinel pointers to point to the new node
        free_list_heads[index].next = (bud_free_block*)header;
        free_list_heads[index].prev = (bud_free_block*)header;

        // and set the new node's pointers to point to the sentinel
        free_list_heads[index].next -> next = &free_list_heads[index];
        free_list_heads[index].prev -> prev = &free_list_heads[index];
        return;
    }

    while (cursor.next != &free_list_heads[index]) {
        cursor = *(cursor.next);
    }
    // Pointers to new node
    cursor.next = (bud_free_block*)header;
    cursor.next -> prev = &cursor;

    // Modify pointers to sentinel
    cursor.next -> next = &free_list_heads[index];
    free_list_heads[index].prev = (bud_free_block*)header;

}

/**
* Helper function for splitting a large memory block
*
* Returns start of block intended to be allocated, places others on the free_list_heads[]
**/
// *** NEED TO FIX THIS FUNCTION ***
void *split_block(uint32_t rsize, void *header) {
    bud_header* bud_split_header = (bud_header*)header;
    //char *char_block = (char*)block;
    //uint64_t order = cast_block -> header.order;
    uint64_t order = bud_split_header -> order;
    uint32_t size_block = ORDER_TO_BLOCK_SIZE(order); // YOU MAY NEED TO FIX THIS
    intptr_t new_size_block = ORDER_TO_BLOCK_SIZE(order - 1);
    if (new_size_block < rsize || new_size_block < 32)
        return bud_split_header;

    // Reduce order
    bud_split_header -> order -= 1;

    // Should we split the block further?
    // This is the "rightmost" block, that should be put in the free list for later use
    // SEGFAULT ON THIS LINE, ARE YOU SPLITTING THE RIGHT WAY?
    char *new_header_address = (header + new_size_block);
    //bud_free_block new_free_block = {};
    // Initialize the struct value
    bud_header *new_block_header = (bud_header*)new_header_address;
    //new_free_block.header = new_block_header;
    // First, set bits and block properties accordingly before adding to list
    new_block_header->allocated = 0; new_block_header->padded = 0;
    uint32_t new_block_order = bud_split_header -> order; // or just use the decremented order
    new_block_header->order = (uint64_t)new_block_order;

    //new_header_address = new_block_header;

    // Store to free list
    free_list_heads_insert(new_block_header, (int)new_block_order);

    return split_block(rsize, header);

}

// Function for getting the first free block possible
void *get_free_block(uint32_t rsize, uint32_t order) {
    uint32_t index = order - ORDER_MIN;
    int block_split_required = 0;
    while (isEmptyList(index)) {
        block_split_required = 1;
        index++;
    }
    if (index >= ORDER_MAX - ORDER_MIN) {
        errno = ENOMEM;
        return NULL;
    }
    if (block_split_required) {
        bud_header *free_header = (bud_header*)free_list_heads[index].next;
        free_list_heads[index].next = free_list_heads[index].next -> next;
        free_list_heads[index].next -> prev = &free_list_heads[index]; // correct?
        return split_block(rsize, free_header);
    }
    else {
        bud_header *free_header = (bud_header*)free_list_heads[index].next;
        free_list_heads[index].next = free_list_heads[index].next -> next;
        free_list_heads[index].next -> prev = &free_list_heads[index];
        return free_header;
    }
}

void *bud_max_malloc(uint32_t rsize, void *ptr) {
    bud_header *bud_max_malloc_header = (bud_header*)ptr;
    bud_max_malloc_header -> allocated = 1;
    bud_max_malloc_header -> padded = 0;
    bud_max_malloc_header -> order = ORDER_MAX - 1;
    bud_max_malloc_header -> rsize = (uint64_t)rsize;

    increment_heap_counter(rsize);

    return ((char*)bud_max_malloc_header) + sizeof(bud_header);
}

//////// BUD_FREE HELPER FUNCTIONS //////////////////////////////////////////
// coalesce function --> FIRST BLOCK IN MEMORY goes FIRST, rightward buddy is SECOND
// blocks must be deallocated first and all padding removed
void coalesce(void *block1, void *block2) {
    bud_header *block1_header = (bud_header*)block1; bud_header *block2_header = (bud_header*)block2;

    // BASE CASES
    if (block2 > bud_heap_end() || block1 < bud_heap_start()) {
        free_list_heads_insert(block1, block1_header -> order);
        return;
    }
    else if (block2 != block1 + ORDER_TO_BLOCK_SIZE(block1_header -> order)) { // should this addition be in terms of char*?
        free_list_heads_insert(block1, block1_header -> order);
        return;
    }
    else if (block1_header -> allocated || block2_header -> allocated) {
        free_list_heads_insert(block1, block1_header -> order);
        return;
    }
    else if (block1_header -> padded || block2_header -> padded) {
        free_list_heads_insert(block1, block1_header -> order);
        return;
    }

    block1_header -> order += 1;
    // How to figure out that a previously existing block was combined with another one?

    bud_header *new_buddy_header = (bud_header*)(block1 + ORDER_TO_BLOCK_SIZE(block1_header -> order));

    // What about the free lists?  How would this function handle it?
    if (block1_header -> order == new_buddy_header -> order)
        coalesce(block1, new_buddy_header);
    else
        free_list_heads_insert(block1, block1_header -> order);
}

// Get free block function to be used for coalescing in bud_free and potentially bud_realloc
void *get_free_block_by_ptr(void *ptr, uint64_t order) {
    int index = (int)order - ORDER_MIN;

    bud_free_block *head = &free_list_heads[index];

    // Search the given linked list for your ptr, or return NULL
    bud_free_block *cursor = head -> next;
    while (cursor != ptr && cursor != head) {
        cursor = cursor -> next;
    }
    if (cursor == head) {
        return NULL;
    }
    else {
        // Check this yourself later to see if this makes sense!!!
        cursor -> prev -> next = cursor -> next;
        cursor -> next -> prev = cursor -> prev;
        return cursor;
    }
}

uint32_t block_size(uint64_t order) {
    uint32_t size = 1;
    for (int i = 0; i < (int)order; i++) {
        size *= 2;
    }
    return size;
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
    int max_malloc = 0;

    // base - error cases
    if (rsize == 0 || rsize > MAX_BLOCK_SIZE - sizeof(bud_header)) {
        errno = EINVAL;
        return NULL;
    }
    else if (rsize == MAX_BLOCK_SIZE - sizeof(bud_header))
        max_malloc = 1;

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
    if (sbrk_value == (void*)0 || sbrk_again()) {
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
        void *bud_header_addr = sbrk_value;

        // CHECK TO SEE THAT THESE ARE THE VALUES YOU ARE SUPPOSED TO SET
        newAllocHeader.allocated = 0;
        newAllocHeader.order = 14;
        newAllocHeader.padded = 0;

        *(bud_header*)bud_header_addr = newAllocHeader;

        // max_malloc case
        if (max_malloc) {
            return bud_max_malloc(rsize, bud_header_addr);
        }

        free_list_heads_insert(bud_header_addr, (int)newAllocHeader.order);
    }

    // Now, take the BIG block, and split it until we get a "almost perfect fit" --> START HERE NEXT TIME
    //bud_free_block *alloc_block = (bud_free_block*)(split_block(rsize, (bud_free_block*)sbrk_value)); // FIX
    bud_header *alloc_header = (bud_header*)(get_free_block(rsize, order));

    if (alloc_header == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    // After we get our allocated block, let's set the bits accordingly
    //bud_header alloc_header = alloc_block -> header;
    (void)alloc_header;
    // Allocated bit
    alloc_header->allocated = 1;
    // Set the order bits
    alloc_header->order = order; // Correct way?

    // Padded bit - if padding is needed as described below
    if (size_alloc - rsize - sizeof(bud_header) > 0) {
        alloc_header->padded = 1;
    }

    // HOW DO WE SET THE UNUSED FIELDS?
    alloc_header->unused2 = rsize;
    alloc_header->rsize = rsize;

    increment_heap_counter(size_alloc);
    //return alloc_header;
    return (char*)((char*)alloc_header + sizeof(bud_header));
}

void *bud_realloc(void *ptr, uint32_t rsize) {
    bud_header *header_ptr = ptr - sizeof(bud_header);
    if (rsize == 0) { // Realloc to 0
        bud_free(ptr);
        return NULL;
    }
    else if (header_ptr == NULL || ptr == NULL) { // && rsize > 0, NULL ptr so we need to bud_malloc
        return bud_malloc(rsize);
    }
    else if (header_ptr -> rsize > rsize) {  // less size, return same pointer, but allocate a smaller block, splitting this one if needed
        // WORK ON THIS !!!!
        header_ptr = split_block(rsize, header_ptr);
        header_ptr -> rsize = rsize;
        return ptr;
    }
    else if (rsize > header_ptr -> rsize) { // more size, free and allocate a new, larger, block
        void *new_block_payload = bud_malloc(rsize);
        bud_free(ptr);
        return new_block_payload;
    }
    else { // rsize stays the same, so return the ptr
        return ptr;
    }
}

void bud_free(void *ptr) {
    // Base cases for bud_free
    bud_free_block *block_ptr = (bud_free_block*)(ptr - sizeof(bud_header));
    int order = (int)(block_ptr -> header.order);
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
    else if (block_ptr -> header.padded == 0 && block_ptr -> header.rsize + sizeof(bud_header) != block_size(block_ptr -> header.order))
        abort();
    else if (block_ptr -> header.padded == 1 && block_ptr -> header.rsize + sizeof(bud_header) == block_size(block_ptr -> header.order))
        abort();
    // FINISH LAST BASE CASE.  ALSO CHECK THE LAST TWO BASE CASES TO VERIFY THEY ARE CORRECT.

    // Header - set allocated, rsize, etc. bits to zero
    bud_header *header_ptr =(bud_header*)(ptr - sizeof(bud_header));
    header_ptr -> allocated = 0;
    header_ptr -> rsize = 0;
    header_ptr -> padded = 0;

    // Header - get the order and figure out if we can coalesce with our "buddy block" --> is it to the left or to the right?
    int64_t header_order = header_ptr -> order;
    // Get the next block
    bud_header *next_block_header = (bud_header*)block_ptr + ORDER_TO_BLOCK_SIZE(header_order); // char*
    bud_header *prev_block_header = (bud_header*)block_ptr - ORDER_TO_BLOCK_SIZE(header_order);
    int64_t next_block_order = next_block_header -> order;
    int64_t prev_block_order = prev_block_header -> order;
    if (next_block_order == header_order) { // Alternative: ORDER_TO_BLOCK_SIZE calls also return equivalent result, coalesce if this is true
        // Coalescing the blocks

        // Retrieve the block to coalesce with this one from the appropriate free list
        bud_header *block_to_coalesce = (bud_header*)get_free_block_by_ptr(next_block_header, header_order);

        // HOW TO COALESCE?
        coalesce(block_ptr, next_block_header);

    }
    else if (prev_block_order == header_order) {
        // Retrieve the block to coalesce with this one from the appropriate free list
        bud_header *block_to_coalesce = (bud_header*)get_free_block_by_ptr(prev_block_header, header_order);

        coalesce(prev_block_header, block_ptr);
    }
    else { // No coalescing, just free
        free_list_heads_insert(header_ptr, header_order);

    }

    return;
}