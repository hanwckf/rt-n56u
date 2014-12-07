/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

//#define MMC_ICE_DOWNLOAD
//#define MMC_BOOT_TEST
/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/
#include <common.h>
#include "msdc.h"
#include "msg.h"
#include "mmc_core.h"
#include "mmc_test.h"
#include "utils.h"

#define MMC_BUF_ADDR       (0x01000000)
#define BLK_SIZE           (512)

#if defined(MMC_TEST)

#define TC_MSG              "[SD%d] <%s> TC%d: %s"

#define MMC_TST_CHK_RESULT   (1)
#define MMC_TST_SBLK_RW      (1)
#define MMC_TST_MBLK_RW      (1)
#define MMC_TST_IMBLK_RW     (1)
#define MMC_TST_COUNTS       (1)

#define MMC_TST_BUF_ADDR     (0x01000000)
#ifdef MMC_PROFILING
#define MMC_TST_SIZE         (4 * 1024 * 1024)  /* 4MB  */
#define MMC_TST_CHUNK_BLKS   (128)              /* 64KB */
#else
#define MMC_TST_SIZE         (1 * 1024 * 1024)  /* 1MB */
#define MMC_TST_CHUNK_BLKS   (16)               /* 8KB */
#endif
#define MMC_TST_START_ADDR   (128 * 1024 * 1024) /* 128MB */
#define MMC_TST_START_BLK    (MMC_TST_START_ADDR / MMC_BLOCK_SIZE)
#define MMC_TST_BLK_NR(x)    (MMC_TST_START_BLK+(x)*(MMC_TST_SIZE/MMC_BLOCK_SIZE))

#define ARRAY_SIZE(x)        (sizeof(x) / sizeof((x)[0]))

static unsigned int clkfreq[]  = { 50000000 };
static unsigned int buswidth[] = { HOST_BUS_WIDTH_4 };

#ifdef MMC_PROFILING
static struct mmc_op_perf mmc_perf[MSDC_MAX_NUM];

struct mmc_op_perf *mmc_prof_handle(int id)
{
    return &mmc_perf[id];
}

void mmc_prof_init(int id, struct mmc_host *host, struct mmc_card *card)
{
    memset(&mmc_perf[id], 0, sizeof(struct mmc_op_perf));
    mmc_perf[id].host = host;
    mmc_perf[id].card = card;
    msdc_timer_init();
    msdc_timer_stop_clear();
}

void mmc_prof_start(void)
{
    msdc_timer_stop_clear();
    msdc_timer_start();
}

void mmc_prof_stop(void)
{
    msdc_timer_stop();
}

unsigned int mmc_prof_count(void)
{
    return msdc_timer_get_count();
}

void mmc_prof_update(mmc_prof_callback cb, ulong id, void *data)
{
    ulong counts = (ulong)msdc_timer_get_count();
    if (cb) {
        cb(data, id, counts);
    }
}

void mmc_prof_report(struct mmc_op_report *rpt)
{
    printf("\t\tCount      : %d\n", rpt->count);
    printf("\t\tMax. Time  : %d counts\n", rpt->max_time);
    printf("\t\tMin. Time  : %d counts\n", rpt->min_time);
    printf("\t\tTotal Size : %d KB\n", rpt->total_size / 1024);
    printf("\t\tTotal Time : %d counts\n", rpt->total_time);
    if (rpt->total_time) {
        printf("\t\tPerformance: %d KB/sec\n", 
            ((rpt->total_size / 1024) * 32768) / rpt->total_time);
    }
}

int mmc_prof_dump(int dev_id)
{
    struct mmc_host *host;
    struct mmc_card *card;
    struct mmc_op_perf *perf;
    u32 total_read_size, total_write_size;
    u32 total_read_time, total_write_time;
      
    perf = &mmc_perf[dev_id];
    host = mmc_perf[dev_id].host;
    card = mmc_perf[dev_id].card;

    total_read_size = total_write_size = 0;
    total_read_time = total_write_time = 0;

    printf("\tSD Host ID     : %d\n", dev_id);
    printf("\tOP Clock Freq. : %d khz\n", host->clk / 1000);    
    printf("\tSD Clock Freq. : %d khz\n", host->sclk / 1000);
    printf("\tCard Type      : %s card\n", (card->type == MMC_TYPE_MMC) ? "MMC" : "SD/SDHC/SDXC");
    printf("\tCard Mode      : UHS1(%d) DDR(%d) HS(%d)\n", 
        mmc_card_uhs1(card) ? 1 : 0, mmc_card_ddr(card) ? 1 : 0, 
        mmc_card_highspeed(card) ? 1 : 0);
    printf("\tCard Size      : %d MB\n", (card->nblks * card->blklen) / 1024 / 1024);
    printf("\tCard Max. Freq.: %d khz\n", card->maxhz / 1000);
    
    if (perf->multi_blks_read.count) {
        printf("\tMulti-Blks-Read:\n");
        mmc_prof_report(&perf->multi_blks_read);
        total_read_size += perf->multi_blks_read.total_size;
        total_read_time += perf->multi_blks_read.total_time;
    }
    if (perf->multi_blks_write.count) {
        printf("\tMulti-Blks-Write:\n");
        mmc_prof_report(&perf->multi_blks_write);
        total_write_size += perf->multi_blks_write.total_size;
        total_write_time += perf->multi_blks_write.total_time;
    }
    if (perf->single_blk_read.count) {
        printf("\tSingle-Blk-Read:\n");
        mmc_prof_report(&perf->single_blk_read);
        total_read_size += perf->single_blk_read.total_size;
        total_read_time += perf->single_blk_read.total_time;
    }
    if (perf->single_blk_write.count) {
        printf("\tSingle-Blk-Write:\n");
        mmc_prof_report(&perf->single_blk_write);
        total_write_size += perf->single_blk_write.total_size;
        total_write_time += perf->single_blk_write.total_time;
    }
    if (total_read_time) {
        printf("\tPerformance Read : %d KB/sec\n",
            ((total_read_size / 1024) * 32768) / total_read_time);
    }
    if (total_write_time) {
        printf("\tPerformance Write: %d KB/sec\n",
            ((total_write_size / 1024) * 32768) / total_write_time);
    }

    return 0;
}
#endif

#ifdef MMC_ICE_DOWNLOAD
volatile u32 mmc_download_addr;
volatile u32 mmc_download_size;
volatile u32 mmc_image_addr;

int mmc_download(int dev_id, u32 imgaddr, u32 size, u32 addr, int bootarea)
{
    int ret;
    int i, j, result = 0;
    u8 val;
    u8 *ext_csd;    
    uchar *buf, *chkbuf;
    u32 chunks, chunk_blks = 128, left_blks, blknr;
    u32 total_blks;
    struct mmc_card *card;

    if (!size)
        return 0;

    if (addr % MMC_BLOCK_SIZE)
        return MMC_ERR_FAILED;

    card    = mmc_get_card(dev_id);
    ext_csd = &card->raw_ext_csd[0];    

    if (bootarea && !mmc_card_sd(card) && card->ext_csd.part_en) {
        /* configure to specified partition */
        val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_BOOT_PART_1;
        if (mmc_set_part_config(card, val) != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }
    }

    blknr      = addr / MMC_BLOCK_SIZE;
    total_blks = (size + MMC_BLOCK_SIZE - 1) / MMC_BLOCK_SIZE;

    /* multiple block write */
    chunks    = total_blks / chunk_blks;
    left_blks = total_blks % chunk_blks;  
    buf       = (uchar*)imgaddr;
    chkbuf    = (uchar*)MMC_BUF_ADDR;

    for (i = 0; i < chunks; i++) {
        ret = mmc_block_write(dev_id, blknr + i * chunk_blks, 
            chunk_blks, (unsigned long*)buf);
        if (ret != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }
        ret = mmc_block_read(dev_id, blknr + i * chunk_blks,
            chunk_blks, (unsigned long*)chkbuf);
        if (ret != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }

        for (j = 0; j < chunk_blks * MMC_BLOCK_SIZE; j++) {
            if (buf[j] == chkbuf[j])
                continue;
            result = -__LINE__;
            goto done;
        }
        printf("[SD%d] Write %3d blocks from 0x%.8x(RAM) to 0x%.8x(FLASH).\n",
            dev_id, chunk_blks, (unsigned int)buf, 
            (blknr + i * chunk_blks) * MMC_BLOCK_SIZE);

        buf += (chunk_blks * MMC_BLOCK_SIZE);
    }
    
    if (left_blks) {
        ret = mmc_block_write(dev_id, blknr + chunks * chunk_blks, 
            left_blks, (unsigned long*)buf);
        if (ret != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }
        ret = mmc_block_read(dev_id, blknr + chunks * chunk_blks,
            left_blks, (unsigned long*)chkbuf);
        if (ret != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }
        for (j = 0; j < left_blks * MMC_BLOCK_SIZE; j++) {
            if (buf[j] == chkbuf[j])
                continue;
            printf("[SD%d] chkbuf[%d] = %xh (!= %xh) \n", dev_id,
                j, chkbuf[j], buf[j]);
            result = -__LINE__;
            goto done;
        }
        printf("[SD%d] Write %3d blocks from 0x%.8x(RAM) to 0x%.8x(FLASH).\n",
            dev_id, left_blks, (unsigned int)buf, 
            (blknr + chunks * chunk_blks) * MMC_BLOCK_SIZE);
    }

done:
    if (bootarea && !mmc_card_sd(card) && card->ext_csd.part_en) {
        /* configure to user partition */
        val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_DEFT_PART;
        if (mmc_set_part_config(card, val) != MMC_ERR_NONE)
            result = -__LINE__;
    }

    if (!result) {
        printf("[SD%d] Download %d blocks (%d bytes) to 0x%.8x successfully\n", 
            dev_id, total_blks, total_blks * MMC_BLOCK_SIZE, blknr * MMC_BLOCK_SIZE);
    } else {
        printf("[SD%d] Download %d blocks (%d bytes) to 0x%.8x failed %d\n", 
            dev_id, total_blks, total_blks * MMC_BLOCK_SIZE, blknr * MMC_BLOCK_SIZE, result);
    }    
    return result;
}

int mmc_download_part(int dev_id, char *part_name, int bootarea)
{
    int ret = -1;
    struct mmc_card *card;
    struct mmc_host *host;
    part_t *part = mt6573_part_get_partition(part_name);

    mmc_download_addr = 0;
    mmc_download_size = 0;
    mmc_image_addr = 0;

    host = mmc_get_host(dev_id);
    card = mmc_get_card(dev_id);

    if (part) {
        printf("[SD%d] Waiting for '%s' image loading from ICE...\n", dev_id, part_name);
        while (!mmc_download_size); /* Wait for loading image from ICE */
        ret = mmc_download(dev_id, mmc_image_addr, mmc_download_size, 
            part->startblk * BLK_SIZE, bootarea);
        if (ret != 0)
            goto done;
        if (bootarea) {
            /* set reset signal function */
            //ret = mmc_set_reset_func(card, 1);
            //if (ret != 0)
            //    goto done;
            /* set boot config */
            ret = mmc_boot_config(card, EXT_CSD_PART_CFG_EN_ACK,
                EXT_CSD_PART_CFG_EN_BOOT_PART_1, EXT_CSD_BOOT_BUS_WIDTH_1, 
                EXT_CSD_BOOT_BUS_MODE_DEFT);
            if (ret != 0)
                goto done;
            ret = mmc_read_ext_csd(host, card);
        }
    }
done:
    return ret;
}
#endif

int mmc_readback_blks(int dev_id, unsigned long addr, int blks, int bootarea)
{
    int i, j, result = 0;
    u8 val;
    u8 *ext_csd;
    unsigned long blknr = addr / MMC_BLOCK_SIZE;
    unsigned char *buf = (unsigned char*)MMC_BUF_ADDR;
    struct mmc_card *card;
    struct mmc_host *host;

    host = mmc_get_host(dev_id);
    card = mmc_get_card(dev_id);
    ext_csd = &card->raw_ext_csd[0];

    if (bootarea && !mmc_card_sd(card)) {
        /* configure to specified partition */
        val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_BOOT_PART_1;
        if (mmc_set_part_config(card, val) != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }
        if (mmc_read_ext_csd(host, card) != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }
    }

    printf("[SD%d] Dump %d blks from 0x%x (FLASH)\n", dev_id, blks, 
        blknr * MMC_BLOCK_SIZE);
    for (i = 0; i < blks; i++) {
        memset(buf, 0, MMC_BLOCK_SIZE);
        if (MMC_ERR_NONE != mmc_block_read(dev_id, blknr + i, 1, (unsigned long*)buf)) {
            printf("\n[SD%d] Read from %dth block error\n", dev_id, blknr + i);
            break;
        }
        
        for (j = 0; j < MMC_BLOCK_SIZE; j++) {
            if (j % 16 == 0)
                printf("\n%xh: ", (blknr + i) * MMC_BLOCK_SIZE + j);
            printf("%x ",  buf[j]);
        }
        printf("\n");
        buf += MMC_BLOCK_SIZE;
    }
done:

    if (bootarea && !mmc_card_sd(card)) {
        /* configure to user partition */
        val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_DEFT_PART;
        if (mmc_set_part_config(card, val) != MMC_ERR_NONE)
            result = -__LINE__;
        if (mmc_read_ext_csd(host, card) != MMC_ERR_NONE) {
            result = -__LINE__;
        }
    }  

    return result;
}

#if 0
int mmc_readback_part(int dev_id, char *part_name, int bootarea)
{
    int ret = -1;
    part_t *part = mt6573_part_get_partition(part_name);

    if (part) {
        ret = mmc_readback_blks(dev_id, part->startblk * BLK_SIZE, 
            1, bootarea);
    }
    return ret;
}
#endif

int mmc_erase_blks(int dev_id, u32 addr, u32 size, int bootarea)
{
    unsigned long ret;
    int i, j, result = 0;
    u8 val;
    u8 *ext_csd;    
    u32 blknr;
    u32 total_blks;
    struct mmc_card *card;

    if (!size)
        return 0;

    if (addr % MMC_BLOCK_SIZE)
        return MMC_ERR_FAILED;

    card    = mmc_get_card(dev_id);
    ext_csd = &card->raw_ext_csd[0];    

    if (bootarea && !mmc_card_sd(card) && card->ext_csd.part_en) {
        /* configure to specified partition */
        val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_BOOT_PART_1;
        if (mmc_set_part_config(card, val) != MMC_ERR_NONE) {
            result = -__LINE__;
            goto done;
        }
    }
    blknr      = addr / MMC_BLOCK_SIZE;
    total_blks = (size + MMC_BLOCK_SIZE - 1) / MMC_BLOCK_SIZE;

    if (mmc_erase_start(card, blknr * MMC_BLOCK_SIZE) != MMC_ERR_NONE) {
        result = -__LINE__;
        goto done;
    }
    if (mmc_erase_end(card, (blknr + total_blks) * MMC_BLOCK_SIZE) != MMC_ERR_NONE) {
        result = -__LINE__;
        goto done;
    }
    if (mmc_erase(card, MMC_ERASE_NORMAL) != MMC_ERR_NONE) {
        result = -__LINE__;
        goto done;        
    }

done:
    if (bootarea && !mmc_card_sd(card) && card->ext_csd.part_en) {
        /* configure to user partition */
        val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_DEFT_PART;
        if (mmc_set_part_config(card, val) != MMC_ERR_NONE)
            result = -__LINE__;
    }

    if (!result) {
        printf("[SD%d] Erase %d blocks (%d bytes) from 0x%x successfully\n", 
            dev_id, total_blks, total_blks * MMC_BLOCK_SIZE, blknr * MMC_BLOCK_SIZE);
    } else {
        printf("[SD%d] Erase %d blocks (%d bytes) from 0x%x failed %d\n", 
            dev_id, total_blks, total_blks * MMC_BLOCK_SIZE, blknr * MMC_BLOCK_SIZE, result);
    }    
    return result;
}

#if 0
int mmc_erase_part(int dev_id, char *part_name, int bootarea)
{
    int ret = -1;
    part_t *part = mt6573_part_get_partition(part_name);

    if (part) {
        /* Notice that the block size is different with different emmc.
         * Thus, it could overwrite other partitions while erasing data.
         */
        ret = mmc_erase_blks(dev_id, part->startblk * BLK_SIZE, 
            part->blknum * BLK_SIZE, bootarea);
    }
    return ret;    
}

#endif

#ifdef MMC_BOOT_TEST
int mmc_boot_up_test(int id, int reset)
{
    int err = MMC_ERR_FAILED;
    struct mmc_host *host;

    host = mmc_get_host(id);
    mmc_init_host(host, id);

    msdc_emmc_boot_reset(host, reset);
    
    err = msdc_emmc_boot_start(host, 25000000, 0, EMMC_BOOT_RST_CMD_MODE, 0);
    if (err) {
        printf("[EMMC] Boot Error: %d\n", err);
        goto done;
    }
    err = msdc_emmc_boot_read(host, 128 * 1024, MMC_BUF_ADDR);
    msdc_emmc_boot_stop(host, EMMC_BOOT_RST_CMD_MODE);
done:
    if (!err) {
        int i, j;
        char *buf = (char*)MMC_BUF_ADDR;
        for (i = 0; i < 16; i++) {            
            for (j = 0; j < MMC_BLOCK_SIZE; j++) {
                if (j % 16 == 0)
                    printf("\n%.8xh: ", i * MMC_BLOCK_SIZE + j);
                printf("%.2x ",  buf[j]);
            }
            printf("\n");
            buf += MMC_BLOCK_SIZE;
        }
    }
    return err;
}

int mmc_boot_enable(int id, int bootpart)
{
    int err = MMC_ERR_FAILED;
    struct mmc_host *host;
    struct mmc_card *card;

    host = mmc_get_host(id);
    card = mmc_get_card(id);

    err = mmc_boot_config(card, EXT_CSD_PART_CFG_EN_ACK,
        bootpart, EXT_CSD_BOOT_BUS_WIDTH_1, EXT_CSD_BOOT_BUS_MODE_DEFT);
    if (err != 0)
        goto done;
    err = mmc_read_ext_csd(host, card);
done:
    return err;
}
#endif

int mmc_test_mem_card(struct mmc_test_config *cfg)
{
    int id, count, forever;
    int ret, chk_result, tid = 0, result = 0;
    unsigned int chunks, chunk_blks, left_blks, pass = 0, fail = 0;
    unsigned int total_blks;
    unsigned int i, j;
    unsigned int blksz;
    unsigned int clkhz;
    unsigned int status;
    char pattern = 0;
    char *buf;
    unsigned long blknr;
    struct mmc_host *host;
    struct mmc_card *card;

    id    = cfg->id;
    count = cfg->count;
    buf   = cfg->buf;
    blknr = cfg->blknr;
    blksz = cfg->blksz;

    chk_result = cfg->chk_result;
    chunk_blks = cfg->chunk_blks;
    total_blks = (cfg->total_size + blksz - 1) / blksz;
    forever    = (count == -1) ? 1 : 0;

    host = mmc_get_host(id);
    card = mmc_get_card(id);

    while (forever || count--) {
        printf("[TST] ==============================================\n");
        printf("[TST] BEGIN: %d/%d, No Stop(%d)\n", 
            (cfg->count != -1) ? cfg->count - count : 0, 
            (cfg->count != -1) ? cfg->count : 0, forever);
        printf("[TST] ----------------------------------------------\n");
        printf("[TST] Mode    : %d\n", cfg->mode);
        printf("[TST] Clock   : %d kHz\n", cfg->clock / 1000);
        printf("[TST] BusWidth: %d bits\n", cfg->buswidth);
        printf("[TST] BurstSz : %d bytes\n", 0x1 << cfg->burstsz);
        printf("[TST] BlkAddr : %xh\n", blknr);
        printf("[TST] BlkSize : %dbytes\n", blksz);
        printf("[TST] TstBlks : %d\n", total_blks);
        printf("[TST] AutoCMD : 12(%d), 23(%d)\n", 
            (cfg->autocmd & MSDC_AUTOCMD12) ? 1 : 0, 
            (cfg->autocmd & MSDC_AUTOCMD23) ? 1 : 0);
        printf("[TST] ----------------------------------------------\n");


        if (mmc_init_host(id, host, cfg->clksrc, cfg->mode) != 0) {
            result = -__LINE__;
            goto failure;
        }
        if (mmc_init_card(host, card) != 0) {
            result = -__LINE__;
            goto failure;
        }

        msdc_set_dma(host, (u8)cfg->burstsz, (u32)cfg->flags);
        msdc_set_autocmd(host, cfg->autocmd, 1);

        /* change uhs-1 mode */
#if 0        
        if (mmc_card_uhs1(card)) {
            if (mmc_switch_uhs1(host, card, cfg->uhsmode) != 0) {
                result = -__LINE__;
                goto failure;
            }
        }
#endif

        /* change clock */
        if (cfg->clock) {
            clkhz = card->maxhz < cfg->clock ? card->maxhz : cfg->clock;
            mmc_set_clock(host, mmc_card_ddr(card), clkhz); 
        }
        if (mmc_card_sd(card) && cfg->buswidth == HOST_BUS_WIDTH_8) {
            printf("[TST] SD card doesn't support 8-bit bus width (SKIP)\n");
            result = MMC_ERR_NONE;
        }
        if (mmc_set_bus_width(host, card, cfg->buswidth) != 0) {
            result = -__LINE__;
            goto failure;
        }

        /* cmd16 is illegal while card is in ddr mode */
        if (!(mmc_card_mmc(card) && mmc_card_ddr(card))) {
            if (mmc_set_blk_length(host, blksz) != 0) {
                result = -__LINE__;
                goto failure;            
            }
        }
        
        if (cfg->piobits) {
            printf("[TST] PIO bits: %d\n", cfg->piobits);
            msdc_set_pio_bits(host, cfg->piobits);
        }

        tid = result = 0;        

        if (mmc_erase_start(card, blknr * blksz) != MMC_ERR_NONE) {
            result = -__LINE__;
            goto failure;
        }
        if (mmc_erase_end(card, (blknr + total_blks) * blksz) != MMC_ERR_NONE) {
            result = -__LINE__;
            goto failure;
        }
        if (mmc_erase(card, MMC_ERASE_NORMAL) != MMC_ERR_NONE) {
            result = -__LINE__;
            goto failure;        
        }
        printf("[TST] 0x%x - 0x%x Erased\n", blknr * blksz, 
            (blknr + total_blks) * blksz);

        mmc_send_status(host, card, &status);

        if (cfg->tst_single) {
            /* single block write */
            for (i = 0; i < total_blks; i++) {
                pattern = (i + count) % 256;
                memset(buf, pattern, blksz);
                ret = mmc_block_write(id, blknr + i, 1, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    printf("test single block write failed (%d)\n", i);
                    result = -__LINE__;
                    goto failure;
                }
            }

            printf(TC_MSG, host->id, result == 0 ? "PASS" : "FAIL", tid++, 
                "test single block write\n");

            if (result)
                break;
            
            /* single block read */
            for (i = 0; i < total_blks && !result; i++) {
                pattern = (i + count) % 256;
                /* populate buffer with different pattern */
                memset(buf, pattern + 1, blksz);
                ret = mmc_block_read(id, blknr + i, 1, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    result = -__LINE__;
                    goto failure;
                }
                if (chk_result) {
                    for (j = 0; j < blksz; j++) {
                        if (buf[j] != pattern) {
                            result = -__LINE__;
                            goto failure;
                        }
                    }
                }
            }
            printf(TC_MSG, host->id, result == 0 ? "PASS" : "FAIL", tid++, 
                "test single block read\n");

            if (result) {
                printf("[SD%d]\t\tread back pattern(0x%.2x) failed\n", 
                    id, pattern);
                goto failure;
            }
        }

        mmc_send_status(host, card, &status);
        
        if (cfg->tst_multiple) {
            /* multiple block write */
            chunks = total_blks / chunk_blks;
            left_blks = total_blks % chunk_blks;   
            for (i = 0; i < chunks; i++) {
                pattern = (i + count) % 256;
                memset(buf, pattern, blksz * chunk_blks);
                ret = mmc_block_write(id, blknr + i * chunk_blks, 
                    chunk_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    result = -__LINE__;
                    goto failure;
                }
            }
            
            if (!result && left_blks) {
                pattern = (i + count) % 256;
                memset(buf, pattern, blksz * left_blks);
                ret = mmc_block_write(id, blknr + chunks * chunk_blks, 
                    left_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    result = -__LINE__;
                    goto failure;
                }
            }

            printf(TC_MSG, host->id, result == 0 ? "PASS" : "FAIL", tid++, 
                "test multiple block write\n");

            if (result)
                goto failure;

            /* multiple block read */
            for (i = 0; i < chunks; i++) {
                pattern = (i + count) % 256;
                /* populate buffer with different pattern */
                memset(buf, pattern + 1, blksz);
                ret = mmc_block_read(id, blknr + i * chunk_blks, 
                    chunk_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    printf("[SD%d]\t\tread %d blks failed(ret = %d blks)\n",
                        host->id, chunk_blks, ret);
                    result = -__LINE__;
                    goto failure;
                }
                if (chk_result) {
                    for (j = 0; j < chunk_blks * blksz; j++) {
                        if (buf[j] == pattern)
                            continue;
                        result = -__LINE__;
                        printf("[SD%d]\t\t%xh = %x (!= %x)\n",
                            host->id, blknr + i * chunk_blks + j, buf[j], pattern);
                        goto failure;
                    }
                }
            }

            if (!result && left_blks) {
                pattern = i % 256;
                /* populate buffer with different pattern */
                memset(buf, pattern + 1, blksz);
                ret = mmc_block_read(id, blknr + chunks * chunk_blks, 
                    left_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    printf("[SD%d]\t\tread %d blks failed(ret = %d blks)\n",
                        host->id, left_blks, ret);
                    result = -__LINE__;
                    goto failure;
                }
                if (chk_result) {
                    for (j = 0; j < left_blks * blksz; j++) {
                        if (buf[j] == pattern)
                            continue;
                        printf("[SD%d]\t\t%xh = %x (!= %x)\n",
                            host->id, blknr + chunks * chunk_blks + j, buf[j], pattern);
                        result = -__LINE__;
                        goto failure;
                    }
                }
            }

            printf(TC_MSG, host->id, result == 0 ? "PASS" : "FAIL", tid++, 
                "test multiple block read\n");

            if (result)
                goto failure;
        }

        mmc_send_status(host, card, &status);

        if (cfg->tst_interleave) {
            /* multiple block write */
            chunks = total_blks / chunk_blks;
            left_blks = total_blks % chunk_blks;   
            for (i = 0; i < chunks; i++) {
                pattern = (i + count) % 256;
                memset(buf, pattern, blksz * chunk_blks);
                ret = mmc_block_write(id, blknr + i * chunk_blks, 
                    chunk_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    result = -__LINE__;
                    goto failure;
                }

                /* populate buffer with different pattern */
                memset(buf, pattern + 1, blksz * chunk_blks);
                ret = mmc_block_read(id, blknr + i * chunk_blks, 
                    chunk_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    result = -__LINE__;
                    goto failure;
                }
                if (chk_result) {
                    for (j = 0; j < chunk_blks * blksz; j++) {
                        if (buf[j] == pattern) 
                            continue;
                        result = -__LINE__;
                        goto failure;
                    }
                }                
            }           

            if (!result && left_blks) {
                pattern = (i + count) % 256;
                memset(buf, pattern, blksz * left_blks);
                ret = mmc_block_write(id, blknr + chunks * chunk_blks, 
                    left_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    result = -__LINE__;
                    goto failure;
                }

                /* populate buffer with different pattern */
                memset(buf, pattern + 1, blksz * left_blks);
                ret = mmc_block_read(id, blknr + chunks * chunk_blks, 
                    left_blks, (unsigned long*)buf);
                if (ret != MMC_ERR_NONE) {
                    result = -__LINE__;
                    break;
                }
                if (chk_result) {
                    for (j = 0; j < left_blks * blksz; j++) {
                        if (buf[j] == pattern)
                            continue;
                        result = -__LINE__;
                        goto failure;
                    }
                }
            }

            printf(TC_MSG, host->id, result == 0 ? "PASS" : "FAIL", tid++, 
                "test multiple block interleave write-read\n");

            if (result)
                goto failure;
        }
        if (cfg->desc) {
            printf("[TST] ----------------------------------------------\n");
            printf("[TST] Report - %s \n", cfg->desc);
            printf("[TST] ----------------------------------------------\n");
        }
        mmc_prof_dump(id);

failure:
        if (result) {
            printf("[SD%d] mmc test failed (%d)\n", host->id, result);
            fail++;
        } else {
            pass++;
        }
        printf("[TST] ----------------------------------------------\n");
        printf("[TST] Test Result: TOTAL(%d/%d), PASS(%d), FAIL(%d) \n", 
            cfg->count - count, cfg->count, pass, fail);
        printf("[TST] ----------------------------------------------\n");    
    	//mdelay(1000);
    }

    return result;
}

int mmc_test(int argc, char *argv[])
{
    int i, j;
    int result = -1;
    struct mmc_test_config cfg;

#ifdef MMC_ICE_DOWNLOAD
    mmc_readback_part(0, PART_UBOOT, 0);
    mmc_erase_part(0, PART_UBOOT, 0);
    mmc_readback_part(0, PART_UBOOT, 0);
    mmc_download_part(0, PART_UBOOT, 0);
    mmc_readback_part(0, PART_UBOOT, 0);
    while(1);
#endif

#ifdef MMC_BOOT_TEST
    printf("[EMMC] Boot up with power reset (MMCv4.3 above)\n");
    mmc_boot_up_test(0, EMMC_BOOT_PWR_RESET);
    mdelay(100);
	for (i = 0; i < 20; i++) {
        printf("[EMMC] Boot up with RST_n reset (MMCv4.41 above)\n");
        mmc_boot_up_test(0, EMMC_BOOT_RST_N_SIG);
        mdelay(100);
    }
    printf("[EMMC] Boot up with PRE_IDLE_CMD reset (MMCv4.41 above)\n");
    mmc_boot_up_test(0, EMMC_BOOT_PRE_IDLE_CMD);

    //mmc_boot_enable(0, EXT_CSD_PART_CFG_EN_NO_BOOT);
    //mmc_boot_enable(0, EXT_CSD_PART_CFG_EN_BOOT_PART_1);
#endif

    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = 0;
    cfg.desc = "Memory Card Read/Write Test";
    cfg.count = MMC_TST_COUNTS;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*)MMC_TST_BUF_ADDR;
    cfg.chk_result = MMC_TST_CHK_RESULT;
    cfg.tst_single = MMC_TST_SBLK_RW;
    cfg.tst_multiple = MMC_TST_MBLK_RW;
    cfg.tst_interleave = MMC_TST_IMBLK_RW;
    
    for (i = 0; i < ARRAY_SIZE(clkfreq); i++) {	
        for (j = 0; j < ARRAY_SIZE(buswidth); j++) {
            cfg.clock = clkfreq[i];
            cfg.buswidth = buswidth[j];
            if (mmc_test_mem_card(&cfg) != 0)
                goto exit;
        }
    }
    result = 0;
    //mmc_readback_blks(0, MMC_TST_BLK_NR(0) * MMC_BLOCK_SIZE, 16, 0);
	
exit:
    while(1);

    return result;
}

#endif /* MMC_TEST */

