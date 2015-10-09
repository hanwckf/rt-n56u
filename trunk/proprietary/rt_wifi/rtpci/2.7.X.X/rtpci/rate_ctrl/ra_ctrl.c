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


UCHAR RateSwitchTable[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x11, 0x00,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
    0x04, 0x21,  0, 30, 50,
    0x05, 0x21,  1, 20, 50,
    0x06, 0x21,  2, 20, 50,
    0x07, 0x21,  3, 15, 50,
    0x08, 0x21,  4, 15, 30,
    0x09, 0x21,  5, 10, 25,
    0x0a, 0x21,  6,  8, 25,
    0x0b, 0x21,  7,  8, 25,
    0x0c, 0x20, 12,  15, 30,
    0x0d, 0x20, 13,  8, 20,
    0x0e, 0x20, 14,  8, 20,
    0x0f, 0x20, 15,  8, 25,
    0x10, 0x22, 15,  8, 25,
    0x11, 0x00,  0,  0,  0,
    0x12, 0x00,  0,  0,  0,
    0x13, 0x00,  0,  0,  0,
    0x14, 0x00,  0,  0,  0,
    0x15, 0x00,  0,  0,  0,
    0x16, 0x00,  0,  0,  0,
    0x17, 0x00,  0,  0,  0,
    0x18, 0x00,  0,  0,  0,
    0x19, 0x00,  0,  0,  0,
    0x1a, 0x00,  0,  0,  0,
    0x1b, 0x00,  0,  0,  0,
    0x1c, 0x00,  0,  0,  0,
    0x1d, 0x00,  0,  0,  0,
    0x1e, 0x00,  0,  0,  0,
    0x1f, 0x00,  0,  0,  0,
};


UCHAR RateSwitchTable11B[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
    0x04, 0x03,  0,  0,  0,						/* Initial used item after association*/
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
};

UCHAR RateSwitchTable11BG[] = {
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.	Mode	Curr-MCS	TrainUp	TrainDown	 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF) */
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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
/* Item No.   Mode   Curr-MCS   TrainUp   TrainDown		 Mode- Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)*/
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


#ifdef NEW_RATE_ADAPT_SUPPORT

#ifdef RANGE_EXTEND
#define SUPPORT_SHORT_GI_RA		/* Support switching to Short GI rates in RA */
#endif /*  RANGE_EXTEND */

/*
	Rate switch tables for New Rate Adaptation

	Each row has 10 bytes of data.
	First row contains table information:
		Byte0=the number of rate entries, Byte1=the initial rate.
	Format of Mode byte:
		Bit0: STBC, Bit1: Short GI, Bit4,5: Mode(0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF)
*/
UCHAR RateSwitchTableAdapt11N1S[] = {
/*  item no.   mcs   highPERThrd  upMcs3     upMcs1
           mode   lowPERThrd  downMcs     upMcs2
*/
	 12,   7,  0,  0,   0,   0,    0,     0,   0,   0,
	 0, 0x21,  0, 30,  50,  11,    1,     8,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,   16,     9,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,   17,     9,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,   17,    10,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,   18,    11,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10,  25,   4,   18,    12,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,   19,    12,   7,  59,/* mcs6 */
	 7, 0x21,  7,  8,  14,   6,   19,    12,   8,  65,/* mcs7 */
	 8, 0x23,  7,  8,  14,   7,   19,    12,   8,  72,/* mcs7+short gi */

	 9, 0x00,  0, 40,  101,  9 ,   9,     9,   10,  1, /* cck-1M */
	10, 0x00,  1, 40,  50,   9,   10,    10,   11,  2, /* cck-2M */
	11, 0x21, 32, 30,  50,  10,    0,     8,    0,  7, /* mcs32 or 20M/mcs0 */
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
     0, 0x21,  0, 30,  50,  21,    1,   8,   1,   7,/* mcs0 */
	 1, 0x21,  1, 20,  50,   0,   16,   9,   2,  13,/* mcs1 */
	 2, 0x21,  2, 20,  50,   1,   17,   9,   3,  20,/* mcs2 */
	 3, 0x21,  3, 15,  50,   2,   17,  10,   4,  26,/* mcs3 */
	 4, 0x21,  4, 15,  30,   3,   18,  11,   5,  39,/* mcs4 */
	 5, 0x21,  5, 10,  25,   4,   18,  12,   6,  52,/* mcs5 */
	 6, 0x21,  6,  8,  14,   5,   19,  12,   7,  59,/* mcs6 */
#ifdef SUPPORT_SHORT_GI_RA
	 7, 0x21,  7,  8,  14,   6,   19,  12, sg07,  65,/* mcs7 */
#else
	 7, 0x21,  7,  8,  14,   6,   19,  12,   7,  65,/* mcs7 */
#endif
	 8, 0x20,  8, 30,  50,   0,   16,   9,   2,  13,/* mcs8 */
	 9, 0x20,  9, 20,  50,   8,   17,  10,   4,  26,/* mcs9 */
	10, 0x20, 10, 20,  40,   9,   18,  11,   5,  39,/* mcs10 */
	11, 0x20, 11, 15,  30,  10,   18,  12,   6,  52,/* mcs11 */
	12, 0x20, 12, 15,  30,  11,   20,  13,  12,  78,/* mcs12 */
	13, 0x20, 13,  8,  20,  12,   20,  14,  13, 104,/* mcs13 */
#ifdef SUPPORT_SHORT_GI_RA
	14, 0x20, 14,  8,  18,  13,   21,  15,sg14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14,   21,sg15,sg14, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15,   21,sg15,sg15, 144,/* mcs15+shortGI */

    17, 0x22, 14,  8,  14,  14,   21,sg15,  15, 130, /* mcs14+shortGI */
    18, 0x23,  7,  8,  14,   7,   19,  12,sg07,  72, /* mcs7+shortGI */
#else
	14, 0x20, 14,  8,  18,  13,   21,  15,  14, 117,/* mcs14 */
	15, 0x20, 15,  8,  25,  14,   21,  16,  15, 130,/* mcs15 */
	16, 0x22, 15,  8,  25,  15,   21,  16,  16, 144,/* mcs15+shortGI */
    17,    0,  0,  0,   0,   0,   0,    0,   0,   0,
    18,    0,  0,  0,   0,   0,   0,    0,   0,   0,
#endif
    19, 0x00,  0, 40,  101, 19 ,  19,    19,   20,  1, /* cck-1M */
    20, 0x00,  1, 40,  50,  19,   20,    20,   21,  2, /* cck-2M */
    21, 0x21, 32, 30,  50,  20,   0,     8,    0,   7, /* mcs32 or 20M/mcs0 */
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
	PRTMP_TX_RATE_SWITCH pCurrTxRate;

	for (idx=0; idx<24; idx++)
		mcs[idx] = -1;

	/*  check the existence and index of each needed MCS */
	for (idx=0; idx<RATE_TABLE_SIZE(pTable); idx++)
	{
		pCurrTxRate = PTX_RATE_SWITCH_ENTRY(pTable, idx);

		/*  Rate Table may contain CCK and MCS rates. Give HT/Legacy priority over CCK */
		if (pCurrTxRate->CurrMCS==MCS_0 && (mcs[0]==-1 || pCurrTxRate->Mode!=MODE_CCK))
		{
			mcs[0] = idx;
		}
		else if (pCurrTxRate->CurrMCS==MCS_1 && (mcs[1]==-1 || pCurrTxRate->Mode!=MODE_CCK))
		{
			mcs[1] = idx;
		}
		else if (pCurrTxRate->CurrMCS==MCS_2 && (mcs[2]==-1 || pCurrTxRate->Mode!=MODE_CCK))
		{
			mcs[2] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_3)
		{
			mcs[3] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_4)
		{
			mcs[4] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_5)
		{
			mcs[5] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_6)
		{
			mcs[6] = idx;
		}
		else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))
		{
			mcs[7] = idx;
		}
#ifdef DOT11_N_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_12)
		{
			mcs[12] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_13)
		{
			mcs[13] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_14)
		{
			mcs[14] = idx;
		}
		else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))
		{
			mcs[15] = idx;
		}
#ifdef DOT11N_SS3_SUPPORT
		else if (pCurrTxRate->CurrMCS == MCS_20)
		{
			mcs[20] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_21)
		{
			mcs[21] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_22)
		{
			mcs[22] = idx;
		}
		else if (pCurrTxRate->CurrMCS == MCS_23)
		{
			mcs[23] = idx;
		}
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
VOID MlmeClearTxQuality(
	IN MAC_TABLE_ENTRY	*pEntry)
{
#ifdef TXBF_SUPPORT
	if (pEntry->phyETxBf || pEntry->phyITxBf)
		NdisZeroMemory(pEntry->BfTxQuality, sizeof(pEntry->BfTxQuality));
	else
#endif /*  TXBF_SUPPORT */
		NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	NdisZeroMemory(pEntry->PER, sizeof(pEntry->PER));
}

/*  MlmeClearAllTxQuality - Clear both BF and non-BF TxQuality history */
VOID MlmeClearAllTxQuality(
	IN MAC_TABLE_ENTRY	*pEntry)
{
#ifdef TXBF_SUPPORT
	NdisZeroMemory(pEntry->BfTxQuality, sizeof(pEntry->BfTxQuality));
#endif
	NdisZeroMemory(pEntry->TxQuality, sizeof(pEntry->TxQuality));

	NdisZeroMemory(pEntry->PER, sizeof(pEntry->PER));
}

/*  MlmeDecTxQuality - Decrement TxQuality of specified rate table entry */
VOID MlmeDecTxQuality(
	IN MAC_TABLE_ENTRY	*pEntry,
	IN UCHAR			rateIndex)
{
#ifdef TXBF_SUPPORT
	if (pEntry->phyETxBf || pEntry->phyITxBf) {
		if (pEntry->BfTxQuality[rateIndex])
			pEntry->BfTxQuality[rateIndex]--;
	}
	else
#endif /*  TXBF_SUPPORT */
	if (pEntry->TxQuality[rateIndex])
		pEntry->TxQuality[rateIndex]--;
}

VOID MlmeSetTxQuality(
	IN MAC_TABLE_ENTRY	*pEntry,
	IN UCHAR			rateIndex,
	IN USHORT			txQuality)
{
#ifdef TXBF_SUPPORT
	if (pEntry->phyETxBf || pEntry->phyITxBf)
		pEntry->BfTxQuality[rateIndex] = txQuality;
	else
#endif /*  TXBF_SUPPORT */
		pEntry->TxQuality[rateIndex] = txQuality;
}


USHORT MlmeGetTxQuality(
	IN MAC_TABLE_ENTRY	*pEntry,
	IN UCHAR			rateIndex)
{
#ifdef TXBF_SUPPORT
	if (pEntry->phyETxBf || pEntry->phyITxBf)
		return pEntry->BfTxQuality[rateIndex];
#endif /*  TXBF_SUPPORT */
	return pEntry->TxQuality[rateIndex];
}


#ifdef CONFIG_AP_SUPPORT
VOID APMlmeSetTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PRTMP_TX_RATE_SWITCH	pTxRate)
{
	UCHAR tx_mode = pTxRate->Mode;

#ifdef DOT11_N_SUPPORT
	if (tx_mode >= MODE_HTMIX)
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
		(pEntry->MaxHTPhyMode.field.BW==BW_20) ||
		(pAd->CommonCfg.BBPCurrentBW==BW_20))
		pEntry->HTPhyMode.field.BW = BW_20;
	else
		pEntry->HTPhyMode.field.BW = BW_40;

#ifdef RANGE_EXTEND
#ifdef NEW_RATE_ADAPT_SUPPORT
	/* 20 MHz Fallback */
	if (tx_mode >= MODE_HTMIX &&
	    pEntry->HTPhyMode.field.BW == BW_40 &&
	    ADAPT_RATE_TABLE(pEntry->pTable))
	{
		if (pEntry->HTPhyMode.field.MCS==32
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0)==0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map HT Duplicate to 20MHz MCS0 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 0;
		}
		else if (pEntry->HTPhyMode.field.MCS==0
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_20MHZ)==0
				&& (pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS1)==0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			/* Map 40MHz MCS0 to 20MHz MCS1 */
			pEntry->HTPhyMode.field.BW = BW_20;
			pEntry->HTPhyMode.field.MCS = 1;
		}
		else if (pEntry->HTPhyMode.field.MCS==8
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

#ifdef FIFO_EXT_SUPPORT
	AsicFifoExtEntryClean(pAd, pEntry);
#endif /* FIFO_EXT_SUPPORT */


	
}
#endif /* CONFIG_AP_SUPPORT */




VOID MlmeSelectTxRateTable(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PUCHAR				*ppTable,
	IN PUCHAR				pTableSize,
	IN PUCHAR				pInitTxRateIdx)
{
	do
	{
		/* decide the rate table for tuning*/
		if (pAd->CommonCfg.TxRateTableSize > 0)
		{
			*ppTable = RateSwitchTable;
			break;
		}


#ifdef DOT11_N_SUPPORT
		/*if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
		/*	((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
		{/* 11BGN 1S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
				*ppTable = AGS1x1HTRateTable;
			else
#endif /* AGS_SUPPORT */
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
				*ppTable = RateSwitchTableAdapt11N1S;
			else
#endif
			if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
				*ppTable = RateSwitchTable11BGN1S;
			else
				*ppTable = RateSwitchTable11N1SForABand;

			break;
		}

#ifdef AGS_SUPPORT
		/* only for station */
		if (SUPPORT_AGS(pAd) && 
			(pEntry->HTCapability.MCSSet[0] != 0x00) && 
			(pEntry->HTCapability.MCSSet[1] != 0x00) && 
			(pEntry->HTCapability.MCSSet[2] != 0x00) && 
			(pAd->CommonCfg.TxStream == 3))
		{/* 11N 3S */
			*ppTable = AGS3x3HTRateTable;
			break;
		}
#endif /* AGS_SUPPORT */

		/*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 12) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&*/
		/*	(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) && 
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00) && 
			(((pAd->Antenna.field.TxPath == 3) && (pEntry->HTCapability.MCSSet[2] == 0x00)) || (pAd->CommonCfg.TxStream == 2)))
		{/* 11BGN 2S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
			{
				*ppTable = AGS2x2HTRateTable;
			}
			else
#endif /* AGS_SUPPORT */
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
				*ppTable = RateSwitchTableAdapt11N2S;
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
				*ppTable = RateSwitchTable11BGN2S;
			else
				*ppTable = RateSwitchTable11BGN2SForABand;

			break;
		}

#ifdef DOT11N_SS3_SUPPORT
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) &&
			(pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00) &&
			(pEntry->HTCapability.MCSSet[2] != 0x00) &&
			(pAd->CommonCfg.TxStream == 3))
		{
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
			{
				*ppTable = RateSwitchTableAdapt11N3S;
			}
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
				*ppTable = RateSwitchTable11N3S;
			else
				*ppTable = RateSwitchTable11N3SForABand;

			break;
		}
#endif /* DOT11N_SS3_SUPPORT */

		/*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && ((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))*/
		if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
			((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
		{/* 11N 1S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
				*ppTable = AGS1x1HTRateTable;
			else
#endif /* AGS_SUPPORT */
			{
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP)
					*ppTable = RateSwitchTableAdapt11N1S;
				else
#endif /* NEW_RATE_ADAPT_SUPPORT */
				if (pAd->LatchRfRegs.Channel <= 14)
					*ppTable = RateSwitchTable11N1S;
				else
					*ppTable = RateSwitchTable11N1SForABand;
			}
			break;
		}

		/*else if ((pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) && (pAd->Antenna.field.TxPath == 2))*/
		if ((pEntry->HTCapability.MCSSet[0] != 0x00) && 
			(pEntry->HTCapability.MCSSet[1] != 0x00) && 
			(pAd->CommonCfg.TxStream == 2))
		{/* 11N 2S AP*/
#ifdef AGS_SUPPORT
			if (SUPPORT_AGS(pAd))
				*ppTable = AGS2x2HTRateTable;
			else
#endif /* AGS_SUPPORT */
			{
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP)
					*ppTable = RateSwitchTableAdapt11N2S;
				else
#endif /* NEW_RATE_ADAPT_SUPPORT */
				if (pAd->LatchRfRegs.Channel <= 14)
					*ppTable = RateSwitchTable11N2S;
				else
					*ppTable = RateSwitchTable11N2SForABand;			
			}
			break;
		}

#ifdef DOT11N_SS3_SUPPORT
		if ((pEntry->HTCapability.MCSSet[0] != 0x00) &&
			(pEntry->HTCapability.MCSSet[1] != 0x00) &&
			(pEntry->HTCapability.MCSSet[2] != 0x00) &&
			(pAd->CommonCfg.TxStream == 3))
		{
#ifdef NEW_RATE_ADAPT_SUPPORT
			if (pAd->rateAlg == RATE_ALG_GRP)
				*ppTable = RateSwitchTableAdapt11N3S;
			else
#endif /* NEW_RATE_ADAPT_SUPPORT */
			{
				if (pAd->LatchRfRegs.Channel <= 14)
					*ppTable = RateSwitchTable11N3S;
				else
					*ppTable = RateSwitchTable11N3SForABand;
			}
			break;
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
#ifdef NEW_RATE_ADAPT_SUPPORT
						if (pAd->rateAlg == RATE_ALG_GRP)
							*ppTable = RateSwitchTableAdapt11N1S;
						else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
							*ppTable = RateSwitchTable11N1S;
						else
							*ppTable = RateSwitchTable11N1SForABand;	
					}
					else
					{
#ifdef NEW_RATE_ADAPT_SUPPORT
						if (pAd->rateAlg == RATE_ALG_GRP)
							*ppTable = RateSwitchTableAdapt11N1S;
						else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if (pAd->LatchRfRegs.Channel <= 14)
							*ppTable = RateSwitchTable11N1S;
						else
							*ppTable = RateSwitchTable11N1SForABand;
					}
					break;
				}
				else if (pEntry->HTCapability.MCSSet[2] == 0x00)
				{	/* Only support 2SS */
					if (pEntry->RateLen > 0)
					{
#ifdef NEW_RATE_ADAPT_SUPPORT
						if (pAd->rateAlg == RATE_ALG_GRP)
							*ppTable = RateSwitchTableAdapt11N2S;
						else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if ((pAd->LatchRfRegs.Channel <= 14) && (pEntry->SupportRateMode & (SUPPORT_CCK_MODE)))
							*ppTable = RateSwitchTable11BGN2S;
						else
							*ppTable = RateSwitchTable11BGN2SForABand;
					}
					else
					{
#ifdef NEW_RATE_ADAPT_SUPPORT
						if (pAd->rateAlg == RATE_ALG_GRP)
							*ppTable = RateSwitchTableAdapt11N2S;
						else
#endif /* NEW_RATE_ADAPT_SUPPORT */
						if (pAd->LatchRfRegs.Channel <= 14)
							*ppTable = RateSwitchTable11N2S;
						else
							*ppTable = RateSwitchTable11N2SForABand;
					}
					break;
				}
				/* For 3SS case, we use the new rate table, so don't care it here */
			}
		}
#endif /* DOT11N_SS3_SUPPORT */
#endif /* DOT11_N_SUPPORT */

		if (((pEntry->SupportRateMode == SUPPORT_CCK_MODE) || pAd->CommonCfg.PhyMode==PHY_11B) 
#ifdef DOT11_N_SUPPORT
		/*Iverson mark for Adhoc b mode,sta will use rate 54  Mbps when connect with sta b/g/n mode */
		/* && (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)*/
#endif /* DOT11_N_SUPPORT */
			)
		{/* B only AP*/
			*ppTable = RateSwitchTable11B;			
			break;
		}

		/*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen > 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
		if ((pEntry->SupportRateMode & (SUPPORT_CCK_MODE)) &&
			(pEntry->SupportRateMode & (SUPPORT_OFDM_MODE))
#ifdef DOT11_N_SUPPORT
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
			)
		{/* B/G  mixed AP*/
			*ppTable = RateSwitchTable11BG;			
			break;
		}

		/*else if ((pAd->StaActive.SupRateLen + pAd->StaActive.ExtRateLen == 8) && (pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0) && (pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0))*/
		if ((pEntry->SupportRateMode & (SUPPORT_OFDM_MODE)) 
#ifdef DOT11_N_SUPPORT
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif /* DOT11_N_SUPPORT */
			)
		{/* G only AP*/
			*ppTable = RateSwitchTable11G;			
			break;
		}
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef DOT11N_SS3_SUPPORT
			if (pAd->CommonCfg.TxStream >= 3)
			{
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (pAd->rateAlg == RATE_ALG_GRP)
				{
					if (pEntry->HTCapability.MCSSet[2] == 0)
						*ppTable = RateSwitchTableAdapt11N2S;
					else
						*ppTable = RateSwitchTableAdapt11N3S;
				}
				else
#endif /* NEW_RATE_ADAPT_SUPPORT */
				{
					if (pEntry->HTCapability.MCSSet[2] == 0)
						*ppTable = RateSwitchTable11N2S;
					else
						*ppTable = RateSwitchTable11N3S;
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
					*ppTable = RateSwitchTable11BG;
				else
					*ppTable = RateSwitchTable11G;
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	} while(FALSE);

	*pTableSize = RATE_TABLE_SIZE(*ppTable);
	*pInitTxRateIdx = RATE_TABLE_INIT_INDEX(*ppTable);

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
		(pTable == RateSwitchTable11N2S) || (pTable == RateSwitchTable11N2SForABand) || (pTable == RateSwitchTable)
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
VOID MlmeRAInit(
	IN PRTMP_ADAPTER	pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	MlmeSetMcsGroup(pAd, pEntry);

	pEntry->lastRateIdx = 1;
	pEntry->lowTrafficCount = 0;
	pEntry->perThrdAdj = PER_THRD_ADJ;
#endif /*  NEW_RATE_ADAPT_SUPPORT */

#ifdef TXBF_SUPPORT
	pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
	pEntry->lastRatePhyTxBf = FALSE;
	pEntry->lastNonBfRate = 0;
#endif /*  TXBF_SUPPORT */

	pEntry->fLastSecAccordingRSSI = FALSE;
	pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
	pEntry->CurrTxRateIndex = 0;
	pEntry->CurrTxRateStableTime = 0;
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
#ifdef TXBF_SUPPORT
	UINT ETxCount = pEntry->TxBFCounters.ETxSuccessCount + pEntry->TxBFCounters.ETxFailCount;
	UINT ITxCount = pEntry->TxBFCounters.ITxSuccessCount + pEntry->TxBFCounters.ITxFailCount;
	UINT TxCount = pEntry->TxBFCounters.TxSuccessCount + pEntry->TxBFCounters.TxFailCount + ETxCount + ITxCount;
	ULONG bfRatio = 0;
#endif /*  TXBF_SUPPORT */
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
		BOOLEAN stbc;
#if defined(DBG) || defined(STREAM_MODE_SUPPORT)
		BOOLEAN csd=FALSE;
#endif
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
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*DEF_QUICK_RA_TIME_INTERVAL);
		else if (pEntry->LastSecTxRateChangeAction==RATE_NO_CHANGE
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
		)
			tp = (100-TxErrorRatio)*TxTotalCnt/100;
		else
			tp = (100-TxErrorRatio)*TxTotalCnt*RA_INTERVAL/(100*(RA_INTERVAL-DEF_QUICK_RA_TIME_INTERVAL));

#ifdef TXBF_SUPPORT
		/*  Compute BF ratio in the last interval */
		if ((TxCount - pEntry->LastTxCount)>0)
		{
			if (pEntry->HTPhyMode.field.eTxBF)
				bfRatio = 100*(ETxCount-pEntry->LastETxCount)/(TxCount - pEntry->LastTxCount);
			else if (pEntry->HTPhyMode.field.iTxBF)
				bfRatio = 100*(ITxCount-pEntry->LastITxCount)/(TxCount - pEntry->LastTxCount);
		}

		if ((pEntry->HTPhyMode.field.eTxBF || pEntry->HTPhyMode.field.iTxBF)
#ifdef DBG_CTRL_SUPPORT
			&& (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG)==0
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR,("%s[%d]: M=%d %c%c%c%c%c PER=%ld%% TP=%ld BF=%ld%% ",
				raLogType==RAL_QUICK_DRS? " Q": (raLogType==RAL_NEW_DRS? "\nRA": "\nra"),
				pEntry->Aid, pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.MODE==MODE_CCK? 'C': (pEntry->HTPhyMode.field.ShortGI? 'S': 'L'),
				pEntry->HTPhyMode.field.BW? '4': '2',
				stbc? 'S': 's',
				csd? 'C': 'c',
				pEntry->HTPhyMode.field.eTxBF? 'E': (pEntry->HTPhyMode.field.iTxBF? 'I': '-'),
				TxErrorRatio, tp, bfRatio) );
		}
		else
#endif /* TXBF_SUPPORT */
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
#ifdef TXBF_SUPPORT
			raLogInfo.bfRatio = bfRatio;
#endif /* TXBF_SUPPORT */
			dbQueueEnqueue(0x7e, (UCHAR *)&raLogInfo);
		}
		else
#endif /*  INCLUDE_DEBUG_QUEUE */
#endif /*  DBG_CTRL_SUPPORT */
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR,("%s[%d]: M=%d %c%c%c%c- PER=%ld%% TP=%ld ",
				raLogType==RAL_QUICK_DRS? " Q": (raLogType==RAL_NEW_DRS? "\nRA": "\nra"),
				pEntry->Aid, pEntry->HTPhyMode.field.MCS,
				pEntry->HTPhyMode.field.MODE==MODE_CCK? 'C': (pEntry->HTPhyMode.field.ShortGI? 'S': 'L'),
				pEntry->HTPhyMode.field.BW? '4': '2',
				stbc? 'S': 's',
				csd? 'C': 'c',
				TxErrorRatio, tp) );
		}
	}

#ifdef TXBF_SUPPORT
	/*  Remember previous counts */
	pEntry->LastETxCount = ETxCount;
	pEntry->LastITxCount = ITxCount;
	pEntry->LastTxCount = TxCount;
#endif /*  TXBF_SUPPORT */
#ifdef TIMESTAMP_RA_LOG
	saveRATime = newTime;
#endif
}


/*  MlmeRestoreLastRate - restore last saved rate */
VOID MlmeRestoreLastRate(
	IN PMAC_TABLE_ENTRY	pEntry)
{
	pEntry->CurrTxRateIndex = pEntry->lastRateIdx;
#ifdef TXBF_SUPPORT
	if (pEntry->eTxBfEnCond>0)
		pEntry->phyETxBf = pEntry->lastRatePhyTxBf;
	else
		pEntry->phyITxBf = pEntry->lastRatePhyTxBf;
#endif /*  TXBF_SUPPORT */
}


#ifdef DOT11N_SS3_SUPPORT
/*  MlmeCheckRDG - check if RDG should be enabled or disabled */
VOID MlmeCheckRDG(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	PUCHAR pTable = pEntry->pTable;

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
		TX_LINK_CFG_STRUC	TxLinkCfg;
		ULONG				TxOpThres;
		UCHAR				TableStep;
		PRTMP_TX_RATE_SWITCH pTempTxRate;

#ifdef NEW_RATE_ADAPT_SUPPORT
		TableStep = ADAPT_RATE_TABLE(pTable)? 10: 5;
#else
		TableStep = 5;
#endif

		pTempTxRate = (PRTMP_TX_RATE_SWITCH)(&pTable[(pEntry->CurrTxRateIndex + 1)*TableStep]);
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
				DBGPRINT_RAW(RT_DEBUG_WARN,("DRS: RDG off!\n"));
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
				DBGPRINT_RAW(RT_DEBUG_WARN,("DRS: RDG on!\n"));
			}
		}
	}
}
#endif /*  DOT11N_SS3_SUPPORT */


/*
	MlmeNewTxRate - called when a new TX rate was selected. Sets TX PHY to
		rate selected by pEntry->CurrTxRateIndex in pTable;
*/
VOID MlmeNewTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry)
{
	PRTMP_TX_RATE_SWITCH pNextTxRate;
	UCHAR *pTable;

	if ((pEntry == NULL) || (pEntry->pTable == NULL))
		return;
	else
		pTable = pEntry->pTable;

	/*  Get pointer to CurrTxRate entry */
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		pNextTxRate = (PRTMP_TX_RATE_SWITCH)PTX_RATE_SWITCH_ENTRY_3S(pTable, pEntry->CurrTxRateIndex);
	else
#endif /*  NEW_RATE_ADAPT_SUPPORT */
		pNextTxRate = PTX_RATE_SWITCH_ENTRY(pTable, pEntry->CurrTxRateIndex);

	/*  Set new rate */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		APMlmeSetTxRate(pAd, pEntry, pNextTxRate);
	}
#endif /*  CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/*  Disable invalid HT Duplicate modes to prevent PHY error */
	if (pEntry->HTPhyMode.field.MCS==32)
	{
		if (pEntry->HTPhyMode.field.BW!=BW_40)
			pEntry->HTPhyMode.field.MCS = 0;
		else
			pEntry->HTPhyMode.field.STBC = 0;
	}
#endif /*  DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgHwTxBfCap)
	{
		/*  If BF has been disabled then force a non-BF rate */
		if (pEntry->eTxBfEnCond==0)
			pEntry->phyETxBf = 0;

		if (pEntry->iTxBfEn==0)
			pEntry->phyITxBf = 0;

#ifdef DBG
		/*  Debug option: In Fixed Rate and BF Unaware mode BF follows the setting of the global option */
		if ((pAd->CommonCfg.FixedRate != -1) 
#ifdef DBG_CTRL_SUPPORT
			|| (pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			if (MlmeTxBfAllowed(pAd, pEntry, pNextTxRate))
			{
				pEntry->phyETxBf = pEntry->eTxBfEnCond > 0;
				pEntry->phyITxBf = pEntry->iTxBfEn;
			}
			else
				pEntry->phyETxBf = pEntry->phyITxBf = 0;
		}
#endif /* DBG */

	   	/*  Set BF options */
		pEntry->HTPhyMode.field.eTxBF = pEntry->phyETxBf;
		pEntry->HTPhyMode.field.iTxBF = pEntry->phyITxBf;

		/*  Give ETxBF priority over ITxBF */
		if (pEntry->HTPhyMode.field.eTxBF)
			pEntry->HTPhyMode.field.iTxBF = 0;

		/*  In ITxBF mode force GI if we have no choice */
		if (pEntry->HTPhyMode.field.iTxBF &&
			(pEntry->OneSecRxLGICount + pEntry->OneSecRxSGICount) > 10)
		{
			if (pEntry->OneSecRxSGICount==0)
				pEntry->HTPhyMode.field.ShortGI = GI_800;

			if (pEntry->OneSecRxLGICount==0)
			{
				if ((pEntry->HTPhyMode.field.BW==BW_20 && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
			    	(pEntry->HTPhyMode.field.BW==BW_40 && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)))
						pEntry->HTPhyMode.field.ShortGI = GI_400;
			}
		}

		/*  Disable STBC if BF is enabled */
		if (pEntry->HTPhyMode.field.eTxBF || pEntry->HTPhyMode.field.iTxBF)
			pEntry->HTPhyMode.field.STBC = STBC_NONE;
	}
#endif /*  TXBF_SUPPORT */

	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);

#ifdef STREAM_MODE_SUPPORT
	/*  Enable/disable stream mode based on MCS */
	if (pAd->CommonCfg.StreamMode!=0 &&
		pEntry->StreamModeMACReg!=0)
	{
		UINT streamWord;
		BOOLEAN mcsDisable;

		/*  OFDM: depends on StreamModeMCS, CCK: always applies stream-mode */
		mcsDisable = (pEntry->HTPhyMode.field.MCS < 16) &&
				(pAd->CommonCfg.StreamModeMCS & (1<<pEntry->HTPhyMode.field.MCS))==0 &&
				(pEntry->HTPhyMode.field.MODE != MODE_CCK);

		streamWord = mcsDisable? 0: StreamModeRegVal(pAd);

		/*  Update Stream Mode control reg */
		RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, streamWord | (ULONG)(pEntry->Addr[4]) | (ULONG)(pEntry->Addr[5] << 8));
	}
#endif /*  STREAM_MODE_SUPPORT */
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
			
			DBGPRINT(RT_DEBUG_TRACE,("RTMPSetSupportMCS - wrong SUPP RATES., Len=%d\n", SupRateLen));
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
		PRT_HT_PHY_INFO	pDesired_ht_phy = NULL;
		UCHAR j, bitmask;
		CHAR i;


#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
		{
#ifdef WDS_SUPPORT
			if (IS_ENTRY_WDS(pEntry))
				pDesired_ht_phy = &pAd->WdsTab.WdsEntry[pEntry->apidx].DesiredHtPhyInfo;
			else
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pEntry))
				pDesired_ht_phy = &pAd->ApCfg.ApCliTab[pEntry->apidx].DesiredHtPhyInfo;
			else
#endif /* APCLI_SUPPORT */
				pDesired_ht_phy = &pAd->ApCfg.MBSSID[pEntry->apidx].DesiredHtPhyInfo;
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

INT	Set_RateAlg_Proc(
	IN	PRTMP_ADAPTER			pAd,
	IN	PSTRING					arg)
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

	DBGPRINT(RT_DEBUG_ERROR, ("%s: Set Alg = %d\n", __FUNCTION__, ra_alg));
	return TRUE;
}


