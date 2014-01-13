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
	dfs_mcu.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
#include "rt_config.h"

#ifdef CONFIG_AP_SUPPORT	
// Radar detection and carrier detection for 2880-SW-MCU

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
#ifdef DFS_SOFTWARE_SUPPORT
static void TimerCB_Radar(PRTMP_ADAPTER pAd);
#endif // DFS_SOFTWARE_SUPPORT //
#ifdef CARRIER_DETECTION_SUPPORT
static void TimerCB_Carrier(PRTMP_ADAPTER pAd);
#endif // CARRIER_DETECTION_SUPPORT //
#ifdef DFS_HARDWARE_SUPPORT
void NewTimerCB_Radar(IN PRTMP_ADAPTER pAd);
#endif // DFS_HARDWARE_SUPPORT //

void TimerCB(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;
	if (pAd->CommonCfg.McuRadarProtection == 1)
	{
		return;
	}

#ifdef DFS_SUPPORT

#ifdef DFS_HARDWARE_SUPPORT
	if (pAd->CommonCfg.dfs_func >= HARDWARE_DFS_V1) 
	{

#ifdef RTMP_RBUS_SUPPORT
		NewTimerCB_Radar(pAd);
#endif // defined (RTMP_RBUS_SUPPORT) //

	}
#endif // DFS_HARDWARE_SUPPORT //
#ifdef DFS_SOFTWARE_SUPPORT
	if (pAd->CommonCfg.dfs_func < HARDWARE_DFS_V1) 
	{

	if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
		TimerCB_Radar(pAd);
	}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.McuRadarCmd & CARRIER_DETECTION)
		TimerCB_Carrier(pAd);
#endif // CARRIER_DETECTION_SUPPORT //

}

#ifdef DFS_SUPPORT

#define CTS_WAIT_LOOP		500
#ifdef DFS_SOFTWARE_SUPPORT
static void TimerCB_Radar(PRTMP_ADAPTER pAd)
{
	ULONG Value;

	if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CARRIER_DETECTING)
	{
		pAd->CommonCfg.McuRadarTick++;
		return;
	}
	else
		pAd->CommonCfg.McuRadarTick++;
	

	if ((pAd->CommonCfg.McuRadarTick >= pAd->CommonCfg.McuRadarPeriod) && (pAd->CommonCfg.McuRadarState != DO_DETECTION))
	{
		/* Roger add to fix false detection(long pulse only) in the first 60 seconds */
		if (pAd->CommonCfg.W56_debug)
		{
			//ULONG time;
			RTMP_IO_READ32(pAd, PBF_LIFE_TIMER, &pAd->CommonCfg.W56_hw_2);
			RTMP_IO_READ32(pAd, CH_IDLE_STA, &Value);
			
			pAd->CommonCfg.W56_hw_sum += pAd->CommonCfg.W56_hw_2 - pAd->CommonCfg.W56_hw_1;
			pAd->CommonCfg.W56_hw_1 = pAd->CommonCfg.W56_hw_2;
			pAd->CommonCfg.W56_idx++;
			pAd->CommonCfg.W56_1s += Value;
			if ((pAd->CommonCfg.W56_idx % 100) == 0)
			{
				int i;
				//printk("~~~ %d   %d   ", pAd->CommonCfg.W56_idx, pAd->CommonCfg.W56_hw_sum - pAd->CommonCfg.W56_1s);
				if (pAd->CommonCfg.W56_hw_sum > pAd->CommonCfg.W56_1s)
					pAd->CommonCfg.W56_4s[(pAd->CommonCfg.W56_idx / 100) % 4] = pAd->CommonCfg.W56_hw_sum - pAd->CommonCfg.W56_1s;
				else
					pAd->CommonCfg.W56_4s[(pAd->CommonCfg.W56_idx / 100) % 4] = 0;
				Value = 0;
				for (i = 0; i < 4; i++)
					Value += pAd->CommonCfg.W56_4s[i];
				//printk("%d\n", Value);
				pAd->CommonCfg.W56_total = Value;
				pAd->CommonCfg.W56_hw_sum = 0;
				pAd->CommonCfg.W56_1s = 0;
			}
		}

		// Start of period
		pAd->CommonCfg.McuRadarTick = 0;
		pAd->CommonCfg.McuRadarDetectCount = 0;
		pAd->CommonCfg.McuRadarState = WAIT_CTS_BEING_SENT;
		pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_RADAR_DETECTING;


		if ((pAd->CommonCfg.McuRadarCtsProtect == 1)
#ifdef CARRIER_DETECTION_SUPPORT
			|| (isCarrierDetectExist(pAd))
#endif
			)
		{
			// no CTS protect
			pAd->CommonCfg.McuRadarState = DO_DETECTION;
			MCURadarDetect(pAd);			
		}
		else if (pAd->CommonCfg.McuRadarCtsProtect == 2 || pAd->CommonCfg.McuRadarCtsProtect == 3)
		{
			int i;
						
			if (pAd->CommonCfg.McuRadarCtsProtect == 3)
			{
				
				RTMP_IO_READ32(pAd, 0x7784, &Value);
				Value &= 0xffff00ff;
				RTMP_IO_WRITE32(pAd, 0x7784, Value);
				
				// pAd->CommonCfg.McuRadarCtsProtect == 3, need to kick CTS two times
				RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x40);
				for (i = 0; i < CTS_WAIT_LOOP; i++)
				{
					RTMP_IO_READ32(pAd, PBF_CTRL, &Value);
					if (!Value)
						break;
				}
				
				if (i >= CTS_WAIT_LOOP)
				{
					pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_CTS_KICKED;
					return;
				}
			}


			RTMP_IO_READ32(pAd, 0x7784, &Value);
			Value &= 0xffff00ff;
			Value |= (DFS_CTS_WCID << 8);
			RTMP_IO_WRITE32(pAd, 0x7784, Value);

			// kick CTS
			RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x40);

		}
		return;
	}

	if (pAd->CommonCfg.McuRadarState == WAIT_CTS_BEING_SENT)
	{
		// check event or timeout
		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_SENT)
		{
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_SENT;
			pAd->CommonCfg.McuRadarDetectCount = 1;
			pAd->CommonCfg.McuRadarState = DO_DETECTION;

			{
				UCHAR BBPR115;
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R115, &BBPR115);
			}

			return;
		}
		
		if (pAd->CommonCfg.McuRadarDetectCount >= (4-1)) // timeout 4ms, start from 0
		{
			pAd->CommonCfg.McuRadarState = FREE_FOR_TX; // timeout 5ms, give up
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_RADAR_DETECTING | RADAR_EVENT_CTS_KICKED);
			pAd->CommonCfg.McuRadarTick = pAd->CommonCfg.McuRadarPeriod;

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg._R65);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);

			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
			
		}

		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_KICKED)
		{
			int i;

			for (i = 0; i < CTS_WAIT_LOOP; i++)
			{
				RTMP_IO_READ32(pAd, PBF_CTRL, &Value);
				if (!Value)
					break;
			}
				
			if (i >= CTS_WAIT_LOOP)
			{
				pAd->CommonCfg.McuRadarDetectCount++;
				return;
			}
			
			RTMP_IO_READ32(pAd, 0x7784, &Value);
			Value &= 0xffff00ff;
			Value |= (DFS_CTS_WCID << 8) ;
			RTMP_IO_WRITE32(pAd, 0x7784, Value);

			// kick CTS
			RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x40);
			
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_KICKED;
		}

		
		pAd->CommonCfg.McuRadarDetectCount++;
	}
	else if (pAd->CommonCfg.McuRadarState == DO_DETECTION)
	{
		UCHAR BBPR115 = 0;
		UCHAR bbp = 0;


		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R115, &BBPR115);

		if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC) 
		{
			if  ((pAd->CommonCfg.McuRadarDetectCount % 2) == 0)
			{
#ifdef DFS_DEBUG
				if (pAd->CommonCfg.McuRadarDebug & RADAR_SIMULATE2) 
				{
					if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
					{
						RTMP_IO_WRITE32(pAd, 0x7100, 0x400);
						pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_WIDTH_RADAR;
					}
					pAd->CommonCfg.McuRadarDebug &= ~RADAR_SIMULATE2;

				}
#endif


				if (BBPR115 & 0x2)
				{
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R115, 0);

					Value = BBPR115 << 24;
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R116, &bbp);
					Value |= (bbp << 16);
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R117, &bbp);
					Value |= (bbp << 8);
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R118, &bbp);
					Value |= bbp;
					RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , Value);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x21);
				
					pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_WIDTH_RADAR;
				}
				else
				{
					RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , 0);
				}
			}
		}
		else if ((pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56) && ((pAd->CommonCfg.McuRadarDetectCount % 2) == 0))
		{
			if (BBPR115 & 0x2)
			{

				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R115, 0);

				Value = BBPR115 << 24;
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R116, &bbp);
				Value |= (bbp << 16);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R117, &bbp);
				Value |= (bbp << 8);
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R118, &bbp);
				Value |= bbp;
				RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , Value);
			
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x21);
			
				pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_WIDTH_RADAR;
				
			}
			else
			{
				RTMP_IO_WRITE32(pAd, 0x7100 + (4 * (pAd->CommonCfg.McuRadarDetectCount / 2 - 1)) , 0);
			}
						
		}
		
		if (pAd->CommonCfg.McuRadarDetectCount >= (pAd->CommonCfg.McuRadarDetectPeriod - 1))
		{
			if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_WIDTH_RADAR)
			{
				pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_WIDTH_RADAR);
				RadarSMDetect(pAd, RADAR_WIDTH);
			}
			
#ifdef DFS_DEBUG
			if (pAd->CommonCfg.McuRadarDebug & RADAR_SIMULATE) 
			{
				if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
				{
					RadarSMDetect(pAd, RADAR_PULSE);
				}
			}
#endif // DFS_DEBUG //
			

			if (BBPR115 & 0x1)
			{
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R115, 0);

				// pulse radar detected
				if (pAd->CommonCfg.McuRadarCmd & RADAR_DETECTION)
				{
					RadarSMDetect(pAd, RADAR_PULSE);
				}
			}


			pAd->CommonCfg.McuRadarState = FREE_FOR_TX;
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_RADAR_DETECTING);

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg._R65);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);

#ifdef DFS_DEBUG
#ifdef RTMP_RBUS_SUPPORT

			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				RTMP_SYS_IO_WRITE32(0xa0300630, 0x2000);
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		}
		pAd->CommonCfg.McuRadarDetectCount++;

	}
	else // FREE_FOR_TX
	{
		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_SENT)
		{

			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_SENT;
			pAd->CommonCfg.McuRadarDetectCount = 1;

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg._R65);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x01);


#ifdef DFS_DEBUG
#ifdef RTMP_RBUS_SUPPORT
			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				RTMP_SYS_IO_WRITE32(0xa0300630, 0x2000);
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
		}
	}	
}
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //

#ifdef CARRIER_DETECTION_SUPPORT

static void TimerCB_Carrier(PRTMP_ADAPTER pAd)
{
	ULONG Value;

	if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_RADAR_DETECTING)
	{
		pAd->CommonCfg.McuCarrierTick++;
		return;
	}
	else
		pAd->CommonCfg.McuCarrierTick++;


	if (pAd->CommonCfg.McuCarrierTick >= pAd->CommonCfg.McuCarrierPeriod)
	{
		// Start of period
		pAd->CommonCfg.McuCarrierTick = 0;
		pAd->CommonCfg.McuCarrierDetectCount = 0;
		pAd->CommonCfg.McuCarrierState = WAIT_CTS_BEING_SENT;
		pAd->CommonCfg.McuRadarEvent |= RADAR_EVENT_CARRIER_DETECTING;

		if ((pAd->CommonCfg.McuCarrierCtsProtect == 1)
			|| (isCarrierDetectExist(pAd))
			)
		{
			// no CTS protect
			pAd->CommonCfg.McuCarrierState = DO_DETECTION;
			MCURadarDetect(pAd);
		}
		else if (pAd->CommonCfg.McuCarrierCtsProtect == 2)
		{
			// kick CTS
			RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x80);
		}
		return;
	}

	if (pAd->CommonCfg.McuCarrierState == WAIT_CTS_BEING_SENT)
	{
		// check event or timeout
		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_CARRIER_SENT)
		{
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_CARRIER_SENT;
			pAd->CommonCfg.McuCarrierDetectCount = 1;
			pAd->CommonCfg.McuCarrierState = DO_DETECTION;

			return;
		}
		
		if (pAd->CommonCfg.McuCarrierDetectCount >= (5-1)) // timeout 5ms, start from 0
		{
			pAd->CommonCfg.McuCarrierState = FREE_FOR_TX; // timeout 5ms, give up
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_CARRIER_DETECTING);

		}
		
		pAd->CommonCfg.McuCarrierDetectCount++;
	}
	else if (pAd->CommonCfg.McuCarrierState == DO_DETECTION)
	{
		if (pAd->CommonCfg.McuCarrierDetectCount >= (pAd->CommonCfg.McuCarrierDetectPeriod - 1))
		{
			UCHAR BBPR115;

			BBP_IO_READ8_BY_REG_ID(pAd, BBP_R115, &BBPR115);

			CarrierDetectionCheck(pAd);

			pAd->CommonCfg.McuCarrierState = FREE_FOR_TX;
			pAd->CommonCfg.McuRadarEvent &= ~(RADAR_EVENT_CARRIER_DETECTING);

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP

				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, pAd->CommonCfg._R69);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, pAd->CommonCfg._R70);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, pAd->CommonCfg._R73);

#ifdef DFS_DEBUG
#ifdef RTMP_RBUS_SUPPORT

			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				RTMP_SYS_IO_WRITE32(0xa0300630, 0x2000);
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

			

		}
		pAd->CommonCfg.McuCarrierDetectCount++;
	}
	else // FREE_FOR_TX
	{
		if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CTS_CARRIER_SENT)
		{
			pAd->CommonCfg.McuRadarEvent &= ~RADAR_EVENT_CTS_CARRIER_SENT;
			pAd->CommonCfg.McuCarrierDetectCount = 1;

			RTMP_IO_WRITE32(pAd, TX_RTS_CFG, pAd->CommonCfg.OldRtsRetryLimit);
			// Restore BBP
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg._R66);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, pAd->CommonCfg._R69);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, pAd->CommonCfg._R70);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, pAd->CommonCfg._R73);


#ifdef DFS_DEBUG
#ifdef RTMP_RBUS_SUPPORT

			if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
				RTMP_SYS_IO_WRITE32(0xa0300630, 0x2000);
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_DEBUG //
			// EnableNormalTx
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value |= 0x04;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
		}
	}	
}
#endif // CARRIER_DETECTION_SUPPORT //

void MCURadarDetect(PRTMP_ADAPTER pAd)
{
	ULONG Value;
	UCHAR bbp = 0;
	
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R113, &bbp);
	if (bbp & 0x20)
		return;

	//DisableNormalTx
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0);
	{
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
	}
	Value &= (ULONG)~(0x04);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
				
#ifdef DFS_DEBUG
#ifdef RTMP_RBUS_SUPPORT

	if ((pAd->CommonCfg.McuRadarDebug & RADAR_GPIO_DEBUG))
		if (pAd->CommonCfg.McuRadarCtsProtect != 1)
			RTMP_SYS_IO_WRITE32(0xa030062c, 0x2000);
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_DEBUG //


	RTMP_IO_READ32(pAd, TX_RTS_CFG, &pAd->CommonCfg.OldRtsRetryLimit);
	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, 0);
	
	//change BBP now
	if (pAd->CommonCfg.McuRadarEvent & RADAR_EVENT_CARRIER_DETECTING)
	{
		UCHAR BBPR4 = 0;
		
		// Clear False CCA
		RTMP_IO_READ32(pAd, RX_STA_CNT1, &Value);
		
		// save old value
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &pAd->CommonCfg._R66);
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R69, &pAd->CommonCfg._R69);
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R70, &pAd->CommonCfg._R70);
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R73, &pAd->CommonCfg._R73);

		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);
		if (BBPR4 & 0x18) // BW 40
		{
			if (pAd->CommonCfg.Channel > 14)
			{
				// BW 40, A band
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x50);
			}
			else
			{
				// BW 40, G band
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x40);
			}
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x30);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x30);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x08);
		}
		else // BW 20
		{
			if (pAd->CommonCfg.Channel > 14)
			{
				// BW 20, A band
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x60);
			}
			else
			{
				// BW 20, G band
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, 0x30);
			}
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x28);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x28);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x08);
		}
		
	}
	else // RADAR_DETECTION
	{
		// save old value				
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &pAd->CommonCfg._R65);
		BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &pAd->CommonCfg._R66);

		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg.R65);
			BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->CommonCfg.R66);
		BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R113, 0x21);
	}		
	return;
}

#ifdef DFS_SUPPORT
#ifdef DFS_HARDWARE_SUPPORT

#define NEW_DFS_WATCH_DOG_TIME		1 // note that carrier detection also need timer interrupt hook

#define PERIOD_MATCH(a, b, c)			((a >= b)? ((a-b) <= c):((b-a) <= c))
#define MATCH_OR_DOUBLE(a,b,c)			( PERIOD_MATCH(a,b,c)? 1 : ((a>b)? PERIOD_MATCH((a>>1),b,c):PERIOD_MATCH(a,(b>>1),c)) )
#define ENTRY_PLUS(a, b, c)				(((a+b) < c)? (a+b) : (a+b-c))
#define ENTRY_MINUS(a, b, c)			((a >= b)? (a - b) : (a+c-b))
#define MAX_PROCESS_ENTRY 				16

#define IS_FCC_RADAR_1(HT_BW, T)			(((HT_BW)? ((T > 57120) && (T < 57160)) : (T > 28560) && (T < 28580)))
#define IS_W53_RADAR_2(HT_BW, T)			(((HT_BW)? ((T > 153820) && (T < 153872)) : (T > 76910) && (T < 76936)))
#define IS_W56_RADAR_3(HT_BW, T)			(((HT_BW)? ((T > 159900) && (T < 160100)) : (T > 79950) && (T < 80050)))


#define NEW_DFS_FCC		0x1 // include Japan
#define NEW_DFS_EU		0x2
#define NEW_DFS_JAP		0x4
#define NEW_DFS_JAP_W53		0x8
#define NEW_DFS_END		0xff
#define MAX_VALID_RADAR_W	5
#define MAX_VALID_RADAR_T	5


typedef struct _NewDFSValidRadar
{
	USHORT type;
	USHORT channel; // bit map
	USHORT WLow;
	USHORT WHigh;
	USHORT W[MAX_VALID_RADAR_W];
	USHORT WMargin;
	ULONG TLow;
	ULONG THigh;
	ULONG T[MAX_VALID_RADAR_T];
	USHORT TMargin;
}NewDFSValidRadar, *pNewDFSValidRadar;

#ifdef DFS_DEBUG

typedef struct _NewDFSDebugResult
{
	char delta_delay_shift;
	char EL_shift;
	char EH_shift;
	char WL_shift;
	char WH_shift;
	ULONG hit_time;
	ULONG false_time;
}NewDFSDebugResult, *pNewDFSDebugResult;

NewDFSDebugResult TestResult[1000];

#endif


NewDFSValidRadar NewDFSValidTable[] = 
{
	// FCC-1  && (Japan W53 Radar 1 / W56 Radar 2)
	{
	(NEW_DFS_FCC | NEW_DFS_JAP | NEW_DFS_JAP_W53),
	//3,
	7,
	//0, 0, 
	//{20, 0, 0, 0, 0},
	10, 1000,
	{0, 0, 0, 0, 0},
	4,
	0, 0, 
	{28570 - 70, 0, 0, 0, 0},
	150
	},
	// FCC-2
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	//3,
	7,
	//20, 100, 
	13, 1000,
	{0, 0, 0, 0, 0},
	1,
	3000, 4600 - 20,
	{0, 0, 0, 0, 0},
	25
	},
	// FCC-3 & FCC-4
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	//6,
	7,
	//120, 200, FCC-3 
	//220, 400, FCC-4
	100, 1500, 
	{0, 0, 0, 0, 0},
	1,
	4000, 10000 - 40, 
	{0, 0, 0, 0, 0},
	60
	},
	// FCC-6
	{
	(NEW_DFS_FCC | NEW_DFS_JAP),
	//3,
	7,
	//0, 0, 
	//{20, 0, 0, 0, 0},
	12, 1000,
	{0, 0, 0, 0, 0},
	1,
	0, 0, 
	{6660-10, 0, 0, 0, 0},
	35
	},
	// Japan W53 Radar 2
	{
	NEW_DFS_JAP_W53,
	7,
	//0, 0, 
	//{50, 0, 0, 0, 0},
	40, 1000, 
	{0, 0, 0, 0, 0},
	1,
	0, 0, 
	{76923 - 30, 0, 0, 0, 0},
	180
	},
	// Japan W56 Radar 1
	{
	NEW_DFS_JAP,
	7,
	//0, 0, 
	//{10, 0, 0, 0, 0},
	5, 500, 
	{0, 0, 0, 0, 0},
	2,
	0, 0, 
	{27777 - 30, 0, 0, 0, 0},
	70
	},
	// Japan W56 Radar 3
	{
	NEW_DFS_JAP,
	7,
	//0, 0, 
	//{40, 0, 0, 0, 0},
	30, 1000, 
	{0, 0, 0, 0, 0},
	1,
	0, 0, 
	{80000 - 50, 0, 0, 0, 0},
	200
	},

// CE Staggered radar

	{
	//	EN-1
	//	width	0.8 - 5 us
	//	PRF		200 - 1000 Hz
	//	PRI		5000 - 1000 us	(T: 20000 - 100000)
	//	
	NEW_DFS_EU,
	0xf,
	10, 1000, 
	{0, 0, 0, 0, 0},
	1,
	20000-15, 100000-70, 
	{0, 0, 0, 0, 0},
	120
	},
	//	EN-2
	//	width	0.8 - 15 us
	//	PRF		200 - 1600 Hz
	//	PRI		5000 - 625 us	(T: 12500 - 100000)
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
	
	//	EN-3
	//	width	0.8 - 15 us
	//	PRF		2300 - 4000 Hz
	//	PRI		434 - 250 us	(T: 5000 - 8695)
	//
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
	//	EN-4
	//	width	20 - 30 us
	//	PRF		2000 - 4000 Hz
	//	PRI		500 - 250 us	(T: 5000 - 10000)
	//	Note : with Chirp Modulation +- 2,5Mhz
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
	//	EN-5
	//	width	0.8 - 2 us
	//	PRF		300 - 400 Hz
	//	PRI		3333 - 2500 us	(T: 50000 - 66666)
	//	Staggered PRF, 20 - 50 pps
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
	//	EN-6
	//	width	0.8 - 2 us
	//	PRF		400 - 1200 Hz
	//	PRI		2500 - 833 us	(T: 16666 - 50000)
	//	Staggered PRF, 80 - 400 pps
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

static void dfs_sw_init(PRTMP_ADAPTER pAd);

#ifdef DFS_1_SUPPORT 
typedef struct _NewDFSProgParam
{
	UCHAR channel;
	UCHAR mode;
	USHORT avgLen; // M = Average length
	USHORT ELow;
	USHORT EHigh;
	USHORT WLow;
	USHORT WHigh;
	USHORT EpsilonW;
	ULONG TLow;
	ULONG THigh;
	USHORT EpsilonT;
}NewDFSProgParam, *pNewDFSProgParam;

typedef struct _NewDFSTable
{
	USHORT type;
	NewDFSProgParam entry[NEW_DFS_MAX_CHANNEL];
}NewDFSTable, *pNewDFSTable;

static NewDFSTable NewDFSTable1[] = 
{
	{
		NEW_DFS_FCC,
		{
		{0, 0,  20,  18,  26,   10, 2000, 1,  3000, 30000, 5},
		{1, 0, 100, 100, 140,   12, 2000, 3,  2500, 30000, 5},
		{2, 0, 100,  60, 120,   12, 2000, 3,  3000, 30000, 5},
		{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200},
		}
	},
	{
		NEW_DFS_EU,
		{
		{0, 0, 12,	16,	20,	10,	1000,	1, 4900, 101000, 5},
		{1, 0, 70,	80,110, 20, 2000,	3, 4900, 101000, 5},
		{2, 0, 80,	60, 120, 20, 3000,	3, 4900, 101000, 5},
		{3, 3, 250,	20, 200, 300,4000,	50, 4900, 10100, 200},
		}
	},
	{
		NEW_DFS_JAP,
		{
		{0, 0, 10,  10,  16,   3,  2000,  1,  2990, 85000, 10},
		{1, 0, 70, 80, 110,   20, 2000, 3,  2990, 85000, 10},
		{2, 0, 80,  60, 120,   20, 2000, 3,  2990, 85000, 10},
		{3, 2, 200,  60, 160, 320, 2000,  50, 15000, 45000, 200},
		}
	},
	{
		NEW_DFS_JAP_W53,
		{
		{0, 0, 12,  10,  16,   12,  2000,  1,  28000, 85000, 10},
		{1, 0, 32, 30, 40,   20, 2000, 3,  28000, 85000, 10},
		{2, 0, 80,  60, 120,   20, 2000, 3,  28000, 85000, 10},
		{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200},
		}
	},

};

#define NEW_DFS_BANDWITH_MONITOR_TIME (NEW_DFS_CHECK_TIME / NEW_DFS_CHECK_TIME_TASKLET)
#define NEW_DFS_CHECK_TIME			300
#define NEW_DFS_CHECK_TIME_TASKLET		2
void schedule_dfs_task(PRTMP_ADAPTER pAd);

#ifndef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
 VOID NewTimerCB_Radar(
 	IN PRTMP_ADAPTER pAd)
#endif // RTMP_RBUS_SUPPORT  //
{
	pNewDFSTable pDFS2Table;
	pNewDFSValidRadar pDFSValidRadar;
	UCHAR channel;
	UCHAR radarDeclared=0;
	INT i, j;
	UCHAR bbp_r140, bbp_r141;
	UCHAR bbp_r156, bbp_r157, bbp_r158, bbp_r159;
	UCHAR bbp_r160, bbp_r161, bbp_r162, bbp_r163;
	
	if (pAd->CommonCfg.PollTime == 0)
		return;
	
	if (pAd->CommonCfg.RadarTimeStampLow++ == 0xffffffff)
		pAd->CommonCfg.RadarTimeStampHigh++;
		if ((pAd->CommonCfg.RadarTimeStampLow & 0x1ff) == 0)
			{
				int busy_delta, idle_delta;
				RTMP_IO_READ32(pAd, CH_IDLE_STA, &pAd->CommonCfg.idle_time);
				RTMP_IO_READ32(pAd, CH_BUSY_STA, &pAd->CommonCfg.busy_time);


  				
				pAd->CommonCfg.rssi = pAd->ApCfg.RssiSample.AvgRssi0; // only RSSI 0 is cared

				busy_delta = pAd->CommonCfg.busy_time - pAd->CommonCfg.ch_busy_sta[pAd->CommonCfg.ch_busy_sta_index];
				idle_delta = pAd->CommonCfg.idle_time - pAd->CommonCfg.ch_idle_sta[pAd->CommonCfg.ch_busy_sta_index];

				if (busy_delta < 0)
				{			
					busy_delta = ~busy_delta;
					busy_delta = (busy_delta >> CH_BUSY_SAMPLE_POWER) ;
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
				
				pAd->CommonCfg.ch_busy_sum += busy_delta;
				pAd->CommonCfg.ch_idle_sum += idle_delta;
				
				// not sure if this is necessary??
				if (pAd->CommonCfg.ch_busy_sum < 0)
					pAd->CommonCfg.ch_busy_sum = 0;
				if (pAd->CommonCfg.ch_idle_sum < 0)
					pAd->CommonCfg.ch_idle_sum = 0;
				

				pAd->CommonCfg.ch_busy_sta[pAd->CommonCfg.ch_busy_sta_index] = pAd->CommonCfg.busy_time;
				pAd->CommonCfg.ch_idle_sta[pAd->CommonCfg.ch_busy_sta_index] = pAd->CommonCfg.idle_time;
				

				pAd->CommonCfg.ch_busy_sta_index++;
				pAd->CommonCfg.ch_busy_sta_index &= CH_BUSY_MASK;


				if ((pAd->CommonCfg.ch_idle_sum >> pAd->CommonCfg.ch_busy_idle_ratio) < pAd->CommonCfg.ch_busy_sum )
				{
					if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_DONT_CHECK_BUSY))
						pAd->CommonCfg.ch_busy = 1;
				}
				else 
				{
					if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_DONT_CHECK_RSSI))
					{
						if ((pAd->ApCfg.RssiSample.AvgRssi0) && (pAd->ApCfg.RssiSample.AvgRssi0 > pAd->CommonCfg.DfsRssiHigh))
							pAd->CommonCfg.ch_busy = 2;
						else if ((pAd->ApCfg.RssiSample.AvgRssi0) && (pAd->ApCfg.RssiSample.AvgRssi0 < pAd->CommonCfg.DfsRssiLow))
							pAd->CommonCfg.ch_busy = 3;
						else
					pAd->CommonCfg.ch_busy = 0;
						}
					}
					if (pAd->CommonCfg.print_ch_busy_sta)
					printk ("%d %d %d %d\n", pAd->CommonCfg.ch_idle_sum, pAd->CommonCfg.ch_busy_sum, pAd->ApCfg.RssiSample.AvgRssi0, pAd->CommonCfg.ch_busy);

			}

			if ( (pAd->CommonCfg.McuRadarTick++ >= pAd->CommonCfg.PollTime) && (!pAd->CommonCfg.ch_busy))
			{
				//int k, count, limit = ((pAd->CommonCfg.RadarDetect.RDDurRegion != CE)? 384: 288);
				int k, count, limit = 384;
				UCHAR BBPR127, BBPR126, LastBBPR127 = 0xff;
				ULONG time = 0;
				USHORT width = 0;
				UCHAR id = 0;
				UCHAR alignment = FALSE;
				pAd->CommonCfg.McuRadarTick = 0;				
				pAd->CommonCfg.dfs_w_counter++;

				// disable debug mode to read debug port of channel 3
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R126, &BBPR126);
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, (BBPR126 & 0xfe));
				count = 0;
				for (k = 0; k < limit; k++)
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);
					
					if (count != 0 && alignment == FALSE)
					{
						LastBBPR127 = BBPR127;
						count++;
						if (count >= 6)
							count = 0;
						continue;
					}
					switch(count)
					{
					case 0:
						width = 0;
						time = 0;
						id = BBPR127;
						if ((id > 3) && (id != 0xff))
						{
							alignment = FALSE;
							LastBBPR127 = BBPR127;
						}
						else
						{
							if ((LastBBPR127 & 0xf8) != 0)
							{
								LastBBPR127 = BBPR127;
								alignment = FALSE;
							}
							else
							{
								if ((LastBBPR127 != 0) && (id == 0))
								{
									LastBBPR127 = BBPR127;
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
							alignment = FALSE;
							LastBBPR127 = BBPR127;
							break;
						}						
						
						width |= (BBPR127 & 0x7) << 10;
						

						// the width in 40Mhz mode will be twice of width in 20Mhz mode
						// if mis-read the debug event, the first bytes will be 00, the second byte may be id 00~03, so 
						// 0x3fff00 will cover this case.
						if ((id == 0x3) && (pAd->CommonCfg.RadarDetect.RDDurRegion != CE) && (pAd->CommonCfg.MCURadarRegion != NEW_DFS_JAP_W53) && (width > 1000) && (time & 0x3fff00)) // FCC && Japan only
						{
							// in case mis-read debug event, the continue 2 entries may have exactly the same values.
							if (pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].timestamp != time)
							{
								if (!(((time & 0xff) == 0) || ((time & 0xff00) == 0) || (((time & 0x3f0000) == 0x3f0000) && ((width & 0x3) == 0x3)) || ((width & 0x3fc) == 0x3fc) || ((width & 0x3fc) == 0)))
								{
									if (pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_EVENT)
									{
										printk("counter = %d  ", (unsigned int)pAd->CommonCfg.dfs_w_counter);
										printk("time = %d (%x) ", (unsigned int)time, (unsigned int)time);
										printk("width = %d\n", (unsigned int)width);
									}
									
									pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].counter = pAd->CommonCfg.dfs_w_counter;
									pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].timestamp = time;
									pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].width = width;
								
									if (pAd->CommonCfg.fcc_5_last_idx != pAd->CommonCfg.fcc_5_idx)
									{
										if (PERIOD_MATCH(pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].counter, pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].counter, 3000))
										{
											// long pulse basicly have different length, if the width is exactly the same, it's probably mis-read debug event entries
											if ((pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].width != pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].width) && (time & 0x3fc000))
											{
												if (((time & 0xff) != 0) || ((pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].timestamp & 0xff) != 0))
												{
													printk("radar detected!!! id == %d\n", id);
													radarDeclared = 1;
												}
											}
										}
									}

									pAd->CommonCfg.fcc_5_last_idx = pAd->CommonCfg.fcc_5_idx;
									pAd->CommonCfg.fcc_5_idx++;
									if (pAd->CommonCfg.fcc_5_idx >= NEW_DFS_FCC_5_ENT_NUM)
										pAd->CommonCfg.fcc_5_idx = 0;
								}
							}
							else
							{
								id = 0xff;
								alignment = TRUE;
								break;
							}
							
						}

						if (pAd->CommonCfg.use_tasklet)
						{
							//if (id <= 0x2) // && (id >= 0)
							if ((id < 0x3) && (time & 0x3fff00)) // && (id >= 0)
							{
								if (time != pAd->CommonCfg.DFS_W[id][((pAd->CommonCfg.dfs_w_idx[id] == 0)? (NEW_DFS_DBG_PORT_ENT_NUM-1):(pAd->CommonCfg.dfs_w_idx[id] - 1))].timestamp)
								{
									if (!(((time & 0xff) == 0) || ((time & 0xff00) == 0) || (((time & 0x3f0000) == 0) && ((width & 0x3) == 0x3) || ((width & 0x3fc) == 0x3fc) || ((width & 0x3fc) == 0))))
									{
										pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].counter = pAd->CommonCfg.dfs_w_counter;
										pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].timestamp = time;
										pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].width = width;
                        	
										pAd->CommonCfg.dfs_w_last_idx[id] = pAd->CommonCfg.dfs_w_idx[id];
										pAd->CommonCfg.dfs_w_idx[id]++;
										if (pAd->CommonCfg.dfs_w_idx[id] >= NEW_DFS_DBG_PORT_ENT_NUM)
											pAd->CommonCfg.dfs_w_idx[id] = 0;
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
						pAd->CommonCfg.TotalEntries[id]++;						
#endif 
						break;

					}
					if ((id == 0xff) && (alignment == TRUE))
					{
						break;
					}
					
#ifdef DFS_DEBUG
					if (pAd->CommonCfg.DebugPortPrint == 1 && (k == 0))
					{
						pAd->CommonCfg.DebugPortPrint = 2;
					}

					if ((pAd->CommonCfg.DebugPortPrint == 2))
					{
						pAd->CommonCfg.DebugPort[k] = BBPR127;
					}
#endif 

					if (alignment != FALSE)
					{
						count++;
						if (count >= 6)
							count = 0;
					}

				
				
				} // for (k = 0; k < limit; k++)
			
#ifdef DFS_DEBUG			
				if (pAd->CommonCfg.DebugPortPrint == 2)
				{
					pAd->CommonCfg.DebugPortPrint = 3;
				}
#endif


#ifdef DFS_DEBUG
				if (pAd->CommonCfg.BBP127Repeat)
				{
					for (k = 0; k < pAd->CommonCfg.BBP127Repeat; k++)
						BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);
					
					pAd->CommonCfg.BBP127Repeat = 0;
				}
				else
#endif
				// read to several times for alignment
				for (k = count; k < 5; k++)
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);
				}
			
				
				// enable debug mode
				if (pAd->CommonCfg.dfs_w_counter & 1)
				{
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 3);
				}
				else
				{
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 7);
				}

				if (pAd->CommonCfg.use_tasklet)
				{
					// set hw_idx
					pAd->CommonCfg.hw_idx[0] = pAd->CommonCfg.dfs_w_idx[0];
					pAd->CommonCfg.hw_idx[1] = pAd->CommonCfg.dfs_w_idx[1];
					pAd->CommonCfg.hw_idx[2] = pAd->CommonCfg.dfs_w_idx[2];
					pAd->CommonCfg.hw_idx[3] = pAd->CommonCfg.dfs_w_idx[3];
					schedule_dfs_task(pAd);
				}

			}

		
		
		// Poll Status register
		BBP_IO_READ8_BY_REG_ID(pAd, 141, &bbp_r141);
		channel = bbp_r141 & 0xf;

		// Hardware Detection
		if (channel)
		{
			UCHAR WMatched, TMatched;
			USHORT W, W1;
			ULONG T, T1;

			DBGPRINT(RT_DEBUG_TRACE, ("NewTimerCB_Radar, bbp_r141 = 0x%x, channel bit= 0x%x\n", bbp_r141, channel));
			
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
				bbp_r140 = 0x80 | pAd->CommonCfg.ChEnable;
			else
				bbp_r140 = 0 | pAd->CommonCfg.ChEnable;

			{
			
				for (j = 0;j < NEW_DFS_MAX_CHANNEL; j++)
				{
					if (channel & (0x1 << j))
					{

						// select Channel first
						BBP_IO_WRITE8_BY_REG_ID(pAd, 140, bbp_r140 | (j << 4));
						DBGPRINT(RT_DEBUG_TRACE, ("Select Channel = 0x%x\n", bbp_r140 | (j << 4)));
						BBP_IO_READ8_BY_REG_ID(pAd, 160, &bbp_r160);
						BBP_IO_READ8_BY_REG_ID(pAd, 161, &bbp_r161);
						BBP_IO_READ8_BY_REG_ID(pAd, 162, &bbp_r162);
						BBP_IO_READ8_BY_REG_ID(pAd, 163, &bbp_r163);
						//printk("... 160 ~ 163  %x %x %x %x\n", bbp_r160, bbp_r161, bbp_r162, bbp_r163);
						BBP_IO_READ8_BY_REG_ID(pAd, 156, &bbp_r156);
						BBP_IO_READ8_BY_REG_ID(pAd, 157, &bbp_r157);
						BBP_IO_READ8_BY_REG_ID(pAd, 158, &bbp_r158);
						BBP_IO_READ8_BY_REG_ID(pAd, 159, &bbp_r159);
						//printk("... 156 ~ 159  %x %x %x %x\n", bbp_r156, bbp_r157, bbp_r158, bbp_r159);


						pDFSValidRadar = &NewDFSValidTable[0];

						while (pDFSValidRadar->type != NEW_DFS_END)
						{
							if ((pDFSValidRadar->type & pAd->CommonCfg.MCURadarRegion) == 0)
							{
								pDFSValidRadar++;
								continue;
							}

							if (! (pDFSValidRadar->channel & channel))
							{
								pDFSValidRadar++;
								continue;
							}

							if (pAd->CommonCfg.MCURadarRegion == NEW_DFS_FCC)
								pDFS2Table = &NewDFSTable1[0];
							else if (pAd->CommonCfg.MCURadarRegion == NEW_DFS_EU)
							{
								pDFS2Table = &NewDFSTable1[1];
							}
							else // Japan
							{
								if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
									pDFS2Table = &NewDFSTable1[3];
								else
									pDFS2Table = &NewDFSTable1[2];
							}

							
								
							//printk("Table type = %d, Channel bit map = 0x%x\n", pDFSValidRadar->type, pDFSValidRadar->channel);

							W = 0;
							T = 0;
							T |= bbp_r156 << 8;
							T |= bbp_r157;
							T |= (bbp_r158 & 0x80) << 9;
			
							W |= (bbp_r158 & 0xf) << 8;
							W |= bbp_r159;

							if ( (pDFS2Table->entry[j].mode == 1) || (pDFS2Table->entry[j].mode == 2) || (pDFS2Table->entry[j].mode == 6) )
							{
								W1 = 0;
								T1 = 0;
								T1 |= bbp_r160 << 8;
								T1 |= bbp_r161;
								T1 |= (bbp_r162 & 0x80) << 9;
			
								W1 |= (bbp_r162 & 0xf) << 8;
								W1 |= bbp_r163;
							}
							

							WMatched = 0;
							// check valid Radar
							if (pDFSValidRadar->WLow)
							{
								if ( (W > (pDFSValidRadar->WLow - pDFSValidRadar->WMargin)) && 
								     (W < (pDFSValidRadar->WHigh + pDFSValidRadar->WMargin)) )
								{
									WMatched = 1;
								}
								
							}
							else
							{
								i = 0;
								while( (i < MAX_VALID_RADAR_W) && (pDFSValidRadar->W[i] != 0) )
								{
									if ( (W > (pDFSValidRadar->W[i] - pDFSValidRadar->WMargin)) &&
									     (W < (pDFSValidRadar->W[i] + pDFSValidRadar->WMargin)) )
									{
										WMatched = 1;
										break;
									}
									
									i++;
								}
							}
					
							//printk("WMatched == %d   ", WMatched);
							TMatched = 0;
							if (pDFSValidRadar->TLow)
							{
								if ( (T > (pDFSValidRadar->TLow - pDFSValidRadar->TMargin)) && 
								     (T < (pDFSValidRadar->THigh + pDFSValidRadar->TMargin)) )
								{
									TMatched = 1;
								}
							}
							else
							{
								i = 0;
								while( (i < MAX_VALID_RADAR_T) && (pDFSValidRadar->T[i] != 0) )
								{
									if ( (T > (pDFSValidRadar->T[i] - pDFSValidRadar->TMargin)) &&
									     (T < (pDFSValidRadar->T[i] + pDFSValidRadar->TMargin)) )
									{
										TMatched = 1;
										break;
									}
									
									i++;
								}
							}
							//printk("TMatched == %d\n", TMatched);

#ifdef DFS_DEBUG
							if (pAd->CommonCfg.DfsDebug.start == 2)
							{
								if ((T > (pAd->CommonCfg.DfsDebug.T_expected - pAd->CommonCfg.DfsDebug.T_margin)) && (T < (pAd->CommonCfg.DfsDebug.T_expected + pAd->CommonCfg.DfsDebug.T_margin)))
								{
									if (pAd->CommonCfg.DfsDebug.idx < 1000)
									{
										TestResult[pAd->CommonCfg.DfsDebug.idx].hit_time ++;
									}
								}
								else
								{
									if (pAd->CommonCfg.DfsDebug.idx < 1000)
									{
										TestResult[pAd->CommonCfg.DfsDebug.idx].false_time ++;
									}
								}
							}

#endif

						if ((TMatched == 1) && (WMatched == 1) && (!pAd->CommonCfg.hw_dfs_disabled))
						{
							// found that seem all false detection are between 3000 ~ 10000
							//if (pAd->CommonCfg.ch_busy == 0)
							if (1)
							{
								if (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
								{
									printk ("%d W = %d, T= %d (%d)\n", (unsigned int)jiffies, (unsigned int)W, (unsigned int)T, (unsigned int)j);
									printk ("radar detected!!\n");
									radarDeclared = 1;
								}
								else
								{
									if (pAd->CommonCfg.RadarDetect.RDDurRegion != CE)
									{
										printk ("%d W = %d, T= %d (%d)\n", (unsigned int)jiffies, (unsigned int)W, (unsigned int)T, (unsigned int)j);
										radarDeclared = 1;
									}
									// false detection observation :
									// almost all false detection is ch0 when rssi is not too high,
									// almost all false detection of ch0 have width < 30
									else if ((j == 0) && (W < 30))
									{
										;
									}
									else 
									{
										printk ("%d W = %d, T= %d (%d)\n", (unsigned int)jiffies, (unsigned int)W, (unsigned int)T, (unsigned int)j);
										radarDeclared = 1;
									}
									
								}
								
							}
#ifdef DFS_DEBUG
							else
							{
								DBGPRINT(RT_DEBUG_TRACE, ("FDF: ch %d, w %d, t %d jf %d\n", (unsigned int)j, (unsigned int)W, (unsigned int)T, (unsigned int)jiffies));
								DBGPRINT(RT_DEBUG_TRACE, ("%d:%d,%d (%d)\n",(unsigned int)pAd->CommonCfg.ch_idle_sum, (unsigned int)pAd->CommonCfg.ch_busy_sum, (unsigned int)pAd->CommonCfg.rssi, (unsigned int)pAd->CommonCfg.ch_busy));
							}
#endif


							if (j < 4)
							{
								pAd->CommonCfg.re_check_jiffies[j] = jiffies;
								pAd->CommonCfg.re_check_Width[j] = W;
								pAd->CommonCfg.re_check_Period[j] = T;
							}

							if (pAd->CommonCfg.RadarReEnable == 0)
							{
								BBP_IO_WRITE8_BY_REG_ID(pAd, 140, bbp_r140);
								printk("Disable detecting : write 140 = 0x%x\n", bbp_r140);
								NewRadarDetectionStop(pAd);
								
							}
						}
						else if ((TMatched == 1) && (!pAd->CommonCfg.hw_dfs_disabled))
						{
							//if (pAd->CommonCfg.ch_busy == 0)
							if (1)
							{
								if (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
								{
									printk ("%d W = %d, T= %d (%d)\n", (unsigned int)jiffies, (unsigned int)W, (unsigned int)T, (unsigned int)j);
									printk ("radar detected!!\n");
									radarDeclared = 1;
								}
								else
								{
									if (pAd->CommonCfg.RadarDetect.RDDurRegion != CE)
									{
										printk ("%d W = %d, T= %d (%d)\n", (unsigned int)jiffies, (unsigned int)W, (unsigned int)T, (unsigned int)j);
										radarDeclared = 1;
									}
									// false detection observation :
									// almost all false detection is ch0 when rssi is not too high,
									// almost all false detection of ch0 have width < 30
									else if ((j == 0) && (W < 30))
									{
										;
									}
									else 
									{
										printk ("%d W = %d, T= %d (%d)\n", (unsigned int)jiffies, (unsigned int)W, (unsigned int)T, (unsigned int)j);
										radarDeclared = 1;
									}
									
								}
								

								
							}
#ifdef DFS_DEBUG
							else
							{
								DBGPRINT(RT_DEBUG_TRACE, ("FDF: ch %d, w %d, t %d jf %d\n", (unsigned int)j, (unsigned int)W, (unsigned int)T, (unsigned int)jiffies));
								DBGPRINT(RT_DEBUG_TRACE, ("%d:%d,%d (%d)\n", (unsigned int)pAd->CommonCfg.ch_idle_sum, (unsigned int)pAd->CommonCfg.ch_busy_sum, (unsigned int)pAd->CommonCfg.rssi, (unsigned int)pAd->CommonCfg.ch_busy));
							}
#endif
							if (j < 4)
							{
								pAd->CommonCfg.re_check_jiffies[j] = jiffies;
								pAd->CommonCfg.re_check_Width[j] = W;
								pAd->CommonCfg.re_check_Period[j] = T;
							}

						}				
								pDFSValidRadar++;						
							
						}
							
					} // if (channel & (0x1 << j))
						
				} // for (j = 0;j < NEW_DFS_MAX_CHANNEL; j++)
			}

		} // if (channel)

		if (pAd->CommonCfg.RadarReEnable == 1)
		{
			BBP_IO_WRITE8_BY_REG_ID(pAd, 141, bbp_r141);
		}

//#ifdef DFS_DEBUG
		if (pAd->CommonCfg.McuRadarDebug & RADAR_SIMULATE)
		{
			radarDeclared = 1;
			pAd->CommonCfg.McuRadarDebug &= ~RADAR_SIMULATE;
		}
//#endif
		

		if (radarDeclared || pAd->CommonCfg.radarDeclared)
		{
			// Radar found!!!
			if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DONT_SWITCH))
			{
			
				for (i=0; i<pAd->ChannelListNum; i++)
				{
					if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
					{
						pAd->ChannelList[i].RemainingTimeForUse = 1800;//30 min = 1800 sec
						break;
					}
				}

				if ((pAd->CommonCfg.RadarDetect.RDDurRegion == CE) && (pAd->CommonCfg.Channel >= 116) && (pAd->CommonCfg.Channel <= 128))
					pAd->CommonCfg.RadarDetect.ChMovingTime = 605;
				else
					pAd->CommonCfg.RadarDetect.ChMovingTime = 65;
			
				if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
				{
					for (i = 0; i < pAd->ChannelListNum ; i++)
					{
						pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
						if ((pAd->CommonCfg.Channel >= 100) && (pAd->CommonCfg.Channel <= 140))
							break;
					}
				}
				else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
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
#endif // DOT11_N_SUPPORT //
				//ApSelectChannelCheck(pAd);
				if (pAd->CommonCfg.RadarDetect.RDMode != RD_SILENCE_MODE)
				{
					pAd->CommonCfg.RadarDetect.RDMode = RD_SWITCHING_MODE;
					pAd->CommonCfg.RadarDetect.CSCount = 0;
				}
				else
				{
					pAd->CommonCfg.DFSAPRestart=1;
					schedule_dfs_task(pAd);
					//APStop(pAd);
					//APStartUp(pAd);
				}
					pAd->CommonCfg.radarDeclared = 0;
				
				
			}
			else
				pAd->CommonCfg.radarDeclared = 0;
			
			// clear long pulse table
			pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].counter = 0;
			pAd->CommonCfg.fcc_5_idx = 0;
			pAd->CommonCfg.fcc_5_last_idx = 0;
			

		
		}
	
	
    
}
#endif // DFS_HWTIMER_SUPPORT //
void TimerCB(unsigned long data);

VOID NewRadarDetectionStart(
	IN PRTMP_ADAPTER pAd)
{
	pNewDFSTable pDFS2Table;
	int i, index;
	UCHAR bbp_r140;
	UCHAR bbp_r142, bbp_r143, bbp_r144, bbp_r145, bbp_r146;
	UCHAR bbp_r147, bbp_r148, bbp_r149, bbp_r150, bbp_r151;
	UCHAR bbp_r152, bbp_r153, bbp_r154, bbp_r155;
		
	UCHAR BBPR4;

	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);

	if ((pAd->CommonCfg.RadarDetect.RDDurRegion == CE) && (pAd->CommonCfg.Channel >= 116) && (pAd->CommonCfg.Channel <= 128))
		pAd->CommonCfg.RadarDetect.ChMovingTime = 605;
	else
		pAd->CommonCfg.RadarDetect.ChMovingTime = 65;

		
	DBGPRINT(RT_DEBUG_TRACE, ("NewRadarDetectionStart\n"));
	// choose Table
	if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
	{
		if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
			pAd->CommonCfg.ch_busy_idle_ratio = 3;


		pDFS2Table = &NewDFSTable1[0];
		index = 0;
		
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use FCC table\n"));
	}
	else if (pAd->CommonCfg.RadarDetect.RDDurRegion == CE)
	{
		if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
			pAd->CommonCfg.ch_busy_idle_ratio = 2;
		
		pDFS2Table = &NewDFSTable1[1];
		index = 1;

		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use CE table\n"));
	}
	else // Japan
	{
		

		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			pDFS2Table = &NewDFSTable1[3];
			index = 3;
			if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
				pAd->CommonCfg.ch_busy_idle_ratio = 2;
		}
		else
		{
			pDFS2Table = &NewDFSTable1[2];
			index = 2;
			if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
				pAd->CommonCfg.ch_busy_idle_ratio = 3;
		}
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use JAP table\n"));
	}
	
	for(i = 0; i<4; i++)
	{
		if ((pAd->CommonCfg.DFSParamFromConfig & (0x1<<i)) && pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].valid)
		{
			pDFS2Table->entry[i].mode = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].mode;
			pDFS2Table->entry[i].avgLen = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].avgLen;
			pDFS2Table->entry[i].ELow = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].ELow;
			pDFS2Table->entry[i].EHigh = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].EHigh;
			pDFS2Table->entry[i].WLow = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].WLow;
			pDFS2Table->entry[i].WHigh = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].WHigh;
			pDFS2Table->entry[i].EpsilonW = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].EpsilonW;
			pDFS2Table->entry[i].TLow = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].TLow;
			pDFS2Table->entry[i].THigh = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].THigh;
			pDFS2Table->entry[i].EpsilonT = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].EpsilonT;


			DBGPRINT(RT_DEBUG_TRACE,("index = %d; i = %d; DFSParam = %d; %d; %d; %d; %d; %d; %d; %d; %d; %d\n", index, i, pDFS2Table->entry[i].mode,
					pDFS2Table->entry[i].avgLen, pDFS2Table->entry[i].ELow, pDFS2Table->entry[i].EHigh,
					pDFS2Table->entry[i].WLow, pDFS2Table->entry[i].WHigh, pDFS2Table->entry[i].EpsilonW,
					pDFS2Table->entry[i].TLow, pDFS2Table->entry[i].THigh, pDFS2Table->entry[i].EpsilonT));
		}
	}

	pAd->CommonCfg.MCURadarRegion = pDFS2Table->type;
	DBGPRINT(RT_DEBUG_TRACE, ("........pAd->CommonCfg.MCURadarRegion = %d\n", (unsigned int)pAd->CommonCfg.MCURadarRegion));

	// delta delay
	BBP_IO_WRITE8_BY_REG_ID(pAd, 141, pAd->CommonCfg.DeltaDelay << 4);
	DBGPRINT(RT_DEBUG_TRACE, ("........Write 141 = 0x%x\n", pAd->CommonCfg.DeltaDelay << 4));
	
	if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
		bbp_r140 = 0x80;
	else
		bbp_r140 = 0;
	
	
	// program channel i
	for (i = 0; i < NEW_DFS_MAX_CHANNEL; i++)
	{
		if (!((1 << i) & pAd->CommonCfg.ChEnable))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("skip channel %d\n", i));
			continue;
		}
		bbp_r142 = 0;
		bbp_r142 |= (pDFS2Table->entry[i].avgLen & 0x100) >> 8;
		bbp_r142 |= (pDFS2Table->entry[i].TLow & 0x10000) >> 13;
		bbp_r142 |= (pDFS2Table->entry[i].THigh & 0x10000) >> 14;
		bbp_r142 |= (pDFS2Table->entry[i].mode & 0x7) << 4;
		
		bbp_r143 = pDFS2Table->entry[i].avgLen & 0xff;

		bbp_r144 = 0;
		bbp_r144 |= (pDFS2Table->entry[i].ELow & 0xf00) >> 4;
		bbp_r144 |= (pDFS2Table->entry[i].EHigh & 0xf00) >> 8;
		
		bbp_r145 = pDFS2Table->entry[i].ELow & 0xff;
		bbp_r146 = pDFS2Table->entry[i].EHigh & 0xff;

		bbp_r147 = (pDFS2Table->entry[i].TLow & 0xff00) >> 8;
		bbp_r148 = pDFS2Table->entry[i].TLow & 0xff;

		bbp_r149 = (pDFS2Table->entry[i].THigh & 0xff00) >> 8;
		bbp_r150 = pDFS2Table->entry[i].THigh & 0xff;

		bbp_r151 = pDFS2Table->entry[i].EpsilonT & 0xff;
		
		bbp_r152 = 0;
		bbp_r152 |= (pDFS2Table->entry[i].WLow & 0xf00) >> 4;
		bbp_r152 |= (pDFS2Table->entry[i].WHigh & 0xf00) >> 8;
		
		bbp_r153 = pDFS2Table->entry[i].WLow & 0xff;

		bbp_r154 = pDFS2Table->entry[i].WHigh & 0xff;
		
		bbp_r155 = pDFS2Table->entry[i].EpsilonW & 0xff;
		
		// select channel
		BBP_IO_WRITE8_BY_REG_ID(pAd, 140, bbp_r140 | (i << 4));
		DBGPRINT(RT_DEBUG_TRACE, ("........select channel, Write 140 = 0x%x\n", bbp_r140 | (i << 4)));
		
		// start programing
		BBP_IO_WRITE8_BY_REG_ID(pAd, 142, bbp_r142);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 143, bbp_r143);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 144, bbp_r144);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 145, bbp_r145);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 146, bbp_r146);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 147, bbp_r147);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 148, bbp_r148);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 149, bbp_r149);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 150, bbp_r150);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 151, bbp_r151);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 152, bbp_r152);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 153, bbp_r153);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 154, bbp_r154);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 155, bbp_r155);
		DBGPRINT(RT_DEBUG_TRACE, ("........142 = 0x%x ", bbp_r142));
		DBGPRINT(RT_DEBUG_TRACE, ("143 = %x ", bbp_r143));
		DBGPRINT(RT_DEBUG_TRACE, ("144 = %x ", bbp_r144));
		DBGPRINT(RT_DEBUG_TRACE, ("145 = %x ", bbp_r145));
		DBGPRINT(RT_DEBUG_TRACE, ("146 = %x ", bbp_r146));
		DBGPRINT(RT_DEBUG_TRACE, ("147 = %x ", bbp_r147));
		DBGPRINT(RT_DEBUG_TRACE, ("148 = %x ", bbp_r148));
		DBGPRINT(RT_DEBUG_TRACE, ("149 = %x\n........", bbp_r149));
		DBGPRINT(RT_DEBUG_TRACE, ("150 = %x ", bbp_r150));
		DBGPRINT(RT_DEBUG_TRACE, ("151 = %x ", bbp_r151));
		DBGPRINT(RT_DEBUG_TRACE, ("152 = %x ", bbp_r152));
		DBGPRINT(RT_DEBUG_TRACE, ("153 = %x ", bbp_r153));
		DBGPRINT(RT_DEBUG_TRACE, ("154 = %x ", bbp_r154));
		DBGPRINT(RT_DEBUG_TRACE, ("155 = %x\n", bbp_r155));
	}
	
	// enable debug mode
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 3);
	
	// Enable Radar detection
	BBP_IO_WRITE8_BY_REG_ID(pAd, 140, bbp_r140 | pAd->CommonCfg.ChEnable);
	DBGPRINT(RT_DEBUG_TRACE, ("........Write BBP 140 = 0x%x\n", bbp_r140 | pAd->CommonCfg.ChEnable));

	// Enable Bandwidth usage monitor
	{
		ULONG Value;
		RTMP_IO_READ32(pAd, CH_TIME_CFG, &Value);
		RTMP_IO_WRITE32(pAd, CH_TIME_CFG, Value | 0x1f);
	}
//KH:The following codes need enclosed by compile flag
#ifdef DFS_HWTIMER_SUPPORT
	// Enable Tasklet anyway
	pAd->CommonCfg.use_tasklet = 1;
	
	// init software
	if (pAd->CommonCfg.use_tasklet == 1)
	{
		int j, k;
		for (k = 0; k < NEW_DFS_MAX_CHANNEL; k++)
		{
			for (j = 0; j < NEW_DFS_DBG_PORT_ENT_NUM; j++)
			{
				pAd->CommonCfg.DFS_W[k][j].start_idx = 0xffff;
			}
		}
	}

	if (pAd->CommonCfg.use_tasklet)
		pAd->CommonCfg.PollTime = NEW_DFS_CHECK_TIME_TASKLET;
	else
		pAd->CommonCfg.PollTime = NEW_DFS_CHECK_TIME;

	// prevent trigger radar when switch to another channel
	{
		UCHAR BBPR141;
		BBP_IO_READ8_BY_REG_ID(pAd, 141, &BBPR141);
		BBP_IO_WRITE8_BY_REG_ID(pAd, 141, BBPR141);
	}
	
#endif // DFS_HWTIMER_SUPPORT //
#ifdef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
	request_tmr_service(NEW_DFS_WATCH_DOG_TIME, &TimerCB, (void *)pAd);
#else
	if(pAd->CommonCfg.DFSWatchDogIsRunning==FALSE)
	{
		UINT32 Value;
		printk("DFS Hardware Timer Trigger\n");
		// Hardware Period Timer interrupt setting.
		RTMP_IO_READ32(pAd, INT_TIMER_CFG, &Value);
		Value &= 0x0000ffff;
		Value |= 1 << 20; // Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, Value);
		// Enable Hardware Period Timer interrupt
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
		Value |=0x2;
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
		pAd->CommonCfg.DFSWatchDogIsRunning=TRUE;
	}
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_HWTIMER_SUPPORT //
	
}


VOID NewRadarDetectionStop(
	IN PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("NewRadarDetectionStop\n"));
#ifdef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
	unregister_tmr_service();
#else
{
	UINT32 Value;
	// Hardware Period Timer interrupt setting.
	RTMP_IO_READ32(pAd, INT_TIMER_CFG, &Value);
	Value &= 0x0000ffff;
	RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, Value);
	// Disable Hardware Period Timer interrupt
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &=~(0x2);
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
        pAd->CommonCfg.DFSWatchDogIsRunning=FALSE;
}
#endif // RTMP_RBUS_SUPPORT //	
#endif // DFS_HWTIMER_SUPPORT //
	BBP_IO_WRITE8_BY_REG_ID(pAd, 140, 0x0);
	
	if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
	{
		BBP_IO_WRITE8_BY_REG_ID(pAd, 92, 0);
	}
	else
	{
		BBP_IO_WRITE8_BY_REG_ID(pAd, 91, 4);
	}
}



// the debug port have timestamp 22 digit, the max number is 0x3fffff, each unit is 25ns for 40Mhz mode and 50ns for 20Mhz mode
// so a round of timestamp is about 25 * 0x3fffff / 1000 = 104857us (about 100ms) or
// 50 * 0x3fffff / 1000 = 209715us (about 200ms) in 20Mhz mode
// 3ms = 3000,000 ns / 25ns = 120000 -- a unit 
// 0x3fffff/120000 = 34.9 ~= 35
// CE Staggered radar check
// At beginning, the goal is to detect staggered radar, now, we also detect regular radar with this function.


int SWRadarCheck(
	IN PRTMP_ADAPTER pAd, USHORT id)
{
	int i, j, k, start_idx, end_idx;
	pNewDFSDebugPort pCurrent, p1, pEnd;
	ULONG period;
	int radar_detected = 0, regular_radar = 0;
	ULONG PRF1 = 0, PRF2 = 0, PRF3 = 0;
	USHORT	minDiff, maxDiff, widthsum;
	UCHAR	Radar2PRF=0, Radar3PRF=0;
	//the sw_idx is initialed at NewRadarDetectStart and value is 255.
	USHORT	Total, SwIdxPlus = ENTRY_PLUS(pAd->CommonCfg.sw_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	UCHAR	CounterToCheck;
	
	/*
	hw_idx is the collected data by NewTimerCB_Radar through DFS Debug function
	*/
	printk("pAd->CommonCfg.hw_idx[%d]=%x,SwIdxPlus=%x\n",id,pAd->CommonCfg.hw_idx[id],SwIdxPlus);
	if (SwIdxPlus == pAd->CommonCfg.hw_idx[id])
		return 0; // no entry to process
	
	// process how many entries?? total NEW_DFS_DBG_PORT_ENT_NUM
	//we will process how many entries this time. the entires is retrieved from hw_idex and stored in sw_idx
	if (pAd->CommonCfg.hw_idx[id] > SwIdxPlus)
		Total = pAd->CommonCfg.hw_idx[id] - SwIdxPlus;
	else
		Total = pAd->CommonCfg.hw_idx[id] + NEW_DFS_DBG_PORT_ENT_NUM - SwIdxPlus;
	
	if (Total > NEW_DFS_DBG_PORT_ENT_NUM)
		pAd->CommonCfg.pr_idx[id] = ENTRY_PLUS(pAd->CommonCfg.sw_idx[id], MAX_PROCESS_ENTRY, NEW_DFS_DBG_PORT_ENT_NUM);
	else
		pAd->CommonCfg.pr_idx[id] = ENTRY_PLUS(pAd->CommonCfg.sw_idx[id], Total, NEW_DFS_DBG_PORT_ENT_NUM);
	
	
	start_idx = ENTRY_PLUS(pAd->CommonCfg.pr_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	end_idx = pAd->CommonCfg.pr_idx[id];
	
	pEnd = &pAd->CommonCfg.DFS_W[id][end_idx];
	//printk("start_idx = %d, end_idx=%d, counter=%d\n", start_idx, end_idx, pEnd->counter);
	
	//if (pAd->CommonCfg.dfs_w_counter != pEnd->counter)
	//	return 0;
	
	if (start_idx > end_idx)
		end_idx += NEW_DFS_DBG_PORT_ENT_NUM;
	
	
	pAd->CommonCfg.sw_idx[id] = pAd->CommonCfg.pr_idx[id];
	

	

	// FCC && Japan

	if (pAd->CommonCfg.RadarDetect.RDDurRegion != CE)
	{
		ULONG minPeriod = (3000 << 1);
		// Calculate how many counters to check
		// if pAd->CommonCfg.PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz
		// if pAd->CommonCfg.PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz
		// if pAd->CommonCfg.PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz
		// if pAd->CommonCfg.PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz
		// the max period to check for 40Mhz for FCC is 28650 * 2
		// the max period to check for 40Mhz for Japan is 80000 * 2
		// 0x40000 = 4194304 / 57129 = 73.xxx
		// 0x40000 = 4194304 / 160000 = 26.2144
		// 53/73 < 1 (1+1)
		// 53/26.2144 = 2.02... (2+1)
		// 27/26.2144 = 1.02... (1+1)
		// 20M should use the same value as 40Mhz mode


		if (pAd->CommonCfg.MCURadarRegion == NEW_DFS_JAP_W53)
		{
			minPeriod = 28500 << 1;
		}
		
		
		if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
		{
			CounterToCheck = 1+1; 
		}
		else // if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP)
		{
			if (pAd->CommonCfg.PollTime <= 2)
				CounterToCheck = 2+1;
			else
				CounterToCheck = 1+1;
		}
		

		
		// First Loop for FCC/JAP
		for (i = end_idx; i > start_idx; i--)
		{
			pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			// we only handle entries has same counter with the last one
			if (pCurrent->counter != pEnd->counter)
				break;
		
			pCurrent->start_idx = 0xffff;

			// calculate if any two pulse become a valid period, add it in period table,
			for (j = i - 1; j > start_idx; j--)
			{
				p1 = &pAd->CommonCfg.DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];
				
				// check period, must within max period
				if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
				{
					if (p1->counter + CounterToCheck < pCurrent->counter)
						break;
            	
					widthsum = p1->width + pCurrent->width;
					if (id == 0)
					{
						if (widthsum < 600)
							pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
						else
							pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
					
					if ( (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE) ||
						 (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff)) )
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
						
						if ((period >= (minPeriod - 2)) && (period <= pAd->CommonCfg.dfs_max_period))
						{
            	
							// add in period table
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
            	
            	
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
							pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
							
							pAd->CommonCfg.dfs_t_idx[id]++;
							if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pAd->CommonCfg.dfs_t_idx[id] = 0;
						}
						else if (period > pAd->CommonCfg.dfs_max_period)
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
							pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
						else
							pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;

            	
					if ( (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE) || 
						 (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff)) )
            	
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
            	
						if ((period >= ((minPeriod >> 1) - 2)) && (period <= (pAd->CommonCfg.dfs_max_period >> 1)))
						{
							// add in period table
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
							
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
							pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
							
							pAd->CommonCfg.dfs_t_idx[id]++;
							if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pAd->CommonCfg.dfs_t_idx[id] = 0;
						}
						else if (period > (pAd->CommonCfg.dfs_max_period >> 1))
							break;
					}
				}

			} // for (j = i - 1; j > start_idx; j--)

		} // for (i = end_idx; i > start_idx; i--)


		// Second Loop for FCC/JAP
		for (i = end_idx; i > start_idx; i--)
		{
			
			pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			// we only handle entries has same counter with the last one
			if (pCurrent->counter != pEnd->counter)
				break;
			if (pCurrent->start_idx != 0xffff)
			{
				//pNewDFSDebugPort	p2, p3, p4, p5, p6;
				pNewDFSDebugPort	p2, p3;
				pNewDFSMPeriod pCE_T;
				ULONG idx[10], T[10];

				for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
				{

					pCE_T = &pAd->CommonCfg.DFS_T[id][idx[0]];
				
					p2 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];
				
					if (p2->start_idx == 0xffff)
						continue;
				
					T[0] = pCE_T->period;


					for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
					{
						
						pCE_T = &pAd->CommonCfg.DFS_T[id][idx[1]];
					
						p3 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];

						if (idx[0] == idx[1])
							continue;
						
						if (p3->start_idx == 0xffff)
							continue;
					


						T[1] = pCE_T->period;
						
						
						if ( PERIOD_MATCH(T[0], T[1], pAd->CommonCfg.dfs_period_err))
						{
							if (id <= 2) // && (id >= 0)
							{

								//if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > minPeriod)) ||
								//	((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > (minPeriod >> 1))) )
								{
									unsigned int loop, PeriodMatched = 0, idx1;
									for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
									{
										idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
										if (PERIOD_MATCH(pAd->CommonCfg.DFS_T[id][idx1].period, T[1], pAd->CommonCfg.dfs_period_err))
										{
#ifdef DFS_DEBUG
											if (PeriodMatched < 5)
											{
												pAd->CommonCfg.CounterStored[PeriodMatched] = pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.DFS_T[id][idx1].idx].counter;
												pAd->CommonCfg.CounterStored2[PeriodMatched] = loop;
												pAd->CommonCfg.CounterStored3 = idx[1];
											}
#endif
											//printk("%d %d\n", loop, pAd->CommonCfg.DFS_T[id][idx[1]-loop].period);
											PeriodMatched++;
										}
										
									}
								
								
									if (PeriodMatched > pAd->CommonCfg.dfs_declare_thres)
									{
#ifdef DFS_DEBUG
										if (PeriodMatched == 3)
										{
											pAd->CommonCfg.T_Matched_3++;
											//printk("counter=%d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2]);
											//printk("idx[1]=%d, loop =%d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2]);
										}
										else if (PeriodMatched == 4)
										{
											pAd->CommonCfg.T_Matched_4++;
											//printk("counter=%d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3]);
											//printk("idx[1]=%d, loop =%d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3]);
										}
										else
										{
											pAd->CommonCfg.T_Matched_5++;
											//printk("counter=%d %d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3], pAd->CommonCfg.CounterStored[4]);
											//printk("idx[1]=%d, loop =%d %d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3], pAd->CommonCfg.CounterStored2[4]);
										}
                                    	
										pAd->CommonCfg.DebugPortPrint = 1;
									
#endif

										{
											pNewDFSValidRadar pDFSValidRadar;
											ULONG T1 = (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)? (T[1]>>1) : T[1];
											
											pDFSValidRadar = &NewDFSValidTable[0];
											
                    					
											while (pDFSValidRadar->type != NEW_DFS_END)
											{
												if ((pDFSValidRadar->type & pAd->CommonCfg.MCURadarRegion) == 0)
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
											else if (pAd->CommonCfg.MCURadarRegion != NEW_DFS_JAP_W53)
												printk("W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T1, (unsigned int)id, PeriodMatched);

										}

										
									}
#ifdef DFS_DEBUG
									else if (PeriodMatched == 2)
									{
										pAd->CommonCfg.T_Matched_2++;
									}
#endif
								
								
								}
								
							} // if (id <= 2) // && (id >= 0)
							
						}
						
					} // for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)


					// increase FCC-1 detection
					if (id <= 2)
					{
						if (IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
								int loop, idx1, PeriodMatched_fcc1 = 0;
								for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
								{
									idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
									if ( IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pAd->CommonCfg.DFS_T[id][idx1].period) )
									{
										//printk("%d %d %d\n", PeriodMatched_fcc1, pAd->CommonCfg.DFS_T[id][idx1].period, loop);
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


					// increase W56-3 detection
					if ((pAd->CommonCfg.MCURadarRegion == NEW_DFS_JAP) && (id <= 2))
					{
						if (IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
								int loop, idx1, PeriodMatched_w56_3 = 0;
								for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
								{
									idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
									if ( IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pAd->CommonCfg.DFS_T[id][idx1].period) )
									{
										//printk("%d %d %d\n", PeriodMatched_w56_3, pAd->CommonCfg.DFS_T[id][idx1].period, loop);
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


					if ((pAd->CommonCfg.MCURadarRegion == NEW_DFS_JAP_W53) && (id <= 2) && IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
					{
						int loop, idx1, PeriodMatched_W56_2 = 0;
						
						for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
						{
							idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
							if ( IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pAd->CommonCfg.DFS_T[id][idx1].period) )
							{
								//printk("%d %d %d\n", PeriodMatched_W56_2, pAd->CommonCfg.DFS_T[id][idx1].period, loop);
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



				} // for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
			} // if (pCurrent->start_idx != 0xffff)
		} // for (i = end_idx; i > start_idx; i--)
		
		return radar_detected;
	}

	// CE have staggered radar	
	
	// Calculate how many counters to check
	// if pAd->CommonCfg.PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz
	// if pAd->CommonCfg.PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz
	// if pAd->CommonCfg.PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz
	// if pAd->CommonCfg.PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz
	// if pAd->CommonCfg.PollTime is 8ms, a round of timestamp is ?? for 20Mhz, 12 for 40Mhz
	// the max period to check for 40Mhz is 133333 + 125000 + 117647 = 375980
	// 0x40000 = 4194304 / 375980 = 11.1556
	// 53/11.1556 = 4.75...
	// 35/11.1556 = 3.1374, (4+1) is safe, (3+1) to save CPU power, but may lost some data
	// 27/11.1556 = 2.42, (3+1) is OK
	// 21/11.1556 = 1.88, (2+1) is OK
	// 20M should use the same value as 40Mhz mode
	if (pAd->CommonCfg.PollTime == 1)
		CounterToCheck = 5+1;
	else if (pAd->CommonCfg.PollTime == 2)
		CounterToCheck = 4+1;
	else if (pAd->CommonCfg.PollTime == 3)
		CounterToCheck = 3+1;
	else if (pAd->CommonCfg.PollTime <= 8)
		CounterToCheck = 2+1;
	else
		CounterToCheck = 1+1;

	// First Loop for CE
	for (i = end_idx; i > start_idx; i--)
	{
		pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		// we only handle entries has same counter with the last one
		if (pCurrent->counter != pEnd->counter)
			break;
		
		pCurrent->start_idx = 0xffff;

		// calculate if any two pulse become a valid period, add it in period table,
		for (j = i - 1; j > start_idx; j--)
		{
			p1 = &pAd->CommonCfg.DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];
			

			// check period, must within 16666 ~ 66666
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
			{
				if (p1->counter + CounterToCheck < pCurrent->counter)
						break;

				widthsum = p1->width + pCurrent->width;
				if (id == 0)
				{
					if (((p1->width > 310) && (pCurrent->width < 300)) || ((pCurrent->width > 310) && ((p1->width < 300))) )
						continue;
					if (widthsum < 620)
						pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_H;
					else
						pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
					
				}
				else if (id == 1)
					pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch1_Shift;
				else if (id == 2)
					pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
				
				if ( (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE) ||
					 (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff)) )
				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;
					
					//if ((period >= (33333 - 20)) && (period <= (133333 + 20)))
					if ((period >= (10000 - 2)) && (period <= pAd->CommonCfg.dfs_max_period))
					{

						// add in period table
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
        
        
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
						pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
						
						pAd->CommonCfg.dfs_t_idx[id]++;
						if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pAd->CommonCfg.dfs_t_idx[id] = 0;
					}
					else if (period > pAd->CommonCfg.dfs_max_period) // to allow miss a pulse
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
						pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_H;
					else
						pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
				}
				else if (id == 1)
					pAd->CommonCfg.dfs_width_diff = widthsum >> 4;
				else if (id == 2)
					pAd->CommonCfg.dfs_width_diff = widthsum >> 6;

				if ( (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE) || 
					 (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff)) )

				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;

					if ((period >= (5000 - 2)) && (period <= (pAd->CommonCfg.dfs_max_period >> 1)))
					{
						// add in period table
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
						
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
						pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
						
						pAd->CommonCfg.dfs_t_idx[id]++;
						if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pAd->CommonCfg.dfs_t_idx[id] = 0;
					}
					else if (period > (pAd->CommonCfg.dfs_max_period >> 1))
						break;
				}
			}
			
		} // for (j = i - 1; j > start_idx; j--)
	}

	// Second Loop for CE
	for (i = end_idx; i > start_idx; i--)
	{
		pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		// we only handle entries has same counter with the last one
		if (pCurrent->counter != pEnd->counter)
			break;
		
		// Check Staggered radar
		if (pCurrent->start_idx != 0xffff)
		{
			pNewDFSDebugPort	p2, p3, p4, p5, p6, p7;
			pNewDFSMPeriod pCE_T;
			ULONG idx[10], T[10];
			
			//printk("pCurrent=%d, idx=%d~%d\n", pCurrent->timestamp, pCurrent->start_idx, pCurrent->end_idx);

			for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
			{
				pCE_T = &pAd->CommonCfg.DFS_T[id][idx[0]];
				
				p2 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];
				
				//printk("idx[0]= %d, idx=%d p2=%d, idx=%d~%d\n", idx[0], pCE_T->idx2, p2->timestamp, p2->start_idx, p2->end_idx);
				
				if (p2->start_idx == 0xffff)
					continue;
				
				T[0] = pCE_T->period;


				for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
				{
					
					pCE_T = &pAd->CommonCfg.DFS_T[id][idx[1]];
					
					p3 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];
					
					//printk("p3=%d, idx=%d~%d\n", p3->timestamp, p3->start_idx, p3->end_idx);

					if (idx[0] == idx[1])
						continue;
						
					if (p3->start_idx == 0xffff)
						continue;
					


					T[1] = pCE_T->period;

		
					if (PERIOD_MATCH(T[0], T[1], pAd->CommonCfg.dfs_period_err))
					{
						if (id <= 2) // && (id >= 0)
						{

							
							if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > 66666)) ||
								((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > 33333)) )
							{
								unsigned int loop, PeriodMatched = 0, idx1;
								
								for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
								{
									idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
									if (PERIOD_MATCH(pAd->CommonCfg.DFS_T[id][idx1].period, T[1], pAd->CommonCfg.dfs_period_err))
									{
#ifdef DFS_DEBUG
										if (PeriodMatched < 5)
										{
											pAd->CommonCfg.CounterStored[PeriodMatched] = pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.DFS_T[id][idx1].idx].counter;
											pAd->CommonCfg.CounterStored2[PeriodMatched] = loop;
											pAd->CommonCfg.CounterStored3 = idx[1];
										}
#endif
										//printk("%d %d\n", loop, pAd->CommonCfg.DFS_T[id][idx[1]-loop].period);
										PeriodMatched++;
									}
									
								}
								
								
								if (PeriodMatched > pAd->CommonCfg.dfs_declare_thres)
								{
#ifdef DFS_DEBUG
									if (PeriodMatched == 3)
									{
										pAd->CommonCfg.T_Matched_3++;
										//printk("counter=%d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2]);
										//printk("idx[1]=%d, loop =%d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2]);
									}
									else if (PeriodMatched == 4)
									{
										pAd->CommonCfg.T_Matched_4++;
										//printk("counter=%d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3]);
										//printk("idx[1]=%d, loop =%d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3]);
									}
									else
									{
										pAd->CommonCfg.T_Matched_5++;
										//printk("counter=%d %d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3], pAd->CommonCfg.CounterStored[4]);
										//printk("idx[1]=%d, loop =%d %d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3], pAd->CommonCfg.CounterStored2[4]);
									}

									pAd->CommonCfg.DebugPortPrint = 1;
#endif
									printk("Radar Detected(CE), W=%d, T=%d (%d), period matched=%d\n", (unsigned int)pCE_T->width, (unsigned int)T[1], (unsigned int)id, PeriodMatched);
									

									if (PeriodMatched > (pAd->CommonCfg.dfs_declare_thres + 1))
 								      		radar_detected = 1;
									return radar_detected;
								}
#ifdef DFS_DEBUG
								else if (PeriodMatched == 2)
								{
									pAd->CommonCfg.T_Matched_2++;
									//printk("counter=%d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1]);
									//printk("idx[1]=%d, loop =%d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1]);
								}
#endif
								
								
							}
						}
						
					}

				} // for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)

			} // for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)

		}
		
	} // for (i = end_idx; i < start_idx; i--)
	
	
	return radar_detected;
	
}
#endif // DFS_1_SUPPORT

#ifdef DFS_2_SUPPORT
typedef struct _NewDFSProgParam
{

	UCHAR channel;
#define DFS_DETECTION_MODE_NORMAL					0x0
#define DFS_DETECTION_MODE_STAGGERED				0x1
#define DFS_DETECTION_MODE_FCC_CHIRP_2_WIDTH		0x2
#define DFS_DETECTION_MODE_EN_CHIRP					0x3
#define DFS_DETECTION_MODE_SINGLE_PERIOD_NORMAL		0x4
#define DFS_DETECTION_MODE_STAGGERED_ALG_2			0x5
#define DFS_DETECTION_MODE_FCC_CHIRP_NO_WIDTH		0x6
#define DFS_DETECTION_MODE_SINGLE_PERIOD_EN_CHIRP	0x7
	UCHAR mode;			// reg 0x10, Detection Mode[2:0]
	USHORT avgLen;		// reg 0x11~0x12, M[7:0] & M[8]
	USHORT ELow;		// reg 0x13~0x14, Energy Low[7:0] & Energy Low[11:8]
	USHORT EHigh;		// reg 0x15~0x16, Energy High[7:0] & Energy High[11:8]
	USHORT WLow;		// reg 0x28~0x29, Width Low[7:0] & Width Low[11:8]
	USHORT WHigh;		// reg 0x2a~0x2b, Width High[7:0] & Width High[11:8]
	UCHAR EpsilonW;		// reg 0x2c, Width Delta[7:0], (Width Measurement Uncertainty) 
	ULONG TLow;			// reg 0x17~0x1a, Period Low[7:0] & Period Low[15:8] & Period Low[23:16] & Period Low[31:24]
	ULONG THigh;		// reg 0x1b~0x1e, Period High[7:0] & Period High[15:8] & Period High[23:16] & Period High[31:24]
	UCHAR EpsilonT;		// reg 0x27, Period Delt[7:0], (Period Measurement Uncertainty) 

	ULONG BLow;			// reg 0x1f~0x22, Burst Low[7:0] & Burst Low[15:8] & Burst Low[23:16] & Burst Low[31:24]
	ULONG BHigh;		// reg 0x23~0x26, Burst High[7:0] & Burst High[15:8] & Burst High[23:16] & Burst High[31:24]		

}NewDFSProgParam, *pNewDFSProgParam;

typedef struct _NewDFSTable
{
	USHORT type;
	NewDFSProgParam entry[NEW_DFS_MAX_CHANNEL];
}NewDFSTable, *pNewDFSTable;

#if defined(RT3883) || defined (RT35xx)
#ifdef DFS_SYMMETRIC_ROUND_2
static NewDFSTable NewDFSTable1[] = 
{	
	{	
		// ch, mode(0~7), M(~511), el, eh(~4095), wl, wh(~4095), err_w, tl, th, err_t, bl, bh
			NEW_DFS_FCC,
			{		
				{0, 0,  10,   8,  16,   6, 2000,  5, 2900, 30000, 5, 0, 0},
				{1, 0,  70,  48, 126,  20, 5000,  5, 2900, 30000, 10, 0, 0},
				{2, 0, 100,  48, 160,  20, 5000, 25, 2900, 30000, 20, 0, 0},
				{3, 2, 200,  20, 150, 300, 2000, 50, 1000, 999999999, 200, 0, 999999999},
			}	
	},
	{
			NEW_DFS_EU,
			{		
				{0, 0,  10,  14,  18,  4, 1000,  5, 4900, 101000, 5, 0, 0},
				{1, 0,  70,  48,  90,  20, 5000,  3, 4900, 101000, 10, 0, 0},
				{2, 0, 100,  48, 160,  20, 5000,  5, 4900, 101000, 20, 0, 0},
				{3, 2, 200,  20, 150, 300, 4000, 50, 4900, 999999999, 200, 0, 999999999},
			}
	},
	{		
			NEW_DFS_JAP,
			{
				{0, 0,  10,   8,  16,   4, 2000,  5, 2500, 85000, 5, 0, 0},
				{1, 0,  70,  48, 126,  20, 5000,  5, 2500, 85000, 10, 0, 0},	
				{2, 0, 100,  48, 160,  20, 5000, 25, 2500, 85000, 20, 0, 0},
				{3, 2, 200,  20, 150, 300, 2000, 50, 1000, 999999999, 200, 0, 999999999},
			}
	},
	{
			NEW_DFS_JAP_W53,
			{
				{0, 0,  10,   8,  16,   8,  2000,  5, 28000, 85000, 10},
				{1, 0,  32,  24,  64,  20,  2000,  5, 28000, 85000, 10},
				{2, 0, 100,  48, 160,  20,  2000, 25, 28000, 85000, 10},
				{3, 2, 200,  20, 150, 300,  2000, 50, 15000, 45000, 200},
			}
	},
};
#else
static NewDFSTable NewDFSTable1[] = 
{
	{

	// ch, mode(0~7), M(~511), el, eh(~4095), wl, wh(~4095), err_w, tl, th, err_t, bl, bh

		NEW_DFS_FCC,
		{
		{0, 0,  10,   8,  16,   6, 2000,  5, 2900, 30000, 5, 0, 0},
		{1, 0,  70,  42, 126,  20, 5000,  5, 2900, 30000, 10, 0, 0},
		{2, 0, 100,  42, 160,  20, 5000, 25, 2900, 30000, 20, 0, 0},
		{3, 2, 200,  20, 150, 300, 2000,  50, 1000, 999999999, 200, 0, 999999999},
		}
	},
	{
		NEW_DFS_EU,
		{
		{0, 0,  10,  10,  18,  4, 1000,  5, 4900, 101000, 5, 0, 0},
		{1, 0,  70,  42,  90,  20, 5000,  3, 4900, 101000, 10, 0, 0},
		{2, 0, 100,  42, 160, 20, 5000,  5, 4900, 101000, 20, 0, 0},
		{3, 2, 200,  20, 150, 300, 4000, 50, 4900, 999999999, 200, 0, 999999999},
		}
	},
	{
		NEW_DFS_JAP,
		{
		{0, 0,  10,   8,  16,   4, 2000,  5, 2500, 85000, 5, 0, 0},
		{1, 0,  70,  42, 126,   20, 5000,  5,  2500, 85000, 10, 0, 0},
		{2, 0, 100,  42, 160,   20, 5000, 25,  2500, 85000, 20, 0, 0},
		{3, 2, 200,  20, 150, 300, 2000,  50, 1000, 999999999, 200, 0, 999999999},
		}
	},
	{
		NEW_DFS_JAP_W53,
		{
		{0, 0, 10,  8,  16,   8,  2000,  5,  28000, 85000, 10},
		{1, 0, 32,  24,  64,   20, 2000, 5,  28000, 85000, 10},
		{2, 0, 100,  42, 160,   20, 2000, 25,  28000, 85000, 10},
		{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200},
		}
	},

	
};
#endif // DFS_SYMMETRIC_ROUND_2 //
#endif // defined(RT3883) || defined (RT35xx) //

#ifdef RT2883
static NewDFSTable NewDFSTable1[] = 
{
	{

	// ch, mode(0~7), M(~511), el, eh(~4095), wl, wh(~4095), err_w, tl, th, err_t, bl, bh

		NEW_DFS_FCC,
		{
		{0, 0,  20,  18,  26,   6, 2000, 5,  3000, 30000, 5, 0, 0},
		{1, 0, 70, 80, 100,   20, 2000, 40,  2500, 30000, 20, 0, 0},
		{2, 0, 100,  80, 140,   20, 2000, 40,  3000, 30000, 20, 0, 0},
		{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200, 0, 0},
		}
	},
	{
		NEW_DFS_EU,
		{
		{0, 0, 12,	16,	20,	6,	1000,	5, 4900, 101000, 5, 0, 0},
		{1, 0, 70,	80,100, 20, 2000,	40, 4900, 101000, 20, 0, 0},
		{2, 0, 100,	80, 140, 20, 3000,	40, 4900, 101000, 20, 0, 0},
		{3, 3, 250,	20, 200, 300,4000,	50, 4900, 10100, 200, 0, 0},
		}
	},
	{
		NEW_DFS_JAP,
		{
		{0, 0,  10,  10,  16,   4,  2000,  5,  3000, 85000, 5, 0, 0},
		{1, 0, 70, 80, 100,   30, 2000, 40,  2500, 85000, 20, 0, 0},
		{2, 0, 100,  80, 140,   30, 2000, 40,  3000, 85000, 20, 0, 0},
		{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200, 0, 0},
		}
	},
	{
		NEW_DFS_JAP_W53,
		{
		{0, 0, 12,  10,  16,   12,  2000,  1,  28000, 85000, 10},
		{1, 0, 32, 30, 40,   20, 2000, 3,  28000, 85000, 10},
		{2, 0, 80,  60, 120,   20, 2000, 3,  28000, 85000, 10},
		{3, 2, 200,  20, 150, 300, 2000,  50, 15000, 45000, 200},
		}
	},

	
};
#endif // RT2883 //



 VOID NewTimerCB_Radar(
 	IN PRTMP_ADAPTER pAd)

{
	UCHAR channel=0;
	UCHAR radarDeclared = 0;
	INT i, j;
	ULONG W, T;
	UCHAR BBP_1=0, BBP_2=0, BBP_3=0, BBP_4=0;
	pNewDFSTable pDFS2Table;
#ifdef DFS_HWTIMER_SUPPORT
	if (pAd->CommonCfg.PollTime == 0)
	{
		return;
	}

	if (pAd->CommonCfg.RadarTimeStampLow++ == 0xffffffff)
		pAd->CommonCfg.RadarTimeStampHigh++;

		/*511ms*/
	if ((pAd->CommonCfg.RadarTimeStampLow & 0x1ff) == 0)
	{
		int busy_delta, idle_delta;


		RTMP_IO_READ32(pAd, CH_IDLE_STA, &pAd->CommonCfg.idle_time);
		RTMP_IO_READ32(pAd, CH_BUSY_STA, &pAd->CommonCfg.busy_time);
		//ch_busy_sta_index begining at 0.
		busy_delta = pAd->CommonCfg.busy_time - pAd->CommonCfg.ch_busy_sta[pAd->CommonCfg.ch_busy_sta_index];
		idle_delta = pAd->CommonCfg.idle_time - pAd->CommonCfg.ch_idle_sta[pAd->CommonCfg.ch_busy_sta_index];


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

				
		pAd->CommonCfg.ch_busy_sum += busy_delta;
		pAd->CommonCfg.ch_idle_sum += idle_delta;
				
				
		// not sure if this is necessary??
		if (pAd->CommonCfg.ch_busy_sum < 0)
			pAd->CommonCfg.ch_busy_sum = 0;
		if (pAd->CommonCfg.ch_idle_sum < 0)
			pAd->CommonCfg.ch_idle_sum = 0;
				

		pAd->CommonCfg.ch_busy_sta[pAd->CommonCfg.ch_busy_sta_index] = pAd->CommonCfg.busy_time;
		pAd->CommonCfg.ch_idle_sta[pAd->CommonCfg.ch_busy_sta_index] = pAd->CommonCfg.idle_time;
				

		pAd->CommonCfg.ch_busy_sta_index++;
		pAd->CommonCfg.ch_busy_sta_index &= CH_BUSY_MASK;
				
		if ((pAd->CommonCfg.ch_idle_sum >> pAd->CommonCfg.ch_busy_idle_ratio) < pAd->CommonCfg.ch_busy_sum )
		{
			if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_DONT_CHECK_BUSY))
			pAd->CommonCfg.ch_busy = 1;
		}
		else 
		{
			if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_DONT_CHECK_RSSI))
			{
				if ((pAd->ApCfg.RssiSample.AvgRssi0) && (pAd->ApCfg.RssiSample.AvgRssi0 > pAd->CommonCfg.DfsRssiHigh))
					pAd->CommonCfg.ch_busy = 2;
				else if ((pAd->ApCfg.RssiSample.AvgRssi0) && (pAd->ApCfg.RssiSample.AvgRssi0 < pAd->CommonCfg.DfsRssiLow))
					pAd->CommonCfg.ch_busy = 3;
				else
					pAd->CommonCfg.ch_busy = 0;
			}
		}

		if (pAd->CommonCfg.print_ch_busy_sta)
			printk ("%d %d %d %d\n", pAd->CommonCfg.ch_idle_sum, pAd->CommonCfg.ch_busy_sum, pAd->ApCfg.RssiSample.AvgRssi0, pAd->CommonCfg.ch_busy);
	}
/*3x511ms~1.5*/

	if ((pAd->CommonCfg.McuRadarTick++ >= pAd->CommonCfg.PollTime) && (!pAd->CommonCfg.ch_busy))
	{
				int k, count, limit = 384;
				UCHAR BBPR127=0, BBPR126=0, LastBBPR127 = 0xff;
				ULONG time = 0;
				USHORT width = 0;
				UCHAR id = 0;
				UCHAR alignment = FALSE;
				UCHAR bitMap[6] = {0,0,0,0,0,0}, c = 0;
		
				pAd->CommonCfg.McuRadarTick = 0;
	
				// disable debug mode to read debug port of channel 3
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R126, &BBPR126);
				//Power up the DFS event buffer and Disable the capture.
				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, (BBPR126 & 0xfe) | 0x2);
				count = 0;

				pAd->CommonCfg.dfs_w_counter++;

				// The first one need to be discard for 2883
				BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);

				for (k = 0; k < limit; k++)
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);
					//each entry for each channel has 6 bytes
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
							//LastBBPR127 = BBPR127;
						}
						else
						{
							if ((LastBBPR127 & 0xf8) != 0)
							{
								//LastBBPR127 = BBPR127;
								alignment = FALSE;
							}
							else
							{
								if ((LastBBPR127 != 0) && (id == 0))
								{
									//LastBBPR127 = BBPR127;
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
							alignment = FALSE;
							//LastBBPR127 = BBPR127;
							break;
						}						

						width |= (BBPR127 & 0x7) << 10;

#ifdef RT2883
						// the width in 40Mhz mode will be twice of width in 20Mhz mode
						// if mis-read the debug event, the first bytes will be 00, the second byte may be id 00~03, so 
						// 0x3fff00 will cover this case.
						if ((id == 0x3) && (pAd->CommonCfg.RadarDetect.RDDurRegion != CE) && (pAd->CommonCfg.MCURadarRegion != NEW_DFS_JAP_W53) && (width > 1000) && (time & 0x3fff00)) // FCC && Japan only
						{
							// in case mis-read debug event, the continue 2 entries may have exactly the same values.
							if (pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].timestamp != time)
							{
							/*
								!(time&0xff==0||
								time&0xff00==0||==> the time is not 0
								width&0x3fc==0x3fc||=>if the first 11 bits is not zero
								width&0x3fc==0)||=>or the last three bit is not zero
								((time & 0x3f0000 == 0x3f0000) && (width&0x3 == 0x3))==>if the last three bits of width is not zero, we will accept if only if the first 7 bits is not zero
								
								
							*/
								if (!(((time & 0xff) == 0) || ((time & 0xff00) == 0) || (((time & 0x3f0000) == 0x3f0000) && ((width & 0x3) == 0x3)) || ((width & 0x3fc) == 0x3fc) || ((width & 0x3fc) == 0)))
								{
									if (pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_EVENT)
									{
										printk("counter = %d  ", (unsigned int)pAd->CommonCfg.dfs_w_counter);
										printk("time = %d (%x) ", (unsigned int)time, (unsigned int)time);
										printk("width = %d (%x)\n", (unsigned int)width, (unsigned int)width);
									}

									pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].counter = pAd->CommonCfg.dfs_w_counter;
									pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].timestamp = time;
									pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].width = width;
										
										if (pAd->CommonCfg.fcc_5_last_idx != pAd->CommonCfg.fcc_5_idx)
										{
										if (PERIOD_MATCH(pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].counter, pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].counter, 3000))
											{
											// long pulse basicly have different length, if the width is exactly the same, it's probably mis-read debug event entries
											if ((pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].width != pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].width) && (time & 0x3fc000))
												{
												if (((time & 0xff) != 0) || ((pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_last_idx].timestamp & 0xff) != 0))
													{
														printk("radar detected!!! id == %d\n", id);
														radarDeclared = 1;
													}
												}
											}
										}
										pAd->CommonCfg.fcc_5_last_idx = pAd->CommonCfg.fcc_5_idx;
										pAd->CommonCfg.fcc_5_idx++;
									if (pAd->CommonCfg.fcc_5_idx >= NEW_DFS_FCC_5_ENT_NUM)
											pAd->CommonCfg.fcc_5_idx = 0;
									}
									else
									{
										id = 0xff;
									alignment = TRUE;
									break;
								}								
							}
							else
							{
								id = 0xff;
								alignment = TRUE;
										break;
									}
									
								}
#endif // RT2883 //


						if (pAd->CommonCfg.use_tasklet)
						{
							//if (id <= 0x2) // && (id >= 0)
							if ((id < 0x3) && (time & 0x3fff00)) // && (id >= 0)
							{
								//Loop=384=>384/3=128
								//The buffer is more than twice space to store two loop values.
								//[0~2][255~0]
								if (time != pAd->CommonCfg.DFS_W[id][((pAd->CommonCfg.dfs_w_idx[id] == 0)? (NEW_DFS_DBG_PORT_ENT_NUM-1):(pAd->CommonCfg.dfs_w_idx[id] - 1))].timestamp)
								{
									if (!(((time & 0xff) == 0) || ((time & 0xff00) == 0) || (((time & 0x3f0000) == 0) && ((width & 0x3) == 0x3) || ((width & 0x3fc) == 0x3fc) || ((width & 0x3fc) == 0))))
									{
										pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].counter = pAd->CommonCfg.dfs_w_counter;
										pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].timestamp = time;
										pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.dfs_w_idx[id]].width = width;
								
										pAd->CommonCfg.dfs_w_last_idx[id] = pAd->CommonCfg.dfs_w_idx[id];
										pAd->CommonCfg.dfs_w_idx[id]++;
										if (pAd->CommonCfg.dfs_w_idx[id] >= NEW_DFS_DBG_PORT_ENT_NUM)
											pAd->CommonCfg.dfs_w_idx[id] = 0;
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
						pAd->CommonCfg.TotalEntries[id]++;						
#endif 
						break;

					}
					if ((id == 0xff) && (alignment == TRUE))
					{
						break;
					}


#ifdef DFS_DEBUG
					if (pAd->CommonCfg.DebugPortPrint == 1 && (k == 0))
					{
						pAd->CommonCfg.DebugPortPrint = 2;
					}

					if ((pAd->CommonCfg.DebugPortPrint == 2))
					{
						pAd->CommonCfg.DebugPort[k] = BBPR127;
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
				if (pAd->CommonCfg.DebugPortPrint == 2)
				{
					pAd->CommonCfg.DebugPortPrint = 3;
				}
#endif


#ifdef DFS_DEBUG
				if (pAd->CommonCfg.BBP127Repeat)
				{
					for (k = 0; k < pAd->CommonCfg.BBP127Repeat; k++)
						BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);

					pAd->CommonCfg.BBP127Repeat = 0;
				}
				else
#endif
				// read to several times for alignment
				//Sometimes the previous loop will not finish reading a count when encountering error counting.
				//The fllowing codes is used to finish the reading of a counting.
				for (k = count; k < 5; k++)
				{
					BBP_IO_READ8_BY_REG_ID(pAd, BBP_R127, &BBPR127);
				}


				// enable debug mode
				if (pAd->CommonCfg.dfs_w_counter & 1)
				{
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 3);
				}
				else
				{
					BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 7);
				}

				if (pAd->CommonCfg.use_tasklet)
				{
					// set hw_idx
					pAd->CommonCfg.hw_idx[0] = pAd->CommonCfg.dfs_w_idx[0];
					pAd->CommonCfg.hw_idx[1] = pAd->CommonCfg.dfs_w_idx[1];
					pAd->CommonCfg.hw_idx[2] = pAd->CommonCfg.dfs_w_idx[2];
					pAd->CommonCfg.hw_idx[3] = pAd->CommonCfg.dfs_w_idx[3];
//dfs tasklet will call SWRadarCheck
					schedule_dfs_task(pAd);
				}

				BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, (BBPR126 | 0x3));
	}
	
#endif // DFS_HWTIMER_SUPPORT //
	//The following codes is used to check if the hardware find the Radar Signal
	//Read the 0~3 channel which had detected radar signals
	// Poll Status register
	//Set BBP_R140=0x02 and Read BBP_R141 to store at channel
	RTMP_DFS_IO_READ8(pAd, 0x2, &channel);
		//Check if any interrupt trigger by Radar Global Status(Radar Signals)
	if ((channel & 0xf) && (!pAd->CommonCfg.ch_busy))
	{
//Select the DFS table based on radar country region
		if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
			pDFS2Table = &NewDFSTable1[0];
		else if (pAd->CommonCfg.RadarDetect.RDDurRegion == CE)
		{
			pDFS2Table = &NewDFSTable1[1];
		}
		else // Japan
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
		//check which channe(0~3) is detecting radar signals
		for (j = 0; j < NEW_DFS_MAX_CHANNEL; j++)
		{
			
			if (channel & (0x1 << j))
			{

				// select channel
				RTMP_DFS_IO_WRITE8(pAd, 0x0, j);
				//DBGPRINT(RT_DEBUG_TRACE, ("ch = %d\n", j));


				// Read reports
				// Period
				RTMP_DFS_IO_READ8(pAd, 0x2d, &BBP_1);
				RTMP_DFS_IO_READ8(pAd, 0x2e, &BBP_2);
				RTMP_DFS_IO_READ8(pAd, 0x2f, &BBP_3);
				RTMP_DFS_IO_READ8(pAd, 0x30, &BBP_4);
				T = BBP_1 | (BBP_2 << 8) | (BBP_3 << 16) | (BBP_4 << 24);

				// Width
				RTMP_DFS_IO_READ8(pAd, 0x31, &BBP_1);
				RTMP_DFS_IO_READ8(pAd, 0x32, &BBP_2);
				W = BBP_1 | ((BBP_2 & 0xf) << 8);
				
				//DBGPRINT(RT_DEBUG_TRACE,("T = %d, W= %d\n", T, W));
				printk("T = %lu, W= %lu detected by ch %d\n", T, W, j);
				//set this variable to 1 for announcing that we find the radar signals.
				radarDeclared = 1;

				//if ((pDFS2Table->entry[j].mode == 1) || (pDFS2Table->entry[j].mode == 2) || (pDFS2Table->entry[j].mode == 6))
				//{
				//}
				
				
				if ( ((j == 3) || (j == 2)) && (pDFS2Table->entry[j].mode != 0) )
				{
					ULONG B, W2;
					
					RTMP_DFS_IO_READ8(pAd, 0x33, &BBP_1);
					RTMP_DFS_IO_READ8(pAd, 0x34, &BBP_2);
					RTMP_DFS_IO_READ8(pAd, 0x35, &BBP_3);
					RTMP_DFS_IO_READ8(pAd, 0x36, &BBP_4);
					B = BBP_1 | (BBP_2 << 8) | (BBP_3 << 16) | (BBP_4 << 24);
					printk("Burst = %lu(0x%lx)\n", B, B);

					RTMP_DFS_IO_READ8(pAd, 0x37, &BBP_1);
					RTMP_DFS_IO_READ8(pAd, 0x38, &BBP_2);
					W2 = BBP_1 | (BBP_2 << 8);
					printk("The second Width = %lu(0x%lx)\n", W2, W2);

				}
				




			}
					
			
		}
		//reset the radar channel for new counting
		RTMP_DFS_IO_WRITE8(pAd, 0x2, channel);

		
	}

	if (pAd->CommonCfg.McuRadarDebug & RADAR_SIMULATE)
	{
		radarDeclared = 1;
		pAd->CommonCfg.McuRadarDebug &= ~RADAR_SIMULATE;
	}

//Now, find an Radar signal
	if (radarDeclared || pAd->CommonCfg.radarDeclared)
	{
		// Radar found!!!
//Announce that this channel could not use in 30 minutes if we need find a clear channel
		if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DONT_SWITCH))
		{
		
			for (i=0; i<pAd->ChannelListNum; i++)
			{
				if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
				{
					pAd->ChannelList[i].RemainingTimeForUse = 1800;//30 min = 1800 sec
					break;
				}
			}
//when find an radar, the ChMovingTime will be set to announce how many seconds to sending software radar detection time.
			if ((pAd->CommonCfg.RadarDetect.RDDurRegion == CE) && (pAd->CommonCfg.Channel >= 116) && (pAd->CommonCfg.Channel <= 128))
				pAd->CommonCfg.RadarDetect.ChMovingTime = 605;
			else
				pAd->CommonCfg.RadarDetect.ChMovingTime = 65;
		//if the Radar country region is JAP, we need find a new clear channel 
			if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
			{
				for (i = 0; i < pAd->ChannelListNum ; i++)
				{
					pAd->CommonCfg.Channel = APAutoSelectChannel(pAd, FALSE);
					if ((pAd->CommonCfg.Channel >= 100) && (pAd->CommonCfg.Channel <= 140))
						break;
				}
			}
			else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
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
#endif // DOT11_N_SUPPORT //
			//ApSelectChannelCheck(pAd);
			if (pAd->CommonCfg.RadarDetect.RDMode != RD_SILENCE_MODE)
			{
				pAd->CommonCfg.RadarDetect.RDMode = RD_SWITCHING_MODE;
				pAd->CommonCfg.RadarDetect.CSCount = 0;
			}
			else
			{
//set this flag to 1 and the AP will restart to switch into new channel
				pAd->CommonCfg.DFSAPRestart=1;
				schedule_dfs_task(pAd);
				//APStop(pAd);
				//APStartUp(pAd);
			}
				pAd->CommonCfg.radarDeclared = 0;
			
			
		}
		else
			pAd->CommonCfg.radarDeclared = 0;			


		// clear long pulse table
		pAd->CommonCfg.FCC_5[pAd->CommonCfg.fcc_5_idx].counter = 0;
		pAd->CommonCfg.fcc_5_idx = 0;
		pAd->CommonCfg.fcc_5_last_idx = 0;


	}
	
}
void NewRadarDetectionStart(PRTMP_ADAPTER pAd)

{
	int i, index;
	pNewDFSTable pDFS2Table;
	UCHAR DFSR3;
	
	DBGPRINT(RT_DEBUG_TRACE, ("NewRadarDetectionStart2\n"));
	if ((pAd->CommonCfg.RadarDetect.RDDurRegion == CE) && (pAd->CommonCfg.Channel >= 116) && (pAd->CommonCfg.Channel <= 128))
		pAd->CommonCfg.RadarDetect.ChMovingTime = 605;
	else
		pAd->CommonCfg.RadarDetect.ChMovingTime = 65;


	if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
	{
		if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
			pAd->CommonCfg.ch_busy_idle_ratio = 3;
		
		pDFS2Table = &NewDFSTable1[0];
		index = 0;
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use FCC table\n"));
	}
	else if (pAd->CommonCfg.RadarDetect.RDDurRegion == CE)
	{
		if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
			pAd->CommonCfg.ch_busy_idle_ratio = 2;
		
		pDFS2Table = &NewDFSTable1[1];
		index = 1;
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use CE table\n"));
	}
	else // JAP
	{

		if ((pAd->CommonCfg.Channel >= 52) && (pAd->CommonCfg.Channel <= 64))
		{
			pDFS2Table = &NewDFSTable1[3];
			index = 3;
			
			if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
				pAd->CommonCfg.ch_busy_idle_ratio = 2;
		}
		else
		{
		pDFS2Table = &NewDFSTable1[2];
			index = 2;

			if (pAd->CommonCfg.ch_busy_idle_ratio == 0)
				pAd->CommonCfg.ch_busy_idle_ratio = 3;
		}
		DBGPRINT(RT_DEBUG_TRACE,("DFS start, use JAP table\n"));
	}
	
	for(i = 0; i<4; i++)
	{
		if ((pAd->CommonCfg.DFSParamFromConfig & (0x1<<i)) && pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].valid)
		{
			pDFS2Table->entry[i].mode = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].mode;
			pDFS2Table->entry[i].avgLen = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].avgLen;
			pDFS2Table->entry[i].ELow = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].ELow;
			pDFS2Table->entry[i].EHigh = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].EHigh;
			pDFS2Table->entry[i].WLow = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].WLow;
			pDFS2Table->entry[i].WHigh = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].WHigh;
			pDFS2Table->entry[i].EpsilonW = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].EpsilonW;
			pDFS2Table->entry[i].TLow = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].TLow;
			pDFS2Table->entry[i].THigh = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].THigh;
			pDFS2Table->entry[i].EpsilonT = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].EpsilonT;
			pDFS2Table->entry[i].BLow = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].BLow;
			pDFS2Table->entry[i].BHigh = pAd->CommonCfg.NewDFSTableEntry[(index<<2)+i].BHigh;

			DBGPRINT(RT_DEBUG_TRACE, ("index = %d; i = %d; DFSParam = %d; %d; %d; %d; %d; %d; %d; %lu; %lu; %d; %lu; %lu\n", index, i, pDFS2Table->entry[i].mode,
					pDFS2Table->entry[i].avgLen, pDFS2Table->entry[i].ELow, pDFS2Table->entry[i].EHigh,
					pDFS2Table->entry[i].WLow, pDFS2Table->entry[i].WHigh, pDFS2Table->entry[i].EpsilonW,
					pDFS2Table->entry[i].TLow, pDFS2Table->entry[i].THigh, pDFS2Table->entry[i].EpsilonT,
					pDFS2Table->entry[i].BLow, pDFS2Table->entry[i].BHigh));
		}
	}
	
	// Symmetric round
        if(pAd->CommonCfg.SymRoundFromCfg != 0)
        {
                pAd->CommonCfg.Symmetric_Round = pAd->CommonCfg.SymRoundFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("Symmetric_Round = %d\n", pAd->CommonCfg.Symmetric_Round));
        }

        // BusyIdleRatio
        if(pAd->CommonCfg.BusyIdleFromCfg != 0)
        {
                pAd->CommonCfg.ch_busy_idle_ratio = pAd->CommonCfg.BusyIdleFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("ch_busy_idle_ratio = %d\n", pAd->CommonCfg.ch_busy_idle_ratio));
        }
        // DfsRssiHigh
        if(pAd->CommonCfg.DfsRssiHighFromCfg != 0)
        {
                pAd->CommonCfg.DfsRssiHigh = pAd->CommonCfg.DfsRssiHighFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("DfsRssiHigh = %d\n", pAd->CommonCfg.DfsRssiHigh));
        }
        // DfsRssiLow
        if(pAd->CommonCfg.DfsRssiLowFromCfg != 0)
        {
                pAd->CommonCfg.DfsRssiLow = pAd->CommonCfg.DfsRssiLowFromCfg;
                DBGPRINT(RT_DEBUG_TRACE, ("DfsRssiLow = %d\n", pAd->CommonCfg.DfsRssiLow));
        }
	
	pAd->CommonCfg.MCURadarRegion = pAd->CommonCfg.RadarDetect.RDDurRegion;
	
	DFSR3 = pAd->CommonCfg.Symmetric_Round << 4;
	// Full 40Mhz
	if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
	{
		// BW 40
		DFSR3 |= 0x80; 
	}
	
	// Delta Delay
	DFSR3 |= (pAd->CommonCfg.DeltaDelay & 0xf);
	
	
	RTMP_DFS_IO_WRITE8(pAd, 0x3, DFSR3);
	DBGPRINT(RT_DEBUG_TRACE,("R3 = 0x%x\n", DFSR3));
	
	// VGA Mask
	RTMP_DFS_IO_WRITE8(pAd, 0x4, pAd->CommonCfg.VGA_Mask);
	DBGPRINT(RT_DEBUG_TRACE,("VGA_Mask = 0x%x\n", pAd->CommonCfg.VGA_Mask));
	
	// packet end Mask
	RTMP_DFS_IO_WRITE8(pAd, 0x5, pAd->CommonCfg.Packet_End_Mask);
	DBGPRINT(RT_DEBUG_TRACE,("Packet_End_Mask = 0x%x\n", pAd->CommonCfg.Packet_End_Mask));
	
	// Rx PE Mask
	RTMP_DFS_IO_WRITE8(pAd, 0x6, pAd->CommonCfg.Rx_PE_Mask);
	DBGPRINT(RT_DEBUG_TRACE,("Rx_PE_Mask = 0x%x\n", pAd->CommonCfg.Rx_PE_Mask));

	// program each channel
	for (i = 0; i < NEW_DFS_MAX_CHANNEL; i++)
	{
		// select channel
		RTMP_DFS_IO_WRITE8(pAd, 0x0, i);

		printk("write DFS Channle[%d] configuration \n",i);
		// start programing

		// reg 0x10, Detection Mode[2:0]
		RTMP_DFS_IO_WRITE8(pAd, 0x10, (pDFS2Table->entry[i].mode & 0x7));
		
		// reg 0x11~0x12, M[7:0] & M[8]
		RTMP_DFS_IO_WRITE8(pAd, 0x11, (pDFS2Table->entry[i].avgLen & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x12, ((pDFS2Table->entry[i].avgLen >> 8) & 0x1));


		// reg 0x13~0x14, Energy Low[7:0] & Energy Low[11:8]
		RTMP_DFS_IO_WRITE8(pAd, 0x13, (pDFS2Table->entry[i].ELow & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x14, ((pDFS2Table->entry[i].ELow >> 8) & 0xf));

		
		// reg 0x15~0x16, Energy High[7:0] & Energy High[11:8]
		RTMP_DFS_IO_WRITE8(pAd, 0x15, (pDFS2Table->entry[i].EHigh & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x16, ((pDFS2Table->entry[i].EHigh >> 8) & 0xf));

		
		// reg 0x28~0x29, Width Low[7:0] & Width Low[11:8]
		RTMP_DFS_IO_WRITE8(pAd, 0x28, (pDFS2Table->entry[i].WLow & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x29, ((pDFS2Table->entry[i].WLow >> 8) & 0xf));

		// reg 0x2a~0x2b, Width High[7:0] & Width High[11:8]
		RTMP_DFS_IO_WRITE8(pAd, 0x2a, (pDFS2Table->entry[i].WHigh & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x2b, ((pDFS2Table->entry[i].WHigh >> 8) & 0xf));

		// reg 0x2c, Width Delta[7:0], (Width Measurement Uncertainty)
		RTMP_DFS_IO_WRITE8(pAd, 0x2c, (pDFS2Table->entry[i].EpsilonW & 0xff));

		// reg 0x17~0x1a, Period Low[7:0] & Period Low[15:8] & Period Low[23:16] & Period Low[31:24]
		RTMP_DFS_IO_WRITE8(pAd, 0x17, (pDFS2Table->entry[i].TLow & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x18, ((pDFS2Table->entry[i].TLow >> 8) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x19, ((pDFS2Table->entry[i].TLow >> 16) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1a, ((pDFS2Table->entry[i].TLow >> 24) & 0xff));

		// reg 0x1b~0x1e, Period High[7:0] & Period High[15:8] & Period High[23:16] & Period High[31:24]
		RTMP_DFS_IO_WRITE8(pAd, 0x1b, (pDFS2Table->entry[i].THigh & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1c, ((pDFS2Table->entry[i].THigh >> 8) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1d, ((pDFS2Table->entry[i].THigh >> 16) & 0xff));
		RTMP_DFS_IO_WRITE8(pAd, 0x1e, ((pDFS2Table->entry[i].THigh >> 24) & 0xff));

		// reg 0x27, Period Delt[7:0], (Period Measurement Uncertainty)
		RTMP_DFS_IO_WRITE8(pAd, 0x27, (pDFS2Table->entry[i].EpsilonT & 0xff));
		
		if (pAd->CommonCfg.RadarEventExpire[i] != 0)
		{
			RTMP_DFS_IO_WRITE8(pAd,0x39, (pAd->CommonCfg.RadarEventExpire[i] & 0xff));
			RTMP_DFS_IO_WRITE8(pAd,0x3a, ((pAd->CommonCfg.RadarEventExpire[i] >> 8) & 0xff) );
	 		RTMP_DFS_IO_WRITE8(pAd,0x3b, ((pAd->CommonCfg.RadarEventExpire[i] >> 16) & 0xff));
			RTMP_DFS_IO_WRITE8(pAd,0x3c, ((pAd->CommonCfg.RadarEventExpire[i] >> 24) & 0xff));
		}
		
	}   
#ifdef DFS_HWTIMER_SUPPORT
	// enable debug mode
	BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R126, 3);
#endif // DFS_HWTIMER_SUPPORT //
	// Enable detection
	RTMP_DFS_IO_WRITE8(pAd, 0x1, (pAd->CommonCfg.ChEnable & 0xf));
//the usage of dfs_sw_init
	dfs_sw_init(pAd);
#ifdef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
	request_tmr_service(NEW_DFS_WATCH_DOG_TIME, &NewTimerCB_Radar, (void*)pAd);
#else
	if(pAd->CommonCfg.DFSWatchDogIsRunning==FALSE)
	{

		UINT32 Value;
		// Hardware Period Timer interrupt setting.
		RTMP_IO_READ32(pAd, INT_TIMER_CFG, &Value);
		Value &= 0x0000ffff;
		Value |= 1 << 20; // Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, Value);
		// Enable Hardware Period Timer interrupt
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
		Value |=0x2;
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
		pAd->CommonCfg.DFSWatchDogIsRunning=TRUE;
	}
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_HWTIMER_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE,("Poll Time=%d\n", pAd->CommonCfg.PollTime));
}

VOID NewRadarDetectionStop(
	IN PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("NewRadarDetectionStop2\n"));
	// Disable detection
	RTMP_DFS_IO_WRITE8(pAd, 0x1, 0);
#ifdef DFS_HWTIMER_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
	unregister_tmr_service();
#else
	{
		UINT32 Value;
		// Hardware Period Timer interrupt setting.
		RTMP_IO_READ32(pAd, INT_TIMER_CFG, &Value);
		Value &= 0x0000ffff;
		RTMP_IO_WRITE32(pAd, INT_TIMER_CFG, Value);
		// Enable Hardware Period Timer interrupt
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
		Value &=~(0x2);
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	pAd->CommonCfg.DFSWatchDogIsRunning=FALSE;
	}
#endif // RTMP_RBUS_SUPPORT //
#endif // DFS_HWTIMER_SUPPORT //
}


// the debug port have timestamp 22 digit, the max number is 0x3fffff, each unit is 25ns for 40Mhz mode and 50ns for 20Mhz mode
// so a round of timestamp is about 25 * 0x3fffff / 1000 = 104857us (about 100ms) or
// 50 * 0x3fffff / 1000 = 209715us (about 200ms) in 20Mhz mode
// 3ms = 3000,000 ns / 25ns = 120000 -- a unit 
// 0x3fffff/120000 = 34.9 ~= 35
// CE Staggered radar check
// At beginning, the goal is to detect staggered radar, now, we also detect regular radar with this function.


int SWRadarCheck(
	IN PRTMP_ADAPTER pAd, USHORT id)
{
	int i, j, k, start_idx, end_idx;
	pNewDFSDebugPort pCurrent, p1, pEnd;
	ULONG period;
	int radar_detected = 0;
	USHORT	widthsum;
	//ENTRY_PLUS could be replace by (pAd->CommonCfg.sw_idx[id]+1)%128
	USHORT	Total, SwIdxPlus = ENTRY_PLUS(pAd->CommonCfg.sw_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	UCHAR	CounterToCheck;
	
	

	if (SwIdxPlus == pAd->CommonCfg.hw_idx[id])
		return 0; // no entry to process
	
	// process how many entries?? total NEW_DFS_DBG_PORT_ENT_NUM
	if (pAd->CommonCfg.hw_idx[id] > SwIdxPlus)
		Total = pAd->CommonCfg.hw_idx[id] - SwIdxPlus;
	else
		Total = pAd->CommonCfg.hw_idx[id] + NEW_DFS_DBG_PORT_ENT_NUM - SwIdxPlus;
	
	if (Total > NEW_DFS_DBG_PORT_ENT_NUM)
		pAd->CommonCfg.pr_idx[id] = ENTRY_PLUS(pAd->CommonCfg.sw_idx[id], MAX_PROCESS_ENTRY, NEW_DFS_DBG_PORT_ENT_NUM);
	else
		pAd->CommonCfg.pr_idx[id] = ENTRY_PLUS(pAd->CommonCfg.sw_idx[id], Total, NEW_DFS_DBG_PORT_ENT_NUM);
	
	
	start_idx = ENTRY_PLUS(pAd->CommonCfg.pr_idx[id], 1, NEW_DFS_DBG_PORT_ENT_NUM);
	end_idx = pAd->CommonCfg.pr_idx[id];
	
	pEnd = &pAd->CommonCfg.DFS_W[id][end_idx];
	//printk("start_idx = %d, end_idx=%d, counter=%d\n", start_idx, end_idx, pEnd->counter);
	
	//if (pAd->CommonCfg.dfs_w_counter != pEnd->counter)
	//	return 0;
	
	if (start_idx > end_idx)
		end_idx += NEW_DFS_DBG_PORT_ENT_NUM;
	
	
	pAd->CommonCfg.sw_idx[id] = pAd->CommonCfg.pr_idx[id];
	
	// FCC && Japan

	if (pAd->CommonCfg.RadarDetect.RDDurRegion != CE)
	{
		ULONG minPeriod = (3000 << 1);
		// Calculate how many counters to check
		// if pAd->CommonCfg.PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz
		// if pAd->CommonCfg.PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz
		// if pAd->CommonCfg.PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz
		// if pAd->CommonCfg.PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz
		// the max period to check for 40Mhz for FCC is 28650 * 2
		// the max period to check for 40Mhz for Japan is 80000 * 2
		// 0x40000 = 4194304 / 57129 = 73.xxx
		// 0x40000 = 4194304 / 160000 = 26.2144
		// 53/73 < 1 (1+1)
		// 53/26.2144 = 2.02... (2+1)
		// 27/26.2144 = 1.02... (1+1)
		// 20M should use the same value as 40Mhz mode


		if (pAd->CommonCfg.MCURadarRegion == NEW_DFS_JAP_W53)
		{
			minPeriod = 28500 << 1;
		}
		
		
		if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
		{
			if (pAd->CommonCfg.PollTime > 5)
				CounterToCheck = 1;
			else
			CounterToCheck = 1+1; 
		}
		else // if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP)
		{
			if (pAd->CommonCfg.PollTime <= 2)
				CounterToCheck = 2+1;
			else
				if (pAd->CommonCfg.PollTime > 5)
					CounterToCheck = 1;
				else
				CounterToCheck = 1+1;
		}
		

		
		// First Loop for FCC/JAP
		for (i = end_idx; i > start_idx; i--)
		{
			pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			// we only handle entries has same counter with the last one
			if (pCurrent->counter != pEnd->counter)
				break;
		
			pCurrent->start_idx = 0xffff;

			// calculate if any two pulse become a valid period, add it in period table,
			for (j = i - 1; j > start_idx; j--)
			{
				p1 = &pAd->CommonCfg.DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];
				
				// check period, must within max period
				if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
				{
					if (p1->counter + CounterToCheck < pCurrent->counter)
						break;
            	
					widthsum = p1->width + pCurrent->width;
					if (id == 0)
					{
						if (widthsum < 600)
							pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
						else
							pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
					
					//if ( (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE) ||
						// (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff)) )
					if (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff))
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
						
						if ((period >= (minPeriod - 2)) && (period <= pAd->CommonCfg.dfs_max_period))
						{
            	
							// add in period table
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
            	
            	
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
							pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
							
							pAd->CommonCfg.dfs_t_idx[id]++;
							if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pAd->CommonCfg.dfs_t_idx[id] = 0;
						}
						else if (period > pAd->CommonCfg.dfs_max_period)
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
							pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
						else
							pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
					}
					else if (id == 1)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch1_Shift;
					else if (id == 2)
						pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;

            	
					//if ( (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE) || 
						// (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff)) )
						if (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff))
            	
					{
						if (p1->timestamp >= pCurrent->timestamp)
							period = 0x400000 + pCurrent->timestamp - p1->timestamp;
						else
							period = pCurrent->timestamp - p1->timestamp;
            	
						if ((period >= ((minPeriod >> 1) - 2)) && (period <= (pAd->CommonCfg.dfs_max_period >> 1)))
						{
							// add in period table
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
							pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
							
							if (pCurrent->start_idx == 0xffff)
								pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
							pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
							
							pAd->CommonCfg.dfs_t_idx[id]++;
							if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
								pAd->CommonCfg.dfs_t_idx[id] = 0;
						}
						else if (period > (pAd->CommonCfg.dfs_max_period >> 1))
							break;
					}
				}

			} // for (j = i - 1; j > start_idx; j--)

		} // for (i = end_idx; i > start_idx; i--)


		// Second Loop for FCC/JAP
		for (i = end_idx; i > start_idx; i--)
		{
			
			pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
			// we only handle entries has same counter with the last one
			if (pCurrent->counter != pEnd->counter)
				break;
			if (pCurrent->start_idx != 0xffff)
			{
				//pNewDFSDebugPort	p2, p3, p4, p5, p6;
				pNewDFSDebugPort	p2, p3;
				pNewDFSMPeriod pCE_T;
				ULONG idx[10], T[10];

				for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
				{

					pCE_T = &pAd->CommonCfg.DFS_T[id][idx[0]];
				
					p2 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];
				
					if (p2->start_idx == 0xffff)
						continue;
				
					T[0] = pCE_T->period;


					for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
					{
						
						pCE_T = &pAd->CommonCfg.DFS_T[id][idx[1]];
					
						p3 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];

						if (idx[0] == idx[1])
							continue;
						
						if (p3->start_idx == 0xffff)
							continue;
					


						T[1] = pCE_T->period;
						
						
						if ( PERIOD_MATCH(T[0], T[1], pAd->CommonCfg.dfs_period_err))
						{
							if (id <= 2) // && (id >= 0)
							{

								//if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > minPeriod)) ||
								//	((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > (minPeriod >> 1))) )
								{
									unsigned int loop, PeriodMatched = 0, idx1;
									for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
									{
										idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
										if (PERIOD_MATCH(pAd->CommonCfg.DFS_T[id][idx1].period, T[1], pAd->CommonCfg.dfs_period_err))
										{
#ifdef DFS_DEBUG
											if (PeriodMatched < 5)
											{
												pAd->CommonCfg.CounterStored[PeriodMatched] = pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.DFS_T[id][idx1].idx].counter;
												pAd->CommonCfg.CounterStored2[PeriodMatched] = loop;
												pAd->CommonCfg.CounterStored3 = idx[1];
											}
#endif
											//printk("%d %d\n", loop, pAd->CommonCfg.DFS_T[id][idx[1]-loop].period);
											PeriodMatched++;
										}
										
									}
								
								
									if (PeriodMatched > pAd->CommonCfg.dfs_declare_thres)
									{
#ifdef DFS_DEBUG
										if (PeriodMatched == 3)
										{
											pAd->CommonCfg.T_Matched_3++;
											//printk("counter=%d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2]);
											//printk("idx[1]=%d, loop =%d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2]);
										}
										else if (PeriodMatched == 4)
										{
											pAd->CommonCfg.T_Matched_4++;
											//printk("counter=%d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3]);
											//printk("idx[1]=%d, loop =%d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3]);
										}
										else
										{
											pAd->CommonCfg.T_Matched_5++;
											//printk("counter=%d %d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3], pAd->CommonCfg.CounterStored[4]);
											//printk("idx[1]=%d, loop =%d %d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3], pAd->CommonCfg.CounterStored2[4]);
										}
                                    	
										pAd->CommonCfg.DebugPortPrint = 1;
									
#endif

										{
											pNewDFSValidRadar pDFSValidRadar;
											ULONG T1 = (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)? (T[1]>>1) : T[1];
											
											pDFSValidRadar = &NewDFSValidTable[0];
											
                    					
											while (pDFSValidRadar->type != NEW_DFS_END)
											{
												if ((pDFSValidRadar->type & pAd->CommonCfg.MCURadarRegion) == 0)
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
												if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SILENCE))
												{
													if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SW_SILENCE))
														printk("ch%d W=%d T=%d sw=%d t=%ld\n", (unsigned int)id, (unsigned int)pCE_T->width, (unsigned int)T1, PeriodMatched, pAd->CommonCfg.RadarTimeStampLow);
													//printk("Radar Detected\n");
												}
												return radar_detected;
											}
											else if (pAd->CommonCfg.MCURadarRegion != NEW_DFS_JAP_W53)
													if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SILENCE))
													if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SW_SILENCE))
														printk("ch%d W=%d T=%d sw=%d, t=%ld\n", (unsigned int)id, (unsigned int)pCE_T->width, (unsigned int)T1, PeriodMatched, pAd->CommonCfg.RadarTimeStampLow);
										}

										
									}
#ifdef DFS_DEBUG
									else if (PeriodMatched == 2)
									{
										pAd->CommonCfg.T_Matched_2++;
									}
#endif
								
								
								}
								
							} // if (id <= 2) // && (id >= 0)
							
						}
						
					} // for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)


					// increase FCC-1 detection
					if (id <= 2)
					{
						if (IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
								int loop, idx1, PeriodMatched_fcc1 = 0;
								for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
								{
									idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
									if ( IS_FCC_RADAR_1((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pAd->CommonCfg.DFS_T[id][idx1].period) )
									{
										//printk("%d %d %d\n", PeriodMatched_fcc1, pAd->CommonCfg.DFS_T[id][idx1].period, loop);
										PeriodMatched_fcc1++;
									}
								}
									
								if (PeriodMatched_fcc1 > 3)
								{
										if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SILENCE))
										if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SW_SILENCE))
											printk("ch%d fcc1=%d t=%ld\n",  id, PeriodMatched_fcc1, pAd->CommonCfg.RadarTimeStampLow);
									radar_detected = 1;
									return radar_detected;
								}
								
						}
						
					}


					// increase W56-3 detection
					if ((pAd->CommonCfg.MCURadarRegion == NEW_DFS_JAP) && (id <= 2))
					{
						if (IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
						{
								int loop, idx1, PeriodMatched_w56_3 = 0;
								for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
								{
									idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
									if ( IS_W56_RADAR_3((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pAd->CommonCfg.DFS_T[id][idx1].period) )
									{
										//printk("%d %d %d\n", PeriodMatched_w56_3, pAd->CommonCfg.DFS_T[id][idx1].period, loop);
										PeriodMatched_w56_3++;
									}
								}
									
								if (PeriodMatched_w56_3 > 3)
								{
									if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SILENCE))
										if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SW_SILENCE))
											printk("ch%d w56_3=%d t=%ld\n", id, PeriodMatched_w56_3, pAd->CommonCfg.RadarTimeStampLow);
									radar_detected = 1;
									return radar_detected;
								}
								
						}
						
					}


					if ((pAd->CommonCfg.MCURadarRegion == NEW_DFS_JAP_W53) && (id <= 2) && IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), T[0]))
					{
						int loop, idx1, PeriodMatched_W56_2 = 0;
						
						for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
						{
							idx1 = (idx[0] >= loop)? (idx[0] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[0] - loop);
							if ( IS_W53_RADAR_2((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40), pAd->CommonCfg.DFS_T[id][idx1].period) )
							{
								//printk("%d %d %d\n", PeriodMatched_W56_2, pAd->CommonCfg.DFS_T[id][idx1].period, loop);
								PeriodMatched_W56_2++;
							}
						}
						
						if (PeriodMatched_W56_2 >= 3)
						{
								if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SILENCE))
								if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SW_SILENCE))
									printk("ch%d W56_2=%d t=%ld\n", id, PeriodMatched_W56_2, pAd->CommonCfg.RadarTimeStampLow);
							radar_detected = 1;
							return radar_detected;
						}
					}



				} // for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
			} // if (pCurrent->start_idx != 0xffff)
		} // for (i = end_idx; i > start_idx; i--)
		
		return radar_detected;
	}

	// CE have staggered radar	
	
	// Calculate how many counters to check
	// if pAd->CommonCfg.PollTime is 1ms, a round of timestamp is 107 for 20Mhz, 53 for 40Mhz
	// if pAd->CommonCfg.PollTime is 2ms, a round of timestamp is 71 for 20Mhz, 35 for 40Mhz
	// if pAd->CommonCfg.PollTime is 3ms, a round of timestamp is 53 for 20Mhz, 27 for 40Mhz
	// if pAd->CommonCfg.PollTime is 4ms, a round of timestamp is 43 for 20Mhz, 21 for 40Mhz
	// if pAd->CommonCfg.PollTime is 8ms, a round of timestamp is ?? for 20Mhz, 12 for 40Mhz
	// the max period to check for 40Mhz is 133333 + 125000 + 117647 = 375980
	// 0x40000 = 4194304 / 375980 = 11.1556
	// 53/11.1556 = 4.75...
	// 35/11.1556 = 3.1374, (4+1) is safe, (3+1) to save CPU power, but may lost some data
	// 27/11.1556 = 2.42, (3+1) is OK
	// 21/11.1556 = 1.88, (2+1) is OK
	// 20M should use the same value as 40Mhz mode
	if (pAd->CommonCfg.PollTime == 1)
		CounterToCheck = 5+1;
	else if (pAd->CommonCfg.PollTime == 2)
		CounterToCheck = 4+1;
	else if (pAd->CommonCfg.PollTime == 3)
		CounterToCheck = 3+1;
	else if (pAd->CommonCfg.PollTime <= 8)
		CounterToCheck = 2+1;
	else if (pAd->CommonCfg.PollTime <= 12)
		CounterToCheck = 1+1;
	else
		CounterToCheck = 1;

	// First Loop for CE
	for (i = end_idx; i > start_idx; i--)
	{
		int jj;
		pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		// we only handle entries has same counter with the last one
		if (pCurrent->counter != pEnd->counter)
			break;
		
		pCurrent->start_idx = 0xffff;

		// calculate if any two pulse become a valid period, add it in period table,
		// for (j = i - 1; j > start_idx; j--)
		// go through all entries untill meet previous counter	
		for (jj = 0; jj < NEW_DFS_DBG_PORT_ENT_NUM; jj++)
		{
			if (i > (jj + 1))
				j = (i - 1) - jj;
			else	
				j = NEW_DFS_DBG_PORT_ENT_NUM + (i - 1) - jj;
			
			p1 = &pAd->CommonCfg.DFS_W[id][j & NEW_DFS_DBG_PORT_MASK];

			// check period, must within 16666 ~ 66666
			if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
			{
				if (p1->counter + CounterToCheck < pCurrent->counter)
						break;

				widthsum = p1->width + pCurrent->width;
				if (id == 0)
				{
					//if (((p1->width > 310) && (pCurrent->width < 300)) || ((pCurrent->width > 310) && ((p1->width < 300))) )
						//continue;
					//if (widthsum < 620)
						//pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_H;
					//else
						pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
					
				}
				else if (id == 1)
					pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch1_Shift;
				else if (id == 2)
					pAd->CommonCfg.dfs_width_diff = widthsum >> pAd->CommonCfg.dfs_width_diff_ch2_Shift;
				
				if (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff))
				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;
					
					//if ((period >= (33333 - 20)) && (period <= (133333 + 20)))
					if ((period >= (10000 - 2)) && (period <= pAd->CommonCfg.dfs_max_period))
					{

						// add in period table
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
        
        
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
						pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
							// Improve CE-5 and CE-6 radar here
							if (id <= pAd->CommonCfg.ce_sw_id_check)
							{
								ULONG kk, kk_index, t_diff, found=0;
								pNewDFSDebugPort pThis;
								for (kk = (NEW_DFS_MPERIOD_ENT_NUM-1); kk > 0; kk--)
								{
									kk_index = (kk + pAd->CommonCfg.dfs_t_idx[id]) & (NEW_DFS_MPERIOD_ENT_NUM - 1);
									pThis = &pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.DFS_T[id][kk_index].idx];
									if (id == 0)
										t_diff = 4;
									else
										t_diff = (period + pAd->CommonCfg.DFS_T[id][kk_index].period) >> pAd->CommonCfg.ce_sw_t_diff;
									if (MATCH_OR_DOUBLE(period, pAd->CommonCfg.DFS_T[id][kk_index].period, t_diff))
									{
										//printk("t=%d, w=%d %d\n",period, p1->width, pCurrent->width);
										//printk("t=%d, w=%d %d\n",pAd->CommonCfg.DFS_T[id][kk_index].period, pAd->CommonCfg.DFS_T[id][kk_index].width, pAd->CommonCfg.DFS_T[id][kk_index].width2);
										if (PERIOD_MATCH(p1->width, pAd->CommonCfg.DFS_T[id][kk_index].width, pAd->CommonCfg.dfs_width_diff))
										{
											found++;	
											//printk("found = %d\n", found);
										}
									}
									if (pThis->counter + CounterToCheck < pCurrent->counter)
										break;
								}
								if (found >= pAd->CommonCfg.ce_sw_check)
								{
									printk("T=%lx, W=%ud, %lx\n",period, p1->width, found);
									radar_detected = 1;
								}
							}
						
						pAd->CommonCfg.dfs_t_idx[id]++;
						if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pAd->CommonCfg.dfs_t_idx[id] = 0;
					}
					else if (period > pAd->CommonCfg.dfs_max_period) // to allow miss a pulse
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
					//if (((p1->width > 300) && (pCurrent->width < 300)) || ((pCurrent->width > 300) && ((p1->width < 300))) )
					//	continue;
					//if (widthsum < 620)
					//	pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_H;
					//else
						pAd->CommonCfg.dfs_width_diff = pAd->CommonCfg.dfs_width_ch0_err_L;
				}
				else if (id == 1)
					pAd->CommonCfg.dfs_width_diff = widthsum >> 4;
				else if (id == 2)
					pAd->CommonCfg.dfs_width_diff = widthsum >> 6;

				if (PERIOD_MATCH(p1->width, pCurrent->width, pAd->CommonCfg.dfs_width_diff))

				{
					if (p1->timestamp >= pCurrent->timestamp)
						period = 0x400000 + pCurrent->timestamp - p1->timestamp;
					else
						period = pCurrent->timestamp - p1->timestamp;

					if ((period >= (5000 - 2)) && (period <= (pAd->CommonCfg.dfs_max_period >> 1)))
					{
						// add in period table
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx = (i & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width = pCurrent->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].idx2 = (j & NEW_DFS_DBG_PORT_MASK);
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].width2 = p1->width;
						pAd->CommonCfg.DFS_T[id][pAd->CommonCfg.dfs_t_idx[id]].period = period;
						
						if (pCurrent->start_idx == 0xffff)
							pCurrent->start_idx = pAd->CommonCfg.dfs_t_idx[id];
						pCurrent->end_idx = pAd->CommonCfg.dfs_t_idx[id];
						// Improve CE-5 and CE-6 radar here
						if (id <= pAd->CommonCfg.ce_sw_id_check)
						{
							ULONG kk, kk_index, t_diff,found=0;
							pNewDFSDebugPort pThis;
							for (kk = (NEW_DFS_MPERIOD_ENT_NUM-1); kk > 0; kk--)
							{
								kk_index = (kk + pAd->CommonCfg.dfs_t_idx[id]) & (NEW_DFS_MPERIOD_ENT_NUM - 1);
								pThis = &pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.DFS_T[id][kk_index].idx];
								if (id == 0)
									t_diff = 4;
								else
									t_diff = (period + pAd->CommonCfg.DFS_T[id][kk_index].period) >> pAd->CommonCfg.ce_sw_t_diff;
								
								if (MATCH_OR_DOUBLE(period, pAd->CommonCfg.DFS_T[id][kk_index].period, t_diff))
								{
									//printk("T=%d, w=%d %d\n",period, p1->width, pCurrent->width);
									//printk("T=%d, w=%d %d\n",pAd->CommonCfg.DFS_T[id][kk_index].period, pAd->CommonCfg.DFS_T[id][kk_index].width, pAd->CommonCfg.DFS_T[id][kk_index].width2);
									if (PERIOD_MATCH(p1->width, pAd->CommonCfg.DFS_T[id][kk_index].width, pAd->CommonCfg.dfs_width_diff))
									{
										found++;
										//printk("found = %d\n", found);
									}
								}
								if (pThis->counter + CounterToCheck < pCurrent->counter)
									break;
							}													
							if (found >= pAd->CommonCfg.ce_sw_check)
							{
								printk("T=%lx, W=%d, %lx\n",period, p1->width, found);
								radar_detected = 1;
							}
						}

						
						pAd->CommonCfg.dfs_t_idx[id]++;
						if (pAd->CommonCfg.dfs_t_idx[id] >= NEW_DFS_MPERIOD_ENT_NUM)
							pAd->CommonCfg.dfs_t_idx[id] = 0;
					}
					else if (period > (pAd->CommonCfg.dfs_max_period >> 1))
						break;
				}
			}
			
		} // for (j = i - 1; j > start_idx; j--)
	}

	// Second Loop for CE
	for (i = end_idx; i > start_idx; i--)
	{
		pCurrent = &pAd->CommonCfg.DFS_W[id][i & NEW_DFS_DBG_PORT_MASK];
				
		// we only handle entries has same counter with the last one
		if (pCurrent->counter != pEnd->counter)
			break;
		
		// Check Staggered radar
		if (pCurrent->start_idx != 0xffff)
		{
			pNewDFSDebugPort	p2, p3;
			pNewDFSMPeriod pCE_T;
			ULONG idx[10], T[10];
			
			//printk("pCurrent=%d, idx=%d~%d\n", pCurrent->timestamp, pCurrent->start_idx, pCurrent->end_idx);

			for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)
			{
				pCE_T = &pAd->CommonCfg.DFS_T[id][idx[0]];
				
				p2 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];
				
				//printk("idx[0]= %d, idx=%d p2=%d, idx=%d~%d\n", idx[0], pCE_T->idx2, p2->timestamp, p2->start_idx, p2->end_idx);
				
				if (p2->start_idx == 0xffff)
					continue;
				
				T[0] = pCE_T->period;


				for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)
				{
					
					pCE_T = &pAd->CommonCfg.DFS_T[id][idx[1]];
					
					p3 = &pAd->CommonCfg.DFS_W[id][pCE_T->idx2];
					
					//printk("p3=%d, idx=%d~%d\n", p3->timestamp, p3->start_idx, p3->end_idx);

					if (idx[0] == idx[1])
						continue;
						
					if (p3->start_idx == 0xffff)
						continue;
					


					T[1] = pCE_T->period;

		
					if (PERIOD_MATCH(T[0], T[1], pAd->CommonCfg.dfs_period_err))
					{
						if (id <= 2) // && (id >= 0)
						{

							
							if (((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && (T[1] > 66666)) ||
								((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_20) && (T[1] > 33333)) )
							{
								unsigned int loop, PeriodMatched = 0, idx1;
								
								for (loop = 1; loop < pAd->CommonCfg.dfs_check_loop; loop++)
								{
									idx1 = (idx[1] >= loop)? (idx[1] - loop): (NEW_DFS_MPERIOD_ENT_NUM + idx[1] - loop);
									if (PERIOD_MATCH(pAd->CommonCfg.DFS_T[id][idx1].period, T[1], pAd->CommonCfg.dfs_period_err))
									{
#ifdef DFS_DEBUG
										if (PeriodMatched < 5)
										{
											pAd->CommonCfg.CounterStored[PeriodMatched] = pAd->CommonCfg.DFS_W[id][pAd->CommonCfg.DFS_T[id][idx1].idx].counter;
											pAd->CommonCfg.CounterStored2[PeriodMatched] = loop;
											pAd->CommonCfg.CounterStored3 = idx[1];
										}
#endif
										//printk("%d %d\n", loop, pAd->CommonCfg.DFS_T[id][idx[1]-loop].period);
										PeriodMatched++;
									}
									
								}
								
								
								if (PeriodMatched > pAd->CommonCfg.dfs_declare_thres)
								{
#ifdef DFS_DEBUG
									if (PeriodMatched == 3)
									{
										pAd->CommonCfg.T_Matched_3++;
										//printk("counter=%d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2]);
										//printk("idx[1]=%d, loop =%d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2]);
									}
									else if (PeriodMatched == 4)
									{
										pAd->CommonCfg.T_Matched_4++;
										//printk("counter=%d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3]);
										//printk("idx[1]=%d, loop =%d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3]);
									}
									else
									{
										pAd->CommonCfg.T_Matched_5++;
										//printk("counter=%d %d %d %d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1], pAd->CommonCfg.CounterStored[2], pAd->CommonCfg.CounterStored[3], pAd->CommonCfg.CounterStored[4]);
										//printk("idx[1]=%d, loop =%d %d %d %d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1], pAd->CommonCfg.CounterStored2[2], pAd->CommonCfg.CounterStored2[3], pAd->CommonCfg.CounterStored2[4]);
									}

									pAd->CommonCfg.DebugPortPrint = 1;
#endif

									if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SILENCE))
										if (!(pAd->CommonCfg.McuRadarDebug & RADAR_DEBUG_SW_SILENCE))
											printk("ch%d W=%d, T=%d sw=%d, t=%ld\n", (unsigned int)id, (unsigned int)pCE_T->width, (unsigned int)T[1], PeriodMatched, pAd->CommonCfg.RadarTimeStampLow);
									if (PeriodMatched > (pAd->CommonCfg.dfs_declare_thres + 1))
 								      		radar_detected = 1;
									return radar_detected;
								}
#ifdef DFS_DEBUG
								else if (PeriodMatched == 2)
								{
									pAd->CommonCfg.T_Matched_2++;
									//printk("counter=%d %d\n", pAd->CommonCfg.CounterStored[0], pAd->CommonCfg.CounterStored[1]);
									//printk("idx[1]=%d, loop =%d %d\n", pAd->CommonCfg.CounterStored3, pAd->CommonCfg.CounterStored2[0], pAd->CommonCfg.CounterStored2[1]);
								}
#endif
								
								
							}
						}
						
					}

				} // for (idx[1] = p2->start_idx; idx[1] <= p2->end_idx; idx[1]++)

			} // for (idx[0] = pCurrent->start_idx; idx[0] <= pCurrent->end_idx; idx[0]++)

		}
		
	} // for (i = end_idx; i < start_idx; i--)
	
	
	return radar_detected;
	
}


#endif // DFS_2_SUPPORT //

static void dfs_sw_init(PRTMP_ADAPTER pAd)
{
	
	int j, k;

	for (k = 0; k < NEW_DFS_MAX_CHANNEL; k++)
	{
		for (j = 0; j < NEW_DFS_DBG_PORT_ENT_NUM; j++)
		{
			pAd->CommonCfg.DFS_W[k][j].start_idx = 0xffff;
		}
	}

}

void 	modify_table1(PRTMP_ADAPTER pAd, ULONG idx, ULONG value)
{
	pNewDFSTable pDFS2Table;
	ULONG x, y;	
	

	if (pAd->CommonCfg.RadarDetect.RDDurRegion == FCC)
		pDFS2Table = &NewDFSTable1[0];
	else if (pAd->CommonCfg.RadarDetect.RDDurRegion == CE)
	{
		pDFS2Table = &NewDFSTable1[1];
	}
	else // Japan
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
		pAd->CommonCfg.DeltaDelay = value;
	}
	else if (idx <= 40)
	{


		idx--;
	
		x = idx / 10;
		y = idx % 10;

		switch (y)
		{
		case 0:
			pDFS2Table->entry[x].mode = (USHORT)value;
			break;
		case 1:
			pDFS2Table->entry[x].avgLen = (USHORT)value;
			break;
		case 2:
			pDFS2Table->entry[x].ELow = (USHORT)value;
			break;
    	
		case 3:
			pDFS2Table->entry[x].EHigh = (USHORT)value;
			break;
    	
		case 4:
			pDFS2Table->entry[x].WLow = (USHORT)value;
			break;
    	
		case 5:
			pDFS2Table->entry[x].WHigh = (USHORT)value;
			break;
    	
		case 6:
			pDFS2Table->entry[x].EpsilonW = (USHORT)value;
			break;
    	
		case 7:
			pDFS2Table->entry[x].TLow = (ULONG)value;
			break;
    	
		case 8:
			pDFS2Table->entry[x].THigh = (ULONG)value;
			break;
    	
		case 9:
			pDFS2Table->entry[x].EpsilonT = (USHORT)value;
			break;
		default:
			break;
		}
    	
	}
#ifdef DFS_2_SUPPORT
#ifdef CONFIG_RALINK_RT2880
	printk("Delta_Delay(0) = %d\n", pAd->CommonCfg.DeltaDelay);
	for (x = 0; x < 4; x++)
		printk("mode(%02d)=%d, M(%02d)=%03d, EL(%02d)=%03d EH(%02d)=%03d, WL(%02d)=%03d WH(%02d)=%04d, eW(%02d)=%02d, TL(%02d)=%05d TH(%02d)=%06d, eT(%02d)=%03d\n", 
		(x*10+1), (unsigned int)pDFS2Table->entry[x].mode, 
		(x*10+2), (unsigned int)pDFS2Table->entry[x].avgLen, 
		(x*10+3), (unsigned int)pDFS2Table->entry[x].ELow, 
		(x*10+4), (unsigned int)pDFS2Table->entry[x].EHigh, 
		(x*10+5), (unsigned int)pDFS2Table->entry[x].WLow, 
		(x*10+6), (unsigned int)pDFS2Table->entry[x].WHigh, 
		(x*10+7), (unsigned int)pDFS2Table->entry[x].EpsilonW, 
		(x*10+8), (unsigned int)pDFS2Table->entry[x].TLow, 
		(x*10+9), (unsigned int)pDFS2Table->entry[x].THigh, 
		(x*10+10), (unsigned int)pDFS2Table->entry[x].EpsilonT);
#else // CONFIG_RALINK_RT2880
	else if (idx <= 48)
	{
		y = idx & 0x1;
		x = (idx - 40) >> 1;
		
		if (y)
		{
			pDFS2Table->entry[x].BLow = (ULONG)value;
		}
		else
		{
			pDFS2Table->entry[x-1].BHigh = (ULONG)value;
		}
		
	}
	else if (idx == 49)
	{
		pAd->CommonCfg.Symmetric_Round = (ULONG)value;
	}
	else if (idx == 50)
	{
		pAd->CommonCfg.VGA_Mask = (ULONG)value;
	}
	else if (idx == 51)
	{
		pAd->CommonCfg.Packet_End_Mask = (ULONG)value;
	}
	else if (idx == 52)
	{
		pAd->CommonCfg.Rx_PE_Mask = (ULONG)value;
	}

	printk("Delta_Delay(0) = %d\n", pAd->CommonCfg.DeltaDelay);
	for (x = 0; x < 4; x++)
	{
		printk("Channel %lu\n", x);
		printk("        mode(%02lu)=%d, M(%02lu)=%03d, EL(%02lu)=%03d EH(%02lu)=%03d, WL(%02lu)=%03d WH(%02lu)=%04d, eW(%02lu)=%02d\n        TL(%02lu)=%05u TH(%02lu)=%06u, eT(%02lu)=%03d, BL(%02lu)=%u, BH(%02lu)=%u\n", 
		(x*10+1), (unsigned int)pDFS2Table->entry[x].mode, 
		(x*10+2), (unsigned int)pDFS2Table->entry[x].avgLen, 
		(x*10+3), (unsigned int)pDFS2Table->entry[x].ELow, 
		(x*10+4), (unsigned int)pDFS2Table->entry[x].EHigh, 
		(x*10+5), (unsigned int)pDFS2Table->entry[x].WLow, 
		(x*10+6), (unsigned int)pDFS2Table->entry[x].WHigh, 
		(x*10+7), (unsigned int)pDFS2Table->entry[x].EpsilonW, 
		(x*10+8), (unsigned int)pDFS2Table->entry[x].TLow, 
		(x*10+9), (unsigned int)pDFS2Table->entry[x].THigh, 
		(x*10+10), (unsigned int)pDFS2Table->entry[x].EpsilonT,
		(2*x+41), (unsigned int)pDFS2Table->entry[x].BLow, 
		(2*x+42), (unsigned int)pDFS2Table->entry[x].BHigh);
	}
	
	printk("Symmetric_Round(49) = %d\n", pAd->CommonCfg.Symmetric_Round);
	printk("VGA_Mask(50) = %d\n", pAd->CommonCfg.VGA_Mask);
	printk("Packet_End_Mask(51) = %d\n", pAd->CommonCfg.Packet_End_Mask);
	printk("Rx_PE_Mask(52) = %d\n", pAd->CommonCfg.Rx_PE_Mask);
#endif // CONFIG_RALINK_RT2880 //
#endif // DFS_2_SUPPORT //
}



void 	modify_table2(PRTMP_ADAPTER pAd, ULONG idx, ULONG value)
{
	pNewDFSValidRadar pDFSValidRadar;
	ULONG x, y;
	
	idx--;

	x = idx / 17;
	y = idx % 17;
	
	pDFSValidRadar = &NewDFSValidTable[0];
	
	while (pDFSValidRadar->type != NEW_DFS_END)
	{
		if (pDFSValidRadar->type & pAd->CommonCfg.MCURadarRegion)
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
		if (pDFSValidRadar->type & pAd->CommonCfg.MCURadarRegion)
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


#endif // NEW_DFS //
#endif //DFS_SUPPORT //

#endif // #if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) //

#endif // CONFIG_AP_SUPPORT //
