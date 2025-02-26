// Extended INI file parsing (implementation)

#include "lib.h"

//
// Global variables
//

full_regex_t eini_re_include, eini_re_section, eini_re_value;

//
// Helper functions and macros
//

// Helper of eini_parse(). Set the type, key, and value fields of ret to mytype,
// mykey, and myvalue respectively.
#define set_ret(mytype, mykey, myvalue)                                        \
  ret.type = mytype;                                                           \
  if (NULL == mykey)                                                           \
    ret.key = NULL;                                                            \
  else {                                                                       \
    wcsncpy(ret_key, mykey, BS_SHORT);                                         \
    ret.key = ret_key;                                                         \
  }                                                                            \
  if (NULL == myvalue)                                                         \
    ret.value = NULL;                                                          \
  else {                                                                       \
    wcsncpy(ret_value, myvalue, BS_LINE);                                      \
    ret.value = ret_value;                                                     \
  }

// Helper of eini_parse(). Strip trailing whitespace from src. If it's
// surrounded by double quotes, strip those as well. Return any syntax errors.
#define wsrc_strip                                                             \
  wlen = wmargtrim(wsrc, NULL);                                                \
  if (L'"' == wsrc[0]) {                                                       \
    /* src begins with a '"' */                                                \
    if (wlen < 2) {                                                            \
      /* src equals '"' */                                                     \
      set_ret(EINI_ERROR, NULL, L"non-terminated quote");                      \
      return ret;                                                              \
    } else if (L'"' == wsrc[wlen - 1] && L'\\' == wsrc[wlen - 2]) {            \
      /* src ends in '\"' */                                                   \
      set_ret(EINI_ERROR, NULL, L"non-terminated quote");                      \
      return ret;                                                              \
    } else if (L'"' != wsrc[wlen - 1]) {                                       \
      /* src does not end in '"' */                                            \
      set_ret(EINI_ERROR, NULL, L"non-terminated quote");                      \
      return ret;                                                              \
    } else {                                                                   \
      /* src ends with a '"' */                                                \
      wsrc[wlen - 1] = L'\0';                                                  \
    }                                                                          \
  }

//
// Functions
//

void eini_init() {
  fr_init(&eini_re_include, "\\s*include\\s*", NULL);
  fr_init(&eini_re_section, "\\s*\\[\\s*[a-zA-Z][a-zA-Z0-9_]\\s*\\]\\s*", NULL);
  fr_init(&eini_re_value, "\\s*[a-zA-Z][a-zA-Z0-9_]*\\s*=\\s*", NULL);
}

eini_t eini_parse(char *src, unsigned len) {
  wchar_t *wsrc = walloca(BS_SHORT); // wchar_t* version of src
  int wlen;                          // length of wsrc
  range_t loc;                       // location of regex match in wsrc
  eini_t ret;                        // return value
  static wchar_t ret_key[BS_SHORT];  // key contents of ret
  static wchar_t ret_value[BS_LINE]; // value contents of ret

  wlen = mbstowcs(wsrc, src, BS_SHORT);
  if (-1 == wlen) {
    // Couldn't convert src
    set_ret(EINI_ERROR, NULL, L"non-string data");
    return ret;
  }

  loc = fr_search(&eini_re_include, wsrc);
  if (!(0 == loc.beg && 0 == loc.end)) {
    // Include directive
    wsrc = &wsrc[loc.end + 1];
    wsrc_strip;
    set_ret(EINI_INCLUDE, NULL, wsrc);
    return ret;
  }

  loc = fr_search(&eini_re_section, wsrc);
  if (!(0 == loc.beg && 0 == loc.end)) {
    // Section
    wsrc = &wsrc[wmargend(wsrc, L"[")];
    wmargtrim(wsrc, L"]");
    set_ret(EINI_SECTION, NULL, wsrc);
    return ret;
  }

  loc = fr_search(&eini_re_value, wsrc);
  if (!(0 == loc.beg && 0 == loc.end)) {
    // Key/value pair
    wchar_t *wkey = wsrc;
    wkey[loc.end] = L'\0';
    wkey = &wkey[wmargend(wkey, NULL)];
    wmargtrim(wkey, L"=");
    wsrc = &wsrc[loc.end + 1];
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
  set_ret(EINI_ERROR, NULL, L"unknown input");
  return ret;
}
