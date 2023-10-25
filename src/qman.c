// Main programs

#include "lib.h"
#include "program.h"
#include "tui.h"
#include "util.h"

// Where the magic happens
int main(int argc, char **argv) {
  init();

  // Parse program options and arguments, and populate history
  int noopt_argc = parse_options(argc, argv);
  int clean_argc = argc - noopt_argc;
  char **clean_argv = &argv[noopt_argc];
  parse_args(clean_argc, clean_argv);

  // init_tui();
  // termsize_changed();
  //
  // page_len = index_page(&page);
  //
  // init_windows();
  //
  // link_loc_t flink = {true, 0, 0};
  // flink = next_link(page, page_len, flink);
  // draw_page(page, page_len, 0, flink);
  // draw_sbar(page_len, 0);
  // draw_stat(L"INDEX", L"All Manual Pages", page_len, 0, L":",
  //           L"Press 'h' for help or 'q' to quit");
  //
  // line_t *manpage;
  // unsigned manpage_len = man(&manpage, "doupdate");
  // draw_page(manpage, manpage_len, 44);
  // draw_sbar(manpage_len, 44);
  // draw_stat(L"MAN", L"curs_refresh(3X)", manpage_len, 60, L":", L"Press 'h'
  // for help or 'q' to quit");
  //
  // doupdate();
  //
  // get_wch(NULL);
  //
  // lines_free(manpage, manpage_len);

  winddown(ES_SUCCESS, NULL);
}
