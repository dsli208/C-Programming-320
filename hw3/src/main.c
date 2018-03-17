#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "budmm.h"

// INSERTING TEST MAIN HERE

int main() {
    bud_mem_init();
    heap_counter_init();
    errno = 0;
    int **x = bud_malloc(sizeof(int *));
    printf("%d", x == NULL);
    //cr_assert_not_null(x, "bud_malloc returned null");

    int a = 4;
    *x = &a;

    bud_mem_fini();
    //cr_assert(*x == &a, "bud_malloc failed to give proper space for a pointer!");

    //bud_header *bhdr = PAYLOAD_TO_HEADER(x);
    //(void)bhdr;
    //assert_header_values(bhdr, ALLOCATED, ORDER_MIN, PADDED, sizeof(int *));
    //expect_errno_value(0);
}


/*int main(int argc, char const *argv[]) {
    int i;

    bud_mem_init();
    heap_counter_init();
    char* ptr1 = (char*)bud_malloc(10); // 64

    ptr1[0] = 'a';
    ptr1[1] = 'b';
    ptr1[2] = 'c';
    ptr1[3] = 'd';
    ptr1[4] = 'e';
    ptr1[5] = 'f';
    ptr1[6] = '\0';
    printf("ptr1 string: %s\n", ptr1);

    int* ptr2 = bud_malloc(sizeof(int) * 100); // 512
    for(i = 0; i < 100; i++)
        ptr2[i] = i;

    void* ptr3 = bud_malloc(3000); // 4192
    printf("ptr3: %p\n", ptr3);

    bud_free(ptr1);

    ptr2 = bud_realloc(ptr2, 124); // 128

    ptr1 = bud_malloc(200); // 256
    ptr1 = bud_realloc(ptr1, 100); // 128

    // intentional error (errno = EINVAL)
    ptr3 = bud_malloc(20000);
    printf("errno: %d (%s)\n", errno, strerror(errno));

    bud_mem_fini();

    return EXIT_SUCCESS;
}*/
