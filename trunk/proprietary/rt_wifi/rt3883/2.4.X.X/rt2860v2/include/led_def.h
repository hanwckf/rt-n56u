
#ifndef __LED_DEF_H__
#define __LED_DEF_H__

#include "rtmp_type.h"


//
// MCU_LEDCS: MCU LED A/G Configure Setting.
//
typedef union _LED_AG_CFG
{
	struct {
#ifdef RT_BIG_ENDIAN
		UINT16 LedAMode_RadioOnLinkA:2;
		UINT16 LedGMode_RadioOnLinkA:2;
		UINT16 LedAMode_RadioOnLinkG:2;
		UINT16 LedGMode_RadioOnLinkG:2;
		UINT16 LedAMode_RadioOnLinkDown:2;
		UINT16 LedGMode_RadioOnLinkDown:2;
		UINT16 LedAMode_RadioOff:2;
		UINT16 LedGMode_RadioOff:2;
#else
		UINT16 LedGMode_RadioOff:2;
		UINT16 LedAMode_RadioOff:2;
		UINT16 LedGMode_RadioOnLinkDown:2;
		UINT16 LedAMode_RadioOnLinkDown:2;
		UINT16 LedGMode_RadioOnLinkG:2;
		UINT16 LedAMode_RadioOnLinkG:2;
		UINT16 LedGMode_RadioOnLinkA:2;
		UINT16 LedAMode_RadioOnLinkA:2;
#endif
	} field;
	UINT16 word;
} LED_AG_CFG, *PLED_AG_CFG;

//
// MCU_LEDCS: MCU LED ACT Configure Setting.
//
typedef union _LED_ACT_CFG
{
	struct {
#ifdef RT_BIG_ENDIAN
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOnLinkA:1;
		UINT16 LedActMode_RadioOnLinkA:2;
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOnLinkG:1;
		UINT16 LedActMode_RadioOnLinkG:2;
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOnLinkDown:1;
		UINT16 LedActMode_RadioOnLinkDown:2;
		UINT16 :1;
		UINT16 LedActModeNoTx_RadioOff:1;
		UINT16 LedActMode_RadioOff:2;
#else
		UINT16 LedActMode_RadioOff:2;
		UINT16 LedActModeNoTx_RadioOff:1;
		UINT16 :1;
		UINT16 LedActMode_RadioOnLinkDown:2;
		UINT16 LedActModeNoTx_RadioOnLinkDown:1;
		UINT16 :1;
		UINT16 LedActMode_RadioOnLinkG:2;
		UINT16 LedActModeNoTx_RadioOnLinkG:1;
		UINT16 :1;
		UINT16 LedActMode_RadioOnLinkA:2;
		UINT16 LedActModeNoTx_RadioOnLinkA:1;
		UINT16 :1;
#endif
	} field;
	UINT16 word;
} LED_ACT_CFG, *PLED_ACT_CFG;

//
// MCU_LEDCS: MCU LED POLARITY Configure Setting.
//
typedef union _LED_POLARITY
{
	struct {
#ifdef RT_BIG_ENDIAN
		UINT16 :1;
		UINT16 LedActPolarity_RadioOnLinkA:1;
		UINT16 LedAPolarity_RadioOnLinkA:1;
		UINT16 LedGPolarity_RadioOnLinkA:1;
		UINT16 :1;
		UINT16 LedActPolarity_RadioOnLinkG:1;
		UINT16 LedAPolarity_RadioOnLinkG:1;
		UINT16 LedGPolarity_RadioOnLinkG:1;
		UINT16 :1;
		UINT16 LedActPolarity_RadioOnLinkDown:1;
		UINT16 LedAPolarity_RadioOnLinkDown:1;
		UINT16 LedGPolarity_RadioOnLinkDown:1;
		UINT16 :1;
		UINT16 LedActPolarity_RadioOff:1;
		UINT16 LedAPolarity_RadioOff:1;
		UINT16 LedGPolarity_RadioOff:1;
#else
		UINT16 LedGPolarity_RadioOff:1;
		UINT16 LedAPolarity_RadioOff:1;
		UINT16 LedActPolarity_RadioOff:1;
		UINT16 :1;
		UINT16 LedGPolarity_RadioOnLinkDown:1;
		UINT16 LedAPolarity_RadioOnLinkDown:1;
		UINT16 LedActPolarity_RadioOnLinkDown:1;
		UINT16 :1;
		UINT16 LedGPolarity_RadioOnLinkG:1;
		UINT16 LedAPolarity_RadioOnLinkG:1;
		UINT16 LedActPolarity_RadioOnLinkG:1;
		UINT16 :1;
		UINT16 LedGPolarity_RadioOnLinkA:1;
		UINT16 LedAPolarity_RadioOnLinkA:1;
		UINT16 LedActPolarity_RadioOnLinkA:1;
		UINT16 :1;
#endif
	} field;
	UINT16 word;
} LED_POLARITY, *PLED_POLARITY;

#endif /* __LED_DEF_H__ */

