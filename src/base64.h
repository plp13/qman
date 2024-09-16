// Base64 encoding and decoding (definition)

#ifndef BASE64_H

#define BASE64_H

#include <lib.h>

//
// Global variables
//

// Encoding table
extern char base64_enct[];

// Decoding table
extern char *base64_dect;

// Modulo table; helper for base64_encode()
extern int base64_modt[];

//
// Functions
// 

// Return the base64-encoded version of data in a freshly allocated buffer, and
// put the size of said buffer in output_length
extern char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);

// Return the base64-decoded version of data in a freshly allocated buffer, and
// put the size of said buffer in output_length
extern unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length);

// Allocate memory for and populate base64_dect
extern void base64_build_dect();

// Deallocate memory used by base64_dect
extern void base64_cleanup();

#endif
