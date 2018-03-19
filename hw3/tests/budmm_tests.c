#include <criterion/criterion.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include "budmm.h"

#define UNALLOCATED 0
#define UNPADDED 0
#define ALLOCATED 1
#define PADDED 1

#define HEADER_TO_PAYLOAD(hdr) (((char *)hdr) + sizeof(bud_header))
#define PAYLOAD_TO_HEADER(ptr) (bud_header *)(((char *)ptr - sizeof(bud_header)))

static int free_list_is_empty(int order) {
    int i = order - ORDER_MIN;
    return(free_list_heads[i].next == &free_list_heads[i]);
}

static void assert_empty_free_list(int order) {
    cr_assert_neq(free_list_is_empty(order), 0,
		  "List [%d] contains an unexpected block!", order - ORDER_MIN);
}

static void assert_nonempty_free_list(int order) {
    cr_assert_eq(free_list_is_empty(order), 0,
		 "List [%d] should not be empty!", order - ORDER_MIN);
}

void assert_null_free_lists() {
    for (int order = ORDER_MIN; order < ORDER_MAX; order++)
	assert_empty_free_list(order);
}

void expect_errno_value(int exp_errno) {
    cr_assert(errno == exp_errno,
	      "`errno` was incorrectly set. Expected [%d] Actual: [%d]\n",
	      exp_errno, errno);
}

void assert_header_values(bud_header *bhdr, int exp_alloc, int exp_order,
			  int exp_pad, int exp_req_sz) {
    cr_assert(bhdr->allocated == exp_alloc,
	      "header `allocated` bits were not properly set. Expected: [%d] Actual: [%d]\n",
	      exp_alloc, bhdr->allocated);
    cr_assert(bhdr->order == exp_order,
	      "header `order` bits were not properly set. Expected: [%d] Actual: [%d]\n",
	      exp_order, bhdr->order);
    cr_assert(bhdr->padded == exp_pad,
	      "header `padded` bits were not properly set. Expected: [%d] Actual: [%d]\n",
	      exp_pad, bhdr->padded);
    cr_assert(bhdr->rsize == exp_req_sz,
	      "header `rsize` bits were not properly set. Expected: [%d] Actual: [%d]\n",
	      exp_req_sz, bhdr->rsize);
}

void assert_free_block_values(bud_free_block *fblk, int exp_order,
			      void *exp_prev_ptr, void *exp_next_ptr) {
    bud_header *bhdr = &fblk->header;

    cr_assert(bhdr->allocated == UNALLOCATED,
	      "header `allocated` bits were not properly set. Expected: [%d] Actual: [%d]\n",
	      UNALLOCATED, bhdr->allocated);
    cr_assert(bhdr->order == exp_order,
	      "header `order` bits were not properly set. Expected: [%d] Actual: [%d]\n",
	      exp_order, bhdr->order);
    cr_assert((void*)fblk->prev == exp_prev_ptr,
	      "`prev` pointer was not properly set. Expected: [%p] Actual: [%p]\n",
	      exp_prev_ptr, (void*)fblk->prev);
    cr_assert((void*)fblk->next == exp_next_ptr,
	      "`next` pointer was not properly set. Expected: [%p] Actual: [%p]\n",
	      exp_next_ptr, (void*)fblk->next);
}

Test(bud_malloc_suite, easy_malloc_a_pointer, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;
    int **x = bud_malloc(sizeof(int *));

    cr_assert_not_null(x, "bud_malloc returned null");

    int a = 4;
    *x = &a;

    cr_assert(*x == &a, "bud_malloc failed to give proper space for a pointer!");

    bud_header *bhdr = PAYLOAD_TO_HEADER(x);
    assert_header_values(bhdr, ALLOCATED, ORDER_MIN, PADDED, sizeof(int *));
    expect_errno_value(0);
}

Test(bud_malloc_suite, medium_malloc_diff_types, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;

    struct s1 {
        int a;
        float b;
        char *c;
    };

    struct s2 {
        int a[100];
        char *b;
    };

    uint32_t size = MIN_BLOCK_SIZE - sizeof(bud_header);
    char* carr = bud_malloc(size);
    cr_assert_not_null(carr, "bud_malloc returned null on the first call");
    for (int i = 0; i < size; i++) {
        carr[i] = 'a';
    }

    uint32_t sizeof_s1 = sizeof(struct s1);
    struct s1 *s_1 = bud_malloc(sizeof_s1);
    cr_assert_not_null(s_1, "bud_malloc returned null on the second call");
    s_1->a = 4;
    s_1->b = 2;

    uint32_t sizeof_s2 = sizeof(struct s2);
    struct s2 *s_2 = bud_malloc(sizeof_s2);
    cr_assert_not_null(s_2, "bud_malloc returned null on the third call");
    for (int i = 0; i < 100; i++) {
        s_2->a[i] = 5;
    }

    bud_header *carr_hdr = PAYLOAD_TO_HEADER(carr);
    bud_header *s1_hdr = PAYLOAD_TO_HEADER(s_1);
    bud_header *s2_hdr = PAYLOAD_TO_HEADER(s_2);

    assert_header_values(carr_hdr, ALLOCATED, ORDER_MIN, UNPADDED, size);
    for(int i = 0; i < size; i++) {
        cr_expect(carr[i] == 'a', "carr[%d] was changed!", i);
    }

    assert_header_values(s1_hdr, ALLOCATED, ORDER_MIN, PADDED, sizeof_s1);
    cr_expect(s_1->a == 4, "field `a` of struct s_1 was changed!");

    assert_header_values(s2_hdr, ALLOCATED, 9, PADDED, sizeof_s2);
    for (int i = 0; i < 100; i++) {
        cr_expect(s_2->a[0] == 5, "field `a` of struct s_2 was changed!");
    }

    expect_errno_value(0);
}

Test(bud_malloc_suite, malloc_max_heap, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;

    for(int n = 0; n < MAX_HEAP_SIZE / MAX_BLOCK_SIZE; n++) {
	char *x = bud_malloc(MAX_BLOCK_SIZE - sizeof(bud_header));
	for(int i = 0; i < MAX_BLOCK_SIZE - sizeof(bud_header); i++) {
	    x[i] = 'b';
	}

	cr_assert_not_null(x);

	bud_header *bhdr = PAYLOAD_TO_HEADER(x);
	assert_header_values(bhdr, ALLOCATED, ORDER_MAX-1, UNPADDED,
			     MAX_BLOCK_SIZE - sizeof(bud_header));
	assert_null_free_lists();
	expect_errno_value(0);
    }

    int *y = bud_malloc(sizeof(int));
    cr_assert_null(y);
    expect_errno_value(ENOMEM);
}

Test(bud_free_suite, free_no_coalesce, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;

    void *a = bud_malloc(4096 - sizeof(bud_header)); // -> 4096
    int *x = bud_malloc(sizeof(int)); // -> MIN_BLOCK_SIZE
    void *b = bud_malloc(sizeof(double)*2); // -> MIN_BLOCK_SIZE
    char *y = bud_malloc(sizeof(char)*100); // -> 128
    bud_header *bhdr_b = PAYLOAD_TO_HEADER(b);

    assert_header_values(bhdr_b, ALLOCATED, ORDER_MIN, PADDED, sizeof(double)*2);

    bud_free(x);

    bud_free_block *blk = free_list_heads[0].next; // only x is expected on the list
    assert_nonempty_free_list(ORDER_MIN);
    assert_free_block_values(blk, ORDER_MIN, &free_list_heads[0], &free_list_heads[0]);

    bud_free(y);

    blk = free_list_heads[7-ORDER_MIN].next;
    assert_nonempty_free_list(7);
    assert_free_block_values(blk, 7, &free_list_heads[7-ORDER_MIN],
			     &free_list_heads[7-ORDER_MIN]);

    cr_expect(bud_heap_start() + 1*MAX_BLOCK_SIZE == bud_heap_end(),
	      "Allocated more heap than necessary!");

    expect_errno_value(0);
}

Test(bud_free_suite, free_coalesce_higher_addr_check_ptrs, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;

                                             //  5   6   7   8   9  10  11  12  13  14
                                             //  0   0   0   0   0   0   0   0   0   0
    void *a = bud_malloc(sizeof(long));      //  1   1   1   1   1   1   1   1   1   0
    void *w = bud_malloc(sizeof(int) * 100); //  1   1   1   1   0   1   1   1   1   0
    void *x = bud_malloc(sizeof(char));      //  0   1   1   1   0   1   1   1   1   0
    void *b = bud_malloc(sizeof(int));       //  1   0   1   1   0   1   1   1   1   0
    void *y = bud_malloc(sizeof(int) * 100); //  1   0   1   1   1   0   1   1   1   0
    void *z = bud_malloc(sizeof(char));      //  0   0   1   1   1   0   1   1   1   0
    void *c = bud_malloc(sizeof(int));       //  1   1   0   1   1   0   1   1   1   0
    void *d = bud_malloc(sizeof(int));       //  0   1   0   1   1   0   1   1   1   0

    assert_empty_free_list(5);
    assert_nonempty_free_list(6);
    assert_empty_free_list(7);
    assert_nonempty_free_list(8);
    assert_nonempty_free_list(9);
    assert_empty_free_list(10);

    bud_free(c);                             //  1   1   0   1   1   0   1   1   1   0
    bud_free(z);                             //  2   1   0   1   1   0   1   1   1   0
    bud_free(y);                             //  2   1   0   1   0   1   1   1   1   0
    bud_free(a);                             //  3   1   0   1   0   1   1   1   1   0
    bud_free(b);                             //  2   2   0   1   0   1   1   1   1   0
    bud_free(x);                             //  1   1   1   1   0   1   1   1   1   0

    assert_nonempty_free_list(5);
    assert_nonempty_free_list(6);
    assert_nonempty_free_list(7);
    assert_nonempty_free_list(8);
    assert_empty_free_list(9);
    assert_nonempty_free_list(10);

    bud_header *y_hdr = PAYLOAD_TO_HEADER(y);
    cr_assert(((void*)free_list_heads[10-ORDER_MIN].next == (void*)y_hdr),
		      "The block in free list %d should be %p!",
	      10-ORDER_MIN, y_hdr);
    assert_free_block_values((bud_free_block*)y_hdr, 10,
			     &free_list_heads[10-ORDER_MIN],
			     &free_list_heads[10-ORDER_MIN]);

    bud_header *a_hdr = PAYLOAD_TO_HEADER(a);
    cr_assert(((void*)free_list_heads[7-ORDER_MIN].next == (void*)a_hdr),
		      "The block in free list %d should be %p!",
	      7-ORDER_MIN, a_hdr);
    assert_free_block_values((bud_free_block*)a_hdr, 7,
			     &free_list_heads[7-ORDER_MIN],
			     &free_list_heads[7-ORDER_MIN]);

    expect_errno_value(0);
}

Test(bud_realloc_suite, realloc_diff_hdr, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5, .signal = SIGABRT) {
    errno = 0;
    int *x = bud_malloc(sizeof(int));

    bud_header *bhdr = PAYLOAD_TO_HEADER(x);

    bhdr->order = ORDER_MIN + 1;

    void *y = bud_realloc(x, 200);
    (void)y;
}

Test(bud_realloc_suite, realloc_size_zero_free, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;

    void *x = bud_malloc(sizeof(int));
    bud_malloc(sizeof(int));

    void *y = bud_realloc(x, 0); // should just free x

    cr_assert_null(y);

    assert_nonempty_free_list(ORDER_MIN);
    assert_free_block_values(free_list_heads[0].next, ORDER_MIN,
			     &free_list_heads[0], &free_list_heads[0]);

    expect_errno_value(0);
}

Test(bud_realloc_suite, realloc_larger_block, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;

    void *original = bud_malloc(sizeof(int));
    bud_malloc(500);
    int* new = bud_realloc(original, sizeof(int) * 100); // 400 -> 512
    // original will do a few steps of coalesce, resulting in 512

    bud_header *bhdr_new = PAYLOAD_TO_HEADER(new);
    assert_header_values(bhdr_new, ALLOCATED, 9, PADDED, sizeof(int) * 100);

    cr_assert_not_null(new, "bud_realloc returned NULL");
    assert_nonempty_free_list(9);
    cr_assert_neq(free_list_heads[9-ORDER_MIN].next->next,
		  &free_list_heads[9-ORDER_MIN],
		  "A second block is expected in free list #%d!",
		  9-ORDER_MIN);

    expect_errno_value(0);
}

Test(bud_realloc_suite, realloc_smaller_block_free_block, .init = bud_mem_init, .fini = bud_mem_fini,
     .timeout = 5) {
    errno = 0;

    void *x = bud_malloc(sizeof(double) * 4); // 32 -> 64
    void *y = bud_realloc(x, sizeof(int));

    cr_assert_not_null(y, "bud_realloc returned NULL!");

    bud_header *bhdr_y = PAYLOAD_TO_HEADER(y);
    assert_header_values(bhdr_y, ALLOCATED, ORDER_MIN, PADDED, sizeof(int));

    assert_nonempty_free_list(ORDER_MIN);
    cr_assert(((char *)(free_list_heads[0].next) == (char *)(bhdr_y) + 32),
	      "The split block of bud_realloc is not at the right place!");

    expect_errno_value(0);
}

//############################################
//STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//############################################

