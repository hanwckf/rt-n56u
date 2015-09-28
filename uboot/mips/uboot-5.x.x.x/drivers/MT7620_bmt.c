#include <common.h>
#include <malloc.h>
#include "ralink_nand.h"

#define __UBOOT_NAND__ (1)
#include "MT7620_bmt.h"
#define MTK_NAND_BMT
//#define MTK_NAND_BMT_DEBUG

typedef struct {
    char signature[3];
    u8 version;
    u8 bad_count;           // bad block count in pool
    u8 mapped_count;        // mapped block count in pool
    u8 checksum;
    u8 reseverd[13];
} phys_bmt_header;

typedef struct {
    phys_bmt_header header;
    bmt_entry table[MAX_BMT_SIZE];
} phys_bmt_struct;

typedef struct {
    char signature[3];
} bmt_oob_data;

static char MAIN_SIGNATURE[] = "BMT";
static char OOB_SIGNATURE[] = "bmt";
#define SIGNATURE_SIZE      (3)

#define MAX_DAT_SIZE        0x1000
#define MAX_OOB_SIZE        0x80

#if defined(__PRELOADER_NAND__)

static struct nand_chip *nand_chip_bmt;
#define BLOCK_SIZE_BMT          (nand_chip_bmt->erasesize)
#define PAGE_SIZE_BMT           (nand_chip_bmt->page_size)

#elif defined(__UBOOT_NAND__)

//static struct mtd_info *mtd_bmt;
//static struct nand_chip *nand_chip_bmt;

//#define BLOCK_SIZE_BMT          (1 << nand_chip_bmt->phys_erase_shift)
//#define PAGE_SIZE_BMT           (1 << nand_chip_bmt->page_shift)
#define BLOCK_SIZE_BMT		(1 << (CONFIG_NUMPAGE_PER_BLOCK_BIT + CONFIG_PAGE_SIZE_BIT))
#define PAGE_SIZE_BMT		(1 << CONFIG_PAGE_SIZE_BIT)

#elif defined(__KERNEL_NAND__)

static struct mtd_info *mtd_bmt;
static struct nand_chip *nand_chip_bmt;
#define BLOCK_SIZE_BMT          (1 << nand_chip_bmt->phys_erase_shift)
#define PAGE_SIZE_BMT           (1 << nand_chip_bmt->page_shift)

#endif


#define OFFSET(block)       ((block) * BLOCK_SIZE_BMT)           //((block) << (mtd->erasesize_shift) + (page) << (mtd->writesize_shift))
#define PAGE_ADDR(block)    ((block) * BLOCK_SIZE_BMT / PAGE_SIZE_BMT)


/*********************************************************************
* Flash is splited into 2 parts, system part is for normal system    *
* system usage, size is system_block_count, another is replace pool  *
*    +-------------------------------------------------+             *
*    |     system_block_count     |   bmt_block_count  |             *
*    +-------------------------------------------------+             *
*********************************************************************/
static u32 total_block_count;       // block number in flash
static u32 system_block_count;
static int bmt_block_count;         // bmt table size
// static int bmt_count;               // block used in bmt
static int page_per_block;          // page per count

static u32 bmt_block_index;         // bmt block index
static bmt_struct bmt;              // dynamic created global bmt table

extern int is_nand_page_2048;
//static u8 dat_buf[MAX_DAT_SIZE];
//static u8 oob_buf[MAX_OOB_SIZE];
static u8 page_buf[MAX_DAT_SIZE+MAX_OOB_SIZE];
static u8 *dat_buf = NULL;
static u8 *oob_buf = NULL;

static bool pool_erased;


/***************************************************************
*                                                              
* Interface adaptor for preloader/uboot/kernel                 
*    These interfaces operate on physical address, read/write
*       physical data.
*                                                              
***************************************************************/
#if defined(__PRELOADER_NAND__)
int nand_read_page_bmt(u32 page, u8 *dat, u8 *oob)
{
//    int offset, start, len, i;
    return mt6575_nand_read_page_hw(page, dat, oob);
        //return false;
/*
    offset = 0;

    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && nand_oob->oobfree[i].length; i++)
    {
        start = nand_oob->oobfree[i].offset;
        len = nand_oob->oobfree[i].length;
        memcpy(buf + PAGE_SIZE_BMT + offset, g_nand_spare + start, len);;
        offset += len;
    }

    return true;
*/
}

bool nand_block_bad_bmt(u32 offset)
{
    return nand_block_bad_hw(offset);
}

bool nand_erase_bmt(u32 offset)
{
    return mt6575_nand_erase_hw(offset);
}

int mark_block_bad_bmt(u32 offset)
{
    return mark_block_bad_hw(offset);
}

bool nand_write_page_bmt(u32 page, u8 *dat, u8 *oob)
{
    return mt6575_nand_write_page_hw(page, dat, oob);
}



#elif defined(__UBOOT_NAND__)
// return true if success
int nand_read_page_bmt(u32 page, u8 *dat, u8 *oob)
{
    //return mt6575_nand_exec_read_page_hw(nand_chip_bmt, page, PAGE_SIZE_BMT, dat, oob);
    // return mt6575_nand_read_page_hw(page, dat, oob);
    //return mt6575_nand_exec_read_page(mtd_bmt, page, PAGE_SIZE_BMT, dat, oob);
    if ((dat+CFG_PAGESIZE) != oob)
    {
		u8 *buf;
    	buf = (u8 *)malloc(CFG_PAGESIZE+CFG_PAGE_OOBSIZE);
    	if (!buf)
    		return false;
    	memcpy(buf, dat, CFG_PAGESIZE);
    	memcpy(buf+CFG_PAGESIZE, oob, CFG_PAGE_OOBSIZE);
    	if (nfc_read_page(buf, page))
		{
			free(buf);
			return false;
		}
		free(buf);
    }
    else
    {
		if (nfc_read_page(dat, page))
			return false;
	}
	
	return true;
}

bool nand_block_bad_bmt(u32 offset)
{
    //return nand_block_bad_hw(nand_chip_bmt, offset);
    return ranand_block_isbad((loff_t)offset);
}

// actually uboot should never use the following 3 functions 
bool nand_erase_bmt(u32 offset)
{
	int page = offset >> CONFIG_PAGE_SIZE_BIT;
	if (nfc_erase_block(page))
		return false;
    return true;        // mt6575_nand_erase_hw(offset);
}

int mark_block_bad_bmt(u32 offset)
{
	u8 bb[CFG_PAGE_OOBSIZE];
	int page = offset >> CONFIG_PAGE_SIZE_BIT;

	memset(bb, 0xff, CFG_PAGE_OOBSIZE);
	bb[CONFIG_BAD_BLOCK_POS] = 0x33;
	nfc_write_oob(page, 0, bb, CFG_PAGE_OOBSIZE);
    return 0;             //mark_block_bad_hw(offset);
}

bool nand_write_page_bmt(u32 page, u8 *dat, u8 *oob)
{
    if ((dat+CFG_PAGESIZE) != oob)
    {
		u8 *buf;
    	buf = (u8 *)malloc(CFG_PAGESIZE+CFG_PAGE_OOBSIZE);
    	if (!buf)
    		return false;
    	memcpy(buf, dat, CFG_PAGESIZE);
    	memcpy(buf+CFG_PAGESIZE, oob, CFG_PAGE_OOBSIZE);
		if (nfc_write_page(buf, page))
		{
			free(buf);
			return false;
		}
		free(buf);
    	
    }
	else
	{
		if (nfc_write_page(dat, page))
			return false;
	}
	return true;        // mt6575_nand_write_page_hw(page, dat, oob);
}

#elif defined(__KERNEL_NAND__)

int nand_read_page_bmt(u32 page, u8 *dat, u8 *oob)
{
    return mt6575_nand_exec_read_page(mtd_bmt, page, PAGE_SIZE_BMT, dat, oob);
}

bool nand_block_bad_bmt(u32 offset)
{
    return mt6575_nand_block_bad_hw(mtd_bmt, (loff_t)offset);
}

bool nand_erase_bmt(u32 offset)
{
    int status;
    if (offset < 0x20000)
    {
        printf( "erase offset: 0x%x\n", offset);
    }
    
    status = mt6575_nand_erase_hw(mtd_bmt, offset / PAGE_SIZE_BMT);      // as nand_chip structure doesn't have a erase function defined
    if (status & NAND_STATUS_FAIL)
        return false;
    else
        return true;
}

int mark_block_bad_bmt(u32 offset)
{
    return mt6575_nand_block_markbad_hw(mtd_bmt, (loff_t)offset);             //mark_block_bad_hw(offset);
}

bool nand_write_page_bmt(u32 page, u8 *dat, u8 *oob)
{
    if (mt6575_nand_exec_write_page(mtd_bmt, page, PAGE_SIZE_BMT, dat, oob))
        return false;
    else
        return true;
}

#endif



/***************************************************************
*                                                              *
* static internal function                                     *
*                                                              *
***************************************************************/
static void dump_bmt_info(bmt_struct *bmt)
{
    int i;
    
    printf( "BMT v%d. total %d mapping:\n", bmt->version, bmt->mapped_count);
    for (i = 0; i < bmt->mapped_count; i++)
    {
        printf( "\t0x%x -> 0x%x\n", bmt->table[i].bad_index, bmt->table[i].mapped_index);
    }
}

void dump_bmt()
{
	dump_bmt_info(&bmt);
}


static bool match_bmt_signature(u8 *dat, u8 *oob)
{
    // int i;
    // char *iter = OOB_SIGNATURE;
    if (memcmp(dat + MAIN_SIGNATURE_OFFSET, MAIN_SIGNATURE, SIGNATURE_SIZE))
    {
        return false;
    }
    
    if (memcmp(oob + OOB_SIGNATURE_OFFSET, OOB_SIGNATURE, SIGNATURE_SIZE))
    {
        printf( "main signature match, oob signature doesn't match, but ignore\n");
    }
    return true;
}

static u8 cal_bmt_checksum(phys_bmt_struct *phys_table, int bmt_size)
{
    int i;
    u8 checksum = 0;
    u8 *dat = (u8 *)phys_table;

    checksum += phys_table->header.version;
    // checksum += phys_table.header.bad_count;
    checksum += phys_table->header.mapped_count;

    dat += sizeof(phys_bmt_header);
    for (i = 0; i < bmt_size * sizeof(bmt_entry); i++)
    {
        checksum += dat[i];
    }

    return checksum;
}

// return -1 for unmapped block, and bad block index if mapped.
static int is_block_mapped(int index)
{
    int i;
    for (i = 0; i < bmt.mapped_count; i++)
    {
        if (index == bmt.table[i].mapped_index)
            return i;
    }
    return -1;
}

static bool is_page_used(u8 *dat, u8 *oob)
{
    return ( (oob[OOB_INDEX_OFFSET] != 0xFF)  || (oob[OOB_INDEX_OFFSET + 1] != 0xFF) || 
    (oob[CONFIG_ECC_OFFSET] != 0xFF) || (oob[CONFIG_ECC_OFFSET+1] != 0xFF) || (oob[CONFIG_ECC_OFFSET+2] != 0xFF) );
}

static bool valid_bmt_data(phys_bmt_struct *phys_table)
{
    int i;
    u8 checksum = cal_bmt_checksum(phys_table, bmt_block_count);
    
    // checksum correct?
    if ( phys_table->header.checksum != checksum)
    {
        printf( "BMT Data checksum error: %x %x\n", phys_table->header.checksum, checksum);
        return false;
    }

    printf( "BMT Checksum is: 0x%x\n", phys_table->header.checksum);
    
    // block index correct?
    for (i = 0; i < phys_table->header.mapped_count; i++)
    {
        if (phys_table->table[i].bad_index >= total_block_count ||
            phys_table->table[i].mapped_index >= total_block_count ||
            phys_table->table[i].mapped_index < system_block_count)
        {
            printf( "index error: bad_index: %d, mapped_index: %d\n", 
                phys_table->table[i].bad_index, phys_table->table[i].mapped_index);
            return false;
        }
    }

    // pass check, valid bmt.
    printf( "Valid BMT, version v%d\n", phys_table->header.version);
    return true;
}


static void fill_nand_bmt_buffer(bmt_struct *bmt, u8 *dat, u8 *oob)
{
    phys_bmt_struct phys_bmt;

    dump_bmt_info(bmt);

    // fill phys_bmt_struct structure with bmt_struct
    memset(&phys_bmt, 0xFF, sizeof(phys_bmt));
    
    memcpy(phys_bmt.header.signature, MAIN_SIGNATURE, SIGNATURE_SIZE);
    phys_bmt.header.version = BMT_VERSION;
    // phys_bmt.header.bad_count = bmt->bad_count;
    phys_bmt.header.mapped_count = bmt->mapped_count;
    memcpy(phys_bmt.table, bmt->table, sizeof(bmt_entry) * bmt_block_count);

    phys_bmt.header.checksum = cal_bmt_checksum(&phys_bmt, bmt_block_count);

    memcpy(dat + MAIN_SIGNATURE_OFFSET, &phys_bmt, sizeof(phys_bmt));
    memcpy(oob + OOB_SIGNATURE_OFFSET, OOB_SIGNATURE, SIGNATURE_SIZE);
}

// return valid index if found BMT, else return 0
static int load_bmt_data(int start, int pool_size)
{
    int bmt_index = start + pool_size - 1;        // find from the end
    phys_bmt_struct phys_table;
    int i;
 
    printf(  "[%s]: begin to search BMT from block 0x%x\n", __FUNCTION__, bmt_index);

    for (bmt_index = start + pool_size - 1; bmt_index >= start; bmt_index--)
    {
        if (nand_block_bad_bmt(OFFSET(bmt_index)))
        {
            printf( "Skip bad block: %d\n", bmt_index);
            continue;
        }
        
        if (!nand_read_page_bmt(PAGE_ADDR(bmt_index), dat_buf, oob_buf))
        {
            printf( "Error found when read block %d\n", bmt_index);
            continue;
        }

        if (!match_bmt_signature(dat_buf, oob_buf))
        {
            continue;
        }

        printf( "Match bmt signature @ block: 0x%x\n", bmt_index);
        
        memcpy(&phys_table, dat_buf + MAIN_SIGNATURE_OFFSET, sizeof(phys_table));

        if (!valid_bmt_data(&phys_table))
        {
            printf( "BMT data is not correct %d\n", bmt_index);
            continue;
        }
        else
        {
            bmt.mapped_count = phys_table.header.mapped_count;
            bmt.version = phys_table.header.version;
            // bmt.bad_count = phys_table.header.bad_count;
            memcpy(bmt.table, phys_table.table, bmt.mapped_count * sizeof(bmt_entry));

            printf( "bmt found at block: %d, mapped block: %d\n", bmt_index, bmt.mapped_count);

            for (i = 0; i < bmt.mapped_count; i++)
            {
                if (!nand_block_bad_bmt(OFFSET(bmt.table[i].bad_index)))
                {
                    printf( "block 0x%x is not mark bad, should be power lost last time\n", bmt.table[i].bad_index);
                    mark_block_bad_bmt(OFFSET(bmt.table[i].bad_index));
                }
            }
            
            return bmt_index;
        }
    }    

    printf( "bmt block not found!\n");
    return 0;
}


/*************************************************************************
* Find an available block and erase.                                     *
* start_from_end: if true, find available block from end of flash.       *
*                 else, find from the beginning of the pool              *
* need_erase: if true, all unmapped blocks in the pool will be erased    *
*************************************************************************/
static int find_available_block(bool start_from_end)
{
    int i;      // , j;
    int block = system_block_count;
    int direction;
    // int avail_index = 0;
    printf( "Try to find_available_block, pool_erase: %d\n", pool_erased);

    // erase all un-mapped blocks in pool when finding avaliable block
    if (!pool_erased)
    {
        printf( "Erase all un-mapped blocks in pool\n");
        for (i = 0; i < bmt_block_count; i++)
        {
	        if (block == bmt_block_index)
	        {
	            printf( "Skip bmt block 0x%x\n", block);
	            continue;
	        }            
	        
	        if (nand_block_bad_bmt(OFFSET(block + i)))
            {
                printf( "Skip bad block 0x%x\n", block + i);
                continue;
            }

//if(block==4095)
//{
//	continue;
//}

            if (is_block_mapped(block + i) >= 0)
            {
                printf( "Skip mapped block 0x%x\n", block + i);
                continue;
            }

            if (!nand_erase_bmt(OFFSET(block + i)))
            {
                printf( "Erase block 0x%x failed\n", block + i);
                mark_block_bad_bmt(OFFSET(block + i));
            }
        }

        pool_erased = 1;
    }

    if (start_from_end)
    {
        block = total_block_count - 1;
        direction = -1;
    }
    else
    {
        block = system_block_count;
        direction = 1;
    }

    for (i = 0; i < bmt_block_count; i++, block += direction)
    {
        if (block == bmt_block_index)
        {
            printf( "Skip bmt block 0x%x\n", block);
            continue;
        }
        
        if (nand_block_bad_bmt(OFFSET(block)))
        {
            printf( "Skip bad block 0x%x\n", block);
            continue;
        }

        if (is_block_mapped(block) >= 0)
        {
            printf( "Skip mapped block 0x%x\n", block);
            continue;
        }

        printf( "Find block 0x%x available\n", block);
        return block;
    }

    return 0;
}


static unsigned short get_bad_index_from_oob(u8 *oob_buf)
{
    unsigned short index;
    memcpy(&index, oob_buf + OOB_INDEX_OFFSET, OOB_INDEX_SIZE);

    return index;
}

void set_bad_index_to_oob(u8 *oob, u16 index)
{
    memcpy(oob + OOB_INDEX_OFFSET, &index, sizeof(index));
}

static int migrate_from_bad(int offset, u8 *write_dat, u8 *write_oob)
{
    int page;
    int error_block = offset / BLOCK_SIZE_BMT;
    int error_page = (offset / PAGE_SIZE_BMT) % page_per_block;
    int to_index;

    //memcpy(oob_buf, write_oob, MAX_OOB_SIZE);
    memset(oob_buf, 0xff, MAX_OOB_SIZE); // in case of BI fields has been set, write_oob should not be copied...

    to_index = find_available_block(false);

    if (!to_index)
    {
        printf( "Cannot find an available block for BMT\n");
        return 0;
    }

    {       // migrate error page first
        printf( "Write error page: 0x%x\n", error_page);
        if (!write_dat)
        {
            nand_read_page_bmt(PAGE_ADDR(error_block) + error_page, dat_buf, NULL);
            write_dat = dat_buf;
        }

        // memcpy(oob_buf, write_oob, MAX_OOB_SIZE);
        
        if (error_block < system_block_count)
            set_bad_index_to_oob(oob_buf, error_block);       // if error_block is already a mapped block, original mapping index is in OOB.
                
        if (!nand_write_page_bmt(PAGE_ADDR(to_index) + error_page, write_dat, oob_buf))
        {
            printf( "Write to page 0x%x fail\n", PAGE_ADDR(to_index) + error_page);
            mark_block_bad_bmt(OFFSET(to_index));
            return migrate_from_bad(offset, write_dat, write_oob);
        }
    }



    for (page = 0; page < page_per_block; page++)
    {
        if (page != error_page)
        {
            nand_read_page_bmt(PAGE_ADDR(error_block) + page, dat_buf, oob_buf);
            if (is_page_used(dat_buf, oob_buf))
            {
                if (error_block < system_block_count)
                {
                    set_bad_index_to_oob(oob_buf, error_block);
                }
                printf( "\tmigrate page 0x%x to page 0x%x\n", 
                PAGE_ADDR(error_block) + page, PAGE_ADDR(to_index) + page);
                oob_buf[CONFIG_BAD_BLOCK_POS] = 0xff;
                if (!nand_write_page_bmt(PAGE_ADDR(to_index) + page, dat_buf, oob_buf))
                {
                    printf( "Write to page 0x%x fail\n", PAGE_ADDR(to_index) + page);
                    mark_block_bad_bmt(OFFSET(to_index));
                    return migrate_from_bad(offset, write_dat, write_oob);
                }
            }
        }
    }

    printf( "Migrate from 0x%x to 0x%x done!\n", error_block, to_index);

    return to_index;
}

static bool write_bmt_to_flash(u8 *dat, u8 *oob)
{
    bool need_erase = true;
    printf( "Try to write BMT\n");
    
    if (bmt_block_index == 0)
    {
        // if we don't have index, we don't need to erase found block as it has been erased in find_available_block()
        need_erase = false;     
        if ( !(bmt_block_index = find_available_block(true)) )
        {
            printf( "Cannot find an available block for BMT\n");
            return false;
        }
    }

    printf( "Find BMT block: 0x%x\n", bmt_block_index);
    
    // write bmt to flash
    if (need_erase)
    {
        if (!nand_erase_bmt(OFFSET(bmt_block_index)))
        {
            printf( "BMT block erase fail, mark bad: 0x%x\n", bmt_block_index);
            mark_block_bad_bmt(OFFSET(bmt_block_index));
            // bmt.bad_count++;

            bmt_block_index = 0;
            return write_bmt_to_flash(dat, oob);        // recursive call 
        }
    }

    if ( !nand_write_page_bmt(PAGE_ADDR(bmt_block_index), dat, oob) )
    {
        printf( "Write BMT data fail, need to write again\n");
        mark_block_bad_bmt(OFFSET(bmt_block_index));
        // bmt.bad_count++;
        
        bmt_block_index = 0;
        return write_bmt_to_flash(dat, oob);        // recursive call 
    }

    printf( "Write BMT data to block 0x%x success\n", bmt_block_index);
    return true;
}

/*******************************************************************
* Reconstruct bmt, called when found bmt info doesn't match bad 
* block info in flash.
* 
* Return NULL for failure
*******************************************************************/
bmt_struct *reconstruct_bmt(bmt_struct * bmt)
{
    int i;
    int index = system_block_count;
    unsigned short bad_index;
    int mapped;

    // init everything in BMT struct 
    bmt->version = BMT_VERSION;
    bmt->bad_count = 0;
    bmt->mapped_count = 0;
    
    memset(bmt->table, 0, bmt_block_count * sizeof(bmt_entry));

    for (i = 0; i < bmt_block_count; i++, index++)
    {
        if (nand_block_bad_bmt(OFFSET(index)))
        {
            printf(  "Skip bad block: 0x%x\n", index);
            // bmt->bad_count++;
            continue;
        }

        printf(  "read page: 0x%x\n", PAGE_ADDR(index));
        nand_read_page_bmt(PAGE_ADDR(index), dat_buf, oob_buf);
        /* if (mt6575_nand_read_page_hw(PAGE_ADDR(index), dat_buf))
        {
            printf(  "Error when read block %d\n", bmt_block_index);
            continue;
        } */

        if ((bad_index = get_bad_index_from_oob(oob_buf)) >= system_block_count)
        {
            printf( "get bad index: 0x%x\n", bad_index);
            if (bad_index != 0xFFFF)
                printf(  "Invalid bad index found in block 0x%x, bad index 0x%x\n", index, bad_index);
            continue;
        }

        printf(  "Block 0x%x is mapped to bad block: 0x%x\n", index, bad_index);

        if (!nand_block_bad_bmt(OFFSET(bad_index)))
        {
            printf(  "\tbut block 0x%x is not marked as bad, invalid mapping\n", bad_index);
            continue;       // no need to erase here, it will be erased later when trying to write BMT
        }


        if ( (mapped = is_block_mapped(bad_index)) >= 0)
        {
            printf( "bad block 0x%x is mapped to 0x%x, should be caused by power lost, replace with one\n", 
                bmt->table[mapped].bad_index, bmt->table[mapped].mapped_index);
            bmt->table[mapped].mapped_index = index;    // use new one instead.
        }
        else
        {
            // add mapping to BMT
            bmt->table[bmt->mapped_count].bad_index = bad_index;
            bmt->table[bmt->mapped_count].mapped_index = index;
            bmt->mapped_count++;
        }

        printf(  "Add mapping: 0x%x -> 0x%x to BMT\n", bad_index, index);

    }

    printf(  "Scan replace pool done, mapped block: %d\n", bmt->mapped_count);
#ifdef MTK_NAND_BMT_DEBUG
    dump_bmt_info(bmt);
#endif
    // dump_bmt_info(bmt);

    // fill NAND BMT buffer
    memset(oob_buf, 0xFF, sizeof(oob_buf));
    fill_nand_bmt_buffer(bmt, dat_buf, oob_buf);

    // write BMT back
    if (!write_bmt_to_flash(dat_buf, oob_buf))
    {
        printf(  "TRAGEDY: cannot find a place to write BMT!!!!\n");
    }

    return bmt;
}

/*******************************************************************
* [BMT Interface]
*
* Description:
*   Init bmt from nand. Reconstruct if not found or data error
*
* Parameter:
*   size: size of bmt and replace pool
* 
* Return: 
*   NULL for failure, and a bmt struct for success
*******************************************************************/
//bmt_struct *init_bmt(struct nand_chip *chip, int size)
bmt_struct *init_bmt(int size)
{
	//return NULL;
#if defined(__KERNEL_NAND__)
    struct mt6575_nand_host *host;
#endif
    
    if (size > 0 && size < MAX_BMT_SIZE)
    {
        printf( "Init bmt table, size: %d\n", size);
        bmt_block_count = size;
    }
    else
    {
        printf( "Invalid bmt table size: %d\n", size);
        return NULL;
    }

#if defined(__PRELOADER_NAND__)

    nand_chip_bmt = chip;    
    system_block_count = chip->chipsize / chip->erasesize;
    total_block_count = bmt_block_count + system_block_count;
    page_per_block = chip->erasesize / chip->page_size;

#elif defined(__UBOOT_NAND__)

    //nand_chip_bmt = chip;
    //system_block_count = chip->chipsize >> chip->phys_erase_shift;
    //total_block_count = bmt_block_count + system_block_count;
    //page_per_block = BLOCK_SIZE_BMT / PAGE_SIZE_BMT;
	//mtd_bmt = &((struct mt6575_nand_host *)(chip->priv))->mtd;

	// current code does not detect 
	total_block_count = (1 << (CONFIG_CHIP_SIZE_BIT - CONFIG_PAGE_SIZE_BIT - CONFIG_NUMPAGE_PER_BLOCK_BIT));
	system_block_count = total_block_count - size;
    page_per_block = BLOCK_SIZE_BMT / PAGE_SIZE_BMT;

#elif defined(__KERNEL_NAND__)

    nand_chip_bmt = chip;
    system_block_count = chip->chipsize >> chip->phys_erase_shift;
    total_block_count = bmt_block_count + system_block_count;
    page_per_block = BLOCK_SIZE_BMT / PAGE_SIZE_BMT;
    host = (struct mt6575_nand_host *)chip->priv;
    mtd_bmt = &host->mtd;

    printf( "mtd_bmt: %p, nand_chip_bmt: %p\n", mtd_bmt, nand_chip_bmt);
#endif

    printf( "bmt count: %d, system count: %d\n", bmt_block_count, system_block_count);

    // set this flag, and unmapped block in pool will be erased.
    pool_erased = 0;

    // alloc size for bmt.
    memset(bmt.table, 0, size * sizeof(bmt_entry));

	memset(page_buf, 0x0, sizeof(page_buf));
	dat_buf = page_buf;
	oob_buf = page_buf + PAGE_SIZE_BMT;
#ifdef MTK_NAND_BMT_DEBUG
	printf("total_block_count = %d \n", total_block_count);
	printf("system_block_count = %d \n", system_block_count);
	printf("PAGE_SIZE_BMT = %d \n", PAGE_SIZE_BMT);
	printf("BLOCK_SIZE_BMT = %d \n", BLOCK_SIZE_BMT);
	printf("dat_buf=0x%x, oob_buf=0x%x  \n", dat_buf, oob_buf);
#endif

    // load bmt if exist
    if ((bmt_block_index = load_bmt_data(system_block_count, size)))
    {
        printf( "Load bmt data success @ block 0x%x\n", bmt_block_index);
        dump_bmt_info(&bmt);
        return &bmt;
    }
    else
    {
        printf(  "Load bmt data fail, need re-construct!\n");
//#ifndef __UBOOT_NAND__            // BMT is not re-constructed in UBOOT.
        if (reconstruct_bmt(&bmt))
        {
#ifdef MTK_NAND_BMT_DEBUG
        	printf("reconstruct_bmt success!\n");
#endif
            return &bmt;
        }
        else
        {
#ifdef MTK_NAND_BMT_DEBUG
        	printf("reconstruct_bmt fail!\n");
#endif
        
//#endif
            return NULL;
        }
    }
}


/*******************************************************************
* [BMT Interface]
*
* Description:
*   Update BMT.
*
* Parameter:
*   offset: update block/page offset.
*   reason: update reason, see update_reason_t for reason.
*   dat/oob: data and oob buffer for write fail.
* 
* Return: 
*   Return true for success, and false for failure.
*******************************************************************/
bool update_bmt(u32 offset, update_reason_t reason, u8 *dat, u8 *oob)
{
    int map_index;
    int orig_bad_block = -1;
    // int bmt_update_index;
    int i;
    int bad_index = offset / BLOCK_SIZE_BMT;

#ifndef MTK_NAND_BMT
	return false;
#endif
    if (reason == UPDATE_WRITE_FAIL)
    {
        printf( "Write fail, need to migrate\n");
        if ( !(map_index = migrate_from_bad(offset, dat, oob)) )
        {
            printf( "migrate fail\n");
            return false;
        }
    }
    else
    {
        if ( !(map_index = find_available_block(false)) )
        {
            printf(  "Cannot find block in pool\n");
            return false;
        }
    }

    // now let's update BMT
    if (bad_index >= system_block_count)     // mapped block become bad, find original bad block
    {
        for (i = 0; i < bmt_block_count; i++)
        {
            if (bmt.table[i].mapped_index == bad_index)
            {
                orig_bad_block = bmt.table[i].bad_index;
                break;
            }
        }
        // bmt.bad_count++;
        printf( "Mapped block becomes bad, orig bad block is 0x%x\n", orig_bad_block);

        bmt.table[i].mapped_index = map_index;
    }
    else
    {
        bmt.table[bmt.mapped_count].mapped_index = map_index;
        bmt.table[bmt.mapped_count].bad_index = bad_index;
        bmt.mapped_count++;
    }

    memset(oob_buf, 0xFF, sizeof(oob_buf));
    fill_nand_bmt_buffer(&bmt, dat_buf, oob_buf);
    if (!write_bmt_to_flash(dat_buf, oob_buf))
        return false;

    mark_block_bad_bmt(offset);

    return true;
}

/*******************************************************************
* [BMT Interface]
*
* Description:
*   Given an block index, return mapped index if it's mapped, else 
*   return given index.
*
* Parameter:
*   index: given an block index. This value cannot exceed 
*   system_block_count.
*
* Return NULL for failure
*******************************************************************/
u16 get_mapping_block_index(int index)
{
    int i;
#ifndef MTK_NAND_BMT
	return index;
#endif
    if (index > system_block_count)
    {
        return index;
    }

    for (i = 0; i < bmt.mapped_count; i++)
    {
        if (bmt.table[i].bad_index == index)
        {
            return bmt.table[i].mapped_index;
        }
    }

    return index;
}

int update_bmt_page(int *page, u8 *oob)
{
    int block;
    u16 page_in_block;
    int mapped_block;

	block = (*page) >> CONFIG_NUMPAGE_PER_BLOCK_BIT;
	page_in_block = (*page) & ((1 << CONFIG_NUMPAGE_PER_BLOCK_BIT) - 1);
	mapped_block = get_mapping_block_index(block);

	if (block < system_block_count)
		set_bad_index_to_oob(oob, block);
	if (mapped_block != block)
		*page = page_in_block + (mapped_block << CONFIG_NUMPAGE_PER_BLOCK_BIT);
	return 0;
}

#ifdef __KERNEL_NAND__
EXPORT_SYMBOL(init_bmt);
EXPORT_SYMBOL(update_bmt);
//EXPORT_SYMBOL(reconstruct_bmt);
EXPORT_SYMBOL(get_mapping_block_index);
EXPORT_SYMBOL(update_bmt_page);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fei Jiang @ MediaTek");
MODULE_DESCRIPTION("Block mapping management for MediaTek NAND Flash Driver");
#endif
