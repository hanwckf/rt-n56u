/*
** Copyright (c) 2008 by Silicon Laboratories
**
** $Id: vdaa_registers.h,v 1.1 2010-07-30 07:55:38 qwert Exp $
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/
#ifndef SI3050REGS_H

#define SI3050REGS_H


enum REGISTERS {
CTRL1						=	1	,	/*	Control    */
CTRL2						=	2	,	/*	Control  */
INTE_MSK					=	3	,	/*	Interrupt Mask  */
INTE_SRC					=	4	,	/*	Interrupt Source  */
DAA_CTRL1					=	5	,	/*	DAA Control 1   */
DAA_CTRL2					=	6	,	/*	DAA Control 2   */
SMPL_CTRL					=	7	,	/*	Sample Rate Control  */ 
RESVD8 						=	8	,	/*	Reserved  */
RESVD9 						=	9	,	/*	Reserved  */
DAA_CTRL3					=	10	,	/*	DAA Control 3  */ 
SYS_LINE_DEV_REV			=	11	,	/*	System- and Line-Side Device Revision  */ 
LSIDE_STAT				 	=	12	,	/*	Line-Side Device Status   */
LSIDE_REV					=	13	,	/*	Line-Side Device Revision   */
DAA_CTRL4				=	14	,	/*	DAA Control 4 RPOL  */
TXRX_GN_CTRL			 	=	15	,	/*	TX/RX Gain Control 1  */ 
INTL_CTRL1				 	=	16	,	/*	International Control 1   */
INTL_CTRL2				 	=	17	,	/*	International Control 2   */
INTL_CTRL3				 	=	18	,	/*	International Control 3   */
INTL_CTRL4				 	=	19	,	/*	International Control 4   */
RXCALL_PROG_ATTEN			=	20	,	/*	Call Progress RX Attenuation  */
TXCALL_PROG_ATTEN			=	21	,	/*	Call Progress TX Attenuation   */
RNG_VLD_CTRL1			 	=	22	,	/*	Ring Validation Control 1   */
RNG_VLD_CTRL2			 	=	23	,	/*	Ring Validation Control 2   */
RNG_VLD_CTRL3			 	=	24	,	/*	ing Validation Control 3   */
RES_CALIB					=	25	,	/*	Resistor Calibration   */
DC_TERM_CTRL			 	=	26	,	/*	DC Termination Control   */
RESVD27						=	27	,	/*	Reserved  */
LP_CRNT_STAT			 	=	28	,	/*	Loop Current Status  */ 
LINE_VOLT_STAT			 	=	29	,	/*	Line Voltage Status   */
AC_TERM_CTRL				=	30	,	/*	AC Termination Control   */
DAA_CTRL5					=	31	,	/*	DAA Control 5   */
GND_STRT_CTRL				=	32	,	/*	Ground Start Control  */ 
PCM_SPI_CTRL			 	=	33	,	/*	PCM/SPI Mode Select   */
PCMTX_CNT_LO			 	=	34	,	/*	PCM Transmit Start Count - Low Byte  */ 
PCMTX_CNT_HI			 	=	35	,	/*	PCM Transmit Start Count - High Byte   */
PCMRX_CNT_LO			 	=	36	,	/*	PCM Receive Start Count - Low Byte   */
PCMRX_CNT_HI			 	=	37	,	/*	PCM Receive Start Count - High Byte   */
TX_GN_CTRL2					=	38	,	/*	TX Gain Control 2   */
RX_GN_CTRL2					=	39	,	/*	RX Gain Control 2   */
TX_GN_CTRL3					=	40	,	/*	TX Gain Control 3   */
RX_GN_CTRL3					=	41	,	/*	RX Gain Control 3   */
GCI_CTRL					=	42	,	/*	GCI Control   */
LN_VI_THRESH_INTE		 	=	43	,	/*	Line Current/Voltage Threshold Interrupt  */ 
LN_VI_THRESH_INTE_CTRL	 	=	44	,	/*	Line Current/Voltage Threshold Interrupt Control  */
HYB1					 	=	45	,	/*	Programmable Hybrid Register 1  */
HYB2					 	=	46	,	/*	Programmable Hybrid Register 2  */
HYB3					 	=	47	,	/*	Programmable Hybrid Register 3  */
HYB4					 	=	48	,	/*	Programmable Hybrid Register 4  */
HYB5					 	=	49	,	/*	Programmable Hybrid Register 5  */
HYB6					 	=	50	,	/*	Programmable Hybrid Register 6  */
HYB7						=	51	,	/*	Programmable Hybrid Register 7  */
HYB8					 	=	52	,	/*	Programmable Hybrid Register 8  */
RESVD53					 	=	53	,	/*	Reserved 53  */
RESVD54						=	54	,	/*	Reserved 54  */
RESVD55					 	=	55	,	/*	Reserved 55  */
RESVD56					 	=	56	,	/*	Reserved 56  */
RESVD57					 	=	57	,	/*	Reserved 57  */
RESVD58					 	=	58	,	/*	Reserved 58  */
SPRK_QNCH_CTRL				=	59		/*	Spark Quenching Control  */ 

};


#endif
