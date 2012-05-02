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
	rt3593.h
 
    Abstract:
	3*3 Wireless Chip PCIe

    Revision History:
    Who			When			What
    ---------	----------		----------------------------------------------
	SampleLin	20091105		Initial version
 */

#ifndef __RT3593_H__
#define __RT3593_H__

#ifdef RT3593

/*
	MCS 16 ~ 23 Test Note:
	Use fix rate mode, HT_MCS = 23, and set bit 30 of MAC Reg 134C to 0
	(disable auto-fallback mode).
*/

#ifndef RTMP_PCI_SUPPORT
#error "For RT3593, you should define the compile flag -DRTMP_PCI_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT3593, you should define the compile flag -DRTMP_MAC_PCI"
#endif

#ifndef RTMP_RF_RW_SUPPORT
#error "For RT3593, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#ifndef RT30xx
#error "For RT3593, you should define the compile flag -DRT30xx"
#endif

#error "For RT3593, you should define the compile flag -DRT35xx"

/* General definition */

/* if you want to support PCIe power save function */
/* 2009/11/06, if open the function, the signal will be bad and sometimes crush */
//#define PCIE_PS_SUPPORT

//
// Device ID & Vendor ID, these values should match EEPROM value
//
#define NIC3593_PCI_OR_PCIe_DEVICE_ID	0x3593

extern REG_PAIR RF3053RegTable[];
extern UCHAR NUM_RF_3053_REG_PARMS;


/* MACRO definition */

/* for 3*3 related */
#define RTMP_RF33_SHARED_MEM_SELECT(__pAd)							\
{																	\
	if (IS_RT3593(__pAd))											\
	{																\
		PBF_SYS_CTRL_STRUC __PbfSysCtrl = {{0}};					\
		RTMP_IO_READ32(__pAd, PBF_SYS_CTRL, &__PbfSysCtrl.word);	\
		__PbfSysCtrl.field.SHR_MSEL = 1;							\
		RTMP_IO_WRITE32(__pAd, PBF_SYS_CTRL, __PbfSysCtrl.word);	\
	}																\
}

#define RTMP_RF33_SHARED_MEM_DESELECT(__pAd)						\
{																	\
	if (IS_RT3593(__pAd))											\
	{																\
		PBF_SYS_CTRL_STRUC __PbfSysCtrl = {{0}};					\
		RTMP_IO_WRITE32(pAd, BCN_OFFSET0, 0x18100800); /* 0x0000(00), 0x0200(08), 0x0400(10), 0x0600(18), 512B for each beacon */ \
		RTMP_IO_WRITE32(pAd, BCN_OFFSET1, 0x38302820); /* 0x0800(20), 0x0A00(28), 0x0C00(30), 0x0E00(38), 512B for each beacon */ \
		RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &__PbfSysCtrl.word);		\
		__PbfSysCtrl.field.SHR_MSEL = 0;							\
		RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, __PbfSysCtrl.word);		\
	}																\
}


#endif // RT3593 //
#endif //__RT3593_H__ //

/* End of rt3593.h */
