#ifndef __MTK_NAND_DEF_H__
#define __MTK_NAND_DEF_H__

#define MTK_NAND_MODULE_VERSION		"v2.1"
#define MTK_NAND_MODULE_TEXT		"MTK NFI"

#define ECC_ENABLE			1
#define ECC_MANUAL_CORRECT		1

#define SKIP_BAD_BLOCK
#define LOAD_FACT_BBT

#if defined (LOAD_FACT_BBT)
#define FACT_BBT_POOL_SIZE		(4)
#define FACT_BBT_BLOCK_NUM		(32) // use the latest 32 BLOCK for factory bbt table
#endif

#define NFI_NAND_ECC_SIZE		(2048)
#define NFI_NAND_ECC_BYTES		(32)
#define NFI_NAND_SECTOR_SIZE		(512)
#define OOB_AVAIL_PER_SECTOR		(8)
#define OOB_BAD_BLOCK_OFFSET		(0)

#define NFI_DEFAULT_ACCESS_TIMING	(0x00844333)

// ---------------------------------------------------------------------------
//  Devices table
// ---------------------------------------------------------------------------

#define ADV_MODE_RANDOM_READ		(1<<0)
#define ADV_MODE_CACHE_READ		(1<<1)

typedef struct
{
	char *devicename;
	u16 id;          //deviceid+manuid
	u32 ext_id;
	u8 addr_cycle;
	u8 iowidth;
	u16 totalsize;
	u16 blocksize;
	u16 pagesize;
	u16 sparesize;
	u32 timmingsetting;
	u32 advancedmode;
} flashdev_info, *pflashdev_info;

// ---------------------------------------------------------------------------
//  Basic Type Definitions
// ---------------------------------------------------------------------------

typedef volatile unsigned short	*P_U16;
typedef volatile unsigned int	*P_U32;

typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;

// ---------------------------------------------------------------------------
//  Register Manipulations
// ---------------------------------------------------------------------------

#define READ_REGISTER_UINT32(reg)	(*(volatile UINT32 * const)(reg))
#define READ_REGISTER_UINT16(reg)	(*(volatile UINT16 * const)(reg))
#define READ_REGISTER_UINT8(reg)	(*(volatile UINT8 * const)(reg))
#define WRITE_REGISTER_UINT32(reg, val)	(*(volatile UINT32 * const)(reg)) = (val)
#define WRITE_REGISTER_UINT16(reg, val)	(*(volatile UINT16 * const)(reg)) = (val)
#define WRITE_REGISTER_UINT8(reg, val)	(*(volatile UINT8 * const)(reg)) = (val)

#define INREG8(x)			READ_REGISTER_UINT8((UINT8*)((void*)(x)))
#define OUTREG8(x, y)			WRITE_REGISTER_UINT8((UINT8*)((void*)(x)), (UINT8)(y))
#define SETREG8(x, y)			OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)			OUTREG8(x, INREG8(x)&~(y))
#define MASKREG8(x, y, z)		OUTREG8(x, (INREG8(x)&~(y))|(z))

#define INREG16(x)			READ_REGISTER_UINT16((UINT16*)((void*)(x)))
#define OUTREG16(x, y)			WRITE_REGISTER_UINT16((UINT16*)((void*)(x)),(UINT16)(y))
#define SETREG16(x, y)			OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)			OUTREG16(x, INREG16(x)&~(y))
#define MASKREG16(x, y, z)		OUTREG16(x, (INREG16(x)&~(y))|(z))

#define INREG32(x)			READ_REGISTER_UINT32((UINT32*)((void*)(x)))
#define OUTREG32(x, y)			WRITE_REGISTER_UINT32((UINT32*)((void*)(x)), (UINT32)(y))
#define SETREG32(x, y)			OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)			OUTREG32(x, INREG32(x)&~(y))
#define MASKREG32(x, y, z)		OUTREG32(x, (INREG32(x)&~(y))|(z))

#define DRV_Reg8(addr)			INREG8(addr)
#define DRV_WriteReg8(addr, data)	OUTREG8(addr, data)
#define DRV_SetReg8(addr, data)		SETREG8(addr, data)
#define DRV_ClrReg8(addr, data)		CLRREG8(addr, data)

#define DRV_Reg16(addr)			INREG16(addr)
#define DRV_WriteReg16(addr, data)	OUTREG16(addr, data)
#define DRV_SetReg16(addr, data)	SETREG16(addr, data)
#define DRV_ClrReg16(addr, data)	CLRREG16(addr, data)

#define DRV_Reg32(addr)			INREG32(addr)
#define DRV_WriteReg32(addr, data)	OUTREG32(addr, data)
#define DRV_SetReg32(addr, data)	SETREG32(addr, data)
#define DRV_ClrReg32(addr, data)	CLRREG32(addr, data)

#define NFI_SET_REG32(reg, value) \
do { \
	u32 val = (DRV_Reg32(reg) | (value));\
	DRV_WriteReg32(reg, val); \
} while(0)

#define NFI_SET_REG16(reg, value) \
do { \
	u32 val = (DRV_Reg16(reg) | (value));\
	DRV_WriteReg16(reg, val); \
} while(0)

#define NFI_CLN_REG32(reg, value) \
do { \
	u32 val = (DRV_Reg32(reg) & (~(value)));\
	DRV_WriteReg32(reg, val); \
} while(0)

#define NFI_CLN_REG16(reg, value) \
do { \
	u32 val = (DRV_Reg16(reg) & (~(value)));\
	DRV_WriteReg16(reg, val); \
} while(0)


#endif /* __MTK_NAND_DEF_H__ */
