#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "hw1.h"

/* the command poses error: sh: 1: Syntax error: "(" unexpected
Test(multiple_branch_suite, branch16) {
    char *cmd = "cmp test_rsrc/branch16.bin <(bin/hw1 -a < test_rsrc/branch16.asm)";
    int return_code = WEXITSTATUS(system(cmd));
    cr_assert_eq(return_code, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS", return_code);
}
*/

#define ERR_RETURN_SUCCESS "Program exited with %d instead of EXIT_SUCCESS"
#define ERR_RETURN_FAILURE "Program exited with %d instead of EXIT_FAILURE"

void remove_enc_branch16_le(void) {
    remove("enc_branch16_le.bin");
}
/* encode 16 line branch instructions in little endian */
Test(multiple_branch_suite, enc_branch16_le, .fini = remove_enc_branch16_le) {
    // little endian encode - write to stdn_write/branch16.bin
    char *cmd1 = "bin/hw1 -a < tests/rsrc/branch16.asm > enc_branch16_le.bin";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/branch16.bin enc_branch16_le.bin";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret2);
}

void remove_enc_branch16_be(void) {
    remove("enc_branch16_be.bin");
}
/* encode 16 line branch instructions in big endian */
Test(multiple_branch_suite, enc_branch16_be, .fini = remove_enc_branch16_be) {
    // big endian encode - write to stdn_write/branch16_be.bin
    char *cmd1 = "bin/hw1 -a -e b < tests/rsrc/branch16.asm > enc_branch16_be.bin";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/branch16_be.bin enc_branch16_be.bin";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret2);
}

void remove_dec_branch16_le(void) {
    remove("dec_branch16_le.asm");
}
/* decode 16 line branch instructions in little endian */
Test(multiple_branch_suite, dec_branch16_le, .fini = remove_dec_branch16_le) {
    // little endian decode - write to stdn_write/branch16.asm
    char *cmd1 = "bin/hw1 -d < tests/rsrc/branch16.bin > dec_branch16_le.asm";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/branch16.asm dec_branch16_le.asm";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret2);
}

void remove_dec_branch16_be(void) {
    remove("dec_branch16_be.asm");
}
/* decode 16 line branch instructions in big endian */
Test(multiple_branch_suite, dec_branch16_be, .fini = remove_dec_branch16_be) {
    // big endian decode - write to stdn_write/branch16_be.asm
    char *cmd1 = "bin/hw1 -d -e b < tests/rsrc/branch16_be.bin > dec_branch16_be.asm";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/branch16.asm dec_branch16_be.asm";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret2);
}

void remove_dec_branch16_cab000(void) {
    remove("dec_branch16_cab000.asm");
}
/* decode 16 line branch instructions with base address cab000 */
Test(multiple_branch_suite, dec_branch16_cab000, .fini = remove_dec_branch16_cab000) {
    // write to stdn_write/branch16_cab000.asm
    char *cmd1 = "bin/hw1 -d -b cab000 < tests/rsrc/branch16.bin > dec_branch16_cab000.asm";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/branch16_cab000.asm dec_branch16_cab000.asm";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret2);
}

void remove_enc_branch16_cab000(void) {
    remove("enc_branch16_cab000.bin");
}
/* encode 16 line branch instructions with base address cab000 (back to original bin file)*/
Test(multiple_branch_suite, enc_branch16_cab000, .fini = remove_enc_branch16_cab000) {
    // write to stdn_write/branch16_cab000.bin
    char *cmd1 = "bin/hw1 -a -b cab000 < tests/rsrc/branch16_cab000.asm > enc_branch16_cab000.bin";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/branch16.bin enc_branch16_cab000.bin";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret2);
}

void remove_enc_jump10_le(void) {
    remove("enc_jump10_le.bin");
}
/* encode 10 line jump instructions in little endian */
Test(multiple_jump_suite, enc_jump10_le, .fini = remove_enc_jump10_le) {
    // little endian encode - write to stdn_write/jump10.bin
    char *cmd1 = "bin/hw1 -a < tests/rsrc/jump10.asm > enc_jump10_le.bin";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/jump10.bin enc_jump10_le.bin";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, , ret2);
}

void remove_enc_jump10_be(void) {
    remove("enc_jump10_be.bin");
}
/* encode 10 line jump instructions in big endian */
Test(multiple_jump_suite, enc_jump10_be, .fini = remove_enc_jump10_be) {
    // big endian encode - write to stdn_write/jump10_be.bin
    char *cmd1 = "bin/hw1 -a -e b < tests/rsrc/jump10.asm > enc_jump10_be.bin";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/jump10_be.bin enc_jump10_be.bin";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS", ret2);
}

void remove_dec_jump10_le(void) {
    remove("dec_jump10_le.asm");
}
/* decode 10 line jump instructions in little endian */
Test(multiple_jump_suite, dec_jump10_le, .fini = remove_dec_jump10_le) {
    // little endian decode - write to stdn_write/jump10.asm
    char *cmd1 = "bin/hw1 -d < tests/rsrc/jump10.bin > dec_jump10_le.asm";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/jump10.asm dec_jump10_le.asm";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS", ret2);
}

void remove_dec_jump10_be(void) {
    remove("dec_jump10_be.asm");
}
/* decode 10 line branch instructions in big endian */
Test(multiple_jump_suite, dec_jump10_be, .fini = remove_dec_jump10_be) {
    // big endian decode - write to stdn_write/jump10_be.asm
    char *cmd1 = "bin/hw1 -d -e b < tests/rsrc/jump10_be.bin > dec_jump10_be.asm";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/jump10.asm dec_jump10_be.asm";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS", ret2);
}

void remove_dec_jump10_bebeb000(void) {
    remove("dec_jump10_bebeb000.asm");
}
/* decode 10 line jump instructions with base address bebeb000 */
Test(multiple_jump_suite, dec_jump10_bebeb000, .fini = remove_dec_jump10_bebeb000) {
    // write to stdn_write/jump10_bebeb000.asm
    char *cmd1 = "bin/hw1 -d -b bebeb000 < tests/rsrc/jump10.bin > dec_jump10_bebeb000.asm";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/jump10_bebeb000.asm dec_jump10_bebeb000.asm";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS", ret2);
}

void remove_enc_jump10_bebeb000(void) {
    remove("enc_jump10_bebeb000.bin");
}
/* encode 10 line jump instructions with base address bebeb000 (back to original bin file)*/
Test(multiple_jump_suite, enc_jump10_bebeb000, .fini = remove_enc_jump10_bebeb000) {
    // write to stdn_write/branch16_cab000.bin
    char *cmd1 = "bin/hw1 -a -b bebeb000 < tests/rsrc/jump10_bebeb000.asm > enc_jump10_bebeb000.bin";
    int ret1 = WEXITSTATUS(system(cmd1));
    cr_assert_eq(ret1, EXIT_SUCCESS, ERR_RETURN_SUCCESS, ret1);

    char *cmd2 = "cmp tests/rsrc/jump10.bin enc_jump10_bebeb000.bin";
    int ret2 = WEXITSTATUS(system(cmd2));
    cr_assert_eq(ret2, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS", ret2);
}

Test(invalid_suite, register_negative) {

    char *cmd1 = "bin/hw1 -a < tests/rsrc/register_negative.asm";

    int ret1 = WEXITSTATUS(system(cmd1));

    cr_assert_eq(ret1, EXIT_FAILURE, ERR_RETURN_FAILURE, ret1);
}

Test(invalid_suite, register_too_big) {

    char *cmd1 = "bin/hw1 -a < tests/rsrc/register_too_big.asm";

    int ret1 = WEXITSTATUS(system(cmd1));

    cr_assert_eq(ret1, EXIT_FAILURE, ERR_RETURN_FAILURE, ret1);
}

Test(invalid_suite, branch_addr_not_multiple4) {

    char *cmd1 = "bin/hw1 -a < tests/rsrc/branch_addr_not_multiple4.asm";

    int ret1 = WEXITSTATUS(system(cmd1));

    cr_assert_eq(ret1, EXIT_FAILURE, ERR_RETURN_FAILURE, ret1);
}

/* test encode with base address 0xa0000000, which has different 4 most significant bits from jump10_bebeb000.asm */
Test(invalid_suite, dif_4msb_target_pc) {

    char *cmd1 = "bin/hw1 -a -b a0000000 < tests/rsrc/jump10_bebeb000.asm";

    int ret1 = WEXITSTATUS(system(cmd1));

    cr_assert_eq(ret1, EXIT_FAILURE, ERR_RETURN_FAILURE, ret1);
}

Test(invalid_suite, junk_after_instruct) {

    char *cmd1 = "bin/hw1 -a < tests/rsrc/junk_after_instruct.asm";

    int ret1 = WEXITSTATUS(system(cmd1));

    cr_assert_eq(ret1, EXIT_FAILURE, ERR_RETURN_FAILURE, ret1);
}