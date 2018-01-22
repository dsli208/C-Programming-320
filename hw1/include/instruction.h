/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

/*
 * Opcode values.
 * The MIPS opcode field consists of bits 31-26 (6 bits) of the instruction word.
 * For most instructions, the value in this table is the actual 6-bit value that
 * is placed in the opcode field.
 * Exceptions are the following:
 *   OP_n, where n is a number, are unused opcodes that never occur in actual code.
 *   SPECIAL and BCOND are values used for irregular instructions that must be
 *     identified using information outside of the normal opcode field.
 *   ILLEGL is a sentinel value that is used in the other tables below when there is
 *     no meaningful opcode value to use.
 */
typedef enum opcode {
    OP_0,     OP_ADD,    OP_ADDI, OP_ADDIU, OP_ADDU, OP_AND,     OP_ANDI,  OP_BEQ,
    OP_BGEZ,  OP_BGEZAL, OP_BGTZ, OP_BLEZ,  OP_BLTZ, OP_BLTZAL,  OP_BNE,   OP_15,
    OP_DIV,   OP_DIVU,   OP_J,    OP_JAL,   OP_JALR, OP_JR,      OP_LB,    OP_LBU,
    OP_LH,    OP_LHU,    OP_LUI,  OP_LW,    OP_LWL,  OP_LWR,     OP_30,    OP_MFHI,
    OP_MFLO,  OP_33,     OP_MTHI, OP_MTLO,  OP_MULT, OP_MULTU,   OP_NOR,   OP_OR,
    OP_ORI,   OP_RFE,    OP_SB,   OP_SH,    OP_SLL,  OP_SLLV,    OP_SLT,   OP_SLTI,
    OP_SLTIU, OP_SLTU,   OP_SRA,  OP_SRAV,  OP_SRL,  OP_SRLV,    OP_SUB,   OP_SUBU,
    OP_SW,    OP_SWL,    OP_SWR,  OP_XOR,   OP_XORI, OP_SYSCALL, OP_BREAK, OP_UNIMP,
    SPECIAL,  BCOND,     ILLEGL
} Opcode;

/*
 * Instruction types: NTYP (not a type), ITYP (immediate operand type),
 * JTYP (jump type), and RTYP (register type).
 */
typedef enum type { NTYP, ITYP, JTYP, RTYP } Type;

/*
 * Argument sources: NSRC (not a source), RS (from RS field), RT (from RT field),
 * RD (from RD field), EXTRA (from various other parts of the instruction word,
 * depending on the type of the instruction).
 */
typedef enum source { NSRC, RS, RT, RD, EXTRA } Source;

/*
 * Structure that records essential information about an instruction type.
 */
typedef struct instr_info
{
    Opcode opcode;     /* Opcode value for the instruction. */
    Type type;         /* Instruction type. */
    Source srcs[3];    /* Sources for the instruction arguments. */
    char *format;      /* Template for textual representation. */
} Instr_info;

/*
 * Structure to hold decoded information about a single instruction.
 */
typedef struct instruction
{
    int value;         /* The binary instruction word. */
    Instr_info *info;  /* Info about the instruction type. */
    char regs[3];      /* Values of the register fields of the instruction word. */
    int extra;         /* Immediate or other non-register argument decoded from
			  the instruction word. */
    int args[3];       /* Instruction arguments as in assembly code. */
} Instruction;

/*
 * Table mapping bits 31:26 the instruction word to the corresponding Opcode value.
 */
extern Opcode opcodeTable[];

/*
 * Table mapping bits 5:0 of SPECIAL instructions to the corresponding Opcode value.
 */
extern Opcode specialTable[];

/*
 * Table mapping each Opcode value to a structure that gives the essential information
 * about that type of instruction.
 */
extern Instr_info instrTable[];

#endif
