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

typedef struct _RADAR_DURATION_TABLE
{
	ULONG RDDurRegion;
	ULONG RadarSignalDuration;
	ULONG Tolerance;
} RADAR_DURATION_TABLE, *PRADAR_DURATION_TABLE;

#ifdef CONFIG_AP_SUPPORT

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
static BOOLEAN RadarSignalDetermination(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN RadarType,
	IN UINT32 RadarSignalDuration);

static RADAR_DURATION_TABLE RadarSignalDurationTable[]=
{//Pattern num, Session interval, Theorical duration, Tolerance
	// CE.
	{CE, 0x682A, 	0x10},
	{CE, 0x186A0, 	0x100},
	{CE, 0x1046A, 	0x100},
	{CE, 0x9C40,  	0x10},
	{CE, 0x61A8,  	0x10},
	{CE, 0x4E20,  	0x10},
	{CE, 0x411A,	0x10},
	{CE, 0x3415, 	0x10},
	{CE, 0x30D4,	0x10},
	{CE, 0x21F7,	0x10},
	{CE, 0x1A0A,	0x10},
	{CE, 0x1652,	0x10},
	{CE, 0x1388,	0x10},
	{CE, 0x2710,	0x10},
	//FCC
	{FCC, 0x6F90, 	0x10},
	{FCC, ((0x11F8+0xBB8)/2), ((0x11F8-0xBB8)/2)},
	{FCC, ((0x2710+0xFA0)/2), ((0x2710-0xFA0)/2)},
	{FCC, 0x1a04,	0x10},
	//Japan
	{JAP, 0x6F9B,	0x10},
	{JAP, 0x12C7B,	0x100},
	{JAP, 0x6C81,	0x10},
	{JAP, 0x6F9B,	0x10},
	{JAP, 0x13880,	0x100},
	{JAP, ((0x11F8+0xBB7)/2), ((0x11F8-0xBB7)/2)},
	{JAP, ((0x2710+0xFA0)/2), ((0x2710-0xFA0)/2)},
	{JAP, 0x12C7B,	0x100},
	//Japan w53
	{JAP_W53, 0x6F9B,	0x10},
	{JAP_W53, 0x12C7B,	0x100},
	//Japan w56
	{JAP_W56, 0x6C81,	0x10},
	{JAP_W56, 0x6F9B,	0x10},
	{JAP_W56, 0x13880,	0x100},
	{JAP_W56, ((0x11F8+0xBB7)/2), ((0x11F8-0xBB7)/2)},
	{JAP_W56, ((0x2710+0xFA0)/2), ((0x2710-0xFA0)/2)},
	{JAP_W56, 0x12C7B,	0x100},
};
#define RD_TAB_SIZE (sizeof(RadarSignalDurationTable) / sizeof(RADAR_DURATION_TABLE))

static RADAR_DURATION_TABLE RadarWidthSignalDurationTable[]=
{ //Pattern num, Session interval, Theorical duration, Tolerance
	//FCC
	{FCC, ((0x7D0+0x3E8)/2), ((0x7D0-0x3E8)/2)},
	{JAP, ((0x7D0+0x3E8)/2), ((0x7D0-0x3E8)/2)},
	{JAP_W56, ((0x7D0+0x3E8)/2), ((0x7D0-0x3E8)/2)},
};
#define RD_WIDTH_TAB_SIZE (sizeof(RadarWidthSignalDurationTable) / sizeof(RADAR_DURATION_TABLE))
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


UCHAR RdIdleTimeTable[MAX_RD_REGION][4] =
{
	{9, 250, 250, 250},		// CE
	{4, 250, 250, 250},		// FCC
	{4, 250, 250, 250},		// JAP
	{15, 250, 250, 250},	// JAP_W53
	{4, 250, 250, 250}		// JAP_W56
};

#ifdef TONE_RADAR_DETECT_SUPPORT
static void ToneRadarProgram(PRTMP_ADAPTER pAd);
static void ToneRadarEnable(PRTMP_ADAPTER pAd);
#endif // TONE_RADAR_DETECT_SUPPORT //

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
/*
	========================================================================

	Routine Description:
		Bbp Radar detection routine

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:

	========================================================================
*/
VOID BbpRadarDetectionStart(
	IN PRTMP_ADAPTER pAd)
{
	UINT8 RadarPeriod;

	if (pAd->CommonCfg.dfs_func >= HARDWARE_DFS_V1) 
	{
		return;
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 114, 0x02);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 121, 0x20);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 122, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 123, 0x08/*0x80*/);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 124, 0x28);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 125, 0xff);

#ifdef RTMP_RBUS_SUPPORT
	if ((pAd->CommonCfg.RadarDetect.RDDurRegion == JAP) || (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53) || (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56))
	{
		pAd->CommonCfg.RadarDetect.RDDurRegion = JAP;
		pAd->CommonCfg.RadarDetect.RDDurRegion = JapRadarType(pAd);
		if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
		{
			pAd->CommonCfg.RadarDetect.DfsSessionTime = 13;
		}
		else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
		{
			pAd->CommonCfg.RadarDetect.DfsSessionTime = 15;
		}
	}
#endif // RTMP_RBUS_SUPPORT //

	RadarPeriod = ((UINT)RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][0] + (UINT)pAd->CommonCfg.RadarDetect.DfsSessionTime) < 250 ?
			(RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][0] + pAd->CommonCfg.RadarDetect.DfsSessionTime) : 250;

#ifdef RTMP_RBUS_SUPPORT
#ifdef RT2880
	pAd->CommonCfg.R65 = 0x1d;
	pAd->CommonCfg.R66 = 0x60;
#endif // RT2880 //

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
		if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		{
			// make sure CarrierDetect wont send CTS
			CARRIER_DETECT_STOP(pAd);
		}
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

#else // Original RT28xx source code.
	RTMP_IO_WRITE8(pAd, 0x7020, 0x1d);
	RTMP_IO_WRITE8(pAd, 0x7021, 0x40);
#endif // RTMP_RBUS_SUPPORT //

	RadarDetectionStart(pAd, 0, RadarPeriod);

	return;
}

/*
	========================================================================

	Routine Description:
		Bbp Radar detection routine

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:

	========================================================================
*/
#ifdef DFS_SOFTWARE_SUPPORT
VOID BbpRadarDetectionStop(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.dfs_func >= HARDWARE_DFS_V1) 
	{
		return;
	}

	RTMP_IO_WRITE8(pAd, 0x7020, 0x1d);
	RTMP_IO_WRITE8(pAd, 0x7021, 0x60);

	RadarDetectionStop(pAd);
	return;
}
#endif // DFS_SOFTWARE_SUPPORT //
/*
	========================================================================

	Routine Description:
		Radar detection routine

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:

	========================================================================
*/
VOID RadarDetectionStart(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN CTSProtect,
	IN UINT8 CTSPeriod)
{
	UINT8 DfsActiveTime = (pAd->CommonCfg.RadarDetect.DfsSessionTime & 0x1f);
	UINT8 CtsProtect = (CTSProtect == 1) ? 0x02 : 0x01; // CTS protect.

	if (CTSProtect != 0)
	{
		switch(pAd->CommonCfg.RadarDetect.RDDurRegion)
		{
		case FCC:
		case JAP_W56:
			CtsProtect = 0x03;
			break;

		case JAP:
			{
				UCHAR RDDurRegion;
				RDDurRegion = JapRadarType(pAd);
				if (RDDurRegion == JAP_W56)
					CtsProtect = 0x03;
				else
					CtsProtect = 0x02;
				break;
			}

		case CE:
		case JAP_W53:
		default:
			CtsProtect = 0x02;
			break;
		}
	}
	else
		CtsProtect = 0x01;
	

	// send start-RD with CTS protection command to MCU
	// highbyte [7]		reserve
	// highbyte [6:5]	0x: stop Carrier/Radar detection
	// highbyte [10]:	Start Carrier/Radar detection without CTS protection, 11: Start Carrier/Radar detection with CTS protection
	// highbyte [4:0]	Radar/carrier detection duration. In 1ms.

	// lowbyte [7:0]	Radar/carrier detection period, in 1ms.
	AsicSendCommandToMcu(pAd, 0x60, 0xff, CTSPeriod, DfsActiveTime | (CtsProtect << 5));
	//AsicSendCommandToMcu(pAd, 0x63, 0xff, 10, 0);

	return;
}

/*
	========================================================================

	Routine Description:
		Radar detection routine

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		TRUE	Found radar signal
		FALSE	Not found radar signal

	========================================================================
*/
VOID RadarDetectionStop(
	IN PRTMP_ADAPTER	pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("RadarDetectionStop.\n"));
	AsicSendCommandToMcu(pAd, 0x60, 0xff, 0x00, 0x00);	// send start-RD with CTS protection command to MCU

	return;
}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //


/*
	========================================================================

	Routine Description:
		Radar channel check routine

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		TRUE	need to do radar detect
		FALSE	need not to do radar detect

	========================================================================
*/
BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT		i;
	BOOLEAN result = FALSE;

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (Ch == pAd->ChannelList[i].Channel)
		{
			result = pAd->ChannelList[i].DfsReq;
			break;
		}
	}

	return result;
}


#ifdef DFS_SUPPORT

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};

	if (pAd->CommonCfg.RadarDetect.RDDurRegion != JAP)
	{
		return pAd->CommonCfg.RadarDetect.RDDurRegion;
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
		return JAP; // W52

}


ULONG RTMPReadRadarDuration(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG result = 0;

#ifdef DFS_SUPPORT
	UINT8 duration1 = 0, duration2 = 0, duration3 = 0;

#ifdef RT2880
#ifdef DFS_DEBUG
	if (pAd->CommonCfg.McuRadarDebug & RADAR_SIMULATE)
	{
		pAd->CommonCfg.McuRadarDebug &= ~RADAR_SIMULATE;
		duration1 = 0;
		duration2 = 0x6f;
		duration3 = 0x9c;
		printk("Simulate Hit R116 = %x\n", duration1);
		printk("Simulate Hit R117 = %x\n", duration2);
		printk("Simulate Hit R118 = %x\n", duration3);
		result = (duration1 << 16) + (duration2 << 8) + duration3;
		return result;
	}
#endif // DFS_DEBUG //
#endif // RT2880 //

	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R116, &duration1);
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R117, &duration2);
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R118, &duration3);
	result = (duration1 << 16) + (duration2 << 8) + duration3;
#endif // DFS_SUPPORT //

	return result;

}

VOID RTMPCleanRadarDuration(
	IN PRTMP_ADAPTER	pAd)
{
	return;
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

	pAd->CommonCfg.RadarDetect.InServiceMonitorCount++;

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
	//radar detect
	if ((pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
	{
		RadarDetectPeriodic(pAd);
	}
	return;
}

// Periodic Radar detection, switch channel will occur in RTMPHandleTBTTInterrupt()
// Before switch channel, driver needs doing channel switch announcement.
VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd)
{
#ifdef RT2880
	ULONG Value;

	/* Roger add to fix false detection(long pulse only) in the first 60 seconds */
	if (pAd->CommonCfg.W56_debug)
	{
		if (pAd->CommonCfg.W56_idx < 300)
		{
			pAd->CommonCfg.RadarElectNum = 5;
		}
		else if (pAd->CommonCfg.W56_total <= 5000)
		{
			if (pAd->CommonCfg.RadarElectNum > 4)
				pAd->CommonCfg.RadarElectNum--;
			else
				pAd->CommonCfg.RadarElectNum = 3;
		}
		else if (pAd->CommonCfg.W56_total <= 10000)
		{
			if (pAd->CommonCfg.RadarElectNum > 5)
				pAd->CommonCfg.RadarElectNum--;
			else
				pAd->CommonCfg.RadarElectNum = 4;
		}
		else if (pAd->CommonCfg.W56_total <= 20000)
		{
			if (pAd->CommonCfg.RadarElectNum > 7)
				pAd->CommonCfg.RadarElectNum--;
			else if (pAd->CommonCfg.RadarElectNum < 5)
				pAd->CommonCfg.RadarElectNum++;
			else
				pAd->CommonCfg.RadarElectNum = 6;
		}
		else if (pAd->CommonCfg.W56_total <= 30000)
		{
			if (pAd->CommonCfg.RadarElectNum > 8)
				pAd->CommonCfg.RadarElectNum--;
			else if (pAd->CommonCfg.RadarElectNum < 6)
				pAd->CommonCfg.RadarElectNum++;
			else
				pAd->CommonCfg.RadarElectNum = 7;
		}
		else if (pAd->CommonCfg.W56_total <= 50000)
		{
			if (pAd->CommonCfg.RadarElectNum > 9)
				pAd->CommonCfg.RadarElectNum--;
			else if (pAd->CommonCfg.RadarElectNum < 6)
				pAd->CommonCfg.RadarElectNum++;
			else
				pAd->CommonCfg.RadarElectNum = 8;
		}
		else if (pAd->CommonCfg.W56_total <= 70000)
		{
			if (pAd->CommonCfg.RadarElectNum > 7)
				pAd->CommonCfg.RadarElectNum--;
			else if (pAd->CommonCfg.RadarElectNum < 8)
				pAd->CommonCfg.RadarElectNum++;
			else
				pAd->CommonCfg.RadarElectNum = 9;
		}
		else
		{
			if (pAd->CommonCfg.RadarElectNum < 9)
				pAd->CommonCfg.RadarElectNum++;
			else
				pAd->CommonCfg.RadarElectNum = 10;
		}		
	}
#endif // RT2880 //

	// need to check channel availability, after switch channel
	if (pAd->CommonCfg.RadarDetect.RDMode != RD_SILENCE_MODE)
			return;

#ifdef RT2880
#ifdef DFS_SOFTWARE_SUPPORT
	if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1) 
	{
		/* Roger add to fix false detection(long pulse only) in the first 60 seconds */
		if ((pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56) || (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC))
		{
			if (pAd->CommonCfg.W56_debug == 0)
			{
				RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &pAd->CommonCfg.W56_hw_1);
				RTMP_IO_READ32(pAd, CH_TIME_CFG, &Value);
				RTMP_IO_WRITE32(pAd, CH_TIME_CFG, Value | 1);
				pAd->CommonCfg.W56_hw_sum = 0;
				pAd->CommonCfg.W56_idx = 0;
				pAd->CommonCfg.W56_debug = 1;
			}
		}
	}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // RT2880 //


	// channel availability check time is 60sec, use 65 for assurance
	if (pAd->CommonCfg.RadarDetect.RDCount++ > pAd->CommonCfg.RadarDetect.ChMovingTime)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Not found radar signal, start send beacon and radar detection in service monitor\n\n"));
#ifdef DFS_SOFTWARE_SUPPORT
		if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1) 
			BbpRadarDetectionStop(pAd);
#endif // DFS_SOFTWARE_SUPPORT //

#ifdef RT2880
		pAd->CommonCfg.R66 = pAd->CommonCfg.DFS_R66;
#endif // RT2880 //

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef CARRIER_DETECTION_SUPPORT
			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
			{
				// trun on Carrier-Detection. (Carrier-Detect with CTS protection).
				CARRIER_DETECT_START(pAd, 1);
			}
#endif // CARRIER_DETECTION_SUPPORT //
		}
#endif // CONFIG_AP_SUPPORT //
		AsicEnableBssSync(pAd);
		pAd->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;

#ifdef RT2880
#ifdef DFS_SOFTWARE_SUPPORT
		if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1) 
		{
			if ((pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56) || (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC))
			{
				pAd->CommonCfg.W56_debug = 0;
				RTMP_IO_READ32(pAd, CH_TIME_CFG, &Value);
				if (Value & 1)
				{
					RTMP_IO_WRITE32(pAd, CH_TIME_CFG, Value & ~1);
				}
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 114, 0x02);
			}
		}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // RT2880 //


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef DFS_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
#ifdef DFS_HARDWARE_SUPPORT
	if ((pAd->MACVersion == 0x28720200) && (pAd->CommonCfg.CID == 0x200))
	{
		if (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
		{
			return;
		}
		//NewRadarDetectionStart(pAd);
	}
	else
#endif // DFS_HARDWARE_SUPPORT //
#endif // RTMP_RBUS_SUPPORT //
	{
#ifdef DFS_SOFTWARE_SUPPORT
		if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1) 
			AdaptRadarDetection(pAd);   // start radar detection.
#endif // DFS_SOFTWARE_SUPPORT //
	}
#endif // DFS_SUPPORT //
		}
#endif // CONFIG_AP_SUPPORT //

		return;
	}

	return;
}
#endif // DFS_SUPPORT //

#ifdef CONFIG_AP_SUPPORT
void RTMPPrepareRDCTSFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA,
	IN	ULONG			Duration,
	IN  UCHAR           RTSRate,
	IN  ULONG           CTSBaseAddr,
	IN  UCHAR			FrameGap)
{
	RTS_FRAME			RtsFrame;
	PRTS_FRAME			pRtsFrame;
	TXWI_STRUC			TxWI;
	PTXWI_STRUC			pTxWI;
	HTTRANSMIT_SETTING	CtsTransmit;
	UCHAR				Wcid;
	UCHAR				*ptr;
	UINT				i;

	if (CTSBaseAddr == HW_CS_CTS_BASE) // CTS frame for carrier-sense.
		Wcid = CS_CTS_WCID;
	else if (CTSBaseAddr == HW_DFS_CTS_BASE) // CTS frame for radar-detection.
		Wcid = DFS_CTS_WCID;
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s illegal address (%lx) for CTS frame buffer.\n", __FUNCTION__, CTSBaseAddr));
		return;
	}

	pRtsFrame = &RtsFrame;
	pTxWI = &TxWI;

	NdisZeroMemory(pRtsFrame, sizeof(RTS_FRAME));
	pRtsFrame->FC.Type    = BTYPE_CNTL;
	pRtsFrame->Duration = (USHORT)Duration;

	// Write Tx descriptor
	pRtsFrame->FC.SubType = SUBTYPE_CTS;
	COPY_MAC_ADDR(pRtsFrame->Addr1, pAd->CurrentAddress);

	if (pAd->CommonCfg.Channel <= 14)
		CtsTransmit.word = 0;
	else
		CtsTransmit.word = 0x4000;

	RTMPWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, 0, Wcid, 
		10, PID_MGMT, 0, 0,IFS_SIFS, FALSE, &CtsTransmit);
	pTxWI->PacketId = 0x5;

	ptr = (PUCHAR)pTxWI;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(ptr, TYPE_TXWI);
#endif
	for (i=0; i<TXWI_SIZE; i+=4)  // 16-byte TXWI field
	{
		UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_IO_WRITE32(pAd, CTSBaseAddr + i, longptr);
		ptr += 4;
	}

	// update CTS frame content. start right after the 24-byte TXINFO field
	ptr = (PUCHAR)pRtsFrame;
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, FALSE);
#endif
	for (i=0; i<10; i+=4)
	{
		UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_IO_WRITE32(pAd, CTSBaseAddr + TXWI_SIZE + i, longptr);
		ptr += 4;
	}

	AsicUpdateRxWCIDTable(pAd, Wcid, pAd->ApCfg.MBSSID[MAIN_MBSSID].Bssid);
}

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
VOID RTMPPrepareRadarDetectParams(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32 DfsSessionTime = pAd->CommonCfg.RadarDetect.DfsSessionTime;

	// Initialize parameters for firmware-base radar detection
	// Fill CTS frame
	// change 1st CTS's duration from 15100 to 25100 for Carrier-Sense feature.
	RTMPPrepareRDCTSFrame(pAd, pAd->CurrentAddress,	25100, pAd->CommonCfg.RtsRate, HW_CS_CTS_BASE, IFS_SIFS);
	RTMPPrepareRDCTSFrame(pAd, pAd->CurrentAddress,	(DfsSessionTime * 1000 + 100), pAd->CommonCfg.RtsRate, HW_DFS_CTS_BASE, IFS_SIFS);

	// Carrier-Sense function Rx power gain.
	// The value here will fill to BBP R66 during Carrier detection  period.
	// There are four bytes (20Mhz-A band)(20Mhz-G band)(40Mhz-A band)(40Mhz-G band).
	RTMP_IO_WRITE32(pAd, 0x702c, 0x40305040);
}


void RadarSMDetect(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN RadarType)
{
	INT	i;
	UINT32 RadarSignalDuration;

	if (pAd->CommonCfg.RadarDetect.RDMode == RD_SWITCHING_MODE)
		return;


	// In service monitor
	if (RadarType == RADAR_PULSE)
	{
	RadarSignalDuration = RTMPReadRadarDuration(pAd);
	}
	else
	{
		int loop;
		UINT32 Value;
		int validCnt = 0;
		int ValidCntLimit;
		UINT32 DurValue[3];

		NdisZeroMemory(DurValue, 3);
		for (loop=0; loop < (pAd->CommonCfg.RadarDetect.DfsSessionTime/2); loop++)
		{
			RTMP_IO_READ32(pAd, 0x7100 + (loop * 4), &Value);
			if (Value)
			{
				DurValue[validCnt] = Value & 0x00ffffff;
				validCnt++;
				if (validCnt >= 3) 
					return;
			}
		}

		if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
			ValidCntLimit = 1;
		else
			ValidCntLimit = 1;

		if (validCnt > ValidCntLimit)
			return;

		RadarSignalDuration = DurValue[0];
	}

#ifdef RTMP_RBUS_SUPPORT
#ifdef DFS_DEBUG
	if (pAd->CommonCfg.McuRadarDebug & RADAR_DONT_SWITCH)
	{
		if (RadarSignalDetermination(pAd, RadarType, RadarSignalDuration))
		{
			printk("Radar Type=%d, RadarDuration=%x\n", RadarType, RadarSignalDuration);
		}
		return;
	}
#endif // DFS_DEBUG //
#endif // RTMP_RBUS_SUPPORT //

	if (RadarSignalDetermination(pAd, RadarType, RadarSignalDuration))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Radar Type=%d, RadarDuration=%x\n", RadarType, RadarSignalDuration));

		if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
		{
			for (i = 0; i < pAd->ChannelListNum ; i++)
			{
				pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, ChannelAlgRandom);
				if ((pAd->CommonCfg.Channel >= 100) && (pAd->CommonCfg.Channel <= 140))
					break;
			}
		}
		else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
		{
			for (i = 0; i < pAd->ChannelListNum ; i++)
			{
				pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, ChannelAlgRandom);
				if ((pAd->CommonCfg.Channel >= 36) && (pAd->CommonCfg.Channel <= 60))
					break;
			}
		}
		else
			pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, ChannelAlgRandom);

#ifdef DOT11_N_SUPPORT
		N_ChannelCheck(pAd);
#endif // DOT11_N_SUPPORT //

#ifdef WDS_SUPPORT
		// info neighbor APs that Radar signal found throgh WDS link.
		for (i = 0; i < MAX_WDS_ENTRY; i++)
		{
			// send Channel Switch Action frame to info Neighbro APs.
			if (ValidWdsEntry(pAd, i))
				EnqueueChSwAnn(pAd, pAd->WdsTab.WdsEntry[i].PeerWdsAddr, 0, pAd->CommonCfg.Channel);
		}
#endif // WDS_SUPPORT //

		for (i=0; i<pAd->ChannelListNum; i++)
		{
			if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
			{
				pAd->ChannelList[i].RemainingTimeForUse = 1800;//30 min = 1800 sec
				break;
			}
		}

		RadarDetectionStop(pAd);   // send stop-RD command to MCU
		mdelay(5);
		RadarDetectionStop(pAd);   // To Make sure command is sucessful

#ifdef RTMP_RBUS_SUPPORT
		// Roger test // to allow TBTTinterrupt do Channel Switch count down
		{
			BCN_TIME_CFG_STRUC csr;
			RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
			csr.field.bTBTTEnable = 1;
			RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
		}
		pAd->CommonCfg.RadarDetect.ChMovingTime = 65;
#endif // RTMP_RBUS_SUPPORT //

		pAd->CommonCfg.RadarDetect.RDMode = RD_SWITCHING_MODE;
		pAd->CommonCfg.RadarDetect.CSCount = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("Radar signal(Duration=%x) exist when in service monitor, will switch to ch%d !!!\n\n\n", RadarSignalDuration, pAd->CommonCfg.Channel));
#ifdef RT2880
		pAd->CommonCfg.W56_debug = 0;
#endif // RT2880 //
	}
	

	RTMPCleanRadarDuration(pAd);

	return;
}

#ifdef RTMP_RBUS_SUPPORT
VOID AdaptRadarDetection(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR Idx;
	UINT8 RadarPeriod;
	BOOLEAN CtsProtect;
	ULONG OneSecTotalXmitCnt = pAd->RalinkCounters.OneSecRxCount + pAd->RalinkCounters.OneSecTxDoneCount;

#ifdef DFS_DEBUG
	DBGPRINT(RT_DEBUG_TRACE, ("OneSecTotalXmitCnt = %ld        ", OneSecTotalXmitCnt));
#endif
	
	if (pAd->CommonCfg.RadarDetect.FixDfsLimit == 0)
	{
		pAd->CommonCfg.RadarDetect.upperlimit = 4000;
		pAd->CommonCfg.RadarDetect.lowerlimit = 250;
		
		if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
		{
			pAd->CommonCfg.RadarDetect.upperlimit = 7500;
		}
		else if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
		{
			pAd->CommonCfg.RadarDetect.upperlimit = 1000;
			pAd->CommonCfg.RadarDetect.lowerlimit = 100;
		}
		else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
		{
			pAd->CommonCfg.RadarDetect.upperlimit = 2500;
		}
	}
	else
	{
		pAd->CommonCfg.RadarDetect.upperlimit = pAd->CommonCfg.RadarDetect.DfsUpperLimit;
		pAd->CommonCfg.RadarDetect.lowerlimit = pAd->CommonCfg.RadarDetect.DfsLowerLimit;
	}
	
	// AP should not send any packet out during Carrier-Signal exist.
	// Those commands below will let Mac start sending CTS for Radar-Detection.
	if (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
	{
		return;
	}

	if (OneSecTotalXmitCnt > pAd->CommonCfg.RadarDetect.upperlimit)
	{
		Idx = 1;
	}
	else
	{
		Idx = 0;
	}

	// Cutomer report a problem that Ralink AP with DFS enable
	// really hurt other wireless divec performance since Ralink-AP
	// send out CTS frame to protect air for Radar-signal detection.
	// To avoid that AP should not send out too much CTS frames out in idle mode by David,Tung recommend.
	if (OneSecTotalXmitCnt < pAd->CommonCfg.RadarDetect.lowerlimit)
		Idx = 3;

	// according to Andy's request. avoid send too much CTS frame out in low signal strenght.
	if (RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi0, pAd->ApCfg.RssiSample.AvgRssi1, pAd->ApCfg.RssiSample.AvgRssi2) < pAd->CommonCfg.RadarDetect.AvgRssiReq)
		Idx = 3;

	if (pAd->CommonCfg.RadarDetect.bFastDfs)
		Idx = 0;


	if (pAd->CommonCfg.RadarDetect.RDDurRegion < MAX_RD_REGION)
	{
		RadarPeriod = ((UINT)RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][Idx] + (UINT)pAd->CommonCfg.RadarDetect.DfsSessionTime) < 250 ?
			(RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][Idx] + pAd->CommonCfg.RadarDetect.DfsSessionTime) : 250;

		// Roger test
		if ((pAd->CommonCfg.RadarDetect.RDDurRegion == JAP) || (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53) || (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56))
		{
			
			pAd->CommonCfg.RadarDetect.RDDurRegion = JAP;
			pAd->CommonCfg.RadarDetect.RDDurRegion = JapRadarType(pAd);
			if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
			{
				RadarPeriod = ((UINT)RdIdleTimeTable[JAP_W56][Idx] + (UINT)pAd->CommonCfg.RadarDetect.DfsSessionTime) < 250 ?
					(RdIdleTimeTable[JAP_W56][Idx] + pAd->CommonCfg.RadarDetect.DfsSessionTime) : 250;
				pAd->CommonCfg.RadarDetect.DfsSessionTime = 13;
			}
			else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
			{
				RadarPeriod = ((UINT)RdIdleTimeTable[JAP_W53][Idx] + (UINT)pAd->CommonCfg.RadarDetect.DfsSessionTime) < 250 ?
					(RdIdleTimeTable[JAP_W53][Idx] + pAd->CommonCfg.RadarDetect.DfsSessionTime) : 250;
				pAd->CommonCfg.RadarDetect.DfsSessionTime = 15;
			}
			else
			{
				//RadarDetectionStop(pAd);
				return;
			}
			
		}
	}
	else
	{
		printk("Unknow RD-Duration Region. RD-Regsion=%d\n", pAd->CommonCfg.RadarDetect.RDDurRegion);
		return;
	}

	if ((pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif // CARRIER_DETECT_SUPPORT //
		)
		CtsProtect = FALSE;
	else
		CtsProtect = TRUE;
		
	RadarDetectionStart(pAd, CtsProtect, RadarPeriod);
}
#else // original Rt28xx source code
VOID AdaptRadarDetection(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR Idx;
	UINT8 RadarPeriod;
	BOOLEAN CtsProtect;
	ULONG OneSecTotalXmitCnt = pAd->RalinkCounters.OneSecRxCount + pAd->RalinkCounters.OneSecTxDoneCount;
	ULONG upperlimit = 4000, lowerlimit = 500;

	//DBGPRINT(RT_DEBUG_TRACE, ("OneSecTotalXmitCnt = %d        ", OneSecTotalXmitCnt));
	
	if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
	{
		upperlimit = 7500;
	}
	else if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
	{
		upperlimit = 1000;
		lowerlimit = 150;
	}
	else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
	{
		upperlimit = 2500;
	}

	// AP should not send any packet out during Carrier-Signal exist.
	// Those commands below will let Mac start sending CTS for Radar-Detection.
	if (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
	{
		return;
	}

	if (OneSecTotalXmitCnt > upperlimit)
	{
		Idx = 1;
	}
	else
	{
		Idx = 0;
	}

	// Cutomer report a problem that Ralink AP with DFS enable
	// really hurt other wireless divec performance since Ralink-AP
	// send out CTS frame to protect air for Radar-signal detection.
	// To avoid that AP should not send out too much CTS frames out in idle mode by David,Tung recommend.
	if (OneSecTotalXmitCnt < lowerlimit)
		Idx = 3;

	// according to Andy's request. avoid send too much CTS frame out in low signal strenght.
	if (RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi0, pAd->ApCfg.RssiSample.AvgRssi1, pAd->ApCfg.RssiSample.AvgRssi2) < -65)
		Idx = 3;

	if (pAd->CommonCfg.RadarDetect.bFastDfs)
		Idx = 0;


	if (pAd->CommonCfg.RadarDetect.RDDurRegion < MAX_RD_REGION)
	{
		RadarPeriod = ((UINT)RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][Idx] + (UINT)pAd->CommonCfg.RadarDetect.DfsSessionTime) < 250 ?
			(RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][Idx] + pAd->CommonCfg.RadarDetect.DfsSessionTime) : 250;
	}
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("Unknow RD-Duration Region. RD-Regsion=%d\n", pAd->CommonCfg.RadarDetect.RDDurRegion));
		return;
	}

	if ((pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif // CARRIER_DETECT_SUPPORT //
		)
		CtsProtect = FALSE;
	else
		CtsProtect = TRUE;
		
	RadarDetectionStart(pAd, CtsProtect, RadarPeriod);
}
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_SOFTWARE_SUPPORT //


VOID DFSStartTrigger(
	IN PRTMP_ADAPTER pAd)
{
	AsicSendCommandToMcu(pAd, 0x62, 0xff, 0x00, 0x00);
}

/* 
    ==========================================================================
    Description:
        Enable or Disable Fast Radar Detection feature.
		The function is only for to pass DFS certification.
		Enabling it will deep impact throughput testing.
		So don't enable it except for DFS certification.

	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set CarrierDetect=[1/0]
    ==========================================================================
*/
INT Set_FastDfs_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	UINT Enable;

	Enable = (UINT) simple_strtol(arg, 0, 10);

	pAd->CommonCfg.RadarDetect.bFastDfs = (BOOLEAN)(Enable == 0 ? FALSE : TRUE);

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %s\n", __FUNCTION__,
		pAd->CommonCfg.RadarDetect.bFastDfs == TRUE ? "Enable FastDfs":"Disable FastDfs"));

	return TRUE;
}

#ifdef DFS_SOFTWARE_SUPPORT
static BOOLEAN RadarSignalDetermination(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN RadarType,
	IN UINT32 RadarSignalDuration)
{
	INT	index;
	ULONG RadarTabSize = RD_TAB_SIZE;
	PRADAR_DURATION_TABLE pRadarSignalTab = RadarSignalDurationTable;
	BOOLEAN result = FALSE;

	static ULONG WidthRadarSamples[10] = {0};
	static ULONG WidthRadarSamplesIdx = 0;
	UINT8 RadarElectNum = pAd->CommonCfg.RadarDetect.LongPulseRadarTh;

#ifdef RT2880
	/* Roger add to fix false detection(long pulse only) in the first 60 seconds */
	if (pAd->CommonCfg.W56_debug)
	{
		RadarElectNum = pAd->CommonCfg.RadarElectNum;
	}
#endif // RT2880 //

	if (!RadarSignalDuration)
		return FALSE;

	if (RadarType == RADAR_WIDTH)
	{
		RadarTabSize = RD_WIDTH_TAB_SIZE;
		pRadarSignalTab = RadarWidthSignalDurationTable;
	}
	else
	{
		RadarTabSize = RD_TAB_SIZE;
		pRadarSignalTab = RadarSignalDurationTable;
	}

	for (index=0; index < RadarTabSize; index++)
	{
		PRADAR_DURATION_TABLE pRadarEntry = pRadarSignalTab + index;

		if ((pRadarEntry->RDDurRegion == pAd->CommonCfg.RadarDetect.RDDurRegion)
			&& (RadarSignalDuration > (pRadarEntry->RadarSignalDuration - pRadarEntry->Tolerance)) 
			&& (RadarSignalDuration < (pRadarEntry->RadarSignalDuration + pRadarEntry->Tolerance))
			)
			break;
	}

	if (index < RadarTabSize)
		result = TRUE;
	else
		result = FALSE;

	if ((RadarType == RADAR_WIDTH)
		&& (result == TRUE))
	{
			ULONG CurRecodTime;
			ULONG PreRecodTime = WidthRadarSamples[(WidthRadarSamplesIdx + 1) % RadarElectNum];

			NdisGetSystemUpTime(&CurRecodTime);
			WidthRadarSamples[WidthRadarSamplesIdx % RadarElectNum] = CurRecodTime;

#ifdef RTMP_RBUS_SUPPORT
#ifdef DFS_DEBUG
			DBGPRINT(RT_DEBUG_TRACE, ("RadarElectNum=%d CurRecodTime = %ld, delta = %ld, Duration = %x\n", 
						RadarElectNum, CurRecodTime, ((CurRecodTime - PreRecodTime) / OS_HZ), RadarSignalDuration));
#endif // DFS_DEBUG //

			if ((PreRecodTime != 0)
				&& ((CurRecodTime - PreRecodTime) <= (6 * OS_HZ))
#else // original RT28xx source code
			if ((PreRecodTime != 0)
				&& ((CurRecodTime - PreRecodTime) < (6 * OS_HZ))
#endif // RTMP_RBUS_SUPPORT //
				)
				result = TRUE;
			else
				result = FALSE;

			WidthRadarSamplesIdx ++;
	}

	return result;
}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //

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

	pAd->CommonCfg.RadarDetect.ChMovingTime = Value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %d\n", __FUNCTION__,
		pAd->CommonCfg.RadarDetect.ChMovingTime));

	return TRUE;
}

INT Set_LongPulseRadarTh_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	UINT8 Value;

	Value = (UINT8) simple_strtol(arg, 0, 10) > 10 ? 10 : simple_strtol(arg, 0, 10);
	
	pAd->CommonCfg.RadarDetect.LongPulseRadarTh = Value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %d\n", __FUNCTION__,
		pAd->CommonCfg.RadarDetect.LongPulseRadarTh));

	return TRUE;
}
#endif // CONFIG_AP_SUPPORT //


#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef TONE_RADAR_DETECT_SUPPORT
static ULONG time[20];
static ULONG idle[20];
static ULONG busy[20];
static ULONG cd_idx=0;
#endif // CARRIER_DETECTION_SUPPORT //
#endif // TONE_RADAR_DETECT_SUPPORT //
#if defined (DFS_SUPPORT) || defined (CARRIER_DETECTION_SUPPORT)
#if defined (TONE_RADAR_DETECT_SUPPORT) || defined (RTMP_RBUS_SUPPORT) || defined (DFS_INTERRUPT_SUPPORT)

void RTMPHandleRadarInterrupt(PRTMP_ADAPTER  pAd)
{
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef TONE_RADAR_DETECT_SUPPORT 
	UINT32 value, delta;
#ifdef TONE_RADAR_DETECT_V2 
	UCHAR bbp=0;
#endif // TONE_RADAR_DETECT_V2 //
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //
#ifdef DFS_SUPPORT
#ifdef DFS_HARDWARE_SUPPORT
#ifdef DFS_INTERRUPT_SUPPORT
	NewTimerCB_Radar(pAd);
#endif // DFS_INTERRUPT_SUPPORT //
#endif // DFS_HARDWARE_SUPPORT //
#endif // DFS_SUPPORT  //
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef TONE_RADAR_DETECT_V2 
	if(pAd->CommonCfg.carrier_func==TONE_RADAR_V2)
		{
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x01);
			BBP_IO_READ8_BY_REG_ID(pAd, BBP_R185, &bbp);
			if ((bbp & 0x1) == 0)
			{
				return;
			}
		}
#endif // TONE_RADAR_DETECT_V2  //
#ifdef TONE_RADAR_DETECT_SUPPORT
	RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &value);
	RTMP_IO_READ32(pAd, CH_IDLE_STA, &pAd->CommonCfg.CarrierDetect.idle_time);
	RTMP_IO_READ32(pAd, CH_BUSY_STA, &pAd->CommonCfg.CarrierDetect.busy_time);
	delta = (value >> 4) - pAd->CommonCfg.CarrierDetect.TimeStamp;
	pAd->CommonCfg.CarrierDetect.TimeStamp = value >> 4;

	pAd->CommonCfg.CarrierDetect.OneSecIntCount++;

	if (pAd->CommonCfg.CarrierDetect.Debug)
	{
		if (cd_idx < 20)
		{
			time[cd_idx] = delta;
			idle[cd_idx] = pAd->CommonCfg.CarrierDetect.idle_time;
			busy[cd_idx] = pAd->CommonCfg.CarrierDetect.busy_time;
			cd_idx++;
		}
		else
		{
			int i;
			pAd->CommonCfg.CarrierDetect.Debug = 0;
			for (i = 0; i < 20; i++)
			{
				printk("%3d %4ld %ld %ld\n", i, time[i], idle[i], busy[i]);
			}
			cd_idx = 0;
			
		}
	}
	

	if (pAd->CommonCfg.CarrierDetect.CD_State == CD_NORMAL)
	{
		if ((delta < pAd->CommonCfg.CarrierDetect.criteria) && (pAd->CommonCfg.CarrierDetect.recheck))
			pAd->CommonCfg.CarrierDetect.recheck --;
		else
			if (pAd->CommonCfg.CarrierDetect.recheck<pAd->CommonCfg.CarrierDetect.recheck1)
			pAd->CommonCfg.CarrierDetect.recheck ++;
		if (pAd->CommonCfg.CarrierDetect.recheck == 0)
		{
			// declare carrier sense
			pAd->CommonCfg.CarrierDetect.CD_State = CD_SILENCE;
			//pAd->CommonCfg.CarrierDetect.recheck = pAd->CommonCfg.CarrierDetect.recheck1;

			if (pAd->CommonCfg.CarrierDetect.CarrierDebug == 0)
			{
	
				DBGPRINT(RT_DEBUG_TRACE, ("Carrier Detected\n"));

				// disconnect all STAs behind AP.
				//MacTableReset(pAd);
				
				// stop all TX actions including Beacon sending.
				AsicDisableSync(pAd);
			}
			else
			{
				printk("Carrier Detected\n");
			}
			

		}
	}

	
	if (pAd->CommonCfg.CarrierDetect.Enable)
	{
		ToneRadarProgram(pAd);
		ToneRadarEnable(pAd);
	}
#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //

}
#endif // define(TONE_RADAR_DETECT_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //
#endif // define(DFS_SUPPORT) || define(CARRIER_DETECTION_SUPPORT) //
#ifdef CARRIER_DETECTION_SUPPORT
VOID CarrierDetectionFsm(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 CurFalseCCA)
{
#define CAR_SAMPLE_NUM 3

	INT i;
	BOOLEAN bCarExist = TRUE;
	BCN_TIME_CFG_STRUC csr;
	CD_STATE CurrentState = pAd->CommonCfg.CarrierDetect.CD_State;
	//UINT32 AvgFalseCCA;
	UINT32 FalseCCAThreshold;
	static int Cnt = 0;
	static UINT32 FalseCCA[CAR_SAMPLE_NUM] = {0};

	FalseCCA[(Cnt++) % CAR_SAMPLE_NUM] = CurFalseCCA;
	//AvgFalseCCA = (FalseCCA[0] + FalseCCA[1] + FalseCCA[2]) / 3;
	//DBGPRINT(RT_DEBUG_TRACE, ("%s: AvgFalseCCA=%d, CurFalseCCA=%d\n", __FUNCTION__, AvgFalseCCA, CurFalseCCA));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: FalseCCA[0]=%d, FalseCCA[1]=%d, FalseCCA[2]=%d,\n",
		__FUNCTION__, FalseCCA[0], FalseCCA[1], FalseCCA[2]));

	if ((pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == TRUE))
	{
		FalseCCAThreshold = CARRIER_DETECT_HIGH_BOUNDARY_1;
	}
	else
	{
		FalseCCAThreshold = CARRIER_DETECT_HIGH_BOUNDARY_2;
	}

	switch (CurrentState)
	{
		case CD_NORMAL:
			for (i = 0; i < CAR_SAMPLE_NUM; i++)
				bCarExist &= (FalseCCA[i] > FalseCCAThreshold);
			//if (AvgFalseCCA > FalseCCAThreshold)
			//if (FalseCCA[0] > FalseCCAThreshold
			//	&& FalseCCA[1] > FalseCCAThreshold
			//	&& FalseCCA[2] > FalseCCAThreshold)
			if (bCarExist)
			{
				// MacTabReset() will clean whole MAC table no matther what kind entry.
				// In the case, the WDS links will be deleted here and never recovery it back again.
				// Ralink STA also added Carrier-Sense function now
				// os it's no necessary to disconnect STAs here.
#ifdef RTMP_RBUS_SUPPORT
				// disconnect all STAs behind AP.
				MacTableReset(pAd);
#else // Original RT28xx source code
				/* disconnect all STAs behind AP. */
				//MacTableReset(pAd);
#endif // RTMP_RBUS_SUPPORT //

				// change state to CD_SILENCE.
				pAd->CommonCfg.CarrierDetect.CD_State = CD_SILENCE;

				// Stop sending CTS for Carrier Detection.

				CARRIER_DETECT_START(pAd, 0);

				// stop all TX actions including Beacon sending.
				AsicDisableSync(pAd);
				if ((pAd->CommonCfg.Channel > 14)
					&& (pAd->CommonCfg.bIEEE80211H == TRUE))
				{
#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
					AdaptRadarDetection(pAd);
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //
				}

				DBGPRINT(RT_DEBUG_TRACE, ("Carrier signal detected. Change State to CD_SILENCE.\n"));
			}
			break;

		case CD_SILENCE:
			// check that all TX been blocked properly.
			RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
			if (csr.field.bBeaconGen == 1)
			{
				// Stop sending CTS for Carrier Detection.

				CARRIER_DETECT_START(pAd, 0);
				AsicDisableSync(pAd);
				if ((pAd->CommonCfg.Channel > 14)
					&& (pAd->CommonCfg.bIEEE80211H == TRUE))
				{
#ifdef RT2880
#ifdef DFS_HARDWARE_SUPPORT
					if (pAd->CommonCfg.dfs_func >= HARDWARE_DFS_V1) 
					{
						if (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
						{
							return;
						}
						//NewRadarDetectionStart(pAd);
					}
					else
#endif // DFS_HARDWARE_SUPPORT //
#endif // RT2880 //
#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
					AdaptRadarDetection(pAd);
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //
				}
			}

			// check carrier signal.
			for (i = 0; i < CAR_SAMPLE_NUM; i++)
				bCarExist &= (FalseCCA[i] < CARRIER_DETECT_LOW_BOUNDARY);

			//if (AvgFalseCCA < CARRIER_DETECT_LOW_BOUNDARY)
			//if (FalseCCA[0] < CARRIER_DETECT_LOW_BOUNDARY
			//	&& FalseCCA[1] < CARRIER_DETECT_LOW_BOUNDARY
			//	&& FalseCCA[2] < CARRIER_DETECT_LOW_BOUNDARY)
			if (bCarExist)
			{
				// change state to CD_NORMAL.
				pAd->CommonCfg.CarrierDetect.CD_State = CD_NORMAL;

				// Start sending CTS for Carrier Detection.
				CARRIER_DETECT_START(pAd, 1);

				// start all TX actions.
				APMakeAllBssBeacon(pAd);
				APUpdateAllBeaconFrame(pAd);
				AsicEnableBssSync(pAd);

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
				if ((pAd->CommonCfg.Channel > 14)
					&& (pAd->CommonCfg.bIEEE80211H == TRUE))
				{
					AdaptRadarDetection(pAd);
				}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //

				DBGPRINT(RT_DEBUG_TRACE, ("Carrier signal gone. Change State to CD_NORMAL.\n"));
			}
			break;

		default:
			DBGPRINT(RT_DEBUG_ERROR, ("Unknow Carrier Detection state.\n"));
			break;
	}
}

VOID CarrierDetectionCheck(
	IN PRTMP_ADAPTER pAd)
{
	RX_STA_CNT1_STRUC RxStaCnt1;

	if (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
		return;

	RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);

	pAd->RalinkCounters.OneSecFalseCCACnt += RxStaCnt1.field.FalseCca;
	pAd->PrivateInfo.PhyRxErrCnt += RxStaCnt1.field.PlcpErr;
	CarrierDetectionFsm(pAd, RxStaCnt1.field.FalseCca);

}

VOID CarrierDetectStartTrigger(
	IN PRTMP_ADAPTER pAd)
{
	AsicSendCommandToMcu(pAd, 0x62, 0xff, 0x01, 0x00);
}

/* 
    ==========================================================================
    Description:
        Enable or Disable Carrier Detection feature.
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set CarrierDetect=[1/0]
    ==========================================================================
*/
INT Set_CarrierDetect_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    UCHAR apidx = pObj->ioctl_if;

	BOOLEAN CarrierSenseCts;
	UINT Enable;


	if (apidx != MAIN_MBSSID)
		return FALSE;

	Enable = (UINT) simple_strtol(arg, 0, 10);

	pAd->CommonCfg.CarrierDetect.Enable = (BOOLEAN)(Enable == 0 ? FALSE : TRUE);

	if (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
		CarrierSenseCts = 0;
	else
		CarrierSenseCts = 1;

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CARRIER_DETECT_START(pAd, CarrierSenseCts);
	else
		CARRIER_DETECT_STOP(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %s\n", __FUNCTION__,
		pAd->CommonCfg.CarrierDetect.Enable == TRUE ? "Enable Carrier Detection":"Disable Carrier Detection"));

	return TRUE;
}

#ifdef TONE_RADAR_DETECT_SUPPORT
INT Set_CarrierCriteria_Proc(
	IN PRTMP_ADAPTER 	pAd, 
	IN PSTRING			arg)
{
	UINT32 Value;

	Value = simple_strtol(arg, 0, 10);

	pAd->CommonCfg.CarrierDetect.criteria = Value;
	// If want to carrier detection debug, set Criteria to odd number
	// To stop debug, set Criteria to even number
	pAd->CommonCfg.CarrierDetect.CarrierDebug = Value & 1;

	return TRUE;
}


INT Set_CarrierReCheck_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	pAd->CommonCfg.CarrierDetect.recheck1 = simple_strtol(arg, 0, 10);
	
	return TRUE;
}

static void ToneRadarProgram(PRTMP_ADAPTER pAd)
{
	UCHAR bbp;

#ifdef TONE_RADAR_DETECT_V1
	if(pAd->CommonCfg.carrier_func==TONE_RADAR_V1)
	{
		// programe delta delay & division bit
		DBGPRINT(RT_DEBUG_TRACE, ("3090 ToneRadarProgram\n"));

		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0xf0);
		bbp = pAd->CommonCfg.CarrierDetect.delta << 4;
		bbp |= (pAd->CommonCfg.CarrierDetect.div_flag & 0x1) << 3;
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, bbp);

		// program threshold
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x34);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (pAd->CommonCfg.CarrierDetect.threshold & 0xff000000) >> 24);

		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x24);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (pAd->CommonCfg.CarrierDetect.threshold & 0xff0000) >> 16);

		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x14);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (pAd->CommonCfg.CarrierDetect.threshold & 0xff00) >> 8);

		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x04);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, pAd->CommonCfg.CarrierDetect.threshold & 0xff);
	}
#endif // TONE_RADAR_DETECT_V1 //


#ifdef TONE_RADAR_DETECT_V2
	if(pAd->CommonCfg.carrier_func==TONE_RADAR_V2)
	{
	
	
	// programe delta delay & division bit
		DBGPRINT(RT_DEBUG_TRACE, ("3390/3090A ToneRadarProgram\n"));
	
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x05);
		bbp = pAd->CommonCfg.CarrierDetect.delta;
		bbp |= 0x10<<4;
		bbp |= (pAd->CommonCfg.CarrierDetect.div_flag & 0x1) << 6;
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, bbp);
		// program *_mask
		//RTMP_CARRIER_IO_WRITE8(pAd, 2, pAd->CommonCfg.CarrierDetect.VGA_Mask);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x02);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, pAd->CommonCfg.CarrierDetect.VGA_Mask);
		//RTMP_CARRIER_IO_WRITE8(pAd, 3, pAd->CommonCfg.CarrierDetect.Packet_End_Mask);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x03);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, pAd->CommonCfg.CarrierDetect.Packet_End_Mask);

		//RTMP_CARRIER_IO_WRITE8(pAd, 4, pAd->CommonCfg.CarrierDetect.Rx_PE_Mask);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x04);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, pAd->CommonCfg.CarrierDetect.Rx_PE_Mask);

	
	// program threshold
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x09);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (pAd->CommonCfg.CarrierDetect.threshold & 0xff000000) >> 24);
	
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x08);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (pAd->CommonCfg.CarrierDetect.threshold & 0xff0000) >> 16);
	
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x07);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, (pAd->CommonCfg.CarrierDetect.threshold & 0xff00) >> 8);
	
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x06);
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, pAd->CommonCfg.CarrierDetect.threshold & 0xff);
	}

#endif // TONE_RADAR_DETECT_V2 //

}

static void ToneRadarEnable(PRTMP_ADAPTER pAd)
{

#ifdef TONE_RADAR_DETECT_V1
	if(pAd->CommonCfg.carrier_func==TONE_RADAR_V1)
	{
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x05);
		DBGPRINT(RT_DEBUG_TRACE, ("3090 ToneRadarEnable\n"));
	}
#endif // TONE_RADAR_DETECT_V1 //

#ifdef TONE_RADAR_DETECT_V2
	if(pAd->CommonCfg.carrier_func==TONE_RADAR_V2)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("3390/3090A ToneRadarEnable\n"));

		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x01);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, 0x01);
		
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R184, 0x00);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R185, 0x01);

		
	}

#endif // TONE_RADAR_DETECT_V2 //

}


void NewCarrierDetectionStart(PRTMP_ADAPTER pAd)
{	
	ULONG Value;
	// Enable Bandwidth usage monitor
		
	DBGPRINT(RT_DEBUG_TRACE, ("NewCarrierDetectionStart\n"));
		
	RTMP_IO_READ32(pAd, CH_TIME_CFG, &Value);
	RTMP_IO_WRITE32(pAd, CH_TIME_CFG, Value | 0x1f);
	pAd->CommonCfg.CarrierDetect.recheck1 = CARRIER_DETECT_RECHECK_TIME;

	//pAd->CommonCfg.CarrierDetect.recheck2 = CARRIER_DETECT_STOP_RECHECK_TIME;
	

	// Init Carrier Detect
	if (pAd->CommonCfg.CarrierDetect.Enable)
	{
		pAd->CommonCfg.CarrierDetect.TimeStamp = 0;
		pAd->CommonCfg.CarrierDetect.recheck = pAd->CommonCfg.CarrierDetect.recheck1;
		ToneRadarProgram(pAd);
		ToneRadarEnable(pAd);
	}
	
}

#endif // TONE_RADAR_DETECT_SUPPORT //
#endif // CARRIER_DETECTION_SUPPORT //


#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
#ifdef WORKQUEUE_BH
void pulse_radar_detect_workq(struct work_struct *work)
{
	POS_COOKIE pObj = container_of(work, struct os_cookie, pulse_radar_detect_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
	
	RadarSMDetect(pAd, RADAR_PULSE);
}


void width_radar_detect_workq(struct work_struct *work)
{
	POS_COOKIE pObj = container_of(work, struct os_cookie, width_radar_detect_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
	
	RadarSMDetect(pAd, RADAR_WIDTH);
}
#else
void pulse_radar_detect_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RadarSMDetect(pAd, RADAR_PULSE);
}

void width_radar_detect_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
	
    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RadarSMDetect(pAd, RADAR_WIDTH);
}
#endif // WORKQUEUE_BH //
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //


#ifdef CARRIER_DETECTION_SUPPORT
#ifdef WORKQUEUE_BH
void carrier_sense_workq(struct work_struct *work)
{
	POS_COOKIE pObj = container_of(work, POS_COOKIE, carrier_sense_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
	
	CarrierDetectionCheck(pAd);
}
#else
void carrier_sense_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	CarrierDetectionCheck(pAd);
}
#endif // WORKQUEUE_BH //
#endif // CARRIER_DETECTION_SUPPORT //


#endif // CONFIG_AP_SUPPORT //

