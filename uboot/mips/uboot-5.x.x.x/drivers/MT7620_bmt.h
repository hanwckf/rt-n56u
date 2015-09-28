#ifndef __BMT_H__
#define __BMT_H__

//#include "nand_def.h"
#include <linux/types.h>

//#define MTK_NAND_BMT_DEBUG

#if defined(__PRELOADER_NAND__)

#include "nand.h"

#elif defined(__UBOOT_NAND__)

#include <linux/mtd/nand.h>
//#include "mt6575_typedefs.h"
//#include "mt6575_nand.h"

#elif defined(__KERNEL_NAND__)

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include "mt6575_nand.h"
#include <linux/mtd/mt6575_typedefs.h>

#endif

#define MAX_BMT_SIZE        (0x80)
#define BMT_VERSION         (1)         // initial version

#define MAIN_SIGNATURE_OFFSET   (0)
//#define OOB_SIGNATURE_OFFSET    (1)
#define OOB_SIGNATURE_OFFSET    (11)
//#define OOB_INDEX_OFFSET        (29)
// in case of 512 byte size page only has 16 bytes spare area, so we move the offset from 29 to 14
#define OOB_INDEX_OFFSET        (14)
#define OOB_INDEX_SIZE          (2)
#define FAKE_INDEX              (0xAAAA)

typedef struct _bmt_entry_
{
    u16 bad_index;      // bad block index
    u16 mapped_index;  // mapping block index in the replace pool
} bmt_entry;

typedef enum
{
    UPDATE_ERASE_FAIL,
    UPDATE_WRITE_FAIL,
    UPDATE_UNMAPPED_BLOCK,
    UPDATE_REASON_COUNT,
} update_reason_t;

typedef struct {
    bmt_entry table[MAX_BMT_SIZE];
    u8 version;
    u8 mapped_count;                // mapped block count in pool
    u8 bad_count;                   // bad block count in pool. Not used in V1
}bmt_struct;

// for compile
typedef int bool;
#define true (1)
#define false (0)

/***************************************************************
*                                                              *
* Interface BMT need to use                                    *
*                                                              *
***************************************************************/
#if defined(__PRELOADER_NAND__)

extern int mt6575_nand_read_page_hw(u32 page, u8 *dat, u8 *oob);
extern bool nand_block_bad_hw(u32 offset);
extern bool mt6575_nand_erase_hw(u32 offset);
extern bool mark_block_bad_hw(u32 offset);
extern int mt6575_nand_write_page_hw(u32 page, u8 *dat, u8 *oob);

#elif defined(__UBOOT_NAND__)

//extern bool mt6575_nand_exec_read_page_hw(struct nand_chip *nand, u32 page, u32 page_size, u8 *dat, u8 *oob);
//extern bool nand_block_bad_hw(struct nand_chip *nand, u32 offset);
extern int nfc_read_page(char *buf, int page);
extern int nfc_write_page(char *buf, int page);
extern int ranand_block_isbad(loff_t offs);
extern int nfc_erase_block(int row_addr);
extern int nfc_write_oob(int page, unsigned int offs, char *buf, int len);

#elif defined(__KERNEL_NAND__)

extern bool mt6575_nand_exec_read_page(struct mtd_info *mtd, u32 row, u32 page_size, u8 *dat, u8 *oob);
extern int mt6575_nand_block_bad_hw(struct mtd_info *mtd, loff_t ofs);
extern int mt6575_nand_erase_hw(struct mtd_info *mtd, int page);
extern int mt6575_nand_block_markbad_hw(struct mtd_info *mtd, loff_t ofs);
extern int mt6575_nand_exec_write_page(struct mtd_info *mtd, u32 row, u32 page_size, u8 *dat, u8 *oob);

#endif



/***************************************************************
*                                                              *
* Different function interface for preloader/uboot/kernel      *
*                                                              *
***************************************************************/
void set_bad_index_to_oob(u8 *oob, u16 index);


#if defined(__PRELOADER_NAND__)

bmt_struct *init_bmt(struct nand_chip* nand, int size);
bool update_bmt(u32 offset, update_reason_t reason, u8 *dat, u8 *oob);
unsigned short get_mapping_block_index(int index);

#elif defined(__UBOOT_NAND__)

bmt_struct *init_bmt(int size);
bool update_bmt(u32 offset, update_reason_t reason, u8 *dat, u8 *oob);
unsigned short get_mapping_block_index(int index);
int update_bmt_page(int *page, u8 *oob);

#elif defined(__KERNEL_NAND__)

bmt_struct *init_bmt(struct nand_chip* nand, int size);
bool update_bmt(u32 offset, update_reason_t reason, u8 *dat, u8 *oob);
unsigned short get_mapping_block_index(int index);
int update_bmt_page(int *page, u8 *oob);

#endif


#endif // #ifndef __BMT_H__
