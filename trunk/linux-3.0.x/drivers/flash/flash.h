/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains all the Flash defines & function prototypes.
//
// Sp. Notes:
//    It is extremenly important that you follow the coding style. The utility
//    program will extract out the comment to generate the programmer's guide.
//
/******************************************************************************/

#ifndef FLASHDEF_H
#define FLASHDEF_H

/*=====================*
 *  Defines            *
 *=====================*/
#define POLARITY_BIT		((uint16_t) 0x0080)
#define TIMEOUT_BIT		((uint16_t) 0x0020)
#define ERASE_WINDOW		((uint16_t) 0x0008)

#define MAX_SECTOR_GROUPS	10  
#define KBYTES(x)		((x)*1024)
#define MAX_SECTOR_SIZE	    	KBYTES(64) /* the max sector size in all sectors */
#define MAX_FLSH_TBL		(sizeof(flsh_tbl) / sizeof(struct flsh_dev))

#if defined (CONFIG_RT2880_FLASH_8M)
#define FL_BASE                 (CONFIG_MTD_PHYSMAP_START - 0x400000)
#elif defined (CONFIG_RT2880_FLASH_16M)
#define FL_BASE                 (CONFIG_MTD_PHYSMAP_START - 0xC00000)
#else
#define FL_BASE                 (CONFIG_MTD_PHYSMAP_START)
#endif
/* Address interface to flash (which is always set to 16-bit mode):
 * When memory controller (MAC) is configured to access flash/ROM using
 * 16-bit width:  program address = flash address << 1
 * 32-bit width:  program address = flash address << 2
 */
#define FL_ADDR_SHIFT_16               (1)


/* Flash register addresses */
#define FL_MFR_ID                      (0x000)
#define FL_DEV_ID1                     (0x001)
#define FL_DEV_ID2                     (0x00E)
#define FL_DEV_ID3                     (0x00F)

// kaiker,define for SST
#define SST_FL_CMD1                    (0x5555)
#define SST_FL_CMD2                    (0x2AAA)
#define SST_FL_CMD1_REG_16             FL_REG_16(SST_FL_CMD1<<FL_ADDR_SHIFT_16)
#define SST_FL_CMD2_REG_16             FL_REG_16((SST_FL_CMD2 << FL_ADDR_SHIFT_16))
#define SST_FL_MANUFACT                (0xBF)

#define FL_CMD1                        (0x555)
#define FL_CMD2                        (0x2AA)

#define FL_REG_16(x)			(*((volatile uint16_t *)(FL_BASE + (x))))
#define FL_REG_32(x)			(*((volatile uint32_t *)(FL_BASE + (x))))

/* Defines of flash register locations: 16-bit access width */
#define FL_BASE_REG_16                 FL_REG_16(0)
#define FL_MFR_REG_16                  FL_REG_16(FL_MFR_ID<<FL_ADDR_SHIFT_16)
#define FL_DEV1_REG_16                 FL_REG_16(FL_DEV_ID1<<FL_ADDR_SHIFT_16)
#define FL_DEV2_REG_16                 FL_REG_16(FL_DEV_ID2<<FL_ADDR_SHIFT_16)
#define FL_DEV3_REG_16                 FL_REG_16(FL_DEV_ID3<<FL_ADDR_SHIFT_16)
#define FL_CMD1_REG_16                 FL_REG_16(FL_CMD1<<FL_ADDR_SHIFT_16)
#define FL_CMD2_REG_16                 FL_REG_16((FL_CMD2 << FL_ADDR_SHIFT_16))

/*=====================*
 *  External Functions *
 *=====================*/
uint32_t FlashReadManID( void );
uint32_t FlashReadDevID( void );
void FlashDumpSect(void);


int32_t FlashSectSize(uint32_t sector_num);
int32_t FlashSectNum(uint32_t sector_addr);
int32_t FlashSectAddr(uint32_t sector_num); 
int32_t FlashRead(uint32_t *dst, uint32_t *src, uint32_t numBytes);
int32_t FlashWrite(uint16_t *source, uint16_t *dest, uint32_t numBytes);
int32_t FlashVerify(uint16_t *source, uint16_t *dest, uint32_t numBytes);
int32_t FlashErase(uint32_t firstSector, uint32_t lastSector);
int32_t FlashGetSector(uint32_t addr, uint32_t *sect_num);


#endif /* FLASHDEF_H */
