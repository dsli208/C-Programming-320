#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "budmm.h"

// INSERTING TEST MAIN HERE

/*int main() {
    bud_mem_init();
    heap_counter_init();

    void *original = bud_malloc(sizeof(int));
    bud_malloc(500);
    int* new = bud_realloc(original, sizeof(int) * 100); // 400 -> 512
    // original will do a few steps of coalesce, resulting in 512

    bud_mem_fini();
}*/

// UNCOMMENT WHAT IS ABOVE THIS AND BELOW THE ABOVE COMMENT BEFORE YOU SUBMIT IT FOR GRADING!!!!!


int main(int argc, char const *argv[]) {
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
}