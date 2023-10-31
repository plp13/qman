// Main programs

#include "lib.h"
#include "program.h"
#include "cli.h"
#include "tui.h"

void tui_up() {
  link_loc_t pl =
      prev_link(page, page_len, page_flink); // link right before page_flink

  if (pl.ok && pl.line >= page_top &&
      pl.line < page_top + config.layout.main_height) {
    // pl exists and is in visible portion; focus on pl
    page_flink = pl;
  } else if (page_top > 0) {
    // Visible portion isn't already at the beginning of page; scroll up one
    // line
    page_top--;
    if (pl.ok && pl.line == page_top && page_flink.line != page_top) {
      // pl is in newly revealed line (and page_flink isn't); focus on pl
      page_flink = pl;
    }
  } else {
    // None of the above; beep
    cbeep();
  }
}

void tui_down() {
  link_loc_t nl =
      next_link(page, page_len, page_flink); // link right after page_flink

  if (nl.ok && nl.line >= page_top &&
      nl.line < page_top + config.layout.main_height) {
    // nl exists and is in visible portion; focus on nl
    page_flink = nl;
  } else if (page_top + config.layout.main_height < page_len) {
    // Visible portion isn't at the very end of page; scroll down one line
    page_top++;
    if (nl.ok && nl.line == page_top + config.layout.main_height - 1 &&
        page_flink.line != page_top + config.layout.main_height) {
      // nl is in newly revealed line (and page_flink isn't); focus on nl
      page_flink = nl;
    }
  } else {
    // None of the above; beep
    cbeep();
  }
}

// Main handler/loop for the TUI
void tui() {
  int input;          // keyboard/mouse input from user
  bool redraw = true; // set this to true to redraw the screen

  // Initialize TUI
  init_tui();
  termsize_changed();
  init_windows();

  // Initialize content, position, and focus
  populate_page();
  page_top = 0;
  page_left = 0;
  page_flink = next_link(page, page_len, page_flink);

  // Initialize action
  action = PA_NULL;

  while (PA_QUIT != action) {
    // If terminal size has changed, regenerate page and ask for a redraw
    if (termsize_changed()) {
      init_windows();
      populate_page();
      redraw = true;
    }

    // If redraw is necessary, redraw
    if (redraw) {
      unsigned pos = page_flink.line;
      if (pos < page_top || pos >= page_top + config.layout.main_height)
        pos = page_top;

      draw_page(page, page_len, page_top, page_flink);
      draw_sbar(page_len, page_top);
      draw_stat(request_type_str(history[history_cur].request_type), page_title,
                page_len, pos + 1, L":",
                L"Press 'h' for help or 'q' to quit");
      doupdate();

      redraw = false;
    }

    // Get user input
    input = getch();
    action = get_action(input);

    // Perform the requested action
    switch (action) {
    case PA_UP:
      tui_up();
      redraw = true;
      break;
    case PA_DOWN:
      tui_down();
      redraw = true;
      break;
    case PA_QUIT:
      break;
    default:
      cbeep();
      break;
    }
  }
}

// Main handler for CLI use
void cli() {
  init_cli();

  populate_page();

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
