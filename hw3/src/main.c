#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "budmm.h"

// INSERTING TEST MAIN HERE
#define HEADER_TO_PAYLOAD(hdr) (((char *)hdr) + sizeof(bud_header))
#define PAYLOAD_TO_HEADER(ptr) (bud_header *)(((char *)ptr - sizeof(bud_header)))

int main() {
    bud_mem_init();
    heap_counter_init();

    // REALLOC LARGER TEST CASE
    /*void *original = bud_malloc(sizeof(int));
    void *second = bud_malloc(500);
    int* new = bud_realloc(original, sizeof(int) * 100); // 400 -> 512*/
    // original will do a few steps of coalesce, resulting in 512

    //(void)new;

    // DIFF HDR CASE

    int *x = bud_malloc(sizeof(int));
    bud_header *bhdr = PAYLOAD_TO_HEADER(x);

    bhdr->order = ORDER_MIN + 1;

    void *y = bud_realloc(x, 200);
    (void)y;

    bud_mem_fini();
}

// UNCOMMENT WHAT IS ABOVE THIS AND BELOW THE ABOVE COMMENT BEFORE YOU SUBMIT IT FOR GRADING!!!!!


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