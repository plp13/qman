// Main program

#include "lib.h"
#include "program.h"
#include "util.h"

// Where the magic happens
int main(int argc, char **argv) {
  init();

  int noopt_argc = parse_options(argc, argv);

  // man_argc and man_argv are our argc and argv with all program options
  // removed. We can now pass them on to the 'man' command.
  int man_argc = argc - noopt_argc;
  char **man_argv = &argv[noopt_argc];

  line_t *aw;
  unsigned aw_len = aprowhat(&aw, AW_APROPOS, "clear", L"APROPOS", L"Search Results for 'clear'");
  for (unsigned i = 0; i < aw_len; i++) {
    wprintf(L"%ls\n", aw[i].text);
  }
  lines_free(aw, aw_len);
  
  winddown(ES_SUCCESS, NULL);
}
