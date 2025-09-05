// Utility infrastructure, not program-specific (definition)

#ifndef UTIL_H

#define UTIL_H

#include "lib.h"

//
// Compiler magic
//

// Macros used for silencing compiler warnings
#ifdef __GNUC__
#define CC_IGNORE_UNUSED_PARAMETER                                             \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")
#define CC_IGNORE_FORMAT_TRUNCATION                                            \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma("GCC diagnostic ignored \"-Wformat-truncation\"")
#define CC_IGNORE_ENDS _Pragma("GCC diagnostic pop")
#else
#define CC_IGNORE_UNUSED_PARAMETER
#define CC_IGNORE_FORMAT_TRUNCATION
#define CC_IGNORE_ENDS
#endif

//
// Types
//

// Array of bits
typedef char *bitarr_t;

// A full regular expression, i.e. one that is represented both as a string and
// a `regex_t`
typedef struct {
  char *str;     // string version
  regex_t re;    // `regex_t` version
  wchar_t *snpt; // a snippet of text that's always contained in matches (used
                 // to improve performance, as `regexec()` is quite expensive)
} full_regex_t;

// A range
typedef struct {
  unsigned beg; // beginning
  unsigned end; // end
} range_t;

// Compressed archive type
typedef enum {
  AR_NONE,  // no compression
  AR_GZIP,  // gzip
  AR_BZIP2, // bzip2
  AR_LZMA   // xz
} archive_type_t;

// A "fat" file pointer to a compressed archive, that supports multiple
// compression types
typedef struct {
  archive_type_t type; // archive type
  char *path;          // path to / filename of archive
  FILE *fp_none;       // file pointer if uncompressed
#ifdef QMAN_GZIP
  gzFile fp_gzip; // file pointer if gzip
#endif
  FILE *fp_bzip2; // file pointer if bzip2
  FILE *fp_lzma;  // file pointer if xz
} archive_t;

//
// Constants
//

// Buffer sizes
#define BS_SHORT 128   // length of a short array
#define BS_LINE 1024   // length of an array that is suitable for a line of text
#define BS_LONG 131072 // length of a long array

// Rudimentary logging, used for debugging
#define F_LOG "./qman.log" // log file

//
// Macros
//

// Swap two integer values `a` and `b` (without using a third variable)
#define swap(a, b)                                                             \
  a = a ^ b;                                                                   \
  b = a ^ b;                                                                   \
  a = a ^ b;

// If you get spurious `gcc` warnings about NULL string arguments being passed
// to functions that require them to be non-NULL, wrap said strings in `nnl()`
// or `wnnl()` to suppress them

// If string `s` is NULL, replace it with ""
#define nnl(s) ((s) ? (s) : "")

// If wide string `w` is NULL, replace it with ""
#define wnnl(w) (NULL != (w) ? (w) : L"")

// Return the size of array `arr`
#define asizeof(arr) (sizeof(arr) / sizeof(arr[0]))

// Allocate heap memory for an array of type `artp` that is `len` elements long
#define aalloc(len, artp) xcalloc(len, sizeof(artp));

// Allocate heap memory for a `char*` string that is `len` characters long
#define salloc(len) xcalloc(len + 1, sizeof(char))

// Allocate heap memory for a `wchar_t*` string that is `len` characters long
#define walloc(len) xcalloc(len + 1, sizeof(wchar_t))

// Allocate heap memory for a bit array `bitarr_t` that is `len` bits long
#define balloc(len) xcalloc(len % 8 == 0 ? len / 8 : 1 + len / 8, 1)

// Allocate stack memory for an array of type `artp` that is `len` elements long
#define aalloca(len, artp) alloca(len * sizeof(artp));

// Allocate stack memory for a `char*` string that is `len` characters long
#define salloca(len) alloca((len + 1) * sizeof(char))

// Allocate stack memory for a `wchar_t*` string that is `len` characters long
#define walloca(len) alloca((len + 1) * sizeof(wchar_t))

// Allocate stack memory for a bit array `bitarr_t` that is `len` bits long
#define balloca(len) alloca(len % 8 == 0 ? len / 8 : 1 + len / 8)

// Assign the value `{ v0, v1, ..., v7 }` to 8-value array `trgt`
#define arr8(trgt, v0, v1, v2, v3, v4, v5, v6, v7)                             \
  trgt[0] = v0;                                                                \
  trgt[1] = v1;                                                                \
  trgt[2] = v2;                                                                \
  trgt[3] = v3;                                                                \
  trgt[4] = v4;                                                                \
  trgt[5] = v5;                                                                \
  trgt[6] = v6;                                                                \
  trgt[7] = v7;

// True if `v` is in array `va` (of length 8), false otherwise. `f` is the
// comparison function/macro used to compare `v` with members of `va`.
#define in8(v, va, f)                                                          \
  (f(v, va[0]) || f(v, va[1]) || f(v, va[2]) || f(v, va[3]) || f(v, va[4]) ||  \
   f(v, va[5]) || f(v, va[6]) || f(v, va[7]))

// True if wide strings `w1` and `w2` are equal, false otherwise
#define wcsequal(w1, w2)                                                       \
  ((NULL == w1 && NULL == w2) ||                                               \
   (NULL != w1 && NULL != w2 && 0 == wcscmp(w1, w2)))

// Make wide string `w` lower-case
#define wcslower(w)                                                            \
  {                                                                            \
    unsigned i = 0;                                                            \
    while (L'\0' != w[i]) {                                                    \
      w[i] = towlower(w[i]);                                                   \
      i++;                                                                     \
    }                                                                          \
  }

// Log a message, together with a timestamp, into `F_LOG`. Use this function
// like you would `printf()`, i.e. specifying a template followed by zero or
// more values. To be used only temporarily for debugging, not in production.
#define logprintf(...)                                                         \
  {                                                                            \
    char ___[64 * BS_LINE];                                                    \
    sprintf(___, __VA_ARGS__);                                                 \
    loggit(___);                                                               \
  }

//
// Functions
//

// `x...()` functions, and also some other functions, will call `winddown()` to
// fail gracefully in case of error

// Perform the same function as `perror()` but, rather than printing the error
// message, place it in `dst`
void serror(wchar_t *dst, const wchar_t *s);

// Fail and `winddown()` if `path` doesn't point to an executable file
void is_executable(const char *path);

// Fail and `winddown()` if `path` doesn't point to a readable file
void is_readable(const char *path);

// The purpose of all of all `x...()` functions is to fail gracefully using
// `winddown()` whenever an error is detected. Otherwise, their behavior is
// identical to that of the standard functions they replace.

// Safely call `calloc()`
extern void *xcalloc(size_t nmemb, size_t size);

// Safely call `reallocarray()` if it exists, or our own local implementation if
// it doesn't
extern void *xreallocarray(void *ptr, size_t nmemb, size_t size);

// Safely call `popen()`
extern FILE *xpopen(const char *command, const char *type);

// Safely call `pclose()`
extern int xpclose(FILE *stream);

// Safely call `gzopen()`
#ifdef QMAN_GZIP
extern gzFile xgzopen(const char *path, const char *mode);
#endif

// Safely call `gzclose()`
#ifdef QMAN_GZIP
extern int xgzclose(gzFile file);
#endif

// Safely call `fopen()`
extern FILE *xfopen(const char *pathname, const char *mode);

// Safely call `fclose()`
extern int xfclose(FILE *stream);

// Safely call `tmpfile()`
extern FILE *xtmpfile();

// Safely call `gzgets()`
#ifdef QMAN_GZIP
extern char *xgzgets(gzFile file, char *buf, int len);
#endif

// Safely call `fgets()`
extern char *xfgets(char *s, int size, FILE *stream);

// Safely call `fputs()`
extern int xfputs(const char *s, FILE *stream);

// Safely call `fread()`
extern size_t xfread(void *ptr, size_t size, size_t nmemb, FILE *stream);

// Safely call `fwrite()`
extern size_t xfwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

// `xbasename()` and `xdirname()` use a static string to guarantee that `path`
// doesn't get modified

// Safely call `basename()`
extern char *xbasename(const char *path);

// Safely call `dirname()`
extern char *xdirname(const char *path);

// Safely call `strdup()`
extern char *xstrdup(const char *s);

// Safely call `wcsdup()`
extern wchar_t *xwcsdup(const wchar_t *s);

// `xwcstombs()` and `xmbstowcs()` will always terminate the string in `dest`,
// even when the length of `n` is exceeded. If you don't want this (e.g. because
// you're converting/copying parts of strings), use their vanilla counterparts
// instead.

// Safely call `wcstombs()`
extern size_t xwcstombs(char *dest, const wchar_t *src, size_t n);

// Safely call `mbstowcs()`
size_t xmbstowcs(wchar_t *dest, const char *src, size_t n);

// Safely call `strcasestr()` if it exists, or our own local implementation if
// it doesn't
extern char *xstrcasestr(const char *haystack, const char *needle);

// Safely call `system(cmd)`, to execute `cmd` in a new shell. If `fail` is
// true, and the return value of `system()` is non-zero, terminate. Otherwise
// return said return value.
extern int xsystem(const char *cmd, bool fail);

// A safe version of `tempnam()`, that also creates the temporary file whose
// name it returns, avoiding potential race conditions. Unlike with `tempnam()`,
// `pfx` can be more than 5 characters long, although it cannot contain an `X`.
// In case of error, `xtempnam()` will call `winddown()`.
extern char *xtempnam(const char *dir, const char *pfx);

// Return the value of environment variable `name` as an integer. Return 0 in
// case of error.
extern int getenvi(const char *name);

// Return the value of the `i`th bit in `ba`
extern bool bget(const bitarr_t ba, unsigned i);

// Set the the `i`th bit of `ba`
extern void bset(bitarr_t ba, unsigned i);

// Clear the the `i`th bit of `ba`
extern void bclear(bitarr_t ba, unsigned i);

// Clear all bits of `ba`. `ba_len` is `ba`'s size.
extern void bclearall(bitarr_t ba, unsigned ba_len);

// Decompress the bzip2-compressed file at `pathname`, place the resulting data
// into a temporary file, and return its path
extern char *bzip2_decompress(const char *pathname);

// Decompress the xz-compressed file at `pathname`, place the resulting data
// into a temporary file, and return its path
extern char *lzma_decompress(const char *pathname);

// Open compressed archive at `pathname` for reading, and return the relevant
// "fat" file pointer
extern archive_t aropen(const char *pathname);

// Read a line of text from "fat" file pointer `ap`, and place it into `buf`,
// `len` being the length of `buf`
extern void argets(archive_t ap, char *buf, int len);

// Return true if "fat" file pointer `ap` has reached `EOF`, false otherwise
extern bool areof(archive_t ap);

// Close "fat" pointer `ap`
extern void arclose(archive_t ap);

// Free all memory in an array of (wide) strings `buf`. `buf_len` is the length
// of `buf`.
extern void wafree(wchar_t **buf, unsigned buf_len);

// Free all memory in an array of (8-bit) strings `buf`. `buf_len` is the length
// of `buf`.
extern void safree(char **buf, unsigned buf_len);

// All `w...()` and `s...()` functions that place their result in an argument
// don't do any memory allocation. Said argument must be a pointer to a buffer
// of already allocated memory.

// Test whether the character at `src[pos]` is escaped
extern bool wescaped(wchar_t *src, unsigned pos);

// Unescape the string in `src`. `\a`, `\b`, `\t`, `\n`, `\v`, `\f`, and `\r`
// are unescaped into character codes 7 to 13, `\e` to character code 27 (ESC),
// `\\` to `\`, `\"` to `"`, and `\'` to `'`. All other escaped characters are
// discarded.
extern void wunescape(wchar_t *src);

// Return the number of occurences of `needle` in `hayst`
extern unsigned wccnt(const wchar_t *hayst, wchar_t needle);

// Replace all occurences of `needle` in `hayst` with `repl`. Place the result
// in `dst` (of size `dst_len`).
extern void wcrepl(wchar_t *dst, const wchar_t *hayst, wchar_t needle,
                   const wchar_t *repl, unsigned dst_len);

// Insert newlines in `trgt` so that it word-wraps before it reaches `cols`
// columns.
extern void wwrap(wchar_t *trgt, unsigned cols);

// Return true if `needle` is in array of (wide) strings `hayst`, false
// otherwise. `hayst_length` is the length of `hayst`.
extern bool wmemberof(const wchar_t *const *hayst, const wchar_t *needle,
                      unsigned hayst_len);

// Case-insensitive version of `wmemberof()`
extern bool wcasememberof(const wchar_t *const *hayst, const wchar_t *needle,
                          unsigned hayst_len);

// Sort the strings in `trgt` alphanumerically. `trgt_len` is `trgt`'s length.
// Setting `rev` to true causes reverse sorting.
extern void wsort(wchar_t **trgt, unsigned trgt_len, bool rev);

// Return the length of the longest (wide) string in `src`. `src_len` holds the
// length of `src`.
extern unsigned wmaxlen(const wchar_t *const *src, unsigned src_len);

// In the following functions, `extras` is ignored if NULL

// Split `src` into a list of words, and place said list in `dst` (of maximum
// length `dst_len`). Words can be separated by either whitespace or any of the
// characters in `extras` (only by those in `extras` if `skipws` is true).
// Return the number of words. This function modifies `src`.
extern unsigned wsplit(wchar_t ***dst, unsigned dst_len, wchar_t *src,
                       const wchar_t *extras, bool skipws);

// Return the position of the first character in `src` that is not whitespace,
// and not one of the characters in `extras`
extern unsigned wmargend(const wchar_t *src, const wchar_t *extras);

// Trim all characters at the end of `trgt` that are either whitespace or one of
// the charactes in `extras`. Trimming is done by inserting 0 or more NULL
// characters at the end of `trgt`. Return the new length of `trgt`.
extern unsigned wmargtrim(wchar_t *trgt, const wchar_t *extras);

// Apply any backspace characters in `trgt`
extern unsigned wbs(wchar_t *trgt);

// Case-insensitive version of `wcsstr()`
extern wchar_t *wcscasestr(const wchar_t *haystack, const wchar_t *needle);

// Copy all data in `source` into `target`, line by line. Both `source` and
// `target` must be text files. Return the number of lines copied.
extern unsigned scopylines(FILE *source, FILE *trgt);

// Read a line from file `fp`, and place the result in `str` (without the
// trailing newline). `size` signifies the maximum number of characters to read.
// If the read was succesful, return the resulting string's length. In case of
// EOF, return -1.
extern int sreadline(char *str, unsigned size, FILE *fp);

// Split path environment variable `src` into a list of paths, placing them into
// `dst` (of maximum length `dst_len`). Return the number of paths found. This
// function modifes `src`.
extern unsigned split_path(char ***dst, char *src);

// Initialize full regular expression `re`, using `str` and `snpt`
extern void fr_init(full_regex_t *re, char *str, wchar_t *snpt);

// Search `src` for a string matching `re`. If found, return its location in
// `src` as a `range_t`. If not, return `{ 0, 0 }`. This function uses libc
// regular expressions, and has plumbing to make it work on `wchar_t*` strings.
extern range_t fr_search(const full_regex_t *re, const wchar_t *src);

// Log `msg`, together with a timestamp, into `F_LOG`. Use this function only
// temporarily for debugging, not in production.
extern void loggit(const char *msg);

#endif
