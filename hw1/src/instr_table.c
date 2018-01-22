/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#include "instruction.h"

/*
 * The table below is used to map bits 31:26 of the instruction word
 * into either an "opcode" value that uniquely identifies the instruction,
 * or else the value SPECIAL or BCOND, indicating that further decoding
 * is required using additional information from the instruction word.
 */
Opcode opcodeTable[] = {
    SPECIAL,  BCOND,    OP_J,     OP_JAL,   OP_BEQ,  OP_BNE, OP_BLEZ, OP_BGTZ,
    OP_ADDI,  OP_ADDIU, OP_SLTI,  OP_SLTIU, OP_ANDI, OP_ORI, OP_XORI, OP_LUI,
    OP_UNIMP, OP_UNIMP, OP_UNIMP, OP_UNIMP, ILLEGL,  ILLEGL, ILLEGL,  ILLEGL,
    ILLEGL,   ILLEGL,   ILLEGL,   ILLEGL,   ILLEGL,  ILLEGL, ILLEGL,  ILLEGL,
    OP_LB,    OP_LH,    OP_LWL,   OP_LW,    OP_LBU,  OP_LHU, OP_LWR,  ILLEGL,
    OP_SB,    OP_SH,    OP_SWL,   OP_SW,    ILLEGL,  ILLEGL, OP_SWR,  ILLEGL,
    OP_UNIMP, OP_UNIMP, OP_UNIMP, OP_UNIMP, ILLEGL,  ILLEGL, ILLEGL,  ILLEGL,
    OP_UNIMP, OP_UNIMP, OP_UNIMP, OP_UNIMP, ILLEGL,  ILLEGL, ILLEGL,  ILLEGL
};

/*
 * The table below is used to map bits 5:0 of SPECIAL instructions into
 * the corresponding "opcode" value.
 */
Opcode specialTable[] = {
    OP_SLL,  ILLEGL,   OP_SRL,  OP_SRA,  OP_SLLV,    ILLEGL,   OP_SRLV, OP_SRAV,
    OP_JR,   OP_JALR,  ILLEGL,  ILLEGL,  OP_SYSCALL, OP_BREAK, ILLEGL,  ILLEGL,
    OP_MFHI, OP_MTHI,  OP_MFLO, OP_MTLO, ILLEGL,     ILLEGL,   ILLEGL,  ILLEGL,
    OP_MULT, OP_MULTU, OP_DIV,  OP_DIVU, ILLEGL,     ILLEGL,   ILLEGL,  ILLEGL,
    OP_ADD,  OP_ADDU,  OP_SUB,  OP_SUBU, OP_AND,     OP_OR,    OP_XOR,  OP_NOR,
    ILLEGL,  ILLEGL,   OP_SLT,  OP_SLTU, ILLEGL,     ILLEGL,   ILLEGL,  ILLEGL,
    ILLEGL,  ILLEGL,   ILLEGL,  ILLEGL,  ILLEGL,     ILLEGL,   ILLEGL,  ILLEGL,
    ILLEGL,  ILLEGL,   ILLEGL,  ILLEGL,  ILLEGL,     ILLEGL,   ILLEGL,  ILLEGL
};

/*
 * This table, which is indexed by "opcode" values, gives the necessary
 * information about each instruction.
 */
Instr_info instrTable[] = {
    {ILLEGL,     NTYP, {NSRC, NSRC, NSRC},  "ILLEGAL"          },
    {OP_ADD,     RTYP, {RD, RS, RT},        "add $%d,$%d,$%d"  },
    {OP_ADDI,    ITYP, {RT, RS, EXTRA},     "addi $%d,$%d,%d"  },
    {OP_ADDIU,   ITYP, {RT, RS, EXTRA},     "addiu $%d,$%d,%d" },
    {OP_ADDU,    RTYP, {RD, RS, RT},        "addu $%d,$%d,$%d" },
    {OP_AND,     RTYP, {RD, RS, RT},        "and $%d,$%d,$%d"  },
    {OP_ANDI,    ITYP, {RT, RS, EXTRA},     "andi $%d,$%d,0x%x"},
    {OP_BEQ,     ITYP, {RS, RT, EXTRA},     "beq $%d,$%d,%d"   }, 
    {OP_BGEZ,    ITYP, {RS, EXTRA, NSRC},   "bgez $%d,%d"      },
    {OP_BGEZAL,  ITYP, {RS, EXTRA, NSRC},   "bgezal $%d,%d"    },
    {OP_BGTZ,    ITYP, {RS, EXTRA, NSRC},   "bgtz $%d,%d"      },
    {OP_BLEZ,    ITYP, {RS, EXTRA, NSRC},   "blez $%d,%d"      }, 
    {OP_BLTZ,    ITYP, {RS, EXTRA, NSRC},   "bltz $%d,%d"      },
    {OP_BLTZAL,  ITYP, {RS, EXTRA, NSRC},   "bltzal $%d,%d"    },
    {OP_BNE,     ITYP, {RS, RT, EXTRA},     "bne $%d,$%d,%d"   },
    {ILLEGL,     NTYP, {NSRC, NSRC, NSRC},  "ILLEGAL"          },
    {OP_DIV,     RTYP, {RS, RT, NSRC},      "div $%d,$%d"      },
    {OP_DIVU,    RTYP, {RS, RT, NSRC},      "divu $%d,$%d"     },
    {OP_J,       JTYP, {EXTRA, NSRC, NSRC}, "j 0x%x"           },
    {OP_JAL,     JTYP, {EXTRA, NSRC, NSRC}, "jal 0x%x"         },
    {OP_JALR,    RTYP, {RD, RS, NSRC},      "jalr $%d,$%d"     },
    {OP_JR,      RTYP, {RS, NSRC, NSRC},    "jr $%d"           },
    {OP_LB,      ITYP, {RT, EXTRA, RS},     "lb $%d,%d($%d)"   },
    {OP_LBU,     ITYP, {RT, EXTRA, RS},     "lbu $%d,%d($%d)"  },
    {OP_LH,      ITYP, {RT, EXTRA, RS},     "lh $%d,%d($%d)"   },
    {OP_LHU,     ITYP, {RT, EXTRA, RS},     "lhu $%d,%d($%d)"  },
    {OP_LUI,     ITYP, {RT, EXTRA, NSRC},   "lui $%d,0x%x"     },
    {OP_LW,      ITYP, {RT, EXTRA, RS},     "lw $%d,%d($%d)"   },
    {OP_LWL,     ITYP, {RT, EXTRA, RS},     "lwl $%d,%d($%d)"  },
    {OP_LWR,     ITYP, {RT, EXTRA, RS},     "lwr $%d,%d($%d)"  },
    {ILLEGL,     NTYP, {NSRC, NSRC, NSRC},  "ILLEGAL"          },
    {OP_MFHI,    RTYP, {RD, NSRC, NSRC},    "mfhi $%d"         },
    {OP_MFLO,    RTYP, {RD, NSRC, NSRC},    "mflo $%d"         },
    {ILLEGL,     NTYP, {NSRC, NSRC, NSRC},  "ILLEGAL"          },
    {OP_MTHI,    RTYP, {RS, NSRC, NSRC},    "mthi $%d"         },
    {OP_MTLO,    RTYP, {RS, NSRC, NSRC},    "mtlo $%d"         }, 
    {OP_MULT,    RTYP, {RS, RT, NSRC},      "mult $%d,$%d"     },
    {OP_MULTU,   RTYP, {RS, RT, NSRC},      "multu $%d,$%d"    },
    {OP_NOR,     RTYP, {RD, RS, RT},        "nor $%d,$%d,$%d"  },
    {OP_OR,      RTYP, {RD, RS, RT},        "or $%d,$%d,$%d"   },
    {OP_ORI,     ITYP, {RT, RS, EXTRA},     "ori $%d,$%d,0x%x" },
    {OP_RFE,     NTYP, {NSRC, NSRC, NSRC},  "rfe"              },
    {OP_SB,      ITYP, {RT, EXTRA, RS},     "sb $%d,%d($%d)"   },
    {OP_SH,      ITYP, {RT, EXTRA, RS},     "sh $%d,%d($%d)"   },
    {OP_SLL,     RTYP, {RD, RT, EXTRA},     "sll $%d,$%d,%d"   },
    {OP_SLLV,    RTYP, {RD, RT, RS},        "sllv $%d,$%d,$%d" },
    {OP_SLT,     RTYP, {RD, RS, RT},        "slt $%d,$%d,$%d"  },
    {OP_SLTI,    ITYP, {RT, RS, EXTRA},     "slti $%d,$%d,%d"  },
    {OP_SLTIU,   ITYP, {RT, RS, EXTRA},     "sltiu $%d,$%d,%d" },
    {OP_SLTU,    RTYP, {RD, RS, RT},        "sltu $%d,$%d,$%d" },
    {OP_SRA,     RTYP, {RD, RT, EXTRA},     "sra $%d,$%d,%d"   },
    {OP_SRAV,    RTYP, {RD, RT, RS},        "srav $%d,$%d,$%d" },
    {OP_SRL,     RTYP, {RD, RT, EXTRA},     "srl $%d,$%d,%d"   },
    {OP_SRLV,    RTYP, {RD, RT, RS},        "srlv $%d,$%d,$%d" },
    {OP_SUB,     RTYP, {RD, RS, RT},        "sub $%d,$%d,$%d"  },
    {OP_SUBU,    RTYP, {RD, RS, RT},        "subu $%d,$%d,$%d" },
    {OP_SW,      ITYP, {RT, EXTRA, RS},     "sw $%d,%d($%d)"   },
    {OP_SWL,     ITYP, {RT, EXTRA, RS},     "swl $%d,%d($%d)"  },
    {OP_SWR,     ITYP, {RT, EXTRA, RS},     "swr $%d,%d($%d)"  },
    {OP_XOR,     RTYP, {RD, RS, RT},        "xor $%d,$%d,$%d"  },
    {OP_XORI,    ITYP, {RT, RS, EXTRA},     "xori $%d,$%d,0x%x"},
    {OP_SYSCALL, RTYP, {NSRC, NSRC, NSRC},  "syscall"          },
    {OP_BREAK,   RTYP, {EXTRA, NSRC, NSRC}, "break 0x%x"       },
    {OP_UNIMP,   NTYP, {NSRC, NSRC, NSRC},  "Unimplemented"    }
};
