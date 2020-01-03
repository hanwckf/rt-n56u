/*
 * bug_fix.c
 */
#include <stdio.h>
#include "oniguruma.h"

static OnigCaseFoldType CF = ONIGENC_CASE_FOLD_MIN;

static int
search(regex_t* reg, unsigned char* str, unsigned char* end)
{
  int r;
  unsigned char *start, *range;
  OnigRegion *region;

  region = onig_region_new();

  start = str;
  range = end;
  r = onig_search(reg, str, end, start, range, region, ONIG_OPTION_NONE);
  if (r >= 0) {
    int i;

    fprintf(stderr, "match at %d  (%s)\n", r,
            ONIGENC_NAME(onig_get_encoding(reg)));
    for (i = 0; i < region->num_regs; i++) {
      fprintf(stderr, "%d: (%d-%d)\n", i, region->beg[i], region->end[i]);
    }
  }
  else if (r == ONIG_MISMATCH) {
    fprintf(stderr, "search fail (%s)\n",
            ONIGENC_NAME(onig_get_encoding(reg)));
  }
  else { /* error */
    char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(s, r);
    fprintf(stderr, "ERROR: %s\n", s);
    fprintf(stderr, "  (%s)\n", ONIGENC_NAME(onig_get_encoding(reg)));
    return -1;
  }

  onig_region_free(region, 1 /* 1:free self, 0:free contents only */);
  return 0;
}

static int
exec_deluxe(OnigEncoding pattern_enc, OnigEncoding str_enc,
            OnigOptionType options, char* apattern, char* astr)
{
  int r;
  unsigned char *end;
  regex_t* reg;
  OnigCompileInfo ci;
  OnigErrorInfo einfo;
  UChar* pattern = (UChar* )apattern;
  UChar* str     = (UChar* )astr;

  onig_initialize(&str_enc, 1);

  ci.num_of_elements = 5;
  ci.pattern_enc = pattern_enc;
  ci.target_enc  = str_enc;
  ci.syntax      = ONIG_SYNTAX_DEFAULT;
  ci.option      = options;
  ci.case_fold_flag  = CF;

  r = onig_new_deluxe(&reg, pattern,
                      pattern + onigenc_str_bytelen_null(pattern_enc, pattern),
                      &ci, &einfo);
  if (r != ONIG_NORMAL) {
    char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(s, r, &einfo);
    fprintf(stderr, "ERROR: %s\n", s);
    return -1;
  }

  end = str + onigenc_str_bytelen_null(str_enc, str);
  r = search(reg, str, end);

  onig_free(reg);
  onig_end();
  return 0;
}

static int
exec(OnigEncoding enc, OnigOptionType options, char* apattern, char* astr)
{
  int r;
  unsigned char *end;
  regex_t* reg;
  OnigErrorInfo einfo;
  UChar* pattern = (UChar* )apattern;
  UChar* str     = (UChar* )astr;

  onig_initialize(&enc, 1);

  r = onig_new(&reg, pattern,
	       pattern + onigenc_str_bytelen_null(enc, pattern),
	       options, enc, ONIG_SYNTAX_DEFAULT, &einfo);
  if (r != ONIG_NORMAL) {
    char s[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(s, r, &einfo);
    fprintf(stderr, "ERROR: %s\n", s);
    return -1;
  }

  end = str + onigenc_str_bytelen_null(enc, str);
  r = search(reg, str, end);

  onig_free(reg);
  onig_end();
  return 0;
}



extern int main(int argc, char* argv[])
{
  /* fix ignore case in look-behind
     commit: 3340ec2cc5627172665303fe248c9793354d2251 */
  exec_deluxe(ONIG_ENCODING_UTF8, ONIG_ENCODING_UTF8,
              ONIG_OPTION_IGNORECASE,
              "(?<=\305\211)a", "\312\274na"); /* \u{0149}a  \u{02bc}na */

  exec(ONIG_ENCODING_UTF8, ONIG_OPTION_NONE, "(\\2)(\\1)", "aa"); /* fail. */

  exec(ONIG_ENCODING_UTF8, ONIG_OPTION_FIND_LONGEST,
       "a*", "aa aaa aaaa aaaaa "); /* match 12-17 */

  return 0;
}
