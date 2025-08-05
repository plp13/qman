// Command line interface (implementation)

#include "lib.h"

//
// Helper macros and functions
//

// Helper of `print_page()` that returns the current link
#define cur_link lines[ln].links[l]

// Helper of `print_page()`. Return a statically allocated string that contains
// a terminal escape sequence that matches the color of `link`.
wchar_t *link_escseq(link_t link) {
  switch (link.type) {
  case LT_MAN:
    return L"\e[1;32m";
    break;
  case LT_HTTP:
    return L"\e[1;35m";
    break;
  case LT_EMAIL:
    return L"\e[1;35m";
    break;
  case LT_FILE:
    return L"\e[1;34m";
    break;
  default:
  case LT_LS:
    return L"\e[1;33m";
    break;
  }
}

//
// Functions (generic)
//

void init_cli() {
  unsigned cols; // terminal width according to the environment

  // Set `cols` to the value indicated by the environment
  cols = getenvi("MANWIDTH");
  if (0 == cols)
    cols = getenvi("COLUMNS");

  // If unable to read anything from the environment, set `cols` using an
  // `ioctl()` call (or set it to 80 if that doesn't work either)
  if (0 == cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
      cols = ws.ws_col;
    else if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0)
      cols = ws.ws_col;
    else if (ioctl(STDERR_FILENO, TIOCGWINSZ, &ws) == 0)
      cols = ws.ws_col;
    else
      cols = 80;
  }

  config.layout.main_width = cols;
}

bool inside_term() {
  if (config.misc.cli_force_color)
    return true;
  else
    return isatty(STDOUT_FILENO);
}

void print_page(const line_t *lines, unsigned lines_len) {
  unsigned ln, c, l;    // current line number, character number, link number
  bool in_link = false; // current character is inside a link
  bool has_hyph_link =
      false;        // there's a hyphenated link from the previous line
  link_t hyph_link; // said hyphenated link
  wchar_t *reg_escseq =
      L""; // sequence to return from non-regular to regular text

  // For each line...
  for (ln = 0; ln < lines_len; ln++) {
    if (inside_term()) {
      // If inside a terminal, format the line's text using terminal escape
      // sequences

      // For each line character...
      l = 0;
      for (c = 0; lines[ln].text[c] != L'\0' && c < lines[ln].length; c++) {
        // Colorize text inside links
        if (has_hyph_link && c == hyph_link.start_next) {
          // Hyphenated link (from previous line) start
          in_link = true;
          fputws(link_escseq(hyph_link), stdout);
        } else if (has_hyph_link && c == hyph_link.end_next) {
          // Hyphenated link (from previous line) end
          in_link = false;
          has_hyph_link = false;
          fputws(L"\e[0;39m", stdout);
        } else if (l < lines[ln].links_length && c == cur_link.start) {
          // Link start
          in_link = true;
          fputws(link_escseq(cur_link), stdout);
        } else if (l < lines[ln].links_length && c == cur_link.end) {
          // Link end
          in_link = false;
          fputws(L"\e[0;39m", stdout);
          if (cur_link.in_next) {
            has_hyph_link = true;
            hyph_link = cur_link;
          }
          l++;
        }

        // For text that is outside links, make text
        // regular/bold/italic/underline as required
        if (!in_link) {
          if (bget(lines[ln].reg, c)) {
            // Regular
            fputws(reg_escseq, stdout);
            reg_escseq = L"";
          } else if (bget(lines[ln].bold, c)) {
            // Bold
            fputws(L"\e[1m", stdout);
            reg_escseq = L"\e[0m";
          } else if (bget(lines[ln].italic, c)) {
            // Italic
            fputws(L"\e[3m", stdout);
            reg_escseq = L"\e[23m";
          } else if (bget(lines[ln].uline, c)) {
            // Underline
            fputws(L"\e[4m", stdout);
            reg_escseq = L"\e[24m";
          }
        }

        // Print the character
        fputwc(lines[ln].text[c], stdout);
      }
    } else {
      // Otherwise, print the line's text without formatting

      fputws(lines[ln].text, stdout);
    }

    // At line end, restore text to regular and print a newline
    fputws(reg_escseq, stdout);
    fputwc(L'\n', stdout);
  }
}

//
// Functions (handlers)
//

// Main handler for the CLI
void cli() {
  configure();
  late_init();
  init_cli();

  populate_page();
  if (err)
    winddown(ES_NOT_FOUND, err_msg);

  print_page(page, page_len);
}
