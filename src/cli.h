// Command line interface (definition)

#ifndef CLI_H

#define CLI_H

#include "lib.h"

//
// Functions (generic)
//

// Initialize the CLI. Currently, this function just snifs the number of columns
// into `config.layout.main_width`.
extern void init_cli();

// If `config.misc.cli_force_color` is true, return true. Otherwise, return the
// return value of `isatty()`.
extern bool inside_term();

// Print the contents of `lines` (of length `lines_len`) to standard output
extern void print_page(const line_t *lines, unsigned lines_len);

//
// Functions (handlers)
//

// Main handler for the CLI
extern void cli();

#endif
