/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_dfs.c

    Abstract:
    Support DFS function.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/

#include "rt_config.h"

#ifdef DFS_SUPPORT
#ifdef CONFIG_AP_SUPPORT

#ifdef DFS_DEBUG
NewDFSDebugResult TestResult[1000];
#endif

NewDFSValidRadar NewDFSValidTable[] = 
{
	/* FCC-1  && (Japan W53 Radar 1 / W56 Radar 2)*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP | NEW_DFS_JAP_W53),
	7,
	10, 1000,
	{0, 0, 0, 0, 0},
	4,
	0, 0, 
	{28570 - 70, 0, 0, 0, 0},
	150
	},
	/* FCC-2*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	7,
	13, 1000,
	{0, 0, 0, 0, 0},
	1,
	3000, 4600 - 20,
	{0, 0, 0, 0, 0},
	25
	},
	/* FCC-3 & FCC-4*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	7,
	/*120, 200, FCC-3 */
	/*220, 400, FCC-4*/
	100, 1500, 
	{0, 0, 0, 0, 0},
	1,
	4000, 10000 - 40, 
	{0, 0, 0, 0, 0},
	60
	},
	/* FCC-6*/
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	7,
	12, 1000,
	{0, 0, 0, 0, 0},
	1,
	0, 0, 
	{6660-10, 0, 0, 0, 0},
	35
	},
	/* Japan W53 Radar 2*/
	{
	NEW_DFS_JAP_W53,
	7,
	40, 1000, 
	{0, 0, 0, 0, 0},
	1,
	0, 0, 
	{76923 - 30, 0, 0, 0, 0},
	180
	},
	/* Japan W56 Radar 1*/
	{
	NEW_DFS_JAP,
	7,
	5, 500, 
	{0, 0, 0, 0, 0},
	2,
	0, 0, 
	{27777 - 30, 0, 0, 0, 0},
	70
	},
	/* Japan W56 Radar 3*/
	{
	NEW_DFS_JAP,
	7,
	30, 1000, 
	{0, 0, 0, 0, 0},
	1,
	0, 0, 
	{80000 - 50, 0, 0, 0, 0},
	200
	},

/* CE Staggered radar*/

	{
	/*	EN-1*/
	/*	width	0.8 - 5 us*/
	/*	PRF		200 - 1000 Hz*/
	/*	PRI		5000 - 1000 us	(T: 20000 - 100000)*/
	/*	*/
	NEW_DFS_EU,
	0xf,
	10, 1000, 
	{0, 0, 0, 0, 0},
	1,
	20000-15, 100000-70, 
	{0, 0, 0, 0, 0},
	120
	},
	/*	EN-2*/
	/*	width	0.8 - 15 us*/
	/*	PRF		200 - 1600 Hz*/
	/*	PRI		5000 - 625 us	(T: 12500 - 100000)*/
	{
	NEW_DFS_EU,
	0xf,
	10, 2000, 
	{0, 0, 0, 0, 0},
	1,
	12500 - 10, 100000 - 70, 
	{0, 0, 0, 0, 0},
	120
	},
	
	/*	EN-3*/
	/*	width	0.8 - 15 us*/
	/*	PRF		2300 - 4000 Hz*/
	/*	PRI		434 - 250 us	(T: 5000 - 8695)*/
	{
	NEW_DFS_EU,
	0xf,
	21, 2000, 
	{0, 0, 0, 0, 0},
	1,
	5000 - 4, 8695 - 7, 
	{0, 0, 0, 0, 0},
	50
	},
	/*	EN-4*/
	/*	width	20 - 30 us*/
	/*	PRF		2000 - 4000 Hz*/
	/*	PRI		500 - 250 us	(T: 5000 - 10000)*/
	/*	Note : with Chirp Modulation +- 2,5Mhz*/
	{
	NEW_DFS_EU,
	0xf,
	380, 3000, 
	{0, 0, 0, 0, 0},
	4,
	5000 - 4, 10000 - 8, 
	{0, 0, 0, 0, 0},
	60
	},
	/*	EN-5*/
	/*	width	0.8 - 2 us*/
	/*	PRF		300 - 400 Hz*/
	/*	PRI		3333 - 2500 us	(T: 50000 - 66666)*/
	/*	Staggered PRF, 20 - 50 pps*/
	{
	NEW_DFS_EU,
	0xf,
	10, 800, 
	{0, 0, 0, 0, 0},
	1,
	50000 - 35, 66666 - 50, 
	{0, 0, 0, 0, 0},
	90
	},
	/*	EN-6*/
	/*	width	0.8 - 2 us*/
	/*	PRF		400 - 1200 Hz*/
	/*	PRI		2500 - 833 us	(T: 16666 - 50000)*/
	/*	Staggered PRF, 80 - 400 pps*/
	{
	NEW_DFS_EU,
	0xf,
	10, 800, 
	{0, 0, 0, 0, 0},
	1,
	16666 - 13, 50000 - 35, 
	{0, 0, 0, 0, 0},
	80
	},
	
	{
	NEW_DFS_END,
	0,
	0, 0, 
	{0, 0, 0, 0, 0},
	0,
	0, 0, 
	{0, 0, 0, 0, 0},
	0,
	},
};

static NewDFSTable NewDFSTable1[] = 
{
	{
		/* ch, mode(0~7), M(~511), el, eh(~4095), wl, wh(~4095), err_w, tl, th, err_t, bl, bh*/

		NEW_DFS_FCC,
		{
		{0, 0,  10,   8,  16,   6, 2000,  5, 2900, 30000,  5, 0, 0},
		{1, 0,  70,  42, 126,  20, 5000,  5, 2900, 30000, 10, 0, 0},
		{2, 0, 100,  42, 160,  20, 5000, 25, 2900, 30000, 20, 0, 0},
		{3, 2, 200,  20, 150, 900, 2200, 50, 1000, 999999999, 200, 0, 999999999},
		}
	},

    {
        NEW_DFS_EU,
        {
	    {0, 0,  10,  10,  18,   4, 1000,   5,  7800, 101000,   5, 0, 0},
        {1, 0,  70,  42,  90,  20, 5000,   3,  4900, 101000,  10, 0, 0},
        {2, 0, 100,  42, 160,  20, 5000,   5,  4900, 101000,  20, 0, 0},
        {3, 3, 200,  20, 150, 300,  850, 100,  4900,  11000, 200, 0, 0},         
		}
    },

	{	
		NEW_DFS_JAP,
		{
		{0, 0, 10,   8,  16,    4, 2000,  5,  4500, 85000,   5,  0,	0},
		{1, 0, 70,  48, 126,   20, 5000,  5,  2500, 85000,  10,  0,	0},
		{2, 0, 100, 48, 160,   20, 5000, 25,  2500, 85000,  20,  0,	0},
		{3, 2, 200, 20, 150,  900, 2200, 50,  1000, 999999999,	200,  0, 999999999},
		}
	},

	{
		NEW_DFS_JAP_W53,
		{
		{0, 0,  10,   8,  16,   8,  2000,  5, 28000, 85000, 10},
		{1, 0, 32,  24,  64,   20, 2000, 5,  28000, 85000, 10},
		{2, 0, 100,  42, 160,   20, 2000, 25,  28000, 85000, 10},
		/*{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200},*/
		}
	},	
};

static void dfs_sw_init(
		IN PRTMP_ADAPTER pAd);

static BOOLEAN StagerRadarCheck(
		IN PRTMP_ADAPTER pAd,
		UINT8 dfs_channel);

static BOOLEAN ChirpRadarCheck(
		IN PRTMP_ADAPTER pAd);

static VOID SwCheckDfsEvent(
		IN PRTMP_ADAPTER pAd);

static VOID DfsCheckBusyIdle(
		IN PRTMP_ADAPTER pAd);

static BOOLEAN DfsChannelCheck(
		IN PRTMP_ADAPTER pAd,
		IN UINT8 DfsChannel);

static VOID ChannelSelectOnRadarDetection(
		IN PRTMP_ADAPTER pAd);


ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};

	if (pAd->CommonCfg.RDDurRegion != JAP)
	{
		return pAd->CommonCfg.RDDurRegion;
	}

	for (i=0; i<15; i++)
	{
		if (pAd->CommonCfg.Channel == Channel[i])
		{
			break;
		}
	}

	if (i < 4)
		return JAP_W53;
	else if (i < 15)
		return JAP_W56;
	else
		return JAP; /* W52*/

}

/*
    ========================================================================
    Routine Description:
        Radar wave detection. The API should be invoke each second.
        
    Arguments:
        pAd         - Adapter pointer
        
    Return Value:
        None
        
    ========================================================================
*/
VOID ApRadarDetectPeriodic(
	IN PRTMP_ADAPTER pAd)
{
	INT	i;

	pAd->CommonCfg.InServiceMonitorCount++;

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].RemainingTimeForUse > 0)
		{
			pAd->ChannelList[i].RemainingTimeForUse --;
			if ((pAd->Mlme.PeriodicRound%5) == 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("RadarDetectPeriodic - ch=%d, RemainingTimeForUse=%d\n", pAd->ChannelList[i].Channel, pAd->ChannelList[i].RemainingTimeForUse));
			}
		}
	}
	/*radar detect*/
	if ((pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
	{
		RadarDetectPeriodic(pAd);
	}
	return;
}

/* Periodic Radar detection, switch channel will occur in RTMPHandleTBTTInterrupt()*/
/* Before switch channel, driver needs doing channel switch announcement.*/
VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd)
{
	/* need to check channel availability, after switch channel*/
	if (pAd->CommonCfg.RDMode != RD_SILENCE_MODE)
			return;

	/* channel availability check time is 60sec, use 65 for assurance*/
	if (pAd->CommonCfg.RDCount++ > pAd->CommonCfg.ChMovingTime)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Not found radar signal, start send beacon and radar detection in service monitor\n\n"));
		AsicEnableBssSync(pAd);
		pAd->CommonCfg.RDMode = RD_NORMAL_MODE;
		return;
	}
}

/* 	0 = Switch Channel when Radar Hit (Normal mode) 
	1 = Don't Switch Channel when Radar Hit */
INT	Set_RadarDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	pRadarDetect->McuRadarDebug = simple_strtol(arg, 0, 16);
	
	if (pRadarDetect->McuRadarDebug & RADAR_DONT_SWITCH)
		printk("Dont Switch Channel\n");
	
	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_SILENCE)
		printk("Silence\n");
	
	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_EVENT)
		printk("Show Long pulse Event\n");
	
	if (pRadarDetect->McuRadarDebug & RADAR_DEBUG_SW_SILENCE)
		printk("SW detection Silence\n");
	return TRUE;
}

INT	Set_ResetRadarHwDetect_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR channel = 0;
	/*Set BBP_R140=0x02 and Read BBP_R141 to store at channel */
	RTMP_DFS_IO_READ8(pAd, 0x2, &channel);
	/* reset the radar channel for new counting */
	RTMP_DFS_IO_WRITE8(pAd, 0x2, channel);
	printk("==>reset the radar channel for new counting channel =0x%x\n",channel);
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
	pAd->CommonCfg.CSPeriod = (USHORT) simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->CommonCfg.CSPeriod));

	return TRUE;
}

INT	Set_DfsLowerLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	pRadarDetect->DfsLowerLimit = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsUpperLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	pRadarDetect->DfsUpperLimit = simple_strtol(arg, 0, 10);
	return TRUE;
}


/* Dump event buffer*/
/* 0 = Don't Log, 1 = Start Log, 2 = Dump Log */
INT	Set_RadarLog_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int k;
	UCHAR BBPR127;
	
	/*BBP_IO_WRITE8_BY_REG_ID(pAd, 126, (pAd->CommonCfg.R126 & 0xfe)); */
	/*DBGPRINT(RT_DEBUG_TRACE, ("Write R126 = %x\n", (pAd->CommonCfg.R126 & 0xfe))); */
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
	
	return TRUE;
}

INT Set_DfsSwDisable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	
	pRadarDetect->bDfsSwDisable = simple_strtol(arg, 0, 10);
	DBGPRINT(RT_DEBUG_TRACE, ("pRadarDetect->bDfsSwDisable = %u\n", pRadarDetect->bDfsSwDisable));
	return TRUE;
}

INT	Set_RadarStart_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;
	
	if (simple_strtol(arg, 0, 10) == 0)
	{
		NewRadarDetectionStart(pAd);
	}
	else if ((simple_strtol(arg, 0, 10) >= 1) && (simple_strtol(arg, 0, 10) <= NEW_DFS_ENABLE_MASK))
	{
		pDfsProgramParam->ChEnable = simple_strtol(arg, 0, 10);
		printk("Ch Enable == 0x%x\n", pDfsProgramParam->ChEnable);
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
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;
	
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
			pDfsProgramParam->DeltaDelay = value;
			printk("Delta_Delay = %d\n", pDfsProgramParam->DeltaDelay);
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
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->fcc_5_threshold = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_ChBusyThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i;
	PUCHAR p1 = arg, p2;
	ULONG value;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	
	pRadarDetect->fdf_num = 0;
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
			
			pRadarDetect->ch_busy_threshold[i] = value;
			printk("pRadarDetect->ch_busy_threshold[%d] = %d\n", i, pRadarDetect->ch_busy_threshold[i]);
			pRadarDetect->fdf_num++;
			p1 = p2 + 1;
		}
		else
		{
			pRadarDetect->ch_busy_threshold[i] = simple_strtol(p1, 0, 10);
			printk("pRadarDetect->ch_busy_threshold[%d] = %d\n", i, pRadarDetect->ch_busy_threshold[i]);
			pRadarDetect->fdf_num++;
			break;
		}
	}
	printk("pRadarDetect->fdf_num = %d\n", pRadarDetect->fdf_num);

	return TRUE;
}


INT	Set_RssiThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i;
	PUCHAR p1 = arg, p2;
	ULONG value;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	
	pRadarDetect->fdf_num = 0;
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
			
			pRadarDetect->rssi_threshold[i] = value;
			printk("pRadarDetect->rssi_threshold[%d] = %d\n", i, pRadarDetect->rssi_threshold[i]);
			pRadarDetect->fdf_num++;
			p1 = p2 + 1;
		}
		else
		{
			pRadarDetect->rssi_threshold[i] = simple_strtol(p1, 0, 10);
			printk("pRadarDetect->rssi_threshold[%d] = %d\n", i, pRadarDetect->rssi_threshold[i]);
			pRadarDetect->fdf_num++;
			break;
		}
	}
	printk("pRadarDetect->fdf_num = %d\n", pRadarDetect->fdf_num);

	return TRUE;
	pRadarDetect->rssi_threshold[0] = simple_strtol(arg, 0, 10);

	return TRUE;
}

INT	Set_PollTime_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->PollTime = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_WidthShift_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_width_diff_Shift = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch0LErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_width_ch0_err_L = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DeclareThres_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_declare_thres = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_CheckLoop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_check_loop = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_MaxPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_max_period = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_PeriodErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_period_err = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch0HErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_width_ch0_err_H = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch1Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_width_diff_ch1_Shift = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch2Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	pDfsSwParam->dfs_width_diff_ch2_Shift = simple_strtol(arg, 0, 10);
	return TRUE;
}


#ifdef DFS_DEBUG
INT	Set_CEPrintDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	int i;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	
	if (simple_strtol(arg, 0, 10) == 0)
	{
		pDfsSwParam->DebugPortPrint = 1;
	}
	
	if (simple_strtol(arg, 0, 10) == 1)
	{
		if (pDfsSwParam->DebugPortPrint == 3)
		{
			for (i = 0; i < 384; i++)
			{
				printk("%02x ", pDfsSwParam->DebugPort[i]);
				if (((i+1) % 18) == 0)
					printk("\n");
			}
			pDfsSwParam->DebugPortPrint = 0;
		}
	}
	return TRUE;
}
#endif /* DFS_DEBUG */


INT	Set_RadarSim_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
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

	pDfsSwParam->dfs_w_counter++;
	for (i = 0; i < 10; i++)
	{
		pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].counter = pDfsSwParam->dfs_w_counter;
		pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].timestamp = dfs_data[2*i+1];
		pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].width = dfs_data[2*i];
		pDfsSwParam->dfs_w_last_idx[id] = pDfsSwParam->dfs_w_idx[id];
		pDfsSwParam->dfs_w_idx[id]++;
		if (pDfsSwParam->dfs_w_idx[id] >= NEW_DFS_DBG_PORT_ENT_NUM)
			pDfsSwParam->dfs_w_idx[id] = 0;	
	}
	pDfsSwParam->hw_idx[0] = pDfsSwParam->dfs_w_idx[0];
	SWRadarCheck(pAd, 0);
	return TRUE;
}

INT	Set_PrintBusyIdle_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->print_ch_busy_sta = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_BusyIdleRatio_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->ch_busy_idle_ratio = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsRssiHigh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->DfsRssiHigh = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_DfsRssiLow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	pRadarDetect->DfsRssiLow = simple_strtol(arg, 0, 10);
	return TRUE;
}

INT	Set_Ch0EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING                 arg) 
{
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;

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
	pDfsProgramParam->RadarEventExpire[0] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 0);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));


		return TRUE;
}

INT	Set_Ch1EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING                 arg) 
{
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;

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
	pDfsProgramParam->RadarEventExpire[1] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 1);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));

		return TRUE;
}

INT	Set_Ch2EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING                 arg) 
{
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;

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
	pDfsProgramParam->RadarEventExpire[2] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 2);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));


		return TRUE;
}

INT	Set_Ch3EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING                 arg) 
{
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pAd->CommonCfg.RadarDetect.DfsProgramParam;

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
	pDfsProgramParam->RadarEventExpire[3] = a;

	RTMP_DFS_IO_WRITE8(pAd, 0x0, 3);
	RTMP_DFS_IO_WRITE8(pAd,0x39, (a & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3a, ((a >> 8) & 0xff) );
 	RTMP_DFS_IO_WRITE8(pAd,0x3b, ((a >> 16) & 0xff));
	RTMP_DFS_IO_WRITE8(pAd,0x3c, ((a >> 24) & 0xff));


		return TRUE;
}

INT	Set_CEPrint_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pAd->CommonCfg.RadarDetect.DfsSwParam;
	int i, j, id = simple_strtol(arg, 0, 10);
	int n = pDfsSwParam->dfs_w_last_idx[id];

	if ((id >= 0) && (id <= 3))
	{
		printk("Last Idx = %d\n", n);
		for (i = 0; i < NEW_DFS_DBG_PORT_ENT_NUM; i++)
		{
			printk("Cnt=%u, w= %u  Time=%u\n", (unsigned int)(pDfsSwParam->DFS_W[id][i].counter),
			pDfsSwParam->DFS_W[id][i].width,
			(unsigned int)pDfsSwParam->DFS_W[id][i].timestamp);
			if (pDfsSwParam->DFS_W[id][i].counter == 0)
				break;
		}
	}
		
	if ((id >= 10) && (id < 13))
	{
		id -= 10;
		for (i = 0; i < NEW_DFS_MPERIOD_ENT_NUM; i++)
		{
			printk("T=%u, w1= %u(%u), w2= %u(%u)\n", (unsigned int)(pDfsSwParam->DFS_T[id][i].period),
			pDfsSwParam->DFS_T[id][i].width, pDfsSwParam->DFS_T[id][i].idx,
			pDfsSwParam->DFS_T[id][i].width2, pDfsSwParam->DFS_T[id][i].idx2);
			if (pDfsSwParam->DFS_T[id][i].period == 0)
				break;
		}
		
	}
	if (id == 77)
	{
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < NEW_DFS_DBG_PORT_ENT_NUM; j++)
			{
				pDfsSwParam->DFS_W[i][j].counter = 0;
				pDfsSwParam->DFS_W[i][j].width = 0;
				pDfsSwParam->DFS_W[i][j].timestamp = 0;
			}
			for (j = 0; j < NEW_DFS_MPERIOD_ENT_NUM; j++)
			{
				pDfsSwParam->DFS_T[i][j].period = 0;	
				pDfsSwParam->DFS_T[i][j].width = 0;
				pDfsSwParam->DFS_T[i][j].width2 = 0;	
				pDfsSwParam->DFS_T[i][j].idx = 0;
				pDfsSwParam->DFS_T[i][j].idx2 = 0;
			}
		}
	}
#ifdef DFS_DEBUG	
	if (id > 20)
	{
		pDfsSwParam->BBP127Repeat = id - 20;
	}
	
	if (id == 5)
	{
		
		for (i = 0; i < NEW_DFS_DBG_PORT_ENT_NUM; i++)
		{
			printk("Cnt=%u, w= %u  Time=%lu,  start_idx = %u end_idx = %u\n", (unsigned int)(pDfsSwParam->CE_DebugCh0[i].counter),
			(unsigned int)pDfsSwParam->CE_DebugCh0[i].width, pDfsSwParam->CE_DebugCh0[i].timestamp,
			pDfsSwParam->CE_DebugCh0[i].start_idx, pDfsSwParam->CE_DebugCh0[i].end_idx);
		}

		for (i = 0; i < NEW_DFS_MPERIOD_ENT_NUM; i++)
		{
			printk("T=%lu, w1= %u(%u), w2= %u(%u)\n", (pDfsSwParam->CE_TCh0[i].period),
			pDfsSwParam->CE_TCh0[i].width, pDfsSwParam->CE_TCh0[i].idx,
			pDfsSwParam->CE_TCh0[i].width2, pDfsSwParam->CE_TCh0[i].idx2);
		}

	}
#endif /* DFS_DEBUG */
	
	return TRUE;
}


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

/* 
    ==========================================================================
    Description:
		change channel moving time for DFS testing.

	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	UINT8 Value;

	Value = (UINT8) simple_strtol(arg, 0, 10);

	pAd->CommonCfg.ChMovingTime = Value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %d\n", __FUNCTION__,
		pAd->CommonCfg.ChMovingTime));

	return TRUE;
}


VOID DFSInit(PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;
	

	pRadarDetect->ch_busy_countdown = -1;
	pRadarDetect->bDfsInit = FALSE;
	pDfsProgramParam->DeltaDelay = 0x3;
	pDfsProgramParam->ChEnable = NEW_DFS_ENABLE_MASK;	
	pAd->CommonCfg.ChMovingTime = 65;
	/*pAd->CommonCfg.RadarReEnable = 1;*/
	pRadarDetect->PollTime = 0;
	pDfsSwParam->fcc_5_threshold = 1000;
	pDfsSwParam->fcc_5_idx = 0;
	pDfsSwParam->fcc_5_last_idx = 0;
	pRadarDetect->DfsRssiHigh = -30;
	pRadarDetect->DfsRssiLow = -90;
	pRadarDetect->DFSWatchDogIsRunning=FALSE;
	{		
		int k;
		for (k = 0; k < pAd->chipCap.DfsEngineNum; k++)
		{
			pDfsSwParam->sw_idx[k] = NEW_DFS_DBG_PORT_ENT_NUM - 1;
			pDfsSwParam->hw_idx[k] = 0;
		}	
	}
	pDfsProgramParam->RadarEventExpire[3] = 0x99999999;
	pDfsProgramParam->Symmetric_Round = 1;
	pDfsProgramParam->VGA_Mask = 0; /*45;*/
	pDfsProgramParam->Packet_End_Mask = 0; /*45;*/
	pDfsProgramParam->Rx_PE_Mask = 0; /*45;*/
	pRadarDetect->PollTime = 3;
	/*pAd->CommonCfg.ce_sw_check = CE_SW_CHECK;*/
	/*pDfsSwParam->ce_sw_id_check = 0;*/
	/*pDfsSwParam->ce_sw_t_diff = 14;*/
	/* from dfs_mcu.c, one time init */
	pRadarDetect->use_tasklet = 1;
	pDfsSwParam->dfs_check_loop = DFS_SW_RADAR_CHECK_LOOP;
	pDfsSwParam->dfs_width_diff_ch1_Shift = DFS_SW_RADAR_CH1_SHIFT;	
	pDfsSwParam->dfs_width_diff_ch2_Shift = DFS_SW_RADAR_CH2_SHIFT;	

	/* Default close s/w detection for new DFS*/
	if (pAd->chipCap.DfsEngineNum > 4)
		pRadarDetect->bDfsSwDisable = TRUE;

	/*pDfsSwParam->dfs_width_diff_Shift = DFS_SW_RADAR_SHIFT;*/
	pDfsSwParam->dfs_width_ch0_err_L = DFS_SW_RADAR_CH0_ERR;
	if (pAd->CommonCfg.RDDurRegion == CE)	
		pDfsSwParam->dfs_period_err = (DFS_SW_RADAR_PERIOD_ERR << 2);
	else
		pDfsSwParam->dfs_period_err = DFS_SW_RADAR_PERIOD_ERR;
	if (pAd->CommonCfg.RDDurRegion == CE)
	{
		pDfsSwParam->dfs_width_ch0_err_H = CE_STAGGERED_RADAR_CH0_H_ERR;
		pDfsSwParam->dfs_declare_thres = CE_STAGGERED_RADAR_DECLARE_THRES;
		pDfsSwParam->dfs_max_period = CE_STAGGERED_RADAR_PERIOD_MAX;
	}
	else	
	{		
		/*pDfsSwParam->dfs_declare_thres = DFS_SW_RADAR_DECLARE_THRES;*/
		if (pAd->CommonCfg.RDDurRegion == FCC)
			pDfsSwParam->dfs_max_period = FCC_RADAR_PERIOD_MAX;
		else if (pAd->CommonCfg.RDDurRegion == JAP)
			pDfsSwParam->dfs_max_period = JAP_RADAR_PERIOD_MAX;
	}

	pRadarDetect->McuRadarDebug = 0;

		pRadarDetect->dfs_func = HARDWARE_DFS_V2;

}

void TimerCB(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	if (pRadarDetect->dfs_func >= HARDWARE_DFS_V1) 
	{
		NewTimerCB_Radar(pAd);
	}
}

VOID NewTimerCB_Radar(
 	IN PRTMP_ADAPTER pAd)
{
	UCHAR channel=0;
	UCHAR radarDeclared = 0;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;

	/* Do not run DFS rotine when initializing DFS BBP or Switching channel (channel switching on radar detection)*/
	if (pRadarDetect->bDfsInit == FALSE ||
		pAd->CommonCfg.DFSAPRestart == 1)
		return;
	
	/* to prevent possible re-entries*/
	pRadarDetect->bDfsInit = FALSE;
	
#ifdef DFS_HWTIMER_SUPPORT
	if (pRadarDetect->PollTime == 0)
	{
		return;
	}

	if (pRadarDetect->RadarTimeStampLow++ == 0xffffffff)
		pRadarDetect->RadarTimeStampHigh++;

		/*511ms*/
	if ((pRadarDetect->RadarTimeStampLow & 0x1ff) == 0)
		DfsCheckBusyIdle(pAd);

	/* 3x511ms~1.5 */
	if ((pRadarDetect->McuRadarTick++ >= pRadarDetect->PollTime) &&
		(!pRadarDetect->ch_busy) &&
		(!pRadarDetect->bDfsSwDisable))
	{
		SwCheckDfsEvent(pAd);
	}
#endif /* DFS_HWTIMER_SUPPORT */

	/*The following codes is used to check if the hardware find the Radar Signal
	*  Read the 0~3 channel which had detected radar signals
	*  Poll Status register
	*  Set BBP_R140=0x02 and Read BBP_R141 to store at channel
	*/
	RTMP_DFS_IO_READ8(pAd, 0x2, &channel);
		/*Check if any interrupt trigger by Radar Global Status(Radar Signals)*/
	if ((channel & NEW_DFS_ENABLE_MASK) && (!pRadarDetect->ch_busy))
	{
		radarDeclared = DfsChannelCheck(pAd, channel);
	}

	/*reset the radar channel for new counting (Write clear) */
	if (channel & NEW_DFS_ENABLE_MASK)
		RTMP_DFS_IO_WRITE8(pAd, 0x2, channel);
	
	if (pRadarDetect->McuRadarDebug & RADAR_SIMULATE)
	{
		radarDeclared = 1;
		pRadarDetect->McuRadarDebug &= ~RADAR_SIMULATE;
	}

	if ((radarDeclared || pRadarDetect->radarDeclared) && (pRadarDetect->ch_busy_countdown == -1))
		pRadarDetect->ch_busy_countdown = 20;
	else if(pRadarDetect->ch_busy_countdown >= 0)
		pRadarDetect->ch_busy_countdown--;

	/*Now, find an Radar signal*/
	if ((!pRadarDetect->ch_busy) && (pRadarDetect->ch_busy_countdown == 0))
	{	
		/* Radar found!!!*/
		pRadarDetect->ch_busy_countdown = -1;
		
		/*Announce that this channel could not use in 30 minutes if we need find a clear channel*/
		if (!(pRadarDetect->McuRadarDebug & RADAR_DONT_SWITCH))
			ChannelSelectOnRadarDetection(pAd);
		else
			pRadarDetect->radarDeclared = 0;			

		/* clear long pulse table */
		pDfsSwParam->FCC_5[pDfsSwParam->fcc_5_idx].counter = 0;
		pDfsSwParam->fcc_5_idx = 0;
		pDfsSwParam->fcc_5_last_idx = 0;
	}
	pRadarDetect->bDfsInit = TRUE;
}

void NewRadarDetectionStart(PRTMP_ADAPTER pAd)
{
	pNewDFSTable pDFS2Table;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;
	
	pRadarDetect->bDfsInit = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("NewRadarDetectionStart2\n"));

	RTMP_CHIP_RADAR_GLRT_COMPENSATE(pAd);

	if ((pAd->CommonCfg.RDDurRegion == CE) && (pAd->CommonCfg.Channel >= 116) && (pAd->CommonCfg.Channel <= 128))
		pAd->CommonCfg.ChMovingTime = 605;
	else
		pAd->CommonCfg.ChMovingTime = 65;


	if (pAd->CommonCfg.RDDurRegion == FCC)
	{
		if (pRadarDetect->ch_busy_idle_ratio == 0)
			pRadarDetect->ch_busy_idle_ratio = 2;
		
		pDFS2Table = &NewDFSTable1[0];
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use FCC table\n"));
	}
	else if (pAd->CommonCfg.RDDurRegion == CE)
	{
		if (pRadarDetect->ch_busy_idle_ratio == 0)
			pRadarDetect->ch_busy_idle_ratio = 3;
		
		pDFS2Table = &NewDFSTable1[1];
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use CE table\n"));
	}
	else /* JAP*/
	{

		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			pDFS2Table = &NewDFSTable1[3];
			
			if (pRadarDetect->ch_busy_idle_ratio == 0)
				pRadarDetect->ch_busy_idle_ratio = 3;
		}
		else
		{
			pDFS2Table = &NewDFSTable1[2];
			pDfsProgramParam->Symmetric_Round = 1;

			if (pRadarDetect->ch_busy_idle_ratio == 0)
				pRadarDetect->ch_busy_idle_ratio = 2;
		}
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use JAP table\n"));
	}	

	NewRadarDetectionProgram(pAd, pDFS2Table);

#ifdef DFS_HWTIMER_SUPPORT
	/* enable debug mode*/
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 3);
#endif /* DFS_HWTIMER_SUPPORT */
	/* reset status */
	RTMP_DFS_IO_WRITE8(pAd, 0x2, NEW_DFS_ENABLE_MASK);
	/* Enable detection*/
	RTMP_DFS_IO_WRITE8(pAd, 0x1, (pDfsProgramParam->ChEnable & NEW_DFS_ENABLE_MASK));
/*the usage of dfs_sw_init*/
	dfs_sw_init(pAd);
#ifdef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
	request_tmr_service(NEW_DFS_WATCH_DOG_TIME, &NewTimerCB_Radar, (void *)pAd);
#else
	if(pRadarDetect->DFSWatchDogIsRunning==FALSE)
	{
		UINT32 Value;
		/* Hardware Period Timer interrupt setting.*/
		RTMP_IO_READ32(pAd, INT_TIMER_CFG, &Value);
		Value &= 0x0000ffff;
		Value |= 1 << 20; /* Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.*/
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, Value);
		/* Enable Hardware Period Timer interrupt*/
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
		Value |=0x2;
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
		pRadarDetect->DFSWatchDogIsRunning = TRUE;
	}
#endif /* RTMP_RBUS_SUPPORT */
#endif /* DFS_HWTIMER_SUPPORT */

	pRadarDetect->bDfsInit = TRUE;
	DBGPRINT(RT_DEBUG_TRACE,("Poll Time=%d\n", pRadarDetect->PollTime));
}

VOID NewRadarDetectionStop(
	IN PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("NewRadarDetectionStop2\n"));
	/* Disable detection*/
	RTMP_DFS_IO_WRITE8(pAd, 0x1, 0);
#ifdef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
	unregister_tmr_service();
#else
	{
		UINT32 Value;
		PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
		
		/* Hardware Period Timer interrupt setting.*/
		RTMP_IO_READ32(pAd, INT_TIMER_CFG, &Value);
		Value &= 0x0000ffff;
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, Value);
		/* Enable Hardware Period Timer interrupt*/
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
		Value &=~(0x2);
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	pRadarDetect->DFSWatchDogIsRunning=FALSE;
	}
#endif /* RTMP_RBUS_SUPPORT */
#endif /* DFS_HWTIMER_SUPPORT */
}


/* the debug port have timestamp 22 digit, the max number is 0x3fffff, each unit is 25ns for 40Mhz mode and 50ns for 20Mhz mode*/
/* so a round of timestamp is about 25 * 0x3fffff / 1000 = 104857us (about 100ms) or*/
/* 50 * 0x3fffff / 1000 = 209715us (about 200ms) in 20Mhz mode*/
/* 3ms = 3000,000 ns / 25ns = 120000 -- a unit */
/* 0x3fffff/120000 = 34.9 ~= 35*/
/* CE Staggered radar check*/
/* At beginning, the goal is to detect staggered radar, now, we also detect regular radar with this function.*/


int SWRadarCheck(
	IN PRTMP_ADAPTER pAd, USHORT id)
{
	int i, j, k, start_idx, end_idx;
	pNewDFSDebugPort pCurrent, p1, pEnd;
	ULONG period;
	int radar_detected = 0;
	USHORT	widthsum;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;	
	/*ENTRY_PLUS could be replace by (pDfsSwParam->sw_idx[id]+1)%128*/
	USHORT	Total, SwIdxPlus = ENTRY_PLUS(pDfsSwParam->sw_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	UCHAR	CounterToCheck;	
	
	if (SwIdxPlus == pDfsSwParam->hw_idx[id])
		return 0; /* no entry to process*/
	
	/* process how many entries?? total NEW_DFS_DBG_PORT_ENT_NUM*/
	if (pDfsSwParam->hw_idx[id] > SwIdxPlus)
		Total = pDfsSwParam->hw_idx[id] - SwIdxPlus;
	else
		Total = pDfsSwParam->hw_idx[id] + NEW_DFS_DBG_PORT_ENT_NUM - SwIdxPlus;
	
	if (Total > NEW_DFS_DBG_PORT_ENT_NUM)
		pDfsSwParam->pr_idx[id] = ENTRY_PLUS(pDfsSwParam->sw_idx[id], MAX_PROCESS_ENTRY, NEW_DFS_DBG_PORT_ENT_NUM);
	else
		pDfsSwParam->pr_idx[id] = ENTRY_PLUS(pDfsSwParam->sw_idx[id], Total, NEW_DFS_DBG_PORT_ENT_NUM);
	
	
	start_idx = ENTRY_PLUS(pDfsSwParam->pr_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	end_idx = pDfsSwParam->pr_idx[id];
	
	pEnd = &pDfsSwParam->DFS_W[id][end_idx];
	/*printk("start_idx = %d, end_idx=%d, counter=%d\n", start_idx, end_idx, pEnd->counter);*/
	
	/*if (pDfsSwParam->dfs_w_counter != pEnd->counter)*/
	/*	return 0;*/
	
	if (start_idx > end_idx)
		end_idx += NEW_DFS_DBG_PORT_ENT_NUM;
	
	
	pDfsSwParam->sw_idx[id] = pDfsSwParam->pr_idx[id];
	
	/* FCC && Japan*/

	if (pAd->CommonCfg.RDDurRegion != CE)
	{
		ULONG minPeriod = (3000 << 1);
		/* Calculate how many counters to check*/
		/* if pRadarDetect->PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz*/
		/* if pRadarDetect->PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz*/
		/* if pRadarDetect->PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz*/
		/* if pRadarDetect->PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz*/
		/* the max period to check for 40Mhz for FCC is 28650 * 2*/
		/* the max period to check for 40Mhz for Japan is 80000 * 2*/
		/* 0x40000 = 4194304 / 57129 = 73.xxx*/
		/* 0x40000 = 4194304 / 160000 = 26.2144*/
		/* 53/73 < 1 (1+1)*/
		/* 53/26.2144 = 2.02... (2+1)*/
		/* 27/26.2144 = 1.02... (1+1)*/
		/* 20M should use the same value as 40Mhz mode*/


		if (pRadarDetect->MCURadarRegion == NEW_DFS_JAP_W53)
		{
			minPeriod = 28500 << 1;
		}
		
		
		if (pAd->CommonCfg.RDDurRegion == FCC)
		{
			CounterToCheck = 1+1; 
		}
		else /* if (pAd->CommonCfg.RDDurRegion == JAP)*/
		{
			if (pRadarDetect->PollTime <= 2)
				CounterToCheck = 2+1;
			else
				CounterToCheck = 1+1;
		}
		

		
		/* First Loop for FCC/JAP*/
		for (i = end_idx; i > start_idx; i--)
		{
			pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			/* we only handle entries has same counter with the last one*/
			if (pCurrent->counter != pEnd->counter)
				break;
		
			pCurrent->start_idx = 0xffff;

			/* calculate if any two pulse become a valid period, add it in period table,*/
			for (j = i - 1; j > start_idx; j--)
			{
				p1 = &pDfsSwParam->DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];
				
				/* check period, must within max period*/
				if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
				{
					if (p1->counter + CounterToCheck < pCurrent->counter)
						break;
            	
					widthsum = p1->width + pCurrent->width;
					if (id == 0)
					{
						if (widthsum < 600)
							pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_L;
						else
							pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;
					
					if ( (pAd->CommonCfg.RDMode == RD_SILENCE_MODE) ||
						 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
						
						if ((period >= (minPeriod - 2)) && (period <= pDfsSwParam->dfs_max_period))
						{
            	
							/* add in period table*/
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
            	
            	
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
							pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
							
							pDfsSwParam->dfs_t_idx[id]++;
							if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pDfsSwParam->dfs_t_idx[id] = 0;
						}
						else if (period > pDfsSwParam->dfs_max_period)
							break;
					}
					
				}
				else
				{
					if (p1->counter + CounterToCheck < pCurrent->counter)
						break;
					
					widthsum = p1->width + pCurrent->width;
					if (id == 0)
					{
						if (widthsum < 600)
							pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_L;
						else
							pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff_ch2_Shift;

            	
					if ( (pAd->CommonCfg.RDMode == RD_SILENCE_MODE) || 
						 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )
            	
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
            	
						if ((period >= ((minPeriod >> 1) - 2)) && (period <= (pDfsSwParam->dfs_max_period >> 1)))
						{
							/* add in period table*/
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
							pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
							
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
							pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
							
							pDfsSwParam->dfs_t_idx[id]++;
							if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pDfsSwParam->dfs_t_idx[id] = 0;
						}
						else if (period > (pDfsSwParam->dfs_max_period >> 1))
							break;
					}
				}

			} /* for (j = i - 1; j > start_idx; j--)*/

		} /* for (i = end_idx; i > start_idx; i--)*/


		/* Second Loop for FCC/JAP*/
		for (i = end_idx; i > start_idx; i--)
		{
			
			pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			/* we only handle entries has same counter with the last one*/
			if (pCurrent->counter != pEnd->counter)
				break;
			if (pCurrent->start_idx != 0xffff)
			{
				/*pNewDFSDebugPort	p2, p3, p4, p5, p6;*/
				pNewDFSDebugPort	p2, p3;
				pNewDFSMPeriod pCE_T;
				ULONG idx[10], T[10];

				for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
				{

					pCE_T = &pDfsSwParam->DFS_T[id][idx[0]];
				
					p2 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];
				
					if (p2->start_idx == 0xffff)
						continue;
				
					T[0] = pCE_T->period;


					for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
					{
						
						pCE_T = &pDfsSwParam->DFS_T[id][idx[1]];
					
						p3 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];

						if (idx[0] == idx[1])
							continue;
						
						if (p3->start_idx == 0xffff)
							continue;
					


						T[1] = pCE_T->period;
						
						
						if ( PERIOD_MATCH(T[0], T[1], pDfsSwParam->dfs_period_err))
						{
							if (id <= 2) /* && (id >= 0)*/
							{

								/*if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > minPeriod)) ||*/
								/*	((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > (minPeriod >> 1))) )*/
								{
									unsigned int loop, PeriodMatched = 0, idx1;
									for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
									{
										idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
										if (PERIOD_MATCH(pDfsSwParam->DFS_T[id][idx1].period, T[1], pDfsSwParam->dfs_period_err))
										{
#ifdef DFS_DEBUG
											if (PeriodMatched < 5)
											{
												pDfsSwParam->CounterStored[PeriodMatched] = pDfsSwParam->DFS_W[id][pDfsSwParam->DFS_T[id][idx1].idx].counter;
												pDfsSwParam->CounterStored2[PeriodMatched] = loop;
												pDfsSwParam->CounterStored3 = idx[1];
											}
#endif
											/*printk("%d %d\n", loop, pDfsSwParam->DFS_T[id][idx[1]-loop].period);*/
											PeriodMatched++;
										}
										
									}
								
								
									if (PeriodMatched > pDfsSwParam->dfs_declare_thres)
									{
#ifdef DFS_DEBUG
										if (PeriodMatched == 3)
										{
											pDfsSwParam->T_Matched_3++;
											/*printk("counter=%d %d %d\n", pDfsSwParam->CounterStored[0], pDfsSwParam->CounterStored[1], pDfsSwParam->CounterStored[2]);*/
											/*printk("idx[1]=%d, loop =%d %d %d\n", pDfsSwParam->CounterStored3, pDfsSwParam->CounterStored2[0], pDfsSwParam->CounterStored2[1], pDfsSwParam->CounterStored2[2]);*/
										}
										else if (PeriodMatched == 4)
										{
											pDfsSwParam->T_Matched_4++;
											/*printk("counter=%d %d %d %d\n", pDfsSwParam->CounterStored[0], pDfsSwParam->CounterStored[1], pDfsSwParam->CounterStored[2], pDfsSwParam->CounterStored[3]);*/
											/*printk("idx[1]=%d, loop =%d %d %d %d\n", pDfsSwParam->CounterStored3, pDfsSwParam->CounterStored2[0], pDfsSwParam->CounterStored2[1], pDfsSwParam->CounterStored2[2], pDfsSwParam->CounterStored2[3]);*/
										}
										else
										{
											pDfsSwParam->T_Matched_5++;
											/*printk("counter=%d %d %d %d %d\n", pDfsSwParam->CounterStored[0], pDfsSwParam->CounterStored[1], pDfsSwParam->CounterStored[2], pDfsSwParam->CounterStored[3], pDfsSwParam->CounterStored[4]);*/
											/*printk("idx[1]=%d, loop =%d %d %d %d %d\n", pDfsSwParam->CounterStored3, pDfsSwParam->CounterStored2[0], pDfsSwParam->CounterStored2[1], pDfsSwParam->CounterStored2[2], pDfsSwParam->CounterStored2[3], pDfsSwParam->CounterStored2[4]);*/
										}
                                    	
										pDfsSwParam->DebugPortPrint = 1;
									
#endif

										{
											pNewDFSValidRadar pDFSValidRadar;
											ULONG T1 = (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)? (T[1]>>1) : T[1];
											
											pDFSValidRadar = &NewDFSValidTable[0];
											
                    					
											while (pDFSValidRadar->type != NEW_DFS_END)
											{
												if ((pDFSValidRadar->type & pRadarDetect->MCURadarRegion) == 0)
												{
													pDFSValidRadar++;
													continue;
												}
												
												if (pDFSValidRadar->TLow)
												{
													if ( (T1 > (pDFSValidRadar->TLow - pDFSValidRadar->TMargin)) && 
													     (T1 < (pDFSValidRadar->THigh + pDFSValidRadar->TMargin)) )
													{
														radar_detected = 1;
													}
												}
												else
												{
													k = 0;
													while( (k < MAX_VALID_RADAR_T) && (pDFSValidRadar->T[k] != 0) )
													{
														if ( (T1 > (pDFSValidRadar->T[k] - pDFSValidRadar->TMargin)) &&
														     (T1 < (pDFSValidRadar->T[k] + pDFSValidRadar->TMargin)) )
														{
															radar_detected = 1;
															break;
														}
														
														k++;
													}
												}												
												
												pDFSValidRadar++;
											}
											if (radar_detected == 1)
											{
												printk("W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T1, (unsigned int)id, PeriodMatched);
												printk("Radar Detected\n");
												return radar_detected;
											}
											else if (pRadarDetect->MCURadarRegion != NEW_DFS_JAP_W53)
												printk("W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T1, (unsigned int)id, PeriodMatched);

										}

										
									}
#ifdef DFS_DEBUG
									else if (PeriodMatched == 2)
									{
										pDfsSwParam->T_Matched_2++;
									}
#endif
								
								
								}
								
							} /* if (id <= 2)  && (id >= 0)*/
							
						}
						
					} /* for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)*/


					/* increase FCC-1 detection*/
					if (id <= 2)
					{
						if (IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
								int loop, idx1, PeriodMatched_fcc1 = 0;
								for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
								{
									idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
									if ( IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pDfsSwParam->DFS_T[id][idx1].period) )
									{
										/*printk("%d %d %d\n", PeriodMatched_fcc1, pDfsSwParam->DFS_T[id][idx1].period, loop);*/
										PeriodMatched_fcc1++;
									}
								}
									
								if (PeriodMatched_fcc1 > 3)
								{
									printk("PeriodMatched_fcc1 = %d (%d)\n", PeriodMatched_fcc1, id);
									radar_detected = 1;
									return radar_detected;
								}
								
						}
						
					}


					/* increase W56-3 detection*/
					if ((pRadarDetect->MCURadarRegion == NEW_DFS_JAP) && (id <= 2))
					{
						if (IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
								int loop, idx1, PeriodMatched_w56_3 = 0;
								for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
								{
									idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
									if ( IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pDfsSwParam->DFS_T[id][idx1].period) )
									{
										/*printk("%d %d %d\n", PeriodMatched_w56_3, pDfsSwParam->DFS_T[id][idx1].period, loop);*/
										PeriodMatched_w56_3++;
									}
								}
									
								if (PeriodMatched_w56_3 > 3)
								{
									printk("PeriodMatched_w56_3 = %d (%d)\n", PeriodMatched_w56_3, id);
									radar_detected = 1;
									return radar_detected;
								}
								
						}
						
					}


					if ((pRadarDetect->MCURadarRegion == NEW_DFS_JAP_W53) && (id <= 2) && IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
					{
						int loop, idx1, PeriodMatched_W56_2 = 0;
						
						for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
						{
							idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
							if ( IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pDfsSwParam->DFS_T[id][idx1].period) )
							{
								/*printk("%d %d %d\n", PeriodMatched_W56_2, pDfsSwParam->DFS_T[id][idx1].period, loop);*/
								PeriodMatched_W56_2++;
							}
						}
						
						if (PeriodMatched_W56_2 >= 3)
						{
							printk("PeriodMatched_W56_2 = %d(%d)\n", PeriodMatched_W56_2, id);
							radar_detected = 1;
							return radar_detected;
						}
					}



				} /* for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)*/
			} /* if (pCurrent->start_idx != 0xffff)*/
		} /* for (i = end_idx; i > start_idx; i--)*/
		
		return radar_detected;
	}

	/* CE have staggered radar	*/
	
	/* Calculate how many counters to check*/
	/* if pRadarDetect->PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz*/
	/* if pRadarDetect->PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz*/
	/* if pRadarDetect->PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz*/
	/* if pRadarDetect->PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz*/
	/* if pRadarDetect->PollTime is 8ms, a round of timestamp is ?? for 20Mhz, 12 for 40Mhz*/
	/* the max period to check for 40Mhz is 133333 + 125000 + 117647 = 375980*/
	/* 0x40000 = 4194304 / 375980 = 11.1556*/
	/* 53/11.1556 = 4.75...*/
	/* 35/11.1556 = 3.1374, (4+1) is safe, (3+1) to save CPU power, but may lost some data*/
	/* 27/11.1556 = 2.42, (3+1) is OK*/
	/* 21/11.1556 = 1.88, (2+1) is OK*/
	/* 20M should use the same value as 40Mhz mode*/
	if (pRadarDetect->PollTime == 1)
		CounterToCheck = 5+1;
	else if (pRadarDetect->PollTime == 2)
		CounterToCheck = 4+1;
	else if (pRadarDetect->PollTime == 3)
		CounterToCheck = 3+1;
	else if (pRadarDetect->PollTime <= 8)
		CounterToCheck = 2+1;
	else
		CounterToCheck = 1+1;

	/* First Loop for CE*/
	for (i = end_idx; i > start_idx; i--)
	{
		pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		/* we only handle entries has same counter with the last one*/
		if (pCurrent->counter != pEnd->counter)
			break;
		
		pCurrent->start_idx = 0xffff;

		/* calculate if any two pulse become a valid period, add it in period table,*/
		for (j = i - 1; j > start_idx; j--)
		{
			p1 = &pDfsSwParam->DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];
			

			/* check period, must within 16666 ~ 66666*/
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
			{
				if (p1->counter + CounterToCheck < pCurrent->counter)
						break;

				widthsum = p1->width + pCurrent->width;
				pDfsSwParam->dfs_width_diff = widthsum >> pDfsSwParam->dfs_width_diff; 
				
				if ( (pAd->CommonCfg.RDMode == RD_SILENCE_MODE) ||
					 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )
				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;
					
					/*if ((period >= (33333 - 20)) && (period <= (133333 + 20)))*/
					if ((period >= (10000 - 2)) && (period <= pDfsSwParam->dfs_max_period))
					{

						/* add in period table*/
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
        
        
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
						pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
						
						pDfsSwParam->dfs_t_idx[id]++;
						if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pDfsSwParam->dfs_t_idx[id] = 0;
					}
					else if (period > pDfsSwParam->dfs_max_period) /* to allow miss a pulse*/
						break;
				}
				
			}
			else
			{
				if (p1->counter + CounterToCheck < pCurrent->counter)
					break;
				
				widthsum = p1->width + pCurrent->width;
				if (id == 0)
				{
					if (((p1->width > 300) && (pCurrent->width < 300)) || ((pCurrent->width > 300) && ((p1->width < 300))) )
						continue;
					if (widthsum < 620)
						pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_H;
					else
						pDfsSwParam->dfs_width_diff = pDfsSwParam->dfs_width_ch0_err_L;
				}
				else if (id == 1)
					pDfsSwParam->dfs_width_diff = widthsum >> 4;
				else if (id == 2)
					pDfsSwParam->dfs_width_diff = widthsum >> 6;

				if ( (pAd->CommonCfg.RDMode == RD_SILENCE_MODE) || 
					 (PERIOD_MATCH(p1->width, pCurrent->width, pDfsSwParam->dfs_width_diff)) )

				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;

					if ((period >= (5000 - 2)) && (period <= (pDfsSwParam->dfs_max_period >> 1)))
					{
						/* add in period table*/
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width = pCurrent->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].width2 = p1->width;
						pDfsSwParam->DFS_T[id][pDfsSwParam->dfs_t_idx[id]].period = period;
						
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pDfsSwParam->dfs_t_idx[id];
						pCurrent->end_idx = pDfsSwParam->dfs_t_idx[id];
						
						pDfsSwParam->dfs_t_idx[id]++;
						if (pDfsSwParam->dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pDfsSwParam->dfs_t_idx[id] = 0;
					}
					else if (period > (pDfsSwParam->dfs_max_period >> 1))
						break;
				}
			}
			
		} /* for (j = i - 1; j > start_idx; j--)*/
	}

	/* Second Loop for CE*/
	for (i = end_idx; i > start_idx; i--)
	{
		pCurrent = &pDfsSwParam->DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		/* we only handle entries has same counter with the last one*/
		if (pCurrent->counter != pEnd->counter)
			break;
		
		/* Check Staggered radar*/
		if (pCurrent->start_idx != 0xffff)
		{
			pNewDFSDebugPort	p2, p3;
			pNewDFSMPeriod pCE_T;
			ULONG idx[10], T[10];
			
			/*printk("pCurrent=%d, idx=%d~%d\n", pCurrent->timestamp, pCurrent->start_idx, pCurrent->end_idx);*/

			for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
			{
				pCE_T = &pDfsSwParam->DFS_T[id][idx[0]];
				
				p2 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];
				
				/*printk("idx[0]= %d, idx=%d p2=%d, idx=%d~%d\n", idx[0], pCE_T->idx2, p2->timestamp, p2->start_idx, p2->end_idx);*/
				
				if (p2->start_idx == 0xffff)
					continue;
				
				T[0] = pCE_T->period;


				for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
				{
					
					pCE_T = &pDfsSwParam->DFS_T[id][idx[1]];
					
					p3 = &pDfsSwParam->DFS_W[id][pCE_T->idx2];
					
					/*printk("p3=%d, idx=%d~%d\n", p3->timestamp, p3->start_idx, p3->end_idx);*/

					if (idx[0] == idx[1])
						continue;
						
					if (p3->start_idx == 0xffff)
						continue;
					


					T[1] = pCE_T->period;

		
					if (PERIOD_MATCH(T[0], T[1], pDfsSwParam->dfs_period_err))
					{
						if (id <= 2) /* && (id >= 0)*/
						{

							
							if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > 66666)) ||
								((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > 33333)) )
							{
								unsigned int loop, PeriodMatched = 0, idx1;
								
								for (loop = 1; loop < pDfsSwParam->dfs_check_loop; loop++)
								{
									idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
									if (PERIOD_MATCH(pDfsSwParam->DFS_T[id][idx1].period, T[1], pDfsSwParam->dfs_period_err))
									{
#ifdef DFS_DEBUG
										if (PeriodMatched < 5)
										{
											pDfsSwParam->CounterStored[PeriodMatched] = pDfsSwParam->DFS_W[id][pDfsSwParam->DFS_T[id][idx1].idx].counter;
											pDfsSwParam->CounterStored2[PeriodMatched] = loop;
											pDfsSwParam->CounterStored3 = idx[1];
										}
#endif
										/*printk("%d %d\n", loop, pDfsSwParam->DFS_T[id][idx[1]-loop].period);*/
										PeriodMatched++;
									}
									
								}
								
								
								if (PeriodMatched > pDfsSwParam->dfs_declare_thres)
								{
#ifdef DFS_DEBUG
									if (PeriodMatched == 3)
									{
										pDfsSwParam->T_Matched_3++;
										/*printk("counter=%d %d %d\n", pDfsSwParam->CounterStored[0], pDfsSwParam->CounterStored[1], pDfsSwParam->CounterStored[2]);*/
										/*printk("idx[1]=%d, loop =%d %d %d\n", pDfsSwParam->CounterStored3, pDfsSwParam->CounterStored2[0], pDfsSwParam->CounterStored2[1], pDfsSwParam->CounterStored2[2]);*/
									}
									else if (PeriodMatched == 4)
									{
										pDfsSwParam->T_Matched_4++;
										/*printk("counter=%d %d %d %d\n", pDfsSwParam->CounterStored[0], pDfsSwParam->CounterStored[1], pDfsSwParam->CounterStored[2], pDfsSwParam->CounterStored[3]);*/
										/*printk("idx[1]=%d, loop =%d %d %d %d\n", pDfsSwParam->CounterStored3, pDfsSwParam->CounterStored2[0], pDfsSwParam->CounterStored2[1], pDfsSwParam->CounterStored2[2], pDfsSwParam->CounterStored2[3]);*/
									}
									else
									{
										pDfsSwParam->T_Matched_5++;
										/*printk("counter=%d %d %d %d %d\n", pDfsSwParam->CounterStored[0], pDfsSwParam->CounterStored[1], pDfsSwParam->CounterStored[2], pDfsSwParam->CounterStored[3], pDfsSwParam->CounterStored[4]);*/
										/*printk("idx[1]=%d, loop =%d %d %d %d %d\n", pDfsSwParam->CounterStored3, pDfsSwParam->CounterStored2[0], pDfsSwParam->CounterStored2[1], pDfsSwParam->CounterStored2[2], pDfsSwParam->CounterStored2[3], pDfsSwParam->CounterStored2[4]);*/
									}

									pDfsSwParam->DebugPortPrint = 1;
#endif
									printk("Radar Detected(CE), W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T[1], (unsigned int)id, PeriodMatched);
									

									if (PeriodMatched > (pDfsSwParam->dfs_declare_thres + 1))
 								      		radar_detected = 1;
									return radar_detected;
								}
#ifdef DFS_DEBUG
								else if (PeriodMatched == 2)
								{
									pDfsSwParam->T_Matched_2++;
									/*printk("counter=%d %d\n", pDfsSwParam->CounterStored[0], pDfsSwParam->CounterStored[1]);*/
									/*printk("idx[1]=%d, loop =%d %d\n", pDfsSwParam->CounterStored3, pDfsSwParam->CounterStored2[0], pDfsSwParam->CounterStored2[1]);*/
								}
#endif
								
								
							}
						}
						
					}

				} /* for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)*/

			} /* for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)*/

		}
		
	} /* for (i = end_idx; i < start_idx; i--)*/
	
	
	return radar_detected;
	
}

/* 
    ==========================================================================
    Description:
		Recheck DFS radar of stager type.
	Arguments:
	    pAdapter                    Pointer to our adapter
	    dfs_channel                DFS detect channel
    Return Value:
        "TRUE" if check pass, "FALSE" otherwise.
    Note:
    ==========================================================================
 */
static BOOLEAN StagerRadarCheck(IN PRTMP_ADAPTER pAd, UINT8 dfs_channel)
{
	UINT T1=0, T2=0, T3=0, T_all=0, F1, F2, F3 = 0, Fmax = 0, freq_diff_min, freq_diff_max;
	UINT8 bbp141=0, dfs_stg2=0, dfs_typ5=0;

	DBGPRINT(RT_DEBUG_TRACE, ("--->StagerRadarCheck()\n"));
	/* select channel */
	RTMP_DFS_IO_WRITE8(pAd, 0x0, dfs_channel);
	
	/*0.	Select dfs channel 4 and read out T1/T2/T3, 50 ns unit; 32bits */	
	RTMP_DFS_IO_READ8(pAd, 0x2d, &bbp141);
	T_all += bbp141;
	RTMP_DFS_IO_READ8(pAd, 0x2e, &bbp141);	
	T_all += bbp141<<8;
	RTMP_DFS_IO_READ8(pAd, 0x2f, &bbp141);	
	T_all += bbp141<<16;
	RTMP_DFS_IO_READ8(pAd, 0x30, &bbp141);	
	T_all += bbp141<<24;

	RTMP_DFS_IO_READ8(pAd, 0x33, &bbp141);
	T1 += bbp141;
	RTMP_DFS_IO_READ8(pAd, 0x34, &bbp141);	
	T1 += bbp141<<8;
	RTMP_DFS_IO_READ8(pAd, 0x35, &bbp141);	
	T1 += bbp141<<16;
	RTMP_DFS_IO_READ8(pAd, 0x36, &bbp141);	
	T1 += bbp141<<24;

	RTMP_DFS_IO_READ8(pAd, 0x3d, &bbp141);
	T2 += bbp141;
	RTMP_DFS_IO_READ8(pAd, 0x3e, &bbp141);	
	T2 += bbp141<<8;
	RTMP_DFS_IO_READ8(pAd, 0x3f, &bbp141);	
	T2 += bbp141<<16;
	RTMP_DFS_IO_READ8(pAd, 0x40, &bbp141);	
	T2 += bbp141<<24;
	
	T3 = T_all - T1 -T2;

	if (T3 < 5)
		T3 = 0;
	
	/*1.	Check radar stagger2 or stagger3*/
	if (T3 == 0 || ((T3 > (T1 + T2) ? (T3 - T1 - T2) : (T1 + T2 - T3)) < 25))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("stg2 confirmed\n"));
		dfs_stg2 =1;
		F1 = 20000000/T1; //hz
		F2 = 20000000/T2;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("stg3 confirmed\n"));
		F1 = 20000000/T1; //hz
		F2 = 20000000/T2;
		F3 = 20000000/T3;
	}

	Fmax = (F1 > F2) ? F1:F2;
	Fmax = (Fmax > F3) ? Fmax:F3;
	
	/*2.	Check radar type 5 or type6*/
	if (Fmax>295 && Fmax<=405)
	{	
		dfs_typ5 = 1;
		DBGPRINT(RT_DEBUG_TRACE, ("type5 confirmed\n"));
		freq_diff_min = 20;
		freq_diff_max = 50;
	}
	else if (Fmax>405 && Fmax<=1205) /* tolerate more range for looser type6 */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("type6 confirmed\n"));
		freq_diff_min = 80;
		freq_diff_max = 400;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("StagerRadarCheck failed, T1=%d, T2=%d, T3=%d\n", T1, T2, T3));
		return FALSE;
	}
	
	/*3.	According to decision of stagger and type do period check */
	if (dfs_stg2 == 1)
	{
		UINT freq_diff = (F1 > F2) ? (F1 - F2) : (F2 - F1);
		if ((freq_diff >= freq_diff_min) && (freq_diff <= freq_diff_max))
			return TRUE; /* S/W check success */
		else
		{			
			DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck failed, F1=%d, F2=%d\n", F1, F2));
			DBGPRINT(RT_DEBUG_TRACE, ("stg2 fail on S/W Freq confirmed\n"));
			return FALSE; 	/* S/W check fail */
		}
	}
	else /* dfs_stg3 */
	{	
		UINT freq_diff_1 = (F1 > F2) ? (F1 - F2) : (F2 - F1);
		UINT freq_diff_2 = (F2 > F3) ? (F2 - F3) : (F3 - F2);
		UINT freq_diff_3 = (F1 > F3) ? (F1 - F3) : (F3 - F1);
		if((freq_diff_1 >= freq_diff_min) && (freq_diff_1 <= freq_diff_max) &&
			(freq_diff_2 >= freq_diff_min) && (freq_diff_2 <= freq_diff_max) &&
			(freq_diff_3 >= freq_diff_min) && (freq_diff_3 <= freq_diff_max))
			return TRUE; /* S/W check success */
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("StagerRadarCheck failed, F1=%d, F2=%d, F3=%d\n", F1, F2, F3));
			DBGPRINT(RT_DEBUG_TRACE, ("stg3 fail on S/W Freq confirmed\n"));					
			return FALSE;   /* S/W check fail */
		}
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<---StagerRadarCheck()\n"));
}


/* 
    ==========================================================================
    Description:
		Recheck DFS radar of chrp type.
	Arguments:
	    pAdapter                    Pointer to our adapter
	    dfs_channel                DFS detect channel
    Return Value:
        "TRUE" if check pass, "FALSE" otherwise.
    Note:
    ==========================================================================
 */
static BOOLEAN ChirpRadarCheck(IN PRTMP_ADAPTER pAd)
{
	UINT32 CurrentTime, delta;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	

	RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &CurrentTime);
	delta = CurrentTime - pRadarDetect->TimeStamp;
	pRadarDetect->TimeStamp = CurrentTime;
	
	/* ChirpCheck = 0 means the very first detection since start up*/
	if (pRadarDetect->ChirpCheck++ == 0)
		return FALSE;
	
	if (delta <= (12*(1<<20)))  /* 12 sec */
	{
		if (pRadarDetect->ChirpCheck >= 2)
		{
			/* Anouce the radar on any mutiple detection within 12 sec*/
			DBGPRINT(RT_DEBUG_TRACE, ("ChirpRadarCheck OK.\n"));
			return TRUE;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ChirpRadarCheck failed, discard previous detection.\n"));
		pRadarDetect->ChirpCheck = 1;		
		return FALSE;
	}
	/* default */
	return FALSE;
}

static VOID DfsCheckBusyIdle(
			IN PRTMP_ADAPTER pAd)
{
	int busy_delta, idle_delta;	
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;

	RTMP_IO_READ32(pAd, CH_IDLE_STA, &pRadarDetect->idle_time);
	RTMP_IO_READ32(pAd, CH_BUSY_STA, &pRadarDetect->busy_time);

	/*ch_busy_sta_index begining at 0.*/
	busy_delta = pRadarDetect->busy_time - pRadarDetect->ch_busy_sta[pRadarDetect->ch_busy_sta_index];
	idle_delta = pRadarDetect->idle_time - pRadarDetect->ch_idle_sta[pRadarDetect->ch_busy_sta_index];

	if (busy_delta < 0)
	{
		busy_delta = ~busy_delta;
		busy_delta = (busy_delta >> CH_BUSY_SAMPLE_POWER);
		busy_delta = ~busy_delta;
	}
	else
		busy_delta = busy_delta >> CH_BUSY_SAMPLE_POWER;

	if (idle_delta < 0)
	{
		idle_delta = ~idle_delta;
		idle_delta = idle_delta >> CH_BUSY_SAMPLE_POWER;
		idle_delta = ~idle_delta;
	}
	else
		idle_delta = idle_delta >> CH_BUSY_SAMPLE_POWER;

	pRadarDetect->ch_busy_sum += busy_delta;
	pRadarDetect->ch_idle_sum += idle_delta;
			
	/* not sure if this is necessary??*/
	if (pRadarDetect->ch_busy_sum < 0)
		pRadarDetect->ch_busy_sum = 0;
	if (pRadarDetect->ch_idle_sum < 0)
		pRadarDetect->ch_idle_sum = 0;
			
	pRadarDetect->ch_busy_sta[pRadarDetect->ch_busy_sta_index] = pRadarDetect->busy_time;
	pRadarDetect->ch_idle_sta[pRadarDetect->ch_busy_sta_index] = pRadarDetect->idle_time;
			
	pRadarDetect->ch_busy_sta_index++;
	pRadarDetect->ch_busy_sta_index &= CH_BUSY_MASK;
			
	if ((pRadarDetect->ch_idle_sum >> pRadarDetect->ch_busy_idle_ratio) < pRadarDetect->ch_busy_sum )
	{
	
		if (!(pRadarDetect->McuRadarDebug & RADAR_DEBUG_DONT_CHECK_BUSY))	
			pRadarDetect->ch_busy = 1;
	}
	else 
	{
		if (!(pRadarDetect->McuRadarDebug & RADAR_DEBUG_DONT_CHECK_RSSI))
		{
			if ((pAd->ApCfg.RssiSample.AvgRssi0) && (pAd->ApCfg.RssiSample.AvgRssi0 > pRadarDetect->DfsRssiHigh))
				pRadarDetect->ch_busy = 2;
			else if ((pAd->ApCfg.RssiSample.AvgRssi0) && (pAd->ApCfg.RssiSample.AvgRssi0 < pRadarDetect->DfsRssiLow))
				pRadarDetect->ch_busy = 3;
			else
				pRadarDetect->ch_busy = 0;
		}
	}

	if (pRadarDetect->print_ch_busy_sta)
		printk ("%d %d %d %d\n", pRadarDetect->ch_idle_sum, pRadarDetect->ch_busy_sum, pAd->ApCfg.RssiSample.AvgRssi0, pRadarDetect->ch_busy);

}

static BOOLEAN DfsChannelCheck(
			IN PRTMP_ADAPTER pAd,
			IN UINT8 DfsChannel)
{
	pNewDFSTable pDFS2Table;
	UINT8 i;
	UINT32 W, T;
	BOOLEAN radarDeclared = 0;
	UCHAR BBP_1 = 0, BBP_2 = 0, BBP_3 = 0, BBP_4 = 0;


	printk("DFS HW check channel = 0x%x\n", DfsChannel);
	/*Select the DFS table based on radar country region*/
	if (pAd->CommonCfg.RDDurRegion == FCC)
		pDFS2Table = &NewDFSTable1[0];
	else if (pAd->CommonCfg.RDDurRegion == CE)
	{
		pDFS2Table = &NewDFSTable1[1];
	}
	else /* Japan*/
	{
		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			pDFS2Table = &NewDFSTable1[3];
		}
		else
		{
		pDFS2Table = &NewDFSTable1[2];
		}
	}
	/*check which channe(0~3) is detecting radar signals*/
	for (i = 0; i < pAd->chipCap.DfsEngineNum; i++)
	{
		
		if (DfsChannel & (0x1 << i))
		{
			/* select channel*/
			RTMP_DFS_IO_WRITE8(pAd, 0x0, i);

			/* Read reports - Period */
			RTMP_DFS_IO_READ8(pAd, 0x2d, &BBP_1);
			RTMP_DFS_IO_READ8(pAd, 0x2e, &BBP_2);
			RTMP_DFS_IO_READ8(pAd, 0x2f, &BBP_3);
			RTMP_DFS_IO_READ8(pAd, 0x30, &BBP_4);
			T = BBP_1 | (BBP_2 << 8) | (BBP_3 << 16) | (BBP_4 << 24);

			/* Read reports - Width */
			RTMP_DFS_IO_READ8(pAd, 0x31, &BBP_1);
			RTMP_DFS_IO_READ8(pAd, 0x32, &BBP_2);
			W = BBP_1 | ((BBP_2 & 0xf) << 8);

			if (DfsSwCheckOnHwDetection(pAd, pDFS2Table, i, T, W) == FALSE)
				continue;
			
			DBGPRINT(RT_DEBUG_TRACE, ("T = %u, W= %u detected by ch %d\n", T, W, i));
			
			/*set this variable to 1 for announcing that we find the radar signals.*/
			radarDeclared = 1;

			if ( ((i == 3) || (i == 2)) && (pDFS2Table->entry[i].mode != 0) )
			{
				ULONG B, W2;
				
				RTMP_DFS_IO_READ8(pAd, 0x33, &BBP_1);
				RTMP_DFS_IO_READ8(pAd, 0x34, &BBP_2);
				RTMP_DFS_IO_READ8(pAd, 0x35, &BBP_3);
				RTMP_DFS_IO_READ8(pAd, 0x36, &BBP_4);
				B = BBP_1 | (BBP_2 << 8) | (BBP_3 << 16) | (BBP_4 << 24);
				DBGPRINT(RT_DEBUG_TRACE, ("Burst = %lu(0x%lx)\n", B, B));

				RTMP_DFS_IO_READ8(pAd, 0x37, &BBP_1);
				RTMP_DFS_IO_READ8(pAd, 0x38, &BBP_2);
				W2 = BBP_1 | (BBP_2 << 8);
				DBGPRINT(RT_DEBUG_TRACE, ("The second Width = %lu(0x%lx)\n", W2, W2));
			}

		}
	}
	return radarDeclared;
}

static VOID SwCheckDfsEvent(
		IN PRTMP_ADAPTER pAd)
{
	int k, count, limit = 384;
	UCHAR BBPR127=0, BBPR126=0, LastBBPR127 = 0xff;
	ULONG time = 0;
	USHORT width = 0;
	UCHAR id = 0;
	UCHAR alignment = FALSE;
	UCHAR bitMap[6] = {0,0,0,0,0,0}, c = 0;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;

	pRadarDetect->McuRadarTick = 0;

	/* disable debug mode to read debug port of channel 3*/
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R126, &BBPR126);
	/*Power up the DFS event buffer and Disable the capture.*/
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, (BBPR126 & 0xfe) | 0x2);
	count = 0;

	pDfsSwParam->dfs_w_counter++;

	for (k = 0; k < limit; k++)
	{
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);
		/*each entry for each channel has 6 bytes*/
		bitMap[c++] |= BBPR127;
		if (c >= 6)
			c = 0;
		if (alignment == FALSE)
		{
			LastBBPR127 = (c<=1)? bitMap[c+4] : bitMap[c-2];

			if ((LastBBPR127 & 0xf8) || ((BBPR127 & 0xfc) && (BBPR127 != 0xff)))
				{
				continue;
				}
			else
				count = 0;
		}
		
												

		/*
		BBPR127 record the data of R127.
		Byte 0	:DFS channel id(0~3). (0xff indicate end of data or invalid entry)
		Byte 1:time stamp[7~0]
		Byte 2:time stamp[15~8]
		Byte 3:
		bit 0- bit 4
		time stamp[19~14]
		bit 5- bit 6
		width[1-0]
		Byte 4:
		width[2-9]
		Byte 5:
		bit 0-bit 2
		width[11-10]
		bit 3-bit 7
		reserved(should be zero or means this the error information)
		bitMap should be byteMap=>6 bytes
		c and count is the same for entry index but have different usages.

		*/
		switch(count)
		{
			case 0:
				width = 0;
				time = 0;
				id = BBPR127;
				LastBBPR127 = (c<=1)? bitMap[c+4] : bitMap[c-2];

				if ((id > 3) && (id != 0xff))
				{
					alignment = FALSE;
					/*LastBBPR127 = BBPR127;*/
				}
				else
				{
					if ((LastBBPR127 & 0xf8) != 0)
					{
						/*LastBBPR127 = BBPR127;*/
						alignment = FALSE;
					}
					else
					{
						if ((LastBBPR127 != 0) && (id == 0))
						{
							/*LastBBPR127 = BBPR127;*/
							alignment = FALSE;
						}
						else
						{
							alignment = TRUE;
						}
					}
				}

				break;
			case 1:
				time |= BBPR127;
				break;
			case 2:
				time |= BBPR127 << 8;
				break;
			case 3:
				time |= (BBPR127 & 0x3f) << 16;
				width |= (BBPR127 & 0xc0) >> 6;
				break;
			case 4:
				width |= BBPR127 << 2;
				break;
			case 5:
				if ((alignment == TRUE) && ((BBPR127 & 0xf8) != 0))
				{
					/*printk("count=5 and the lefe 5 bit is not zeor of BBPR127=%d, it should be a error counting\n",BBPR127);*/
					alignment = FALSE;
					/*LastBBPR127 = BBPR127;*/
					break;
				}						

			width |= (BBPR127 & 0x7) << 10;
					
			if (pRadarDetect->use_tasklet)
			{
				/*if (id <= 0x2)  && (id >= 0)*/
				if ((id < 0x3) && (time & 0x3fff00)) /* && (id >= 0)*/
				{
					/*Loop=384=>384/3=128*/
					/*The buffer is more than twice space to store two loop values.*/
					/*[0~2][255~0]*/
					if (time != pDfsSwParam->DFS_W[id][((pDfsSwParam->dfs_w_idx[id] == 0)? (NEW_DFS_DBG_PORT_ENT_NUM-1):(pDfsSwParam->dfs_w_idx[id] - 1))].timestamp)
					{
				if (!(((time & 0xff) == 0) || ((time & 0xff00) == 0) || ((((time & 0x3f0000) == 0) && ((width & 0x3) == 0x3)) || ((width & 0x3fc) == 0x3fc) || ((width & 0x3fc) == 0))))
						{
							pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].counter = pDfsSwParam->dfs_w_counter;
							pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].timestamp = time;
							pDfsSwParam->DFS_W[id][pDfsSwParam->dfs_w_idx[id]].width = width;
					
							pDfsSwParam->dfs_w_last_idx[id] = pDfsSwParam->dfs_w_idx[id];
							pDfsSwParam->dfs_w_idx[id]++;
							if (pDfsSwParam->dfs_w_idx[id] >= NEW_DFS_DBG_PORT_ENT_NUM)
								pDfsSwParam->dfs_w_idx[id] = 0;
						}
					}
					else
					{
						id = 0xff;
						alignment = TRUE;
						break;
					}
				}
			}
#ifdef DFS_DEBUG
			pDfsSwParam->TotalEntries[id]++;						
#endif 
			break;
		}
		if ((id == 0xff) && (alignment == TRUE))
		{
			break;
		}

#ifdef DFS_DEBUG
		if (pDfsSwParam->DebugPortPrint == 1 && (k == 0))
		{
			pDfsSwParam->DebugPortPrint = 2;
		}

		if ((pDfsSwParam->DebugPortPrint == 2))
		{
			pDfsSwParam->DebugPort[k] = BBPR127;
		}
#endif 
		if (alignment == TRUE)
		{
			count++;
			if (count >= 6)
				count = 0;
		}
	}

#ifdef DFS_DEBUG			
	if (pDfsSwParam->DebugPortPrint == 2)
	{
		pDfsSwParam->DebugPortPrint = 3;
	}

	if (pDfsSwParam->BBP127Repeat)
	{
		for (k = 0; k < pDfsSwParam->BBP127Repeat; k++)
			BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);

		pDfsSwParam->BBP127Repeat = 0;
	}
	else
#endif
	/* read to several times for alignment*/
	/*Sometimes the previous loop will not finish reading a count when encountering error counting.*/
	/*The fllowing codes is used to finish the reading of a counting.*/
	for (k = count; k < 5; k++)
	{
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);
	}


	/* enable debug mode*/
	if (pDfsSwParam->dfs_w_counter & 1)
	{
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 3);
	}
	else
	{
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 7);
	}

	if (pRadarDetect->use_tasklet)
	{
		/* set hw_idx*/
		pDfsSwParam->hw_idx[0] = pDfsSwParam->dfs_w_idx[0];
		pDfsSwParam->hw_idx[1] = pDfsSwParam->dfs_w_idx[1];
		pDfsSwParam->hw_idx[2] = pDfsSwParam->dfs_w_idx[2];
		pDfsSwParam->hw_idx[3] = pDfsSwParam->dfs_w_idx[3];
		/*dfs tasklet will call SWRadarCheck*/
		schedule_dfs_task(pAd);
	}

	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, (BBPR126 | 0x3));
}

VOID NewRadarDetectionProgram(PRTMP_ADAPTER pAd, pNewDFSTable pDFS2Table)
{
	UINT8 idx, TalbeIdx, DFSR3;
	UINT8 DfsEngineNum = pAd->chipCap.DfsEngineNum;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;


	pRadarDetect->bDfsInit = FALSE;

	/* Get Table index*/
	for (TalbeIdx = 0; !((1<<TalbeIdx) & pDFS2Table->type); TalbeIdx++)
	{
		if (TalbeIdx > MAX_RD_REGION)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Table index out of range.\n"));
			return;
		}
	}

	for(idx = 0; idx<DfsEngineNum; idx++)
	{
		if ((pRadarDetect->DFSParamFromConfig & (0x1<<idx)) && pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].valid)
		{
			pDFS2Table->entry[idx].mode = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].mode;
			pDFS2Table->entry[idx].avgLen = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].avgLen;
			pDFS2Table->entry[idx].ELow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].ELow;
			pDFS2Table->entry[idx].EHigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].EHigh;
			pDFS2Table->entry[idx].WLow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].WLow;
			pDFS2Table->entry[idx].WHigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].WHigh;
			pDFS2Table->entry[idx].EpsilonW = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].EpsilonW;
			pDFS2Table->entry[idx].TLow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].TLow;
			pDFS2Table->entry[idx].THigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].THigh;
			pDFS2Table->entry[idx].EpsilonT = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].EpsilonT;
			pDFS2Table->entry[idx].BLow = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].BLow;
			pDFS2Table->entry[idx].BHigh = pDfsProgramParam->NewDFSTableEntry[(TalbeIdx*DfsEngineNum)+idx].BHigh;

			DBGPRINT(RT_DEBUG_TRACE, ("TalbeIdx = %d; idx = %d; DFSParam = %2d; %3d; %3d; %3d; %3d; %4d; %3d; %6lu; %7lu; %4d; %2lu; %2lu\n", TalbeIdx, idx,
					pDFS2Table->entry[idx].mode, pDFS2Table->entry[idx].avgLen, pDFS2Table->entry[idx].ELow, 
					pDFS2Table->entry[idx].EHigh, pDFS2Table->entry[idx].WLow, pDFS2Table->entry[idx].WHigh,
					pDFS2Table->entry[idx].EpsilonW, pDFS2Table->entry[idx].TLow, pDFS2Table->entry[idx].THigh,
					pDFS2Table->entry[idx].EpsilonT, pDFS2Table->entry[idx].BLow, pDFS2Table->entry[idx].BHigh));
		}
	}
	
	/* Symmetric round*/
        if(pRadarDetect->SymRoundFromCfg != 0)
        {
                pDfsProgramParam->Symmetric_Round = pRadarDetect->SymRoundFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("Symmetric_Round = %d\n", pDfsProgramParam->Symmetric_Round));
        }

        /* BusyIdleRatio*/
        if(pRadarDetect->BusyIdleFromCfg != 0)
        {
                pRadarDetect->ch_busy_idle_ratio = pRadarDetect->BusyIdleFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("ch_busy_idle_ratio = %d\n", pRadarDetect->ch_busy_idle_ratio));
        }
        /* DfsRssiHigh*/
        if(pRadarDetect->DfsRssiHighFromCfg != 0)
        {
                pRadarDetect->DfsRssiHigh = pRadarDetect->DfsRssiHighFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("DfsRssiHigh = %d\n", pRadarDetect->DfsRssiHigh));
        }
        /* DfsRssiLow*/
        if(pRadarDetect->DfsRssiLowFromCfg != 0)
        {
                pRadarDetect->DfsRssiLow = pRadarDetect->DfsRssiLowFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("DfsRssiLow = %d\n", pRadarDetect->DfsRssiLow));
        }
	
	/*pRadarDetect->MCURadarRegion = pAd->CommonCfg.RDDurRegion;*/
	pRadarDetect->MCURadarRegion = pDFS2Table->type;
	
	DFSR3 = pDfsProgramParam->Symmetric_Round << 4;
	/* Full 40Mhz*/
	if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
	{
		/* BW 40*/
		DFSR3 |= 0x80; 
	}
		/* Delta Delay*/
		DFSR3 |= (pDfsProgramParam->DeltaDelay & 0xf);
		RTMP_DFS_IO_WRITE8(pAd, 0x3, DFSR3);
		DBGPRINT(RT_DEBUG_TRACE,("R3 = 0x%x\n", DFSR3));
		
		/* VGA Mask*/
		RTMP_DFS_IO_WRITE8(pAd, 0x4, pDfsProgramParam->VGA_Mask);
		DBGPRINT(RT_DEBUG_TRACE,("VGA_Mask = 0x%x\n", pDfsProgramParam->VGA_Mask));
		
		/* packet end Mask*/
		RTMP_DFS_IO_WRITE8(pAd, 0x5, pDfsProgramParam->Packet_End_Mask);
		DBGPRINT(RT_DEBUG_TRACE,("Packet_End_Mask = 0x%x\n", pDfsProgramParam->Packet_End_Mask));
		
		/* Rx PE Mask*/
		RTMP_DFS_IO_WRITE8(pAd, 0x6, pDfsProgramParam->Rx_PE_Mask);
		DBGPRINT(RT_DEBUG_TRACE,("Rx_PE_Mask = 0x%x\n", pDfsProgramParam->Rx_PE_Mask));
	
		/* program each channel*/
		for (idx = 0; idx < DfsEngineNum; idx++)
		{
			/* select channel*/
			RTMP_DFS_IO_WRITE8(pAd, 0x0, idx);
	
			printk("write DFS Channle[%d] configuration \n",idx);
			/* start programing*/
	
			/* reg 0x10, Detection Mode[2:0]*/
			RTMP_DFS_IO_WRITE8(pAd, 0x10, (pDFS2Table->entry[idx].mode & 0xf));
			
			/* reg 0x11~0x12, M[7:0] & M[8]*/
			if (idx==4 || idx==5)
			{
				RTMP_DFS_IO_WRITE8(pAd, 0x12, ((pDFS2Table->entry[idx].avgLen << 4) & 0x30)); //to do: add comment
			}
			else
			{
				RTMP_DFS_IO_WRITE8(pAd, 0x11, (pDFS2Table->entry[idx].avgLen & 0xff));
				RTMP_DFS_IO_WRITE8(pAd, 0x12, ((pDFS2Table->entry[idx].avgLen >> 8) & 0x1));
			}
	
			/* reg 0x13~0x14, Energy Low[7:0] & Energy Low[11:8]*/
			RTMP_DFS_IO_WRITE8(pAd, 0x13, (pDFS2Table->entry[idx].ELow & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x14, ((pDFS2Table->entry[idx].ELow >> 8) & 0xf));
	
			
			/* reg 0x15~0x16, Energy High[7:0] & Energy High[11:8]*/
			RTMP_DFS_IO_WRITE8(pAd, 0x15, (pDFS2Table->entry[idx].EHigh & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x16, ((pDFS2Table->entry[idx].EHigh >> 8) & 0xf));
	
			
			/* reg 0x28~0x29, Width Low[7:0] & Width Low[11:8]*/
			RTMP_DFS_IO_WRITE8(pAd, 0x28, (pDFS2Table->entry[idx].WLow & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x29, ((pDFS2Table->entry[idx].WLow >> 8) & 0xf));
	
			/* reg 0x2a~0x2b, Width High[7:0] & Width High[11:8]*/
			RTMP_DFS_IO_WRITE8(pAd, 0x2a, (pDFS2Table->entry[idx].WHigh & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x2b, ((pDFS2Table->entry[idx].WHigh >> 8) & 0xf));
	
			/* reg 0x2c, Width Delta[7:0], (Width Measurement Uncertainty)*/
			RTMP_DFS_IO_WRITE8(pAd, 0x2c, (pDFS2Table->entry[idx].EpsilonW & 0xff));
	
			/* reg 0x17~0x1a, Period Low[7:0] & Period Low[15:8] & Period Low[23:16] & Period Low[31:24]*/
			RTMP_DFS_IO_WRITE8(pAd, 0x17, (pDFS2Table->entry[idx].TLow & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x18, ((pDFS2Table->entry[idx].TLow >> 8) & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x19, ((pDFS2Table->entry[idx].TLow >> 16) & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x1a, ((pDFS2Table->entry[idx].TLow >> 24) & 0xff));
	
			/* reg 0x1b~0x1e, Period High[7:0] & Period High[15:8] & Period High[23:16] & Period High[31:24]*/
			RTMP_DFS_IO_WRITE8(pAd, 0x1b, (pDFS2Table->entry[idx].THigh & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x1c, ((pDFS2Table->entry[idx].THigh >> 8) & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x1d, ((pDFS2Table->entry[idx].THigh >> 16) & 0xff));
			RTMP_DFS_IO_WRITE8(pAd, 0x1e, ((pDFS2Table->entry[idx].THigh >> 24) & 0xff));
	
			/* reg 0x27, Period Delt[7:0], (Period Measurement Uncertainty)*/
			RTMP_DFS_IO_WRITE8(pAd, 0x27, (pDFS2Table->entry[idx].EpsilonT & 0xff));
			
			if (pDfsProgramParam->RadarEventExpire[idx] != 0)
			{
				RTMP_DFS_IO_WRITE8(pAd,0x39, (pDfsProgramParam->RadarEventExpire[idx] & 0xff));
				RTMP_DFS_IO_WRITE8(pAd,0x3a, ((pDfsProgramParam->RadarEventExpire[idx] >> 8) & 0xff) );
				RTMP_DFS_IO_WRITE8(pAd,0x3b, ((pDfsProgramParam->RadarEventExpire[idx] >> 16) & 0xff));
				RTMP_DFS_IO_WRITE8(pAd,0x3c, ((pDfsProgramParam->RadarEventExpire[idx] >> 24) & 0xff));
			}
			
		}	
		pRadarDetect->bDfsInit = TRUE;

}

BOOLEAN DfsSwCheckOnHwDetection(
	 IN PRTMP_ADAPTER pAd,
	 IN pNewDFSTable pDFS2Table,
	 IN UINT8 DfsChannel,
	 IN ULONG RadarPeriod,
	 IN ULONG RadarWidth)
{
	BOOLEAN bRadarCheck = TRUE;
	if (!RadarPeriod || !RadarWidth)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Block eception on zero RadarPeriod or RadarWidth\n"));
		return FALSE;
	}

	if (pDFS2Table->type == NEW_DFS_JAP)
	{
		/* Double check on pusle Width and Period*/
		if (DfsChannel < 3)
		{
			/*check short pulse*/
			if (RadarWidth < 375) 
			{
				/* block the illegal period */
				if ((RadarPeriod < 2800) ||
					(RadarPeriod > 5000 && RadarPeriod < 6400) ||
					(RadarPeriod > 6800 && RadarPeriod < 27560)||
					(RadarPeriod > 27960 && RadarPeriod < 28360) ||
					(RadarPeriod > 28700 && RadarPeriod < 79900) ||
					(RadarPeriod > 80100))
				{ 
					 /*(0~140), (250~320us), (340~1378us), (1398~1418), (1435~3995us) and (4005us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 bRadarCheck = FALSE;
				}
			}
			else if (RadarWidth > 375)
			{
				if ((RadarPeriod<3500) || (RadarPeriod>10400))
				{ 
					 /* block the illegal period */
					 /*(0~175) and (520us~) according to the spec*/
					 DBGPRINT(RT_DEBUG_TRACE,
							 ("Radar check: ch=%u, T=%lu, W=%lu, blocked\n", DfsChannel, RadarPeriod, RadarWidth));
					 bRadarCheck = FALSE;
				}
			}
		}
		else if (DfsChannel == 3)
		{
			bRadarCheck = ChirpRadarCheck(pAd);
		}
	}
	else if (pDFS2Table->type == NEW_DFS_EU)
	{
		if (DfsChannel == 4) /* to do: check dfs mode 8or9*/
		{
			if (StagerRadarCheck(pAd, DfsChannel) == FALSE)
				bRadarCheck = FALSE;
		}
	}
	return bRadarCheck;
}

static VOID ChannelSelectOnRadarDetection(
		IN PRTMP_ADAPTER pAd)
{
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;	
	UINT i;

	NewRadarDetectionStop(pAd);
	
	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
		{
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40)
			{
				if ((pAd->ChannelList[i].Channel >> 2) & 1)
				{
					if ((pAd->ChannelList[i+1].Channel - pAd->ChannelList[i].Channel) == 4 )
					{
						DBGPRINT(RT_DEBUG_TRACE, ("Find extend channel = %u\n", pAd->ChannelList[i+1].Channel));
						pAd->ChannelList[i+1].RemainingTimeForUse = 1800;
					}
				}
				else 
				{
					if ((pAd->ChannelList[i].Channel - pAd->ChannelList[i-1].Channel) == 4 )
					{
						DBGPRINT(RT_DEBUG_TRACE, ("Find extend channel = %u\n", pAd->ChannelList[i-1].Channel));
						pAd->ChannelList[i-1].RemainingTimeForUse = 1800;
					}
				}
			}
			else
				DBGPRINT(RT_DEBUG_TRACE, ("BW is not 40.\n"));
			
			pAd->ChannelList[i].RemainingTimeForUse = 1800;/*30 min = 1800 sec*/
			break;
		}
	}

	/*when find an radar, the ChMovingTime will be set to announce how many seconds to sending software radar detection time.*/
	if ((pAd->CommonCfg.RDDurRegion == CE) && (pAd->CommonCfg.Channel >= 116) && (pAd->CommonCfg.Channel <= 128))
		pAd->CommonCfg.ChMovingTime = 605;
	else
		pAd->CommonCfg.ChMovingTime = 65;

	/*if the Radar country region is JAP, we need find a new clear channel */
	if (pAd->CommonCfg.RDDurRegion == JAP_W56)
	{
		for (i = 0; i < pAd->ChannelListNum ; i++)
		{
			pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
			if ((pAd->CommonCfg.Channel >= 100) && (pAd->CommonCfg.Channel <= 140))
				break;
		}
	}
	else if (pAd->CommonCfg.RDDurRegion == JAP_W53)
	{
		for (i = 0; i < pAd->ChannelListNum ; i++)
		{
			pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
			if ((pAd->CommonCfg.Channel >= 36) && (pAd->CommonCfg.Channel <= 60))
				break;
		}
	}
	else
		pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
		
#ifdef DOT11_N_SUPPORT
	N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */
	/*ApSelectChannelCheck(pAd);*/
	if (pAd->CommonCfg.RDMode != RD_SILENCE_MODE)
	{
		pAd->CommonCfg.RDMode = RD_SWITCHING_MODE;
		pAd->CommonCfg.CSCount = 0;
	}
	else
	{
		/*set this flag to 1 and the AP will restart to switch into new channel */
		pAd->CommonCfg.DFSAPRestart = 1;
		schedule_dfs_task(pAd);
	}
		pRadarDetect->radarDeclared = 0;
}


static void dfs_sw_init(PRTMP_ADAPTER pAd)
{
	int j, k;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;

	for (k = 0; k < pAd->chipCap.DfsEngineNum; k++)
	{
		for (j = 0; j < NEW_DFS_DBG_PORT_ENT_NUM; j++)
		{
			pDfsSwParam->DFS_W[k][j].start_idx = 0xffff;
		}
	}
	pRadarDetect->use_tasklet = 1;
	pDfsSwParam->dfs_check_loop = DFS_SW_RADAR_CHECK_LOOP;
	pDfsSwParam->dfs_width_diff_ch1_Shift = DFS_SW_RADAR_CH1_SHIFT;
	pDfsSwParam->dfs_width_diff_ch2_Shift = DFS_SW_RADAR_CH2_SHIFT;
	pDfsSwParam->dfs_width_diff_Shift = DFS_SW_RADAR_SHIFT;
	pDfsSwParam->dfs_width_ch0_err_L = DFS_SW_RADAR_CH0_ERR;
	if (pAd->CommonCfg.RDDurRegion == CE)
		pDfsSwParam->dfs_period_err = (DFS_SW_RADAR_PERIOD_ERR << 2);
	else
		pDfsSwParam->dfs_period_err = DFS_SW_RADAR_PERIOD_ERR;

	if (pAd->CommonCfg.RDDurRegion == CE)
	{
		pDfsSwParam->dfs_width_ch0_err_H = CE_STAGGERED_RADAR_CH0_H_ERR;
		pDfsSwParam->dfs_declare_thres = CE_STAGGERED_RADAR_DECLARE_THRES;
		pDfsSwParam->dfs_max_period = CE_STAGGERED_RADAR_PERIOD_MAX;
	}
	else
	{
		/*pDfsSwParam->dfs_declare_thres = DFS_SW_RADAR_DECLARE_THRES;*/
		if (pAd->CommonCfg.RDDurRegion == FCC)
			pDfsSwParam->dfs_max_period = FCC_RADAR_PERIOD_MAX;
		else if (pAd->CommonCfg.RDDurRegion == JAP)
			pDfsSwParam->dfs_max_period = JAP_RADAR_PERIOD_MAX;
	}

	for (k = 0; k < pAd->chipCap.DfsEngineNum; k++)
	{
		pDfsSwParam->sw_idx[k] = NEW_DFS_DBG_PORT_ENT_NUM - 1;
		pDfsSwParam->hw_idx[k] = 0;
	}
		
	if (pRadarDetect->use_tasklet)
		pRadarDetect->PollTime = NEW_DFS_CHECK_TIME_TASKLET;
	else
		pRadarDetect->PollTime = NEW_DFS_CHECK_TIME;

}

void modify_table1(PRTMP_ADAPTER pAd, ULONG idx, ULONG value)
{
	pNewDFSTable pDFS2Table;
	ULONG x, y;	
	UINT8 DfsEngineNum = pAd->chipCap.DfsEngineNum;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_PROGRAM_PARAM pDfsProgramParam = &pRadarDetect->DfsProgramParam;

	if (pAd->CommonCfg.RDDurRegion == FCC)
		pDFS2Table = &NewDFSTable1[0];
	else if (pAd->CommonCfg.RDDurRegion == CE)
	{
		pDFS2Table = &NewDFSTable1[1];
	}
	else /* Japan*/
	{
		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			pDFS2Table = &NewDFSTable1[3];
		}
		else
		{
		pDFS2Table = &NewDFSTable1[2];
		}
	}

	if (idx == 0)
	{
		pDfsProgramParam->DeltaDelay = value;
	}
	else if (idx <= (DfsEngineNum*16))
	{	
		x = idx / 16;
		y = idx % 16;
		pRadarDetect->DFSParamFromConfig = 0; /* to prevent table be loaded from config file again */
		switch (y)
		{
		case 1:
			pDFS2Table->entry[x].mode = (USHORT)value;
			break;
		case 2:
			pDFS2Table->entry[x].avgLen = (USHORT)value;
			break;
		case 3:
			pDFS2Table->entry[x].ELow = (USHORT)value;
			break;
    	
		case 4:
			pDFS2Table->entry[x].EHigh = (USHORT)value;
			break;
    	
		case 5:
			pDFS2Table->entry[x].WLow = (USHORT)value;
			break;
    	
		case 6:
			pDFS2Table->entry[x].WHigh = (USHORT)value;
			break;
    	
		case 7:
			pDFS2Table->entry[x].EpsilonW = (USHORT)value;
			break;
    	
		case 8:
			pDFS2Table->entry[x].TLow = (ULONG)value;
			break;
    	
		case 9:
			pDFS2Table->entry[x].THigh = (ULONG)value;
			break;
    	
		case 0xa:
			pDFS2Table->entry[x].EpsilonT = (USHORT)value;
			break;

		case 0xb:
			pDFS2Table->entry[x].BLow= (USHORT)value;
			break;
		case 0xc:
			pDFS2Table->entry[x].BHigh = (USHORT)value;
			break;

		default:
			break;
		}
    	
	}
	else if (idx == (DfsEngineNum*16 +1))
	{
		pDfsProgramParam->Symmetric_Round = (ULONG)value;
	}
	else if (idx == (DfsEngineNum*16 +2))
	{
		pDfsProgramParam->VGA_Mask = (ULONG)value;
	}
	else if (idx == (DfsEngineNum*16 +3))
	{
		pDfsProgramParam->Packet_End_Mask = (ULONG)value;
	}
	else if (idx == (DfsEngineNum*16 +4))
	{
		pDfsProgramParam->Rx_PE_Mask = (ULONG)value;
	}

	printk("Delta_Delay(0) = %d\n", pDfsProgramParam->DeltaDelay);

	for (x = 0; x < DfsEngineNum; x++)
	{
		printk("Channel %lu\n", x);
		printk("\t\tmode(%02lu)=%d, M(%02lu)=%03d, EL(%02lu)=%03d EH(%02lu)=%03d, WL(%02lu)=%03d WH(%02lu)=%04d, eW(%02lu)=%02d\n\t\tTL(%02lu)=%05u TH(%02lu)=%06u, eT(%02lu)=%03d, BL(%02lu)=%u, BH(%02lu)=%u\n",
		(x*16+1), (unsigned int)pDFS2Table->entry[x].mode,
		(x*16+2), (unsigned int)pDFS2Table->entry[x].avgLen,
		(x*16+3), (unsigned int)pDFS2Table->entry[x].ELow,
		(x*16+4), (unsigned int)pDFS2Table->entry[x].EHigh,
		(x*16+5), (unsigned int)pDFS2Table->entry[x].WLow,
		(x*16+6), (unsigned int)pDFS2Table->entry[x].WHigh,
		(x*16+7), (unsigned int)pDFS2Table->entry[x].EpsilonW,
		(x*16+8), (unsigned int)pDFS2Table->entry[x].TLow,
		(x*16+9), (unsigned int)pDFS2Table->entry[x].THigh,
		(x*16+0xa), (unsigned int)pDFS2Table->entry[x].EpsilonT,
		(x*16+0xb), (unsigned int)pDFS2Table->entry[x].BLow,
		(x*16+0xc), (unsigned int)pDFS2Table->entry[x].BHigh);
	}

	printk("Symmetric_Round(%02d) = %d\n", (DfsEngineNum*16 +1), pDfsProgramParam->Symmetric_Round);
	printk("VGA_Mask(%02d) = %d\n", (DfsEngineNum*16 +2), pDfsProgramParam->VGA_Mask);
	printk("Packet_End_Mask(%02d) = %d\n", (DfsEngineNum*16 +3), pDfsProgramParam->Packet_End_Mask);
	printk("Rx_PE_Mask(%02d) = %d\n", (DfsEngineNum*16 +4), pDfsProgramParam->Rx_PE_Mask);

}


void modify_table2(PRTMP_ADAPTER pAd, ULONG idx, ULONG value)
{
	pNewDFSValidRadar pDFSValidRadar;
	ULONG x, y;
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	
	idx--;

	x = idx / 17;
	y = idx % 17;
	
	pDFSValidRadar = &NewDFSValidTable[0];
	
	while (pDFSValidRadar->type != NEW_DFS_END)
	{
		if (pDFSValidRadar->type & pRadarDetect->MCURadarRegion)
		{
			if (x == 0)
				break;
			else
			{
				x--;
				pDFSValidRadar++;
			}
		}
		else
			pDFSValidRadar++;
	}
	
	if (pDFSValidRadar->type == NEW_DFS_END)
	{
		printk("idx=%d exceed max number\n", (unsigned int)idx);
		return;
	}
	switch(y)
	{
	case 0:
		pDFSValidRadar->channel = value;
		break;
	case 1:
		pDFSValidRadar->WLow = value;
		break;
	case 2:
		pDFSValidRadar->WHigh = value;
		break;
	case 3:
		pDFSValidRadar->W[0] = value;
		break;
	case 4:
		pDFSValidRadar->W[1] = value;
		break;
	case 5:
		pDFSValidRadar->W[2] = value;
		break;
	case 6:
		pDFSValidRadar->W[3] = value;
		break;
	case 7:
		pDFSValidRadar->W[4] = value;
		break;
	case 8:
		pDFSValidRadar->WMargin = value;
		break;
	case 9:
		pDFSValidRadar->TLow = value;
		break;
	case 10:
		pDFSValidRadar->THigh = value;
		break;
	case 11:
		pDFSValidRadar->T[0] = value;
		break;
	case 12:
		pDFSValidRadar->T[1] = value;
		break;
	case 13:
		pDFSValidRadar->T[2] = value;
		break;
	case 14:
		pDFSValidRadar->T[3] = value;
		break;
	case 15:
		pDFSValidRadar->T[4] = value;
		break;
	case 16:
		pDFSValidRadar->TMargin = value;
		break;
	}
	
	pDFSValidRadar = &NewDFSValidTable[0];
	while (pDFSValidRadar->type != NEW_DFS_END)
	{
		if (pDFSValidRadar->type & pRadarDetect->MCURadarRegion)
		{
			printk("ch = %x  --- ", pDFSValidRadar->channel);
			printk("wl:wh = %d:%d  ", pDFSValidRadar->WLow, pDFSValidRadar->WHigh);
			printk("w[] = %d %d %d %d %d --- ", pDFSValidRadar->W[0], pDFSValidRadar->W[1], pDFSValidRadar->W[2], pDFSValidRadar->W[3], pDFSValidRadar->W[4]);
			printk("W Margin = %d\n", pDFSValidRadar->WMargin);
			printk("        --- Tl:Th = %d:%d  ", (unsigned int)pDFSValidRadar->TLow, (unsigned int)pDFSValidRadar->THigh);
			printk("T[] = %d %d %d %d %d --- ", (unsigned int)pDFSValidRadar->T[0], (unsigned int)pDFSValidRadar->T[1], (unsigned int)pDFSValidRadar->T[2], (unsigned int)pDFSValidRadar->T[3], (unsigned int)pDFSValidRadar->T[4]);
			printk("T Margin = %d\n", pDFSValidRadar->TMargin);
		}
		pDFSValidRadar++;
	}

}

#endif /*CONFIG_AP_SUPPORT*/
#endif /* DFS_SUPPORT */

