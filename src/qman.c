// Main programs

#include "lib.h"

// Where the magic happens
int main(int argc, char **argv) {
  init();

  // Parse program options and arguments, and populate history
  int noopt_argc = parse_options(argc, argv);
  int clean_argc = argc - noopt_argc;
  char **clean_argv = &argv[noopt_argc];
  parse_args(clean_argc, clean_argv);

  toc_entry_t *my_toc;
  unsigned my_toc_len = toc(&my_toc, L"'virtualenv(1)'", false);
  for (unsigned i = 0; i < my_toc_len; i++) {
    if (TT_SUBHEAD == my_toc[i].type)
      printf("  ");
    else if (TT_TAGPAR == my_toc[i].type)
        printf("    ");
    printf("%ls\n", my_toc[i].text);
  }
  toc_free(my_toc, my_toc_len);
  winddown(ES_SUCCESS, NULL);

  // Run the main handler
  if (config.layout.tui)
    tui();
  else
    cli();

  winddown(ES_SUCCESS, NULL);
}
