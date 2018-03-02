/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "budmm.h"
#include "debug.h"

static char *bud_heap_area;     /* Malloc'ed block containing the entire heap */
static char *bud_heap_startp;   /* First byte of the heap */
static char *bud_heap_endp;     /* Last byte of the heap + 1 */
static char *bud_breakp;         /* Current heap break pointer */

void bud_mem_init() {
    // Initialize free lists as empty circular doubly linked lists.
    for(int i = 0; i < NUM_FREE_LIST; i++)
	free_list_heads[i].next = free_list_heads[i].prev = &free_list_heads[i];

    // Allocate a memory chunk large enough that we can make the entire
    // heap a single block aligned at the maximum heap size, regardless of
    // the address returned by malloc().  This will ensure the heap starts
    // on a left child for all possible sizes, making the behavior independent
    // of the address returned by malloc().
    bud_heap_area = malloc(2*MAX_HEAP_SIZE - 1);
    bud_heap_startp =
	(char *)((((uint64_t)bud_heap_area) + MAX_HEAP_SIZE - 1) & ~(MAX_HEAP_SIZE - 1));
    bud_heap_endp = bud_heap_startp;
    bud_breakp = bud_heap_startp;
    debug("heap start: %p, heap end: %p, max block size: 0x%x, max heap size: 0x%x",
	  bud_heap_startp, bud_heap_endp, MAX_BLOCK_SIZE, MAX_HEAP_SIZE);
}

void bud_mem_fini() {
    free(bud_heap_area);
}

void *bud_sbrk() {
    uint32_t incr = MAX_BLOCK_SIZE;
    char *old_brk = bud_breakp;
    char *new_brk = old_brk + incr;
    debug("bud_sbrk: %p -> %p", old_brk, new_brk);
    if(new_brk > bud_heap_startp + MAX_HEAP_SIZE) {
	errno = ENOMEM;
	return (void *)-1;
    }
    bud_breakp = new_brk;
    bud_heap_endp += incr;
    return old_brk;
}

void *bud_heap_start() {
    return bud_heap_startp;
}

void *bud_heap_end() {
    return bud_heap_endp;
}

