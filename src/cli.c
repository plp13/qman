// Command line interface (implementation)

#include "lib.h"

//
// Functions (utility)
//

void init_cli() {
  unsigned cols; // terminal width according to the environment

  // Set config.layout.main_width to the value indicated by the environment
  cols = getenvi("MANWIDTH");
  if (0 == cols)
    cols = getenvi("COLUMNS");
  if (0 != cols)
    config.layout.main_width = cols;
  else {
    // Can't read anything from the environment; use ncurses to set main_width
    newterm(NULL, stderr, stdin);
    config.layout.main_width = getmaxx(stdscr);
    endwin();
  }
}

void print_page(line_t *lines, unsigned lines_len) {
  unsigned i, j; // iterators
  wchar_t *reg =
      L""; // sequence to return from bold/italic/underline to regular text

  // For each line...
  for (i = 0; i < lines_len; i++) {
    // For each line character...
    for (j = 0; lines[i].text[j] != L'\0' && j < lines[i].length; j++) {
      // Set text attributes to regular/bold/italic/underline, if necessary
      if (bget(lines[i].reg, j)) {
        fputws(reg, stdout);
        reg = L"";
      } else if (bget(lines[i].bold, j)) {
        fputws(L"\e[1m", stdout);
        reg = L"\e[0m";
      } else if (bget(lines[i].italic, j)) {
        fputws(L"\e[3m", stdout);
        reg = L"\e[23m";
      } else if (bget(lines[i].uline, j)) {
        fputws(L"\e[4m", stdout);
        reg = L"\e[24m";
      }

      // Print the character
      fputwc(lines[i].text[j], stdout);
    }

    // At line end, restore text to regular and print a newline
    fputws(reg, stdout);
    fputwc(L'\n', stdout);
  }
}

//
// Functions (handlers)
//

// Main handler for the CLI
void cli() {
  init_cli();

  populate_page();
  if (err)
    winddown(ES_NOT_FOUND, err_msg);

  print_page(page, page_len);
}
