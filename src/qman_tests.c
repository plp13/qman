// Unit testing

// To create a test named `foo`:
// - Define your test function `void test_foo()`
// - Insert `add_test(foo)` into `main()`
//
// Invoking `qman_tests <test name>` runs a specific test.
// Exit code 0: test run was successful
//           1: test run failed
//          -1: test not found
//
// Invoking `qman_tests all` runs all tests, and returns the number of failures
// in its exit code.

#include "lib.h"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

//
// Helper macros
//

#define init_test_suite()                                                      \
  bool test_found = false;                                                     \
  int errors = 0;                                                              \
  CU_pSuite suite;                                                             \
  if (argc > 1) {                                                              \
    printf("Running: ");                                                       \
    CU_initialize_registry();                                                  \
    suite = CU_add_suite("qman", NULL, NULL);                                  \
  }

#define add_test(tst)                                                          \
  if (argc > 1) {                                                              \
    if (0 == strcmp(argv[1], #tst) || 0 == strcmp(argv[1], "all")) {           \
      test_found = true;                                                       \
      printf(#tst "\n         ");                                              \
      CU_add_test(suite, #tst, test_##tst);                                    \
    }                                                                          \
  }

#define winddown_test_suite()                                                  \
  if (argc > 1) {                                                              \
    if (test_found) {                                                          \
      printf("\n");                                                            \
      CU_basic_run_tests();                                                    \
      errors = CU_get_number_of_failures();                                    \
      CU_cleanup_registry();                                                   \
      return errors;                                                           \
    } else {                                                                   \
      printf("No such test '%s'\n", argv[1]);                                  \
      return -1;                                                               \
    }                                                                          \
  } else {                                                                     \
    printf("Usage: %s <test name>  # run a single test\n", argv[0]);           \
    printf("       %s all          # run all tests\n", argv[0]);               \
    return 0;                                                                  \
  }

//
// Test functions
//

void test_eini_parse() { CU_ASSERT_EQUAL(0, 2); }
void test_foo() { CU_ASSERT_EQUAL(1, 1); }
void test_bar() { CU_ASSERT_EQUAL(2, 2); }

// Where we hope it works
int main(int argc, char **argv) {
  init_test_suite();

  // `add_test()` all your tests here
  add_test(eini_parse);
  add_test(foo);
  add_test(bar);

  winddown_test_suite();
}
