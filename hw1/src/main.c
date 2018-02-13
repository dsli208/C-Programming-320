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

#include "hw1.h" // includes global_options and other variables
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

    else if (global_options & 0x2) { // -d
        //unsigned int value = 0x10aeff;
        //int decodeVal = decode(&i, value);
        //printf("%d\n", decodeVal);
        int shift = 0;
        int shifted = 0;
        unsigned int val = 0;
        char c;
        while ((c = fgetc(stdin)) != EOF) {

            if (shifted % 4 == 0 && shift) {
                i.value = val;
                decode(&i, base_addr);
                val = 0;
                shift = 0;
                continue;
            }

            int charValue = 48;
            switch (c) {
                case 'a':
                case 'A': charValue = 10; break;
                case 'b':
                case 'B': charValue = 11; break;
                case 'c':
                case 'C': charValue = 12; break;
                case 'd':
                case 'D': charValue = 13; break;
                case 'e':
                case 'E': charValue = 14; break;
                case 'f':
                case 'F': charValue = 15; break;
                case '\0': continue; break;
                default: charValue = (c - 48);
            }

            val |= charValue;
            val <<= 7;

            shifted++;
            shift = 1;
        }

    }

    else if ((global_options & 0x0) == 0) { // -a
        //unsigned int value = 0x10aeff;
        //int encodeVal = encode(&i, value);
        char instr[120];//NULL;
        char instr_type[10];
        char c;

        int rs = 0, rt = 0, rd = 0;
        int *rsp = &rs, *rtp = &rt, *rdp = &rd;

        //printf("Starts encode");

        char *instructionFormat;
        //while ((c = getc(stdin)) != EOF)  {
        while ((instructionFormat = fgets(instr, 120, stdin)) != NULL) {
            //fgets(instr, 120, stdin); // size == 120?
            //sscanf(instr, "%s $%d,$%d,$%d", instr_type, rsp, rtp, rdp);
            for (int n = 0; n < 63; n++) {
                Instr_info tableInfo = instrTable[n];
                if (sscanf(instructionFormat, tableInfo.format, instr_type, rsp, rtp, rdp)) {
                    inin.format = instructionFormat;
                    break;
                }
            }
            if (!encode(&i, base_addr)) {
                return EXIT_FAILURE;
            }
            //instr.value

            int valShift = 0;
            unsigned int shifter = 0xFF;
            int val = i.value;
            while (valShift < 32) {
                // val = 0x00000000 format
                int lowestTwoValBits = val & shifter;
                lowestTwoValBits >>= valShift;
                putchar(lowestTwoValBits);
                valShift += 8;
                shifter <<= 8;
                val >>= 8;
            }

            base_addr += 4;
        }

        //printf("Finished encode");

    }




    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */

// BE --> LE and vice-versa