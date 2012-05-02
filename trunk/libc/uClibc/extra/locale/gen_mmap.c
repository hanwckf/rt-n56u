#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#define WANT_DATA
#include "c8tables.h"

/*  #define Cctype_TBL_LEN 328 */
/*  #define Cuplow_TBL_LEN 400 */
/*  #define Cc2wc_TBL_LEN 1448 */
/*  #define Cwc2c_TBL_LEN 3744 */

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

/*  #define WCctype_TBL_LEN		(WCctype_II_LEN + WCctype_TI_LEN + WCctype_UT_LEN) */
/*  #define WCuplow_TBL_LEN		(WCuplow_II_LEN + WCuplow_TI_LEN + WCuplow_UT_LEN) */
/*  #define WCuplow_diff_TBL_LEN (2 * WCuplow_diffs) */
/*  #define WCcomb_TBL_LEN		(WCcomb_II_LEN + WCcomb_TI_LEN + WCcomb_UT_LEN) */

#include "locale_tables.h"

#include "locale_mmap.h"

/*  #undef __PASTE2 */
/*  #define __PASTE2(A,B)		A ## B */
/*  #undef __PASTE3 */
/*  #define __PASTE3(A,B,C)		A ## B ## C */


/*  #define MAGIC_SIZE 64 */

/*  #define COMMON_MMAP(X) \ */
/*  	unsigned char	__PASTE3(lc_,X,_data)[__PASTE3(__lc_,X,_data_LEN)]; */

/*  #define COMMON_MMIDX(X) \ */
/*  	unsigned char	__PASTE3(lc_,X,_rows)[__PASTE3(__lc_,X,_rows_LEN)]; \ */
/*  	uint16_t		__PASTE3(lc_,X,_item_offsets)[__PASTE3(__lc_,X,_item_offsets_LEN)]; \ */
/*  	uint16_t		__PASTE3(lc_,X,_item_idx)[__PASTE3(__lc_,X,_item_idx_LEN)]; \ */

#define WRITE_COMMON_MMAP(X) \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE3(lc_,X,_data)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(__lc_,X,_data_LEN) ; i++) { \
		putc(__PASTE3(__lc_,X,_data)[i], fp); \
	}

#define WRITE_COMMON_MMIDX(X) \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE3(lc_,X,_rows)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(__lc_,X,_rows_LEN) ; i++) { \
		putc(__PASTE3(__lc_,X,_rows)[i], fp); \
	} \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE3(lc_,X,_item_offsets)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(__lc_,X,_item_offsets_LEN) ; i++) { \
		putc( ((unsigned char *) &(__PASTE3(__lc_,X,_item_offsets)[i]))[0], fp); \
		putc( ((unsigned char *) &(__PASTE3(__lc_,X,_item_offsets)[i]))[1], fp); \
	} \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE3(lc_,X,_item_idx)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(__lc_,X,_item_idx_LEN) ; i++) { \
		putc( ((unsigned char *) &(__PASTE3(__lc_,X,_item_idx)[i]))[0], fp); \
		putc( ((unsigned char *) &(__PASTE3(__lc_,X,_item_idx)[i]))[1], fp); \
	}

#define WRITE_WC_DATA(X) \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE2(tblw,X)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(WC,X,_TBL_LEN) ; i++) { \
		putc(__PASTE3(WC,X,_data)[i], fp); \
	}

#define WRITE_WC_I16_DATA(X) \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE2(tblw,X)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(WC,X,_TBL_LEN) ; i++) { \
		putc( ((unsigned char *) &(__PASTE3(WC,X,_data)[i]))[0], fp); \
		putc( ((unsigned char *) &(__PASTE3(WC,X,_data)[i]))[1], fp); \
	}

#define WRITE_C_DATA(X) \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE2(tbl8,X)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(C,X,_TBL_LEN) ; i++) { \
		putc(__PASTE3(C,X,_data)[i], fp); \
	}

#define WRITE_C_U16_DATA(X) \
	fseek(fp, (long) offsetof(__locale_mmap_t, __PASTE2(tbl8,X)), SEEK_SET); \
	for (i=0 ; i < __PASTE3(C,X,_TBL_LEN) ; i++) { \
		putc( ((unsigned char *) &(__PASTE3(C,X,_data)[i]))[0], fp); \
		putc( ((unsigned char *) &(__PASTE3(C,X,_data)[i]))[1], fp); \
	}

/**********************************************************************/

#define COMMON_OFFSETS(X) \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_rows)), \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_item_offsets)), \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_item_idx)), \
	offsetof(__locale_mmap_t, __PASTE3(lc_,X,_data)) \


static const size_t common_tbl_offsets[CATEGORIES*4] = {
	0, 0, 0, 0,					/* ctype */
	COMMON_OFFSETS(numeric),
	COMMON_OFFSETS(monetary),
	COMMON_OFFSETS(time),
	0, 0, 0, 0,					/* collate */
	COMMON_OFFSETS(messages),
};





int main(void)
{
	FILE *fp;
	size_t i;
	unsigned char *p;

	if (!(fp = fopen("locale.mmap", "w"))) {
		printf("error - can't open locale.mmap for writing!");
		return EXIT_FAILURE;
	}

	for (i=0 ; i < sizeof(__locale_mmap_t) ; i++) {
		putc(0, fp);			/* Zero out the file. */
	}

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	WRITE_C_DATA(ctype);
	WRITE_C_DATA(uplow);
#ifdef __WCHAR_ENABLED
	WRITE_C_U16_DATA(c2wc);
	WRITE_C_DATA(wc2c);
	/* translit  */
#endif /* __WCHAR_ENABLED */
#endif /* __CTYPE_HAS_8_BIT_LOCALES */

#ifdef __WCHAR_ENABLED
	WRITE_WC_DATA(ctype);
	WRITE_WC_DATA(uplow);
	WRITE_WC_I16_DATA(uplow_diff);
/* 	WRITE_WC_DATA(comb); */
	/* width?? */
#endif /* __WCHAR_ENABLED */

	WRITE_COMMON_MMAP(numeric);
	WRITE_COMMON_MMAP(monetary);
	WRITE_COMMON_MMAP(time);
	/* TODO -- collate*/
	WRITE_COMMON_MMAP(messages);

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	fseek(fp, (long) offsetof(__locale_mmap_t, codeset_8_bit), SEEK_SET); \
	p = (unsigned char *) codeset_8_bit;
	for (i=0 ; i < sizeof(codeset_8_bit) ; i++) {
		putc(p[i], fp);
	}
#endif /* __CTYPE_HAS_8_BIT_LOCALES */

	WRITE_COMMON_MMIDX(numeric);
	WRITE_COMMON_MMIDX(monetary);
	WRITE_COMMON_MMIDX(time);
	/* TODO -- collate*/
	WRITE_COMMON_MMIDX(messages);

	fseek(fp, (long) offsetof(__locale_mmap_t, lc_common_item_offsets_LEN), SEEK_SET);
	putc(1, fp);				/* ctype -- (codeset) handled specially */
	putc(__lc_numeric_item_offsets_LEN, fp);
	putc(__lc_monetary_item_offsets_LEN, fp);
	putc(__lc_time_item_offsets_LEN, fp);
	putc(0, fp);				/* collate */
	putc(__lc_messages_item_offsets_LEN, fp);

	fseek(fp, (long) offsetof(__locale_mmap_t, lc_common_tbl_offsets), SEEK_SET);
	for (i=0 ; i < sizeof(common_tbl_offsets) ; i++) {
		putc(((unsigned char *)common_tbl_offsets)[i], fp);
	}

#ifdef NUM_LOCALES
	fseek(fp, (long) offsetof(__locale_mmap_t, locales), SEEK_SET);
	for (i=0 ; i < (NUM_LOCALES * WIDTH_LOCALES) ; i++) {
		putc(__locales[i], fp);
	}

	fseek(fp, (long) offsetof(__locale_mmap_t, locale_names5), SEEK_SET);
	for (i=0 ; i < 5 * NUM_LOCALE_NAMES ; i++) {
		putc(__locale_names5[i], fp);
	}

#ifdef LOCALE_AT_MODIFIERS_LENGTH
	fseek(fp, (long) offsetof(__locale_mmap_t, locale_at_modifiers), SEEK_SET);
	for (i=0 ; i < LOCALE_AT_MODIFIERS_LENGTH ; i++) {
		putc(__locale_at_modifiers[i], fp);
	}
#endif /* LOCALE_AT_MODIFIERS_LENGTH */
#endif /* NUM_LOCALES */

	fseek(fp, (long) offsetof(__locale_mmap_t, lc_names), SEEK_SET);
	for (i=0 ; i < lc_names_LEN ; i++) {
		putc(lc_names[i], fp);
	}

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	fseek(fp, (long) offsetof(__locale_mmap_t, codeset_list), SEEK_SET);
	for (i=0 ; i < sizeof(CODESET_LIST) ; i++) {
		putc((unsigned char)(CODESET_LIST[i]), fp);
	}
#endif /* __CTYPE_HAS_8_BIT_LOCALES */


	if (ferror(fp) || (fclose(fp) == EOF)) {
		printf("error - stream in error state or fclose failed!");
		return EXIT_FAILURE;
	}

	printf("sizeof(__locale_mmap_t) = %zd\n", sizeof(__locale_mmap_t));

	return EXIT_SUCCESS;
}

/* TODO:
 * collate data (8-bit weighted single char only)
 * @ mappings!
 * codeset list? yes, since we'll want to be able to inspect them...
 * that means putting some header stuff in magic
 * fix ctype LEN defines in gen_c8tables
 */
