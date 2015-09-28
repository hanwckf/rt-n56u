#ifndef __NAND_DEF_H__
#define __NAND_DEF_H__

#define VERSION  	"v2.1 Fix AHB virt2phys error"
#define MODULE_NAME	"# MTK NAND #"
#define PROCNAME    "driver/nand"

#undef TESTTIME
#define __UBOOT_NAND__			1
//#define __KERNEL_NAND__		1
//#define __PRELOADER_NAND__	1
//#define PMT 1
//#define _MTK_NAND_DUMMY_DRIVER_
#define CONFIG_BADBLOCK_CHECK	1
#ifdef CONFIG_BADBLOCK_CHECK
//#define MTK_NAND_BMT	1
#endif
#define FACT_BBT		1
#define ECC_ENABLE		1
#define MANUAL_CORRECT	1
//#define __INTERNAL_USE_AHB_MODE__ 	(0)

#ifndef NAND_OTP_SUPPORT
#define NAND_OTP_SUPPORT 0
#endif

/*******************************************************************************
 * Macro definition 
 *******************************************************************************/
//#define NFI_SET_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) | (value))) 
//#define NFI_SET_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) | (value)))
//#define NFI_CLN_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) & (~(value))))
//#define NFI_CLN_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) & (~(value))))

#if defined (__KERNEL_NAND__)
#define NFI_SET_REG32(reg, value) \
do {	\
	g_value = (DRV_Reg32(reg) | (value));\
	DRV_WriteReg32(reg, g_value); \
} while(0)

#define NFI_SET_REG16(reg, value) \
do {	\
	g_value = (DRV_Reg16(reg) | (value));\
	DRV_WriteReg16(reg, g_value); \
} while(0)

#define NFI_CLN_REG32(reg, value) \
do {	\
	g_value = (DRV_Reg32(reg) & (~(value)));\
	DRV_WriteReg32(reg, g_value); \
} while(0)

#define NFI_CLN_REG16(reg, value) \
do {	\
	g_value = (DRV_Reg16(reg) & (~(value)));\
	DRV_WriteReg16(reg, g_value); \
} while(0)
#endif

#define NFI_WAIT_STATE_DONE(state) do{;}while (__raw_readl(NFI_STA_REG32) & state)
#define NFI_WAIT_TO_READY()  do{;}while (!(__raw_readl(NFI_STA_REG32) & STA_BUSY2READY))


#define NAND_SECTOR_SIZE (512)
#define OOB_PER_SECTOR      (16)
#define OOB_AVAI_PER_SECTOR (8)

#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE       (8)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE	(2)

#define TIMEOUT_1   0x1fff
#define TIMEOUT_2   0x8ff
#define TIMEOUT_3   0xffff
#define TIMEOUT_4   0xffff//5000   //PIO


/* temporarity definiation */
#if !defined (__KERNEL_NAND__) 
#define KERN_INFO
#define KERN_WARNING
#define KERN_ERR
#define PAGE_SIZE	(4096)
#endif
#define AddStorageTrace				//AddStorageTrace
#define STORAGE_LOGGER_MSG_NAND		0
#define NFI_BASE 					RALINK_NAND_CTRL_BASE
#define NFIECC_BASE 				RALINK_NANDECC_CTRL_BASE

#if (__INTERNAL_USE_AHB_MODE__)
#define MT65xx_POLARITY_LOW   0
#define MT65XX_PDN_PERI_NFI   0
#define MT65xx_EDGE_SENSITIVE 0
#define MT6575_NFI_IRQ_ID                    (58)
#endif

#if defined (__KERNEL_NAND__)
#else
#define CONFIG_MTD_NAND_VERIFY_WRITE	(1)
#define printk	printf
#define ra_dbg printf
#define BUG()							//BUG()
#define BUG_ON(x)						//BUG_ON()
#define NUM_PARTITIONS 				1
#endif

#define NFI_DEFAULT_ACCESS_TIMING        (0x00844333)

//uboot only support 1 cs
#define NFI_CS_NUM                  (1)
#define NFI_DEFAULT_CS              (0)

#include "mt6575_typedefs.h"

#endif /* __NAND_DEF_H__ */
