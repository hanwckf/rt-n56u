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
			{50, 5250},        
			{52, 5260},
			{54, 5270},         
			{56, 5280},
			{58, 5290},        
			{60, 5300},
			{62, 5310},			
			{64, 5320},
			{149, 5745},
			{151, 5755},             
			{153, 5765},
			{155, 5775},			
			{157, 5785},
			{159, 5795},			
			{161, 5805},
			{165, 5825},
			{167, 5835},
			{169, 5845},
			{171, 5855},
			{173, 5865},
						
			/* HiperLAN2 */
			{100, 5500},
			{102, 5510},			
			{104, 5520},
			{106, 5530},			
			{108, 5540},
			{110, 5550},		
			{112, 5560},
			{114, 5570},
			{116, 5580},
			{118, 5590},			
			{120, 5600},
			{122, 5610},			
			{124, 5620},
			{126, 5630},			
			{128, 5640},
			{132, 5660},
			{134, 5670},			
			{136, 5680},
			{138, 5690},			
			{140, 5700},
			{142, 5710},	
			{144, 5720},            
						
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
	{149, 5, CHANNEL_DEFAULT_PROP},		
//    {100, 14, CHANNEL_DEFAULT_PROP},
//    {149, 7, CHANNEL_DEFAULT_PROP},
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

/* for FCC capable of using 144 , mapping of Country_Region1 */
CH_DESC Country_Region12_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{}
};
/* for FCC capable of using 144 , mapping of Country_Region7 */
CH_DESC Country_Region13_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}	
};
/* for FCC capable of using 144 , mapping of Country_Region9 */
CH_DESC Country_Region14_ChDesc_5GHZ[] =
{
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 4, CHANNEL_DEFAULT_PROP},
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

CH_DESC Country_Region22_ChDesc_5GHZ[] =
{
	{100, 11, CHANNEL_DEFAULT_PROP},
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
	{REGION_22_A_BAND, Country_Region22_ChDesc_5GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_5GHZ = sizeof(Country_Region_ChDesc_5GHZ) / sizeof(COUNTRY_REGION_CH_DESC);



CH_GROUP_DESC Channel_GRP[] =
{
	{36, 4},
	{52, 4},
	{100, 12},
	{149, 5},
	{}
};


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
	{ 100, 12, 20, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~144, Max BW: 40 */
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
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 24, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  5, 24, BOTH, TRUE}, 	/*5490~5600MHz, Ch 100~116, Max BW: 40 */
	{ 132,  4, 24, BOTH, TRUE}, 	/*5650~5710MHz, Ch 132~144, Max BW: 40 */
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
	{ 1,   13, 20, BOTH, FALSE},	/* 2.4 G, ch 1~13 */
	{ 36,   4, 30, BOTH, FALSE},	/* 5G band 1, ch 36~48 */
	{ 149,	5, 30, BOTH, FALSE},	/* 5G band 4, ch 149~165 */
	{ 0},               	    	/* end */
};

CH_REGION ChRegion[] =
{
	{"AL", CE, TRUE, Country_AL_ChDesp}, /* Albania */
	{"DZ", CE, TRUE, Country_DZ_ChDesp}, /* Algeria */
	{"AR", CE, TRUE, Country_AR_ChDesp}, /* Argentina */
	{"AM", CE, TRUE, Country_AM_ChDesp}, /* Armenia */
	{"AW", CE, TRUE, Country_AW_ChDesp}, /* Aruba */
	{"AU", CE, FALSE,Country_AU_ChDesp}, /* Australia */
	{"AT", CE, TRUE, Country_AT_ChDesp}, /* Austria */
	{"AZ", CE, TRUE, Country_AZ_ChDesp}, /* Azerbaijan */
	{"BH", CE, TRUE, Country_BH_ChDesp}, /* Bahrain */
	{"BD", CE, TRUE, Country_BD_ChDesp}, /* Bangladesh */
	{"BB", CE, TRUE, Country_BB_ChDesp}, /* Barbados */
	{"BY", CE, TRUE, Country_BY_ChDesp}, /* Belarus */
	{"BE", CE, TRUE, Country_BE_ChDesp}, /* Belgium */
	{"BZ", CE, TRUE, Country_BZ_ChDesp}, /* Belize */
	{"BO", CE, TRUE, Country_BO_ChDesp}, /* Bolivia */
	{"BA", CE, TRUE, Country_BA_ChDesp}, /* Bosnia and Herzegovina */
	{"BR", CE, FALSE,Country_BR_ChDesp}, /* Brazil */
	{"BN", CE, TRUE, Country_BN_ChDesp}, /* Brunei Darussalam */
	{"BG", CE, TRUE, Country_BG_ChDesp}, /* Bulgaria */
	{"KH", CE, TRUE, Country_KH_ChDesp}, /* Cambodia */
	{"CA", FCC,FALSE,Country_CA_ChDesp}, /* Canada */
	{"CL", CE, TRUE, Country_CL_ChDesp}, /* Chile */
	{"CN", CN, FALSE,Country_CN_ChDesp}, /* China */
	{"CO", CE, TRUE, Country_CO_ChDesp}, /* Colombia */
	{"CR", CE, TRUE, Country_CR_ChDesp}, /* Costa Rica */
	{"HR", CE, TRUE, Country_HR_ChDesp}, /* Croatia */
	{"CY", CE, TRUE, Country_CY_ChDesp}, /* Cyprus */
	{"CZ", CE, TRUE, Country_CZ_ChDesp}, /* Czech Republic */
	{"DK", CE, TRUE, Country_DK_ChDesp}, /* Denmark */
	{"DO", CE, TRUE, Country_DO_ChDesp}, /* Dominican Republic */
	{"EC", CE, TRUE, Country_EC_ChDesp}, /* Ecuador */
	{"EG", CE, TRUE, Country_EG_ChDesp}, /* Egypt */
	{"SV", CE, TRUE, Country_SV_ChDesp}, /* El Salvador */
	{"EE", CE, TRUE, Country_EE_ChDesp}, /* Estonia */
	{"FI", CE, TRUE, Country_FI_ChDesp}, /* Finland */
	{"FR", CE, TRUE, Country_FR_ChDesp}, /* France */
	{"GE", CE, TRUE, Country_GE_ChDesp}, /* Georgia */
	{"DE", CE, TRUE, Country_DE_ChDesp}, /* Germany */
	{"GR", CE, TRUE, Country_GR_ChDesp}, /* Greece */
	{"GL", CE, TRUE, Country_GL_ChDesp}, /* Greenland */
	{"GD", CE, TRUE, Country_GD_ChDesp}, /* Grenada */
	{"GU", CE, TRUE, Country_GU_ChDesp}, /* Guam */
	{"GT", CE, TRUE, Country_GT_ChDesp}, /* Guatemala */
	{"HT", CE, TRUE, Country_HT_ChDesp}, /* Haiti */
	{"HN", CE, TRUE, Country_HN_ChDesp}, /* Honduras */
	{"HK", CE, TRUE, Country_HK_ChDesp}, /* Hong Kong */
	{"HU", CE, TRUE, Country_HU_ChDesp}, /* Hungary */
	{"IS", CE, TRUE, Country_IS_ChDesp}, /* Iceland */
	{"IN", CE, TRUE, Country_IN_ChDesp}, /* India */
	{"ID", CE, TRUE, Country_ID_ChDesp}, /* Indonesia */
	{"IR", CE, TRUE, Country_IR_ChDesp}, /* Iran, Islamic Republic of */
	{"IE", CE, TRUE, Country_IE_ChDesp}, /* Ireland */
	{"IL", CE, FALSE,Country_IL_ChDesp}, /* Israel */
	{"IT", CE, TRUE, Country_IT_ChDesp}, /* Italy */
	{"JM", CE, TRUE, Country_JM_ChDesp}, /* Jamaica */
	{"JP", JAP,TRUE, Country_JP_ChDesp}, /* Japan */		
	{"JO", CE, TRUE, Country_JO_ChDesp}, /* Jordan */	
	{"KZ", CE, TRUE, Country_KZ_ChDesp}, /* Kazakhstan */			
	{"KE", CE, TRUE, Country_KE_ChDesp}, /* Kenya */	
	{"KP", CE, TRUE, Country_KP_ChDesp}, /* Korea, Democratic People's Republic of */
	{"KR", CE, FALSE,Country_KR_ChDesp}, /* Korea, Republic of */			
	{"KW", CE, TRUE, Country_KW_ChDesp}, /* Kuwait */			
	{"LV", CE, TRUE, Country_LV_ChDesp}, /* Latvia */			
	{"LB", CE, TRUE, Country_LB_ChDesp}, /* Lebanon */			
	{"LI", CE, TRUE, Country_LI_ChDesp}, /* Liechtenstein */			
	{"LT", CE, TRUE, Country_LT_ChDesp}, /* Lithuania */			
	{"LU", CE, TRUE, Country_LU_ChDesp}, /* Luxembourg */			
	{"MO", CE, TRUE, Country_MO_ChDesp}, /* Macao */			
	{"MK", CE, TRUE, Country_MK_ChDesp}, /* Macedonia, Republic of */			
	{"MY", CE, TRUE, Country_MY_ChDesp}, /* Malaysia */			
	{"MT", CE, TRUE, Country_MT_ChDesp}, /* Malta */			
	{"MX", CE, FALSE,Country_MX_ChDesp}, /* Mexico */			
	{"MC", CE, TRUE, Country_MC_ChDesp}, /* Monaco */			
	{"MA", CE, TRUE, Country_MA_ChDesp}, /* Morocco */			
	{"NP", CE, TRUE, Country_NP_ChDesp}, /* Nepal */			
	{"NL", CE, TRUE, Country_NL_ChDesp}, /* Netherlands */			
	{"AN", CE, TRUE, Country_AN_ChDesp}, /* Netherlands Antilles */			
	{"NZ", CE, TRUE, Country_NZ_ChDesp}, /* New Zealand */			
	{"NO", CE, TRUE, Country_NO_ChDesp}, /* Norway */			
	{"OM", CE, TRUE, Country_OM_ChDesp}, /* Oman */		
	{"PK", CE, TRUE, Country_PK_ChDesp}, /* Pakistan */		
	{"PA", CE, TRUE, Country_PA_ChDesp}, /* Panama */	
	{"PG", CE, TRUE, Country_PG_ChDesp}, /* Papua New Guinea */	
	{"PE", CE, TRUE, Country_PE_ChDesp}, /* Peru */			
	{"PH", CE, TRUE, Country_PH_ChDesp}, /* Philippines */		
	{"PL", CE, TRUE, Country_PL_ChDesp}, /* Poland */			
	{"PT", CE, TRUE, Country_PT_ChDesp}, /* Portuga l*/			
	{"PR", CE, TRUE, Country_PR_ChDesp}, /* Puerto Rico */			
	{"QA", CE, TRUE, Country_QA_ChDesp}, /* Qatar */			
	{"RO", CE, TRUE, Country_RO_ChDesp}, /* Romania */			
	{"RU", CE, FALSE,Country_RU_ChDesp}, /* Russian Federation */			
	{"BL", CE, TRUE, Country_BL_ChDesp}, /* Saint Barth'elemy */			
	{"SA", CE, TRUE, Country_SA_ChDesp}, /* Saudi Arabia */			
	{"SG", CE, TRUE, Country_SG_ChDesp}, /* Singapore */			
	{"SK", CE, TRUE, Country_SK_ChDesp}, /* Slovakia */			
	{"SI", CE, TRUE, Country_SI_ChDesp}, /* Slovenia */					
	{"ZA", CE, FALSE,Country_ZA_ChDesp}, /* South Africa */					
	{"ES", CE, TRUE, Country_ES_ChDesp}, /* Spain */				
	{"LK", CE, TRUE, Country_LK_ChDesp}, /* Sri Lanka */				
	{"SE", CE, TRUE, Country_SE_ChDesp}, /* Sweden */					
	{"CH", CE, TRUE, Country_CH_ChDesp}, /* Switzerland */					
	{"SY", CE, TRUE, Country_SY_ChDesp}, /* Syrian Arab Republic */					
	{"TW", FCC,FALSE,Country_TW_ChDesp}, /* Taiwan */			
	{"TH", CE, FALSE,Country_TH_ChDesp}, /* Thailand */					
	{"TT", CE, TRUE, Country_TT_ChDesp}, /* Trinidad and Tobago */			
	{"TN", CE, TRUE, Country_TN_ChDesp}, /* Tunisia */				
	{"TR", CE, TRUE, Country_TR_ChDesp}, /* Turkey */					
	{"UA", CE, TRUE, Country_UA_ChDesp}, /* Ukraine */					
	{"AE", CE, TRUE, Country_AE_ChDesp}, /* United Arab Emirates */					
	{"GB", CE, TRUE, Country_GB_ChDesp}, /* United Kingdom */			
	{"US", FCC,FALSE,Country_US_ChDesp}, /* United States */			
	{"UY", CE, TRUE, Country_UY_ChDesp}, /* Uruguay */					
	{"UZ", CE, TRUE, Country_UZ_ChDesp}, /* Uzbekistan */				
	{"VE", CE, TRUE, Country_VE_ChDesp}, /* Venezuela */				
	{"VN", CE, TRUE, Country_VN_ChDesp}, /* Viet Nam */					
	{"YE", CE, TRUE, Country_YE_ChDesp}, /* Yemen */					
	{"ZW", CE, TRUE, Country_ZW_ChDesp}, /* Zimbabwe */	
	{"EU", CE, TRUE, Country_EU_ChDesp}, /* Europe */
	{"NA", FCC,FALSE,Country_NA_ChDesp}, /* North America */
	{"WO", CE, FALSE,Country_WO_ChDesp}, /* World Wide */
	{""  , 0, FALSE, NULL}	     , /* End */	
};

PCH_REGION GetChRegion(
	IN PUCHAR CountryCode)
{
	INT loop = 0;
	PCH_REGION pChRegion = NULL;

	while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0)
	{
		if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, (RTMP_STRING *)CountryCode, 2) == 0)
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

BOOLEAN CheckCountryCodeSanity(
	IN UCHAR* country_code)
{
	INT loop = 0;
	PCH_REGION pChRegion = NULL;

	while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0){	
		if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, (RTMP_STRING *)country_code, 2) == 0){		
			pChRegion = &ChRegion[loop];
			break;
		}
		loop++;
	}

	/* return FALSE to indicate invalid countrycode */
	if (pChRegion == NULL){
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("=============================================\n"));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("CountryCode [%s] is not a valid country...\n", country_code));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("=============================================\n"));
		return FALSE;
	}

	return TRUE;

}


#ifdef EXT_BUILD_CHANNEL_LIST
static VOID ChBandCheck(
	IN UCHAR PhyMode,
	OUT PUCHAR pChType)
{
	*pChType = 0;
	if (WMODE_CAP_5G(PhyMode))
		*pChType |= BAND_5G;
	if (WMODE_CAP_2G(PhyMode))
		*pChType |= BAND_24G;

	if (*pChType == 0)
		*pChType = BAND_24G;
}

static UCHAR FillChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_DESP pChDesp,
	IN UCHAR Offset, 
	IN UCHAR increment,
	IN UCHAR regulatoryDomain,
	IN UCHAR PhyMode)
{
	INT i, j;//sachin - TODO, l;
	UCHAR channel;
#if defined(CONFIG_AP_SUPPORT) || defined(RT_CFG80211_SUPPORT)
	UCHAR rfic = wmode_2_rfic(PhyMode);
	UCHAR bw = HcGetBwByRf(pAd,rfic);
#endif

	j = Offset;
	for (i = 0; i < pChDesp->NumOfCh; i++)
	{
		channel = pChDesp->FirstChannel + i * increment;
		if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "JP", 2))
        {
            /* for JP, ch14 can only be used when PhyMode is "B only" */
            if ( (channel==14) && 
			(!WMODE_EQUAL(PhyMode, WMODE_B)))
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
			if (RESTRICTION_BAND_1(pAd,channel,bw))
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
//sachin - TODO
		pAd->ChannelList[j].Channel = pChDesp->FirstChannel + i * increment;
		pAd->ChannelList[j].MaxTxPwr = pChDesp->MaxTxPwr;
		pAd->ChannelList[j].DfsReq = pChDesp->DfsReq;
		pAd->ChannelList[j].RegulatoryDomain = regulatoryDomain;
#ifdef DOT11_N_SUPPORT
		if (N_ChannelGroupCheck(pAd, pAd->ChannelList[j].Channel))
			pAd->ChannelList[j].Flags |= CHANNEL_40M_CAP;
#ifdef DOT11_VHT_AC
		if (vht80_channel_group(pAd, pAd->ChannelList[j].Channel))
			pAd->ChannelList[j].Flags |= CHANNEL_80M_CAP;

		if (vht160_channel_group(pAd, pAd->ChannelList[j].Channel))
            pAd->ChannelList[j].Flags |= CHANNEL_160M_CAP;

#endif /* DOT11_VHT_AC */	
#endif /* DOT11_N_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		CFG80211OS_ChanInfoInit(
					pAd->pCfg80211_CB,
					j,
					pAd->ChannelList[j].Channel,
					pAd->ChannelList[j].MaxTxPwr,
					WMODE_CAP_N(PhyMode),
					(bw == BW_20));
#endif /* RT_CFG80211_SUPPORT */

		j++;
	}
	pAd->ChannelListNum = j;

	return j;
}


static UCHAR CeateChListByRf(RTMP_ADAPTER *pAd,UCHAR RfIC,PCH_REGION pChRegion,UCHAR Geography, UCHAR offset)
{
	UCHAR i;
	PCH_DESP pChDesp;
	UCHAR ChType;
	UCHAR increment;
	UCHAR regulatoryDomain;
	BOOLEAN IsRfSupport = HcIsRfSupport(pAd,RfIC);
	UCHAR PhyMode = HcGetPhyModeByRf(pAd,RfIC);

	if(IsRfSupport)
	{
		ChBandCheck(PhyMode, &ChType);

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
					regulatoryDomain = pChRegion->op_class_region;
				
				offset = FillChList(pAd, &pChDesp[i], offset, increment, regulatoryDomain,PhyMode);
		        }
		}
	}
	return offset;
}


static inline VOID CreateChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_REGION pChRegion,
	IN UCHAR Geography)
{
	UCHAR offset = 0;
//	INT i,PhyIdx;	
//	PCH_DESP pChDesp;
//	UCHAR ChType;
//	UCHAR increment;
//	UCHAR regulatoryDomain;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN Is2GRun = HcIsRfRun(pAd,RFIC_24GHZ);
	BOOLEAN Is5GRun = HcIsRfRun(pAd,RFIC_5GHZ); 
#endif /* CONFIG_AP_SUPPORT */

	if (pChRegion == NULL)
		return;
	
#ifdef CONFIG_AP_SUPPORT
	if(Is2GRun)
		offset = CeateChListByRf(pAd,RFIC_24GHZ,pChRegion,Geography,offset);
	if(Is5GRun)
		offset = CeateChListByRf(pAd,RFIC_5GHZ,pChRegion,Geography,offset);
#endif /* CONFIG_STA_SUPPORT */
	
	

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
	IN struct wifi_dev *wdev,
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

	ChBandCheck(wdev->PhyMode, &ChType);
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

BOOLEAN GetEDCCASupport(
    IN PRTMP_ADAPTER pAd)
{
    BOOLEAN ret = FALSE;
    PCH_REGION pChReg;

    pChReg = GetChRegion(pAd->CommonCfg.CountryCode);

    if ((pChReg->op_class_region != FCC) && (pChReg->edcca_on == TRUE) )
    {
        // actually need to check PM's table in CE country
        ret = TRUE;
    }
	
    return ret;

}

UCHAR GetCountryRegionFromCountryCode(
	IN UCHAR* country_code)
{
	UCHAR ret=FCC;
	PCH_REGION pChReg;
	
	pChReg = GetChRegion(country_code);
	ret = pChReg->op_class_region;
	
	return ret;
}

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

BOOLEAN ExtChCheck(
    IN PRTMP_ADAPTER pAd, 
    IN UCHAR Channel,
    IN UCHAR Direction)
{
    UCHAR ExtCh;
        
    /* Get extension channel by current direction */         
    ExtCh = GetExtCh(Channel, Direction);

    /* Check whether current extension channel is in channel list or not */
    if (IsValidChannel(pAd, ExtCh))
        return TRUE;
    else
        return FALSE;
}

BOOLEAN N_ChannelGroupCheck(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
	BOOLEAN	RetVal = FALSE;
	
	if (Channel > 14)
	{
		if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
		    (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 140)|| (Channel == 149) || (Channel == 157))
		{
			RetVal = TRUE;
		}
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
				(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 144) || (Channel == 153) || (Channel == 161))
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


UCHAR get_channel_bw_by_rule(struct wifi_dev *wdev, UCHAR bwByCap)
{
	UCHAR bwByRule = BW_20;
	UCHAR bwByCfg = wlan_config_get_vht_bw(wdev);	
    
	if (bwByCfg == VHT_BW_2040)
    {
    	if (wlan_config_get_ht_bw(wdev) == HT_BW_20)
       		bwByCfg = BW_20;
        else
            bwByCfg = BW_40;
    }
    else if (bwByCfg == VHT_BW_80)
        bwByCfg = BW_80;
    else if (bwByCfg == VHT_BW_160)
        bwByCfg = BW_160;
    else if (bwByCfg == VHT_BW_8080)
        bwByCfg = BW_8080;

    /* check per-Wdev Bw with Channel Bw from region */
    if (bwByCfg == BW_8080)
    {
        if (bwByCap == BW_80)
        	bwByRule = BW_8080;
        else if (bwByCap == BW_160)
            bwByRule = BW_8080;
        else
            bwByRule = bwByCap;

        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: IN BW8080 wdev(%d)  %d [%d, %d] %d\n"
                ,__FUNCTION__, wdev->wdev_idx, wlan_operate_get_bw(wdev), bwByCfg, bwByCap, bwByRule));
    }
    else
    {
        bwByRule = (bwByCap > bwByCfg) ? bwByCfg : bwByCap;
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wdev(%d)  %d [%d, %d] %d\n"
               ,__FUNCTION__, wdev->wdev_idx, wlan_operate_get_bw(wdev), bwByCfg, bwByCap, bwByRule));
    }

	return bwByRule;

}

INT8 get_max_vht_bw_by_region(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad;

	UCHAR i = 0;
	INT8 vht_bw;
	UCHAR vht_bw_by_rule;
	UCHAR vht_bw_cap = BW_20;
	BOOLEAN bFoundCh = FALSE;
	
	ad = wdev->sys_handle;

	if (wdev->channel<= 14)
		return -1;

	for (i = 0; (i < ad->ChannelListNum) && (i < MAX_NUM_OF_CHANNELS); i++)
	{
		if (wdev->channel == ad->ChannelList[i].Channel)
		{
			bFoundCh = TRUE;
			break;
		}
	}

	if (bFoundCh == FALSE) 
		return -1;

	if (ad->ChannelList[i].Flags & CHANNEL_160M_CAP)
		vht_bw_cap = BW_160;
	else if (ad->ChannelList[i].Flags & CHANNEL_80M_CAP)
		vht_bw_cap = BW_80;
	else if (ad->ChannelList[i].Flags & CHANNEL_40M_CAP)
		vht_bw_cap = BW_40;

	vht_bw_by_rule = get_channel_bw_by_rule(wdev,vht_bw_cap);

	switch (vht_bw_by_rule)
	{
		case BW_40:
			vht_bw = VHT_BW_2040;
			break;
		case BW_80:
			vht_bw = VHT_BW_80;	
			break;
		case BW_8080:
			vht_bw = VHT_BW_8080;
			break;
		case BW_160:
			vht_bw = VHT_BW_160;
			break;
		default:
			vht_bw = VHT_BW_2040;			
			break;
	}		
	
	return vht_bw;
}

VOID N_ChannelCheck(RTMP_ADAPTER *pAd, UCHAR PhyMode, UCHAR Channel)
{
	INT idx;
	unsigned char i;
	struct wifi_dev *wdev;
	UCHAR op_ht_bw;
	UCHAR ext_cha;
	static const UCHAR wfa_ht_ch_ext[] = {
			36, EXTCHA_ABOVE, 40, EXTCHA_BELOW,
			44, EXTCHA_ABOVE, 48, EXTCHA_BELOW,
			52, EXTCHA_ABOVE, 56, EXTCHA_BELOW,
			60, EXTCHA_ABOVE, 64, EXTCHA_BELOW,
			100, EXTCHA_ABOVE, 104, EXTCHA_BELOW,
			108, EXTCHA_ABOVE, 112, EXTCHA_BELOW,
			116, EXTCHA_ABOVE, 120, EXTCHA_BELOW,
			124, EXTCHA_ABOVE, 128, EXTCHA_BELOW,
			132, EXTCHA_ABOVE, 136, EXTCHA_BELOW,
			140, EXTCHA_ABOVE, 144, EXTCHA_BELOW,
			149, EXTCHA_ABOVE, 153, EXTCHA_BELOW, 
			157, EXTCHA_ABOVE, 161, EXTCHA_BELOW,
			0, 0};

	if (!WMODE_CAP_N(PhyMode))
		return;

	for(i=0;i<WDEV_NUM_MAX;i++){
		wdev = pAd->wdev_list[i];
		if(!wdev || (wdev->PhyMode!=PhyMode) || (wdev->channel != Channel))
			continue;

		op_ht_bw = wlan_operate_get_ht_bw(wdev);
		ext_cha = wlan_operate_get_ext_cha(wdev);

		if(op_ht_bw == BW_40)
		{
			if (Channel > 14)
			{
				idx = 0;
				while(wfa_ht_ch_ext[idx] != 0) {
					if (wfa_ht_ch_ext[idx] == Channel && IsValidChannel(pAd, GetExtCh(Channel, wfa_ht_ch_ext[idx + 1]))) {
						ext_cha = wfa_ht_ch_ext[idx + 1];
						break;
					}
					idx += 2;
				};
				if (wfa_ht_ch_ext[idx] == 0)
				{
					op_ht_bw = HT_BW_20;
					ext_cha = EXTCHA_NONE;
				}
			}
			else
			{
				do
				{
					UCHAR ExtCh;
					UCHAR Dir = ext_cha;
					if(Dir == EXTCHA_NONE){
						Dir = EXTCHA_ABOVE;
					}
					ExtCh = GetExtCh(Channel, Dir);
					if (IsValidChannel(pAd, ExtCh)){
						ext_cha = Dir;
						break;
					}
					Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
					ExtCh = GetExtCh(Channel, Dir);
					if (IsValidChannel(pAd, ExtCh))
					{
						ext_cha = Dir;
						break;
					}
					op_ht_bw = HT_BW_20;
				} while(FALSE);

				if (Channel == 14)
				{
					op_ht_bw = HT_BW_20;
				}
			}
		}else
		if (op_ht_bw == BW_20)
		{
			ext_cha = EXTCHA_NONE;
		}
		wlan_operate_set_ht_bw(wdev,op_ht_bw);
		wlan_operate_set_ext_cha(wdev,ext_cha);
	}
}


UCHAR N_SetCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch, UCHAR ht_bw)
{
	UCHAR op_ext_cha = EXTCHA_NONE;

	op_ext_cha = HcGetExtCha(pAd, prim_ch);

	if (ht_bw== BW_40)
	{
		if (op_ext_cha == EXTCHA_ABOVE)
			pAd->CommonCfg.CentralChannel = prim_ch + 2;
		else
		{
			if (prim_ch == 14)
				pAd->CommonCfg.CentralChannel = prim_ch - 1;
			else
				pAd->CommonCfg.CentralChannel = prim_ch - 2;
		}
	}
	else
		pAd->CommonCfg.CentralChannel = prim_ch;

	return pAd->CommonCfg.CentralChannel;
}
#endif /* DOT11_N_SUPPORT */


UINT8 GetCuntryMaxTxPwr(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR PhyMode,
	IN UINT8 channel,
	IN UCHAR ht_bw)
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
		UCHAR TxStream;

		if (pAd->CommonCfg.dbdc_mode)
		{
			UCHAR band_idx = HcGetBandByWdev(wdev);

			if (band_idx == DBDC_BAND0)
				TxStream = pAd->dbdc_2G_Tx_stream;
			else
				TxStream = pAd->dbdc_5G_Tx_stream;
		} else {
			TxStream = pAd->Antenna.field.TxPath;
		}

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) && (TxStream == 2))
			deltaTxStreamPwr = 3; /* If 2Tx case, antenna gain will increase 3dBm*/
#endif /* DOT11_N_SUPPORT */

		if (pAd->ChannelList[i].RegulatoryDomain == FCC)
		{
			/* FCC should maintain 20/40 Bandwidth, and without antenna gain */
#ifdef DOT11_N_SUPPORT
			if (WMODE_CAP_N(PhyMode) &&
				(ht_bw == BW_40) &&
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

INT32 ChannelFreqToGroup(UINT32 ChannelFreq)
{
    INT32 GroupIndex = 0;    

    if (ChannelFreq <= 2484) /* 2G CH14 = 2484 */
    {
        GroupIndex = 0;
    }
    else if ((4850 <= ChannelFreq) && (ChannelFreq <= 5140))
    {
        GroupIndex = 1;
    }
    else if ((5145 <= ChannelFreq) && (ChannelFreq <= 5250))
    {
        GroupIndex = 2;
    }
    else if ((5255 <= ChannelFreq) && (ChannelFreq <= 5360))
    {
        GroupIndex = 3;
    }
    else if ((5365 <= ChannelFreq) && (ChannelFreq <= 5470))
    {
        GroupIndex = 4;
    }
    else if ((5475 <= ChannelFreq) && (ChannelFreq <= 5580))
    {
        GroupIndex = 5;
    }
    else if ((5585 <= ChannelFreq) && (ChannelFreq <= 5690))
    {
        GroupIndex = 6;
    }
    else if ((5695 <= ChannelFreq) && (ChannelFreq <= 5800))
    {
        GroupIndex = 7;
    }
    else if ((5805 <= ChannelFreq) && (ChannelFreq <= 5950))
    {
        GroupIndex = 8;
    }
    else
    {
    	GroupIndex = -1;
    	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("%s : Can't find group for [%d]. \n", __FUNCTION__,ChannelFreq));
    }

    return GroupIndex;
}

BOOLEAN MTChGrpValid(
    IN PRTMP_ADAPTER pAd)
{
	return (pAd->CommonCfg.ChGrpChannelNum > 0 &&
		pAd->CommonCfg.ChGrpChannelNum < MAX_NUM_OF_CHANNELS &&
		pAd->CommonCfg.ChGrpEn != 0);
}

void MTSetChGrp(RTMP_ADAPTER *pAd, RTMP_STRING *buf)
{
			UCHAR i,j;
			UCHAR  *macptr;
			BOOLEAN bEnable;
			PCH_GROUP_DESC  pChDescGrp;
			UCHAR TotalChNum;

			pAd->CommonCfg.ChGrpChannelNum=0;
			TotalChNum=0;
			NdisZeroMemory(pAd->CommonCfg.ChGrpChannelList,(MAX_NUM_OF_CHANNELS)*sizeof(UCHAR));
			pAd->CommonCfg.ChGrpEn=0;
			
			for (i = 0, macptr = rstrtok(buf,";");
			(macptr && i < 4);
			macptr = rstrtok(NULL,";"), i++)
			{
				bEnable=0;
				bEnable = (UCHAR) simple_strtol(macptr, 0, 10);
				pChDescGrp = &(Channel_GRP[i]);
				
				if (bEnable == 1)
				{
					if ((TotalChNum + pChDescGrp->NumOfCh) < MAX_NUM_OF_CHANNELS)
					{
					pAd->CommonCfg.ChGrpEn |= ( 1<< i );

					for(j = 0; j < pChDescGrp->NumOfCh; j++)
					{
							if ((TotalChNum + j) < MAX_NUM_OF_CHANNELS)
						pAd->CommonCfg.ChGrpChannelList[TotalChNum + j] = pChDescGrp->FirstChannel + (j * 4);
					}
					TotalChNum +=pChDescGrp->NumOfCh;
				}
		    	}
		    	}

			pAd->CommonCfg.ChGrpChannelNum=TotalChNum;
						
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TriBandChGrp=%d/%d/%d/%d\n", 
						((pAd->CommonCfg.ChGrpEn & (CH_GROUP_BAND0)) ? 1: 0),
						((pAd->CommonCfg.ChGrpEn & (CH_GROUP_BAND1)) ? 1: 0),
						((pAd->CommonCfg.ChGrpEn & (CH_GROUP_BAND2)) ? 1: 0),
						((pAd->CommonCfg.ChGrpEn & (CH_GROUP_BAND3)) ? 1: 0)));

}	

BOOLEAN MTChGrpChannelChk(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR ch)
{
	UCHAR i;
	BOOLEAN result = FALSE;
	
	for (i = 0; i < pAd->CommonCfg.ChGrpChannelNum; i++)
	{
		if (ch == pAd->CommonCfg.ChGrpChannelList[i])
		{
			result = TRUE;
			break;
		}
			
	}	
	return result;
}
