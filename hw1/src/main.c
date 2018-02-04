#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

#include "hw1.h" // includes global_options
#include "debug.h"

int main(int argc, char **argv)
{   
    if(!validargs(argc, argv)) {
        //printf("%d\n", global_options);
        USAGE(*argv, EXIT_FAILURE);
    }
    debug("Options: 0x%X", global_options);

    // Structure of how the main function works?
    // 
    if(global_options & 0x1) { // -h
        USAGE(*argv, EXIT_SUCCESS);
    }
    if (global_options | 0x10) { // -d
         
    }
    else if (global_options & 0x00) { // -a
    }
    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
