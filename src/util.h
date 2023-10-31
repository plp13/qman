// Utility infrastructure, not program-specific (definition)

#ifndef UTIL_H

#define UTIL_H

#include "lib.h"

//
// Types
//

// Memory allocation type
typedef enum {
  AT_STACK, // in the stack
  AT_HEAP   // in the heap
} alloc_type_t;

// Array of bits
typedef char *bitarr_t;

// A full regular expression, i.e. one that is represented both as a string and
// a regex_t
typedef struct {
  char *str;  // string version
  regex_t re; // regex_t version
} full_regex_t;

// A range
typedef struct {
  unsigned beg; // beginning
  unsigned end; // end
} range_t;

//
// Constants
//

// Buffer sizes
#define BS_SHORT 128 // length of a short array
#define BS_LINE 1024 // length of an array that is suitable for a line of text

//
// Macros
//

// Swap two numerical values a and b (without using a third variable)
#define swap(a, b)                                                             \
  a = a ^ b;                                                                   \
  b = a ^ b;                                                                   \
  a = a ^ b;
// Return the size of array arr
#define asizeof(arr) (sizeof(arr) / sizeof(arr[0]))

// Allocate heap memory for an array of type artp that is len elements long
#define aalloc(len, artp) xcalloc(len, sizeof(artp));

// Allocate heap memory for a char* string that is len characters long
#define salloc(len) xcalloc(len + 1, sizeof(char))

// Allocate heap memory for a wchar_t* string that is len characters long
#define walloc(len) xcalloc(len + 1, sizeof(wchar_t))

// Allocate heap memory for a bit array bitarr_t that is len bits long
#define balloc(len) xcalloc(len % 8 == 0 ? len / 8 : 1 + len / 8, 1)

// Allocate stack memory for an array of type artp that is len elements long
#define aalloca(len, artp) alloca(len * sizeof(artp));

// Allocate stack memory for a char* string that is len characters long
#define salloca(len) alloca((len + 1) * sizeof(char))

// Allocate stack memory for a wchar_t* string that is len characters long
#define walloca(len) alloca((len + 1) * sizeof(wchar_t))

// Allocate stack memory for a bit array bitarr_t that is len bits long
#define balloca(len) alloca(len % 8 == 0 ? len / 8 : 1 + len / 8)

//
// Functions
//

// Many of these functions call winddown() to fail gracefully in case of error

// Perform the same function as perror() but, rather than printing the error
// message, place it in dst
void serror(wchar_t *dst, const wchar_t *s);

// The arguments of all x...() functions are identical to those of the standard
// functions they replace.

// Safely call calloc()
extern void *xcalloc(size_t nmemb, size_t size);

// Safely call reallocarray()
extern void *xreallocarray(void *ptr, size_t nmemb, size_t size);

// Safely call popen()
extern FILE *xpopen(const char *command, const char *type);

// Safely call pclose()
extern int xpclose(FILE *stream);

// Safely call fopen()
extern FILE *xfopen(const char *pathname, const char *mode);

// Safely call fclose()
extern int xfclose(FILE *stream);

// Safely call tmpfile()
extern FILE *xtmpfile();

// Safely call fgets()
extern char *xfgets(char *s, int size, FILE *stream);

// Safely call fwrite()
extern size_t xfwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

// Return the value of environment variable name as an integer. Return 0 in case
// of error.
extern int getenvi(const char *name);

// Return the value of the i'th bit in bit array ba
extern bool bget(bitarr_t ba, unsigned i);

// Set the the i'th bit of bit array ba
extern void bset(bitarr_t ba, unsigned i);

// Clear the the i'th bit of bit array ba
extern void bclear(bitarr_t ba, unsigned i);

// Clear all bits of ba. ba_len is ba's size.
extern void bclearall(bitarr_t ba, unsigned ba_len);

// Free all memory in an array of (wide) strings buf. buf_len is the length of
// buf.
extern void wafree(wchar_t **buf, unsigned buf_len);

// Free all memory in an array of (encoded) strings buf. buf_len is the length
// of buf.
extern void safree(char **buf, unsigned buf_len);

// All w...() and s...() functions that place their result in an argument don't
// do any memory allocation. Said argument must be a pointer to a buffer of
// already allocated memory.

// Return the number of uccurences of needle in hayst
extern unsigned wccnt(const wchar_t *hayst, wchar_t needle);

// Replace all occurences of needle in hayst with repl. Place the result in
// dst.
extern void wcrepl(wchar_t *dst, const wchar_t *hayst, wchar_t needle,
                   const wchar_t *repl);

// Insert newlines in trgt so that it word-wraps before it reaches cols columns.
// Each newline is inserted in the place of a space or tab.
extern void wwrap(wchar_t *trgt, unsigned cols);

// Return true if needle is in array of (wide) strings hayst, false otherwise.
// hayst_length is the length of hayst.
extern bool wmemberof(wchar_t *const *hayst, const wchar_t *needle,
                      unsigned hayst_len);

// Case-insensitive version of wmemberof()
extern bool wcasememberof(wchar_t *const *hayst, const wchar_t *needle,
                          unsigned hayst_len);

// Sort the strings in trgt alphanumerically. trgt_len is trgt's length. Setting
// rev to true causes reverse sorting.
extern void wsort(wchar_t **trgt, unsigned trgt_len, bool rev);

// Return the length of the longest (wide) string in src. src_len holds the
// length of src.
extern unsigned wmaxlen(wchar_t *const *src, unsigned src_len);

// Copy all data in source into trgt, line by line. Both source and target must
// be text files. Return the number of lines copied.
extern unsigned scopylines(FILE *source, FILE *target);

// Read a line from file fp, and place the result in str (without the trailing
// newline). If the read was succesful, return the resulting string's length.
// Otherwise, return -1.
extern int sreadline(char *str, unsigned size, FILE *fp);

// Initialize full regular expression re, using str
extern void fr_init(full_regex_t *re, char *str);

// Search src for a string matching re. If found, return its location in src as
// a range_t. If not, return { 0, 0 }. This function uses libc regular
// expressions, and has plumbing to make it work on wchar_t* strings.
extern range_t fr_search(const full_regex_t *re, wchar_t *src);

#endif
