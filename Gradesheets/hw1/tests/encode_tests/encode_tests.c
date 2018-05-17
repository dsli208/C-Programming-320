#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "../include/hw1.h"

#define ENCODE_SUCCESS 1
#define ENCODE_FAILURE 0
#define ERR_RETURN "error: return value invalid exp:%d but Got:%d"
#define ERR_VALUE "error: value field invalid exp:%X but Got:%X"

Test(encode_suite, example1_from_doc) {

    Instr_info info = {.opcode = OP_ADD, .type = RTYP, .srcs = {RD, RS, RT}, .format = "add $%d,$%d,$%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {6, 7, 5},
        .extra = 0,
        .args = {5, 6, 7},
    };
    int value_exp = 0x00c72820;

    int ret_act = encode(&instruction, 0x0);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, example2_from_doc) {

    Instr_info info = {.opcode = OP_LW, .type = ITYP, .srcs = {RT, EXTRA, RS}, .format = "lw $%d,%d($%d)"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {6, 5, 0},
        .extra = 7,
        .args = {5, 7, 6},
    };
    int value_exp = 0x8cc50007;

    int ret_act = encode(&instruction, 0x0);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, example3_from_doc) {

    Instr_info info = {.opcode = OP_BEQ, .type = ITYP, .srcs = {RS, RT, EXTRA}, .format = "beq $%d,$%d,%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {7, 15, 31},
        .extra = 0xfffffc1f,
        .args = {7, 15, 128},
    };
    int value_exp = 0x10effc1f;

    int ret_act = encode(&instruction, 0x1000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, example4_from_doc) {

    Instr_info info = {OP_J, JTYP, {EXTRA, NSRC, NSRC}, "j 0x%x"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {0, 0, 0},
        .extra = 0x400,
        .args = {4096, 0, 0},
    };
    int value_exp = 0x08000400;

    int ret_act = encode(&instruction, 0x1000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, unaligned_branch_offset) {

    Instr_info info = {.opcode = OP_BEQ, .type = ITYP, .srcs = {RS, RT, EXTRA}, .format = "beq $%d,$%d,%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {7, 15, 31},
        .extra = 0xfffffc1f,
        .args = {7, 15, 127},
    };

    int ret_act = encode(&instruction, 0x1000);

    cr_assert_eq(ret_act, ENCODE_FAILURE, ERR_RETURN, ENCODE_FAILURE, ret_act);
}

Test(encode_suite, bcond_bltz) {

    Instr_info info = {OP_BLTZ, ITYP, {RS, EXTRA, NSRC}, "bltz $%d,%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {7, 0, 31},
        .extra = 0xfffffc19,
        .args = {7, 104, 0},
    };
    int value_exp = 0x04e0fc19;

    int ret_act = encode(&instruction, 0x1000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, bcond_bgez) {

    Instr_info info = {OP_BGEZ, ITYP, {RS, EXTRA, NSRC}, "bgez $%d,%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {7, 1, 31},
        .extra = 0xfffffc1d,
        .args = {7, 120, 0},
    };
    int value_exp = 0x04e1fc1d;

    int ret_act = encode(&instruction, 0x1000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, bcond_bltzal) {

    Instr_info info = {OP_BLTZAL, ITYP, {RS, EXTRA, NSRC}, "bltzal $%d,%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {7, 16, 31},
        .extra = 0xfffffc18,
        .args = {7, 100, 0},
    };
    int value_exp = 0x04f0fc18;

    int ret_act = encode(&instruction, 0x1000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, bcond_bgezal) {

    Instr_info info = {OP_BGEZAL, ITYP, {RS, EXTRA, NSRC}, "bgezal $%d,%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {7, 17, 31},
        .extra = 0xfffffc1c,
        .args = {7, 116, 0},
    };
    int value_exp = 0x04f1fc1c;

    int ret_act = encode(&instruction, 0x1000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, op_break) {

    Instr_info info = {OP_BREAK, RTYP, {EXTRA, NSRC, NSRC}, "break 0x%x"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {0, 2, 21},
        .extra = 0xfffffc1c,
        .args = {0xabc, 0, 0},
    };
    int value_exp = 0x0002af0d;

    int ret_act = encode(&instruction, 0x3000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, rtype_or) {

    Instr_info info = {OP_OR, RTYP, {RD, RS, RT}, "or $%d,$%d,$%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {3, 4, 2},
        .extra = 0,
        .args = {2, 3, 4},
    };
    int value_exp = 0x00641025;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, rtype_sllv) {

    Instr_info info = {OP_SLLV, RTYP, {RD, RT, RS}, "sllv $%d,$%d,$%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {4, 3, 2},
        .extra = 0,
        .args = {2, 3, 4},
    };
    int value_exp = 0x00831004;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, itype_addi) {

    Instr_info info = {OP_ADDI, ITYP, {RT, RS, EXTRA}, "addi $%d,$%d,%d"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {6, 5, 0},
        .extra = 777,
        .args = {5, 6, 777},
    };
    int value_exp = 0x20c50309;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, itype_lui) {

    Instr_info info = {OP_LUI, ITYP, {RT, EXTRA, NSRC}, "lui $%d,0x%x"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {0, 5, 1},
        .extra = 0xabc,
        .args = {5, 0xabc, 0},
    };
    int value_exp = 0x3c050abc;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, itype_sw) {

    Instr_info info = {OP_SW, ITYP, {RT, EXTRA, RS}, "sw $%d,%d($%d)"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {9, 3, 0},
        .extra = 24,
        .args = {3, 24, 9},
    };
    int value_exp = 0xad230018;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

Test(encode_suite, jtype_j) {

    Instr_info info = {OP_J, JTYP, {EXTRA, NSRC, NSRC}, "j 0x%x"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {1, 10, 30},
        .extra = 0x2af000,
        .args = {0xabc000, 0, 0},
    };
    int value_exp = 0x082af000;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}
/*
Test(encode_suite, jtype_jal) {

    Instr_info info = {OP_JAL, JTYP, {EXTRA, NSRC, NSRC}, "jal 0x%x"};
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {1, 10, 30},
        .extra = 0x2af000,
        .args = {0xabc000, 0, 0},
    };
    int value_exp = 0x0c2af000;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}
*/

Test(encode_suite, nagative_immediate) {

    Instr_info info = {OP_ADDIU, ITYP, {RT, RS, EXTRA}, "addiu $%d,$%d,%d" };
    Instruction instruction = {
        .value = 0,
        .info = &info,
        .regs = {11, 10, 31},
        .extra = -20,
        .args = {10, 11, -20},
    };
    int value_exp = 0x256affec;

    int ret_act = encode(&instruction, 0xcde000);

    cr_assert_eq(ret_act, ENCODE_SUCCESS, ERR_RETURN, ENCODE_SUCCESS, ret_act);
    cr_assert_eq(instruction.value, value_exp, ERR_VALUE, value_exp, instruction.value);
}

