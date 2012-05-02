#ifndef _RTL8370_ASICDRV_PHY_H_
#define _RTL8370_ASICDRV_PHY_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_PHY_INTERNALNOMAX	    0x7
#define RTL8370_PHY_REGNOMAX		    0x1F
#define RTL8370_PHY_EXTERNALMAX	    0x7

#define	RTL8370_PHY_BASE   	        0x2000
#define	RTL8370_PHY_EXT_BASE   	    0xA000

#define	RTL8370_PHY_OFFSET	            5
#define	RTL8370_PHY_EXT_OFFSET  	    9
#define RTL8370_PHY_EXTLED_OFFSET  	8

#define	RTL8370_PHY_PAGE_ADDRESS       31


extern ret_t rtl8370_setAsicPHYReg( uint32 phyNo, uint32 phyAddr, uint32 data );
extern ret_t rtl8370_getAsicPHYReg( uint32 phyNo, uint32 phyAddr, uint32* data );

#endif /*#ifndef _RTL8370_ASICDRV_PHY_H_*/

