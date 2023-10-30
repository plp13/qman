// Main programs

#include "lib.h"
#include "program.h"
#include "cli.h"

// Main handler/loop for the TUI
void tui() {}

// Main handler for CLI use
void cli() {
  init_cli();
  populate();
  print_page(page, page_len);
}

// Where the magic happens
int main(int argc, char **argv) {
  init();

  // Parse program options and arguments, and populate history
  int noopt_argc = parse_options(argc, argv);
  int clean_argc = argc - noopt_argc;
  char **clean_argv = &argv[noopt_argc];
  parse_args(clean_argc, clean_argv);

  // Run the main handler
  if (config.layout.tui)
    tui();
  else
    cli();

  winddown(ES_SUCCESS, NULL);
}
