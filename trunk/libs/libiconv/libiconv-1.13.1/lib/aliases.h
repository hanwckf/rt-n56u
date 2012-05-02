/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: gperf -m 10 lib/aliases.gperf  */
/* Computed positions: -k'4-7,10,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "lib/aliases.gperf"
struct alias { int name; unsigned int encoding_index; };

#define TOTAL_KEYWORDS 89
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 18
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 108
/* maximum key range = 105, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
aliases_hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
      109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
      109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
      109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
      109, 109, 109, 109, 109,   9, 109, 109,  27,   2,
        3,  34,  10,   0,  13,  22,   0,   0, 109, 109,
      109, 109, 109, 109, 109,   0,  31,  14,   0,  10,
      109,   2, 109,   7, 109, 109,   0,   0,  14,  22,
        7, 109,   1,  39,  13,  17, 109,   5,   8,  28,
      109, 109, 109, 109, 109,  14, 109, 109, 109, 109,
      109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
      109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
      109, 109, 109, 109, 109, 109, 109, 109
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
      case 8:
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
      case 2:
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str4[sizeof("L1")];
    char stringpool_str5[sizeof("L2")];
    char stringpool_str6[sizeof("CHAR")];
    char stringpool_str7[sizeof("CP819")];
    char stringpool_str8[sizeof("IBM819")];
    char stringpool_str9[sizeof("ISO8859-5")];
    char stringpool_str10[sizeof("ISO8859-15")];
    char stringpool_str11[sizeof("ISO8859-1")];
    char stringpool_str12[sizeof("ISO8859-2")];
    char stringpool_str13[sizeof("CP1251")];
    char stringpool_str14[sizeof("UTF-8")];
    char stringpool_str15[sizeof("CP1252")];
    char stringpool_str16[sizeof("866")];
    char stringpool_str17[sizeof("KOI8-R")];
    char stringpool_str19[sizeof("ISO-8859-5")];
    char stringpool_str20[sizeof("UCS-2")];
    char stringpool_str22[sizeof("ISO-8859-15")];
    char stringpool_str23[sizeof("ISO-8859-1")];
    char stringpool_str24[sizeof("ISO_8859-5")];
    char stringpool_str25[sizeof("ISO-8859-2")];
    char stringpool_str26[sizeof("ASCII")];
    char stringpool_str27[sizeof("ISO_8859-15")];
    char stringpool_str28[sizeof("ISO_8859-1")];
    char stringpool_str29[sizeof("ISO_8859-5:1988")];
    char stringpool_str30[sizeof("ISO_8859-2")];
    char stringpool_str31[sizeof("LATIN1")];
    char stringpool_str32[sizeof("ISO_8859-15:1998")];
    char stringpool_str33[sizeof("LATIN2")];
    char stringpool_str34[sizeof("UCS-4")];
    char stringpool_str35[sizeof("MS-EE")];
    char stringpool_str36[sizeof("CYRILLIC")];
    char stringpool_str37[sizeof("LATIN-9")];
    char stringpool_str38[sizeof("CSKOI8R")];
    char stringpool_str39[sizeof("UCS-2LE")];
    char stringpool_str40[sizeof("ISO-IR-101")];
    char stringpool_str41[sizeof("US")];
    char stringpool_str42[sizeof("UTF-16LE")];
    char stringpool_str43[sizeof("UTF-16")];
    char stringpool_str44[sizeof("CP866")];
    char stringpool_str45[sizeof("IBM866")];
    char stringpool_str46[sizeof("UCS-4LE")];
    char stringpool_str47[sizeof("ISO-IR-6")];
    char stringpool_str48[sizeof("WCHAR_T")];
    char stringpool_str49[sizeof("KOI8-U")];
    char stringpool_str50[sizeof("MS-CYRL")];
    char stringpool_str51[sizeof("KOI8-RU")];
    char stringpool_str52[sizeof("UCS-2-INTERNAL")];
    char stringpool_str53[sizeof("ISO_8859-1:1987")];
    char stringpool_str54[sizeof("ISO_8859-2:1987")];
    char stringpool_str55[sizeof("UTF-32")];
    char stringpool_str56[sizeof("ISO-IR-144")];
    char stringpool_str58[sizeof("UTF-7")];
    char stringpool_str59[sizeof("UCS-4-INTERNAL")];
    char stringpool_str60[sizeof("UNICODEBIG")];
    char stringpool_str62[sizeof("CP367")];
    char stringpool_str63[sizeof("CP1250")];
    char stringpool_str64[sizeof("UTF-32LE")];
    char stringpool_str65[sizeof("CSIBM866")];
    char stringpool_str68[sizeof("UNICODE-1-1")];
    char stringpool_str69[sizeof("ISO_646.IRV:1991")];
    char stringpool_str70[sizeof("UCS-2BE")];
    char stringpool_str72[sizeof("CSUNICODE11")];
    char stringpool_str73[sizeof("UTF-16BE")];
    char stringpool_str74[sizeof("MS-ANSI")];
    char stringpool_str75[sizeof("US-ASCII")];
    char stringpool_str76[sizeof("CSUNICODE")];
    char stringpool_str77[sizeof("UCS-4BE")];
    char stringpool_str78[sizeof("ISO-10646-UCS-2")];
    char stringpool_str79[sizeof("CSUCS4")];
    char stringpool_str80[sizeof("UCS-2-SWAPPED")];
    char stringpool_str81[sizeof("CSASCII")];
    char stringpool_str82[sizeof("UNICODELITTLE")];
    char stringpool_str83[sizeof("WINDOWS-1251")];
    char stringpool_str84[sizeof("WINDOWS-1252")];
    char stringpool_str85[sizeof("ISO-10646-UCS-4")];
    char stringpool_str86[sizeof("ANSI_X3.4-1968")];
    char stringpool_str87[sizeof("UCS-4-SWAPPED")];
    char stringpool_str88[sizeof("CSISOLATIN1")];
    char stringpool_str89[sizeof("CSISOLATIN2")];
    char stringpool_str90[sizeof("ISO-IR-100")];
    char stringpool_str93[sizeof("ISO646-US")];
    char stringpool_str94[sizeof("UNICODE-1-1-UTF-7")];
    char stringpool_str95[sizeof("UTF-32BE")];
    char stringpool_str96[sizeof("CSUNICODE11UTF7")];
    char stringpool_str97[sizeof("IBM367")];
    char stringpool_str99[sizeof("ANSI_X3.4-1986")];
    char stringpool_str104[sizeof("ISO-IR-203")];
    char stringpool_str107[sizeof("CSISOLATINCYRILLIC")];
    char stringpool_str108[sizeof("WINDOWS-1250")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "L1",
    "L2",
    "CHAR",
    "CP819",
    "IBM819",
    "ISO8859-5",
    "ISO8859-15",
    "ISO8859-1",
    "ISO8859-2",
    "CP1251",
    "UTF-8",
    "CP1252",
    "866",
    "KOI8-R",
    "ISO-8859-5",
    "UCS-2",
    "ISO-8859-15",
    "ISO-8859-1",
    "ISO_8859-5",
    "ISO-8859-2",
    "ASCII",
    "ISO_8859-15",
    "ISO_8859-1",
    "ISO_8859-5:1988",
    "ISO_8859-2",
    "LATIN1",
    "ISO_8859-15:1998",
    "LATIN2",
    "UCS-4",
    "MS-EE",
    "CYRILLIC",
    "LATIN-9",
    "CSKOI8R",
    "UCS-2LE",
    "ISO-IR-101",
    "US",
    "UTF-16LE",
    "UTF-16",
    "CP866",
    "IBM866",
    "UCS-4LE",
    "ISO-IR-6",
    "WCHAR_T",
    "KOI8-U",
    "MS-CYRL",
    "KOI8-RU",
    "UCS-2-INTERNAL",
    "ISO_8859-1:1987",
    "ISO_8859-2:1987",
    "UTF-32",
    "ISO-IR-144",
    "UTF-7",
    "UCS-4-INTERNAL",
    "UNICODEBIG",
    "CP367",
    "CP1250",
    "UTF-32LE",
    "CSIBM866",
    "UNICODE-1-1",
    "ISO_646.IRV:1991",
    "UCS-2BE",
    "CSUNICODE11",
    "UTF-16BE",
    "MS-ANSI",
    "US-ASCII",
    "CSUNICODE",
    "UCS-4BE",
    "ISO-10646-UCS-2",
    "CSUCS4",
    "UCS-2-SWAPPED",
    "CSASCII",
    "UNICODELITTLE",
    "WINDOWS-1251",
    "WINDOWS-1252",
    "ISO-10646-UCS-4",
    "ANSI_X3.4-1968",
    "UCS-4-SWAPPED",
    "CSISOLATIN1",
    "CSISOLATIN2",
    "ISO-IR-100",
    "ISO646-US",
    "UNICODE-1-1-UTF-7",
    "UTF-32BE",
    "CSUNICODE11UTF7",
    "IBM367",
    "ANSI_X3.4-1986",
    "ISO-IR-203",
    "CSISOLATINCYRILLIC",
    "WINDOWS-1250"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct alias aliases[] =
  {
    {-1}, {-1}, {-1}, {-1},
#line 58 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str4, ei_iso8859_1},
#line 66 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str5, ei_iso8859_2},
#line 99 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str6, ei_local_char},
#line 55 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str7, ei_iso8859_1},
#line 56 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str8, ei_iso8859_1},
#line 75 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str9, ei_iso8859_5},
#line 81 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str10, ei_iso8859_15},
#line 60 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str11, ei_iso8859_1},
#line 68 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str12, ei_iso8859_2},
#line 89 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str13, ei_cp1251},
#line 23 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str14, ei_utf8},
#line 92 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str15, ei_cp1252},
#line 97 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str16, ei_cp866},
#line 82 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str17, ei_koi8_r},
    {-1},
#line 69 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str19, ei_iso8859_5},
#line 24 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str20, ei_ucs2},
    {-1},
#line 76 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str22, ei_iso8859_15},
#line 51 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str23, ei_iso8859_1},
#line 70 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str24, ei_iso8859_5},
#line 61 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str25, ei_iso8859_2},
#line 13 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str26, ei_ascii},
#line 77 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str27, ei_iso8859_15},
#line 52 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str28, ei_iso8859_1},
#line 71 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str29, ei_iso8859_5},
#line 62 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str30, ei_iso8859_2},
#line 57 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str31, ei_iso8859_1},
#line 78 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str32, ei_iso8859_15},
#line 65 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str33, ei_iso8859_2},
#line 33 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str34, ei_ucs4},
#line 88 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str35, ei_cp1250},
#line 73 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str36, ei_iso8859_5},
#line 80 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str37, ei_iso8859_15},
#line 83 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str38, ei_koi8_r},
#line 31 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str39, ei_ucs2le},
#line 64 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str40, ei_iso8859_2},
#line 21 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str41, ei_ascii},
#line 40 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str42, ei_utf16le},
#line 38 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str43, ei_utf16},
#line 95 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str44, ei_cp866},
#line 96 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str45, ei_cp866},
#line 37 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str46, ei_ucs4le},
#line 16 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str47, ei_ascii},
#line 100 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str48, ei_local_wchar_t},
#line 84 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str49, ei_koi8_u},
#line 91 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str50, ei_cp1251},
#line 85 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str51, ei_koi8_ru},
#line 47 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str52, ei_ucs2internal},
#line 53 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str53, ei_iso8859_1},
#line 63 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str54, ei_iso8859_2},
#line 41 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str55, ei_utf32},
#line 72 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str56, ei_iso8859_5},
    {-1},
#line 44 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str58, ei_utf7},
#line 49 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str59, ei_ucs4internal},
#line 28 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str60, ei_ucs2be},
    {-1},
#line 19 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str62, ei_ascii},
#line 86 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str63, ei_cp1250},
#line 43 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str64, ei_utf32le},
#line 98 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str65, ei_cp866},
    {-1}, {-1},
#line 29 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str68, ei_ucs2be},
#line 15 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str69, ei_ascii},
#line 27 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str70, ei_ucs2be},
    {-1},
#line 30 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str72, ei_ucs2be},
#line 39 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str73, ei_utf16be},
#line 94 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str74, ei_cp1252},
#line 12 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str75, ei_ascii},
#line 26 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str76, ei_ucs2},
#line 36 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str77, ei_ucs4be},
#line 25 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str78, ei_ucs2},
#line 35 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str79, ei_ucs4},
#line 48 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str80, ei_ucs2swapped},
#line 22 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str81, ei_ascii},
#line 32 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str82, ei_ucs2le},
#line 90 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str83, ei_cp1251},
#line 93 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str84, ei_cp1252},
#line 34 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str85, ei_ucs4},
#line 17 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str86, ei_ascii},
#line 50 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str87, ei_ucs4swapped},
#line 59 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str88, ei_iso8859_1},
#line 67 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str89, ei_iso8859_2},
#line 54 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str90, ei_iso8859_1},
    {-1}, {-1},
#line 14 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str93, ei_ascii},
#line 45 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str94, ei_utf7},
#line 42 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str95, ei_utf32be},
#line 46 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str96, ei_utf7},
#line 20 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str97, ei_ascii},
    {-1},
#line 18 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str99, ei_ascii},
    {-1}, {-1}, {-1}, {-1},
#line 79 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str104, ei_iso8859_15},
    {-1}, {-1},
#line 74 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str107, ei_iso8859_5},
#line 87 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str108, ei_cp1250}
  };

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct alias *
aliases_lookup (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = aliases_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int o = aliases[key].name;
          if (o >= 0)
            {
              register const char *s = o + stringpool;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &aliases[key];
            }
        }
    }
  return 0;
}
