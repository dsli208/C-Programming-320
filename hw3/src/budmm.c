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
    uint32_t size_payload = size_alloc - (rsize - size_header);
    uint32_t size_padding; // IMPLEMENT?

    // Determine if we need to allocate more memory
    if ((char*)bud_heap_end - (char*)bud_heap_start < size_alloc) {

    }

    // Case of failure - request CANNOT be satisfied
    void *sbrk_char_value = bud_sbrk();
    if (sbrk_value == (void*)-1) {
        errno = ENOMEM;
        return NULL;
    }
    // Case of success - request CAN be satisfied
    char *sbrk_value = (char*)sbrk_char_value;

    return NULL;
}

void *bud_realloc(void *ptr, uint32_t rsize) {
    return NULL;
}

void bud_free(void *ptr) {
    return;
}
