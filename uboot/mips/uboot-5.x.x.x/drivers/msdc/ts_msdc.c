/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (c) 2010 MediaTek Inc.
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

//#include "CTP_type.h"
//#include "CTP_shell.h"
#include <common.h>
#include <command.h>
//#include "version.h"
//#include "pdn_sw.h"
//#include "pdn_hw.h"
//#include "gpt_sw.h"
//#include "sleep_ctrl.h"
#include "utils.h"
#include "mmc_core.h"
#include "mmc_test.h"
#include "msdc.h"
#include "msdc_cust.h"
#include <asm/addrspace.h>

#define TEST_HOST_ID         (0)

//#define MMC_TST_BUF_ADDR     (0x01000000)
//#define MSDC_INTSRAM
#if defined(MSDC_INTSRAM)
#define MMC_TST_SIZE         (2 * 1024)        /* 128KB */
#define MMC_TST_CHUNK_BLKS   (1)               /* 32KB  */
#else
#ifdef MMC_PROFILING
#define MMC_TST_SIZE         (4* 1024 * 1024)  /* 4MB */
#define MMC_TST_CHUNK_BLKS   (128)             /* 64KB */
#else
#define MMC_TST_SIZE         (128 * 1024)      /* 128KB */
#define MMC_TST_CHUNK_BLKS   (64)              /* 32KB  */
#endif

#endif
#define MMC_TST_START_ADDR   (128 * 1024 * 1024) /* 128MB */
#define MMC_TST_START_BLK    (MMC_TST_START_ADDR / MMC_BLOCK_SIZE)
#define MMC_TST_BLK_NR(x)    (MMC_TST_START_BLK+(x)*(MMC_TST_SIZE/MMC_BLOCK_SIZE))

/*******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/
//void TS_MSDC_ShowUsage(UINT32 u4TestCaseId);
void TS_MSDC_Switch(void);

void SDMMC_RegTest(void);
void SDMMC_AutoTest(void);
void SDMMC_CardInitTest(void);
void SDMMC_CardEraseTest(void);
void SDMMC_CardStreamTest(void);
void SDMMC_CardPIOTest(void);
void SDMMC_CardBasicDMATest(void);
void SDMMC_CardDescDMATest(void);
void SDMMC_CardEnhancedDMATest(void);
void SDMMC_BlockLenTest(void);
void SDMMC_DDRModeTest(void);
void SDMMC_AutoCmd12Test(void);
void SDMMC_AutoCmd19Test(void);
void SDMMC_AutoCmd23Test(void);
void SDMMC_CardDetectTest(void);
void SDMMC_MMCIRQTest(void);
void SDMMC_EmmcBootTest(void);
void SDMMC_StressTest(int);
void SDMMC_SuspendResume(void);
void SDMMC_TuningCmdTest(void);
void SDMMC_TuningWriteTest(void);
void SDMMC_TuningReadTest(void);
void SDIO_InterruptTest(void);
void EMMC_BootModeConfig(void);
void SD30_SDR104Test(void);

/*******************************************************************************
 * LOCAL CONST DEFINATION
 ******************************************************************************/
static int g_MSDC_id = TEST_HOST_ID;
//static unsigned int clkfreq[]  = { MSDC_MAX_SCLK / 8, MSDC_MAX_SCLK / 4, MSDC_MAX_SCLK / 2, MSDC_MAX_SCLK };
//static unsigned int clkfreq[]  = { MSDC_MAX_SCLK, MSDC_MAX_SCLK / 2};
static unsigned int clkfreq[]  = { MSDC_MAX_SCLK };
static unsigned int buswidth[] = { 
#if defined (EMMC_8BIT)
					HOST_BUS_WIDTH_8, 
#endif
					HOST_BUS_WIDTH_4, 
					HOST_BUS_WIDTH_1};
static unsigned int burstsz[] = { MSDC_BRUST_64B, MSDC_BRUST_32B, MSDC_BRUST_16B, MSDC_BRUST_8B };
#if defined(MSDC_INTSRAM)
__attribute__ ((unused, __section__ ("INTERNAL_SRAM"))) static unsigned char buf[MMC_TST_SIZE * 2];
//static unsigned char buf[MMC_TST_SIZE * 2];
#else
static unsigned char buf[MMC_TST_SIZE * 2];
#endif
#define MMC_TST_BUF_ADDR &buf[0]

/*******************************************************************************
 * FUNCTION DEFINATIONS
 ******************************************************************************/

#if 0
/*******************************************************************************
* FUNCTION
*   TS_MSDC_Init
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/
CTP_STATUS_T ts_msdc_Init(void **ppvInitStruct)
{
    CTP_TEST_SUITE_T sTestSuite;
    int msdc_ver;

    sTestSuite.pzCommand     = "MSDC";
    sTestSuite.pzDescription = "TS_MSDC";
    sTestSuite.psTestCase    = g_asTS_MSDC_TestCases;
    sTestSuite.pfUsageFunc   = TS_MSDC_ShowUsage;

    CTP_RegisterTestSuite(&sTestSuite);

    *ppvInitStruct = (void *)"TC_MSDC Init";

    return CTP_SUCCESS;
}

/*******************************************************************************
* FUNCTION
*   TS_MSDC_ShowUsage
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/
CTP_STATUS_T TS_MSDC_ShowUsage(UINT32 u4TestCaseId)
{
    switch (u4TestCaseId)
    {   
    case 1:
        printf("MSDC CMD1 [/A] [/B]");
        printf("    /A          Parameter A and its behave\r\n");
        printf("    /B          Parameter B and its behave\r\n");
        break;
    case 2:
        printf("MSDC CMD2 [/A] [/B]");
        printf("    /X          Parameter X and its behave\r\n");
        printf("    /Y          Parameter Y and its behave\r\n");
        break;
    default:
        printf("MSDC CMDX [/E] [/H]");
        printf("    /E          Parameter E and its behave\r\n");
        printf("    /H          Parameter H and its behave\r\n"); 
        break;
    }

    return CTP_SUCCESS;
}

CTP_STATUS_T TS_MSDC_Switch(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_PASS;

    CTP_AskUserDecision(0, -1, &g_MSDC_id, "Please input MSDC host id to be tested (0~3) : ");

    return CTP_SUCCESS;    
}
#endif

/*******************************************************************************
* FUNCTION
*   SDMMC_RegTest
*
* DESCRIPTION
*   MMC/SD/SDIO register test
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/
void  SDMMC_RegTest(void)
{
    int id = g_MSDC_id;
    char buf[512];

    if (msdc_reg_test(id) == 0) {
	    printf("Register Test pass!\n");
    }
}

#if 0
/*******************************************************************************
* FUNCTION
*   SDMMC_AutoTest
*
* DESCRIPTION
*   SD/MMC card auto test
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/
CTP_STATUS_T SDMMC_AutoTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int id = g_MSDC_id;

    if (psOutput == NULL)
        return CTP_FAIL;

    /* TODO */

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_PASS;
out:

    return CTP_SUCCESS;
}
#endif

/*******************************************************************************
* FUNCTION
*   SDMMC_BlockLenTest
*
* DESCRIPTION
*   SD/MMC card detect and remove test
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_BlockLenTest(void)
{
    int err = 0, ret;
    int id = g_MSDC_id;
    unsigned int i, j, m, k, f, w;
    unsigned int total_blks;    
    struct mmc_host *host;
    struct mmc_card *card;
    char *buf = (char *)MMC_TST_BUF_ADDR;
    /* FIXME.
    int mode[] = {MSDC_MODE_PIO, MSDC_MODE_DMA_BASIC, MSDC_MODE_DMA_DESC, 
        MSDC_MODE_DMA_ENHANCED};
    */
    int mode[] = {MSDC_MODE_PIO};
    char pattern = 0;
    unsigned int addr = 0;
    unsigned long blknr;

    /* all test cases */
    u32 blklen[] = {1, 2, 3, 4, 15, 16, 17, 18, 31, 32, 33, 63, 64, 65, 
        127, 128, 129, 255, 256, 257, 509, 510, 511, 512};

    #if 0    
    /* Pass in multiple read/write */
    //u32 blklen[] = {1, 2, 3, 4, 15, 16, 17, 18, 32, 64, 128, 256, 512};

    /* Pass in single read/write */
    //u32 blklen[] = {1, 2, 3, 4, 15, 16, 17, 18, 31, 32, 33, 64, 128, 256, 512};

    /* Fail in multiple read (data timeout) */
    //u32 blklen[] = {127, 129, 255, 257, 509, 510, 511};
    /* Fail in single read (data crc error) */
    //u32 blklen[] = {63, 65, 127, 129, 255, 257, 509, 510, 511};
    u32 blklen[] = {511};
    #endif        

    u8 result;
    u8 report[ARRAY_SIZE(blklen)][4];
    u32 blksz, clkhz;
    u32 status;
    u32 blknum = 10;
    u32 total_sz;


    for (m = 0; m < ARRAY_SIZE(mode); m++) {
        for (f = 0; f < ARRAY_SIZE(clkfreq); f++) { 
            for (w = 0; w < ARRAY_SIZE(buswidth); w++) {

                if (0 != mmc_init(id))
                    goto exit;
                
                host = mmc_get_host(id);
                card = mmc_get_card(id);
                
                if (card->csd.write_partial || card->csd.read_partial) {
                    printf("[SD%d] Partial_write:%d, Partial_read:%d\n", id, 
                        card->csd.write_partial, card->csd.read_partial);
                } else {
                    printf("[SD%d] Doesn't support partial_write and partial_read\n", id);
                }

                /* set freq */
                clkhz = card->maxhz < clkfreq[f] ? card->maxhz : clkfreq[f];
                mmc_set_clock(host, mmc_card_ddr(card), clkhz);

                /* set buswidth */
                if (mmc_set_bus_width(host, card, buswidth[w]) != 0) {
                    err = -__LINE__;
                    goto exit;
                }

                /* set mode */
                msdc_set_dmode(host, mode[m]);

                memset(report, 0, 4 * ARRAY_SIZE(blklen));

                for (i = 0; i < ARRAY_SIZE(blklen); i++) {
                    printf("[SD%d] %d bytes Block Size Test\n", id, blklen[i]);
                    //pattern = (blklen[i] + 1) % 256;
                    pattern = 0xAA;
                    err = 0;
                    blksz = card->csd.write_partial ? blklen[i] : 512;
                    ERR_EXIT(mmc_set_blk_length(host, blksz), err, MMC_ERR_NONE);
                    total_sz = blksz * blknum;
                    total_blks = (total_sz + blksz - 1) / blksz;
                    blknr = addr / blksz;

                    /* single block write */
                    for (j = 0; j < total_blks; j++) {
                        memset(buf, pattern, blksz);
                        ret = mmc_block_write(id, blknr + j, 1, (unsigned long*)buf);
                        if (ret != 1) {
                            err = -__LINE__;
                            break;
                        }
                    }
                    if (err == 0)
                        report[i][0] = 1; /* mark as pass */

                    /* Wait until card not in programming state with 1-bit mode 
                     * Need to check why need this before issue CMD16 (CHECKME)
                     */
                    do {
                        ERR_EXIT(mmc_send_status(host, card, &status), err, MMC_ERR_NONE);
                    } while (R1_CURRENT_STATE(status) == 7);

                    err = 0;
                    blksz = card->csd.read_partial ? blklen[i] : 512;
                    ERR_EXIT(mmc_set_blk_length(host, blksz), err, MMC_ERR_NONE);
                    total_sz = blksz * blknum;
                    total_blks = (total_sz + blksz - 1) / blksz;
                    blknr = addr / blksz;

                    /* single block read */
                    for (j = 0; j < total_blks; j++) {
                        memset(buf, 0, blksz);
                        ret = mmc_block_read(id, blknr + j, 1, (unsigned long*)buf);
                        if (ret != 1) {
                            err = -__LINE__;
                            break;
                        }
                        for (k = 0; k < blksz; k++) {
                            if (buf[k] != pattern) {
                                err = -__LINE__;
                                break;
                            }
                        }
                    }
                    if (err == 0)
                        report[i][1] = 1; /* mark as pass */
                    
                    err = 0;
                    blksz = card->csd.write_partial ? blklen[i] : 512;
                    ERR_EXIT(mmc_set_blk_length(host, blksz), err, MMC_ERR_NONE);
                    total_sz = blksz * blknum;
                    total_blks = (total_sz + blksz - 1) / blksz;
                    blknr = addr / blksz;
                    memset(buf, 0xCD, total_sz);

                    /* multiple block write */
                    ret = mmc_block_write(id, blknr, total_blks, (unsigned long*)buf);
                    if (ret != (int)total_blks) {
                        err = -__LINE__;
                    }
                    if (err == 0)
                        report[i][2] = 1; /* mark as pass */

                    /* Wait until card not in programming state with 1-bit mode 
                     * Need to check why need this before issue CMD16 (CHECKME)
                     */
                    do {
                        ERR_EXIT(mmc_send_status(host, card, &status), err, MMC_ERR_NONE);
                    } while (R1_CURRENT_STATE(status) == 7);

                    err = 0;
                    blksz = card->csd.read_partial ? blklen[i] : 512;
                    ERR_EXIT(mmc_set_blk_length(host, blksz), err, MMC_ERR_NONE);
                    total_sz = blksz * blknum;
                    total_blks = (total_sz + blksz - 1) / blksz;
                    blknr = addr / blksz;
                    memset(buf, 0, total_sz);

                    /* multiple block read */
                    ret = mmc_block_read(id, blknr, total_blks, (unsigned long*)buf);
                    if (ret != (int)total_blks) {
                        err = -__LINE__;
                    }
                    if (err == 0) {
                        for (j = 0; j < total_sz; j++) {
                            if (buf[j] == 0xCD)
                                continue;
                            err = -__LINE__;
                            break;
                        }
                    }
                    if (err == 0)
                        report[i][3] = 1; /* mark as pass */
                    
                    ERR_EXIT(mmc_send_status(host, card, &status), err, MMC_ERR_NONE);
                }
                for (i = 0; i < ARRAY_SIZE(blklen); i++) {
                    printf("[SD%d] SW=%d, SR=%d, MW=%d, MR=%d, Blksz=%d\n",
                        id, report[i][0], report[i][1], report[i][2], 
                        report[i][3], blklen[i]);
                }
            }
        }
    }
    printf("Block Length Test pass!\n");
exit:
    if (err) {
        printf("[SD%d] error = %d\n", id, err);
    }
}

/*******************************************************************************
* FUNCTION
*   SDMMC_CardInitTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_CardInitTest(void)
{
    int i;
    int id = g_MSDC_id;

    for (i = 0; i < 5; i++) {
	    if (0 != mmc_init(id)) {
		    printf("Card Init Test fail!\n");
		    goto exit;
	    }
    }
    printf("Card Init Test pass!\n");
exit:   
    return;
}

/*******************************************************************************
* FUNCTION
*   SDMMC_CardEraseTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_CardEraseTest(void)
{
    unsigned int i;
    int err;
    int id = g_MSDC_id;
    struct mmc_host *host;
    struct mmc_card *card;
    u32 status;
    u32 addr[] = { MMC_TST_BLK_NR(10) * MMC_BLOCK_SIZE,
                   MMC_TST_BLK_NR(20) * MMC_BLOCK_SIZE };
    u32 offset = 32768;

    if (0 != mmc_init(id))
        goto exit;

	host = mmc_get_host(id);
	card = mmc_get_card(id);

    for (i = 0; i < ARRAY_SIZE(addr); i++) {
        printf("[MSDC%d] Erase Addr: 0x%x - 0x%x\n", id, addr[i], addr[i] + offset);
        err = mmc_erase_start(card, addr[i]);
        if (err) goto exit;
        err = mmc_erase_end(card, addr[i] + offset);
        if (err) goto exit;
        printf("[MSDC%d] Erasing....\n", id);
        err = mmc_erase(card, MMC_ERASE_NORMAL);
        if (err) goto exit;
        do {
            err = mmc_send_status(host, card, &status);
            if (err) goto exit;
            if (R1_STATUS(status) != 0) goto exit;
        } while (R1_CURRENT_STATE(status) == 7);
        printf("[MSDC%d] Erasing....Done\n\n", id);
    }
    
    printf("Card Erase Test pass!\n");
exit:   
    return;
}

/*******************************************************************************
* FUNCTION
*   SDMMC_CadrDetection
*
* DESCRIPTION
*   SD/MMC card detect and remove test
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_CardDetectTest(void)
{
    int id = g_MSDC_id;
    int	tstcnt, cnt, avail, expect_avail, prot;
    struct mmc_host *host;
    struct mmc_card *card;
	#if MSDC_USE_IRQ
	u32 status;
	u32 wints = MSDC_INT_CDSC;
	#endif
	
    expect_avail = 1;
    tstcnt = cnt = 5;
    host = mmc_get_host(id);
    card = mmc_get_card(id);
    mmc_init_host(id, host, -1, MSDC_MODE_PIO);
	#if MSDC_USE_IRQ
	do {
		status = msdc_intr_wait(host,wints);
		if(status & MSDC_INT_CDSC){	
		printf("[MSDC%d] Card inserted or remove interrupt come.\n", host->id);
        avail = mmc_card_avail(host);
        if (avail) {
			prot = mmc_card_protected(host);
            if (mmc_init_card(host, card) == 0) {
                printf("[MSDC%d] Card inserted(WP=%d). Please remove it...[%d/%d]\n", 
                    id, prot, tstcnt - cnt + 1, tstcnt);
                cnt--;
                expect_avail = 0;
            } else {
                break;
            }
        } else if (!avail) {        
            prot = mmc_card_protected(host);
            printf("[MSDC%d] Card removed (WP=%d). Please insert it...[%d/%d]\n",
                id, prot, tstcnt - cnt + 1, tstcnt);
            expect_avail = 1;
            /* reset the host controller, include freq, bus width ... */
            msdc_remove_detected(host);
        }
	    }
	}while(cnt > 0);
	#else
    do {
        avail = mmc_card_avail(host);
        if (avail && expect_avail == avail) {
            prot = mmc_card_protected(host);
            if (mmc_init_card(host, card) == 0) {
                printf("[MSDC%d] Card inserted(WP=%d). Please remove it...[%d/%d]\n", 
                    id, prot, tstcnt - cnt + 1, tstcnt);
                cnt--;
                expect_avail = 0;
            } else {
                break;
            }
        } else if (!avail && expect_avail == avail) {        
            prot = mmc_card_protected(host);
            printf("[MSDC%d] Card removed (WP=%d). Please insert it...[%d/%d]\n",
                id, prot, tstcnt - cnt + 1, tstcnt);
            expect_avail = 1;
            /* reset the host controller, include freq, bus width ... */
            mmc_init_host(id, host, -1, MSDC_MODE_PIO);
        }
    } while(cnt > 0);
	#endif
	
    if (cnt == 0)
	    printf("Card Detect Test pass!\n");
end:	
    return;
}

#if 0
/*******************************************************************************
* FUNCTION
*   SDMMC_CardStreamTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
CTP_STATUS_T SDMMC_CardStreamTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int i;
    int id = g_MSDC_id;
    struct mmc_test_config cfg;    
    struct mmc_host *host;
    struct mmc_card *card;
	
    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = id;
    cfg.desc = " MMC Stream R/W Test (only 1-bit)";
    cfg.mode = MSDC_MODE_MMC_STREAM;
    cfg.uhsmode = MMC_SWITCH_MODE_SDR12;
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.flags = 0;
    cfg.count = 1;
    cfg.clksrc = -1;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*)MMC_TST_BUF_ADDR;
    cfg.chk_result = 1;
#ifndef MMC_PROFILING
    cfg.tst_single = 1;
    cfg.tst_interleave = 1;
#endif
    cfg.tst_multiple = 1;

    /* stream read/write only availabe in 1-bit and non-DDR mode.
     * also note that some cards doesn't support stream read/write commands
     * (command class 1,3)
     */
    for (i = 0; i < 1; i++) { 
        cfg.clock = MSDC_MIN_SCLK;
        cfg.buswidth = HOST_BUS_WIDTH_1;
        if (mmc_test_mem_card(&cfg) != 0)
            goto exit;
	}

	psOutput->eResult = CTP_RESULT_PASS;
exit:	
    return CTP_SUCCESS;
}
#endif

/*******************************************************************************
* FUNCTION
*   SDMMC_CardPIOTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_CardPIOTest(void)
{
    unsigned int i, j, k;
    int id = g_MSDC_id;
#ifdef MMC_PROFILING
    int piobits[] = {32};
#else
    int piobits[] = {32, 16, 8};
#endif    
    struct mmc_test_config cfg;    
    struct mmc_host *host;
    struct mmc_card *card;
	
    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = id;
    cfg.desc = " PIO R/W Test";
    cfg.mode = MSDC_MODE_PIO;
    cfg.uhsmode = MMC_SWITCH_MODE_SDR25;
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.flags = 0;
    cfg.count = 1;
    cfg.clksrc = -1;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*)MMC_TST_BUF_ADDR;
    cfg.chk_result = 1;
#ifndef MMC_PROFILING
    cfg.tst_single = 1;
    cfg.tst_interleave = 1;
#endif
    cfg.tst_multiple = 1;

    
    for (i = 0; i < ARRAY_SIZE(clkfreq); i++) {	
        for (j = 0; j < ARRAY_SIZE(buswidth); j++) {
            for (k = 0; k < ARRAY_SIZE(piobits); k++) {
                cfg.clock = clkfreq[i];
                cfg.buswidth = buswidth[j];
                cfg.piobits = piobits[k];
                if (mmc_test_mem_card(&cfg) != 0)
                    goto exit;
            }
        }
    }

    printf("PIO Test pass!\n");
exit:	
    return;
}

/*******************************************************************************
* FUNCTION
*   SDMMC_CardBasicDMATest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_CardBasicDMATest(void)
{
    unsigned int i, j, k;
    int id = g_MSDC_id;
    struct mmc_test_config cfg;
    struct mmc_host *host;
    struct mmc_card *card;
    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = id;
    cfg.desc = " Basic DMA R/W Test";
    cfg.mode = MSDC_MODE_DMA_BASIC;
    cfg.uhsmode = MMC_SWITCH_MODE_SDR12;
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.flags = 0;
    cfg.count = 1;
    cfg.clksrc = -1;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*) PHYSADDR(MMC_TST_BUF_ADDR);
    cfg.chk_result = 1;
#ifndef MMC_PROFILING
    cfg.tst_single = 1;
    cfg.tst_interleave = 1;
#endif
    cfg.tst_multiple = 1;
    
    for (i = 0; i < ARRAY_SIZE(clkfreq); i++) {	
        for (j = 0; j < ARRAY_SIZE(buswidth); j++) {
            for (k = 0; k < ARRAY_SIZE(burstsz); k++) {
                cfg.clock = clkfreq[i];
                cfg.buswidth = buswidth[j];
                cfg.burstsz = burstsz[k];
                if (mmc_test_mem_card(&cfg) != 0)
                    goto exit;
            }
        }
    }

    printf("Basic DMA Test pass!\n");
exit:	
    return;
}

/*******************************************************************************
* FUNCTION
*   SDMMC_CardDescDMATest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_CardDescDMATest(void)
{
    unsigned int i, j, k;
    int id = g_MSDC_id;
    struct mmc_test_config cfg;    
    struct mmc_host *host;
    struct mmc_card *card;

#if defined(MSDC_INTSRAM)
    unsigned int iVal = *((volatile unsigned int *)0xC1000200);
    *((volatile unsigned int *)0xC1000200) = iVal | (1 << 28) | (1 << 23);
#endif


    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = id;
    cfg.desc = " Desc. DMA R/W Test";
    cfg.mode = MSDC_MODE_DMA_DESC;
    cfg.uhsmode = MMC_SWITCH_MODE_SDR12;
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.flags = DMA_FLAG_EN_CHKSUM; /* enable checksum */
    cfg.count = 1;
    cfg.clksrc = -1;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*) PHYSADDR(MMC_TST_BUF_ADDR);
    cfg.chk_result = 1;
#ifndef MMC_PROFILING
    cfg.tst_single = 1;
    cfg.tst_interleave = 1;
#endif
    cfg.tst_multiple = 1;
    
    for (i = 0; i < ARRAY_SIZE(clkfreq); i++) {	
        for (j = 0; j < ARRAY_SIZE(buswidth); j++) {
            for (k = 0; k < ARRAY_SIZE(burstsz); k++) {
                cfg.clock = clkfreq[i];
                cfg.buswidth = buswidth[j];
                cfg.burstsz = burstsz[k];
                if (mmc_test_mem_card(&cfg) != 0)
                    goto exit;
            }
        }
    }

    /* disable checksum */
    cfg.clock = clkfreq[0];
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.buswidth = HOST_BUS_WIDTH_4;
    cfg.flags = 0;

    if (mmc_test_mem_card(&cfg) != 0)
        goto exit;

    printf("Desc. DMA Test pass!\n");
exit:	
    return;
}

/*******************************************************************************
* FUNCTION
*   SDMMC_CardEnhancedDMATest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
void SDMMC_CardEnhancedDMATest(void)
{
    unsigned int i, j, k;
    int id = g_MSDC_id;
    struct mmc_test_config cfg;    
    struct mmc_host *host;
    struct mmc_card *card;
	
    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = id;
    cfg.desc = " Enhanced DMA R/W Test";
    cfg.mode = MSDC_MODE_DMA_ENHANCED;
    cfg.uhsmode = MMC_SWITCH_MODE_SDR12;
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.flags = DMA_FLAG_EN_CHKSUM; /* enable checksum */
    cfg.count = 1;
    cfg.clksrc = -1;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*) PHYSADDR(MMC_TST_BUF_ADDR);
    cfg.chk_result = 1;
#ifndef MMC_PROFILING
    cfg.tst_single = 1;
    cfg.tst_interleave = 1;
#endif
    cfg.tst_multiple = 1;

    for (i = 0; i < ARRAY_SIZE(clkfreq); i++) {	
        for (j = 0; j < ARRAY_SIZE(buswidth); j++) {
            for (k = 0; k < ARRAY_SIZE(burstsz); k++) {
                cfg.clock = clkfreq[i];
                cfg.buswidth = buswidth[j];
                cfg.burstsz = burstsz[k];
                if (mmc_test_mem_card(&cfg) != 0)
                    goto exit;
            }
        }
    }

    /* enable checksum */
    cfg.clock = clkfreq[0];
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.buswidth = HOST_BUS_WIDTH_4;
    cfg.flags = 0;

    if (mmc_test_mem_card(&cfg) != 0)
        goto exit;

    printf("Enhance DMA Test pass!\n");
exit:	
    return;
}

void SDMMC_AutoCmd12Test(void)
{
    unsigned int i, j, k;
    int id = g_MSDC_id;
    struct mmc_test_config cfg;    
    struct mmc_host *host;
    struct mmc_card *card;
    int mode[] = {MSDC_MODE_DMA_BASIC, MSDC_MODE_DMA_DESC, 
        MSDC_MODE_DMA_ENHANCED};

    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = id;
    cfg.desc = " Auto CMD12 Test";
    cfg.uhsmode = MMC_SWITCH_MODE_SDR12;
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.autocmd = MSDC_AUTOCMD12;
    cfg.flags = 0;
    cfg.count = 1;
    cfg.clksrc = -1;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*) PHYSADDR(MMC_TST_BUF_ADDR);
    cfg.chk_result = 1;
#ifndef MMC_PROFILING
    cfg.tst_single = 1;
    cfg.tst_interleave = 1;
#endif
    cfg.tst_multiple = 1;

    for (i = 0; i < ARRAY_SIZE(mode); i++) {
        for (j = 0; j < ARRAY_SIZE(clkfreq); j++) { 
            for (k = 0; k < ARRAY_SIZE(buswidth); k++) {
                cfg.mode = mode[i];
                cfg.clock = clkfreq[j];
                cfg.buswidth = buswidth[k];
                if (mmc_test_mem_card(&cfg) != 0)
                    goto exit;               
            }
        }
    }

    printf("Auto Command 12 Test pass!\n");
exit:   
    return;
}

#if 0
CTP_STATUS_T SDMMC_AutoCmd19Test(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    unsigned int i;
    int err, avail;
    int id = g_MSDC_id;
    struct mmc_host *host;
    struct mmc_card *card;
    char *buf = (char *)MMC_TST_BUF_ADDR;

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    for (i = 0; i < 3; i++) {
        if (0 != mmc_init(id))
            goto exit;
        
        host = mmc_get_host(id);
        card = mmc_get_card(id);

        /* case1. autocmd19 success */
        ERR_EXIT(mmc_tune_timing(host, card), err, MMC_ERR_NONE);
        printf("[SD%d] [PASS] test autocmd 19 done\n", id);
        
        /* case2. autocmd19 failed */
        printf("[SD%d] please plug-out card...\n", id);
        do {
            avail = mmc_card_avail(host);
        } while (avail);

        ERR_EXIT(mmc_tune_timing(host, card), err, MMC_ERR_FAILED);
        printf("[SD%d] [PASS] test autocmd 19 failed\n", id);
        
        printf("[SD%d] please plug-in card...\n", id);
        do {
            avail = mmc_card_avail(host);
        } while (!avail);
    }

    psOutput->eResult = CTP_RESULT_PASS;
exit:
    return CTP_SUCCESS;
}

CTP_STATUS_T SDMMC_AutoCmd23Test(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    unsigned int i, j, k;
    int id = g_MSDC_id;
    struct mmc_test_config cfg;
    struct mmc_host *host;
    struct mmc_card *card;
    int mode[] = {MSDC_MODE_DMA_BASIC, MSDC_MODE_DMA_DESC, 
        MSDC_MODE_DMA_ENHANCED};

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    if (0 != mmc_init(id))
        goto exit;
    
    host = mmc_get_host(id);
    card = mmc_get_card(id);

    /* check if support cmd23 for sd card */
    if (mmc_card_sd(card) && (((card->scr.cmd_support >> 1) & 0x1) == 0)) {
        printf("[SD%d] Card doesn't support CMD23\n", id);
        goto exit;
    }

    memset(&cfg, 0, sizeof(struct mmc_test_config));

    cfg.id = id;
    cfg.desc = " Auto CMD23 Test";
    cfg.uhsmode = MMC_SWITCH_MODE_SDR12;
    cfg.burstsz = MSDC_BRUST_64B;
    cfg.autocmd = MSDC_AUTOCMD23;
    cfg.flags = 0;
    cfg.count = 1;
    cfg.clksrc = -1;
    cfg.blksz = MMC_BLOCK_SIZE;
    cfg.blknr = MMC_TST_BLK_NR(0);
    cfg.total_size = MMC_TST_SIZE;
    cfg.chunk_blks = MMC_TST_CHUNK_BLKS;
    cfg.buf = (char*)MMC_TST_BUF_ADDR;
    cfg.chk_result = 1;
#ifndef MMC_PROFILING
    cfg.tst_single = 1;
    cfg.tst_interleave = 1;
#endif
    cfg.tst_multiple = 1;

    for (i = 0; i < ARRAY_SIZE(mode); i++) {
        for (j = 0; j < ARRAY_SIZE(clkfreq); j++) { 
            for (k = 0; k < ARRAY_SIZE(buswidth); k++) {
                cfg.mode = mode[i];
                cfg.clock = clkfreq[j];
                cfg.buswidth = buswidth[k];
                if (mmc_test_mem_card(&cfg) != 0)
                    goto exit;               
            }
        }
    }
    psOutput->eResult = CTP_RESULT_PASS;
exit:   
    return CTP_SUCCESS;
}

CTP_STATUS_T SDMMC_DDRModeTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    unsigned int i, j, count = 3;
    int id = g_MSDC_id;
    struct mmc_host *host;
    struct mmc_card *card;
    char *buf = (char *)MMC_TST_BUF_ADDR;

    if (psOutput == NULL)
        return CTP_FAIL;

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    if (mmc_init(id) != 0)
        goto exit;

    host = mmc_get_host(id);
    card = mmc_get_card(id);

    if (host->caps & MMC_CAP_DDR == 0) {
        printf("[SD%d] Host doesn't support DDR mode\n", id);
        goto exit;
    }
    if ((mmc_card_mmc(card) && !card->ext_csd.ddr_support) ||
        (mmc_card_sd(card) && !card->sw_caps.ddr)) {
        printf("[SD%d] Card doesn't support DDR mode\n", id);
        goto exit;
    }

    if (mmc_card_sd(card) && mmc_card_uhs1(card)) { 
        /* force to DDR mode */
        if (mmc_switch_uhs1(host, card, MMC_SWITCH_MODE_DDR50) != MMC_ERR_NONE)
            goto exit;
    }

    for (i = 0; i < ARRAY_SIZE(clkfreq); i++) {
        mmc_set_clock(host, mmc_card_ddr(card), clkfreq[i]);

        /* Single block write/read test */
        for (j = 0; j < 512; j+=4) {
            buf[j+0] = 0xa5;
            buf[j+1] = 0x5a;
            buf[j+2] = 0xa5;
            buf[j+3] = 0x5a;
        }
        if (mmc_block_write(id, 0, 1, (unsigned long *)buf) != 1)
            goto exit;
        printf("[SD%d] DDR Single Write Done\n", id, count);

        memset(buf, 0, 512);
        if (mmc_readback_blks(id, 0, 1, 0) != 0)
            goto exit;
        printf("[SD%d] DDR Single Read Done\n\n", id, count);

        /* Multiple block write/read test */
        for (j = 0; j < 1024; j+=4) {
            buf[j+0] = 0xa5;
            buf[j+1] = 0x5a;
            buf[j+2] = 0xa5;
            buf[j+3] = 0x5a;
        }
        if (mmc_block_write(id, 0, 2, (unsigned long *)buf) != 2)
            goto exit;
        printf("[SD%d] DDR Multiple Write Done\n", id, count);
        
        memset(buf, 0, 1024);
        if (mmc_readback_blks(id, 0, 2, 0) != 0)
            goto exit;
        printf("[SD%d] DDR Multiple Read Done\n\n", id, count);

    }
    psOutput->eResult = CTP_RESULT_PASS;
exit:
    return CTP_SUCCESS;

}

CTP_STATUS_T SDMMC_MMCIRQTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int i, err;
    int id = g_MSDC_id;
    u32 status, oldsclk;    
    struct mmc_host *host;
    struct mmc_card *card;
    struct mmc_command cmd;

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    if (mmc_init(id) != 0)
        goto exit;

	host = mmc_get_host(id);
	card = mmc_get_card(id);    

    if (mmc_card_sd(card))
        goto exit;

    if ((card->csd.cmdclass & CCC_IO_MODE) == 0) {
        printf("[SD%d] WARNING: Card doesn't support I/O mode for IRQ state\n", id);
    }

    cmd.opcode  = MMC_CMD_GO_IRQ_STATE;
    cmd.rsptyp  = RESP_R5;
    cmd.arg     = 0;
    cmd.retries = 3;
    cmd.timeout = 100;

    /* deselect the card to enter standby state */
    ERR_EXIT(mmc_deselect_all_card(host), err, MMC_ERR_NONE);

    for (i = 0; i < 1; i++) {
        /* scenario 1. S/W can terminate IRQ-state */
        ERR_EXIT(mmc_send_status(host, card, &status), err, MMC_ERR_NONE);
        if (R1_CURRENT_STATE(status) != 3) {
            printf("[SD%d] Card state (%d) isn't standby state\n", id, 
                R1_CURRENT_STATE(status));
            goto exit;
        }
        /* change the response time to 100kHz if PATCH_BIT[3](EN_MMC_DRV_RESP)
         * is 0 (default is 1), which would let the card/pull-up resistor to 
         * pull up the transmitter bit. it could be too slow to ramp up so that 
         * CRC response could be detected. decreasing the clock to lower 
         * frequency can avoid this situation.
         */        
        oldsclk = host->sclk;
        mmc_set_clock(host, mmc_card_ddr(card), 100000);

        ERR_EXIT(msdc_send_cmd(host, &cmd), err, MMC_ERR_NONE);
        ERR_EXIT(msdc_wait_rsp(host, &cmd), err, MMC_ERR_TIMEOUT);
        mdelay(250);
        printf("[SD%d] Wait 250ms...\n", id);
        /* generate CMD40 response */
        msdc_brk_cmd(host);
        ERR_EXIT(msdc_wait_rsp(host, &cmd), err, MMC_ERR_NONE);

        /* resume the clock  */
        mmc_set_clock(host, mmc_card_ddr(card), oldsclk);

        ERR_EXIT(mmc_send_status(host, card, &status), err, MMC_ERR_NONE);

        /* scenario 2. card irq triggered. Need I/O card */        
    }
    
    psOutput->eResult = CTP_RESULT_PASS;

exit:   
    return CTP_SUCCESS;
}

CTP_STATUS_T SDMMC_EmmcBootTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    unsigned int i, j, k, l, f, m;
    int ret;
    int id = g_MSDC_id;
    struct mmc_host *host;
    struct mmc_card *card;
    int ackdis;
#if MSDC_USE_SDXC_FPGA    
    /* FIXME. FPGA has no reset pin wired */
    //int reset[] = { EMMC_BOOT_PWR_RESET, EMMC_BOOT_PRE_IDLE_CMD, EMMC_BOOT_RST_N_SIG };
    //int reset[] = { EMMC_BOOT_PWR_RESET, EMMC_BOOT_PRE_IDLE_CMD };
    int reset[] = { EMMC_BOOT_PRE_IDLE_CMD };
#else    
    int reset[] = { EMMC_BOOT_PWR_RESET, EMMC_BOOT_PRE_IDLE_CMD, EMMC_BOOT_RST_N_SIG };
#endif
    int mode[] = { EMMC_BOOT_PULL_CMD_MODE, EMMC_BOOT_RST_CMD_MODE };
    u8 ack[] = { EXT_CSD_PART_CFG_EN_NO_ACK, EXT_CSD_PART_CFG_EN_ACK };
    u8 *rbuf, *wbuf;
    u8 enpart = EXT_CSD_PART_CFG_EN_BOOT_PART_1;
    u8 partno = EXT_CSD_PART_CFG_BOOT_PART_1;
    u8 buswidth = EXT_CSD_BOOT_BUS_WIDTH_1;
    u8 busmode[] = {EXT_CSD_BOOT_BUS_MODE_DEFT, EXT_CSD_BOOT_BUS_MODE_HS};
    u32 maxhz, hz;

    if (psOutput == NULL)
        return CTP_FAIL;

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    for (i = 0; i < ARRAY_SIZE(ack); i++) {
        for (m = 0; m < ARRAY_SIZE(busmode); m++) {

            if (mmc_init(id) != 0)
                goto exit;

        	host = mmc_get_host(id);
        	card = mmc_get_card(id);

        	if (mmc_card_sd(card))
        	    goto exit;

            maxhz = card->maxhz;

            if (busmode[m] == EXT_CSD_BOOT_BUS_MODE_DDR) {
                printf("[SD%d] EMMC Boot: DDR mode unsupported by MSDC IP\n", id);
                return CTP_FAIL;
            } else if (busmode[m] == EXT_CSD_BOOT_BUS_MODE_HS) {
                maxhz = maxhz >= 52000000 ? 52000000 : maxhz;
            } else {
                maxhz = maxhz >= 26000000 ? 26000000 : maxhz;
            }

        	wbuf = (u8 *)MMC_TST_BUF_ADDR;
        	rbuf = (u8 *)MMC_TST_BUF_ADDR + MMC_TST_SIZE;

            for (j = 0; j < MMC_TST_SIZE; j++)
                wbuf[j] = i + j % 256;

            memset(rbuf, 0, MMC_TST_SIZE);

            /* FIXME. switch to 1-bit mode and avoid in 4-bit/8-bit DDR mode */
            ERR_EXIT(mmc_set_bus_width(host, card, HOST_BUS_WIDTH_1), ret, 
                MMC_ERR_NONE);

            /* set ERASE_GROUP_DEF */
            ERR_EXIT(mmc_read_ext_csd(host, card), ret, MMC_ERR_NONE);
            ERR_EXIT(mmc_set_erase_grp_def(card, 1), ret, MMC_ERR_NONE);
            ERR_EXIT(mmc_read_ext_csd(host, card), ret, MMC_ERR_NONE);

            /* write data to partition */
            ERR_EXIT(mmc_part_write(card, partno, 0, MMC_TST_SIZE / 512, wbuf), ret,
                MMC_ERR_NONE);

            /* read data from partition */
            ERR_EXIT(mmc_part_read(card, partno, 0, MMC_TST_SIZE / 512, rbuf), ret,
                MMC_ERR_NONE);

            /* check data from partition */
            for (j = 0; j < MMC_TST_SIZE; j++) {
                if (wbuf[j] != rbuf[j]) {
                    printf("[SD%d] EMMC Boot: write data to partition failed\n", id);        
                    goto exit;
                }
            }
            printf("[SD%d] EMMC Boot: write to partition done\n", id);

            /* enable reset signal */
            //ERR_EXIT(mmc_set_reset_func(card, 1), ret, MMC_ERR_NONE);

            /* boot up configuration */
            ERR_EXIT(mmc_boot_config(card, ack[i], enpart, buswidth, busmode[m]), ret,
                MMC_ERR_NONE);
            ERR_EXIT(mmc_read_ext_csd(host, card), ret, MMC_ERR_NONE);

            ackdis = (ack[i] == EXT_CSD_PART_CFG_EN_NO_ACK) ? 1 : 0;
            
            for (k = 0; k < ARRAY_SIZE(reset); k++) {
                for (l = 0; l < ARRAY_SIZE(mode); l++) {
                    for (f = 0; f < ARRAY_SIZE(clkfreq); f++) {
                        printf("[SD%d] EMMC Boot: Reset(%d), Mode(%d) ACK(%d)\n", 
                            id, reset[k], mode[l], !ackdis);

                        /* read boot data at a time */
                        printf("[SD%d] EMMC Boot: read boot data (once mode)\n", id);
                        memset(rbuf, 0, MMC_TST_SIZE);

                        /* determine test freq */
                        hz = clkfreq[f] >= maxhz ? maxhz : hz;

                        
                        mmc_init_host(id, host, -1, MSDC_MODE_PIO);
                        mmc_set_clock(host, 0, hz);
                        mmc_boot_reset(host, card, reset[k]);
                        ERR_EXIT(mmc_boot_up(host, mode[k], ackdis, rbuf, MMC_TST_SIZE), ret,
                            MMC_ERR_NONE);      
                        for (j = 0; j < MMC_TST_SIZE; j++) {
                            if (wbuf[j] != rbuf[j]) {
                                printf("[SD%d] EMMC Boot: incorrect data\n", id);
                                goto exit;
                            }
                        }
                        printf("[SD%d] EMMC Boot: read boot data (once mode) - PASS\n", id);
                        
                        /* read boot data in 512-bytes trunks */
                        printf("[SD%d] EMMC Boot: read boot data (trunks mode)\n", id);
                        memset(rbuf, 0, MMC_TST_SIZE);
                        mmc_init_host(id, host, -1, MSDC_MODE_PIO);
                        mmc_set_clock(host, 0, hz);
                        mmc_boot_reset(host, card, reset[k]);
                        ERR_EXIT(msdc_emmc_boot_start(host, hz, 0, mode, ackdis), ret, MMC_ERR_NONE);
                        for (j = 0; j < MMC_TST_SIZE / 512; j++) {
                            ERR_EXIT(msdc_emmc_boot_read(host, 512, rbuf + j * 512), 
                                ret, MMC_ERR_NONE);
                        }                    
                        msdc_emmc_boot_stop(host);
                        for (j = 0; j < MMC_TST_SIZE; j++) {
                            if (wbuf[j] != rbuf[j]) {
                                printf("[SD%d] EMMC Boot: incorrect data\n", id);
                                goto exit;
                            }
                        }                    
                        printf("[SD%d] EMMC Boot: read boot data (trunks mode) - PASS\n", id);
                    }
                }
            }
        }
    }
    
    psOutput->eResult = CTP_RESULT_PASS;
exit:
    msdc_emmc_boot_stop(host);
    
    return CTP_SUCCESS;
}
#endif

/*******************************************************************************
* FUNCTION
*   SDMMC_StressTest
*
* DESCRIPTION
*   MMC/SD stress test
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/
void SDMMC_StressTest(int num)
{
    int i;
    struct mmc_host *host;
    struct mmc_card *card;
    struct mmc_command cmd;

    if (num < 1) 
	    num = 1;
    for (i = 0; i < num; i++) {
	printf("[SDMMC] the %d times Stress test \n", i);
        SDMMC_CardInitTest();
        SDMMC_CardEraseTest();
        SDMMC_CardPIOTest();
        SDMMC_CardBasicDMATest();
        SDMMC_CardDescDMATest();
        SDMMC_CardEnhancedDMATest();
        SDMMC_AutoCmd12Test();
    }

    printf("[SDMMC] %d Stress test done\n", num);
}
#if 0
/*******************************************************************************
* FUNCTION
*   SDMMC_SuspendResumeTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
CTP_STATUS_T SDMMC_SuspendResume(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    CTP_STATUS_T ret = CTP_SUCCESS;

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    /* TODO */

    return ret;
}
/*******************************************************************************
* FUNCTION
*   SDMMC_TuningCmdTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
CTP_STATUS_T SDMMC_TuningCmdTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int id = g_MSDC_id;
    struct mmc_host *host;
    
    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    if (0 != mmc_init(id))
        goto exit;

    host = mmc_get_host(id);

    /* plug-in different type card, ex. DDR, HS, SDR104..., then dump tuning result */
    msdc_tune_debug(host, 1);
    msdc_tune_cmdrsp(host, NULL);
    msdc_tune_debug(host, 0);

    psOutput->eResult = CTP_RESULT_PASS;
exit:   
    return CTP_SUCCESS;
}
/*******************************************************************************
* FUNCTION
*   SDMMC_TuningWriteTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
CTP_STATUS_T SDMMC_TuningWriteTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int id = g_MSDC_id;
    struct mmc_host *host;
    
    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    if (0 != mmc_init(id))
        goto exit;

    host = mmc_get_host(id);

    /* plug-in different type card, ex. DDR, HS, SDR104..., then dump tuning result */
    msdc_tune_debug(host, 1);
    msdc_tune_bwrite(host, 0, NULL, 0);
    msdc_tune_debug(host, 0);

    psOutput->eResult = CTP_RESULT_PASS;
exit:   
    return CTP_SUCCESS;
}

/*******************************************************************************
* FUNCTION
*   SDMMC_TuningReadTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
CTP_STATUS_T SDMMC_TuningReadTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int id = g_MSDC_id;
    struct mmc_host *host;
    
    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    if (0 != mmc_init(id))
        goto exit;

    host = mmc_get_host(id);

    /* plug-in different type card, ex. DDR, HS, SDR104..., then dump tuning result */
    msdc_tune_debug(host, 1);
    msdc_tune_bread(host, 0, NULL, 0);
    msdc_tune_debug(host, 0);

    psOutput->eResult = CTP_RESULT_PASS;
exit:   
    return CTP_SUCCESS;
}
/*******************************************************************************
* FUNCTION
*   SDIO_InterruptTest
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
#define RAM_SIZE_ADDRESS (0x1FF00000-4)
#define RAM_ADDRESS 0x1FF00000
    unsigned char*WIFI_RAM_CODE;
    unsigned int ram_size;

extern int wlan_sdio_test(struct sdio_func *func, int tcount);

CTP_STATUS_T SDIO_InterruptTest(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int id = g_MSDC_id;
    int i;
    unsigned char data_pins = msdc_cap.data_pins;
    CTP_STATUS_T ret = CTP_SUCCESS;
    struct mmc_host *host;
    struct mmc_card *card;
    int mode[] = {MSDC_MODE_PIO, MSDC_MODE_DMA_BASIC, MSDC_MODE_DMA_DESC, 
        MSDC_MODE_DMA_ENHANCED};
    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    ram_size = *((unsigned int*)RAM_SIZE_ADDRESS);
    WIFI_RAM_CODE =  (unsigned char* )malloc(ram_size/4*4+4);
    memcpy(WIFI_RAM_CODE, RAM_ADDRESS, ram_size);
    printf("RAM_CODE Address: 0x%x image size %d\r\n", WIFI_RAM_CODE, ram_size);

    /* set to SDIO 4-bit mode to test SDIO interrupt gap of 4-bit mode */
    msdc_cap.data_pins = 4;

#if MSDC_USE_IRQ	
    for (i = 0; i < ARRAY_SIZE(mode); i++) { 
        if (mmc_init(id) != 0)
            goto exit;

    	host = mmc_get_host(id);
    	card = mmc_get_card(id);

        msdc_set_dmode(host, mode[i]);
    	if (wlan_sdio_test(card->io_func[0], 3) != 0)
    	    goto exit;

        printf("[SDIO][PASS] SDIO interrupt gap test in 4-bit mode. DMODE=%d\n",
            mode[i]);

        CTP_WaitUntilKeyPress("[SDIO] Please reset SDIO device power and press any key to continue...\n", NULL);
    }

#if 0   
    /* set to SDIO 1-bit mode to test SDIO interrupt in DAT[1] */
    msdc_cap.data_pins = 1;

    for (i = 0; i < ARRAY_SIZE(mode); i++) { 
        if (mmc_init(id) != 0)
            goto exit;

    	host = mmc_get_host(id);
    	card = mmc_get_card(id);

        msdc_set_dmode(host, mode[i]);
    	if (wlan_sdio_test(card->io_func[0], 3) != 0)
    	    goto exit;

        printf("[SDIO][PASS] SDIO interrupt test in 1-bit mode(DAT[0]). DMODE=%d\n",
            mode[i]);

        CTP_WaitUntilKeyPress("[SDIO] Please reset SDIO device power and press any key to continue...\n", NULL);
    }
#endif    
    psOutput->eResult = CTP_RESULT_PASS;

#else
    printf("[SDIO] Unable to test in non-interrupt mode!!!\n");
    ret = CTP_FAIL;
#endif

exit:
    /* restore msdc data pins */
    msdc_cap.data_pins = data_pins;

    return ret;
}
/*******************************************************************************
* FUNCTION
*   EMMC_BootModeConfig
*
* DESCRIPTION
*   None
*
* CALLS
*   None
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*   None
*******************************************************************************/ 
CTP_STATUS_T EMMC_BootModeConfig(CTP_INPUT_DATA_T *psInput, CTP_OUTPUT_DATA_T *psOutput)
{
    int id = g_MSDC_id;
    CTP_STATUS_T ret = CTP_SUCCESS;
    struct mmc_host *host;
    struct mmc_card *card;
    u8 enpart = EXT_CSD_PART_CFG_EN_BOOT_PART_1;
    u8 acken = EXT_CSD_PART_CFG_EN_ACK;

    psOutput->eType = CTP_OUTPUT_RESULT;
    psOutput->eResult = CTP_RESULT_FAIL;

    if (mmc_init(id) != MMC_ERR_NONE)
        goto exit;

	host = mmc_get_host(id);
	card = mmc_get_card(id);

    if (mmc_boot_config(card, acken, enpart, EXT_CSD_BOOT_BUS_WIDTH_1, 
        EXT_CSD_BOOT_BUS_MODE_DEFT) == MMC_ERR_NONE) {
        (void)mmc_read_ext_csd(host, card);
        psOutput->eResult = CTP_RESULT_PASS;

        /* set reset signal function */
        //mmc_set_reset_func(card, 1);
    }

exit:
    return ret;
}
#endif
/*******************************************************************************
 * GLOBAL VARIABLE DEFINATIONS
 ******************************************************************************/
int ralink_msdc_command(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD)
    MSDC_CLR_BIT32(RALINK_SYSCTL_BASE+0x60, 0x1 << 19);
    MSDC_SET_BIT32(RALINK_SYSCTL_BASE+0x60, 0x1 << 18);
#elif defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD)
    MSDC_CLR_BIT32(RALINK_SYSCTL_BASE+0x60, 0x1 << 19);
    MSDC_CLR_BIT32(RALINK_SYSCTL_BASE+0x60, 0x1 << 18);
#elif defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
    MSDC_SET_BIT32(0xb000003c, 0x1e << 16); // TODO: maybe omitted when RAether already toggle AGPIO_CFG
    MSDC_CLR_BIT32(RALINK_SYSCTL_BASE+0x60, 0x3 << 10);
#if defined (EMMC_8BIT)
    MSDC_SET_BIT32(RALINK_SYSCTL_BASE+0x60, 0x3 << 30);
    MSDC_SET_BIT32(RALINK_SYSCTL_BASE+0x60, 0x3 << 28);
    MSDC_SET_BIT32(RALINK_SYSCTL_BASE+0x60, 0x3 << 26);
#endif
#endif

	if (!strncmp(argv[1], "register", 9)) {
		SDMMC_RegTest();
	} else if (!strncmp(argv[1], "card", 5)) {
		if (!strncmp(argv[2], "init", 5)) {
			SDMMC_CardInitTest();
		} else if (!strncmp(argv[2], "erase", 6)) {
			SDMMC_CardEraseTest();
		} else if (!strncmp(argv[2], "detect", 7)) {
			SDMMC_CardDetectTest();
		} else
			goto fail;
	} else if (!strncmp(argv[1], "pio", 4)) {
		SDMMC_CardPIOTest();
	} else if (!strncmp(argv[1], "dma", 4)) {
		if (!strncmp(argv[2], "basic", 6)) {
			SDMMC_CardBasicDMATest();
		} else if (!strncmp(argv[2], "desc", 5)) {
			SDMMC_CardDescDMATest();
		} else if (!strncmp(argv[2], "enhance", 8)) {
			SDMMC_CardEnhancedDMATest();
		} else
			goto fail;
	} else if (!strncmp(argv[1], "cmd12", 6)) {
		SDMMC_AutoCmd12Test();
	} else if (!strncmp(argv[1], "blklen", 7)) {
		SDMMC_BlockLenTest();
	} else if (!strncmp(argv[1], "stress", 7)) {
		SDMMC_StressTest(simple_strtoul(argv[2], NULL, 10));
#if 0
	} else if (!strncmp(argv[1], "switch", 7)) {
		TS_MSDC_Switch();
	} else if (!strncmp(argv[1], "auto", 5)) {
		SDMMC_AutoTest();
	} else if (!strncmp(argv[1], "mmc", 4)) {
		if (!strncmp(argc[2], "stream", 7)) {
			SDMMC_CardStreamTest();
		} else if (!strncmp(argv[2], "irq",4)){
			SDMMC_MMCIRQTest();
		} else
			goto fail;
	} else if (!strncmp(argv[1], "cmd19", 6)) {
		SDMMC_AutoCmd19Test();
	} else if (!strncmp(argv[1], "cmd23", 6)) {
		SDMMC_AutoCmd23Test();
	} else if (!strncmp(argv[1], "ddrmode", 8)) {
		SDMMC_DDRModeTest();
	} else if (!strncmp(argv[1], "emmc", 5)) {
		if (!strncmp(argv[2], "boot", 5)) {
			SDMMC_EmmcBootTest();
		} else if (!strncmp(argv[2], "bootcfg", 8)) {
			EMMC_BootModeConfig();
		} else
			goto fail;
	} else if (!strncmp(argv[1], "tune", 5)) {
		if (!strncmp(argv[2], "cmd", 4)) {
			SDMMC_TuningCmdTest();
		} else if (!strncmp(argv[2], "write", 6)) {
			SDMMC_TuningWriteTest();
		} else if (!strncmp(argv[2], "read", 5)) {
			SDMMC_TuningReadTest();
		} else
			goto fail;
	} else if (!strncmp(argv[1], "sdio", 5)) {
		SDIO_InterruptTest();
	} else if (!strncmp(argv[1], "uhs", 4)) {
		SD30_SDR104Test();
#endif
	} else
		goto fail;

	return 0;
fail:
	printf("Usage:\n%s\n use \"help msdc\" for detail!\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	msdc,	3,	1, 	ralink_msdc_command,
	"msdc - msdc verified command\n",
	"msdc usage:\n"
//	"msdc switch - Switch MSDC\n"			/* MSDC Host Select */
//	"msdc register - SDMMC Register Test\n"		/* SDMMC Register Test */
////	"msdc auto - SDMMC Auto Test\n"			/* SDMMC Auto Test */
	"msdc card init - SDMMC Card Init Test\n"	/* Card Init Test */
	"msdc card erase - SDMMC Card Erase Test\n"	/* Card Erase Test */
	"msdc card detect - SDMMC Card Detect Test\n"	/* Card Detect Test */
//	"msdc mmc stream - SDMMC Stream Test\n"		/* MMC Stream Test */
	"msdc pio - SDMMC PIO Test\n"			/* PIO Test */
	"msdc dma basic - SDMMC Basic DMA Test\n" 	/* Basic DMA Test */
	"msdc dma desc - SDMMC Desc. DMA Test\n"	/* Desc. DMA Test */
	"msdc dma enhance - SDMMC Enhanced DMA Test\n" 	/* Enhanced DMA Test */
	"msdc cmd12 - SDMMC Auto CMD12 Test\n"		/* Auto CMD12 Test */
//	"msdc cmd19 - SDMMC Auto CMD19 Test\n"		/* Auto CMD19 Test */
//	"msdc cmd23 - SDMMC Auto CMD23 Test\n"		/* Auto CMD23 Test */
	"msdc blklen - SDMMC Blk Len Test\n"		/* Change Block Len */
//	"msdc ddrmode - SDMMC DDR Mode Test\n"		/* DDR Mode Test */
//	"msdc mmc irq - SDMMC MMC IRQ Test\n"		/* MMC IRQ Test */
//	"msdc emmc boot - SDMMC EMMC Boot Test\n"	/* EMMC Boot Test */
	"msdc stress [num]\n"
////	"msdc tune cmd - SDMMC Tuning Cmd Test\n"	/* SDMMC Tuning Cmd Test */
////	"msdc tune write - SDMMC Tuning Write Test\n"	/* SDMMC Tuning Write Test */
////	"msdc tune read - SDMMC Tuning Read Test\n"	/* SDMMC Tuning Read Test */
//	"msdc sdio int - SDIO Interrupt Test\n"		/* SDIO Interrupt Test */
////	"msdc emmc bootcfg - EMMC Boot Mode Config\n"	/* EMMC Boot Mode Config */
//	"msdc uhs - uhs-i sdr104 test\n"		/* 3.0 SDR104 test */
);
