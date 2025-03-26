// Extended INI file parsing (implementation)

#include "lib.h"

//
// Global variables
//

regex_t eini_re_include, eini_re_section, eini_re_value;

//
// Helper functions and macros
//

// Helper of `eini_parse()`. Set the `type`, `key`, and `value` fields of `ret`
// to `mytype`, `mykey`, and `myvalue` respectively.
#define set_ret(mytype, mykey, myvalue)                                        \
  ret.type = mytype;                                                           \
  if (NULL == mykey)                                                           \
    ret.key = NULL;                                                            \
  else {                                                                       \
    wcsncpy(ret_key, wnnl(mykey), BS_SHORT);                                   \
    ret.key = ret_key;                                                         \
  }                                                                            \
  if (NULL == myvalue)                                                         \
    ret.value = NULL;                                                          \
  else {                                                                       \
    wcsncpy(ret_value, wnnl(myvalue), BS_LINE);                                \
    wunescape(ret_value);                                                      \
    ret.value = ret_value;                                                     \
  }

// Helper of `eini_parse()`. Strip trailing whitespace from `src`. If it's
// surrounded by single or double quotes, strip those as well. Return any syntax
// errors.
#define wsrc_strip                                                             \
  wlen = wmargtrim(wsrc, NULL);                                                \
  if (L'"' == wsrc[0]) {                                                       \
    /* wsrc begins with a `"` */                                               \
    if (wlen < 2) {                                                            \
      /* wsrc equals `"`; accept it as it is */                                \
    } else if (L'"' == wsrc[wlen - 1]) {                                       \
      /* wsrc ends in '"' */                                                   \
      if (wescaped(wsrc, wlen - 1)) {                                          \
        /* the ending `"` is escaped; reject it */                             \
        set_ret(EINI_ERROR, NULL, L"Non-terminated quote");                    \
        return ret;                                                            \
      } else {                                                                 \
        /* the ending `"` is not escaped; remove the `"`s and accept it */     \
        wsrc[wlen - 1] = L'\0';                                                \
        wsrc = &wsrc[1];                                                       \
      }                                                                        \
    } else {                                                                   \
      /* wsrc does not end in `"`; reject it */                                \
      set_ret(EINI_ERROR, NULL, L"Non-terminated quote");                      \
      return ret;                                                              \
    }                                                                          \
  } else if (L'\'' == wsrc[0]) {                                               \
    /* wsrc begins with a `'` */                                               \
    if (wlen < 2) {                                                            \
      /* wsrc equals `'`; accept it as it is */                                \
    } else if (L'\'' == wsrc[wlen - 1]) {                                      \
      /* wsrc ends in `'` */                                                   \
      if (wescaped(wsrc, wlen - 1)) {                                          \
        /* the ending `'` is escaped; reject it */                             \
        set_ret(EINI_ERROR, NULL, L"Non-terminated quote");                    \
        return ret;                                                            \
      } else {                                                                 \
        /* the ending `"` is not escaped; remove the `"`s and accept it */     \
        wsrc[wlen - 1] = L'\0';                                                \
        wsrc = &wsrc[1];                                                       \
      }                                                                        \
    } else {                                                                   \
      /* wsrc does not end in `'`; reject it */                                \
      set_ret(EINI_ERROR, NULL, L"Non-terminated quote");                      \
      return ret;                                                              \
    }                                                                          \
  }

// Helper of `eini_parse()`. Discard all comments in `src`.
void decomment(wchar_t *src) {
  unsigned i = 0;   // iterator
  bool inq = false; // true if we are inside a quoted string
  wchar_t qtype =
      L'?'; // type of quotes used by the string we are in: `'` or `"`

  while (L'\0' != src[i]) {
    if (inq) {
      // we are inside a quoted string
      if (qtype == src[i] && !wescaped(src, i)) {
        // `src[i]` is an unescaped `"` or `'`, thus the quoted string ends
        inq = false;
      }
    } else {
      // we are not inside a quoted string
      if (L';' == src[i]) {
        // `src[i]` is `;`, thus we have a comment to the end of line
        src[i] = L'\0';
        return;
      } else if (L'"' == src[i] && !wescaped(src, i)) {
        // `src[i]` is an unescaped `"`, thus a double-quoted string begins
        inq = true;
        qtype = L'"';
      } else if (L'\'' == src[i] && !wescaped(src, i)) {
        // src[i] is an unescaped `'`, thus a single-quoted string begins
        inq = true;
        qtype = L'\'';
      }
    }
    i++;
  }
}

// Helper of `eini_parse()`. Find the first match of `re` in `src` and return
// its location.
range_t match(regex_t re, char *src) {
  regmatch_t pmatch[1]; // regex match
  range_t res;          // return value

  // Try to match `src`
  int err = regexec(&re, src, 1, pmatch, 0);

  if (0 == err) {
    // A match was found
    res.beg = pmatch[0].rm_so;
    res.end = pmatch[0].rm_eo;
  } else {
    // No match found, or an error has occured; return `{0, 0}`
    res.beg = 0;
    res.end = 0;
  }

  return res;
}

//
// Functions
//

void eini_init() {
  regcomp(&eini_re_include, "\\s*include\\s*", REG_EXTENDED);
  regcomp(&eini_re_section, "\\s*\\[\\s*[a-zA-Z][a-zA-Z0-9_]*\\s*\\]\\s*",
          REG_EXTENDED);
  regcomp(&eini_re_value, "\\s*[a-zA-Z][a-zA-Z0-9_]*\\s*=\\s*", REG_EXTENDED);
}

eini_t eini_parse(char *src) {
  wchar_t *wsrc = walloca(BS_LINE); // `wchar_t*` version of `src`
  char *csrc =
      salloca(BS_LINE); // `char*` version of `src` (after modification)
  int wlen;             // length of `wsrc`
  range_t loc;          // location of regex match in `wsrc`
  eini_t ret;           // return value
  static wchar_t ret_key[BS_SHORT];  // `key` contents of `ret`
  static wchar_t ret_value[BS_LINE]; // `value` contents of `ret`

  wlen = mbstowcs(wsrc, src, BS_LINE);
  if (-1 == wlen) {
    // Couldn't convert `src`
    set_ret(EINI_ERROR, NULL, L"Non-string data");
    return ret;
  }

  decomment(wsrc);
  wcstombs(csrc, wsrc, BS_LINE);

  loc = match(eini_re_include, csrc);
  if (0 == loc.beg && loc.end > loc.beg) {
    // Include directive
    wsrc = &wsrc[loc.end];
    wsrc_strip;
    set_ret(EINI_INCLUDE, NULL, wsrc);
    return ret;
  }

  loc = match(eini_re_section, csrc);
  if (!(0 == loc.beg && 0 == loc.end)) {
    // Section
    wsrc = &wsrc[wmargend(wsrc, L"[")];
    wmargtrim(wsrc, L"]");
    set_ret(EINI_SECTION, NULL, wsrc);
    return ret;
  }

  loc = match(eini_re_value, csrc);
  if (!(0 == loc.beg && 0 == loc.end)) {
    // Key/value pair
    wchar_t *wkey = wsrc;
    wkey[loc.end - 1] = L'\0';
    wkey = &wkey[wmargend(wkey, NULL)];
    wmargtrim(wkey, L"=");
    wsrc = &wsrc[loc.end];
    wsrc_strip;
    set_ret(EINI_VALUE, wkey, wsrc);
    return ret;
  }

  wlen = wmargtrim(wsrc, NULL);
  if (0 == wlen) {
    // Empty line
    set_ret(EINI_NONE, NULL, NULL);
    return ret;
  }

  // None of the above
  wchar_t errmsg[BS_LINE];
  swprintf(errmsg, BS_LINE, L"Unable to parse '%ls'", wsrc);
  set_ret(EINI_ERROR, NULL, errmsg);
  return ret;
}

void eini(eini_handler_t hf, eini_error_t ef, const char *path) {
  unsigned i = 0;              // current line number
  FILE *fp;                    // file pointer
  char ln[BS_LINE];            // current config line
  eini_t lne;                  // current config line contents
  wchar_t sec[BS_SHORT] = L""; // current section

  fp = xfopen(path, "r");

  while (!feof(fp)) {
    xfgets(ln, BS_LINE, fp);
    i++;
    lne = eini_parse(ln);

    switch (lne.type) {
    case EINI_INCLUDE: {
      char ipath[BS_LINE]; // include file path
      FILE *ifp;           // include file pointer
      if (L'/' == lne.value[0]) {
        wcstombs(ipath, lne.value, BS_LINE);
      } else {
        char apath[BS_LINE];
        wcstombs(apath, lne.value, BS_LINE);
        strncpy(ipath, xdirname(path), BS_LINE);
        strncat(ipath, "/", BS_LINE - strlen(ipath) - 1);
        strncat(ipath, apath, BS_LINE - strlen(ipath) - 1);
      }
      ifp = fopen(ipath, "r");
      if (NULL == ifp) {
        wchar_t errmsg[BS_SHORT];
        swprintf(errmsg, BS_SHORT, L"Unable to open '%s'", ipath);
        ef(errmsg, path, i);
      }
      fclose(ifp);
      eini(hf, ef, ipath);
      break;
    }
    case EINI_SECTION: {
      wcsncpy(sec, lne.value, BS_SHORT);
      break;
    }
    case EINI_VALUE: {
      if (0 == wcslen(sec)) {
        wchar_t errmsg[BS_SHORT];
        swprintf(errmsg, BS_SHORT, L"Option '%ls' does not have a section",
                 lne.key);
        ef(errmsg, path, i);
      } else
        hf(sec, lne.key, lne.value, path, i);
      break;
    }
    case EINI_ERROR: {
      ef(lne.value, path, i);
      break;
    }
    case EINI_NONE:
    default:
      break;
    }
  }

  xfclose(fp);
}

void eini_winddown() {
  regfree(&eini_re_include);
  regfree(&eini_re_section);
  regfree(&eini_re_value);
}
