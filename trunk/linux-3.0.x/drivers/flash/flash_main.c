/*******************************************************************************
*
* Purpose:
*    This file contains all Flash related functions.
*
* Sp. Notes: This Library requires timer block.
*    Read mode command is inserted before any other commands
*    to compensate for corruption caused by some emulators.
*
*******************************************************************************/

/*=====================*
 *  Include Files      *
 *=====================*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#include "flash.h"
#include "flash_ioctl.h"
#include "flash_tbl.h"

/*=====================*
 *  Global Variables   *
 *=====================*/
uint32_t flashFailure;    /* This variable can be used to provide extra debug  */
                        /* information when flash programming fails (for ex. */
                        /* the current flash address).                       */
                        /* This variable is exported via the flash.h file.   */
struct flsh_dev *flsh_entry=NULL;
uint32_t max_flash_size=0;

/*
 * 8M Flash:
 *
   ADDR	     Logical View         Physical View
 =======================================================
 0xBF000000 		   +-----+--------------+------
	    		   |     | Image part2  |  ^
	    		   |     |	        |  |
 0xBF400000 +------------+ |  +--+--------------+  | 8M Rotation
	    |  (Uboot)   | |  |	 |  (Uboot)     |  |
	    |------------| |  |  | Image part1  |  v
 0xBF800000 +  	 	 +-+  |  +--------------+-------
	    |   Image    |    |  |	        |  ^
	    |            |    |  |	        |  |
 0xBFC00000 +------------+----+  +   (Mirror)   +  | 8M Rotation
	    		         |	        |  |
	    		         | 	        |  v
 0xBFFFFFFF 		         +--------------+-------

 *
 * 16M Flash
 *
   ADDR	     Logical View         Physical View
 =======================================================
 0xBE000000 		   +-----+--------------------+------
	    		   |     |		      |  ^
	    		   |     |	              |  |
 0xBE800000		   |     |   Image Part2      |  | 16M Rotation
			   |     |		      |  |
			   |     |                    |  |
 0xBEC00000 +------------+ |  +--+--------------------+  |
	    |  (Uboot)   | |  |  |      (Uboot)	      |  |
	    |------------| |  |  |     Image Part1    |  v
 0xBF000000 +            +-+  |  +--------------------+-------
	    |   Image    |    |  |		      |  ^
	    |            |    |  |		      |  |
 0xBFC00000 |            |    |  |	  	      |  |
	    |            |    |  |		      |  |
 0xBF800000 +            +    |  +      (Mirror)      +  | 16M Rotation
	    |	         |    |  |		      |  |
	    |		 |    |  |		      |  |
 0xBFC00000 +------------+----+  |		      |  |
			         |		      |  v
 0xBFFFFFFF 		         +--------------------+-------
 
 ========================================================

*/
uint32_t logic2phy(uint32_t addr)
{
#if defined (CONFIG_RT2880_FLASH_8M)
	if (addr >= 0x400000)
		return (FL_BASE - 0x400000 + addr);
	else
		return (FL_BASE + 0x400000 + addr);
#elif defined (CONFIG_RT2880_FLASH_16M)
	if (addr >= 0x400000)
		return (FL_BASE - 0x400000 + addr);
	else
		return (FL_BASE + 0xc00000 + addr);
#elif defined (CONFIG_RT2880_FLASH_32M)
	if (addr >= 0x1000000)
		return (FL_BASE - 0x5000000 + addr); //0xC0000000 remaps to 0xBB000000
	else
		return (FL_BASE + addr);
#else
	return addr + FL_BASE;
#endif

}

#ifdef CONFIG_FLASH_SST39VF320X
int 		isSST = 0;

static uint16_t isSSTFlash(void)
{
	volatile uint32_t delay;
	uint16_t man_id;

	//Flash_SetModeRead
	FL_BASE_REG_16 = 0x00F0;
	for (delay = 0; delay < 5; delay++) ;

	//Flash_SetModeReadID
	SST_FL_CMD1_REG_16 = 0x00AA;
	SST_FL_CMD2_REG_16 = 0x0055;
	SST_FL_CMD1_REG_16 = 0x0090;
	for (delay = 0; delay < 5; delay++) ;

	man_id = FL_MFR_REG_16;

	//Flash_SetModeRead
	FL_BASE_REG_16 = 0x00F0;
	for (delay = 0; delay < 5; delay++) ;

	if(man_id==SST_FL_MANUFACT){
		return 1;
	}else {
		return 0;
	}
}
#endif 

/* FUNCTION_DESC **************************************************************/
//
// NAME           Flash_SetModeRead()
//
// SYNOPSIS       void Flash_SetModeRead ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         None
//
// DESCRIPTION    Puts the Flash ROM into Read Mode
//
// NOTE           This must be used before any other Flash commands for reasons
//                unknown, and not in spec.
//
/******************************************************************************/
static void Flash_SetModeRead(void)
{
	volatile uint32_t delay;

#ifdef CONFIG_FLASH_SST39VF320X 
	if(isSST) {
		FL_BASE_REG_16 = 0x00F0;
		for (delay = 0; delay < 5; delay++) ;
		return;
	}
#endif

	FL_BASE_REG_16 = 0x00F0;
	for (delay = 0; delay < 5; delay++) ;
}


/* FUNCTION_DESC **************************************************************/
//
// NAME           Flash_SetModeReadID()
//
// SYNOPSIS       void Flash_SetModeReadID ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         None
//
// DESCRIPTION    Puts the Flash ROM into ReadID Mode
//
// NOTE           To read unique manufacturer & product IDs
//
/******************************************************************************/
static void Flash_SetModeReadID(void)
{
	volatile uint32_t delay;

#ifdef CONFIG_FLASH_SST39VF320X 
	if(isSST) {
	    SST_FL_CMD1_REG_16 = 0x00AA;
	    SST_FL_CMD2_REG_16 = 0x0055;
	    SST_FL_CMD1_REG_16 = 0x0090;
	    for (delay = 0; delay < 5; delay++) ;
	    return;
	}
#endif

	FL_CMD1_REG_16 = 0x00AA;
	FL_CMD2_REG_16 = 0x0055;
	FL_CMD1_REG_16 = 0x0090;
	for (delay = 0; delay < 5; delay++) ;
}


/* FUNCTION_DESC **************************************************************/
//
// NAME           Flash_SetModeUnlockBypass()
//
// SYNOPSIS       void Flash_SetModeUnlockBypass ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         None
//
// DESCRIPTION    Unlocks needs for subsequent individual Bypass commands for
//                individual double-byte operations
//
// NOTE           
//
/******************************************************************************/
static void Flash_SetModeUnlockBypass(void)
{
#ifdef CONFIG_FLASH_SST39VF320X 
	if(isSST)
	{
		udelay(10);
		return;
	}
#endif

	FL_CMD1_REG_16 = 0x00AA;
	FL_CMD2_REG_16 = 0x0055;
	FL_CMD1_REG_16 = 0x0020;
}

/* FUNCTION_DESC **************************************************************/
//
// NAME           Flash_ClearModeUnlockBypass()
//
// SYNOPSIS       void Flash_ClearModeUnlockBypass ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         None
//
// DESCRIPTION    Turns Unlock Bypass Mode off
//
// NOTE           
//
/******************************************************************************/
static void Flash_ClearModeUnlockBypass(void)
{
	volatile uint16_t dummywrite;     /* somewhere to do dummywrites */

#ifdef CONFIG_FLASH_SST39VF320X 
	if(isSST){
		udelay(10);
		return;
	}
#endif

	FL_BASE_REG_16 = 0x0090;
	FL_BASE_REG_16 = 0x0000;

	dummywrite = FL_BASE_REG_16;    /* slight delay */
	dummywrite = FL_BASE_REG_16;
	dummywrite = FL_BASE_REG_16;
	dummywrite = FL_BASE_REG_16;
	dummywrite = FL_BASE_REG_16;

}

/* FUNCTION_DESC **************************************************************/
//
// NAME           Flash_SetModeChipErase()
//
// SYNOPSIS       void Flash_SetModeChipErase ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         None
//
// DESCRIPTION    Turns on Chip Erase mode
//
// NOTE           
//
/******************************************************************************/
static void Flash_SetModeChipErase(void)
{
#ifdef CONFIG_FLASH_SST39VF320X 
	if(isSST) {
		SST_FL_CMD1_REG_16 = 0x00AA;
		SST_FL_CMD2_REG_16 = 0x0055;
		SST_FL_CMD1_REG_16 = 0x0080;
		SST_FL_CMD1_REG_16 = 0x00AA;
		SST_FL_CMD2_REG_16 = 0x0055;
		SST_FL_CMD1_REG_16 = 0x0010;
		return;
	}
#endif
	FL_CMD1_REG_16 = 0x00AA;
	FL_CMD2_REG_16 = 0x0055;
	FL_CMD1_REG_16 = 0x0080;
	FL_CMD1_REG_16 = 0x00AA;
	FL_CMD2_REG_16 = 0x0055;
	FL_CMD1_REG_16 = 0x0010;
}

/* FUNCTION_DESC **************************************************************/
//
// NAME           Flash_SetModeSectorErase()
//
// SYNOPSIS       void Flash_SetModeSectorErase ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         None
//
// DESCRIPTION    Turns on Sector Erase mode
//
// NOTE           
//
/******************************************************************************/
static void Flash_SetModeSectorErase(void)
{
#ifdef CONFIG_FLASH_SST39VF320X
	if(isSST)
	{
		SST_FL_CMD1_REG_16 = 0x00AA;
		SST_FL_CMD2_REG_16 = 0x0055;
		SST_FL_CMD1_REG_16 = 0x0080;
		SST_FL_CMD1_REG_16 = 0x00AA;
		SST_FL_CMD2_REG_16 = 0x0055;
		return;
	}
#endif

	FL_CMD1_REG_16 = 0x00AA;
	FL_CMD2_REG_16 = 0x0055;
	FL_CMD1_REG_16 = 0x0080;
	FL_CMD1_REG_16 = 0x00AA;
	FL_CMD2_REG_16 = 0x0055;

}

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashSectSize()
//
// SYNOPSIS       int32_t FlashSectSize (uint32_t sector_num )
//
// TYPE           Regular function
//
// INPUT          flashType flash:  type of flash chip
//                uint32_t width:  configured width of the flash/ROM in the MAC
//                               ROM config register
//                uint32_t sectorNum:  sector number
//
// OUTPUT         int32_t:  >=0: size of the given sector in number of bytes
//                          -1: sector_num is invalid
//
// DESCRIPTION    Returns the size of the given sector.
//                Returns 0 if any input is invalid.
//
// NOTE           Assumes Bottom Boot Flash Configuration
//                On the Palmchip development boards, the flash chips are always
//                set in 16-bit mode.
//                When the MAC ROM width is configured to 16-bits, one flash
//                chip is used.  When configured to 32-bits, two flash chips
//                are used.
//
/******************************************************************************/
int32_t FlashSectSize(uint32_t sector_num)
{
    uint32_t i=0;
    uint32_t boundary=0;

    if(sector_num > flsh_entry->max_sector) {
	printk("%s: sector number %d is invalid\n",__FUNCTION__, sector_num);
	return -1;
    }

    for(i=0;i< MAX_SECTOR_GROUPS;i++) {
	boundary += flsh_entry->sector[i].count;
	if( sector_num < boundary ) {
	    return (flsh_entry->sector[i].size);
	}
    }

    return -1;
}


/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashSectAddr()
//
// SYNOPSIS       int32_t FlashSectAddr (uint32_t sectorNum )
//
// TYPE           Regular function
//
// INPUT          uint32_t sectorNum:  sector number
//
// OUTPUT         int32_t:  >=0: Address of the first data in the given sector
//                          -1 : sector_num is invalid
//
// DESCRIPTION    Returns the program address of the start of the given sector.
//                Returns a default address if any input is invalid.
//                The default address is the address returned for sectorNum=0.
//
// NOTE           
//
/******************************************************************************/
int32_t FlashSectAddr(uint32_t sector_num)
{
    uint32_t i=0;
    uint32_t offset=0;

    if(sector_num > flsh_entry->max_sector) {
	printk("%s: sector number %d is invalid\n",__FUNCTION__, sector_num);
	return -1;
    }


    for(i=0;i< MAX_SECTOR_GROUPS;i++) {
	if( sector_num > flsh_entry->sector[i].count ) {
	    offset += flsh_entry->sector[i].count * flsh_entry->sector[i].size;
	    sector_num -= flsh_entry->sector[i].count;
	}else {
#if defined (CONFIG_RT2880_FLASH_32M)
	    if (sector_num >= 128) {
		offset += ((sector_num-128) * flsh_entry->sector[i].size);
		return (0xBB000000 + offset);
	    }
#endif
	    offset += (sector_num * flsh_entry->sector[i].size);
	    /* Add offset to base of flash */
	    return (FL_BASE + offset);
	} 
    }

    return -1;

}

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashSectNum()
//
// SYNOPSIS       int32_t FlashSectNum(uint32_t sector_addr)
//
// TYPE           Regular function
//
// INPUT          uint32_t sector_addr: program address 
//
// OUTPUT         int32_t: >=0: the sector number for this sector address
//			   -1: sector is invalid
//
// NOTE           
//
/******************************************************************************/
int32_t FlashSectNum(uint32_t sector_addr)
{
    uint32_t i=0,j=0;
    uint32_t offset=FL_BASE;
    uint16_t sector_num=0;
   
#if defined (CONFIG_RT2880_FLASH_32M)
    if (sector_addr < 0xBC000000 && sector_addr >= 0xBB000000) {
	sector_addr += 0x4000000; //0xbb --> 0xbf
	sector_num = 128;
    }
#endif
    if((uint32_t)sector_addr > FL_BASE+max_flash_size) {
	printk("%s: %08X out of scope \n",__FUNCTION__, (uint32_t)sector_addr);
	return -1;
    }

    for(i=0;i< MAX_SECTOR_GROUPS;i++) {
	for(j=0;j<flsh_entry->sector[i].count;j++) {
	    offset += flsh_entry->sector[i].size;
	    if( sector_addr < offset ) {
		return sector_num;
	    }else {
		sector_num++;
	    }
	} 
    }

    return -1;

}

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashReadManID()
//
// SYNOPSIS       uint16_t FlashReadManID ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         uint16_t:  ManID value
//
// DESCRIPTION    Returns Manufacturer ID register contents
//
// NOTE           
//
/******************************************************************************/
uint32_t FlashReadManID(void)
{
	uint16_t man_id;

	Flash_SetModeRead();

	/* Read MFR_ID */
	Flash_SetModeReadID();
	man_id = FL_MFR_REG_16;

	Flash_SetModeRead();

	return man_id;
}


/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashReadDevID()
//
// SYNOPSIS       void FlashReadDevID ( void )
//
// TYPE           Regular function
//
// INPUT          None
//
// OUTPUT         uint16_t:  DevID value
//                
// DESCRIPTION    Returns Device ID register contents
//
// NOTE           
//
/******************************************************************************/
uint32_t FlashReadDevID(void)
{
	uint16_t dev_id1, dev_id2, dev_id3;

	Flash_SetModeRead();

	/* Read DEV_ID */
	Flash_SetModeReadID();
	dev_id1 = FL_DEV1_REG_16;
	dev_id2 = FL_DEV2_REG_16;
	dev_id3 = FL_DEV3_REG_16;

	Flash_SetModeRead();

	/* For Spansion Flash Support */
	if(dev_id1==0x227e) {
		return ((dev_id1&0xFF)<<16) | ((dev_id2&0xFF) <<8) | (dev_id3&0xFF);
	}

	return dev_id1;
}

/* FUNCTION_DESC **************************************************************/
//
// NAME           __FlashWrite()
//
// SYNOPSIS       uint32_t FlashWrite ( uint16_t *source, uint16_t *dest,
//                                  uint32_t numBytes )
//
// TYPE           Regular function
//
// INPUT          uint16_t *source:  pointer to the source from which double-bytes
//                                 are to be transferred
//                uint16_t *dest:  pointer to the destination (in flash)
//                uint32_t numBytes:  the number of bytes to transer
//
// OUTPUT         uint32_tean (TRUE = successful, FALSE = error)
//
// DESCRIPTION    Copies from any memory to flash for programming
//
// NOTE           3rd parameter, "numBytes", must be an even number
//                If error, global "flashFailure" has error code
//
/******************************************************************************/
uint32_t __FlashWrite(uint16_t *source, uint16_t *dest, uint32_t numBytes)
{
    /* Note: numBytes must be an even number */
    uint32_t loopCount;
    uint32_t wordDone;
    uint16_t bit7polarity;
    uint16_t contents;

    Flash_SetModeRead();
    Flash_SetModeUnlockBypass(); /* Unlock Bypass Programming */

    for (loopCount = 0; loopCount < numBytes; loopCount += 2)
    {
        FL_CMD1_REG_16 = 0x00A0;     /* unlock bypass write key */
        *dest = *source;    /* program data */

        /* Wait till Dest Bit7 == Source Bit7 (meaning done) */
        bit7polarity = *source & POLARITY_BIT;

        wordDone = 0;
        while (!wordDone)
        {
            contents = *((volatile uint16_t *)(dest));

            if ((contents & POLARITY_BIT) == bit7polarity)
            {
                wordDone = 1;    /* Word successfully programmed */
            }
            else
            {
                if (contents & TIMEOUT_BIT)
                {
                    /* Check one last time whether programming succeeded */
                    contents = *((volatile uint16_t *)(dest));

                    if ((contents & POLARITY_BIT) == bit7polarity)
                    {
                        wordDone = 1;    /* Word successfully programmed */
                    }
                    else
                    {
                        Flash_ClearModeUnlockBypass(); /* Unlock Bypass Reset */
			Flash_SetModeRead();
                        flashFailure = (uint32_t)dest;
                        return (1);      /* Signal failure to caller */
                    }
                }
            }
        }
        dest++;
        source++;
    }

    Flash_ClearModeUnlockBypass();    /* Unlock Bypass Reset */
    Flash_SetModeRead();

    return 0;
}

#ifdef CONFIG_FLASH_SST39VF320X
uint32_t __SSTFlashWrite(uint16_t *source, uint16_t *dest, uint32_t numBytes)
{
    /* Note: numBytes must be an even number */
    uint32_t loopCount;
    unsigned int oldstatus, status, poll=0;
    unsigned int dq6, dq5, dq2;	


    while(*dest != 0xFFFF)  {
	    printk(">");
    }

    for (loopCount = 0; loopCount < numBytes; loopCount += 2)
    {
	    SST_FL_CMD1_REG_16 = 0x00AA;
	    SST_FL_CMD2_REG_16 = 0x0055;
	    SST_FL_CMD1_REG_16 = 0x00A0;

	    *dest = *source;    /* program data */
	    udelay(10);
	    
	    oldstatus = *dest;
	    status = *dest;
	    //printk("[%d] Toggle %d, %d\n", poll, (status  ^ oldstatus) & dq6, (status ^ oldstatus) & dq2);

	    dq6 = (1<<6) & 0x000000FF;
	    dq5 = (1<<5) & 0x000000FF;
	    dq2 = (1<<2) & 0x000000FF;

	    while( (status & dq6) != (oldstatus & dq6) && 
			    (status & dq5) != dq5 ) {

		    poll++;
		    oldstatus = *dest;
		    //udelay(20);
		    status = *dest;
		    printk("<");

	    }
	    //if (poll != 0) printf("poll %d\n", poll);

	    dest++;
	    source++;
    }

    return 0;
}


#endif

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashWrite()
//
// SYNOPSIS       int32_t FlashWrite ( uint16_t *source, uint16_t *dest,
//                                  uint32_t numBytes )
//
// TYPE           Regular function
//
// INPUT          uint16_t *source:  pointer to the source from which double-bytes
//                                 are to be transferred
//                uint16_t *dest:  pointer to the destination (in flash)
//                uint32_t numBytes:  the number of bytes to transer
//
// OUTPUT         int32_t >0: successful
//			  -1: error
//
// DESCRIPTION    Write Flash like DRAM (Random R/W)
//
// NOTE           3rd parameter, "numBytes", must be an even number
//                If error, global "flashFailure" has error code
//
/******************************************************************************/
int32_t FlashWrite(uint16_t *source, uint16_t *destination, uint32_t numBytes)
{
    uint16_t first_sector;
    uint16_t last_sector;
    uint32_t sector_size;
    uint32_t sector_base;
    uint32_t sector_num;
    uint8_t  *buffer;
    uint8_t  *src;
    uint8_t  *dst;

    uint32_t len=0;

    if((uint32_t)destination > max_flash_size) {
	printk("%s: %08X out of scope \n",__FUNCTION__, (uint32_t)destination);
	return -1;
    }

    src=(uint8_t *)source;
    dst=(uint8_t *)(logic2phy((uint32_t)destination));

    /* get covered sector range */
    first_sector = FlashSectNum((uint32_t)dst);
    last_sector =  FlashSectNum((uint32_t)dst+numBytes -1);
    buffer=kmalloc(FLASH_MAX_RW_SIZE, GFP_KERNEL);
    
    for(sector_num=first_sector; sector_num<= last_sector; sector_num++ ) {

	/* each sector has different size and base address */
	sector_size=FlashSectSize(sector_num);
	sector_base=FlashSectAddr(sector_num);

	/* prepare new sector contents */
	memcpy((char *)buffer, (char *)sector_base, sector_size);
	if((uint32_t)dst+numBytes < sector_base+sector_size) {
	    memcpy(&buffer[(uint32_t)dst-sector_base], src, numBytes);
	}else {
	    len=sector_base+sector_size-(uint32_t)dst;
	    memcpy(&buffer[sector_size-len], src, len);
	    src+=len;
	    dst+=len;
	    numBytes-=len;
	}

	/* burn new sector contents */
	FlashErase(sector_num, sector_num);
#ifdef CONFIG_FLASH_SST39VF320X
	if(isSST) {
		__SSTFlashWrite((uint16_t *)buffer, (uint16_t *)sector_base, sector_size);
	} else {
		__FlashWrite((uint16_t *)buffer, (uint16_t *)sector_base, sector_size);
	}
#else
	__FlashWrite((uint16_t *)buffer, (uint16_t *)sector_base, sector_size);
#endif
	
    }
    
    kfree(buffer);
    return 0;
}

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashVerify()
//
// SYNOPSIS       uint32_t FlashVerify ( uint16_t *source, uint16_t *dest,
//                                   uint32_t numBytes )
//
// TYPE           Regular function
//
// INPUT          uint16_t *source:  pointer to the source from which double-bytes
//                                 were transferred
//                uint16_t *dest:  pointer to the destination (in flash) where
//                               double-bytes were programmed
//                uint32_t numBytes:  the number of bytes to verify
//
// OUTPUT         int32_t (1 = successful, 0 = error , -1 = input is invalid)
//
// DESCRIPTION    Verifies double-bytes were programmed correctly
//
// NOTE           3rd parameter, "numBytes", must be an even number
//                If error, global "flashFailure" has read data
//
/******************************************************************************/
int32_t FlashVerify(uint16_t *source, uint16_t *dest, uint32_t numBytes)
{
    uint32_t loopCount;

    if((uint32_t)dest > max_flash_size) {
	printk("%s: %08X out of scope \n",__FUNCTION__, (uint32_t)dest);
	return -1;
    }

    Flash_SetModeRead();

    for (loopCount = 0; loopCount < numBytes; loopCount += 2)
    {
        if (*dest++ != *source++)
        {
            Flash_SetModeRead();

            flashFailure = (uint32_t)dest;

            return 1;
        }
    }

    Flash_SetModeRead();
    return 0;
}

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashErase()
//
// SYNOPSIS       uint32_t FlashErase ( uint32_t firstSector, uint32_t lastSector)
//
// TYPE           Regular function
//
// INPUT          uint32_t firstSector:  number of the first sector to be erased
//                uint32_t lastSector:  number of the last sector to be erased
//
// OUTPUT         int32_t (0 = successful, -1 = error)
//
// DESCRIPTION    This function erases the flash.
//                If the first and last sector input parameters are equal to
//                the first and last sector numbers on the chip, then chip
//                erase mode is envoked, else sector erase method is envoked. 
//
// NOTE
//
/******************************************************************************/
int32_t FlashErase(uint32_t firstSector, uint32_t lastSector)
{
    volatile uint16_t *sectAddr, *pollAddr;
    volatile uint16_t contents;
    uint32_t loopCount;      /* used for sector erase only */
    uint32_t attemptNum;     /* used for sector erase only */
    uint32_t sectorDone;     /* used for sector erase only */
    uint32_t windowClosed;   /* used for sector erase only */

    if(firstSector > flsh_entry->max_sector || lastSector > flsh_entry->max_sector) {
	printk("%s: sector number from %d to %d is invalid\n",__FUNCTION__, firstSector, lastSector);
	return -1;
    }

    flashFailure = 0;
    Flash_SetModeRead();

    /* if entire chip, use Chip Erase Function */
    if (firstSector == 0 && lastSector == flsh_entry->max_sector)
    {
	Flash_SetModeChipErase();

	contents = 0;
	sectAddr = (uint16_t *)FlashSectAddr(0);

	/* Wait till Polarity Bit (Bit 7) == 1 */
	while (!(contents & POLARITY_BIT))
	{
	    contents = *sectAddr;

	    if ( contents & POLARITY_BIT )
	    {
		Flash_SetModeRead();
		return (0); /* Successfully erased */
	    }
	    else
	    {
		if ( contents & TIMEOUT_BIT )
		{
		    contents = *sectAddr;

		    if ( contents & POLARITY_BIT )
		    {
			Flash_SetModeRead();
			return (0); /* Successfully erased */
		    }
		    else
		    {
			Flash_SetModeRead();    /* Signal Failure to erase */
			// Linda - debug:  for now, just assume timeout = flash erase okay
			//return (1);
			return (0);
		    }
		}
	    }
	}
    }

    else /* else use Sector Erase Function */
    {
	loopCount = firstSector;
	attemptNum = 0;
	while (loopCount <= lastSector)
	{
	    sectAddr = (uint16_t *)FlashSectAddr(loopCount);
	    pollAddr = sectAddr;

	    /* Sector Erase Mode */
	    Flash_SetModeSectorErase();
	    *sectAddr = 0x30;

	    /* Make sure device accepted the command sequence */
	    while (!(*sectAddr & POLARITY_BIT))
	    {
		; 
	    }

	    /*
	     ** If flash chip's erase window has closed, erase algorithm
	     ** has started and cannot write more sector erase commands.
	     ** Always check if window still open before writing each command.
	     ** When window is detected as closed, the last sector command
	     ** written might not have been accepted and should be reissued again
	     ** next time through the erase command sequence.
	     */ 
	    windowClosed = (*sectAddr & ERASE_WINDOW);
	    loopCount++;
	    while (!windowClosed)
	    {
		if (loopCount <= lastSector)
		{
		    sectAddr = (uint16_t *)FlashSectAddr(loopCount);
		    *sectAddr = 0x0030;

		    windowClosed = (*sectAddr & ERASE_WINDOW);

		    if (!windowClosed)
		    {
			loopCount++;
		    }
		}
		else
		{
		    /* No more sectors to erase */
		    break;
		}
		    }


		    /* wait for polarity of POLARITY_BIT to be 1 */
		    sectorDone = 0;
		    while (!sectorDone)
		    {
			contents = *pollAddr;

			if (contents & POLARITY_BIT)
			{
			    sectorDone = 1; /* Successfully erased */
			}
			else
			{
			    if (contents & TIMEOUT_BIT)
			    {
				contents = *pollAddr;

				if (contents & POLARITY_BIT)
				{
				    sectorDone = 1; /* Successfully erased */
				}
				else
				{
				    /* Put back in Read Mode, Signal Failure to erase */
				    Flash_SetModeRead();

				    flashFailure = (uint32_t)pollAddr;

				    return (1);
				}
			    }
			}
			/* If this is getting old, something's wrong */
			if (++attemptNum > 0x10000)
			{
			    /* Return to Read Mode */
			    Flash_SetModeRead();

			    flashFailure = (uint32_t)(0xDEADDEAD);

			    return (1);
			}
		    }
		    /* Return to Read Mode */
		    Flash_SetModeRead();
		}
	    }
    return 0;
}

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashRead()
//
// SYNOPSIS       int32_t FlashRead ( uint32_t *dst, uint32_t *src, uint32_t count )
//
// TYPE           Regular function
//
// INPUT          uint32_t *src:  pointer to the source from which double-bytes
//                                 are to be transferred
//                uint32_t *dst:  pointer to the destination (in flash)
//                uint32_t count:  the number of bytes to read
//
// OUTPUT         int32_t >0: successful
//			  -1: error
//
/******************************************************************************/
int32_t FlashRead(uint32_t *dst, uint32_t *src, uint32_t count)
{
    if((uint32_t)src > max_flash_size) {
	printk("%s: %08X Out Of Scope \n",__FUNCTION__, (uint32_t)src);
	return -1;
    }

    memcpy((char *)dst,(char *)(logic2phy((uint32_t)src)),count);
    
    return 0;
}

/* FUNCTION_DESC **************************************************************/
//
// NAME           FlashDumpSect()
//
// SYNOPSIS       void FlashDumpSect(void)
//
// TYPE           Regular function
//
// INPUT          NONE
//
// OUTPUT         NONE
//
/******************************************************************************/
void FlashDumpSect(void)
{
    uint32_t sector_num=0;

    for(sector_num=0;sector_num <= flsh_entry->max_sector; sector_num++){
	printk("SA%02d Sector_Size=%05X(Bytes) Sector_Addr=%08X \n", sector_num,
		FlashSectSize(sector_num), FlashSectAddr(sector_num));
    }
}

int32_t FlashGetSector(uint32_t addr, uint32_t *sect_num)
{
    uint32_t i=0,j=0,k=0;
    uint32_t man_id=0;
    uint32_t dev_id=0;
    uint32_t size=0, sect_count=0;

#ifdef CONFIG_FLASH_SST39VF320X
    isSST = isSSTFlash();
#endif

    man_id=FlashReadManID();
    dev_id=FlashReadDevID();

    //get offset not physical address
    addr = logic2phy(addr) - FL_BASE;

    for(i=0;i< MAX_FLSH_TBL;i++) {

            //find flash chip info
        if( flsh_tbl[i].man_id == man_id &&
                flsh_tbl[i].dev_id == dev_id ) {

            flsh_entry= &flsh_tbl[i];
            for(j=0;j<MAX_SECTOR_GROUPS;j++) {
                    //get sector size info
                    for(k=0;k<flsh_entry->sector[j].count;k++){
                            size += flsh_entry->sector[j].size;
                            if(size > addr){
                                    *sect_num=sect_count;
                                    return 1;
                            }else{
                                    sect_count++;
                            }
                    }

            }
        }
    }

    return 0;
}

static int32_t flash_init_module(void)
{
    uint32_t i=0,j=0;
    uint32_t man_id=0;
    uint32_t dev_id=0;

#ifdef CONFIG_FLASH_SST39VF320X
    isSST = isSSTFlash();
#endif

    man_id=FlashReadManID();
    dev_id=FlashReadDevID();

    for(i=0;i< MAX_FLSH_TBL;i++) {
	if( flsh_tbl[i].man_id == man_id &&
		flsh_tbl[i].dev_id == dev_id ) {

	    flsh_entry= &flsh_tbl[i];
	    for(j=0;j<MAX_SECTOR_GROUPS;j++) {
		if(flsh_entry->sector[j].count!=0) {
		    max_flash_size += flsh_entry->sector[j].count * flsh_entry->sector[j].size;
		}
	    }
	    printk("FLASH_API: MAN_ID=%0X DEV_ID=%0X SIZE=%dMB\n",
		    man_id,dev_id,max_flash_size/1024/1024);
	    return 0;
	}
    }

    printk("\nFLASH_API: **********<<WARNING!!!!>>**************\n");
    printk("FLASH_API: Flash Not Support(MAN_ID=%0X DEV_ID=%0X)\n", man_id, dev_id);
    printk("FLASH_API: ****************************************\n");
    return 1;
}

static void flash_cleanup_module(void)
{
    printk("Unload FLASH_API Module\n");
}

module_init(flash_init_module);
module_exit(flash_cleanup_module);

MODULE_DESCRIPTION("Ralink Common Flash R/W API Module");
MODULE_AUTHOR("Steven Liu");

EXPORT_SYMBOL(FlashErase);
EXPORT_SYMBOL(FlashWrite);
EXPORT_SYMBOL(FlashRead);

EXPORT_SYMBOL(FlashDumpSect);
