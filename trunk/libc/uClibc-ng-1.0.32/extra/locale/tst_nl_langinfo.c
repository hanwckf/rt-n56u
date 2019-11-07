#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <langinfo.h>
#include <nl_types.h>

#if !defined(__UCLIBC__) && 0
#define DO_EXTRA
#endif

int main(int argc, char **argv)
{
	char *l;
	const unsigned char *x;
/*  	const unsigned char *y; */
	const unsigned char *p;

	if (argc > 2) {
		printf("invalid args\n");
		return EXIT_FAILURE;
	}
	if (argc == 1) {
		l = "";
	} else {
		l = *++argv;
	}

	if (!(x = setlocale(LC_ALL,l))) {
		printf("couldn't set locale %s\n", l);
		return EXIT_FAILURE;
	}

/*  	printf("\nsetlocale returned:\n  "); */
/*  	do { */
/*  		printf("\\x%02x", *x); */
/*  	} while (*x++); */
/*  	printf("\n"); */

#ifndef __BCC__
#define STR(X) #X
#else
#define STR(X) __STR(X)
#endif
#define __PASTE2(A,B) A.B

#define DO_NL_I(X) \
	printf( STR(X) " = %d\n", (int) nl_langinfo(X) );
#define DO_NL_S(X) \
	printf( STR(X) " = \"%s\"\n", nl_langinfo(X) );
#define DO_NL_C(X) \
	printf( STR(X) " = \"\\x%02x\"\n", *((unsigned char *) nl_langinfo(X)) );

	printf("ctype\n");

		DO_NL_S(CODESET);
#ifdef DO_EXTRA
		DO_NL_I(_NL_CTYPE_INDIGITS_MB_LEN);
		DO_NL_S(_NL_CTYPE_INDIGITS0_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS1_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS2_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS3_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS4_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS5_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS6_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS7_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS8_MB);
		DO_NL_S(_NL_CTYPE_INDIGITS9_MB);
#endif
		DO_NL_S(_NL_CTYPE_OUTDIGIT0_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT1_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT2_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT3_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT4_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT5_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT6_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT7_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT8_MB);
		DO_NL_S(_NL_CTYPE_OUTDIGIT9_MB);


	printf("numeric\n");

		DO_NL_S(RADIXCHAR);		/* DECIMAL_POINT */
		DO_NL_S(THOUSEP);		/* THOUSANDS_SEP */
/*  		DO_NL_S(GROUPING); */

	printf("GROUPING = \"");
	for (p = (unsigned char *) nl_langinfo(GROUPING) ; *p ; p++) {
		printf("\\x%02x", *p);
	}
	printf("\"\n\n");

	printf("monetary\n");

		DO_NL_S(INT_CURR_SYMBOL);
		DO_NL_S(CURRENCY_SYMBOL);
		DO_NL_S(MON_DECIMAL_POINT);
		DO_NL_S(MON_THOUSANDS_SEP);
/*  		DO_NL_S(MON_GROUPING); */

	printf("MON_GROUPING = \"");
	for (p = (unsigned char *) nl_langinfo(MON_GROUPING) ; *p ; p++) {
		printf("\\x%02x", *p);
	}
	printf("\"\n\n");

		DO_NL_S(POSITIVE_SIGN);
		DO_NL_S(NEGATIVE_SIGN);
		DO_NL_C(INT_FRAC_DIGITS);
		DO_NL_C(FRAC_DIGITS);
		DO_NL_C(P_CS_PRECEDES);
		DO_NL_C(P_SEP_BY_SPACE);
		DO_NL_C(N_CS_PRECEDES);
		DO_NL_C(N_SEP_BY_SPACE);
		DO_NL_C(P_SIGN_POSN);
		DO_NL_C(N_SIGN_POSN);
		DO_NL_C(INT_P_CS_PRECEDES);
		DO_NL_C(INT_P_SEP_BY_SPACE);
		DO_NL_C(INT_N_CS_PRECEDES);
		DO_NL_C(INT_N_SEP_BY_SPACE);
		DO_NL_C(INT_P_SIGN_POSN);
		DO_NL_C(INT_N_SIGN_POSN);

		DO_NL_S(CRNCYSTR);		/* CURRENCY_SYMBOL */


	printf("time\n");

		DO_NL_S(ABDAY_1);
		DO_NL_S(ABDAY_2);
		DO_NL_S(ABDAY_3);
		DO_NL_S(ABDAY_4);
		DO_NL_S(ABDAY_5);
		DO_NL_S(ABDAY_6);
		DO_NL_S(ABDAY_7);

		DO_NL_S(DAY_1);
		DO_NL_S(DAY_2);
		DO_NL_S(DAY_3);
		DO_NL_S(DAY_4);
		DO_NL_S(DAY_5);
		DO_NL_S(DAY_6);
		DO_NL_S(DAY_7);

		DO_NL_S(ABMON_1);
		DO_NL_S(ABMON_2);
		DO_NL_S(ABMON_3);
		DO_NL_S(ABMON_4);
		DO_NL_S(ABMON_5);
		DO_NL_S(ABMON_6);
		DO_NL_S(ABMON_7);
		DO_NL_S(ABMON_8);
		DO_NL_S(ABMON_9);
		DO_NL_S(ABMON_10);
		DO_NL_S(ABMON_11);
		DO_NL_S(ABMON_12);

		DO_NL_S(MON_1);
		DO_NL_S(MON_2);
		DO_NL_S(MON_3);
		DO_NL_S(MON_4);
		DO_NL_S(MON_5);
		DO_NL_S(MON_6);
		DO_NL_S(MON_7);
		DO_NL_S(MON_8);
		DO_NL_S(MON_9);
		DO_NL_S(MON_10);
		DO_NL_S(MON_11);
		DO_NL_S(MON_12);

		DO_NL_S(AM_STR);
		DO_NL_S(PM_STR);

		DO_NL_S(D_T_FMT);
		DO_NL_S(D_FMT);
		DO_NL_S(T_FMT);
		DO_NL_S(T_FMT_AMPM);
/* 		DO_NL_S(ERA); */
		{
		    const char *p = nl_langinfo(ERA);
		    if (!p || !*p) {
			printf("ERA = (none)\n");
		    } else {
			int i;
			printf("ERA:\n");
			for (i=0 ; i < 100 ; i++) {
			    printf("  %3d: \"%s\"\n", i, p);
			    while (*p) ++p;
			    ++p;
			    if (!*p) break;
			}
		    }
		}

		DO_NL_S(ERA_YEAR);		/* non SuSv3 */
		DO_NL_S(ERA_D_FMT);
/* 		DO_NL_S(ALT_DIGITS); */
		{
		    const char *p = nl_langinfo(ALT_DIGITS);
		    if (!p || !*p) {
			printf("ALT_DIGITS = (none)\n");
		    } else {
			int i;
			printf("ALT_DIGITS:\n");
			for (i=0 ; i < 100 ; i++) {
			    printf("  %3d: \"%s\"\n", i, p);
			    while (*p) ++p;
			    ++p;
			}
		    }
		}
		DO_NL_S(ERA_D_T_FMT);
		DO_NL_S(ERA_T_FMT);

#ifdef DO_EXTRA
		DO_NL_C(_NL_TIME_WEEK_NDAYS);
		DO_NL_I(_NL_TIME_WEEK_1STDAY); /* grr... this won't work with 16bit ptrs */
		DO_NL_C(_NL_TIME_WEEK_1STWEEK);
		DO_NL_C(_NL_TIME_FIRST_WEEKDAY);
		DO_NL_C(_NL_TIME_FIRST_WORKDAY);
		DO_NL_C(_NL_TIME_CAL_DIRECTION);
		DO_NL_S(_NL_TIME_TIMEZONE);
		DO_NL_S(_DATE_FMT);
#endif

	printf("messages\n");

		DO_NL_S(YESEXPR);
		DO_NL_S(NOEXPR);
		DO_NL_S(YESSTR);
		DO_NL_S(NOSTR);

#ifdef DO_EXTRA

	printf("paper\n");

		DO_NL_I(_NL_PAPER_HEIGHT);
		DO_NL_I(_NL_PAPER_WIDTH);

	printf("name\n");

		DO_NL_S(_NL_NAME_NAME_FMT);
		DO_NL_S(_NL_NAME_NAME_GEN);
		DO_NL_S(_NL_NAME_NAME_MR);
		DO_NL_S(_NL_NAME_NAME_MRS);
		DO_NL_S(_NL_NAME_NAME_MISS);
		DO_NL_S(_NL_NAME_NAME_MS);

	printf("address\n");

		DO_NL_S(_NL_ADDRESS_POSTAL_FMT);
		DO_NL_S(_NL_ADDRESS_COUNTRY_NAME);
		DO_NL_S(_NL_ADDRESS_COUNTRY_POST);
		DO_NL_S(_NL_ADDRESS_COUNTRY_AB2);
		DO_NL_S(_NL_ADDRESS_COUNTRY_AB3);
		DO_NL_S(_NL_ADDRESS_COUNTRY_CAR);
		DO_NL_I(_NL_ADDRESS_COUNTRY_NUM);
		DO_NL_S(_NL_ADDRESS_COUNTRY_ISBN);
		DO_NL_S(_NL_ADDRESS_LANG_NAME);
		DO_NL_S(_NL_ADDRESS_LANG_AB);
		DO_NL_S(_NL_ADDRESS_LANG_TERM);
		DO_NL_S(_NL_ADDRESS_LANG_LIB);

	printf("telephone\n");

		DO_NL_S(_NL_TELEPHONE_TEL_INT_FMT);
		DO_NL_S(_NL_TELEPHONE_TEL_DOM_FMT);
		DO_NL_S(_NL_TELEPHONE_INT_SELECT);
		DO_NL_S(_NL_TELEPHONE_INT_PREFIX);

	printf("measurement\n");

		DO_NL_C(_NL_MEASUREMENT_MEASUREMENT); /* 1 is metric, 2 is US */

	printf("identification\n");

		DO_NL_S(_NL_IDENTIFICATION_TITLE);
		DO_NL_S(_NL_IDENTIFICATION_SOURCE);
		DO_NL_S(_NL_IDENTIFICATION_ADDRESS);
		DO_NL_S(_NL_IDENTIFICATION_CONTACT);
		DO_NL_S(_NL_IDENTIFICATION_EMAIL);
		DO_NL_S(_NL_IDENTIFICATION_TEL);
		DO_NL_S(_NL_IDENTIFICATION_FAX);
		DO_NL_S(_NL_IDENTIFICATION_LANGUAGE);
		DO_NL_S(_NL_IDENTIFICATION_TERRITORY);
		DO_NL_S(_NL_IDENTIFICATION_AUDIENCE);
		DO_NL_S(_NL_IDENTIFICATION_APPLICATION);
		DO_NL_S(_NL_IDENTIFICATION_ABBREVIATION);
		DO_NL_S(_NL_IDENTIFICATION_REVISION);
		DO_NL_S(_NL_IDENTIFICATION_DATE);
		DO_NL_S(_NL_IDENTIFICATION_CATEGORY);

#endif

	return EXIT_SUCCESS;
}
