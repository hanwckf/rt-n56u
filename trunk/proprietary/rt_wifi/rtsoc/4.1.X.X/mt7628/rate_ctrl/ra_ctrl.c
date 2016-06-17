/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

#include "rt_config.h"



UCHAR RateSwitchTable11B[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x04, 0x03,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
};

UCHAR RateSwitchTable11BG[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0a, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
    0x04, 0x10,  2, 20, 35,
    0x05, 0x10,  3, 16, 35,
    0x06, 0x10,  4, 10, 25,
    0x07, 0x10,  5, 16, 25,
    0x08, 0x10,  6, 10, 25,
    0x09, 0x10,  7, 10, 13,
};

UCHAR RateSwitchTable11G[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x08, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x10,  0, 20, 101,
    0x01, 0x10,  1, 20, 35,
    0x02, 0x10,  2, 20, 35,
    0x03, 0x10,  3, 16, 35,
    0x04, 0x10,  4, 10, 25,
    0x05, 0x10,  5, 16, 25,
    0x06, 0x10,  6, 10, 25,
    0x07, 0x10,  7, 10, 13,
};


#ifdef DOT11_N_SUPPORT
UCHAR RateSwitchTable11N1S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0c, 0x0a,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x21,  5, 10, 25,
    0x09, 0x21,  6,  8, 14,
    0x0a, 0x21,  7,  8, 14,
    0x0b, 0x23,  7,  8, 14,
};


UCHAR RateSwitchTable11N2S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x0c,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 30,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 15,  8, 25,
    0x0d, 0x22, 15,  8, 15,
};


UCHAR RateSwitchTable11N3S[] = {
/* Item No.	Mode	Curr-MCS	TrainUp	TrainDown	 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x11, 0x0c,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 22,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 20,  8, 20,
    0x0d, 0x20, 21,  8, 20,
    0x0e, 0x20, 22,  8, 20,
    0x0f, 0x20, 23,  8, 20,
    0x10, 0x22, 23,  8, 15,
};


UCHAR RateSwitchTable11BGN1S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0c, 0x0a,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x21,  5, 10, 25,
    0x09, 0x21,  6,  8, 14,
    0x0a, 0x21,  7,  8, 14,
    0x0b, 0x23,  7,  8, 14,
};


UCHAR RateSwitchTable11BGN2S[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x0c,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 22,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 15,  8, 20,
    0x0d, 0x22, 15,  8, 15,
};


UCHAR RateSwitchTable11BGN3S[] = { /* 3*3*/
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30,101,	/*50*/
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 20, 50,
    0x04, 0x21,  4, 15, 50,
    0x05, 0x20, 11, 15, 30,
    0x06, 0x20, 12, 15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    /*0x0a, 0x20, 20, 15, 30,*/
    0x0a, 0x20, 21,  8, 20,
    0x0b, 0x20, 22,  8, 20,
    0x0c, 0x20, 23,  8, 25,
    0x0d, 0x22, 23,  8, 25,
};


UCHAR RateSwitchTable11N1SForABand[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown	*/
/* Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF) */
    0x09, 0x07,  0,  0,  0,						/* Initial used item after association */
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 10, 25,
    0x06, 0x21,  6,  8, 14,
    0x07, 0x21,  7,  8, 14,
    0x08, 0x23,  7,  8, 14,
};


UCHAR RateSwitchTable11N2SForABand[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0b, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12,  15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    0x0a, 0x22, 15,  8, 25,
};


UCHAR RateSwitchTable11BGN2SForABand[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0b, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30,101,	/*50*/
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12, 15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    0x0a, 0x22, 15,  8, 25,
};


UCHAR RateSwitchTable11N3SForABand[] = { /* 3*3*/
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12,  15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    /*0x0a, 0x22, 15,  8, 25,*/
    /*0x0a, 0x20, 20, 15, 30,*/
    0x0a, 0x20, 21,  8, 20,
    0x0b, 0x20, 22,  8, 20,
    0x0c, 0x20, 23,  8, 25,
    0x0d, 0x22, 23,  8, 25,
};


UCHAR RateSwitchTable11BGN3SForABand[] = { /* 3*3*/
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x0e, 0x09,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x21,  0, 30,101,	/*50*/
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12, 15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    /*0x0a, 0x22, 15,  8, 25,*/
    /*0x0a, 0x20, 20, 15, 30,*/
    0x0a, 0x20, 21,  8, 20,
    0x0b, 0x20, 22,  8, 20,
    0x0c, 0x20, 23,  8, 25,
    0x0d, 0x22, 23,  8, 25,
};
#endif /* DOT11_N_SUPPORT */


#ifdef NEW_RATE_ADAPT_SUPPORT
UCHAR RateSwitchTableAdapt11B[] = {
/*  item no.   mcs   TrainDown  upMcs3     upMcs1
           mode   TrainUp  downMcs     upMcs2
*/
	 4,    3,  0,  0,   0,   0,   0,    0,   0,  0,
	 0, 0x00,  0, 40, 101,   0,   1,    1,   1,  1, /* CCK 1M */
	 1, 0x00,  1, 40,  50,   0,   2,    2,   2,  2, /* CCK 2M */
	 2, 0x00,  2, 35,  45,   1,   3,    3,   3,  5, /* CCK 5M */
	 3, 0x00,  3, 20,  45,   2,   3,    3,   3, 11, /* CCK 11M*/
};

UCHAR RateSwitchTableAdapt11BG[] = {
/*  item no.   mcs   TrainDown  upMcs3     upMcs1
           mode   TrainUp  downMcs     upMcs2
*/
	 10,   9,  0,  0,   0,   0,    0,     0,   0,   0,
	 0, 0x00,  0, 40, 101,   0,    1,     1,   1,   1, /* CCK 1M */
	 1, 0x00,  1, 40,  50,   0,    2,     2,   2,   2, /* CCK 2M */
	 2, 0x00,  2, 35,  45,   1,    3,     3,   3,   5, /* CCK 5M */
	 3, 0x10,  1, 20,  35,   2,    4,     4,   4,   9, /* OFDM 9M */
	 4, 0x10,  2, 20,  35,   3,    5,     5,   5,  12, /* OFDM 12M */
	 5, 0x10,  3, 16,  35,   4,    6,     6,   6,  18, /* OFDM 18M */
	 6, 0x10,  4, 10,  25,   5,    7,     7,   7,  24, /* OFDM 24M */
	 7, 0x10,  5, 16,  25,   6,    8,     8,   8,  36, /* OFDM 36M */
	 8, 0x10,  6, 10,  25,   7,    9,     9,   9,  48, /* OFDM 48M */
	 9, 0x10,  7, 10,  13,   8,    9,     9,   9,  54, /* OFDM 54M */
};

UCHAR RateSwitchTableAdapt11G[] = {
/*  item no.   mcs   TrainDown  upMcs3     upMcs1
           mode   TrainUp  downMcs     upMcs2
*/
	 8,    7,  0,  0,    0,   0,    0,     0,   0,   0,
	 0, 0x10,  0,  20, 101,   0,    1,     1,   1,   6, /* OFDM 6M */
	 1, 0x10,  1,  20,  35,   0,    2,     2,   2,   9, /* OFDM 9M */
	 2, 0x10,  2,  20,  35,   1,    3,     3,   3,  12, /* OFDM 12M */
	 3, 0x10,  3,  16,  35,   2,    4,     4,   4,  18, /* OFDM 18M*/
	 4, 0x10,  4,  10,  25,   3,    5,     5,   5,  24, /* OFDM 24M */
	 5, 0x10,  5,  16,  25,   4,    6,     6,   6,  36, /* OFDM 36M */
	 6, 0x10,  6,  10,  25,   5,    7,     7,   7,  48, /* OFDM 48M */
	 7, 0x10,  7,  10,  13,   6,    7,     7,   7,  54, /* OFDM 54M */
};

#ifdef DOT11_N_SUPPORT

#ifdef RANGE_EXTEND
#define SUPPORT_SHORT_GI_RA		/* Support switching to Short GI rates in RA */
#endif /*  RANGE_EXTEND */

/*
	Rate switch tables for New Rate Adaptation

	Each row has 10 bytes of data.
	First row contains table information:
		Byte0=the number of rate entries, Byte1=the initial rate.
	Format of Mode byte:
		Bit0: STBC,
		Bit1: Short GI,
		Bit4~6: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)
*/
UCHAR RateSwitchTableAdapt11N1S[] = {
/*  item no.   mcs   highPERThrd  upMcs3     upMcs1
           mode   lowPERThrd  downMcs     upMcs2
*/
	 12,   7,  0,  0,   0,   0,    0,     0,   0,   0,
	 0, 0x21,  0, 30,  50,  11,    1,     1,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    2,     2,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    3,     3,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,    4,     4,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,    5,     5,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10,  25,   4,    6,     6,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,    7,     7,   7,  59,/* mcs6 */
	 7, 0x21,  7,  8,  14,   6,    8,     8,   8,  65,/* mcs7 */
	 8, 0x23,  7,  8,  14,   7,    8,     8,   8,  72,/* mcs7+short gi */

	 9, 0x00,  0, 40,  101,  9,   10,    10,  10,   1, /* cck-1M */
	10, 0x00,  1, 40,  50,   9,   11,    11,  11,   2, /* cck-2M */
	11, 0x10, 0,  30,  50,  10,    0,     0,   0,   6, /* OFDM 6M */	
};

#ifdef SUPPORT_SHORT_GI_RA
/*  Indices for Short GI rates in 11N2S table */
  #define sg07	18		/*  mcs7+shortGI index */
  #define sg14	17		/*  mcs14+shortGI index */
  #define sg15	16		/*  mcs15+shortGI index */
#endif

UCHAR RateSwitchTableAdapt11N2S[] = {
/* item no.   mcs   highPERThrd  upMcs3    upMcs1
        mode   lowPERThrd  downMcs    upMcs2
*/
	22,   15,  0,  0,   0,   0,    0,   0,   0,   0,
     0, 0x21,  0, 30,  50,  21,    8,   8,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,    9,   9,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,    9,   9,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,   10,  10,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,   11,  11,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10,  25,   4,   12,  12,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,   12,  12,   7,  59,/* mcs6 */
#ifdef SUPPORT_SHORT_GI_RA
	 7, 0x21,  7,  8,  14,   6,   12,  12, sg07,  65,/* mcs7 */
#else
	 7, 0x21,  7,  8,  14,   6,   12,  12,   7,  65,/* mcs7 */
#endif
	 8, 0x20,  8, 30,  50,   0,    9,   9,   2,  13,/* mcs8 */
	 9, 0x20,  9, 20,  50,   8,   10,  10,   4,  26,/* mcs9 */
	10, 0x20, 10, 20,  40,   9,   11,  11,   5,  39,/* mcs10 */
	11, 0x20, 11, 15,  30,  10,   12,  12,   6,  52,/* mcs11 */
	12, 0x20, 12, 15,  30,  11,   13,  13,  12,  78,/* mcs12 */
	13, 0x20, 13,  8,  20,  12,   14,  14,  13, 104,/* mcs13 */
#ifdef SUPPORT_SHORT_GI_RA
	14, 0x20, 14,  8,  18,  13,   15,  15,sg14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14,   sg15,sg15,sg14, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15,   sg15,sg15,sg15, 144,/* mcs15+shortGI */

    17, 0x22, 14,  8,  14,  14,   sg15,sg15,  15, 130, /* mcs14+shortGI */
    18, 0x23,  7,  8,  14,   7,   12,  12,sg07,  72, /* mcs7+shortGI */
#else
	14, 0x20, 14,  8,  18,  13,   16,  16,  14, 117,/* mcs14 */
	15, 0x22, 14,  8,  14,  14,   17,  17,  15, 130,/* mcs14+shortGI */	
	16, 0x20, 15,  8,  25,  15,   17,  17,  16, 130,/* mcs15 */
	17, 0x22, 15,  8,  25,  16,   17,  17,  17, 144,/* mcs15+shortGI */
    18,    0,  0,  0,   0,   0,   0,    0,   0,   0,
#endif
    19, 0x00,  0, 40,  101, 19 ,  19,    19,   20,  1, /* cck-1M */
    20, 0x00,  1, 40,  50,  19,   20,    20,   21,  2, /* cck-2M */
    21, 0x10,  0, 30,  50,  20,   8,     8,    0,   6, /* OFDM 6M */
};

#ifdef SUPPORT_SHORT_GI_RA
/*  Indices for Short GI rates in 11N3S table */
  #undef sg07
  #undef sg14
  #undef sg15
  #define sg07	29		/*  mcs7+shortGI index */
  #define sg14	27		/*  mcs14+shortGI index */
  #define sg15	28		/*  mcs15+shortGI index */
  #define sg21	25		/*  mcs21+shortGI index */
  #define sg22	26		/*  mcs22+shortGI index */
  #define sg23	24		/*  mcs23+shortGI index */
#endif

UCHAR RateSwitchTableAdapt11N3S[] = {
/* item no   mcs   highPERThrd   upMcs3     upMcs1
        mode   lowPERThrd  downMcs     upMcs2
*/
	33,   23,  0,  0,   0,   0,    0,    0,    0,   0,
	 0, 0x21,  0, 30,  50,  32,    1,    8,    1,   7, /* mcs0 */
     1, 0x21,  1, 20,  50,   0,   16,    9,    2,  13, /* mcs1 */
     2, 0x21,  2, 20,  50,   1,   17,    9,    3,  20, /* mcs2 */
     3, 0x21,  3, 15,  50,   2,   17,   10,    4,  26, /* mcs3 */
     4, 0x21,  4, 15,  30,   3,   18,   11,    5,  39, /* mcs4 */
     5, 0x21,  5, 10,  25,   4,   18,   12,    6,  52, /* mcs5 */
     6, 0x21,  6,  8,  14,   5,   19,   12,    7,  59, /* mcs6 */
#ifdef SUPPORT_SHORT_GI_RA
	 7, 0x21,  7,  8,  14,   6,   19,   12, sg07,  65, /* mcs7 */
#else
	 7, 0x21,  7,  8,  14,   6,   19,   12,    7,  65, /* mcs7 */
#endif
     8, 0x20,  8, 30,  50,   0,   16,    9,    2,  13, /* mcs8 */
     9, 0x20,  9, 20,  50,   8,   17,   10,    4,  26, /* mcs9 */
    10, 0x20, 10, 20,  40,   9,   18,   11,    5,  39, /* mcs10 */
    11, 0x20, 11, 15,  30,  10,   18,   12,    6,  52, /* mcs11 */
    12, 0x20, 12, 15,  30,  11,   20,   13,   12,  78, /* mcs12 */
    13, 0x20, 13,  8,  20,  12,   20,   14,   13, 104, /* mcs13 */
#ifdef SUPPORT_SHORT_GI_RA
    14, 0x20, 14,  8,  18,  13,   21,   15, sg14, 117, /* mcs14 */
    15, 0x20, 15,  8,  14,  14,   21, sg15, sg14, 130, /* mcs15 */
#else
    14, 0x20, 14,  8,  18,  13,   21,   15,   14, 117, /* mcs14 */
    15, 0x20, 15,  8,  14,  14,   21,   15,   15, 130, /* mcs15 */
#endif
    16, 0x20, 16, 30,  50,   8,   17,    9,    3,  20, /* mcs16 */
    17, 0x20, 17, 20,  50,  16,   18,   11,    5,  39, /* mcs17 */
    18, 0x20, 18, 20,  40,  17,   19,   12,    7,  59, /* mcs18 */
    19, 0x20, 19, 15,  30,  18,   20,   13,   19,  78, /* mcs19 */
    20, 0x20, 20, 15,  30,  19,   21,   15,   20, 117, /* mcs20 */
#ifdef SUPPORT_SHORT_GI_RA
    21, 0x20, 21,  8,  20,  20,   22, sg21,   21, 156, /* mcs21 */
    22, 0x20, 22,  8,  20,  21,   23, sg22, sg21, 176, /* mcs22 */
    23, 0x20, 23,  6,  18,  22, sg23,   23, sg22, 195, /* mcs23 */
    24, 0x22, 23,  6,  14,  23, sg23, sg23, sg23, 217, /* mcs23+shortGI */

    25, 0x22, 21,  6,  18,  21, sg22,   22, sg21, 173, /* mcs21+shortGI */
    26, 0x22, 22,  6,  18,  22, sg23,   23, sg22, 195, /* mcs22+shortGI */
    27, 0x22, 14,  8,  14,  14,   21, sg15,   15, 130, /* mcs14+shortGI */
    28, 0x22, 15,  8,  14,  15,   21, sg15, sg15, 144, /* mcs15+shortGI */
    29, 0x23,  7,  8,  14,   7,   19,   12,   29,  72, /* mcs7+shortGI */
#else
    21, 0x20, 21,  8,  20,  20,   22,   21,   21, 156, /* mcs21 */
    22, 0x20, 22,  8,  20,  21,   23,   22,   22, 176, /* mcs22 */
    23, 0x20, 23,  6,  18,  22,   24,   23,   23, 195, /* mcs23 */
    24, 0x22, 23,  6,  14,  23,   24,   24,   24, 217, /* mcs23+shortGI */
    25,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    26,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    27,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    28,    0,  0,  0,   0,   0,   0,     0,    0,   0,
    29,    0,  0,  0,   0,   0,   0,     0,    0,   0,
#endif
    30, 0x00,  0, 40,  101, 30 ,  30,    30,   31,  1, /* cck-1M */
    31, 0x00,  1, 40,  50,  30,   31,    31,   32,  2, /* cck-2M */
    32, 0x21, 32, 30,  50,  31,   0,     8,    0,   7, /* mcs32 or 20M/mcs0 */
};



#endif /*  NEW_RATE_ADAPT_SUPPORT */

#endif /* DOT11_N_SUPPORT */


/* MlmeGetSupportedMcs - fills in the table of mcs with index into the pTable
		pAd - pointer to adapter
		pTable - pointer to the Rate Table. Assumed to be a table without mcsGroup values
		mcs - table of MCS index into the Rate Table. -1 => not supported
*/
VOID MlmeGetSupportedMcs(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	*pTable,
	OUT CHAR 	mcs[])
{
	CHAR	idx;
	RTMP_RA_LEGACY_TB *pCurrTxRate;

	for (idx = 0; idx < 24; idx++)
		mcs[idx] = -1;

	/*  check the existence and index of each needed MCS */
	for (idx=0; idx<RATE_TABLE_SIZE(pTable); idx++)
	{
		pCurrTxRate = PTX_RA_LEGACY_ENTRY(pTable, idx);

		/*  Rate Table may contain CCK and MCS rates. Give HT/Legacy priority over CCK */
		if (pCurrTxRate->CurrMCS==MCS_0 && (mcs[0]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[0] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_1 && (mcs[1]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[1] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_2 && (mcs[2]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[2] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_3)
			mcs[3] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_4)
			mcs[4] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_5)
			mcs[5] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_6)
			mcs[6] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))
			mcs[7] = idx;
#ifdef DOT11_N_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_12)
			mcs[12] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_13)
			mcs[13] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_14)
			mcs[14] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))
		{
			mcs[15] = idx;
		}
#ifdef DOT11N_SS3_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_20)
			mcs[20] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_21)
			mcs[21] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_22)
			mcs[22] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_23)
			mcs[23] = idx;
#endif /*  DOT11N_SS3_SUPPORT */
#endif /*  DOT11_N_SUPPORT */
	}

#ifdef DBG_CTRL_SUPPORT
	/*  Debug Option: Disable highest MCSs when picking initial MCS based on RSSI */
	if (pAd->CommonCfg.DebugFlags & DBF_INIT_MCS_DIS1)
		mcs[23] = mcs[15] = mcs[7] = mcs[22] = mcs[14] = mcs[6] = 0;
#endif /* DBG_CTRL_SUPPORT */
}


/*  MlmeClearTxQuality - Clear TxQuality history only for the active BF state */
VOID MlmeClearTxQuality(MAC_TABLE_ENTRY *pEntry)
{
		NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	NdisZeroMemory(pEntry->PER, sizeof(pEntry->PER));
}


/*  MlmeClearAllTxQuality - Clear both BF and non-BF TxQuality history */
VOID MlmeClearAllTxQuality(MAC_TABLE_ENTRY *pEntry)
{
	NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	NdisZeroMemory(pEntry->PER, sizeof(pEntry->PER));
}


/*  MlmeDecTxQuality - Decrement TxQuality of specified rate table entry */
VOID MlmeDecTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex)
{
	if (pEntry->TxQuality[rateIndex])
		pEntry->TxQuality[rateIndex]--;
}


VOID MlmeSetTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rate_idx, USHORT quality)
{
		pEntry->TxQuality[rate_idx] = quality;
}


USHORT MlmeGetTxQuality(MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex)
{
	return pEntry->TxQuality[rateIndex];
}


#ifdef CONFIG_AP_SUPPORT
VOID APMlmeSetTxRate(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate)
{
	UCHAR tx_mode = pTxRate->Mode;


#ifdef DOT11_N_SUPPORT
	if (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD)
	{
		if ((pTxRate->STBC) && (pEntry->MaxHTPhyMode.field.STBC))
			pEntry->HTPhyMode.field.STBC = STBC_USE;
		else
			pEntry->HTPhyMode.field.STBC = STBC_NONE;

		if ((pTxRate->ShortGI || pAd->WIFItestbed.bShortGI) && (pEntry->MaxHTPhyMode.field.ShortGI))
			pEntry->HTPhyMode.field.ShortGI = GI_400;
		else
			pEntry->HTPhyMode.field.ShortGI = GI_800;
	}
		
	/* TODO: will check ldpc if related to rate table */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE) ||
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE)) {
		pEntry->HTPhyMode.field.ldpc = TRUE;
	} else {
		pEntry->HTPhyMode.field.ldpc = FALSE;
	}

#endif /* DOT11_N_SUPPORT */

	if (pTxRate->CurrMCS < MCS_AUTO)
		pEntry->HTPhyMode.field.MCS = pTxRate->CurrMCS;

	pEntry->HTPhyMode.field.MODE = tx_mode;

#ifdef DOT11_N_SUPPORT
	if ((pAd->WIFItestbed.bGreenField & pEntry->HTCapability.HtCapInfo.GF) && (pEntry->HTPhyMode.field.MODE == MODE_HTMIX))
	{
		/* force Tx GreenField */
		pEntry->HTPhyMode.field.MODE = MODE_HTGREENFIELD;
	}

	/* BW depends on BSSWidthTrigger and Negotiated BW */
	if (pAd->CommonCfg.bRcvBSSWidthTriggerEvents ||
		(pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ||
		(pEntry->MaxHTPhyMode.field.BW==BW_20) ||
		(pAd->CommonCfg.BBPCurrentBW==BW_20))
		pEntry->HTPhyMode.field.BW = BW_20;
	else
		pEntry->HTPhyMode.field.BW = BW_40;


#ifdef RANGE_EXTEND
#ifdef NEW_RATE_ADAPT_SUPPORT
	/* 20 MHz Fallback */
	if ((tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD) &&
	    pEntry->HTPhyMode.field.BW == BW_40 &&
	    ADAPT_RATE_TABLE(pEntry->pTable))
	{
		if (pEntry->HTPhyMode.field.MCS == 32
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0) == 0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map HT Duplicate to 20MHz MCS0 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 0;
		}
		else if (pEntry->HTPhyMode.field.MCS == 0 &&
				(pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ) == 0
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS1) == 0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map 40MHz MCS0 to 20MHz MCS1 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 1;
		}
		else if (pEntry->HTPhyMode.field.MCS == 8
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_ENABLE_20MHZ_MCS8)
#endif /* DBG_CTRL_SUPPORT */
			)
		{
			/* Map 40MHz MCS8 to 20MHz MCS8 */
			pEntry->HTPhyMode.field.BW = BW_20;
		}
	}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
	/* Debug Option: Force BW */
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_40MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_40;
	}
	else if (pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)
	{
		pEntry->HTPhyMode.field.BW = BW_20;
	}
#endif /* DBG_CTRL_SUPPORT */
#endif /* RANGE_EXTEND */

	/* Reexam each bandwidth's SGI support. */
	if ((pEntry->HTPhyMode.field.BW==BW_20 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
		(pEntry->HTPhyMode.field.BW==BW_40 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)) )
		pEntry->HTPhyMode.field.ShortGI = GI_800;

#ifdef DBG_CTRL_SUPPORT
	/* Debug option: Force Short GI */
	if (pAd->CommonCfg.DebugFlags & DBF_FORCE_SGI)
		pEntry->HTPhyMode.field.ShortGI = GI_400;
#endif /* DBG_CTRL_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
		RtAsicFifoExtEntryClean(pAd, pEntry);
#endif /* FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MCS_LUT_SUPPORT
	AsicMcsLutUpdate(pAd, pEntry);
	pEntry->LastTxRate = (USHORT) (pEntry->HTPhyMode.word);
#endif /* MCS_LUT_SUPPORT */


}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID MlmeSetTxRate(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate)
{
	struct wifi_dev *wdev;
	UCHAR	MaxMode = MODE_OFDM;
	UCHAR tx_mode, tx_bw;
	HTTRANSMIT_SETTING *tx_setting;


	if (!pAd || !pEntry || !pTxRate || !pEntry->pTable || (pEntry->EntryType == ENTRY_NONE))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid parameters!\n", __FUNCTION__));
		return;
	}

	wdev = &pAd->StaCfg.wdev;
	tx_mode = pTxRate->Mode;
	tx_bw = pTxRate->BW;
	tx_setting = &pAd->StaCfg.wdev.HTPhyMode;

#ifdef DOT11_N_SUPPORT
	MaxMode = MODE_HTGREENFIELD;



	{
		if (pTxRate->STBC &&
			(((pAd->StaCfg.wdev.MaxHTPhyMode.field.STBC) && (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD))
			)
		)
			wdev->HTPhyMode.field.STBC = STBC_USE;
		else
#endif /*  DOT11_N_SUPPORT */
			wdev->HTPhyMode.field.STBC = STBC_NONE;
	}

	if (pTxRate->CurrMCS < MCS_AUTO)
		wdev->HTPhyMode.field.MCS = pTxRate->CurrMCS;

	/* TODO: will check ldpc if related to rate table */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE) ||
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE)) {
		wdev->HTPhyMode.field.ldpc = TRUE;
	} else {
		wdev->HTPhyMode.field.ldpc = FALSE;
	}


	if (wdev->HTPhyMode.field.MCS > 7)
		wdev->HTPhyMode.field.STBC = STBC_NONE;

   	if (ADHOC_ON(pAd))
	{
		/*  If peer adhoc is b-only mode, we can't send 11g rate. */
		wdev->HTPhyMode.field.ShortGI = GI_800;
		pEntry->HTPhyMode.field.STBC	= STBC_NONE;

		/* For Adhoc MODE_CCK, driver will use AdhocBOnlyJoined flag to roll back to B only if necessary */
		pEntry->HTPhyMode.field.MODE	= tx_mode;
		pEntry->HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
		pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;

		/*  Patch speed error in status page */
		wdev->HTPhyMode.field.MODE = pEntry->HTPhyMode.field.MODE;
	}
	else
	{
		USHORT OperationMode =0xffff;

#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.RegTransmitSetting.field.HTMODE == HTMODE_GF) &&
			(pAd->MlmeAux.HtCapability.HtCapInfo.GF == HTMODE_GF))
			wdev->HTPhyMode.field.MODE = MODE_HTGREENFIELD;
		else
#endif /*  DOT11_N_SUPPORT */
		if (tx_mode <= MaxMode)
			wdev->HTPhyMode.field.MODE = tx_mode;

#ifdef DOT11_N_SUPPORT
		if (pTxRate->ShortGI && (wdev->MaxHTPhyMode.field.ShortGI))
			wdev->HTPhyMode.field.ShortGI = GI_400;
		else
#endif /*  DOT11_N_SUPPORT */
			wdev->HTPhyMode.field.ShortGI = GI_800;

#ifdef DOT11_N_SUPPORT
		/*  BW depends on Negotiated BW */
		if (pEntry->MaxHTPhyMode.field.BW==BW_20 || pAd->CommonCfg.BBPCurrentBW==BW_20)
			pEntry->HTPhyMode.field.BW = BW_20;
		else
			pEntry->HTPhyMode.field.BW = BW_40;


#ifdef RANGE_EXTEND
#ifdef NEW_RATE_ADAPT_SUPPORT
		/*  20 MHz Fallback */
		if (tx_mode >=MODE_HTMIX && pEntry->HTPhyMode.field.BW==BW_40 &&
			ADAPT_RATE_TABLE(pEntry->pTable)
		)
		{
			if ((wdev->HTPhyMode.field.MCS==32)
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0)==0
#endif /* DBG_CTRL_SUPPORT */
			)
			{
				/*  Map HT Duplicate to 20MHz MCS0 */
				pEntry->HTPhyMode.field.BW = BW_20;
				wdev->HTPhyMode.field.MCS = 0;
				if (pTxRate->STBC && wdev->MaxHTPhyMode.field.STBC)
					wdev->HTPhyMode.field.STBC = STBC_USE;
			}
			else if (wdev->HTPhyMode.field.MCS==0
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)==0
				&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS1)==0
#endif /* DBG_CTRL_SUPPORT */
			)
			{
				/*  Map 40MHz MCS0 to 20MHz MCS1 */
				pEntry->HTPhyMode.field.BW = BW_20;
				wdev->HTPhyMode.field.MCS = 1;
			}
			else if (wdev->HTPhyMode.field.MCS==8
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_ENABLE_20MHZ_MCS8)
#endif /* DBG_CTRL_SUPPORT */
			)
			{
				/*  Map 40MHz MCS8 to 20MHz MCS8 */
				pEntry->HTPhyMode.field.BW = BW_20;
			}
		}
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef DBG_CTRL_SUPPORT
		/*  Debug Option: Force BW */
		if (pAd->CommonCfg.DebugFlags & DBF_FORCE_40MHZ)
			pEntry->HTPhyMode.field.BW = BW_40;
		else if (pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)
			pEntry->HTPhyMode.field.BW = BW_20;
#endif /* DBG_CTRL_SUPPORT */
#endif /*  RANGE_EXTEND */

		/*  Reexam each bandwidth's SGI support. */
		if ((pEntry->HTPhyMode.field.BW==BW_20 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
			(pEntry->HTPhyMode.field.BW==BW_40 && !CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)) )
			wdev->HTPhyMode.field.ShortGI = GI_800;

#ifdef DBG_CTRL_SUPPORT
		/*  Debug option: Force Short GI */
		if (pAd->CommonCfg.DebugFlags & DBF_FORCE_SGI)
			wdev->HTPhyMode.field.ShortGI = GI_400;
#endif /*  DBG_CTRL_SUPPORT */

		/*  Turn RTS/CTS rate to 6Mbps. */
		if (((pEntry->HTPhyMode.field.MCS == 0) && (wdev->HTPhyMode.field.MCS != 0)) ||
			((pEntry->HTPhyMode.field.MCS == 8) && (wdev->HTPhyMode.field.MCS != 8)))
		{
			pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
			if (pAd->MacTab.fAnyBASession)
				OperationMode = HT_FORCERTSCTS;
			else
				OperationMode = pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;
		}
		else if ((pEntry->HTPhyMode.field.MCS != 0) && (wdev->HTPhyMode.field.MCS == 0))
			OperationMode = HT_RTSCTS_6M;
		else if ((pEntry->HTPhyMode.field.MCS != 8) && (wdev->HTPhyMode.field.MCS == 8))
			OperationMode = HT_RTSCTS_6M;

		if (OperationMode != 0xffff)
			AsicUpdateProtect(pAd, OperationMode , ALLN_SETPROTECT, TRUE,
							(BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
#endif /* DOT11_N_SUPPORT */

		pEntry->HTPhyMode.field.STBC	= wdev->HTPhyMode.field.STBC;
		pEntry->HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
		pEntry->HTPhyMode.field.ldpc = wdev->HTPhyMode.field.ldpc; 
		pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
		pEntry->HTPhyMode.field.MODE = wdev->HTPhyMode.field.MODE;
	}

#ifdef MCS_LUT_SUPPORT
	AsicMcsLutUpdate(pAd, pEntry);
	pEntry->LastTxRate = (USHORT) (pEntry->HTPhyMode.word);
#endif /* MCS_LUT_SUPPORT */

}
#endif /* CONFIG_STA_SUPPORT */


#ifdef NEW_RATE_ADAPT_SUPPORT


UCHAR* SelectTxRateTableGRP(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;

#ifdef CONFIG_STA_SUPPORT
    if ((pAd->OpMode == OPMODE_STA) && ADHOC_ON(pAd))
    {
        /* for ADHOC mode */
#ifdef DOT11_N_SUPPORT
        if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && 
                (pEntry->HTCapability.MCSSet[0] != 0x00) && 
                ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))
        {/* 11N 1S Adhoc*/

            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N1S;
            } else {
                if (pAd->LatchRfRegs.Channel <= 14) {
                    pTable = RateSwitchTable11N1S;
                } else {
                    pTable = RateSwitchTable11N1SForABand;
                }
            }
        }
        else if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && 
                (pEntry->HTCapability.MCSSet[0] != 0x00) && 
                (pEntry->HTCapability.MCSSet[1] != 0x00) &&
                (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->Antenna.field.TxPath == 2)))
        {/* 11N 2S Adhoc*/
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N2S;
            } else {
                if (pAd->LatchRfRegs.Channel <= 14) {
                    pTable = RateSwitchTable11N2S;
                } else {
                    pTable = RateSwitchTable11N2SForABand;
                }
            }
        }
        else
#endif /* DOT11_N_SUPPORT */
        if ((pEntry->RateLen == 4)
#ifdef DOT11_N_SUPPORT
                && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
        )
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11B;
            } else {
                pTable = RateSwitchTable11B;
            }
        }
        else if (pAd->LatchRfRegs.Channel <= 14)
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11BG;
            } else {
                pTable = RateSwitchTable11BG;
            }
        }
        else
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11G;
            } else {
                pTable = RateSwitchTable11G;
            }
        }
        return pTable;
    }
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
    /*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
    {/* 11BGN 1S AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11N1S;
        }
        else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN1S;
        } else {
            pTable = RateSwitchTable11N1SForABand;
        }

        return pTable;
    }


    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
#ifdef THERMAL_PROTECT_SUPPORT
            (pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
            (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->CommonCfg.TxStream == 2)))
    {/* 11BGN 2S AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
#if defined(MT7603) || defined(MT7628) || defined(MT7636)
            if (IS_MT7603(pAd) || (IS_MT7628(pAd)) || (IS_MT76x6(pAd))) {
                if ( pEntry->MmpsMode == MMPS_STATIC) {
                    pTable = RateSwitchTableAdapt11N1S;
                } else {
                    pTable = RateSwitchTableAdapt11N2S;
                }
            } else
#endif /* defined(MT7603) || defined(MT7628) || defined(MT7636) */
                pTable = RateSwitchTableAdapt11N2S;
        } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN2S;
        } else {
            pTable = RateSwitchTable11BGN2SForABand;
        }

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (pAd->CommonCfg.TxStream == 3))
    {
        if (pAd->rateAlg == RATE_ALG_GRP)
        {
#if defined(MT7603) || defined(MT7628) || defined(MT7636)
            if (IS_MT7603(pAd) || (IS_MT7628(pAd)) || (IS_MT76x6(pAd))) {
                if ( pEntry->MmpsMode == MMPS_STATIC ) {
                    pTable = RateSwitchTableAdapt11N1S;
                } else {
                    pTable = RateSwitchTableAdapt11N2S;
                }
            } else
#endif /* defined(MT7603) || defined(MT7628) || defined(MT7636) */
                pTable = RateSwitchTableAdapt11N3S;
        } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11N3S;
        } else {
            pTable = RateSwitchTable11N3SForABand;
        }

        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)
#ifdef THERMAL_PROTECT_SUPPORT
            || (pAd->force_one_tx_stream == TRUE)
#endif /* THERMAL_PROTECT_SUPPORT */
    ))
    {/* 11N 1S AP*/
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N1S;
            } else if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N1S;
            } else {
                pTable = RateSwitchTable11N1SForABand;
            }
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (pAd->CommonCfg.TxStream == 2))
    {/* 11N 2S AP*/
        {
            if (pAd->rateAlg == RATE_ALG_GRP) {
                pTable = RateSwitchTableAdapt11N2S;
            } else if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N2S;
            } else {
                pTable = RateSwitchTable11N2SForABand;			
            }
        }
        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (pAd->CommonCfg.TxStream == 3))
    {
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11N3S;
        } else {
            if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N3S;
            } else {
                pTable = RateSwitchTable11N3SForABand;
            }
        }
        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
    if (pAd->CommonCfg.TxStream == 3)
    {
        if  (pEntry->HTCapability.MCSSet[0] != 0x00)
        {
            if (pEntry->HTCapability.MCSSet[1] == 0x00)
            {	/* Only support 1SS */
                if (pEntry->RateLen > 0)
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N1S;
                    } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
                        pTable = RateSwitchTable11N1S;
                    } else {
                        pTable = RateSwitchTable11N1SForABand;	
                    }
                }
                else
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N1S;
                    } else if (pAd->LatchRfRegs.Channel <= 14) {
                        pTable = RateSwitchTable11N1S;
                    } else {
                        pTable = RateSwitchTable11N1SForABand;
                    }
                }
                break;
            }
            else if (pEntry->HTCapability.MCSSet[2] == 0x00)
            {	/* Only support 2SS */
                if (pEntry->RateLen > 0)
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N2S;
                    } else if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
                        pTable = RateSwitchTable11BGN2S;
                    } else {
                        pTable = RateSwitchTable11BGN2SForABand;
                    }
                }
                else
                {
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N2S;
                    } else if (pAd->LatchRfRegs.Channel <= 14) {
                        pTable = RateSwitchTable11N2S;
                    } else {
                        pTable = RateSwitchTable11N2SForABand;
                    }
                }
                return pTable;
            }
				/* For 3SS case, we use the new rate table, so don't care it here */
        }
    }
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

    if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || 
            WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
#ifdef DOT11_N_SUPPORT
            /*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
            /* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
    )
    {/* B only AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11B;
        } else {
            pTable = RateSwitchTable11B;
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
            (pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* B/G  mixed AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11BG;
        } else {
            pTable = RateSwitchTable11BG;
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* G only AP*/
        if (pAd->rateAlg == RATE_ALG_GRP) {
            pTable = RateSwitchTableAdapt11G;
        } else {
            pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
#ifdef DOT11N_SS3_SUPPORT
        if (pAd->CommonCfg.TxStream >= 3)
        {
            if (pAd->rateAlg == RATE_ALG_GRP)
            {
                if (pEntry->HTCapability.MCSSet[2] == 0) {
                    pTable = RateSwitchTableAdapt11N2S;
                } else
                    pTable = RateSwitchTableAdapt11N3S;
            }
            else
            {
                if (pEntry->HTCapability.MCSSet[2] == 0)
                    pTable = RateSwitchTable11N2S;
                else
                    pTable = RateSwitchTable11N3S;
            }
        }
        else
#endif /* DOT11N_SS3_SUPPORT */
        {
            /*
                Temp solution for:
                EX: when the extend rate only supports 6, 12, 24 in
                the association req frame. So the pEntry->RateLen is 7.
            */
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11BG;
            else
                pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
    {
#ifdef DOT11_N_SUPPORT
        /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
        if ((pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
#endif /* DOT11_N_SUPPORT */
        {	/* Legacy mode*/
            if (pAd->CommonCfg.MaxTxRate <= RATE_11)
            {
                pTable = RateSwitchTable11B;
            }
            else if ((pAd->CommonCfg.MaxTxRate > RATE_11) && (pAd->CommonCfg.MinTxRate > RATE_11))
            {
                pTable = RateSwitchTable11G;
            }
            else
            {
                pTable = RateSwitchTable11BG;
            }
            return pTable;
        }
#ifdef DOT11_N_SUPPORT
        {
            if (pAd->LatchRfRegs.Channel <= 14)
            {
                if (pAd->CommonCfg.TxStream == 1)
                {
                    pTable = RateSwitchTable11N1S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                }
                else if (pAd->CommonCfg.TxStream == 2)
                {
                    pTable = RateSwitchTable11N2S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));	
                }
                else
                {
#ifdef DOT11N_SS3_SUPPORT
                    if (pAd->rateAlg == RATE_ALG_GRP)
                        pTable = RateSwitchTableAdapt11N3S;
                    else
                        pTable = RateSwitchTable11N3S;

#else
                    pTable = RateSwitchTable11N2S;
#endif /* DOT11N_SS3_SUPPORT */
                }
            }
            else
            {
                if (pAd->CommonCfg.TxStream == 1)
                {
                    pTable = RateSwitchTable11N1S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                }
                else if (pAd->CommonCfg.TxStream == 2)
                {
                    pTable = RateSwitchTable11N2S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));	
                }
                else
                {
#ifdef DOT11N_SS3_SUPPORT
                    if (pAd->rateAlg == RATE_ALG_GRP) {
                        pTable = RateSwitchTableAdapt11N3S;
                    } else {
                        pTable = RateSwitchTable11N3S;
                    }
#else
                    pTable = RateSwitchTable11N2SForABand;
#endif /* DOT11N_SS3_SUPPORT */
                }
            }
        }
#endif /* DOT11_N_SUPPORT */
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode (SupRateLen=%d, ExtRateLen=%d, MCSSet[0]=0x%x, MCSSet[1]=0x%x)\n",
                pAd->StaActive.SupRateLen,
                pAd->StaActive.ExtRateLen,
                pAd->StaActive.SupportedPhyInfo.MCSSet[0],
                pAd->StaActive.SupportedPhyInfo.MCSSet[1]));
    }
#endif /* CONFIG_STA_SUPPORT */

    return pTable;
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


#ifdef AGS_SUPPORT

UCHAR* SelectTxRateTableAGS(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;

#ifdef CONFIG_STA_SUPPORT
    if ((pAd->OpMode == OPMODE_STA) && ADHOC_ON(pAd))
    {
        /* for ADHOC mode */
#ifdef DOT11_N_SUPPORT
        if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && 
                (pEntry->HTCapability.MCSSet[0] != 0x00) && 
                ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))
        {/* 11N 1S Adhoc*/

            if (SUPPORT_AGS(pAd)) {
                pTable = AGS1x1HTRateTable;
            } else {
                if (pAd->LatchRfRegs.Channel <= 14) {
                    pTable = RateSwitchTable11N1S;
                } else {
                    pTable = RateSwitchTable11N1SForABand;
                }
            }
        }
        else if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && 
                (pEntry->HTCapability.MCSSet[0] != 0x00) && 
                (pEntry->HTCapability.MCSSet[1] != 0x00) &&
                (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->Antenna.field.TxPath == 2)))
        {/* 11N 2S Adhoc*/
            if (SUPPORT_AGS(pAd))
            {
                pTable = AGS2x2HTRateTable;
            }
            else
            {
                if (pAd->LatchRfRegs.Channel <= 14) {
                    pTable = RateSwitchTable11N2S;
                } else {
                    pTable = RateSwitchTable11N2SForABand;
                }
            }
        }
#ifdef AGS_SUPPORT
        else if (SUPPORT_AGS(pAd) && 
                (pEntry->HTCapability.MCSSet[0] != 0x00) && 
                (pEntry->HTCapability.MCSSet[1] != 0x00) && 
                (pEntry->HTCapability.MCSSet[2] != 0x00) && 
                (pAd->Antenna.field.TxPath == 3))
        {
            pTable = AGS3x3HTRateTable;
        }
#endif /* AGS_SUPPORT */
        else
#endif /* DOT11_N_SUPPORT */
        if ((pEntry->RateLen == 4)
#ifdef DOT11_N_SUPPORT
                && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
        )
        {
            pTable = RateSwitchTable11B;
        }
        else if (pAd->LatchRfRegs.Channel <= 14)
        {

            pTable = RateSwitchTable11BG;
        }
        else
        {
            pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
    /*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
    {/* 11BGN 1S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd)) {
            pTable = AGS1x1HTRateTable;
        } else
#endif /* AGS_SUPPORT */
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN1S;
        } else {
            pTable = RateSwitchTable11N1SForABand;
        }

        return pTable;
    }

#ifdef AGS_SUPPORT
    /* only for station */
    if (SUPPORT_AGS(pAd) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (pEntry->HTCapability.MCSSet[2] != 0x00) && 
            (pAd->CommonCfg.TxStream == 3))
    {/* 11N 3S */
        pTable = AGS3x3HTRateTable;
        return pTable;
    }
#endif /* AGS_SUPPORT */

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
#ifdef THERMAL_PROTECT_SUPPORT
            (pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
            (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->CommonCfg.TxStream == 2)))
    {/* 11BGN 2S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd))
        {
            pTable = AGS2x2HTRateTable;
        }
        else
#endif /* AGS_SUPPORT */
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11BGN2S;
        else
            pTable = RateSwitchTable11BGN2SForABand;

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (pAd->CommonCfg.TxStream == 3))
    {
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11N3S;
        else
            pTable = RateSwitchTable11N3SForABand;

        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)
#ifdef THERMAL_PROTECT_SUPPORT
            || (pAd->force_one_tx_stream == TRUE)
#endif /* THERMAL_PROTECT_SUPPORT */
    ))
    {/* 11N 1S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd))
            pTable = AGS1x1HTRateTable;
        else
#endif /* AGS_SUPPORT */
        {
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11N1S;
            else
                pTable = RateSwitchTable11N1SForABand;
        }
        return pTable;
    }

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (pAd->CommonCfg.TxStream == 2))
    {/* 11N 2S AP*/
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd))
            pTable = AGS2x2HTRateTable;
        else
#endif /* AGS_SUPPORT */
        {
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11N2S;
            else
                pTable = RateSwitchTable11N2SForABand;			
        }
        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (pAd->CommonCfg.TxStream == 3))
    {
        {
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11N3S;
            else
                pTable = RateSwitchTable11N3SForABand;
        }
        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
    if (pAd->CommonCfg.TxStream == 3)
    {
        if  (pEntry->HTCapability.MCSSet[0] != 0x00)
        {
            if (pEntry->HTCapability.MCSSet[1] == 0x00)
            {	/* Only support 1SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;	
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;
                }
                break;
            }
            else if (pEntry->HTCapability.MCSSet[2] == 0x00)
            {	/* Only support 2SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11BGN2S;
                    else
                        pTable = RateSwitchTable11BGN2SForABand;
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N2S;
                    else
                        pTable = RateSwitchTable11N2SForABand;
                }
                return pTable;
            }
				/* For 3SS case, we use the new rate table, so don't care it here */
        }
    }
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

    if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || 
            WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
#ifdef DOT11_N_SUPPORT
            /*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
            /* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
    )
    {/* B only AP*/
        pTable = RateSwitchTable11B;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
            (pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* B/G  mixed AP*/
        pTable = RateSwitchTable11BG;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* G only AP*/
        pTable = RateSwitchTable11G;
        return pTable;
    }
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
#ifdef DOT11N_SS3_SUPPORT
        if (pAd->CommonCfg.TxStream >= 3)
        {
            if (pEntry->HTCapability.MCSSet[2] == 0)
                pTable = RateSwitchTable11N2S;
            else
                pTable = RateSwitchTable11N3S;
        }
        else
#endif /* DOT11N_SS3_SUPPORT */
        {
            /*
                Temp solution for:
                EX: when the extend rate only supports 6, 12, 24 in
                the association req frame. So the pEntry->RateLen is 7.
            */
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11BG;
            else
                pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
    {
#ifdef DOT11_N_SUPPORT
        /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
        if ((pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
#endif /* DOT11_N_SUPPORT */
        {	/* Legacy mode*/
            if (pAd->CommonCfg.MaxTxRate <= RATE_11)
            {
                pTable = RateSwitchTable11B;
            }
            else if ((pAd->CommonCfg.MaxTxRate > RATE_11) && (pAd->CommonCfg.MinTxRate > RATE_11))
            {
                pTable = RateSwitchTable11G;
            }
            else
            {
                pTable = RateSwitchTable11BG;
            }
            break;
        }
#ifdef DOT11_N_SUPPORT
#ifdef AGS_SUPPORT
        if (SUPPORT_AGS(pAd) && (pAd->CommonCfg.TxStream == 3))
            pTable = AGS3x3HTRateTable;
        else
#endif /* AGS_SUPPORT */
        {
            if (pAd->LatchRfRegs.Channel <= 14)
            {
                if (pAd->CommonCfg.TxStream == 1)
                {
                    pTable = RateSwitchTable11N1S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                }
                else if (pAd->CommonCfg.TxStream == 2)
                {
                    pTable = RateSwitchTable11N2S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));	
                }
                else
                {
#ifdef DOT11N_SS3_SUPPORT
                        pTable = RateSwitchTable11N3S;

#else
                    pTable = RateSwitchTable11N2S;
#endif /* DOT11N_SS3_SUPPORT */
                }
            }
            else
            {
                if (pAd->CommonCfg.TxStream == 1)
                {
                    pTable = RateSwitchTable11N1S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                }
                else if (pAd->CommonCfg.TxStream == 2)
                {
                    pTable = RateSwitchTable11N2S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));	
                }
                else
                {
#ifdef DOT11N_SS3_SUPPORT
                    pTable = RateSwitchTable11N3S;
#else
                    pTable = RateSwitchTable11N2SForABand;
#endif /* DOT11N_SS3_SUPPORT */
                }
            }
        }
#endif /* DOT11_N_SUPPORT */
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode (SupRateLen=%d, ExtRateLen=%d, MCSSet[0]=0x%x, MCSSet[1]=0x%x)\n",
                pAd->StaActive.SupRateLen,
                pAd->StaActive.ExtRateLen,
                pAd->StaActive.SupportedPhyInfo.MCSSet[0],
                pAd->StaActive.SupportedPhyInfo.MCSSet[1]));
    }
#endif /* CONFIG_STA_SUPPORT */

    return pTable;
}
#endif /* AGS_SUPPORT */


UCHAR* SelectTxRateTable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
    UCHAR *pTable = NULL;

#ifdef CONFIG_STA_SUPPORT
    if ((pAd->OpMode == OPMODE_STA) && ADHOC_ON(pAd))
    {
        /* for ADHOC mode */
#ifdef DOT11_N_SUPPORT
        if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && 
                (pEntry->HTCapability.MCSSet[0] != 0x00) && 
                ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))
        {/* 11N 1S Adhoc*/

            if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N1S;
            } else {
                pTable = RateSwitchTable11N1SForABand;
            }
        }
        else if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && 
                (pEntry->HTCapability.MCSSet[0] != 0x00) && 
                (pEntry->HTCapability.MCSSet[1] != 0x00) &&
                (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->Antenna.field.TxPath == 2)))
        {/* 11N 2S Adhoc*/
            if (pAd->LatchRfRegs.Channel <= 14) {
                pTable = RateSwitchTable11N2S;
            } else {
                pTable = RateSwitchTable11N2SForABand;
            }
        }
        else
#endif /* DOT11_N_SUPPORT */
        if ((pEntry->RateLen == 4)
#ifdef DOT11_N_SUPPORT
                && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
        )
        {
            pTable = RateSwitchTable11B;
        }
        else if (pAd->LatchRfRegs.Channel <= 14)
        {
            pTable = RateSwitchTable11BG;
        }
        else
        {
            pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
    /*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
    {/* 11BGN 1S AP*/
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE))) {
            pTable = RateSwitchTable11BGN1S;
        } else {
            pTable = RateSwitchTable11N1SForABand;
        }

        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
    /*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
#ifdef THERMAL_PROTECT_SUPPORT
            (pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
            (((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->CommonCfg.TxStream == 2)))
    {/* 11BGN 2S AP*/
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11BGN2S;
        else
            pTable = RateSwitchTable11BGN2SForABand;

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
            (pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (pAd->CommonCfg.TxStream == 3))
    {
        if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
            pTable = RateSwitchTable11N3S;
        else
            pTable = RateSwitchTable11N3SForABand;

        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)
#ifdef THERMAL_PROTECT_SUPPORT
            || (pAd->force_one_tx_stream == TRUE)
#endif /* THERMAL_PROTECT_SUPPORT */
    ))
    {/* 11N 1S AP*/
        if (pAd->LatchRfRegs.Channel <= 14)
            pTable = RateSwitchTable11N1S;
        else
            pTable = RateSwitchTable11N1SForABand;

        return pTable;
    }

    /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
            (pEntry->HTCapability.MCSSet[1] != 0x00) && 
            (pAd->CommonCfg.TxStream == 2))
    {/* 11N 2S AP*/

        if (pAd->LatchRfRegs.Channel <= 14)
            pTable = RateSwitchTable11N2S;
        else
            pTable = RateSwitchTable11N2SForABand;			

        return pTable;
    }

#ifdef DOT11N_SS3_SUPPORT
    if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
            (pEntry->HTCapability.MCSSet[1] != 0x00) &&
            (pEntry->HTCapability.MCSSet[2] != 0x00) &&
            (pAd->CommonCfg.TxStream == 3))
    {
        if (pAd->LatchRfRegs.Channel <= 14)
            pTable = RateSwitchTable11N3S;
        else
            pTable = RateSwitchTable11N3SForABand;
        return pTable;
    }
#endif /* DOT11N_SS3_SUPPORT */

#ifdef DOT11N_SS3_SUPPORT
    if (pAd->CommonCfg.TxStream == 3)
    {
        if  (pEntry->HTCapability.MCSSet[0] != 0x00)
        {
            if (pEntry->HTCapability.MCSSet[1] == 0x00)
            {	/* Only support 1SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;	
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N1S;
                    else
                        pTable = RateSwitchTable11N1SForABand;
                }
                return pTable;
            }
            else if (pEntry->HTCapability.MCSSet[2] == 0x00)
            {	/* Only support 2SS */
                if (pEntry->RateLen > 0)
                {
                    if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
                        pTable = RateSwitchTable11BGN2S;
                    else
                        pTable = RateSwitchTable11BGN2SForABand;
                }
                else
                {
                    if (pAd->LatchRfRegs.Channel <= 14)
                        pTable = RateSwitchTable11N2S;
                    else
                        pTable = RateSwitchTable11N2SForABand;
                }
                return pTable;
            }
				/* For 3SS case, we use the new rate table, so don't care it here */
        }
    }
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

    if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || 
            WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
#ifdef DOT11_N_SUPPORT
            /*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
            /* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
    )
    {/* B only AP*/
        pTable = RateSwitchTable11B;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
            (pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* B/G  mixed AP*/
        pTable = RateSwitchTable11BG;			
        return pTable;
    }

    /*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
    if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
            && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
    )
    {/* G only AP*/
        pTable = RateSwitchTable11G;
        return pTable;
    }
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
    {
#ifdef DOT11N_SS3_SUPPORT
        if (pAd->CommonCfg.TxStream >= 3)
        {
            if (pEntry->HTCapability.MCSSet[2] == 0)
                pTable = RateSwitchTable11N2S;
            else
                pTable = RateSwitchTable11N3S;
        }
        else
#endif /* DOT11N_SS3_SUPPORT */
        {
            /*
                Temp solution for:
                EX: when the extend rate only supports 6, 12, 24 in
                the association req frame. So the pEntry->RateLen is 7.
            */
            if (pAd->LatchRfRegs.Channel <= 14)
                pTable = RateSwitchTable11BG;
            else
                pTable = RateSwitchTable11G;
        }
        return pTable;
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
    IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
    {
#ifdef DOT11_N_SUPPORT
        /*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
        if ((pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
#endif /* DOT11_N_SUPPORT */
        {	/* Legacy mode*/
            if (pAd->CommonCfg.MaxTxRate <= RATE_11)
            {
                pTable = RateSwitchTable11B;
            }
            else if ((pAd->CommonCfg.MaxTxRate > RATE_11) && (pAd->CommonCfg.MinTxRate > RATE_11))
            {
                pTable = RateSwitchTable11G;
            }
            else
            {
                pTable = RateSwitchTable11BG;
            }
            return pTable;
        }
#ifdef DOT11_N_SUPPORT
        {
            if (pAd->LatchRfRegs.Channel <= 14)
            {
                if (pAd->CommonCfg.TxStream == 1)
                {
                    pTable = RateSwitchTable11N1S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                }
                else if (pAd->CommonCfg.TxStream == 2)
                {
                    pTable = RateSwitchTable11N2S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));	
                }
                else
                {
#ifdef DOT11N_SS3_SUPPORT
                        pTable = RateSwitchTable11N3S;

#else
                    pTable = RateSwitchTable11N2S;
#endif /* DOT11N_SS3_SUPPORT */
                }
            }
            else
            {
                if (pAd->CommonCfg.TxStream == 1)
                {
                    pTable = RateSwitchTable11N1S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
                }
                else if (pAd->CommonCfg.TxStream == 2)
                {
                    pTable = RateSwitchTable11N2S;
                    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));	
                }
                else
                {
#ifdef DOT11N_SS3_SUPPORT
                        pTable = RateSwitchTable11N3S;
#else
                    pTable = RateSwitchTable11N2SForABand;
#endif /* DOT11N_SS3_SUPPORT */
                }
            }
        }
#endif /* DOT11_N_SUPPORT */
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("DRS: unkown mode (SupRateLen=%d, ExtRateLen=%d, MCSSet[0]=0x%x, MCSSet[1]=0x%x)\n",
                pAd->StaActive.SupRateLen,
                pAd->StaActive.ExtRateLen,
                pAd->StaActive.SupportedPhyInfo.MCSSet[0],
                pAd->StaActive.SupportedPhyInfo.MCSSet[1]));
    }
#endif /* CONFIG_STA_SUPPORT */
    return pTable;
}


VOID MlmeSelectTxRateTable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR **ppTable,
	IN UCHAR *pTableSize,
	IN UCHAR *pInitTxRateIdx)
{
    *ppTable = NULL;

	do
	{

#ifdef NEW_RATE_ADAPT_SUPPORT
        if (pAd->rateAlg == RATE_ALG_GRP) {
            *ppTable = SelectTxRateTableGRP(pAd, pEntry);
        }
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef AGS_SUPPORT
        if (pAd->rateAlg == RATE_ALG_AGS) {
            *ppTable = SelectTxRateTableAGS(pAd, pEntry);
        }
#endif /* AGS_SUPPORT */

        if (*ppTable == NULL)
            *ppTable = SelectTxRateTable(pAd, pEntry);

	} while(FALSE);

    if ( *ppTable)
    {
        *pTableSize = RATE_TABLE_SIZE(*ppTable);
        *pInitTxRateIdx = RATE_TABLE_INIT_INDEX(*ppTable);
        pEntry->LowestTxRateIndex = ra_get_lowest_rate(pAd, *ppTable);    /* update the LowestRateIndex */
    }

}


/*
	MlmeSelectTxRate - select the MCS based on the RSSI and the available MCSs
		pAd - pointer to adapter
		pEntry - pointer to MAC table entry
		mcs - table of MCS index into the Rate Table. -1 => not supported
		Rssi - the Rssi value
		RssiOffset - offset to apply to the Rssi
*/
UCHAR MlmeSelectTxRate(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN CHAR		mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset)
{
	UCHAR TxRateIdx = 0;
	UCHAR *pTable = pEntry->pTable;

#ifdef DOT11_N_SUPPORT
#ifdef NEW_RATE_ADAPT_SUPPORT
#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef DOT11N_SS3_SUPPORT
	if ((pTable == RateSwitchTable11BGN3S) || (pTable == RateSwitchTable11N3S) || (pTable == RateSwitchTable11BGN3SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N3S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 3 stream */
		if (mcs[23]>=0 && (Rssi >= (-66+RssiOffset)) && (pEntry->SupportHTMCS[MCS_23]))
			TxRateIdx = mcs[23];
		else if (mcs[22]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportHTMCS[MCS_22]))
			TxRateIdx = mcs[22];
		else if (mcs[21]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportHTMCS[MCS_21]))
			TxRateIdx = mcs[21];
		else if (mcs[20]>=0 && (Rssi >= (-74+RssiOffset)) && (pEntry->SupportHTMCS[MCS_20]))
			TxRateIdx = mcs[20];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportHTMCS[MCS_13]))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportHTMCS[MCS_12]))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportHTMCS[MCS_4]))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportHTMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportHTMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportHTMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else
#endif /*  DOT11N_SS3_SUPPORT */
	if ((pTable == RateSwitchTable11BGN2S) || (pTable == RateSwitchTable11BGN2SForABand) ||
		(pTable == RateSwitchTable11N2S) || (pTable == RateSwitchTable11N2SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N2S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 2 stream */
		if (mcs[15]>=0 && (Rssi >= (-70+RssiOffset)) && (pEntry->SupportHTMCS[MCS_15]))
			TxRateIdx = mcs[15];
		else if (mcs[14]>=0 && (Rssi >= (-72+RssiOffset)) && (pEntry->SupportHTMCS[MCS_14]))
			TxRateIdx = mcs[14];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)) && (pEntry->SupportHTMCS[MCS_13]))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)) && (pEntry->SupportHTMCS[MCS_12]))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)) && (pEntry->SupportHTMCS[MCS_4]))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)) && (pEntry->SupportHTMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)) && (pEntry->SupportHTMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)) && (pEntry->SupportHTMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else if ((pTable == RateSwitchTable11BGN1S) ||
			 (pTable == RateSwitchTable11N1S) ||
			 (pTable == RateSwitchTable11N1SForABand)
#ifdef NEW_RATE_ADAPT_SUPPORT
			|| (pTable == RateSwitchTableAdapt11N1S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
	)
	{/*  N mode with 1 stream */
		{
			if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)) && (pEntry->SupportHTMCS[MCS_7]))
				TxRateIdx = mcs[7];
			else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)) && (pEntry->SupportHTMCS[MCS_6]))
				TxRateIdx = mcs[6];
			else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)) && (pEntry->SupportHTMCS[MCS_5]))
				TxRateIdx = mcs[5];
			else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)) && (pEntry->SupportHTMCS[MCS_4]))
				TxRateIdx = mcs[4];
			else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)) && (pEntry->SupportHTMCS[MCS_3]))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)) && (pEntry->SupportHTMCS[MCS_2]))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)) && (pEntry->SupportHTMCS[MCS_1]))
				TxRateIdx = mcs[1];
			else
				TxRateIdx = mcs[0];
		}
	}
	else
#endif /*  DOT11_N_SUPPORT */
	{/*  Legacy mode */
		if (mcs[7]>=0 && (Rssi > -70) && (pEntry->SupportOFDMMCS[MCS_7]))
		TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > -74) && (pEntry->SupportOFDMMCS[MCS_7]))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > -78) && (pEntry->SupportOFDMMCS[MCS_7]))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > -82) && (pEntry->SupportOFDMMCS[MCS_7]))
			TxRateIdx = mcs[4];
		else if (mcs[4] == -1)							/*  for B-only mode */
		{
			if (mcs[3]>=0 && (Rssi > -85) && (pEntry->SupportCCKMCS[MCS_3]))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > -87) && (pEntry->SupportCCKMCS[MCS_2]))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > -90) && (pEntry->SupportCCKMCS[MCS_1]))
				TxRateIdx = mcs[1];
			else if (pEntry->SupportCCKMCS[MCS_0])
				TxRateIdx = mcs[0];
			else
			TxRateIdx = mcs[3];
		}
		else if (mcs[3]>=0 && (Rssi > -85) && (pEntry->SupportOFDMMCS[MCS_3]))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > -87) && (pEntry->SupportOFDMMCS[MCS_2]))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > -90) && (pEntry->SupportOFDMMCS[MCS_1]))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}


	return TxRateIdx;
}


/*  MlmeRAInit - Initialize Rate Adaptation for this entry */
VOID MlmeRAInit(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	MlmeSetMcsGroup(pAd, pEntry);

	//pEntry->lastRateIdx = 1;
	pEntry->lastRateIdx = 0xFF;
	pEntry->lowTrafficCount = 0;
	pEntry->perThrdAdj = PER_THRD_ADJ;
	pEntry->TrafficLoading = RA_INIT_STATE;
	pEntry->RaHoldTime = 0;
#endif /* NEW_RATE_ADAPT_SUPPORT */


	pEntry->fLastSecAccordingRSSI = FALSE;
	pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
	pEntry->CurrTxRateIndex = 0;
	pEntry->TxRateUpPenalty = 0;

	MlmeClearAllTxQuality(pEntry);
}


/* #define TIMESTAMP_RA_LOG	*/ /* Include timestamp in RA Log */

/*
	MlmeRALog - Prints concise Rate Adaptation log entry
		The BF percentage counters are also updated
*/
VOID MlmeRALog(
	IN PRTMP_ADAPTER	pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN RA_LOG_TYPE		raLogType,
	IN ULONG			TxErrorRatio,
	IN ULONG			TxTotalCnt)
{
#ifdef TIMESTAMP_RA_LOG
	ULONG newTime;
	static ULONG saveRATime;
	struct timeval tval;

	do_gettimeofday(&tval);
	newTime = (tval.tv_sec*1000000L + tval.tv_usec);
#endif

	if (TxTotalCnt !=0 || raLogType==RAL_QUICK_DRS
#ifdef DBG_CTRL_SUPPORT
		|| (pAd->CommonCfg.DebugFlags & DBF_SHOW_ZERO_RA_LOG)
#endif /* DBG_CTRL_SUPPORT */
	)
	{
		BOOLEAN stbc, csd=FALSE;
		ULONG tp;

		/*  Get STBC and StreamMode state */
		stbc = (pEntry->HTPhyMode.field.STBC && pEntry->HTPhyMode.field.MCS<8);

#ifdef STREAM_MODE_SUPPORT
		if (pEntry->StreamModeMACReg != 0)
		{
			ULONG streamWord;

			RTMP_IO_READ32(pAd, pEntry->StreamModeMACReg+4, &streamWord);
			if (pEntry->HTPhyMode.field.MCS < 8)
				csd = (streamWord & 0x30000)==0x30000;
			else if (pEntry->HTPhyMode.field.MCS < 16)
				csd = (streamWord & 0xC0000)==0xC0000;
		}
#endif /* STREAM_MODE_SUPPORT */

		/*  Normalized throughput - packets per RA Interval */
		if (raLogType==RAL_QUICK_DRS)
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*pAd->ra_fast_interval);
		else if (pEntry->LastSecTxRateChangeAction==RATE_NO_CHANGE
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
		)
			tp = (100-TxErrorRatio)*TxTotalCnt/100;
		else
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*(RA_INTERVAL-pAd->ra_fast_interval));

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG)
		{
			struct {
				USHORT phyMode;
				USHORT per;
				USHORT tp;
				USHORT bfRatio;
			} raLogInfo;

			raLogInfo.phyMode = pEntry->HTPhyMode.word;
			raLogInfo.per = TxErrorRatio;
			raLogInfo.tp = tp;
			dbQueueEnqueue(0x7e, (UCHAR *)&raLogInfo);
		}
		else
#endif /*  INCLUDE_DEBUG_QUEUE */
#endif /*  DBG_CTRL_SUPPORT */
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s[%d]: M=%d %c%c%c%c- PER=%ld%% TP=%ld ",
				raLogType==RAL_QUICK_DRS? " Q": (raLogType==RAL_NEW_DRS? "\nRA": "\nra"),
				pEntry->wcid, pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.MODE==MODE_CCK? 'C': (pEntry->HTPhyMode.field.ShortGI? 'S': 'L'),
				pEntry->HTPhyMode.field.BW? '4': '2',
				stbc? 'S': 's',
				csd? 'C': 'c',
				TxErrorRatio, tp) );
		}
	}

#ifdef TIMESTAMP_RA_LOG
	saveRATime = newTime;
#endif
}


/*  MlmeRestoreLastRate - restore last saved rate */
VOID MlmeRestoreLastRate(
	IN PMAC_TABLE_ENTRY	pEntry)
{
	pEntry->CurrTxRateIndex = pEntry->lastRateIdx;
}


#ifdef DOT11N_SS3_SUPPORT
/*  MlmeCheckRDG - check if RDG should be enabled or disabled */
VOID MlmeCheckRDG(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	PUCHAR pTable = pEntry->pTable;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return;
	}

	/*  Turn off RDG when 3s and rx count > tx count*5 */
	if (((pTable == RateSwitchTable11BGN3S) || 
		(pTable == RateSwitchTable11BGN3SForABand) || 
		(pTable == RateSwitchTable11N3S)
#ifdef NEW_RATE_ADAPT_SUPPORT
		|| (pTable == RateSwitchTableAdapt11N3S)
#endif /* NEW_RATE_ADAPT_SUPPORT */
		) && pAd->RalinkCounters.OneSecReceivedByteCount > 50000 &&
		pAd->RalinkCounters.OneSecTransmittedByteCount > 50000 &&
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE))
	{
		TX_LINK_CFG_STRUC TxLinkCfg;
		UINT32 TxOpThres;
		UCHAR				TableStep;
		RTMP_RA_LEGACY_TB *pTempTxRate;

#ifdef NEW_RATE_ADAPT_SUPPORT
		TableStep = ADAPT_RATE_TABLE(pTable)? 10: 5;
#else
		TableStep = 5;
#endif

		pTempTxRate = (RTMP_RA_LEGACY_TB *)(&pTable[(pEntry->CurrTxRateIndex + 1)*TableStep]);
		RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
		if (pAd->RalinkCounters.OneSecReceivedByteCount > (pAd->RalinkCounters.OneSecTransmittedByteCount * 5) &&
				pTempTxRate->CurrMCS != 23 && pTempTxRate->ShortGI != 1)
		{
			if (TxLinkCfg.field.TxRDGEn == 1)
			{
				TxLinkCfg.field.TxRDGEn = 0;
				RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
				RTMP_IO_READ32(pAd, TXOP_THRES_CFG, &TxOpThres);
				TxOpThres |= 0xff00;
				RTMP_IO_WRITE32(pAd, TXOP_THRES_CFG, TxOpThres);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("DRS: RDG off!\n"));
			}
		}
		else
		{
			if (TxLinkCfg.field.TxRDGEn == 0)
			{
				TxLinkCfg.field.TxRDGEn = 1;
				RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
				RTMP_IO_READ32(pAd, TXOP_THRES_CFG, &TxOpThres);
				TxOpThres &= 0xffff00ff;
				RTMP_IO_WRITE32(pAd, TXOP_THRES_CFG, TxOpThres);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("DRS: RDG on!\n"));
			}
		}
	}
}
#endif /*  DOT11N_SS3_SUPPORT */




#ifdef NEW_RATE_ADAPT_SUPPORT
UCHAR ra_get_lowest_rate(RTMP_ADAPTER *pAd, UCHAR *pTable)
{
	RTMP_RA_GRP_TB *pNextTxRate;
	UCHAR TxRateIndex, NextTxRateIndex;

	if (ADAPT_RATE_TABLE(pTable))
	{
		NextTxRateIndex = RATE_TABLE_INIT_INDEX(pTable);
	
		do {
			TxRateIndex = NextTxRateIndex;
			/*  Get pointer to CurrTxRate entry */
			pNextTxRate = PTX_RA_GRP_ENTRY(pTable, TxRateIndex);
			NextTxRateIndex = pNextTxRate->downMcs;
		} while ( TxRateIndex != NextTxRateIndex );
	} else {
		TxRateIndex = 0;
	}

	return TxRateIndex;
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


INT rtmp_get_rate_from_rate_tb(UCHAR *table, INT idx, RTMP_TX_RATE *tx_rate)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(table)) {
		RTMP_RA_GRP_TB *rate_entry;

		rate_entry = PTX_RA_GRP_ENTRY(table, idx);
		tx_rate->mode = rate_entry->Mode;
		tx_rate->bw = rate_entry->BW;
		tx_rate->mcs = rate_entry->CurrMCS;
		tx_rate->sgi = rate_entry->ShortGI;
		tx_rate->stbc = rate_entry->STBC;
			tx_rate->nss = 0;
	}
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
	{
		RTMP_RA_LEGACY_TB *rate_entry;

		rate_entry = PTX_RA_LEGACY_ENTRY(table, idx);
		tx_rate->mode = rate_entry->Mode;
		tx_rate->bw = rate_entry->BW;
		tx_rate->mcs = rate_entry->CurrMCS;
		tx_rate->sgi = rate_entry->ShortGI;
		tx_rate->stbc = rate_entry->STBC;
		tx_rate->nss = 0;
	}

	return TRUE;
}


/*
	MlmeNewTxRate - called when a new TX rate was selected. Sets TX PHY to
		rate selected by pEntry->CurrTxRateIndex in pTable;
*/
VOID MlmeNewTxRate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	RTMP_RA_LEGACY_TB *pNextTxRate;
	UCHAR *pTable;

	if ((pEntry == NULL) || (pEntry->pTable == NULL))
		return;
	else
		pTable = pEntry->pTable;

	/*  Get pointer to CurrTxRate entry */
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, pEntry->CurrTxRateIndex);
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		pNextTxRate = PTX_RA_LEGACY_ENTRY(pTable, pEntry->CurrTxRateIndex);

#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef WAPI_SUPPORT
    if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd))
    {
        if ((pEntry->AuthMode == Ndis802_11AuthModeWAICERT) || (pEntry->AuthMode == Ndis802_11AuthModeWAIPSK))
        {
            if (pTable == RateSwitchTableAdapt11N2S)
            {
                if ((pEntry->CurrTxRateIndex >= 14) && (pEntry->CurrTxRateIndex <= 16))
                {
                    pNextTxRate = (RTMP_RA_LEGACY_TB *)PTX_RA_GRP_ENTRY(pTable, 13);
                }
            }
        }
    }
#endif /* WAPI_SUPPORT */
#endif /* NEW_RATE_ADAPT_SUPPORT */

	/*  Set new rate */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		APMlmeSetTxRate(pAd, pEntry, pNextTxRate);
	}
#endif /*  CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		MlmeSetTxRate(pAd, pEntry, pNextTxRate);
	}
#endif /*  CONFIG_STA_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/*  Disable invalid HT Duplicate modes to prevent PHY error */
	if (pEntry->HTPhyMode.field.MCS==32)
	{
		if ((pEntry->HTPhyMode.field.BW!=BW_40) && (pEntry->HTPhyMode.field.BW!=BW_80))
			pEntry->HTPhyMode.field.MCS = 0;
		else
			pEntry->HTPhyMode.field.STBC = 0;
	}
#endif /*  DOT11_N_SUPPORT */


	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);

#ifdef STREAM_MODE_SUPPORT
	/*  Enable/disable stream mode based on MCS */
	if (pAd->CommonCfg.StreamMode!=0 &&
		pEntry->StreamModeMACReg!=0)
	{
		UINT streamWord;
		BOOLEAN mcsDisable;

		/* OFDM: depends on StreamModeMCS, CCK: always applies stream-mode */
		mcsDisable = (pEntry->HTPhyMode.field.MCS < 16) &&
				(pAd->CommonCfg.StreamModeMCS & (1<<pEntry->HTPhyMode.field.MCS))==0 &&
				(pEntry->HTPhyMode.field.MODE != MODE_CCK);

		streamWord = mcsDisable ? 0 : StreamModeRegVal(pAd);

		/*  Update Stream Mode control reg */
		RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, streamWord | (ULONG)(pEntry->Addr[4]) | (ULONG)(pEntry->Addr[5] << 8));
	}
#endif /* STREAM_MODE_SUPPORT */
}


VOID RTMPSetSupportMCS(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR OpMode,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen)
{
	UCHAR idx, SupportedRatesLen = 0;
	UCHAR SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];

	if (SupRateLen > 0)
	{
		if (SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)
		{
			NdisMoveMemory(SupportedRates, SupRate, SupRateLen);
			SupportedRatesLen = SupRateLen;
		}
		else
		{
			UCHAR RateDefault[8] = {0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c};
			
			NdisMoveMemory(SupportedRates, RateDefault, 8);
			SupportedRatesLen = 8;
			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s():wrong SUPP RATES., Len=%d\n",
							__FUNCTION__, SupRateLen));
		}
	}

	if (ExtRateLen > 0)
	{
		if ((SupRateLen + ExtRateLen) <= MAX_LEN_OF_SUPPORTED_RATES)
		{
			NdisMoveMemory(&SupportedRates[SupRateLen], ExtRate, ExtRateLen);
			SupportedRatesLen += ExtRateLen;
		}
		else
		{
			NdisMoveMemory(&SupportedRates[SupRateLen], ExtRate, MAX_LEN_OF_SUPPORTED_RATES - ExtRateLen);
			SupportedRatesLen = MAX_LEN_OF_SUPPORTED_RATES;

		}
	}

	/* Clear Supported MCS Table */
	NdisZeroMemory(pEntry->SupportCCKMCS, MAX_LEN_OF_CCK_RATES);
	NdisZeroMemory(pEntry->SupportOFDMMCS, MAX_LEN_OF_OFDM_RATES);
	NdisZeroMemory(pEntry->SupportHTMCS, MAX_LEN_OF_HT_RATES);

	pEntry->SupportRateMode = 0;

	for(idx = 0; idx < SupportedRatesLen; idx ++)
	{
		switch((SupportedRates[idx] & 0x7F)*5)
		{
			case 10:
				pEntry->SupportCCKMCS[MCS_0] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 20:
				pEntry->SupportCCKMCS[MCS_1] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 55:
				pEntry->SupportCCKMCS[MCS_2] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 110:
				pEntry->SupportCCKMCS[MCS_3] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case 60:
				pEntry->SupportOFDMMCS[MCS_0] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 90:
				pEntry->SupportOFDMMCS[MCS_1] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 120:
				pEntry->SupportOFDMMCS[MCS_2] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 180:
				pEntry->SupportOFDMMCS[MCS_3] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 240:
				pEntry->SupportOFDMMCS[MCS_4] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 360:
				pEntry->SupportOFDMMCS[MCS_5] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 480:
				pEntry->SupportOFDMMCS[MCS_6] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;

			case 540:
				pEntry->SupportOFDMMCS[MCS_7] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_OFDM_MODE;
				break;
		}	
	}

	if (HtCapabilityLen)
	{
		RT_PHY_INFO *pDesired_ht_phy = NULL;
		UCHAR j, bitmask;
		CHAR i;

#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA)
			pDesired_ht_phy = &pAd->StaCfg.wdev.DesiredHtPhyInfo;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
		{
#ifdef WDS_SUPPORT
			if (IS_ENTRY_WDS(pEntry))
				pDesired_ht_phy = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
			else
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pEntry))
				pDesired_ht_phy = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
			else
#endif /* APCLI_SUPPORT */
				pDesired_ht_phy = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo;
		}
#endif /* CONFIG_AP_SUPPORT */

		if (pDesired_ht_phy == NULL)
			return;

		for (i = 23; i >= 0; i--)
		{
			j = i / 8;
			bitmask = (1 << (i - (j * 8)));

			if ((pDesired_ht_phy->MCSSet[j] & bitmask)
				&& (pHtCapability->MCSSet[j] & bitmask))
			{
				pEntry->SupportHTMCS[i] = TRUE;
				pEntry->SupportRateMode |= SUPPORT_HT_MODE;
			}
		}

	}
}


INT	Set_RateAlg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 ra_alg;

	ra_alg = simple_strtol(arg, 0, 10);

	if ((ra_alg < RATE_ALG_MAX_NUM) && (ra_alg != pAd->rateAlg))
	{
		UINT32 IdEntry;

		pAd->rateAlg = ra_alg;
		for(IdEntry = 0; IdEntry < MAX_LEN_OF_MAC_TABLE; IdEntry++)
			pAd->MacTab.Content[IdEntry].rateAlg = ra_alg;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Set Alg = %d\n", __FUNCTION__, ra_alg));
	return TRUE;
}

