#ifndef TGN_LOCDEF_H
#define TGN_LOCDEF_H

/* Defines for all locales used in the suite.  */

/* POSIX C locale.  */
#define TST_LOC_C	"C"

/* German locale with ISO-8859-1.  */
#define TST_LOC_de	"de_DE.ISO-8859-1"

/* For US we use ANSI_X3.4-1968 (ASCII). Changed in en_US.ISO-8859-1  */
#define TST_LOC_en	"en_US.ISO-8859-1"
#define TST_LOC_enUS	TST_LOC_C

/* NOTE: ja_JP.EUC-JP locale isn't supported into the uClibc!
         UTF-8 is the only multibyte codeset supported. */
/* Japanese locale with EUC-JP. */
#if 0
#define TST_LOC_eucJP	"ja_JP.EUC-JP"
#endif

/* Japanese locale with UTF-8. */
#define TST_LOC_ja_UTF8	"ja_JP.UTF-8"

/* German locale with UTF-8.  */
#define TST_LOC_de_UTF8	"de_DE.UTF-8"

/* End marker - must appear in each table as last entry.  */
#define	TST_LOC_end	"lastEntry"

#endif /* TGN_LOCDEF_H */
