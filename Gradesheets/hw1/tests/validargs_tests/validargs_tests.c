#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "hw1.h"
#include "const.h"


//-h with correct flags
Test(validargs_suite, help_test_01) {
	char* cmd = "bin/hw1 -h -a";
	int return_code = WEXITSTATUS(system(cmd));

	cr_assert_eq(return_code, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}

//-h with incorrect format
Test(validargs_suite, help_test_02) {
	char* cmd = "bin/hw1 -h g";
	int return_code = WEXITSTATUS(system(cmd));

	cr_assert_eq(return_code, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);


	cmd = "bin/hw1 -h -a -b 1000 e b -d";
	return_code = WEXITSTATUS(system(cmd));

	cr_assert_eq(return_code, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);


	cmd = "bin/hw1 -h -a -b -d";
	return_code = WEXITSTATUS(system(cmd));

	cr_assert_eq(return_code, EXIT_SUCCESS, "Program exited with %d instead of EXIT_SUCCESS",
		 return_code);
}


Test(validargs_suite, assemble_valid_01) {
	char* argv1[] = {"bin/hw1", "-a", NULL};
	int argc1 = 2;
	validargs(argc1, argv1);
	int ret = global_options;
	int exp = 0;

	cr_assert_eq(ret, exp, "Invalid return for valid args.  Got: %d | Expected: %d",
		 ret, exp);
}

Test(validargs_suite, assemble_valid_02) {
	char *argv2[] = {"bin/hw1", "-a", "-e", "b", "-b", "C000", NULL};
	int argc2 = 6;
	validargs(argc2, argv2);
	int ret = global_options;
	int exp = 0xC004;

	cr_assert_eq(ret, exp, "Invalid return for valid args.  Got: %d | Expected: %d",
		 ret, exp);
}

Test(validargs_suite, assemble_valid_03) {
	char *argv3[] = {"bin/hw1", "-a", "-e", "b", NULL};
	int argc3 = 4;
	validargs(argc3, argv3);
	int ret = global_options;
	int exp = 0x4;

	cr_assert_eq(ret, exp, "Invalid return for valid args.  Got: %d | Expected: %d",
		 ret, exp);
}

//incorrect number of args
Test(validargs_suite, incorrect_format_01) {
	char* argv1[] = {"bin/hw1", NULL};
	int argc1 = 1;
	int ret = validargs(argc1, argv1);
	int exp = 0;

	cr_assert_eq(ret, exp, "validargs returned %d instead of %d",
		 ret, exp);	
}

//make sure students implemented strcmp
Test(validargs_suite, incorrect_format_02) {
	char* argv2[] = {"bin/hw1", "a", NULL};
	int argc2 = 2;
	int ret = validargs(argc2, argv2);
	int exp = 0;

	cr_assert_eq(ret, exp, "validargs returned %d instead of %d",
		 ret, exp);	

	char* argv3[] = {"bin/hw1", "h", NULL};
	int argc3 = 2;
	ret = validargs(argc3, argv3);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned %d instead of %d",
		 ret, exp);	
	
	char* argv4[] = {"bin/hw1", "-", NULL};
	int argc4 = 2;
	ret = validargs(argc4, argv4);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned %d instead of %d",
		 ret, exp);	
}
	
//invalid format/missing args for b flag
Test(validargs_suite, b_flag_invalid_01) {
	char* argv1[] = {"bin/hw1", "-b", NULL};
	int argc1 = 2;
	int ret = validargs(argc1, argv1);
	int exp = 0;
	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	

	char* argv2[] = {"bin/hw1", "-a", "-b", NULL};
	int argc2 = 3;
	ret = validargs(argc2, argv2);
	exp = 0;
	
	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	

	char* argv5[] = {"bin/hw1", "-d", "b", "1000", NULL};
	int argc5 = 4;
	ret = validargs(argc5, argv5);
	exp = 0;
	
	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	
}

//invalid address for -b
Test(validargs_suite, b_flag_invalid_02) {
	char* argv3[] = {"bin/hw1", "-a", "-b", "BaB005", NULL};
	int argc3 = 4;
	int ret = validargs(argc3, argv3);
	int exp = 0;
	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	
	
	char* argv4[] = {"bin/hw1", "-d", "-b", "CSSE320", NULL};
	int argc4 = 4;
	ret = validargs(argc4, argv4);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	

	char* argv5[] = {"bin/hw1", "-d", "-b", "100000000", NULL};
	int argc5 = 4;
	ret = validargs(argc5, argv5);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	
}	

//wrong format
Test(validargs_suite, e_flag_invalid_01) {
	char* argv1[] = {"bin/hw1", "-e", "l", NULL};
	int argc1 = 3;
	int ret = validargs(argc1, argv1);
	int exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	

	char* argv3[] = {"bin/hw1", "-d", "-e", NULL};
	int argc3 = 3;
	ret = validargs(argc3, argv3);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	
}

//false argument
Test(validargs_suite, e_flag_invalid_02) {
	char* argv2[] = {"bin/hw1", "-a", "-e", "E", NULL};
	int argc2 = 4;
	int ret = validargs(argc2, argv2);
	int exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);		

	char* argv3[] = {"bin/hw1", "-a", "-e", "ll", NULL};
	int argc3 = 4;
	ret = validargs(argc3, argv3);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);
}

//incorrect ordering
Test(validargs_suite, incorrect_ordering) {
	char* argv1[] = {"bin/hw1", "-a", "-d", NULL};
	int argc1 = 3;
	int ret = validargs(argc1, argv1);
	int exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);		 

	char* argv2[] = {"bin/hw1", "-e", "b", "-a", NULL};
	int argc2 = 4;
	ret = validargs(argc2, argv2);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	


	char* argv3[] = {"bin/hw1", "-d", "-e", "-b", "l", NULL};
	int argc3 = 5;
	ret = validargs(argc3, argv3);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	
		 

	char* argv4[] = {"bin/hw1", "-d", "-e", "b", "-h", NULL};
	int argc4 = 5;
	ret = validargs(argc4, argv4);
	exp = 0;

	cr_assert_eq(ret, exp, "validargs returned with %d instead of %d",
		 ret, exp);	
}
