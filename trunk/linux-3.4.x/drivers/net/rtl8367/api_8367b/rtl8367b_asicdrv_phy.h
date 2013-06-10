#ifndef _RTL8367B_ASICDRV_PHY_H_
#define _RTL8367B_ASICDRV_PHY_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_PHY_INTERNALNOMAX	    0x4
#define RTL8367B_PHY_REGNOMAX		    0x1F
#define RTL8367B_PHY_EXTERNALMAX	    0x7

#define	RTL8367B_PHY_BASE   	        0x2000
#define	RTL8367B_PHY_EXT_BASE   	    0xA000

#define	RTL8367B_PHY_OFFSET	            5
#define	RTL8367B_PHY_EXT_OFFSET  	    9

#define	RTL8367B_PHY_PAGE_ADDRESS       31


extern ret_t rtl8367b_setAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 regData );
extern ret_t rtl8367b_getAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32* pRegData );

#endif /*#ifndef _RTL8367B_ASICDRV_PHY_H_*/

