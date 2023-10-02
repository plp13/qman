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
#define swap(a, b) a = a ^ b; b = a ^ b; a = a ^ b;
// Return the size of array arr
#define asizeof(arr) (sizeof(arr) / sizeof(arr[0]))

// Allocate heap memory for an array of type artp that is len elements long
#define aalloc(len, artp) xcalloc(len, sizeof(artp));

// Allocate heap memory for a char* string that is len characters long
#define salloc(len) xcalloc(len + 1, sizeof(char))

// Allocate heap memory for a wchar_t* string that is len characters long
#define walloc(len) xcalloc(len + 1, sizeof(wchar_t))

// Allocate heap memory for a bit array bitarr_t that is len bits long
#define balloc(len) xcalloc(len % 8 == 0 ? len / 8: 1 + len / 8, 1)

// Allocate stack memory for an array of type artp that is len elements long
#define aalloca(len, artp) alloca(len * sizeof(artp));

// Allocate stack memory for a char* string that is len characters long
#define salloca(len) alloca((len + 1) * sizeof(char))

// Allocate stack memory for a wchar_t* string that is len characters long
#define walloca(len) alloca((len + 1) * sizeof(wchar_t))

// Allocate stack memory for a bit array bitarr_t that is len bits long
#define balloca(len) alloca(len % 8 == 0 ? len / 8 : 1 + len / 8)

//
// Functions (comments are in util.c)
//

void serror(wchar_t *dst, const wchar_t *s);

extern void *xcalloc(size_t nmemb, size_t size);

extern void *xreallocarray(void *ptr, size_t nmemb, size_t size);

extern FILE *xpopen(const char *command, const char *type);

extern int xpclose(FILE *stream);

extern FILE *xfopen(const char *pathname, const char *mode);

extern int xfclose(FILE *stream);

extern FILE *xtmpfile();

extern bool bget(bitarr_t ba, unsigned i);

extern void bset(bitarr_t ba, unsigned i);

extern void bclear(bitarr_t ba, unsigned i);

extern void bclearall(bitarr_t ba, unsigned ba_len);

extern void wafree(wchar_t **buf, unsigned buf_len);

extern void safree(char **buf, unsigned buf_len);

extern unsigned wccnt(const wchar_t *hayst, wchar_t needle);

extern void wcrepl(wchar_t *dst, const wchar_t *hayst, wchar_t needle,
                   const wchar_t *repl);

extern void wwrap(wchar_t *trgt, unsigned cols);

extern bool wmemberof(wchar_t *const *hayst, const wchar_t *needle, unsigned needle_len);

extern void wsort(wchar_t **trgt, unsigned trgt_len, bool rev);

extern unsigned wmaxlen(wchar_t *const *src, unsigned src_len);

extern unsigned scopylines(FILE *source, FILE *target);

extern int sreadline(char *str, unsigned size, FILE *fp);

#endif
