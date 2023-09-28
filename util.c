// Utility infrastructure, not program-specific (implementation)

#include "util.h"
#include "lib.h"
#include "program.h"

//
// Functions
//

// Many of these functions call winddown() to fail gracefully in case of error

// Perform the same function as perror() but, rather than printing the error
// message, place a pointer to a string that contains it in dst
void serror(wchar_t *dst, const wchar_t *s) {
  if (NULL != s && L'\0' != s[0])
    swprintf(dst, BS_SHORT, L"%ls: %s", s, strerror(errno));
  else
    swprintf(dst, BS_SHORT, L"%s", strerror(errno));
}

// The arguments of all x...() functions are identical to those of the standard
// functions they replace.

// Safely call calloc()
void *xcalloc(size_t nmemb, size_t size) {
  void *ptr = calloc(nmemb, size);

  if (NULL == ptr) {
    wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to calloc()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return ptr;
}

// Safely call popen()
FILE *xpopen(const char *command, const char *type) {
  FILE *pipe = popen(command, type);

  if (NULL == pipe) {
    wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to popen()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return pipe;
}

// Safely call pclose()
int xpclose(FILE *stream) {
  int status = pclose(stream);

  if (-1 == status) {
    wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to pclose()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return status;
}

// Safely call fopen()
FILE *xfopen(const char *pathname, const char *mode) {
  FILE *file = fopen(pathname, mode);

  if (NULL == file) {
    wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to fopen()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return file;
}

// Safely call fclose()
int xfclose(FILE *stream) {
  int status = fclose(stream);

  if (EOF == status) {
    wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to fclose()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return status;
}

// Safely call tmpfile()
FILE *xtmpfile() {
  FILE *file = tmpfile();

  if (NULL == file) {
    wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to tmpfile()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return file;
}

// Return the value of the i'th bit in bit array ba
bool bget(bitarr_t ba, unsigned i) {
  unsigned seg = i / 8;
  unsigned mask = 1 << (i % 8);

  return ba[seg] & mask;
}

// Set the the i'th bit in bit array ba
void bset(bitarr_t ba, unsigned i) {
  unsigned seg = i / 8;
  unsigned mask = 1 << (i % 8);

    ba[seg] = ba[seg] | mask;
}

// Set the the i'th bit in bit array ba
void bclear(bitarr_t ba, unsigned i) {
  unsigned seg = i / 8;
  unsigned mask = 1 << (i % 8);

    ba[seg] = ba[seg] & (0xff - mask);
}

// Set all bits of bit array ba to 0. ba_len is ba's size.
void bclearall(bitarr_t ba, unsigned ba_len) {
  unsigned ba_bytes = ba_len % 8 == 0 ? ba_len / 8 : 1 + ba_len / 8;
  
  for (unsigned i = 0; i < ba_bytes; i++)
    ba[i] = 0;
}

// Free all memory in an array of (wide) strings
void wafree(wchar_t **buf, unsigned buf_len) {
  unsigned i;

  for (i = 0; i < buf_len; i++)
    free(buf[i]);

  free(buf);
}

// Free all memory in an array of (encoded) strings
void safree(char **buf, unsigned buf_len) {
  unsigned i;

  for (i = 0; i < buf_len; i++)
    free(buf[i]);

  free(buf);
}

// All w...() and s...() functions that place their result in an argument don't
// do any memory allocation. Said argument must be a pointer to a buffer of
// already allocated memory.

// Return the number of uccurences of needle in hayst
unsigned wccnt(const wchar_t *hayst, wchar_t needle) {
  unsigned cnt = 0;

  hayst = wcschr(hayst, needle);
  while (NULL != hayst) {
    cnt++;
    hayst = wcschr(hayst + 1, needle);
  }

  return cnt;
}

// Replace all occurences of needle in hayst with repl. Place the result in
// dst.
void wcrepl(wchar_t *dst, const wchar_t *hayst, wchar_t needle,
            const wchar_t *repl) {
  wchar_t *hayst_start = (wchar_t *)hayst;
  unsigned offset = 0, repl_cnt = 0, repl_len = wcslen(repl);

  wcscpy(dst, hayst);

  do {
    if (offset == 0)
      hayst = wcschr(hayst, needle);
    else {
      wcscpy(&dst[offset], repl);
      wcscpy(&dst[offset + repl_len], hayst + 1);
      repl_cnt++;
      hayst = wcschr(hayst + 1, needle);
    }
    offset = hayst - hayst_start + repl_cnt * (repl_len - 1);
  } while (NULL != hayst);
}

// Insert newlines in trgt so that it word-wraps before it reaches cols columns.
// Each newline is inserted in the place of a space or tab.
void wwrap(wchar_t *trgt, unsigned cols) {
  unsigned len = wcslen(trgt), line = 0, line_start = 0, line_end;

  while (len > line_start + cols) {
    for (line_end = line_start + cols;
         line_end > line_start && trgt[line_end] != L' ' &&
         trgt[line_end] != L'\t';
         line_end--)
      ;
    if (line_end == line_start)
      break;
    trgt[line_end] = L'\n';
    line_start = line_end + 1;
  }
}

// Return true if needle is in array of strings hayst, false otherwise
bool wmemberof(wchar_t *const *hayst, const wchar_t *needle,
               unsigned needle_len) {
  unsigned i;

  for (i = 0; i < needle_len; i++) {
    if (0 == wcscmp(hayst[i], needle))
      return true;
  }

  return false;
}

// Sort the strings in trgt alphanumerically. trgt_len is trgt's length. Setting
// rev to true causes reverse sorting.
void wsort(wchar_t **trgt, unsigned trgt_len, bool rev) {
  unsigned i;
  int cur_cmp;
  bool sorted = false;
  wchar_t *tmp;

  while (!sorted) {
    sorted = true;
    for (i = 0; i < trgt_len - 1; i++) {
      cur_cmp = wcscoll(trgt[i], trgt[i + 1]);
      if (rev) {
        if (cur_cmp < 0) {
          tmp = trgt[i];
          trgt[i] = trgt[i + 1];
          trgt[i + 1] = tmp;
          sorted = false;
        }
      } else {
        if (cur_cmp > 0) {
          tmp = trgt[i];
          trgt[i] = trgt[i + 1];
          trgt[i + 1] = tmp;
          sorted = false;
        }
      }
    }
  }
}

// Copy all data in source into trgt, line by line. Both source and target must
// be text files. Return the number of lines copied.
unsigned scopylines(FILE *source, FILE *trgt) {
  unsigned cnt = 0;
  char tmp[BS_LINE];

  for (cnt = 0; !feof(source); cnt++) {
    fgets(tmp, BS_LINE, source);
    if (ferror(source)) {
      wchar_t errmsg[BS_LINE];
      serror(errmsg, L"Unable to fgets()");
      winddown(ES_OPER_ERROR, errmsg);
    }

    fwrite(tmp, sizeof(char), strlen(tmp), trgt);
    if (ferror(trgt)) {
      wchar_t errmsg[BS_LINE];
      serror(errmsg, L"Unable to fwrite()");
      winddown(ES_OPER_ERROR, errmsg);
    }
  }

  return cnt - 1;
}

// Read a line from file fp, and place the result in str (without the trailing
// newline). If the read was succesful, return the resulting string's length.
// Otherwise, return -1.
int sreadline(char *str, unsigned size, FILE *fp) {
  fgets(str, BS_LINE, fp);
  if (feof(fp)) {
    str[0] = L'\0';
    return -1;
  } else if (ferror(fp)) {
    wchar_t errmsg[BS_LINE];
    serror(errmsg, L"Unable to fgets()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  char *nlc = strrchr(str, '\n');
  if (NULL != nlc)
    *nlc = '\0';

  return nlc - str;
}
