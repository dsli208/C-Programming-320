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

int hexvalidate(char *s) {
    char c = *s;
    while (c != '\0') {
        if ((c < 'A'|| c > 'F') && (c < 'a' || c > 'f') && (c < '0' || c > '9')) {
            return 0;
        }
        s++;
        c = *s;
    }
    return 1;
}

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

int slen(char* s) { // Useful to check length of BASEADDR and ENDIANESS
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
    // assume bin/hw1 is the first argument, so there is one more argument
    
    if (argc > 7) {
        return 0;
    }

    // Make sure we're incremented to the SECOND argument    
    argv++;    

    // Iterate through the char**
    // Look for -h
    if (strcompare(*argv, "-h")) {
        argv++;
        global_options += 1;
        return 1;
    }
    // Next argument should be either -a|-d
    int assembleDisassemble = 0; // 0 if -a, 1 if -d
    if (strcompare(*argv, "-a")) {
        argv++;
    }    
    else if (strcompare(*argv, "-d")) {
        argv++;
    }
    else {
        return 0; // error if anything else than -a|-d
    }

    // Now -b, or -e arguments, can be in any order
    int contains_b = 0;
    int contains_e = 0;
    int ebbit = 0;
    while (*argv != NULL) {
        if (strcompare(*argv, "-b") && contains_b == 0) {
            contains_b = 1;
            argv++;
            if (*argv == NULL || **argv == '-') {
                return 0;
            }
            if (slen(*argv) > 8) {
                return 0;
            }
            if (hexvalidate(*argv) == 0) {
                return 0;
            }
            // FIGURE OUT HOW TO CHECK THE LEAST 12 SIGNIFICANT BITS ARE ALL '0'
            
            argv++;
            
        }
        else if (strcompare(*argv, "-e") && contains_e == 0) {
            contains_e = 1;
            argv++;

            if (strcompare(*argv, "b")) {
                ebbit = 1;
            }
            else if (strcompare(*argv, "l")) {
            }
            //if (!(strcompare(*argv, "b") || strcompare(*argv, "l")))
            else {
                return 0;
            }

            argv++;
        }
        // CHECK TO SEE IF THIS IS RIGHT
        else {
            return 0;
        }
    }
    
    // Constructing global_options
    // Third least significant bit, -e b
    global_options = global_options << 29;
    if (contains_e && ebbit) {
        
        global_options += 1;
    }

    // Second least significant bit
    global_options = global_options << 1;
    if (assembleDisassemble) {
        global_options += 1;
    }

    global_options = global_options << 1;

    // should -h provision go here?

    // if -b is specified ...
    unsigned int base_addr = global_options;
    if (contains_b) {
        base_addr &= 0x000;
    }

    
    return 1; // MAKE SURE EVERYTHING IS RIGHT FIRST
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
    Instruction i = *ip;
    Instr_info i_info = *(i.info); // opcode, type, srcs[3], format
    if (i_info.type == NTYP) {
        return 0;
    }

    unsigned int val = 0;

    if (i_info.type == RTYP) {
        // Instruction format can be unsigned int, will be stored in 2's complement binary

        // Add opcode, then shift left by 5 bits
        val += i_info.opcode;
        val <<= 5;
        // Add rs, rt, and rd, then shift left by 5 bits each time

        // Find RS in srcs[3] array
        int n = 0;
        for (n = 0; n <= 3; n++) {
            if (i_info.srcs[n] == RS) {
                break;
            }
            // Error case?
            if (n == 3) {
                return 0;
            }
        }
                
        val += i.args[n];
        val <<= 5;

        // Find RT in srcs[3] array
        for (n = 0; n <= 3; n++) {
            if (i_info.srcs[n] == RT) {
                break;
            }
            if (n == 3) {
                return 0;
            }
        }

        val += i.args[n];
        val <<= 5;

        // Find RD in srcs[3] array
        for (n = 0; n <= 3; n++) {
            if (i_info.srcs[n] == RD) {
                break;
            }
            if (n == 3) {
                return 0;
            }
        }

        val += i.args[n];
        val <<= 5;

        // Add shamt, then shift by 6 
        // is the shamt instruction.extra or of type EXTRA in srcs?
        val += i.extra;

        // Add function
        // Get it from specialTable, since it's bits 5:0 --> should it be int or UNSIGNED int?
        int func = 0;
        for (int x = 0; x < 5; x++) {
            func += specialTable[x];
            if (x < 4)
                func <<= 1;
        }
        val += func;
        
        // Finally, set the value
        i.value = val;
    }
    else if (i_info.type == ITYP) {
        // Add opcode, shift left by 5 and add rs
        val += i_info.opcode;
        val <<= 5;

        // Find RS in srcs[3] array
        int n = 0;
        for (n = 0; n <= 3; n++) {
            if (i_info.srcs[n] == RS) {
                break;
            }
            // Error case?
            if (n == 3) {
                return 0;
            }
        }
        val <<= 5;

        // After adding rs, shift left by 5 and add rt
        // Find RT in srcs[3] array
        for (n = 0; n <= 3; n++) {
            if (i_info.srcs[n] == RT) {
                break;
            }
            if (n == 3) {
                return 0;
            }
        }
        val <<= 16;

        // Shift left 16 and add the immediate value
        val += i.extra;

        // Set val
        i.value = val;
    }
    else { // JTYP
        // add opcode, then shift by 26 and add address
        val += i_info.opcode;
        val <<= 26;
        val += addr;

        // Set val
        i.value = val;
    }
    return 1;
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
