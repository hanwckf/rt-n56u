/*

*/
#include "rt_config.h"


CH_FREQ_MAP CH_HZ_ID_MAP[]=
		{
			{1, 2412},
			{2, 2417},
			{3, 2422},
			{4, 2427},
			{5, 2432},
			{6, 2437},
			{7, 2442},
			{8, 2447},
			{9, 2452},
			{10, 2457},
			{11, 2462},
			{12, 2467},
			{13, 2472},
			{14, 2484},

			/*  UNII */
			{36, 5180},
			{40, 5200},
			{44, 5220},
			{48, 5240},
			{52, 5260},
			{56, 5280},
			{60, 5300},
			{64, 5320},
			{149, 5745},
			{153, 5765},
			{157, 5785},
			{161, 5805},
			{165, 5825},
			{167, 5835},
			{169, 5845},
			{171, 5855},
			{173, 5865},
						
			/* HiperLAN2 */
			{100, 5500},
			{104, 5520},
			{108, 5540},
			{112, 5560},
			{116, 5580},
			{120, 5600},
			{124, 5620},
			{128, 5640},
			{132, 5660},
			{136, 5680},
			{140, 5700},
						
			/* Japan MMAC */
			{34, 5170},
			{38, 5190},
			{42, 5210},
			{46, 5230},
					
			/*  Japan */
			{184, 4920},
			{188, 4940},
			{192, 4960},
			{196, 4980},
			
			{208, 5040},	/* Japan, means J08 */
			{212, 5060},	/* Japan, means J12 */   
			{216, 5080},	/* Japan, means J16 */
};

INT	CH_HZ_ID_MAP_NUM = (sizeof(CH_HZ_ID_MAP)/sizeof(CH_FREQ_MAP));

CH_DESC Country_Region0_ChDesc_2GHZ[] =
{
	{1, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_2GHZ[] =
{
	{1, 13, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_2GHZ[] =
{
	{10, 2, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_2GHZ[] =
{
	{10, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_2GHZ[] =
{
	{14, 1, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region5_ChDesc_2GHZ[] =
{
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_2GHZ[] =
{
	{3, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_2GHZ[] =
{
	{5, 9, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region31_ChDesc_2GHZ[] =
{
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 3, CHANNEL_PASSIVE_SCAN},
	{}
};

CH_DESC Country_Region32_ChDesc_2GHZ[] =
{
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 2, CHANNEL_PASSIVE_SCAN},	
	{}
};

CH_DESC Country_Region33_ChDesc_2GHZ[] =
{
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[] =
{
	{REGION_0_BG_BAND, Country_Region0_ChDesc_2GHZ},
	{REGION_1_BG_BAND, Country_Region1_ChDesc_2GHZ},
	{REGION_2_BG_BAND, Country_Region2_ChDesc_2GHZ},
	{REGION_3_BG_BAND, Country_Region3_ChDesc_2GHZ},
	{REGION_4_BG_BAND, Country_Region4_ChDesc_2GHZ},
	{REGION_5_BG_BAND, Country_Region5_ChDesc_2GHZ},
	{REGION_6_BG_BAND, Country_Region6_ChDesc_2GHZ},
	{REGION_7_BG_BAND, Country_Region7_ChDesc_2GHZ},
	{REGION_31_BG_BAND, Country_Region31_ChDesc_2GHZ},
	{REGION_32_BG_BAND, Country_Region32_ChDesc_2GHZ},
	{REGION_33_BG_BAND, Country_Region33_ChDesc_2GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_2GHZ = sizeof(Country_Region_ChDesc_2GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_DESC Country_Region0_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{}	
};

CH_DESC Country_Region3_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_5GHZ[] =
{
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};
CH_DESC Country_Region5_ChDesc_5GHZ[] =
{
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_5GHZ[] =
{
	{36, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region8_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region9_ChDesc_5GHZ[] =
{
	{36, 8 , CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region10_ChDesc_5GHZ[] =
{
	{36,4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region11_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 6, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}		
};

CH_DESC Country_Region12_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region13_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}	
};

CH_DESC Country_Region14_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{136, 2, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}	
};

CH_DESC Country_Region15_ChDesc_5GHZ[] =
{
	{149, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region16_ChDesc_5GHZ[] =
{
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region17_ChDesc_5GHZ[] =
{
	{36, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region18_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{}	
};

CH_DESC Country_Region19_ChDesc_5GHZ[] =
{
	{56, 3, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region20_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 7, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}		
};

CH_DESC Country_Region21_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}		
};


COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[] =
{
	{REGION_0_A_BAND, Country_Region0_ChDesc_5GHZ},
	{REGION_1_A_BAND, Country_Region1_ChDesc_5GHZ},
	{REGION_2_A_BAND, Country_Region2_ChDesc_5GHZ},
	{REGION_3_A_BAND, Country_Region3_ChDesc_5GHZ},
	{REGION_4_A_BAND, Country_Region4_ChDesc_5GHZ},
	{REGION_5_A_BAND, Country_Region5_ChDesc_5GHZ},
	{REGION_6_A_BAND, Country_Region6_ChDesc_5GHZ},
	{REGION_7_A_BAND, Country_Region7_ChDesc_5GHZ},
	{REGION_8_A_BAND, Country_Region8_ChDesc_5GHZ},
	{REGION_9_A_BAND, Country_Region9_ChDesc_5GHZ},
	{REGION_10_A_BAND, Country_Region10_ChDesc_5GHZ},
	{REGION_11_A_BAND, Country_Region11_ChDesc_5GHZ},
	{REGION_12_A_BAND, Country_Region12_ChDesc_5GHZ},
	{REGION_13_A_BAND, Country_Region13_ChDesc_5GHZ},
	{REGION_14_A_BAND, Country_Region14_ChDesc_5GHZ},
	{REGION_15_A_BAND, Country_Region15_ChDesc_5GHZ},
	{REGION_16_A_BAND, Country_Region16_ChDesc_5GHZ},
	{REGION_17_A_BAND, Country_Region17_ChDesc_5GHZ},
	{REGION_18_A_BAND, Country_Region18_ChDesc_5GHZ},
	{REGION_19_A_BAND, Country_Region19_ChDesc_5GHZ},
	{REGION_20_A_BAND, Country_Region20_ChDesc_5GHZ},
	{REGION_21_A_BAND, Country_Region21_ChDesc_5GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_5GHZ = sizeof(Country_Region_ChDesc_5GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

UINT16 TotalChNum(PCH_DESC pChDesc)
{
	UINT16 TotalChNum = 0;
	
	while(pChDesc->FirstChannel)
	{
		TotalChNum += pChDesc->NumOfCh;
		pChDesc++;
	}
	
	return TotalChNum;
}

UCHAR GetChannel_5GHZ(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel)
	{
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index * 4;
		else
		{
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannel_2GHZ(PCH_DESC pChDesc, UCHAR index)
{

	while (pChDesc->FirstChannel)
	{
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index;
		else
		{
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannelFlag(PCH_DESC pChDesc, UCHAR index)
{

	while (pChDesc->FirstChannel)
	{
		if (index < pChDesc->NumOfCh)
			return pChDesc->ChannelProp;
		else
		{
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

#ifdef EXT_BUILD_CHANNEL_LIST

/*Albania*/
CH_DESP Country_AL_ChDesp[] =
{
	{ 1, 13, 20, BOTH, FALSE},  	/*2402~2482MHz, Ch 1~13, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Algeria*/
CH_DESP Country_DZ_ChDesp[] =
{
	{ 1, 13, 20, BOTH, FALSE},  	/*2402~2482MHz, Ch 1~13, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Argentina*/		
CH_DESP Country_AR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13, Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Armenia*/	
CH_DESP Country_AM_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 18, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 18, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Aruba*/	
CH_DESP Country_AW_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Australia*/	
CH_DESP Country_AU_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Austria*/	
CH_DESP Country_AT_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Azerbaijan*/	
CH_DESP Country_AZ_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 18, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 18, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Bahrain*/	
CH_DESP Country_BH_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Bangladesh*/	
CH_DESP Country_BD_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Barbados*/	
CH_DESP Country_BB_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Belarus*/	
CH_DESP Country_BY_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Belgium*/	
CH_DESP Country_BE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Belize*/	
CH_DESP Country_BZ_ChDesp[] =
{
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Bolivia*/	
CH_DESP Country_BO_ChDesp[] =
{
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Bosnia and Herzegovina*/	
CH_DESP Country_BA_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Brazil*/	
CH_DESP Country_BR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Brunei Darussalam*/	
CH_DESP Country_BN_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Bulgaria*/	
CH_DESP Country_BG_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   2, 23, BOTH, TRUE}, 	/*5250~5290MHz, Ch 52~56, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Cambodia*/	
CH_DESP Country_KH_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Canada*/	
CH_DESP Country_CA_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */ 
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Chile*/	
CH_DESP Country_CL_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*China*/		
CH_DESP Country_CN_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */	
	{ 0},               	    	/* end*/
};
/*Colombia*/		
CH_DESP Country_CO_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Costa Rica*/		
CH_DESP Country_CR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Croatia*/		
CH_DESP Country_HR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Cyprus*/		
CH_DESP Country_CY_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */ 
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Czech Republic*/		
CH_DESP Country_CZ_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2400~2483.5MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5470~5725MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Denmark*/		
CH_DESP Country_DK_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Dominican Republic*/		
CH_DESP Country_DO_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Ecuador*/		
CH_DESP Country_EC_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Egypt*/		
CH_DESP Country_EG_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*El Salvador*/		
CH_DESP Country_SV_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Estonia*/		
CH_DESP Country_EE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Finland*/		
CH_DESP Country_FI_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*France*/		
CH_DESP Country_FR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Georgia*/		
CH_DESP Country_GE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 18, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 18, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Germany*/		
CH_DESP Country_DE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2400~2483.5MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5470~5725MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Greece*/		
CH_DESP Country_GR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Greenland*/		
CH_DESP Country_GL_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Grenada*/		
CH_DESP Country_GD_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Guam*/		
CH_DESP Country_GU_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */
	{ 0},               	    	/* end*/
};
/*Guatemala*/		
CH_DESP Country_GT_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Haiti*/		
CH_DESP Country_HT_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Honduras*/		
CH_DESP Country_HN_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Hong Kong*/		
CH_DESP Country_HK_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Hungary*/		
CH_DESP Country_HU_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,  Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Iceland*/		
CH_DESP Country_IS_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*India*/		
CH_DESP Country_IN_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Indonesia*/		
CH_DESP Country_ID_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Iran, Islamic Republic of*/		
CH_DESP Country_IR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */	
	{ 0},               	    	/* end*/
};
/*Ireland*/		
CH_DESP Country_IE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Israel*/		
CH_DESP Country_IL_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE}, 	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Italy*/		
CH_DESP Country_IT_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Jamaica*/		
CH_DESP Country_JM_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Japan*/		
CH_DESP Country_JP_ChDesp[] =
{
	{ 1,    14, 20, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 20 */
		   			/*2457~2482MHz, Ch10~13,  Max BW: 20 */
		   			/*2474~2494MHz, Ch14,  	  Max BW: 20, No OFDM */		
	{  36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{  52,   4, 20, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Jordan*/		
CH_DESP Country_JO_ChDesp[] =
{
	{ 1,  13, 20, BOTH, FALSE}, 	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,  4, 18, BOTH, FALSE}, 	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Kazakhstan*/		
CH_DESP Country_KZ_ChDesp[] =
{
	{ 1,  13, 20, BOTH, FALSE}, 	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Kenya*/		
CH_DESP Country_KE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */	
	{ 0},               	    	/* end*/
};
/*Korea, Democratic People's Republic of*/		
CH_DESP Country_KP_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, TRUE}, 	/*5160~5250MHz, Ch 36~48, Max BW: 40 */
	{ 36,   8, 20, BOTH, FALSE},	/*5170~5330MHz, Ch 36~64, Max BW: 40 */
	{ 100,  7, 30, BOTH, TRUE}, 	/*5490~5630MHz, Ch 100~124, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Korea, Republic of*/		
CH_DESP Country_KR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 20 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 100,  7, 30, BOTH, TRUE}, 	/*5490~5630MHz, Ch 100~124, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Kuwait*/		
CH_DESP Country_KW_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Latvia*/		
CH_DESP Country_LV_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Lebanon*/		
CH_DESP Country_LB_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */	
	{ 0},               	    	/* end*/
};
/*Liechtenstein*/		
CH_DESP Country_LI_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Lithuania*/		
CH_DESP Country_LT_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Luxembourg*/		
CH_DESP Country_LU_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Macao*/		
CH_DESP Country_MO_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */	
	{ 0},               	    	/* end*/
};
/*Macedonia, Republic of*/		
CH_DESP Country_MK_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Malaysia*/		
CH_DESP Country_MY_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Malta*/		
CH_DESP Country_MT_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Mexico*/		
CH_DESP Country_MX_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Monaco*/		
CH_DESP Country_MC_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 18, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 18, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Morocco*/		
CH_DESP Country_MA_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Nepal*/		
CH_DESP Country_NP_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */	
	{ 0},               	    	/* end*/
};
/*Netherlands*/		
CH_DESP Country_NL_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Netherlands Antilles*/
CH_DESP Country_AN_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*New Zealand*/
CH_DESP Country_NZ_ChDesp[] =
{
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */		
	{ 0},               	    	/* end*/
};
/*Norway*/	
CH_DESP Country_NO_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Oman*/		
CH_DESP Country_OM_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Pakistan*/		
CH_DESP Country_PK_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Panama*/		
CH_DESP Country_PA_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Papua New Guinea*/		
CH_DESP Country_PG_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Peru*/		
CH_DESP Country_PE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Philippines*/		
CH_DESP Country_PH_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Poland*/		
CH_DESP Country_PL_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Portuga*/		
CH_DESP Country_PT_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Puerto Rico*/		
CH_DESP Country_PR_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Qatar*/		
CH_DESP Country_QA_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Romania*/		
CH_DESP Country_RO_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Russian Federation*/		
CH_DESP Country_RU_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */		
	{ 0},               	    	/* end*/
};
/*Saint Barth'elemy*/		
CH_DESP Country_BL_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 18, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 18, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Saudi Arabia*/		
CH_DESP Country_SA_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */			
	{ 0},               	    	/* end*/
};
/*Singapore*/		
CH_DESP Country_SG_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Slovakia*/		
CH_DESP Country_SK_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Slovenia*/		
CH_DESP Country_SI_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*South Africa*/		
CH_DESP Country_ZA_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Spain*/		
CH_DESP Country_ES_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Sri Lanka*/		
CH_DESP Country_LK_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 20 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */			
	{ 0},               	    	/* end*/
};
/*Sweden*/		
CH_DESP Country_SE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Switzerland*/		
CH_DESP Country_CH_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Syrian Arab Republic*/		
CH_DESP Country_SY_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Taiwan*/		
CH_DESP Country_TW_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 56,   3, 17, IDOR, TRUE}, 	/*5270~5330MHz, Ch 56~64, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Thailand*/		
CH_DESP Country_TH_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Trinidad and Tobago*/		
CH_DESP Country_TT_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE},    	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Tunisia*/		
CH_DESP Country_TN_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */		
	{ 0},               	    	/* end*/
};
/*Turkey*/		
CH_DESP Country_TR_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 20 */		
	{ 0},               	    	/* end*/
};
/*Ukraine*/		
CH_DESP Country_UA_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*United Arab Emirates*/		
CH_DESP Country_AE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*United Kingdom*/		
CH_DESP Country_GB_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */  
	{ 52,   4, 20, IDOR, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */  
	{ 100, 11, 27, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*United States*/		
CH_DESP Country_US_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  5, 20, BOTH, TRUE}, 	/*5490~5600MHz, Ch 100~116, Max BW: 40 */
	{ 132,  3, 20, BOTH, TRUE}, 	/*5650~5710MHz, Ch 132~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},               	    	/* end*/
};		
/*Uruguay*/		
CH_DESP Country_UY_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};		
/*Uzbekistan*/		
CH_DESP Country_UZ_ChDesp[] =
{
	{ 1,   11, 27, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */			
	{ 0},               	    	/* end*/
};
/*Venezuela*/		
CH_DESP Country_VE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */	
	{ 0},               	    	/* end*/
};
/*Viet Nam*/		
CH_DESP Country_VN_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */		
	{ 0},               	    	/* end*/
};
/*Yemen*/		
CH_DESP Country_YE_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};
/*Zimbabwe*/		
CH_DESP Country_ZW_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},               	    	/* end*/
};

/* Group Region */
/*Europe*/		
CH_DESP Country_EU_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/* 2.4 G, ch 1~13 */
	{ 36,   4, 17, BOTH, FALSE},	/* 5G band 1, ch 36~48*/
	{ 0},               	    	/* end*/
};
/*North America*/		
CH_DESP Country_NA_ChDesp[] =
{
	{ 1,   11,	27, BOTH, FALSE},	/* 2.4 G, ch 1~11*/  
	{ 36,   4,	17, IDOR, FALSE},	/* 5G band 1, ch 36~48*/  
	{ 149,	5, 30, BOTH, FALSE},	/* 5G band 4, ch 149~165*/
	{ 0},               	    	/* end*/
};
/*World Wide*/		
CH_DESP Country_WO_ChDesp[] =
{
	{ 1,   13, 20, BOTH, FALSE},	/* 2.4 G, ch 1~13*/
	{ 36,   4, 17, BOTH, FALSE},	/* 5G band 1, ch 36~48*/
	{ 149,	5, 22, BOTH, FALSE},	/* 5G band 4, ch 149~165*/
	{ 0},               	    	/* end*/
};

CH_REGION ChRegion[] =
{
	{"AL", CE, Country_AL_ChDesp}, /* Albania */
	{"DZ", CE, Country_DZ_ChDesp}, /* Algeria */
	{"AR", CE, Country_AR_ChDesp}, /* Argentina */
	{"AM", CE, Country_AM_ChDesp}, /* Armenia */
	{"AW", CE, Country_AW_ChDesp}, /* Aruba */
	{"AU", CE, Country_AU_ChDesp}, /* Australia */
	{"AT", CE, Country_AT_ChDesp}, /* Austria */
	{"AZ", CE, Country_AZ_ChDesp}, /* Azerbaijan */
	{"BH", CE, Country_BH_ChDesp}, /* Bahrain */
	{"BD", CE, Country_BD_ChDesp}, /* Bangladesh */
	{"BB", CE, Country_BB_ChDesp}, /* Barbados */
	{"BY", CE, Country_BY_ChDesp}, /* Belarus */
	{"BE", CE, Country_BE_ChDesp}, /* Belgium */
	{"BZ", CE, Country_BZ_ChDesp}, /* Belize */
	{"BO", CE, Country_BO_ChDesp}, /* Bolivia */
	{"BA", CE, Country_BA_ChDesp}, /* Bosnia and Herzegovina */
	{"BR", CE, Country_BR_ChDesp}, /* Brazil */
	{"BN", CE, Country_BN_ChDesp}, /* Brunei Darussalam */
	{"BG", CE, Country_BG_ChDesp}, /* Bulgaria */
	{"KH", CE, Country_KH_ChDesp}, /* Cambodia */
	{"CA", FCC,Country_CA_ChDesp}, /* Canada */
	{"CL", CE, Country_CL_ChDesp}, /* Chile */
	{"CN", CE, Country_CN_ChDesp}, /* China */
	{"CO", CE, Country_CO_ChDesp}, /* Colombia */
	{"CR", CE, Country_CR_ChDesp}, /* Costa Rica */
	{"HR", CE, Country_HR_ChDesp}, /* Croatia */
	{"CY", CE, Country_CY_ChDesp}, /* Cyprus */
	{"CZ", CE, Country_CZ_ChDesp}, /* Czech Republic */
	{"DK", CE, Country_DK_ChDesp}, /* Denmark */
	{"DO", CE, Country_DO_ChDesp}, /* Dominican Republic */
	{"EC", CE, Country_EC_ChDesp}, /* Ecuador */
	{"EG", CE, Country_EG_ChDesp}, /* Egypt */
	{"SV", CE, Country_SV_ChDesp}, /* El Salvador */
	{"EE", CE, Country_EE_ChDesp}, /* Estonia */
	{"FI", CE, Country_FI_ChDesp}, /* Finland */
	{"FR", CE, Country_FR_ChDesp}, /* France */
	{"GE", CE, Country_GE_ChDesp}, /* Georgia */
	{"DE", CE, Country_DE_ChDesp}, /* Germany */
	{"GR", CE, Country_GR_ChDesp}, /* Greece */
	{"GL", CE, Country_GL_ChDesp}, /* Greenland */
	{"GD", CE, Country_GD_ChDesp}, /* Grenada */
	{"GU", CE, Country_GU_ChDesp}, /* Guam */
	{"GT", CE, Country_GT_ChDesp}, /* Guatemala */
	{"HT", CE, Country_HT_ChDesp}, /* Haiti */
	{"HN", CE, Country_HN_ChDesp}, /* Honduras */
	{"HK", CE, Country_HK_ChDesp}, /* Hong Kong */
	{"HU", CE, Country_HU_ChDesp}, /* Hungary */
	{"IS", CE, Country_IS_ChDesp}, /* Iceland */
	{"IN", CE, Country_IN_ChDesp}, /* India */
	{"ID", CE, Country_ID_ChDesp}, /* Indonesia */
	{"IR", CE, Country_IR_ChDesp}, /* Iran, Islamic Republic of */
	{"IE", CE, Country_IE_ChDesp}, /* Ireland */
	{"IL", CE, Country_IL_ChDesp}, /* Israel */
	{"IT", CE, Country_IT_ChDesp}, /* Italy */
	{"JM", CE, Country_JM_ChDesp}, /* Jamaica */
	{"JP", JAP,Country_JP_ChDesp}, /* Japan */		
	{"JO", CE, Country_JO_ChDesp}, /* Jordan */	
	{"KZ", CE, Country_KZ_ChDesp}, /* Kazakhstan */			
	{"KE", CE, Country_KE_ChDesp}, /* Kenya */	
	{"KP", CE, Country_KP_ChDesp}, /* Korea, Democratic People's Republic of */
	{"KR", CE, Country_KR_ChDesp}, /* Korea, Republic of */			
	{"KW", CE, Country_KW_ChDesp}, /* Kuwait */			
	{"LV", CE, Country_LV_ChDesp}, /* Latvia */			
	{"LB", CE, Country_LB_ChDesp}, /* Lebanon */			
	{"LI", CE, Country_LI_ChDesp}, /* Liechtenstein */			
	{"LT", CE, Country_LT_ChDesp}, /* Lithuania */			
	{"LU", CE, Country_LU_ChDesp}, /* Luxembourg */			
	{"MO", CE, Country_MO_ChDesp}, /* Macao */			
	{"MK", CE, Country_MK_ChDesp}, /* Macedonia, Republic of */			
	{"MY", CE, Country_MY_ChDesp}, /* Malaysia */			
	{"MT", CE, Country_MT_ChDesp}, /* Malta */			
	{"MX", CE, Country_MX_ChDesp}, /* Mexico */			
	{"MC", CE, Country_MC_ChDesp}, /* Monaco */			
	{"MA", CE, Country_MA_ChDesp}, /* Morocco */			
	{"NP", CE, Country_NP_ChDesp}, /* Nepal */			
	{"NL", CE, Country_NL_ChDesp}, /* Netherlands */			
	{"AN", CE, Country_AN_ChDesp}, /* Netherlands Antilles */			
	{"NZ", CE, Country_NZ_ChDesp}, /* New Zealand */			
	{"NO", CE, Country_NO_ChDesp}, /* Norway */			
	{"OM", CE, Country_OM_ChDesp}, /* Oman */		
	{"PK", CE, Country_PK_ChDesp}, /* Pakistan */		
	{"PA", CE, Country_PA_ChDesp}, /* Panama */	
	{"PG", CE, Country_PG_ChDesp}, /* Papua New Guinea */	
	{"PE", CE, Country_PE_ChDesp}, /* Peru */			
	{"PH", CE, Country_PH_ChDesp}, /* Philippines */		
	{"PL", CE, Country_PL_ChDesp}, /* Poland */			
	{"PT", CE, Country_PT_ChDesp}, /* Portuga l*/			
	{"PR", CE, Country_PR_ChDesp}, /* Puerto Rico */			
	{"QA", CE, Country_QA_ChDesp}, /* Qatar */			
	{"RO", CE, Country_RO_ChDesp}, /* Romania */			
	{"RU", CE, Country_RU_ChDesp}, /* Russian Federation */			
	{"BL", CE, Country_BL_ChDesp}, /* Saint Barth'elemy */			
	{"SA", CE, Country_SA_ChDesp}, /* Saudi Arabia */			
	{"SG", CE, Country_SG_ChDesp}, /* Singapore */			
	{"SK", CE, Country_SK_ChDesp}, /* Slovakia */			
	{"SI", CE, Country_SI_ChDesp}, /* Slovenia */					
	{"ZA", CE, Country_ZA_ChDesp}, /* South Africa */					
	{"ES", CE, Country_ES_ChDesp}, /* Spain */				
	{"LK", CE, Country_LK_ChDesp}, /* Sri Lanka */				
	{"SE", CE, Country_SE_ChDesp}, /* Sweden */					
	{"CH", CE, Country_CH_ChDesp}, /* Switzerland */					
	{"SY", CE, Country_SY_ChDesp}, /* Syrian Arab Republic */					
	{"TW", FCC,Country_TW_ChDesp}, /* Taiwan */			
	{"TH", CE, Country_TH_ChDesp}, /* Thailand */					
	{"TT", CE, Country_TT_ChDesp}, /* Trinidad and Tobago */			
	{"TN", CE, Country_TN_ChDesp}, /* Tunisia */				
	{"TR", CE, Country_TR_ChDesp}, /* Turkey */					
	{"UA", CE, Country_UA_ChDesp}, /* Ukraine */					
	{"AE", CE, Country_AE_ChDesp}, /* United Arab Emirates */					
	{"GB", CE, Country_GB_ChDesp}, /* United Kingdom */			
	{"US", FCC,Country_US_ChDesp}, /* United States */			
	{"UY", CE, Country_UY_ChDesp}, /* Uruguay */					
	{"UZ", CE, Country_UZ_ChDesp}, /* Uzbekistan */				
	{"VE", CE, Country_VE_ChDesp}, /* Venezuela */				
	{"VN", CE, Country_VN_ChDesp}, /* Viet Nam */					
	{"YE", CE, Country_YE_ChDesp}, /* Yemen */					
	{"ZW", CE, Country_ZW_ChDesp}, /* Zimbabwe */	
	{"EU", CE, Country_EU_ChDesp}, /* Europe */
	{"NA", FCC,Country_NA_ChDesp}, /* North America */
	{"WO", CE, Country_WO_ChDesp}, /* World Wide */
	{""  , 0,  NULL}	     , /* End */	
};

static PCH_REGION GetChRegion(
	IN PUCHAR CntryCode)
{
	INT loop = 0;
	PCH_REGION pChRegion = NULL;

	while (strcmp((PSTRING) ChRegion[loop].CountReg, "") != 0)
	{
		if (strncmp((PSTRING) ChRegion[loop].CountReg, (PSTRING) CntryCode, 2) == 0)
		{
			pChRegion = &ChRegion[loop];
			break;
		}
		loop++;
	}

	/* Default: use WO*/
	if (pChRegion == NULL)
		pChRegion = GetChRegion("WO");

	return pChRegion;
}

static VOID ChBandCheck(
	IN UCHAR PhyMode,
	OUT PUCHAR pChType)
{
	switch(PhyMode)
	{
		case PHY_11A:
#ifdef DOT11_N_SUPPORT
		case PHY_11AN_MIXED:
		case PHY_11N_5G:
#endif /* DOT11_N_SUPPORT */
			*pChType = BAND_5G;
			break;
		case PHY_11ABG_MIXED:
#ifdef DOT11_N_SUPPORT
		case PHY_11AGN_MIXED:
		case PHY_11ABGN_MIXED:
#endif /* DOT11_N_SUPPORT */
			*pChType = BAND_BOTH;
			break;

		default:
			*pChType = BAND_24G;
			break;
	}
}

static UCHAR FillChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_DESP pChDesp,
	IN UCHAR Offset, 
	IN UCHAR increment,
	IN UCHAR regulatoryDomain)
{
	INT i, j, l;
	UCHAR channel;

	j = Offset;
	for (i = 0; i < pChDesp->NumOfCh; i++)
	{
		channel = pChDesp->FirstChannel + i * increment;
		if (!strncmp((PSTRING) pAd->CommonCfg.CountryCode, "JP", 2))
        {
            /* for JP, ch14 can only be used when PhyMode is "B only" */
            if ( (channel==14) && (pAd->CommonCfg.PhyMode != PHY_11B) )
            {
                    pChDesp->NumOfCh--;
                    break;
            }
        }
/*New FCC spec restrict the used channel under DFS */
#ifdef CONFIG_AP_SUPPORT	
		if ((pAd->CommonCfg.bIEEE80211H == 1) &&
			(pAd->CommonCfg.RDDurRegion == FCC) &&
			(pAd->Dot11_H.bDFSIndoor == 1))
		{
			if (RESTRICTION_BAND_1(pAd))
				continue;
		}
		else if ((pAd->CommonCfg.bIEEE80211H == 1) &&
				 (pAd->CommonCfg.RDDurRegion == FCC) &&
				 (pAd->Dot11_H.bDFSIndoor == 0))
		{
			if ((channel >= 100) && (channel <= 140))
				continue;
		}

#endif /* CONFIG_AP_SUPPORT */
		for (l=0; l<MAX_NUM_OF_CHANNELS; l++)
		{
			if (channel == pAd->TxPower[l].Channel)
			{
				pAd->ChannelList[j].Power = pAd->TxPower[l].Power;
				pAd->ChannelList[j].Power2 = pAd->TxPower[l].Power2;
#ifdef DOT11N_SS3_SUPPORT
					pAd->ChannelList[j].Power3 = pAd->TxPower[l].Power3;
#endif /* DOT11N_SS3_SUPPORT */
				break;
			}
		}
		if (l == MAX_NUM_OF_CHANNELS)
			continue;

		pAd->ChannelList[j].Channel = pChDesp->FirstChannel + i * increment;
		pAd->ChannelList[j].MaxTxPwr = pChDesp->MaxTxPwr;
		pAd->ChannelList[j].DfsReq = pChDesp->DfsReq;
		pAd->ChannelList[j].RegulatoryDomain = regulatoryDomain;
#ifdef DOT11_N_SUPPORT
		if (N_ChannelGroupCheck(pAd, pAd->ChannelList[j].Channel))
			pAd->ChannelList[j].Flags |= CHANNEL_40M_CAP;
#endif /* DOT11_N_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		CFG80211OS_ChanInfoInit(
					pAd->pCfg80211_CB,
					j,
					pAd->ChannelList[j].Channel,
					pAd->ChannelList[j].MaxTxPwr,
					(pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED),
					(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_20));
#endif /* RT_CFG80211_SUPPORT */

		j++;
	}
	pAd->ChannelListNum = j;

	return j;
}


static inline VOID CreateChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_REGION pChRegion,
	IN UCHAR Geography)
{
	INT i;
	UCHAR offset = 0;
	PCH_DESP pChDesp;
	UCHAR ChType;
	UCHAR increment;
	UCHAR regulatoryDomain;

	if (pChRegion == NULL)
		return;

	ChBandCheck(pAd->CommonCfg.PhyMode, &ChType);
	
	if (pAd->CommonCfg.pChDesp != NULL)
	   pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
	else
	   pChDesp = pChRegion->pChDesp;
	   
	for (i = 0; pChDesp[i].FirstChannel != 0; i++)
	{
		if (pChDesp[i].FirstChannel == 0)
			break;

		if (ChType == BAND_5G)
		{
			if (pChDesp[i].FirstChannel <= 14)
				continue;
		}
		else if (ChType == BAND_24G)
		{
			if (pChDesp[i].FirstChannel > 14)
				continue;
		}

		if ((pChDesp[i].Geography == BOTH)
				|| (Geography == BOTH)
				|| (pChDesp[i].Geography == Geography))
        {
			if (pChDesp[i].FirstChannel > 14)
                increment = 4;
            else
                increment = 1;
			if (pAd->CommonCfg.DfsType != MAX_RD_REGION)
				regulatoryDomain = pAd->CommonCfg.DfsType;
			else
				regulatoryDomain = pChRegion->DfsType;
			offset = FillChList(pAd, &pChDesp[i], offset, increment, regulatoryDomain);
        }
	}
}


VOID BuildChannelListEx(
	IN PRTMP_ADAPTER pAd)
{
	PCH_REGION pChReg;

	pChReg = GetChRegion(pAd->CommonCfg.CountryCode);
	CreateChList(pAd, pChReg, pAd->CommonCfg.Geography);
}

VOID BuildBeaconChList(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen)
{
	INT i;
	ULONG TmpLen;
	PCH_REGION pChRegion;
	PCH_DESP pChDesp;
	UCHAR ChType;

	pChRegion = GetChRegion(pAd->CommonCfg.CountryCode);

	if (pChRegion == NULL)
		return;

	ChBandCheck(pAd->CommonCfg.PhyMode, &ChType);
	*pBufLen = 0;

	if (pAd->CommonCfg.pChDesp != NULL)
		pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
	else
		pChDesp = pChRegion->pChDesp;

	for (i=0; pChRegion->pChDesp[i].FirstChannel != 0; i++)
	{
		if (pChDesp[i].FirstChannel == 0)
			break;

		if (ChType == BAND_5G)
		{
			if (pChDesp[i].FirstChannel <= 14)
				continue;
		}
		else if (ChType == BAND_24G)
		{
			if (pChDesp[i].FirstChannel > 14)
				continue;
		}

		if ((pChDesp[i].Geography == BOTH) ||
			(pChDesp[i].Geography == pAd->CommonCfg.Geography))
		{
			MakeOutgoingFrame(pBuf + *pBufLen,		&TmpLen,
								1,					&pChDesp[i].FirstChannel,
								1,					&pChDesp[i].NumOfCh,
								1,					&pChDesp[i].MaxTxPwr,
								END_OF_ARGS);
			*pBufLen += TmpLen;
		}
	}
}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DOT11_N_SUPPORT
static BOOLEAN IsValidChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)

{
	INT i;

	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
			break;
	}

	if (i == pAd->ChannelListNum)
		return FALSE;
	else
		return TRUE;
}

static UCHAR GetExtCh(
	IN UCHAR Channel,
	IN UCHAR Direction)
{
	CHAR ExtCh;

	if (Direction == EXTCHA_ABOVE)
		ExtCh = Channel + 4;
	else
		ExtCh = (Channel - 4) > 0 ? (Channel - 4) : 0;

	return ExtCh;
}

BOOLEAN N_ChannelGroupCheck(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	BOOLEAN	RetVal = FALSE;
	
	if (Channel > 14)
	{
		if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
		    (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
		{
			RetVal = TRUE;
		}
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
				(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
		{
			RetVal = TRUE;
		}
	}
	else
	{
		do
		{
			UCHAR ExtCh;

			if (Channel == 14)
			{
				RetVal = FALSE;
				break;
			}

			ExtCh = GetExtCh(Channel, EXTCHA_ABOVE);
			if (IsValidChannel(pAd, ExtCh))
				RetVal = TRUE;
			else
			{
				ExtCh = GetExtCh(Channel, EXTCHA_BELOW);
				if (IsValidChannel(pAd, ExtCh))
				RetVal = TRUE;
			}
		} while(FALSE);
	}

	return RetVal;
}

VOID N_ChannelCheck(
	IN PRTMP_ADAPTER pAd)
{
	/*UCHAR ChannelNum = pAd->ChannelListNum;*/
	UCHAR Channel = pAd->CommonCfg.Channel;

	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->CommonCfg.RegTransmitSetting.field.BW  == BW_40))
	{
		if (Channel > 14)
		{
			if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
			    (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
			{
				pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
			}
			else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
					(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
			{
				pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_BELOW;
			}
			else
			{
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
			}
		}
		else
		{
			do
			{
				UCHAR ExtCh;
				UCHAR Dir = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
				ExtCh = GetExtCh(Channel, Dir);
				if (IsValidChannel(pAd, ExtCh))
					break;

				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = GetExtCh(Channel, Dir);
				if (IsValidChannel(pAd, ExtCh))
				{
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = Dir;
					break;
				}
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
			} while(FALSE);

			if (Channel == 14)
			{
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
				/*pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_NONE; We didn't set the ExtCh as NONE due to it'll set in RTMPSetHT()*/
			}
		}
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11N_DRAFT3
#endif /* DOT11N_DRAFT3 */
#endif /* CONFIG_AP_SUPPORT */

}


VOID N_SetCenCh(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
	{
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		{

			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		}
		else
		{
			if (pAd->CommonCfg.Channel == 14)
				pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 1;
			else
				pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
		}
	}
	else
	{
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	}
}
#endif /* DOT11_N_SUPPORT */

UINT8 GetCuntryMaxTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 channel)
{
	int i;
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
			break;
	}

	if (i == pAd->ChannelListNum)
		return 0xff;
#ifdef SINGLE_SKU
	if (pAd->CommonCfg.bSKUMode == TRUE)
	{
		UINT deltaTxStreamPwr = 0;

#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->CommonCfg.TxStream == 2))
			deltaTxStreamPwr = 3; /* If 2Tx case, antenna gain will increase 3dBm*/
#endif /* DOT11_N_SUPPORT */
		if (pAd->ChannelList[i].RegulatoryDomain == FCC)
		{
			/* FCC should maintain 20/40 Bandwidth, and without antenna gain */
#ifdef DOT11_N_SUPPORT
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
				(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40) &&
				(channel == 1 || channel == 11))
				return (pAd->ChannelList[i].MaxTxPwr - pAd->CommonCfg.BandedgeDelta - deltaTxStreamPwr);
			else
#endif /* DOT11_N_SUPPORT */
				return (pAd->ChannelList[i].MaxTxPwr - deltaTxStreamPwr);
		}
		else if (pAd->ChannelList[i].RegulatoryDomain == CE)
		{
			return (pAd->ChannelList[i].MaxTxPwr - pAd->CommonCfg.AntGain - deltaTxStreamPwr);
		}
		else
			return 0xff;
	}
	else
#endif /* SINGLE_SKU */
		return pAd->ChannelList[i].MaxTxPwr;
}


/* for OS_ABL */
VOID RTMP_MapChannelID2KHZ(
	IN UCHAR Ch,
	OUT UINT32 *pFreq)
{
	int chIdx;
	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++)
	{
		if ((Ch) == CH_HZ_ID_MAP[chIdx].channel)
		{
			(*pFreq) = CH_HZ_ID_MAP[chIdx].freqKHz * 1000;
			break;
		}
	}
	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pFreq) = 2412000;
}

/* for OS_ABL */
VOID RTMP_MapKHZ2ChannelID(
	IN ULONG Freq,
	OUT INT *pCh)
{
	int chIdx;
	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++)
	{
		if ((Freq) == CH_HZ_ID_MAP[chIdx].freqKHz)
		{
			(*pCh) = CH_HZ_ID_MAP[chIdx].channel;
			break;
		}
	}
	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pCh) = 1;
}

