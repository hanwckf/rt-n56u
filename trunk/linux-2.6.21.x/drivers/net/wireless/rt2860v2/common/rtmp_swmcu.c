/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_swmcu.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include "rt_config.h"
#include "led.h"


#ifdef WLAN_LED
extern unsigned char   LinkStatus;
extern unsigned char   SignalStrength;
extern unsigned char   GPIOPolarity;
extern ULED_PARAMETER LedParameter;
#endif // WLAN_LED //

// for 2880-SW-MCU
#ifdef CONFIG_AP_SUPPORT
#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
extern void TimerCB(PRTMP_ADAPTER pAd);
#endif
#endif // CONFIG_AP_SUPPORT //



int RtmpAsicSendCommandToSwMcu(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR Command, 
	IN UCHAR Token,
	IN UCHAR Arg0, 
	IN UCHAR Arg1)
{
	BBP_CSR_CFG_STRUC  BbpCsr, BbpCsr2;
	int             j, k;
	UINT16 Temp;
	
	switch(Command)
	{
		case 0x80:
			RTMP_IO_READ32(pAd, H2M_BBP_AGENT, &BbpCsr.word);
			if ((BbpCsr.field.Busy != 1) || (BbpCsr.field.BBP_RW_MODE != 1))
				DBGPRINT(RT_DEBUG_ERROR, ("error read write BBP 1\n"));
			
			for (j=0; j<MAX_BUSY_COUNT; j++)
			{
				RTMP_IO_READ32(pAd, BBP_CSR_CFG, &BbpCsr2.word);
				if (BbpCsr2.field.Busy == BUSY)
				{
					continue;
				}
				BbpCsr2.word = BbpCsr.word;
				RTMP_IO_WRITE32(pAd, BBP_CSR_CFG, BbpCsr2.word);
				
				
				if (BbpCsr.field.fRead == 1)
				{
					// read
					for (k=0; k<MAX_BUSY_COUNT; k++)
					{
						RTMP_IO_READ32(pAd, BBP_CSR_CFG, &BbpCsr2.word);
						if (BbpCsr2.field.Busy == IDLE)
							break;
					}
					if (k == MAX_BUSY_COUNT)
						DBGPRINT(RT_DEBUG_ERROR, ("error read write BBP 2\n"));
					
					if ((BbpCsr2.field.Busy == IDLE) && (BbpCsr2.field.RegNum == BbpCsr.field.RegNum))
					{
						BbpCsr.field.Value = BbpCsr2.field.Value;
						BbpCsr.field.Busy = IDLE;
						RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
						break;
					}
				}
				else
				{
					//write
					BbpCsr.field.Busy = IDLE;
					RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
					pAd->BbpWriteLatch[BbpCsr.field.RegNum] = BbpCsr2.field.Value;
					break;
				}
			}
			
			if (j == MAX_BUSY_COUNT)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("error read write BBP 3\n"));
				if (BbpCsr.field.Busy != IDLE)
				{
					BbpCsr.field.Busy = IDLE;
					RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, BbpCsr.word);
				}
			}
			break;
		case 0x30:
			break;
		case 0x31:
			break;
#ifdef WLAN_LED
		case 0x50:
			{					
	                        LedParameter.LedMode    = Arg0;
	                        LinkStatus                  = Arg1;
	                        SetLinkStatusEntry(pAd);
	                        break;
			}
		case 0x51:
	                        GPIOPolarity                    = Arg1;
	                        SignalStrength                  = Arg0;
	                        break;
		  case 0x52:
			Temp = ((UINT16)Arg1 << 8) | (UINT16)Arg0;
			NdisMoveMemory(&LedParameter.LedAgCfg, &Temp, 2);
			break;
		  case 0x53:
			Temp = ((UINT16)Arg1 << 8) | (UINT16)Arg0;
			NdisMoveMemory(&LedParameter.LedActCfg, &Temp, 2);
			break;
		  case 0x54:
			Temp = ((UINT16)Arg1 << 8) | (UINT16)Arg0;
			NdisMoveMemory(&LedParameter.LedPolarityCfg, &Temp, 2);
			break;
#endif // WLAN_LED //

	// 2880-SW-MCU
#ifdef CONFIG_AP_SUPPORT

#ifdef DFS_SUPPORT
		case 0x60:
			if (Arg0 == 0 && Arg1 == 0)
			{
				ULONG Value;

				pAd->CommonCfg.McuRadarCmd &= ~(RADAR_DETECTION);
				if (pAd->CommonCfg.McuRadarCmd == DETECTION_STOP)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("AsicSendCommandToMcu 0x60 ==> stop detection\n"));
#ifdef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
					unregister_tmr_service();
#else
	BOOLEAN Cancelled;
	RTMPCancelTimer(&pAd->CommonCfg.CSWatchDogTimer, &Cancelled);
#endif // RTMP_RBUS_SUPPORT //					
#endif
					RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
					Value |= 0x04;
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
				}
			}
			else
			{
				if (!(pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION))
				{
					pAd->CommonCfg.McuRadarPeriod = Arg0;
						pAd->CommonCfg.McuRadarDetectPeriod = Arg1 & 0x1f;							
					pAd->CommonCfg.McuRadarCtsProtect = (Arg1 & 0x60) >> 5;
					pAd->CommonCfg.McuRadarState = WAIT_CTS_BEING_SENT;
					pAd->CommonCfg.McuRadarTick = pAd->CommonCfg.McuRadarPeriod;
					pAd->CommonCfg.McuRadarDetectCount = 0;

					RTMPPrepareRDCTSFrame(pAd, pAd->CurrentAddress,	(pAd->CommonCfg.McuRadarDetectPeriod * 1000 + 100), pAd->CommonCfg.RtsRate, HW_DFS_CTS_BASE, IFS_SIFS);

					if (pAd->CommonCfg.McuRadarCmd == DETECTION_STOP)
					{
						pAd->CommonCfg.McuRadarCmd |= RADAR_DETECTION;
						pAd->CommonCfg.McuRadarProtection = 0;
#ifdef RTMP_RBUS_SUPPORT
						request_tmr_service(1, &TimerCB, pAd);
#else
						RTMPInitTimer(pAd, &pAd->CommonCfg.CSWatchDogTimer, GET_TIMER_FUNCTION(TimerCB), pAd,  TRUE); 
						RTMPSetTimer(&pAd->CommonCfg.DFSWatchDogTimer, NEW_DFS_WATCH_DOG_TIME);
#endif // RTMP_RBUS_SUPPORT //
					}
					else
						pAd->CommonCfg.McuRadarCmd |= RADAR_DETECTION;
#ifdef RTMP_RBUS_SUPPORT
					request_tmr_service(1, &TimerCB, pAd);
#else
					RTMPInitTimer(pAd, &pAd->CommonCfg.CSWatchDogTimer, GET_TIMER_FUNCTION(TimerCB), pAd,  TRUE); 
					RTMPSetTimer(&pAd->CommonCfg.DFSWatchDogTimer, NEW_DFS_WATCH_DOG_TIME);
#endif // RTMP_RBUS_SUPPORT //
				}
				else
				{
					pAd->CommonCfg.McuRadarPeriod = Arg0;
						pAd->CommonCfg.McuRadarDetectPeriod = Arg1 & 0x1f;							
					pAd->CommonCfg.McuRadarCtsProtect = (Arg1 & 0x60) >> 5;
					RTMPPrepareRDCTSFrame(pAd, pAd->CurrentAddress,	(pAd->CommonCfg.McuRadarDetectPeriod * 1000 + 100), pAd->CommonCfg.RtsRate, HW_DFS_CTS_BASE, IFS_SIFS);
				}
				
				
			}
			break;
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
		case 0x61:
		case 0x63:
			if (Arg0 == 0 && Arg1 == 0)
			{
				pAd->CommonCfg.McuRadarCmd &= ~(CARRIER_DETECTION);
				if (pAd->CommonCfg.McuRadarCmd == DETECTION_STOP)
				{
					ULONG Value;
					DBGPRINT(RT_DEBUG_TRACE, ("AsicSendCommandToMcu 0x60 ==> stop detection\n"));
#ifdef RTMP_RBUS_SUPPORT
					unregister_tmr_service();
#else
					BOOLEAN Cancelled;
					RTMPCancelTimer(&pAd->CommonCfg.CSWatchDogTimer, &Cancelled);
#endif // RTMP_RBUS_SUPPORT //
					RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
					Value |= 0x04;
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
				}
			}
			else
			{
				// Prepare CTS frame again, for debug 
				if (!(pAd->CommonCfg.McuRadarCmd & CARRIER_DETECTION))
				{
					pAd->CommonCfg.McuCarrierPeriod = Arg0 << ((Command == 0x61) ? 0 : 7);
					pAd->CommonCfg.McuCarrierDetectPeriod = Arg1 & 0x1f;
					pAd->CommonCfg.McuCarrierCtsProtect = (Arg1 & 0x60) >> 5;
					pAd->CommonCfg.McuCarrierState = WAIT_CTS_BEING_SENT;
					pAd->CommonCfg.McuCarrierTick = pAd->CommonCfg.McuCarrierPeriod;
					pAd->CommonCfg.McuCarrierDetectCount = 0;

					RTMPPrepareRDCTSFrame(pAd, pAd->CurrentAddress,	(pAd->CommonCfg.McuCarrierDetectPeriod * 1000 + 100), pAd->CommonCfg.RtsRate, HW_CS_CTS_BASE, IFS_SIFS);


					if (pAd->CommonCfg.McuRadarCmd == DETECTION_STOP)
					{
						pAd->CommonCfg.McuRadarCmd |= CARRIER_DETECTION;
						pAd->CommonCfg.McuRadarProtection = 0;
#ifdef RTMP_RBUS_SUPPORT
						request_tmr_service(1, &TimerCB, pAd);
#else
					RTMPInitTimer(pAd, &pAd->CommonCfg.CSWatchDogTimer, GET_TIMER_FUNCTION(TimerCB), pAd,  TRUE); 
					RTMPSetTimer(&pAd->CommonCfg.DFSWatchDogTimer, NEW_DFS_WATCH_DOG_TIME);
#endif // RTMP_RBUS_SUPPORT //
					}
					else
						pAd->CommonCfg.McuRadarCmd |= CARRIER_DETECTION;
#ifdef RTMP_RBUS_SUPPORT
					request_tmr_service(1, &TimerCB, pAd);
#else
					RTMPInitTimer(pAd, &pAd->CommonCfg.CSWatchDogTimer, GET_TIMER_FUNCTION(TimerCB), pAd,  TRUE); 
					RTMPSetTimer(&pAd->CommonCfg.CSWatchDogTimer, NEW_DFS_WATCH_DOG_TIME);
#endif // RTMP_RBUS_SUPPORT //
				}
				else
				{
					pAd->CommonCfg.McuCarrierPeriod = Arg0 << ((Command == 0x61) ? 0 : 7);
					pAd->CommonCfg.McuCarrierDetectPeriod = Arg1 & 0x1f;
					pAd->CommonCfg.McuCarrierCtsProtect = (Arg1 & 0x60) >> 5;
					RTMPPrepareRDCTSFrame(pAd, pAd->CurrentAddress,	(pAd->CommonCfg.McuCarrierDetectPeriod * 1000 + 100), pAd->CommonCfg.RtsRate, HW_CS_CTS_BASE, IFS_SIFS);
				}
				
			}
			break;
#endif // CARRIER_DETECTION_SUPPORT //

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
		case 0x62:
			if (pAd->CommonCfg.McuRadarCmd != DETECTION_STOP)
			{
				
				
				if (Arg0) // Carrier
				{
					if ((pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CARRIER_DETECTING) && (pAd->CommonCfg.McuCarrierState == WAIT_CTS_BEING_SENT))
					{
						pAd->CommonCfg.McuRadarProtection = 1;
						MCURadarDetect(pAd);
						pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_CTS_CARRIER_SENT;
						pAd->CommonCfg.McuRadarProtection = 0;
					}
				}
				else
				{
					if ((pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_RADAR_DETECTING) && (pAd->CommonCfg.McuRadarState == WAIT_CTS_BEING_SENT))
					{
						pAd->CommonCfg.McuRadarProtection = 1;
						MCURadarDetect(pAd);
						pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_CTS_SENT;
						pAd->CommonCfg.McuRadarProtection = 0;
					}
					
				}
			}
			break;
#endif // defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) //

#endif // CONFIG_AP_SUPPORT //

		default:
			break;
	}

	return 0;
}


