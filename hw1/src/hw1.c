#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 */

int strcompare(char *a, char *b) { // checking for string/argument equality
    char c1 = *a;
    char c2 = *b;
    while (c1 != '\0' && c2 != '\0') {
        if (c1 != c2) {
            return 0;
        }
        a++; b++; // increment pointers
        c1 = *a; c2 = *b; // update c1 and c2
    }
    return 1;
}

int strlen(char* s) { // Useful to check length of BASEADDR and ENDIANESS
    int len = 0;
    char c = *s;
    while (c != '\0') {
        len++;
        s++;
        c = *s;
    }
    return len;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    int contains_h = 0;
    int contains_a = 0; // if -a, 1, else 0    
    if (argc > 6) {
        return 0;
    }
    // Iterate through the char**
    // Look for -h
    if (strcompare(*argv, "-h")) {
        contains_h = 1;
        argv++;
    }
    // Next argument should be either -a|-d
    if (strcompare(*argv, "-a")) {
        contains_a = 1;
        argv++;
    }    
    else if (strcompare(*argv, "-d")) {
        contains_a = 0;
        argv++;
    }
    else {
        return 0; // error if anything else than -a|-d
    }

    // Now -b, or -e arguments, can be in any order
    while (argv != NULL) {
        
    }
    return 0;
}

/**
 * @brief Computes the binary code for a MIPS machine instruction.
 * @details This function takes a pointer to an Instruction structure
 * that contains information defining a MIPS machine instruction and
 * computes the binary code for that instruction.  The code is returne
 * in the "value" field of the Instruction structure.
 *
 * @param ip The Instruction structure containing information about the
 * instruction, except for the "value" field.
 * @param addr Address at which the instruction is to appear in memory.
 * The address is used to compute the PC-relative offsets used in branch
 * instructions.
 * @return 1 if the instruction was successfully encoded, 0 otherwise.
 * @modifies the "value" field of the Instruction structure to contain the
 * binary code for the instruction.
 */
int encode(Instruction *ip, unsigned int addr) {
    return 0;
}

/**
 * @brief Decodes the binary code for a MIPS machine instruction.
 * @details This function takes a pointer to an Instruction structure
 * whose "value" field has been initialized to the binary code for
 * MIPS machine instruction and it decodes the instruction to obtain
 * details about the type of instruction and its arguments.
 * The decoded information is returned by setting the other fields
 * of the Instruction structure.
 *
 * @param ip The Instruction structure containing the binary code for
 * a MIPS instruction in its "value" field.
 * @param addr Address at which the instruction appears in memory.
 * The address is used to compute absolute branch addresses from the
 * the PC-relative offsets that occur in the instruction.
 * @return 1 if the instruction was successfully decoded, 0 otherwise.
 * @modifies the fields other than the "value" field to contain the
 * decoded information about the instruction.
 */
int decode(Instruction *ip, unsigned int addr) {
    return 0;
}
