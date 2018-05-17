#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "hw1.h"

Test(hw1_tests_suite, validargs_help_test) {
    int argc = 2;
    char *argv[] = {"bin/hw1", "-h", NULL};
    int ret = validargs(argc, argv);
    int exp_ret = 1;
    unsigned int opt = global_options;
    unsigned int flag = 0x1;
    cr_assert_eq(ret, exp_ret, "Invalid return for valid args.  Got: %d | Expected: %d",
		 ret, exp_ret);
    cr_assert_eq(opt & flag, flag, "Least significant bit not set for -h. Got: %d",
		 opt);
}

Test(hw1_tests_suite, validargs_disassem_test) {
    int argc = 4;
    char *argv[] = {"bin/hw1", "-d", "-b", "1000", NULL};
    int ret = validargs(argc, argv);
    int exp_ret = 1;
    unsigned int opt = global_options;
    unsigned int flag = 0x2;
    cr_assert_eq(ret, exp_ret, "Invalid return for valid args.  Got: %d | Expected: %d",
		 ret, exp_ret);
    cr_assert_eq(opt & flag, flag, "Disassemble bit wasn't set. Got: %d",
		 opt);
}

Test(hw1_tests_suite, help_system_test) {
    char *cmd = "bin/hw1 -h";

    // system is a syscall defined in stdlib.h
    // it takes a shell command as a string and runs it
    // we use WEXITSTATUS to get the return code from the run
    // use 'man 3 system' to find out more
    int return_code = WEXITSTATUS(system(cmd));

    cr_assert_eq(return_code, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}
