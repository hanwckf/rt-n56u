#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#ifndef __WCHAR_ENABLED
#warning WHOA!!! __WCHAR_ENABLED is not defined! defining it now...
#define __WCHAR_ENABLED
#endif

#define WANT_DATA
#include "c8tables.h"
#ifndef __CTYPE_HAS_8_BIT_LOCALES
#warning __CTYPE_HAS_8_BIT_LOCALES is not defined...
/* #define __CTYPE_HAS_8_BIT_LOCALES */
#endif

/*  #define __LOCALE_DATA_Cctype_TBL_LEN 328 */
/*  #define __LOCALE_DATA_Cuplow_TBL_LEN 400 */
/*  #define __LOCALE_DATA_Cc2wc_TBL_LEN 1448 */
/*  #define __LOCALE_DATA_Cwc2c_TBL_LEN 3744 */

#define WANT_WCctype_data
#define WANT_WCuplow_data
#define WANT_WCuplow_diff_data
/* #define WANT_WCcomb_data */
/*  #define WANT_WCwidth_data */
#include "wctables.h"
#undef WANT_WCctype_data
#undef WANT_WCuplow_data
#undef WANT_WCuplow_diff_data
/* #undef WANT_WCcomb_data */
/*  #undef WANT_WCwidth_data */

 #define __LOCALE_DATA_WCctype_TBL_LEN		(__LOCALE_DATA_WCctype_II_LEN + __LOCALE_DATA_WCctype_TI_LEN + __LOCALE_DATA_WCctype_UT_LEN)
 #define __LOCALE_DATA_WCuplow_TBL_LEN		(__LOCALE_DATA_WCuplow_II_LEN + __LOCALE_DATA_WCuplow_TI_LEN + __LOCALE_DATA_WCuplow_UT_LEN)
 #define __LOCALE_DATA_WCuplow_diff_TBL_LEN (2 * __LOCALE_DATA_WCuplow_diffs)
/*  #define WCcomb_TBL_LEN		(WCcomb_II_LEN + WCcomb_TI_LEN + WCcomb_UT_LEN) */

#include "locale_collate.h"
#include "locale_tables.h"

#include "locale_mmap.h"

/*  #undef __PASTE2 */
/*  #define __PASTE2(A,B)		A ## B */
/*  #undef __PASTE3 */
/*  #define __PASTE3(A,B,C)		A ## B ## C */


/*  #define __LOCALE_DATA_MAGIC_SIZE 64 */

/*  #define COMMON_MMAP(X) \ */
/*  	unsigned char	__PASTE3(lc_,X,_data)[__PASTE3(__lc_,X,_data_LEN)]; */

/*  #define COMMON_MMIDX(X) \ */
/*  	unsigned char	__PASTE3(lc_,X,_rows)[__PASTE3(__lc_,X,_rows_LEN)]; \ */
/*  	uint16_t		__PASTE3(lc_,X,_item_offsets)[__PASTE3(__lc_,X,_item_offsets_LEN)]; \ */
/*  	uint16_t		__PASTE3(lc_,X,_item_idx)[__PASTE3(__lc_,X,_item_idx_LEN)]; \ */

/* ---------------------------------------------------------------------- */

#define COMMON_OFFSETS(X) \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_rows)), \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_item_offsets)), \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_item_idx)), \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_data)) \


static const size_t common_tbl_offsets[__LOCALE_DATA_CATEGORIES*4] = {
	COMMON_OFFSETS(ctype),
	COMMON_OFFSETS(numeric),
	COMMON_OFFSETS(monetary),
	COMMON_OFFSETS(time),
	0, 0, 0, 0,					/* collate */
	COMMON_OFFSETS(messages),
};


void out_uc(FILE *f, const unsigned char *p, size_t n, char *comment)
{
	size_t i;

	fprintf(f, "{\t/* %s */", comment);
	for (i = 0 ; i < n ; i++) {
		if (!(i & 7)) {
			fprintf(f, "\n\t");
		}
		if (p[i]) {
			fprintf(f, "%#04x, ", p[i]);
		} else {
			fprintf(f, "%#4x, ", p[i]);
		}
	}
	fprintf(f, "\n},\n");
}

void out_u16(FILE *f, const uint16_t *p, size_t n, char *comment)
{
	size_t i;

	fprintf(f, "{\t/* %s */", comment);
	for (i = 0 ; i < n ; i++) {
		if (!(i & 7)) {
			fprintf(f, "\n\t");
		}
		if (p[i]) {
			fprintf(f, "%#06x, ", p[i]);
		} else {
			fprintf(f, "%#6x, ", p[i]);
		}
	}
	fprintf(f, "\n},\n");
}

void out_i16(FILE *f, const int16_t *p, size_t n, char *comment)
{
	size_t i;

	fprintf(f, "{\t/* %s */", comment);
	for (i = 0 ; i < n ; i++) {
		if (!(i & 7)) {
			fprintf(f, "\n\t");
		}
		fprintf(f, "%6d, ", p[i]);
	}
	fprintf(f, "\n},\n");
}

void out_size_t(FILE *f, const size_t *p, size_t n, char *comment)
{
	size_t i;

	fprintf(f, "{\t/* %s */", comment);
	for (i = 0 ; i < n ; i++) {
		if (!(i & 3)) {
			fprintf(f, "\n\t");
		}
		if (p[i]) {
			fprintf(f, "%#010zx, ", p[i]);
		} else {
			fprintf(f, "%#10zx, ", p[i]);
		}
	}
	fprintf(f, "\n},\n");
}


int main(void)
{
	FILE *lso;					/* static object */
	int i;
#ifdef __LOCALE_DATA_MAGIC_SIZE
	unsigned char magic[__LOCALE_DATA_MAGIC_SIZE];

	memset(magic, 0, __LOCALE_DATA_MAGIC_SIZE);
#endif /* __LOCALE_DATA_MAGIC_SIZE */

	if (!(lso = fopen("locale_data.c", "w"))) {
		printf("can't open locale_data.c!\n");
		return EXIT_FAILURE;
	}

	fprintf(lso,
			"#include <stddef.h>\n"
			"#include <stdint.h>\n"
/* 			"#define __CTYPE_HAS_8_BIT_LOCALES\n" */
			"#ifndef __WCHAR_ENABLED\n"
			"#error __WCHAR_ENABLED not defined\n"
			"#endif\n"
			"#include \"c8tables.h\"\n"
			"#include \"wctables.h\"\n"
			"#include \"lt_defines.h\"\n"
			"#include \"locale_mmap.h\"\n\n"
			"static const __locale_mmap_t locale_mmap = {\n\n"
			);
#ifdef __LOCALE_DATA_MAGIC_SIZE
	out_uc(lso, magic, __LOCALE_DATA_MAGIC_SIZE, "magic");
#endif /* __LOCALE_DATA_MAGIC_SIZE */
#ifdef __CTYPE_HAS_8_BIT_LOCALES
	out_uc(lso, __LOCALE_DATA_Cctype_data, __LOCALE_DATA_Cctype_TBL_LEN, "tbl8ctype");
	out_uc(lso, __LOCALE_DATA_Cuplow_data, __LOCALE_DATA_Cuplow_TBL_LEN, "tbl8uplow");
#ifdef __WCHAR_ENABLED
	out_u16(lso, __LOCALE_DATA_Cc2wc_data, __LOCALE_DATA_Cc2wc_TBL_LEN, "tbl8c2wc");
	out_uc(lso, __LOCALE_DATA_Cwc2c_data, __LOCALE_DATA_Cwc2c_TBL_LEN, "tbl8wc2c");
	/* translit  */
#endif /* __WCHAR_ENABLED */
#endif /* __CTYPE_HAS_8_BIT_LOCALES */
#ifdef __WCHAR_ENABLED
	out_uc(lso, __LOCALE_DATA_WCctype_data, __LOCALE_DATA_WCctype_TBL_LEN, "tblwctype");
	out_uc(lso, __LOCALE_DATA_WCuplow_data, __LOCALE_DATA_WCuplow_TBL_LEN, "tblwuplow");
	out_i16(lso, __LOCALE_DATA_WCuplow_diff_data, __LOCALE_DATA_WCuplow_diff_TBL_LEN, "tblwuplow_diff");
/* 	const unsigned char tblwcomb[WCcomb_TBL_LEN]; */
	/* width?? */
#endif /* __WCHAR_ENABLED */
	out_uc(lso, __lc_ctype_data, __lc_ctype_data_LEN, "lc_ctype_data");
	out_uc(lso, __lc_numeric_data, __lc_numeric_data_LEN, "lc_numeric_data");
	out_uc(lso, __lc_monetary_data, __lc_monetary_data_LEN, "lc_monetary_data");
	out_uc(lso, __lc_time_data, __lc_time_data_LEN, "lc_time_data");
	/* TODO -- collate*/
	out_uc(lso, __lc_messages_data, __lc_messages_data_LEN, "lc_messages_data");

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	fprintf(lso, "{ /* codeset_8_bit array */\n");
	for (i = 0 ; i < __LOCALE_DATA_NUM_CODESETS ; i++) {
		fprintf(lso, "{ /* codeset_8_bit[%d] */\n", i);
		out_uc(lso, codeset_8_bit[i].idx8ctype, __LOCALE_DATA_Cctype_IDX_LEN, "idx8ctype");
		out_uc(lso, codeset_8_bit[i].idx8uplow, __LOCALE_DATA_Cuplow_IDX_LEN, "idx8uplow");
		out_uc(lso, codeset_8_bit[i].idx8c2wc, __LOCALE_DATA_Cc2wc_IDX_LEN, "idx8c2wc");
		out_uc(lso, codeset_8_bit[i].idx8wc2c, __LOCALE_DATA_Cwc2c_II_LEN, "idx8wc2c");
		fprintf(lso, "},\n");
	}
	fprintf(lso, "},\n");
#endif /* __CTYPE_HAS_8_BIT_LOCALES */

	out_uc(lso, __lc_ctype_rows, __lc_ctype_rows_LEN, "lc_ctype_rows");
	out_u16(lso, __lc_ctype_item_offsets, __lc_ctype_item_offsets_LEN, "lc_ctype_item_offsets");
	out_u16(lso, __lc_ctype_item_idx, __lc_ctype_item_idx_LEN, "lc_ctype_item_idx");

	out_uc(lso, __lc_numeric_rows, __lc_numeric_rows_LEN, "lc_numeric_rows");
	out_u16(lso, __lc_numeric_item_offsets, __lc_numeric_item_offsets_LEN, "lc_numeric_item_offsets");
	out_u16(lso, __lc_numeric_item_idx, __lc_numeric_item_idx_LEN, "lc_numeric_item_idx");

	out_uc(lso, __lc_monetary_rows, __lc_monetary_rows_LEN, "lc_monetary_rows");
	out_u16(lso, __lc_monetary_item_offsets, __lc_monetary_item_offsets_LEN, "lc_monetary_item_offsets");
	out_u16(lso, __lc_monetary_item_idx, __lc_monetary_item_idx_LEN, "lc_monetary_item_idx");

	out_uc(lso, __lc_time_rows, __lc_time_rows_LEN, "lc_time_rows");
	out_u16(lso, __lc_time_item_offsets, __lc_time_item_offsets_LEN, "lc_time_item_offsets");
	out_u16(lso, __lc_time_item_idx, __lc_time_item_idx_LEN, "lc_time_item_idx");

	out_uc(lso, __lc_messages_rows, __lc_messages_rows_LEN, "lc_messages_rows");
	out_u16(lso, __lc_messages_item_offsets, __lc_messages_item_offsets_LEN, "lc_messages_item_offsets");
	out_u16(lso, __lc_messages_item_idx, __lc_messages_item_idx_LEN, "lc_messages_item_idx");

	/* collate should be last*/
	assert(sizeof(__locale_collate_tbl)/sizeof(__locale_collate_tbl[0]) == __lc_collate_data_LEN) ;
	out_u16(lso, __locale_collate_tbl, __lc_collate_data_LEN, "collate_data");
	

	{
		unsigned char co_buf[__LOCALE_DATA_CATEGORIES] = {
			__lc_ctype_item_offsets_LEN,
			__lc_numeric_item_offsets_LEN,
			__lc_monetary_item_offsets_LEN,
			__lc_time_item_offsets_LEN,
			0,
			__lc_messages_item_offsets_LEN
		};
		out_uc(lso, co_buf, __LOCALE_DATA_CATEGORIES, "lc_common_item_offsets_LEN");
	}
	
	out_size_t(lso, common_tbl_offsets, __LOCALE_DATA_CATEGORIES * 4, "lc_common_tbl_offsets");
	/* offsets from start of locale_mmap_t */
	/* rows, item_offsets, item_idx, data */

#ifdef __LOCALE_DATA_NUM_LOCALES
	out_uc(lso, __locales, __LOCALE_DATA_NUM_LOCALES * __LOCALE_DATA_WIDTH_LOCALES, "locales");
	out_uc(lso, __locale_names5, 5 * __LOCALE_DATA_NUM_LOCALE_NAMES, "locale_names5");
#ifdef __LOCALE_DATA_AT_MODIFIERS_LENGTH
	out_uc(lso, __locale_at_modifiers, __LOCALE_DATA_AT_MODIFIERS_LENGTH, "locale_at_modifiers");
#else
#error __LOCALE_DATA_AT_MODIFIERS_LENGTH not defined!
#endif /* __LOCALE_DATA_AT_MODIFIERS_LENGTH */
#endif /* __LOCALE_DATA_NUM_LOCALES */

	out_uc(lso, lc_names, __lc_names_LEN, "lc_names");
#ifdef __CTYPE_HAS_8_BIT_LOCALES
	out_uc(lso, (const unsigned char*) __LOCALE_DATA_CODESET_LIST, sizeof(__LOCALE_DATA_CODESET_LIST), "codeset_list");
#endif /* __CTYPE_HAS_8_BIT_LOCALES */

	fprintf(lso,
			"\n};\n\n"
			"const __locale_mmap_t *__locale_mmap = &locale_mmap;\n\n"
			);

	if (ferror(lso) || fclose(lso)) {
		printf("error writing!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/* ---------------------------------------------------------------------- */

/* TODO:
 * collate data (8-bit weighted single char only)
 * @ mappings!
 * codeset list? yes, since we'll want to be able to inspect them...
 * that means putting some header stuff in magic
 * fix ctype LEN defines in gen_c8tables
 */
