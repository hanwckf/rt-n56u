/* #define __LOCALE_DATA_MAGIC_SIZE 64 */
#ifndef __WCHAR_ENABLED
#if 0
#warning WHOA!!! __WCHAR_ENABLED is not defined! defining it now...
#endif
#define __WCHAR_ENABLED
#endif

/* TODO - fix */
#ifdef __WCHAR_ENABLED
#define __LOCALE_DATA_WCctype_TBL_LEN      (__LOCALE_DATA_WCctype_II_LEN + __LOCALE_DATA_WCctype_TI_LEN + __LOCALE_DATA_WCctype_UT_LEN)
#define __LOCALE_DATA_WCuplow_TBL_LEN      (__LOCALE_DATA_WCuplow_II_LEN + __LOCALE_DATA_WCuplow_TI_LEN + __LOCALE_DATA_WCuplow_UT_LEN)
#define __LOCALE_DATA_WCuplow_diff_TBL_LEN (2 * __LOCALE_DATA_WCuplow_diffs)
/* #define WCcomb_TBL_LEN		(WCcomb_II_LEN + WCcomb_TI_LEN + WCcomb_UT_LEN) */
#endif

#undef __PASTE2
#define __PASTE2(A,B)   A ## B
#undef __PASTE3
#define __PASTE3(A,B,C) A ## B ## C

#define __LOCALE_DATA_COMMON_MMAP(X) \
	unsigned char   __PASTE3(lc_,X,_data)[__PASTE3(__lc_,X,_data_LEN)];

#define __LOCALE_DATA_COMMON_MMIDX(X) \
	unsigned char   __PASTE3(lc_,X,_rows)[__PASTE3(__lc_,X,_rows_LEN)]; \
	uint16_t        __PASTE3(lc_,X,_item_offsets)[__PASTE3(__lc_,X,_item_offsets_LEN)]; \
	uint16_t        __PASTE3(lc_,X,_item_idx)[__PASTE3(__lc_,X,_item_idx_LEN)]; \


typedef struct {
#ifdef __LOCALE_DATA_MAGIC_SIZE
	unsigned char magic[__LOCALE_DATA_MAGIC_SIZE];
#endif

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	const unsigned char tbl8ctype[__LOCALE_DATA_Cctype_TBL_LEN];
	const unsigned char tbl8uplow[__LOCALE_DATA_Cuplow_TBL_LEN];
#ifdef __WCHAR_ENABLED
	const uint16_t tbl8c2wc[__LOCALE_DATA_Cc2wc_TBL_LEN]; /* char > 0x7f to wide char */
	const unsigned char tbl8wc2c[__LOCALE_DATA_Cwc2c_TBL_LEN];
	/* translit  */
#endif /* __WCHAR_ENABLED */
#endif /* __CTYPE_HAS_8_BIT_LOCALES */
#ifdef __WCHAR_ENABLED
	const unsigned char tblwctype[__LOCALE_DATA_WCctype_TBL_LEN];
	const unsigned char tblwuplow[__LOCALE_DATA_WCuplow_TBL_LEN];
	const int16_t tblwuplow_diff[__LOCALE_DATA_WCuplow_diff_TBL_LEN];
/* 	const unsigned char tblwcomb[WCcomb_TBL_LEN]; */
	/* width?? */
#endif

	__LOCALE_DATA_COMMON_MMAP(ctype)
	__LOCALE_DATA_COMMON_MMAP(numeric)
	__LOCALE_DATA_COMMON_MMAP(monetary)
	__LOCALE_DATA_COMMON_MMAP(time)
	/* collate is different */
	__LOCALE_DATA_COMMON_MMAP(messages)


#ifdef __CTYPE_HAS_8_BIT_LOCALES
	const __codeset_8_bit_t codeset_8_bit[__LOCALE_DATA_NUM_CODESETS];
#endif

	__LOCALE_DATA_COMMON_MMIDX(ctype)
	__LOCALE_DATA_COMMON_MMIDX(numeric)
	__LOCALE_DATA_COMMON_MMIDX(monetary)
	__LOCALE_DATA_COMMON_MMIDX(time)
	/* collate is different */
	__LOCALE_DATA_COMMON_MMIDX(messages)

	const uint16_t collate_data[__lc_collate_data_LEN];

	unsigned char lc_common_item_offsets_LEN[__LOCALE_DATA_CATEGORIES];
	size_t lc_common_tbl_offsets[__LOCALE_DATA_CATEGORIES * 4];
	/* offsets from start of locale_mmap_t */
	/* rows, item_offsets, item_idx, data */

#ifdef __LOCALE_DATA_NUM_LOCALES
	unsigned char locales[__LOCALE_DATA_NUM_LOCALES * __LOCALE_DATA_WIDTH_LOCALES];
	unsigned char locale_names5[5*__LOCALE_DATA_NUM_LOCALE_NAMES];
	unsigned char locale_at_modifiers[__LOCALE_DATA_AT_MODIFIERS_LENGTH];
#endif

	unsigned char lc_names[__lc_names_LEN];
#ifdef __CTYPE_HAS_8_BIT_LOCALES
	unsigned char codeset_list[sizeof(__LOCALE_DATA_CODESET_LIST)]; /* TODO - fix */
#endif
} __locale_mmap_t;

extern const __locale_mmap_t *__locale_mmap;
