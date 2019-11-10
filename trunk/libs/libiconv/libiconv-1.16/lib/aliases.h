/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -m 10 lib/aliases.gperf  */
/* Computed positions: -k'1,3-7,10,$' */

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

#define TOTAL_KEYWORDS 141
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 8
#define MAX_HASH_VALUE 252
/* maximum key range = 245, duplicates = 0 */

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
      253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253,   1, 253, 253,   3,   4,
        2,  40,   6,   1,  12,  55,   1,  48, 253, 253,
      253, 253, 253, 253, 253,  76,  25,   1,  51,  15,
       83,  45,  20,   1, 253,  43,   9,  54,   8,   2,
       75, 253,  30,   1,   7,   5,   2,  55,   1,   5,
        7, 253, 253, 253, 253,  50, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 253
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
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str8[sizeof("US")];
    char stringpool_str10[sizeof("850")];
    char stringpool_str11[sizeof("CN")];
    char stringpool_str13[sizeof("L2")];
    char stringpool_str14[sizeof("CP850")];
    char stringpool_str15[sizeof("L1")];
    char stringpool_str16[sizeof("UCS-2")];
    char stringpool_str18[sizeof("CP1252")];
    char stringpool_str19[sizeof("ISO-8859-5")];
    char stringpool_str20[sizeof("CP1250")];
    char stringpool_str21[sizeof("ISO-8859-2")];
    char stringpool_str22[sizeof("CP1251")];
    char stringpool_str23[sizeof("ISO-8859-15")];
    char stringpool_str24[sizeof("UCS-4")];
    char stringpool_str25[sizeof("ISO-8859-1")];
    char stringpool_str26[sizeof("CSUCS4")];
    char stringpool_str27[sizeof("CSISO58GB231280")];
    char stringpool_str28[sizeof("866")];
    char stringpool_str29[sizeof("HZ")];
    char stringpool_str30[sizeof("CSISO2022CN")];
    char stringpool_str31[sizeof("ISO-2022-CN")];
    char stringpool_str34[sizeof("ISO-2022-CN-EXT")];
    char stringpool_str35[sizeof("LATIN2")];
    char stringpool_str37[sizeof("CSUNICODE11")];
    char stringpool_str38[sizeof("EUCCN")];
    char stringpool_str39[sizeof("LATIN1")];
    char stringpool_str40[sizeof("EUC-CN")];
    char stringpool_str41[sizeof("ISO-10646-UCS-2")];
    char stringpool_str42[sizeof("CSUNICODE")];
    char stringpool_str43[sizeof("CP866")];
    char stringpool_str44[sizeof("ISO646-US")];
    char stringpool_str45[sizeof("ISO-10646-UCS-4")];
    char stringpool_str46[sizeof("ISO-IR-58")];
    char stringpool_str48[sizeof("ISO-IR-165")];
    char stringpool_str49[sizeof("UCS-2-INTERNAL")];
    char stringpool_str51[sizeof("ISO646-CN")];
    char stringpool_str52[sizeof("ISO-IR-100")];
    char stringpool_str53[sizeof("UCS-4-INTERNAL")];
    char stringpool_str54[sizeof("ISO-IR-101")];
    char stringpool_str55[sizeof("UCS-2LE")];
    char stringpool_str56[sizeof("ISO-IR-6")];
    char stringpool_str58[sizeof("ISO-IR-144")];
    char stringpool_str59[sizeof("UCS-4LE")];
    char stringpool_str60[sizeof("CYRILLIC")];
    char stringpool_str61[sizeof("CP950")];
    char stringpool_str62[sizeof("KOI8-U")];
    char stringpool_str63[sizeof("CHINESE")];
    char stringpool_str64[sizeof("ISO8859-5")];
    char stringpool_str65[sizeof("ISO8859-2")];
    char stringpool_str66[sizeof("ISO8859-15")];
    char stringpool_str67[sizeof("ISO8859-1")];
    char stringpool_str68[sizeof("ISO_8859-5")];
    char stringpool_str69[sizeof("IBM850")];
    char stringpool_str70[sizeof("ISO_8859-2")];
    char stringpool_str71[sizeof("UCS-2BE")];
    char stringpool_str72[sizeof("ISO_8859-15")];
    char stringpool_str73[sizeof("ISO_8859-5:1988")];
    char stringpool_str74[sizeof("ISO_8859-1")];
    char stringpool_str75[sizeof("UCS-4BE")];
    char stringpool_str76[sizeof("BIG5")];
    char stringpool_str77[sizeof("ISO_8859-15:1998")];
    char stringpool_str78[sizeof("BIG-5")];
    char stringpool_str79[sizeof("CSISO57GB1988")];
    char stringpool_str80[sizeof("CSBIG5")];
    char stringpool_str82[sizeof("CN-BIG5")];
    char stringpool_str85[sizeof("ASCII")];
    char stringpool_str89[sizeof("CSASCII")];
    char stringpool_str90[sizeof("CN-GB-ISOIR165")];
    char stringpool_str91[sizeof("UNICODE-1-1")];
    char stringpool_str92[sizeof("CSUNICODE11UTF7")];
    char stringpool_str93[sizeof("KOI8-RU")];
    char stringpool_str94[sizeof("US-ASCII")];
    char stringpool_str96[sizeof("UTF-8")];
    char stringpool_str98[sizeof("IBM866")];
    char stringpool_str100[sizeof("ISO-IR-57")];
    char stringpool_str101[sizeof("GB2312")];
    char stringpool_str102[sizeof("CN-GB")];
    char stringpool_str105[sizeof("MS-EE")];
    char stringpool_str106[sizeof("GB18030")];
    char stringpool_str107[sizeof("CP819")];
    char stringpool_str108[sizeof("HZ-GB-2312")];
    char stringpool_str110[sizeof("UNICODELITTLE")];
    char stringpool_str111[sizeof("CSISOLATIN2")];
    char stringpool_str112[sizeof("KOI8-R")];
    char stringpool_str113[sizeof("CSISOLATIN1")];
    char stringpool_str114[sizeof("CSIBM866")];
    char stringpool_str115[sizeof("CSKOI8R")];
    char stringpool_str116[sizeof("MS-CYRL")];
    char stringpool_str117[sizeof("CSISOLATINCYRILLIC")];
    char stringpool_str118[sizeof("CP936")];
    char stringpool_str119[sizeof("CSPC850MULTILINGUAL")];
    char stringpool_str122[sizeof("KSC_5601")];
    char stringpool_str123[sizeof("UTF-16")];
    char stringpool_str126[sizeof("ISO-IR-203")];
    char stringpool_str127[sizeof("CSGB2312")];
    char stringpool_str128[sizeof("ISO_8859-2:1987")];
    char stringpool_str129[sizeof("LATIN-9")];
    char stringpool_str130[sizeof("ISO_8859-1:1987")];
    char stringpool_str131[sizeof("CSKSC56011987")];
    char stringpool_str133[sizeof("ISO_646.IRV:1991")];
    char stringpool_str134[sizeof("GBK")];
    char stringpool_str137[sizeof("UTF-16LE")];
    char stringpool_str138[sizeof("EUCTW")];
    char stringpool_str139[sizeof("UTF-32")];
    char stringpool_str140[sizeof("EUC-TW")];
    char stringpool_str141[sizeof("CHAR")];
    char stringpool_str142[sizeof("ISO-IR-149")];
    char stringpool_str145[sizeof("BIG5HKSCS")];
    char stringpool_str146[sizeof("CSEUCTW")];
    char stringpool_str147[sizeof("BIG5-HKSCS")];
    char stringpool_str148[sizeof("UNICODE-1-1-UTF-7")];
    char stringpool_str149[sizeof("MS-ANSI")];
    char stringpool_str150[sizeof("UCS-2-SWAPPED")];
    char stringpool_str152[sizeof("BIG5-HKSCS:2008")];
    char stringpool_str153[sizeof("UTF-16BE")];
    char stringpool_str154[sizeof("UCS-4-SWAPPED")];
    char stringpool_str155[sizeof("BIG5-HKSCS:2001")];
    char stringpool_str157[sizeof("BIG5-HKSCS:2004")];
    char stringpool_str159[sizeof("GB_2312-80")];
    char stringpool_str162[sizeof("IBM819")];
    char stringpool_str163[sizeof("UTF-32LE")];
    char stringpool_str165[sizeof("GB_1988-80")];
    char stringpool_str168[sizeof("CP367")];
    char stringpool_str171[sizeof("MS936")];
    char stringpool_str175[sizeof("UNICODEBIG")];
    char stringpool_str179[sizeof("UTF-32BE")];
    char stringpool_str180[sizeof("BIG-FIVE")];
    char stringpool_str185[sizeof("ANSI_X3.4-1968")];
    char stringpool_str186[sizeof("KOREAN")];
    char stringpool_str188[sizeof("WINDOWS-1252")];
    char stringpool_str189[sizeof("WINDOWS-1250")];
    char stringpool_str190[sizeof("WINDOWS-1251")];
    char stringpool_str193[sizeof("BIGFIVE")];
    char stringpool_str196[sizeof("ANSI_X3.4-1986")];
    char stringpool_str199[sizeof("BIG5-HKSCS:1999")];
    char stringpool_str204[sizeof("UTF-7")];
    char stringpool_str220[sizeof("KS_C_5601-1989")];
    char stringpool_str223[sizeof("IBM367")];
    char stringpool_str227[sizeof("KS_C_5601-1987")];
    char stringpool_str235[sizeof("WINDOWS-936")];
    char stringpool_str252[sizeof("WCHAR_T")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "US",
    "850",
    "CN",
    "L2",
    "CP850",
    "L1",
    "UCS-2",
    "CP1252",
    "ISO-8859-5",
    "CP1250",
    "ISO-8859-2",
    "CP1251",
    "ISO-8859-15",
    "UCS-4",
    "ISO-8859-1",
    "CSUCS4",
    "CSISO58GB231280",
    "866",
    "HZ",
    "CSISO2022CN",
    "ISO-2022-CN",
    "ISO-2022-CN-EXT",
    "LATIN2",
    "CSUNICODE11",
    "EUCCN",
    "LATIN1",
    "EUC-CN",
    "ISO-10646-UCS-2",
    "CSUNICODE",
    "CP866",
    "ISO646-US",
    "ISO-10646-UCS-4",
    "ISO-IR-58",
    "ISO-IR-165",
    "UCS-2-INTERNAL",
    "ISO646-CN",
    "ISO-IR-100",
    "UCS-4-INTERNAL",
    "ISO-IR-101",
    "UCS-2LE",
    "ISO-IR-6",
    "ISO-IR-144",
    "UCS-4LE",
    "CYRILLIC",
    "CP950",
    "KOI8-U",
    "CHINESE",
    "ISO8859-5",
    "ISO8859-2",
    "ISO8859-15",
    "ISO8859-1",
    "ISO_8859-5",
    "IBM850",
    "ISO_8859-2",
    "UCS-2BE",
    "ISO_8859-15",
    "ISO_8859-5:1988",
    "ISO_8859-1",
    "UCS-4BE",
    "BIG5",
    "ISO_8859-15:1998",
    "BIG-5",
    "CSISO57GB1988",
    "CSBIG5",
    "CN-BIG5",
    "ASCII",
    "CSASCII",
    "CN-GB-ISOIR165",
    "UNICODE-1-1",
    "CSUNICODE11UTF7",
    "KOI8-RU",
    "US-ASCII",
    "UTF-8",
    "IBM866",
    "ISO-IR-57",
    "GB2312",
    "CN-GB",
    "MS-EE",
    "GB18030",
    "CP819",
    "HZ-GB-2312",
    "UNICODELITTLE",
    "CSISOLATIN2",
    "KOI8-R",
    "CSISOLATIN1",
    "CSIBM866",
    "CSKOI8R",
    "MS-CYRL",
    "CSISOLATINCYRILLIC",
    "CP936",
    "CSPC850MULTILINGUAL",
    "KSC_5601",
    "UTF-16",
    "ISO-IR-203",
    "CSGB2312",
    "ISO_8859-2:1987",
    "LATIN-9",
    "ISO_8859-1:1987",
    "CSKSC56011987",
    "ISO_646.IRV:1991",
    "GBK",
    "UTF-16LE",
    "EUCTW",
    "UTF-32",
    "EUC-TW",
    "CHAR",
    "ISO-IR-149",
    "BIG5HKSCS",
    "CSEUCTW",
    "BIG5-HKSCS",
    "UNICODE-1-1-UTF-7",
    "MS-ANSI",
    "UCS-2-SWAPPED",
    "BIG5-HKSCS:2008",
    "UTF-16BE",
    "UCS-4-SWAPPED",
    "BIG5-HKSCS:2001",
    "BIG5-HKSCS:2004",
    "GB_2312-80",
    "IBM819",
    "UTF-32LE",
    "GB_1988-80",
    "CP367",
    "MS936",
    "UNICODEBIG",
    "UTF-32BE",
    "BIG-FIVE",
    "ANSI_X3.4-1968",
    "KOREAN",
    "WINDOWS-1252",
    "WINDOWS-1250",
    "WINDOWS-1251",
    "BIGFIVE",
    "ANSI_X3.4-1986",
    "BIG5-HKSCS:1999",
    "UTF-7",
    "KS_C_5601-1989",
    "IBM367",
    "KS_C_5601-1987",
    "WINDOWS-936",
    "WCHAR_T"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct alias aliases[] =
  {
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 21 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str8, ei_ascii},
    {-1},
#line 97 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str10, ei_cp850},
#line 106 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str11, ei_iso646_cn},
    {-1},
#line 66 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str13, ei_iso8859_2},
#line 95 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str14, ei_cp850},
#line 58 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str15, ei_iso8859_1},
#line 24 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str16, ei_ucs2},
    {-1},
#line 92 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str18, ei_cp1252},
#line 69 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str19, ei_iso8859_5},
#line 86 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str20, ei_cp1250},
#line 61 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str21, ei_iso8859_2},
#line 89 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str22, ei_cp1251},
#line 76 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str23, ei_iso8859_15},
#line 33 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str24, ei_ucs4},
#line 51 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str25, ei_iso8859_1},
#line 35 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str26, ei_ucs4},
#line 110 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str27, ei_gb2312},
#line 101 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str28, ei_cp866},
#line 133 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str29, ei_hz},
#line 131 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str30, ei_iso2022_cn},
#line 130 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str31, ei_iso2022_cn},
    {-1}, {-1},
#line 132 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str34, ei_iso2022_cn_ext},
#line 65 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str35, ei_iso8859_2},
    {-1},
#line 30 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str37, ei_ucs2be},
#line 121 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str38, ei_euc_cn},
#line 57 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str39, ei_iso8859_1},
#line 120 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str40, ei_euc_cn},
#line 25 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str41, ei_ucs2},
#line 26 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str42, ei_ucs2},
#line 99 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str43, ei_cp866},
#line 14 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str44, ei_ascii},
#line 34 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str45, ei_ucs4},
#line 109 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str46, ei_gb2312},
    {-1},
#line 112 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str48, ei_isoir165},
#line 47 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str49, ei_ucs2internal},
    {-1},
#line 104 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str51, ei_iso646_cn},
#line 54 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str52, ei_iso8859_1},
#line 49 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str53, ei_ucs4internal},
#line 64 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str54, ei_iso8859_2},
#line 31 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str55, ei_ucs2le},
#line 16 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str56, ei_ascii},
    {-1},
#line 72 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str58, ei_iso8859_5},
#line 37 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str59, ei_ucs4le},
#line 73 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str60, ei_iso8859_5},
#line 144 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str61, ei_cp950},
#line 84 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str62, ei_koi8_u},
#line 111 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str63, ei_gb2312},
#line 75 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str64, ei_iso8859_5},
#line 68 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str65, ei_iso8859_2},
#line 81 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str66, ei_iso8859_15},
#line 60 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str67, ei_iso8859_1},
#line 70 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str68, ei_iso8859_5},
#line 96 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str69, ei_cp850},
#line 62 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str70, ei_iso8859_2},
#line 27 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str71, ei_ucs2be},
#line 77 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str72, ei_iso8859_15},
#line 71 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str73, ei_iso8859_5},
#line 52 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str74, ei_iso8859_1},
#line 36 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str75, ei_ucs4be},
#line 138 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str76, ei_ces_big5},
#line 78 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str77, ei_iso8859_15},
#line 139 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str78, ei_ces_big5},
#line 107 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str79, ei_iso646_cn},
#line 143 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str80, ei_ces_big5},
    {-1},
#line 142 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str82, ei_ces_big5},
    {-1}, {-1},
#line 13 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str85, ei_ascii},
    {-1}, {-1}, {-1},
#line 22 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str89, ei_ascii},
#line 113 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str90, ei_isoir165},
#line 29 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str91, ei_ucs2be},
#line 46 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str92, ei_utf7},
#line 85 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str93, ei_koi8_ru},
#line 12 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str94, ei_ascii},
    {-1},
#line 23 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str96, ei_utf8},
    {-1},
#line 100 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str98, ei_cp866},
    {-1},
#line 105 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str100, ei_iso646_cn},
#line 122 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str101, ei_euc_cn},
#line 123 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str102, ei_euc_cn},
    {-1}, {-1},
#line 88 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str105, ei_cp1250},
#line 129 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str106, ei_gb18030},
#line 55 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str107, ei_iso8859_1},
#line 134 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str108, ei_hz},
    {-1},
#line 32 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str110, ei_ucs2le},
#line 67 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str111, ei_iso8859_2},
#line 82 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str112, ei_koi8_r},
#line 59 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str113, ei_iso8859_1},
#line 102 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str114, ei_cp866},
#line 83 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str115, ei_koi8_r},
#line 91 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str116, ei_cp1251},
#line 74 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str117, ei_iso8859_5},
#line 126 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str118, ei_cp936},
#line 98 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str119, ei_cp850},
    {-1}, {-1},
#line 114 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str122, ei_ksc5601},
#line 38 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str123, ei_utf16},
    {-1}, {-1},
#line 79 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str126, ei_iso8859_15},
#line 124 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str127, ei_euc_cn},
#line 63 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str128, ei_iso8859_2},
#line 80 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str129, ei_iso8859_15},
#line 53 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str130, ei_iso8859_1},
#line 118 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str131, ei_ksc5601},
    {-1},
#line 15 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str133, ei_ascii},
#line 125 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str134, ei_ces_gbk},
    {-1}, {-1},
#line 40 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str137, ei_utf16le},
#line 136 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str138, ei_euc_tw},
#line 41 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str139, ei_utf32},
#line 135 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str140, ei_euc_tw},
#line 151 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str141, ei_local_char},
#line 117 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str142, ei_ksc5601},
    {-1}, {-1},
#line 149 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str145, ei_big5hkscs2008},
#line 137 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str146, ei_euc_tw},
#line 148 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str147, ei_big5hkscs2008},
#line 45 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str148, ei_utf7},
#line 94 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str149, ei_cp1252},
#line 48 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str150, ei_ucs2swapped},
    {-1},
#line 150 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str152, ei_big5hkscs2008},
#line 39 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str153, ei_utf16be},
#line 50 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str154, ei_ucs4swapped},
#line 146 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str155, ei_big5hkscs2001},
    {-1},
#line 147 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str157, ei_big5hkscs2004},
    {-1},
#line 108 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str159, ei_gb2312},
    {-1}, {-1},
#line 56 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str162, ei_iso8859_1},
#line 43 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str163, ei_utf32le},
    {-1},
#line 103 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str165, ei_iso646_cn},
    {-1}, {-1},
#line 19 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str168, ei_ascii},
    {-1}, {-1},
#line 127 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str171, ei_cp936},
    {-1}, {-1}, {-1},
#line 28 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str175, ei_ucs2be},
    {-1}, {-1}, {-1},
#line 42 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str179, ei_utf32be},
#line 140 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str180, ei_ces_big5},
    {-1}, {-1}, {-1}, {-1},
#line 17 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str185, ei_ascii},
#line 119 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str186, ei_ksc5601},
    {-1},
#line 93 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str188, ei_cp1252},
#line 87 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str189, ei_cp1250},
#line 90 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str190, ei_cp1251},
    {-1}, {-1},
#line 141 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str193, ei_ces_big5},
    {-1}, {-1},
#line 18 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str196, ei_ascii},
    {-1}, {-1},
#line 145 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str199, ei_big5hkscs1999},
    {-1}, {-1}, {-1}, {-1},
#line 44 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str204, ei_utf7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 116 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str220, ei_ksc5601},
    {-1}, {-1},
#line 20 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str223, ei_ascii},
    {-1}, {-1}, {-1},
#line 115 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str227, ei_ksc5601},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 128 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str235, ei_cp936},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 152 "lib/aliases.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str252, ei_local_wchar_t}
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
