// Command line interface (definition)

#ifndef CLI_H

#define CLI_H

#include "lib.h"
#include "program.h"

//
// Functions (utility)
//

// Initialize the terminal
extern void init_cli();

// Print the contents of lines (of length lines_len) to standard output
extern void print_page(line_t *lines, unsigned lines_len);

//
// Functions (handlers)
//

// Main handler for the CLI
extern void cli();

#endif

