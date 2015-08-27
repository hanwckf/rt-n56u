#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include "../mtdcore.h"

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboardint.h>

#include <ralink/mtk_nand_dev.h>

#include "mtk_nand_def.h"
#include "mtk_nand.h"

#include "ralink-flash.h"
#if defined (CONFIG_MTD_NAND_USE_UBI_PART)
#include "ralink-nand-map-ubi.h"
#else
#include "ralink-nand-map.h"
#endif

#if defined (CONFIG_MTD_UBI) || defined (CONFIG_MTD_UBI_MODULE)
#define UBIFS_ECC_0_PATCH
#if defined (CONFIG_MTD_NAND_USE_UBI_PART)
#define UBI_PART_START_OFFSET	NAND_MTD_UBI_PART_OFFSET
#else
#define UBI_PART_START_OFFSET	NAND_MTD_RWFS_PART_OFFSET
#endif
#endif

#if defined(SKIP_BAD_BLOCK)
static int shift_on_bbt = 0;
static int is_skip_bad_block(struct mtd_info *mtd, int page);
extern void nand_bbt_set_bad(struct mtd_info *mtd, int page);
extern int nand_bbt_get(struct mtd_info *mtd, int page);
#endif
static int mtk_nand_read_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page);
static int mtk_nand_read_oob_raw(struct mtd_info *mtd, uint8_t * buf, int page_addr, int len);

extern void nand_release_device(struct mtd_info *mtd);
extern int nand_get_device(struct mtd_info *mtd, int new_state);

int mtk_nand_erase_hw(struct mtd_info *mtd, int page);
int mtk_nand_block_markbad_hw(struct mtd_info *mtd, loff_t ofs);

/*******************************************************************************
 * Gloable Varible Definition
 *******************************************************************************/

static u8 local_buffer[4096 + 512];
static u8 *local_buffer_16_align;   // 16 byte aligned buffer, for HW issue

static u8 local_oob_buf[NAND_MAX_OOBSIZE];

static flashdev_info nand_devinfo;

static struct NAND_CMD g_kCMD;
static bool g_bcmdstatus;

static struct nand_ecclayout nand_oob_16 = {
	.eccbytes = 8,
	.eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
	.oobfree = {{1, 6}, {0, 0}}
};

static struct nand_ecclayout nand_oob_64 = {
	.eccbytes = 32,
	.eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

static struct nand_ecclayout nand_oob_128 = {
	.eccbytes = 64,
	.eccpos = {
		64, 65, 66, 67, 68, 69, 70, 71,
		72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 86,
		88, 89, 90, 91, 92, 93, 94, 95,
		96, 97, 98, 99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125, 126, 127},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

static const flashdev_info gen_FlashTable[]= {
	/* Macronix */
	{"MX30LF1G08AA",    0xC2F1, 0x801DC2, 4, 8,  128, 128, 2048,  64, 0x00844333, 0},
	{"MX30LF1G18AC",    0xC2F1, 0x809502, 4, 8,  128, 128, 2048,  64, 0x00444332, 0},
	{"MX30LF2G18AC",    0xC2DA, 0x909506, 5, 8,  256, 128, 2048,  64, 0x00444332, 0},
	{"MX30LF4G18AC",    0xC2DC, 0x909556, 5, 8,  512, 128, 2048,  64, 0x00444332, 0},

	/* ESMT */
	{"F59L1G81A",       0x92F1, 0x8095FF, 4, 8,  128, 128, 2048,  64, 0x00844333, 0},
	{"F59L2G81A",       0xC8DA, 0x909544, 5, 8,  256, 128, 2048,  64, 0x00844333, 0},
	{"F59L4G81A",       0xC8DC, 0x909554, 5, 8,  512, 128, 2048,  64, 0x00844333, 0},

	/* Spansion */
	{"S34ML01G100TF",   0x01F1, 0x801D01, 4, 8,  128, 128, 2048,  64, 0x00844333, 0},
	{"S34ML02G200TF",   0x01DA, 0x909546, 5, 8,  256, 128, 2048, 112, 0x00844333, 0},
	{"S34ML04G200TF",   0x01DC, 0x909556, 5, 8,  512, 128, 2048, 112, 0x00844333, 0},

	/* Samsung */
	{"K9K8G8000",       0xECD3, 0x519558, 5, 8, 1024, 128, 2048,  64, 0x00044333, 0},

	/* Toshiba */
	{"TC58NVG3S0F",     0x98D3, 0x902676, 5, 8, 1024, 256, 4096, 224, 0x00C25332, 0},

	/* Micron */
	{"MT29F1G08ABAEA",  0x2CF1, 0x809504, 4, 8,  128, 128, 2048,  64, 0x00844333, 0},
	{"MT29F16G08ABABA", 0x2C48, 0x0026A9, 5, 8, 2048, 512, 4096, 224, 0x00844333, 0},

	{NULL,              0x0000, 0x000000, 0, 0,    0,   0,    0,   0,          0, 0},
};

static bool get_nand_device_info_table(u16 id, u32 ext_id)
{
	u32 index;

	for (index = 0; gen_FlashTable[index].id != 0; index++) {
		if (id == gen_FlashTable[index].id && ext_id == gen_FlashTable[index].ext_id) {
			memcpy(&nand_devinfo, &gen_FlashTable[index], sizeof(flashdev_info));
			printk("%s: NAND chip found in MTK table: %s\n", MTK_NAND_MODULE_TEXT, nand_devinfo.devicename);
			return true;
		}
	}

	printk(KERN_WARNING "%s: NAND chip with device ID %x is not found in MTK table!\n", MTK_NAND_MODULE_TEXT, id);
	return false;
}

static void get_nand_device_info_bootstrap(void)
{
	u32 chip_mode = (*((volatile u32 *)(RALINK_SYSCTL_BASE+0x010))) & 0x0F;

	printk("%s: try detect NAND chip from bootstrap mode (%02X)\n", MTK_NAND_MODULE_TEXT, chip_mode);

	nand_devinfo.devicename = "Bootstrap";
	nand_devinfo.iowidth = 8;
	nand_devinfo.addr_cycle = 5;
	nand_devinfo.timmingsetting = NFI_DEFAULT_ACCESS_TIMING;
	nand_devinfo.advancedmode = 0;

	switch (chip_mode) {
	case 10:
		nand_devinfo.pagesize = 2048;
		nand_devinfo.sparesize = 128;
		nand_devinfo.totalsize = 128;
		nand_devinfo.blocksize = 128;
		break;
	case 11:
		nand_devinfo.pagesize = 4096;
		nand_devinfo.sparesize = 128;
		nand_devinfo.totalsize = 1024;
		nand_devinfo.blocksize = 256;
		break;
	case 12:
		nand_devinfo.pagesize = 4096;
		nand_devinfo.sparesize = 224;
		nand_devinfo.totalsize = 2048;
		nand_devinfo.blocksize = 512;
		break;
	default:
		nand_devinfo.pagesize = 2048;
		nand_devinfo.sparesize = 64;
		nand_devinfo.totalsize = 128;
		nand_devinfo.blocksize = 128;
		break;
	}
}

static int mtk_nand_init_size(struct mtd_info *mtd, struct nand_chip *chip, u8 *id_data)
{
	u16 id;
	u32 id_ext;

	printk("%s: NAND ID [%02X %02X, %02X %02X %02X %02X]\n",
		MTK_NAND_MODULE_TEXT, id_data[0], id_data[1], id_data[2], id_data[3], id_data[4], id_data[5]);

	id = ((u16)id_data[0] << 8) | id_data[1];
	id_ext = ((u32)id_data[2] << 16) | ((u32)id_data[3] << 8) | ((u32)id_data[4]);

	if (!get_nand_device_info_table(id, id_ext))
		get_nand_device_info_bootstrap();

	mtd->writesize = nand_devinfo.pagesize;
	mtd->oobsize = nand_devinfo.sparesize;
	mtd->erasesize = ((u32)nand_devinfo.blocksize<<10);

	chip->chipsize = ((uint64_t)nand_devinfo.totalsize<<20);

	return (nand_devinfo.iowidth == 16) ? NAND_BUSWIDTH_16 : 0;
}

static void ECC_Config(u32 ecc_bit)
{
	u32 u4ENCODESize;
	u32 u4DECODESize;
	u32 ecc_bit_cfg = ECC_CNFG_ECC4;

	switch(ecc_bit) {
	case 4:
		ecc_bit_cfg = ECC_CNFG_ECC4;
		break;
	case 8:
		ecc_bit_cfg = ECC_CNFG_ECC8;
		break;
	case 10:
		ecc_bit_cfg = ECC_CNFG_ECC10;
		break;
	case 12:
		ecc_bit_cfg = ECC_CNFG_ECC12;
		break;
	default:
		break;
	}

	DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
	do{;
	}
	while (!DRV_Reg16(ECC_DECIDLE_REG16));

	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
	do {;
	}
	while (!DRV_Reg32(ECC_ENCIDLE_REG32));

	/* setup FDM register base */
	DRV_WriteReg32(ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

	/* Sector + FDM */
	u4ENCODESize = (NFI_NAND_SECTOR_SIZE + 8) << 3;

	/* Sector + FDM + YAFFS2 meta data bits */
	u4DECODESize = ((NFI_NAND_SECTOR_SIZE + 8) << 3) + ecc_bit * 13;

	/* configure ECC decoder && encoder */
	DRV_WriteReg32(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));

	DRV_WriteReg32(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));
#if ECC_MANUAL_CORRECT
	NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#else
	NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#endif
}

static void ECC_Decode_Start(void)
{
	/* wait for device returning idle */
	while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
	DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}

static void ECC_Decode_End(void)
{
	/* wait for device returning idle */
	while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
	DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

static void ECC_Encode_Start(void)
{
	/* wait for device returning idle */
	while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
	mb();
	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

static void ECC_Encode_End(void)
{
	/* wait for device returning idle */
	while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
	mb();
	DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

static bool mtk_nand_check_bch_error(struct mtd_info *mtd, u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
	bool bRet = true;
	u16 u2SectorDoneMask = 1 << u4SecIndex;
	u32 u4ErrorNumDebug, i, u4ErrNum;
	u32 timeout = 0xFFFF;
#if ECC_MANUAL_CORRECT
	u32 au4ErrBitLoc[6];
	u32 u4ErrByteLoc, u4BitOffset;
	u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#else
	u32 correct_count = 0;
#endif

	//4 // Wait for Decode Done
	while (0 == (u2SectorDoneMask & DRV_Reg16(ECC_DECDONE_REG16))) {
		timeout--;
		if (timeout == 0)
			return false;
	}

#if ECC_MANUAL_CORRECT
	/* We will manually correct the error bits in the last sector, not all the sectors of the page! */
	memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
	u4ErrorNumDebug = DRV_Reg32(ECC_DECENUM_REG32);
	u4ErrNum = DRV_Reg32(ECC_DECENUM_REG32) >> (u4SecIndex << 2);
	u4ErrNum &= 0xF;

	if (u4ErrNum) {
		if (0xF == u4ErrNum) {
			mtd->ecc_stats.failed++;
			bRet = false;
			printk(KERN_WARNING "%s: Uncorrectable at PageAddr=%d\n", MTK_NAND_MODULE_TEXT, u4PageAddr);
		} else {
			for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i) {
				au4ErrBitLoc[i] = DRV_Reg32(ECC_DECEL0_REG32 + i);
				u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;
				if (u4ErrBitLoc1th < 0x1000) {
					u4ErrByteLoc = u4ErrBitLoc1th / 8;
					u4BitOffset = u4ErrBitLoc1th % 8;
					pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
#if defined(SKIP_BAD_BLOCK)
					if (!is_skip_bad_block(mtd, u4PageAddr))
#endif
						mtd->ecc_stats.corrected++;
				} else {
#if defined(SKIP_BAD_BLOCK)
					if (!is_skip_bad_block(mtd, u4PageAddr))
#endif
						mtd->ecc_stats.corrected++;
				}
				u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
				if (u4ErrBitLoc2nd != 0) {
					if (u4ErrBitLoc2nd < 0x1000) {
						u4ErrByteLoc = u4ErrBitLoc2nd / 8;
						u4BitOffset = u4ErrBitLoc2nd % 8;
						pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
#if defined(SKIP_BAD_BLOCK)
						if (!is_skip_bad_block(mtd, u4PageAddr))
#endif
							mtd->ecc_stats.corrected++;
					} else {
#if defined(SKIP_BAD_BLOCK)
						if (!is_skip_bad_block(mtd, u4PageAddr))
#endif
							mtd->ecc_stats.corrected++;
					}
				}
			}
		}
		if (0 == (DRV_Reg16(ECC_DECFER_REG16) & (1 << u4SecIndex))) {
			bRet = false;
		}
	}
#else
	u4ErrorNumDebug = DRV_Reg32(ECC_DECENUM_REG32);
	if (0 != (u4ErrorNumDebug & 0xFFFF)) {
		for (i = 0; i <= u4SecIndex; ++i) {
			u4ErrNum = DRV_Reg32(ECC_DECENUM_REG32) >> (i << 2);
			u4ErrNum &= 0xF;
			correct_count += u4ErrNum;
			if (0xF == u4ErrNum) {
				mtd->ecc_stats.failed++;
				bRet = false;
				printk(KERN_WARNING "%s: Uncorrectable at PageAddr=%d, Sector=%d\n", MTK_NAND_MODULE_TEXT, u4PageAddr, i);
			} else {
				if (u4ErrNum) {
//					printk(KERN_WARNING "%s: BCH correct %x at PageAddr=%d, Sector=%d\n", MTK_NAND_MODULE_TEXT, u4ErrNum, u4PageAddr, i);
				}
			}
		}
		if ((correct_count > 2) && bRet) {
#if defined(SKIP_BAD_BLOCK)
			if (!is_skip_bad_block(mtd, u4PageAddr))
#endif
				mtd->ecc_stats.corrected++;
		} else {
//			printk(KERN_INFO "Less than 2 bit error, ignore\n");
		}
	}
#endif
	return bRet;
}

static bool mtk_nand_RFIFOValidSize(u16 u2Size)
{
	int timeout = 0xFFFF;

	while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) < u2Size) {
		timeout--;
		if (timeout == 0)
			return false;
	}
	return true;
}

static bool mtk_nand_WFIFOValidSize(u16 u2Size)
{
	int timeout = 0xFFFF;

	while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) > u2Size) {
		timeout--;
		if (timeout == 0)
			return false;
	}
	return true;
}

static bool mtk_nand_status_ready(u32 u4Status)
{
	int timeout = 0xFFFF;

	while ((DRV_Reg32(NFI_STA_REG32) & u4Status) != 0) {
		timeout--;
		if (timeout == 0) {
			printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__, "NFI_STA");
			return false;
		}
	}
	return true;
}

static bool mtk_nand_reset(void)
{
	// HW recommended reset flow
	int timeout = 0xFFFF;

	if (DRV_Reg16(NFI_MASTERSTA_REG16)) { // master is busy
		mb();
		DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);
		while (DRV_Reg16(NFI_MASTERSTA_REG16)) {
			timeout--;
			if (timeout == 0) {
				printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
					MTK_NAND_MODULE_TEXT, __FUNCTION__, "NFI_MASTERSTA");
			}
		}
	}

	/* issue reset operation */
	mb();
	DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

	return mtk_nand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && mtk_nand_RFIFOValidSize(0) && mtk_nand_WFIFOValidSize(0);
}

static void mtk_nand_set_mode(u16 u2OpMode)
{
	u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
	u2Mode &= ~CNFG_OP_MODE_MASK;
	u2Mode |= u2OpMode;
	DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

static void mtk_nand_set_autoformat(bool bEnable)
{
	if (bEnable) {
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
	} else {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
	}
}

static void mtk_nand_configure_fdm(u16 u2FDMSize)
{
	NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
	NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
	NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

static void mtk_nand_configure_lock(void)
{
	u32 u4WriteColNOB = 2;
	u32 u4WriteRowNOB = 3;
	u32 u4EraseColNOB = 0;
	u32 u4EraseRowNOB = 3;

	DRV_WriteReg16(NFI_LOCKANOB_REG16, (u4WriteColNOB << PROG_CADD_NOB_SHIFT) | (u4WriteRowNOB << PROG_RADD_NOB_SHIFT) | (u4EraseColNOB << ERASE_CADD_NOB_SHIFT) | (u4EraseRowNOB << ERASE_RADD_NOB_SHIFT));
}

static bool mtk_nand_pio_ready(void)
{
	int count = 0;

	while (!(DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)) {
		count++;
		if (count > 0xffff) {
			printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__, "PIO_DIRDY");
			return false;
		}
	}

	return true;
}

static bool mtk_nand_set_command(u16 command)
{
	/* Write command to device */
	mb();
	DRV_WriteReg16(NFI_CMD_REG16, command);
	return mtk_nand_status_ready(STA_CMD_STATE);
}

static bool mtk_nand_set_address(u32 u4ColAddr, u32 u4RowAddr, u16 u2ColNOB, u16 u2RowNOB)
{
	/* fill cycle addr */
	mb();
	DRV_WriteReg32(NFI_COLADDR_REG32, u4ColAddr);
	DRV_WriteReg32(NFI_ROWADDR_REG32, u4RowAddr);
	DRV_WriteReg16(NFI_ADDRNOB_REG16, u2ColNOB | (u2RowNOB << ADDR_ROW_NOB_SHIFT));
	return mtk_nand_status_ready(STA_ADDR_STATE);
}

static bool mtk_nand_check_RW_count(u16 u2WriteSize)
{
	u32 timeout = 0xFFFF;
	u16 u2SecNum = u2WriteSize >> 9;

	while (ADDRCNTR_CNTR(DRV_Reg16(NFI_ADDRCNTR_REG16)) < u2SecNum) {
		timeout--;
		if (0 == timeout) {
			printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__, "ADDRCNTR");
			return false;
		}
	}
	return true;
}

static bool mtk_nand_ready_for_read(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, bool full, u8 * buf)
{
	/* Reset NFI HW internal state machine and flush NFI in/out FIFO */
	bool bRet = false;
	u16 sec_num = 1 << (nand->page_shift - 9);
	u32 col_addr = u4ColAddr;
	u32 colnob = 2, rownob = nand_devinfo.addr_cycle - 2;

	if (nand->options & NAND_BUSWIDTH_16)
		col_addr /= 2;

	if (!mtk_nand_reset())
		goto cleanup;

#if ECC_ENABLE
	/* Enable HW ECC */
	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#else
	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#endif

	mtk_nand_set_mode(CNFG_OP_READ);
	NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
	DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);

	if (full) {
#if ECC_ENABLE
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#endif
	} else {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	}

	mtk_nand_set_autoformat(full);

#if ECC_ENABLE
	if (full) {
		ECC_Decode_Start();
	}
#endif

	if (!mtk_nand_set_command(NAND_CMD_READ0))
		goto cleanup;

	if (!mtk_nand_set_address(col_addr, u4RowAddr, colnob, rownob))
		goto cleanup;

	if (!mtk_nand_set_command(NAND_CMD_READSTART))
		goto cleanup;

	if (!mtk_nand_status_ready(STA_NAND_BUSY))
		goto cleanup;

	bRet = true;

cleanup:
	return bRet;
}

static bool mtk_nand_ready_for_write(struct nand_chip *nand, u32 u4RowAddr, u32 col_addr, bool full, u8 * buf)
{
	bool bRet = false;
	u32 sec_num = 1 << (nand->page_shift - 9);
	u32 colnob = 2, rownob = nand_devinfo.addr_cycle - 2;

	if (nand->options & NAND_BUSWIDTH_16)
		col_addr /= 2;

	/* Reset NFI HW internal state machine and flush NFI in/out FIFO */
	if (!mtk_nand_reset())
		goto cleanup;

	mtk_nand_set_mode(CNFG_OP_PRGM);

	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

	DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);

	if (full) {
#if ECC_ENABLE
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#endif
	} else {
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	}

	mtk_nand_set_autoformat(full);

#if ECC_ENABLE
	if (full) {
		ECC_Encode_Start();
	}
#endif

	if (!mtk_nand_set_command(NAND_CMD_SEQIN))
		goto cleanup;

	//1 FIXED ME: For Any Kind of AddrCycle
	if (!mtk_nand_set_address(col_addr, u4RowAddr, colnob, rownob))
		goto cleanup;

	if (!mtk_nand_status_ready(STA_NAND_BUSY))
		goto cleanup;

	bRet = true;

cleanup:
	return bRet;
}

static bool mtk_nand_check_dececc_done(u32 u4SecNum)
{
	int timeout = 0xffff;
	u32 dec_mask;

	dec_mask = (1 << u4SecNum) - 1;
	while ((dec_mask != DRV_Reg16(ECC_DECDONE_REG16)) && timeout > 0)
		timeout--;

	if (timeout == 0) {
		printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
			MTK_NAND_MODULE_TEXT, __FUNCTION__, "ECC_DECDONE");
		return false;
	}

	return true;
}

static bool mtk_nand_mcu_read_data(u8 *buf, u32 length)
{
	int timeout = 0xffff;
	u32 i;
	u32 *buf32 = (u32 *) buf;

	if ((u32) buf % 4 || length % 4)
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

	mb();
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

	if ((u32) buf % 4 || length % 4) {
		for (i = 0; (i < (length)) && (timeout > 0);) {
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				*buf++ = (u8) DRV_Reg32(NFI_DATAR_REG32);
				i++;
			} else {
				timeout--;
			}
			if (timeout == 0) {
				printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
					MTK_NAND_MODULE_TEXT, __FUNCTION__, "PIO_DIRDY");
				return false;
			}
		}
	} else {
		for (i = 0; (i < (length >> 2)) && (timeout > 0);) {
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				*buf32++ = DRV_Reg32(NFI_DATAR_REG32);
				i++;
			} else {
				timeout--;
			}
			if (timeout == 0) {
				printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
					MTK_NAND_MODULE_TEXT, __FUNCTION__, "PIO_DIRDY");
				return false;
			}
		}
	}

	return true;
}

static bool mtk_nand_mcu_write_data(const u8 * buf, u32 length)
{
	int timeout = 0xffff;
	u32 i;
	u32 *pBuf32;
	NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	mb();
	NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);
	pBuf32 = (u32 *) buf;

	if ((u32) buf % 4 || length % 4)
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

	if ((u32) buf % 4 || length % 4) {
		for (i = 0; (i < (length)) && (timeout > 0);) {
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				DRV_WriteReg32(NFI_DATAW_REG32, *buf++);
				i++;
			} else {
				timeout--;
			}
			if (timeout == 0) {
				printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
					MTK_NAND_MODULE_TEXT, __FUNCTION__, "PIO_DIRDY");
				return false;
			}
		}
	} else {
		for (i = 0; (i < (length >> 2)) && (timeout > 0);) {
			// if (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) <= 12)
			if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1) {
				DRV_WriteReg32(NFI_DATAW_REG32, *pBuf32++);
				i++;
			} else {
				timeout--;
			}
			if (timeout == 0) {
				printk(KERN_WARNING "%s: [%s] wait for %s timeout!\n",
					MTK_NAND_MODULE_TEXT, __FUNCTION__, "PIO_DIRDY");
				return false;
			}
		}
	}

	return true;
}

static void mtk_nand_read_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
	u32 i;
	u32 *pBuf32 = (u32 *) pDataBuf;

	if (pBuf32) {
		for (i = 0; i < u4SecNum; ++i) {
			*pBuf32++ = DRV_Reg32(NFI_FDM0L_REG32 + (i << 1));
			*pBuf32++ = DRV_Reg32(NFI_FDM0M_REG32 + (i << 1));
		}
	}
}

static u8 fdm_buf[64];
static void mtk_nand_write_fdm_data(struct nand_chip *chip, u8 * pDataBuf, u32 u4SecNum)
{
	u32 i, j;
	u8 checksum = 0;
	bool empty = true;
	struct nand_oobfree *free_entry;
	u32 *pBuf32;

	memcpy(fdm_buf, pDataBuf, u4SecNum * 8);

	free_entry = chip->ecc.layout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && free_entry[i].length; i++) {
		for (j = 0; j < free_entry[i].length; j++) {
			if (pDataBuf[free_entry[i].offset + j] != 0xFF)
				empty = false;
			checksum ^= pDataBuf[free_entry[i].offset + j];
		}
	}

	if (!empty)
		fdm_buf[free_entry[i - 1].offset + free_entry[i - 1].length] = checksum;

	pBuf32 = (u32 *) fdm_buf;
	for (i = 0; i < u4SecNum; ++i) {
		DRV_WriteReg32(NFI_FDM0L_REG32 + (i << 1), *pBuf32++);
		DRV_WriteReg32(NFI_FDM0M_REG32 + (i << 1), *pBuf32++);
	}
}

static void mtk_nand_stop_read(void)
{
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
	mtk_nand_reset();
#if ECC_ENABLE
	ECC_Decode_End();
#endif
	DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
}

static void mtk_nand_stop_write(void)
{
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
#if ECC_ENABLE
	ECC_Encode_End();
#endif
	DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
}

bool mtk_nand_exec_read_page(struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
	u8 *buf;
	bool bRet = true;
	struct nand_chip *nand = mtd->priv;
	u32 u4SecNum = u4PageSize >> 9;

	if (((u32) pPageBuf % 16) && local_buffer_16_align)
		buf = local_buffer_16_align;
	else
		buf = pPageBuf;

	if (mtk_nand_ready_for_read(nand, u4RowAddr, 0, true, buf)) {
#if (ECC_MANUAL_CORRECT && ECC_ENABLE)
		int j;
		for (j = 0; j < u4SecNum; j++) {
			if (!mtk_nand_mcu_read_data(buf+j*512, 512))
				bRet = false;
			if (!mtk_nand_check_dececc_done(j+1))
				bRet = false;
			if (!mtk_nand_check_bch_error(mtd, buf+j*512, j, u4RowAddr))
				bRet = false;
		}
		
		if (!mtk_nand_status_ready(STA_NAND_BUSY))
			bRet = false;
		mtk_nand_read_fdm_data(pFDMBuf, u4SecNum);
#else
		if (!mtk_nand_mcu_read_data(buf, u4PageSize))
			bRet = false;
		if (!mtk_nand_status_ready(STA_NAND_BUSY))
			bRet = false;
#if ECC_ENABLE
		if (!mtk_nand_check_dececc_done(u4SecNum))
			bRet = false;
#endif
		mtk_nand_read_fdm_data(pFDMBuf, u4SecNum);
#if ECC_ENABLE
		if (!mtk_nand_check_bch_error(mtd, buf, u4SecNum - 1, u4RowAddr))
			bRet = false;
#endif
#endif
		mtk_nand_stop_read();
	}

	if (buf == local_buffer_16_align)
		memcpy(pPageBuf, buf, u4PageSize);

	return bRet;
}

bool mtk_nand_exec_write_page(struct mtd_info *mtd, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
	struct nand_chip *chip = mtd->priv;
	u32 u4SecNum = u4PageSize >> 9;
	u8 *buf;
	u8 status;

	MSG(WRITE, "%s: [%s] page: 0x%x\n", MTK_NAND_MODULE_TEXT, __FUNCTION__, u4RowAddr);

	if (((u32) pPageBuf % 16) && local_buffer_16_align) {
		memcpy(local_buffer_16_align, pPageBuf, mtd->writesize);
		buf = local_buffer_16_align;
	} else
		buf = pPageBuf;

	if (mtk_nand_ready_for_write(chip, u4RowAddr, 0, true, buf)) {
		mtk_nand_write_fdm_data(chip, pFDMBuf, u4SecNum);
		mtk_nand_mcu_write_data(buf, u4PageSize);
		mtk_nand_check_RW_count(u4PageSize);
		mtk_nand_stop_write();
		mtk_nand_set_command(NAND_CMD_PAGEPROG);
		while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
	}

	status = chip->waitfunc(mtd, chip);
	if (status & NAND_STATUS_FAIL)
		return false;

	return true;
}

#if defined(SKIP_BAD_BLOCK)

static int get_start_end_block(struct mtd_info *mtd, int block, int *start_blk, int *end_blk)
{
	struct nand_chip *chip = mtd->priv;
	int i, end_blk_last, part_num = ARRAY_SIZE(rt2880_partitions);

	*start_blk = 0;
	end_blk_last = 0;

	for (i = 0; i < part_num; i++) {
		if (rt2880_partitions[i].offset == MTDPART_OFS_APPEND)
			*start_blk = end_blk_last;
		else
			*start_blk = (int)(rt2880_partitions[i].offset >> chip->phys_erase_shift);
		end_blk_last = *start_blk + (int)(rt2880_partitions[i].size >> chip->phys_erase_shift);
		
		if (end_blk_last > *start_blk)
			*end_blk = end_blk_last - 1;
		else
			*end_blk = end_blk_last;
		
		if ((block >= *start_blk) && (block <= *end_blk)) {
#if !defined (CONFIG_MTD_NAND_USE_UBI_PART)
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
			/* use merged partition */
			if (i == NAND_MTD_KERNEL_PART_IDX || i == NAND_MTD_ROOTFS_PART_IDX) {
				*start_blk = (NAND_MTD_KERNEL_PART_OFFSET >> chip->phys_erase_shift);
				*end_blk = *start_blk + (NAND_MTD_KERNEL_PART_SIZE >> chip->phys_erase_shift) - 1;
			}
#endif
#endif
			return 0;
		}
	}

	return -1;
}

static int block_remap(struct mtd_info *mtd, int block)
{
	struct nand_chip *chip = mtd->priv;
	int start_blk, end_blk;
	int j, block_offset;
	int bad_block = 0;

	if (!chip->bbt)
		return block;

	if (get_start_end_block(mtd, block, &start_blk, &end_blk) < 0)
		return block;

	block_offset = block - start_blk;
	for (j = start_blk; j <= end_blk; j++) {
		if (((chip->bbt[j >> 2] >> ((j<<1) & 0x6)) & 0x3) == 0x0) {
			if (!block_offset)
				break;
			block_offset--;
		} else {
			bad_block++;
		}
	}

	if (j <= end_blk)
		return j;

	// remap to the bad block
	for (j = end_blk; bad_block > 0; j--) {
		if (((chip->bbt[j >> 2] >> ((j<<1) & 0x6)) & 0x3) != 0x0) {
			bad_block--;
			if (bad_block <= block_offset)
				return j;
		}
	}

	return -1;
}

static int write_next_on_fail(struct mtd_info *mtd, char *write_buf, int page, int * to_blk)
{
	struct nand_chip *chip = mtd->priv;
	int i, j, to_page = 0, first_page;
	char *buf, *oob;
	int start_blk = 0, end_blk;
	int page_per_block_bit = chip->phys_erase_shift - chip->page_shift;
	int block = page >> page_per_block_bit;

	// find next available block in the same MTD partition 
	if (get_start_end_block(mtd, block, &start_blk, &end_blk) < 0)
		return -1;

	buf = kzalloc(mtd->writesize + mtd->oobsize, GFP_KERNEL | GFP_DMA);
	if (buf == NULL)
		return -1;

	oob = buf + mtd->writesize;
	for ((*to_blk) = block + 1; (*to_blk) <= end_blk ; (*to_blk)++) {
		if (nand_bbt_get(mtd, (*to_blk) << page_per_block_bit) == 0) {
			int status = mtk_nand_erase_hw(mtd, (*to_blk) << page_per_block_bit);
			if (status & NAND_STATUS_FAIL) {
				mtk_nand_block_markbad_hw(mtd, (*to_blk) << chip->phys_erase_shift);
				nand_bbt_set_bad(mtd, (*to_blk) << page_per_block_bit);
			} else {
				/* good block */
				to_page = (*to_blk) << page_per_block_bit;
				break;
			}
		}
	}

	if (!to_page) {
		kfree(buf);
		return -1;
	}

	first_page = (page >> page_per_block_bit) << page_per_block_bit;
	for (i = 0; i < (1 << page_per_block_bit); i++) {
		if ((first_page + i) != page) {
			mtk_nand_read_oob_hw(mtd, chip, (first_page+i));
			for (j = 0; j < mtd->oobsize; j++) {
				if (chip->oob_poi[j] != (unsigned char)0xff) {
					break;
				}
			}
			if (j < mtd->oobsize) {
				mtk_nand_exec_read_page(mtd, (first_page+i), mtd->writesize, buf, oob);
				memset(oob, 0xff, mtd->oobsize);
				if (!mtk_nand_exec_write_page(mtd, to_page + i, mtd->writesize, (u8 *)buf, oob)) {
					int ret, new_blk = 0;
					nand_bbt_set_bad(mtd, to_page);
					ret =  write_next_on_fail(mtd, buf, to_page + i, &new_blk);
					if (ret) {
						kfree(buf);
						mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
						return ret;
					}
					mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
					*to_blk = new_blk;
					to_page = ((*to_blk) <<  page_per_block_bit);
				}
			}
		} else {
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			if (!mtk_nand_exec_write_page(mtd, to_page + i, mtd->writesize, (u8 *)write_buf, chip->oob_poi)) {
				int ret, new_blk = 0;
				nand_bbt_set_bad(mtd, to_page);
				ret =  write_next_on_fail(mtd, write_buf, to_page + i, &new_blk);
				if (ret) {
					kfree(buf);
					mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
					return ret;
				}
				mtk_nand_block_markbad_hw(mtd, to_page << chip->page_shift);
				*to_blk = new_blk;
				to_page = ((*to_blk) <<  page_per_block_bit);
			}
		}
	}

	kfree(buf);

	return 0;
}

static int is_skip_bad_block(struct mtd_info *mtd, int page)
{
#if defined (CONFIG_MTD_UBI) || defined (CONFIG_MTD_UBI_MODULE)
	struct nand_chip *chip = mtd->priv;

	if ((page << chip->page_shift) >= UBI_PART_START_OFFSET)
		return 0;
#endif
	return 1;
}

int check_block_remap(struct mtd_info *mtd, int block)
{
	if (shift_on_bbt) {
#if defined (CONFIG_MTD_UBI) || defined (CONFIG_MTD_UBI_MODULE)
		struct nand_chip *chip = mtd->priv;

		if ((block << chip->phys_erase_shift) >= UBI_PART_START_OFFSET)
			return block;
#endif
		return block_remap(mtd, block);
	}

	return block;
}

#else
int check_block_remap(struct mtd_info *mtd, int block)
{
	return block;
}
#endif
EXPORT_SYMBOL(check_block_remap);


#if defined(UBIFS_ECC_0_PATCH)
static int check_ecc_0(struct mtd_info *mtd, int page)
{
	int i;
	struct nand_chip *chip = mtd->priv;
	u8 local_oob[NAND_MAX_OOBSIZE];

	// for 4 bits ecc protection, the all 0xff is 26 20 98 1b 87 6e fc
	if (chip->ecc.layout->eccbytes == 32) {
		if (mtk_nand_read_oob_raw(mtd, local_oob, page, mtd->oobsize) == 0) {
			for (i = 0; i < 64; i++) {
				switch (i & 0xf)
				{
				case 8:
					if (local_oob[i] != 0x26)
						return 0;
					else
						break;
				case 9:
					if (local_oob[i] != 0x20)
						return 0;
					else
						break;
				case 10:
					if (local_oob[i] != 0x98)
						return 0;
					else
						break;
				case 11:
					if (local_oob[i] != 0x1b)
						return 0;
					else
						break;
				case 12:
					if (local_oob[i] != 0x87)
						return 0;
					else
						break;
				case 13:
					if (local_oob[i] != 0x6e)
						return 0;
					else
						break;
				case 14:
					if (local_oob[i] != 0xfc)
						return 0;
					else
						break;
				default:
					break;
				}
			}
		}
	} else {
//		printk("Not support FIX_ECC_0 now\n");
		return 0;
	}

	MSG(VERIFY, "%s: [%s] clean page with ECC at 0x%x\n",
		MTK_NAND_MODULE_TEXT, __FUNCTION__, page);
	return 1;
}

static void fix_ecc_0(struct mtd_info *mtd, int page)
{
	struct nand_chip *chip = mtd->priv;
	u8 *block_buf;
	u8 oob_buf[NAND_MAX_OOBSIZE];
	int i, j, offset, page_is_empty, status;
	int page_per_block_shift = chip->phys_erase_shift - chip->page_shift;
	int page_per_block = 1 << page_per_block_shift;
	int start_page = (page >> page_per_block_shift) << page_per_block_shift;

	block_buf = (unsigned char *) kzalloc((mtd->writesize + mtd->oobsize) * page_per_block, GFP_KERNEL | GFP_DMA);
	if (!block_buf)
		return;

	memset(block_buf, 0xff, (mtd->writesize + mtd->oobsize) * page_per_block);

	// read all pages in the block
	for(i=0; i < page_per_block; i++) {
		offset = i * (mtd->writesize + mtd->oobsize);
		if (!mtk_nand_exec_read_page(mtd, start_page+i, mtd->writesize, block_buf + offset, oob_buf)) {
			printk(KERN_WARNING "%s: [%s] page 0x%x read error!\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__, start_page+i);
			kfree(block_buf);
			return;
		}
	}

	// erase the block
	status = mtk_nand_erase_hw(mtd, start_page);
	if (status & NAND_STATUS_FAIL) {
		mtk_nand_block_markbad_hw(mtd, start_page << chip->page_shift);
		nand_bbt_set_bad(mtd, start_page);
		kfree(block_buf);
		return;
	}

	// program, skip all 0xff pages
	for(i=0; i < page_per_block; i++) {
		page_is_empty = 1;
		offset = i * (mtd->writesize + mtd->oobsize);
		for (j = 0; j < mtd->writesize; j++) {
			if (*(block_buf + j + offset) != (unsigned char)0xff) {
				page_is_empty = 0;
				break;
			}
		}
		if (!page_is_empty) {
			// write page
			if (!mtk_nand_exec_write_page(mtd, start_page + i, mtd->writesize, block_buf + offset, block_buf + offset + mtd->writesize)) {
				printk(KERN_WARNING "%s: [%s] page 0x%x write error!\n",
					MTK_NAND_MODULE_TEXT, __FUNCTION__, start_page+i);
				kfree(block_buf);
				return;
			}
		}
	}

	kfree(block_buf);
}
#endif

/******************************************************************************
 *
 * Write a page to a logical address
 *
 *****************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
static int mtk_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip, uint32_t offset, int data_len, const u8 * buf, int oob_required, int page, int cached, int raw)
#else
static int mtk_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip, const u8 * buf, int page, int cached, int raw)
#endif
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, page)) {
		// bmt code
	} else {
		if (shift_on_bbt) {
			mapped_block = block_remap(mtd, block);
			if (mapped_block < 0)
				return -EIO;
			if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
				return -EIO;
			goto do_write;
		}
	}
#endif

#if defined(UBIFS_ECC_0_PATCH)
	if (check_ecc_0(mtd, page_in_block + mapped_block * page_per_block)) {
		fix_ecc_0(mtd, page_in_block + mapped_block * page_per_block);
	}
#endif

do_write:

	do {
		if (!mtk_nand_exec_write_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, (u8 *)buf, chip->oob_poi)) {
			printk(KERN_WARNING "%s: [%s] write fail at block: 0x%x, page: 0x%x\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__, mapped_block, page_in_block);
#if defined(SKIP_BAD_BLOCK)
			if (!is_skip_bad_block(mtd, page)) {
				return -EIO;
			} else {
				int new_blk = 0;
				nand_bbt_set_bad(mtd, page_in_block + mapped_block * page_per_block);
				if (write_next_on_fail(mtd, (char *)buf, page_in_block + mapped_block * page_per_block, &new_blk) != 0) {
					mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
					return -EIO;
				}
				mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
				break;
			}
#else
			return -EIO;
#endif
		}
		else
			break;
	} while(1);

	return 0;
}

static void mtk_nand_command_bp(struct mtd_info *mtd, unsigned int command, int column, int page_addr)
{
	struct nand_chip *nand = mtd->priv;

	switch (command) {
	case NAND_CMD_SEQIN:
		memset(g_kCMD.au1OOB, 0xFF, sizeof(g_kCMD.au1OOB));
		g_kCMD.pDataBuf = NULL;
		g_kCMD.u4RowAddr = page_addr;
		g_kCMD.u4ColAddr = column;
		break;

	 case NAND_CMD_PAGEPROG:
		if (g_kCMD.pDataBuf || (0xFF != g_kCMD.au1OOB[OOB_BAD_BLOCK_OFFSET])) {
			u8 *pDataBuf = g_kCMD.pDataBuf ? g_kCMD.pDataBuf : nand->buffers->databuf;
			mtk_nand_exec_write_page(mtd, g_kCMD.u4RowAddr, mtd->writesize, pDataBuf, g_kCMD.au1OOB);
			g_kCMD.u4RowAddr = (u32) - 1;
			g_kCMD.u4OOBRowAddr = (u32) - 1;
		}
		break;

	case NAND_CMD_READOOB:
		g_kCMD.u4RowAddr = page_addr;
		g_kCMD.u4ColAddr = column + mtd->writesize;
		break;

	case NAND_CMD_READ0:
		g_kCMD.u4RowAddr = page_addr;
		g_kCMD.u4ColAddr = column;
		break;

	case NAND_CMD_ERASE1:
		nand->state = FL_ERASING;
		mtk_nand_reset();
		mtk_nand_set_mode(CNFG_OP_ERASE);
		mtk_nand_set_command(NAND_CMD_ERASE1);
		mtk_nand_set_address(0, page_addr, 0, nand_devinfo.addr_cycle - 2);
		break;

	case NAND_CMD_ERASE2:
		mtk_nand_set_command(NAND_CMD_ERASE2);
		while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
		break;

	case NAND_CMD_STATUS:
		mtk_nand_reset();
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
		mtk_nand_set_mode(CNFG_OP_SRD);
		mtk_nand_set_mode(CNFG_READ_EN);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		mtk_nand_set_command(NAND_CMD_STATUS);
		NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_NOB_MASK);
		mb();
		DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NFI_NOB_SHIFT));
		g_bcmdstatus = true;
		break;

	case NAND_CMD_RESET:
		mtk_nand_reset();
		DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_RST_DONE_EN);
		mtk_nand_set_command(NAND_CMD_RESET);
		DRV_WriteReg16(NFI_BASE+0x44, 0xF1);
		while(!(DRV_Reg16(NFI_INTR_REG16)&INTR_RST_DONE_EN));
		break;

	case NAND_CMD_READID:
		/* Issue NAND chip reset command */
		mtk_nand_reset();
		/* Disable HW ECC */
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
		/* Disable 16-bit I/O */
		//NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN | CNFG_BYTE_RW);
		mtk_nand_reset();
		mb();
		mtk_nand_set_mode(CNFG_OP_SRD);
		mtk_nand_set_command(NAND_CMD_READID);
		mtk_nand_set_address(0, 0, 1, 0);
		DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD);
		while (DRV_Reg32(NFI_STA_REG32) & STA_DATAR_STATE) ;
		break;

	default:
		BUG();
		break;
	}
}

static void mtk_nand_select_chip(struct mtd_info *mtd, int chip)
{
	switch (chip)
	{
	case 0:
	case 1:
		/*  Jun Shen, 2011.04.13  */
		/* Note: MT6577 EVB NAND  is mounted on CS0, but FPGA is CS1  */
		DRV_WriteReg16(NFI_CSEL_REG16, chip);
		break;
	}
}

static uint8_t mtk_nand_read_byte(struct mtd_info *mtd)
{
	uint8_t retval = 0;

	if (!mtk_nand_pio_ready()) {
		retval = false;
	}

	if (g_bcmdstatus) {
		retval = DRV_Reg8(NFI_DATAR_REG32);
		NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_NOB_MASK);
		mtk_nand_reset();
#if ECC_ENABLE
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#else
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
#endif
		g_bcmdstatus = false;
	} else
		retval = DRV_Reg8(NFI_DATAR_REG32);

	return retval;
}

static void mtk_nand_read_buf(struct mtd_info *mtd, uint8_t * buf, int len)
{
	struct nand_chip *nand = (struct nand_chip *)mtd->priv;
	struct NAND_CMD *pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;

	if (u4ColAddr < u4PageSize) {
		if ((u4ColAddr == 0) && (len >= u4PageSize)) {
			mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, buf, pkCMD->au1OOB);
			if (len > u4PageSize) {
				u32 u4Size = min(len - u4PageSize, sizeof(pkCMD->au1OOB));
				memcpy(buf + u4PageSize, pkCMD->au1OOB, u4Size);
			}
		} else {
			mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
			memcpy(buf, nand->buffers->databuf + u4ColAddr, len);
		}
		pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
	} else {
		u32 u4Offset = u4ColAddr - u4PageSize;
		u32 u4Size = min(len - u4Offset, sizeof(pkCMD->au1OOB));
		if (pkCMD->u4OOBRowAddr != pkCMD->u4RowAddr) {
			mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, nand->buffers->databuf, pkCMD->au1OOB);
			pkCMD->u4OOBRowAddr = pkCMD->u4RowAddr;
		}
		memcpy(buf, pkCMD->au1OOB + u4Offset, u4Size);
	}
	pkCMD->u4ColAddr += len;
}

static void mtk_nand_write_buf(struct mtd_info *mtd, const uint8_t * buf, int len)
{
	struct NAND_CMD *pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;
	int i4Size, i;

	if (u4ColAddr >= u4PageSize) {
		u32 u4Offset = u4ColAddr - u4PageSize;
		u8 *pOOB = pkCMD->au1OOB + u4Offset;
		i4Size = min(len, (int)(sizeof(pkCMD->au1OOB) - u4Offset));
		for (i = 0; i < i4Size; i++) {
			pOOB[i] &= buf[i];
		}
	} else {
		pkCMD->pDataBuf = (u8 *) buf;
	}

	pkCMD->u4ColAddr += len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
static int mtk_nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t * buf, int oob_required)
#else
static void mtk_nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t * buf)
#endif
{
	mtk_nand_write_buf(mtd, buf, mtd->writesize);
	mtk_nand_write_buf(mtd, chip->oob_poi, mtd->oobsize);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
	return 0;
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
static int mtk_nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, uint8_t * buf, int oob_required, int page)
#else
static int mtk_nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, uint8_t * buf, int page)
#endif
{
	struct NAND_CMD *pkCMD = &g_kCMD;
	u32 u4ColAddr = pkCMD->u4ColAddr;
	u32 u4PageSize = mtd->writesize;

	if (u4ColAddr == 0) {
		mtk_nand_exec_read_page(mtd, pkCMD->u4RowAddr, u4PageSize, buf, chip->oob_poi);
		pkCMD->u4ColAddr += u4PageSize + mtd->oobsize;
	}

	return 0;
}

/******************************************************************************
 *
 * Read a page to a logical address
 *
 *****************************************************************************/
static int mtk_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip, u8 * buf, int page)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, page)) {
		if (mtk_nand_exec_read_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, buf, chip->oob_poi))
			return 0;
		else
			return -EIO;
	} else {
		if (shift_on_bbt) {
			mapped_block = block_remap(mtd, block);
			if (mapped_block < 0)
				return -EIO;
			if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
				return -EIO;
		}
		if (mtk_nand_exec_read_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, buf, chip->oob_poi))
			return 0;
		else
			return -EIO;
	}
#else
	if (mtk_nand_exec_read_page(mtd, page_in_block + mapped_block * page_per_block, mtd->writesize, buf, chip->oob_poi))
		return 0;
	else
		return -EIO;
#endif
	return 0;
}

/******************************************************************************
 *
 * Erase a block at a logical address
 *
 *****************************************************************************/
int mtk_nand_erase_hw(struct mtd_info *mtd, int page)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;

	chip->erase_cmd(mtd, page);

	return chip->waitfunc(mtd, chip);
}

static int mtk_nand_erase(struct mtd_info *mtd, int page)
{
	// get mapping 
	struct nand_chip *chip = mtd->priv;
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int page_in_block = page % page_per_block;
	int block = page / page_per_block;
	int mapped_block = block;

#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, page)) {
		// bmt code
	} else {
		if (shift_on_bbt) {
			mapped_block = block_remap(mtd, block);
			if (mapped_block < 0)
				return -EIO;
			if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
				return -EIO;
		}
	}
#endif

	do {
		int status = mtk_nand_erase_hw(mtd, page_in_block + page_per_block * mapped_block);
		if (status & NAND_STATUS_FAIL) {
#if defined(SKIP_BAD_BLOCK)
			if (!is_skip_bad_block(mtd, page)) {
				mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
				nand_bbt_set_bad(mtd, page_in_block + mapped_block * page_per_block);
				return -EIO;
			} else {
				mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
				nand_bbt_set_bad(mtd, page_in_block + mapped_block * page_per_block);
				if (shift_on_bbt) {
					mapped_block = block_remap(mtd, block);
					if (mapped_block < 0)
						return -EIO;
					if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
						return -EIO;
				} else
					return -EIO;
			}
#else
			return -EIO;
#endif
		}
		else
			break;
	} while(1);

	return 0;
}

static int mtk_nand_read_oob_raw(struct mtd_info *mtd, uint8_t * buf, int page_addr, int len)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	u32 col_addr = 0;
	u32 sector = 0;
	int res = 0;
	u32 colnob = 2, rawnob = nand_devinfo.addr_cycle - 2;
	int randomread = 0;
	int read_len = 0;
	int sec_num = 1<<(chip->page_shift-9);
	int spare_per_sector = mtd->oobsize/sec_num;

	if (len > NAND_MAX_OOBSIZE || (len % OOB_AVAIL_PER_SECTOR) || !buf) {
		printk(KERN_WARNING "%s: [%s] invalid parameter, len: %d\n",
			MTK_NAND_MODULE_TEXT, __FUNCTION__, len);
		return -EINVAL;
	}

	if (len > spare_per_sector) {
		randomread = 1;
	}

	if (!randomread || !(nand_devinfo.advancedmode & ADV_MODE_RANDOM_READ)) {
		while (len > 0) {
			read_len = min(len, spare_per_sector);
			col_addr = NFI_NAND_SECTOR_SIZE + sector * (NFI_NAND_SECTOR_SIZE + spare_per_sector); // TODO: Fix this hard-code 16
			if (!mtk_nand_ready_for_read(chip, page_addr, col_addr, false, NULL)) {
//				printk(KERN_WARNING "mtk_nand_ready_for_read return failed\n");
				res = -EIO;
				goto error;
			}
			if (!mtk_nand_mcu_read_data(buf + spare_per_sector * sector, read_len)) {   // TODO: and this 8
				printk(KERN_WARNING "%s: [%s] mcu_read failed!\n",
					MTK_NAND_MODULE_TEXT, __FUNCTION__);
				res = -EIO;
				goto error;
			}
			mtk_nand_check_RW_count(read_len);
			mtk_nand_stop_read();
			sector++;
			len -= read_len;
		}
	} else {                      //should be 64
		col_addr = NFI_NAND_SECTOR_SIZE;
		if (chip->options & NAND_BUSWIDTH_16) {
			col_addr /= 2;
		}
		if (!mtk_nand_reset()) {
			goto error;
		}
		mtk_nand_set_mode(0x6000);
		NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
		DRV_WriteReg16(NFI_CON_REG16, 4 << CON_NFI_SEC_SHIFT);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
		NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		mtk_nand_set_autoformat(false);
		
		if (!mtk_nand_set_command(NAND_CMD_READ0)) {
			goto error;
		}
		
		//1 FIXED ME: For Any Kind of AddrCycle
		if (!mtk_nand_set_address(col_addr, page_addr, colnob, rawnob)) {
			goto error;
		}
		
		if (!mtk_nand_set_command(NAND_CMD_READSTART)) {
			goto error;
		}
		
		if (!mtk_nand_status_ready(STA_NAND_BUSY)) {
			goto error;
		}
		
		read_len = min(len, spare_per_sector);
		if (!mtk_nand_mcu_read_data(buf + spare_per_sector * sector, read_len)) {    // TODO: and this 8
//			printk(KERN_WARNING "mtk_nand_mcu_read_data return failed first 16\n");
			res = -EIO;
			goto error;
		}
		sector++;
		len -= read_len;
		mtk_nand_stop_read();
		
		while (len > 0) {
			read_len = min(len,  spare_per_sector);
			if (!mtk_nand_set_command(0x05)) {
				goto error;
			}
			col_addr = NFI_NAND_SECTOR_SIZE + sector * (NFI_NAND_SECTOR_SIZE + spare_per_sector);
			if (chip->options & NAND_BUSWIDTH_16) {
				col_addr /= 2;
			}
			DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);
			DRV_WriteReg16(NFI_ADDRNOB_REG16, 2);
			DRV_WriteReg16(NFI_CON_REG16, 4 << CON_NFI_SEC_SHIFT);
			if (!mtk_nand_status_ready(STA_ADDR_STATE)) {
				goto error;
			}
			if (!mtk_nand_set_command(0xE0)) {
				goto error;
			}
			if (!mtk_nand_status_ready(STA_NAND_BUSY)) {
				goto error;
			}
			if (!mtk_nand_mcu_read_data(buf + spare_per_sector * sector, read_len)) {    // TODO: and this 8
//				printk(KERN_WARNING "mtk_nand_mcu_read_data return failed first 16\n");
				res = -EIO;
				goto error;
			}
			mtk_nand_stop_read();
			sector++;
			len -= read_len;
		}
	}

error:
	NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
	return res;
}

static int mtk_nand_write_oob_raw(struct mtd_info *mtd, const uint8_t * buf, int page_addr, int len)
{
	struct nand_chip *chip = mtd->priv;
	u32 col_addr = 0;
	u32 sector = 0;
	int write_len = 0;
	int status;
	int sec_num = 1<<(chip->page_shift-9);
	int spare_per_sector = mtd->oobsize/sec_num;

	if (len > NAND_MAX_OOBSIZE || len % OOB_AVAIL_PER_SECTOR || !buf) {
		printk(KERN_WARNING "%s: [%s] invalid parameter, len: %d\n",
			MTK_NAND_MODULE_TEXT, __FUNCTION__, len);
		return -EINVAL;
	}

	while (len > 0) {
		write_len = min(len,  spare_per_sector);
		col_addr = sector * (NFI_NAND_SECTOR_SIZE +  spare_per_sector) + NFI_NAND_SECTOR_SIZE;
		if (!mtk_nand_ready_for_write(chip, page_addr, col_addr, false, NULL)) {
			return -EIO;
		}
		
		if (!mtk_nand_mcu_write_data(buf + sector * spare_per_sector, write_len)) {
			printk(KERN_WARNING "%s: [%s] mcu_write failed!\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__);
			return -EIO;
		}
		
		mtk_nand_check_RW_count(write_len);
		NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
		mtk_nand_set_command(NAND_CMD_PAGEPROG);
		
		while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
		
		status = chip->waitfunc(mtd, chip);
		if (status & NAND_STATUS_FAIL) {
			printk(KERN_WARNING "%s: [%s] failed, status: %d\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__, status);
			return -EIO;
		}
		
		len -= write_len;
		sector++;
	}

	return 0;
}

static int mtk_nand_write_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int i, iter;
	int sec_num = 1<<(chip->page_shift-9);
	int spare_per_sector = mtd->oobsize/sec_num;

	memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

	// copy ecc data
	for (i = 0; i < chip->ecc.layout->eccbytes; i++) {
		iter = (i / (spare_per_sector-OOB_AVAIL_PER_SECTOR)) *  spare_per_sector + OOB_AVAIL_PER_SECTOR + i % (spare_per_sector-OOB_AVAIL_PER_SECTOR);
		local_oob_buf[iter] = chip->oob_poi[chip->ecc.layout->eccpos[i]];
	}

	// copy FDM data
	for (i = 0; i < sec_num; i++) {
		memcpy(&local_oob_buf[i * spare_per_sector], &chip->oob_poi[i * OOB_AVAIL_PER_SECTOR], OOB_AVAIL_PER_SECTOR);
	}

	return mtk_nand_write_oob_raw(mtd, local_oob_buf, page, mtd->oobsize);
}

static int mtk_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, page)) {
		// bmt code
	} else {
		if (shift_on_bbt) {
			mapped_block = block_remap(mtd, block);
			if (mapped_block < 0)
				return -EIO;
			if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
				return -EIO;
		}
	}
#endif

	do {
		if (mtk_nand_write_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block /* page */)) {
			printk(KERN_WARNING "%s: [%s] write oob fail at block: 0x%x, page: 0x%x\n",
				MTK_NAND_MODULE_TEXT, __FUNCTION__, mapped_block, page_in_block);
#if defined(SKIP_BAD_BLOCK)
			if (!is_skip_bad_block(mtd, page)) {
				mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
				nand_bbt_set_bad(mtd, page_in_block + mapped_block * page_per_block);
				return -EIO;
			} else {
				mtk_nand_block_markbad_hw(mtd, (page_in_block + mapped_block * page_per_block) << chip->page_shift);
				nand_bbt_set_bad(mtd, page_in_block + mapped_block * page_per_block);
				if (shift_on_bbt) {
					mapped_block = block_remap(mtd, mapped_block);
					if (mapped_block < 0)
						return -EIO;
					if (nand_bbt_get(mtd, mapped_block << (chip->phys_erase_shift - chip->page_shift)) != 0x0)
						return -EIO;
				} else
					return -EIO;
			}
#else
			return -EIO;
#endif
		} else
			break;
	} while (1);

	return 0;
}

int mtk_nand_block_markbad_hw(struct mtd_info *mtd, loff_t offset)
{
	struct nand_chip *chip = mtd->priv;
	int block = (int)offset >> chip->phys_erase_shift;
	int page = block * (1 << (chip->phys_erase_shift - chip->page_shift));
	int ret;
	u8 buf[8];

	memset(buf, 0xFF, sizeof(buf));
	buf[OOB_BAD_BLOCK_OFFSET] = 0;

	ret = mtk_nand_write_oob_raw(mtd, buf, page, 8);
	return ret;
}

static int mtk_nand_block_markbad(struct mtd_info *mtd, loff_t offset)
{
	struct nand_chip *chip = mtd->priv;
	int block = (int)offset >> chip->phys_erase_shift;
	int ret;
	int mapped_block = block;

	nand_get_device(mtd, FL_WRITING);

#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, offset >> chip->page_shift)) {
		// bmt code
	} else {
		if (shift_on_bbt) {
			mapped_block = block_remap(mtd, block);
			if (mapped_block < 0) {
				nand_release_device(mtd);
				return 1;
			}
		}
	}
#endif
	ret = mtk_nand_block_markbad_hw(mtd, mapped_block << chip->phys_erase_shift);
	nand_release_device(mtd);

	return ret;
}

static int mtk_nand_read_oob_hw(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	int i;
	u8 iter = 0;

	int sec_num = 1 << (chip->page_shift - 9);
	int spare_per_sector = mtd->oobsize/sec_num;

	if (mtk_nand_read_oob_raw(mtd, chip->oob_poi, page, mtd->oobsize)) {
		return -EIO;
	}

	// adjust to ecc physical layout to memory layout
	/*********************************************************/
	/* FDM0 | ECC0 | FDM1 | ECC1 | FDM2 | ECC2 | FDM3 | ECC3 */
	/*  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  */
	/*********************************************************/

	memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

	// copy ECC data
	for (i = 0; i < chip->ecc.layout->eccbytes; i++) {
		iter = (i / (spare_per_sector-OOB_AVAIL_PER_SECTOR)) *  spare_per_sector + OOB_AVAIL_PER_SECTOR + i % (spare_per_sector-OOB_AVAIL_PER_SECTOR);
		chip->oob_poi[chip->ecc.layout->eccpos[i]] = local_oob_buf[iter];
	}

	// copy FDM data
	for (i = 0; i < sec_num; i++) {
		memcpy(&chip->oob_poi[i * OOB_AVAIL_PER_SECTOR], &local_oob_buf[i *  spare_per_sector], OOB_AVAIL_PER_SECTOR);
	}

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
static int mtk_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
#else
static int mtk_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page, int sndcmd)
#endif
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	u16 page_in_block = page % page_per_block;
	int mapped_block = block;

#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, page)) {
		// bmt code
	} else {
		if (shift_on_bbt) {
			mapped_block = block_remap(mtd, block);
			if (mapped_block < 0)
				return -EIO;
			// allow to read oob even if the block is bad
		}
	}
#endif
	if (mtk_nand_read_oob_hw(mtd, chip, page_in_block + mapped_block * page_per_block)!=0)
		return -EIO;

	return 0;                   // the return value is sndcmd
}

int mtk_nand_block_bad_hw(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	int page_addr = (int)(ofs >> chip->page_shift);
	unsigned int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	unsigned char oob_buf[8];

	page_addr &= ~(page_per_block - 1);

	if (mtk_nand_read_oob_raw(mtd, oob_buf, page_addr, sizeof(oob_buf))) {
		return 1;
	}

	if (oob_buf[OOB_BAD_BLOCK_OFFSET] != 0xFF) {
		printk(KERN_WARNING "%s: bad block detected at page 0x%x, oob_buf[0] is 0x%x\n",
			MTK_NAND_MODULE_TEXT, page_addr, oob_buf[0]);
		return 1;
	}

	/* everything is OK, good block */
	return 0;
}

static int mtk_nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int chipnr = 0;

	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	int block = (int)ofs >> chip->phys_erase_shift;
	int mapped_block = block;
	int ret;

	if (getchip) {
		chipnr = (int)(ofs >> chip->chip_shift);
		nand_get_device(mtd, FL_READING);
		
		/* Select the NAND device */
		chip->select_chip(mtd, chipnr);
	}

#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, ofs >> chip->page_shift)) {
		// bmt code
	} else {
		if (shift_on_bbt) {
			mapped_block = block_remap(mtd, block);
			if (mapped_block < 0) {
				if (getchip)
					nand_release_device(mtd);
				return 1;
			}
		}
	}
#endif

	ret = mtk_nand_block_bad_hw(mtd, mapped_block << chip->phys_erase_shift);
#if defined(SKIP_BAD_BLOCK)
	if (!is_skip_bad_block(mtd, ofs >> chip->page_shift)) {
		// bmt code
	}
#endif

	if (getchip) {
		chip->select_chip(mtd, -1);
		nand_release_device(mtd);
	}

	return ret;
}

static void mtk_nand_init_hw(void)
{
	u32 data;

	/* GPIO mode set pins as NAND */
	data = DRV_Reg32(RALINK_SYSCTL_BASE+0x60);
	data &= ~((0x3<<18)|(0x3<<16));
	data |=  ((0x2<<18)|(0x2<<16));
	DRV_WriteReg32(RALINK_SYSCTL_BASE+0x60, data);

	g_kCMD.u4OOBRowAddr = (u32) - 1;

	/* Set default NFI access timing control */
	DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);
	DRV_WriteReg16(NFI_CNFG_REG16, 0);
	DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

	/* Reset the state machine and data FIFO, because flushing FIFO */
	mtk_nand_reset();

	/* Set the ECC engine */
#if ECC_ENABLE
	NFI_SET_REG32(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	ECC_Config(4);
	mtk_nand_configure_fdm(8);
	mtk_nand_configure_lock();
#endif

	NFI_SET_REG16(NFI_IOCON_REG16, 0x47);

	/* Clear interrupt */
	DRV_Reg16(NFI_INTR_REG16);
}

static void mtk_nand_update_hw(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	u32 ecc_bit = 4;
	u32 spare_bit = PAGEFMT_SPARE_16;
	u32 spare_per_sector;

	spare_per_sector = mtd->oobsize / (mtd->writesize / NFI_NAND_SECTOR_SIZE);

	if (spare_per_sector >= 28) {
		spare_bit = PAGEFMT_SPARE_28;
		ecc_bit = 12;
		spare_per_sector = 28;
	} else if (spare_per_sector >= 27) {
		spare_bit = PAGEFMT_SPARE_27;
		ecc_bit = 8;
		spare_per_sector = 27;
	} else if (spare_per_sector >= 26) {
		spare_bit = PAGEFMT_SPARE_26;
		ecc_bit = 8;
		spare_per_sector = 26;
	} else if (spare_per_sector >= 16) {
		spare_bit = PAGEFMT_SPARE_16;
		ecc_bit = 4;
		spare_per_sector = 16;
	} else {
		printk(KERN_WARNING "%s: not support oobsize: %d\n", MTK_NAND_MODULE_TEXT, spare_per_sector);
	}

	mtd->oobsize = spare_per_sector * (mtd->writesize / NFI_NAND_SECTOR_SIZE);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
	chip->ecc.strength = ecc_bit;
#endif

	MSG(INIT, "%s: ecc bit: %d, oobsize: %d, spare_per_sector: %d\n", MTK_NAND_MODULE_TEXT, ecc_bit, mtd->oobsize, spare_per_sector);

	/* Set NFI access timing control */
	DRV_WriteReg32(NFI_ACCCON_REG32, nand_devinfo.timmingsetting);

	/* 16-bit bus width */
	if (nand_devinfo.iowidth == 16) {
		MSG(INIT, "%s: set the 16-bit I/O settings!\n", MTK_NAND_MODULE_TEXT);
		chip->options |= NAND_BUSWIDTH_16;
		NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
	}

	/* Setup PageFormat */
	if (mtd->writesize == 4096) {
		NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
	} else if (mtd->writesize == 2048) {
		NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
	}

	/* Setup the ECC engine */
	ECC_Config(ecc_bit);
}

//-------------------------------------------------------------------------------

static int mtk_nand_dev_ready(struct mtd_info *mtd)
{
	return !(DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY);
}

#if defined (LOAD_FACT_BBT)

#define OOB_FACT_BBT_SIG_OFS	(1)
#define OOB_FACT_BBT_SIG_LEN	(7)

static int read_fact_bbt(struct mtd_info *mtd, u32 page, u8 *fact_bbt, u32 fact_bbt_size)
{
	const u8 oob_signature[OOB_FACT_BBT_SIG_LEN] = "mtknand";
	struct nand_chip *chip = mtd->priv;

	// read oob
	if (mtk_nand_read_oob_hw(mtd, chip, page) == 0) {
		if (chip->oob_poi[OOB_BAD_BLOCK_OFFSET] != 0xFF) {
			printk(KERN_WARNING "%s: [%s] bad block on FACT_BBT page %x\n", MTK_NAND_MODULE_TEXT, __FUNCTION__, page);
			return -1;
		}
		if (memcmp(&chip->oob_poi[OOB_FACT_BBT_SIG_OFS], oob_signature, OOB_FACT_BBT_SIG_LEN) != 0) {
			return -1;
		}
		if (mtk_nand_exec_read_page(mtd, page, mtd->writesize, chip->buffers->databuf, chip->oob_poi)) {
			u32 data_size = (fact_bbt_size <= mtd->writesize) ? fact_bbt_size : mtd->writesize;
			memcpy(fact_bbt, chip->buffers->databuf, data_size);
			return 0;
		}
	}

	printk(KERN_WARNING "%s: [%s] failed at page %x\n", MTK_NAND_MODULE_TEXT, __FUNCTION__, page);
	return -1;
}

static int load_fact_bbt(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	u32 i, page, total_block, fact_bbt_size;
	u8 *fact_bbt = NULL;
	int ret = -1;

	if (!chip->bbt)
		return -1;

	total_block = 1u << (chip->chip_shift - chip->phys_erase_shift);
	fact_bbt_size = total_block >> 2;

	if (fact_bbt_size)
		fact_bbt = (u8 *)kzalloc(fact_bbt_size, GFP_KERNEL);

	if (!fact_bbt)
		return -ENOMEM;

	for (i = total_block - 1; i >= (total_block - FACT_BBT_BLOCK_NUM); i--) {
		page = i << (chip->phys_erase_shift - chip->page_shift);
		if (read_fact_bbt(mtd, page, fact_bbt, fact_bbt_size) == 0) {
			printk("%s: success load FACT_BBT from block %d\n", MTK_NAND_MODULE_TEXT, i);
			ret = 0;
			break;
		}
	}

	if (ret == 0) {
		for (i = 0; i < fact_bbt_size; i++)
			chip->bbt[i] |= fact_bbt[i];
	}

	kfree(fact_bbt);

#if __DEBUG_NAND
	for (i = 0; i < fact_bbt_size; i++) {
		printk("%02x ", chip->bbt[i]);
		if (!((i+1) & 0x1f))
			printk("\n");
	}
#endif

	return ret;
}
#endif

static int mtk_nand_probe(struct platform_device *pdev)
{
	struct mtk_nand_host *host;
	struct mtk_nand_host_hw *hw;
	struct mtd_info *mtd;
	struct nand_chip *chip;
	int err = 0;
	u32 i;
#if !defined (CONFIG_MTD_NAND_USE_UBI_PART)
	uint32_t kernel_size = 0x200000;
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	_ihdr_t hdr;
	loff_t offs;
	size_t ret_len = 0;
#endif
#endif

	hw = (struct mtk_nand_host_hw *)pdev->dev.platform_data;
	BUG_ON(!hw);

	/* Allocate memory for the device structure (and zero it) */
	host = kzalloc(sizeof(struct mtk_nand_host), GFP_KERNEL);
	if (!host) {
		MSG(INIT, "%s: failed to allocate device structure!\n", MTK_NAND_MODULE_TEXT);
		return -ENOMEM;
	}

	/* Allocate memory for 16 byte aligned buffer */
	local_buffer_16_align = local_buffer + 16 - ((u32) local_buffer % 16);

	host->hw = hw;

	/* init mtd data structure */
	chip = &host->nand_chip;
	chip->priv = host;     /* link the private data structures */

	mtd = &host->mtd;
	mtd->priv = chip;
	mtd->owner = THIS_MODULE;
	mtd->name = MTK_NAND_DRV_NAME;

	/* Set address of NAND IO lines */
	chip->IO_ADDR_R = (void __iomem *)NFI_DATAR_REG32;
	chip->IO_ADDR_W = (void __iomem *)NFI_DATAW_REG32;
	chip->chip_delay = 20; /* 20us command delay time */

	chip->init_size = mtk_nand_init_size;
	chip->select_chip = mtk_nand_select_chip;
	chip->dev_ready = mtk_nand_dev_ready;
	chip->cmdfunc = mtk_nand_command_bp;

	/* erase and read_page is api extension */
	chip->erase = mtk_nand_erase;
	chip->read_page = mtk_nand_read_page;

	chip->read_byte = mtk_nand_read_byte;
	chip->read_buf = mtk_nand_read_buf;
	chip->write_buf = mtk_nand_write_buf;
	chip->write_page = mtk_nand_write_page;

#if ECC_ENABLE
	chip->ecc.mode = NAND_ECC_HW;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
	chip->ecc.strength = 4;
#endif
#else
	chip->ecc.mode = NAND_ECC_NONE;
#endif
	chip->ecc.layout = &nand_oob_64;
	chip->ecc.size = NFI_NAND_ECC_SIZE;
	chip->ecc.bytes = NFI_NAND_ECC_BYTES;
	chip->ecc.read_page = mtk_nand_read_page_hwecc;
	chip->ecc.write_page = mtk_nand_write_page_hwecc;
	chip->ecc.read_oob = mtk_nand_read_oob;
	chip->ecc.write_oob = mtk_nand_write_oob;

	/* skip bbt scan (scan later) */
	chip->options = NAND_SKIP_BBTSCAN;

	chip->block_bad = mtk_nand_block_bad;
	chip->block_markbad = mtk_nand_block_markbad;   // need to add nand_get_device()/nand_release_device().

	/* init NFI host */
	mtk_nand_init_hw();

	memset(&nand_devinfo, 0, sizeof(flashdev_info));

	/* scan the NAND devices */
	if (nand_scan(mtd, hw->nfi_cs_num)) {
		printk("%s: nand_scan failed!\n", MTK_NAND_MODULE_TEXT);
		err = -ENXIO;
		goto out_err;
	}

	/* check init_size completed (ONFI?) */
	if (!nand_devinfo.pagesize) {
		nand_devinfo.devicename = "Generic";
		nand_devinfo.iowidth = 8;
		nand_devinfo.addr_cycle = 5;
		nand_devinfo.timmingsetting = NFI_DEFAULT_ACCESS_TIMING;
		nand_devinfo.advancedmode = 0;
		
		nand_devinfo.pagesize = mtd->writesize;
		nand_devinfo.sparesize = mtd->oobsize;
		nand_devinfo.blocksize = (u16)(mtd->erasesize >> 10);
		nand_devinfo.totalsize = (u16)(chip->chipsize >> 20);
	}

	/* update ecc layout */
	if (nand_devinfo.pagesize == 4096)
		chip->ecc.layout = &nand_oob_128;
	else if (nand_devinfo.pagesize == 2048)
		chip->ecc.layout = &nand_oob_64;
	else if (nand_devinfo.pagesize == 512)
		chip->ecc.layout = &nand_oob_16;

	chip->ecc.layout->eccbytes = nand_devinfo.sparesize - OOB_AVAIL_PER_SECTOR*(nand_devinfo.pagesize/NFI_NAND_SECTOR_SIZE);
	for (i = 0; i < chip->ecc.layout->eccbytes; i++)
		chip->ecc.layout->eccpos[i] = OOB_AVAIL_PER_SECTOR * (nand_devinfo.pagesize/NFI_NAND_SECTOR_SIZE) + i;

	/* update NFI hc after chip detect */
	mtk_nand_update_hw(mtd);
	mtk_nand_select_chip(mtd, NFI_DEFAULT_CS);

	/* now bbt scan */
	chip->scan_bbt(mtd);

#if defined (LOAD_FACT_BBT)
	load_fact_bbt(mtd);
	chip->chipsize -= (FACT_BBT_POOL_SIZE << chip->phys_erase_shift);
#endif
	mtd->size = chip->chipsize;

#if defined (SKIP_BAD_BLOCK)
	shift_on_bbt = 1;
#endif

#if !defined (CONFIG_MTD_NAND_USE_UBI_PART)
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	offs = NAND_MTD_KERNEL_PART_OFFSET;
	memset(&hdr, 0, sizeof(hdr));
	mtd_read(mtd, offs, sizeof(hdr), &ret_len, (u_char *)(&hdr));
	if (ret_len == sizeof(hdr) && hdr.ih_ksz != 0)
		kernel_size = ntohl(hdr.ih_ksz);
#endif
	/* calculate partition table */
	recalc_partitions(mtd->size, kernel_size);
#else
	/* calculate partition table for UBIFS */
	recalc_partitions(mtd->size);
#endif

	/* register the partitions */
	err = add_mtd_partitions(mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
	if (err)
		goto out_err;

	platform_set_drvdata(pdev, host);
	return 0;

out_err:
	MSG(INIT, "%s: [%s] failed, err = %d!\n", MTK_NAND_MODULE_TEXT, __FUNCTION__, err);

	nand_release(mtd);
	kfree(host);

	return err;
}

static int mtk_nand_remove(struct platform_device *pdev)
{
	struct mtk_nand_host *host = platform_get_drvdata(pdev);
	struct mtd_info *mtd;

	if (!host)
		return 0;

	mtd = &host->mtd;

	platform_set_drvdata(pdev, NULL);

	nand_release(mtd);
	kfree(host);

	return 0;
}

#ifdef CONFIG_PM
static int mtk_nand_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) {
		MSG(POWERCTL, "%s: [%s] device busy, suspend fail!\n", MTK_NAND_MODULE_TEXT, __FUNCTION__);
		return 1; // BUSY
	}

	MSG(POWERCTL, "%s: [%s]\n", MTK_NAND_MODULE_TEXT, __FUNCTION__);
	return 0;
}

static int mtk_nand_resume(struct platform_device *pdev)
{
	MSG(POWERCTL, "%s: [%s]\n", MTK_NAND_MODULE_TEXT, __FUNCTION__);
	return 0;
}
#endif

static struct platform_driver mtk_nand_driver = {
	.probe = mtk_nand_probe,
	.remove = mtk_nand_remove,
#ifdef CONFIG_PM
	.suspend = mtk_nand_suspend,
	.resume = mtk_nand_resume,
#endif
	.driver = {
		.name = MTK_NAND_DRV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init mtk_nand_init(void)
{
	if (ra_check_flash_type() != BOOT_FROM_NAND)
		return 0;

	printk(KERN_INFO "MediaTek NAND driver init, version %s\n", MTK_NAND_MODULE_VERSION);

	return platform_driver_register(&mtk_nand_driver);
}

static void __exit mtk_nand_exit(void)
{
	platform_driver_unregister(&mtk_nand_driver);
}

module_init(mtk_nand_init);
module_exit(mtk_nand_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek MTD NAND driver for flash chips");
