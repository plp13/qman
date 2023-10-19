// Main programs

#include "lib.h"
#include "program.h"
#include "tui.h"
#include "util.h"

// Where the magic happens
int main(int argc, char **argv) {
  init();
  init_tui();

  int noopt_argc = parse_options(argc, argv);

  // man_argc and man_argv are our argc and argv with all program options
  // removed
  int man_argc = argc - noopt_argc;
  char **man_argv = &argv[noopt_argc];

  termsize_changed();
  
  line_t *lines;
  unsigned lines_len = aprowhat(&lines, AW_APROPOS, "clear", L"APROPOS",
                                L"Search Results for 'clear'");

  init_windows();
  draw_page(lines, lines_len, 0);
  draw_sbar(lines_len, 0);
  draw_stat(lines_len, 5, L"Press 'h' for help or 'q' to quit", L":");
  doupdate();
  get_wch(NULL);
  
  lines_free(lines, lines_len);
  
  winddown(ES_SUCCESS, NULL);

  // termsize_changed();
  // printw("has_colors=%d, can_change_color=%d COLORS=%d, COLOR_PAIRS=%d\n\n",
  //        has_colors(), can_change_color(), COLORS, COLOR_PAIRS);
  // unsigned i;
  // for (i = 0; i < 128; i++) {
  //   unsigned y = 2 + i % (config.layout.height - 2);
  //   unsigned x = 12 * (i / (config.layout.height - 2));
  //   init_pair(i, i, 0);
  //   init_pair(128 + i, 0, i);
  //   attrset(0);
  //   mvprintw(y, x, "%02x:", i);
  //   attrset(COLOR_PAIR(i));
  //   printw("[fg]");
  //   attrset(COLOR_PAIR(128 + i));
  //   printw("[bg]");
  // }

}
