/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	ap_cfg.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/


#include "rt_config.h"

#define A_BAND_REGION_0				0
#define A_BAND_REGION_1				1
#define A_BAND_REGION_2				2
#define A_BAND_REGION_3				3
#define A_BAND_REGION_4				4
#define A_BAND_REGION_5				5
#define A_BAND_REGION_6				6
#define A_BAND_REGION_7				7
#define A_BAND_REGION_8				8
#define A_BAND_REGION_9				9
#define A_BAND_REGION_10			10

#define G_BAND_REGION_0				0
#define G_BAND_REGION_1				1
#define G_BAND_REGION_2				2
#define G_BAND_REGION_3				3
#define G_BAND_REGION_4				4
#define G_BAND_REGION_5				5
#define G_BAND_REGION_6				6

COUNTRY_CODE_TO_COUNTRY_REGION allCountry[] = {
	/* {Country Number, ISO Name, Country Name, Support 11A, 11A Country Region, Support 11G, 11G Country Region} */
	{0,		"DB",	"Debug",				TRUE,	A_BAND_REGION_7,	TRUE,	G_BAND_REGION_5},
	{8,		"AL",	"ALBANIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{12,	"DZ",	"ALGERIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{32,	"AR",	"ARGENTINA",			TRUE,	A_BAND_REGION_3,	TRUE,	G_BAND_REGION_1},
	{51,	"AM",	"ARMENIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{36,	"AU",	"AUSTRALIA",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{40,	"AT",	"AUSTRIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{31,	"AZ",	"AZERBAIJAN",			TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{48,	"BH",	"BAHRAIN",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{112,	"BY",	"BELARUS",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{56,	"BE",	"BELGIUM",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{84,	"BZ",	"BELIZE",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{68,	"BO",	"BOLIVIA",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{76,	"BR",	"BRAZIL",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{96,	"BN",	"BRUNEI DARUSSALAM",	TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{100,	"BG",	"BULGARIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{124,	"CA",	"CANADA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{152,	"CL",	"CHILE",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{156,	"CN",	"CHINA",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{170,	"CO",	"COLOMBIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{188,	"CR",	"COSTA RICA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{191,	"HR",	"CROATIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{196,	"CY",	"CYPRUS",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{203,	"CZ",	"CZECH REPUBLIC",		TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{208,	"DK",	"DENMARK",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{214,	"DO",	"DOMINICAN REPUBLIC",	TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{218,	"EC",	"ECUADOR",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{818,	"EG",	"EGYPT",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{222,	"SV",	"EL SALVADOR",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{233,	"EE",	"ESTONIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{246,	"FI",	"FINLAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{250,	"FR",	"FRANCE",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{268,	"GE",	"GEORGIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{276,	"DE",	"GERMANY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{300,	"GR",	"GREECE",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{320,	"GT",	"GUATEMALA",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{340,	"HN",	"HONDURAS",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{344,	"HK",	"HONG KONG",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{348,	"HU",	"HUNGARY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{352,	"IS",	"ICELAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{356,	"IN",	"INDIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{360,	"ID",	"INDONESIA",			TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{364,	"IR",	"IRAN",					TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{372,	"IE",	"IRELAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{376,	"IL",	"ISRAEL",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{380,	"IT",	"ITALY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{392,	"JP",	"JAPAN",				TRUE,	A_BAND_REGION_9,	TRUE,	G_BAND_REGION_1},
	{400,	"JO",	"JORDAN",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{398,	"KZ",	"KAZAKHSTAN",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{408,	"KP",	"KOREA DEMOCRATIC PEOPLE'S REPUBLIC OF",TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{410,	"KR",	"KOREA REPUBLIC OF",	TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{414,	"KW",	"KUWAIT",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{428,	"LV",	"LATVIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{422,	"LB",	"LEBANON",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{438,	"LI",	"LIECHTENSTEIN",		TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{440,	"LT",	"LITHUANIA",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{442,	"LU",	"LUXEMBOURG",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{446,	"MO",	"MACAU",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{807,	"MK",	"MACEDONIA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{458,	"MY",	"MALAYSIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{484,	"MX",	"MEXICO",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{492,	"MC",	"MONACO",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{504,	"MA",	"MOROCCO",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{528,	"NL",	"NETHERLANDS",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{554,	"NZ",	"NEW ZEALAND",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{578,	"NO",	"NORWAY",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{512,	"OM",	"OMAN",					TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{586,	"PK",	"PAKISTAN",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{591,	"PA",	"PANAMA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{604,	"PE",	"PERU",					TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{608,	"PH",	"PHILIPPINES",			TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{616,	"PL",	"POLAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{620,	"PT",	"PORTUGAL",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{630,	"PR",	"PUERTO RICO",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{634,	"QA",	"QATAR",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{642,	"RO",	"ROMANIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{643,	"RU",	"RUSSIA FEDERATION",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{682,	"SA",	"SAUDI ARABIA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{702,	"SG",	"SINGAPORE",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{703,	"SK",	"SLOVAKIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{705,	"SI",	"SLOVENIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{710,	"ZA",	"SOUTH AFRICA",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{724,	"ES",	"SPAIN",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{752,	"SE",	"SWEDEN",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{756,	"CH",	"SWITZERLAND",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{760,	"SY",	"SYRIAN ARAB REPUBLIC",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{158,	"TW",	"TAIWAN",				TRUE,	A_BAND_REGION_3,	TRUE,	G_BAND_REGION_0},
	{764,	"TH",	"THAILAND",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{780,	"TT",	"TRINIDAD AND TOBAGO",	TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{788,	"TN",	"TUNISIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{792,	"TR",	"TURKEY",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{804,	"UA",	"UKRAINE",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{784,	"AE",	"UNITED ARAB EMIRATES",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{826,	"GB",	"UNITED KINGDOM",		TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{840,	"US",	"UNITED STATES",		TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{858,	"UY",	"URUGUAY",				TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{860,	"UZ",	"UZBEKISTAN",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_0},
	{862,	"VE",	"VENEZUELA",			TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{704,	"VN",	"VIET NAM",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{887,	"YE",	"YEMEN",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{716,	"ZW",	"ZIMBABWE",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{999,	"",	"",	0,	0,	0,	0}
};

#define NUM_OF_COUNTRIES	(sizeof(allCountry)/sizeof(COUNTRY_CODE_TO_COUNTRY_REGION))

INT Set_CountryString_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_CountryCode_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ChGeography_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // EXT_BUILD_CHANNEL_LIST //

INT Set_AP_SSID_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_TxRate_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);


#if defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
INT Set_RfReg_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT	Set_RadarShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarHit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef DFS_DEBUG
INT	Set_R65_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_R66_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set__R65_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set__R66_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DfsT_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DfsLowerLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DfsUpperLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_FixDfsLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AvgRssiReq_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DfsC_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarGPIO_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarLog_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#endif // DFS_DEBUG //

#ifdef DFS_HARDWARE_SUPPORT
INT	Set_RadarStart_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarSetTbl1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarSetTbl2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Fcc5Thrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ChBusyThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RssiThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_PollTime_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WidthShift_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef DFS_DEBUG
INT	Set_CEPrintDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // DFS_DEBUG //

#ifdef DFS_SUPPORT
INT	Set_PrintBusyIdle_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarSim_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_BusyIdleRatio_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DfsRssiHigh_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_DfsRssiLow_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_Ch0EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_Ch1EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_Ch2EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_Ch3EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg); 
#endif // DFS_SUPPORT //

INT	Set_CEPrint_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch0LErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_MaxPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_PeriodErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch0HErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch1Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch2Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DeclareThres_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CheckLoop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CEStagCheck_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_HWDFSDisabled_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CeSwCheck_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif // DFS_HARDWARE_SUPPORT //
#endif // defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //

#ifdef RT305x
INT Set_RfRead_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_RfWrite_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);
#endif // RT305x //

#ifdef TONE_RADAR_DETECT_SUPPORT
INT	Set_CarrierDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CarrierDelta_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CarrierDivFlag_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CarrierThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // TONE_RADAR_DETECT_SUPPORT //


INT	Set_OLBCDetection_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AP_MaxStaNum_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AP_IdleTimeout_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef IAPP_SUPPORT
INT	Set_IappPID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // IAPP_SUPPORT //

INT Set_AP_AuthMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_EncrypType_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_WpaMixPairCipher_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_RekeyInterval_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_RekeyMethod_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_PMKCachePeriod_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_DefaultKeyID_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_Key1_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_Key2_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_Key3_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_Key4_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_AP_WPAPSK_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT Set_BasicRate_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_BeaconPeriod_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_DtimPeriod_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_NoForwarding_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_NoForwardingBTNSSID_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_AP_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_HideSSID_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_VLANID_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_VLANPriority_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_VLAN_TAG_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_AccessPolicy_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);


INT	Set_ACLAddEntry_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_ACLDelEntry_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_ACLShowAll_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_ACLClearAll_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT Set_SiteSurvey_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_AutoChannelSel_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_BADecline_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Show_MacTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
	
INT	Show_StaSecurityInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
	
INT	Show_DriverInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef DFS_SUPPORT
INT	Show_BlockCh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // DFS_SUPPORT //

#ifdef DOT11_N_SUPPORT
INT	Show_BaTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // DOT11_N_SUPPORT //

INT	Show_Sat_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Show_RAInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
INT Set_DiagOpt_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_BDInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Show_Diag_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // DBG_DAIGNOSE //
#endif // RTMP_MAC_PCI //

INT	Show_Sat_Reset_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Show_MATTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef DOT1X_SUPPORT
VOID RTMPIoctlQueryRadiusConf(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq);

INT	Set_IEEE8021X_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_PreAuth_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg);

INT	Set_RADIUS_Server_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RADIUS_Port_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RADIUS_Key_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // DOT1X_SUPPORT //

INT	Set_DisConnectSta_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DisConnectAllSta_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);


#ifdef APCLI_SUPPORT
INT Set_ApCli_Enable_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_Ssid_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_Bssid_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_DefaultKeyID_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_WPAPSK_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_Key1_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_Key2_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_Key3_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_Key4_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_ApCli_TxMode_Proc(IN PRTMP_ADAPTER pAd, IN  PSTRING arg);
INT Set_ApCli_TxMcs_Proc(IN PRTMP_ADAPTER pAd, IN  PSTRING arg);

#ifdef WSC_AP_SUPPORT
INT Set_AP_WscSsid_Proc(IN PRTMP_ADAPTER	pAd, IN	PSTRING arg);
#endif // WSC_AP_SUPPORT //
#endif // APCLI_SUPPORT //
#ifdef UAPSD_AP_SUPPORT
INT Set_UAPSD_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // UAPSD_AP_SUPPORT //

#ifdef WSC_AP_SUPPORT
INT	Set_WscStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_WscStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlWscPINCode(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlWscStatus(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlGetWscDynInfo(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

VOID RTMPIoctlGetWscRegsDynInfo(
	IN PRTMP_ADAPTER pAdapter, 
	IN struct iwreq *wrq);

BOOLEAN WscCheckEnrolleeNonceFromUpnp(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			pData,
	IN  USHORT			Length,
	IN  PWSC_CTRL       pWscControl);

UCHAR	WscRxMsgTypeFromUpnp(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PSTRING				pData,
	IN	USHORT				Length);

INT	    WscGetConfForUpnp(
	IN	PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL       pWscControl);

INT	Set_AP_WscConfMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AP_WscConfStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AP_WscMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AP_WscGetConf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AP_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_AP_WscSecurityMode_Proc(    
	IN  PRTMP_ADAPTER   pAd,     
	IN  PSTRING          arg);
#endif // WSC_AP_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
#ifdef MCAST_RATE_SPECIFIC
INT Set_McastPhyMode(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_McastMcs(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Show_McastRate(IN PRTMP_ADAPTER	pAd, IN PSTRING arg);
#endif // MCAST_RATE_SPECIFIC //

#ifdef DOT11N_DRAFT3
INT Set_OBSSScanParam_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

INT Set_AP2040ReScan_Proc(
	IN PRTMP_ADAPTER   pAd,
	IN PSTRING arg);

#ifdef DOT11N_PF_DEBUG
INT Set_HT_BssCoexNotify_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam);

INT Set_HT_MaxBa_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING	pParam);

INT Set_HT_McsSet_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam);

INT Set_HT_BssBWFallback_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam);
#endif // DOT11N_PF_DEBUG //
#endif // DOT11N_DRAFT3 //

INT Set_EntryLifeCheck_Proc(
	IN PRTMP_ADAPTER 	pAd,
	IN PSTRING			arg);

#ifdef RTMP_RBUS_SUPPORT
#ifdef WLAN_LED
INT Set_WlanLed_Proc(
	IN PRTMP_ADAPTER 	pAd,
	IN PSTRING			arg);
#endif
#endif // RTMP_RBUS_SUPPORT //


#ifdef AP_QLOAD_SUPPORT
INT	Set_QloadClr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

/* QLOAD ALARM */
INT	Set_QloadAlarmTimeThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			Arg);

INT	Set_QloadAlarmNumThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			Arg);
#endif // AP_QLOAD_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#ifdef RT3883
INT Set_CwCTest_Proc(
		IN PRTMP_ADAPTER pAd, 
		IN PSTRING   arg);
#endif // RT3883 //

INT	Set_MemDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

static struct {
	PSTRING name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, PSTRING arg);
} *PRTMP_PRIVATE_SET_PROC, RTMP_PRIVATE_SUPPORT_PROC[] = {
	{"DriverVersion",				Set_DriverVersion_Proc},
	{"CountryRegion",				Set_CountryRegion_Proc},
	{"CountryRegionABand",			Set_CountryRegionABand_Proc},
	{"CountryString",				Set_CountryString_Proc},
	{"CountryCode",				Set_CountryCode_Proc},
#ifdef EXT_BUILD_CHANNEL_LIST
	{"ChGeography",				Set_ChGeography_Proc},
#endif // EXT_BUILD_CHANNEL_LIST //
	{"SSID",						Set_AP_SSID_Proc},
	{"WirelessMode",				Set_WirelessMode_Proc},
	{"BasicRate",					Set_BasicRate_Proc},
	{"ShortSlot",					Set_ShortSlot_Proc},
	{"Channel",					Set_Channel_Proc},
	{"BeaconPeriod",				Set_BeaconPeriod_Proc},
	{"DtimPeriod",					Set_DtimPeriod_Proc},
	{"TxPower",					Set_TxPower_Proc},
	{"BGProtection",				Set_BGProtection_Proc},
	{"DisableOLBC", 				Set_OLBCDetection_Proc},
	{"TxPreamble",				Set_TxPreamble_Proc},
	{"RTSThreshold",				Set_RTSThreshold_Proc},
	{"FragThreshold",				Set_FragThreshold_Proc},
	{"TxBurst",					Set_TxBurst_Proc},
	{"MaxStaNum",					Set_AP_MaxStaNum_Proc},
#ifdef RTMP_MAC_PCI
	{"ShowRF",					Set_ShowRF_Proc},
#endif // RTMP_MAC_PCI //
	{"IdleTimeout",					Set_AP_IdleTimeout_Proc},	
#ifdef DOT11_N_SUPPORT
	{"BASetup",					Set_BASetup_Proc},
	{"BADecline",					Set_BADecline_Proc},
	{"SendMIMOPS",				Set_SendPSMPAction_Proc},
	{"BAOriTearDown",				Set_BAOriTearDown_Proc},
	{"BARecTearDown",				Set_BARecTearDown_Proc},
	{"HtBw",						Set_HtBw_Proc},
	{"HtMcs",						Set_HtMcs_Proc},
	{"HtGi",						Set_HtGi_Proc},
	{"HtOpMode",					Set_HtOpMode_Proc},
	{"HtStbc",					Set_HtStbc_Proc},
	{"HtHtc",						Set_HtHtc_Proc},
	{"HtExtcha",					Set_HtExtcha_Proc},
	{"HtMpduDensity",				Set_HtMpduDensity_Proc},
	{"HtBaWinSize",				Set_HtBaWinSize_Proc},
	{"HtMIMOPS",					Set_HtMIMOPSmode_Proc},
	{"HtRdg",						Set_HtRdg_Proc},
	{"HtLinkAdapt",				Set_HtLinkAdapt_Proc},
	{"HtAmsdu",					Set_HtAmsdu_Proc},
	{"HtAutoBa",					Set_HtAutoBa_Proc},
	{"HtProtect",					Set_HtProtect_Proc},
	{"HtMimoPs",					Set_HtMimoPs_Proc},
	{"HtTxStream",				Set_HtTxStream_Proc},
	{"HtRxStream",				Set_HtRxStream_Proc},
	{"ForceShortGI",				Set_ForceShortGI_Proc},
	{"ForceGF",		        		Set_ForceGF_Proc},
	{"HtTxBASize",					Set_HtTxBASize_Proc},
	{"BurstMode",					Set_BurstMode_Proc},
#ifdef GREENAP_SUPPORT
	{"GreenAP",					Set_GreenAP_Proc},
#endif // GREENAP_SUPPORT //
	{"HtDisallowTKIP",				Set_HtDisallowTKIP_Proc},	
#endif // DOT11_N_SUPPORT //

#ifdef IAPP_SUPPORT
	{"IappPID",					Set_IappPID_Proc},
#endif // IAPP_SUPPORT //

#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",				Set_PktAggregate_Proc},
#endif // AGGREGATION_SUPPORT //

#ifdef INF_PPA_SUPPORT
	{"INF_AMAZON_SE_PPA",			Set_INF_AMAZON_SE_PPA_Proc},
#endif // INF_PPA_SUPPORT //

#ifdef WMM_SUPPORT
	{"WmmCapable",				Set_AP_WmmCapable_Proc},
#endif // WMM_SUPPORT //
	{"NoForwarding",				Set_NoForwarding_Proc},
	{"NoForwardingBTNBSSID",		Set_NoForwardingBTNSSID_Proc},
	{"HideSSID",					Set_HideSSID_Proc},
	{"IEEE80211H",				Set_IEEE80211H_Proc},
	{"CSPeriod",					Set_CSPeriod_Proc},
	{"VLANID",					Set_VLANID_Proc},
	{"VLANPriority",				Set_VLANPriority_Proc},
	{"VLANTag",					Set_VLAN_TAG_Proc},
	{"AuthMode",					Set_AP_AuthMode_Proc},
	{"EncrypType",				Set_AP_EncrypType_Proc},
	{"WpaMixPairCipher", 			Set_AP_WpaMixPairCipher_Proc},
	{"RekeyInterval",				Set_AP_RekeyInterval_Proc},
	{"RekeyMethod", 				Set_AP_RekeyMethod_Proc}, 
	{"DefaultKeyID",				Set_AP_DefaultKeyID_Proc},
	{"Key1",						Set_AP_Key1_Proc},
	{"Key2",						Set_AP_Key2_Proc},
	{"Key3",						Set_AP_Key3_Proc},
	{"Key4",						Set_AP_Key4_Proc},
	{"AccessPolicy",				Set_AccessPolicy_Proc},
	{"ACLAddEntry",					Set_ACLAddEntry_Proc},
	{"ACLDelEntry",					Set_ACLDelEntry_Proc},
	{"ACLShowAll",					Set_ACLShowAll_Proc},
	{"ACLClearAll",					Set_ACLClearAll_Proc},
	{"WPAPSK",					Set_AP_WPAPSK_Proc},
	{"RadioOn",					Set_RadioOn_Proc},
#ifdef AP_SCAN_SUPPORT
	{"SiteSurvey",					Set_SiteSurvey_Proc},
	{"AutoChannelSel",				Set_AutoChannelSel_Proc},
#endif // AP_SCAN_SUPPORT //
	{"ResetCounter",				Set_ResetStatCounter_Proc},
	{"DisConnectSta",				Set_DisConnectSta_Proc},
	{"DisConnectAllSta",			Set_DisConnectAllSta_Proc},
#ifdef DOT1X_SUPPORT
	{"IEEE8021X",					Set_IEEE8021X_Proc},
	{"PreAuth",						Set_PreAuth_Proc},
	{"PMKCachePeriod", 				Set_AP_PMKCachePeriod_Proc},	
	{"own_ip_addr",					Set_OwnIPAddr_Proc},
	{"EAPifname",					Set_EAPIfName_Proc},
	{"PreAuthifname",				Set_PreAuthIfName_Proc},
	{"RADIUS_Server",				Set_RADIUS_Server_Proc},
	{"RADIUS_Port",					Set_RADIUS_Port_Proc},
	{"RADIUS_Key",					Set_RADIUS_Key_Proc},
#endif // DOT1X_SUPPORT //	
#ifdef DBG	
	{"Debug",					Set_Debug_Proc},
#endif // DBG //

#if defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
	{"RadarShow",					Set_RadarShow_Proc},
#endif // defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) //

#ifdef DFS_SUPPORT
	{"RadarDebug",					Set_RadarDebug_Proc},
	{"RadarHit",					Set_RadarHit_Proc},
#ifdef DFS_DEBUG
	{"R65",							Set_R65_Proc},
	{"R66",							Set_R66_Proc},
	{"_R65",						Set__R65_Proc},
	{"_R66",						Set__R66_Proc},
	{"DfsDetect",					Set_DfsT_Proc},
	{"DfsPeriod",					Set_DfsC_Proc},
	{"DfsLowerLimit",				Set_DfsLowerLimit_Proc},
	{"DfsUpperLimit",				Set_DfsUpperLimit_Proc},
	{"FixDfsLimit",					Set_FixDfsLimit_Proc},
	{"AvgRssiReq",					Set_AvgRssiReq_Proc},
	{"RadarGPIO",					Set_RadarGPIO_Proc},
	{"RadarLog",					Set_RadarLog_Proc},
#endif // DFS_DEBUG //

#ifdef DFS_HARDWARE_SUPPORT
	{"RadarStart",					Set_RadarStart_Proc},
	{"RadarStop",					Set_RadarStop_Proc},
	{"RadarT1",						Set_RadarSetTbl1_Proc},
	{"RadarT2",						Set_RadarSetTbl2_Proc},
	{"Fcc5Thrd",					Set_Fcc5Thrd_Proc},
	{"ChBusyThrd",					Set_ChBusyThrd_Proc},
	{"RssiThrd",					Set_RssiThrd_Proc},
	{"PollTime",					Set_PollTime_Proc},
	{"WidthShift",					Set_WidthShift_Proc},
	{"CEPrint",						Set_CEPrint_Proc},

#ifdef DFS_SUPPORT
	{"RadarSim",					Set_RadarSim_Proc},
	{"PrintBusyIdle",				Set_PrintBusyIdle_Proc},
	{"BusyIdleRatio",				Set_BusyIdleRatio_Proc},
	{"DfsRssiHigh",   				Set_DfsRssiHigh_Proc},
	{"DfsRssiLow",					Set_DfsRssiLow_Proc},
	{"Ch0EventExpire",				Set_Ch0EventExpire_Proc},
	{"Ch1EventExpire",				Set_Ch1EventExpire_Proc},
	{"Ch2EventExpire",				Set_Ch2EventExpire_Proc},
	{"Ch3EventExpire",				Set_Ch3EventExpire_Proc},
#endif // DFS_SUPPORT //

#ifdef DFS_DEBUG
	{"CEPrintDebug",				Set_CEPrintDebug_Proc},
#endif // DFS_DEBUG //
	{"Ch0LErr",						Set_Ch0LErr_Proc},
	{"PeriodErr",					Set_PeriodErr_Proc},
	{"MaxPeriod",					Set_MaxPeriod_Proc},
	{"Ch0HErr",						Set_Ch0HErr_Proc},
	{"Ch1Shift",					Set_Ch1Shift_Proc},
	{"Ch2Shift",					Set_Ch2Shift_Proc},
	
	{"CheckLoop",					Set_CheckLoop_Proc},
	{"DeclareThres",				Set_DeclareThres_Proc},
	{"CEStagCheck",					Set_CEStagCheck_Proc},
	{"HWDFSDisabled",				Set_HWDFSDisabled_Proc},
	{"CeSwCheck",					Set_CeSwCheck_Proc},
#endif // DFS_HARDWARE_SUPPORT //
#endif // DFS_SUPPORT //

	{"RfReg",							Set_RfReg_Proc},
#endif // defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //

#ifdef TONE_RADAR_DETECT_SUPPORT
	{"CarrierDebug",				Set_CarrierDebug_Proc},
	{"Delta",						Set_CarrierDelta_Proc},
	{"DivFlag",						Set_CarrierDivFlag_Proc},
	{"CarrThrd",					Set_CarrierThrd_Proc},
#endif // TONE_RADAR_DETECT_SUPPORT //	

#ifdef RT305x
	{"RfRead",							Set_RfRead_Proc},
	{"RfWrite",							Set_RfWrite_Proc},
	{"HiPower",							Set_HiPower_Proc},
#endif // RT305x //


#ifdef RALINK_ATE
	{"ATE",						Set_ATE_Proc},
	{"ATEDA",					Set_ATE_DA_Proc},
	{"ATESA",						Set_ATE_SA_Proc},
	{"ATEBSSID",					Set_ATE_BSSID_Proc},
	{"ATECHANNEL",				Set_ATE_CHANNEL_Proc},
	{"ATEINITCHAN",				Set_ATE_INIT_CHAN_Proc},
	{"ATETXPOW0",				Set_ATE_TX_POWER0_Proc},
	{"ATETXPOW1",				Set_ATE_TX_POWER1_Proc},
#ifdef DOT11N_SS3_SUPPORT
	{"ATETXPOW2",				Set_ATE_TX_POWER2_Proc},
#endif // DOT11N_SS3_SUPPORT //
	{"ATETXANT",					Set_ATE_TX_Antenna_Proc},
	{"ATERXANT",					Set_ATE_RX_Antenna_Proc},
#ifdef RT3350
	{"ATEPABIAS",				Set_ATE_PA_Bias_Proc},
#endif // RT3350 //
	{"ATETXFREQOFFSET",			Set_ATE_TX_FREQOFFSET_Proc},
	{"ATETXBW",					Set_ATE_TX_BW_Proc},
	{"ATETXLEN",					Set_ATE_TX_LENGTH_Proc},
	{"ATETXCNT",					Set_ATE_TX_COUNT_Proc},
	{"ATETXMCS",					Set_ATE_TX_MCS_Proc},
	{"ATETXMODE",				Set_ATE_TX_MODE_Proc},
	{"ATETXGI",					Set_ATE_TX_GI_Proc},
	{"ATERXFER",					Set_ATE_RX_FER_Proc},
	{"ATERRF",					Set_ATE_Read_RF_Proc},
#ifndef RTMP_RF_RW_SUPPORT
	{"ATEWRF1",						Set_ATE_Write_RF1_Proc},
	{"ATEWRF2",						Set_ATE_Write_RF2_Proc},
	{"ATEWRF3",						Set_ATE_Write_RF3_Proc},
	{"ATEWRF4",						Set_ATE_Write_RF4_Proc},
#endif // RTMP_RF_RW_SUPPORT //
	{"ATELDE2P",					Set_ATE_Load_E2P_Proc},
	{"ATERE2P",						Set_ATE_Read_E2P_Proc},
	{"ATEIPG",						Set_ATE_IPG_Proc},
	{"ATEPAYLOAD",					Set_ATE_Payload_Proc},
	{"ATEAUTOALC",					Set_ATE_AUTO_ALC_Proc},
	{"ATEIPG",						Set_ATE_IPG_Proc},
	{"ATEPAYLOAD",					Set_ATE_Payload_Proc},
#ifdef TXBF_SUPPORT
	{"ATETXBF",						Set_ATE_TXBF_Proc},
	{"ATETXSOUNDING",				Set_ATE_TXSOUNDING_Proc},
	{"ATETXBFDIVCAL",				Set_ATE_TXBF_DIVCAL_Proc},
	{"ATETXBFLNACAL",				Set_ATE_TXBF_LNACAL_Proc},
	{"ATETxBfInit",					Set_ATE_TXBF_INIT_Proc},
	{"ATETxBfCal",					Set_ATE_TXBF_CAL_Proc},
	{"ATETxBfGolden",					Set_ATE_TXBF_GOLDEN_Proc},
	{"ATETxBfVerify",					Set_ATE_TXBF_VERIFY_Proc},
	{"ATETxBfVerifyN",					Set_ATE_TXBF_VERIFY_NoComp_Proc},
	{"ATEForceBBP",					Set_ATE_ForceBBP_Proc},
#endif // TXBF_SUPPORT //
	{"ATESHOW",					Set_ATE_Show_Proc},
	{"ATEHELP",					Set_ATE_Help_Proc},

#ifdef RALINK_28xx_QA
	{"TxStop",					Set_TxStop_Proc},
	{"RxStop",					Set_RxStop_Proc},
#ifdef DBG
	{"EERead",						Set_EERead_Proc},
	{"EEWrite",						Set_EEWrite_Proc},
	{"BBPRead",						Set_BBPRead_Proc},
	{"BBPWrite",					Set_BBPWrite_Proc},
#endif // DBG //
#endif // RALINK_28xx_QA //
#endif // RALINK_ATE //

#ifdef APCLI_SUPPORT
	{"ApCliEnable",				Set_ApCli_Enable_Proc},
	{"ApCliSsid",					Set_ApCli_Ssid_Proc},
	{"ApCliBssid",					Set_ApCli_Bssid_Proc},
	{"ApCliAuthMode",				Set_ApCli_AuthMode_Proc},
	{"ApCliEncrypType",				Set_ApCli_EncrypType_Proc},
	{"ApCliDefaultKeyID",			Set_ApCli_DefaultKeyID_Proc},	
	{"ApCliWPAPSK",				Set_ApCli_WPAPSK_Proc},
	{"ApCliKey1",					Set_ApCli_Key1_Proc},
	{"ApCliKey2",					Set_ApCli_Key2_Proc},
	{"ApCliKey3",					Set_ApCli_Key3_Proc},
	{"ApCliKey4",					Set_ApCli_Key4_Proc},
	{"ApCliTxMode",					Set_ApCli_TxMode_Proc},
	{"ApCliTxMcs",					Set_ApCli_TxMcs_Proc},	
#ifdef WSC_AP_SUPPORT	
	{"ApCliWscSsid",				Set_AP_WscSsid_Proc},
#endif // WSC_AP_SUPPORT //
#endif // APCLI_SUPPORT //
#ifdef WSC_AP_SUPPORT
	{"WscConfMode",				Set_AP_WscConfMode_Proc},
	{"WscConfStatus",				Set_AP_WscConfStatus_Proc},
	{"WscMode",					Set_AP_WscMode_Proc},
	{"WscStatus",					Set_WscStatus_Proc},
	{"WscGetConf",				Set_AP_WscGetConf_Proc},
	{"WscPinCode",				Set_AP_WscPinCode_Proc},
	{"WscStop",                     		Set_WscStop_Proc},
	{"WscGenPinCode",               		Set_WscGenPinCode_Proc},
	{"WscVendorPinCode",            Set_WscVendorPinCode_Proc},
	{"WscSecurityMode",				Set_AP_WscSecurityMode_Proc},
#endif // WSC_AP_SUPPORT //
#ifdef UAPSD_AP_SUPPORT
	{"UAPSDCapable",				Set_UAPSD_Proc},
#endif // UAPSD_AP_SUPPORT //
#ifdef IGMP_SNOOP_SUPPORT
	{"IgmpSnEnable",				Set_IgmpSn_Enable_Proc},
	{"IgmpAdd",					Set_IgmpSn_AddEntry_Proc},
	{"IgmpDel",					Set_IgmpSn_DelEntry_Proc},
#endif // IGMP_SNOOP_SUPPORT //
#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
	{"FastDfs",					Set_FastDfs_Proc},
	{"ChMovTime",					Set_ChMovingTime_Proc},
	{"LPRadarTh",					Set_LongPulseRadarTh_Proc},
#endif
#ifdef CARRIER_DETECTION_SUPPORT
	{"CarrierDetect",				Set_CarrierDetect_Proc},
#ifdef TONE_RADAR_DETECT_SUPPORT
	{"CarrierCriteria",				Set_CarrierCriteria_Proc},
	{"CarrierReCheck",				Set_CarrierReCheck_Proc},
/*
	{"CarrierStopCheck",			Set_CarrierStopCheck_Proc},
*/
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //
#ifdef MCAST_RATE_SPECIFIC
	{"McastPhyMode",				Set_McastPhyMode},
	{"McastMcs",					Set_McastMcs},
#endif // MCAST_RATE_SPECIFIC //
#endif // CONFIG_AP_SUPPORT //
	{"FixedTxMode",                 Set_FixedTxMode_Proc},
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	{"OpMode",					Set_OpMode_Proc},
#endif // CONFIG_APSTA_MIXED_SUPPORT //

#ifdef TXBF_SUPPORT
	{"TxBfTag",				        Set_TxBfTag_Proc},
	{"ReadITxBf",				    Set_ReadITxBf_Proc},
	{"WriteITxBf",				    Set_WriteITxBf_Proc},
	{"StatITxBf",				    Set_StatITxBf_Proc},
	{"ReadETxBf",				    Set_ReadETxBf_Proc},
	{"WriteETxBf",				    Set_WriteETxBf_Proc},
	{"StatETxBf",				    Set_StatETxBf_Proc},
	{"ITxBfTimeout",		        Set_ITxBfTimeout_Proc},
	{"ETxBfTimeout",		        Set_ETxBfTimeout_Proc},
	{"InvTxBfTag",				    Set_InvTxBfTag_Proc},
	{"ITxBfCal",				    Set_ITxBfCal_Proc},
	{"ITxBfDivCal",				    Set_ITxBfDivCal_Proc},
	{"ITxBfLnaCal",				    Set_ITxBfLnaCal_Proc},

	{"ITxBfEn",						Set_ITxBfEn_Proc},
	{"ETxBfEnCond",					Set_ETxBfEnCond_Proc},
	{"ETxBfCodebook",				Set_ETxBfCodebook_Proc},
	{"ETxBfCoefficient",			Set_ETxBfCoefficient_Proc},
	{"ETxBfGrouping",				Set_ETxBfGrouping_Proc},
	{"ETxBfNoncompress",			Set_ETxBfNoncompress_Proc},
	{"ETxBfIncapable",			Set_ETxBfIncapable_Proc},
	{"NoSndgCntThrd",				Set_NoSndgCntThrd_Proc},		
	{"NdpSndgStreams",				Set_NdpSndgStreams_Proc},		
	{"TriggerSounding",				Set_Trigger_Sounding_Proc},
#endif // TXBF_SUPPORT //

#ifdef NEW_RATE_ADAPT_SUPPORT
	{"PerThrdAdj",					Set_PerThrdAdj_Proc},
#ifdef RANGE_EXT_SUPPORT
	{"LowTrafficThrd",				Set_LowTrafficThrd_Proc},
	{"TrainUpRule",					Set_TrainUpRule_Proc},
	{"TrainUpRuleRSSI",				Set_TrainUpRuleRSSI_Proc},
	{"TrainUpLowThrd",				Set_TrainUpLowThrd_Proc},
	{"TrainUpHighThrd",				Set_TrainUpHighThrd_Proc},
#endif // RANGE_EXT_SUPPORT //
#endif // NEW_RATE_ADAPT_SUPPORT //

#if defined (RT2883) || defined (RT3883)
	{"PreAntSwitch",		        Set_PreAntSwitch_Proc},
#ifdef RANGE_EXT_SUPPORT
	{"PreAntSwitchRSSI",		    Set_PreAntSwitchRSSI_Proc},
	{"PreAntSwitchTimeout",		    Set_PreAntSwitchTimeout_Proc},
	{"RateTable",					Set_RateTable_Proc},
#endif // RANGE_EXT_SUPPORT //
	{"PhyRateLimit",				Set_PhyRateLimit_Proc},
	{"FixedRate",					Set_FixedRate_Proc},
#endif // defined (RT2883) || defined (RT3883) //

#ifdef RT3883
	{"CFOTrack",				Set_CFOTrack_Proc},
#endif // RT3883 //

#ifdef STREAM_MODE_SUPPORT
	{"StreamMode",					Set_StreamMode_Proc},
	{"StreamModeMac",				Set_StreamModeMac_Proc},
#ifdef RANGE_EXT_SUPPORT
	{"StreamModeMCS",			Set_StreamModeMCS_Proc},
#endif // RANGE_EXT_SUPPORT //
#endif // STREAM_MODE_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
	{"DebugFlags",					Set_DebugFlags_Proc},
#ifdef INCLUDE_DEBUG_QUEUE
	{"DebugQueue",					Set_DebugQueue_Proc},
#endif // INCLUDE_DEBUG_QUEUE //
#endif // RTMP_RBUS_SUPPORT //

	{"LongRetry",	        		Set_LongRetryLimit_Proc},
	{"ShortRetry",	        		Set_ShortRetryLimit_Proc},
	{"AutoFallBack",	        	Set_AutoFallBack_Proc},
#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
	{"DiagOpt",					Set_DiagOpt_Proc},
	{"BDInfo",					Set_BDInfo_Proc},
#endif // DBG_DIAGNOSE //
#endif // RTMP_MAC_PCI //

	{"MeasureReq",					Set_MeasureReq_Proc},
	{"TpcReq",						Set_TpcReq_Proc},
	{"PwrConstraint",				Set_PwrConstraint},
#ifdef DOT11N_DRAFT3
	{"OBSSScanParam",				Set_OBSSScanParam_Proc},
	{"AP2040Rescan",				Set_AP2040ReScan_Proc},
	{"HtBssCoex",						Set_HT_BssCoex_Proc},
	{"HtBssCoexApCntThr",				Set_HT_BssCoexApCntThr_Proc},
#ifdef DOT11N_PF_DEBUG
	{"HtBssFallback", 					Set_HT_BssBWFallback_Proc},
	{"HtMaxBa",						Set_HT_MaxBa_Proc},
	{"HtMcsSet",						Set_HT_McsSet_Proc},
	{"HtBssCoexNotify",				Set_HT_BssCoexNotify_Proc},
#endif // DOT11N_PF_DEBUG //

#endif // DOT11N_DRAFT3 //
	{"EntryLifeCheck",				Set_EntryLifeCheck_Proc},
#ifdef WMM_ACM_SUPPORT
	{"acm",							ACM_Ioctl},
#endif // WMM_ACM_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
#ifdef WLAN_LED
	{"WlanLed",					Set_WlanLed_Proc},
#endif // WLAN_LED //
#ifdef RT3883
	{"VCORecalibrationThreshold",		Set_VCORecalibrationThreshold_Proc},
	{"VCORecalibration",				Set_VCORecalibration_Proc},
#endif // RT3883 //
#endif // RTMP_RBUS_SUPPORT //

#ifdef AP_QLOAD_SUPPORT
	{"qloadclr",					Set_QloadClr_Proc},
	{"qloadalarmtimethres",			Set_QloadAlarmTimeThreshold_Proc}, /* QLOAD ALARM */
	{"qloadalarmnumthres",			Set_QloadAlarmNumThreshold_Proc}, /* QLOAD ALARM */
#endif // AP_QLOAD_SUPPORT //


	{"memdebug",					Set_MemDebug_Proc},

	{NULL,}
};


static struct {
	PSTRING name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, PSTRING arg);
} *PRTMP_PRIVATE_SHOW_PROC, RTMP_PRIVATE_SHOW_SUPPORT_PROC[] = {
	{"stainfo",			Show_MacTable_Proc},
	{"stasecinfo", 			Show_StaSecurityInfo_Proc},	
	{"descinfo",			Show_DescInfo_Proc},
	{"driverinfo", 			Show_DriverInfo_Proc},
#ifdef WDS_SUPPORT
	{"wdsinfo",				Show_WdsTable_Proc},
#endif // WDS_SUPPORT //
#ifdef DOT11_N_SUPPORT
	{"bainfo",				Show_BaTable_Proc},
#endif // DOT11_N_SUPPORT //
	{"stat",				Show_Sat_Proc}, 
#ifdef DBG_DIAGNOSE
	{"diag",				Show_Diag_Proc},
#endif // DBG_DIAGNOSE //
	{"stat_reset",			Show_Sat_Reset_Proc},
#ifdef IGMP_SNOOP_SUPPORT
	{"igmpinfo",			Set_IgmpSn_TabDisplay_Proc},
#endif // IGMP_SNOOP_SUPPORT //
#ifdef MCAST_RATE_SPECIFIC
	{"mcastrate",			Show_McastRate},
#endif // MCAST_RATE_SPECIFIC //
#ifdef MAT_SUPPORT
	{"matinfo",			Show_MATTable_Proc},
#endif // MAT_SUPPORT //
#ifdef DFS_SUPPORT
	{"blockch", 			Show_BlockCh_Proc},
#endif // DFS_SUPPORT //
#ifdef AP_QLOAD_SUPPORT
	{"qload",				Show_QoSLoad_Proc},
#endif // AP_QLOAD_SUPPORT //
#ifdef APCLI_SUPPORT
	{"connStatus",			RTMPIoctlConnStatus},
#endif
	{"rainfo",				Show_RAInfo_Proc},
	{NULL,}
};


INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER *pAd, 
	IN struct iwreq *pIoctlCmdStr)
{
	PSTRING this_char;
	PSTRING value;
	INT Status = NDIS_STATUS_SUCCESS;
	
	while ((this_char = strsep((char **)&pIoctlCmdStr->u.data.pointer, ",")) != NULL) 
	{
		if (!*this_char)
			 continue;

		if ((value = strchr(this_char, '=')) != NULL)
			*value++ = 0;

		if (!value 
#ifdef WSC_AP_SUPPORT                        
            && (
                 (strcmp(this_char, "WscStop") != 0) &&
                 (strcmp(this_char, "WscGenPinCode")!= 0)
               )
#endif // WSC_AP_SUPPORT //
            )
			continue;  							

		for (PRTMP_PRIVATE_SET_PROC = RTMP_PRIVATE_SUPPORT_PROC; PRTMP_PRIVATE_SET_PROC->name; PRTMP_PRIVATE_SET_PROC++)
		{
			if (!strcmp(this_char, PRTMP_PRIVATE_SET_PROC->name)) 
			{					
				if(!PRTMP_PRIVATE_SET_PROC->set_proc(pAd, value))
				{   //FALSE:Set private failed then return Invalid argument 								
					Status = -EINVAL;							
				}
				break;  //Exit for loop.
			}
		}

		if(PRTMP_PRIVATE_SET_PROC->name == NULL)
		{  //Not found argument
			Status = -EINVAL;
			DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::(iwpriv) Command not Support [%s=%s]\n", this_char, value));
			break;
		}	
	}

	return Status;
}


INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd, 
	IN struct iwreq *pIoctlCmdStr)
{
	PSTRING this_char;
	PSTRING value = NULL;
	INT Status = NDIS_STATUS_SUCCESS;
	
	while ((this_char = strsep((char **)&pIoctlCmdStr->u.data.pointer, ",")) != NULL) 
	{
		if (!*this_char)
			continue;

		for (PRTMP_PRIVATE_SHOW_PROC = RTMP_PRIVATE_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_SHOW_PROC->name; PRTMP_PRIVATE_SHOW_PROC++)
		{
			if (!strcmp(this_char, PRTMP_PRIVATE_SHOW_PROC->name)) 
			{						
				if(!PRTMP_PRIVATE_SHOW_PROC->set_proc(pAd, value))
				{   //FALSE:Set private failed then return Invalid argument 								
					Status = -EINVAL;							
				}
				break;  //Exit for loop.
			}
		}

		if(PRTMP_PRIVATE_SHOW_PROC->name == NULL)
		{  //Not found argument
			Status = -EINVAL;
#ifdef RTMP_RBUS_SUPPORT
			for (PRTMP_PRIVATE_SHOW_PROC = RTMP_PRIVATE_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_SHOW_PROC->name; PRTMP_PRIVATE_SHOW_PROC++)
				DBGPRINT(RT_DEBUG_TRACE, ("%s\n", PRTMP_PRIVATE_SHOW_PROC->name));
#endif // RTMP_RBUS_SUPPORT //
			DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::(iwpriv) Command not Support [%s=%s]\n", this_char, value));
			break;
		}	
	}

	return Status;
	
}

#ifdef INF_AR9
#ifdef AR9_MAPI_SUPPORT
INT RTMPAPPrivIoctlAR9Show(
	IN RTMP_ADAPTER *pAd, 
	IN struct iwreq *pIoctlCmdStr)
{
	INT Status = NDIS_STATUS_SUCCESS;

		if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_mac_table"))
		{
			RTMPAR9IoctlGetMacTable(pAd,pIoctlCmdStr);
		}
		else if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_stat2"))
		{
			RTMPIoctlGetSTAT2(pAd,pIoctlCmdStr);
		}		
		else if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_radio_dyn_info"))
		{
			RTMPIoctlGetRadioDynInfo(pAd,pIoctlCmdStr);
		}
#ifdef WSC_AP_SUPPORT
		else if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_profile"))
		{
			RTMPAR9IoctlWscProfile(pAd,pIoctlCmdStr);		
		}
		else if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_pincode"))
		{
			RTMPIoctlWscPINCode(pAd,pIoctlCmdStr);
		}
		else if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_status"))
		{
			RTMPIoctlWscStatus(pAd,pIoctlCmdStr);
		}
		else if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_wps_dyn_info"))
		{
			RTMPIoctlGetWscDynInfo(pAd,pIoctlCmdStr);
		}
		else if(!strcmp(pIoctlCmdStr->u.data.pointer, "get_wps_regs_dyn_info"))
		{
			RTMPIoctlGetWscRegsDynInfo(pAd,pIoctlCmdStr);
		}
#endif
	return Status;
}
#endif //AR9_MAPI_SUPPORT//
#endif//AR9_INF//

INT RTMPAPSetInformation(
	IN	PRTMP_ADAPTER pAd,
	IN	OUT	struct iwreq	*rq,
	IN	INT				cmd)
{
	struct iwreq						*wrq = (struct iwreq *) rq;
	UCHAR						        Addr[MAC_ADDR_LEN];
	INT									Status = NDIS_STATUS_SUCCESS;

#ifdef SNMP_SUPPORT	
	//snmp
    UINT						KeyIdx = 0;
    PNDIS_AP_802_11_KEY			pKey = NULL;
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						ctmp;
#endif // SNMP_SUPPORT //	

	
#ifdef HOSTAPD_SUPPORT
 	NDIS_802_11_WEP_STATUS              WepStatus;
 	NDIS_802_11_AUTHENTICATION_MODE     AuthMode = Ndis802_11AuthModeMax;
	NDIS_802_11_SSID                    Ssid;
	MAC_TABLE_ENTRY						*pEntry;
	struct ieee80211req_mlme			mlme;

	struct ieee80211req_key				Key;
	struct ieee80211req_del_key			delkey;
	ULONG								KeyIdx;
	PMULTISSID_STRUCT	pMbss ;
#ifdef WSC_AP_SUPPORT
	WSC_LV_INFO            WscIEBeacon;
   	WSC_LV_INFO            WscIEProbeResp;
#endif //WSC_AP_SUPPORT//
	int i;
#endif //HOSTAPD_SUPPORT//

	
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	switch(cmd & 0x7FFF)
    {
    	case OID_802_11_DEAUTHENTICATION:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DEAUTHENTICATION\n"));
			if (wrq->u.data.length != sizeof(MLME_DEAUTH_REQ_STRUCT))
				Status  = -EINVAL;
			else
			{                								
				MAC_TABLE_ENTRY 		*pEntry = NULL;
				MLME_DEAUTH_REQ_STRUCT  *pInfo;
				MLME_QUEUE_ELEM 		*Elem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);

				if (Elem)
				{
					pInfo = (MLME_DEAUTH_REQ_STRUCT *) Elem->Msg;
					Status = copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length);

					if ((pEntry = MacTableLookup(pAd, pInfo->Addr)) != NULL)
					{					
						Elem->Wcid = pEntry->Aid;
						MlmeEnqueue(pAd, AP_AUTH_STATE_MACHINE, APMT2_MLME_DEAUTH_REQ,
										sizeof(MLME_DEAUTH_REQ_STRUCT), Elem, 0);
						DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_DEAUTHENTICATION (Reason=%d)\n", pInfo->Reason));
					}
					kfree(Elem);
				}
				else
					Status = -EFAULT;
			}
			
			break;
#ifdef IAPP_SUPPORT
    	case RT_SET_IAPP_PID:
			{
				unsigned long IappPid;
				if (copy_from_user(&IappPid, wrq->u.data.pointer, wrq->u.data.length))
				{
					Status = -EFAULT; 	
				}
    			else
    			{
					RTMP_GET_OS_PID(pObj->IappPid, IappPid);
					pObj->IappPid_nr = IappPid;
					DBGPRINT(RT_DEBUG_TRACE, ("RT_SET_APD_PID::(IappPid=%lu(0x%lx))\n", IappPid, IappPid));
				}
    		}
			break;
#endif // IAPP_SUPPORT //

    	case RT_SET_APD_PID:
			{
				unsigned long apd_pid;
				if (copy_from_user(&apd_pid, wrq->u.data.pointer, wrq->u.data.length))
				{
					Status = -EFAULT; 	
				}
    			else
    			{
					RTMP_GET_OS_PID(pObj->apd_pid, apd_pid);
					pObj->apd_pid_nr = apd_pid;
					DBGPRINT(RT_DEBUG_TRACE, ("RT_SET_APD_PID::(ApdPid=%lu(0x%lx))\n", apd_pid, apd_pid));
				}
    		}
			break;
		case RT_SET_DEL_MAC_ENTRY:
    		if (copy_from_user(Addr, wrq->u.data.pointer, wrq->u.data.length))
			{
				Status = -EFAULT; 	
			}
    		else
    		{
				MAC_TABLE_ENTRY *pEntry = NULL;
			
				DBGPRINT(RT_DEBUG_TRACE, ("RT_SET_DEL_MAC_ENTRY::(%02x:%02x:%02x:%02x:%02x:%02x)\n", Addr[0],Addr[1],Addr[2],Addr[3],Addr[4],Addr[5]));

				pEntry = MacTableLookup(pAd, Addr);
			
				if (pEntry)
				{
					MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
//					MacTableDeleteEntry(pAd, pEntry->Aid, Addr);				
				}
    		}
			break;
#ifdef WSC_AP_SUPPORT
		case RT_OID_WSC_SET_SELECTED_REGISTRAR:
			{	
				PUCHAR      upnpInfo;
				UCHAR	    apidx = pObj->ioctl_if;
				
#ifdef HOSTAPD_SUPPORT
				if (pAd->ApCfg.Hostapd == TRUE)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
						Status = -EINVAL;
					}
				else
				{
#endif //HOSTAPD_SUPPORT//

				
				DBGPRINT(RT_DEBUG_TRACE, ("WSC::RT_OID_WSC_SET_SELECTED_REGISTRAR, wrq->u.data.length=%d!\n", wrq->u.data.length));
				upnpInfo = kmalloc(wrq->u.data.length, GFP_KERNEL);
				if(upnpInfo)
				{
					int len;
					
					len = copy_from_user(upnpInfo, wrq->u.data.pointer, wrq->u.data.length);
					len = wrq->u.data.length;
					
					if((pAd->ApCfg.MBSSID[apidx].WscControl.WscConfMode & WSC_PROXY))
					{
						WscSelectedRegistrar(pAd, upnpInfo, len, apidx);
						if (pAd->ApCfg.MBSSID[apidx].WscControl.Wsc2MinsTimerRunning == TRUE)
						{
							BOOLEAN Cancelled;
							RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].WscControl.Wsc2MinsTimer, &Cancelled);
						}
						// 2mins time-out timer
						RTMPSetTimer(&pAd->ApCfg.MBSSID[apidx].WscControl.Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
						pAd->ApCfg.MBSSID[apidx].WscControl.Wsc2MinsTimerRunning = TRUE;
					}
					kfree(upnpInfo);
				} 
				else 
				{
					Status = -EINVAL;
				}
#ifdef HOSTAPD_SUPPORT
					}
#endif //HOSTAPD_SUPPORT//

			}
			break;
		case RT_OID_WSC_EAPMSG:
			{
				RTMP_WSC_U2KMSG_HDR *msgHdr = NULL;
				PUCHAR pUPnPMsg = NULL;
				UINT msgLen = 0, Machine = 0, msgType = 0;
				int retVal, senderID = 0;

#ifdef HOSTAPD_SUPPORT
				if (pAd->ApCfg.Hostapd == TRUE)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
						Status = -EINVAL;
					}
				else
				{
#endif //HOSTAPD_SUPPORT//


				DBGPRINT(RT_DEBUG_TRACE, ("WSC::RT_OID_WSC_EAPMSG, wrq->u.data.length=%d, ioctl_if=%d\n", wrq->u.data.length, pObj->ioctl_if));
			
				msgLen = wrq->u.data.length;				
				if((pUPnPMsg = kmalloc(msgLen, GFP_KERNEL)) == NULL)
					Status = -EINVAL;
				else
				{
					int HeaderLen;
					PSTRING pWpsMsg;
					UINT WpsMsgLen;
					PWSC_CTRL pWscControl;

					memset(pUPnPMsg, 0, msgLen);
					retVal = copy_from_user(pUPnPMsg, wrq->u.data.pointer, msgLen);

					msgHdr = (RTMP_WSC_U2KMSG_HDR *)pUPnPMsg;
					senderID = get_unaligned((INT32 *)(&msgHdr->Addr2[0]));
					//senderID = *((int *)&msgHdr->Addr2);

					HeaderLen = LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME) + sizeof(EAP_FRAME);
					pWpsMsg = (PSTRING) &pUPnPMsg[HeaderLen];
					WpsMsgLen = msgLen - HeaderLen;

					//assign the STATE_MACHINE type
					Machine = WSC_STATE_MACHINE;
					msgType = WSC_EAPOL_UPNP_MSG;

					pWscControl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].WscControl;
					// If AP is unconfigured, WPS state machine will be triggered after received M2.
					if (pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
					{
						if (strstr(pWpsMsg, "SimpleConfig") &&
							!pWscControl->EapMsgRunning &&
							!pWscControl->WscUPnPNodeInfo.bUPnPInProgress)
						{
							// GetDeviceInfo
							WscInit(pAd, FALSE, pObj->ioctl_if);
							// trigger wsc re-generate public key
							pWscControl->RegData.ReComputePke = 1;
						}
						else if (WscRxMsgTypeFromUpnp(pAd, pWpsMsg, WpsMsgLen) == WSC_MSG_M2 &&
								!pWscControl->EapMsgRunning &&
								!pWscControl->WscUPnPNodeInfo.bUPnPInProgress)
						{
							// Check Enrollee Nonce of M2
							if (WscCheckEnrolleeNonceFromUpnp(pAd, pWpsMsg, WpsMsgLen, pWscControl))
							{
								WscGetConfWithoutTrigger(pAd, pWscControl, TRUE);
								pWscControl->WscState = WSC_STATE_SENT_M1;
							}
						}
					}

					retVal = MlmeEnqueueForWsc(pAd, msgHdr->envID, senderID, Machine, msgType, msgLen, pUPnPMsg);
					if((retVal == FALSE) && (msgHdr->envID != 0))
					{
						DBGPRINT(RT_DEBUG_TRACE, ("MlmeEnqueuForWsc return False and envID=0x%x!\n", msgHdr->envID));
						Status = -EINVAL;
					}

					kfree(pUPnPMsg);
				}
				DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_EAPMSG finished!\n"));
#ifdef HOSTAPD_SUPPORT
					}
#endif //HOSTAPD_SUPPORT//
			}
			break;

		case RT_OID_WSC_READ_UFD_FILE:
			if (wrq->u.data.length > 0)
			{
				STRING		*pWscUfdFileName = NULL;
				UCHAR 		apIdx = pObj->ioctl_if;
				PWSC_CTRL	pWscCtrl = &pAd->ApCfg.MBSSID[apIdx].WscControl;
				pWscUfdFileName = (PSTRING)kmalloc(wrq->u.data.length+1, MEM_ALLOC_FLAG);
				if (pWscUfdFileName)
				{
					RTMPZeroMemory(pWscUfdFileName, wrq->u.data.length+1);
					if (copy_from_user(pWscUfdFileName, wrq->u.data.pointer, wrq->u.data.length))
						Status = -EFAULT;
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_READ_UFD_FILE (WscUfdFileName=%s)\n", pWscUfdFileName));
						if (pWscCtrl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
						{
							if (WscReadProfileFromUfdFile(pAd, apIdx, pWscUfdFileName))
							{
								pWscCtrl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
								APStop(pAd);
								APStartUp(pAd);
							}
						}
						else
						{
							DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_READ_UFD_FILE: AP is configured.\n"));
							Status = -EINVAL;
						}
					}
					kfree(pWscUfdFileName);
				}
				else
					Status = -ENOMEM;
			}
			else
				Status = -EINVAL;
			break;
			
		case RT_OID_WSC_WRITE_UFD_FILE:
			if (wrq->u.data.length > 0)
			{
				STRING		*pWscUfdFileName = NULL;
				UCHAR 		apIdx = pObj->ioctl_if;
				PWSC_CTRL	pWscCtrl = &pAd->ApCfg.MBSSID[apIdx].WscControl;
				pWscUfdFileName = (PSTRING)kmalloc(wrq->u.data.length+1, MEM_ALLOC_FLAG);
				if (pWscUfdFileName)
				{
					RTMPZeroMemory(pWscUfdFileName, wrq->u.data.length+1);
					if (copy_from_user(pWscUfdFileName, wrq->u.data.pointer, wrq->u.data.length))
						Status = -EFAULT;
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_WRITE_UFD_FILE (WscUfdFileName=%s)\n", pWscUfdFileName));
						if (pWscCtrl->WscConfStatus == WSC_SCSTATE_CONFIGURED)
						{
							WscWriteProfileToUfdFile(pAd, apIdx, pWscUfdFileName);
						}
						else
						{
							DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_WSC_WRITE_UFD_FILE: AP is un-configured.\n"));
							Status = -EINVAL;
						}
					}
					kfree(pWscUfdFileName);
				}
				else
					Status = -ENOMEM;
			}
			else
				Status = -EINVAL;
			break;

		case RT_OID_WSC_UUID:
			if (wrq->u.data.length == (UUID_LEN_STR-1))
			{
				UCHAR 		apIdx = pObj->ioctl_if;
				pAd->ApCfg.MBSSID[apIdx].WscControl.Wsc_Uuid_Str[0] = '\0';
				Status = copy_from_user(&pAd->ApCfg.MBSSID[apIdx].WscControl.Wsc_Uuid_Str[0],
										wrq->u.data.pointer, 
										wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("UUID ASCII string: %s\n", 
										pAd->ApCfg.MBSSID[apIdx].WscControl.Wsc_Uuid_Str));
			}
			else if (wrq->u.data.length == UUID_LEN_HEX)
			{
				UCHAR 		apIdx = pObj->ioctl_if, ii;				
				Status = copy_from_user(&pAd->ApCfg.MBSSID[apIdx].WscControl.Wsc_Uuid_E[0],
										wrq->u.data.pointer, 
										wrq->u.data.length);

				for (ii=0; ii< 16; ii++)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%02x", (pAd->ApCfg.MBSSID[apIdx].WscControl.Wsc_Uuid_E[ii] & 0xff)));
				}
			}
			else
				Status = -EINVAL;
			break;
#endif // WSC_AP_SUPPORT //


#ifdef SNMP_SUPPORT
		case OID_802_11_SHORTRETRYLIMIT:
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&ShortRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
				RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
				tx_rty_cfg.field.ShortRtyLimit = ShortRetryLimit;
				RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_SHORTRETRYLIMIT (tx_rty_cfg.field.ShortRetryLimit=%d, ShortRetryLimit=%ld)\n", tx_rty_cfg.field.ShortRtyLimit, ShortRetryLimit));
			}
			break;

		case OID_802_11_LONGRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT \n"));
			if (wrq->u.data.length != sizeof(ULONG))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&LongRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
				RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
				tx_rty_cfg.field.LongRtyLimit = LongRetryLimit;
				RTMP_IO_WRITE32(pAd, TX_RTY_CFG, tx_rty_cfg.word);
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_LONGRETRYLIMIT (tx_rty_cfg.field.LongRetryLimit= %d,LongRetryLimit=%ld)\n", tx_rty_cfg.field.LongRtyLimit, LongRetryLimit));
			}
			break;

		case OID_802_11_WEPDEFAULTKEYVALUE:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE\n"));
			pKey = kmalloc(wrq->u.data.length, GFP_KERNEL);
			if (pKey == NULL)
			{
				Status= -EINVAL;
				break;
			}
			Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);
			//pKey = &WepKey;
			
			if ( pKey->Length != wrq->u.data.length)
			{
				Status = -EINVAL;
				DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n"));
			}
			KeyIdx = pKey->KeyIndex & 0x0fffffff;
			DBGPRINT(RT_DEBUG_TRACE,("pKey->KeyIndex =%d, pKey->KeyLength=%d\n", pKey->KeyIndex, pKey->KeyLength));

			// it is a shared key
			if (KeyIdx > 4)
				Status = -EINVAL;
			else
			{
				pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(&pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].Key, &pKey->KeyMaterial, pKey->KeyLength);
				if (pKey->KeyIndex & 0x80000000)
				{
					// Default key for tx (shared key)
					pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId = (UCHAR) KeyIdx;
				}
				//RestartAPIsRequired = TRUE;
			}
			break;


		case OID_802_11_WEPDEFAULTKEYID:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEPDEFAULTKEYID \n"));

			if (wrq->u.data.length != sizeof(UCHAR))
				Status = -EINVAL;
			else
				Status = copy_from_user(&pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId, wrq->u.data.pointer, wrq->u.data.length);

			break;


		case OID_802_11_CURRENTCHANNEL:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_CURRENTCHANNEL \n"));
			if (wrq->u.data.length != sizeof(UCHAR))
				Status = -EINVAL;
			else
			{
				Status = copy_from_user(&ctmp, wrq->u.data.pointer, wrq->u.data.length);
				sprintf((PSTRING)&ctmp,"%d", ctmp);
				Set_Channel_Proc(pAd, (PSTRING)&ctmp);
			}
			break;
#endif // SNMP_SUPPORT //


#ifdef WAPI_SUPPORT
		case OID_802_11_WAPI_PID:
			{
				unsigned long wapi_pid;
    			if (copy_from_user(&pObj->wapi_pid, wrq->u.data.pointer, wrq->u.data.length))
				{
					Status = -EFAULT; 	
				}
    			else
    			{
					RTMP_GET_OS_PID(pObj->wapi_pid, wapi_pid);
					pObj->wapi_pid_nr = wapi_pid;
					DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_WAPI_PID::(WapiPid=%lu(0x%x))\n", wapi_pid, pObj->wapi_pid));
				}
    		}
			break;

		case OID_802_11_PORT_SECURE_STATE:
			if (wrq->u.data.length != sizeof(WAPI_PORT_SECURE_STRUCT))
                Status  = -EINVAL;
            else
            {                								
				MAC_TABLE_ENTRY 		*pEntry = NULL;
				WAPI_PORT_SECURE_STRUCT  wapi_port;

				Status = copy_from_user(&wapi_port, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {
					if ((pEntry = MacTableLookup(pAd, wapi_port.Addr)) != NULL)
					{
						switch (wapi_port.state)
						{
							case WAPI_PORT_SECURED:
								pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
								pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
								break;
							
							default:
								pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
								pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
								break;								
						}											
					}
					DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_PORT_SECURE_STATE (state=%d)\n", wapi_port.state));
				}				
            }
			break;
			
		case OID_802_11_UCAST_KEY_INFO:
			if (wrq->u.data.length != sizeof(WAPI_UCAST_KEY_STRUCT))
                Status  = -EINVAL;
            else
            {                								
				MAC_TABLE_ENTRY 		*pEntry = NULL;
				WAPI_UCAST_KEY_STRUCT   wapi_ukey;

				Status = copy_from_user(&wapi_ukey, wrq->u.data.pointer, wrq->u.data.length);
                if (Status == NDIS_STATUS_SUCCESS)
                {
					if ((pEntry = MacTableLookup(pAd, wapi_ukey.Addr)) != NULL)
					{
						pEntry->usk_id = wapi_ukey.key_id;
						NdisMoveMemory(pEntry->PTK, wapi_ukey.PTK, 64);								

						/* Install pairwise key */
						WAPIInstallPairwiseKey(pAd, pEntry, TRUE);

						// Start or re-start USK rekey mechanism, if necessary.
						RTMPCancelWapiRekeyTimerAction(pAd, pEntry);
						RTMPStartWapiRekeyTimerAction(pAd, pEntry);
					}
					DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_UCAST_KEY_INFO complete\n"));
					hex_dump("WAPI UCAST KEY", pEntry->PTK, 64);
				}				
            }
			break;	
			
#endif // WAPI_SUPPORT //

#ifdef DOT1X_SUPPORT
		case OID_802_DOT1X_PMKID_CACHE:
			RTMPIoctlAddPMKIDCache(pAd, wrq);
			break;

		case OID_802_DOT1X_RADIUS_DATA:
			RTMPIoctlRadiusData(pAd, wrq);
			break;

		case OID_802_DOT1X_WPA_KEY:
			RTMPIoctlAddWPAKey(pAd, wrq);
			break;

		case OID_802_DOT1X_STATIC_WEP_COPY:
			RTMPIoctlStaticWepCopy(pAd, wrq);
			break;

		case OID_802_DOT1X_IDLE_TIMEOUT:
			RTMPIoctlSetIdleTimeout(pAd, wrq);
			break;
#endif // DOT1X_SUPPORT //

#ifdef HOSTAPD_SUPPORT
        case OID_802_11_AUTHENTICATION_MODE:
            if (wrq->u.data.length != sizeof(NDIS_802_11_AUTHENTICATION_MODE)) 
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&AuthMode, wrq->u.data.pointer, wrq->u.data.length);
                if (AuthMode > Ndis802_11AuthModeMax)
                {
                    Status  = -EINVAL;
                    break;
                }
                else
                {
                    if (pAd->ApCfg.MBSSID[pObj->ioctl_if].AuthMode != AuthMode)
                    {
                        // Config has changed
                        pAd->bConfigChanged = TRUE;
                    }
                    pAd->ApCfg.MBSSID[pObj->ioctl_if].AuthMode = AuthMode;
                }
                pAd->ApCfg.MBSSID[pObj->ioctl_if].PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_AUTHENTICATION_MODE (=%d) \n",pAd->ApCfg.MBSSID[0].AuthMode));
            }
		APStop(pAd);
		APStartUp(pAd);
            break;

        case OID_802_11_WEP_STATUS:
            if (wrq->u.data.length != sizeof(NDIS_802_11_WEP_STATUS)) 
                Status  = -EINVAL;
            else
            {
                Status = copy_from_user(&WepStatus, wrq->u.data.pointer, wrq->u.data.length);
                // Since TKIP, AES, WEP are all supported. It should not have any invalid setting
                if (WepStatus <= Ndis802_11GroupWEP104Enabled)
                {
                    if (pAd->ApCfg.MBSSID[pObj->ioctl_if].WepStatus != WepStatus)
                    {
                        // Config has changed
                        pAd->bConfigChanged = TRUE;
                    }
                    pAd->ApCfg.MBSSID[pObj->ioctl_if].WepStatus     = WepStatus;
                    pAd->ApCfg.MBSSID[pObj->ioctl_if].GroupKeyWepStatus = WepStatus;
                }
                else
                {
                    Status  = -EINVAL;
                    break;
                }
			APStop(pAd);
			APStartUp(pAd);
                	DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_802_11_WEP_STATUS (=%d)\n",WepStatus));
            }
            break;
			
        case OID_802_11_SSID:
            if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
                Status = -EINVAL;
            else
            {
            		PSTRING pSsidString = NULL;
                	Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);

                	if (Ssid.SsidLength > MAX_LEN_OF_SSID)
                    		Status = -EINVAL;
                	else
                	{
                		if (Ssid.SsidLength == 0)
		    			{
                				Status = -EINVAL;
		    			}
				else
                    		{
	                		pSsidString = (PSTRING)kmalloc(MAX_LEN_OF_SSID+1, MEM_ALLOC_FLAG);
					if (pSsidString)
					{
						NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID+1);
						NdisMoveMemory(pSsidString, Ssid.Ssid, Ssid.SsidLength);
						NdisZeroMemory((PCHAR)pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid,MAX_LEN_OF_SSID);
						strcpy((PCHAR)pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid,pSsidString);
						pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen=strlen(pSsidString);
						kfree(pSsidString);
					}
					else
						Status = -ENOMEM;
                    }
                }
            }
            break;


	case HOSTAPD_OID_SET_802_1X://pure 1x is enabled.
			Set_IEEE8021X_Proc(pAd,"1");
			break;

	case HOSTAPD_OID_SET_KEY:

			Status  = -EINVAL;
			UCHAR	apidx;
			POS_COOKIE pObj;
			pObj = (POS_COOKIE) pAd->OS_Cookie;
			apidx = pObj->ioctl_if;
			if(wrq->u.data.length != sizeof(struct ieee80211req_key) || !wrq->u.data.pointer)
				break;
			Status = copy_from_user(&Key, wrq->u.data.pointer, wrq->u.data.length);
			pEntry = MacTableLookup(pAd, Key.ik_macaddr);


			if((Key.ik_type == CIPHER_WEP64) ||(Key.ik_type == CIPHER_WEP128))//dynamic wep with 1x
			{
				if (pEntry)//pairwise key
				{
					pEntry->PairwiseKey.KeyLen = Key.ik_keylen;
					NdisMoveMemory(pEntry->PairwiseKey.Key, Key.ik_keydata, Key.ik_keylen);
					pEntry->PairwiseKey.CipherAlg = Key.ik_type;
					KeyIdx=pAd->ApCfg.MBSSID[pEntry->apidx].DefaultKeyId;
                                  AsicAddPairwiseKeyEntry(
                                      pAd, 
                                      (UCHAR)pEntry->Aid, 
                                      &pEntry->PairwiseKey);

					RTMPAddWcidAttributeEntry(
						pAd, 
						pEntry->apidx, 
						KeyIdx, // The value may be not zero
						pEntry->PairwiseKey.CipherAlg, 
						pEntry);
				}
				else//group key 
				{
					pMbss = &pAd->ApCfg.MBSSID[0];
					KeyIdx = Key.ik_keyix& 0x0fff;
					// it is a shared key
					if (KeyIdx <= 4)
					{
						pAd->SharedKey[apidx][KeyIdx].KeyLen = (UCHAR) Key.ik_keylen;
						NdisMoveMemory(pAd->SharedKey[apidx][KeyIdx].Key, &Key.ik_keydata, Key.ik_keylen);
						if (Key.ik_keyix & 0x8000)
						{
							// Default key for tx (shared key)
							pMbss->DefaultKeyId = (UCHAR) KeyIdx;
						}
						pAd->SharedKey[apidx][KeyIdx].CipherAlg = Key.ik_type;
						AsicAddSharedKeyEntry(
							pAd,
							apidx,
							KeyIdx,
						  	&pAd->SharedKey[apidx][KeyIdx]
						  	);

						RTMPAddWcidAttributeEntry(
							pAd, 
							apidx, 
							KeyIdx, 
							pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg,
							NULL);
					}
				}
			}
			else if (pEntry)
			{

				if (pEntry->WepStatus == Ndis802_11Encryption2Enabled)
				{
					pEntry->PairwiseKey.KeyLen = LEN_TK;
					NdisMoveMemory(&pEntry->PTK[32], Key.ik_keydata, 32);
					//NdisMoveMemory(&pEntry->PTK[32+16], (Key.ik_keydata+16), 8);
					//NdisMoveMemory(&pEntry->PTK[32+16+8], (Key.ik_keydata+16+8), 8);
					NdisMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[32], Key.ik_keylen);
				}

				if(pEntry->WepStatus == Ndis802_11Encryption3Enabled)
				{
					pEntry->PairwiseKey.KeyLen = LEN_TK;
					NdisMoveMemory(&pEntry->PTK[32], Key.ik_keydata, 32);
					NdisMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[32], Key.ik_keylen);
				}


    				pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
    				if (pEntry->WepStatus == Ndis802_11Encryption2Enabled)
        				pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
    				else if (pEntry->WepStatus == Ndis802_11Encryption3Enabled)
        				pEntry->PairwiseKey.CipherAlg = CIPHER_AES;
				
				pEntry->PairwiseKey.CipherAlg = Key.ik_type;
				 
                            AsicAddPairwiseKeyEntry(
                                pAd, 
                                (UCHAR)pEntry->Aid, 
                                &pEntry->PairwiseKey);	

				RTMPSetWcidSecurityInfo(pAd, 
					pEntry->apidx, 
					(UINT8)KeyIdx, 										 
					pEntry->PairwiseKey.CipherAlg, 
					pEntry->Aid,
					PAIRWISEKEYTABLE);
			} 
			else
			{
				pMbss = &pAd->ApCfg.MBSSID[0];
				KeyIdx = Key.ik_keyix& 0x0fff;
				
				if (Key.ik_keyix & 0x8000)
				{
					pMbss->DefaultKeyId = (UCHAR) KeyIdx-1;
				}

				if (pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11Encryption2Enabled)
				{
					pAd->SharedKey[apidx][pMbss->DefaultKeyId].KeyLen= LEN_TK;
					NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].Key, Key.ik_keydata, 16);
					NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].RxMic, (Key.ik_keydata+16+8), 8);
					NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].TxMic, (Key.ik_keydata+16), 8);
				}

				if(pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
				{
					pAd->SharedKey[apidx][pMbss->DefaultKeyId].KeyLen= LEN_TK;	
					NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].Key, Key.ik_keydata, 16);
					NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].RxMic, (Key.ik_keydata+16+8), 8);
					NdisMoveMemory(pAd->SharedKey[apidx][pMbss->DefaultKeyId].TxMic, (Key.ik_keydata+16), 8);
				}

    				pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg  = CIPHER_NONE;
    				if (pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11Encryption2Enabled)
        				pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg = CIPHER_TKIP;
    				else if (pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
        				pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg = CIPHER_AES;

				hex_dump("Key.ik_keydata,", (unsigned char*) Key.ik_keydata, 32);

				AsicAddSharedKeyEntry(
					pAd,
					apidx,
					pMbss->DefaultKeyId,
					&pAd->SharedKey[apidx][pMbss->DefaultKeyId]
					);

				RTMPAddWcidAttributeEntry(
				pAd, 
				apidx,
				pMbss->DefaultKeyId, 
				pAd->SharedKey[apidx][pMbss->DefaultKeyId].CipherAlg,
				NULL);
			}
			break;

		case HOSTAPD_OID_DEL_KEY:

			Status  = -EINVAL;
			if(wrq->u.data.length != sizeof(struct ieee80211req_del_key) || !wrq->u.data.pointer)
				break;
			Status = copy_from_user(&delkey, wrq->u.data.pointer, wrq->u.data.length);
			pEntry = MacTableLookup(pAd, delkey.idk_macaddr);
			if (pEntry){
				// clear the previous Pairwise key table
				if(pEntry->Aid != 0)
				{
					NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));
					AsicRemovePairwiseKeyEntry(pAd,(UCHAR)pEntry->Aid);
				}
			}
			else if((delkey.idk_macaddr == NULL) && (delkey.idk_keyix < 4))
				// remove group key
				AsicRemoveSharedKeyEntry(pAd, pEntry->apidx, delkey.idk_keyix);
			break;

		case HOSTAPD_OID_SET_STA_AUTHORIZED://for portsecured flag.

			if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
			{
				Status  = -EINVAL;
			}
			else
			{
				Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
				pEntry = MacTableLookup(pAd, mlme.im_macaddr);
				if (!pEntry){
					Status = -EINVAL;
				}
				else
				{
					switch (mlme.im_op)
					{
						case IEEE80211_MLME_AUTHORIZE:
							pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
							pEntry->WpaState = AS_PTKINITDONE;//wpa state machine is not in use.
							//pAd->StaCfg.PortSecured= WPA_802_1X_PORT_SECURED;
							pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
							break;
						case IEEE80211_MLME_UNAUTHORIZE:
							pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
							pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
						break;
						default:
							Status = -EINVAL;
					}
				}
			}
			break;

		case HOSTAPD_OID_STATIC_WEP_COPY:
			
			if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
			{
				Status  = -EINVAL;
			}
			else
			{
				Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
				pEntry = MacTableLookup(pAd, mlme.im_macaddr);
				if (!pEntry){
					Status = -EINVAL;
				}
			}
			break;

		case HOSTAPD_OID_SET_STA_DEAUTH:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::HOSTAPD_OID_SET_STA_DEAUTH\n"));
			MLME_DEAUTH_REQ_STRUCT  *pInfo;
			MLME_QUEUE_ELEM 		*Elem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);
			if(Elem)
			{
				pInfo = (MLME_DEAUTH_REQ_STRUCT *) Elem->Msg;

				if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
				{
					Status  = -EINVAL;
				}
				else
				{
					Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
					NdisMoveMemory(pInfo->Addr, mlme.im_macaddr, MAC_ADDR_LEN);
					if ((pEntry = MacTableLookup(pAd, pInfo->Addr)) != NULL)
					{
						pInfo->Reason = mlme.im_reason;
						Elem->Wcid = pEntry->Aid;
						MlmeEnqueue(pAd, AP_AUTH_STATE_MACHINE, APMT2_MLME_DEAUTH_REQ, sizeof(MLME_DEAUTH_REQ_STRUCT), Elem,0);
					}
				}
			}
			break;

		case HOSTAPD_OID_SET_STA_DISASSOC://hostapd request to disassoc the station.
			DBGPRINT(RT_DEBUG_TRACE, ("Set::HOSTAPD_OID_SET_STA_DISASSOC\n"));
			MLME_DISASSOC_REQ_STRUCT DisassocReq;
			if (wrq->u.data.length != sizeof(struct ieee80211req_mlme))
			{
				Status  = -EINVAL;
			}
			else
			{
				Status = copy_from_user(&mlme, wrq->u.data.pointer, wrq->u.data.length);
				NdisMoveMemory(DisassocReq.Addr, mlme.im_macaddr, MAC_ADDR_LEN);
				DisassocReq.Reason = mlme.im_reason;
				MlmeEnqueue(pAd, AP_ASSOC_STATE_MACHINE, APMT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq,0);
			}
			break;

		case OID_HOSTAPD_SUPPORT://notify the driver to support hostapd.
			
			if (wrq->u.data.length != sizeof(BOOLEAN))
				Status  = -EINVAL;
			else
			{
				BOOLEAN hostapd_enable;
				int v;

				Status = copy_from_user(&hostapd_enable, wrq->u.data.pointer, wrq->u.data.length);
				pAd->ApCfg.Hostapd = hostapd_enable;
				MULTISSID_STRUCT *pMBSSStruct;

				for(v=0;v<MAX_MBSSID_NUM;v++)
				{
					pMBSSStruct = &pAd->ApCfg.MBSSID[v];
					pMBSSStruct->WPAREKEY.ReKeyInterval = 0;
					pMBSSStruct->WPAREKEY.ReKeyMethod = DISABLE_REKEY;
				}
			}
		break;

		case HOSTAPD_OID_COUNTERMEASURES://report txtsc to hostapd.
			
			if (wrq->u.data.length != sizeof(BOOLEAN))
				Status  = -EINVAL;
			else
			{
				BOOLEAN countermeasures_enable;
				Status = copy_from_user(&countermeasures_enable, wrq->u.data.pointer, wrq->u.data.length);

				if(countermeasures_enable)
				{

    						{
        						DBGPRINT(RT_DEBUG_ERROR, ("Receive CM Attack Twice within 60 seconds ====>>> \n"));
        
							// send wireless event - for counter measures
							pAd->ApCfg.CMTimerRunning = FALSE;

						        for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
						        {
						        	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
						            // happened twice within 60 sec,  AP SENDS disaccociate all associated STAs.  All STA's transition to State 2
						            if (IS_ENTRY_CLIENT(pEntry))
						            {
						                MlmeDeAuthAction(pAd, &pAd->MacTab.Content[i], REASON_MIC_FAILURE, FALSE);
						            }
						        }
	        
						        // Further,  ban all Class 3 DATA transportation for  a period 0f 60 sec
						        // disallow new association , too
						        pAd->ApCfg.BANClass3Data = TRUE;        

						    }


				}
				else
				{        
    					    pAd->ApCfg.BANClass3Data = FALSE;
				}
			}
			break;

	case HOSTAPD_OID_SET_WPS_BEACON_IE://pure 1x is enabled.
#ifdef WSC_AP_SUPPORT
				DBGPRINT(RT_DEBUG_TRACE,("HOSTAPD_OID_SET_WPS_BEACON_IE\n"));
				if (wrq->u.data.length != sizeof(WSC_LV_INFO))
				{
					Status  = -EINVAL;
				}
				else
				{	
					pAd->ApCfg.HostapdWPS = TRUE;
					MULTISSID_STRUCT *pMBSSStruct;
					NdisZeroMemory(&WscIEBeacon,sizeof(WSC_LV_INFO));
					Status = copy_from_user(&WscIEBeacon, wrq->u.data.pointer, wrq->u.data.length);
					pMBSSStruct = &pAd->ApCfg.MBSSID[0];
					NdisMoveMemory(pMBSSStruct->WscIEBeacon.Value,WscIEBeacon.Value, WscIEBeacon.ValueLen);
					pMBSSStruct->WscIEBeacon.ValueLen=WscIEBeacon.ValueLen;
					APMakeAllBssBeacon(pAd);
					APUpdateAllBeaconFrame(pAd);									
				}
#endif
			break;

	case HOSTAPD_OID_SET_WPS_PROBE_RESP_IE://pure 1x is enabled.
#ifdef WSC_AP_SUPPORT
				DBGPRINT(RT_DEBUG_TRACE,("HOSTAPD_OID_SET_WPS_PROBE_RESP_IE\n"));
				if (wrq->u.data.length != sizeof(WSC_LV_INFO))
				{
					DBGPRINT(RT_DEBUG_TRACE,("HOSTAPD_OID_SET_WPS_PROBE_RESP_IE failed\n"));
					Status  = -EINVAL;
				}
				else
				{	
					pAd->ApCfg.HostapdWPS = TRUE;
					MULTISSID_STRUCT *pMBSSStruct;
					NdisZeroMemory(&WscIEProbeResp,sizeof(WSC_LV_INFO));
					Status = copy_from_user(&WscIEProbeResp, wrq->u.data.pointer, wrq->u.data.length);
					pMBSSStruct = &pAd->ApCfg.MBSSID[0];
					NdisMoveMemory(pMBSSStruct->WscIEProbeResp.Value,WscIEProbeResp.Value, WscIEProbeResp.ValueLen);
					pMBSSStruct->WscIEProbeResp.ValueLen=WscIEProbeResp.ValueLen;

					APMakeAllBssBeacon(pAd);
					APUpdateAllBeaconFrame(pAd);
									
				}
#endif
			break;
#endif //HOSTAPD_SUPPORT//


   		default:
			DBGPRINT(RT_DEBUG_TRACE, ("Set::unknown IOCTL's subcmd = 0x%08x\n", cmd));
			Status = -EOPNOTSUPP;
			break;
    }
	

	return Status;
}


INT RTMPAPQueryInformation(
	IN	PRTMP_ADAPTER       pAd,
	IN	OUT	struct iwreq    *rq,
	IN	INT                 cmd)
{
	struct iwreq						*wrq = (struct iwreq *) rq;
    INT	Status = NDIS_STATUS_SUCCESS;
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    STRING	driverVersion[8];

#if defined(DBG) || defined(WSC_AP_SUPPORT) || defined(LLTD_SUPPORT)
	UCHAR	apidx = pObj->ioctl_if;
#endif
#ifdef WSC_AP_SUPPORT
	UINT	WscPinCode = 0;
	PWSC_PROFILE	pProfile;
#endif // WSC_AP_SUPPORT //

#ifdef SNMP_SUPPORT	
	//for snmp, kathy
	DefaultKeyIdxValue			*pKeyIdxValue;
	INT							valueLen;
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						tmp[64];
#endif // SNMP_SUPPORT //	

#ifdef HOSTAPD_SUPPORT
	struct default_group_key			group_key;
	struct ieee80211req_key			ik;
	unsigned char						*p;
	MAC_TABLE_ENTRY				*pEntry=(MAC_TABLE_ENTRY *)NULL;
	struct ieee80211req_wpaie			wpaie;
	PMULTISSID_STRUCT	pMbss ;
#endif //HOSTAPD_SUPPORT//

#ifdef RTMP_RBUS_SUPPORT
	NDIS_802_11_STATISTICS	*pStatistics;
	ULONG ulInfo;
#endif // RTMP_RBUS_SUPPORT //

    switch(cmd)
    {
		case RT_OID_VERSION_INFO:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_VERSION_INFO \n"));
			wrq->u.data.length = 8*sizeof(CHAR);
			sprintf(&driverVersion[0], "%s", AP_DRIVER_VERSION);
			driverVersion[7] = '\0';
			if (copy_to_user(wrq->u.data.pointer, &driverVersion, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			break;

		case OID_802_11_NETWORK_TYPES_SUPPORTED:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_NETWORK_TYPES_SUPPORTED \n"));
			wrq->u.data.length = sizeof(UCHAR);
			if (copy_to_user(wrq->u.data.pointer, &pAd->RfIcType, wrq->u.data.length))
			{
				Status = -EFAULT; 	
			}
			break;

#ifdef IAPP_SUPPORT
		case RT_QUERY_SIGNAL_CONTEXT:
		{
			BOOLEAN FlgIs11rSup = FALSE;

			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_QUERY_SIGNAL_CONTEXT \n"));


			if (FlgIs11rSup == FALSE)
			{
			{
				Status = -EFAULT;
			}
		}
		}
			break;


#endif // IAPP_SUPPORT //

#ifdef WMM_ACM_SUPPORT
		case RT_OID_WMM_ACM_TSPEC:
		{
			UINT32 NumStream;
			CHAR *pMac;
			UINT32 *pNumOfTspec;
			BOOLEAN FlgIsOk;

			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WMM_ACM_TSPEC \n"));

			FlgIsOk = FALSE;
			pMac = wrq->u.data.pointer; /* from user */
			pNumOfTspec = (UINT32 *)wrq->u.data.pointer; /* to user */
			NumStream = ACMP_StreamNumGet(pAd, ACM_SM_CATEGORY_PEER, 1, (PUCHAR)pMac);

			if (NumStream > 0)
			{
				/* at least one TSPEC */
				if (wrq->u.data.length >= \
						(sizeof(UINT32)+sizeof(ACM_STREAM_INFO)*NumStream))
				{
					/* user buffer is enough to fill all TSPECs */
					if (ACMP_StreamsGet(
							pAd, ACM_SM_CATEGORY_PEER, 1,
							&NumStream, (PUCHAR)pMac,
							wrq->u.data.pointer+sizeof(UINT32)) == ACM_RTN_OK)
					{
						/* fill the actual number of TSPEC */
						*pNumOfTspec = NumStream;
						FlgIsOk = TRUE;
					}
				}
			}

			if (FlgIsOk == FALSE)
				*pNumOfTspec = 0; /* get fail */
		}
			break;
#endif // WMM_ACM_SUPPORT //

#ifdef WSC_AP_SUPPORT
		case RT_OID_WSC_QUERY_STATUS:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_STATUS \n"));
			wrq->u.data.length = sizeof(INT);
			if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].WscControl.WscStatus, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			break;

		case RT_OID_WSC_PIN_CODE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_PIN_CODE \n"));
			wrq->u.data.length = sizeof(UINT);
			//WscPinCode = GenerateWpsPinCode(pAd, FALSE, apidx);
			WscPinCode = pAd->ApCfg.MBSSID[0].WscControl.WscEnrolleePinCode;
			
			if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			break;
#ifdef APCLI_SUPPORT
        case RT_OID_APCLI_WSC_PIN_CODE:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_APCLI_WSC_PIN_CODE \n"));
			wrq->u.data.length = sizeof(UINT);
			//WscPinCode = GenerateWpsPinCode(pAd, TRUE, apidx);
			WscPinCode = pAd->ApCfg.ApCliTab[0].WscControl.WscEnrolleePinCode;
			
			if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
            break;
#endif // APCLI_SUPPORT //
		case RT_OID_WSC_UUID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_UUID \n"));
			wrq->u.data.length = UUID_LEN_STR;
			if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].WscControl.Wsc_Uuid_Str[0], UUID_LEN_STR))
			{
				Status = -EFAULT;
			}
			break;
		case RT_OID_WSC_MAC_ADDRESS:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_MAC_ADDRESS \n"));
			wrq->u.data.length = MAC_ADDR_LEN;
			if (copy_to_user(wrq->u.data.pointer, pAd->ApCfg.MBSSID[apidx].Bssid, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			break;
		case RT_OID_WSC_CONFIG_STATUS:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_CONFIG_STATUS \n"));
			wrq->u.data.length = sizeof(UCHAR);
			if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].WscControl.WscConfStatus, wrq->u.data.length))
			{
				Status = -EFAULT;
			}
			break;

		case RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING \n"));

			if (pAd->ApCfg.MBSSID[apidx].WscControl.WscState > WSC_STATE_WAIT_M2)
			{
				wrq->u.data.length = sizeof(WSC_PEER_DEV_INFO);
				if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].WscControl.WscPeerInfo, wrq->u.data.length))
				{
					Status = -EFAULT;
				}
			}
			else
			{
				Status = -EFAULT;
			}
			break;

		case RT_OID_802_11_WSC_QUERY_PROFILE:
			wrq->u.data.length = sizeof(WSC_PROFILE);
			pProfile = kmalloc(sizeof(WSC_PROFILE), MEM_ALLOC_FLAG);
			if (pProfile == NULL)
			{
				Status = -EFAULT;
				DBGPRINT(RT_DEBUG_TRACE, ("RT_OID_802_11_WSC_QUERY_PROFILE fail!\n"));
				break;
			}

			RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
			NdisMoveMemory(pProfile, &pAd->ApCfg.MBSSID[apidx].WscControl.WscProfile, sizeof(WSC_PROFILE));
            if ((pProfile->Profile[0].AuthType == WSC_AUTHTYPE_OPEN) && (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_NONE))
            {
                pProfile->Profile[0].KeyLength = 0;
                NdisZeroMemory(pProfile->Profile[0].Key, 64);
            }
			if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			{
				Status = -EFAULT;
			}			

			kfree(pProfile);
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WSC_QUERY_PROFILE \n"));
			break;
#endif // WSC_AP_SUPPORT //
#ifdef LLTD_SUPPORT
        case RT_OID_GET_PHY_MODE:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::Get phy mode (%02X) \n", pAd->CommonCfg.PhyMode));
            wrq->u.mode = (u32)pAd->CommonCfg.PhyMode;
            break;

        case RT_OID_GET_LLTD_ASSO_TABLE:
            DBGPRINT(RT_DEBUG_TRACE, ("Query::Get LLTD association table\n"));
            if ((wrq->u.data.pointer == NULL) || (apidx != MAIN_MBSSID))
            {
                Status = -EFAULT;
            }
            else
            {
                INT						    i;
                RT_LLTD_ASSOICATION_TABLE	AssocTab;

            	AssocTab.Num = 0;
            	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
            	{
            		if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].Sst == SST_ASSOC))
            		{
            			COPY_MAC_ADDR(AssocTab.Entry[AssocTab.Num].Addr, &pAd->MacTab.Content[i].Addr);
                        AssocTab.Entry[AssocTab.Num].phyMode = pAd->CommonCfg.PhyMode;
                        AssocTab.Entry[AssocTab.Num].MOR = RateIdToMbps[pAd->ApCfg.MBSSID[apidx].MaxTxRate] * 2;
            			AssocTab.Num += 1;
            		}
            	}            
                wrq->u.data.length = sizeof(RT_LLTD_ASSOICATION_TABLE);
            	if (copy_to_user(wrq->u.data.pointer, &AssocTab, wrq->u.data.length))
            	{
            		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
                    Status = -EFAULT;
            	}
                DBGPRINT(RT_DEBUG_TRACE, ("AssocTab.Num = %d \n", AssocTab.Num));
            }
            break;
#ifdef APCLI_SUPPORT
		case RT_OID_GET_REPEATER_AP_LINEAGE:
			DBGPRINT(RT_DEBUG_TRACE, ("Not Support : Get repeater AP lineage.\n"));
			break;
#endif // APCLI_SUPPORT //

#endif // LLTD_SUPPORT //
#ifdef DOT1X_SUPPORT
		case OID_802_DOT1X_CONFIGURATION:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::Get Radius setting(%d)\n", sizeof(DOT1X_CMM_CONF)));
				RTMPIoctlQueryRadiusConf(pAd, wrq);	
			break;
#endif // DOT1X_SUPPORT //			
#ifdef SNMP_SUPPORT
		case RT_OID_802_11_MAC_ADDRESS:
            wrq->u.data.length = MAC_ADDR_LEN;
            Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTUREROUI:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTUREROUI \n"));
			wrq->u.data.length = ManufacturerOUI_LEN;
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTURERNAME:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTURERNAME \n"));
			wrq->u.data.length = strlen(ManufacturerNAME);
			Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
			break;

		case RT_OID_802_11_RESOURCETYPEIDNAME:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_RESOURCETYPEIDNAME \n"));
			wrq->u.data.length = strlen(ResourceTypeIdName);
			Status = copy_to_user(wrq->u.data.pointer, ResourceTypeIdName, wrq->u.data.length);
			break;

		case RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED \n"));
			ulInfo = 1; // 1 is support wep else 2 is not support.
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;

		case RT_OID_802_11_POWERMANAGEMENTMODE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_POWERMANAGEMENTMODE \n"));
			ulInfo = 1; // 1 is power active else 2 is power save.
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			break;

		case OID_802_11_WEPDEFAULTKEYVALUE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WEPDEFAULTKEYVALUE \n"));
			//KeyIdxValue.KeyIdx = pAd->PortCfg.MBSSID[pObj->ioctl_if].DefaultKeyId;
			pKeyIdxValue = wrq->u.data.pointer;
			DBGPRINT(RT_DEBUG_TRACE,("KeyIdxValue.KeyIdx = %d, \n",pKeyIdxValue->KeyIdx));

			valueLen = pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen;
			NdisMoveMemory(pKeyIdxValue->Value,
						   &pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].Key,
						   valueLen);
			pKeyIdxValue->Value[valueLen]='\0';

			wrq->u.data.length = sizeof(DefaultKeyIdxValue);

			Status = copy_to_user(wrq->u.data.pointer, pKeyIdxValue, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE,("DefaultKeyId = %d, total len = %d, str len=%d, KeyValue= %02x %02x %02x %02x \n", pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId, wrq->u.data.length, pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen,
			pAd->SharedKey[pObj->ioctl_if][0].Key[0],
			pAd->SharedKey[pObj->ioctl_if][1].Key[0],
			pAd->SharedKey[pObj->ioctl_if][2].Key[0],
			pAd->SharedKey[pObj->ioctl_if][3].Key[0]));
			break;

		case OID_802_11_WEPDEFAULTKEYID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WEPDEFAULTKEYID \n"));
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("DefaultKeyId =%d \n", pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId));
			break;

		case RT_OID_802_11_WEPKEYMAPPINGLENGTH:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_WEPKEYMAPPINGLENGTH \n"));
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_to_user(wrq->u.data.pointer,
									&pAd->SharedKey[pObj->ioctl_if][pAd->ApCfg.MBSSID[pObj->ioctl_if].DefaultKeyId].KeyLen,
									wrq->u.data.length);
			break;

		case OID_802_11_SHORTRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_SHORTRETRYLIMIT \n"));
			wrq->u.data.length = sizeof(ULONG);
			RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
			ShortRetryLimit = tx_rty_cfg.field.ShortRtyLimit;
			DBGPRINT(RT_DEBUG_TRACE, ("ShortRetryLimit =%ld,  tx_rty_cfg.field.ShortRetryLimit=%d\n", ShortRetryLimit, tx_rty_cfg.field.ShortRtyLimit));
			Status = copy_to_user(wrq->u.data.pointer, &ShortRetryLimit, wrq->u.data.length);
			break;

		case OID_802_11_LONGRETRYLIMIT:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_LONGRETRYLIMIT \n"));
			wrq->u.data.length = sizeof(ULONG);
			RTMP_IO_READ32(pAd, TX_RTY_CFG, &tx_rty_cfg.word);
			LongRetryLimit = tx_rty_cfg.field.LongRtyLimit;
			DBGPRINT(RT_DEBUG_TRACE, ("LongRetryLimit =%ld,  tx_rty_cfg.field.LongRtyLimit=%d\n", LongRetryLimit, tx_rty_cfg.field.LongRtyLimit));
			Status = copy_to_user(wrq->u.data.pointer, &LongRetryLimit, wrq->u.data.length);
			break;
			
		case RT_OID_802_11_PRODUCTID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_PRODUCTID \n"));
		
#ifdef RTMP_MAC_PCI
			{
			
				USHORT  device_id=0;
				if (((POS_COOKIE)pAd->OS_Cookie)->pci_dev != NULL)
			    	pci_read_config_word(((POS_COOKIE)pAd->OS_Cookie)->pci_dev, PCI_DEVICE_ID, &device_id);
				else 
					DBGPRINT(RT_DEBUG_TRACE, (" pci_dev = NULL\n"));
				sprintf((PSTRING)tmp, "%04x %04x\n", NIC_PCI_VENDOR_ID, device_id);
			}
#endif // RTMP_MAC_PCI //
			wrq->u.data.length = strlen((PSTRING) tmp);
			Status = copy_to_user(wrq->u.data.pointer, tmp, wrq->u.data.length);
			break;

		case RT_OID_802_11_MANUFACTUREID:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MANUFACTUREID \n"));
			wrq->u.data.length = strlen(ManufacturerNAME);
			Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
			break;

		case OID_802_11_CURRENTCHANNEL:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_CURRENTCHANNEL \n"));
			wrq->u.data.length = sizeof(UCHAR);
			DBGPRINT(RT_DEBUG_TRACE, ("sizeof UCHAR=%d, channel=%d \n", sizeof(UCHAR), pAd->CommonCfg.Channel));
			Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Channel, wrq->u.data.length);
			DBGPRINT(RT_DEBUG_TRACE, ("Status=%d\n", Status));
			break;
#endif // SNMP_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
        case OID_802_11_STATISTICS:
            pStatistics = (NDIS_802_11_STATISTICS *) kmalloc(sizeof(NDIS_802_11_STATISTICS), MEM_ALLOC_FLAG);
            if (pStatistics)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_STATISTICS \n"));
                // add the most up-to-date h/w raw counters into software counters
			    NICUpdateRawCounters(pAd);
                
                pStatistics->TransmittedFragmentCount.QuadPart = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;
                pStatistics->MulticastTransmittedFrameCount.QuadPart = pAd->WlanCounters.MulticastTransmittedFrameCount.QuadPart;
                pStatistics->FailedCount.QuadPart = pAd->WlanCounters.FailedCount.QuadPart;
                pStatistics->RetryCount.QuadPart = pAd->WlanCounters.RetryCount.QuadPart;
                pStatistics->MultipleRetryCount.QuadPart = pAd->WlanCounters.MultipleRetryCount.QuadPart;
                pStatistics->RTSSuccessCount.QuadPart = pAd->WlanCounters.RTSSuccessCount.QuadPart;
                pStatistics->RTSFailureCount.QuadPart = pAd->WlanCounters.RTSFailureCount.QuadPart;
                pStatistics->ACKFailureCount.QuadPart = pAd->WlanCounters.ACKFailureCount.QuadPart;
                pStatistics->FrameDuplicateCount.QuadPart = pAd->WlanCounters.FrameDuplicateCount.QuadPart;
                pStatistics->ReceivedFragmentCount.QuadPart = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
                pStatistics->MulticastReceivedFrameCount.QuadPart = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;
#ifdef DBG	
                pStatistics->FCSErrorCount = pAd->RalinkCounters.RealFcsErrCount;
#else
                pStatistics->FCSErrorCount.QuadPart = pAd->WlanCounters.FCSErrorCount.QuadPart;
                pStatistics->FrameDuplicateCount.u.LowPart = pAd->WlanCounters.FrameDuplicateCount.u.LowPart / 100;
#endif
                wrq->u.data.length = sizeof(NDIS_802_11_STATISTICS);
                Status = copy_to_user(wrq->u.data.pointer, pStatistics, wrq->u.data.length);
                kfree(pStatistics);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_STATISTICS(kmalloc failed)\n"));
                Status = -EFAULT;
            }
            break;
#endif // RTMP_RBUS_SUPPORT //

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
		case RT_OID_802_11_QUERY_TXBF_TABLE:
			{
				INT i;
				RT_802_11_TXBF_TABLE MacTab;

				MacTab.Num = 0;
				for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
				{
					if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].Sst == SST_ASSOC))
					{
						memcpy(&MacTab.Entry[MacTab.Num], &pAd->MacTab.Content[i].TxBFCounters, sizeof(RT_COUNTER_TXBF));
						MacTab.Num++;
					}
				}

				wrq->u.data.length = sizeof(RT_802_11_TXBF_TABLE);
				Status = copy_to_user(wrq->u.data.pointer, &MacTab, wrq->u.data.length);
			}
			break;
#endif // TXBF_SUPPORT //
#endif // DOT11_N_SUPPORT //


#ifdef WAPI_SUPPORT
		case OID_802_11_MCAST_TXIV:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_MCAST_TXIV \n"));
			//wrq->u.data.length = LEN_WAPI_TSC;
			//Status = copy_to_user(wrq->u.data.pointer, 0/*pAd->ApCfg.MBSSID[pObj->ioctl_if].tx_iv*/, wrq->u.data.length);
			Status  = -EINVAL;
			break;			
		case OID_802_11_WAPI_CONFIGURATION:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::Get WAPI Configuration(%d)\n", sizeof(WAPI_CONF)));
			RTMPIoctlQueryWapiConf(pAd, wrq);	
			break;			
		case OID_802_11_WAPI_IE:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_WAPI_IE\n"));
			if (wrq->u.data.length != sizeof(WAPI_WIE_STRUCT))
                Status  = -EINVAL;
            else
            {                												
				WAPI_WIE_STRUCT   	wapi_ie;
				MAC_TABLE_ENTRY		*pWapiEntry;

				NdisZeroMemory(&wapi_ie, sizeof(WAPI_WIE_STRUCT));
				NdisMoveMemory(wapi_ie.addr, wrq->u.data.pointer, MAC_ADDR_LEN);
					
				pWapiEntry = MacTableLookup(pAd, wapi_ie.addr);
						
				if (pWapiEntry && IS_ENTRY_CLIENT(pWapiEntry) && (pWapiEntry->RSNIE_Len > 0))
				{										
					wapi_ie.wie_len = pWapiEntry->RSNIE_Len;
					NdisMoveMemory(wapi_ie.wie, pWapiEntry->RSN_IE, pWapiEntry->RSNIE_Len);						
				}
								
				if (copy_to_user(wrq->u.data.pointer, &wapi_ie, wrq->u.data.length))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
				}								
            }
			break;		

		case OID_802_11_MCAST_KEY_INFO:
			{
				PMULTISSID_STRUCT pMbss;
				WAPI_MCAST_KEY_STRUCT   wapi_mkey;

				DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_MCAST_KEY_INFO\n"));
							
				pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];					
				NdisZeroMemory(&wapi_mkey, sizeof(WAPI_MCAST_KEY_STRUCT));

				if (pMbss->sw_wpi_encrypt)
				{					
					NdisMoveMemory(wapi_mkey.m_tx_iv, 
								   pAd->SharedKey[pObj->ioctl_if][pMbss->DefaultKeyId].TxTsc,
								   LEN_WAPI_TSC);
				}
				else
				{
					INT	m_wcid;	
				
					GET_GroupKey_WCID(m_wcid, apidx);
					RTMPGetWapiTxTscFromAsic(pAd, m_wcid, wapi_mkey.m_tx_iv);
				}				
				wapi_mkey.key_id = pMbss->DefaultKeyId;													
				NdisMoveMemory(wapi_mkey.key_announce, pMbss->key_announce_flag, LEN_WAPI_TSC);
				NdisMoveMemory(wapi_mkey.NMK, pMbss->NMK, 16);

				wrq->u.data.length = sizeof(WAPI_MCAST_KEY_STRUCT);
				Status = copy_to_user(wrq->u.data.pointer, &wapi_mkey, wrq->u.data.length);
			}
			break;		
			
#endif // WAPI_SUPPORT //

#ifdef HOSTAPD_SUPPORT

		case HOSTAPD_OID_GETWPAIE://report wpa ie of the new station to hostapd.
			
			if (wrq->u.data.length != sizeof(wpaie))
			{
				Status = -EINVAL;
			}
			else if (copy_from_user(&wpaie, wrq->u.data.pointer, IEEE80211_ADDR_LEN))
			{
				Status = -EFAULT;
			}
			else
			{
				pEntry = MacTableLookup(pAd, wpaie.wpa_macaddr);
				if (!pEntry){
					Status = -EINVAL;
					break;
				}
				NdisZeroMemory(wpaie.wpa_ie,sizeof(wpaie.wpa_ie));
				// For WPA1, RSN_IE=221
				if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) 
					|| (pEntry->AuthMode == Ndis802_11AuthModeWPA1WPA2) ||(pEntry->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
				{
					int ielen = pEntry->RSNIE_Len;
					DBGPRINT(RT_DEBUG_TRACE, ("pEntry->RSNIE_Len=%d\n",pEntry->RSNIE_Len));
					if (ielen > sizeof(wpaie.wpa_ie))
						ielen = sizeof(wpaie.wpa_ie);
					p = wpaie.wpa_ie;
					hex_dump("HOSTAPD_OID_GETWPAIE woody==>pEntry->RSN_IE", (unsigned char*)pEntry->RSN_IE,ielen);
					NdisMoveMemory(p, pEntry->RSN_IE, ielen);
				}
				// For WPA2, RSN_IE=48
				if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
					|| (pEntry->AuthMode == Ndis802_11AuthModeWPA1WPA2) ||(pEntry->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
				{
					int ielen = pEntry->RSNIE_Len;
					if (ielen > sizeof(wpaie.rsn_ie))
						ielen = sizeof(wpaie.rsn_ie);
					p = wpaie.rsn_ie;
					hex_dump("HOSTAPD_OID_GETWPAIE woody==>pEntry->RSN_IE", (unsigned char*)pEntry->RSN_IE,ielen);
					NdisMoveMemory(p, pEntry->RSN_IE, ielen);
				}
			}
			if(copy_to_user(wrq->u.data.pointer, &wpaie, sizeof(wpaie)))
				Status = -EFAULT;
			break;

			
		case HOSTAPD_OID_GET_SEQ://report txtsc to hostapd.

			pMbss = &pAd->ApCfg.MBSSID[0];
			if (wrq->u.data.length != sizeof(ik))
			{
				Status = -EINVAL;
			}
			else if (copy_from_user(&ik, wrq->u.data.pointer, IEEE80211_ADDR_LEN))
			{
				Status = -EFAULT;
			}
			else
			{
				NdisZeroMemory(&ik.ik_keytsc, sizeof(ik.ik_keytsc));
				p = (unsigned char *)&ik.ik_keytsc;
				NdisMoveMemory(p+2, pAd->SharedKey[pAd->IoctlIF][ pMbss->DefaultKeyId].TxTsc, 6);
				if(copy_to_user(wrq->u.data.pointer, &ik, sizeof(ik)))
					Status = -EFAULT;
			}
			break;

			
		case HOSTAPD_OID_GET_1X_GROUP_KEY://report default group key to hostapd.

			pMbss = &pAd->ApCfg.MBSSID[0];
			if (wrq->u.data.length != sizeof(group_key))
			{
				Status = -EINVAL;
			}
			else
			{
				if(pAd->SharedKey[pAd->IoctlIF][ pMbss->DefaultKeyId].KeyLen!=0 && pAd->SharedKey[pAd->IoctlIF][ pMbss->DefaultKeyId].Key!=NULL)
				{
					group_key.ik_keyix = pMbss->DefaultKeyId;
					group_key.ik_keylen = pAd->SharedKey[pAd->IoctlIF][ pMbss->DefaultKeyId].KeyLen;
					NdisMoveMemory(group_key.ik_keydata, pAd->SharedKey[pAd->IoctlIF][ pMbss->DefaultKeyId].Key,pAd->SharedKey[pAd->IoctlIF][ pMbss->DefaultKeyId].KeyLen);
					if(copy_to_user(wrq->u.data.pointer, &group_key, sizeof(group_key)))
						Status = -EFAULT;
				}
			}
			break;

#endif//HOSTAPD_SUPPORT//

#ifdef APCLI_SUPPORT
		case OID_GEN_MEDIA_CONNECT_STATUS:
			{
				ULONG ApCliIdx = pObj->ioctl_if;

				NDIS_MEDIA_STATE MediaState;
				PMAC_TABLE_ENTRY pEntry;
				PAPCLI_STRUCT pApCliEntry;

				if (pObj->ioctl_if_type != INT_APCLI)
				{
					Status = -EOPNOTSUPP;
					break;
				}
				else
				{
					APCLI_MR_APIDX_SANITY_CHECK(ApCliIdx);
					pApCliEntry = &pAd->ApCfg.ApCliTab[ApCliIdx];
					pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID]; 

					if (!IS_ENTRY_APCLI(pEntry))
					{
						Status = -EOPNOTSUPP;
						break;
					}

					if ((pAd->ApCfg.ApCliTab[ApCliIdx].Valid == TRUE)
						&& (pEntry->PortSecured == WPA_802_1X_PORT_SECURED))
						MediaState = NdisMediaStateConnected;
					else
						MediaState = NdisMediaStateDisconnected;

					wrq->u.data.length = sizeof(NDIS_MEDIA_STATE);
					Status = copy_to_user(wrq->u.data.pointer, &MediaState, wrq->u.data.length);
				}
			}
			break;
#endif // APCLI_SUPPORT //

#ifdef RTMP_RBUS_SUPPORT
		case RT_OID_802_11_SNR_0:
			if ((pAd->ApCfg.LastSNR0 > 0))
			{
#ifdef DOT11N_SS3_SUPPORT
				ulInfo = (pAd->ApCfg.LastSNR0 * 3 + 8) >> 4;
#else
				ulInfo = ((0xeb	- pAd->ApCfg.LastSNR0) * 3) >> 4 ;
#endif // DOT11N_SS3_SUPPORT//
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE, ("Query::RT_OID_802_11_SNR_0(0x=%lx)\n", ulInfo));
			}
            else
			    Status = -EFAULT;
			break;
		case RT_OID_802_11_SNR_1:
			if ((pAd->Antenna.field.RxPath	> 1) && 
                (pAd->ApCfg.LastSNR1 > 0))
			{
#ifdef DOT11N_SS3_SUPPORT
				ulInfo = (pAd->ApCfg.LastSNR1 * 3 + 8) >> 4;
#else
				ulInfo = ((0xeb	- pAd->ApCfg.LastSNR1) * 3) >> 4 ;
#endif // DOT11N_SS3_SUPPORT //
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_1(0x=%lx)\n",ulInfo));
			}
			else
				Status = -EFAULT;
            DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_1(pAd->ApCfg.LastSNR1=%d)\n",pAd->ApCfg.LastSNR1));
			break;
#ifdef DOT11N_SS3_SUPPORT
		case RT_OID_802_11_SNR_2:
			if ((pAd->Antenna.field.RxPath	> 2) && 
                (pAd->ApCfg.LastSNR2 > 0))
			{
				ulInfo = (pAd->ApCfg.LastSNR2 * 3 + 8) >> 4;
				wrq->u.data.length = sizeof(ulInfo);
				Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
				DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_2(0x=%lx)\n",ulInfo));
			}
			else
				Status = -EFAULT;
            DBGPRINT(RT_DEBUG_TRACE,("Query::RT_OID_802_11_SNR_2(pAd->ApCfg.LastSNR2=%d)\n",pAd->ApCfg.LastSNR2));
			break;
#endif // DOT11N_SS3_SUPPORT //
#endif // RTMP_RBUS_SUPPORT //

   		default:
			DBGPRINT(RT_DEBUG_TRACE, ("Query::unknown IOCTL's subcmd = 0x%08x, apidx=%d\n", cmd, apidx));
			Status = -EOPNOTSUPP;
			break;
    }

	return Status;
}


VOID RT28XX_IOCTL_MaxRateGet(
	IN	RTMP_ADAPTER			*pAd,
	IN	PHTTRANSMIT_SETTING		pHtPhyMode,
	OUT	UINT32					*pRate)
{
	int rate_index = 0;
	UINT32 ralinkrate[256] =
		{2,  4, 11, 22, 12, 18,   24,  36, 48, 72, 96, 108, 109, 110, 111, 112,
		13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,
		39, 78,  117, 156, 234, 312, 351, 390,
		27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
		81, 162, 243, 324, 486, 648, 729, 810,
		14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,
		43, 87,  130, 173, 260, 317, 390, 433,
		30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
		90, 180, 270, 360, 540, 720, 810, 900,
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
		20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
		40,41,42,43,44,45,46,47}; // 3*3

#ifdef DOT11_N_SUPPORT
	if (pHtPhyMode->field.MODE >= MODE_HTMIX)
	{
	//                	rate_index = 16 + ((UCHAR)pHtPhyMode->field.BW *16) + ((UCHAR)pHtPhyMode->field.ShortGI *32) + ((UCHAR)pHtPhyMode->field.MCS);
		rate_index = 16 + ((UCHAR)pHtPhyMode->field.BW *24) + ((UCHAR)pHtPhyMode->field.ShortGI *48) + ((UCHAR)pHtPhyMode->field.MCS);
	}
	else 
#endif // DOT11_N_SUPPORT //
		if (pHtPhyMode->field.MODE == MODE_OFDM)
			rate_index = (UCHAR)(pHtPhyMode->field.MCS) + 4;
		else 
			rate_index = (UCHAR)(pHtPhyMode->field.MCS);

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index > 255)
		rate_index = 255;
    
	*pRate = ralinkrate[rate_index] * 500000;
}


/* 
    ==========================================================================
    Description:
        Set Country Code.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{

	if(strlen(arg) == 2)
	{
		NdisMoveMemory(pAd->CommonCfg.CountryCode, arg, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
	}
	else
	{
		NdisZeroMemory(pAd->CommonCfg.CountryCode, 3);
		pAd->CommonCfg.bCountryFlag = FALSE;
	}	
		
	DBGPRINT(RT_DEBUG_TRACE, ("Set_CountryCode_Proc::(bCountryFlag=%d, CountryCode=%s)\n", pAd->CommonCfg.bCountryFlag, pAd->CommonCfg.CountryCode));

	return TRUE;
}

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ChGeography_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG Geography;
		
	Geography = simple_strtol(arg, 0, 10);
	if (Geography <= BOTH)
		pAd->CommonCfg.Geography = Geography;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ChannelGeography_Proc::(wrong setting. 0: Out-door, 1: in-door, 2: both)\n"));

	pAd->CommonCfg.CountryCode[2] =
		(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');

	DBGPRINT(RT_DEBUG_ERROR, ("Set_ChannelGeography_Proc:: Geography = %s\n", pAd->CommonCfg.Geography == ODOR ? "out-door" : (pAd->CommonCfg.Geography == IDOR ? "in-door" : "both")));
	
	// After Set ChGeography need invoke SSID change procedural again for Beacon update.
	// it's no longer necessary since APStartUp will rebuild channel again.
	//BuildChannelListEx(pAd);

	return TRUE;			
}
#endif // EXT_BUILD_CHANNEL_LIST //


/*
    ==========================================================================
    Description:
        Set Country String.
        This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryString_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT   index = 0;
	INT   success = TRUE;
	STRING  name_buffer[40] = {0};

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif // EXT_BUILD_CHANNEL_LIST //

	if(strlen(arg) <= 38)
	{
		if (strlen(arg) < 4)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryString_Proc::Parameter of CountryString are too short !\n"));
			return FALSE;
		}
		
		for (index = 0; index < strlen(arg); index++)
		{
			if ((arg[index] >= 'a') && (arg[index] <= 'z'))
				arg[index] = toupper(arg[index]);
		}

		for (index = 0; index < NUM_OF_COUNTRIES; index++)
		{
			NdisZeroMemory(name_buffer, 40);
			sprintf(name_buffer, "\"%s\"", (PSTRING) allCountry[index].pCountryName);

			if (strncmp((PSTRING) allCountry[index].pCountryName, arg, strlen(arg)) == 0)
				break;
			else if (strncmp(name_buffer, arg, strlen(arg)) == 0)
				break;
		}

		if (index == NUM_OF_COUNTRIES)
			success = FALSE;
	}
	else
	{
		success = FALSE;
	}			

	if (success == TRUE)
	{
		switch(pAd->CommonCfg.PhyMode)
		{
			case PHY_11BG_MIXED:	// 0
			case PHY_11B:			// 1
			case PHY_11G:			// 4
#ifdef DOT11_N_SUPPORT
			case PHY_11N_2_4G:		// 6
			case PHY_11GN_MIXED:	// 7
			case PHY_11BGN_MIXED:	// 9
#endif // DOT11_N_SUPPORT //
				if (pAd->CommonCfg.CountryRegionForABand & 0x80)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryString_Proc::parameter of CountryRegion in eeprom is programmed \n"));
					success = FALSE;
				}
				else
				{
					if (allCountry[index].SupportGBand == TRUE)
					{
						NdisZeroMemory(pAd->CommonCfg.CountryCode, 3);
						NdisMoveMemory(pAd->CommonCfg.CountryCode, allCountry[index].IsoName, 2);
						pAd->CommonCfg.CountryCode[2] = ' ';

						pAd->CommonCfg.bCountryFlag = TRUE;

						pAd->CommonCfg.CountryRegion = (UCHAR) allCountry[index].RegDomainNum11G;

						// After Set ChGeography need invoke SSID change procedural again for Beacon update.
						// it's no longer necessary since APStartUp will rebuild channel again.
						//BuildChannelList(pAd);

						success = TRUE;
					}
					else
					{
						success = FALSE;
						DBGPRINT(RT_DEBUG_TRACE, ("The Country are not Support G Band Channel\n"));
					}
				}

				break;
			case PHY_11A:			// 2
#ifdef DOT11_N_SUPPORT
			case PHY_11AN_MIXED:	// 8
			case PHY_11N_5G:		// 11
#endif // DOT11_N_SUPPORT //
				if (pAd->CommonCfg.CountryRegion & 0x80)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryString_Proc::parameter of CountryRegion in eeprom is programmed \n"));
					success = FALSE;
				}
				else
				{
					if (allCountry[index].SupportABand == TRUE)
					{
						NdisZeroMemory(pAd->CommonCfg.CountryCode, 3);
						NdisMoveMemory(pAd->CommonCfg.CountryCode, allCountry[index].IsoName, 2);
						pAd->CommonCfg.CountryCode[2] = ' ';

						pAd->CommonCfg.bCountryFlag = TRUE;

						pAd->CommonCfg.CountryRegionForABand = (UCHAR) allCountry[index].RegDomainNum11A;

						// After Set ChGeography need invoke SSID change procedural again for Beacon update.
						// it's no longer necessary since APStartUp will rebuild channel again.
						//BuildChannelList(pAd);

						success = TRUE;
					}
					else
					{
						success = FALSE;
						DBGPRINT(RT_DEBUG_TRACE, ("The Country are not Support A Band Channel\n"));
					}
				}
				break;

			default :
				success = FALSE;
				break;
		}
	}

	if (success == TRUE)
	{
		// if set country string, driver needs to be reset
		DBGPRINT(RT_DEBUG_TRACE, ("Set_CountryString_Proc::(CountryString=%s CountryRegin=%d CountryCode=%s)\n", 
							allCountry[index].pCountryName, pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryCode));
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_CountryString_Proc::Parameters out of range\n"));
	}

	return success;
}


/* 
    ==========================================================================
    Description:
        Set SSID
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_SSID_Proc(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			arg)
{
	INT   success = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAdapter->OS_Cookie;
	
	if(strlen(arg) <= MAX_LEN_OF_SSID)
	{
		NdisZeroMemory(pAdapter->ApCfg.MBSSID[pObj->ioctl_if].Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAdapter->ApCfg.MBSSID[pObj->ioctl_if].Ssid, arg, strlen(arg));
		pAdapter->ApCfg.MBSSID[pObj->ioctl_if].SsidLen = (UCHAR)strlen(arg);
		success = TRUE;

		// If in detection mode, need to stop detect first.
		if (pAdapter->CommonCfg.bIEEE80211H == FALSE)
		{
			APStop(pAdapter);
			APStartUp(pAdapter);
		}
		else
		{
			// each mode has different restart method
			if (pAdapter->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
			{
				APStop(pAdapter);
				APStartUp(pAdapter);
			}
			else if (pAdapter->CommonCfg.RadarDetect.RDMode == RD_SWITCHING_MODE)
			{
			}
			else if (pAdapter->CommonCfg.RadarDetect.RDMode == RD_NORMAL_MODE)
			{
				APStop(pAdapter);
				APStartUp(pAdapter);
				AsicEnableBssSync(pAdapter);
			}
		}
		DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) Set_SSID_Proc::(Len=%d,Ssid=%s)\n", pObj->ioctl_if,
			pAdapter->ApCfg.MBSSID[pObj->ioctl_if].SsidLen, pAdapter->ApCfg.MBSSID[pObj->ioctl_if].Ssid));
	}
	else
		success = FALSE;

	return success;
}


/* 
    ==========================================================================
    Description:
        Set TxRate
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_TxRate_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	NdisZeroMemory(pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredRates, MAX_LEN_OF_SUPPORTED_RATES);

	pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredRatesIndex = simple_strtol(arg, 0, 10);
	// todo RTMPBuildDesireRate(pAd, pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredRatesIndex);
	
	//todo MlmeUpdateTxRates(pAd);

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set BasicRate
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_BasicRate_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

    pAd->CommonCfg.BasicRateBitmap = (ULONG) simple_strtol(arg, 0, 10);

    if (pAd->CommonCfg.BasicRateBitmap > 4095) // (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1
        return FALSE;

    MlmeUpdateTxRates(pAd, FALSE, (UCHAR)pObj->ioctl_if);

    DBGPRINT(RT_DEBUG_TRACE, ("Set_BasicRate_Proc::(BasicRateBitmap=0x%08lx)\n", pAd->CommonCfg.BasicRateBitmap));
    
    return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set Beacon Period
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_BeaconPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT BeaconPeriod;
	INT   success = FALSE;

	BeaconPeriod = (USHORT) simple_strtol(arg, 0, 10);
	if((BeaconPeriod >= 20) && (BeaconPeriod < 1024))
	{
		pAd->CommonCfg.BeaconPeriod = BeaconPeriod;
		success = TRUE;

#ifdef AP_QLOAD_SUPPORT
		/* re-calculate QloadBusyTimeThreshold */
		QBSS_LoadAlarmReset(pAd);
#endif // AP_QLOAD_SUPPORT //
	}
	else
		success = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_BeaconPeriod_Proc::(BeaconPeriod=%d)\n", pAd->CommonCfg.BeaconPeriod));

	return success;
}

/* 
    ==========================================================================
    Description:
        Set Dtim Period
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_DtimPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT DtimPeriod;
	INT   success = FALSE;

	DtimPeriod = (USHORT) simple_strtol(arg, 0, 10);
	if((DtimPeriod >= 1) && (DtimPeriod <= 255))
	{
		pAd->ApCfg.DtimPeriod = DtimPeriod;
		success = TRUE;
	}
	else
		success = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_DtimPeriod_Proc::(DtimPeriod=%d)\n", pAd->ApCfg.DtimPeriod));

	return success;
}


#if defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
INT	Set_RadarShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{

#ifdef DFS_SUPPORT
#ifdef DFS_HARDWARE_SUPPORT
	int i;
	if (pAd->CommonCfg.dfs_func >= HARDWARE_DFS_V1)
	{
		printk("DFSUseTasklet = %d\n", pAd->CommonCfg.use_tasklet);
		printk("McuRadarDebug = %x\n", (unsigned int)pAd->CommonCfg.McuRadarDebug);
		printk("PollTime = %d\n", pAd->CommonCfg.PollTime);
		printk("ChEnable = %d\n", pAd->CommonCfg.ChEnable);
		printk("DeltaDelay = %d\n", pAd->CommonCfg.DeltaDelay);
		printk("Fcc5Thrd = %d\n", pAd->CommonCfg.fcc_5_threshold);
		printk("PeriodErr = %d\n", pAd->CommonCfg.dfs_period_err);
		printk("MaxPeriod = %d\n", (unsigned int)pAd->CommonCfg.dfs_max_period);
		printk("Ch0LErr = %d\n", pAd->CommonCfg.dfs_width_ch0_err_L);
		printk("Ch0HErr = %d\n", pAd->CommonCfg.dfs_width_ch0_err_H);
		printk("Ch1Shift = %d\n", pAd->CommonCfg.dfs_width_diff_ch1_Shift);
		printk("Ch2Shift = %d\n", pAd->CommonCfg.dfs_width_diff_ch2_Shift);
		printk("CeSwCheck = %d\n", pAd->CommonCfg.ce_sw_check);
		printk("HWDFSDisabled = %d\n", pAd->CommonCfg.hw_dfs_disabled);
		printk("WidthDiffShift = %d\n", pAd->CommonCfg.dfs_width_diff_Shift);
		printk("DfsRssiHigh = %d\n", pAd->CommonCfg.DfsRssiHigh);
		printk("DfsRssiLow = %d\n", pAd->CommonCfg.DfsRssiLow);
		
		printk("CheckLoop = %d\n", pAd->CommonCfg.dfs_check_loop);
		printk("DeclareThres = %d\n", pAd->CommonCfg.dfs_declare_thres);
		for (i =0; i < pAd->CommonCfg.fdf_num; i++)
		{
			printk("ChBusyThrd[%d] = %d\n", i, pAd->CommonCfg.ch_busy_threshold[i]);
			printk("RssiThrd[%d] = %d\n", i, pAd->CommonCfg.rssi_threshold[i]);
		}
		
		
		printk("sw_idx[0] = %u\n", pAd->CommonCfg.sw_idx[0]);
		printk("sw_idx[1] = %u\n", pAd->CommonCfg.sw_idx[1]);
		printk("sw_idx[2] = %u\n", pAd->CommonCfg.sw_idx[2]);
		printk("sw_idx[3] = %u\n", pAd->CommonCfg.sw_idx[3]);
		printk("hw_idx[0] = %u\n", pAd->CommonCfg.hw_idx[0]);
		printk("hw_idx[1] = %u\n", pAd->CommonCfg.hw_idx[1]);
		printk("hw_idx[2] = %u\n", pAd->CommonCfg.hw_idx[2]);
		printk("hw_idx[3] = %u\n", pAd->CommonCfg.hw_idx[3]);
#ifdef DFS_DEBUG
		printk("Total[0] = %lu\n", pAd->CommonCfg.TotalEntries[0]);
		printk("Total[1] = %lu\n", pAd->CommonCfg.TotalEntries[1]);
		printk("Total[2] = %lu\n", pAd->CommonCfg.TotalEntries[2]);
		printk("Total[3] = %lu\n", pAd->CommonCfg.TotalEntries[3]);

		pAd->CommonCfg.TotalEntries[0] = pAd->CommonCfg.TotalEntries[1] = pAd->CommonCfg.TotalEntries[2] = pAd->CommonCfg.TotalEntries[3] = 0;

		printk("T_Matched_2 = %lu\n", pAd->CommonCfg.T_Matched_2);
		printk("T_Matched_3 = %lu\n", pAd->CommonCfg.T_Matched_3);
		printk("T_Matched_4 = %lu\n", pAd->CommonCfg.T_Matched_4);
		printk("T_Matched_5 = %lu\n", pAd->CommonCfg.T_Matched_5);		
#endif // DFS_DEBUG //
	}
#endif // DFS_HARDWARE_SUPPORT //

#ifdef DFS_SOFTWARE_SUPPORT
	if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1)
	{
		printk("pAd->CommonCfg.McuRadarCmd = %x\n", pAd->CommonCfg.McuRadarCmd);
		printk("pAd->CommonCfg.McuRadarPeriod = %d\n", pAd->CommonCfg.McuRadarPeriod);
		printk("pAd->CommonCfg.McuRadarDetectPeriod = %d\n", pAd->CommonCfg.McuRadarDetectPeriod);
		printk("pAd->CommonCfg.McuRadarCtsProtect = %d\n", pAd->CommonCfg.McuRadarCtsProtect);
		printk("pAd->CommonCfg.RadarDetect.LongPulseRadarTh = %d\n", pAd->CommonCfg.RadarDetect.LongPulseRadarTh);
		printk("pAd->CommonCfg.RadarDetect.AvgRssiReq = %d\n", pAd->CommonCfg.RadarDetect.AvgRssiReq);
		printk("pAd->CommonCfg.RadarDetect.FixDfsLimit = %d\n", pAd->CommonCfg.RadarDetect.FixDfsLimit);
		printk("pAd->CommonCfg.RadarDetect.DfsUpperLimit = %ld\n", pAd->CommonCfg.RadarDetect.upperlimit);
		printk("pAd->CommonCfg.RadarDetect.DfsLowerLimit = %ld\n", pAd->CommonCfg.RadarDetect.lowerlimit);
	}
#endif // DFS_SOFTWARE_SUPPORT //
	printk("pAd->CommonCfg.RadarDetect.ChMovingTime = %d\n", pAd->CommonCfg.RadarDetect.ChMovingTime);
	printk("pAd->CommonCfg.RadarDetect.RDMode = %d\n", pAd->CommonCfg.RadarDetect.RDMode);
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
	printk("pAd->CommonCfg.McuCarrierPeriod = %d\n", pAd->CommonCfg.McuCarrierPeriod);
	printk("pAd->CommonCfg.McuCarrierDetectPeriod = %d\n", pAd->CommonCfg.McuCarrierDetectPeriod);
	printk("pAd->CommonCfg.McuCarrierCtsProtect = %d\n", pAd->CommonCfg.McuCarrierCtsProtect);

	printk("pAd->CommonCfg.CarrierDetect.CD_State = %d\n", pAd->CommonCfg.CarrierDetect.CD_State);
#ifdef TONE_RADAR_DETECT_SUPPORT
	printk("pAd->CommonCfg.CarrierDetect.criteria = %d\n", pAd->CommonCfg.CarrierDetect.criteria);
	printk("pAd->CommonCfg.CarrierDetect.Delta = %d\n", pAd->CommonCfg.CarrierDetect.delta);
	printk("pAd->CommonCfg.CarrierDetect.DivFlag = %d\n", pAd->CommonCfg.CarrierDetect.div_flag);
	printk("pAd->CommonCfg.CarrierDetect.Threshold = %d(0x%x)\n", pAd->CommonCfg.CarrierDetect.threshold, pAd->CommonCfg.CarrierDetect.threshold);
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //

	return TRUE;
}

#ifdef DFS_SUPPORT
/* 	0 = Switch Channel when Radar Hit (Normal mode) 
	1 = Don't Switch Channel when Radar Hit */
#ifdef DFS_HARDWARE_SUPPORT
INT	Set_RadarDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.McuRadarDebug = simple_strtol(arg, 0, 16);

	if (pAd->CommonCfg.McuRadarDebug & RADAR_DONT_SWITCH)
		printk("Dont Switch Channel\n");

	if (pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SILENCE)
		printk("Silence\n");

	if (pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_EVENT)
		printk("Show Long pulse Event\n");

	if (pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SW_SILENCE)
		printk("SW detection Silence\n");

	return TRUE;
}
#endif // DFS_HARDWARE_SUPPORT //

INT	Set_RadarHit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	if (simple_strtol(arg, 0, 10) == 1)
		pAd->CommonCfg.McuRadarDebug |= RADAR_SIMULATE;
	else if (simple_strtol(arg, 0, 10) == 2)
		pAd->CommonCfg.McuRadarDebug |= RADAR_SIMULATE2;
	return TRUE;
}


INT	Set_R65_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.R65 = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_R66_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.R66 = simple_strtol(arg, 0, 10);
	return TRUE;
}


INT	Set__R65_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg._R65 = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set__R66_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg._R66 = simple_strtol(arg, 0, 10);
	return TRUE;
}

extern UCHAR RdIdleTimeTable[MAX_RD_REGION][4];

INT	Set_DfsT_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.RadarDetect.DfsSessionTime = simple_strtol(arg, 0, 10);
	//pAd->CommonCfg.McuRadarDetectPeriod = simple_strtol(arg, 0, 10);
	RTMPPrepareRDCTSFrame(pAd, pAd->CurrentAddress,	(pAd->CommonCfg.McuRadarDetectPeriod * 1000 + 100), pAd->CommonCfg.RtsRate, HW_DFS_CTS_BASE, IFS_SIFS);
	return TRUE;
}


INT	Set_DfsC_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR i;

	i = (UCHAR) simple_strtol(arg, 0, 10);
	if (pAd->CommonCfg.RadarDetect.bFastDfs)
	{
		RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][0] = i;
	}
	else
	{
		RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][1] = i;
		RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][2] = i;
		RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][3] = i;
	}
	//pAd->CommonCfg.McuRadarPeriod = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsLowerLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.RadarDetect.DfsLowerLimit = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsUpperLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.RadarDetect.DfsUpperLimit = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_FixDfsLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.RadarDetect.FixDfsLimit = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_AvgRssiReq_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.RadarDetect.AvgRssiReq = simple_strtol(arg, 0, 10);
	return TRUE;
}


INT	Set_RadarGPIO_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	if (simple_strtol(arg, 0, 10) == 0)
		pAd->CommonCfg.McuRadarDebug &= ~RADAR_GPIO_DEBUG;
	else
		pAd->CommonCfg.McuRadarDebug |= RADAR_GPIO_DEBUG;
	return TRUE;
}

/* 0 = Don't Log, 1 = Start Log, 2 = Dump Log */
INT	Set_RadarLog_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef DFS_HARDWARE_SUPPORT
	{
		int k;
		UCHAR BBPR127;
		
		//BBP_IO_WRITE8_BY_REG_ID(pAd, 126, (pAd->CommonCfg.R126 & 0xfe));
		//DBGPRINT(RT_DEBUG_TRACE, ("Write R126 = %x\n", (pAd->CommonCfg.R126 & 0xfe)));
		DBGPRINT(RT_DEBUG_TRACE, ("R127 = \n"));
		for (k = 0; k < 384; k++)
		{
			BBPR127 = 0;
			BBP_IO_READ8_BY_REG_ID(pAd, 127, &BBPR127);
			printk ("%02x ", BBPR127);
			RTMPusecDelay(50);
			if ((k % 6) == 5)
				printk("  ");
			
			if ((k % 24) == 23)
				printk("\n");
		}
	}
#else
	int a = simple_strtol(arg, 0, 10);
	
	if (a == 0)
		pAd->CommonCfg.McuRadarDebug &= ~RADAR_LOG;
	else if (a == 1)
		pAd->CommonCfg.McuRadarDebug |= RADAR_LOG;
#ifdef RTMP_RBUS_SUPPORT
	else if (a == 2)
		DFS_Dump(pAd);
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_HARDWARE_SUPPORT //

	return TRUE;
}

#endif // DFS_SUPPORT //

#ifdef DFS_HARDWARE_SUPPORT
INT	Set_RadarStart_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	if (simple_strtol(arg, 0, 10) == 0)
	{
		NewRadarDetectionStart(pAd);
	}
	else if ((simple_strtol(arg, 0, 10) >= 1) && (simple_strtol(arg, 0, 10) <= 15))
	{
		pAd->CommonCfg.ChEnable = simple_strtol(arg, 0, 10);
		printk("Ch Enable == 0x%x\n", pAd->CommonCfg.ChEnable);
		NewRadarDetectionStart(pAd);
	}
	
	return TRUE;
}

INT	Set_RadarStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	NewRadarDetectionStop(pAd);
	return TRUE;
}


INT	Set_RadarSetTbl1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PUCHAR p2 = arg;
	ULONG idx, value;
	
	while((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Dec(idx, arg);
		A2Dec(value, p2+ 1);
		if (idx == 0)
		{
			pAd->CommonCfg.DeltaDelay = value;
			printk("Delta_Delay = %d\n", pAd->CommonCfg.DeltaDelay);
		}
		else
			modify_table1(pAd, idx, value);
		NewRadarDetectionStart(pAd);
	}
	else
		printk("please enter iwpriv ra0 set RadarT1=xxx:yyy\n");
	
	return TRUE;
}

INT	Set_RadarSetTbl2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PUCHAR p2 = arg;
	ULONG idx, value;
	
	while((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Dec(idx, arg);
		A2Dec(value, p2+ 1);
		modify_table2(pAd, idx, value);
	}
	else
		printk("please enter iwpriv ra0 set RadarT2=xxx:yyy\n");
	
	return TRUE;
}


INT	Set_Fcc5Thrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.fcc_5_threshold = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_ChBusyThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i;
	PUCHAR p1 = arg, p2;
	ULONG value;
	
	pAd->CommonCfg.fdf_num = 0;
	for (i = 0; i < MAX_FDF_NUMBER; i++)
	{
		p2 = p1;
		while((*p2 != ':') && (*p2 != '\0'))
		{
			p2++;
		}
	
		if (*p2 == ':')
		{
			if (*p1 == '-')
			{
				p1++;
				A2Dec(value, p1);
				value *= -1;
			}
			else
				A2Dec(value, p1);
			
			pAd->CommonCfg.ch_busy_threshold[i] = value;
			printk("pAd->CommonCfg.ch_busy_threshold[%d] = %d\n", i, pAd->CommonCfg.ch_busy_threshold[i]);
			pAd->CommonCfg.fdf_num++;
			p1 = p2 + 1;
		}
		else
		{
			pAd->CommonCfg.ch_busy_threshold[i] = simple_strtol(p1, 0, 10);
			printk("pAd->CommonCfg.ch_busy_threshold[%d] = %d\n", i, pAd->CommonCfg.ch_busy_threshold[i]);
			pAd->CommonCfg.fdf_num++;
			break;
		}
	}
	printk("pAd->CommonCfg.fdf_num = %d\n", pAd->CommonCfg.fdf_num);

	return TRUE;
}


INT	Set_RssiThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i;
	PUCHAR p1 = arg, p2;
	ULONG value;
	
	pAd->CommonCfg.fdf_num = 0;
	for (i = 0; i < MAX_FDF_NUMBER; i++)
	{
		p2 = p1;
		while((*p2 != ':') && (*p2 != '\0'))
		{
			p2++;
		}
	
		if (*p2 == ':')
		{
			if (*p1 == '-')
			{
				p1++;
				A2Dec(value, p1);
				value *= -1;
			}
			else
				A2Dec(value, p1);
			
			pAd->CommonCfg.rssi_threshold[i] = value;
			printk("pAd->CommonCfg.rssi_threshold[%d] = %d\n", i, pAd->CommonCfg.rssi_threshold[i]);
			pAd->CommonCfg.fdf_num++;
			p1 = p2 + 1;
		}
		else
		{
			pAd->CommonCfg.rssi_threshold[i] = simple_strtol(p1, 0, 10);
			printk("pAd->CommonCfg.rssi_threshold[%d] = %d\n", i, pAd->CommonCfg.rssi_threshold[i]);
			pAd->CommonCfg.fdf_num++;
			break;
		}
	}
	printk("pAd->CommonCfg.fdf_num = %d\n", pAd->CommonCfg.fdf_num);

	return TRUE;
	pAd->CommonCfg.rssi_threshold[0] = simple_strtol(arg, 0, 10);

	return TRUE;
}

INT	Set_PollTime_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.PollTime = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_WidthShift_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_width_diff_Shift = simple_strtol(arg, 0, 10);

	return TRUE;
}

INT	Set_Ch0LErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_width_ch0_err_L = simple_strtol(arg, 0, 10);
	return TRUE;
}


INT	Set_DeclareThres_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_declare_thres = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_CheckLoop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_check_loop = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_MaxPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_max_period = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_PeriodErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_period_err = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch0HErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_width_ch0_err_H = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch1Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_width_diff_ch1_Shift = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch2Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.dfs_width_diff_ch2_Shift = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_CEStagCheck_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.ce_staggered_check = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_HWDFSDisabled_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.hw_dfs_disabled = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_CeSwCheck_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->CommonCfg.ce_sw_check = simple_strtol(arg, 0, 10);
	return TRUE;
}

#ifdef DFS_SUPPORT
#ifdef DFS_DEBUG
INT	Set_CEPrintDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i;
	
	if (simple_strtol(arg, 0, 10) == 0)
	{
		pAd->CommonCfg.DebugPortPrint = 1;
	}
	
	if (simple_strtol(arg, 0, 10) == 1)
	{
		if (pAd->CommonCfg.DebugPortPrint == 3)
		{
			for (i = 0; i < 384; i++)
			{
				printk("%02x ", pAd->CommonCfg.DebugPort[i]);
				if (((i+1) % 18) == 0)
					printk("\n");
			}
			
			
			pAd->CommonCfg.DebugPortPrint = 0;
		}
		
	}
	return TRUE;
}
#endif // DFS_DEBUG //

INT	Set_RadarSim_Proc(
IN	PRTMP_ADAPTER	pAd,
IN	PSTRING			arg)
{
int i=0, id=0;
int dfs_data[] = {		208, 142172,
					160, 260514,
					208, 363873,
					160, 482216,
					208, 585575,
					164, 703918,
					208, 807277,
					160, 925620,
					208, 1028979,
					164, 1147321};

	pAd->CommonCfg.dfs_w_counter++;
	for (i = 0; i < 10; i++)
	{
		pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].counter = pAd->CommonCfg.dfs_w_counter;
		pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].timestamp = dfs_data[2*i+1];
		pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].width = dfs_data[2*i];
		pAd->CommonCfg.dfs_w_last_idx[id] = pAd->CommonCfg.dfs_w_idx[id];
		pAd->CommonCfg.dfs_w_idx[id]++;
		if (pAd->CommonCfg.dfs_w_idx[id] >= NEW_DFS_DBG_PORT_ENT_NUM)
			pAd->CommonCfg.dfs_w_idx[id] = 0;	
	}
	pAd->CommonCfg.hw_idx[0] = pAd->CommonCfg.dfs_w_idx[0];
	SWRadarCheck(pAd, 0);
	return TRUE;
}
INT	Set_PrintBusyIdle_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{

	pAd->CommonCfg.print_ch_busy_sta = simple_strtol(arg, 0, 10);
	return TRUE;
}


INT	Set_BusyIdleRatio_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.ch_busy_idle_ratio = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsRssiHigh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.DfsRssiHigh = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsRssiLow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.DfsRssiLow = simple_strtol(arg, 0, 10);
	return TRUE;
}


INT	Set_Ch0EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg) 
{
	 ULONG a = 0;
         PSTRING p;

	if ((arg[0] == '0') && (arg[1] == 'x'))
	{
		p = &arg[2];
	}
	else
		p = &arg[0];
		
	while (*p != '\0')
	{
		a = a * 0x10;
		if (*p >= 0x30 && *p <= 0x39)
			a += (ULONG)(*p - 0x30);
		else if (*p >= 0x41 && *p <= 0x46)
			a += (ULONG)(*p - 0x37);
		else if (*p >= 0x61 && *p <= 0x66)
			a += (ULONG)(*p - 0x57);
		else
		{
			printk("invalid hex value\n");
			return FALSE;
		}
		p++;
	}
	pAd->CommonCfg.RadarEventExpire[0] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 0);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));


		return TRUE;
}

INT	Set_Ch1EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg) 
{
	 ULONG a = 0;
         PSTRING p;

	if ((arg[0] == '0') && (arg[1] == 'x'))
	{
		p = &arg[2];
	}
	else
		p = &arg[0];
		
	while (*p != '\0')
	{
		a = a * 0x10;
		if (*p >= 0x30 && *p <= 0x39)
			a += (ULONG)(*p - 0x30);
		else if (*p >= 0x41 && *p <= 0x46)
			a += (ULONG)(*p - 0x37);
		else if (*p >= 0x61 && *p <= 0x66)
			a += (ULONG)(*p - 0x57);
		else
		{
			printk("invalid hex value\n");
			return FALSE;
		}
		p++;
	}
	pAd->CommonCfg.RadarEventExpire[1] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 1);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));


		return TRUE;
}

INT	Set_Ch2EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg) 
{
	 ULONG a = 0;
         PSTRING p;

	if ((arg[0] == '0') && (arg[1] == 'x'))
	{
		p = &arg[2];
	}
	else
		p = &arg[0];
		
	while (*p != '\0')
	{
		a = a * 0x10;
		if (*p >= 0x30 && *p <= 0x39)
			a += (ULONG)(*p - 0x30);
		else if (*p >= 0x41 && *p <= 0x46)
			a += (ULONG)(*p - 0x37);
		else if (*p >= 0x61 && *p <= 0x66)
			a += (ULONG)(*p - 0x57);
		else
		{
			printk("invalid hex value\n");
			return FALSE;
		}
		p++;
	}
	pAd->CommonCfg.RadarEventExpire[2] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 2);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));


		return TRUE;
}

INT	Set_Ch3EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg) 
{
	 ULONG a = 0;
         PSTRING p;

	if ((arg[0] == '0') && (arg[1] == 'x'))
	{
		p = &arg[2];
	}
	else
		p = &arg[0];
		
	while (*p != '\0')
	{
		a = a * 0x10;
		if (*p >= 0x30 && *p <= 0x39)
			a += (ULONG)(*p - 0x30);
		else if (*p >= 0x41 && *p <= 0x46)
			a += (ULONG)(*p - 0x37);
		else if (*p >= 0x61 && *p <= 0x66)
			a += (ULONG)(*p - 0x57);
		else
		{
			printk("invalid hex value\n");
			return FALSE;
		}
		p++;
	}
	pAd->CommonCfg.RadarEventExpire[3] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 3);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));


		return TRUE;
}
#endif // DFS_SUPPORT //

INT	Set_CEPrint_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i,j, id = simple_strtol(arg, 0, 10);
	int n = pAd->CommonCfg.dfs_w_last_idx[id];

	if ((id >= 0) && (id <= 3))
	{
		printk("Last Idx = %d\n", n);
		for (i = 0; i < NEW_DFS_DBG_PORT_ENT_NUM; i++)
		{
			printk("Cnt=%u, w= %u  Time=%u\n", (unsigned int)(pAd->CommonCfg.DFS_W[id][i].counter),
			pAd->CommonCfg.DFS_W[id][i].width,
			(unsigned int)pAd->CommonCfg.DFS_W[id][i].timestamp);
			if (pAd->CommonCfg.DFS_W[id][i].counter == 0)
				break;
		}
	}
		
	if ((id >= 10) && (id < 13))
	{
		id -= 10;
		for (i = 0; i < NEW_DFS_MPERIOD_ENT_NUM; i++)
		{
			printk("T=%u, w1= %u(%u), w2= %u(%u)\n", (unsigned int)(pAd->CommonCfg.DFS_T[id][i].period),
			pAd->CommonCfg.DFS_T[id][i].width, pAd->CommonCfg.DFS_T[id][i].idx,
			pAd->CommonCfg.DFS_T[id][i].width2, pAd->CommonCfg.DFS_T[id][i].idx2);
			if (pAd->CommonCfg.DFS_T[id][i].period == 0)
				break;
		}
		
	}
	if (id == 77)
	{
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < NEW_DFS_DBG_PORT_ENT_NUM; j++)
			{
				pAd->CommonCfg.DFS_W[i][j].counter = 0;
				pAd->CommonCfg.DFS_W[i][j].width = 0;
				pAd->CommonCfg.DFS_W[i][j].timestamp = 0;
			}
			for (j = 0; j < NEW_DFS_MPERIOD_ENT_NUM; j++)
			{
				pAd->CommonCfg.DFS_T[i][j].period = 0;	
				pAd->CommonCfg.DFS_T[i][j].width = 0;
				pAd->CommonCfg.DFS_T[i][j].width2 = 0;	
				pAd->CommonCfg.DFS_T[i][j].idx = 0;
				pAd->CommonCfg.DFS_T[i][j].idx2 = 0;
			}
		}
	}
#ifdef DFS_DEBUG	
	if (id > 20)
	{
		pAd->CommonCfg.BBP127Repeat = id - 20;
	}
	
	if (id == 5)
	{
		
		for (i = 0; i < NEW_DFS_DBG_PORT_ENT_NUM; i++)
		{
			printk("Cnt=%u, w= %u  Time=%lu,  start_idx = %u end_idx = %u\n", (unsigned int)(pAd->CommonCfg.CE_DebugCh0[i].counter),
			(unsigned int)pAd->CommonCfg.CE_DebugCh0[i].width, pAd->CommonCfg.CE_DebugCh0[i].timestamp,
			pAd->CommonCfg.CE_DebugCh0[i].start_idx, pAd->CommonCfg.CE_DebugCh0[i].end_idx);
		}

		for (i = 0; i < NEW_DFS_MPERIOD_ENT_NUM; i++)
		{
			printk("T=%lu, w1= %u(%u), w2= %u(%u)\n", (pAd->CommonCfg.CE_TCh0[i].period),
			pAd->CommonCfg.CE_TCh0[i].width, pAd->CommonCfg.CE_TCh0[i].idx,
			pAd->CommonCfg.CE_TCh0[i].width2, pAd->CommonCfg.CE_TCh0[i].idx2);
		}

	}
#endif // DFS_DEBUG //

	return TRUE;
}
#endif // DFS_HARDWARE_SUPPORT //
#endif // defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) //

INT Set_RfReg_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING          arg)
{

	printk("1 %x\n", pAd->LatchRfRegs.R1);
	printk("2 %x\n", pAd->LatchRfRegs.R2);
	printk("3 %x\n", pAd->LatchRfRegs.R3);
	printk("4 %x\n", pAd->LatchRfRegs.R4);
	return TRUE;
}
#endif // defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //


#ifdef TONE_RADAR_DETECT_SUPPORT
INT	Set_CarrierDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.Debug = 1;
	printk("pAd->CommonCfg.CarrierDetect.Debug = %ld\n", pAd->CommonCfg.CarrierDetect.Debug);
	return TRUE;
}

INT	Set_CarrierDelta_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.delta = simple_strtol(arg, 0, 10);
	printk("Delta = %d\n", pAd->CommonCfg.CarrierDetect.delta);
	NewCarrierDetectionStart(pAd);
	NewCarrierDetectionStart(pAd);

	return TRUE;
}

INT	Set_CarrierDivFlag_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.div_flag = simple_strtol(arg, 0, 10);
	printk("DivFlag = %d\n", pAd->CommonCfg.CarrierDetect.div_flag);
	NewCarrierDetectionStart(pAd);
	NewCarrierDetectionStart(pAd);

	return TRUE;
}

INT	Set_CarrierThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.CarrierDetect.threshold = simple_strtol(arg, 0, 10);
	printk("CarrThrd = %d(0x%x)\n", pAd->CommonCfg.CarrierDetect.threshold, pAd->CommonCfg.CarrierDetect.threshold);
	NewCarrierDetectionStart(pAd);
	NewCarrierDetectionStart(pAd);

	return TRUE;
}
#endif // TONE_RADAR_DETECT_SUPPORT //

#ifdef RT305x
INT Set_RfRead_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
	int i;
	UCHAR Value;
	
	for (i = 0; i < 32; i++)
	{
		RT30xxReadRFRegister(pAdapter, i, &Value);
		printk("%02x ", Value);
		if (((i + 1) % 4) == 0)
			printk("\n");
	}
	return TRUE;
}

INT Set_RfWrite_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg)
{
	ULONG offset = 0;
	ULONG value = 0;
	PUCHAR p2 = arg;
	
	while((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}
	
	if (*p2 == ':')
	{
		A2Hex(offset, arg);
		A2Hex(value, p2+ 1);
	}
	else
	{
		A2Hex(value, arg);
	}
	
	if (offset >= 32)
	{
		return FALSE;
	}
	
	RT30xxWriteRFRegister(pAdapter, offset, value);

	return TRUE;
}
#endif // RT305x //


/* 
    ==========================================================================
    Description:
        Disable/enable OLBC detection manually
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_OLBCDetection_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	switch (simple_strtol(arg, 0, 10))
	{
		case 0: //enable OLBC detect
			pAd->CommonCfg.DisableOLBCDetect = 0;
			break;
		case 1: //disable OLBC detect
			pAd->CommonCfg.DisableOLBCDetect = 1;
			break;
		default:  //Invalid argument 
			return FALSE;
	}

	return TRUE;
}


#ifdef WMM_SUPPORT
/* 
    ==========================================================================
    Description:
        Set WmmCapable Enable or Disable
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN	bWmmCapable;
	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	bWmmCapable = simple_strtol(arg, 0, 10);

	if (bWmmCapable == 1)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].bWmmCapable = TRUE;
	else if (bWmmCapable == 0)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].bWmmCapable = FALSE;
	else
		return FALSE;  //Invalid argument 

	pAd->ApCfg.MBSSID[pObj->ioctl_if].bWmmCapableOrg = \
								pAd->ApCfg.MBSSID[pObj->ioctl_if].bWmmCapable;
	
#ifdef RTL865X_FAST_PATH	
	if (!isFastPathCapable(pAd)) {
		rtlairgo_fast_tx_unregister();
		rtl865x_extDev_unregisterUcastTxDev(pAd->net_dev);		
	}
#endif

#ifdef DOT11_N_SUPPORT
	//Sync with the HT relate info. In N mode, we should re-enable it
	SetCommonHT(pAd);
#endif // DOT11_N_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WmmCapable_Proc::(bWmmCapable=%d)\n", 
		pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].bWmmCapable));

	return TRUE;
}
#endif /* WMM_SUPPORT */


INT	Set_AP_MaxStaNum_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT			apidx = pObj->ioctl_if;

	return ApCfg_Set_MaxStaNum_Proc(pAd, apidx, arg);
}

/* 
    ==========================================================================
    Description:
        Set session idle timeout
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_IdleTimeout_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	return ApCfg_Set_IdleTimeout_Proc(pAd, arg);
}
/* 
    ==========================================================================
    Description:
        Set No Forwarding Enable or Disable
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_NoForwarding_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG NoForwarding;

	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	NoForwarding = simple_strtol(arg, 0, 10);

	if (NoForwarding == 1)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].IsolateInterStaTraffic = TRUE;
	else if (NoForwarding == 0)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].IsolateInterStaTraffic = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_NoForwarding_Proc::(NoForwarding=%ld)\n", 
		pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].IsolateInterStaTraffic));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set No Forwarding between each SSID
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_NoForwardingBTNSSID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG NoForwarding;

	NoForwarding = simple_strtol(arg, 0, 10);

	if (NoForwarding == 1)
		pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
	else if (NoForwarding == 0)
		pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;
	else
		return FALSE;  //Invalid argument

	DBGPRINT(RT_DEBUG_TRACE, ("Set_NoForwardingBTNSSID_Proc::(NoForwarding=%ld)\n", pAd->ApCfg.IsolateInterStaTrafficBTNBSSID));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set Hide SSID Enable or Disable
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HideSSID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN bHideSsid;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	bHideSsid = simple_strtol(arg, 0, 10);

	if (bHideSsid == 1)
		bHideSsid = TRUE;
	else if (bHideSsid == 0)
		bHideSsid = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	if (pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid != bHideSsid)
	{
		pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid = bHideSsid;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_HideSSID_Proc::(HideSSID=%d)\n", pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].bHideSsid));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set channel switch Period
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	pAd->CommonCfg.RadarDetect.CSPeriod = (USHORT) simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->CommonCfg.RadarDetect.CSPeriod));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set VLAN's ID field
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLANID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	pAd->ApCfg.MBSSID[pObj->ioctl_if].VLAN_VID = simple_strtol(arg, 0, 10);
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_VLANID_Proc::(VLAN_VID=%d)\n", pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].VLAN_VID));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set VLAN's priority field
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLANPriority_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	pAd->ApCfg.MBSSID[pObj->ioctl_if].VLAN_Priority = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_VLANPriority_Proc::(VLAN_Priority=%d)\n", pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].VLAN_Priority));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set enable or disable carry VLAN in the air
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLAN_TAG_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN	bVLAN_Tag;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	bVLAN_Tag = simple_strtol(arg, 0, 10);

	if (bVLAN_Tag == 1)
		bVLAN_Tag = TRUE;
	else if (bVLAN_Tag == 0)
		bVLAN_Tag = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	if (pAd->ApCfg.MBSSID[pObj->ioctl_if].bVLAN_Tag != bVLAN_Tag)
	{
		pAd->ApCfg.MBSSID[pObj->ioctl_if].bVLAN_Tag = bVLAN_Tag;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_VLAN_TAG_Proc::(VLAN_Tag=%d)\n", pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].bVLAN_Tag));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set Authentication mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_AuthMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG       i;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;  

	/* Set Authentication mode */
	ApCfg_Set_AuthMode_Proc(pAd, apidx, arg);

	/* reset the portSecure for all entries */
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]))
		{
			pAd->MacTab.Content[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		}
	}

	/* reset the PortSecure this BSS */
	pAd->ApCfg.MBSSID[apidx].PortSecured = WPA_802_1X_PORT_NOT_SECURED;

	/* Default key index is always 2 in WPA mode */	
	if(pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA)
		pAd->ApCfg.MBSSID[apidx].DefaultKeyId = 1;

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set Encryption Type
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_EncrypType_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	
	if ((strcmp(arg, "NONE") == 0) || (strcmp(arg, "none") == 0))
		pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11WEPDisabled;
	else if ((strcmp(arg, "WEP") == 0) || (strcmp(arg, "wep") == 0))
		pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11WEPEnabled;
	else if ((strcmp(arg, "TKIP") == 0) || (strcmp(arg, "tkip") == 0))
		pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11Encryption2Enabled;
	else if ((strcmp(arg, "AES") == 0) || (strcmp(arg, "aes") == 0))
		pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11Encryption3Enabled;
	else if ((strcmp(arg, "TKIPAES") == 0) || (strcmp(arg, "tkipaes") == 0))
		pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11Encryption4Enabled;
#ifdef WAPI_SUPPORT
	else if ((strcmp(arg, "SMS4") == 0) || (strcmp(arg, "sms4") == 0))
		pAd->ApCfg.MBSSID[apidx].WepStatus = Ndis802_11EncryptionSMS4Enabled;
#endif // WAPI_SUPPORT //		
	else
		return FALSE;

	if (pAd->ApCfg.MBSSID[apidx].WepStatus >= Ndis802_11Encryption2Enabled)
		pAd->ApCfg.MBSSID[apidx].DefaultKeyId = 1;

	// decide the group key encryption type
	if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption4Enabled)	
		pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus = Ndis802_11Encryption2Enabled;		
	else
		pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus = pAd->ApCfg.MBSSID[apidx].WepStatus;

	// move to ap.c::APStartUp to process
    //RTMPMakeRSNIE(pAd, pAd->ApCfg.MBSSID[apidx].AuthMode, pAd->ApCfg.MBSSID[apidx].WepStatus, apidx);
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_EncrypType_Proc::(EncrypType=%d)\n", apidx, pAd->ApCfg.MBSSID[apidx].WepStatus));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set WPA pairwise mix-cipher combination
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_WpaMixPairCipher_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;

	/*
		In WPA-WPA2 mix mode, it provides a more flexible cipher combination. 
		-	WPA-AES and WPA2-TKIP
		-	WPA-AES and WPA2-TKIPAES
		-	WPA-TKIP and WPA2-AES
		-	WPA-TKIP and WPA2-TKIPAES
		-	WPA-TKIPAES and WPA2-AES
		-	WPA-TKIPAES and WPA2-TKIP
		-	WPA-TKIPAES and WPA2-TKIPAES (default)																 																	
	 */									
	if ((strncmp(arg, "WPA_AES_WPA2_TKIPAES", 20) == 0) || (strncmp(arg, "wpa_aes_wpa2_tkipaes", 20) == 0))
		pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = WPA_AES_WPA2_TKIPAES;																			
	else if ((strncmp(arg, "WPA_AES_WPA2_TKIP", 17) == 0) || (strncmp(arg, "wpa_aes_wpa2_tkip", 17) == 0))
		pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = WPA_AES_WPA2_TKIP;								 						
	else if ((strncmp(arg, "WPA_TKIP_WPA2_AES", 17) == 0) || (strncmp(arg, "wpa_tkip_wpa2_aes", 17) == 0))
		pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = WPA_TKIP_WPA2_AES;								
	else if ((strncmp(arg, "WPA_TKIP_WPA2_TKIPAES", 21) == 0) || (strncmp(arg, "wpa_tkip_wpa2_tkipaes", 21) == 0))
		pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = WPA_TKIP_WPA2_TKIPAES;
	else if ((strncmp(arg, "WPA_TKIPAES_WPA2_AES", 20) == 0) || (strncmp(arg, "wpa_tkipaes_wpa2_aes", 20) == 0))
		pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = WPA_TKIPAES_WPA2_AES;
	else if ((strncmp(arg, "WPA_TKIPAES_WPA2_TKIPAES", 24) == 0) || (strncmp(arg, "wpa_tkipaes_wpa2_tkipaes", 24) == 0))
		pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;
	else if ((strncmp(arg, "WPA_TKIPAES_WPA2_TKIP", 21) == 0) || (strncmp(arg, "wpa_tkipaes_wpa2_tkip", 21) == 0))
		pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIP;
	else
		return FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) Set_AP_WpaMixPairCipher_Proc=0x%02x\n", apidx, pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher));
	
	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set WPA rekey interval value
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_RekeyInterval_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	INT32	val;

	val = simple_strtol(arg, 0, 10);

	if((val >= 10) && (val < MAX_REKEY_INTER))
		pAd->ApCfg.MBSSID[apidx].WPAREKEY.ReKeyInterval = val;
	else //Default
		pAd->ApCfg.MBSSID[apidx].WPAREKEY.ReKeyInterval = 3600;

	DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) Set_AP_RekeyInterval_Proc=%ld\n", 
								apidx, pAd->ApCfg.MBSSID[apidx].WPAREKEY.ReKeyInterval));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set WPA rekey method
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_RekeyMethod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	PRT_WPA_REKEY	pInfo = &pAd->ApCfg.MBSSID[apidx].WPAREKEY;
	
	if ((strcmp(arg, "TIME") == 0) || (strcmp(arg, "time") == 0))
		pInfo->ReKeyMethod = TIME_REKEY;
	else if ((strcmp(arg, "PKT") == 0) || (strcmp(arg, "pkt") == 0))
		pInfo->ReKeyMethod = PKT_REKEY;
	else if ((strcmp(arg, "DISABLE") == 0) || (strcmp(arg, "disable") == 0))
		pInfo->ReKeyMethod = DISABLE_REKEY;
	else
		pInfo->ReKeyMethod = DISABLE_REKEY;

	DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) Set_AP_RekeyMethod_Proc=%ld\n", 
								apidx, pInfo->ReKeyMethod));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set PMK-cache period
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_PMKCachePeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	UINT32 val = simple_strtol(arg, 0, 10);

	pAd->ApCfg.MBSSID[apidx].PMKCachePeriod = val * 60 * OS_HZ;

	DBGPRINT(RT_DEBUG_TRACE, ("I/F(ra%d) Set_AP_PMKCachePeriod_Proc=%ld\n", 
									apidx, pAd->ApCfg.MBSSID[apidx].PMKCachePeriod));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Default Key ID
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_DefaultKeyID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG KeyIdx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	apidx = pObj->ioctl_if;


	KeyIdx = simple_strtol(arg, 0, 10);
	if((KeyIdx >= 1 ) && (KeyIdx <= 4))
		pAd->ApCfg.MBSSID[apidx].DefaultKeyId = (UCHAR) (KeyIdx - 1 );
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_DefaultKeyID_Proc::(DefaultKeyID(0~3)=%d)\n", apidx, pAd->ApCfg.MBSSID[apidx].DefaultKeyId));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY1
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_Key1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj;
	UCHAR	apidx;
	CIPHER_KEY	*pSharedKey;
	INT		retVal;		

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	pSharedKey = &pAd->SharedKey[apidx][0];
	retVal = RT_CfgSetWepKey(pAd, arg, pSharedKey, 0);
	if (retVal == TRUE)
	{		
		// Set keys (into ASIC)
		if (pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA)
			;   // not support
		else    // Old WEP stuff
		{
			AsicAddSharedKeyEntry(pAd, apidx, 0, pSharedKey);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_Key1_Proc::(Key1=%s) success!\n", apidx, arg));
	}
	
	return retVal;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY2
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_Key2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj;
	UCHAR	apidx;
	CIPHER_KEY	*pSharedKey;
	INT		retVal;	

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	pSharedKey = &pAd->SharedKey[apidx][1];
	retVal = RT_CfgSetWepKey(pAd, arg, pSharedKey, 1);
	if (retVal == TRUE)
	{		
		// Set keys (into ASIC)
		if (pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA)
			;   // not support
		else    // Old WEP stuff
		{
			AsicAddSharedKeyEntry(pAd, apidx, 1, pSharedKey);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_Key2_Proc::(Key2=%s) success!\n", apidx, arg));
	}

	return retVal;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY3
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_Key3_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj;
	UCHAR	apidx;
	CIPHER_KEY	*pSharedKey;
	INT		retVal;	

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	pSharedKey = &pAd->SharedKey[apidx][2];
	retVal = RT_CfgSetWepKey(pAd, arg, pSharedKey, 2);
	if (retVal == TRUE)
	{		
		// Set keys (into ASIC)
		if (pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA)
			;   // not support
		else    // Old WEP stuff
		{
			AsicAddSharedKeyEntry(pAd, apidx, 2, pSharedKey);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_Key3_Proc::(Key3=%s) success!\n", apidx, arg));
	}

	return retVal;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY4
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_Key4_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{

	POS_COOKIE pObj;
	UCHAR	apidx;
	CIPHER_KEY	*pSharedKey;
	INT		retVal;	

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	pSharedKey = &pAd->SharedKey[apidx][3];
	retVal = RT_CfgSetWepKey(pAd, arg, pSharedKey, 3);
	if (retVal == TRUE)
	{		
		// Set keys (into ASIC)
		if (pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA)
			;   // not support
		else    // Old WEP stuff
		{
			AsicAddSharedKeyEntry(pAd, apidx, 3, pSharedKey);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_Key4_Proc::(Key4=%s) success!\n", apidx, arg));
	}

	return retVal;
}


/* 
    ==========================================================================
    Description:
        Set Access ctrol policy
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AccessPolicy_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	switch (simple_strtol(arg, 0, 10))
	{
		case 0: //Disable
			pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = 0;
			break;
		case 1: // Allow All, and ACL is positive.
			pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = 1;
			break;
		case 2: // Reject All, and ACL is negative.
			pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = 2;
			break;
		default: //Invalid argument 
			DBGPRINT(RT_DEBUG_ERROR, ("Set_AccessPolicy_Proc::Invalid argument (=%s)\n", arg));		
			return FALSE;
	}

	// check if the change in ACL affects any existent association
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_AccessPolicy_Proc::(AccessPolicy=%ld)\n", pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy));

	return TRUE;	
}


/* Replaced by Set_ACLAddEntry_Proc() and Set_ACLClearAll_Proc() */

/* 
    ==========================================================================
    Description:
        Add one entry or several entries(if allowed to)
        	into Access control mac table list
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLAddEntry_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	RT_802_11_ACL			acl;
	PSTRING					this_char;
	PSTRING					value;
	INT						i, j;
	BOOLEAN					isDuplicate=FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	NdisZeroMemory(&acl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&acl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	
	while ((this_char = strsep((char **)&arg, ";")) != NULL)
	{
		if (*this_char == '\0')
		{
			DBGPRINT(RT_DEBUG_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}
		if (strlen(this_char) != 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
		{
			DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address length!\n"));
			continue;
		}
        for (i=0, value = rstrtok(this_char,":"); value; value = rstrtok(NULL,":")) 
		{
			if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			{
				DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address format or octet!\n"));
				/* Do not use "continue" to replace "break" */
				break;
			}
			AtoH(value, &macAddr[i++], 1);
		}

		if (i != MAC_ADDR_LEN)
		{
			continue;
		}

		// Check if this entry is duplicate.
		isDuplicate = FALSE;
		for (j=0; j<acl.Num; j++)
		{
			if (memcmp(acl.Entry[j].Addr, &macAddr, 6) == 0)
			{
				isDuplicate = TRUE;
				DBGPRINT(RT_DEBUG_WARN, ("You have added an entry before :\n"));
	        	DBGPRINT(RT_DEBUG_WARN, ("The duplicate entry is %02x:%02x:%02x:%02x:%02x:%02x\n",
	        		macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]));
			}
		}

		if (!isDuplicate)
		{
			NdisMoveMemory(acl.Entry[acl.Num++].Addr, &macAddr, MAC_ADDR_LEN);
		}

		if (acl.Num == MAX_NUM_OF_ACL_LIST)
	    {
			DBGPRINT(RT_DEBUG_WARN, ("The AccessControlList is full, and no more entry can join the list!\n"));
        	DBGPRINT(RT_DEBUG_WARN, ("The last entry of ACL is %02x:%02x:%02x:%02x:%02x:%02x\n",
        		macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]));
			break;
		}
	}
	
	ASSERT(acl.Num <= MAX_NUM_OF_ACL_LIST);

	NdisZeroMemory(&pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, &acl, sizeof(RT_802_11_ACL));

	// check if the change in ACL affects any existent association
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);
	DBGPRINT(RT_DEBUG_TRACE, ("Set::%s(Policy=%ld, Entry#=%ld)\n",
		__FUNCTION__ , pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num));

#ifdef DBG
	DBGPRINT(RT_DEBUG_TRACE, ("=============== Entry ===============\n"));
	for (i=0; i<pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num; i++)
	{
		printk("Entry #%02d: ", i+1);
		for (j=0; j<MAC_ADDR_LEN; j++)
		   printk("%02X ", pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Entry[i].Addr[j]);
		printk("\n");
	}
#endif
	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Delete one entry or several entries(if allowed to)
        	from Access control mac table list
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLDelEntry_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	UCHAR					nullAddr[MAC_ADDR_LEN];
	RT_802_11_ACL			acl;
	PSTRING					this_char;
	PSTRING					value;
	INT						i, j;
	BOOLEAN					isFound=FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	NdisZeroMemory(&acl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&acl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	NdisZeroMemory(nullAddr, MAC_ADDR_LEN);
	
	while ((this_char = strsep((char **)&arg, ";")) != NULL)
	{
		if (*this_char == '\0')
		{
			DBGPRINT(RT_DEBUG_WARN, ("An unnecessary delimiter entered!\n"));
			continue;
		}
		if (strlen(this_char) != 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
		{
			DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address length!\n"));
			continue;
		}

		for (i=0, value = rstrtok(this_char,":"); value; value = rstrtok(NULL,":")) 
		{
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			{
				DBGPRINT(RT_DEBUG_ERROR, ("illegal MAC address format or octet!\n"));
				/* Do not use "continue" to replace "break" */
				break;
			}
			AtoH(value, &macAddr[i++], 1);
		}

		if (i != MAC_ADDR_LEN)
		{
			continue;
		}

		// Check if this entry existed.
		isFound = FALSE;
		for (j=0; j<acl.Num; j++)
		{
			if (memcmp(acl.Entry[j].Addr, &macAddr, MAC_ADDR_LEN) == 0)
			{
				isFound = TRUE;
				NdisZeroMemory(acl.Entry[j].Addr, MAC_ADDR_LEN);
				DBGPRINT(RT_DEBUG_TRACE, ("The entry %02x:%02x:%02x:%02x:%02x:%02x founded will be deleted!\n",
	        		macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]));
			}
		}

		if (!isFound)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("The entry %02x:%02x:%02x:%02x:%02x:%02x is not in the list!\n",
        		macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]));
		}
	}
	
	NdisZeroMemory(&pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy = acl.Policy;
	ASSERT(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num == 0);
	i = 0;

	for (j=0; j<acl.Num; j++)
	{
		if (memcmp(acl.Entry[j].Addr, &nullAddr, MAC_ADDR_LEN) == 0)
		{
			continue;			
		}
		else
		{
			NdisMoveMemory(&(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Entry[i++]), acl.Entry[j].Addr, MAC_ADDR_LEN);
		}
	}

	pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num = i;
	ASSERT(acl.Num >= pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num);

	// check if the change in ACL affects any existent association
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);
	DBGPRINT(RT_DEBUG_TRACE, ("Set::%s(Policy=%ld, Entry#=%ld)\n",
		__FUNCTION__ , pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num));

#ifdef DBG
	DBGPRINT(RT_DEBUG_TRACE, ("=============== Entry ===============\n"));
	for (i=0; i<pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num; i++)
	{
		printk("Entry #%02d: ", i+1);
		for (j=0; j<MAC_ADDR_LEN; j++)
		   printk("%02X ", pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Entry[i].Addr[j]);
		printk("\n");
	}
#endif
	return TRUE;
}


// for ACL policy message
#define ACL_POLICY_TYPE_NUM	3
char const *pACL_PolicyMessage[ACL_POLICY_TYPE_NUM] = {   	
	"the Access Control feature is disabled",						/* 0 : Disable */
	"only the following entries are allowed to join this BSS",			/* 1 : Allow */
	"all the following entries are rejected to join this BSS",			/* 2 : Reject */
};


/* 
    ==========================================================================
    Description:
        Dump all the entries in the Access control 
        	mac table list of a specified BSS
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLShowAll_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	RT_802_11_ACL			acl;
	BOOLEAN					bDumpAll=FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT						i, j;
	
	bDumpAll = simple_strtol(arg, 0, 10);

	if (bDumpAll == 1)
	{
		bDumpAll = TRUE;
	}
	else if (bDumpAll == 0)
	{
		bDumpAll = FALSE;
		DBGPRINT(RT_DEBUG_WARN, ("Your input is 0!\n"));
		DBGPRINT(RT_DEBUG_WARN, ("The Access Control List will not be dumped!\n"));
		return TRUE;
	}
	else
	{
		return FALSE;  // Invalid argument
	}

	NdisZeroMemory(&acl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&acl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	
	/* Check if the list is already empty. */
	if (acl.Num == 0)
	{
		DBGPRINT(RT_DEBUG_WARN, ("The Access Control List is empty!\n"));
		return TRUE;
	}

	ASSERT(((bDumpAll == 1) && (acl.Num > 0)));

	/* Show the corresponding policy first. */
	printk("=============== Access Control Policy ===============\n");
	printk("Policy is %ld : ", acl.Policy);
	printk("%s\n", pACL_PolicyMessage[acl.Policy]);

	/* Dump the entry in the list one by one */
	printk("===============  Access Control List  ===============\n");
	for (i=0; i<acl.Num; i++)
	{
		printk("Entry #%02d: ", i+1);
		for (j=0; j<MAC_ADDR_LEN; j++)
		   printk("%02X ", acl.Entry[i].Addr[j]);
		printk("\n");
	}
	
	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Clear all the entries in the Access control 
        	mac table list of a specified BSS
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLClearAll_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	RT_802_11_ACL			acl;
	BOOLEAN					bClearAll=FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	bClearAll = simple_strtol(arg, 0, 10);

	if (bClearAll == 1)
	{
		bClearAll = TRUE;
	}
	else if (bClearAll == 0)
	{
		bClearAll = FALSE;
		DBGPRINT(RT_DEBUG_WARN, ("Your input is 0!\n"));
		DBGPRINT(RT_DEBUG_WARN, ("The Access Control List will be kept unchanged!\n"));
		return TRUE;
	}
	else
	{
		return FALSE;  // Invalid argument
	}

	NdisZeroMemory(&acl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&acl, &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList, sizeof(RT_802_11_ACL));
	
	/* Check if the list is already empty. */
	if (acl.Num == 0)
	{
		DBGPRINT(RT_DEBUG_WARN, ("The Access Control List is empty!\n"));
		DBGPRINT(RT_DEBUG_WARN, ("No need to clear the Access Control List!\n"));
		return TRUE;
	}

	ASSERT(((bClearAll == 1) && (acl.Num > 0)));

	/* Clear the entry in the list one by one */
	/* Keep the corresponding policy unchanged. */
	do
	{
		NdisZeroMemory(acl.Entry[acl.Num - 1].Addr, MAC_ADDR_LEN);
		acl.Num -= 1;
	}while (acl.Num > 0);
	
	ASSERT(acl.Num == 0);

	NdisZeroMemory(&(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList), sizeof(RT_802_11_ACL));
	NdisMoveMemory(&(pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList), &acl, sizeof(RT_802_11_ACL));

	// check if the change in ACL affects any existent association
	ApUpdateAccessControlList(pAd, pObj->ioctl_if);
	DBGPRINT(RT_DEBUG_TRACE, ("Set::%s(Policy=%ld, Entry#=%ld)\n",
		__FUNCTION__, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Policy, pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList.Num));

	return TRUE;
}

#ifdef DBG
static void _rtmp_hexdump(int level, const char *title, const UINT8 *buf,
			 size_t len, int show)
{
	size_t i;
	if (level < RTDebugLevel)
		return;
	printk("%s - hexdump(len=%lu):", title, (unsigned long) len);
	if (show) {
		for (i = 0; i < len; i++)
			printk(" %02x", buf[i]);
	} else {
		printk(" [REMOVED]");
	}
	printk("\n");
}

void rtmp_hexdump(int level, const char *title, const UINT8 *buf, size_t len)
{
	_rtmp_hexdump(level, title, buf, len, 1);
}
#endif



/* 
    ==========================================================================
    Description:
        Set WPA PSK key

    Arguments:
        pAdapter            Pointer to our adapter
        arg                 WPA pre-shared key string

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_WPAPSK_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	apidx = pObj->ioctl_if;
	INT	retval;
	MULTISSID_STRUCT *pMBSSStruct;
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_WPAPSK_Proc::(WPAPSK=%s)\n", arg));

	pMBSSStruct = &pAd->ApCfg.MBSSID[apidx];
	retval = RT_CfgSetWPAPSKKey(pAd, arg, (PUCHAR)pMBSSStruct->Ssid, pMBSSStruct->SsidLen, pMBSSStruct->PMK);
	if (retval == FALSE)
		return FALSE;

#ifdef WSC_AP_SUPPORT
    NdisZeroMemory(pMBSSStruct->WscControl.WpaPsk, 64);
    pMBSSStruct->WscControl.WpaPskLen = 0;
    pMBSSStruct->WscControl.WpaPskLen = strlen(arg);
    NdisMoveMemory(pMBSSStruct->WscControl.WpaPsk, arg, pMBSSStruct->WscControl.WpaPskLen);    
#endif // WSC_AP_SUPPORT //    

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Reset statistics counter

    Arguments:
        pAdapter            Pointer to our adapter
        arg                 

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR radio;

	radio = simple_strtol(arg, 0, 10);

	if (radio)
	{
		MlmeRadioOn(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("==>Set_RadioOn_Proc (ON)\n"));
	}
	else
	{
		MlmeRadioOff(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("==>Set_RadioOn_Proc (OFF)\n"));
	}

	return TRUE;
}

#ifdef AP_SCAN_SUPPORT
/* 
    ==========================================================================
    Description:
        Issue a site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set site_survey
    ==========================================================================
*/
INT Set_SiteSurvey_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	NDIS_802_11_SSID Ssid;
    NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	if ((strlen(arg) != 0) && (strlen(arg) <= MAX_LEN_OF_SSID))
    {
        NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
        Ssid.SsidLength = strlen(arg);
	}

	if (Ssid.SsidLength == 0)
		ApSiteSurvey(pAd, &Ssid, SCAN_PASSIVE, FALSE);
	else
		ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE);
    
    DBGPRINT(RT_DEBUG_TRACE, ("Set_SiteSurvey_Proc\n"));

    return TRUE;
}

/* 
    ==========================================================================
    Description:
        Issue a Auto-Channel Selection command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None
    
    Note:
        Usage: 
               1.) iwpriv ra0 set AutoChannelSel=1
                   Ues the number of AP to choose
               2.) iwpriv ra0 set AutoChannelSel=2
                   Ues the False CCA count to choose
    ==========================================================================
*/
INT Set_AutoChannelSel_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING		arg)
{
	NDIS_802_11_SSID Ssid;
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	if (strlen(arg) <= MAX_LEN_OF_SSID)
	{
		if (strlen(arg) != 0)
		{
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
		}
		else   //ANY ssid
		{
			Ssid.SsidLength = 0; 
			memcpy(Ssid.Ssid, "", 0);
		}
	}
	if (strcmp(arg,"1") == 0)
		pAd->ApCfg.AutoChannelAlg = ChannelAlgApCnt;
	else if (strcmp(arg,"2") == 0)
		pAd->ApCfg.AutoChannelAlg = ChannelAlgCCA;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_AutoChannelSel_Proc Alg isn't defined\n"));
		return FALSE;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Set_AutoChannelSel_Proc Alg=%d \n", pAd->ApCfg.AutoChannelAlg));
	if (Ssid.SsidLength == 0)
		ApSiteSurvey(pAd, &Ssid, SCAN_PASSIVE, TRUE);
	else
		ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, TRUE);
    
    return TRUE;

}

#endif // AP_SCAN_SUPPORT //

INT Show_DriverInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
    printk("Driver version: %s\n", AP_DRIVER_VERSION);
    
    return TRUE;
}


#ifdef DFS_SUPPORT
int	Show_BlockCh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i; 

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].RemainingTimeForUse != 0)
		{
			printk("%d,%d;", pAd->ChannelList[i].Channel, pAd->ChannelList[i].RemainingTimeForUse);
		}
	}
	return TRUE;
}
#endif // DFS_SUPPORT //


INT	Show_MacTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT i;//, QueIdx=0;
    	UINT32 RegValue;
	ULONG DataRate=0;
	//PRXD_STRUC pRxD;
	//PTXD_STRUC pTxD;
	//PRTMP_TX_RING	pTxRing = &pAd->TxRing[QueIdx];	
	//PRTMP_MGMT_RING	pMgmtRing = &pAd->MgmtRing;	
	//PRTMP_RX_RING	pRxRing = &pAd->RxRing;	
	
	printk("\n");
	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	printk("BackOff Slot      : %s slot time, BKOFF_SLOT_CFG(0x1104) = 0x%08x\n", 
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED) ? "short" : "long",
 			RegValue);

#ifdef DOT11_N_SUPPORT
	printk("HT Operating Mode : %d\n", pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode);
	printk("\n");
#endif // DOT11_N_SUPPORT //
	
	printk("\n%-19s%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1", 
		   "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");
	
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry))
			&& (pEntry->Sst == SST_ASSOC))
		{
			DataRate=0;
			getRate(pEntry->HTPhyMode, &DataRate);
			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			printk("%-4d", (int)pEntry->Aid);
			printk("%-4d", (int)pEntry->apidx);
			printk("%-4d", (int)pEntry->PsMode);
			printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
			printk("%-8d", (int)pEntry->MmpsMode);
#endif // DOT11_N_SUPPORT //
			printk("%-7d", pEntry->RssiSample.AvgRssi0);
			printk("%-7d", pEntry->RssiSample.AvgRssi1);
			printk("%-7d", pEntry->RssiSample.AvgRssi2);
			printk("%-10s", GetPhyMode(pEntry->HTPhyMode.field.MODE));
			printk("%-6s", GetBW(pEntry->HTPhyMode.field.BW));
			printk("%-6d", pEntry->HTPhyMode.field.MCS);
			printk("%-6d", pEntry->HTPhyMode.field.ShortGI);
			printk("%-6d", pEntry->HTPhyMode.field.STBC);
			printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
			printk("%-7d", (int)DataRate);
			printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount, 
						(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);
			printk("\n");
		}
	} 

	return TRUE;
}

INT	Show_StaSecurityInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT i;
	UCHAR	apidx;
    	
	printk("\n");
	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
	{
		printk(" BSS(%d) AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s, WPAMixPairCipher(0x%02X)\n", 
							apidx, 
							pAd->ApCfg.MBSSID[apidx].AuthMode, 
							GetAuthMode(pAd->ApCfg.MBSSID[apidx].AuthMode), 
							pAd->ApCfg.MBSSID[apidx].WepStatus, 
							GetEncryptType(pAd->ApCfg.MBSSID[apidx].WepStatus), 
							pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus, 
							GetEncryptType(pAd->ApCfg.MBSSID[apidx].GroupKeyWepStatus),
							pAd->ApCfg.MBSSID[apidx].WpaMixPairCipher);		
	}
	printk("\n");
	
	printk("\n%-19s%-4s%-4s%-15s%-12s\n",
		   "MAC", "AID", "BSS", "Auth", "Encrypt");
	
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
		{
			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			printk("%-4d", (int)pEntry->Aid);
			printk("%-4d", (int)pEntry->apidx);
			printk("%-15s", GetAuthMode(pEntry->AuthMode));
			printk("%-12s", GetEncryptType(pEntry->WepStatus));						
			printk("\n");
		}
	} 

	return TRUE;
}


#ifdef DOT11_N_SUPPORT
INT	Show_BaTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT i, j;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;
	STRING		 tmpBuf[6];

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;

		if (IS_ENTRY_APCLI(pEntry))
			strcpy(tmpBuf, "ApCli");
		else if (IS_ENTRY_WDS(pEntry))
			strcpy(tmpBuf, "WDS");
		else if (IS_ENTRY_MESH(pEntry))
			strcpy(tmpBuf, "Mesh");
		else
			strcpy(tmpBuf, "STA");
	
		printk("%02X:%02X:%02X:%02X:%02X:%02X (Aid = %d) (%s) -\n",
			pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
			pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5], pEntry->Aid, tmpBuf);
		
		printk("[Recipient]\n");
		for (j=0; j < NUM_OF_TID; j++)
		{
			if (pEntry->BARecWcidArray[j] != 0)
			{
				pRecBAEntry =&pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]];
				printk("TID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d\n", j, pRecBAEntry->BAWinSize, pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen);
			}
		}
		printk("\n");

		printk("[Originator]\n");
		for (j=0; j < NUM_OF_TID; j++)
		{
			if (pEntry->BAOriWcidArray[j] != 0)
			{
				pOriBAEntry =&pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]];
				printk("TID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n", j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence, pEntry->TxSeq[j]);
			}
		}
		printk("\n\n");
	}

	return TRUE;
}
#endif // DOT11_N_SUPPORT //

INT	Show_RAInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
#if defined (RT2883) || defined (RT3883)
	printk("PreAntSwitch: %d\n", pAd->CommonCfg.PreAntSwitch);
	printk("PreAntSwitchRSSI: %d\n", pAd->CommonCfg.PreAntSwitchRSSI);
	printk("FixedRate: %d\n", pAd->CommonCfg.FixedRate);
#endif // defined (RT2883) || defined (RT3883) //
#ifdef RT3883
	printk("CFOTrack: %d\n", pAd->CommonCfg.CFOTrack);
#endif // RT3883 //

#ifdef NEW_RATE_ADAPT_SUPPORT
	printk("LowTrafficThrd: %d\n", pAd->CommonCfg.lowTrafficThrd);
	printk("TrainUpRule: %d\n", pAd->CommonCfg.TrainUpRule);
	printk("TrainUpRuleRSSI: %d\n", pAd->CommonCfg.TrainUpRuleRSSI);
	printk("TrainUpLowThrd: %d\n", pAd->CommonCfg.TrainUpLowThrd);
	printk("TrainUpHighThrd: %d\n", pAd->CommonCfg.TrainUpHighThrd);
#endif // NEW_RATE_ADAPT_SUPPORT //

#ifdef STREAM_MODE_SUPPORT
	printk("StreamMode: %d\n", pAd->CommonCfg.StreamMode);
	printk("StreamModeMCS: 0x%04x\n", pAd->CommonCfg.StreamModeMCS);
#endif // STREAM_MODE_SUPPORT //
#ifdef TXBF_SUPPORT
	printk("ITxBfEn: %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
	printk("ITxBfTimeout: %ld\n", pAd->CommonCfg.ITxBfTimeout);
	printk("ETxBfTimeout: %ld\n", pAd->CommonCfg.ETxBfTimeout);
	printk("ETxBfEnCond: %ld\n", pAd->CommonCfg.ETxBfEnCond);
	printk("ETxBfNoncompress: %d\n", pAd->CommonCfg.ETxBfNoncompress);
	printk("ETxBfIncapable: %d\n", pAd->CommonCfg.ETxBfIncapable);
#endif // TXBF_SUPPORT //

	printk("DebugFlags: 0x%04x\n", pAd->CommonCfg.DebugFlags);

	return TRUE;
}



#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
INT Set_DiagOpt_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG diagOpt;
	//POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	diagOpt = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("DiagOpt=%ld!\n", diagOpt));


	return TRUE;
}

INT Set_BDInfo_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT32 i, QueIdx=0;
	ULONG RegValue;
    PRXD_STRUC pRxD;
    PTXD_STRUC pTxD;
	PRTMP_TX_RING	pTxRing = &pAd->TxRing[QueIdx];	
	PRTMP_MGMT_RING	pMgmtRing = &pAd->MgmtRing;	
	PRTMP_RX_RING	pRxRing = &pAd->RxRing;	

	printk("\n");
	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	printk("BackOff Slot      : %s slot time, BKOFF_SLOT_CFG(0x1104) = 0x%08lx\n", 
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED) ? "short" : "long",
 			RegValue);
#ifdef DOT11_N_SUPPORT
	printk("HT Operating Mode : %d\n", pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode);
#endif // DOT11_N_SUPPORT //
	printk("\n");
	printk("[Tx]: SwFreeIdx=%d, CpuIdx=%d, DmaIdx=%d\n",
		pAd->TxRing[QueIdx].TxSwFreeIdx, 
		pAd->TxRing[QueIdx].TxCpuIdx,
		pAd->TxRing[QueIdx].TxDmaIdx);
			
	pTxD = (PTXD_STRUC) pTxRing->Cell[pAd->TxRing[QueIdx].TxSwFreeIdx].AllocVa;
	hex_dump("Tx SwFreeIdx Descriptor", (char *)pTxD, 16);
	printk("pTxD->DMADONE = %x\n", pTxD->DMADONE);

	pTxD = (PTXD_STRUC) pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa;
	hex_dump("Tx CpuIdx Descriptor", (char *)pTxD, 16);
	printk("pTxD->DMADONE = %x\n", pTxD->DMADONE);
	
	pTxD = (PTXD_STRUC) pTxRing->Cell[pAd->TxRing[QueIdx].TxDmaIdx].AllocVa;
	hex_dump("Tx DmaIdx Descriptor", (char *)pTxD, 16);
	printk("pTxD->DMADONE = %x\n", pTxD->DMADONE);
	
	printk("[Mgmt]: SwFreeIdx=%d, CpuIdx=%d, DmaIdx=%d\n",
		pAd->MgmtRing.TxSwFreeIdx, 
		pAd->MgmtRing.TxCpuIdx,
		pAd->MgmtRing.TxDmaIdx);
			
	pTxD = (PTXD_STRUC) pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa;
	hex_dump("Mgmt SwFreeIdx Descriptor", (char *)pTxD, 16);
	
	pTxD = (PTXD_STRUC) pMgmtRing->Cell[pAd->MgmtRing.TxCpuIdx].AllocVa;
	hex_dump("Mgmt CpuIdx Descriptor", (char *)pTxD, 16);
	
	pTxD = (PTXD_STRUC) pMgmtRing->Cell[pAd->MgmtRing.TxDmaIdx].AllocVa;
	hex_dump("Mgmt DmaIdx Descriptor", (char *)pTxD, 16);
	printk("\n");
	printk("[Rx]:  SwRedIdx=%d, CpuIdx=%d, DmaIdx=%d\n", 
		pAd->RxRing.RxSwReadIdx,
		pAd->RxRing.RxCpuIdx,
		pAd->RxRing.RxDmaIdx);
	
	pRxD = (PRXD_STRUC) pRxRing->Cell[pAd->RxRing.RxSwReadIdx].AllocVa;
	hex_dump("RX SwRedIdx Descriptor", (char *)pRxD, 16);
	
	pRxD = (PRXD_STRUC) pRxRing->Cell[pAd->RxRing.RxCpuIdx].AllocVa;
	hex_dump("RX RxCupIdx Descriptor", (char *)pRxD, 16);
	
	pRxD = (PRXD_STRUC) pRxRing->Cell[pAd->RxRing.RxDmaIdx].AllocVa;
	hex_dump("RX RxDmaIdx Descritpro", (char *)pRxD, 16);
	printk("\n%-19s%-4s%-4s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s\n",
		"MAC", "AID", "PSM", "RSSI0", "RSSI1", "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC");
	
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{
			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			printk("%-4d", (int)pEntry->Aid);
			printk("%-4d", (int)pEntry->PsMode);
			printk("%-7d", pEntry->RssiSample.AvgRssi0);
			printk("%-7d", pEntry->RssiSample.AvgRssi1);
			printk("%-7d", pEntry->RssiSample.AvgRssi2);


			printk("%-10s", GetPhyMode(pEntry->HTPhyMode.field.MODE));
			printk("%-6s", GetBW(pEntry->HTPhyMode.field.BW));
			printk("%-6d", pEntry->HTPhyMode.field.MCS);
			printk("%-6d", pEntry->HTPhyMode.field.ShortGI);
			printk("%-6d", pEntry->HTPhyMode.field.STBC);
			printk("%-10d, %d, %d\n", pEntry->FIFOCount, pEntry->DebugTxCount, pEntry->DebugTxCount-pEntry->FIFOCount);

			printk("\n");
		}
	} 

	return TRUE;
}

INT Show_Diag_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	RtmpDiagStruct	*pDiag;
	UCHAR			i, start, stop, McsIdx, SwQNumLevel, TxDescNumLevel;
	unsigned long		irqFlags;
	UCHAR			McsMaxIdx = 16;
	
#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
	McsMaxIdx = 24;
#endif // DOT11N_SS3_SUPPORT //
	pDiag = &pAd->DiagStruct;
	
	if (pDiag->inited == FALSE)
		return TRUE;

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqFlags);
	start = pDiag->ArrayStartIdx;
	stop = pDiag->ArrayCurIdx;
	printk("Start=%d, stop=%d!\n\n", start, stop);
	printk("    %-12s", "Time(Sec)");
	for(i=1; i< DIAGNOSE_TIME; i++)
	{
		printk("%-7d", i);
	}
	printk("\n    -------------------------------------------------------------------------------\n");
	printk("Tx Info:\n");
	printk("    %-12s", "TxDataCnt");
	for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
	{
		printk("%-7d", pDiag->TxDataCnt[i]);
	}
	printk("\n    %-12s", "TxFailCnt");
	for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
	{
		printk("%-7d", pDiag->TxFailCnt[i]);
	}
	printk("\n    %-12s", "TxAggCnt");
	for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
	{
		printk("%-7d", pDiag->TxAggCnt[i]);
	}
	printk("\n");

	
	printk("\n    %-12s\n", "Sw-Queued TxSwQCnt");
	for (SwQNumLevel = 0 ; SwQNumLevel < 9; SwQNumLevel++)
	{	
		if (SwQNumLevel == 8)
			printk("\t>%-5d",  SwQNumLevel);
		else
			printk("\t%-6d", SwQNumLevel);
		for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
		{
			printk("%-7d", pDiag->TxSWQueCnt[i][SwQNumLevel]);
		}
		printk("\n");
	}
	
	printk("\n    %-12s\n", "DMA-Queued TxDescCnt");
	for(TxDescNumLevel = 0; TxDescNumLevel < 16; TxDescNumLevel++)
	{
		if (TxDescNumLevel == 15)
			printk("\t>%-5d",  TxDescNumLevel);
		else
			printk("\t%-6d",  TxDescNumLevel);
		for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
		{
			printk("%-7d", pDiag->TxDescCnt[i][TxDescNumLevel]);
		}
		printk("\n");
	}
	
#ifdef DOT11_N_SUPPORT	
	printk("\n    %-12s\n", "Tx-Agged AMPDUCnt");
	for (McsIdx =0 ; McsIdx < 16; McsIdx++)
	{
		printk("\t%-6d", (McsIdx+1));
		for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
		{		
			printk("%d(%d%%)  ", pDiag->TxAMPDUCnt[i][McsIdx], pDiag->TxAMPDUCnt[i][McsIdx] ? (pDiag->TxAMPDUCnt[i][McsIdx] * 100 / pDiag->TxAggCnt[i]) : 0);
		}		
//		printk("\n\t%-6s", "R(%)");
//		for (i = start, count=0; count < DIAGNOSE_TIME;  i = (i+1) % DIAGNOSE_TIME, count++)
//		{
//			printk("%-5d", pDiag->TxAMPDUCnt[i][McsIdx] ? (pDiag->TxAMPDUCnt[i][McsIdx] * 100 / pDiag->TxTotalCnt[i]) : 0);
//		}
		printk("\n");
	}
#endif // DOT11_N_SUPPORT //

	printk("\n    %-12s\n", "TxMcsCnt");
	for (McsIdx =0 ; McsIdx < McsMaxIdx; McsIdx++)
	{
		printk("\t%-6d", McsIdx);
		for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
		{
			printk("%-7d", pDiag->TxMcsCnt[i][McsIdx]);
		}
		printk("\n");
	}
	
	printk("Rx Info\n");
	printk("    %-12s", "RxDataCnt");	
	for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
	{
		printk("%-7d", pDiag->RxDataCnt[i]);
	}
	printk("\n    %-12s", "RxCrcErrCnt");	
	for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
	{
		printk("%-7d", pDiag->RxCrcErrCnt[i]);
	}
	printk("\n    %-12s\n", "RxMcsCnt");
	for (McsIdx =0 ; McsIdx < McsMaxIdx; McsIdx++) // 3*3
	{
		printk("\t%-6d", McsIdx);
		for (i = start; i != stop;  i = (i+1) % DIAGNOSE_TIME)
		{
			printk("%-7d", pDiag->RxMcsCnt[i][McsIdx]);
		}
		printk("\n");
	}
	printk("\n-------------\n");

	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqFlags);

	return TRUE;

}
#endif // DBG_DIAGNOSE //
#endif // RTMP_MAC_PCI //


#ifdef DOT11_N_SUPPORT
#if defined (CONFIG_RALINK_RT2883)
#define MAX_AGG_CNT		16
#elif defined (CONFIG_RALINK_RT3883)
#define MAX_AGG_CNT		16
#else
#define MAX_AGG_CNT		8
#endif

// DisplayTxAgg - display Aggregation statistics from MAC
static void DisplayTxAgg (
	IN PRTMP_ADAPTER	pAd)
{
	TX_AGG_CNT_STRUC	TxAggCnt;
	TX_AGG_CNT0_STRUC	TxAggCnt0;
	TX_AGG_CNT1_STRUC	TxAggCnt1;
	TX_AGG_CNT2_STRUC	TxAggCnt2;
	TX_AGG_CNT3_STRUC	TxAggCnt3;
#if MAX_AGG_CNT>8
	TX_AGG_CNTN_STRUC	TxAggCntN;
	static USHORT aggReg[] = {
						TX_AGG_CNT4, TX_AGG_CNT5, TX_AGG_CNT6, TX_AGG_CNT7,
						};
#endif

	UINT32				totalCount;
	UINT32				aggCnt[MAX_AGG_CNT];
	int 				i;

	RTMP_IO_READ32(pAd, TX_AGG_CNT, &TxAggCnt.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT0, &TxAggCnt0.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT1, &TxAggCnt1.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT2, &TxAggCnt2.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT3, &TxAggCnt3.word);

	aggCnt[0] = TxAggCnt0.field.AggSize1Count;
	aggCnt[1] = TxAggCnt0.field.AggSize2Count;
	aggCnt[2] = TxAggCnt1.field.AggSize3Count;
	aggCnt[3] = TxAggCnt1.field.AggSize4Count;
	aggCnt[4] = TxAggCnt2.field.AggSize5Count;
	aggCnt[5] = TxAggCnt2.field.AggSize6Count;
	aggCnt[6] = TxAggCnt3.field.AggSize7Count;
	aggCnt[7] = TxAggCnt3.field.AggSize8Count;

#if MAX_AGG_CNT>8
	for (i=0; i<(MAX_AGG_CNT-8)/2; i++) {
		RTMP_IO_READ32(pAd, aggReg[i], &TxAggCntN.word);
		aggCnt[2*i + 8] = TxAggCntN.field.AggSizeLowCount;
		aggCnt[2*i + 9] = TxAggCntN.field.AggSizeHighCount;
	}
#endif

	totalCount = TxAggCnt.field.NonAggTxCount + TxAggCnt.field.AggTxCount;
	if (totalCount > 0)
		for (i=0; i<MAX_AGG_CNT; i++) {
			DBGPRINT(RT_DEBUG_OFF, ("\t%d MPDU=%d (%d%%)\n", i+1, aggCnt[i], aggCnt[i]*100/totalCount));
		}
	printk("====================\n");

}
#endif // DOT11_N_SUPPORT //

INT	Show_Sat_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	// Sanity check for calculation of sucessful count

	printk("TransmittedFragmentCount = %d\n", pAd->WlanCounters.TransmittedFragmentCount.u.LowPart);
	printk("MulticastTransmittedFrameCount = %d\n", pAd->WlanCounters.MulticastTransmittedFrameCount.u.LowPart);
	printk("FailedCount = %d\n", pAd->WlanCounters.FailedCount.u.LowPart);
	printk("RetryCount = %d\n", pAd->WlanCounters.RetryCount.u.LowPart);
	printk("MultipleRetryCount = %d\n", pAd->WlanCounters.MultipleRetryCount.u.LowPart);
	printk("RTSSuccessCount = %d\n", pAd->WlanCounters.RTSSuccessCount.u.LowPart);
	printk("RTSFailureCount = %d\n", pAd->WlanCounters.RTSFailureCount.u.LowPart);
	printk("ACKFailureCount = %d\n", pAd->WlanCounters.ACKFailureCount.u.LowPart);
	printk("FrameDuplicateCount = %d\n", pAd->WlanCounters.FrameDuplicateCount.u.LowPart);
	printk("ReceivedFragmentCount = %d\n", pAd->WlanCounters.ReceivedFragmentCount.u.LowPart);
	printk("MulticastReceivedFrameCount = %d\n", pAd->WlanCounters.MulticastReceivedFrameCount.u.LowPart);
#ifdef DBG 		
	printk("RealFcsErrCount = %d\n", pAd->RalinkCounters.RealFcsErrCount.u.LowPart);
#else
	printk("FCSErrorCount = %d\n", pAd->WlanCounters.FCSErrorCount.u.LowPart);
	printk("FrameDuplicateCount.LowPart = %d\n", pAd->WlanCounters.FrameDuplicateCount.u.LowPart / 100);
#endif

#ifdef DOT11_N_SUPPORT
	printk("\n===Some 11n statistics variables: \n");
	// Some 11n statistics variables
	printk("TransmittedAMSDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedAMSDUCount.u.LowPart);
	printk("TransmittedOctetsInAMSDU = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedOctetsInAMSDU.QuadPart);
	printk("ReceivedAMSDUCount = %ld\n", (ULONG)pAd->RalinkCounters.ReceivedAMSDUCount.u.LowPart);	
	printk("ReceivedOctesInAMSDUCount = %ld\n", (ULONG)pAd->RalinkCounters.ReceivedOctesInAMSDUCount.QuadPart);	
	printk("TransmittedAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart);
	printk("TransmittedMPDUsInAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart);
	printk("TransmittedOctetsInAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.u.LowPart);
	printk("MPDUInReceivedAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.MPDUInReceivedAMPDUCount.u.LowPart);
#ifdef DOT11N_DRAFT3
	printk("fAnyStaFortyIntolerant=%d\n", pAd->MacTab.fAnyStaFortyIntolerant);
#endif // DOT11N_DRAFT3 //
#endif // DOT11_N_SUPPORT //

{
	int apidx;
		
	for (apidx=0; apidx < pAd->ApCfg.BssidNum; apidx++)
	{
		printk("-- IF-ra%d -- \n", apidx);
		printk("Packets Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxCount);
		printk("Packets Sent = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].TxCount);
		printk("Bytes Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].ReceivedByteCount);
		printk("Byte Sent = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].TransmittedByteCount);
		printk("Error Packets Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxErrorCount);
		printk("Drop Received Packets = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxDropCount);
		
#ifdef WSC_INCLUDED
		if (pAd->ApCfg.MBSSID[apidx].WscControl.WscConfMode != WSC_DISABLE)
		{
			WSC_CTRL *pWscCtrl;

			pWscCtrl = &pAd->ApCfg.MBSSID[apidx].WscControl;
			printk("WscInfo:\n"
					"\tWscConfMode=%d\n"
					"\tWscMode=%s\n"
					"\tWscConfStatus=%d\n"
					"\tWscPinCode=%d\n"
					"\tWscState=0x%x\n"
					"\tWscStatus=0x%x\n",
					pWscCtrl->WscConfMode, 
					((pWscCtrl->WscMode == WSC_PIN_MODE) ? "PIN" : "PBC"),
					pWscCtrl->WscConfStatus, pWscCtrl->WscEnrolleePinCode, 
					pWscCtrl->WscState, pWscCtrl->WscStatus);	
		}
#endif // WSC_INCLUDED //

		printk("-- IF-ra%d end -- \n", apidx);
	}
}

{
	int i, j, k, maxMcs = 15;
	PMAC_TABLE_ENTRY pEntry;

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
		maxMcs = 23;
#endif // DOT11N_SS3_SUPPORT //

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		{

			printk("\n%02X:%02X:%02X:%02X:%02X:%02X - ",
				   pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				   pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			printk("%-4d\n", (int)pEntry->Aid);

			for (j=maxMcs; j>=0; j--)
			{
				if ((pEntry->TXMCSExpected[j] != 0) || (pEntry->TXMCSFailed[j] !=0))
				{
					printk("MCS[%02d]: Expected %u, Successful %u (%d%%), Failed %u\n",
						   j, pEntry->TXMCSExpected[j], pEntry->TXMCSSuccessful[j], 
						   pEntry->TXMCSExpected[j] ? (100*pEntry->TXMCSSuccessful[j])/pEntry->TXMCSExpected[j] : 0,
						   pEntry->TXMCSFailed[j]);
					for(k=maxMcs; k>=0; k--)
					{
						if (pEntry->TXMCSAutoFallBack[j][k] != 0)
						{
							printk("\t\t\tAutoMCS[%02d]: %u (%d%%)\n", k, pEntry->TXMCSAutoFallBack[j][k],
								   (100*pEntry->TXMCSAutoFallBack[j][k])/pEntry->TXMCSExpected[j]);
						}
					}
				}
			}
		}
	}

}

#ifdef DOT11_N_SUPPORT
{
	// Display Tx Aggregation statistics
	DisplayTxAgg(pAd);
}
#endif // DOT11_N_SUPPORT //

	return TRUE;
}

INT	Show_Sat_Reset_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	// Sanity check for calculation of sucessful count

	printk("TransmittedFragmentCount = %d\n", pAd->WlanCounters.TransmittedFragmentCount.u.LowPart);
	printk("MulticastTransmittedFrameCount = %d\n", pAd->WlanCounters.MulticastTransmittedFrameCount.u.LowPart);
	printk("FailedCount = %d\n", pAd->WlanCounters.FailedCount.u.LowPart);
	printk("RetryCount = %d\n", pAd->WlanCounters.RetryCount.u.LowPart);
	printk("MultipleRetryCount = %d\n", pAd->WlanCounters.MultipleRetryCount.u.LowPart);
	printk("RTSSuccessCount = %d\n", pAd->WlanCounters.RTSSuccessCount.u.LowPart);
	printk("RTSFailureCount = %d\n", pAd->WlanCounters.RTSFailureCount.u.LowPart);
	printk("ACKFailureCount = %d\n", pAd->WlanCounters.ACKFailureCount.u.LowPart);
	printk("FrameDuplicateCount = %d\n", pAd->WlanCounters.FrameDuplicateCount.u.LowPart);
	printk("ReceivedFragmentCount = %d\n", pAd->WlanCounters.ReceivedFragmentCount.u.LowPart);
	printk("MulticastReceivedFrameCount = %d\n", pAd->WlanCounters.MulticastReceivedFrameCount.u.LowPart);
#ifdef DBG 		
	printk("RealFcsErrCount = %d\n", pAd->RalinkCounters.RealFcsErrCount.u.LowPart);
#else
	printk("FCSErrorCount = %d\n", pAd->WlanCounters.FCSErrorCount.u.LowPart);
	printk("FrameDuplicateCount.LowPart = %d\n", pAd->WlanCounters.FrameDuplicateCount.u.LowPart / 100);
#endif

	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart = 0;
	pAd->WlanCounters.MulticastTransmittedFrameCount.u.LowPart = 0;
	pAd->WlanCounters.FailedCount.u.LowPart = 0;
	pAd->WlanCounters.RetryCount.u.LowPart = 0;
	pAd->WlanCounters.MultipleRetryCount.u.LowPart = 0;
	pAd->WlanCounters.RTSSuccessCount.u.LowPart = 0;
	pAd->WlanCounters.RTSFailureCount.u.LowPart = 0;
	pAd->WlanCounters.ACKFailureCount.u.LowPart = 0;
	pAd->WlanCounters.FrameDuplicateCount.u.LowPart = 0;
	pAd->WlanCounters.ReceivedFragmentCount.u.LowPart = 0;
	pAd->WlanCounters.MulticastReceivedFrameCount.u.LowPart = 0;
#ifdef DBG 		
	pAd->RalinkCounters.RealFcsErrCount.u.LowPart = 0;
#else
	pAd->WlanCounters.FCSErrorCount.u.LowPart = 0;
	pAd->WlanCounters.FrameDuplicateCount.u.LowPart = 0;
#endif

{
	int i, j, k, maxMcs = 15;
	PMAC_TABLE_ENTRY pEntry;

#ifdef DOT11N_SS3_SUPPORT
	if (IS_RT2883(pAd) || IS_RT3883(pAd))
		maxMcs = 23;
#endif // DOT11N_SS3_SUPPORT //

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++) {
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {

			printk("\n%02X:%02X:%02X:%02X:%02X:%02X - ",
				   pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				   pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			printk("%-4d\n", (int)pEntry->Aid);

			for (j=maxMcs; j>=0; j--) {
				if ((pEntry->TXMCSExpected[j] != 0) || (pEntry->TXMCSFailed[j] !=0)) {
					printk("MCS[%02d]: Expected %u, Successful %u (%d%%), Failed %u\n",
						   j, pEntry->TXMCSExpected[j], pEntry->TXMCSSuccessful[j], 
						   pEntry->TXMCSExpected[j] ? (100*pEntry->TXMCSSuccessful[j])/pEntry->TXMCSExpected[j] : 0,
						   pEntry->TXMCSFailed[j]
						   );
					for(k=maxMcs; k>=0; k--) {
						if (pEntry->TXMCSAutoFallBack[j][k] != 0) {
							printk("\t\t\tAutoMCS[%02d]: %u (%d%%)\n", k, pEntry->TXMCSAutoFallBack[j][k],
								   (100*pEntry->TXMCSAutoFallBack[j][k])/pEntry->TXMCSExpected[j]);
						}
					}
				}
			}
		}
		
		for (j = 0; j< (maxMcs + 1); j++) {
			pEntry->TXMCSExpected[j] = 0;
			pEntry->TXMCSSuccessful[j] = 0;
			pEntry->TXMCSFailed[j] = 0;
			for(k = maxMcs; k >= 0; k--)
				pEntry->TXMCSAutoFallBack[j][k] = 0;
			}
		}
	}

#ifdef DOT11_N_SUPPORT
	// Display Tx Aggregation statistics
	DisplayTxAgg(pAd);
#endif // DOT11_N_SUPPORT //

	return TRUE;
}


#ifdef MAT_SUPPORT
INT	Show_MATTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	extern VOID dumpIPMacTb(MAT_STRUCT *pMatCfg, int index);
	extern NDIS_STATUS dumpSesMacTb(MAT_STRUCT *pMatCfg, int hashIdx);
	extern NDIS_STATUS dumpUidMacTb(MAT_STRUCT *pMatCfg, int hashIdx);
	extern NDIS_STATUS dumpIPv6MacTb(MAT_STRUCT *pMatCfg, int hashIdx);

	dumpIPMacTb(&pAd->MatCfg, -1);
	dumpSesMacTb(&pAd->MatCfg, -1);
	dumpUidMacTb(&pAd->MatCfg, -1);
	dumpIPv6MacTb(&pAd->MatCfg, -1);

	printk("Default BroadCast Address=%02x:%02x:%02x:%02x:%02x:%02x!\n", BROADCAST_ADDR[0], BROADCAST_ADDR[1],
			BROADCAST_ADDR[2], BROADCAST_ADDR[3], BROADCAST_ADDR[4], BROADCAST_ADDR[5]);
	return TRUE;
}
#endif // MAT_SUPPORT //


#ifdef DOT1X_SUPPORT
/* 
    ==========================================================================
    Description:
        It only shall be queried by 802.1x daemon for querying radius configuration.        
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlQueryRadiusConf(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq)
{
	UCHAR	apidx, srv_idx, keyidx, KeyLen = 0;
	UCHAR	*mpool;
	PDOT1X_CMM_CONF	pConf;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlQueryRadiusConf==>\n"));
	
	// Allocate memory
	os_alloc_mem(NULL, (PUCHAR *)&mpool, sizeof(DOT1X_CMM_CONF));	
    if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s: out of resource!!!\n", __FUNCTION__));
        return;
    }
	NdisZeroMemory(mpool, sizeof(DOT1X_CMM_CONF));

	pConf = (PDOT1X_CMM_CONF)mpool;

	// get MBSS number
	pConf->mbss_num = pAd->ApCfg.BssidNum;

	// get own ip address
	pConf->own_ip_addr = pAd->ApCfg.own_ip_addr;

	// get retry interval
	pConf->retry_interval = pAd->ApCfg.retry_interval;

	// get session timeout interval
	pConf->session_timeout_interval = pAd->ApCfg.session_timeout_interval;

	/* Get the quiet interval */
	pConf->quiet_interval = pAd->ApCfg.quiet_interval;

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
	{
		PMULTISSID_STRUCT 	pMbss = &pAd->ApCfg.MBSSID[apidx];
		PDOT1X_BSS_INFO  	p1xBssInfo = &pConf->Dot1xBssInfo[apidx];
	
		p1xBssInfo->radius_srv_num = pMbss->radius_srv_num;
	
		/* prepare radius ip, port and key */
		for (srv_idx = 0; srv_idx < pMbss->radius_srv_num; srv_idx++)
		{
			if (pMbss->radius_srv_info[srv_idx].radius_ip != 0)
			{
				p1xBssInfo->radius_srv_info[srv_idx].radius_ip = pMbss->radius_srv_info[srv_idx].radius_ip;
				p1xBssInfo->radius_srv_info[srv_idx].radius_port = pMbss->radius_srv_info[srv_idx].radius_port;
				p1xBssInfo->radius_srv_info[srv_idx].radius_key_len = pMbss->radius_srv_info[srv_idx].radius_key_len;
				if (pMbss->radius_srv_info[srv_idx].radius_key_len > 0)
				{
					NdisMoveMemory(p1xBssInfo->radius_srv_info[srv_idx].radius_key, 
									pMbss->radius_srv_info[srv_idx].radius_key, 
									pMbss->radius_srv_info[srv_idx].radius_key_len);
				}
			}
		}
		
		p1xBssInfo->ieee8021xWEP = (pMbss->IEEE8021X) ? 1 : 0;
		
		if (p1xBssInfo->ieee8021xWEP)
		{
			// Default Key index, length and material
			keyidx = pMbss->DefaultKeyId;
			p1xBssInfo->key_index = keyidx;

			// Determine if the key is valid. 
			KeyLen = pAd->SharedKey[apidx][keyidx].KeyLen;
			if (KeyLen == 5 || KeyLen == 13)
			{
				p1xBssInfo->key_length = KeyLen;
				NdisMoveMemory(p1xBssInfo->key_material, pAd->SharedKey[apidx][keyidx].Key, KeyLen);
			}
		}

		/* Get NAS-ID per BSS */
		if (pMbss->NasIdLen > 0)
		{
			p1xBssInfo->nasId_len = pMbss->NasIdLen;
			NdisMoveMemory(p1xBssInfo->nasId, pMbss->NasId, pMbss->NasIdLen);
		}

		// get EAPifname
		if (pAd->ApCfg.EAPifname_len[apidx] > 0)
		{
			pConf->EAPifname_len[apidx] = pAd->ApCfg.EAPifname_len[apidx];
			NdisMoveMemory(pConf->EAPifname[apidx], pAd->ApCfg.EAPifname[apidx], pAd->ApCfg.EAPifname_len[apidx]);
		}	

		// get PreAuthifname
		if (pAd->ApCfg.PreAuthifname_len[apidx] > 0)
		{
			pConf->PreAuthifname_len[apidx] = pAd->ApCfg.PreAuthifname_len[apidx];
			NdisMoveMemory(pConf->PreAuthifname[apidx], pAd->ApCfg.PreAuthifname[apidx], pAd->ApCfg.PreAuthifname_len[apidx]);
		}	

	}
				
	wrq->u.data.length = sizeof(DOT1X_CMM_CONF);
	if (copy_to_user(wrq->u.data.pointer, pConf, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	os_free_mem(NULL, mpool);
	
}


/* 
    ==========================================================================
    Description:
        UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlRadiusData(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	if ((pAd->ApCfg.MBSSID[pObj->ioctl_if].AuthMode == Ndis802_11AuthModeWPA) 
    	|| (pAd->ApCfg.MBSSID[pObj->ioctl_if].AuthMode == Ndis802_11AuthModeWPA2)
    	|| (pAd->ApCfg.MBSSID[pObj->ioctl_if].AuthMode == Ndis802_11AuthModeWPA1WPA2) 
    	|| (pAd->ApCfg.MBSSID[pObj->ioctl_if].IEEE8021X == TRUE))
    	WpaSend(pAd, wrq->u.data.pointer, wrq->u.data.length);
}

/* 
    ==========================================================================
    Description:
        UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlAddWPAKey(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	NDIS_AP_802_11_KEY 	*pKey;
	ULONG				KeyIdx;
	MAC_TABLE_ENTRY  	*pEntry;
	UCHAR				apidx;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx =	(UCHAR) pObj->ioctl_if;
		

	pKey = (PNDIS_AP_802_11_KEY) wrq->u.data.pointer;

	if (pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA)
	{
		if ((pKey->KeyLength == 32) || (pKey->KeyLength == 64))
		{
			if ((pEntry = MacTableLookup(pAd, pKey->addr)) != NULL)
			{
				INT	k_offset = 0;
			
		
				NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].PMK, pKey->KeyMaterial + k_offset, 32);				
    	        DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlAddWPAKey-IF(ra%d) : Add PMK=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x....\n", apidx,
            	pAd->ApCfg.MBSSID[apidx].PMK[0],pAd->ApCfg.MBSSID[apidx].PMK[1],pAd->ApCfg.MBSSID[apidx].PMK[2],pAd->ApCfg.MBSSID[apidx].PMK[3],
            	pAd->ApCfg.MBSSID[apidx].PMK[4],pAd->ApCfg.MBSSID[apidx].PMK[5],pAd->ApCfg.MBSSID[apidx].PMK[6],pAd->ApCfg.MBSSID[apidx].PMK[7]));
			}
		}
	}
	else	// Old WEP stuff
	{
		UCHAR	CipherAlg;
    	PUCHAR	Key;

		if(pKey->KeyLength >= 32)
			return;
		
		KeyIdx = pKey->KeyIndex & 0x0fffffff;

		if (KeyIdx < 4)
		{
			// it is a shared key
			if (pKey->KeyIndex & 0x80000000)
			{
				UINT8	Wcid;
							
				DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlAddWPAKey-IF(ra%d) : Set Group Key\n", apidx));

				// Default key for tx (shared key)
				pAd->ApCfg.MBSSID[apidx].DefaultKeyId = (UCHAR) KeyIdx;								
                     
				// set key material and key length
				if (pKey->KeyLength > 16)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlAddWPAKey-IF(ra%d) : Key length too long %d\n", apidx, pKey->KeyLength));
					pKey->KeyLength = 16;
				}
				pAd->SharedKey[apidx][KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(pAd->SharedKey[apidx][KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);
				
				// Set Ciper type
				if (pKey->KeyLength == 5)
					pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_WEP64;
				else
					pAd->SharedKey[apidx][KeyIdx].CipherAlg = CIPHER_WEP128;
			
    			CipherAlg = pAd->SharedKey[apidx][KeyIdx].CipherAlg;
    			Key = pAd->SharedKey[apidx][KeyIdx].Key;

				// Set Group key material to Asic
				AsicAddSharedKeyEntry(pAd, apidx, (UINT8)KeyIdx, &pAd->SharedKey[apidx][KeyIdx]);
		
				/* Get a specific WCID to record this MBSS key attribute */
				GET_GroupKey_WCID(Wcid, apidx);
												
				RTMPSetWcidSecurityInfo(pAd, apidx,(UINT8)KeyIdx, 
									CipherAlg, Wcid, SHAREDKEYTABLE);												
			}
			else	// For Pairwise key setting
			{
				pEntry = MacTableLookup(pAd, pKey->addr);

				if (pEntry)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlAddWPAKey-IF(ra%d) : Set Pair-wise Key\n", apidx));
		
					// set key material and key length
 					pEntry->PairwiseKey.KeyLen = (UCHAR)pKey->KeyLength;
					NdisMoveMemory(pEntry->PairwiseKey.Key, &pKey->KeyMaterial, pKey->KeyLength);
					
					// set Cipher type
					if (pKey->KeyLength == 5)
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP64;
					else
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP128;
						
					// Add Pair-wise key to Asic
					AsicAddPairwiseKeyEntry(
						pAd, 
						(UCHAR)pEntry->Aid,
                		&pEntry->PairwiseKey);

					/* update WCID attribute table and IVEIV table for this entry */
					RTMPSetWcidSecurityInfo(pAd, 
										pEntry->apidx, 
										(UINT8)KeyIdx, 										 
										pEntry->PairwiseKey.CipherAlg, 
										pEntry->Aid,
										PAIRWISEKEYTABLE);

				}	
			}
		}
	}
}


/* 
    ==========================================================================
    Description:
        UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	UCHAR				apidx;
	NDIS_AP_802_11_KEY 	*pKey;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx =	(UCHAR) pObj->ioctl_if;

	pKey = (PNDIS_AP_802_11_KEY) wrq->u.data.pointer;
    
    if (pAd->ApCfg.MBSSID[apidx].AuthMode >= Ndis802_11AuthModeWPA2)
	{
		if(pKey->KeyLength == 32)
		{
			UCHAR	digest[80], PMK_key[20], macaddr[MAC_ADDR_LEN];
			
			// Calculate PMKID
			NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
			NdisMoveMemory(&PMK_key[8], pAd->ApCfg.MBSSID[apidx].Bssid, MAC_ADDR_LEN);
			NdisMoveMemory(&PMK_key[14], pKey->addr, MAC_ADDR_LEN);
			RT_HMAC_SHA1(pKey->KeyMaterial, PMK_LEN, PMK_key, 20, digest, SHA1_DIGEST_SIZE);

			NdisMoveMemory(macaddr, pKey->addr, MAC_ADDR_LEN);
			RTMPAddPMKIDCache(pAd, apidx, macaddr, digest, pKey->KeyMaterial);
			
			DBGPRINT(RT_DEBUG_TRACE, ("WPA2(pre-auth):(%02x:%02x:%02x:%02x:%02x:%02x)Calc PMKID=%02x:%02x:%02x:%02x:%02x:%02x\n", 
				pKey->addr[0],pKey->addr[1],pKey->addr[2],pKey->addr[3],pKey->addr[4],pKey->addr[5],digest[0],digest[1],digest[2],digest[3],digest[4],digest[5]));
			DBGPRINT(RT_DEBUG_TRACE, ("PMK =%02x:%02x:%02x:%02x-%02x:%02x:%02x:%02x\n",pKey->KeyMaterial[0],pKey->KeyMaterial[1],
				pKey->KeyMaterial[2],pKey->KeyMaterial[3],pKey->KeyMaterial[4],pKey->KeyMaterial[5],pKey->KeyMaterial[6],pKey->KeyMaterial[7]));
		}
		else
            DBGPRINT(RT_DEBUG_ERROR, ("Set::RT_OID_802_11_WPA2_ADD_PMKID_CACHE ERROR or is wep key \n"));
	}
    
    DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPIoctlAddPMKIDCache\n"));
}


/* 
    ==========================================================================
    Description:
        UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlStaticWepCopy(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	MAC_TABLE_ENTRY  *pEntry;
	UCHAR	MacAddr[MAC_ADDR_LEN];
	UCHAR			apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	apidx =	(UCHAR) pObj->ioctl_if;
	
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlStaticWepCopy-IF(ra%d)\n", apidx));

    if (wrq->u.data.length != sizeof(MacAddr))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("RTMPIoctlStaticWepCopy: the length isn't match (%d)\n", wrq->u.data.length));
        return;
    }
    else
    {
    	UINT32 len;
		
        len = copy_from_user(&MacAddr, wrq->u.data.pointer, wrq->u.data.length);    
        pEntry = MacTableLookup(pAd, MacAddr);
        if (!pEntry)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("RTMPIoctlStaticWepCopy: the mac address isn't match\n"));
            return;
        }
        else
        {
            UCHAR	KeyIdx;
            
            KeyIdx = pAd->ApCfg.MBSSID[apidx].DefaultKeyId;
            
            //need to copy the default shared-key to pairwise key table for this entry in 802.1x mode
			if (pAd->SharedKey[apidx][KeyIdx].KeyLen == 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: Can not get Default shared-key (index-%d)\n", KeyIdx));
				return;
			}
			else
        	{
            	pEntry->PairwiseKey.KeyLen = pAd->SharedKey[apidx][KeyIdx].KeyLen;
            	NdisMoveMemory(pEntry->PairwiseKey.Key, pAd->SharedKey[apidx][KeyIdx].Key, pEntry->PairwiseKey.KeyLen);
            	pEntry->PairwiseKey.CipherAlg = pAd->SharedKey[apidx][KeyIdx].CipherAlg;

				// Add Pair-wise key to Asic
            	AsicAddPairwiseKeyEntry(
                		pAd, 
                		(UCHAR)pEntry->Aid,
                		&pEntry->PairwiseKey);

				/* update WCID attribute table and IVEIV table for this entry */
				RTMPSetWcidSecurityInfo(pAd, 
										pEntry->apidx, 
										(UINT8)KeyIdx, 
                						pEntry->PairwiseKey.CipherAlg, 
										pEntry->Aid, 
										PAIRWISEKEYTABLE);
        	}
			
        }
	}
    return;
}

/* 
    ==========================================================================
    Description:
        UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlSetIdleTimeout(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	MAC_TABLE_ENTRY  		*pEntry;	
	PDOT1X_IDLE_TIMEOUT		pIdleTime;
		
	if (wrq->u.data.length != sizeof(DOT1X_IDLE_TIMEOUT))
	{
        DBGPRINT(RT_DEBUG_ERROR, ("%s : the length is mis-match\n", __FUNCTION__));
        return;
    }

	pIdleTime = (PDOT1X_IDLE_TIMEOUT)wrq->u.data.pointer;

	if ((pEntry = MacTableLookup(pAd, pIdleTime->StaAddr)) == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : the entry is empty\n", __FUNCTION__));
        return;
    }
	else
	{
		pEntry->NoDataIdleCount = 0;
		pEntry->StaIdleTimeout = pIdleTime->idle_timeout;
		DBGPRINT(RT_DEBUG_TRACE, ("%s : Update Idle-Timeout(%d) from dot1x daemon\n",
									__FUNCTION__, pEntry->StaIdleTimeout));
	}
	
	return;
}
#endif // DOT1X_SUPPORT //

#ifdef DBG
/* 
    ==========================================================================
    Description:
        Read / Write BBP
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 bbp               ==> read all BBP
               2.) iwpriv ra0 bbp 1             ==> read BBP where RegID=1
               3.) iwpriv ra0 bbp 1=10		    ==> write BBP R1=0x10
    ==========================================================================
*/
VOID RTMPAPIoctlBBP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	UCHAR				regBBP = 0;
	PSTRING				mpool, msg; //msg[2048];
	PSTRING				arg; //arg[255];
	PSTRING				ptr;
	INT					bbpId;
	LONG				bbpValue;
	BOOLEAN				bIsPrintAllBBP = FALSE, bAllowDump, bCopyMsg;
	INT					argLen, Status;



	mpool = (PSTRING)kmalloc(sizeof(CHAR)*(MAX_BBP_MSG_SIZE+256+12), MEM_ALLOC_FLAG);
	if (mpool == NULL) {
		return;
	}

	NdisZeroMemory(mpool, MAX_BBP_MSG_SIZE+256+12);
	msg = (PSTRING)((ULONG)(mpool+3) & (ULONG)~0x03);
	arg = (PSTRING)((ULONG)(msg+MAX_BBP_MSG_SIZE+3) & (ULONG)~0x03);

	bAllowDump = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_NODUMPMSG) == RTPRIV_IOCTL_FLAG_NODUMPMSG) ? FALSE : TRUE;
	bCopyMsg = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_NOSPACE) == RTPRIV_IOCTL_FLAG_NOSPACE) ? FALSE : TRUE;
	argLen = strlen((char*)wrq->u.data.pointer);

	if (argLen > 0)
	{
		NdisMoveMemory(arg, wrq->u.data.pointer, (argLen > 255) ? 255 : argLen);
		ptr = arg;
		sprintf(msg, "\n");
		//Parsing Read or Write
		while ((this_char = strsep((char **)&ptr, ",")) != NULL)
		{
			if (!*this_char)
				continue;

			if ((value = strchr(this_char, '=')) != NULL)
				*value++ = 0;

			if (!value || !*value)
			{ //Read
				if (sscanf(this_char, "%d", &(bbpId)) == 1)
				{
					if (bbpId <= MAX_BBP_ID)
					{
#ifdef RALINK_ATE
						/*
							In RT2860 ATE mode, we do not load 8051 firmware.
							We must access BBP directly.
							For RT2870 ATE mode, ATE_BBP_IO_WRITE8(/READ8)_BY_REG_ID are redefined.
						*/
						if (ATE_ON(pAdapter))
						{
							ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							// Sync with QA for comparation
							sprintf(msg+strlen(msg), "%03d = %02X\n", bbpId, regBBP);
						}
						else
#endif // RALINK_ATE //
						{
							// according to Andy, Gary, David require.
							// the command bbp shall read BBP register directly for dubug.
							BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							sprintf(msg+strlen(msg), "R%02d[0x%02X]:%02X\n", bbpId, bbpId, regBBP);
						}
					}
					else
					{
						//Invalid parametes, so default printk all bbp
						bIsPrintAllBBP = TRUE;
						break;
					}
				}
				else
				{
					//Invalid parametes, so default printk all bbp
					bIsPrintAllBBP = TRUE;
					break;
				}
			}
			else
			{ //Write
				if ((sscanf(this_char, "%d", &(bbpId)) == 1) && (sscanf(value, "%lx", &(bbpValue)) == 1))
				{
					if (bbpId <= MAX_BBP_ID)
					{
#ifdef RALINK_ATE
						/*
							In RT2860 ATE mode, we do not load 8051 firmware.
							We must access BBP directly.
							For RT2870 ATE mode, ATE_BBP_IO_WRITE8(/READ8)_BY_REG_ID are redefined.
						*/
						if (ATE_ON(pAdapter))
						{
							ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							ATE_BBP_IO_WRITE8_BY_REG_ID(pAdapter, (UCHAR)bbpId,(UCHAR) bbpValue);

							//Read it back for showing
							ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							// Sync with QA for comparation
							sprintf(msg+strlen(msg), "%03d = %02X\n", bbpId, regBBP);
						}
						else
#endif // RALINK_ATE //
						{
							// according to Andy, Gary, David require.
							// the command bbp shall read/write BBP register directly for dubug.
							BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							BBP_IO_WRITE8_BY_REG_ID(pAdapter, (UCHAR)bbpId,(UCHAR) bbpValue);
							//Read it back for showing
							BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							sprintf(msg+strlen(msg), "R%02d[0x%02X]:%02X\n", bbpId, bbpId, regBBP);
						}
					}
					else
					{	
						//Invalid parametes, so default printk all bbp
						bIsPrintAllBBP = TRUE;
						break;
					}
				}
				else
				{
					//Invalid parametes, so default printk all bbp
					bIsPrintAllBBP = TRUE;
					break;
				}
			}
		}
	}
	else
		bIsPrintAllBBP = TRUE;

	if (bIsPrintAllBBP)
	{
		memset(msg, 0x00, MAX_BBP_MSG_SIZE);
		sprintf(msg, "\n");
		for (bbpId = 0; bbpId <= MAX_BBP_ID; bbpId++)
		{
#ifdef RALINK_ATE
			/*
				In RT2860 ATE mode, we do not load 8051 firmware.
				We must access BBP directly.
				For RT2870 ATE mode, ATE_BBP_IO_WRITE8(/READ8)_BY_REG_ID are redefined.
			*/
			if (ATE_ON(pAdapter))
			{
				ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
				// Sync with QA for comparation
				sprintf(msg+strlen(msg), "%03d = %02X\n", bbpId, regBBP);
			}
			else
#endif // RALINK_ATE //
			{
			// according to Andy, Gary, David require.
			// the command bbp shall read/write BBP register directly for dubug.
				BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
			sprintf(msg+strlen(msg), "R%02d[0x%02X]:%02X    ", bbpId, bbpId, regBBP);
			if (bbpId%5 == 4)
				sprintf(msg+strlen(msg), "\n");
			}
		}
	}
	else
	{
	}

	if (bCopyMsg)
	{
		// Copy the information into the user buffer
		wrq->u.data.length = strlen(msg);
		Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	}

	if (!bAllowDump)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("strlen(msg)=%d\n", (UINT32)strlen(msg)));
		DBGPRINT(RT_DEBUG_OFF, ("%s\n", msg));
	}

	kfree(mpool);
	if (!bAllowDump)
		DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlBBP\n\n"));
}


/* 
    ==========================================================================
    Description:
        Read / Write MAC
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 mac 0        ==> read MAC where Addr=0x0
               2.) iwpriv ra0 mac 0=12     ==> write MAC where Addr=0x0, value=12
    ==========================================================================
*/
VOID RTMPAPIoctlMAC(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	struct iwreq	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	INT					j = 0, k = 0;
	PSTRING				mpool, msg; //msg[1024];
	PSTRING				arg; //arg[255];
	PSTRING				ptr;
	UINT32				macAddr = 0;
	UCHAR				temp[16];
	STRING				temp2[16];
	UINT32				macValue;
	BOOLEAN				bIsPrintAllMAC = FALSE, bFromUI;


	mpool = (PSTRING)kmalloc(sizeof(CHAR)*(4096+256+12), MEM_ALLOC_FLAG);

	if (mpool == NULL) {
		return;
	}

	bFromUI = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_UI) == RTPRIV_IOCTL_FLAG_UI) ? TRUE : FALSE;
	
	msg = (PSTRING)((ULONG)(mpool+3) & (ULONG)~0x03);
	arg = (PSTRING)((ULONG)(msg+4096+3) & (ULONG)~0x03);

	memset(msg, 0x00, 4096);
	memset(arg, 0x00, 256);	
	if ((wrq->u.data.length > 1) //No parameters.
		)
	{
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		ptr = arg;
		sprintf(msg, "\n");
		//Parsing Read or Write
		while ((this_char = strsep((char **)&ptr, ",")) != NULL)
		{
			if (!*this_char)
				continue;

			if ((value = strchr(this_char, '=')) != NULL)
				*value++ = 0;

			if (!value || !*value)
			{ //Read
				// Sanity check
				if(strlen(this_char) > 4)
					break;

				j = strlen(this_char);
				while(j-- > 0)
				{
					if(this_char[j] > 'f' || this_char[j] < '0')
						goto done; //return;
				}

				// Mac Addr
				k = j = strlen(this_char);
				while(j-- > 0)
				{
					this_char[4-k+j] = this_char[j];
				}
				
				while(k < 4)
					this_char[3-k++]='0';
				this_char[4]='\0';

				if(strlen(this_char) == 4)
				{
					AtoH(this_char, temp, 2);
					macAddr = *temp*256 + temp[1];					
					if (macAddr < 0xFFFF)
					{
						RTMP_IO_READ32(pAdapter, macAddr, &macValue);
					    if (!bFromUI)
						DBGPRINT(RT_DEBUG_OFF, ("MacAddr=0x%x, MacValue=0x%x\n", macAddr, macValue));
						sprintf(msg+strlen(msg), "[0x%08x]:%08x  ", macAddr , macValue);
					}
					else
					{//Invalid parametes, so default printk all bbp
						break;
					}
				}
			}
			else
			{ //Write
				NdisMoveMemory(&temp2, value, strlen(value));
				temp2[strlen(value)] = '\0';

				// Sanity check
				if((strlen(this_char) > 4) || strlen(temp2) > 8)
					break;

				j = strlen(this_char);
				while(j-- > 0)
				{
					if(this_char[j] > 'f' || this_char[j] < '0')
						goto done; //return;
				}

				j = strlen(temp2);
				while(j-- > 0)
				{
					if(temp2[j] > 'f' || temp2[j] < '0')
						goto done; //return;
				}

				//MAC Addr
				k = j = strlen(this_char);
				while(j-- > 0)
				{
					this_char[4-k+j] = this_char[j];
				}

				while(k < 4)
					this_char[3-k++]='0';
				this_char[4]='\0';

				//MAC value
				k = j = strlen(temp2);
				while(j-- > 0)
				{
					temp2[8-k+j] = temp2[j];
				}
				
				while(k < 8)
					temp2[7-k++]='0';
				temp2[8]='\0';

				{
					AtoH(this_char, temp, 2);
					macAddr = *temp*256 + temp[1];

					AtoH(temp2, temp, 4);
					macValue = *temp*256*256*256 + temp[1]*256*256 + temp[2]*256 + temp[3];

					// debug mode
					if (macAddr == (HW_DEBUG_SETTING_BASE + 4))
					{
						// 0x2bf4: byte0 non-zero: enable R66 tuning, 0: disable R66 tuning
                        if (macValue & 0x000000ff) 
                        {
                            pAdapter->BbpTuning.bEnable = TRUE;
                            DBGPRINT(RT_DEBUG_TRACE, ("turn on R17 tuning\n"));
                        }
                        else
                        {
                            UCHAR R66;
                            pAdapter->BbpTuning.bEnable = FALSE;
                            R66 = 0x26 + GET_LNA_GAIN(pAdapter);
                            // todo RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, (0x26 + GET_LNA_GAIN(pAd)));
				if (!bFromUI)
					DBGPRINT(RT_DEBUG_TRACE, ("turn off R66 tuning, restore to 0x%02x\n", R66));
                        }
				return;
					}
				    if (!bFromUI)
					DBGPRINT(RT_DEBUG_OFF, ("MacAddr=%02x, MacValue=0x%x\n", macAddr, macValue));
					
					RTMP_IO_WRITE32(pAdapter, macAddr, macValue);
					sprintf(msg+strlen(msg), "[0x%08x]:%08x  ", macAddr, macValue);
				}
			}
		}
	} else {
		UINT32 IdMac;

		for(IdMac=0; IdMac<=0x1500; IdMac+=4)
		{
			if ((IdMac & 0x0f) == 0)
			{
				DBGPRINT(RT_DEBUG_OFF, ("\n0x%04x: ", IdMac));
			}

			RTMP_IO_READ32(pAdapter, IdMac, &macValue);
			DBGPRINT(RT_DEBUG_OFF, ("%08x ", macValue));
		}

		bIsPrintAllMAC = TRUE;
	}

	
	if(strlen(msg) == 1)
		sprintf(msg+strlen(msg), "===>Error command format!");
	
#ifdef LINUX
	// Copy the information into the user buffer
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
	}
#endif // LINUX //




done:
	kfree(mpool);
	if (!bFromUI)	
		DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlMAC\n\n"));
}


#ifdef RTMP_RF_RW_SUPPORT
/* 
    ==========================================================================
    Description:
        Read / Write RF register
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 rf                ==> read all RF registers
               2.) iwpriv ra0 rf 1              ==> read RF where RegID=1
               3.) iwpriv ra0 rf 1=10		    ==> write RF R1=0x10
    ==========================================================================
*/
VOID RTMPAPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	UCHAR				regRF = 0;
	PSTRING				mpool, msg; //msg[2048];
	PSTRING				arg; //arg[255];
	PSTRING				ptr;
	INT					rfId, maxRFIdx = 31;
	LONG				rfValue;
	BOOLEAN				bIsPrintAllRF = FALSE, bFromUI;
	INT					memLen = sizeof(CHAR) * (2048+256+12);
	
#if defined (RT3883) || defined (RT3352)
	//if (IS_RT3883(pAdapter))
		maxRFIdx = 63;
#endif // RT3883 //		
		
	mpool = (PSTRING)kmalloc(memLen, MEM_ALLOC_FLAG);
	if (mpool == NULL) {
		return;
	}

	bFromUI = ((wrq->u.data.flags & RTPRIV_IOCTL_FLAG_UI) == RTPRIV_IOCTL_FLAG_UI) ? TRUE : FALSE;
	
	NdisZeroMemory(mpool, memLen);
	msg = (PSTRING)((ULONG)(mpool+3) & (ULONG)~0x03);
	arg = (PSTRING)((ULONG)(msg+2048+3) & (ULONG)~0x03);
	
	if ((wrq->u.data.length > 1) //No parameters.
		)
	{
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		ptr = arg;
		sprintf(msg, "\n");
		//Parsing Read or Write
		while ((this_char = strsep((char **)&ptr, ",")) != NULL)
		{
			if (!*this_char)
				continue;

			if ((value = strchr(this_char, '=')) != NULL)
				*value++ = 0;

			if (!value || !*value)
			{ //Read
				if (sscanf(this_char, "%d", &(rfId)) == 1)
				{
#if defined (RT3883) || defined (RT3352)
					if (rfId <= 63)
#else
					if (rfId <= 31)
#endif // defined (RT3883) || defined (RT3352) //
					{
#ifdef RALINK_ATE
						/*
							In RT2860 ATE mode, we do not load 8051 firmware.
							We must access BBP directly.
							For RT2870 ATE mode, ATE_BBP_IO_WRITE8(/READ8)_BY_REG_ID are redefined.
						*/
						if (ATE_ON(pAdapter))
						{
							ATE_RF_IO_READ8_BY_REG_ID(pAdapter, rfId, &regRF);
							// Sync with QA for comparation
							sprintf(msg+strlen(msg), "%03d = %02X\n", rfId, regRF);
						}
						else
#endif // RALINK_ATE //
						{
							// according to Andy, Gary, David require.
							// the command rf shall read rf register directly for dubug.
							// BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							RT30xxReadRFRegister(pAdapter, rfId, &regRF);
							sprintf(msg+strlen(msg), "R%02d[0x%02x]:%02X  ", rfId, rfId, regRF);
						}
					}
					else
					{
						//Invalid parametes, so default printk all RF
						bIsPrintAllRF = TRUE;
						break;
					}
				}
				else
				{
					//Invalid parametes, so default printk all RF
					bIsPrintAllRF = TRUE;
					break;
				}
			}
			else
			{ //Write
				if ((sscanf(this_char, "%d", &(rfId)) == 1) && (sscanf(value, "%lx", &(rfValue)) == 1))
				{
#if defined (RT3883) || defined (RT3352)
					if (rfId <= 63)
#else
					if (rfId <= 31)
#endif // defined (RT3883) || defined (RT3352) //
					{
#ifdef RALINK_ATE
						/*
							In RT2860 ATE mode, we do not load 8051 firmware.
							We must access BBP directly.
							For RT2870 ATE mode, ATE_BBP_IO_WRITE8(/READ8)_BY_REG_ID are redefined.
						*/
						if (ATE_ON(pAdapter))
						{
							ATE_RF_IO_READ8_BY_REG_ID(pAdapter, rfId, &regRF);
							ATE_RF_IO_WRITE8_BY_REG_ID(pAdapter, (UCHAR)rfId,(UCHAR) rfValue);

							// Read it back for showing.
							ATE_RF_IO_READ8_BY_REG_ID(pAdapter, rfId, &regRF);
							// Sync with QA for comparation
							sprintf(msg+strlen(msg), "%03d = %02X\n", rfId, regRF);
						}
						else
#endif // RALINK_ATE //
						{
							// according to Andy, Gary, David require.
							// the command RF shall read/write RF register directly for dubug.
							//BBP_IO_READ8_BY_REG_ID(pAdapter, bbpId, &regBBP);
							//BBP_IO_WRITE8_BY_REG_ID(pAdapter, (UCHAR)bbpId,(UCHAR) bbpValue);
							RT30xxReadRFRegister(pAdapter, rfId, &regRF);
								RT30xxWriteRFRegister(pAdapter, (UCHAR)rfId,(UCHAR) rfValue);
							//Read it back for showing
							RT30xxReadRFRegister(pAdapter, rfId, &regRF);
							sprintf(msg+strlen(msg), "R%02d[0x%02X]:%02X\n", rfId, rfId, regRF);
						}
					}
					else
					{	//Invalid parametes, so default printk all RF
						bIsPrintAllRF = TRUE;
						break;
					}
				}
				else
				{	//Invalid parametes, so default printk all RF
					bIsPrintAllRF = TRUE;
					break;
				}
			}
		}
	}
	else
		bIsPrintAllRF = TRUE;

	if (bIsPrintAllRF)
	{
		memset(msg, 0x00, 2048);
		sprintf(msg, "\n");
		for (rfId = 0; rfId <= maxRFIdx; rfId++)
		{
#ifdef RALINK_ATE
			/*
				In RT2860 ATE mode, we do not load 8051 firmware.
				We must access RF registers directly.
				For RT2870 ATE mode, ATE_RF_IO_WRITE8(/READ8)_BY_REG_ID are redefined.
			*/
			if (ATE_ON(pAdapter))
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAdapter, rfId, &regRF);
				// Sync with QA for comparation
				sprintf(msg+strlen(msg), "%03d = %02X\n", rfId, regRF);
			}
			else
#endif // RALINK_ATE //
			{
				// according to Andy, Gary, David require.
				// the command RF shall read/write RF register directly for dubug.
				RT30xxReadRFRegister(pAdapter, rfId, &regRF);
				sprintf(msg+strlen(msg), "R%02d[0x%02X]:%02X    ", rfId, rfId*2, regRF);
				if (rfId%5 == 4)
					sprintf(msg+strlen(msg), "\n");
			}
		}
		// Copy the information into the user buffer

#ifdef LINUX
		wrq->u.data.length = strlen(msg);
		if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
		}
#endif // LINUX //
	}
	else
	{
#ifdef LINUX
		// Copy the information into the user buffer
		wrq->u.data.length = strlen(msg);
		if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
		}
#endif // LINUX //
	}
	
	if (!bFromUI)
		DBGPRINT(RT_DEBUG_TRACE, ("Dump RF msg[%d]=%s]\n", strlen(msg), msg));
	
	kfree(mpool);
	if (!bFromUI)
		DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlRF\n\n"));
	
}
#endif // RTMP_RF_RW_SUPPORT //
#endif //#ifdef DBG

/* 
    ==========================================================================
    Description:
        Read / Write E2PROM
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 e2p 0     	==> read E2PROM where Addr=0x0
               2.) iwpriv ra0 e2p 0=1234    ==> write E2PROM where Addr=0x0, value=1234
    ==========================================================================
*/
VOID RTMPAPIoctlE2PROM(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	struct iwreq	*wrq)
{
	PSTRING				this_char;
	PSTRING				value;
	INT					j = 0, k = 0;
	PSTRING				mpool, msg;//msg[1024];
	PSTRING				arg; //arg[255];
	PSTRING				ptr;
	USHORT				eepAddr = 0;
	UCHAR				temp[16];
	STRING				temp2[16];
	USHORT				eepValue;
	BOOLEAN				bIsPrintAllE2PROM = FALSE;

	mpool = (PSTRING)kmalloc(sizeof(CHAR)*(4096+256+12), MEM_ALLOC_FLAG);

	if (mpool == NULL) {
		return;
	}

	msg = (PSTRING)((ULONG)(mpool+3) & (ULONG)~0x03);
	arg = (PSTRING)((ULONG)(msg+4096+3) & (ULONG)~0x03);


	memset(msg, 0x00, 4096);
	memset(arg, 0x00, 256);		
	if ((wrq->u.data.length > 1) //No parameters.
		)
	{
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		ptr = arg;
		sprintf(msg, "\n");
		//Parsing Read or Write
		while ((this_char = strsep((char **)&ptr, ",")) != NULL)
		{
			if (!*this_char)
				continue;

			if ((value = strchr(this_char, '=')) != NULL)
				*value++ = 0;

			if (!value || !*value)
			{ //Read

				// Sanity check
				if(strlen(this_char) > 4)
					break;

				j = strlen(this_char);
				while(j-- > 0)
				{
					if(this_char[j] > 'f' || this_char[j] < '0')
						goto done; //return;
				}

				// E2PROM addr
				k = j = strlen(this_char);
				while(j-- > 0)
				{
					this_char[4-k+j] = this_char[j];
				}
				
				while(k < 4)
					this_char[3-k++]='0';
				this_char[4]='\0';

				if(strlen(this_char) == 4)
				{
					AtoH(this_char, temp, 2);
					eepAddr = *temp*256 + temp[1];					
					if (eepAddr < 0xFFFF)
					{
						RT28xx_EEPROM_READ16(pAdapter, eepAddr, eepValue);
						sprintf(msg+strlen(msg), "[0x%04X]:0x%04X  ", eepAddr , eepValue);
					}
					else
					{//Invalid parametes, so default printk all bbp
						break;
					}
				}
			}
			else
			{ //Write
				NdisMoveMemory(&temp2, value, strlen(value));
				temp2[strlen(value)] = '\0';

				// Sanity check
				if((strlen(this_char) > 4) || strlen(temp2) > 8)
					break;

				j = strlen(this_char);
				while(j-- > 0)
				{
					if(this_char[j] > 'f' || this_char[j] < '0')
						goto done; //return;
				}
				j = strlen(temp2);
				while(j-- > 0)
				{
					if(temp2[j] > 'f' || temp2[j] < '0')
						goto done; //return;
				}

				//MAC Addr
				k = j = strlen(this_char);
				while(j-- > 0)
				{
					this_char[4-k+j] = this_char[j];
				}

				while(k < 4)
					this_char[3-k++]='0';
				this_char[4]='\0';

				//MAC value
				k = j = strlen(temp2);
				while(j-- > 0)
				{
					temp2[4-k+j] = temp2[j];
				}
				
				while(k < 4)
					temp2[3-k++]='0';
				temp2[4]='\0';

				AtoH(this_char, temp, 2);
				eepAddr = *temp*256 + temp[1];

				AtoH(temp2, temp, 2);
				eepValue = *temp*256 + temp[1];

				RT28xx_EEPROM_WRITE16(pAdapter, eepAddr, eepValue);
				sprintf(msg+strlen(msg), "[0x%02X]:%02X  ", eepAddr, eepValue);
			}
		}
	} else {
		bIsPrintAllE2PROM = TRUE;
	}

	if (bIsPrintAllE2PROM)
	{
		sprintf(msg, "\n");
		
		/* E2PROM Registers */
		for (eepAddr = 0x00; eepAddr < 0x200; eepAddr += 2)
		{
			RT28xx_EEPROM_READ16(pAdapter, eepAddr, eepValue);
			sprintf(msg+strlen(msg), "[0x%04X]:%04X  ", eepAddr , eepValue);
			//if ((eepAddr & 0x7) == 0x7)
			if (eepAddr % 12 == 10)
				sprintf(msg+strlen(msg), "\n");
		}
	}

	if(strlen(msg) == 1)
		sprintf(msg+strlen(msg), "===>Error command format!");

	// Copy the information into the user buffer


#ifdef LINUX
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
	}
#endif // LINUX //

done:	
	kfree(mpool);
    if (wrq->u.data.flags != RT_OID_802_11_HARDWARE_REGISTER)	
	DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlE2PROM\n"));
}

#ifdef RANGE_EXT_SUPPORT
#define ENHANCED_STAT_DISPLAY	// Display PER and PLR statistics
#endif // RANGE_EXT_SUPPORT //

/* 
    ==========================================================================
    Description:
        Read statistics counter
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 stat 0     	==> Read statistics counter
    ==========================================================================
*/
VOID RTMPIoctlStatistics(
	IN PRTMP_ADAPTER pAd, 
	IN struct iwreq *wrq)
{
	INT					Status;
	PSTRING             msg;
#ifdef WSC_AP_SUPPORT
    UCHAR               idx = 0;
#endif // WSC_AP_SUPPORT //
	ULONG txCount = 0;
#ifdef ENHANCED_STAT_DISPLAY
	ULONG per, plr;
	int i;
#endif

	msg = (PSTRING)kmalloc(sizeof(CHAR)*(2048), MEM_ALLOC_FLAG);
	if (msg == NULL) {
		return;
	}


    memset(msg, 0x00, 1600);
    sprintf(msg, "\n");

#ifdef RALINK_ATE
	if(ATE_ON(pAd))
	{
		txCount = pAd->ate.TxDoneCount;
	}
	else
#endif // RALINK_ATE //
	{
		txCount = pAd->WlanCounters.TransmittedFragmentCount.u.LowPart;
	}

    sprintf(msg+strlen(msg), "Tx success                      = %ld\n", txCount);
#ifdef ENHANCED_STAT_DISPLAY
	per = txCount==0? 0: 1000*(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart)/(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart+txCount);
    sprintf(msg+strlen(msg), "Tx retry count                  = %ld, PER=%ld.%1ld%%\n",
									(ULONG)pAd->WlanCounters.RetryCount.u.LowPart,
									per/10, per % 10);
	plr = txCount==0? 0: 10000*pAd->WlanCounters.FailedCount.u.LowPart/(pAd->WlanCounters.FailedCount.u.LowPart+txCount);
    sprintf(msg+strlen(msg), "Tx fail to Rcv ACK after retry  = %ld, PLR=%ld.%02ld%%\n",
									(ULONG)pAd->WlanCounters.FailedCount.u.LowPart, plr/100, plr%100);

    sprintf(msg+strlen(msg), "Rx success                      = %ld\n", (ULONG)pAd->WlanCounters.ReceivedFragmentCount.QuadPart);
	per = pAd->WlanCounters.ReceivedFragmentCount.u.LowPart==0? 0: 1000*(pAd->WlanCounters.FCSErrorCount.u.LowPart)/(pAd->WlanCounters.FCSErrorCount.u.LowPart+pAd->WlanCounters.ReceivedFragmentCount.u.LowPart);
    sprintf(msg+strlen(msg), "Rx with CRC                     = %ld, PER=%ld.%1ld%%\n",
										(ULONG)pAd->WlanCounters.FCSErrorCount.u.LowPart, per/10, per % 10);
	sprintf(msg+strlen(msg), "Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer);
	sprintf(msg+strlen(msg), "Rx duplicate frame              = %ld\n", (ULONG)pAd->WlanCounters.FrameDuplicateCount.u.LowPart);

	sprintf(msg+strlen(msg), "False CCA                       = %ld\n", (ULONG)pAd->RalinkCounters.FalseCCACnt);
#else
    sprintf(msg+strlen(msg), "Tx retry count          		  = %ld\n", (ULONG)pAd->WlanCounters.RetryCount.u.LowPart);
	sprintf(msg+strlen(msg), "Tx fail to Rcv ACK after retry  = %ld\n", (ULONG)pAd->WlanCounters.FailedCount.u.LowPart);
	sprintf(msg+strlen(msg), "RTS Success Rcv CTS             = %ld\n", (ULONG)pAd->WlanCounters.RTSSuccessCount.u.LowPart);
	sprintf(msg+strlen(msg), "RTS Fail Rcv CTS                = %ld\n", (ULONG)pAd->WlanCounters.RTSFailureCount.u.LowPart);

    sprintf(msg+strlen(msg), "Rx success                      = %ld\n", (ULONG)pAd->WlanCounters.ReceivedFragmentCount.QuadPart);
    sprintf(msg+strlen(msg), "Rx with CRC                     = %ld\n", (ULONG)pAd->WlanCounters.FCSErrorCount.u.LowPart);
    sprintf(msg+strlen(msg), "Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer);
    sprintf(msg+strlen(msg), "Rx duplicate frame              = %ld\n", (ULONG)pAd->WlanCounters.FrameDuplicateCount.u.LowPart);

    sprintf(msg+strlen(msg), "False CCA (one second)          = %ld\n", (ULONG)pAd->RalinkCounters.OneSecFalseCCACnt);
#endif // ENHANCED_STAT_DISPLAY //

#ifdef RALINK_ATE
	if(ATE_ON(pAd))
	{
		if (pAd->ate.RxAntennaSel == 0)
		{
    		sprintf(msg+strlen(msg), "RSSI-A                          = %ld\n", (LONG)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta));
			sprintf(msg+strlen(msg), "RSSI-B (if available)           = %ld\n", (LONG)(pAd->ate.LastRssi1 - pAd->BbpRssiToDbmDelta));
			sprintf(msg+strlen(msg), "RSSI-C (if available)           = %ld\n\n", (LONG)(pAd->ate.LastRssi2 - pAd->BbpRssiToDbmDelta));
		}
		else
		{
    		sprintf(msg+strlen(msg), "RSSI                            = %ld\n", (LONG)(pAd->ate.LastRssi0 - pAd->BbpRssiToDbmDelta));
		}
	}
	else
#endif // RALINK_ATE //
	{
#ifdef ENHANCED_STAT_DISPLAY
    	sprintf(msg+strlen(msg), "RSSI                            = %ld %ld %ld\n",
    			(LONG)(pAd->ApCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta),
    			(LONG)(pAd->ApCfg.RssiSample.LastRssi1 - pAd->BbpRssiToDbmDelta),
    			(LONG)(pAd->ApCfg.RssiSample.LastRssi2 - pAd->BbpRssiToDbmDelta));

    	// Display Last Rx Rate and BF SNR of first Associated entry in MAC table
    	if (pAd->MacTab.Size > 0)
    	{
    		static char *phyMode[4] = {"CCK", "OFDM", "MM", "GF"};

    		for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
			{
    			PMAC_TABLE_ENTRY pEntry = &(pAd->MacTab.Content[i]);
    			if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst==SST_ASSOC)
				{
					UINT32 lastRxRate = pEntry->LastRxRate;

					sprintf(msg+strlen(msg), "Last RX Rate                    = MCS %d, %2dM, %cGI, %s%s\n",
							lastRxRate & 0x7F,  ((lastRxRate>>7) & 0x1)? 40: 20,
							((lastRxRate>>8) & 0x1)? 'S': 'L',
							phyMode[(lastRxRate>>14) & 0x3],
							((lastRxRate>>9) & 0x3)? ", STBC": " ");

#if defined(RT2883) || defined(RT3883)
					sprintf(msg+strlen(msg), "BF SNR                          = %d.%02d, %d.%02d, %d.%02d  FO:%02X\n",
							pEntry->BF_SNR[0]/4, (pEntry->BF_SNR[0] % 4)*25,
							pEntry->BF_SNR[1]/4, (pEntry->BF_SNR[1] % 4)*25,
							pEntry->BF_SNR[2]/4, (pEntry->BF_SNR[2] % 4)*25,
							pEntry->freqOffset & 0xFF);
#endif // defined(RT2883) || defined(RT3883) //
					break;
				}
			}
    	}
#else
    	sprintf(msg+strlen(msg), "RSSI-A                          = %ld\n", (LONG)(pAd->ApCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta));
		sprintf(msg+strlen(msg), "RSSI-B (if available)           = %ld\n", (LONG)(pAd->ApCfg.RssiSample.LastRssi1 - pAd->BbpRssiToDbmDelta));
		sprintf(msg+strlen(msg), "RSSI-C (if available)           = %ld\n\n", (LONG)(pAd->ApCfg.RssiSample.LastRssi2 - pAd->BbpRssiToDbmDelta));
#endif // ENHANCED_STAT_DISPLAY //
	}

#ifdef WSC_AP_SUPPORT
	sprintf(msg+strlen(msg), "WPS Information:\n");
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	// display pin code
		sprintf(msg+strlen(msg), "Enrollee PinCode(ra%d)           %08u\n", idx, pAd->ApCfg.MBSSID[idx].WscControl.WscEnrolleePinCode);
#ifdef APCLI_SUPPORT
    sprintf(msg+strlen(msg), "\n");
    sprintf(msg+strlen(msg), "Enrollee PinCode(ApCli0)        %08u\n", pAd->ApCfg.ApCliTab[0].WscControl.WscEnrolleePinCode);
    sprintf(msg+strlen(msg), "Ap Client WPS Profile Count     = %d\n", pAd->ApCfg.ApCliTab[0].WscControl.WscProfile.ProfileCnt);
    for (idx = 0; idx < pAd->ApCfg.ApCliTab[0].WscControl.WscProfile.ProfileCnt ; idx++)
    {
        PWSC_CREDENTIAL pCredential = &pAd->ApCfg.ApCliTab[0].WscControl.WscProfile.Profile[idx];
        sprintf(msg+strlen(msg), "Profile[%d]:\n", idx);        
        sprintf(msg+strlen(msg), "SSID                            = %s\n", pCredential->SSID.Ssid);
        sprintf(msg+strlen(msg), "AuthType                        = %s\n", WscGetAuthTypeStr(pCredential->AuthType));
        sprintf(msg+strlen(msg), "EncrypType                      = %s\n", WscGetEncryTypeStr(pCredential->EncrType)); 
        sprintf(msg+strlen(msg), "KeyIndex                        = %d\n", pCredential->KeyIndex);
        if (pCredential->KeyLength != 0)
        {
            sprintf(msg+strlen(msg), "Key                             = %s\n", pCredential->Key);
        }
    }
    sprintf(msg+strlen(msg), "\n");
#endif // APCLI_SUPPORT //
#endif // WSC_AP_SUPPORT //
    
    // Copy the information into the user buffer
    wrq->u.data.length = strlen(msg);
    Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);

	kfree(msg);

#if defined(TXBF_SUPPORT) && defined(ENHANCED_STAT_DISPLAY)
	// Debug code to display BF statistics
	if (pAd->CommonCfg.DebugFlags & DBF_SHOW_BF_STATS)
	{
		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++) {
			PMAC_TABLE_ENTRY pEntry = &(pAd->MacTab.Content[i]);
			COUNTER_TXBF *pCnt;
			ULONG totalNBF, totalEBF, totalIBF, totalTx, totalRetry, totalSuccess;

			if (!IS_ENTRY_CLIENT(pEntry) || pEntry->Sst!=SST_ASSOC)
				continue;

			pCnt = &pEntry->TxBFCounters;

			totalNBF = pCnt->TxSuccessCount + pCnt->TxFailCount;
			totalEBF = pCnt->ETxSuccessCount + pCnt->ETxFailCount;
			totalIBF = pCnt->ITxSuccessCount + pCnt->ITxFailCount;

			totalTx = totalNBF + totalEBF + totalIBF;
			totalRetry = pCnt->TxRetryCount + pCnt->ETxRetryCount + pCnt->ITxRetryCount;
			totalSuccess = pCnt->TxSuccessCount + pCnt->ETxSuccessCount + pCnt->ITxSuccessCount;

			DBGPRINT(RT_DEBUG_OFF, ("MacTable[%d]     Success    Retry/PER    Fail/PLR\n", i) );
			if (totalTx==0) {
				DBGPRINT(RT_DEBUG_OFF, ("   Total = 0\n") );
				continue;
			}

			if (totalNBF!=0) {
				DBGPRINT(RT_DEBUG_OFF, ("   NonBF (%3lu%%): %7lu  %7lu (%2lu%%) %5lu (%1lu%%)\n",
					100*totalNBF/totalTx, pCnt->TxSuccessCount,
					pCnt->TxRetryCount, 100*pCnt->TxRetryCount/(pCnt->TxSuccessCount+pCnt->TxRetryCount),
					pCnt->TxFailCount, 100*pCnt->TxFailCount/totalNBF) );
			}

			if (totalEBF!=0) {
				DBGPRINT(RT_DEBUG_OFF, ("   ETxBF (%3lu%%): %7lu  %7lu (%2lu%%) %5lu (%1lu%%)\n",
					 100*totalEBF/totalTx, pCnt->ETxSuccessCount,
					pCnt->ETxRetryCount, 100*pCnt->ETxRetryCount/(pCnt->ETxSuccessCount+pCnt->ETxRetryCount),
					pCnt->ETxFailCount, 100*pCnt->ETxFailCount/totalEBF) );
			}

			if (totalIBF!=0) {
				DBGPRINT(RT_DEBUG_OFF, ("   ITxBF (%3lu%%): %7lu  %7lu (%2lu%%) %5lu (%1lu%%)\n",
					100*totalIBF/totalTx, pCnt->ITxSuccessCount,
					pCnt->ITxRetryCount, 100*pCnt->ITxRetryCount/(pCnt->ITxSuccessCount+pCnt->ITxRetryCount),
					pCnt->ITxFailCount, 100*pCnt->ITxFailCount/totalIBF) );
			}

			DBGPRINT(RT_DEBUG_OFF, ("   Total         %7lu  %7lu (%2lu%%) %5lu (%1lu%%)\n",
				totalSuccess, totalRetry, 100*totalRetry/(totalSuccess + totalRetry),
				pCnt->TxFailCount+pCnt->ETxFailCount+pCnt->ITxFailCount,
				100*(pCnt->TxFailCount+pCnt->ETxFailCount+pCnt->ITxFailCount)/totalTx) );
		}
	}
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("<==RTMPIoctlStatistics\n"));
}

#ifdef DOT11_N_SUPPORT
/* 
    ==========================================================================
    Description:
        Get Block ACK Table
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage:
        		1.) iwpriv ra0 get_ba_table
        		3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	//char *msg;
	UCHAR	TotalEntry, i, j, index;
	QUERYBA_TABLE		*BAT;

	BAT = vmalloc(sizeof(QUERYBA_TABLE));

	RTMPZeroMemory(BAT, sizeof(QUERYBA_TABLE));

	TotalEntry = pAd->MacTab.Size;
	index = 0;
	for (i=0; ((i < MAX_LEN_OF_MAC_TABLE) && (TotalEntry > 0)); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->TXBAbitmap))
		{
			NdisMoveMemory(BAT->BAOriEntry[index].MACAddr, pEntry->Addr, 6);
			for (j=0;j<8;j++)
			{
				if (pEntry->BAOriWcidArray[j] != 0)
					BAT->BAOriEntry[index].BufSize[j] = pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]].BAWinSize;
				else
					BAT->BAOriEntry[index].BufSize[j] = 0;
			}

			TotalEntry--;
			index++;
			BAT->OriNum++;
		}
	}

	TotalEntry = pAd->MacTab.Size;
	index = 0;
	for (i=0; ((i < MAX_LEN_OF_MAC_TABLE) && (TotalEntry > 0)); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->RXBAbitmap))
		{
			NdisMoveMemory(BAT->BARecEntry[index].MACAddr, pEntry->Addr, 6);
			BAT->BARecEntry[index].BaBitmap = (UCHAR)pEntry->RXBAbitmap;
			for (j = 0; j < 8; j++)
			{
				if (pEntry->BARecWcidArray[j] != 0)
					BAT->BARecEntry[index].BufSize[j] = pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]].BAWinSize;
				else
					BAT->BARecEntry[index].BufSize[j] = 0;
			}

			TotalEntry--;
			index++;
			BAT->RecNum++;
		}
	}

	wrq->u.data.length = sizeof(QUERYBA_TABLE);

	if (copy_to_user(wrq->u.data.pointer, BAT, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	vfree(BAT);

}
#endif // DOT11_N_SUPPORT //


#ifdef APCLI_SUPPORT
INT Set_ApCli_Enable_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	
	Enable = simple_strtol(arg, 0, 16);

	pAd->ApCfg.ApCliTab[ifIndex].Enable = (Enable > 0) ? TRUE : FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) Set_ApCli_Enable_Proc::(enable = %d)\n", ifIndex, pAd->ApCfg.ApCliTab[ifIndex].Enable));
	
	ApCliIfDown(pAd);

	return TRUE;
}

INT Set_ApCli_Ssid_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	INT success = FALSE;
	//UCHAR keyMaterial[40];
	UCHAR PskKey[100];

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	
	if(strlen(arg) <= MAX_LEN_OF_SSID)
	{
		apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;

		// bring apcli interface down first
		if(apcliEn == TRUE )
		{
			pAd->ApCfg.ApCliTab[ifIndex].Enable = FALSE;
			ApCliIfDown(pAd);
		}

		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, arg, strlen(arg));
		pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = (UCHAR)strlen(arg);
		success = TRUE;

		// Upadte PMK and restart WPAPSK state machine for ApCli link
		if (((pAd->ApCfg.ApCliTab[ifIndex].AuthMode == Ndis802_11AuthModeWPAPSK) ||
				(pAd->ApCfg.ApCliTab[ifIndex].AuthMode == Ndis802_11AuthModeWPA2PSK)) && 
					pAd->ApCfg.ApCliTab[ifIndex].PSKLen > 0)
		{
			NdisZeroMemory(PskKey, 100);
			NdisMoveMemory(PskKey, pAd->ApCfg.ApCliTab[ifIndex].PSK, pAd->ApCfg.ApCliTab[ifIndex].PSKLen);

			RT_CfgSetWPAPSKKey(pAd, (PSTRING)PskKey, 
									(PUCHAR)pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, 
									pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen, 
									pAd->ApCfg.ApCliTab[ifIndex].PMK);
		}

		DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) Set_ApCli_Ssid_Proc::(Len=%d,Ssid=%s)\n", ifIndex,
			pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen, pAd->ApCfg.ApCliTab[ifIndex].CfgSsid));

		pAd->ApCfg.ApCliTab[ifIndex].Enable = apcliEn;
	}
	else
		success = FALSE;

	return success;
}


INT Set_ApCli_Bssid_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	INT i;
	PSTRING value;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	POS_COOKIE pObj;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	
	apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;

	// bring apcli interface down first
	if(apcliEn == TRUE )
	{
		pAd->ApCfg.ApCliTab[ifIndex].Enable = FALSE;
		ApCliIfDown(pAd);
	}

	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, MAC_ADDR_LEN);

	if(strlen(arg) == 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
	{
		for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"), i++) 
		{
			if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
				return FALSE;  //Invalid

			AtoH(value, &pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[i], 1);
		}

		if(i != 6)
			return FALSE;  //Invalid
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ApCli_Bssid_Proc (%2X:%2X:%2X:%2X:%2X:%2X)\n",
		pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[0],
		pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[1],
		pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[2],
		pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[3],
		pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[4],
		pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid[5]));

	pAd->ApCfg.ApCliTab[ifIndex].Enable = apcliEn;

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set ApCli-IF Authentication mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_AuthMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG       i;
	POS_COOKIE 	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR 		ifIndex;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if ((strncmp(arg, "WEPAUTO", 7) == 0) || (strncmp(arg, "wepauto", 7) == 0))
		pAd->ApCfg.ApCliTab[ifIndex].AuthMode = Ndis802_11AuthModeAutoSwitch;
	else if ((strncmp(arg, "SHARED", 6) == 0) || (strncmp(arg, "shared", 6) == 0))
		pAd->ApCfg.ApCliTab[ifIndex].AuthMode = Ndis802_11AuthModeShared;
	else if ((strncmp(arg, "WPAPSK", 6) == 0) || (strncmp(arg, "wpapsk", 6) == 0))
		pAd->ApCfg.ApCliTab[ifIndex].AuthMode = Ndis802_11AuthModeWPAPSK;
	else if ((strncmp(arg, "WPA2PSK", 7) == 0) || (strncmp(arg, "wpa2psk", 7) == 0))
		pAd->ApCfg.ApCliTab[ifIndex].AuthMode = Ndis802_11AuthModeWPA2PSK;
	else
		pAd->ApCfg.ApCliTab[ifIndex].AuthMode = Ndis802_11AuthModeOpen;

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		if (IS_ENTRY_APCLI(&pAd->MacTab.Content[i]))
		{
			pAd->MacTab.Content[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		}
	}
		
    RTMPMakeRSNIE(pAd, pAd->ApCfg.ApCliTab[ifIndex].AuthMode, pAd->ApCfg.ApCliTab[ifIndex].WepStatus, (ifIndex + MIN_NET_DEVICE_FOR_APCLI));

	pAd->ApCfg.ApCliTab[ifIndex].DefaultKeyId  = 0;

	if(pAd->ApCfg.ApCliTab[ifIndex].AuthMode >= Ndis802_11AuthModeWPA)
		pAd->ApCfg.ApCliTab[ifIndex].DefaultKeyId = 1;

	DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_ApCli_AuthMode_Proc::(AuthMode=%d)\n", ifIndex, pAd->ApCfg.ApCliTab[ifIndex].AuthMode));		
	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set ApCli-IF Encryption Type
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_EncrypType_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR 		ifIndex;
	PAPCLI_STRUCT   pApCliEntry = NULL;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	pApCliEntry->WepStatus = Ndis802_11WEPDisabled; 
	if ((strncmp(arg, "WEP", 3) == 0) || (strncmp(arg, "wep", 3) == 0))
    {
		if (pApCliEntry->AuthMode < Ndis802_11AuthModeWPA)
			pApCliEntry->WepStatus = Ndis802_11WEPEnabled;				  
	}
	else if ((strncmp(arg, "TKIP", 4) == 0) || (strncmp(arg, "tkip", 4) == 0))
	{
		if (pApCliEntry->AuthMode >= Ndis802_11AuthModeWPA)
			pApCliEntry->WepStatus = Ndis802_11Encryption2Enabled;                       
    }
	else if ((strncmp(arg, "AES", 3) == 0) || (strncmp(arg, "aes", 3) == 0))
	{
		if (pApCliEntry->AuthMode >= Ndis802_11AuthModeWPA)
			pApCliEntry->WepStatus = Ndis802_11Encryption3Enabled;                            
	}    
	else
	{
		pApCliEntry->WepStatus = Ndis802_11WEPDisabled;                 
	}

	pApCliEntry->PairCipher     = pApCliEntry->WepStatus;
	pApCliEntry->GroupCipher    = pApCliEntry->WepStatus;
	pApCliEntry->bMixCipher		= FALSE;

	if (pApCliEntry->WepStatus >= Ndis802_11Encryption2Enabled)
		pApCliEntry->DefaultKeyId = 1;

	RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (ifIndex + MIN_NET_DEVICE_FOR_APCLI));
	DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_ApCli_EncrypType_Proc::(EncrypType=%d)\n", ifIndex, pApCliEntry->WepStatus));

	return TRUE;
}



/* 
    ==========================================================================
    Description:
        Set Default Key ID
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_DefaultKeyID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ULONG 			KeyIdx;
	POS_COOKIE 		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR 			ifIndex;
	PAPCLI_STRUCT   pApCliEntry = NULL;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	KeyIdx = simple_strtol(arg, 0, 10);
	if((KeyIdx >= 1 ) && (KeyIdx <= 4))
		pApCliEntry->DefaultKeyId = (UCHAR) (KeyIdx - 1 );
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_DefaultKeyID_Proc::(DefaultKeyID(0~3)=%d)\n", ifIndex, pApCliEntry->DefaultKeyId));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set WPA PSK key for ApCli link

    Arguments:
        pAdapter            Pointer to our adapter
        arg                 WPA pre-shared key string

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_WPAPSK_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR ifIndex;
	POS_COOKIE pObj;
	PAPCLI_STRUCT   pApCliEntry = NULL;
	INT retval;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ApCli_WPAPSK_Proc::(WPAPSK=%s)\n", arg));

	retval = RT_CfgSetWPAPSKKey(pAd, arg, (PUCHAR)pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, pApCliEntry->PMK);
	if (retval == FALSE)
		return FALSE;
	
	NdisMoveMemory(pApCliEntry->PSK, arg, strlen(arg));
	pApCliEntry->PSKLen = strlen(arg);

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY1 for ApCli-IF
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_Key1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 		pObj = (POS_COOKIE) pAd->OS_Cookie;
	PAPCLI_STRUCT	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;
	
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[0], 0);
	if(retVal == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_ApCli_Key1_Proc::(Key1=%s) success!\n", ifIndex, arg));
	
	return retVal;
	
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY2 for ApCli-IF
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_Key2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 		pObj;
	PAPCLI_STRUCT	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[1], 1);
	if(retVal == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_ApCli_Key2_Proc::(Key2=%s) success!\n", ifIndex, arg));
	
	return retVal;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY3 for ApCli-IF
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_Key3_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 		pObj;
	PAPCLI_STRUCT	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[2], 2);
	if(retVal == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_ApCli_Key3_Proc::(Key3=%s) success!\n", ifIndex, arg));
	
	return retVal;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY4 for ApCli-IF
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ApCli_Key4_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE 		pObj;
	PAPCLI_STRUCT	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[3], 3);
	if(retVal == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_ApCli_Key4_Proc::(Key4=%s) success!\n", ifIndex, arg));
	
	return retVal;
}

INT Set_ApCli_TxMode_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{	
	POS_COOKIE 		pObj;	
	UCHAR 			ifIndex;
	PAPCLI_STRUCT	pApCliEntry = NULL;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	pApCliEntry->DesiredTransmitSetting.field.FixedTxMode = 
								RT_CfgSetFixedTxPhyMode(arg);
	DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) Set_ApCli_TxMode_Proc = %d\n", ifIndex,
									pApCliEntry->DesiredTransmitSetting.field.FixedTxMode));

	return TRUE;
}

INT Set_ApCli_TxMcs_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	PAPCLI_STRUCT	pApCliEntry = NULL;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	pApCliEntry->DesiredTransmitSetting.field.MCS = 
			RT_CfgSetTxMCSProc(arg, &pApCliEntry->bAutoTxRateSwitch);

	if (pApCliEntry->DesiredTransmitSetting.field.MCS == MCS_AUTO)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) Set_ApCli_TxMcs_Proc = AUTO\n", ifIndex));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("I/F(apcli%d) Set_ApCli_TxMcs_Proc = %d\n", ifIndex, 
									pApCliEntry->DesiredTransmitSetting.field.MCS));
	}	

	return TRUE;
}

#ifdef WSC_AP_SUPPORT
INT Set_AP_WscSsid_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
    POS_COOKIE 		pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR			ifIndex = pObj->ioctl_if;
	PWSC_CTRL	    pWscControl = &pAd->ApCfg.ApCliTab[ifIndex].WscControl;

    if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;
    
	NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));

	if( (strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID))
    {
		NdisMoveMemory(pWscControl->WscSsid.Ssid, arg, strlen(arg));
		pWscControl->WscSsid.SsidLength = strlen(arg);

		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, arg, strlen(arg));
		pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = (UCHAR)strlen(arg);

		DBGPRINT(RT_DEBUG_TRACE, ("Set_WscSsid_Proc:: (Select SsidLen=%d,Ssid=%s)\n", 
				pWscControl->WscSsid.SsidLength, pWscControl->WscSsid.Ssid));
	}
	else
		return FALSE;	//Invalid argument 

	return TRUE;	

}
#endif // WSC_AP_SUPPORT
#endif // APCLI_SUPPORT //


#ifdef WSC_AP_SUPPORT
INT	 Set_AP_WscConfMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT         ConfModeIdx;
	//INT         IsAPConfigured;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
    BOOLEAN     bFromApCli = FALSE;
    PWSC_CTRL   pWscControl;

	ConfModeIdx = simple_strtol(arg, 0, 10);

#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//


#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
        bFromApCli = TRUE;
        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscConfMode_Proc:: This command is from apcli interface now.\n", apidx));
    }
    else
#endif // APCLI_SUPPORT //
    {
        bFromApCli = FALSE;
        pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscConfMode_Proc:: This command is from ra interface now.\n", apidx));
    }
        
    pWscControl->bWscTrigger = FALSE;
    if ((ConfModeIdx & WSC_ENROLLEE_PROXY_REGISTRAR) == WSC_DISABLE)
    {
        pWscControl->WscConfMode = WSC_DISABLE;
		pWscControl->WscStatus = STATUS_WSC_NOTUSED;
        if (bFromApCli)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscConfMode_Proc:: WPS is disabled.\n", apidx));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscConfMode_Proc:: WPS is disabled.\n", apidx));
            // Clear WPS IE in Beacon and ProbeResp
            pAd->ApCfg.MBSSID[apidx].WscIEBeacon.ValueLen = 0;
        	pAd->ApCfg.MBSSID[apidx].WscIEProbeResp.ValueLen = 0;
            APMakeAllBssBeacon(pAd);
            APUpdateAllBeaconFrame(pAd);
        }        
    }
    else
    {
        if (bFromApCli)
        {
            if (ConfModeIdx == WSC_ENROLLEE)
            {
                pWscControl->WscConfMode = WSC_ENROLLEE;
                WscInit(pAd, TRUE, apidx);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscConfMode_Proc:: Ap Client only supports Enrollee mode.(ConfModeIdx=%d)\n", apidx, ConfModeIdx));
                return FALSE;
            }
        }
        else
        {
        	pWscControl->WscConfMode = (ConfModeIdx & WSC_ENROLLEE_PROXY_REGISTRAR);
            WscInit(pAd, FALSE, apidx);
        }
        pWscControl->WscStatus = STATUS_WSC_IDLE;
    }

#ifdef APCLI_SUPPORT
    if (bFromApCli)
    {
        memcpy(mac_addr, &pAd->ApCfg.ApCliTab[apidx].CurrentAddress[0], MAC_ADDR_LEN);
    }
    else
#endif // APCLI_SUPPORT //        
    {
        memcpy(mac_addr, &pAd->ApCfg.MBSSID[apidx].Bssid[0], MAC_ADDR_LEN);
    }

	DBGPRINT(RT_DEBUG_TRACE, ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscConfMode_Proc::(WscConfMode(0~7)=%d)\n", 
                            mac_addr[0], 
                            mac_addr[1], 
                            mac_addr[2], 
                            mac_addr[3], 
                            mac_addr[4], 
                            mac_addr[5], 
                            pWscControl->WscConfMode));
	return TRUE;
}

INT	Set_AP_WscConfStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR       IsAPConfigured = 0;
	INT         IsSelectedRegistrar;
	USHORT      WscMode;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	    apidx = pObj->ioctl_if;

#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//


#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscConfStatus_Proc:: Ap Client doesn't need this command.\n", apidx));
        return FALSE;
    }
#endif // APCLI_SUPPORT //

	IsAPConfigured = (UCHAR)simple_strtol(arg, 0, 10);
	IsSelectedRegistrar = pAd->ApCfg.MBSSID[apidx].WscControl.WscSelReg;
    if (pAd->ApCfg.MBSSID[apidx].WscControl.WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

	if ((IsAPConfigured  > 0) && (IsAPConfigured  <= 2))
    {   
        pAd->ApCfg.MBSSID[apidx].WscControl.WscConfStatus = IsAPConfigured;
        // Change SC State of WPS IE in Beacon and ProbeResp
        WscBuildBeaconIE(pAd, IsAPConfigured, IsSelectedRegistrar, WscMode, 0, apidx);
    	WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, IsSelectedRegistrar, WscMode, 0, apidx);

    	APMakeAllBssBeacon(pAd);
    	APUpdateAllBeaconFrame(pAd);
    }
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscConfStatus_Proc:: Set failed!!(WscConfStatus=%s), WscConfStatus is 1 or 2 \n", apidx, arg));
        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscConfStatus_Proc:: WscConfStatus is not changed (%d) \n", apidx, pAd->ApCfg.MBSSID[apidx].WscControl.WscConfStatus));
		return FALSE;  //Invalid argument	
	}

	DBGPRINT(RT_DEBUG_TRACE, ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscConfStatus_Proc::(WscConfStatus=%d)\n", 
                               pAd->ApCfg.MBSSID[apidx].Bssid[0],
                               pAd->ApCfg.MBSSID[apidx].Bssid[1],
                               pAd->ApCfg.MBSSID[apidx].Bssid[2],
                               pAd->ApCfg.MBSSID[apidx].Bssid[3],
                               pAd->ApCfg.MBSSID[apidx].Bssid[4],
                               pAd->ApCfg.MBSSID[apidx].Bssid[5],
                               pAd->ApCfg.MBSSID[apidx].WscControl.WscConfStatus));

	return TRUE;
}

INT	Set_AP_WscMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT         WscMode;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
    PWSC_CTRL   pWscControl;
    BOOLEAN     bFromApCli = FALSE;
    
#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//
    
#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
        bFromApCli = TRUE;
        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscMode_Proc:: This command is from apcli interface now.\n", apidx));
    }
    else
#endif // APCLI_SUPPORT //
    {
        bFromApCli = FALSE;
        pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscMode_Proc:: This command is from ra interface now.\n", apidx));
    }

	WscMode = simple_strtol(arg, 0, 10);
    
    if ((WscMode  > 0) && (WscMode  <= 2))
    {
        pWscControl->WscMode = WscMode;
        if (WscMode == 2)
        {
	        WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
        }
    }
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WscMode_Proc:: Set failed!!(Set_WscMode_Proc=%s), WscConfStatus is 1 or 2 \n", arg));
        DBGPRINT(RT_DEBUG_TRACE, ("Set_WscMode_Proc:: WscMode is not changed (%d) \n", pWscControl->WscMode));
		return FALSE;  //Invalid argument
	}

#ifdef APCLI_SUPPORT
    if (bFromApCli)
    {
        memcpy(mac_addr, pAd->ApCfg.ApCliTab[apidx].CurrentAddress, MAC_ADDR_LEN);
    }
    else
#endif // APCLI_SUPPORT //        
    {
        memcpy(mac_addr, pAd->ApCfg.MBSSID[apidx].Bssid, MAC_ADDR_LEN);
    }
	DBGPRINT(RT_DEBUG_TRACE, ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscMode_Proc::(WscMode=%d)\n", 
                                mac_addr[0],
                                mac_addr[1],
                                mac_addr[2],
                                mac_addr[3],
                                mac_addr[4],
                                mac_addr[5],
                                pWscControl->WscMode));

	return TRUE;
}

INT	Set_WscStatus_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef DBG
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	    apidx = pObj->ioctl_if;
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscStatus_Proc::(WscStatus=%d)\n", apidx, pAd->ApCfg.MBSSID[apidx].WscControl.WscStatus));
#endif
	return TRUE;
}

#define WSC_GET_CONF_MODE_EAP	1
#define WSC_GET_CONF_MODE_UPNP	2
INT	Set_AP_WscGetConf_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT                 WscMode, wscGetConfMode = 0;
	INT                 IsAPConfigured;
	PWSC_CTRL           pWscControl;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo;
    INT	                idx;
    POS_COOKIE          pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	            apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
    BOOLEAN             bFromApCli = FALSE;
#ifdef APCLI_SUPPORT
	BOOLEAN 			apcliEn = pAd->ApCfg.ApCliTab[apidx].Enable;
#endif // APCLI_SUPPORT //

#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//

#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
    	if (apcliEn == FALSE)
    	{
    		DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscMode_Proc:: ApCli is disabled.\n", apidx));
    		return FALSE;
    	}
        bFromApCli = TRUE;
		apidx &= (~MIN_NET_DEVICE_FOR_APCLI);
        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscMode_Proc:: This command is from apcli interface now.\n", apidx));
    }
    else
#endif // APCLI_SUPPORT //
    {
        bFromApCli = FALSE;
        pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscMode_Proc:: This command is from ra interface now.\n", apidx));
    }

	wscGetConfMode = simple_strtol(arg, 0, 10);

    IsAPConfigured = pWscControl->WscConfStatus;
    pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;

    if (pWscControl->WscConfMode == WSC_DISABLE)
    {
        pWscControl->bWscTrigger = FALSE;
        DBGPRINT(RT_DEBUG_TRACE, ("Set_WscGetConf_Proc: WPS is disabled.\n"));
		return FALSE;
    }

	WscStop(pAd, bFromApCli, pWscControl);
    
	// trigger wsc re-generate public key
    pWscControl->RegData.ReComputePke = 1;

	if (pWscControl->WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
	{
		WscMode = DEV_PASS_ID_PBC;
    
		if (bFromApCli == FALSE)
		{
			WscPBCSessionOverlapCheck(pAd);			
			if (pAd->ApCfg.WscPBCOverlap == TRUE)
			{
				// PBC session overlap
				pWscControl->WscStatus = STATUS_WSC_PBC_SESSION_OVERLAP;
				RTMPSendWirelessEvent(pAd, IW_WSC_PBC_SESSION_OVERLAP, NULL, BSS0, 0); 
				DBGPRINT(RT_DEBUG_TRACE, ("Set_WscGetConf_Proc: PBC session overlap, abort this command!!\n"));
				return FALSE;
			}
		}
	}
    
	WscInitRegistrarPair(pAd, pWscControl, apidx);
    // Enrollee 192 random bytes for DH key generation
	for (idx = 0; idx < 192; idx++)
		pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);
    
#ifdef APCLI_SUPPORT
	if (bFromApCli)
    {
    	// bring apcli interface down first
		pAd->ApCfg.ApCliTab[apidx].Enable = FALSE;
		ApCliIfDown(pAd);
			
 		if (WscMode == DEV_PASS_ID_PIN)
    	{
			NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr,
	                       pAd->ApCfg.ApCliTab[apidx].CurrentAddress, 
	                       6);
	        
	        pAd->ApCfg.ApCliTab[apidx].Enable = apcliEn;
			memcpy(mac_addr, pAd->ApCfg.ApCliTab[apidx].CurrentAddress, MAC_ADDR_LEN);
    	}
		else
		{
			pWscControl->WscSsid.SsidLength = 0;
			NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
			pWscControl->WscPBCBssCount = 0;
			// WPS - SW PBC
			WscPushPBCAction(pAd);
		}
    }
	else
#endif // APCLI_SUPPORT //
	{
		WscBuildBeaconIE(pAd, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, apidx);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, apidx);

		APMakeAllBssBeacon(pAd);
		APUpdateAllBeaconFrame(pAd);
		memcpy(mac_addr, pAd->ApCfg.MBSSID[apidx].Bssid, MAC_ADDR_LEN);
	}

#ifdef APCLI_SUPPORT
	if (bFromApCli && (WscMode == DEV_PASS_ID_PBC))
		;
	else
#endif // APCLI_SUPPORT //
	{
	    // 2mins time-out timer
	    RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
	    pWscControl->Wsc2MinsTimerRunning = TRUE;
	    pWscControl->WscStatus = STATUS_WSC_LINK_UP;
		pWscControl->bWscTrigger = TRUE;
	}

    if (!bFromApCli)
		WscSendUPnPConfReqMsg(pAd, pWscControl->EntryIfIdx, 
	    							(PUCHAR)pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx].Ssid, 
	    							pAd->ApCfg.MBSSID[apidx].Bssid, 3, 0);

	DBGPRINT(RT_DEBUG_TRACE, ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscGetConf_Proc trigger WSC state machine, wscGetConfMode=%d\n", 
                                mac_addr[0],
                                mac_addr[1],
                                mac_addr[2],
                                mac_addr[3],
                                mac_addr[4],
                                mac_addr[5],
                                wscGetConfMode));

	return TRUE;
}

INT	Set_AP_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT        PinCode = 0;
	BOOLEAN     validatePin, bFromApCli = FALSE;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
    PWSC_CTRL   pWscControl;
#define IsZero(c) ('0' == (c) ? TRUE:FALSE)
	PinCode = simple_strtol(arg, 0, 10); // When PinCode is 03571361, return value is 3571361.

#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//

#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
        bFromApCli = TRUE;
        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscPinCode_Proc:: This command is from apcli interface now.\n", apidx));
    }
    else
#endif // APCLI_SUPPORT //
    {
        bFromApCli = FALSE;
        pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscPinCode_Proc:: This command is from ra interface now.\n", apidx));
    }
    
	if (strlen(arg) == 4)
		validatePin = TRUE;
	else
	validatePin = ValidateChecksum(PinCode);

	if ( validatePin )
	{
	    if (pWscControl->WscRejectSamePinFromEnrollee && 
            (PinCode == pWscControl->WscLastPinFromEnrollee))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PIN authentication or communication error occurs!!\n"
                                      "Registrar does NOT accept the same PIN again!(PIN:%s)\n", arg));
            return FALSE;
        }
        else
        {
    		pWscControl->WscPinCode = PinCode;
            pWscControl->WscLastPinFromEnrollee = pWscControl->WscPinCode;
            pWscControl->WscRejectSamePinFromEnrollee = FALSE;
            // PIN Code
			if (strlen(arg) == 4)
			{
				pWscControl->WscPinCodeLen = 4;
				pWscControl->RegData.PinCodeLen = 4;
				NdisMoveMemory(pWscControl->RegData.PIN, arg, 4);
			}
			else
			{
				pWscControl->WscPinCodeLen = 8;

            if (IsZero(*arg))
				{
					pWscControl->RegData.PinCodeLen = 8;
                NdisMoveMemory(pWscControl->RegData.PIN, arg, 8);
				}
            else
    	        WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
        }        
	}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set failed!!(Set_WscPinCode_Proc=%s), PinCode Checksum invalid \n", arg));
		return FALSE;  //Invalid argument
	}

#ifdef APCLI_SUPPORT
    if (bFromApCli)
    {
        memcpy(mac_addr, pAd->ApCfg.ApCliTab[apidx].CurrentAddress, MAC_ADDR_LEN);
    }
    else
#endif // APCLI_SUPPORT //        
    {
        memcpy(mac_addr, pAd->ApCfg.MBSSID[apidx].Bssid, MAC_ADDR_LEN);
    }
	DBGPRINT(RT_DEBUG_TRACE, ("IF(%02X:%02X:%02X:%02X:%02X:%02X) Set_WscPinCode_Proc::(PinCode=%d)\n", 
                                mac_addr[0],
                                mac_addr[1],
                                mac_addr[2],
                                mac_addr[3],
                                mac_addr[4],
                                mac_addr[5],
                                pWscControl->WscPinCode));

	return TRUE;
}

INT	Set_WscOOB_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
    char        *pTempSsid = NULL;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;
    
#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//

#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscPinCode_Proc:: Ap Client doesn't need this command.\n", apidx));
        return FALSE;
    }
#endif // APCLI_SUPPORT //

    Set_AP_WscConfStatus_Proc(pAd, "1");
    Set_AP_AuthMode_Proc(pAd, "WPAPSK");
    Set_AP_EncrypType_Proc(pAd, "TKIP");
    pTempSsid = vmalloc(33);
    if (pTempSsid)
    {
        memset(pTempSsid, 0, 33);
        sprintf(pTempSsid, "RalinkInitialAP%02X%02X%02X", pAd->ApCfg.MBSSID[apidx].Bssid[3],
                                                          pAd->ApCfg.MBSSID[apidx].Bssid[4],
                                                          pAd->ApCfg.MBSSID[apidx].Bssid[5]);
        Set_AP_SSID_Proc(pAd, pTempSsid);
        vfree(pTempSsid);
    }
	Set_AP_WPAPSK_Proc(pAd, "RalinkInitialAPxx1234");
    
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscOOB_Proc\n", apidx));
	return TRUE;
}

INT	Set_WscStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR	    apidx = pObj->ioctl_if;
    PWSC_CTRL   pWscControl;
    BOOLEAN     bFromApCli = FALSE;
    
#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//

    
#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
        bFromApCli = TRUE;
        pWscControl = &pAd->ApCfg.ApCliTab[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscStop_Proc:: This command is from apcli interface now.\n", apidx));
    }
    else
#endif // APCLI_SUPPORT //
    {
        bFromApCli = FALSE;
        pWscControl = &pAd->ApCfg.MBSSID[apidx].WscControl;
        DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscStop_Proc:: This command is from ra interface now.\n", apidx));
    }

    if (bFromApCli)
    {
	    WscStop(pAd, TRUE, pWscControl);
		pWscControl->WscConfMode = WSC_DISABLE;
    }
    else
    {
        INT	 IsAPConfigured = pWscControl->WscConfStatus;
        WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, apidx);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, apidx);
		APMakeAllBssBeacon(pAd);
		APUpdateAllBeaconFrame(pAd);
        WscStop(pAd, FALSE, pWscControl);
    }

    pWscControl->bWscTrigger = FALSE;
    DBGPRINT(RT_DEBUG_TRACE, ("<===== Set_WscStop_Proc"));
    return TRUE;
}

/* 
    ==========================================================================
    Description:
        Get WSC Profile
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage:
        		1.) iwpriv ra0 get_wsc_profile
        		3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPIoctlWscProfile(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	WSC_CONFIGURED_VALUE Profile;
	PSTRING msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;

#ifdef HOSTAPD_SUPPORT
	if (pAd->ApCfg.Hostapd == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
		return FALSE;
	}
#endif //HOSTAPD_SUPPORT//


	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
	Profile.WscConfigured = pAd->ApCfg.MBSSID[apidx].WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32 + 1);
	NdisMoveMemory(Profile.WscSsid, pAd->ApCfg.MBSSID[apidx].Ssid, 
								    pAd->ApCfg.MBSSID[apidx].SsidLen);
	Profile.WscSsid[pAd->ApCfg.MBSSID[apidx].SsidLen] = '\0';
	if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
		Profile.WscAuthMode = WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK;
	else
		Profile.WscAuthMode = WscGetAuthType(pAd->ApCfg.MBSSID[apidx].AuthMode);
	if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption4Enabled)
		Profile.WscEncrypType = WSC_ENCRTYPE_TKIP |WSC_ENCRTYPE_AES;
	else
		Profile.WscEncrypType = WscGetEncryType(pAd->ApCfg.MBSSID[apidx].WepStatus);
	NdisZeroMemory(Profile.WscWPAKey, 64 + 1);

	if (Profile.WscEncrypType == 2)
	{
		Profile.DefaultKeyIdx = pAd->ApCfg.MBSSID[apidx].DefaultKeyId + 1;
		{
			int i;
			for (i=0; i<pAd->SharedKey[apidx][pAd->ApCfg.MBSSID[apidx].DefaultKeyId].KeyLen; i++)
			{
				sprintf((PSTRING) Profile.WscWPAKey,
						"%s%02x", Profile.WscWPAKey,
									pAd->SharedKey[apidx][pAd->ApCfg.MBSSID[apidx].DefaultKeyId].Key[i]);
			}
			Profile.WscWPAKey[(pAd->SharedKey[apidx][pAd->ApCfg.MBSSID[apidx].DefaultKeyId].KeyLen)*2] = '\0';
		}
	}
	else if (Profile.WscEncrypType >= 4)
	{
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pAd->ApCfg.MBSSID[apidx].WscControl.WpaPsk, 
						pAd->ApCfg.MBSSID[apidx].WscControl.WpaPskLen);
		Profile.WscWPAKey[pAd->ApCfg.MBSSID[apidx].WscControl.WpaPskLen] = '\0';
	}
	else
	{
		Profile.DefaultKeyIdx = 1;
	}
	
	wrq->u.data.length = sizeof(Profile);

	if (copy_to_user(wrq->u.data.pointer, &Profile, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	msg = (PSTRING)kmalloc(sizeof(CHAR)*(2048), MEM_ALLOC_FLAG);
	if (msg == NULL) {
		return;
	}

	memset(msg, 0x00, 2048);
	sprintf(msg,"%s","\n");

	if (Profile.WscEncrypType == 1)
	{
		sprintf(msg+strlen(msg),"%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
	}
	else if (Profile.WscEncrypType == 2)
	{
		sprintf(msg+strlen(msg),"%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType", "DefaultKeyID", "Key");
	}
	else
	{
		sprintf(msg+strlen(msg),"%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");
	}

	if (Profile.WscConfigured == 1)
		sprintf(msg+strlen(msg),"%-12s", "No");
	else
		sprintf(msg+strlen(msg),"%-12s", "Yes");
	sprintf(msg+strlen(msg), "%-33s", Profile.WscSsid);
	if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
		sprintf(msg+strlen(msg), "%-12s", "WPAPSKWPA2PSK");
	else
		sprintf(msg+strlen(msg), "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));
	if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption4Enabled)
		sprintf(msg+strlen(msg), "%-12s", "TKIPAES");
	else
		sprintf(msg+strlen(msg), "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));

	if (Profile.WscEncrypType == 1)
	{
		sprintf(msg+strlen(msg), "%s\n", "");
	}
	else if (Profile.WscEncrypType == 2)
	{
		sprintf(msg+strlen(msg), "%-13d",Profile.DefaultKeyIdx);
		sprintf(msg+strlen(msg), "%-26s\n",Profile.WscWPAKey);
	}
	else if (Profile.WscEncrypType >= 4)
	{
	    sprintf(msg+strlen(msg), "%-64s\n",Profile.WscWPAKey);
	}
#ifdef INF_AR9
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
#endif//INF_AR9//

	DBGPRINT(RT_DEBUG_TRACE, ("%s", msg));
	kfree(msg);
}

#ifdef INF_AR9
#ifdef AR9_MAPI_SUPPORT

/* 
    ==========================================================================
    Description:
        Get WSC Profile
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage:
        		1.) iwpriv ra0 ar9_show get_wsc_profile
        		3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPAR9IoctlWscProfile(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	WSC_CONFIGURED_VALUE Profile;
	PSTRING msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;

	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
	Profile.WscConfigured = pAd->ApCfg.MBSSID[apidx].WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32 + 1);
	NdisMoveMemory(Profile.WscSsid, pAd->ApCfg.MBSSID[apidx].Ssid, 
								    pAd->ApCfg.MBSSID[apidx].SsidLen);
	Profile.WscSsid[pAd->ApCfg.MBSSID[apidx].SsidLen] = '\0';
	if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
		Profile.WscAuthMode = WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK;
	else
		Profile.WscAuthMode = WscGetAuthType(pAd->ApCfg.MBSSID[apidx].AuthMode);
	if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption4Enabled)
		Profile.WscEncrypType = WSC_ENCRTYPE_TKIP |WSC_ENCRTYPE_AES;
	else
		Profile.WscEncrypType = WscGetEncryType(pAd->ApCfg.MBSSID[apidx].WepStatus);
	NdisZeroMemory(Profile.WscWPAKey, 64 + 1);

	if (Profile.WscEncrypType == 2)
	{
		Profile.DefaultKeyIdx = pAd->ApCfg.MBSSID[apidx].DefaultKeyId + 1;
		{
			int i;
			for (i=0; i<pAd->SharedKey[apidx][pAd->ApCfg.MBSSID[apidx].DefaultKeyId].KeyLen; i++)
			{
				sprintf((PSTRING) Profile.WscWPAKey,
						"%s%02x", Profile.WscWPAKey,
									pAd->SharedKey[apidx][pAd->ApCfg.MBSSID[apidx].DefaultKeyId].Key[i]);
			}
			Profile.WscWPAKey[(pAd->SharedKey[apidx][pAd->ApCfg.MBSSID[apidx].DefaultKeyId].KeyLen)*2] = '\0';
		}
	}
	else if (Profile.WscEncrypType >= 4)
	{
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pAd->ApCfg.MBSSID[apidx].WscControl.WpaPsk, 
						pAd->ApCfg.MBSSID[apidx].WscControl.WpaPskLen);
		Profile.WscWPAKey[pAd->ApCfg.MBSSID[apidx].WscControl.WpaPskLen] = '\0';
	}
	else
	{
		Profile.DefaultKeyIdx = 1;
	}


	msg = (PSTRING)kmalloc(sizeof(CHAR)*(2048), MEM_ALLOC_FLAG);
	if (msg == NULL) {
		return;
	}

	memset(msg, 0x00, 2048);
	sprintf(msg,"%s","\n");

	if (Profile.WscEncrypType == 1)
	{
		sprintf(msg+strlen(msg),"%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
	}
	else if (Profile.WscEncrypType == 2)
	{
		sprintf(msg+strlen(msg),"%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType", "DefaultKeyID", "Key");
	}
	else
	{
		sprintf(msg+strlen(msg),"%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");
	}

	if (Profile.WscConfigured == 1)
		sprintf(msg+strlen(msg),"%-12s", "No");
	else
		sprintf(msg+strlen(msg),"%-12s", "Yes");
	sprintf(msg+strlen(msg), "%-33s", Profile.WscSsid);
	if (pAd->ApCfg.MBSSID[apidx].AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
		sprintf(msg+strlen(msg), "%-12s", "WPAPSKWPA2PSK");
	else
		sprintf(msg+strlen(msg), "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));
	if (pAd->ApCfg.MBSSID[apidx].WepStatus == Ndis802_11Encryption4Enabled)
		sprintf(msg+strlen(msg), "%-12s", "TKIPAES");
	else
		sprintf(msg+strlen(msg), "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));

	if (Profile.WscEncrypType == 1)
	{
		sprintf(msg+strlen(msg), "%s\n", "");
	}
	else if (Profile.WscEncrypType == 2)
	{
		sprintf(msg+strlen(msg), "%-13d",Profile.DefaultKeyIdx);
		sprintf(msg+strlen(msg), "%-26s\n",Profile.WscWPAKey);
	}
	else if (Profile.WscEncrypType >= 4)
	{
	    sprintf(msg+strlen(msg), "%-64s\n",Profile.WscWPAKey);
	}

	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s", msg));
	}
	kfree(msg);
}

VOID RTMPIoctlWscPINCode(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	PSTRING msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	UCHAR        tempPIN[9]={0};

	msg = (PSTRING)kmalloc(sizeof(CHAR)*(128), MEM_ALLOC_FLAG);
	if (msg == NULL) {
		return;
	}

	memset(msg, 0x00, 128);
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"WSC_PINCode=");
	if(pAd->ApCfg.MBSSID[apidx].WscControl.WscEnrolleePinCode)
	{
		sprintf((PSTRING) tempPIN, "%08u", pAd->ApCfg.MBSSID[apidx].WscControl.WscEnrolleePinCode);
		sprintf(msg,"%s%s\n",msg,tempPIN);
	}
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s", msg));
	}
	kfree(msg);
}

VOID RTMPIoctlWscStatus(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	PSTRING msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;

	msg = (PSTRING)kmalloc(sizeof(CHAR)*(128), MEM_ALLOC_FLAG);
	if (msg == NULL) {
		return;
	}

	memset(msg, 0x00, 128);
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"WSC_Status=");
	sprintf(msg,"%s%d\n",msg,pAd->ApCfg.MBSSID[apidx].WscControl.WscStatus);
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s", msg));
	}
	kfree(msg);
}

VOID RTMPIoctlGetWscDynInfo(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	char *msg;
	PMULTISSID_STRUCT	pMbss;
	INT apidx,configstate;


	msg = kmalloc(sizeof(CHAR)*(pAd->ApCfg.BssidNum*(14*128)), MEM_ALLOC_FLAG);
	if (msg == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return;
	}
	memset(msg, 0 ,pAd->ApCfg.BssidNum*(14*128));
	sprintf(msg,"%s","\n");
	
	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		pMbss=&pAd->ApCfg.MBSSID[apidx];

		if(pMbss->WscControl.WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
			configstate = 0;
		else
			configstate = 1;
		
		sprintf(msg+strlen(msg),"ra%d\n",apidx);
		sprintf(msg+strlen(msg),"UUID = %s\n",(pMbss->WscControl.Wsc_Uuid_Str));
		sprintf(msg+strlen(msg),"wpsVersion = 0x%x\n",WSC_VERSION);
		sprintf(msg+strlen(msg),"setuoLockedState = %d\n",0);
		sprintf(msg+strlen(msg),"configstate = %d\n",configstate);
		sprintf(msg+strlen(msg),"lastConfigError = %d\n",0);
		
	}

	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s", msg));
	}

	kfree(msg);
}

VOID RTMPIoctlGetWscRegsDynInfo(
	IN	PRTMP_ADAPTER	pAd, 
	IN	struct iwreq	*wrq)
{
	char *msg;
	PMULTISSID_STRUCT	pMbss;
	INT apidx;


	msg = kmalloc(sizeof(CHAR)*(pAd->ApCfg.BssidNum*(14*128)), MEM_ALLOC_FLAG);
	if (msg == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return;
	}
	memset(msg, 0 ,pAd->ApCfg.BssidNum*(14*128));
	sprintf(msg,"%s","\n");
	
	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		pMbss=&pAd->ApCfg.MBSSID[apidx];
		sprintf(msg+strlen(msg),"ra%d\n",apidx);
		sprintf(msg+strlen(msg),"UUID_R = %s\n",(pMbss->WscControl.RegData.PeerInfo.Uuid));	
	}

	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s", msg));
	}

	kfree(msg);
}
#endif //AR9_MAPI_SUPPORT//
#endif//INF_AR9//
BOOLEAN WscCheckEnrolleeNonceFromUpnp(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	PSTRING			pData,
	IN  USHORT			Length,
	IN  PWSC_CTRL       pWscControl) 
{
	USHORT	WscType, WscLen;
    USHORT  WscId = WSC_ID_ENROLLEE_NONCE;

    DBGPRINT(RT_DEBUG_TRACE, ("check Enrollee Nonce\n"));
   
    // We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4
	while (Length > 4)
	{
		WSC_TLV_0B	TLV_Recv;
        char ZeroNonce[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        
		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = cpu2be16(TLV_Recv.tag);
		WscLen  = cpu2be16(TLV_Recv.len);
		pData  += 4;
		Length -= 4;
        
		if (WscType == WscId)
		{
			if (RTMPCompareMemory(pWscControl->RegData.SelfNonce, pData, 16) == 0)
			{
			    DBGPRINT(RT_DEBUG_TRACE, ("Nonce match!!\n"));
                DBGPRINT(RT_DEBUG_TRACE, ("<----- WscCheckNonce\n"));
				return TRUE;
			}
            else if (NdisEqualMemory(pData, ZeroNonce, 16))
            {
                // Intel external registrar will send WSC_NACK with enrollee nonce
                // "10 1A 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"
                // when AP is configured and user selects not to configure AP.
                DBGPRINT(RT_DEBUG_TRACE, ("Zero Enrollee Nonce!!\n"));
                DBGPRINT(RT_DEBUG_TRACE, ("<----- WscCheckNonce\n"));
                return TRUE;
            }
		}
        
		// Offset to net WSC Ie
		pData  += WscLen;
		Length -= WscLen;
	}

    DBGPRINT(RT_DEBUG_TRACE, ("Nonce mismatch!!\n"));
    return FALSE;
}

UCHAR	WscRxMsgTypeFromUpnp(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PSTRING				pData,
	IN	USHORT				Length) 
{
	
	USHORT WscType, WscLen;
    
    {   // Eap-Esp(Messages)
        // the first TLV item in EAP Messages must be WSC_IE_VERSION
        NdisMoveMemory(&WscType, pData, 2);
        if (ntohs(WscType) != WSC_ID_VERSION)
            goto out;

        // Not Wsc Start, We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4
		while (Length > 4)
		{
			// arm-cpu has packet alignment issue, it's better to use memcpy to retrieve data
			NdisMoveMemory(&WscType, pData, 2);
			NdisMoveMemory(&WscLen,  pData + 2, 2);
			WscLen = ntohs(WscLen);
			if (ntohs(WscType) == WSC_ID_MSG_TYPE)
			{
				return(*(pData + 4));	// Found the message type
			}
			else
			{
				pData  += (WscLen + 4);
				Length -= (WscLen + 4);
			}
		}
    }

out:
	return  WSC_MSG_UNKNOWN;
}

VOID RTMPIoctlSetWSCOOB(
	IN PRTMP_ADAPTER pAd)
{
    char        *pTempSsid = NULL;
    POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR       apidx = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("IF(apcli%d) Set_WscPinCode_Proc:: Ap Client doesn't need this command.\n", apidx));
        return;
    }
#endif // APCLI_SUPPORT //

    Set_AP_WscConfStatus_Proc(pAd, "1");
    Set_AP_AuthMode_Proc(pAd, "WPAPSK");
    Set_AP_EncrypType_Proc(pAd, "TKIP");
    pTempSsid = vmalloc(33);
    if (pTempSsid)
    {
        memset(pTempSsid, 0, 33);
        sprintf(pTempSsid, "RalinkInitialAP%02X%02X%02X", pAd->ApCfg.MBSSID[apidx].Bssid[3],
                                                          pAd->ApCfg.MBSSID[apidx].Bssid[4],
                                                          pAd->ApCfg.MBSSID[apidx].Bssid[5]);
        Set_AP_SSID_Proc(pAd, pTempSsid);
        vfree(pTempSsid);
    }
	Set_AP_WPAPSK_Proc(pAd, "RalinkInitialAPxx1234");
    
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscOOB_Proc\n", apidx));
	return;
}

/*     
	==========================================================================    
	Description:       	
	Set Wsc Security Mode        
	0 : WPA2PSK AES         
	1 : WPA2PSK TKIP         
	2 : WPAPSK AES        
	3 : WPAPSK TKIP
	Return:        
	TRUE if all parameters are OK, FALSE otherwise    
	==========================================================================
*/
INT	Set_AP_WscSecurityMode_Proc(	
	IN	PRTMP_ADAPTER	pAd, 	
	IN	PSTRING			arg)
{	
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;	
	UCHAR		apidx = pObj->ioctl_if;	
	
	if (strcmp(arg, "0") == 0)		
		pAd->ApCfg.MBSSID[apidx].WscSecurityMode = WPA2PSKAES;	
	else if (strcmp(arg, "1") == 0)		
		pAd->ApCfg.MBSSID[apidx].WscSecurityMode = WPA2PSKTKIP;	
	else if (strcmp(arg, "2") == 0)		
		pAd->ApCfg.MBSSID[apidx].WscSecurityMode = WPAPSKAES;	
	else if (strcmp(arg, "3") == 0)		
		pAd->ApCfg.MBSSID[apidx].WscSecurityMode = WPAPSKTKIP;	
	else		
		return FALSE;	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_WscSecurityMode_Proc::(WscSecurityMode=%d)\n", 
		apidx, pAd->ApCfg.MBSSID[apidx].WscSecurityMode ));	
	
	return TRUE;
}
#endif // WSC_AP_SUPPORT //


#ifdef IAPP_SUPPORT
INT	Set_IappPID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	unsigned long IappPid;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

	IappPid = simple_strtol(arg, 0, 10);
	RTMP_GET_OS_PID(pObj->IappPid, IappPid);
	pObj->IappPid_nr = IappPid;

	DBGPRINT(RT_DEBUG_TRACE, ("pObj->IappPid = %d", GET_PID_NUMBER(pObj->IappPid)));
	return TRUE;
} /* End of Set_IappPID_Proc */
#endif // IAPP_SUPPORT //


INT	Set_DisConnectSta_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	PSTRING					value;
	INT						i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if(strlen(arg) != 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  //Invalid

		AtoH(value, &macAddr[i++], 1);
	}

	pEntry = MacTableLookup(pAd, macAddr);
	if (pEntry)
	{
		MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
//		MacTableDeleteEntry(pAd, pEntry->Aid, Addr);				
	}

	return TRUE;
}

INT	Set_DisConnectAllSta_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	MacTableReset(pAd);

	return TRUE;
}


#ifdef DOT1X_SUPPORT
/* 
    ==========================================================================
    Description:
        Set IEEE8021X.
        This parameter is 1 when 802.1x-wep turn on, otherwise 0
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_IEEE8021X_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
    ULONG ieee8021x;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	ieee8021x = simple_strtol(arg, 0, 10);

	if (ieee8021x == 1)
        pAd->ApCfg.MBSSID[pObj->ioctl_if].IEEE8021X = TRUE;
	else if (ieee8021x == 0)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].IEEE8021X = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_IEEE8021X_Proc::(IEEE8021X=%d)\n", pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].IEEE8021X));

	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set pre-authentication enable or disable when WPA/WPA2 turn on
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_PreAuth_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
    ULONG PreAuth;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	PreAuth = simple_strtol(arg, 0, 10);

	if (PreAuth == 1)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].PreAuth = TRUE;
	else if (PreAuth == 0)
		pAd->ApCfg.MBSSID[pObj->ioctl_if].PreAuth = FALSE;
	else
		return FALSE;  //Invalid argument 
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_PreAuth_Proc::(PreAuth=%d)\n", pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].PreAuth));

	return TRUE;
}

INT	Set_OwnIPAddr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32		ip_addr;

	if (rtinet_aton(arg, &ip_addr))
 	{
        pAd->ApCfg.own_ip_addr = ip_addr;  
		DBGPRINT(RT_DEBUG_TRACE, ("own_ip_addr=%s(%x)\n", arg, pAd->ApCfg.own_ip_addr));
	}	 
	return TRUE;
}

INT	Set_EAPIfName_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT			i;
	PSTRING		macptr;	

	for (i=0, macptr = rstrtok(arg,";"); (macptr && i < MAX_MBSSID_NUM); macptr = rstrtok(NULL,";"), i++) 
	{
		if (strlen(macptr) > 0)
		{
			pAd->ApCfg.EAPifname_len[i] = strlen(macptr); 
			NdisMoveMemory(pAd->ApCfg.EAPifname[i], macptr, strlen(macptr));
			DBGPRINT(RT_DEBUG_TRACE, ("NO.%d EAPifname=%s, len=%d\n", i, 
														pAd->ApCfg.EAPifname[i], 
														pAd->ApCfg.EAPifname_len[i]));
		}
	}
	return TRUE;
}

INT	Set_PreAuthIfName_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT			i;
	PSTRING		macptr;	

	for (i=0, macptr = rstrtok(arg,";"); (macptr && i < MAX_MBSSID_NUM); macptr = rstrtok(NULL,";"), i++) 
	{
		if (strlen(macptr) > 0)
		{
			pAd->ApCfg.PreAuthifname_len[i] = strlen(macptr); 
			NdisMoveMemory(pAd->ApCfg.PreAuthifname[i], macptr, strlen(macptr));
			DBGPRINT(RT_DEBUG_TRACE, ("NO.%d PreAuthifname=%s, len=%d\n", i,
														pAd->ApCfg.PreAuthifname[i], 
														pAd->ApCfg.PreAuthifname_len[i]));
		}
	}
	return TRUE;

}

INT	Set_RADIUS_Server_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	PSTRING		macptr;	
	INT			count;
	UINT32		ip_addr;
	INT			srv_cnt = 0;

	for (count = 0, macptr = rstrtok(arg,";"); (macptr && count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL,";"), count++) 
	{
		if (rtinet_aton(macptr, &ip_addr))
		{
			PRADIUS_SRV_INFO pSrvInfo = &pAd->ApCfg.MBSSID[apidx].radius_srv_info[srv_cnt];
		
			pSrvInfo->radius_ip = ip_addr;
			srv_cnt++;
			DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), radius_ip(seq-%d)=%s(%x)\n", 
										apidx, srv_cnt, macptr, 
										pSrvInfo->radius_ip));			
		}	    
	}

	if (srv_cnt > 0)
		pAd->ApCfg.MBSSID[apidx].radius_srv_num = srv_cnt;

	return TRUE;
}

INT	Set_RADIUS_Port_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	PSTRING		macptr;	
	INT			count;
	INT			srv_cnt = 0;

	for (count = 0, macptr = rstrtok(arg,";"); (macptr && count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL,";"), count++) 
	{	  
		if (srv_cnt < pAd->ApCfg.MBSSID[apidx].radius_srv_num)
		{		
			PRADIUS_SRV_INFO pSrvInfo = &pAd->ApCfg.MBSSID[apidx].radius_srv_info[srv_cnt];
			
        	pSrvInfo->radius_port = (UINT32) simple_strtol(macptr, 0, 10); 
			srv_cnt ++;
			DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), radius_port(seq-%d)=%d\n", 
									  apidx, srv_cnt, pSrvInfo->radius_port));
		}
	}

	return TRUE;
}

INT	Set_RADIUS_Key_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	PSTRING		macptr;	
	INT			count;
	INT			srv_cnt = 0;

	for (count = 0, macptr = rstrtok(arg,";"); (macptr && count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL,";"), count++) 
	{	  
		if (strlen(macptr) > 0 && srv_cnt < pAd->ApCfg.MBSSID[apidx].radius_srv_num)
		{		
			PRADIUS_SRV_INFO pSrvInfo = &pAd->ApCfg.MBSSID[apidx].radius_srv_info[srv_cnt];

			pSrvInfo->radius_key_len = strlen(macptr); 
			NdisMoveMemory(pSrvInfo->radius_key, macptr, pSrvInfo->radius_key_len);	
			srv_cnt ++;
			DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d), radius_key(seq-%d)=%s, len=%d\n", 
										apidx, srv_cnt,
										pSrvInfo->radius_key, 
										pSrvInfo->radius_key_len));			
		}
	}				
	return TRUE;
}
#endif // DOT1X_SUPPORT //

#ifdef UAPSD_AP_SUPPORT
INT Set_UAPSD_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	if (simple_strtol(arg, 0, 10) != 0)
		pAd->CommonCfg.bAPSDCapable = TRUE;
	else
		pAd->CommonCfg.bAPSDCapable = FALSE;
	/* End of if */

	return TRUE;
} /* End of Set_UAPSD_Proc */
#endif // UAPSD_AP_SUPPORT //



#ifdef MCAST_RATE_SPECIFIC
INT Set_McastPhyMode(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	UCHAR PhyMode = simple_strtol(arg, 0, 10);

	pAd->CommonCfg.MCastPhyMode.field.BW = pAd->CommonCfg.RegTransmitSetting.field.BW;
	switch (PhyMode)
	{
		case MCAST_DISABLE: // disable
			NdisMoveMemory(&pAd->CommonCfg.MCastPhyMode, &pAd->MacTab.Content[MCAST_WCID].HTPhyMode, sizeof(HTTRANSMIT_SETTING));
			break;

		case MCAST_CCK:	// CCK
			pAd->CommonCfg.MCastPhyMode.field.MODE = MODE_CCK;
			pAd->CommonCfg.MCastPhyMode.field.BW =  BW_20;
			break;

		case MCAST_OFDM:	// OFDM
			pAd->CommonCfg.MCastPhyMode.field.MODE = MODE_OFDM;
			break;
#ifdef DOT11_N_SUPPORT
		case MCAST_HTMIX:	// HTMIX
			pAd->CommonCfg.MCastPhyMode.field.MODE = MODE_HTMIX;
			break;
#endif // DOT11_N_SUPPORT //
		default:
			printk("unknow Muticast PhyMode %d.\n", PhyMode);
			printk("0:Disable 1:CCK, 2:OFDM, 3:HTMIX.\n");
			break;
	}

	return TRUE;
}

INT Set_McastMcs(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	UCHAR Mcs = simple_strtol(arg, 0, 10);

	if (Mcs > 15)
		printk("Mcs must in range of 0 to 15\n");

	switch(pAd->CommonCfg.MCastPhyMode.field.MODE)
	{
		case MODE_CCK:
			if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11))
				pAd->CommonCfg.MCastPhyMode.field.MCS = Mcs;
			else
				printk("MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n");
			break;

		case MODE_OFDM:
			if (Mcs > 7)
				printk("MCS must in range from 0 to 7 for CCK Mode.\n");
			else
				pAd->CommonCfg.MCastPhyMode.field.MCS = Mcs;
			break;

		default:
			pAd->CommonCfg.MCastPhyMode.field.MCS = Mcs;
			break;
	}

	return TRUE;
}

INT Show_McastRate(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	printk("Mcast PhyMode =%d\n", pAd->CommonCfg.MCastPhyMode.field.MODE);
	printk("Mcast Mcs =%d\n", pAd->CommonCfg.MCastPhyMode.field.MCS);
	return TRUE;
}
#endif // MCAST_RATE_SPECIFIC //

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
INT Set_OBSSScanParam_Proc(
	IN PRTMP_ADAPTER 	pAd,
	IN PSTRING			arg)
{
	
	INT ObssScanValue;
	UINT Idx;
	PSTRING thisChar;
	
	Idx = 0;
	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		ObssScanValue = (INT) simple_strtol(thisChar, 0, 10);
		switch (Idx)
		{
			case 0:
				if (ObssScanValue < 5 || ObssScanValue > 1000)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue));
				}
				else
				{
					pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	// Unit : TU. 5~1000
					DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n", ObssScanValue));
				}
				break;
			case 1:
				if (ObssScanValue < 10 || ObssScanValue > 1000)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue));
				}
				else
				{
					pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	// Unit : TU. 10~1000
					DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanActiveDwell=%d\n", ObssScanValue));
				}
				break;
			case 2:
				pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	// Unit : Second
				DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n", ObssScanValue));
				break;
			case 3:
				if (ObssScanValue < 200 || ObssScanValue > 10000)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue));
				}
				else
				{
					pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	// Unit : TU. 200~10000
					DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n", ObssScanValue));
				}
				break;
			case 4:
				if (ObssScanValue < 20 || ObssScanValue > 10000)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue));
				}
				else
				{
					pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	// Unit : TU. 20~10000
					DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11OBssScanActiveTotalPerChannel=%d\n", ObssScanValue));
				}
				break;
			case 5:
				pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = ObssScanValue;
				DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n", ObssScanValue));
				break;
			case 6:
				pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	// Unit : percentage
				DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n", ObssScanValue));
				break;
		}
		Idx++;
	}

	if (Idx != 7)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong OBSSScanParamtetrs format in ioctl cmd!!!!! Use default value\n"));
		
		pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	// Unit : TU. 5~1000
		pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	// Unit : TU. 10~1000
		pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	// Unit : Second	
		pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	// Unit : TU. 200~10000
		pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	// Unit : TU. 20~10000
		pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
		pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	// Unit : percentage
	}
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	DBGPRINT(RT_DEBUG_TRACE, ("OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n", pAd->CommonCfg.Dot11BssWidthChanTranDelay));
	
	return TRUE;
}

INT	Set_AP2040ReScan_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	PSTRING arg)
{
	APOverlappingBSSScan(pAd);

	/* apply setting */
	SetCommonHT(pAd);
	AsicBBPAdjust(pAd);
	AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_AP2040ReScan_Proc() Trigger AP ReScan !!!\n"));

	return TRUE;
}

#endif // DOT11N_DRAFT3 //

#ifdef DOT11N_PF_DEBUG
INT Set_HT_BssCoexNotify_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam)
{
	UCHAR bBssCoexNotify = simple_strtol(pParam, 0, 10);

	pAd->CommonCfg.bBssCoexNotify = ((bBssCoexNotify == 1) ? TRUE: FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("Set bBssCoexNotify=%d!\n", pAd->CommonCfg.bBssCoexNotify));
	
	return TRUE;
}


INT Set_HT_MaxBa_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam)
{
	UCHAR BaMaxSize = simple_strtol(pParam, 0, 10);

	if (BaMaxSize < 64)
	{
		pAd->CommonCfg.BAMaxSize = BaMaxSize;
		DBGPRINT(RT_DEBUG_TRACE, ("Set the MaxBaSize=%d!\n", pAd->CommonCfg.BAMaxSize));
	}
	
	return TRUE;
}


INT Set_HT_McsSet_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam)
{
	UCHAR mcsSet = simple_strtol(pParam, 0, 10);
	BOOLEAN needRestart;
	
	/* 
		HtMcsSet value     Meaning
		0                        disable fixMCSSet
		1                        enable MCS 0~7
		2                        enable MCS 8~15
		4                        enable MCS 16~23
	*/

	if (mcsSet > 7)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid MCSSet(%d)", __FUNCTION__, mcsSet));
		return FALSE;
	}

	needRestart = (pAd->CommonCfg.fixRxMcsSet != mcsSet);
	pAd->CommonCfg.fixRxMcsSet = mcsSet;
	if (needRestart)
	{
		/* We need to restart the AP to make sure all data */
		APStop(pAd);
		APStartUp(pAd);
	}
	
	return TRUE;
}


INT Set_HT_BssBWFallback_Proc(
	IN	PRTMP_ADAPTER		pAd,
	IN	PSTRING				pParam)
{
	BOOLEAN bBssFallBack = simple_strtol(pParam, 0, 10);

	if (bBssFallBack)
	{
		//int apidx;
		BSS_2040_COEXIST_IE bssCoexistIe;
			
		memset(&bssCoexistIe, 0, sizeof(BSS_2040_COEXIST_IE));
		bssCoexistIe.field.Intolerant40 = 1;
		bssCoexistIe.field.BSS20WidthReq = 1;
						
		NdisMoveMemory((PUCHAR)&pAd->CommonCfg.LastBSSCoexist2040, (PUCHAR)&bssCoexistIe, sizeof(BSS_2040_COEXIST_IE));
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;

		if (!(pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED))
		{	
			DBGPRINT(RT_DEBUG_TRACE, ("Fire the Bss2040CoexistTimer with timeout=%ld!\n", 
						pAd->CommonCfg.Dot11BssWidthChanTranDelay));

			pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_TIMER_FIRED;
			// More 5 sec for the scan report of STAs.
			RTMPSetTimer(&pAd->CommonCfg.Bss2040CoexistTimer,  (pAd->CommonCfg.Dot11BssWidthChanTranDelay + 5) * 1000);
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Already fallback to 20MHz, Extend the timeout of Bss2040CoexistTimer!\n"));
			// More 5 sec for the scan report of STAs.
			RTMPSetTimer(&pAd->CommonCfg.Bss2040CoexistTimer, (pAd->CommonCfg.Dot11BssWidthChanTranDelay + 5) * 1000);
		}
		
		//for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		//	SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID, apidx, 0);
	}

	return TRUE;
}
#endif // DOT11N_PF_DEBUG //

#endif // DOT11_N_SUPPORT //

INT Set_EntryLifeCheck_Proc(
	IN PRTMP_ADAPTER 	pAd,
	IN PSTRING			arg)
{
	ULONG LifeCheckCnt = (ULONG) simple_strtol(arg, 0, 10);

	if (LifeCheckCnt <= 65535)
		pAd->ApCfg.EntryLifeCheck = LifeCheckCnt;
	else
		printk("LifeCheckCnt must in range of 0 to 65535\n");

	printk("EntryLifeCheck Cnt = %ld.\n", pAd->ApCfg.EntryLifeCheck);
	return TRUE;
}

/* 
    ==========================================================================
    Description:
        Set Authentication mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	ApCfg_Set_AuthMode_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	INT				apidx,
	IN	PSTRING			arg)
{
	if ((strcmp(arg, "WEPAUTO") == 0) || (strcmp(arg, "wepauto") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeAutoSwitch;
	else if ((strcmp(arg, "OPEN") == 0) || (strcmp(arg, "open") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeOpen;
	else if ((strcmp(arg, "SHARED") == 0) || (strcmp(arg, "shared") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeShared;
	else if ((strcmp(arg, "WPAPSK") == 0) || (strcmp(arg, "wpapsk") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWPAPSK;
	else if ((strcmp(arg, "WPA2PSK") == 0) || (strcmp(arg, "wpa2psk") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWPA2PSK;
	else if ((strcmp(arg, "WPAPSKWPA2PSK") == 0) || (strcmp(arg, "wpapskwpa2psk") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWPA1PSKWPA2PSK;
#ifdef DOT1X_SUPPORT
	else if ((strcmp(arg, "WPA") == 0) || (strcmp(arg, "wpa") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWPA;
	else if ((strcmp(arg, "WPA2") == 0) || (strcmp(arg, "wpa2") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWPA2;
	else if ((strcmp(arg, "WPA1WPA2") == 0) || (strcmp(arg, "wpa1wpa2") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWPA1WPA2;
#endif // DOT1X_SUPPORT //
#ifdef WAPI_SUPPORT
	else if ((strcmp(arg, "WAICERT") == 0) || (strcmp(arg, "waicert") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWAICERT;
	else if ((strcmp(arg, "WAIPSK") == 0) || (strcmp(arg, "waipsk") == 0))
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeWAIPSK;
#endif // WAPI_SUPPORT //	
	else
		pAd->ApCfg.MBSSID[apidx].AuthMode = Ndis802_11AuthModeOpen;  

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d)::AuthMode=%d\n", apidx, pAd->ApCfg.MBSSID[apidx].AuthMode));		

	return TRUE;
}

INT	ApCfg_Set_MaxStaNum_Proc(
	IN PRTMP_ADAPTER 	pAd,
	IN INT				apidx,
	IN PSTRING 			arg)
{
	pAd->ApCfg.MBSSID[apidx].MaxStaNum = (UCHAR)simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) %s::(MaxStaNum=%d)\n", 
					apidx, __FUNCTION__, pAd->ApCfg.MBSSID[apidx].MaxStaNum));
	return TRUE;
}

INT	ApCfg_Set_IdleTimeout_Proc(
	IN PRTMP_ADAPTER 	pAd, 
	IN PSTRING 			arg)
{
	LONG idle_time;

	idle_time = simple_strtol(arg, 0, 10);

	if (idle_time < MAC_TABLE_MIN_AGEOUT_TIME)
		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_MIN_AGEOUT_TIME;
	else
		pAd->ApCfg.StaIdleTimeout = idle_time;

	DBGPRINT(RT_DEBUG_TRACE, ("%s : IdleTimeout=%d\n", __FUNCTION__, pAd->ApCfg.StaIdleTimeout));

	return TRUE;
}

#ifdef RTMP_RBUS_SUPPORT
#ifdef WLAN_LED

ULONG WlanLed=1;

INT Set_WlanLed_Proc(
	IN PRTMP_ADAPTER 	pAd,
	IN PSTRING		arg)
{
	WlanLed = (ULONG) simple_strtol(arg, 0, 10);

	return TRUE;
}
#endif // WLAN_LED //
#endif // RTMP_RBUS_SUPPORT //


#ifdef RT3883
INT Set_CwCTest_Proc(
		IN PRTMP_ADAPTER pAd, 
		IN PSTRING   arg)
{
	UINT32 MacReg;
	long x = simple_strtol(arg, 0, 10);
	if (x == 0)
	{
		pAd->FlgCWC = x;
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &MacReg);
		MacReg &= 0xffff0000;
		MacReg |= 0x1f0f;
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, MacReg);

		RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &MacReg);
		MacReg &= 0xffff0000;
		MacReg |= 0x1010;
		RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, MacReg);
		DBGPRINT(RT_DEBUG_OFF, ("CwC OFF\n"));
	}
	else if (x == 1)
	{
		pAd->FlgCWC = x;
		RTMP_IO_READ32(pAd, TX_RTY_CFG, &MacReg);
		MacReg &= 0xffff0000;
		MacReg |= 0x3f3f;
		RTMP_IO_WRITE32(pAd, TX_RTY_CFG, MacReg);

		RTMP_IO_READ32(pAd, XIFS_TIME_CFG, &MacReg);
		MacReg &= 0xffff0000;
		MacReg |= 0x0e0e;
		RTMP_IO_WRITE32(pAd, XIFS_TIME_CFG, MacReg);
		DBGPRINT(RT_DEBUG_OFF, ("CwC ON\n"));
	}
	else if (x == 2)
	{
		pAd->FlgCWC = x;
		DBGPRINT(RT_DEBUG_OFF, ("CwC Auto\n"));
	}
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("current FlgCWC = %d\n", pAd->FlgCWC));
	}

	return TRUE;
}
#endif // RT3883 //

INT	Set_MemDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
#ifdef VENDOR_FEATURE2_SUPPORT
	printk("Number of Packet Allocated = %d\n", pAd->NumOfPktAlloc);
	printk("Number of Packet Freed = %d\n", pAd->NumOfPktFree);
	printk("Offset of Packet Allocated/Freed = %d\n", pAd->NumOfPktAlloc - pAd->NumOfPktFree);
#endif // VENDOR_FEATURE2_SUPPORT //
	return TRUE;
}
