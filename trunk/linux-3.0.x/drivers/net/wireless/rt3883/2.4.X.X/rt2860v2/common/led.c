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
	led.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

 
#if defined(WLAN_LED)

#ifdef LINUX
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#endif // LINUX //

#include "rt_config.h"
#include "led.h"

#define GetBitX(x, y)			(	((x) & (1 << (y)))>>y		)
#define GetBit(x, y)			(	((x) & (1 << (y)))		)
#define GetBitX(x, y)			(	((x) & (1 << (y)))>>y		)
#define SetBit(x, y)			(	(x) = (x) | (1 << (y))	)
#define ClearBit(x, y)			{	(x) &= (~(1 << (y)));	}
#define DSetBit(x, y)			(	(x) = (1 << (y))		)
#define MakeWord(HI, LO)		(	((WORD)(LO)) | (((WORD)(HI))<<8) )	// Note! LO must be read first
#define HiByte(W)				(	(BYTE)(((WORD)W) >> 8)	)
#define LoByte(W)				(	(BYTE)(W)				)
#define FireInterruptToHost()	{	IntToHost = 0xff;		}

#define LED_CHECK_INTERVAL 	50 /* 50ms */

RALINK_TIMER_STRUCT LedCheckTimer;

unsigned char LinkStatus;
unsigned char GPIOPolarity;
unsigned char SignalStrength;
unsigned char LedBlinkTimer = 0xff;
unsigned long BlinkFor8sTimer;

ULED_PARAMETER LedParameter;
LED_OPERATION_MODE CurrentLedCfg;
BYTE CheckTimerEbl=0;

// Produce CurrentLedCfg and CurrentLedPolarity from LedCfgAct/LedCfgAG/LedPolarity and LinkStatus
// CurrentLedCfg format:
//  [7:4] LED ACT configuration
// 	  0x00: Solid off when Tx
//    0x01: Solid on when Tx
//    0x10: Blink when transmitting data and management packet
//    0x11: Blink when transmitting data, management packet and beacon
//    01xx: Slow blink when no traffic
//  [3:2] LED A configuration
//  [1:0] LED G configuration
//    00: Off		01: On		10: Slow blink		11: Fast blink
// AbstractMacLedCfg format:
//  [5:4] ACT configuration
//  [3:2] LED A configuration
//  [1:0] LED G configuration
//    00: Off		01: On		10: Slow blink		11: Fast blink
// CurrentLedPolarity format:
//  [3] LED ACT polarity inversion when link to A
//  [2] LED ACT polarity
//  [1] LED A
//  [0] LED G
//
//						H2MMailbox
//	SetLinkStatusEntry()	<------------- Driver
//		|
//		V
//	SetLinkStatus()
//		|
//		+--->	CurrentLedPolarity, CurrentLedCfg are generated
//		
//	LedCtrlMainEntry()	<----------------- Driven by 50ms/500ms tick when LedMode is not 0
//		|
//		V
//	LedCtrlMain()
//		|
//		V
//	ChgMacLedCfg()	(The default LED control module)
//		^
//		|----	AbstractMacLedCfg is generated here. MAC LED state is changed in ChgMacLedCfg()
//		

INT Show_LedCfg_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	LED_CFG_T macLedCfg;
	RTMP_IO_READ32(pAd, MAC_LED_CFG, &macLedCfg.word);

	printk("LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkA=%d\n", LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkA);
	printk("LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkA=%d\n", LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkA);
	printk("LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkG=%d\n", LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkG);
	printk("LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkG=%d\n", LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkG);
	printk("LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkDown=%d\n", LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkDown);
	printk("LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkDown=%d\n", LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkDown);
	printk("LedParameter.LedAgCfg.field.LedAMode_RadioOff=%d\n", LedParameter.LedAgCfg.field.LedAMode_RadioOff);
	printk("LedParameter.LedAgCfg.field.LedGMode_RadioOff=%d\n", LedParameter.LedAgCfg.field.LedGMode_RadioOff);


	printk("LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkA=%d\n", LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkA);
	printk("LedParameter.LedActCfg.field.LedActMode_RadioOnLinkA=%d\n", LedParameter.LedActCfg.field.LedActMode_RadioOnLinkA);
	printk("LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkG=%d\n", LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkG);
	printk("LedParameter.LedActCfg.field.LedActMode_RadioOnLinkG=%d\n", LedParameter.LedActCfg.field.LedActMode_RadioOnLinkG);
	printk("LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkDown=%d\n", LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkDown);
	printk("LedParameter.LedActCfg.field.LedActMode_RadioOnLinkDown=%d\n", LedParameter.LedActCfg.field.LedActMode_RadioOnLinkDown);
	printk("LedParameter.LedActCfg.field.LedActModeNoTx_RadioOff=%d\n", LedParameter.LedActCfg.field.LedActModeNoTx_RadioOff);
	printk("LedParameter.LedActCfg.field.LedActMode_RadioOff=%d\n", LedParameter.LedActCfg.field.LedActMode_RadioOff);


	printk("LED_CFG = %x\n\n", macLedCfg.word);
	printk("LED_ON_TIME = %d\n", macLedCfg.field.LED_ON_TIME);
	printk("LED_OFF_TIME = %d\n", macLedCfg.field.LED_OFF_TIME);
	printk("SLOW_BLK_TIME = %d\n", macLedCfg.field.SLOW_BLK_TIME);
	printk("R_LED_MODE (A) = %d\n", macLedCfg.field.R_LED_MODE);
	printk("G_LED_MODE (ACT) = %d\n", macLedCfg.field.G_LED_MODE);
	printk("Y_LED_MODE (A) = %d\n", macLedCfg.field.Y_LED_MODE);
	printk("LED_POL = %d\n", macLedCfg.field.LED_POL);

	return TRUE;
}

INT Set_LedCfg_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	INT loop;
	PCHAR thisChar;
	long polarity=0;
	long ledOnTime=0;
	long ledOffTime=0;
	long slowBlkTime=0;
	
	loop = 0;

	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		switch(loop)
		{
			case 0:
				polarity = simple_strtol(thisChar, 0, 10);
				break;

			case 1:
				ledOnTime = simple_strtol(thisChar, 0, 10);
				break;

			case 2:
				ledOffTime = simple_strtol(thisChar, 0, 10);
				break;

			case 3:
				slowBlkTime = simple_strtol(thisChar, 0, 10);
				break;

			default:
				break;
		}
		loop ++;
	}

	SetLedCfg(pAd, (INT)polarity, (INT)ledOnTime, (INT)ledOffTime, (INT)slowBlkTime);

	return TRUE;
}

INT Set_LedCheck_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	INT loop;
	PCHAR thisChar;
	long ledSel=0;
	long ledMode=0;

	loop = 0;
	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		switch(loop)
		{
			case 0:
				ledSel = simple_strtol(thisChar, 0, 10);
				break;

			case 1:
				ledMode = simple_strtol(thisChar, 0, 10);
				break;

			default:
				break;
		}
		loop ++;
	}

	SetLedMode(pAd, (INT)ledSel, (INT)ledMode);

	return TRUE;
}

VOID SetLedCfg(
	IN PRTMP_ADAPTER pAd,
	IN INT ledPolarity,
	IN INT ledOnTime,
	IN INT ledOffTime,
	IN INT slowBlkTime)
{
	LED_CFG_T macLedCfg;
	RTMP_IO_READ32(pAd, MAC_LED_CFG, &macLedCfg.word);

	macLedCfg.field.LED_POL = ledPolarity;
	macLedCfg.field.LED_ON_TIME = ledOnTime;
	macLedCfg.field.LED_OFF_TIME = ledOffTime;
	macLedCfg.field.SLOW_BLK_TIME = slowBlkTime;

	RTMP_IO_WRITE32(pAd, MAC_LED_CFG, macLedCfg.word);

	return;
}

VOID SetLedMode(
	IN PRTMP_ADAPTER pAd,
	IN INT LedSel,
	IN INT LedMode)
{
	LED_CFG_T macLedCfg;
	RTMP_IO_READ32(pAd, MAC_LED_CFG, &macLedCfg.word);

	switch(LedSel)
	{
		case LED_G: /* LED G. */
			macLedCfg.field.Y_LED_MODE = LedMode;
			break;

		case LED_A: /* LED A. */
			macLedCfg.field.G_LED_MODE = LedMode;
			break;

		case LED_ACT: /* LED ACT. */
			macLedCfg.field.R_LED_MODE = LedMode;
			break;

		default:
			break;
	}

	RTMP_IO_WRITE32(pAd, MAC_LED_CFG, macLedCfg.word);

	return;
}

// Called when host sets LED mode.
// Called in interrupt level
void SetLinkStatus(IN PRTMP_ADAPTER pAd)
{
	BlinkFor8sTimer = 0;
	switch(LinkStatus)
	{
		case 0x00:		// Radio off
			CurrentLedCfg.field.LedGMode = LedParameter.LedAgCfg.field.LedGMode_RadioOff;
			CurrentLedCfg.field.LedAMode = LedParameter.LedAgCfg.field.LedAMode_RadioOff;
			CurrentLedCfg.field.LedActMode = LedParameter.LedActCfg.field.LedActMode_RadioOff;
			CurrentLedCfg.field.LedActModeNoTx = LedParameter.LedActCfg.field.LedActModeNoTx_RadioOff;
			CurrentLedCfg.field.LedGPolarity = LedParameter.LedPolarityCfg.field.LedGPolarity_RadioOff;
			CurrentLedCfg.field.LedAPolarity = LedParameter.LedPolarityCfg.field.LedAPolarity_RadioOff;
			CurrentLedCfg.field.LedActPolarity = LedParameter.LedPolarityCfg.field.LedActPolarity_RadioOff;
			break;

		case 0x20:		// Radio on but not link up
			CurrentLedCfg.field.LedGMode = LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkDown;
			CurrentLedCfg.field.LedAMode = LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkDown;
			CurrentLedCfg.field.LedActMode = LedParameter.LedActCfg.field.LedActMode_RadioOnLinkDown;
			CurrentLedCfg.field.LedActModeNoTx = LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkDown;
			CurrentLedCfg.field.LedGPolarity = LedParameter.LedPolarityCfg.field.LedGPolarity_RadioOnLinkDown;
			CurrentLedCfg.field.LedAPolarity = LedParameter.LedPolarityCfg.field.LedAPolarity_RadioOnLinkDown;
			CurrentLedCfg.field.LedActPolarity = LedParameter.LedPolarityCfg.field.LedActPolarity_RadioOnLinkDown;
  			break;

		case 0x60:		// Radio on and link to G
			CurrentLedCfg.field.LedGMode = LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkG;
			CurrentLedCfg.field.LedAMode = LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkG;
			CurrentLedCfg.field.LedActMode = LedParameter.LedActCfg.field.LedActMode_RadioOnLinkG;
			CurrentLedCfg.field.LedActModeNoTx = LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkG;
			CurrentLedCfg.field.LedGPolarity = LedParameter.LedPolarityCfg.field.LedGPolarity_RadioOnLinkG;
			CurrentLedCfg.field.LedAPolarity = LedParameter.LedPolarityCfg.field.LedAPolarity_RadioOnLinkG;
			CurrentLedCfg.field.LedActPolarity = LedParameter.LedPolarityCfg.field.LedActPolarity_RadioOnLinkG;
  			break;

		case 0xa0:		// Radio on and link to A
			CurrentLedCfg.field.LedGMode = LedParameter.LedAgCfg.field.LedGMode_RadioOnLinkA;
			CurrentLedCfg.field.LedAMode = LedParameter.LedAgCfg.field.LedAMode_RadioOnLinkA;
			CurrentLedCfg.field.LedActMode = LedParameter.LedActCfg.field.LedActMode_RadioOnLinkA;
			CurrentLedCfg.field.LedActModeNoTx = LedParameter.LedActCfg.field.LedActModeNoTx_RadioOnLinkA;
			CurrentLedCfg.field.LedGPolarity = LedParameter.LedPolarityCfg.field.LedGPolarity_RadioOnLinkA;
			CurrentLedCfg.field.LedAPolarity = LedParameter.LedPolarityCfg.field.LedAPolarity_RadioOnLinkA;
			CurrentLedCfg.field.LedActPolarity = LedParameter.LedPolarityCfg.field.LedActPolarity_RadioOnLinkA;
  			break;

		case 0x10:		// WPS
			if(LedParameter.LedMode == 0x04)
			{
				CurrentLedCfg.field.LedActPolarity = 0;
				CurrentLedCfg.field.LedActMode = 1;	// Force ACT to solid on/off
				CurrentLedCfg.field.LedActMode = 0;	// Force ACT to solid on/off
			}
			if(LedParameter.LedMode == 0x05)
			{
				CurrentLedCfg.field.LedActPolarity = 1;
				CurrentLedCfg.field.LedActMode = 1;	// Force ACT to solid on/off
				CurrentLedCfg.field.LedActMode = 0;	// Force ACT to solid on/off
			}
			break;

		case 0x08:
			if (LedParameter.LedMode == 2)
				BlinkFor8sTimer = (8000 / LED_CHECK_INTERVAL);
			break;
	}

	if(CheckTimerEbl==0)
	{
		memset(&LedCheckTimer, 0, sizeof(RALINK_TIMER_STRUCT));
		RTMPInitTimer(pAd, &LedCheckTimer, GET_TIMER_FUNCTION(LedCtrlMain), pAd, TRUE);
		RTMPSetTimer(&LedCheckTimer, LED_CHECK_INTERVAL);

		CheckTimerEbl=1;
	}
}


void SetLinkStatusEntry(IN PRTMP_ADAPTER pAd)
{	
	SetLinkStatus(pAd);	
}	// @ 0x1025


// Entry for Led control.  Triggered by timer per 50ms
void LedCtrlMain(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	switch(LedParameter.LedMode)
	{
		case 0:		// Hardware controlled mode. Just ignore it.
			return;
		case 0x40:	// In addition to mode 0, set signal strength LED
			ChgSignalStrengthLed(pAd);
		default:
			ChgMacLedCfg(pAd);
			break;
	}

	if(LedBlinkTimer!=0)
		LedBlinkTimer--;
	else
		LedBlinkTimer = 0xff;
}


static inline int TX_TRAFFIC_EXIST(
	IN PRTMP_ADAPTER pAd)
{
	UINT RegValue;

	RTMP_IO_READ32(pAd, MCU_INT_STATUS, &RegValue);
	return (RegValue & 0x0e);
}

static inline int BEN_TC_ACT(
	IN PRTMP_ADAPTER pAd)
{
	UINT RegValue;

	RTMP_IO_READ32(pAd, MCU_INT_STATUS, &RegValue);
	return (RegValue & 0x10);
}

// Change LED State according to CurrentLedPolarity, CurrentLedCfg, and Tx activity
// The result is stored in AbstractMacLedCfg (Abstract MacLedCfg).
// AbstractMacLedCfg is an abstract layer. Since MAC's LED_CFG doesn't match firmware requirement totally,
// this abstract layer is to hide these difference.  
void ChgMacLedCfg(
	IN PRTMP_ADAPTER pAd)
{
	LED_CFG_T LedCfgBuf;
	BYTE DataTxActivity, BeaconTxActivity;
	extern ULONG WlanLed;


	/* 
	** MCU_INT_STA (offset: 0x0414)
	** bit 1: MTX2_INT, TX2Q to MAC frame transfer complete interrupt.
	** bit 2: MTX1_INT, TX1Q to MAC frame transfer complete interrupt.
	** bit 3: MTX0_INT, TX0Q to MAC frame transfer complete interrupt.
	*/

	if (TX_TRAFFIC_EXIST(pAd)) // Check if there is Tx Traffic
		DataTxActivity = 1;
	else
		DataTxActivity = 0;

	// Check if there are beacon
	BeaconTxActivity = BEN_TC_ACT(pAd);

	// Clear Tx and beacon Tx complete interrupt
	RTMP_IO_WRITE32(pAd, MCU_INT_STATUS, 0x1e);

	RTMP_IO_READ32(pAd, MAC_LED_CFG, &LedCfgBuf.word);

	/* For backward compatible issue,
         * LedActMode: 0: None, 1: Solid ON, 2: Blink (data/mgr), 3: Blink (data,mgr,beacon)
	 * =>Solid off = solid on + high polarity
         */
	LedCfgBuf.field.LED_POL = CurrentLedCfg.field.LedActPolarity;

	/* LED Act. connect to G_LED. */
	if (LedParameter.LedMode == 2 && BlinkFor8sTimer)
	{
		UINT8 LedPolarity = CurrentLedCfg.field.LedActPolarity ? 0 : 3;
		LedCfgBuf.field.G_LED_MODE = GetBitX(LedBlinkTimer, 0) ? LedPolarity : ~LedPolarity;
		BlinkFor8sTimer--;
	}
	else if (CurrentLedCfg.field.LedActMode == 0) /* dark. */
	{
		LedCfgBuf.field.G_LED_MODE =
				CurrentLedCfg.field.LedActPolarity ? 3 : 0;	/* dark mode. */
	}
	else if (CurrentLedCfg.field.LedActMode == 1) /* solid on. */
	{
		LedCfgBuf.field.G_LED_MODE =
				CurrentLedCfg.field.LedActPolarity ? 0 : 3;	/* solid on mode. */
	}
	else if ((DataTxActivity && CurrentLedCfg.field.LedActMode > 1) /* Data packet transmited. */
		|| (BeaconTxActivity && CurrentLedCfg.field.LedActMode == 3)) /* Beacon frame transmited. */
	{
		LedCfgBuf.field.G_LED_MODE = GetBitX(LedBlinkTimer, 0) ? 3 : 0;
	}
	else if (!DataTxActivity && !BeaconTxActivity)
	{
		if (CurrentLedCfg.field.LedActModeNoTx == 0) /* solid on when no tx. */
			LedCfgBuf.field.G_LED_MODE =
				CurrentLedCfg.field.LedActPolarity ? 0 : 3;	/* solid on mode. */
		else /* slow blink. */
			LedCfgBuf.field.G_LED_MODE = GetBitX(LedBlinkTimer, 4) ? 3 : 0;
	}


	/* LED G. connect to Y_LED. */
	if (CurrentLedCfg.field.LedGMode == 3)	/* fast blinking. */
		LedCfgBuf.field.Y_LED_MODE = GetBitX(LedBlinkTimer, 0) ? 3 : 0;
	else if (CurrentLedCfg.field.LedGMode == 2) /* slow blinkg. */
		LedCfgBuf.field.Y_LED_MODE = GetBitX(LedBlinkTimer, 4) ? 3 : 0;
	else if (CurrentLedCfg.field.LedGMode == 1)	/* solid on. */
		LedCfgBuf.field.Y_LED_MODE =
			CurrentLedCfg.field.LedGPolarity ? 0 : 3;	/* solid on mode. */
	else /* dark */
		LedCfgBuf.field.Y_LED_MODE =
			CurrentLedCfg.field.LedGPolarity ? 3 : 0;	/* dark mode. */

	/* LED A. connect to R_LED. */
	if (CurrentLedCfg.field.LedAMode == 3)	/* fast blinking. */
		LedCfgBuf.field.R_LED_MODE = GetBitX(LedBlinkTimer, 0) ? 3 : 0;
	else if (CurrentLedCfg.field.LedAMode ==  2) /* slow blinkg. */
		LedCfgBuf.field.R_LED_MODE = GetBitX(LedBlinkTimer, 4) ? 3 : 0;
	else if (CurrentLedCfg.field.LedAMode == 1)	/* solid on. */
		LedCfgBuf.field.R_LED_MODE =
			CurrentLedCfg.field.LedAPolarity ? 0 : 3;	/* solid on mode. */
	else /* dark */
		LedCfgBuf.field.R_LED_MODE =
			CurrentLedCfg.field.LedAPolarity ? 3 : 0;	/* dark mode. */

	if ( (WlanLed == 0) ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) {
	    RTMP_IO_WRITE32(pAd, MAC_LED_CFG, 0);
	} else {
	RTMP_IO_WRITE32(pAd, MAC_LED_CFG, LedCfgBuf.word);
	}

	return;
}


void ChgSignalStrengthLed(
		IN PRTMP_ADAPTER pAd)
{
	RTMP_IO_WRITE32(pAd, GPIO_DIR, 0x00); /* set GPIO to output. */
	RTMP_IO_WRITE32(pAd, GPIO_DAT, (GPIOPolarity ? SignalStrength : ~SignalStrength));
}

#ifdef OS_ABL_SUPPORT
EXPORT_SYMBOL(LedCheckTimer);
EXPORT_SYMBOL(CheckTimerEbl);
#endif // OS_ABL_SUPPORT //
#endif

