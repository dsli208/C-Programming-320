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
    Instr_info inin;
    Instruction i = {0x7, &inin, {}, 0x0, {}};


    if(!validargs(argc, argv)) {
        //printf("%d\n", global_options);
        USAGE(*argv, EXIT_FAILURE);
    }
    debug("Options: 0x%X", global_options);

    // Structure of how the main function works?

    if(global_options & 0x1) { // -h
        USAGE(*argv, EXIT_SUCCESS);
    }

    if ((global_options & 0x0) == 0) { // -a
        //unsigned int value = 0x10aeff;
        //int encodeVal = encode(&i, value);
        char instr[120];//NULL;
        char instr_type[10];
        char c;

        int rs = 0, rt = 0, rd = 0;
        int *rsp = &rs, *rtp = &rt, *rdp = &rd;

        while ((c = getc(stdin)) != EOF)  {
        //while (fgets(instr, 120, stdin) != NULL) {
            //fgets(instr, 120, stdin); // size == 120?
            sscanf(instr, "%s $%d,$%d,$%d", instr_type, rsp, rtp, rdp);
            encode(&i, base_addr);
            //instr.value

            base_addr += 4;
        }

        int valShift = 0;
        int val = i.value;
        while (valShift < 32) {
            // val = 0x00000000 format
            int lowestTwoValBits = val & 0xFF;
            putchar(lowestTwoValBits);
            valShift += 8;
            val >>= 8;
        }
    }
    else if (global_options & 0x2) { // -d
        //unsigned int value = 0x10aeff;
        //int decodeVal = decode(&i, value);
        //printf("%d\n", decodeVal);
        int valShift = 0;
        int val = 0;
        char c;
        while ((c = getchar()) != EOF && valShift < 32) {
            val |= c;
            valShift += 8;
            val <<= 8;
        }
        i.value = val;
        decode(&i, base_addr);
    }



    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */

// BE --> LE and vice-versa