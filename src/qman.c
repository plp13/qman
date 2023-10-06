// Main program

#include "lib.h"
#include "program.h"
#include "util.h"

// Where the magic happens
int main(int argc, char **argv) {
  init();

  int noopt_argc = parse_options(argc, argv);

  // man_argc and man_argv are our argc and argv with all program options
  // removed
  int man_argc = argc - noopt_argc;
  char **man_argv = &argv[noopt_argc];

  if (termsize_changed()) {
    unsigned lines_len = aprowhat(&lines, AW_APROPOS, "clear", L"APROPOS",
                                  L"Search Results for 'clear'");
    draw_page(lines, lines_len, 0);
    getch();
    lines_free(lines, lines_len);
  }

  winddown(ES_SUCCESS, NULL);
}
