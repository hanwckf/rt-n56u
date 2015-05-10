#ifndef _NAND_API_H_
#define _NAND_API_H_
#include "../autoconf.h"

#if defined(UBI_SUPPORT)
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <ubi_uboot.h>
#include <asm/errno.h>
#include <jffs2/load_kernel.h>

extern struct mtd_info mt7620a_mtdinfo;

extern int get_ubi_volume_idseq_by_addr(const char *name);
extern char *get_ubi_volume_param(unsigned long addr, unsigned long *offset, unsigned int *size);
extern int init_ubi_volumes(struct ubi_device *ubi);
#endif

extern unsigned long ranand_init(void);
#if defined(CONFIG_RTAC52U) || defined(ON_BOARD_NAND_FLASH_COMPONENT)
extern int ranand_set_sbb_max_addr(unsigned long addr);
#else
static inline int ranand_set_sbb_max_addr(unsigned long addr) {return 0;}
#endif

#if defined(CFG_ENV_IS_IN_NAND)
extern void ranand_set_allow_erase_badblock(unsigned int dont);

extern char *ranand_id(void);
extern int nand_env_init(void);

extern int ranand_block_isbad(loff_t offs);
extern int ranand_write(unsigned char *buf, unsigned int to, int len);
extern int ranand_read(unsigned char *buf, unsigned int from, int len);
extern int ranand_erase(unsigned int offs, int len);
extern int ranand_erase_write(unsigned char *buf, unsigned int offs, int count);
extern int ranand_check_space(unsigned int offset, unsigned int len, unsigned int bound);
extern int nfc_read_page(unsigned char *buf, int page);
#endif

#if defined(UBOOT_STAGE1) || defined(UBOOT_STAGE2)
extern int ranand_write_stage2(unsigned int addr, unsigned int len);
#endif

#if defined(UBOOT_STAGE1)
/* Prepare large enough buffer to hold stage2 code, even it becomes big as much as whole bootstage2 area. */
struct stage2_desc {
	unsigned int	offset;			/* start offset of a stage2 code in flash */
	unsigned int	boundary;		/* block-aligned end offset of a stage2 code in flash */
	unsigned int	len;			/* length of stage2 code */
	unsigned int	blk_len;		/* length of stage2 code and aligned to block size boundary */
	uint32_t	failed;			/* number of uncorrectable error */
	uint32_t	corrected;		/* number of correctable error */
	unsigned int	crc_error;		/* 0: good stage2 code; otherwise: CRC error */
	char		name[10];		/* such as: stage2-0, stage2-1, etc */
	unsigned char	*code;			/* pointer to start address of this copy stage2 code in RAM */
};

#define MAX_NR_STAGE2	(6 + 1)
struct stage2_loc {
	unsigned int	count;			/* number of all items */
	unsigned int	nr_blk_read;		/* number of blocks are readed to buffer */
	struct stage2_desc *good;		/* pointer to descriptor of good stage2 code */
	struct stage2_desc desc[MAX_NR_STAGE2];
	unsigned char code[CFG_BOOTSTAGE2_SIZEALL]  __attribute__((aligned(4)));	/* buffer to hold all stage2 code. */
};

extern int ranand_check_and_fix_stage1(void);
extern int ranand_locate_stage2(struct stage2_loc *s2);
#endif

#endif
