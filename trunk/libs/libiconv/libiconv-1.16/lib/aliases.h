/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -m 10 lib/aliases.gperf  */
/* Computed positions: -k'1,4-7,10,$' */

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
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "lib/aliases.gperf"
struct alias { int name; unsigned int encoding_index; };

#define TOTAL_KEYWORDS 95
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 10
#define MAX_HASH_VALUE 154
/* maximum key range = 145, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
aliases_hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
      155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
      155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
      155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
      155, 155, 155, 155, 155,   4, 155, 155,   6,   5,
        4,  31,  17,   4,   4,  28,   5,  45, 155, 155,
      155, 155, 155, 155, 155,   5,  25,   4,  14,   5,
      155,  34, 155,   4, 155,   6,   4,  67,  11,  38,
       32, 155,  32,  46,  14,  15, 155,   4,   5,  12,
      155, 155, 155, 155, 155,  53, 155, 155, 155, 155,
      155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
      155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
      155, 155, 155, 155, 155, 155, 155, 155
    };
  register unsigned int hval = len;

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
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str10[sizeof("L2")];
    char stringpool_str11[sizeof("L1")];
    char stringpool_str12[sizeof("866")];
    char stringpool_str14[sizeof("850")];
    char stringpool_str17[sizeof("CN")];
    char stringpool_str21[sizeof("CP866")];
    char stringpool_str22[sizeof("ASCII")];
    char stringpool_str25[sizeof("CP850")];
    char stringpool_str26[sizeof("CP1252")];
    char stringpool_str27[sizeof("IBM866")];
    char stringpool_str28[sizeof("CP1251")];
    char stringpool_str30[sizeof("CP1250")];
    char stringpool_str31[sizeof("IBM850")];
    char stringpool_str32[sizeof("UCS-2")];
    char stringpool_str33[sizeof("LATIN2")];
    char stringpool_str34[sizeof("UTF-8")];
    char stringpool_str35[sizeof("LATIN1")];
    char stringpool_str36[sizeof("EUCCN")];
    char stringpool_str38[sizeof("UTF-16")];
    char stringpool_str40[sizeof("ISO-8859-2")];
    char stringpool_str41[sizeof("EUC-CN")];
    char stringpool_str42[sizeof("ISO-8859-1")];
    char stringpool_str43[sizeof("GBK")];
    char stringpool_str44[sizeof("UCS-2LE")];
    char stringpool_str45[sizeof("UTF-16LE")];
    char stringpool_str46[sizeof("ISO-10646-UCS-2")];
    char stringpool_str48[sizeof("CP936")];
    char stringpool_str50[sizeof("CSPC850MULTILINGUAL")];
    char stringpool_str53[sizeof("ISO646-CN")];
    char stringpool_str54[sizeof("UCS-2-INTERNAL")];
    char stringpool_str57[sizeof("UCS-4LE")];
    char stringpool_str58[sizeof("UCS-4")];
    char stringpool_str59[sizeof("ISO-10646-UCS-4")];
    char stringpool_str60[sizeof("ISO-IR-6")];
    char stringpool_str62[sizeof("ISO-IR-58")];
    char stringpool_str63[sizeof("US")];
    char stringpool_str64[sizeof("UTF-32")];
    char stringpool_str65[sizeof("UCS-2BE")];
    char stringpool_str66[sizeof("UTF-16BE")];
    char stringpool_str67[sizeof("UCS-4-INTERNAL")];
    char stringpool_str68[sizeof("ISO-IR-101")];
    char stringpool_str69[sizeof("CP367")];
    char stringpool_str70[sizeof("ISO-IR-100")];
    char stringpool_str71[sizeof("UTF-32LE")];
    char stringpool_str72[sizeof("CHAR")];
    char stringpool_str73[sizeof("CSASCII")];
    char stringpool_str75[sizeof("CSUNICODE")];
    char stringpool_str76[sizeof("ISO8859-2")];
    char stringpool_str77[sizeof("ISO8859-1")];
    char stringpool_str78[sizeof("UCS-4BE")];
    char stringpool_str80[sizeof("UTF-7")];
    char stringpool_str81[sizeof("CSGB2312")];
    char stringpool_str82[sizeof("CSUNICODE11")];
    char stringpool_str83[sizeof("CHINESE")];
    char stringpool_str84[sizeof("GB2312")];
    char stringpool_str85[sizeof("ISO-IR-57")];
    char stringpool_str86[sizeof("US-ASCII")];
    char stringpool_str87[sizeof("MS-EE")];
    char stringpool_str88[sizeof("ISO646-US")];
    char stringpool_str89[sizeof("ISO_8859-2")];
    char stringpool_str91[sizeof("ISO_8859-1")];
    char stringpool_str92[sizeof("UTF-32BE")];
    char stringpool_str93[sizeof("CN-GB")];
    char stringpool_str94[sizeof("CSUCS4")];
    char stringpool_str95[sizeof("GB18030")];
    char stringpool_str96[sizeof("UNICODE-1-1")];
    char stringpool_str100[sizeof("GB_2312-80")];
    char stringpool_str101[sizeof("IBM367")];
    char stringpool_str104[sizeof("CP819")];
    char stringpool_str108[sizeof("UNICODELITTLE")];
    char stringpool_str109[sizeof("CSUNICODE11UTF7")];
    char stringpool_str110[sizeof("IBM819")];
    char stringpool_str111[sizeof("MS936")];
    char stringpool_str116[sizeof("GB_1988-80")];
    char stringpool_str117[sizeof("CSIBM866")];
    char stringpool_str118[sizeof("ISO_8859-2:1987")];
    char stringpool_str119[sizeof("ISO_8859-1:1987")];
    char stringpool_str120[sizeof("ANSI_X3.4-1986")];
    char stringpool_str121[sizeof("ANSI_X3.4-1968")];
    char stringpool_str122[sizeof("CSISO58GB231280")];
    char stringpool_str123[sizeof("CSISOLATIN2")];
    char stringpool_str124[sizeof("CSISOLATIN1")];
    char stringpool_str125[sizeof("UNICODE-1-1-UTF-7")];
    char stringpool_str126[sizeof("WINDOWS-1252")];
    char stringpool_str127[sizeof("WINDOWS-1251")];
    char stringpool_str128[sizeof("WINDOWS-1250")];
    char stringpool_str129[sizeof("WCHAR_T")];
    char stringpool_str130[sizeof("MS-CYRL")];
    char stringpool_str132[sizeof("UCS-2-SWAPPED")];
    char stringpool_str135[sizeof("ISO_646.IRV:1991")];
    char stringpool_str143[sizeof("CSISO57GB1988")];
    char stringpool_str144[sizeof("MS-ANSI")];
    char stringpool_str145[sizeof("UCS-4-SWAPPED")];
    char stringpool_str152[sizeof("WINDOWS-936")];
    char stringpool_str154[sizeof("UNICODEBIG")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "L2",
    "L1",
    "866",
    "850",
    "CN",
    "CP866",
    "ASCII",
    "CP850",
    "CP1252",
    "IBM866",
    "CP1251",
    "CP1250",
    "IBM850",
    "UCS-2",
    "LATIN2",
    "UTF-8",
    "LATIN1",
    "EUCCN",
    "UTF-16",
    "ISO-8859-2",
    "EUC-CN",
    "ISO-8859-1",
    "GBK",
    "UCS-2LE",
    "UTF-16LE",
    "ISO-10646-UCS-2",
    "CP936",
    "CSPC850MULTILINGUAL",
    "ISO646-CN",
    "UCS-2-INTERNAL",
    "UCS-4LE",
    "UCS-4",
    "ISO-10646-UCS-4",
    "ISO-IR-6",
    "ISO-IR-58",
    "US",
    "UTF-32",
    "UCS-2BE",
    "UTF-16BE",
    "UCS-4-INTERNAL",
    "ISO-IR-101",
    "CP367",
    "ISO-IR-100",
    "UTF-32LE",
    "CHAR",
    "CSASCII",
    "CSUNICODE",
    "ISO8859-2",
    "ISO8859-1",
    "UCS-4BE",
    "UTF-7",
    "CSGB2312",
    "CSUNICODE11",
    "CHINESE",
    "GB2312",
    "ISO-IR-57",
    "US-ASCII",
    "MS-EE",
    "ISO646-US",
    "ISO_8859-2",
    "ISO_8859-1",
    "UTF-32BE",
    "CN-GB",
    "CSUCS4",
    "GB18030",
    "UNICODE-1-1",
    "GB_2312-80",
    "IBM367",
    "CP819",
    "UNICODELITTLE",
    "CSUNICODE11UTF7",
    "IBM819",
    "MS936",
    "GB_1988-80",
    "CSIBM866",
    "ISO_8859-2:1987",
    "ISO_8859-1:1987",
    "ANSI_X3.4-1986",
    "ANSI_X3.4-1968",
    "CSISO58GB231280",
    "CSISOLATIN2",
    "CSISOLATIN1",
    "UNICODE-1-1-UTF-7",
    "WINDOWS-1252",
    "WINDOWS-1251",
    "WINDOWS-1250",
    "WCHAR_T",
    "MS-CYRL",
    "UCS-2-SWAPPED",
    "ISO_646.IRV:1991",
    "CSISO57GB1988",
    "MS-ANSI",
    "UCS-4-SWAPPED",
    "WINDOWS-936",
    "UNICODEBIG"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct alias aliases[] =
  {
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 66 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str10, ei_iso8859_2},
#line 58 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str11, ei_iso8859_1},
#line 84 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str12, ei_cp866},
    {-1},
#line 80 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str14, ei_cp850},
    {-1}, {-1},
#line 89 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str17, ei_iso646_cn},
    {-1}, {-1}, {-1},
#line 82 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str21, ei_cp866},
#line 13 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str22, ei_ascii},
    {-1}, {-1},
#line 78 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str25, ei_cp850},
#line 75 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str26, ei_cp1252},
#line 83 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str27, ei_cp866},
#line 72 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str28, ei_cp1251},
    {-1},
#line 69 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str30, ei_cp1250},
#line 79 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str31, ei_cp850},
#line 24 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str32, ei_ucs2},
#line 65 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str33, ei_iso8859_2},
#line 23 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str34, ei_utf8},
#line 57 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str35, ei_iso8859_1},
#line 96 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str36, ei_euc_cn},
    {-1},
#line 38 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str38, ei_utf16},
    {-1},
#line 61 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str40, ei_iso8859_2},
#line 95 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str41, ei_euc_cn},
#line 51 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str42, ei_iso8859_1},
#line 100 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str43, ei_ces_gbk},
#line 31 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str44, ei_ucs2le},
#line 40 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str45, ei_utf16le},
#line 25 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str46, ei_ucs2},
    {-1},
#line 101 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str48, ei_cp936},
    {-1},
#line 81 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str50, ei_cp850},
    {-1}, {-1},
#line 87 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str53, ei_iso646_cn},
#line 47 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str54, ei_ucs2internal},
    {-1}, {-1},
#line 37 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str57, ei_ucs4le},
#line 33 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str58, ei_ucs4},
#line 34 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str59, ei_ucs4},
#line 16 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str60, ei_ascii},
    {-1},
#line 92 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str62, ei_gb2312},
#line 21 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str63, ei_ascii},
#line 41 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str64, ei_utf32},
#line 27 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str65, ei_ucs2be},
#line 39 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str66, ei_utf16be},
#line 49 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str67, ei_ucs4internal},
#line 64 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str68, ei_iso8859_2},
#line 19 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str69, ei_ascii},
#line 54 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str70, ei_iso8859_1},
#line 43 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str71, ei_utf32le},
#line 105 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str72, ei_local_char},
#line 22 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str73, ei_ascii},
    {-1},
#line 26 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str75, ei_ucs2},
#line 68 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str76, ei_iso8859_2},
#line 60 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str77, ei_iso8859_1},
#line 36 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str78, ei_ucs4be},
    {-1},
#line 44 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str80, ei_utf7},
#line 99 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str81, ei_euc_cn},
#line 30 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str82, ei_ucs2be},
#line 94 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str83, ei_gb2312},
#line 97 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str84, ei_euc_cn},
#line 88 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str85, ei_iso646_cn},
#line 12 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str86, ei_ascii},
#line 71 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str87, ei_cp1250},
#line 14 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str88, ei_ascii},
#line 62 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str89, ei_iso8859_2},
    {-1},
#line 52 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str91, ei_iso8859_1},
#line 42 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str92, ei_utf32be},
#line 98 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str93, ei_euc_cn},
#line 35 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str94, ei_ucs4},
#line 104 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str95, ei_gb18030},
#line 29 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str96, ei_ucs2be},
    {-1}, {-1}, {-1},
#line 91 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str100, ei_gb2312},
#line 20 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str101, ei_ascii},
    {-1}, {-1},
#line 55 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str104, ei_iso8859_1},
    {-1}, {-1}, {-1},
#line 32 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str108, ei_ucs2le},
#line 46 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str109, ei_utf7},
#line 56 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str110, ei_iso8859_1},
#line 102 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str111, ei_cp936},
    {-1}, {-1}, {-1}, {-1},
#line 86 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str116, ei_iso646_cn},
#line 85 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str117, ei_cp866},
#line 63 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str118, ei_iso8859_2},
#line 53 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str119, ei_iso8859_1},
#line 18 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str120, ei_ascii},
#line 17 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str121, ei_ascii},
#line 93 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str122, ei_gb2312},
#line 67 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str123, ei_iso8859_2},
#line 59 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str124, ei_iso8859_1},
#line 45 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str125, ei_utf7},
#line 76 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str126, ei_cp1252},
#line 73 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str127, ei_cp1251},
#line 70 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str128, ei_cp1250},
#line 106 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str129, ei_local_wchar_t},
#line 74 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str130, ei_cp1251},
    {-1},
#line 48 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str132, ei_ucs2swapped},
    {-1}, {-1},
#line 15 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str135, ei_ascii},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 90 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str143, ei_iso646_cn},
#line 77 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str144, ei_cp1252},
#line 50 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str145, ei_ucs4swapped},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 103 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str152, ei_cp936},
    {-1},
#line 28 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str154, ei_ucs2be}
  };

const struct alias *
aliases_lookup (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = aliases_hash (str, len);

      if (key <= MAX_HASH_VALUE)
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
