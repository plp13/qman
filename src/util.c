// Utility infrastructure, not program-specific (implementation)

#include "lib.h"
#include "program.h"

//
// Functions
//

void serror(wchar_t *dst, const wchar_t *s) {
  if (NULL != s && L'\0' != s[0])
    swprintf(dst, BS_SHORT, L"%ls: %s", s, strerror(errno));
  else
    swprintf(dst, BS_SHORT, L"%s", strerror(errno));
}

void *xcalloc(size_t nmemb, size_t size) {
  void *res = calloc(nmemb, size);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to calloc()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

void *xreallocarray(void *ptr, size_t nmemb, size_t size) {
  void *res = reallocarray(ptr, nmemb, size);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to reallocarray()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

FILE *xpopen(const char *command, const char *type) {
  FILE *pipe = popen(command, type);

  if (NULL == pipe) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to popen()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return pipe;
}

int xpclose(FILE *stream) {
  int status = pclose(stream);

  if (-1 == status) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to pclose()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return status;
}

FILE *xfopen(const char *pathname, const char *mode) {
  FILE *file = fopen(pathname, mode);

  if (NULL == file) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to fopen()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return file;
}

int xfclose(FILE *stream) {
  int status = fclose(stream);

  if (EOF == status) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to fclose()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return status;
}

FILE *xtmpfile() {
  FILE *file = tmpfile();

  if (NULL == file) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to tmpfile()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return file;
}

char *xfgets(char *s, int size, FILE *stream) {
  char *res = fgets(s, size, stream);
  if (ferror(stream)) {
    // Ugly hack: this function sometimes gets rudley interrupted by ncurses; if
    // that happens, we simply ignore the error and return NULL
    if (EINTR != errno) {
      static wchar_t errmsg[BS_SHORT];
      serror(errmsg, L"Unable to fgets()");
      winddown(ES_OPER_ERROR, errmsg);
    }
  }

  return res;
}

size_t xfwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t cnt = fwrite(ptr, size, nmemb, stream);
  if (ferror(stream)) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to write()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return cnt;
}

char *xstrdup(const char *s) {
  char *res = strdup(s);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to strdup()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

wchar_t *xwcsdup(const wchar_t *s) {
  wchar_t *res = wcsdup(s);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to strdup()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

int getenvi(const char *name) {
  char *val = getenv(name);
  if (NULL == val)
    return 0;
  else
    return atoi(val);
}

bool bget(bitarr_t ba, unsigned i) {
  unsigned seg = i / 8;
  unsigned mask = 1 << (i % 8);

  return ba[seg] & mask;
}

void bset(bitarr_t ba, unsigned i) {
  unsigned seg = i / 8;
  unsigned mask = 1 << (i % 8);

  ba[seg] = ba[seg] | mask;
}

void bclear(bitarr_t ba, unsigned i) {
  unsigned seg = i / 8;
  unsigned mask = 1 << (i % 8);

  ba[seg] = ba[seg] & (0xff - mask);
}

void bclearall(bitarr_t ba, unsigned ba_len) {
  unsigned ba_bytes = ba_len % 8 == 0 ? ba_len / 8 : 1 + ba_len / 8;

  for (unsigned i = 0; i < ba_bytes; i++)
    ba[i] = 0;
}

void wafree(wchar_t **buf, unsigned buf_len) {
  unsigned i;

  for (i = 0; i < buf_len; i++)
    free(buf[i]);

  free(buf);
}

void safree(char **buf, unsigned buf_len) {
  unsigned i;

  for (i = 0; i < buf_len; i++)
    free(buf[i]);

  free(buf);
}

unsigned wccnt(const wchar_t *hayst, wchar_t needle) {
  unsigned cnt = 0;

  hayst = wcschr(hayst, needle);
  while (NULL != hayst) {
    cnt++;
    hayst = wcschr(hayst + 1, needle);
  }

  return cnt;
}

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

void wwrap(wchar_t *trgt, unsigned cols) {
  unsigned len = wcslen(trgt), line_start = 0, line_end;

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

bool wcasememberof(wchar_t *const *hayst, const wchar_t *needle,
                   unsigned hayst_len) {
  unsigned i;

  for (i = 0; i < hayst_len; i++) {
    if (0 == wcscasecmp(hayst[i], needle))
      return true;
  }

  return false;
}

bool wmemberof(wchar_t *const *hayst, const wchar_t *needle,
               unsigned hayst_len) {
  unsigned i;

  for (i = 0; i < hayst_len; i++) {
    if (0 == wcscmp(hayst[i], needle))
      return true;
  }

  return false;
}

void wsort(wchar_t **trgt, unsigned trgt_len, bool rev) {
  unsigned i;
  int cur_cmp;
  bool sorted = false;
  wchar_t *tmp;

  if (0 == trgt_len)
    return;

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

unsigned wmaxlen(wchar_t *const *src, unsigned src_len) {
  unsigned maxlen = 0, i;

  for (i = 0; i < src_len; i++)
    maxlen = MAX(maxlen, wcslen(src[i]));

  return maxlen;
}

unsigned scopylines(FILE *source, FILE *trgt) {
  unsigned cnt = 0;
  char tmp[BS_LINE];

  for (cnt = 0; !feof(source); cnt++) {
    xfgets(tmp, BS_LINE, source);
    xfwrite(tmp, sizeof(char), strlen(tmp), trgt);
  }

  return cnt - 1;
}

int sreadline(char *str, unsigned size, FILE *fp) {
  xfgets(str, BS_LINE, fp);
  if (feof(fp)) {
    str[0] = L'\0';
    return -1;
  }

  char *nlc = strrchr(str, '\n');
  if (NULL != nlc)
    *nlc = '\0';

  return nlc - str;
}

void fr_init(full_regex_t *re, char *str) {
  re->str = str;
  int err = regcomp(&re->re, str, REG_EXTENDED);

  if (0 != err) {
    static wchar_t errmsg[BS_SHORT];
    char err_str[BS_SHORT];
    regerror(err, NULL, err_str, BS_SHORT);
    swprintf(errmsg, BS_SHORT, L"Unable to regcomp(): %s", err_str);
    winddown(ES_OPER_ERROR, errmsg);
  }
}

range_t fr_search(const full_regex_t *re, wchar_t *src) {
  char ssrc[BS_LINE]; // char* version of src
  wcstombs(ssrc, src, BS_LINE);
  regmatch_t pmatch[1]; // regex match
  range_t res;          // return value

  // Try to find a match in ssrc
  int err = regexec(&re->re, ssrc, 1, pmatch, 0);

  if (0 == err) {
    // A match was found in ssrc
    regoff_t sbeg = pmatch[0].rm_so; // match begin offset (in ssrc)
    regoff_t send = pmatch[0].rm_eo; // match end offset (in ssrc)
    regoff_t slen = send - sbeg;     // match length (in ssrc)
    wchar_t *wmatch = walloca(slen); // the match as a wchar_t*
    unsigned wlen = mbstowcs(wmatch, &ssrc[sbeg], slen); // wmatch length
    wmatch[wlen] = L'\0';
    wchar_t *wptr = wcsstr(src, wmatch); // match begin memory location (in src)
    if (NULL == wptr) {
      // Cannot replicate match in src; return {0, 0}
      res.beg = 0;
      res.end = 0;
    } else {
      // Match found in src; return its location
      res.beg = wptr - src;
      res.end = res.beg + wlen;
    }
  } else {
    // No match found, or an error has occured; return {0, 0}
    res.beg = 0;
    res.end = 0;
  }

  return res;
}

void loggit(const char *msg) {
  static FILE *lfp = NULL;

  time_t now;

  if (NULL == lfp)
    lfp = fopen(F_LOG, "w");

  time(&now);
  fwprintf(lfp, L"[%s] %s\n", strtok(ctime(&now), "\n"), msg);
}
