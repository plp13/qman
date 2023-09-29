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

  aprowhat_t *aw;
  unsigned aw_len = aprowhat(&aw, AW_APROPOS, "''");
  for (unsigned i = 0; i < aw_len; i++) {
    // wprintf(L"page=%ls\n", aw[i].page);
    // wprintf(L"page=%ls\n", aw[i].section);
    // wprintf(L"page=%ls\n\n", aw[i].descr);
    i++;
  }
  // wprintf(L"Total lines: %d\n", aw_len);
  wchar_t **sc;
  unsigned sc_len = aprowhat_sections(&sc, aw, aw_len);
  // wprintf(L"%d sections: ", sc_len);
  for (unsigned i = 0; i < sc_len; i++) {
    // wprintf(L"%ls ", sc[i]);
  }
  // wprintf(L"\n");
  line_t *lines;
  unsigned lines_len;
  lines_len = aprowhat_render(&lines, aw, aw_len, sc, sc_len, L"INDEX", L"Manual Pages Index", L"qman version 0.0", L"September 2023");
  for (unsigned i = 0; i < lines_len; i++)
    wprintf(L"%ls\n", lines[i].text);
  aprowhat_free(aw, aw_len);
  wafree(sc, sc_len);
  lines_free(lines, lines_len);

  bitarr_t ba = balloc(32);
  bset(ba, 2);
  bclear(ba, 12);
  bset(ba, 29);
  bclear(ba, 0);
  assert(bget(ba, 2));
  assert(!bget(ba, 12));
  assert(bget(ba, 29));
  assert(!bget(ba, 0));
  bset(ba, 0);
  assert(bget(ba, 0));
  bclearall(ba, 32);
  assert(!bget(ba, 2));
  assert(!bget(ba, 12));
  assert(!bget(ba, 29));
  assert(!bget(ba, 0));
  free(ba);

  winddown(ES_SUCCESS, NULL);
}
