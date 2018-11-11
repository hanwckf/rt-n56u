/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	gpio.h
*/

#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_BASE 			0x80023000
#define GPIO_PU1 			(GPIO_BASE + 0x00)
#define GPIO_PU1_SET 		(GPIO_BASE +0x04)
#define GPIO_PU1_RESET 		(GPIO_BASE + 0x8)
#define GPIO_PD1 			(GPIO_BASE + 0x10)
#define GPIO_PD1_SET 		(GPIO_BASE + 0x14)
#define GPIO_PD1_RESET 		(GPIO_BASE + 0x18)
#define GPIO_DOUT1 			(GPIO_BASE + 0x20)
#define GPIO_DOUT1_SET 		(GPIO_BASE + 0x24)
#define GPIO_DOUT1_RESET 	(GPIO_BASE + 0x28)
#define GPIO_OE1 			(GPIO_BASE + 0x30)
#define GPIO_OE1_SET 		(GPIO_BASE + 0x34)
#define GPIO_OE1_RESET 		(GPIO_BASE + 0x38)
#define GPIO_DIN1 			(GPIO_BASE + 0x40)
#ifdef MT7615
#define PAD_CONTROL_12    	(GPIO_BASE + 0x8C)
#define GPIO_PU2 			(GPIO_BASE + 0x100)
#define GPIO_PU2_SET 		(GPIO_BASE + 0x104)
#define GPIO_PU2_RESET 		(GPIO_BASE + 0x108)
#define GPIO_PD2 			(GPIO_BASE + 0x110)
#define GPIO_PD2_SET 		(GPIO_BASE + 0x114)
#define GPIO_PD2_RESET 		(GPIO_BASE + 0x118)
#define GPIO_DOUT2 			(GPIO_BASE + 0x120)
#define GPIO_DOUT2_SET 		(GPIO_BASE + 0x124)
#define GPIO_DOUT2_RESET 	(GPIO_BASE + 0x128)
#define GPIO_OE2 			(GPIO_BASE + 0x130)
#define GPIO_OE2_SET 		(GPIO_BASE + 0x134)
#define GPIO_OE2_RESET		(GPIO_BASE + 0x138)
#define GPIO_DIN2 			(GPIO_BASE + 0x140)
#define PINMUX_SEL1 		(GPIO_BASE + 0x160)
#define PINMUX_SEL2 		(GPIO_BASE + 0x164)
#define PINMUX_SEL3 		(GPIO_BASE + 0X168)
#define PINMUX_SEL4 		(GPIO_BASE + 0x16C)
#define PINMUX_SEL5 		(GPIO_BASE + 0x170)
#define PINMUX_SEL6 		(GPIO_BASE + 0X174)
#else
#define PINMUX_SEL1 		(GPIO_BASE + 0x80)
#define PINMUX_SEL2 		(GPIO_BASE + 0x84)
#define PINMUX_SEL3 		(GPIO_BASE + 0X88)
#endif
#define GPIO0_SEL_MASK 		(0x7)
#define GPIO0_SEL(p) 		(((p) & 0x7))
#define GPIO0_SEL_VALUE 	0x0
#define GPIO1_SEL_MASK 		(0x7 << 4)
#define GPIO1_SEL(p) 		(((p) & 0x7) << 4)
#ifdef MT7615
#define GPIO1_SEL_VALUE 	0x0
#else
#define GPIO1_SEL_VALUE 	0x2
#endif
#define GPIO2_SEL_MASK 		(0x7 << 8)
#define GPIO2_SEL(p) 		(((p) & 0x7) << 8)
#ifdef MT7615
#define GPIO2_SEL_VALUE 	0x0
#else
#define GPIO2_SEL_VALUE 	0x2
#endif
#define GPIO3_SEL_MASK 		(0x7 << 12)
#define GPIO3_SEL(p) 		(((p) & 0x7) << 12)
#ifdef MT7615
#define GPIO3_SEL_VALUE 	0x0
#else
#define GPIO3_SEL_VALUE 	0x2
#endif
#define GPIO4_SEL_MASK 		(0x7 << 16)
#define GPIO4_SEL(p) 		(((p) & 0x7) << 16)
#ifdef MT7615
#define GPIO4_SEL_VALUE 	0x0
#else
#define GPIO4_SEL_VALUE 	0x2
#endif
#define GPIO5_SEL_MASK 		(0x7 << 20)
#define GPIO5_SEL(p) 		(((p) & 0x7) << 20)
#ifdef MT7615
#define GPIO5_SEL_VALUE 	0x0
#else
#define GPIO5_SEL_VALUE 	0x2
#endif
#define GPIO6_SEL_MASK 		(0x7 << 24)
#define GPIO6_SEL(p) 		(((p) & 0x7) << 24)
#define GPIO6_SEL_VALUE 	0x0
#define GPIO7_SEL_MASK 		(0x7 << 28)
#define GPIO7_SEL(p) 		(((p) & 0x7) << 28)
#define GPIO7_SEL_VALUE 	0x0
#define GPIO8_SEL_MASK 		(0x7)
#define GPIO8_SEL(p) 		(((p) & 0x7))
#define GPIO8_SEL_VALUE 	0x0
#define GPIO9_SEL_MASK 		(0x7 << 4)
#define GPIO9_SEL(p) 		(((p) & 0x7) << 4)
#define GPIO9_SEL_VALUE 	0x0
#define GPIO10_SEL_MASK 	(0x7 << 8)
#define GPIO10_SEL(p) 		(((p) & 0x7) << 8)
#define GPIO10_SEL_VALUE 	0x0
#define GPIO11_SEL_MASK 	(0x7 << 12)
#define GPIO11_SEL(p) 		(((p) & 0x7) << 12)
#ifdef MT7615
#define GPIO11_SEL_VALUE 	0x0
#else
#define GPIO11_SEL_VALUE 	0x2
#endif
#define GPIO12_SEL_MASK 	(0x7 << 16)
#define GPIO12_SEL(p) 		(((p) & 0x7) << 16)
#ifdef MT7615
#define GPIO12_PAD_CONTROL 	16
#define GPIO12_SEL_VALUE 	0x0
#else
#define GPIO12_SEL_VALUE 	0x2
#endif
#define GPIO13_SEL_MASK 	(0x7 << 20)
#define GPIO13_SEL(p) 		(((p) & 0x7) << 20)
#ifdef MT7615
#define GPIO13_SEL_VALUE 	0x0
#else
#define GPIO13_SEL_VALUE 	0x2
#endif
#define GPIO14_SEL_MASK 	(0x7 << 24)
#define GPIO14_SEL(p) 		(((p) & 0x7) << 24)
#define GPIO14_SEL_VALUE 	0x2
#define GPIO15_SEL_MASK 	(0x7 << 28)
#define GPIO15_SEL(p) 		(((p) & 0x7) << 28)
#define GPIO15_SEL_VALUE 	0x2
#define GPIO16_SEL_MASK 	(0x7)
#define GPIO16_SEL(p) 		(((p) & 0x7))
#define GPIO16_SEL_VALUE 	0x2
#ifdef MT7615
#define GPIO17_SEL_MASK 	(0x7 << 4)
#define GPIO17_SEL(p) 		(((p) & 0x7) << 4)
#define GPIO17_SEL_VALUE 	0x2
#define GPIO18_SEL_MASK 	(0x7 << 8)
#define GPIO18_SEL(p) 		(((p) & 0x7) << 8)
#define GPIO18_SEL_VALUE 	0x2
#define GPIO19_SEL_MASK 	(0x7 << 12)
#define GPIO19_SEL(p) 		(((p) & 0x7) << 12)
#define GPIO19_SEL_VALUE 	0x2
#define GPIO32_SEL_MASK 	(0x7)
#define GPIO32_SEL(p) 		(((p) & 0x7))
#define GPIO32_SEL_VALUE 	0x0
#define GPIO32_PAD_CONTROL 	17
#define GPIO33_SEL_MASK 	(0x7 << 4)
#define GPIO33_SEL(p) 		(((p) & 0x7) << 4)
#define GPIO33_SEL_VALUE 	0x0
#define GPIO33_PAD_CONTROL 	18
#define GPIO34_SEL_MASK 	(0x7 << 8)
#define GPIO34_SEL(p) 		(((p) & 0x7) << 8)
#define GPIO34_SEL_VALUE 	0x0
#define GPIO34_PAD_CONTROL 	19
#define GPIO35_SEL_MASK 	(0x7 << 12)
#define GPIO35_SEL(p) 		(((p) & 0x7) << 12)
#define GPIO35_SEL_VALUE 	0x0
#define GPIO35_PAD_CONTROL 	20
#endif

#define OUTPUT_HIGH 		1
#define OUTPUT_LOW 			0
#define INPUT_HIGH 			1
#define INPUT_LOW 			0
#define GPIO_OUTPUT 		1
#define GPIO_INPUT 			0
#define PULL_UP 			1
#define NO_PULL_UP 			0
#define PULL_DOWN			1
#define NO_PULL_DOWN 		0

#define GPIO0 				0
#define GPIO1 				1
#define GPIO2 				2
#define GPIO3 				3
#define GPIO4 				4
#define GPIO5 				5
#define GPIO6 				6
#define GPIO7 				7
#define GPIO8 				8
#define GPIO9 				9
#define GPIO10 				10
#define GPIO11 				11
#define GPIO12 				12
#define GPIO13 				13
#define GPIO14 				14
#define GPIO15 				15
#define GPIO16 				16
#ifdef MT7615
#define GPIO17 				17
#define GPIO18 				18
#define GPIO19 				19
#define GPIO32 				32
#define GPIO33 				33
#define GPIO34 				34
#define GPIO35 				35
#endif
#ifdef GPIO_CONTROL_SUPPORT
#define MAX_GPIO_AVAILABLE			9
#define IS_GPIO_AVAILABLE(_x)		((_x == GPIO11)||(_x == GPIO12)||(_x == GPIO13)||(_x == GPIO18)||(_x == GPIO19)||(_x == GPIO32)||(_x == GPIO33)||(_x == GPIO34)||(_x == GPIO35))
#endif

INT32 GPIODirectionInput(struct _RTMP_ADAPTER *pAd, UINT32 GPIO);
INT32 GPIODirectionOuput(struct _RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value);
UINT32 GPIOGetValue(struct _RTMP_ADAPTER *pAd, UINT32 GPIO);
VOID GPIOSetValue(struct _RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value);
UINT32 GPIOGetMode(struct _RTMP_ADAPTER *pAd, UINT32 GPIO);
INT32 GPIOPullUp(struct _RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value);
INT32 GPIOPullDown(struct _RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value);

#endif
