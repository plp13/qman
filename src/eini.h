// Extended INI file parsing (definition)

#ifndef EINI_H

#define EINI_H

#include <lib.h>

//
// Types
//

// INI line type
typedef enum {
  EINI_ERROR,   // parse error
  EINI_NONE,    // line is empty
  EINI_INCLUDE, // line contains an include directive
  EINI_SECTION, // line contains a section header
  EINI_VALUE    // line contains a key/value pair
} eini_type_t;

// Contents of an INI line
typedef struct {
  eini_type_t type; // line type
  wchar_t *key;     // key name if type is `EINI_VALUE`, NULL otherwise
  wchar_t
      *value; // error message if type is `EINI_ERROR`, file to include if
              // type is `EINI_INCLUDE`, section name if type is `EINI_SECTION`,
              // value if type is `EINI_VALUE`, NULL otherwise
} eini_t;

// Handler function
typedef void (*eini_handler_t)(const wchar_t *section, // current section
                               const wchar_t *key,     // key name
                               const wchar_t *value,   // value
                               const char *path,       // config file path
                               const unsigned line     // config file line
);

typedef void (*eini_error_t)(const wchar_t *error, // error message
                             const char *path,     // config file path
                             const unsigned line   // config file line
);

//
// Global variables
//

// Regular expressions for...
extern regex_t eini_re_include, // an include directive
    eini_re_section,            // a section header
    eini_re_value;              // a key/value pair

//
// Functions
//

// Initialize the parser
extern void eini_init();

// Parse INI line `src` and return its contents
extern eini_t eini_parse(char *src);

// Parse INI file at `path`, calling `hf` whenever a configuration directive
// is found or `ef` if a parse error is encountered. This function recursively
// calls itself whenever it encounters an `include` directive.
extern void eini(eini_handler_t hf, eini_error_t ef, const char *path);

// Wind down the parser
extern void eini_winddown();

#endif
