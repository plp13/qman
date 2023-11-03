// Main programs

#include "cli.h"
#include "lib.h"
#include "program.h"
#include "tui.h"
#include <curses.h>
#include <wchar.h>

// Handler for PA_UP
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
    if (page[page_top].links_length > 0) {
      // Newly revealed line has links; focus on the last one
      page_flink =
          (link_loc_t){true, page_top, page[page_top].links_length - 1};
    }
  } else {
    // None of the above; beep
    cbeep();
  }
}

// Handler for PA_DOWN
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
    if (page[page_top + config.layout.main_height - 1].links_length > 0) {
      // Newly revealed line has links; focus on the first one
      page_flink =
          (link_loc_t){true, page_top + config.layout.main_height - 1, 0};
    }
  } else {
    // None of the above; beep
    cbeep();
  }
}

// Handler for PA_PGUP
void tui_pgup() {
  if (page_top >= config.layout.main_height) {
    // If there's space, scroll up one window height
    page_top -= config.layout.main_height;
  } else if (page_top > 0) {
    // If not, but we're still not at the very top, go there
    page_top = 0;
  } else {
    // None of the above; focus on page's first link, or beep if already there
    link_loc_t fl = first_link(page, page_len, page_top,
                               page_top + config.layout.main_height - 1);
    if (fl.ok && (page_flink.line != fl.line || page_flink.link != fl.link))
      page_flink = fl;
    else
      cbeep();
    return;
  }

  // Focus on the last link in new visible portion
  link_loc_t ll = last_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
  if (ll.ok)
    page_flink = ll;
}

// Handler for PA_PGDN
void tui_pgdn() {
  if (page_top + 2 * config.layout.main_height < page_len) {
    // If there's space, scroll down one window height
    page_top += config.layout.main_height;
  } else if (page_top + config.layout.main_height < page_len) {
    // If not, but we're still not at the very bottom, go there
    page_top = page_len - config.layout.main_height;
  } else {
    // None of the above; focus on page's last link, or beep if already there
    link_loc_t ll = last_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);
    if (ll.ok && (page_flink.line != ll.line || page_flink.link != ll.link))
      page_flink = ll;
    else
      cbeep();
    return;
  }

  // Focus on the first link in new visible portion
  link_loc_t fl = first_link(page, page_len, page_top,
                             page_top + config.layout.main_height - 1);
  if (fl.ok)
    page_flink = fl;
}

// Handler for PA_HOME
void tui_home() {
  // Go to the very top
  page_top = 0;

  // Focus on the first link in the visible portion
  link_loc_t fl = first_link(page, page_len, page_top,
                             page_top + config.layout.main_height - 1);
  if (fl.ok)
    page_flink = fl;
}

// Handler for PA_END
void tui_end() {
  // Go to the very bottom
  page_top = page_len - config.layout.main_height;

  // Focus on the last link in the visible portion
  link_loc_t ll = last_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
  if (ll.ok)
    page_flink = ll;
}

// Helper of tui_open(), tui_open_apropos() and tui_open_whatis(). If page_flink
// isn't valid, beep and return.
#define beep_on_invalid_flink                                                  \
  if (!page_flink.ok || page_flink.line < page_top ||                          \
      page_flink.line >= page_top + config.layout.main_height ||               \
      page_flink.line >= page_len ||                                           \
      page_flink.link >= page[page_flink.line].links_length) {                 \
    cbeep();                                                                   \
    return;                                                                    \
  }

// Handler for PA_OPEN
void tui_open() {
  wchar_t wtrgt[BS_SHORT];
  char strgt[BS_SHORT];

  beep_on_invalid_flink;

  // Open the link
  switch (page[page_flink.line].links[page_flink.link].type) {
  case LT_MAN:
    // The link is a manual page; add a new page request to show it
    swprintf(wtrgt, BS_SHORT, L"'%ls'",
             page[page_flink.line].links[page_flink.link].trgt);
    history_push(RT_MAN, wtrgt);
    populate_page();
    page_top = 0;
    page_flink = first_link(page, page_len, page_top,
                            page_top + config.layout.main_height - 1);
    break;
  case LT_HTTP:
    // The link is http(s); open it with the external web browser
    snprintf(strgt, BS_SHORT, "%s '%ls'", config.misc.browser_path,
             page[page_flink.line].links[page_flink.link].trgt);
    system(strgt);
    break;
  case LT_EMAIL:
    // The link is an email address; open it with the external mailer
    snprintf(strgt, BS_SHORT, "%s '%ls'", config.misc.mailer_path,
             page[page_flink.line].links[page_flink.link].trgt);
    system(strgt);
    break;
  case LT_LS:
    // TBD
    break;
  }
}

// Handler of PA_OPEN_APROPOS
void tui_open_apropos() {
  wchar_t wtrgt[BS_LINE];
  wchar_t *wtrgt_stripped, *buf;

  beep_on_invalid_flink;

  if (LT_MAN == page[page_flink.line].links[page_flink.link].type) {
    wcscpy(wtrgt, page[page_flink.line].links[page_flink.link].trgt);
    wtrgt_stripped = wcstok(wtrgt, L"()", &buf);

    if (NULL != wtrgt_stripped) {
      history_push(RT_APROPOS, wtrgt);
      populate_page();
      page_top = 0;
      page_flink = first_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);

      return;
    }
  }

  cbeep();
}

// Handler of PA_OPEN_WHATIS
void tui_open_whatis() {
  wchar_t wtrgt[BS_LINE];
  wchar_t *wtrgt_stripped, *buf;

  beep_on_invalid_flink;

  if (LT_MAN == page[page_flink.line].links[page_flink.link].type) {
    wcscpy(wtrgt, page[page_flink.line].links[page_flink.link].trgt);
    wtrgt_stripped = wcstok(wtrgt, L"()", &buf);

    if (NULL != wtrgt_stripped) {
      history_push(RT_WHATIS, wtrgt);
      populate_page();
      page_top = 0;
      page_flink = first_link(page, page_len, page_top,
                              page_top + config.layout.main_height - 1);

      return;
    }
  }

  cbeep();
}

// Handler for PA_INDEX
void tui_index() {
  history_cur++;
  if (history_top < history_cur)
    history_top = history_cur;
  history[history_cur].request_type = RT_INDEX;
  populate_page();
  page_top = 0;
  page_flink = first_link(page, page_len, page_top,
                          page_top + config.layout.main_height - 1);
}

// Handler for PA_BACK
void tui_back() {
  if (!history_back(1))
    cbeep();
  populate_page();
  page_top = 0;
  page_flink = first_link(page, page_len, page_top,
                          page_top + config.layout.main_height - 1);
}

// Handler for PA_FWRD
void tui_fwrd() {
  if (!history_forward(1))
    cbeep();
  populate_page();
  page_top = 0;
  page_flink = first_link(page, page_len, page_top,
                          page_top + config.layout.main_height - 1);
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
                page_len, pos + 1, L":", L"Press 'h' for help or 'q' to quit");
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
    case PA_PGUP:
      tui_pgup();
      redraw = true;
      break;
    case PA_PGDN:
      tui_pgdn();
      redraw = true;
      break;
    case PA_HOME:
      tui_home();
      redraw = true;
      break;
    case PA_END:
      tui_end();
      redraw = true;
      break;
    case PA_OPEN:
      tui_open();
      redraw = true;
      break;
    case PA_OPEN_APROPOS:
      tui_open_apropos();
      redraw = true;
      break;
    case PA_OPEN_WHATIS:
      tui_open_whatis();
      redraw = true;
      break;
    case PA_INDEX:
      tui_index();
      redraw = true;
      break;
    case PA_BACK:
      tui_back();
      redraw = true;
      break;
    case PA_FWRD:
      tui_fwrd();
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
