
#ifndef __LED_H__
#define __LED_H__

#include "rt_config.h"
#include "led_def.h"

/*************************************************************************
  *
  *	LED related data type and MARCO definitions.
  *
  ************************************************************************/

#define LED_G			0
#define LED_A			1
#define LED_ACT			2

#define MCU_INT_STATUS	0x0414
#define GPIO_DAT		0x0228
#define GPIO_DIR		0x0229
#define MAC_LED_CFG		0x102c


typedef unsigned char	BYTE;

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE	0
#endif

typedef union _LED_OPERATION_MODE
{
	struct {
		UINT16 :6;
		UINT16 LedGMode:2;		/* 1: solid on, 2: slow blink, 3: first blink. */
		UINT16 LedAMode:2;		/* 1: solid on, 2: slow blink, 3: first blink. */
		UINT16 LedActMode:2;	/* 1: Solid on, 2: Blink when Tx Data, Mng pkt.
									3: Blink when Tx, Data, Mng, Ben pkt. */
		UINT16 LedActModeNoTx:1;

		UINT16 LedGPolarity:1;
		UINT16 LedAPolarity:1;
		UINT16 LedActPolarity:1;
	} field;
	UINT word;
} LED_OPERATION_MODE, *PLED_OPERATION_MODE;

typedef union _LED_CFG_T
{
	struct{
#ifdef RT_BIG_ENDIAN
		UINT32 :1;
		UINT32 LED_POL:1;		/* 0: active low, 1:active high. */
		UINT32 Y_LED_MODE:2;	/* 0: off, 1: blinking upon Tx,
									2:Periodic slow blinking, 3:sloid on. */
		UINT32 G_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 R_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 :2;
		UINT32 SLOW_BLK_TIME:6;	/* slow blinking period (uint: 1sec). */
		UINT32 LED_OFF_TIIME:8;	/* Tx blinking off period (unit: 1ms). */
		UINT32 LED_ON_TIME:8; 	/* Tx blinking on period (uint: 1ms). */
#else
		UINT32 LED_ON_TIME:8; 	/* Tx blinking on period (uint: 1ms). */
		UINT32 LED_OFF_TIME:8;	/* Tx blinking off period (unit: 1ms). */
		UINT32 SLOW_BLK_TIME:6;	/* slow blinking period (uint: 1sec). */
		UINT32 :2;
		UINT32 R_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 G_LED_MODE:2;	/* same as Y_LED_MODE. */
		UINT32 Y_LED_MODE:2;	/* 0: off, 1: blinking upon Tx,
									2:Periodic slow blinking, 3:sloid on. */
		UINT32 LED_POL:1;		/* 0: active low, 1:active high. */
		UINT32 :1;
#endif
	} field;
	UINT32 word;
} LED_CFG_T, *PLED_CFG_T;

typedef struct _ULED_PARAMETER
{
	LED_AG_CFG LedAgCfg;
	LED_ACT_CFG LedActCfg;
	LED_POLARITY LedPolarityCfg;
	UINT8 LedMode;
} ULED_PARAMETER, *PULED_PARAMETER;

extern RALINK_TIMER_STRUCT LedCheckTimer;
extern unsigned char CheckTimerEbl;

INT Show_LedCfg_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

INT Set_LedCfg_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

INT Set_LedCheck_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg);

VOID SetLedCfg(
	IN PRTMP_ADAPTER pAd,
	IN INT ledPolarity,
	IN INT ledOnTime,
	IN INT ledOffTime,
	IN INT slowBlkTime);

VOID SetLedMode(
	IN PRTMP_ADAPTER pAd,
	IN INT LedSel,
	IN INT LedMode);

void ChgSignalStrengthLed(
	IN PRTMP_ADAPTER pAd);

void ChgMacLedCfg(
	IN PRTMP_ADAPTER pAd);

void LedCtrlMain(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

void SetLinkStatusEntry(
	IN PRTMP_ADAPTER pAd);

#endif /* __LED_H__ */

