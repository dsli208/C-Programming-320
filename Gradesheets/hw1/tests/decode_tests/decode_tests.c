#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "../include/hw1.h"
#include <string.h>

#define DECODE_SUCCESS 1
#define DECODE_FAILURE 0
#define ERR_RETURN "error: return value invalid exp:%d but Got:%d"
#define ERR_REGS   "error: regs field not assigned correctly exp:{%d, %d, %d}, but Got:{%d, %d, %d}"
#define ERR_ARGS   "error: args field not assigned correclty exp:{%d, %d, %d}, but Got:{%d, %d, %d}"
#define ERR_INFO   "error: info field not assigned correclty exp:{.opcode=%d, .type=%d, .srcs={%d, %d, %d}, .format=%s}, but Got:{.opcode=%d, .type=%d, .srcs={%d, %d, %d}, .format=%s}"

int args_test(int *args, int *exp) {
    if (args[0] == exp[0] && args[1] == exp[1] && args[2] == exp[2]) {
        return DECODE_SUCCESS;
    }
    return DECODE_FAILURE;
}

int regs_test(char *regs, char *exp) {
    if (regs[0] == exp[0] && regs[1] == exp[1] && regs[2] == regs[2]) {
        return DECODE_SUCCESS;
    }
    return DECODE_FAILURE;
}

int info_test(Instr_info info, Instr_info exp) {
    if (info.opcode == exp.opcode && info.type == exp.type && !strcmp(info.format, exp.format)) {
        return DECODE_SUCCESS;
    }
    return DECODE_FAILURE;
}

Test(decode_suite, example1_from_doc) {

    Instruction instruction = {.value = 0x00c72820};
    char regs_exp[3] = {6, 7, 5};
    int extra_exp = 0;
    int args_exp[3] = {5, 6, 7};
    Instr_info info_exp ={.opcode = OP_ADD, .type = RTYP, .srcs = {RD, RS, RT}, .format = "add $%d,$%d,$%d"};

    int ret_act = decode(&instruction, 0x0);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, example2_from_doc) {

    Instruction instruction = {.value = 0x8cc50007};
    char regs_exp[3] = {6, 5, 0};
    int extra_exp = 7;
    int args_exp[3] = {5, 7, 6};
    Instr_info info_exp = {.opcode = OP_LW, .type = ITYP, .srcs = {RT, EXTRA, RS}, .format = "lw $%d,%d($%d)"};

    int ret_act = decode(&instruction, 0x0);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, example3_from_doc) {

    Instruction instruction = {.value = 0x10effc1f};
    char regs_exp[3] = {7, 15, 31};
    int extra_exp = 0xfffffc1f;
    int args_exp[3] = {7, 15, 128};
    Instr_info info_exp = {.opcode = OP_BEQ, .type = ITYP, .srcs = {RS, RT, EXTRA}, .format = "beq $%d,$%d,%d"};

    int ret_act = decode(&instruction, 0x1000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, example4_from_doc) {

    Instruction instruction = {.value = 0x08000400};
    char regs_exp[3] = {0, 0, 0};
    int extra_exp = 0x400;
    int args_exp[3] = {4096, 0, 0};
    Instr_info info_exp = {OP_J, JTYP, {EXTRA, NSRC, NSRC}, "j 0x%x"};

    int ret_act = decode(&instruction, 0x1000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);
/*
    printf("ret_act %d\n", ret_act);
    printf("i.regs  %d %d %d\n", instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    printf("i.args  %d %d %d\n", instruction.args[0], instruction.args[1], instruction.args[2]);
    printf("i.extra %d\n", instruction.extra);
    printf("i.info->opcode %d exp %d  \n", instruction.info->opcode, OP_ADD);
    printf("i.info->type %d   exp %d \n", instruction.info->type, RTYP);
    printf("i.info->srcs %d %d %d\n", instruction.info->srcs[0],instruction.info->srcs[1],instruction.info->srcs[2]);
    printf("i.info->format %s\n", instruction.info->format);
*/
    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, illegl_from_opcodeTable) {

    int illgl_index = 63;  // we will change this value when we change the order of opcodeTable.
    Instruction instruction = {.value = illgl_index<<26};

    int ret_act = decode(&instruction, 0x0);

    cr_assert_eq(ret_act, DECODE_FAILURE, ERR_RETURN, DECODE_FAILURE, ret_act);
}

Test(decode_suite, illegl_from_specialTable) {

    int special_index = 0;  // we will change this value when we change the order of opcodeTable.
    int illgl_index = 1;  // we will change this value when we change the order of specialTable.
    Instruction instruction = {.value = special_index<<26 | illgl_index};

    int ret_act = decode(&instruction, 0x0);

    cr_assert_eq(ret_act, DECODE_FAILURE, ERR_RETURN, DECODE_FAILURE, ret_act);
}

Test(decode_suite, bcond_bltz) {

    Instruction instruction = {.value = 0x04e0fc19};
    char regs_exp[3] = {7, 0, 31};
    int extra_exp = 0xfffffc19;
    int args_exp[3] = {7, 104, 0};
    Instr_info info_exp = {OP_BLTZ, ITYP, {RS, EXTRA, NSRC}, "bltz $%d,%d"};

    int ret_act = decode(&instruction, 0x1000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, bcond_bgez) {

    Instruction instruction = {.value = 0x04e1fc1d};
    char regs_exp[3] = {7, 1, 31};
    int extra_exp = 0xfffffc1d;
    int args_exp[3] = {7, 120, 0};
    Instr_info info_exp = {OP_BGEZ, ITYP, {RS, EXTRA, NSRC}, "bgez $%d,%d"};

    int ret_act = decode(&instruction, 0x1000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, bcond_bltzal) {

    Instruction instruction = {.value = 0x04f0fc18};
    char regs_exp[3] = {7, 16, 31};
    int extra_exp = 0xfffffc18;
    int args_exp[3] = {7, 100, 0};
    Instr_info info_exp = {OP_BLTZAL, ITYP, {RS, EXTRA, NSRC}, "bltzal $%d,%d"};
    int ret_act = decode(&instruction, 0x1000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, bcond_bgezal) {

    Instruction instruction = {.value = 0x04f1fc1c};
    char regs_exp[3] = {7, 17, 31};
    int extra_exp = 0xfffffc1c;
    int args_exp[3] = {7, 116, 0};
    Instr_info info_exp = {OP_BGEZAL, ITYP, {RS, EXTRA, NSRC}, "bgezal $%d,%d"};
    int ret_act = decode(&instruction, 0x1000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, op_break) {

    Instruction instruction = {.value = 0x0002af0d};
    char regs_exp[3] = {0, 2, 21};
    int extra_exp = 0xabc;
    int args_exp[3] = {0xabc, 0, 0};
    Instr_info info_exp = {OP_BREAK, RTYP, {EXTRA, NSRC, NSRC}, "break 0x%x"};
    int ret_act = decode(&instruction, 0x3000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, rtype_or) {

    Instruction instruction = {.value = 0x00641025};
    char regs_exp[3] = {3, 4, 2};
    int extra_exp = 0;
    int args_exp[3] = {2, 3, 4};
    Instr_info info_exp = {OP_OR, RTYP, {RD, RS, RT}, "or $%d,$%d,$%d"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, rtype_sllv) {

    Instruction instruction = {.value = 0x00831004};
    char regs_exp[3] = {4, 3, 2};
    int extra_exp = 0;
    int args_exp[3] = {2, 3, 4};
    Instr_info info_exp = {OP_SLLV, RTYP, {RD, RT, RS}, "sllv $%d,$%d,$%d"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, itype_addi) {

    Instruction instruction = {.value = 0x20c50309};
    char regs_exp[3] = {6, 5, 0};
    int extra_exp = 777;
    int args_exp[3] = {5, 6, 777};
    Instr_info info_exp = {OP_ADDI, ITYP, {RT, RS, EXTRA}, "addi $%d,$%d,%d"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, itype_lui) {

    Instruction instruction = {.value = 0x3c050abc};
    char regs_exp[3] = {0, 5, 1};
    int extra_exp = 0xabc;
    int args_exp[3] = {5, 0xabc, 0};
    Instr_info info_exp = {OP_LUI, ITYP, {RT, EXTRA, NSRC}, "lui $%d,0x%x"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, itype_sw) {

    Instruction instruction = {.value = 0xad230018};
    char regs_exp[3] = {9, 3, 0};
    int extra_exp = 24;
    int args_exp[3] = {3, 24, 9};
    Instr_info info_exp = {OP_SW, ITYP, {RT, EXTRA, RS}, "sw $%d,%d($%d)"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}
/*
Test(decode_suite, jtype_j) {

    Instruction instruction = {.value = 0x082af000};
    char regs_exp[3] = {1, 10, 30};
    int extra_exp = 0x2af000;
    int args_exp[3] = {0xabc000, 0, 0};
    Instr_info info_exp = {OP_J, JTYP, {EXTRA, NSRC, NSRC}, "j 0x%x"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}
*/
Test(decode_suite, jtype_jal) {

    Instruction instruction = {.value = 0x0c2af000};
    char regs_exp[3] = {1, 10, 30};
    int extra_exp = 0x2af000;
    int args_exp[3] = {0xabc000, 0, 0};
    Instr_info info_exp = {OP_JAL, JTYP, {EXTRA, NSRC, NSRC}, "jal 0x%x"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

Test(decode_suite, negative_immediate) {
    Instruction instruction = {.value = 0x256affec};
    char regs_exp[3] = {11, 10, 31};
    int extra_exp = -20;
    int args_exp[3] = {10, 11, -20};
    Instr_info info_exp = {OP_ADDIU, ITYP, {RT, RS, EXTRA}, "addiu $%d,$%d,%d"};

    int ret_act = decode(&instruction, 0xcde000);
    int regs_act = regs_test(instruction.regs, regs_exp);
    int args_act = args_test(instruction.args, args_exp);
    int info_act = info_test(*instruction.info, info_exp);

    cr_assert_eq(ret_act, DECODE_SUCCESS, ERR_RETURN, DECODE_SUCCESS, ret_act);
    cr_assert_eq(regs_act, DECODE_SUCCESS, ERR_REGS, regs_exp[0], regs_exp[1], regs_exp[2], instruction.regs[0], instruction.regs[1], instruction.regs[2]);
    cr_assert_eq(args_act, DECODE_SUCCESS, ERR_ARGS, args_exp[0], args_exp[1], args_exp[2], instruction.args[0], instruction.args[1], instruction.args[2]);
    cr_assert_eq(info_act, DECODE_SUCCESS, ERR_INFO, info_exp.opcode, info_exp.type, info_exp.srcs[0], info_exp.srcs[1], info_exp.srcs[2], info_exp.format, instruction.info->opcode, instruction.info->type, instruction.info->srcs[0], instruction.info->srcs[1], instruction.info->srcs[2], instruction.info->format);
}

