// Utility infrastructure, not program-specific (implementation)

#include "lib.h"

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
  void *const res = calloc(nmemb, size);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to calloc()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

void *xreallocarray(void *ptr, size_t nmemb, size_t size) {
  void *const res = reallocarray(ptr, nmemb, size);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to reallocarray()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

FILE *xpopen(const char *command, const char *type) {
  FILE *const pipe = popen(command, type);

  if (NULL == pipe) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to popen()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return pipe;
}

int xpclose(FILE *stream) {
  const int status = pclose(stream);

  if (-1 == status) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to pclose()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return status;
}

gzFile xgzopen(const char *path, const char *mode) {
  gzFile gzfp = gzopen(path, mode);

  if (NULL == gzfp) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to gzopen()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return gzfp;
}

int xgzclose(gzFile file) {
  const int status = gzclose(file);

  if (Z_OK != status) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to gzclose()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return status;
}

FILE *xfopen(const char *pathname, const char *mode) {
  FILE *const file = fopen(pathname, mode);

  if (NULL == file) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to fopen()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return file;
}

int xfclose(FILE *stream) {
  const int status = fclose(stream);

  if (EOF == status) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to fclose()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return status;
}

FILE *xtmpfile() {
  FILE *const file = tmpfile();

  if (NULL == file) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to tmpfile()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return file;
}

char *xgzgets(gzFile file, char *buf, int len) {
  char *res;

  while (true) {
    res = gzgets(file, buf, len);

    if (NULL == res && !gzeof(file)) {
      // There has been an error
      if (EINTR == errno) {
        // Sometimes ncurses rudely interrupts gzgets(). If that's the case, try
        // calling gzgets() again
        gzclearerr(file);
      } else {
        // Otherwise, fail gracefully
        static wchar_t errmsg[BS_SHORT];
        serror(errmsg, L"Unable to gzgets()");
        winddown(ES_OPER_ERROR, errmsg);
      }
    } else {
      // No error; return the result
      return res;
    }
  }
}

char *xfgets(char *s, int size, FILE *stream) {
  char *res;

  while (true) {
    res = fgets(s, size, stream);

    if (ferror(stream) && !feof(stream)) {
      // There has been an error
      if (EINTR == errno) {
        // Sometimes ncurses rudely interrupts fgets(). If that's the case, try
        // calling fgets() again
        clearerr(stream);
      } else {
        // Otherwise, fail gracefully
        static wchar_t errmsg[BS_SHORT];
        serror(errmsg, L"Unable to fgets()");
        winddown(ES_OPER_ERROR, errmsg);
      }
    } else {
      // No error; return the result
      return res;
    }
  }
}

int xfputs(const char *s, FILE *stream) {
  int res = fputs(s, stream);

  if (EOF == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to fputs()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

size_t xfread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  const size_t cnt = fread(ptr, size, nmemb, stream);

  if (ferror(stream)) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to read()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return cnt;
}

size_t xfwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  const size_t cnt = fwrite(ptr, size, nmemb, stream);

  if (ferror(stream)) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to write()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return cnt;
}

char *xstrdup(const char *s) {
  char *const res = strdup(s);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to strdup()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

wchar_t *xwcsdup(const wchar_t *s) {
  wchar_t *const res = wcsdup(s);

  if (NULL == res) {
    static wchar_t errmsg[BS_SHORT];
    serror(errmsg, L"Unable to strdup()");
    winddown(ES_OPER_ERROR, errmsg);
  }

  return res;
}

void xsystem(const char *cmd, bool fail) {
  int res = system(cmd);

  if (fail && 0 != res) {
    static wchar_t errmsg[BS_SHORT];
    swprintf(errmsg, BS_SHORT, L"Failed to execute: %s", cmd);
    winddown(ES_CHILD_ERROR, errmsg);
  }
}

char *xtempnam(const char *dir, const char *pfx) {
  char *fn;
  int fd;

  if (NULL != getenv("TMPDIR"))
    dir = getenv("TMPDIR");
  if (NULL == dir)
    dir = P_tmpdir;

  if (NULL == pfx)
    winddown(ES_OPER_ERROR, L"Unable to xtempnam(): prefix is NULL");
  for (unsigned i = 0; i < strlen(pfx); i++)
    if ('X' == pfx[i])
      winddown(ES_OPER_ERROR, L"Unable to xtempnam(): prefix contains 'X'");
  if (strlen(dir) + strlen(pfx) + 2 > BS_SHORT)
    winddown(ES_OPER_ERROR, L"Unable to xtempnam(): prefix too long");

  fn = salloc(BS_SHORT);
  snprintf(fn, BS_SHORT, "%s/%sXXXXXX", dir, pfx);

  fd = mkstemp(fn);
  if (-1 == fd)
    winddown(ES_OPER_ERROR, L"Unable to xtempnam(): mkstemp() failed");
  if (-1 == close(fd))
    winddown(ES_OPER_ERROR, L"Unable to xtempnam(): close() failed");

  return fn;
}

int getenvi(const char *name) {
  const char *const val = getenv(name);

  if (NULL == val)
    return 0;
  else
    return atoi(val);
}

bool bget(const bitarr_t ba, unsigned i) {
  const unsigned seg = i / 8;
  const unsigned mask = 1 << (i % 8);

  return ba[seg] & mask;
}

void bset(bitarr_t ba, unsigned i) {
  const unsigned seg = i / 8;
  const unsigned mask = 1 << (i % 8);

  ba[seg] = ba[seg] | mask;
}

void bclear(bitarr_t ba, unsigned i) {
  const unsigned seg = i / 8;
  const unsigned mask = 1 << (i % 8);

  ba[seg] = ba[seg] & (0xff - mask);
}

void bclearall(bitarr_t ba, unsigned ba_len) {
  const unsigned ba_bytes = ba_len % 8 == 0 ? ba_len / 8 : 1 + ba_len / 8;

  for (unsigned i = 0; i < ba_bytes; i++)
    ba[i] = 0;
}

char *bzip2_decompress(const char *bzpath) {
#ifdef QMAN_BZIP2
  char *path;        // Decompressed file pathname
  FILE *fp;          // Decompressed file pointer
  BZFILE *bzhand;    // Compressed file handle
  FILE *bzfp;        // Compressed file pointer
  int bzerror;       // Used for BZ2 library error reporting
  char buf[BS_LINE]; // Data buffer
  int len;           // Bytes written into data buffer

  bzfp = xfopen(bzpath, "r");
  bzhand = BZ2_bzReadOpen(&bzerror, bzfp, 0, 0, NULL, 0);
  if (BZ_OK != bzerror) {
    winddown(ES_OPER_ERROR,
             L"Unable to decompress Bzip2 archive: BZ2_bzReadOpen() failed");
  }

  path = xtempnam(NULL, "qman");
  fp = xfopen(path, "w");

  while (true) {
    len = BZ2_bzRead(&bzerror, bzhand, buf, BS_LINE);
    if (BZ_OK == bzerror)
      xfwrite(buf, len, 1, fp);
    else if (BZ_STREAM_END == bzerror) {
      xfwrite(buf, len, 1, fp);
      break;
    } else
      winddown(ES_OPER_ERROR,
               L"Unable to decompress Bzip2 archive: BZ2Read() failed");
  }

  BZ2_bzReadClose(&bzerror, bzhand);
  if (BZ_OK != bzerror) {
    winddown(ES_OPER_ERROR,
             L"Unable to decompress Bzip2 archive: BZ2_bzReadClose() failed");
  }
  xfclose(bzfp);
  xfclose(fp);

  return path;
#else
  winddown(ES_OPER_ERROR, L"Bzip2 archives are not supported");
  return NULL;
#endif
}

char *lzma_decompress(const char *pathname) {
#ifdef QMAN_LZMA
  char *path;                            // Decompressed file pathname
  FILE *fp;                              // Decompressed file pointer
  lzma_ret lzret;                        // Lzma return status
  lzma_stream lzstrm = LZMA_STREAM_INIT; // Lzma stream
  FILE *lzfp;                            // Compressed file pointer
  uint8_t buf[BUFSIZ];                   // Decompressed data buffer
  uint8_t lzbuf[BUFSIZ];                 // Compressed data buffer

  lzfp = xfopen(pathname, "r");

  lzret = lzma_stream_decoder(&lzstrm, UINT64_MAX, LZMA_CONCATENATED);
  if (LZMA_OK != lzret) {
    winddown(ES_OPER_ERROR,
             L"Unable to decompress XZ archive: lzma_stream_decoder() failed");
  }

  path = xtempnam(NULL, "qman");
  fp = xfopen(path, "w");

  lzstrm.next_in = NULL;
  lzstrm.avail_in = 0;
  lzstrm.next_out = buf;
  lzstrm.avail_out = sizeof(buf);

  while (true) {

    if (0 == lzstrm.avail_in && !feof(lzfp)) {
      lzstrm.next_in = lzbuf;
      lzstrm.avail_in = xfread(lzbuf, 1, sizeof(lzbuf), lzfp);
    }

    if (!feof(lzfp))
      lzret = lzma_code(&lzstrm, LZMA_RUN);
    else
      lzret = lzma_code(&lzstrm, LZMA_FINISH);

    if (0 == lzstrm.avail_out || LZMA_STREAM_END == lzret) {
      xfwrite(buf, 1, sizeof(buf) - lzstrm.avail_out, fp);
      lzstrm.next_out = buf;
      lzstrm.avail_out = sizeof(buf);
    }

    if (LZMA_STREAM_END == lzret)
      break;

    if (LZMA_OK != lzret) {
      winddown(ES_OPER_ERROR,
               L"Unable to decompress XZ archive: lzma_code() failed");
    }
  }

  xfclose(fp);
  lzma_end(&lzstrm);
  xfclose(lzfp);

  return path;
#else
  winddown(ES_OPER_ERROR, L"XZ archives are not supported");
  return NULL;
#endif
}

archive_t aropen(const char *pathname) {
  archive_t a;
  char *pathext = strrchr(pathname, '.');

  if (NULL == pathext)
    a.type = AR_NONE;
  else if (0 == strcasecmp(".xz", pathext))
    a.type = AR_LZMA;
  else if (0 == strcasecmp(".bz2", pathext))
    a.type = AR_BZIP2;
  else if (0 == strcasecmp(".gz", pathext))
    a.type = AR_GZIP;
  else
    a.type = AR_NONE;

  switch (a.type) {
  case AR_LZMA:
    a.path = lzma_decompress(pathname);
    a.fp_lzma = xfopen(a.path, "r");
    break;
  case AR_BZIP2:
    a.path = bzip2_decompress(pathname);
    a.fp_bzip2 = xfopen(a.path, "r");
    break;
  case AR_GZIP:
    a.path = xstrdup(pathname);
    a.fp_gzip = xgzopen(a.path, "rb");
    break;
  case AR_NONE:
  default:
    a.path = xstrdup(pathname);
    a.fp_none = xfopen(a.path, "r");
    break;
  }

  return a;
}

void argets(archive_t ap, char *buf, int len) {
  switch (ap.type) {
  case AR_LZMA:
    xfgets(buf, len, ap.fp_lzma);
    break;
  case AR_BZIP2:
    xfgets(buf, len, ap.fp_bzip2);
    break;
  case AR_GZIP:
    xgzgets(ap.fp_gzip, buf, len);
    break;
  case AR_NONE:
  default:
    xfgets(buf, len, ap.fp_none);
  }
}

bool areof(archive_t ap) {
  switch (ap.type) {
  case AR_LZMA:
    return feof(ap.fp_lzma);
    break;
  case AR_BZIP2:
    return feof(ap.fp_bzip2);
    break;
  case AR_GZIP:
    return gzeof(ap.fp_gzip);
    break;
  case AR_NONE:
  default:
    return feof(ap.fp_none);
  }
}

void arclose(archive_t ap) {
  switch (ap.type) {
  case AR_LZMA:
    xfclose(ap.fp_lzma);
    unlink(ap.path);
    break;
  case AR_BZIP2:
    xfclose(ap.fp_bzip2);
    unlink(ap.path);
    break;
  case AR_GZIP:
    xgzclose(ap.fp_gzip);
    break;
  case AR_NONE:
  default:
    xfclose(ap.fp_none);
    break;
  }

  free(ap.path);
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
  const wchar_t *const hayst_start = (wchar_t *)hayst;
  unsigned offset = 0, repl_cnt = 0;
  const unsigned repl_len = wcslen(repl);

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
  const unsigned len = wcslen(trgt);
  unsigned line_start = 0, line_end;

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

bool wcasememberof(const wchar_t *const *hayst, const wchar_t *needle,
                   unsigned hayst_len) {
  unsigned i;

  for (i = 0; i < hayst_len; i++) {
    if (0 == wcscasecmp(hayst[i], needle))
      return true;
  }

  return false;
}

bool wmemberof(const wchar_t *const *hayst, const wchar_t *needle,
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

unsigned wmaxlen(const wchar_t *const *src, unsigned src_len) {
  unsigned maxlen = 0, i;

  for (i = 0; i < src_len; i++)
    maxlen = MAX(maxlen, wcslen(src[i]));

  return maxlen;
}

unsigned wsplit(wchar_t ***dst, unsigned dst_len, wchar_t *src,
                const wchar_t *extras) {
  wchar_t **res = *dst; // results
  unsigned res_cnt = 0; // number of results
  bool ws = true;       // whether current character is whitespace or in extras
  bool pws;             // whether previous character is whitespace or in extras
  unsigned i, j = 0;    // iterators

  if (NULL == extras)
    extras = L"";

  for (i = 0; L'\0' != src[i]; i++) {
    pws = ws;

    ws = false;
    if (iswspace(src[i]))
      ws = true;
    for (j = 0; L'\0' != extras[j]; j++)
      if (src[i] == extras[j])
        ws = true;

    if (!ws) {
      if (i == 0 || pws) {
        if (res_cnt < dst_len) {
          res[res_cnt] = &src[i];
          res_cnt++;
        }
      }
    } else
      src[i] = L'\0';
  }

  return res_cnt;
}

unsigned wmargend(const wchar_t *src, const wchar_t *extras) {
  bool ws;       // whether current character is whitespace or in extras
  unsigned i, j; // iterators

  if (NULL == extras)
    extras = L"";

  for (i = 0; L'\0' != src[i]; i++) {
    ws = false;
    if (iswspace(src[i]))
      ws = true;
    for (j = 0; L'\0' != extras[j]; j++)
      if (src[i] == extras[j])
        ws = true;

    if (!ws)
      return i;
  }

  return 0;
}

unsigned wmargtrim(wchar_t *trgt, const wchar_t *extras) {
  int i;      // iterator
  unsigned j; // iterator
  bool trim;  // true if we'll be trimming trgt[i]

  if (NULL == extras)
    extras = L"";

  for (i = 0; L'\0' != trgt[i]; i++)
    ;

  i--;
  while (i >= 0) {
    trim = false;

    if (iswspace(trgt[i]))
      trim = true;
    else
      for (j = 0; L'\0' != extras[j]; j++)
        if (trgt[i] == extras[j])
          trim = true;

    if (!trim) {
      trgt[i + 1] = L'\0';
      return i + 1;
    }

    i--;
  }

  trgt[0] = L'\0';
  return 0;
}

unsigned wbs(wchar_t *trgt) {
  unsigned i, j; // iterators

  j = 0;
  for (i = 0; L'\0' != trgt[i]; i++) {
    if (L'\b' == trgt[i]) {
      if (j > 0)
        j--;
    } else {
      trgt[j] = trgt[i];
      j++;
    }
  }

  trgt[j] = L'\0';
  return j;
}

wchar_t *wcscasestr(const wchar_t *haystack, const wchar_t *needle) {
  unsigned i = 0, j;
  wchar_t haystack_c, needle_c;

  if (L'\0' == needle[0])
    return (wchar_t *)haystack;

  while (L'\0' != haystack[i]) {
    j = 0;
    while (TRUE) {
      haystack_c = towlower(haystack[i + j]);
      needle_c = towlower(needle[j]);
      if (L'\0' == needle_c)
        return (wchar_t *)&haystack[i];
      else if (haystack_c != needle_c)
        break;
      else
        j++;
    }
    i++;
  }

  return NULL;
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

unsigned split_path(char ***dst, unsigned dst_len, char *src) {
  char **res = *dst;    // results
  unsigned res_cnt = 0; // number of results
  unsigned pos = 0;     // starting position of last path found
  unsigned i;           // iterator

  for (i = 0;; i++) {
    if (':' == src[i] || '\0' == src[i]) {
      res[res_cnt] = &src[pos];
      res_cnt++;
      pos = i + 1;

      if ('\0' == src[i])
        break;
      else
        src[i] = '\0';
    }
  }

  return res_cnt;
}

void fr_init(full_regex_t *re, char *str, wchar_t *snpt) {
  re->str = str;
  re->snpt = snpt;

  int err = regcomp(&re->re, str, REG_EXTENDED);
  if (0 != err) {
    static wchar_t errmsg[BS_SHORT];
    char err_str[BS_SHORT];
    regerror(err, NULL, err_str, BS_SHORT);
    swprintf(errmsg, BS_SHORT, L"Unable to regcomp(): %s", err_str);
    winddown(ES_OPER_ERROR, errmsg);
  }
}

range_t fr_search(const full_regex_t *re, const wchar_t *src) {
  char ssrc[BS_LINE];   // char* version of src
  regmatch_t pmatch[1]; // regex match
  range_t res;          // return value

  // If re->snpt isn't in src, return {0, 0}
  if (NULL == wcsstr(src, re->snpt)) {
    res.beg = 0;
    res.end = 0;
    return res;
  }

  // Convert src to ssrc and try to find a match in it
  wcstombs(ssrc, src, BS_LINE);
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
