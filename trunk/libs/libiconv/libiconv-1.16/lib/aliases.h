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

#define TOTAL_KEYWORDS 112
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 182
/* maximum key range = 180, duplicates = 0 */

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
      183, 183, 183, 183, 183, 183, 183, 183, 183, 183,
      183, 183, 183, 183, 183, 183, 183, 183, 183, 183,
      183, 183, 183, 183, 183, 183, 183, 183, 183, 183,
      183, 183, 183, 183, 183, 183, 183, 183, 183, 183,
      183, 183, 183, 183, 183,   6, 183, 183,   0,   2,
        3,  41,   7,   0,   4,  18,   0,   0, 183, 183,
      183, 183, 183, 183, 183,   0,  55,   8,  21,   2,
      183,  59, 183,   0, 183,  59,  22,  62,  14,   0,
        8, 183,  30,  65,  15,  17, 183,   4,   1,   0,
      183, 183, 183, 183, 183,  67, 183, 183, 183, 183,
      183, 183, 183, 183, 183, 183, 183, 183, 183, 183,
      183, 183, 183, 183, 183, 183, 183, 183, 183, 183,
      183, 183, 183, 183, 183, 183, 183, 183
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
    char stringpool_str3[sizeof("850")];
    char stringpool_str5[sizeof("ASCII")];
    char stringpool_str6[sizeof("IBM850")];
    char stringpool_str7[sizeof("866")];
    char stringpool_str8[sizeof("IBM819")];
    char stringpool_str9[sizeof("ISO8859-5")];
    char stringpool_str10[sizeof("ISO8859-15")];
    char stringpool_str11[sizeof("ISO8859-1")];
    char stringpool_str12[sizeof("ISO8859-2")];
    char stringpool_str13[sizeof("CP850")];
    char stringpool_str15[sizeof("CP819")];
    char stringpool_str16[sizeof("ISO-8859-5")];
    char stringpool_str17[sizeof("CP1250")];
    char stringpool_str18[sizeof("IBM866")];
    char stringpool_str19[sizeof("ISO-8859-15")];
    char stringpool_str20[sizeof("ISO-8859-1")];
    char stringpool_str21[sizeof("CP1251")];
    char stringpool_str22[sizeof("ISO-8859-2")];
    char stringpool_str23[sizeof("CP1252")];
    char stringpool_str24[sizeof("CN")];
    char stringpool_str25[sizeof("CP866")];
    char stringpool_str26[sizeof("L1")];
    char stringpool_str27[sizeof("L2")];
    char stringpool_str28[sizeof("UTF-8")];
    char stringpool_str34[sizeof("UCS-2")];
    char stringpool_str36[sizeof("ISO-10646-UCS-2")];
    char stringpool_str39[sizeof("UTF-16")];
    char stringpool_str40[sizeof("ISO-10646-UCS-4")];
    char stringpool_str41[sizeof("CSUNICODE")];
    char stringpool_str42[sizeof("UCS-4")];
    char stringpool_str43[sizeof("EUCCN")];
    char stringpool_str44[sizeof("ISO646-CN")];
    char stringpool_str45[sizeof("CSUNICODE11")];
    char stringpool_str46[sizeof("LATIN1")];
    char stringpool_str48[sizeof("LATIN2")];
    char stringpool_str49[sizeof("LATIN-9")];
    char stringpool_str50[sizeof("EUC-CN")];
    char stringpool_str51[sizeof("ISO-IR-58")];
    char stringpool_str52[sizeof("ISO-IR-100")];
    char stringpool_str53[sizeof("CP367")];
    char stringpool_str54[sizeof("ISO-IR-6")];
    char stringpool_str56[sizeof("ISO-IR-101")];
    char stringpool_str59[sizeof("UCS-2LE")];
    char stringpool_str61[sizeof("UTF-16LE")];
    char stringpool_str62[sizeof("CP936")];
    char stringpool_str63[sizeof("UCS-4LE")];
    char stringpool_str64[sizeof("UTF-7")];
    char stringpool_str65[sizeof("CSUNICODE11UTF7")];
    char stringpool_str66[sizeof("ISO-IR-144")];
    char stringpool_str67[sizeof("UNICODE-1-1")];
    char stringpool_str68[sizeof("CYRILLIC")];
    char stringpool_str69[sizeof("ISO-IR-57")];
    char stringpool_str70[sizeof("UCS-2-INTERNAL")];
    char stringpool_str71[sizeof("GB_1988-80")];
    char stringpool_str72[sizeof("CHAR")];
    char stringpool_str73[sizeof("MS-EE")];
    char stringpool_str74[sizeof("UCS-4-INTERNAL")];
    char stringpool_str75[sizeof("CSKOI8R")];
    char stringpool_str76[sizeof("UTF-32")];
    char stringpool_str77[sizeof("ISO_8859-5")];
    char stringpool_str78[sizeof("UNICODELITTLE")];
    char stringpool_str79[sizeof("CSPC850MULTILINGUAL")];
    char stringpool_str80[sizeof("ISO_8859-15")];
    char stringpool_str81[sizeof("ISO_8859-1")];
    char stringpool_str82[sizeof("ISO_8859-5:1988")];
    char stringpool_str83[sizeof("ISO_8859-2")];
    char stringpool_str84[sizeof("US")];
    char stringpool_str85[sizeof("ISO_8859-15:1998")];
    char stringpool_str87[sizeof("IBM367")];
    char stringpool_str88[sizeof("CSASCII")];
    char stringpool_str89[sizeof("UNICODE-1-1-UTF-7")];
    char stringpool_str91[sizeof("CSISO58GB231280")];
    char stringpool_str92[sizeof("UCS-2BE")];
    char stringpool_str94[sizeof("UTF-16BE")];
    char stringpool_str95[sizeof("ISO646-US")];
    char stringpool_str96[sizeof("UCS-4BE")];
    char stringpool_str98[sizeof("US-ASCII")];
    char stringpool_str99[sizeof("UTF-32LE")];
    char stringpool_str100[sizeof("CHINESE")];
    char stringpool_str101[sizeof("CSUCS4")];
    char stringpool_str102[sizeof("ISO_8859-1:1987")];
    char stringpool_str103[sizeof("ISO_8859-2:1987")];
    char stringpool_str105[sizeof("KOI8-U")];
    char stringpool_str106[sizeof("CSISO57GB1988")];
    char stringpool_str107[sizeof("GB18030")];
    char stringpool_str109[sizeof("WINDOWS-1250")];
    char stringpool_str111[sizeof("WINDOWS-1251")];
    char stringpool_str112[sizeof("WINDOWS-1252")];
    char stringpool_str114[sizeof("GB2312")];
    char stringpool_str116[sizeof("MS936")];
    char stringpool_str118[sizeof("GB_2312-80")];
    char stringpool_str120[sizeof("CSGB2312")];
    char stringpool_str121[sizeof("GBK")];
    char stringpool_str122[sizeof("CSISOLATIN1")];
    char stringpool_str123[sizeof("CSISOLATIN2")];
    char stringpool_str129[sizeof("ANSI_X3.4-1968")];
    char stringpool_str130[sizeof("ISO_646.IRV:1991")];
    char stringpool_str131[sizeof("KOI8-R")];
    char stringpool_str132[sizeof("UTF-32BE")];
    char stringpool_str133[sizeof("ANSI_X3.4-1986")];
    char stringpool_str134[sizeof("ISO-IR-203")];
    char stringpool_str135[sizeof("CSISOLATINCYRILLIC")];
    char stringpool_str136[sizeof("KOI8-RU")];
    char stringpool_str138[sizeof("WCHAR_T")];
    char stringpool_str139[sizeof("UCS-2-SWAPPED")];
    char stringpool_str141[sizeof("CSIBM866")];
    char stringpool_str143[sizeof("UCS-4-SWAPPED")];
    char stringpool_str148[sizeof("MS-ANSI")];
    char stringpool_str150[sizeof("WINDOWS-936")];
    char stringpool_str151[sizeof("MS-CYRL")];
    char stringpool_str176[sizeof("UNICODEBIG")];
    char stringpool_str182[sizeof("CN-GB")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "850",
    "ASCII",
    "IBM850",
    "866",
    "IBM819",
    "ISO8859-5",
    "ISO8859-15",
    "ISO8859-1",
    "ISO8859-2",
    "CP850",
    "CP819",
    "ISO-8859-5",
    "CP1250",
    "IBM866",
    "ISO-8859-15",
    "ISO-8859-1",
    "CP1251",
    "ISO-8859-2",
    "CP1252",
    "CN",
    "CP866",
    "L1",
    "L2",
    "UTF-8",
    "UCS-2",
    "ISO-10646-UCS-2",
    "UTF-16",
    "ISO-10646-UCS-4",
    "CSUNICODE",
    "UCS-4",
    "EUCCN",
    "ISO646-CN",
    "CSUNICODE11",
    "LATIN1",
    "LATIN2",
    "LATIN-9",
    "EUC-CN",
    "ISO-IR-58",
    "ISO-IR-100",
    "CP367",
    "ISO-IR-6",
    "ISO-IR-101",
    "UCS-2LE",
    "UTF-16LE",
    "CP936",
    "UCS-4LE",
    "UTF-7",
    "CSUNICODE11UTF7",
    "ISO-IR-144",
    "UNICODE-1-1",
    "CYRILLIC",
    "ISO-IR-57",
    "UCS-2-INTERNAL",
    "GB_1988-80",
    "CHAR",
    "MS-EE",
    "UCS-4-INTERNAL",
    "CSKOI8R",
    "UTF-32",
    "ISO_8859-5",
    "UNICODELITTLE",
    "CSPC850MULTILINGUAL",
    "ISO_8859-15",
    "ISO_8859-1",
    "ISO_8859-5:1988",
    "ISO_8859-2",
    "US",
    "ISO_8859-15:1998",
    "IBM367",
    "CSASCII",
    "UNICODE-1-1-UTF-7",
    "CSISO58GB231280",
    "UCS-2BE",
    "UTF-16BE",
    "ISO646-US",
    "UCS-4BE",
    "US-ASCII",
    "UTF-32LE",
    "CHINESE",
    "CSUCS4",
    "ISO_8859-1:1987",
    "ISO_8859-2:1987",
    "KOI8-U",
    "CSISO57GB1988",
    "GB18030",
    "WINDOWS-1250",
    "WINDOWS-1251",
    "WINDOWS-1252",
    "GB2312",
    "MS936",
    "GB_2312-80",
    "CSGB2312",
    "GBK",
    "CSISOLATIN1",
    "CSISOLATIN2",
    "ANSI_X3.4-1968",
    "ISO_646.IRV:1991",
    "KOI8-R",
    "UTF-32BE",
    "ANSI_X3.4-1986",
    "ISO-IR-203",
    "CSISOLATINCYRILLIC",
    "KOI8-RU",
    "WCHAR_T",
    "UCS-2-SWAPPED",
    "CSIBM866",
    "UCS-4-SWAPPED",
    "MS-ANSI",
    "WINDOWS-936",
    "MS-CYRL",
    "UNICODEBIG",
    "CN-GB"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct alias aliases[] =
  {
    {-1}, {-1}, {-1},
#line 97 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str3, ei_cp850},
    {-1},
#line 13 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str5, ei_ascii},
#line 96 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str6, ei_cp850},
#line 101 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str7, ei_cp866},
#line 56 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str8, ei_iso8859_1},
#line 75 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str9, ei_iso8859_5},
#line 81 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str10, ei_iso8859_15},
#line 60 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str11, ei_iso8859_1},
#line 68 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str12, ei_iso8859_2},
#line 95 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str13, ei_cp850},
    {-1},
#line 55 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str15, ei_iso8859_1},
#line 69 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str16, ei_iso8859_5},
#line 86 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str17, ei_cp1250},
#line 100 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str18, ei_cp866},
#line 76 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str19, ei_iso8859_15},
#line 51 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str20, ei_iso8859_1},
#line 89 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str21, ei_cp1251},
#line 61 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str22, ei_iso8859_2},
#line 92 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str23, ei_cp1252},
#line 106 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str24, ei_iso646_cn},
#line 99 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str25, ei_cp866},
#line 58 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str26, ei_iso8859_1},
#line 66 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str27, ei_iso8859_2},
#line 23 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str28, ei_utf8},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 24 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str34, ei_ucs2},
    {-1},
#line 25 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str36, ei_ucs2},
    {-1}, {-1},
#line 38 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str39, ei_utf16},
#line 34 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str40, ei_ucs4},
#line 26 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str41, ei_ucs2},
#line 33 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str42, ei_ucs4},
#line 113 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str43, ei_euc_cn},
#line 104 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str44, ei_iso646_cn},
#line 30 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str45, ei_ucs2be},
#line 57 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str46, ei_iso8859_1},
    {-1},
#line 65 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str48, ei_iso8859_2},
#line 80 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str49, ei_iso8859_15},
#line 112 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str50, ei_euc_cn},
#line 109 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str51, ei_gb2312},
#line 54 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str52, ei_iso8859_1},
#line 19 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str53, ei_ascii},
#line 16 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str54, ei_ascii},
    {-1},
#line 64 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str56, ei_iso8859_2},
    {-1}, {-1},
#line 31 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str59, ei_ucs2le},
    {-1},
#line 40 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str61, ei_utf16le},
#line 118 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str62, ei_cp936},
#line 37 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str63, ei_ucs4le},
#line 44 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str64, ei_utf7},
#line 46 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str65, ei_utf7},
#line 72 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str66, ei_iso8859_5},
#line 29 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str67, ei_ucs2be},
#line 73 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str68, ei_iso8859_5},
#line 105 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str69, ei_iso646_cn},
#line 47 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str70, ei_ucs2internal},
#line 103 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str71, ei_iso646_cn},
#line 122 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str72, ei_local_char},
#line 88 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str73, ei_cp1250},
#line 49 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str74, ei_ucs4internal},
#line 83 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str75, ei_koi8_r},
#line 41 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str76, ei_utf32},
#line 70 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str77, ei_iso8859_5},
#line 32 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str78, ei_ucs2le},
#line 98 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str79, ei_cp850},
#line 77 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str80, ei_iso8859_15},
#line 52 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str81, ei_iso8859_1},
#line 71 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str82, ei_iso8859_5},
#line 62 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str83, ei_iso8859_2},
#line 21 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str84, ei_ascii},
#line 78 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str85, ei_iso8859_15},
    {-1},
#line 20 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str87, ei_ascii},
#line 22 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str88, ei_ascii},
#line 45 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str89, ei_utf7},
    {-1},
#line 110 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str91, ei_gb2312},
#line 27 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str92, ei_ucs2be},
    {-1},
#line 39 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str94, ei_utf16be},
#line 14 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str95, ei_ascii},
#line 36 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str96, ei_ucs4be},
    {-1},
#line 12 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str98, ei_ascii},
#line 43 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str99, ei_utf32le},
#line 111 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str100, ei_gb2312},
#line 35 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str101, ei_ucs4},
#line 53 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str102, ei_iso8859_1},
#line 63 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str103, ei_iso8859_2},
    {-1},
#line 84 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str105, ei_koi8_u},
#line 107 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str106, ei_iso646_cn},
#line 121 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str107, ei_gb18030},
    {-1},
#line 87 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str109, ei_cp1250},
    {-1},
#line 90 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str111, ei_cp1251},
#line 93 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str112, ei_cp1252},
    {-1},
#line 114 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str114, ei_euc_cn},
    {-1},
#line 119 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str116, ei_cp936},
    {-1},
#line 108 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str118, ei_gb2312},
    {-1},
#line 116 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str120, ei_euc_cn},
#line 117 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str121, ei_ces_gbk},
#line 59 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str122, ei_iso8859_1},
#line 67 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str123, ei_iso8859_2},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 17 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str129, ei_ascii},
#line 15 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str130, ei_ascii},
#line 82 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str131, ei_koi8_r},
#line 42 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str132, ei_utf32be},
#line 18 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str133, ei_ascii},
#line 79 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str134, ei_iso8859_15},
#line 74 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str135, ei_iso8859_5},
#line 85 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str136, ei_koi8_ru},
    {-1},
#line 123 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str138, ei_local_wchar_t},
#line 48 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str139, ei_ucs2swapped},
    {-1},
#line 102 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str141, ei_cp866},
    {-1},
#line 50 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str143, ei_ucs4swapped},
    {-1}, {-1}, {-1}, {-1},
#line 94 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str148, ei_cp1252},
    {-1},
#line 120 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str150, ei_cp936},
#line 91 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str151, ei_cp1251},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 28 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str176, ei_ucs2be},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 115 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str182, ei_euc_cn}
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
