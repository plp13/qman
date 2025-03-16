// Unit testing

// To create a test named `foo`:
// - Define your test function `void test_foo()`
// - Insert `add_test(foo)` into `main()`
//
// Invoking `qman_tests <test name>` runs a specific test, while `qman_tests
// all` runs all tests.
//
// Exit codes:
//           0: all tests succeeded
//           n: n failures happened during testing
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

// Initialize the test suite
#define init_test_suite()                                                      \
  bool test_found = false;                                                     \
  int errors = 0;                                                              \
  CU_pSuite suite;                                                             \
  if (argc > 1) {                                                              \
    printf("Running: ");                                                       \
    CU_initialize_registry();                                                  \
    suite = CU_add_suite("qman", NULL, NULL);                                  \
  }

// Add test `tst`. The name of the test function must be `test_<tst>`.
#define add_test(tst)                                                          \
  if (argc > 1) {                                                              \
    if (0 == strcmp(argv[1], #tst) || 0 == strcmp(argv[1], "all")) {           \
      test_found = true;                                                       \
      printf(#tst "\n         ");                                              \
      CU_add_test(suite, #tst, test_##tst);                                    \
    }                                                                          \
  }

// Depending on `argc`/`argv`, run one test, or run all tests, or print usage
// information. Then exit.
#define run_tests_and_exit()                                                   \
  if (argc > 1) {                                                              \
    if (test_found) {                                                          \
      printf("\n");                                                            \
      CU_basic_run_tests();                                                    \
      errors = CU_get_number_of_failures();                                    \
      CU_cleanup_registry();                                                   \
      winddown(errors, NULL);                                                  \
    } else {                                                                   \
      printf("No such test '%s'\n", argv[1]);                                  \
      winddown(-1, NULL);                                                      \
    }                                                                          \
  } else {                                                                     \
    printf("Usage: %s <test name>  # run a single test\n", argv[0]);           \
    printf("       %s all          # run all tests\n", argv[0]);               \
    winddown(0, NULL);                                                         \
  }

//
// Test functions
//

void test_eini_parse() {
  eini_t parsed;

  eini_init();

  parsed = eini_parse("include /usr/share/foo");
  CU_ASSERT_EQUAL(parsed.type, EINI_INCLUDE);
  CU_ASSERT(0 == wcscmp(parsed.value, L"/usr/share/foo"));

  parsed = eini_parse("\t include\t\t/usr/share/foo ");
  CU_ASSERT_EQUAL(parsed.type, EINI_INCLUDE);
  CU_ASSERT(0 == wcscmp(parsed.value, L"/usr/share/foo"));

  parsed = eini_parse("include \"/usr/share/foo\"");
  CU_ASSERT_EQUAL(parsed.type, EINI_INCLUDE);
  CU_ASSERT(0 == wcscmp(parsed.value, L"/usr/share/foo"));

  parsed = eini_parse(" include \t  \'/usr/share/foo\'  \t ");
  CU_ASSERT_EQUAL(parsed.type, EINI_INCLUDE);
  CU_ASSERT(0 == wcscmp(parsed.value, L"/usr/share/foo"));

  parsed = eini_parse("include /usr/share/foo ; comment");
  CU_ASSERT_EQUAL(parsed.type, EINI_INCLUDE);
  CU_ASSERT(0 == wcscmp(parsed.value, L"/usr/share/foo"));

  parsed = eini_parse("include \"/usr/share/foo\" ; comment");
  CU_ASSERT_EQUAL(parsed.type, EINI_INCLUDE);
  CU_ASSERT(0 == wcscmp(parsed.value, L"/usr/share/foo"));

  parsed = eini_parse("include \'/usr/share/foo ; comment\'");
  CU_ASSERT_EQUAL(parsed.type, EINI_INCLUDE);
  CU_ASSERT(0 == wcscmp(parsed.value, L"/usr/share/foo ; comment"));

  parsed = eini_parse("[section_one]");
  CU_ASSERT_EQUAL(parsed.type, EINI_SECTION);
  CU_ASSERT(0 == wcscmp(parsed.value, L"section_one"));

  parsed = eini_parse("  [\tSectionTwo  ]\t\t \n");
  CU_ASSERT_EQUAL(parsed.type, EINI_SECTION);
  CU_ASSERT(0 == wcscmp(parsed.value, L"SectionTwo"));

  parsed = eini_parse("\t [ Section3  ] \t");
  CU_ASSERT_EQUAL(parsed.type, EINI_SECTION);
  CU_ASSERT(0 == wcscmp(parsed.value, L"Section3"));

  parsed = eini_parse("[SectionIV]; comment");
  CU_ASSERT_EQUAL(parsed.type, EINI_SECTION);
  CU_ASSERT(0 == wcscmp(parsed.value, L"SectionIV"));

  parsed = eini_parse("key_1=an egg");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key_1"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"an egg"));

  parsed = eini_parse("\t  KeyTwo = another eggie  \t ");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"KeyTwo"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"another eggie"));

  parsed = eini_parse(
      "\t third_key =  ένα αυγουλάκι που in English το λένε eggie  \n");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"third_key"));
  CU_ASSERT(
      0 == wcscmp(parsed.value, L"ένα αυγουλάκι που in English το λένε eggie"));

  parsed = eini_parse("key4= \"ακόμη ένα eggie ή αυγό\"");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key4"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"ακόμη ένα eggie ή αυγό"));

  parsed =
      eini_parse("key5=ASCII specials are \\a \\b \\t \\n \\v \\f \\r and \\e");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key5"));
  CU_ASSERT(0 == wcscmp(parsed.value,
                        L"ASCII specials are \a \b \t \n \v \f \r and \e"));

  parsed = eini_parse("key_VI=Other specials: \\\\ \\' and \\\", naturally");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key_VI"));
  CU_ASSERT(0 ==
            wcscmp(parsed.value, L"Other specials: \\ ' and \", naturally"));

  parsed = eini_parse("se7en=\"in double \\\" quotes\\r\"");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"se7en"));
  CU_ASSERT(0 ==
            wcscmp(parsed.value, L"in double \" quotes\r"));

  parsed = eini_parse("se7enUp=  \'in single \\' quotes\\\\\'");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"se7enUp"));
  CU_ASSERT(0 ==
            wcscmp(parsed.value, L"in single \' quotes\\"));

  parsed = eini_parse("");
  CU_ASSERT_EQUAL(parsed.type, EINI_NONE);

  parsed = eini_parse("[garbled$ect_on]");
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"unable to parse"));

  parsed = eini_parse("κλειδί=value");
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"unable to parse"));

  parsed = eini_parse("key=\"value"); // "value
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"non-terminated quote"));

  parsed = eini_parse("key=\"value\\\""); // "value\"
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"non-terminated quote"));

  parsed = eini_parse("key=\"value\\\\\""); // "value\\"
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"value\\"));

  parsed = eini_parse("key=\"value\\\\\\\""); // "value\\\"
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"non-terminated quote"));

  parsed = eini_parse("key='value"); // 'value
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"non-terminated quote"));

  parsed = eini_parse("key='value\\'"); // 'value\'
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"non-terminated quote"));

  parsed = eini_parse("key='value\\\\'"); // 'value\\'
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"value\\"));

  parsed = eini_parse("key='value\\\\\\'"); // 'value\\\'
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"non-terminated quote"));

  parsed = eini_parse("key=value ; comment");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"value"));

  parsed = eini_parse("key='value' ; comment");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"value"));

  parsed = eini_parse("key=\"value ; comment\"");
  CU_ASSERT_EQUAL(parsed.type, EINI_VALUE);
  CU_ASSERT(0 == wcscmp(parsed.key, L"key"));
  CU_ASSERT(0 == wcscmp(parsed.value, L"value ; comment"));

  parsed = eini_parse("key='value ; comment");
  CU_ASSERT_EQUAL(parsed.type, EINI_ERROR);
  CU_ASSERT(0 == wcscmp(parsed.value, L"non-terminated quote"));

  eini_free();
}

// Where we hope it works
int main(int argc, char **argv) {
  init();
  init_cli();
  init_test_suite();

  // `add_test()` all your tests here
  add_test(eini_parse);

  run_tests_and_exit();
}
