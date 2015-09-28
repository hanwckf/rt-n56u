/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2010
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

#include <common.h>
#include "msdc.h"
#include "msdc_cust.h"
#include "utils.h"
#include "mmc_core.h"
#include "mmc_test.h"
#include "sdio.h"
#include "msg.h"
#include <asm/addrspace.h>
//#include "api.h"
//#include "cache_api.h"
//#include "pmic.h"
//#include "clock_manager.h"
#define CMD_RETRIES        (5)
#define CMD_TIMEOUT        (100) /* 100ms */

#define GPO_BASE           (0xc100c000) /* 0xc101f000 */

#define PERI_MSDC_SRCSEL   (0xc100000c)
#define MSDC1_IRQ_SEL	   (1 << 9)
/* Tuning Parameter */
#define DEFAULT_DEBOUNCE   (8)	/* 8 cycles */
#define DEFAULT_DTOC       (40)	/* data timeout counter. 65536x40 sclk. */
#define DEFAULT_WDOD       (0)	/* write data output delay. no delay. */
#define DEFAULT_BSYDLY     (8)	/* card busy delay. 8 extend sclk */

#define MAX_GPD_POOL_SZ    (512)
#define MAX_BD_POOL_SZ     (1024)
#define MAX_SG_POOL_SZ     (MAX_BD_POOL_SZ)
#define MAX_DMA_TRAN_SIZE  (MAX_SG_POOL_SZ*MAX_SG_BUF_SZ)

#ifdef MMC_PROFILING       /* use optimized parameters */
#define MAX_DMA_CNT        (32768)
#define MAX_SG_BUF_SZ      (MAX_DMA_CNT)    /* sg size = DMA size */
#define MAX_BD_PER_GPD     (MAX_BD_POOL_SZ) /* only one gpd for all bd */
#else
#define MAX_DMA_CNT        (32768)
#define MAX_SG_BUF_SZ      (4096)
#define MAX_BD_PER_GPD     (MAX_BD_POOL_SZ/(MAX_GPD_POOL_SZ-1)) /* except null gpd */
#endif

#if MAX_SG_BUF_SZ > MAX_DMA_CNT
#error "incorrect max sg buffer size"
#endif

typedef struct {
    /* register offset */
    u32  offset;
    /* r: read only, w: write only, a:readable & writable
     * k: read clear, x: don't care
     * s: readable and write 1 set, c: readable and write 1 clear
     */
    char attr[32];
    /* 0: default is 0
     * 1: default is 1
     * x: don't care
     */
    char reset[32];
} reg_desc_t;

typedef struct {
    int    pio_bits;
    int    stream_stop;
    int    autocmd;
    struct dma_config  cfg;
    struct scatterlist sg[MAX_SG_POOL_SZ];
    int    alloc_gpd;
    int    alloc_bd;
    int    dsmpl;
    int    rsmpl;
    gpd_t *active_head;
    gpd_t *active_tail;
    gpd_t *gpd_pool;
    msdc_bd_t  *bd_pool;
} msdc_priv_t;

static int msdc_rsp[] = {
    0,  /* RESP_NONE */
    1,  /* RESP_R1 */
    2,  /* RESP_R2 */
    3,  /* RESP_R3 */
    4,  /* RESP_R4 */
    1,  /* RESP_R5 */
    1,  /* RESP_R6 */
    1,  /* RESP_R7 */
    7,  /* RESP_R1b */
};

static reg_desc_t msdc_reg_desc[] = {
    {OFFSET_MSDC_IOCON    , {"aaaaaaxxxxxxxxxxaaaaaaaaaaxxxxxx"}, {"000000xxxxxxxxxx0000000000xxxxxx"}},
    {OFFSET_MSDC_PS       , {"arxxxxxxxxxxaaaarrrrrrrrrxxxxxxr"}, {"01xxxxxxxxxx0000111100001xxxxxx1"}},
    {OFFSET_MSDC_INT      , {"ccxccccccccrcccccxxxxxxxxxxxxxxx"}, {"00x00000000000000xxxxxxxxxxxxxxx"}},
    {OFFSET_MSDC_INTEN    , {"aaxaaaaaaaaaaaaaaxxxxxxxxxxxxxxx"}, {"00x00000000000000xxxxxxxxxxxxxxx"}},   
    {OFFSET_MSDC_FIFOCS   , {"rrrrrrrrxxxxxxxxrrrrrrrrxxxxxxxa"}, {"00000000xxxxxxxx00000000xxxxxxx0"}},
    {OFFSET_SDC_CFG       , {"aaxxxxxxxxxxxxxxaaxaaaxxaaaaaaaa"}, {"00xxxxxxxxxxxxxx00x010xx00000000"}},
    {OFFSET_SDC_CMD       , {"aaaaaaaaaaxaaaaaaaaaaaaaaaaaaaax"}, {"0000000000x00000000000000000000x"}},
    {OFFSET_SDC_ARG       , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_STS       , {"rrxxxxxxxxxxxxxxxxxxxxxxxxxxxxxr"}, {"00xxxxxxxxxxxxxxxxxxxxxxxxxxxxx0"}},
    {OFFSET_SDC_RESP0     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_RESP1     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_RESP2     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_RESP3     , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_BLK_NUM   , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"10000000000000000000000000000000"}},
    {OFFSET_SDC_CSTS      , {"cccccccccccccccccccccccccccccccc"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_CSTS_EN   , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_DCRC_STS  , {"rrrrrrrrrrrrxxxxxxxxxxxxxxxxxxxx"}, {"000000000000xxxxxxxxxxxxxxxxxxxx"}},
    {OFFSET_EMMC_CFG0     , {"wwaaxxxxxxxxaaaaxxxxxxxxxxxxxxxx"}, {"0000xxxxxxxx0000xxxxxxxxxxxxxxxx"}},
    {OFFSET_EMMC_CFG1     , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"01000000000000000000010000000000"}},
    {OFFSET_EMMC_STS      , {"ccccrcrxxxxxxxxxxxxxxxxxxxxxxxxx"}, {"0000001xxxxxxxxxxxxxxxxxxxxxxxxx"}},
    {OFFSET_EMMC_IOCON    , {"axxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}, {"0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}},
    {OFFSET_SDC_ACMD_RESP , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_SDC_ACMD19_TRG, {"aaaaxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}, {"0000xxxxxxxxxxxxxxxxxxxxxxxxxxxx"}},
    {OFFSET_SDC_ACMD19_STS, {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_DMA_SA   , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_DMA_CA   , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_DMA_CTRL , {"wwwxxxxxaxaxaaaxaaaaaaaaaaaaaaaa"}, {"000xxxxx0x0x011x0000000000000000"}},
    {OFFSET_MSDC_DMA_CFG  , {"raxxrrxxxxxxxxxxxxxxxxxxxxxxxxxx"}, {"00xx00xxxxxxxxxxxxxxxxxxxxxxxxxx"}},
    {OFFSET_MSDC_DBG_SEL  , {"aaaaaaaaaaaaaaaaxxxxxxxxxxxxxxxx"}, {"0000000000000000xxxxxxxxxxxxxxxx"}},
    {OFFSET_MSDC_DBG_OUT  , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
    {OFFSET_MSDC_PATCH_BIT0  , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"11110010000000000011110000000010"}},
    {OFFSET_MSDC_PATCH_BIT1  , {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}, {"10010000000000000000000000000000"}},

    /* ERROR: Write 1 Reg Failed in 0xC12300F0/F4[0]=0 != 0 */
    //{OFFSET_MSDC_DAT_RDDLY0,{"aaaaaxxxaaaaaxxxaaaaaxxxaaaaaxxx"}, {"00000xxx00000xxx00000xxx00000xxx"}},
    //{OFFSET_MSDC_DAT_RDDLY1,{"aaaaaxxxaaaaaxxxaaaaaxxxaaaaaxxx"}, {"00000xxx00000xxx00000xxx00000xxx"}},
    //{OFFSET_MSDC_PAD_TUNE , {"aaaaaxxxaaaaaxxxaaaaaxaaaaaaaaaa"},{"00000xxx00000xxx00000x0000000000"}},
    {OFFSET_MSDC_HW_DBG , {"aaaaaaaaaaaaaaxxaaaaaaaaaaaaaaax"},{"00000000000000xx000000000000000x"}},
    {OFFSET_MSDC_VERSION , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"},{"10010100100100001000100000000100"}},
    {OFFSET_MSDC_ECO_VER , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"},{"00000000000000000000000000000000"}},

    {OFFSET_MSDC_CFG      , {"aaaaaarraaaaaaaaaaxxxxxxxxxxxxxx"}, {"000110010000000000xxxxxxxxxxxxxx"}},
    {OFFSET_MSDC_TXDATA   , {"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww"}, {"00000000000000000000000000000000"}},
    /* Should not be touched. */
    //{OFFSET_MSDC_RXDATA   , {"rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"}, {"00000000000000000000000000000000"}},
};

#if MSDC_DEBUG
static struct msdc_regs *msdc_reg[MSDC_MAX_NUM];
#endif

static msdc_priv_t msdc_priv[MSDC_MAX_NUM];
static gpd_t msdc_gpd_pool[MSDC_MAX_NUM][MAX_GPD_POOL_SZ];
static msdc_bd_t  msdc_bd_pool[MSDC_MAX_NUM][MAX_BD_POOL_SZ];
static int msdc_tune_dbg[MSDC_MAX_NUM] = {0, 0, 0, 0};

#if MSDC_USE_IRQ
typedef void (*irq_handler_t)(void);
static const u32 msdc_irq_line[MSDC_MAX_NUM] = {MSDC0_IRQ_ID, MSDC1_IRQ_ID, MSDC2_IRQ_ID, MSDC3_IRQ_ID};
static void *msdc_irq_data[MSDC_MAX_NUM];
static volatile u32 msdc_irq_sts[MSDC_MAX_NUM];
static hw_irq_handler_t sdio_irq_handler[MSDC_MAX_NUM];

#define DECLARE_MSDC_IRQ_HANDLER(x) \
static void msdc_irq_handler_##x(void) \
{ \
    msdc_irq_handler(msdc_irq_data[(x)]); \
}

#define REGISTER_MSDC_IRQ_HANDLER(id, hndl, data) \
do { \
    msdc_irq_data[(id)] = (void*)(data); \
    IRQSensitivity(msdc_irq_line[(id)], LEVEL_SENSITIVE); \
    IRQPolarity(msdc_irq_line[(id)], LOW_LEVEL_TRIGGER); \
    IRQ_Register_LISR(msdc_irq_line[(id)], hndl, NULL); \
    IRQUnmask(msdc_irq_line[(id)]); \
} while(0)

#define UNREGISTER_MSDC_IRQ_HANDLER(id) \
do { \
    IRQMask(msdc_irq_line[(id)]); \
    msdc_irq_data[(id)] = NULL; \
    IRQ_Register_LISR(msdc_irq_line[(id)], IRQ_Default_LISR, NULL); \
} while(0)

void msdc_register_hwirq(struct mmc_host *host, hw_irq_handler_t handler)
{
    sdio_irq_handler[host->id] = handler;
}
void msdc_power(struct mmc_host *host, u8 mode);

static void msdc_irq_handler(void *data)
{
    struct mmc_host *host = (struct mmc_host*)data;
    u32 id = host->id;
    u32 base = host->base;
    u32 intsts;
    msdc_int_reg *int_reg = (msdc_int_reg*)&intsts;

    BUG_ON(base == 0);

    //IRQMask(msdc_irq_line[id]);
    DisableIRQ();
	if(host->card_detect_flag == 0)
		msdc_power(host,MMC_POWER_ON);
    intsts = MSDC_READ32(MSDC_INT);
    msdc_irq_sts[id] |= intsts;

    if (intsts & MSDC_INT_CDSC) {
        /* card detection */
		if(host->card_detect_flag == 0){
			MSDC_SET_FIELD(PERI_MSDC_SRCSEL,MSDC1_IRQ_SEL,0);
			IRQSensitivity(msdc_irq_line[(host->id)], LEVEL_SENSITIVE);
			host->card_detect_flag = 1;
		}			
        printf("\n[SD%d] Card %s\n", id, msdc_card_avail(host) ? "Inserted" : "Removed");
    }

    if (intsts & MSDC_INT_SDIOIRQ) {
        /* sdio bus interrupt */
        MSG(INT, "[SD%d] SDIO interrupt <===\n", id);
        /* Note. msdc detects logical-low of dat1 to trigger sdio interrupt.
         * It will be triggered and should be _IGNORED_ since DAT1 of 
         * SDIO/MMC/SD/SDXC card is pulled low when it's inserted into the slot.
         */
        if (sdio_irq_handler[id]) {
            sdio_irq_handler[id]();

            /* clear it since it's already handled */
            msdc_irq_sts[id] &= ~MSDC_INT_SDIOIRQ;
        }
    }

    if (intsts & (MSDC_INT_CMDRDY|MSDC_INT_CMDTMO|MSDC_INT_RSPCRCERR)) {
        /* command done */
    }

    if (intsts & (MSDC_INT_ACMDRDY|MSDC_INT_ACMDTMO|MSDC_INT_ACMDCRCERR)) {
        /* auto command done */
    }

    if (intsts & MSDC_INT_ACMD19_DONE) {
        /* auto command 10 done */
    }
    
    if (intsts & MSDC_INT_XFER_COMPL) {
        /* data transfer done */
    }
    
    if (intsts & (MSDC_INT_DATCRCERR|MSDC_INT_DATTMO)) {
        /* data error */
    }

    if (intsts & MSDC_INT_DXFER_DONE) {
        /* dma transfer done */
    }
    
    if (intsts & MSDC_INT_DMAQ_EMPTY) {
        /* DMA queue empty */
    }

    if (intsts & MSDC_INT_CSTA) {
        /* CSTA available */
    }

    if (intsts & MSDC_INT_MMCIRQ) {
        /* MMCIRQ available */
    }

    MSG(INT, "[SD%d] IRQ_EVT(0x%x): MMCIRQ(%d) CDSC(%d), ACRDY(%d), ACTMO(%d), ACCRE(%d) AC19DN(%d)\n", 
        id,
        intsts,
        int_reg->mmcirq,
        int_reg->cdsc,
        int_reg->atocmdrdy,
        int_reg->atocmdtmo,
        int_reg->atocmdcrc,
        int_reg->atocmd19done);
    MSG(INT, "[SD%d] IRQ_EVT(0x%x): SDIO(%d) CMDRDY(%d), CMDTMO(%d), RSPCRC(%d), CSTA(%d)\n", 
        id,
        intsts,
        int_reg->sdioirq,
        int_reg->cmdrdy,
        int_reg->cmdtmo,
        int_reg->rspcrc,
        int_reg->csta);
    MSG(INT, "[SD%d] IRQ_EVT(0x%x): XFCMP(%d) DXDONE(%d), DATTMO(%d), DATCRC(%d), DMAEMP(%d)\n", 
        id,
        intsts,
        int_reg->xfercomp,
        int_reg->dxferdone,
        int_reg->dattmo,
        int_reg->datcrc,
        int_reg->dmaqempty);

    MSDC_WRITE32(MSDC_INT, intsts); /* clear interrupts */

    //IRQUnmask(msdc_irq_line[id]);
    EnableIRQ();
}

DECLARE_MSDC_IRQ_HANDLER(0);
DECLARE_MSDC_IRQ_HANDLER(1);
DECLARE_MSDC_IRQ_HANDLER(2);
DECLARE_MSDC_IRQ_HANDLER(3);
#else

#endif

void msdc_dump_card_status(u32 card_status)
{
#if MSDC_DEBUG
    static char *state[] = {
        "Idle",			/* 0 */
        "Ready",		/* 1 */
        "Ident",		/* 2 */
        "Stby",			/* 3 */
        "Tran",			/* 4 */
        "Data",			/* 5 */
        "Rcv",			/* 6 */
        "Prg",			/* 7 */
        "Dis",			/* 8 */
        "Reserved",		/* 9 */
        "Reserved",		/* 10 */
        "Reserved",		/* 11 */
        "Reserved",		/* 12 */
        "Reserved",		/* 13 */
        "Reserved",		/* 14 */
        "I/O mode",		/* 15 */
    };
    if (card_status & R1_OUT_OF_RANGE)
        printf("\t[CARD_STATUS] Out of Range\n");
    if (card_status & R1_ADDRESS_ERROR)
        printf("\t[CARD_STATUS] Address Error\n");
    if (card_status & R1_BLOCK_LEN_ERROR)
        printf("\t[CARD_STATUS] Block Len Error\n");
    if (card_status & R1_ERASE_SEQ_ERROR)
        printf("\t[CARD_STATUS] Erase Seq Error\n");
    if (card_status & R1_ERASE_PARAM)
        printf("\t[CARD_STATUS] Erase Param\n");
    if (card_status & R1_WP_VIOLATION)
        printf("\t[CARD_STATUS] WP Violation\n");
    if (card_status & R1_CARD_IS_LOCKED)
        printf("\t[CARD_STATUS] Card is Locked\n");
    if (card_status & R1_LOCK_UNLOCK_FAILED)
        printf("\t[CARD_STATUS] Lock/Unlock Failed\n");
    if (card_status & R1_COM_CRC_ERROR)
        printf("\t[CARD_STATUS] Command CRC Error\n");
    if (card_status & R1_ILLEGAL_COMMAND)
        printf("\t[CARD_STATUS] Illegal Command\n");
    if (card_status & R1_CARD_ECC_FAILED)
        printf("\t[CARD_STATUS] Card ECC Failed\n");
    if (card_status & R1_CC_ERROR)
        printf("\t[CARD_STATUS] CC Error\n");
    if (card_status & R1_ERROR)
        printf("\t[CARD_STATUS] Error\n");
    if (card_status & R1_UNDERRUN)
        printf("\t[CARD_STATUS] Underrun\n");
    if (card_status & R1_OVERRUN)
        printf("\t[CARD_STATUS] Overrun\n");
    if (card_status & R1_CID_CSD_OVERWRITE)
        printf("\t[CARD_STATUS] CID/CSD Overwrite\n");
    if (card_status & R1_WP_ERASE_SKIP)
        printf("\t[CARD_STATUS] WP Eraser Skip\n");
    if (card_status & R1_CARD_ECC_DISABLED)
        printf("\t[CARD_STATUS] Card ECC Disabled\n");
    if (card_status & R1_ERASE_RESET)
        printf("\t[CARD_STATUS] Erase Reset\n");
    if (card_status & R1_READY_FOR_DATA)
        printf("\t[CARD_STATUS] Ready for Data\n");
    if (card_status & R1_SWITCH_ERROR)
        printf("\t[CARD_STATUS] Switch error\n");
    if (card_status & R1_URGENT_BKOPS)
        printf("\t[CARD_STATUS] Urgent background operations\n");
    if (card_status & R1_APP_CMD)
        printf("\t[CARD_STATUS] App Command\n");

    printf("\t[CARD_STATUS] '%s' State\n", 
    state[R1_CURRENT_STATE(card_status)]);
#endif
}

void msdc_dump_ocr_reg(u32 resp)
{
#if MSDC_DEBUG
    if (resp & (1 << 7))
        printf("\t[OCR] Low Voltage Range\n");
    if (resp & (1 << 15))
        printf("\t[OCR] 2.7-2.8 volt\n");
    if (resp & (1 << 16))
        printf("\t[OCR] 2.8-2.9 volt\n");
    if (resp & (1 << 17))
        printf("\t[OCR] 2.9-3.0 volt\n");
    if (resp & (1 << 18))
        printf("\t[OCR] 3.0-3.1 volt\n");
    if (resp & (1 << 19))
        printf("\t[OCR] 3.1-3.2 volt\n");
    if (resp & (1 << 20))
        printf("\t[OCR] 3.2-3.3 volt\n");
    if (resp & (1 << 21))
        printf("\t[OCR] 3.3-3.4 volt\n");
    if (resp & (1 << 22))
        printf("\t[OCR] 3.4-3.5 volt\n");
    if (resp & (1 << 23))
        printf("\t[OCR] 3.5-3.6 volt\n");
    if (resp & (1 << 24))
        printf("\t[OCR] Switching to 1.8V Accepted (S18A)\n");
    if (resp & (1 << 30))
        printf("\t[OCR] Card Capacity Status (CCS)\n");
    if (resp & (1UL << 31))
        printf("\t[OCR] Card Power Up Status (Idle)\n");
    else
        printf("\t[OCR] Card Power Up Status (Busy)\n");
#endif
}

void msdc_dump_io_resp(u32 resp)
{
#if MSDC_DEBUG
    u32 flags = (resp >> 8) & 0xFF;
    char *state[] = {"DIS", "CMD", "TRN", "RFU"};
    
    if (flags & (1 << 7))
        printf("\t[IO] COM_CRC_ERR\n");
    if (flags & (1 << 6))
        printf("\t[IO] Illgal command\n");   
    if (flags & (1 << 3))
        printf("\t[IO] Error\n");
    if (flags & (1 << 2))
        printf("\t[IO] RFU\n");
    if (flags & (1 << 1))
        printf("\t[IO] Function number error\n");
    if (flags & (1 << 0))
        printf("\t[IO] Out of range\n");

    printf("[IO] State: %s, Data:0x%x\n", state[(resp >> 12) & 0x3], resp & 0xFF);
#endif
}

void msdc_dump_rca_resp(u32 resp)
{
#if MSDC_DEBUG
    u32 card_status = (((resp >> 15) & 0x1) << 23) |
                      (((resp >> 14) & 0x1) << 22) |
                      (((resp >> 13) & 0x1) << 19) |
                        (resp & 0x1fff);

    printf("\t[RCA] 0x%x\n", resp >> 16);
    msdc_dump_card_status(card_status);
#endif
}

#if MSDC_DEBUG
static void msdc_dump_dma_desc(struct mmc_host *host)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    int i;
    u32 *ptr;

    if (MSG_EVT_MASK & MSG_EVT_DMA) {
        for (i = 0; i < priv->alloc_gpd; i++) {
            ptr = (u32*)&priv->gpd_pool[i];
            printf("[SD%d] GD[%d](0x%xh): %xh %xh %xh %xh %xh %xh %xh\n", 
                host->id, i, (u32)ptr, *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), 
                *(ptr+5), *(ptr+6));
        }

        for (i = 0; i < priv->alloc_bd; i++) {
            ptr = (u32*)&priv->bd_pool[i];
            printf("[SD%d] BD[%d](0x%xh): %xh %xh %xh %xh\n", 
                host->id, i, (u32)ptr, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
        }
    }
}
#endif

void msdc_dump_register(struct mmc_host *host)
{
    u32 base = host->base;

    printf("[SD%d] MSDC_CFG   : %xh\n", host->id, *(u32*)(base + 0x0));
    printf("[SD%d] MSDC_INT   : %xh\n", host->id, *(u32*)(base + 0xc));
    printf("[SD%d] MSDC_FIFOCS: %xh\n", host->id, *(u32*)(base + 0x14));
    printf("[SD%d] SDC_CFG    : %xh\n", host->id, *(u32*)(base + 0x30));
    printf("[SD%d] SDC_STS    : %xh\n", host->id, *(u32*)(base + 0x3c));
}

int msdc_reg_test(int id)
{
    // u32 baddr[] = {MSDC0_BASE, MSDC1_BASE, MSDC2_BASE, MSDC3_BASE};
    u32 baddr[] = {MSDC0_BASE};   /* RT6352's MSDC base address: 0x10130000 - chhung */
    u32 base = baddr[id];
    u32 i, j, k;
    char v;

    /* check register reset value */
    for (i = 0; i < ARRAY_SIZE(msdc_reg_desc); i++) {
        for (j = 0; j < 32; j++) {
            if (('w' == (msdc_reg_desc[i].attr[j])) ||
                ('x' == (msdc_reg_desc[i].attr[j])))
                continue;
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;
            if (v != (msdc_reg_desc[i].reset[j] - '0')) {
                printf("[SD%d] Invalid Reset Value in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v, 
                    msdc_reg_desc[i].reset[j] - '0');
                return MMC_ERR_FAILED;
            }
        }
    }

    /* check read only register */
    for (i = 0; i < ARRAY_SIZE(msdc_reg_desc); i++) {
        for (j = 0; j < 32; j++) {
            if ('r' != (msdc_reg_desc[i].attr[j]))
                continue;
            if (0 == (msdc_reg_desc[i].reset[j] - '0'))
                MSDC_SET_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            else
                MSDC_CLR_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;
            if (v != (msdc_reg_desc[i].reset[j] - '0')) {
                printf("[SD%d] Read Only Reg Modified in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v, 
                    msdc_reg_desc[i].reset[j] - '0');
                return MMC_ERR_FAILED;
            }
        }
    }

    /* check write register */
    for (i = 0; i < ARRAY_SIZE(msdc_reg_desc); i++) {
        for (j = 0; j < 32; j++) {
            if ('a' != (msdc_reg_desc[i].attr[j]))
                continue;

            /* write 1 to target bit */
            MSDC_SET_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;
            
            if (v != 1) {
                printf("[SD%d] Write 1 Reg Failed in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v, 
                    msdc_reg_desc[i].reset[j] - '0');
                return MMC_ERR_FAILED;
            }

            /* exception rule for clock stable */
            if (((u32)MSDC_CFG == (base + msdc_reg_desc[i].offset)) && 
                (j >= 8 && j < 18)) { /* wait clock stable */
                while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
            }
            
            /* check other bits are not affected by write 1 */
            for (k = 0; k < 32; k++) {
                if (k == j)
                    continue;
                if (('w' == (msdc_reg_desc[i].attr[k])) ||
                    ('x' == (msdc_reg_desc[i].attr[k])))
                    continue;
                v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> k) & 0x1;
                if (v != (msdc_reg_desc[i].reset[k] - '0')) {
                    printf("[SD%d] Affected by Write 1 to 0x%x[%d] and [%d]=%d != %d\n",
                        id, base + msdc_reg_desc[i].offset, j, k, v, 
                        msdc_reg_desc[i].reset[k] - '0');
                    return MMC_ERR_FAILED;
                }
            }

            /* write 0 to target bit */
            MSDC_CLR_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> j) & 0x1;
            
            if (v != 0) {
                printf("[SD%d] Write 0 Reg Failed in 0x%x[%d]=%d != %d\n",
                    id, base + msdc_reg_desc[i].offset, j, v, 
                    msdc_reg_desc[i].reset[j] - '0');
                return MMC_ERR_FAILED;
            }

            /* exception rule for clock stable */
            if (((u32)MSDC_CFG == (base + msdc_reg_desc[i].offset)) && 
                (j >= 8 && j < 18)) { /* wait clock stable */
                while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));
            }

            /* check other bits are not affected by write 1 */
            for (k = 0; k < 32; k++) {
                if (k == j)
                    continue;
                if (('w' == (msdc_reg_desc[i].attr[k])) ||
                    ('x' == (msdc_reg_desc[i].attr[k])))
                    continue;
                v = (MSDC_READ32(base + msdc_reg_desc[i].offset) >> k) & 0x1;
                if (v != (msdc_reg_desc[i].reset[k] - '0')) {
                    printf("[SD%d] Affected by Write 0 to 0x%x[%d] and [%d]=%d != %d\n",
                        id, base + msdc_reg_desc[i].offset, j, k, v, 
                        msdc_reg_desc[i].reset[k] - '0');
                    return MMC_ERR_FAILED;
                }
            }

            /* reset to default value */
            if ((msdc_reg_desc[i].reset[j] - '0') == 1)
                MSDC_SET_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
            else
                MSDC_CLR_BIT32(base + msdc_reg_desc[i].offset, 0x1 << j);
        }
    }
    return MMC_ERR_NONE;
}

void msdc_flush_membuf(void *buf, u32 len)
{
	//cache_clean_invalidate();
}

u8 msdc_cal_checksum(u8 *buf, u32 len)
{
    u32 i, sum = 0;
    for (i = 0; i < len; i++) {
        sum += buf[i];
    }
    return 0xFF - (u8)sum;
}

/* allocate gpd link-list from gpd_pool */
gpd_t *msdc_alloc_gpd(struct mmc_host *host, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    gpd_t *gpd, *ptr, *prev;

    if (priv->alloc_gpd + num + 1 > MAX_GPD_POOL_SZ || num == 0)
        return NULL;

    gpd = priv->gpd_pool + priv->alloc_gpd;
    priv->alloc_gpd += (num + 1); /* include null gpd */

    memset(gpd, 0, sizeof(gpd_t) * (num + 1));

    ptr = gpd + num - 1;
    ptr->next = (void*)(gpd + num); /* pointer to null gpd */
    
    /* create link-list */
    if (ptr != gpd) {
        do {
            prev = ptr - 1;
            prev->next = ptr;
            ptr = prev;
        } while (ptr != gpd);
    }

    return gpd;
}

/* allocate bd link-list from bd_pool */
msdc_bd_t *msdc_alloc_bd(struct mmc_host *host, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    msdc_bd_t *bd, *ptr, *prev;

    if (priv->alloc_bd + num > MAX_BD_POOL_SZ || num == 0)
        return NULL;
    
    bd = priv->bd_pool + priv->alloc_bd;
    priv->alloc_bd += num;

    memset(bd, 0, sizeof(msdc_bd_t) * num);

    ptr = bd + num - 1;
    ptr->eol  = 1;
    ptr->next = 0;

    /* create link-list */
    if (ptr != bd) {
        do {
            prev = ptr - 1;
            prev->next = ptr;
            prev->eol  = 0;
            ptr = prev;
        } while (ptr != bd);
    }

    return bd;
}

/* queue bd link-list to one gpd */
void msdc_queue_bd(struct mmc_host *host, gpd_t *gpd, msdc_bd_t *bd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    BUG_ON(gpd->ptr);

    gpd->hwo = 1;
    gpd->bdp = 1;
    gpd->ptr = (void*)bd;

    if (priv->cfg.flags & DMA_FLAG_EN_CHKSUM == 0) 
        return;

    /* calculate and fill bd checksum */
    while (bd) {
        bd->chksum = msdc_cal_checksum((u8*)bd, 16);
        bd = bd->next;
    }
}

/* queue data buf to one gpd */
void msdc_queue_buf(struct mmc_host *host, gpd_t *gpd, u8 *buf)
{
    BUG_ON(gpd->ptr);

    gpd->hwo = 1;
    gpd->bdp = 0;
    gpd->ptr = (void*)buf;
}

/* add gpd link-list to active list */
void msdc_add_gpd(struct mmc_host *host, gpd_t *gpd, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    if (num > 0) {
        if (!priv->active_head) {
            priv->active_head = gpd;
        } else {
            priv->active_tail->next = gpd;
        }
        priv->active_tail = gpd + num - 1;

        if (priv->cfg.flags & DMA_FLAG_EN_CHKSUM == 0)
            return;

        /* calculate and fill gpd checksum */
        while (gpd) {
            gpd->chksum = msdc_cal_checksum((u8 *)gpd, 16);
            gpd = gpd->next;
        }
    }
}

void msdc_reset_gpd(struct mmc_host *host)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    priv->alloc_bd  = 0;
    priv->alloc_gpd = 0;
    priv->active_head = NULL;
    priv->active_tail = NULL;
}

#if MSDC_USE_LEVEL_SHIFT
#define GPO_SDXC_SEL        (0x1 << 0)
#define GPO_CARD_PWR        (0x1 << 1)
#define GPO_LVL_PWR_18V     (0x1 << 2)
#define GPO_LVL_PWR_33V     (0x1 << 3)
static u32 gpo_val = GPO_SDXC_SEL;

static void msdc_clr_gpo(u32 mask)
{
    gpo_val &= ~mask;
    MSDC_WRITE32(GPO_BASE, gpo_val);
}

static void msdc_set_gpo(u32 bits)
{
    gpo_val |= bits;
    MSDC_SET_BIT32(GPO_BASE, gpo_val);
}

/* 0: 3.3V 1: 1.8V */
void msdc_set_host_level_pwr(int level)
{
    /* GPO[3:2] = {LVL_PWR33, LVL_PWR18}; */
    msdc_clr_gpo((0x3<<2));

    if (level)
        msdc_set_gpo(GPO_LVL_PWR_18V);
    else
        msdc_set_gpo(GPO_LVL_PWR_33V);
}

void msdc_set_card_pwr(int on)
{
    if (on)
        msdc_set_gpo(GPO_CARD_PWR);
    else
        msdc_clr_gpo(GPO_CARD_PWR);
    mdelay(10);
}
#else
void msdc_set_host_level_pwr(int level)
{
    unsigned int ret;
    
#if 0
    if (level) {
        ret = pmic_config_interface(0xA7,0x2,0x7,4); /* VMC=1.8V */
    } else {
        ret = pmic_config_interface(0xA7,0x7,0x7,4); /* VMC=3.3V */
    }
    if (ret != 0) {
        printf("PMIC: Set MSDC Vol Level Fail\n");
    }
    mdelay(100); /* requires before voltage stable */
#endif
}
void msdc_set_card_pwr(int on)
{
    unsigned int ret;

#if 0
    ret = pmic_config_interface(0xAB,0x7,0x7,4); /* VMCH=3.3V */

    mdelay(50);

    if (ret == 0) {
        if(on) {            
            ret = pmic_config_interface(0xAB,0x1,0x1,0); /* VMCH_EN=1 */
        } else {
            ret = pmic_config_interface(0xAB,0x0,0x1,0); /* VMCH_EN=0 */
        }
    }
    if (ret != 0) {
        printf("PMIC: Set MSDC Card Power Fail\n");
    }
#endif
}
void msdc_set_host_pwr(int on)
{
    unsigned int ret;

#if 0
    ret = pmic_config_interface(0xA7,0x7,0x7,4); /* VMC=3.3V */

    if (ret == 0) {
        if(on) {
            ret = pmic_config_interface(0xA7,0x1,0x1,0); /* VMC_EN=1 */
        } else {
            ret = pmic_config_interface(0xA7,0x0,0x1,0); /* VMC_EN=0 */
        }
    }
    
    if (ret != 0) {
        printf("PMIC: Set MSDC Host Power Fail\n");
    }
    
    mdelay(50);
#endif
}
#endif

void msdc_set_smpl(struct mmc_host *host, u8 dsmpl, u8 rsmpl)
{
    u32 base = host->base;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    /* set sampling edge */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, rsmpl);
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, dsmpl);

    /* wait clock stable */
    while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));

    priv->rsmpl = rsmpl;
    priv->dsmpl = dsmpl;    
}

static u32 msdc_cal_timeout(struct mmc_host *host, u32 ns, u32 clks, u32 clkunit)
{
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout / clkunit;
    return timeout;
}

void msdc_set_timeout(struct mmc_host *host, u32 ns, u32 clks)
{
    u32 base = host->base;
    u32 timeout, clk_ns;

    clk_ns  = 1000000000UL / host->sclk;
    timeout = ns / clk_ns + clks;
    timeout = timeout >> 16; /* in 65536 sclk cycle unit */
    timeout = timeout > 1 ? timeout - 1 : 0;
    timeout = timeout > 255 ? 255 : timeout;

    MSDC_SET_FIELD(SDC_CFG, SDC_CFG_DTOC, timeout);

    MSG(OPS, "[SD%d] Set read data timeout: %dns %dclks -> %d x 65536 cycles\n",
        host->id, ns, clks, timeout + 1);
}

void msdc_set_blklen(struct mmc_host *host, u32 blklen)
{
    u32 base = host->base;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    host->blklen     = blklen;
    priv->cfg.blklen = blklen;
    MSDC_CLR_FIFO();
}

void msdc_set_blknum(struct mmc_host *host, u32 blknum)
{
    u32 base = host->base;

    MSDC_WRITE32(SDC_BLK_NUM, blknum);
}

void msdc_set_dmode(struct mmc_host *host, int mode)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    priv->cfg.mode = mode;
    
    if (mode == MSDC_MODE_PIO) {
        host->blk_read  = msdc_pio_bread;
        host->blk_write = msdc_pio_bwrite;
    } else if (mode == MSDC_MODE_MMC_STREAM) {
        host->blk_read  = msdc_stream_bread;
        host->blk_write = msdc_stream_bwrite;
    } else {
        host->blk_read  = msdc_dma_bread;
        host->blk_write = msdc_dma_bwrite;
    }
}

void msdc_set_pio_bits(struct mmc_host *host, int bits)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    priv->pio_bits = bits;
}

void msdc_set_dma(struct mmc_host *host, u8 burstsz, u32 flags)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;

    cfg->burstsz = burstsz;
    cfg->flags   = flags;
}

void msdc_set_autocmd(struct mmc_host *host, int cmd, int on)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    if (on) {
        priv->autocmd |= cmd;
    } else {
        priv->autocmd &= ~cmd;
    }
}

void msdc_clr_fifo(struct mmc_host *host)
{
    u32 base = host->base;
    MSDC_CLR_FIFO();
}

void msdc_abort(struct mmc_host *host)
{
    u32 base = host->base;

    printf("[SD%d] Abort: MSDC_FIFOCS=%xh MSDC_PS=%xh SDC_STS=%xh\n", 
        host->id, MSDC_READ32(MSDC_FIFOCS), MSDC_READ32(MSDC_PS), MSDC_READ32(SDC_STS));

    /* reset controller */
    MSDC_RESET();

    /* clear fifo */
    MSDC_CLR_FIFO();

    /* make sure txfifo and rxfifo are empty */
    if (MSDC_TXFIFOCNT() != 0 || MSDC_RXFIFOCNT() != 0) {
        printf("[SD%d] Abort: TXFIFO(%d), RXFIFO(%d) != 0\n",
            host->id, MSDC_TXFIFOCNT(), MSDC_RXFIFOCNT());
    }

    /* clear all interrupts */
    MSDC_WRITE32(MSDC_INT, MSDC_READ32(MSDC_INT));
}

void msdc_abort_handler(struct mmc_host *host, int abort_card)
{
    u32 base = host->base;
    struct mmc_command stop;

    msdc_abort(host);

    if (abort_card) {
        stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
        stop.rsptyp  = RESP_R1B;
        stop.arg     = 0;
        stop.retries = CMD_RETRIES;
        stop.timeout = CMD_TIMEOUT;
        msdc_send_cmd(host, &stop);
        msdc_wait_rsp(host, &stop);
    }
}

u32 msdc_intr_wait(struct mmc_host *host, u32 intrs)
{
    u32 base = host->base;
    u32 sts;

    /* warning that interrupts are not enabled */
    WARN_ON((MSDC_READ32(MSDC_INTEN) & intrs) != intrs);

#if MSDC_USE_IRQ
    while (1) {
        DisableIRQ();    
        if (msdc_irq_sts[host->id] & intrs) {
            sts = msdc_irq_sts[host->id];
            msdc_irq_sts[host->id] &= ~intrs;
            EnableIRQ();
            break;
        }        
        EnableIRQ();
    }
#else
    #if 0 /* FIXME. E1 ECO workaround */
    {
        u32 tmo = 3000;
        WAIT_COND(((sts = MSDC_READ32(MSDC_INT)) & intrs), tmo, tmo);

        if (tmo == 0) {
            printf("[SD%d] ECO WARNNING ==> Wait INT timeout\n", host->id);
            MSDC_RESET();
        }
    }
    #else
    while (((sts = MSDC_READ32(MSDC_INT)) & intrs) == 0);
    #endif
#endif
    MSG(INT, "[SD%d] INT(0x%x)\n", host->id, sts);
#if !MSDC_USE_IRQ
    MSDC_WRITE32(MSDC_INT, (sts & intrs));
#endif
    if (~intrs & sts) {
        MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
            host->id, ~intrs & sts);
    }
    return sts;
}

void msdc_intr_unmask(struct mmc_host *host, u32 bits)
{
    u32 base = host->base;
    u32 val;

    val  = MSDC_READ32(MSDC_INTEN);
    val |= bits;
    MSDC_WRITE32(MSDC_INTEN, val);    
}

void msdc_intr_mask(struct mmc_host *host, u32 bits)
{
    u32 base = host->base;
    u32 val;

    val  = MSDC_READ32(MSDC_INTEN);
    val &= ~bits;
    MSDC_WRITE32(MSDC_INTEN, val);
}

void msdc_intr_sdio(struct mmc_host *host, int enable)
{
    u32 base = host->base;

    MSG(INT, "[SD%d] %s SDIO INT\n", host->id, enable ? "Enable" : "Disable");

    if (enable) {
        MSDC_SET_BIT32(SDC_CFG, SDC_CFG_SDIOIDE|SDC_CFG_SDIOINTWKUP);
        msdc_intr_unmask(host, MSDC_INT_SDIOIRQ);
    } else {
        msdc_intr_mask(host, MSDC_INT_SDIOIRQ);
        MSDC_CLR_BIT32(SDC_CFG, SDC_CFG_SDIOIDE|SDC_CFG_SDIOINTWKUP);
    }
}

void msdc_intr_sdio_gap(struct mmc_host *host, int enable)
{
    u32 base = host->base;

    MSG(INT, "[SD%d] %s SDIO GAP\n", host->id, enable ? "Enable" : "Disable");

    if (enable) {
        MSDC_SET_BIT32(SDC_CFG, SDC_CFG_INTATGAP);
    } else {
        MSDC_CLR_BIT32(SDC_CFG, SDC_CFG_INTATGAP);
    }
}

int msdc_send_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base   = host->base;
    u32 opcode = cmd->opcode;
    u32 rsptyp = cmd->rsptyp;   
    u32 rawcmd;
    u32 timeout = cmd->timeout;
    u32 error = MMC_ERR_NONE;

    /* rawcmd :
     * vol_swt << 30 | auto_cmd << 28 | blklen << 16 | go_irq << 15 | 
     * stop << 14 | rw << 13 | dtype << 11 | rsptyp << 7 | brk << 6 | opcode
     */
    rawcmd = (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)) | 
        msdc_rsp[rsptyp] << 7 | host->blklen << 16;

    if (opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
        rawcmd |= ((2 << 11) | (1 << 13));
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_WRITE_BLOCK) {
        rawcmd |= ((1 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
        rawcmd |= (2 << 11);
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_READ_SINGLE_BLOCK || 
               opcode == SD_ACMD_SEND_SCR ||
               opcode == SD_CMD_SWITCH ||
               opcode == MMC_CMD_SEND_EXT_CSD) {
        rawcmd |= (1 << 11);    
    } else if (opcode == SD_IO_RW_EXTENDED) {
        if (cmd->arg & 0x80000000)  /* R/W flag */
            rawcmd |= (1 << 13);
        if ((cmd->arg & 0x08000000) && ((cmd->arg & 0x1FF) > 1))
            rawcmd |= (2 << 11); /* multiple block mode */
        else
            rawcmd |= (1 << 11);
    } else if (opcode == MMC_CMD_STOP_TRANSMISSION) {
        rawcmd |= (1 << 14);
        rawcmd &= ~(0x0FFF << 16);
    } else if (opcode == SD_IO_RW_DIRECT) {
        if ((cmd->arg & 0x80000000) && ((cmd->arg >> 9) & 0x1FFFF))/* I/O abt */
            rawcmd |= (1 << 14);
    } else if (opcode == SD_CMD_VOL_SWITCH) {
        rawcmd |= (1 << 30);
    } else if (opcode == SD_CMD_SEND_TUNING_BLOCK) {
        rawcmd |= (1 << 11); /* CHECKME */
        if (priv->autocmd & MSDC_AUTOCMD19)
            rawcmd |= (3 << 28);
    } else if (opcode == MMC_CMD_GO_IRQ_STATE) {
        rawcmd |= (1 << 15);
    } else if (opcode == MMC_CMD_WRITE_DAT_UNTIL_STOP) {
        rawcmd |= ((1<< 13) | (3 << 11));
    } else if (opcode == MMC_CMD_READ_DAT_UNTIL_STOP) {
        rawcmd |= (3 << 11);
    }
    
    MSG(CMD, "[SD%d] CMD(%d): ARG(0x%x), RAW(0x%x), RSP(%d)\n", 
        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)), cmd->arg, rawcmd, rsptyp);

    if (!priv->stream_stop) {
        /* FIXME. Need to check if SDC is busy before data read/write transfer */
        if (opcode == MMC_CMD_SEND_STATUS) {
            if (SDC_IS_CMD_BUSY()) {
                WAIT_COND(!SDC_IS_CMD_BUSY(), cmd->timeout, timeout);
                if (timeout == 0) {
                    error = MMC_ERR_TIMEOUT;
                    printf("[SD%d] CMD(%d): SDC_IS_CMD_BUSY timeout\n", 
                        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
                    goto end;
                }
            }
        } else {
            if (SDC_IS_BUSY()) {
                WAIT_COND(!SDC_IS_BUSY(), 1000, timeout);
                if (timeout == 0) {
                    error = MMC_ERR_TIMEOUT;
                    printf("[SD%d] CMD(%d): SDC_IS_BUSY timeout\n", 
                        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)));
                    goto end;
                }
            }        
        }
    }
    
    SDC_SEND_CMD(rawcmd, cmd->arg);

end:
    cmd->error = error;

    return error;
}

int msdc_wait_rsp(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base   = host->base;
    u32 rsptyp = cmd->rsptyp;
    u32 status;
    u32 opcode = (cmd->opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT));
    u32 error = MMC_ERR_NONE;
    u32 wints = MSDC_INT_CMDTMO | MSDC_INT_CMDRDY | MSDC_INT_RSPCRCERR |
        MSDC_INT_ACMDRDY | MSDC_INT_ACMDCRCERR | MSDC_INT_ACMDTMO |
        MSDC_INT_ACMD19_DONE;

    if (cmd->opcode == MMC_CMD_GO_IRQ_STATE)
        wints |= MSDC_INT_MMCIRQ;

    status = msdc_intr_wait(host, wints);

    if (status == 0) {
        error = MMC_ERR_TIMEOUT;
        goto end;
    }
    
    if ((status & MSDC_INT_CMDRDY) || (status & MSDC_INT_ACMDRDY) ||
        (status & MSDC_INT_ACMD19_DONE)) {
        switch (rsptyp) {
        case RESP_NONE:
            MSG(RSP, "[SD%d] CMD(%d): RSP(%d)\n", host->id, opcode, rsptyp);
            break;
        case RESP_R2:
        {
            u32 *resp = &cmd->resp[0];
            *resp++ = MSDC_READ32(SDC_RESP3);
            *resp++ = MSDC_READ32(SDC_RESP2);
            *resp++ = MSDC_READ32(SDC_RESP1);
            *resp++ = MSDC_READ32(SDC_RESP0);
            MSG(RSP, "[SD%d] CMD(%d): RSP(%d) = 0x%x 0x%x 0x%x 0x%x\n", 
                host->id, opcode, cmd->rsptyp, cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);         
            break;
        }
        default: /* Response types 1, 3, 4, 5, 6, 7(1b) */
            if ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE))
                cmd->resp[0] = MSDC_READ32(SDC_ACMD_RESP);
            else
                cmd->resp[0] = MSDC_READ32(SDC_RESP0);
            MSG(RSP, "[SD%d] CMD(%d): RSP(%d) = 0x%x AUTO(%d)\n", host->id, opcode, 
                cmd->rsptyp, cmd->resp[0], 
                ((status & MSDC_INT_ACMDRDY) || (status & MSDC_INT_ACMD19_DONE)) ? 1 : 0);
            break;
        }
    } else if ((status & MSDC_INT_RSPCRCERR) || (status & MSDC_INT_ACMDCRCERR)) {
        error = MMC_ERR_BADCRC;
        printf("[SD%d] CMD(%d): RSP(%d) ERR(BADCRC)\n", 
            host->id, opcode, cmd->rsptyp);
    } else if ((status & MSDC_INT_CMDTMO) || (status & MSDC_INT_ACMDTMO)) {
        error = MMC_ERR_TIMEOUT;
        MSG(RSP, "[SD%d] CMD(%d): RSP(%d) ERR(CMDTO) AUTO(%d)\n", 
            host->id, opcode, cmd->rsptyp, status & MSDC_INT_ACMDTMO ? 1: 0);
    } else {
        error = MMC_ERR_INVALID;
        printf("[SD%d] CMD(%d): RSP(%d) ERR(INVALID), Status:%x\n", 
            host->id, opcode, cmd->rsptyp, status);        
    }

end:

    if (rsptyp == RESP_R1B) {
        while ((MSDC_READ32(MSDC_PS) & 0x10000) != 0x10000);
    }

#if MSDC_DEBUG    
    if ((error == MMC_ERR_NONE) && (MSG_EVT_MASK & MSG_EVT_RSP)){
        switch(cmd->rsptyp) {
        case RESP_R1:
        case RESP_R1B:
            msdc_dump_card_status(cmd->resp[0]);
            break;
        case RESP_R3:
            msdc_dump_ocr_reg(cmd->resp[0]);
            break;
        case RESP_R5:
            msdc_dump_io_resp(cmd->resp[0]);
            break;
        case RESP_R6:
            msdc_dump_rca_resp(cmd->resp[0]);
            break;
        }
    }
#endif

    cmd->error = error;
 
    return error;
}


int msdc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    int err;

    err = msdc_send_cmd(host, cmd);
    if (err != MMC_ERR_NONE)
        return err;

    err = msdc_wait_rsp(host, cmd);

    if (err == MMC_ERR_BADCRC) {
        u32 base = host->base;
        u32 tmp = MSDC_READ32(SDC_CMD);

        /* check if data is used by the command or not */
        if (tmp & 0x1800) {
            msdc_abort_handler(host, 1);
        }
        err = msdc_tune_cmdrsp(host, cmd);
    }
    return err;
}

void msdc_brk_cmd(struct mmc_host *host)
{
    u32 base = host->base;
    
    SDC_SEND_CMD(0x000000e8, 0);
}

int msdc_sg_init(struct scatterlist *sg, void *buf, u32 buflen)
{
    int i = MAX_SG_POOL_SZ;
    char *ptr = (char *)buf;

    BUG_ON(buflen > MAX_SG_POOL_SZ * MAX_SG_BUF_SZ);
    
    while (i > 0) {
        if (buflen > MAX_SG_BUF_SZ) {
            sg->addr = (u32)ptr;
            sg->len  = MAX_SG_BUF_SZ;
            buflen  -= MAX_SG_BUF_SZ;
            ptr     += MAX_SG_BUF_SZ;
            sg++; i--; 
        } else {
            sg->addr = (u32)ptr;
            sg->len  = buflen;
            i--;
            break;
        }
    }

    msdc_flush_membuf(buf, buflen);

    return MAX_SG_POOL_SZ - i;
}

void msdc_dma_init(struct mmc_host *host, struct dma_config *cfg, void *buf, u32 buflen)
{
    u32 base = host->base;
    
    cfg->xfersz = buflen;
    
    if (cfg->mode == MSDC_MODE_DMA_BASIC) {
        cfg->sglen = 1;
        cfg->sg[0].addr = (u32)buf;
        cfg->sg[0].len = buflen;
        msdc_flush_membuf(buf, buflen);
    } else {
        cfg->sglen = msdc_sg_init(cfg->sg, buf, buflen);
    }

    MSDC_CLR_FIFO();
    MSDC_DMA_ON();    
}

int msdc_dma_cmd(struct mmc_host *host, struct dma_config *cfg, struct mmc_command *cmd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 opcode = cmd->opcode;
    u32 rsptyp = cmd->rsptyp;    
    u32 rawcmd;

    rawcmd = (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)) | 
        rsptyp << 7 | host->blklen << 16;

    if (opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
        rawcmd |= ((2 << 11) | (1 << 13));
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_WRITE_BLOCK) {
        rawcmd |= ((1 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
        rawcmd |= (2 << 11);
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_READ_SINGLE_BLOCK) {
        rawcmd |= (1 << 11);
    } else if (opcode == SD_IO_RW_EXTENDED) {
        if (cmd->arg & 0x80000000)  /* R/W flag */
            rawcmd |= (1 << 13);
        if ((cmd->arg & 0x08000000) && ((cmd->arg & 0x1FF) > 1))
            rawcmd |= (2 << 11); /* multiple block mode */
        else
            rawcmd |= (1 << 11);
    } else if (opcode == SD_IO_RW_DIRECT) {
        if ((cmd->arg & 0x80000000) && ((cmd->arg >> 9) & 0x1FFFF))/* I/O abt */
            rawcmd |= (1 << 14);
    } else {
        return -1;
    }

    MSG(DMA, "[SD%d] DMA CMD(%d), AUTOCMD12(%d), AUTOCMD23(%d)\n",
        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)), 
        (priv->autocmd & MSDC_AUTOCMD12) ? 1 : 0,
        (priv->autocmd & MSDC_AUTOCMD23) ? 1 : 0);
        
    cfg->cmd = rawcmd;
    cfg->arg = cmd->arg;

    return 0;
}

int msdc_dma_config(struct mmc_host *host, struct dma_config *cfg)
{
    u32 base = host->base;
    u32 sglen = cfg->sglen;
    u32 i, j, num, bdlen, arg, xfersz;
    u8  blkpad, dwpad, chksum;
    struct scatterlist *sg = cfg->sg;
    gpd_t *gpd;
    msdc_bd_t *bd;

    switch (cfg->mode) {
    case MSDC_MODE_DMA_BASIC:
        BUG_ON(cfg->xfersz > MAX_DMA_CNT);
        BUG_ON(cfg->sglen != 1);
        MSDC_WRITE32(MSDC_DMA_SA, sg->addr);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_LASTBUF, 1);
#if defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) || defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
        MSDC_WRITE32(RALINK_MSDC_BASE+0xa8, sg->len);
#else
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_XFERSZ, sg->len);
#endif
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 0);
        break;
    case MSDC_MODE_DMA_DESC:
        blkpad = (cfg->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
        dwpad  = (cfg->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
        chksum = (cfg->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;

#if 0 /* YD: current design doesn't support multiple GPD in descriptor dma mode */
        /* calculate the required number of gpd */
        num = (sglen + MAX_BD_PER_GPD - 1) / MAX_BD_PER_GPD;        
        gpd = msdc_alloc_gpd(host, num);
        for (i = 0; i < num; i++) {
            gpd[i].intr = 0;
            if (sglen > MAX_BD_PER_GPD) {
                bdlen  = MAX_BD_PER_GPD;
                sglen -= MAX_BD_PER_GPD;
            } else {
                bdlen = sglen;
                sglen = 0;
            }
            bd = msdc_alloc_bd(host, bdlen);
            for (j = 0; j < bdlen; j++) {
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
                sg++;
            }
            msdc_queue_bd(host, &gpd[i], bd);
            msdc_flush_membuf(bd, bdlen * sizeof(msdc_bd_t));
        }     
        msdc_add_gpd(host, gpd, num);
        #if MSDC_DEBUG
        msdc_dump_dma_desc(host);
        #endif
        msdc_flush_membuf(gpd, num * sizeof(gpd_t));
        MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
        MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);
#else
        /* calculate the required number of gpd */
        BUG_ON(sglen > MAX_BD_POOL_SZ);
        
        gpd = msdc_alloc_gpd(host, 1);
        gpd->intr = 0;
        
        bd = msdc_alloc_bd(host, sglen);
        for (j = 0; j < sglen; j++) {
            MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
            sg++;
        }
        msdc_queue_bd(host, &gpd[0], bd);
        msdc_flush_membuf(bd, sglen * sizeof(msdc_bd_t));

        msdc_add_gpd(host, gpd, 1);
        #if MSDC_DEBUG
        msdc_dump_dma_desc(host);
        #endif
        msdc_flush_membuf(gpd, (1 + 1) * sizeof(gpd_t)); /* include null gpd */
        MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
        MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);

#endif        
        break;
    case MSDC_MODE_DMA_ENHANCED:
        arg = cfg->arg;
        blkpad = (cfg->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
        dwpad  = (cfg->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
        chksum = (cfg->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;
        
        /* calculate the required number of gpd */
        num = (sglen + MAX_BD_PER_GPD - 1) / MAX_BD_PER_GPD;        
        gpd = msdc_alloc_gpd(host, num);
        for (i = 0; i < num; i++) {            
            xfersz = 0;
            if (sglen > MAX_BD_PER_GPD) {
                bdlen  = MAX_BD_PER_GPD;
                sglen -= MAX_BD_PER_GPD;
            } else {
                bdlen = sglen;
                sglen = 0;
            }
            bd = msdc_alloc_bd(host, bdlen);
            for (j = 0; j < bdlen; j++) {
                xfersz += sg->len;
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
                sg++;
            }
            /* YD: 1 XFER_COMP interrupt will be triggerred by each GPD when it
             * is done. For multiple GPDs, multiple XFER_COMP interrupts will be 
             * triggerred. In such situation, it's not easy to know which 
             * interrupt indicates the transaction is done. So, we use the 
             * latest one GPD's INT as the transaction done interrupt.
             */
            //gpd[i].intr = cfg->intr;
            gpd[i].intr = (i == num - 1) ? 0 : 1;
            gpd[i].cmd  = cfg->cmd;
            gpd[i].blknum = xfersz / cfg->blklen;
            gpd[i].arg  = arg;
            gpd[i].extlen = 0xC;

            arg += xfersz;

            msdc_queue_bd(host, &gpd[i], bd);
            msdc_flush_membuf(bd, bdlen * sizeof(msdc_bd_t));
        }     
        msdc_add_gpd(host, gpd, num);
        #if MSDC_DEBUG
        msdc_dump_dma_desc(host);
        #endif
        msdc_flush_membuf(gpd, (num + 1) * sizeof(gpd_t)); /* include null gpd */
        MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
        MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
        MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);
        break;
    default:
        break;
    }
    MSG(DMA, "[SD%d] DMA_SA   = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_SA));
    MSG(DMA, "[SD%d] DMA_CA   = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CA));
    MSG(DMA, "[SD%d] DMA_CTRL = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CTRL));
    MSG(DMA, "[SD%d] DMA_CFG  = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CFG));

    return 0;
}

void msdc_dma_resume(struct mmc_host *host)
{
    u32 base = host->base;

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_RESUME, 1);

    MSG(DMA, "[SD%d] DMA resume\n", host->id);
}

void msdc_dma_start(struct mmc_host *host)
{
    u32 base = host->base;

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_START, 1);

    MSG(DMA, "[SD%d] DMA start\n", host->id);
}

void msdc_dma_stop(struct mmc_host *host)
{
    u32 base = host->base;

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP, 1);
    while ((MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS) != 0);
    MSDC_DMA_OFF();

    MSG(DMA, "[SD%d] DMA Stopped\n", host->id);
    
    msdc_reset_gpd(host);
}

int msdc_dma_wait_done(struct mmc_host *host, u32 timeout)
{
    u32 base = host->base;
    u32 tmo = timeout;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    u32 status;
    u32 error = MMC_ERR_NONE;
    u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR |
        MSDC_INT_DXFER_DONE | MSDC_INT_DMAQ_EMPTY | 
        MSDC_INT_ACMDRDY | MSDC_INT_ACMDTMO | MSDC_INT_ACMDCRCERR | 
        MSDC_INT_CMDRDY | MSDC_INT_CMDTMO | MSDC_INT_RSPCRCERR;

    do {
        MSG(DMA, "[SD%d] DMA Curr Addr: 0x%x, Active: %d\n", host->id, 
            MSDC_READ32(MSDC_DMA_CA), MSDC_READ32(MSDC_DMA_CFG) & 0x1);    

        status = msdc_intr_wait(host, wints);

        if (status == 0 || status & MSDC_INT_DATTMO) {
            printf("[SD%d] DMA DAT timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;
        } else if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DMA DAT CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;
        } else if (status & MSDC_INT_CMDTMO) {
            printf("[SD%d] DMA CMD timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;        
        } else if (status & MSDC_INT_RSPCRCERR) {
            printf("[SD%d] DMA CMD CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;    
        } else if (status & MSDC_INT_ACMDTMO) {
            printf("[SD%d] DMA ACMD timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;        
        } else if (status & MSDC_INT_ACMDCRCERR) {
            printf("[SD%d] DMA ACMD CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;
        }

        if ((cfg->mode == MSDC_MODE_DMA_ENHANCED) && (status & MSDC_INT_CMDRDY)) {
            cfg->rsp = MSDC_READ32(SDC_RESP0);
            MSG(DMA, "[SD%d] DMA ENH CMD Rdy, Resp(%xh)\n", host->id, cfg->rsp);
            #if MSDC_DEBUG 
            msdc_dump_card_status(cfg->rsp);
            #endif
        }
        if (status & MSDC_INT_ACMDRDY) {
            cfg->autorsp = MSDC_READ32(SDC_ACMD_RESP);
            MSG(DMA, "[SD%d] DMA AUTO CMD Rdy, Resp(%xh)\n", host->id, cfg->autorsp);
            #if MSDC_DEBUG 
            msdc_dump_card_status(cfg->autorsp);
            #endif
        }
        if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
            /* YD: 1 XFER_COMP interrupt will be triggerred by each GPD when it
             * is done. For multiple GPDs, multiple XFER_COMP interrupts will be 
             * triggerred. In such situation, it's not easy to know which 
             * interrupt indicates the transaction is done. So, we use the 
             * latest one GPD's INT as the transaction done interrupt.
             */        
            if (status & MSDC_INT_DXFER_DONE)
                break;
        } else {
            if (status & MSDC_INT_XFER_COMPL)
                break;
        }
    } while (1);

    /* check dma status */
    do {
        status = MSDC_READ32(MSDC_DMA_CFG);
        if (status & MSDC_DMA_CFG_GPDCSERR) {
            MSG(DMA, "[SD%d] GPD checksum error\n", host->id);
            error = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_DMA_CFG_BDCSERR) {
            MSG(DMA, "[SD%d] BD checksum error\n", host->id);
            error = MMC_ERR_BADCRC;
            break;
        } else if ((status & MSDC_DMA_CFG_STS) == 0) {
            break;
        }
    } while (1);
end:
    return error;
}

int msdc_dma_iorw(struct mmc_card *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)

{   
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    struct mmc_host *host = card->host;
    struct mmc_command cmd;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    u32 nblks = (u32)blocks;

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.opcode = SD_IO_RW_EXTENDED;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= fn << 28;
    cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.arg |= addr << 9;
    if (blocks == 1 && blksz <= 512) {        
        cmd.arg |= (blksz == 512) ? 0 : blksz;	/* byte mode */
    } else {
        cmd.arg |= 0x08000000 | blocks;		/* block mode */
    }
    cmd.rsptyp  = RESP_R5;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
        /* NOTICE: SDIO can't not issue multiple commands for one transcation data
         * so can't use multiple GPDs for that. But multiple transactions can
         * use multiple GPDs.
         * If BUG_ON is triggerred, please increase MAX_BD_PER_GPD number.
         */
        BUG_ON((blocks * blksz / MAX_SG_BUF_SZ) > MAX_BD_PER_GPD);
        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);
        msdc_dma_cmd(host, cfg, &cmd);
        msdc_dma_init(host, cfg, (void*)buf, nblks * blksz);
        msdc_dma_config(host, cfg);
        msdc_dma_start(host);
        err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
        msdc_dma_stop(host);
        /* SDIO workaround for CMD53 multiple block transfer */
        #if 1
        if (!err && nblks > 1) {
            struct mmc_command abort;
            memset(&abort, 0, sizeof(struct mmc_command));
            abort.opcode = SD_IO_RW_DIRECT;
            abort.arg    = 0x80000000;            /* write */
            abort.arg   |= 0 << 28;               /* function 0 */
            abort.arg   |= SDIO_CCCR_ABORT << 9;  /* address */
            abort.arg   |= 0;                     /* abort function 0 */
            abort.rsptyp = RESP_R1B;
            abort.retries = CMD_RETRIES;
            abort.timeout = CMD_TIMEOUT;
    
            err = msdc_cmd(host, &abort);
        }
        #endif
    } else {
        u32 left_sz, xfer_sz;

        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);

        left_sz = nblks * blksz;

        if (cfg->mode == MSDC_MODE_DMA_BASIC) {
            /* NOTICE: SDIO can't not issue multiple commands for one transcation 
             * data. If BUG_ON is triggerred, please decrease transaction data size.
             */
            BUG_ON(left_sz > MAX_DMA_CNT);
            xfer_sz = left_sz > MAX_DMA_CNT ? MAX_DMA_CNT : left_sz;
            nblks   = xfer_sz / blksz;
        } else {
            xfer_sz = left_sz;
        }

        while (left_sz) {

            msdc_set_blknum(host, nblks);
            msdc_dma_init(host, cfg, (void*)buf, xfer_sz);
            msdc_dma_config(host, cfg);

            err = msdc_cmd(host, &cmd);

            if (err != MMC_ERR_NONE) {
                msdc_reset_gpd(host);
                goto done;
            }

            msdc_dma_start(host);
            err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
            msdc_dma_stop(host);

            /* SDIO workaround for CMD53 multiple block transfer */
            #if 1
            if (!err && nblks > 1) {
                struct mmc_command abort;
                memset(&abort, 0, sizeof(struct mmc_command));
                abort.opcode = SD_IO_RW_DIRECT;
                abort.arg    = 0x80000000;            /* write */
                abort.arg   |= 0 << 28;               /* function 0 */
                abort.arg   |= SDIO_CCCR_ABORT << 9;  /* address */
                abort.arg   |= 0;                     /* abort function 0 */
                abort.rsptyp = RESP_R1B;
                abort.retries = CMD_RETRIES;
                abort.timeout = CMD_TIMEOUT;
        
                err = msdc_cmd(host, &abort);
            }
            #endif
            if (err != MMC_ERR_NONE)
                goto done;
            buf     += xfer_sz;
            left_sz -= xfer_sz;
            if (left_sz) {
                xfer_sz  = (xfer_sz > left_sz) ? left_sz : xfer_sz;
                nblks    = (xfer_sz > left_sz) ? nblks : left_sz / blksz;
            }
        }
    }

done:
    if (derr != MMC_ERR_NONE) {
        printf("[SD%d] <CMD%d> IO DMA data error (%d)\n", host->id, cmd.opcode & ~SD_CMD_BIT, derr);
        msdc_abort_handler(host, 0);
    }

    return err;
}

int msdc_dma_transfer(struct mmc_host *host, struct mmc_command *cmd, uchar *buf, ulong nblks)
{
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    struct mmc_command stop;

    BUG_ON(nblks * blksz > MAX_DMA_TRAN_SIZE);

    multi = nblks > 1 ? 1 : 0;

    if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
        if (multi && (priv->autocmd == 0))
            msdc_set_autocmd(host, MSDC_AUTOCMD12, 1);
        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);
        msdc_dma_cmd(host, cfg, cmd);
        msdc_dma_init(host, cfg, (void*)buf, nblks * blksz);
        msdc_dma_config(host, cfg);
        msdc_dma_start(host);
        err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
        msdc_dma_stop(host);
        if (multi && (priv->autocmd == 0))
            msdc_set_autocmd(host, MSDC_AUTOCMD12, 0);
    } else {
        u32 left_sz, xfer_sz;

        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);

        left_sz = nblks * blksz;

        if (cfg->mode == MSDC_MODE_DMA_BASIC) {
            xfer_sz = left_sz > MAX_DMA_CNT ? MAX_DMA_CNT : left_sz;
            nblks   = xfer_sz / blksz;
        } else {
            xfer_sz = left_sz;
        }

        while (left_sz) {
#if 1
            msdc_set_blknum(host, nblks);
            msdc_dma_init(host, cfg, (void*)buf, xfer_sz);

            err = msdc_send_cmd(host, cmd);
            msdc_dma_config(host, cfg);
            msdc_dma_start(host);

            if (err != MMC_ERR_NONE) {
                msdc_dma_stop(host);
                msdc_reset_gpd(host);
                goto done;
            }

            err = msdc_wait_rsp(host, cmd);
            
            if (err == MMC_ERR_BADCRC) {
                u32 base = host->base;
                u32 tmp = MSDC_READ32(SDC_CMD);
            
                /* check if data is used by the command or not */
                if (tmp & 0x1800) {
                    msdc_abort_handler(host, 1);
                }
                err = msdc_tune_cmdrsp(host, cmd);
            }

            err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
            msdc_dma_stop(host);
#else
            msdc_set_blknum(host, nblks);
            msdc_dma_init(host, cfg, (void*)buf, xfer_sz);
            msdc_dma_config(host, cfg);

            err = msdc_cmd(host, cmd);
            if (err != MMC_ERR_NONE) {
                msdc_reset_gpd(host);
                goto done;
            }

            msdc_dma_start(host);
            err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
            msdc_dma_stop(host);
#endif

            if (multi && (priv->autocmd == 0)) {
                stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
                stop.rsptyp  = RESP_R1B;
                stop.arg     = 0;
                stop.retries = CMD_RETRIES;
                stop.timeout = CMD_TIMEOUT;
                err = msdc_cmd(host, &stop) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
            }
            if (err != MMC_ERR_NONE)
                goto done;
            buf     += xfer_sz;
            left_sz -= xfer_sz;

            /* left_sz > 0 only when in basic dma mode */
            if (left_sz) {
                cmd->arg += nblks; /* update to next start address */
                xfer_sz  = (xfer_sz > left_sz) ? left_sz : xfer_sz;
                nblks    = (left_sz > xfer_sz) ? nblks : left_sz / blksz;
            }
        }
    }
done:
    if (derr != MMC_ERR_NONE) {
        printf("[SD%d] <CMD%d> DMA data error (%d)\n", host->id, cmd->opcode, derr);
        msdc_abort_handler(host, 1);
    }

    return err;
}

int msdc_dma_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    int multi;
    struct mmc_command cmd;    

    BUG_ON(nblks > host->max_phys_segs);

    MSG(OPS, "[SD%d] Read data %d blks from 0x%x\n", host->id, nblks, src);

    multi = nblks > 1 ? 1 : 0;

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    return msdc_dma_transfer(host, &cmd, dst, nblks);
}

int msdc_dma_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    int multi;
    struct mmc_command cmd;

    BUG_ON(nblks > host->max_phys_segs);

    MSG(OPS, "[SD%d] Write data %d blks to 0x%x\n", host->id, nblks, dst);

    multi = nblks > 1 ? 1 : 0;

    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    return msdc_dma_transfer(host, &cmd, src, nblks);
}

int msdc_pio_read_byte(struct mmc_host *host, u8 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;    
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 100000;
    u32 status;
    u32 totalsz = size;
    u8  done = 0;

    while (1) {
#if MSDC_USE_IRQ
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left DAT: %d/%d bytes\n", 
                host->id, status, size, totalsz);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left DAT: %d/%d bytes\n", 
                host->id, status, size, totalsz);
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            done = 1;
        }

        if (size == 0 && done)
            break;

        /* Note. RXFIFO count would be aligned to 4-bytes alignment size */
        if ((size >=  MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD)) {
            int left = MSDC_FIFO_THD;
            do {
                *ptr++ = MSDC_FIFO_READ8();
            } while (--left);
            size -= MSDC_FIFO_THD;
            MSG(FIO, "[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
                host->id, MSDC_FIFO_THD, MSDC_RXFIFOCNT(), size, totalsz);
        } else if ((size < MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= size)) {
            while (size) {
                *ptr++ = MSDC_FIFO_READ8();
                size--;
            }
            MSG(FIO, "[SD%d] Read left bytes, RXFIFOCNT: %d, Left: %d/%d\n",
                host->id, MSDC_RXFIFOCNT(), size, totalsz);
        }

    }

    return err;
}

int msdc_pio_read_hword(struct mmc_host *host, u16 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;    
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 100000;
    u32 status;
    u32 totalsz = size;
    u8  done = 0;

    while (1) {
#if MSDC_USE_IRQ
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left DAT: %d/%d bytes\n", 
                host->id, status, size, totalsz);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left DAT: %d/%d bytes\n", 
                host->id, status, size, totalsz);
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            done = 1;
        }

        if (size == 0 && done)
            break;

        /* Note. RXFIFO count would be aligned to 4-bytes alignment size */
        if ((size >=  MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD)) {
            int left = MSDC_FIFO_THD >> 1;
            do {
                *ptr++ = MSDC_FIFO_READ16();
            } while (--left);
            size -= MSDC_FIFO_THD;
            MSG(FIO, "[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
                host->id, MSDC_FIFO_THD, MSDC_RXFIFOCNT(), size, totalsz);
        } else if ((size < MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= size)) {
            while (size) {
                if (size > 2) {
                    *ptr++ = MSDC_FIFO_READ16();
                    size -= 2;
                } else {
                    u16 val = MSDC_FIFO_READ16();
                    memcpy(ptr, &val, size);
                    size = 0;
                }
            }
            MSG(FIO, "[SD%d] Read left bytes, RXFIFOCNT: %d, Left: %d/%d\n",
                host->id, MSDC_RXFIFOCNT(), size, totalsz);
        }
    }

    return err;
}

int msdc_pio_read_word(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 100000;
    u32 status;
    u32 totalsz = size;
    u8  done = 0;

    while (1) {
#if MSDC_USE_IRQ
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left:%d/%d bytes, RXFIFO:%d\n", 
                host->id, status, size, totalsz, MSDC_RXFIFOCNT());
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left: %d/%d bytes, RXFIFO:%d\n", 
                host->id, status, size, totalsz, MSDC_RXFIFOCNT());
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            done = 1;
        }

        if (size == 0 && done)
            break;

        /* Note. RXFIFO count would be aligned to 4-bytes alignment size */
        if ((size >=  MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD)) {
            int left = MSDC_FIFO_THD >> 2;
            do {
                *ptr++ = MSDC_FIFO_READ32();
            } while (--left);
            size -= MSDC_FIFO_THD;
            MSG(FIO, "[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
                host->id, MSDC_FIFO_THD, MSDC_RXFIFOCNT(), size, totalsz);
        } else if ((size < MSDC_FIFO_THD) && MSDC_RXFIFOCNT() >= size) {
            while (size) {
                if (size > 3) {
                    *ptr++ = MSDC_FIFO_READ32();
                    size -= 4;
                } else {
                    u32 val = MSDC_FIFO_READ32();
                    memcpy(ptr, &val, size);
                    size = 0;
                }
            }
            MSG(FIO, "[SD%d] Read left bytes, RXFIFOCNT: %d, Left: %d/%d\n",
                host->id, MSDC_RXFIFOCNT(), size, totalsz);
        }
    }

    return err;
}

int msdc_pio_read(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    msdc_priv_t *priv = host->priv;

    switch (priv->pio_bits) {
    case 32:
        err = msdc_pio_read_word(host, (u32*)ptr, size);
        break;
    case 16:
        err = msdc_pio_read_hword(host, (u16*)ptr, size);
        break;
    case 8:
        err = msdc_pio_read_byte(host, (u8*)ptr, size);
        break;
    }

    if (err != MMC_ERR_NONE) {
        msdc_abort(host); /* reset internal fifo and state machine */
        printf("[SD%d] %d-bit PIO Read Error (%d)\n", host->id,
            priv->pio_bits, err);
    }
    
    return err;
}

int msdc_pio_write_byte(struct mmc_host *host, u8 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    u32 timeout = 250000;
    u32 status;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u8  stream = (priv->cfg.mode == MSDC_MODE_MMC_STREAM) ? 1 : 0;

    while (1) {
#if MSDC_USE_IRQ
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left DAT: %d bytes\n", 
                host->id, status, size);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left DAT: %d bytes\n", 
                host->id, status, size);
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            if (size == 0) {
                MSG(OPS, "[SD%d] all data flushed to card\n", host->id); 
                break;
            } else {
                MSG(WRN, "[SD%d]<CHECKME> XFER_COMPL before all data written\n",
                    host->id);               
            }
        } else if (stream) {
            if (MSDC_READ32(SDC_STS) & SDC_STS_SWR_COMPL)
                break;
            MSG(OPS, "[SD%d] Wait for stream write data flush\n", host->id);
        }

        if (size == 0)
            continue;

        if (size >= MSDC_FIFO_SZ) {
            if (MSDC_TXFIFOCNT() == 0) {
                int left = MSDC_FIFO_SZ;
                do {
                    MSDC_FIFO_WRITE8(*ptr); ptr++;
                } while (--left);
                size -= MSDC_FIFO_SZ;
            }
        } else if (size < MSDC_FIFO_SZ && MSDC_TXFIFOCNT() == 0) {            
            while (size ) {
                MSDC_FIFO_WRITE8(*ptr); ptr++;
                size--;
            }
        }
    }

    return err;
}

int msdc_pio_write_hword(struct mmc_host *host, u16 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 250000;
    u32 status;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u8  stream = (priv->cfg.mode == MSDC_MODE_MMC_STREAM) ? 1 : 0;

    while (1) {
#if MSDC_USE_IRQ
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left DAT: %d bytes\n", 
                host->id, status, size);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left DAT: %d bytes, MSDC_FIFOCS=%xh\n", 
                host->id, status, size, MSDC_READ32(MSDC_FIFOCS));
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            if (size == 0) {
                MSG(OPS, "[SD%d] all data flushed to card\n", host->id); 
                break;
            } else {
                MSG(WRN, "[SD%d]<CHECKME> XFER_COMPL before all data written\n",
                    host->id);               
            }
        } else if (stream) {
            if (MSDC_READ32(SDC_STS) & SDC_STS_SWR_COMPL)
                break;
            MSG(OPS, "[SD%d] Wait for stream write data flush\n", host->id);
        }


        if (size == 0)
            continue;

        if (size >= MSDC_FIFO_SZ) {
            if (MSDC_TXFIFOCNT() == 0) {
                int left = MSDC_FIFO_SZ >> 1;
                do {
                    MSDC_FIFO_WRITE16(*ptr); ptr++;
                } while (--left);
                size -= MSDC_FIFO_SZ;
            }
        } else if (size < MSDC_FIFO_SZ && MSDC_TXFIFOCNT() == 0) {
            while (size) {
                if (size > 1) {
                    MSDC_FIFO_WRITE16(*ptr); ptr++;
                    size -= 2;
                } else {
                    u16 val = 0;                    
                    memcpy(&val, ptr, size);
                    MSDC_FIFO_WRITE16(val);
                    size = 0;
                }
            }
        }
    }

    return err;
}

int msdc_pio_write_word(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    u32 base = host->base;
    u32 ints = MSDC_INT_DATCRCERR | MSDC_INT_DATTMO | MSDC_INT_XFER_COMPL;
    //u32 timeout = 250000;
    u32 status;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u8  stream = (priv->cfg.mode == MSDC_MODE_MMC_STREAM) ? 1 : 0;

    while (1) {
#if MSDC_USE_IRQ
        DisableIRQ();
        status = msdc_irq_sts[host->id];
        msdc_irq_sts[host->id] &= ~ints;
        EnableIRQ();
#else
        status = MSDC_READ32(MSDC_INT);
        MSDC_WRITE32(MSDC_INT, status);
#endif
        if (status & ~ints) {
            MSG(WRN, "[SD%d]<CHECKME> Unexpected INT(0x%x)\n", 
                host->id, status);
        }
        if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DAT CRC error (0x%x), Left DAT: %d bytes\n", 
                host->id, status, size);
            err = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_INT_DATTMO) {
            printf("[SD%d] DAT TMO error (0x%x), Left DAT: %d bytes\n", 
                host->id, status, size);
            err = MMC_ERR_TIMEOUT;
            break;
        } else if (status & MSDC_INT_XFER_COMPL) {
            if (size == 0) {
                MSG(OPS, "[SD%d] all data flushed to card\n", host->id); 
                break;
            } else {
                MSG(WRN, "[SD%d]<CHECKME> XFER_COMPL before all data written\n",
                    host->id);               
            }
        } else if (stream) {
    	    if (MSDC_READ32(SDC_STS) & SDC_STS_SWR_COMPL)
    	        break;
    	    MSG(OPS, "[SD%d] Wait for stream write data flush\n", host->id);    
        }

        if (size == 0)
            continue;

        if (size >= MSDC_FIFO_THD) {
            if (MSDC_TXFIFOCNT() == 0) {
                int left = MSDC_FIFO_THD >> 2;
                do {
                    MSDC_FIFO_WRITE32(*ptr); ptr++;
                } while (--left);
                size -= MSDC_FIFO_THD;
            }
        } else if (size < MSDC_FIFO_THD && MSDC_TXFIFOCNT() == 0) {            
            while (size ) {
                if (size > 3) {
                    MSDC_FIFO_WRITE32(*ptr); ptr++;
                    size -= 4;
                } else {
                    u32 val = 0;                    
                    memcpy(&val, ptr, size);
                    MSDC_FIFO_WRITE32(val);
                    size = 0;
                }
            }
        }
    }

    return err;
}

int msdc_pio_write(struct mmc_host *host, u32 *ptr, u32 size)
{
    int err = MMC_ERR_NONE;
    msdc_priv_t *priv = host->priv;

    switch (priv->pio_bits) {
    case 32:
        err = msdc_pio_write_word(host, (u32*)ptr, size);
        break;
    case 16:
        err = msdc_pio_write_hword(host, (u16*)ptr, size);
        break;
    case 8:
        err = msdc_pio_write_byte(host, (u8*)ptr, size);
        break;
    }
    
    if (err != MMC_ERR_NONE) {
        msdc_abort(host); /* reset internal fifo and state machine */
        printf("[SD%d] %d-bit PIO Write Error (%d)\n", host->id,
            priv->pio_bits, err);
    }

    return err;
}

int msdc_pio_iorw(struct mmc_card *card, int write, unsigned fn,
	unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
{
    int err;
    struct mmc_host *host = card->host;
    struct mmc_command cmd;

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.opcode = SD_IO_RW_EXTENDED;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= fn << 28;
    cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.arg |= addr << 9;
    if (blocks == 1 && blksz <= 512) {        
        cmd.arg |= (blksz == 512) ? 0 : blksz;	/* byte mode */
    } else {
        cmd.arg |= 0x08000000 | blocks;		/* block mode */
    }
    cmd.rsptyp  = RESP_R5;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    msdc_clr_fifo(host);
    msdc_set_blknum(host, blocks);
    msdc_set_blklen(host, blksz);
    err = msdc_cmd(host, &cmd);

    if (err)
        return err;

    if (cmd.resp[0] & R5_ERROR)
        return MMC_ERR_FAILED;
    if (cmd.resp[0] & R5_FUNCTION_NUMBER)
        return MMC_ERR_INVALID;
    if (cmd.resp[0] & R5_OUT_OF_RANGE)
        return MMC_ERR_INVALID;

    if (write) {
        err = msdc_pio_write(host, (u32*)buf, blocks * blksz);
    } else {
        err = msdc_pio_read(host, (u32*)buf, blocks * blksz);
    }

    /* SDIO workaround for CMD53 multiple block transfer */
#if 1
    if (!err && blocks > 1) {
        struct mmc_command abort;
        memset(&abort, 0, sizeof(struct mmc_command));
        abort.opcode = SD_IO_RW_DIRECT;
        abort.arg    = 0x80000000;            /* write */
        abort.arg   |= 0 << 28;               /* function 0 */
        abort.arg   |= SDIO_CCCR_ABORT << 9;  /* address */
        abort.arg   |= 0;                     /* abort function 0 */
        abort.rsptyp = RESP_R1B;
        abort.retries = CMD_RETRIES;
        abort.timeout = CMD_TIMEOUT;

        err = msdc_cmd(host, &abort);
    }
#endif

    return err;
}

int msdc_pio_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    int multi;
    struct mmc_command cmd;
    struct mmc_command stop;
    ulong *ptr = (ulong *)dst;

    MSG(OPS, "[SD%d] Read data %d bytes from 0x%x\n", host->id, nblks * blksz, src);

    multi = nblks > 1 ? 1 : 0;

    MSDC_CLR_FIFO();

    #if 0 /* disable it since tuning algorithm cover this */
    /* Note: DDR50 PIO-8/16 data write timeout issue. Fore write DSMPL=1 to 
     * avoid/reduce tuning, which could cause abnormal data latching and 
     * trigger data timeout.
     */
    if (mmc_card_ddr(host->card)) {
        MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, MSDC_SMPL_RISING);
    }
    #endif

    msdc_set_blknum(host, nblks);
    msdc_set_blklen(host, blksz);
    msdc_set_timeout(host, 100000000, 0);

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = derr = msdc_pio_read(host, (u32*)ptr, nblks * blksz);
    
    if (multi && (priv->autocmd == 0)) {
        stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
        stop.rsptyp  = RESP_R1B;
        stop.arg     = 0;
        stop.retries = CMD_RETRIES;
        stop.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &stop) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
    }

done:
    if (err != MMC_ERR_NONE) {
        if (derr != MMC_ERR_NONE) {
            printf("[SD%d] Read data error (%d)\n", host->id, derr);
            msdc_abort_handler(host, 1);
        } else {
            printf("[SD%d] Read error (%d)\n", host->id, err);
        }
    }
    return (derr == MMC_ERR_NONE) ? err : derr;
}

int msdc_pio_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    struct mmc_command cmd;
    struct mmc_command stop;
    ulong *ptr = (ulong *)src;

    MSG(OPS, "[SD%d] Write data %d bytes to 0x%x\n", host->id, nblks * blksz, dst);

    multi = nblks > 1 ? 1 : 0;	

    MSDC_CLR_FIFO();

    #if 0 /* disable it since tuning algorithm cover this */
    /* Note: DDR50 PIO-8/16 data write timeout issue. Fore write DSMPL=1 to 
     * avoid/reduce tuning, which could cause abnormal data latching and 
     * trigger data timeout.
     */
    if (mmc_card_ddr(host->card)) {
        MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, MSDC_SMPL_FALLING);
    }
    #endif

    msdc_set_blknum(host, nblks);
    msdc_set_blklen(host, blksz);
    
    /* No need since MSDC always waits 8 cycles for write data timeout */
    
    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

    err = derr = msdc_pio_write(host, (u32*)ptr, nblks * blksz);

    if (multi && (priv->autocmd == 0)) {
        stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
        stop.rsptyp  = RESP_R1B;
        stop.arg     = 0;
        stop.retries = CMD_RETRIES;
        stop.timeout = CMD_TIMEOUT;
        err = msdc_cmd(host, &stop) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
    }

done:
    if (err != MMC_ERR_NONE) {
        if (derr != MMC_ERR_NONE) {
           	printf("[SD%d] Write data error (%d)\n", host->id, derr);
            msdc_abort_handler(host, 1);
        } else {
            printf("[SD%d] Write error (%d)\n", host->id, err);
        }
    }
    return (derr == MMC_ERR_NONE) ? err : derr;
}

int msdc_iorw(struct mmc_card *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
{
    int err;
    struct mmc_host *host = card->host;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    MSG(OPS, "[SD%d] IO: wr(%d) fn(%xh) adr(%xh), inc(%d), blks(%d), blksz(%d)\n", 
        host->id, write, fn, addr, incr_addr, blocks, blksz);

    /* TODO: refine me */
    if (priv->cfg.mode == MSDC_MODE_PIO)
        err = msdc_pio_iorw(card, write, fn, addr, incr_addr, buf, blocks, blksz);
    else
        err = msdc_dma_iorw(card, write, fn, addr, incr_addr, buf, blocks, blksz);

    return err;
}

int msdc_stream_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    struct mmc_card *card = host->card;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE;
    struct mmc_command cmd;
    ulong *ptr = (ulong *)dst;

    if (!(card->csd.cmdclass & CCC_STREAM_READ)) {
        printf("[SD%d]<WARNING>: Card doesn't support stream read\n", host->id);
        return -1;
    }

    MSG(OPS, "[SD%d] Stream read data %d bytes from 0x%x\n", host->id,
        nblks * blksz, src);

    MSDC_CLR_FIFO();

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, blksz);
    msdc_set_timeout(host, 100000000, 0);

    /* send stream read command */
    cmd.opcode  = MMC_CMD_READ_DAT_UNTIL_STOP;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE) 
        goto done;

    err = msdc_pio_read(host, (u32*)ptr, nblks * blksz);
    if (priv->autocmd == 0) {
        cmd.opcode	= MMC_CMD_STOP_TRANSMISSION;
        cmd.rsptyp	= RESP_R1B; /* for stream read/write, must R1b */
        cmd.arg 	= 0;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
        priv->stream_stop = 1;
        err = msdc_cmd(host, &cmd) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
        priv->stream_stop = 0;
    }

done:
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Stream read data error %d\n", host->id, err);
    }

    return err;
}

int msdc_stream_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    struct mmc_card *card = host->card;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 base = host->base;
    u32 blksz = host->blklen;
    int err = MMC_ERR_NONE;
    struct mmc_command cmd;
    ulong *ptr = (ulong *)src;

    if (!(card->csd.cmdclass & CCC_STREAM_WRITE)) {
        printf("[SD%d]<WARNING>: Card doesn't support stream write\n", host->id);
        return -1;
    }

    MSG(OPS, "[SD%d] Stream write data %d bytes to 0x%x\n", host->id, 
        nblks * blksz, dst);

    MSDC_CLR_FIFO();

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, blksz);
    msdc_set_timeout(host, 100000000, 0);
    
    /* send write command */
    cmd.opcode  = MMC_CMD_WRITE_DAT_UNTIL_STOP;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;
    err = msdc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto done;

	err = msdc_pio_write(host, (u32*)ptr, nblks * blksz);
	if (priv->autocmd == 0) {
        cmd.opcode	= MMC_CMD_STOP_TRANSMISSION;
        cmd.rsptyp	= RESP_R1B; /* for stream read/write, must R1b */
        cmd.arg 	= 0;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
        priv->stream_stop = 1;
        err = msdc_cmd(host, &cmd) != MMC_ERR_NONE ? MMC_ERR_FAILED : err;
        priv->stream_stop = 0;
	}

done:
    if (err != MMC_ERR_NONE) {
       	printf("[SD%d] Stream write data error %d\n", host->id, err);
    }

    return err;
}

void msdc_config_clksrc(struct mmc_host *host, clk_source_t clksrc)
{
    // u32 clks[] = {26000000, 197000000, 208000000, 0};
#if defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD) || defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
    u32 clks[] = {48000000};   /* MT7620 fixs 48M - chhung */
#elif defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) 
    u32 clks[] = {50000000};   /* MT7621 fixs 50M - chhung */
#endif
    u32 tmp;
	clk_source_t tmp_clksrc = clksrc;

#if 0
    MSDC_SET_BIT32(0xc1000010, 1 << (15 + host->id)); /* Enable PDN MSDC */

    /* 2 msdc clock cycles is required before gating. Use 1ms delay to avoid
     * glitch in switching clock source
     */
    mdelay(1);

    tmp = MSDC_READ32(PERI_MSDC_SRCSEL);
    
	switch (host->id){
		case 0 : tmp_clksrc = tmp_clksrc << 0;
				 tmp &= ~0x3;
				 break;
		case 1 : tmp_clksrc = tmp_clksrc << 3;
				 tmp &= ~(0x3 << 3);
				 break;
		case 2 : tmp_clksrc = tmp_clksrc << 5;
				 tmp &= ~(0x3 << 5);
				 break;
		case 3 : tmp_clksrc = tmp_clksrc << 7;
				 tmp &= ~(0x3 << 7);
				 break;
		default: break;
		}
	
    tmp |= tmp_clksrc;
    MSDC_WRITE32(PERI_MSDC_SRCSEL, tmp);
#endif
    
    host->clksrc = clksrc;
    host->clk  = clks[clksrc];

    // MSDC_CLR_BIT32(0xc1000010, 1 << (15 + host->id)); /* Disable PDN MSDC */
}

void msdc_config_clock(struct mmc_host *host, int ddr, u32 hz)
{
    msdc_priv_t *priv = host->priv;
    u32 base = host->base;
    u32 mode;
    u32 div;
    u32 sclk;
    u32 orig_clksrc = host->clksrc;

#if defined (MT7621_FPGA_BOARD) || defined (MT7628_FPGA_BOARD)
    mode = 0x0; /* use divisor */
    if (hz >= (host->clk >> 1)) {
	    div  = 0;			   /* mean div = 1/2 */
	    sclk = host->clk >> 1; /* sclk = clk / 2 */
    } else {
	    div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
	    sclk = (host->clk >> 2) / div;
    }
#else
    if (ddr) {
        mode = 0x2; /* ddr mode and use divisor */
        if (hz >= (host->clk >> 2)) {
            div  = 0;			   /* mean div = 1/2 */
            sclk = host->clk >> 2; /* sclk = clk/div/2. 2: internal divisor */
        } else {
            div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
            sclk = (host->clk >> 2) / div;
            div  = (div >> 1);     /* since there is 1/2 internal divisor */
        }
    } else if (hz >= host->clk) {
        mode = 0x1; /* no divisor and divisor is ignored */
        div  = 0;
        sclk = host->clk; 
    } else {
        mode = 0x0; /* use divisor */
        if (hz >= (host->clk >> 1)) {
            div  = 0;			   /* mean div = 1/2 */
            sclk = host->clk >> 1; /* sclk = clk / 2 */
        } else {
            div  = (host->clk + ((hz << 2) - 1)) / (hz << 2);
            sclk = (host->clk >> 2) / div;
        }
    }
#endif
    host->sclk = sclk;

    if (hz > 100000000 && mmc_card_uhs1(host->card)) {
        MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVN, msdc_cap.clk_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVP, msdc_cap.clk_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVN, msdc_cap.cmd_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVP, msdc_cap.cmd_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVN, msdc_cap.dat_18v_drv);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVP, msdc_cap.dat_18v_drv);
        /* don't enable cksel for ddr mode */
        if (mmc_card_ddr(host->card) == 0)
            MSDC_SET_FIELD(MSDC_PATCH_BIT0,MSDC_CKGEN_MSDC_CK_SEL, 1);
    }

    msdc_config_clksrc(host, MSDC_CLKSRC_NONE);

    /* set clock mode and divisor */
    MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKMOD, mode);
    MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_CKDIV, div);

    msdc_config_clksrc(host, orig_clksrc);

    /* wait clock stable */
    while (!(MSDC_READ32(MSDC_CFG) & MSDC_CFG_CKSTB));

    printf("[SD%d] SET_CLK(%dkHz): SCLK(%dkHz) MODE(%d) DDR(%d) DIV(%d) DS(%d) RS(%d)\n",
        host->id, hz/1000, sclk/1000, mode, ddr > 0 ? 1 : 0, div, priv->dsmpl, priv->rsmpl);
}

void msdc_config_bus(struct mmc_host *host, u32 width)
{
    u32 base = host->base;
    u32 val  = MSDC_READ32(SDC_CFG);

    val &= ~SDC_CFG_BUSWIDTH;

    switch (width) {
    default:
        width = HOST_BUS_WIDTH_1;
    case HOST_BUS_WIDTH_1:
        val |= (MSDC_BUS_1BITS << 16);
        break;
    case HOST_BUS_WIDTH_4:
        val |= (MSDC_BUS_4BITS << 16);
        break;
    case HOST_BUS_WIDTH_8:
        val |= (MSDC_BUS_8BITS << 16);
        break;
    }
    MSDC_WRITE32(SDC_CFG, val);

    printf("[SD%d] Bus Width: %d\n", host->id, width);
}

void msdc_config_pin(struct mmc_host *host, int mode)
{
    u32 base = host->base;

    MSG(CFG, "[SD%d] Pins mode(%d), none(0), down(1), up(2), keep(3)\n", 
        host->id, mode);

    switch (mode) {
    case MSDC_PIN_PULL_UP:
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPU, 1);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPD, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPU, 1);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPD, 0);
        break;
    case MSDC_PIN_PULL_DOWN:
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPD, 1);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPD, 1);
        break;
    case MSDC_PIN_PULL_NONE:
    default:
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDPD, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPU, 0);
        MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATPD, 0);
        break;
    }
}

int msdc_switch_volt(struct mmc_host *host, int volt)
{
    u32 base = host->base;
    int err = MMC_ERR_FAILED;
    u32 timeout = 1000;
    u32 status;
    u32 sclk = host->sclk;

    /* make sure SDC is not busy (TBC) */
    WAIT_COND(!SDC_IS_BUSY(), timeout, timeout);
    if (timeout == 0) {
        err = MMC_ERR_TIMEOUT;
        goto out;
    }

    /* check if CMD/DATA lines both 0 */
    if ((MSDC_READ32(MSDC_PS) & ((1 << 24) | (0xF << 16))) == 0) {

        /* pull up disabled in CMD and DAT[3:0] */
        msdc_config_pin(host, MSDC_PIN_PULL_NONE);

        /* change signal from 3.3v to 1.8v */
        msdc_set_host_level_pwr(1);

        /* wait at least 5ms for 1.8v signal switching in card */
        mdelay(10);

        /* config clock to 10~12MHz mode for volt switch detection by host */
        msdc_config_clock(host, 0, 10000000);

        /* pull up enabled in CMD and DAT[3:0] */
        msdc_config_pin(host, MSDC_PIN_PULL_UP);
        mdelay(5);

        /* start to detect volt change by providing 1.8v signal to card */
        MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_BV18SDT);

        /* wait at max. 1ms */
        mdelay(1);

        while ((status = MSDC_READ32(MSDC_CFG)) & MSDC_CFG_BV18SDT);

        if (status & MSDC_CFG_BV18PSS)
            err = MMC_ERR_NONE;

        /* config clock back to init clk freq. */
        msdc_config_clock(host, 0, sclk);
    }   

out:
        
    return err;
}


void msdc_clock(struct mmc_host *host, int on)
{
#if 0
	if(on)
		hwEnableClock(host->id+MT65XX_PDN_PERI_MSDC0,"MSDC");
	else
		hwDisableClock(host->id+MT65XX_PDN_PERI_MSDC0,"MSDC");
    MSG(CFG, "[SD%d] Turn %s %s clock \n", host->id, on ? "on" : "off", "host");
#endif
}

void msdc_host_power(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "host");

    if (on) {
        msdc_config_pin(host, MSDC_PIN_PULL_UP);      
        msdc_set_host_pwr(1);
        msdc_clock(host, 1);
    } else {
        msdc_clock(host, 0);
        msdc_set_host_pwr(0);
        msdc_config_pin(host, MSDC_PIN_PULL_DOWN);
    }
}

void msdc_card_power(struct mmc_host *host, int on)
{
    MSG(CFG, "[SD%d] Turn %s %s power \n", host->id, on ? "on" : "off", "card");

    if (on) {
        msdc_set_card_pwr(1);
    } else {
        msdc_set_card_pwr(0);
    }
}

void msdc_power(struct mmc_host *host, u8 mode)
{
    if (mode == MMC_POWER_ON || mode == MMC_POWER_UP) {
        msdc_host_power(host, 1);
        msdc_card_power(host, 1);
    } else {
        msdc_card_power(host, 0);
        msdc_host_power(host, 0);
    }
}
#if MSDC_USE_IRQ
void msdc_remove_detected(struct mmc_host *host)
{
	msdc_config_bus(host,HOST_BUS_WIDTH_1);
	msdc_config_clock(host,0,MSDC_MIN_SCLK);
	MSDC_SET_FIELD(PERI_MSDC_SRCSEL,MSDC1_IRQ_SEL,1);
	IRQSensitivity(msdc_irq_line[(host->id)], EDGE_SENSITIVE);
	msdc_power(host,MMC_POWER_OFF);
	host->card_detect_flag = 0;
}
#endif

void msdc_tune_debug(struct mmc_host *host, int enable)
{
    msdc_tune_dbg[host->id] = enable;
}

#if MSDC_USE_CM_TUNING
#if 0
int msdc_tune_cmdrsp(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base = host->base;
    u32 rsmpl, rrdly, sel = 1;
    u32 cur_rsmpl, orig_rsmpl, cur_rrdly, orig_rrdly;
    u32 cksel, cur_cksel, orig_cksel;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel, latch_selcnt;
    u32 times = 0;
    int result = MMC_ERR_FAILED;

    if (mmc_card_uhs1(host->card) && host->sclk > 104000000)
        sel = 2;

    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, orig_cksel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, orig_rrdly);

    for (cksel = 0; cksel < sel; cksel) {
        cur_cksel = (orig_cksel + cksel) % 2;
        if (mmc_card_uhs1(host->card) && host->sclk > 104000000)
            cur_cksel = 1;
        MSDC_SET_FIELD(MSDC_PATCH_BIT0,MSDC_CKGEN_MSDC_CK_SEL, cur_cksel);
        latch_selcnt = (cur_cksel) ? 8 : 1;
        for (dl_cksel = 0; dl_cksel < latch_selcnt; dl_cksel++) {
            cur_dl_cksel = (orig_dl_cksel + dl_cksel) % latch_selcnt;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);

            for (rsmpl = 0; rsmpl < 2; rsmpl++) {
                cur_rsmpl = (orig_rsmpl + rsmpl) % 2;
                MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_rsmpl);
                for (rrdly = 0; rrdly < 32; rrdly++) {
                    cur_rrdly = (orig_rrdly + rrdly + 1) % 32;
                    MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, cur_rrdly);
                    result = msdc_send_cmd(host, cmd);
                    if (result != MMC_ERR_NONE)
                        continue;
                    result = msdc_wait_rsp(host, cmd);

                    /* for debugging */
                    {
                        printf("[SD%d] <TUNE_CMD_%d><%s> CMDRRDLY=%d, RSPL=%dh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", cur_rrdly, cur_rsmpl);
                        printf("[SD%d] <TUNE_CMD_%d><%s> CKGEN_MSDC_CK_SEL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", cur_cksel);
                        printf("[SD%d] <TUNE_CMD_%d><%s> INT_DAT_LATCH_CK_SEL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", cur_dl_cksel);
                    }
                    if (result == MMC_ERR_NONE)                        
                        goto done;
                }
            }
        }
    }
done:    
    return result;
}
#else /* optimized for tuning loop */
int msdc_tune_cmdrsp(struct mmc_host *host, struct mmc_command *cmd)
{
    u32 base = host->base;
    u32 rsmpl, rrdly, sel = 0;
    u32 cur_rsmpl, orig_rsmpl, cur_rrdly, orig_rrdly;
    u32 cksel, cur_cksel, orig_cksel;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel, latch_sel;
    u32 times = 0;
    int result = MMC_ERR_FAILED;

    if (mmc_card_uhs1(host->card) && host->sclk > 104000000)
        sel = 1;

    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, orig_cksel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, orig_rsmpl);
    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, orig_rrdly);

    cksel = 0;
    do {
        cur_cksel = (orig_cksel + cksel) % 2;
        latch_sel = cur_cksel ? 8 : 1;
        dl_cksel = 0;
        do {
            rrdly = 0;
            do {
                for (rsmpl = 0; rsmpl < 2; rsmpl++) {
                    cur_rsmpl = (orig_rsmpl + rsmpl) % 2;
                    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, cur_rsmpl);
                    if (!msdc_tune_dbg[host->id]) {
                        result = msdc_send_cmd(host, cmd);
                        if (result != MMC_ERR_NONE)
                            continue;
                        result = msdc_wait_rsp(host, cmd);                        
                    } else {
                        result = MMC_ERR_BADCRC;
                    }
                    /* for debugging */
                    {
                        u32 t_rrdly, t_rsmpl, t_cksel, t_dl_cksel;

                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, t_cksel);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);
                        MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, t_rsmpl);
                        MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, t_rrdly);

                        times++;
                        printf("[SD%d] <TUNE_CMD><%d><%s> CMDRRDLY=%d, RSPL=%dh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", t_rrdly, t_rsmpl);
                        printf("[SD%d] <TUNE_CMD><%d><%s> CKGEN_MSDC_CK_SEL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", t_cksel);
                        printf("[SD%d] <TUNE_CMD><%d><%s> INT_DAT_LATCH_CK_SEL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE) ?
                            "PASS" : "FAIL", t_dl_cksel);
                    }
                    if (result == MMC_ERR_NONE)                        
                        goto done;
                }    
                cur_rrdly = (orig_rrdly + rrdly + 1) % 32;
                MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, cur_rrdly);
                
            } while (++rrdly < 32);
            /* no need to update data ck sel */
            if (latch_sel == 1 || dl_cksel >= latch_sel)
                break;
        
            cur_dl_cksel = ++orig_dl_cksel % latch_sel;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);
            dl_cksel++;                
        } while(1);
        /* no need to update ck sel */
        if (sel == 1 || cksel >= sel)
            break;

        MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, cur_cksel);
        cksel++;
    } while(1);

done:    
    return result;
}

#endif
#endif

#if MSDC_USE_RD_TUNING
#if 0
int msdc_tune_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    u32 base = host->base;
    u32 i, dcrc, ddr, cksel, dl_cksel, rxdly, sel = 1, orig_dcrc;
    u32 cur_rxdly0, cur_rxdly1;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
    u32 cur_dat0, cur_dat1, cur_dat2, cur_dat3, cur_dat4, cur_dat5, 
        cur_dat6, cur_dat7, cur_cksel, cur_dl_cksel, latch_selcnt;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3, orig_dat4, orig_dat5, 
        orig_dat6, orig_dat7, orig_cksel, orig_dl_cksel;
    u32 times = 0;
    int result = MMC_ERR_FAILED;

    if (mmc_card_uhs1(host->card) && host->sclk > 104000000)
        sel = 2;

    ddr = mmc_card_ddr(host->card);

    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, orig_cksel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);

    /* Tune Method 2. delay each data line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

    for (cksel = 0; cksel < sel; cksel++) {
        cur_cksel = (orig_cksel + cksel) % 2;
        MSDC_SET_FIELD(MSDC_PATCH_BIT0,MSDC_CKGEN_MSDC_CK_SEL, cur_cksel);
        latch_selcnt = (cur_cksel) ? 8 : 1;
        for (dl_cksel = 0; dl_cksel < latch_selcnt; dl_cksel++) {
            cur_dl_cksel = (orig_dl_cksel + dl_cksel) % latch_selcnt;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);

            for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl);

                MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);

                if (!ddr) dcrc &= ~SDC_DCRC_STS_NEG;

                WARN_ON(dcrc == 0);

                /* crc error in this data line */
                for (rxdly = 0; rxdly < 32; rxdly++) {

                    result = host->blk_read(host, dst, src, nblks);
                    
                    MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
                    
                    if (!ddr) dcrc &= ~SDC_DCRC_STS_NEG;

                    { /* for debugging */
                        times++;
                        printf("[SD%d] <TUNE_BREAD_%d><%s> DCRC=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", dcrc);
                        printf("[SD%d] <TUNE_BREAD_%d><%s> DATRDDLY0=%xh, DATRDDLY1=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", MSDC_READ32(MSDC_DAT_RDDLY0), MSDC_READ32(MSDC_DAT_RDDLY1));
                        printf("[SD%d] <TUNE_BREAD_%d><%s> CKGEN_MSDC_CK_SEL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", cur_cksel);
                        printf("[SD%d] <TUNE_BREAD_%d><%s> INT_DAT_LATCH_CK_SEL=%xh, DSMPL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", cur_dl_cksel, cur_dsmpl);
                    }
                    
                    /* no cre error in this data line */
                    if (result == MMC_ERR_NONE && dcrc == 0) {
                        goto done;
                    } else {
                        result = MMC_ERR_BADCRC;
                    }

                    cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
                    cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);

                    #if 0 /* FIXME. E1 ECO. YD: Reverse */
                    orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
                    orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
                    orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
                    orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
                    orig_dat4 = (cur_rxdly1 >> 24) & 0x1F;
                    orig_dat5 = (cur_rxdly1 >> 16) & 0x1F;
                    orig_dat6 = (cur_rxdly1 >> 8) & 0x1F;
                    orig_dat7 = (cur_rxdly1 >> 0) & 0x1F;
                    #else
                    orig_dat0 = (cur_rxdly0 >> 0) & 0x1F;
                    orig_dat1 = (cur_rxdly0 >> 8) & 0x1F;
                    orig_dat2 = (cur_rxdly0 >> 16) & 0x1F;
                    orig_dat3 = (cur_rxdly0 >> 24) & 0x1F;
                    orig_dat4 = (cur_rxdly1 >> 0) & 0x1F;
                    orig_dat5 = (cur_rxdly1 >> 8) & 0x1F;
                    orig_dat6 = (cur_rxdly1 >> 16) & 0x1F;
                    orig_dat7 = (cur_rxdly1 >> 24) & 0x1F;
                    #endif
                    if (ddr) {
                        cur_dat0 = (dcrc & (1 << 0) || dcrc & (1 << 8)) ? ((orig_dat0 + 1) % 32) : orig_dat0;
                        cur_dat1 = (dcrc & (1 << 1) || dcrc & (1 << 9)) ? ((orig_dat1 + 1) % 32) : orig_dat1;
                        cur_dat2 = (dcrc & (1 << 2) || dcrc & (1 << 10)) ? ((orig_dat2 + 1) % 32) : orig_dat2;
                        cur_dat3 = (dcrc & (1 << 3) || dcrc & (1 << 11)) ? ((orig_dat3 + 1) % 32) : orig_dat3;
                    } else {
                        cur_dat0 = (dcrc & (1 << 0)) ? ((orig_dat0 + 1) % 32) : orig_dat0;
                        cur_dat1 = (dcrc & (1 << 1)) ? ((orig_dat1 + 1) % 32) : orig_dat1;
                        cur_dat2 = (dcrc & (1 << 2)) ? ((orig_dat2 + 1) % 32) : orig_dat2;
                        cur_dat3 = (dcrc & (1 << 3)) ? ((orig_dat3 + 1) % 32) : orig_dat3;
                    }
                    cur_dat4 = (dcrc & (1 << 4)) ? ((orig_dat4 + 1) % 32) : orig_dat4;
                    cur_dat5 = (dcrc & (1 << 5)) ? ((orig_dat5 + 1) % 32) : orig_dat5;
                    cur_dat6 = (dcrc & (1 << 6)) ? ((orig_dat6 + 1) % 32) : orig_dat6;
                    cur_dat7 = (dcrc & (1 << 7)) ? ((orig_dat7 + 1) % 32) : orig_dat7;

                    cur_rxdly0 = (cur_dat0 << 24) | (cur_dat1 << 16) |
                        (cur_dat2 << 8) | (cur_dat3 << 0);
                    cur_rxdly1 = (cur_dat4 << 24) | (cur_dat5 << 16) |
                        (cur_dat6 << 8) | (cur_dat7 << 0);

                    MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
                    MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1);
                }
            }
        }
    }
done:

    return result;
}
#else /* optimized for tuning loop */
int msdc_tune_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    u32 base = host->base;
    u32 i, dcrc, ddr, cksel, dl_cksel, rxdly, sel = 0, orig_dcrc;
    u32 cur_rxdly0, cur_rxdly1;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
    u32 cur_dat0, cur_dat1, cur_dat2, cur_dat3, cur_dat4, cur_dat5, 
        cur_dat6, cur_dat7, cur_cksel, cur_dl_cksel, latch_sel;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3, orig_dat4, orig_dat5, 
        orig_dat6, orig_dat7, orig_cksel, orig_dl_cksel;
    u32 times = 0;
    int result = MMC_ERR_FAILED;

    if (mmc_card_uhs1(host->card) && host->sclk > 104000000)
        sel = 1;

    ddr = mmc_card_ddr(host->card);

    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, orig_cksel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);

    /* Tune Method 2. delay each data line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

    cksel = 0;
    do {
        cur_cksel = (orig_cksel + cksel) % 2;
        latch_sel = cur_cksel ? 8 : 1;
        dl_cksel = 0;
        do {
            rxdly = 0;
            do {
                for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                    cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl);

                    if (!msdc_tune_dbg[host->id]) {
                        result = host->blk_read(host, dst, src, nblks);

                        MSDC_GET_FIELD(SDC_DCRC_STS, SDC_DCRC_STS_POS|SDC_DCRC_STS_NEG, dcrc);
                    } else {
                        result = MMC_ERR_BADCRC;
                        dcrc = 0xFFF;
                    }
                    
                    if (!ddr) dcrc &= ~SDC_DCRC_STS_NEG;

                    /* for debugging */
                    {
                        u32 t_dspl, t_cksel, t_dl_cksel;

                        MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, t_dspl);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, t_cksel);
                        MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

                        times++;
                        printf("[SD%d] <TUNE_BREAD_%d><%s> DCRC=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", dcrc);
                        printf("[SD%d] <TUNE_BREAD_%d><%s> DATRDDLY0=%xh, DATRDDLY1=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", MSDC_READ32(MSDC_DAT_RDDLY0), MSDC_READ32(MSDC_DAT_RDDLY1));
                        printf("[SD%d] <TUNE_BREAD_%d><%s> CKGEN_MSDC_CK_SEL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", t_cksel);
                        printf("[SD%d] <TUNE_BREAD_%d><%s> INT_DAT_LATCH_CK_SEL=%xh, DSMPL=%xh\n",
                            host->id, times, (result == MMC_ERR_NONE && dcrc == 0) ?
                            "PASS" : "FAIL", t_dl_cksel, t_dspl);
                    }
                    
                    /* no cre error in this data line */
                    if (result == MMC_ERR_NONE && dcrc == 0) {
                        goto done;
                    } else {
                        result = MMC_ERR_BADCRC;
                    }      
                }
                cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
                cur_rxdly1 = MSDC_READ32(MSDC_DAT_RDDLY1);

                #if 1 /* FIXME. E1 ECO. YD: Reverse */
                orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
                orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
                orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
                orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
                orig_dat4 = (cur_rxdly1 >> 24) & 0x1F;
                orig_dat5 = (cur_rxdly1 >> 16) & 0x1F;
                orig_dat6 = (cur_rxdly1 >> 8) & 0x1F;
                orig_dat7 = (cur_rxdly1 >> 0) & 0x1F;
                #else
                orig_dat0 = (cur_rxdly0 >> 0) & 0x1F;
                orig_dat1 = (cur_rxdly0 >> 8) & 0x1F;
                orig_dat2 = (cur_rxdly0 >> 16) & 0x1F;
                orig_dat3 = (cur_rxdly0 >> 24) & 0x1F;
                orig_dat4 = (cur_rxdly1 >> 0) & 0x1F;
                orig_dat5 = (cur_rxdly1 >> 8) & 0x1F;
                orig_dat6 = (cur_rxdly1 >> 16) & 0x1F;
                orig_dat7 = (cur_rxdly1 >> 24) & 0x1F;
                #endif
                if (ddr) {
                    cur_dat0 = (dcrc & (1 << 0) || dcrc & (1 << 8)) ? ((orig_dat0 + 1) % 32) : orig_dat0;
                    cur_dat1 = (dcrc & (1 << 1) || dcrc & (1 << 9)) ? ((orig_dat1 + 1) % 32) : orig_dat1;
                    cur_dat2 = (dcrc & (1 << 2) || dcrc & (1 << 10)) ? ((orig_dat2 + 1) % 32) : orig_dat2;
                    cur_dat3 = (dcrc & (1 << 3) || dcrc & (1 << 11)) ? ((orig_dat3 + 1) % 32) : orig_dat3;
                } else {
                    cur_dat0 = (dcrc & (1 << 0)) ? ((orig_dat0 + 1) % 32) : orig_dat0;
                    cur_dat1 = (dcrc & (1 << 1)) ? ((orig_dat1 + 1) % 32) : orig_dat1;
                    cur_dat2 = (dcrc & (1 << 2)) ? ((orig_dat2 + 1) % 32) : orig_dat2;
                    cur_dat3 = (dcrc & (1 << 3)) ? ((orig_dat3 + 1) % 32) : orig_dat3;
                }
                cur_dat4 = (dcrc & (1 << 4)) ? ((orig_dat4 + 1) % 32) : orig_dat4;
                cur_dat5 = (dcrc & (1 << 5)) ? ((orig_dat5 + 1) % 32) : orig_dat5;
                cur_dat6 = (dcrc & (1 << 6)) ? ((orig_dat6 + 1) % 32) : orig_dat6;
                cur_dat7 = (dcrc & (1 << 7)) ? ((orig_dat7 + 1) % 32) : orig_dat7;

                cur_rxdly0 = (cur_dat0 << 24) | (cur_dat1 << 16) |
                    (cur_dat2 << 8) | (cur_dat3 << 0);
                cur_rxdly1 = (cur_dat4 << 24) | (cur_dat5 << 16) |
                    (cur_dat6 << 8) | (cur_dat7 << 0);

                MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
                MSDC_WRITE32(MSDC_DAT_RDDLY1, cur_rxdly1);            
            } while (++rxdly < 32);

            /* no need to update data ck sel */
            if (latch_sel == 1 || dl_cksel >= latch_sel)
                break;
        
            cur_dl_cksel = ++orig_dl_cksel % latch_sel;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);
            dl_cksel++;                
        } while(1);
        /* no need to update ck sel */
        if (sel == 1 || cksel >= sel)
            break;

        MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, cur_cksel);
        cksel++;
    } while(1);

done:

    return result;
}
#endif
#endif

#if MSDC_USE_WR_TUNING
#if 0
int msdc_tune_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    u32 base = host->base;
    u32 cksel, cur_cksel, orig_cksel;
    u32 wrrdly, cur_wrrdly, orig_wrrdly;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel;
    u32 rxdly, cur_rxdly0;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3;
    u32 cur_dat0, cur_dat1, cur_dat2, cur_dat3;
    u32 cur_ddrdly, orig_ddrdly, ddrdly;
    u32 latch_sel, sel = 1, ddrckdly = 1;
    u32 status;
    int result = MMC_ERR_FAILED;  

    if (mmc_card_uhs1(host->card) && host->sclk > 104000000)
        sel = 2;

    if (mmc_card_ddr(host->card))
        ddrckdly = 2;

    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, orig_wrrdly);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DDR50CKD, orig_ddrdly);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, orig_cksel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

    /* Tune Method 2. delay data0 line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

    for (ddrdly = 0; ddrdly < ddrckdly; ddrdly++) {
        cur_ddrdly = (orig_ddrdly + ddrdly) % 2;
        MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDR50CKD, cur_ddrdly);

        for (cksel = 0; cksel < sel; cksel++) {
            cur_cksel = (orig_cksel + cksel) % 2;
            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, cur_cksel);
            latch_sel = cur_cksel ? 8 : 1;

            for (dl_cksel = 0; dl_cksel < latch_sel; dl_cksel++) {
                cur_dl_cksel = (orig_dl_cksel + dl_cksel) % latch_sel;
                MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);

                cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);
                
                #if 0 /* FIXME. E1 ECO. YD: Reverse */
                orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
                orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
                orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
                orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
                #else
                orig_dat0 = (cur_rxdly0 >> 0) & 0x1F;
                orig_dat1 = (cur_rxdly0 >> 8) & 0x1F;
                orig_dat2 = (cur_rxdly0 >> 16) & 0x1F;
                orig_dat3 = (cur_rxdly0 >> 24) & 0x1F;
                #endif

                /* crc error in this data line */
                for (rxdly = 0; rxdly < 32; rxdly++) {
                    cur_dat0 = (orig_dat0 + rxdly) % 32; /* only adjust bit-1 for crc */
                    cur_dat1 = orig_dat1;
                    cur_dat2 = orig_dat2;
                    cur_dat3 = orig_dat3;                    

                    cur_rxdly0 = (cur_dat0 << 24) | (cur_dat1 << 16) |
                        (cur_dat2 << 8) | (cur_dat3 << 0);
                    
                    MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);

                    for (wrrdly = 0; wrrdly < 32; wrrdly++) {
                        cur_wrrdly = (orig_wrrdly + wrrdly) % 32;
                        MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, cur_wrrdly);

                        for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                            cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl);

                            result = host->blk_write(host, dst, src, nblks);

                            printf("[SD%d] <TUNE_BWRITE><%s> DSPL=%d, DATWRDLY=%d\n",
                                host->id, result == MMC_ERR_NONE ? "PASS" : "FAIL", 
                                cur_dsmpl, cur_wrrdly);
                            printf("[SD%d] <TUNE_BWRITE><%s> DDR50CKD=%xh\n",
                                host->id, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                cur_ddrdly);
                            printf("[SD%d] <TUNE_BWRITE><%s> CKGEN_MSDC_CK_SEL=%xh\n",
                                host->id, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                cur_cksel);
                            printf("[SD%d] <TUNE_BWRITE><%s> INT_DAT_LATCH_CK_SEL=%xh\n",
                                host->id, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                cur_dl_cksel);
                            printf("[SD%d] <TUNE_BWRITE><%s> MSDC_DAT_RDDLY0=%xh\n",
                                host->id, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                cur_rxdly0);

                            if (result == MMC_ERR_NONE) {
                                goto done;
                            }
                        }
                    }
                }            
            }
        }
    }

done:

    return result;
}
#else /* optimized tuning loop */
int msdc_tune_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    u32 base = host->base;
    u32 cksel, cur_cksel, orig_cksel;
    u32 wrrdly, cur_wrrdly, orig_wrrdly;
    u32 dsmpl, cur_dsmpl, orig_dsmpl;
    u32 dl_cksel, cur_dl_cksel, orig_dl_cksel;
    u32 rxdly, cur_rxdly0;
    u32 orig_dat0, orig_dat1, orig_dat2, orig_dat3;
    u32 cur_dat0, cur_dat1, cur_dat2, cur_dat3;
    u32 cur_ddrdly, orig_ddrdly, ddrdly;
    u32 latch_sel, sel = 0, ddrckdly = 0;
    u32 times = 0;
    u32 status;
    int result = MMC_ERR_FAILED;  

    if (mmc_card_uhs1(host->card) && host->sclk > 104000000)
        sel = 1;

    if (mmc_card_ddr(host->card))
        ddrckdly = 1;

    MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, orig_wrrdly);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, orig_dsmpl);
    MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DDR50CKD, orig_ddrdly);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, orig_cksel);
    MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, orig_dl_cksel);

    /* Tune Method 2. delay data0 line */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDLSEL, 1);

    cur_rxdly0 = MSDC_READ32(MSDC_DAT_RDDLY0);

#if 1 /* FIXME. E1 ECO. YD: Reverse */
    orig_dat0 = (cur_rxdly0 >> 24) & 0x1F;
    orig_dat1 = (cur_rxdly0 >> 16) & 0x1F;
    orig_dat2 = (cur_rxdly0 >> 8) & 0x1F;
    orig_dat3 = (cur_rxdly0 >> 0) & 0x1F;
#else
    orig_dat0 = (cur_rxdly0 >> 0) & 0x1F;
    orig_dat1 = (cur_rxdly0 >> 8) & 0x1F;
    orig_dat2 = (cur_rxdly0 >> 16) & 0x1F;
    orig_dat3 = (cur_rxdly0 >> 24) & 0x1F;
#endif

    ddrdly = 0;
    do {
        cksel = 0;
        do {
            cur_cksel = (orig_cksel + cksel) % 2;
            latch_sel = cur_cksel ? 8 : 1;
            dl_cksel = 0;
            do {
                rxdly = 0;
                do {
                    wrrdly = 0;
                    do {
                        for (dsmpl = 0; dsmpl < 2; dsmpl++) {
                            cur_dsmpl = (orig_dsmpl + dsmpl) % 2;
                            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, cur_dsmpl);

                            if (!msdc_tune_dbg[host->id]) {
                                result = host->blk_write(host, dst, src, nblks);
                            } else {
                                result = MMC_ERR_BADCRC;
                            }

                            /* for debugging */
                            {
                                u32 t_dspl, t_wrrdly, t_ddrdly, t_cksel, t_dl_cksel;

                                MSDC_GET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, t_wrrdly);
                                MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, t_dspl);
                                MSDC_GET_FIELD(MSDC_IOCON, MSDC_IOCON_DDR50CKD, t_ddrdly);
                                MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, t_cksel);
                                MSDC_GET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, t_dl_cksel);

                                times++;

                                printf("[SD%d] <TUNE_BWRITE_%d><%s> DSPL=%d, DATWRDLY=%d\n",
                                    host->id, times, result == MMC_ERR_NONE ? "PASS" : "FAIL", 
                                    t_dspl, t_wrrdly);
                                printf("[SD%d] <TUNE_BWRITE_%d><%s> DDR50CKD=%xh\n",
                                    host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                    t_ddrdly);
                                printf("[SD%d] <TUNE_BWRITE_%d><%s> CKGEN_MSDC_CK_SEL=%xh\n",
                                    host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                    t_cksel);
                                printf("[SD%d] <TUNE_BWRITE_%d><%s> INT_DAT_LATCH_CK_SEL=%xh\n",
                                    host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                    t_dl_cksel);
                                printf("[SD%d] <TUNE_BWRITE_%d><%s> MSDC_DAT_RDDLY0=%xh\n",
                                    host->id, times, (result == MMC_ERR_NONE) ? "PASS" : "FAIL", 
                                    MSDC_READ32(MSDC_DAT_RDDLY0));
                            }
                            if (result == MMC_ERR_NONE) {
                                goto done;
                            }
                        }
                        cur_wrrdly = ++orig_wrrdly % 32;
                        MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, cur_wrrdly);
                    } while (++wrrdly < 32);
                    
                    cur_dat0 = ++orig_dat0 % 32; /* only adjust bit-1 for crc */
                    cur_dat1 = orig_dat1;
                    cur_dat2 = orig_dat2;
                    cur_dat3 = orig_dat3;

                    cur_rxdly0 = (cur_dat0 << 24) | (cur_dat1 << 16) |
                        (cur_dat2 << 8) | (cur_dat3 << 0);
                    
                    MSDC_WRITE32(MSDC_DAT_RDDLY0, cur_rxdly0);
                } while (++rxdly < 32);

                /* no need to update data ck sel */
                if (latch_sel == 1 || dl_cksel >= latch_sel)
                    break;

                cur_dl_cksel = ++orig_dl_cksel % latch_sel;
                MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_INT_DAT_LATCH_CK_SEL, cur_dl_cksel);
                dl_cksel++;                
            } while (1);

            /* no need to update ck sel */
            if (sel == 1 || cksel >= sel)
                break;

            MSDC_SET_FIELD(MSDC_PATCH_BIT0, MSDC_CKGEN_MSDC_CK_SEL, cur_cksel);
            cksel++;
        } while (1);

        /* no need to update ddr ck dly */
        if (ddrckdly == 0 || ddrdly >= ddrckdly)
            break;

        cur_ddrdly = ++orig_ddrdly % 2;
        MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DDR50CKD, cur_ddrdly);
        ddrdly++;
    } while (1);

done:

    return result;
}
#endif

#endif

int msdc_tune_uhs1(struct mmc_host *host, struct mmc_card *card)
{
    u32 base = host->base;
    u32 status;
    int i;
    int err = MMC_ERR_FAILED;
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SEND_TUNING_BLOCK;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = 0xFFFFFFFF;

    msdc_set_timeout(host, 100000000, 0);
    msdc_set_autocmd(host, MSDC_AUTOCMD19, 1);

    for (i = 0; i < 13; i++) {
        /* Note. select a pad to be tuned. msdc only tries 32 times to tune the 
         * pad since there is only 32 tuning steps for a pad.
         */
        MSDC_SET_FIELD(SDC_ACMD19_TRG, SDC_ACMD19_TRG_TUNESEL, i);

        /* Note. autocmd19 will only trigger done interrupt and won't trigger
         * autocmd timeout and crc error interrupt. (autocmd19 is a special command
         * and is different from autocmd12 and autocmd23.
         */
        err = msdc_cmd(host, &cmd);
        if (err != MMC_ERR_NONE)
            goto out;

        /* read and check acmd19 sts. bit-1: success, bit-0: fail */
        status = MSDC_READ32(SDC_ACMD19_STS);

        if (!status) {
            printf("[SD%d] ACMD19_TRG(%d), STS(0x%x) Failed\n", host->id, i,
                status);
            err = MMC_ERR_FAILED;
            goto out;
        }
    }
    err = MMC_ERR_NONE;
    
out:
    msdc_set_autocmd(host, MSDC_AUTOCMD19, 0);
    return err;
}

void msdc_card_detect(struct mmc_host *host, int on)
{
    u32 base = host->base;
    
    if ((msdc_cap.flags & MSDC_CD_PIN_EN) == 0) {
        MSDC_CARD_DETECTION_OFF();
        return;
    }

    if (on) {
        MSDC_SET_FIELD(MSDC_PS, MSDC_PS_CDDEBOUNCE, DEFAULT_DEBOUNCE);   
        MSDC_CARD_DETECTION_ON();
    } else {
        MSDC_CARD_DETECTION_OFF();
        MSDC_SET_FIELD(MSDC_PS, MSDC_PS_CDDEBOUNCE, 0);
    }
}

int msdc_card_avail(struct mmc_host *host)
{
    u32 base = host->base;
    int sts, avail = 0;

    if ((msdc_cap.flags & MSDC_REMOVABLE) == 0)
        return 1;

    if (msdc_cap.flags & MSDC_CD_PIN_EN) {
        MSDC_GET_FIELD(MSDC_PS, MSDC_PS_CDSTS, sts);
        avail = sts == 0 ? 1 : 0;        
    }

    return avail;
}

int msdc_card_protected(struct mmc_host *host)
{
    u32 base = host->base;
    int prot;

    if (msdc_cap.flags & MSDC_WP_PIN_EN) {
        MSDC_GET_FIELD(MSDC_PS, MSDC_PS_WP, prot);
    } else {
        prot = 0;
    }

    return prot;
}

void msdc_hard_reset(void)
{
    msdc_set_card_pwr(0);
    msdc_set_card_pwr(1);
}

void msdc_soft_reset(struct mmc_host *host)
{
    u32 base = host->base;
    u32 tmo = 0x0000ffff;

    MSDC_RESET();
    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP, 1);
    WAIT_COND((MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS) == 0, 0xFFFF, tmo);

    if (tmo == 0) {
        MSG(DMA, "[SD%d] MSDC_DMA_CFG_STS != inactive\n", host->id);
    }
    MSDC_CLR_FIFO();
}

int msdc_emmc_boot_start(struct mmc_host *host, u32 hz, int ddr, int mode, int ackdis)
{
    int err = MMC_ERR_NONE;
    u32 sts;
    u32 base = host->base;
    u32 tmo = 0xFFFFFFFF;
    u32 acktmo, dattmo;

    /* configure msdc and clock frequency */    
    MSDC_RESET();
    MSDC_CLR_FIFO();    
    msdc_set_blklen(host, 512);
    msdc_config_bus(host, HOST_BUS_WIDTH_1);
    msdc_config_clock(host, ddr, hz);

    /* requires 74 clocks/1ms before CMD0 */
    MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);
    mdelay(2);
    MSDC_CLR_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);

    /* configure boot timeout value */
    WAIT_COND(SDC_IS_BUSY() == 0, tmo, tmo);

    acktmo = msdc_cal_timeout(host, 50 * 1000 * 1000, 0, 256);   /* 50ms */
    dattmo = msdc_cal_timeout(host, 1000 * 1000 * 1000, 0, 256); /* 1sec */

    acktmo = acktmo > 0xFFF ? 0xFFF : acktmo;
    dattmo = dattmo > 0xFFFFF ? 0xFFFFF : dattmo;

    printf("[SD%d] EMMC BOOT ACK timeout: %d ms (clkcnt: %d)\n", host->id, 
        (acktmo * 256) / (host->sclk / 1000), acktmo);
    printf("[SD%d] EMMC BOOT DAT timeout: %d ms (clkcnt: %d)\n", host->id,
        (dattmo * 256) / (host->sclk / 1000), dattmo);
    
    MSDC_SET_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSUPP);
    MSDC_SET_FIELD(EMMC_CFG0, EMMC_CFG0_BOOTACKDIS, ackdis);
    MSDC_SET_FIELD(EMMC_CFG0, EMMC_CFG0_BOOTMODE, mode);
    MSDC_SET_FIELD(EMMC_CFG1, EMMC_CFG1_BOOTACKTMC, acktmo);
    MSDC_SET_FIELD(EMMC_CFG1, EMMC_CFG1_BOOTDATTMC, dattmo);

    if (mode == EMMC_BOOT_RST_CMD_MODE) {
        MSDC_WRITE32(SDC_ARG, 0xFFFFFFFA);
    } else {
        MSDC_WRITE32(SDC_ARG, 0);
    }
    MSDC_WRITE32(SDC_CMD, 0x02001000); /* multiple block read */
    MSDC_SET_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSTART);

    WAIT_COND((MSDC_READ32(EMMC_STS) & EMMC_STS_BOOTUPSTATE) == 
        EMMC_STS_BOOTUPSTATE, tmo, tmo);

    if (!ackdis) {
        do {
            sts = MSDC_READ32(EMMC_STS);
            if (sts == 0)
                continue;
            MSDC_WRITE32(EMMC_STS, sts);    /* write 1 to clear */
            if (sts & EMMC_STS_BOOTACKRCV) {
                printf("[SD%d] EMMC_STS(%x): boot ack received\n", host->id, sts);
                break;
            } else if (sts & EMMC_STS_BOOTACKERR) {
                printf("[SD%d] EMMC_STS(%x): boot up ack error\n", host->id, sts);
                err = MMC_ERR_BADCRC;
                goto out;
            } else if (sts & EMMC_STS_BOOTACKTMO) {
                printf("[SD%d] EMMC_STS(%x): boot up ack timeout\n", host->id, sts);
                err = MMC_ERR_TIMEOUT;
                goto out;
            } else if (sts & EMMC_STS_BOOTUPSTATE) {
                printf("[SD%d] EMMC_STS(%x): boot up mode state\n", host->id, sts);
            } else {
                printf("[SD%d] EMMC_STS(%x): boot up unexpected\n", host->id, sts);
            }
        } while (1);
    }

    /* check if data received */
    do {
        sts = MSDC_READ32(EMMC_STS);
        if (sts == 0)
            continue;
        if (sts & EMMC_STS_BOOTDATRCV) {
            printf("[SD%d] EMMC_STS(%x): boot dat received\n", host->id, sts);
            break;
        }
        if (sts & EMMC_STS_BOOTCRCERR) {
            err = MMC_ERR_BADCRC;
            goto out;
        } else if (sts & EMMC_STS_BOOTDATTMO) {
            err = MMC_ERR_TIMEOUT;
            goto out;
        }
    } while(1);
out:
    return err;    
}

void msdc_emmc_boot_stop(struct mmc_host *host, int mode)
{
    u32 base = host->base;
    u32 tmo = 0xFFFFFFFF;

    /* Step5. stop the boot mode */
    MSDC_WRITE32(SDC_ARG, 0x00000000);
    MSDC_WRITE32(SDC_CMD, 0x00001000);

    MSDC_SET_FIELD(EMMC_CFG0, EMMC_CFG0_BOOTWDLY, 2);
    MSDC_SET_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSTOP);
    WAIT_COND((MSDC_READ32(EMMC_STS) & EMMC_STS_BOOTUPSTATE) == 0, tmo, tmo);
    
    /* Step6. */
    MSDC_CLR_BIT32(EMMC_CFG0, EMMC_CFG0_BOOTSUPP);

    /* Step7. clear EMMC_STS bits */
    MSDC_WRITE32(EMMC_STS, MSDC_READ32(EMMC_STS));
}

int msdc_emmc_boot_read(struct mmc_host *host, u32 size, u32 *to)
{
    int err = MMC_ERR_NONE;
    u32 sts;
    u32 totalsz = size;
    u32 base = host->base;
  
    while (size) {
        sts = MSDC_READ32(EMMC_STS);
        if (sts & EMMC_STS_BOOTCRCERR) {
            err = MMC_ERR_BADCRC;
            goto out;
        } else if (sts & EMMC_STS_BOOTDATTMO) {
            err = MMC_ERR_TIMEOUT;
            goto out;
        }
        /* Note. RXFIFO count would be aligned to 4-bytes alignment size */
        if ((size >=  MSDC_FIFO_THD) && (MSDC_RXFIFOCNT() >= MSDC_FIFO_THD)) {
            int left = MSDC_FIFO_THD >> 2;
            do {
                *to++ = MSDC_FIFO_READ32();
            } while (--left);
            size -= MSDC_FIFO_THD;
            MSG(FIO, "[SD%d] Read %d bytes, RXFIFOCNT: %d,  Left: %d/%d\n",
                host->id, MSDC_FIFO_THD, MSDC_RXFIFOCNT(), size, totalsz);
        } else if ((size < MSDC_FIFO_THD) && MSDC_RXFIFOCNT() >= size) {
            while (size) {
                if (size > 3) {
                    *to++ = MSDC_FIFO_READ32();
                    size -= 4;
                } else {
                    u32 val = MSDC_FIFO_READ32();
                    memcpy(to, &val, size);
                    size = 0;
                }
            }
            MSG(FIO, "[SD%d] Read left bytes, RXFIFOCNT: %d, Left: %d/%d\n",
                host->id, MSDC_RXFIFOCNT(), size, totalsz);
        }        
    }

out:
    if (err) {
        printf("[SD%d] EMMC_BOOT: read boot code fail(%d), FIFOCNT=%d\n", 
            host->id, err, MSDC_RXFIFOCNT());
    }
    return err;
}

void msdc_emmc_boot_reset(struct mmc_host *host, int reset)
{
    u32 base = host->base;

    switch (reset) {
    case EMMC_BOOT_PWR_RESET:
        msdc_hard_reset();
        break;
    case EMMC_BOOT_RST_N_SIG:
        if (msdc_cap.flags & MSDC_RST_PIN_EN) {
            /* set n_reset pin to low */
            MSDC_SET_BIT32(EMMC_IOCON, EMMC_IOCON_BOOTRST);

            /* tRSTW (RST_n pulse width) at least 1us */
            mdelay(1);

            /* set n_reset pin to high */
            MSDC_CLR_BIT32(EMMC_IOCON, EMMC_IOCON_BOOTRST);
            MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);
            /* tRSCA (RST_n to command time) at least 200us, 
               tRSTH (RST_n high period) at least 1us */
            mdelay(1);
            MSDC_CLR_BIT32(MSDC_CFG, MSDC_CFG_CKPDN);
        }
        break;
    case EMMC_BOOT_PRE_IDLE_CMD:
        /* bring emmc to pre-idle mode by software reset command. (MMCv4.41)*/
        SDC_SEND_CMD(0x0, 0xF0F0F0F0);   
        break;
    }
}

int msdc_init(int id, struct mmc_host *host, int clksrc, int mode)
{
    //u32 baddr[] = {MSDC0_BASE, MSDC1_BASE, MSDC2_BASE, MSDC3_BASE};
    u32 baddr[] = {MSDC0_BASE};   /* RT6352's MSDC base address: 0x10130000 - chhung */
    u32 base = baddr[id];
    gpd_t *gpd;
    msdc_bd_t *bd;
    msdc_priv_t *priv;
    struct dma_config *cfg;
#if MSDC_USE_IRQ
    static irq_handler_t isr[] = {
        msdc_irq_handler_0,
        msdc_irq_handler_1,    
        msdc_irq_handler_2,
        msdc_irq_handler_3
    };
#endif

    clksrc = (clksrc == -1) ? msdc_cap.clk_src : clksrc;

#if MSDC_USE_SDXC_FPGA
    {
        /* CHECKME. MSDC_INT.SDIOIRQ is set if this has been set */
        static int sdxc_switch = 0;
        if (sdxc_switch == 0) {
            MSDC_WRITE32(GPO_BASE, 0x1);
            sdxc_switch = 1;
        }
    }
#endif

    gpd  = &msdc_gpd_pool[id][0];
    bd   = &msdc_bd_pool[id][0];
    priv = &msdc_priv[id];
    cfg  = &priv->cfg;

#if MSDC_DEBUG
    msdc_reg[id] = (struct msdc_regs*)base;
#endif

    memset(gpd, 0, sizeof(gpd_t) * MAX_GPD_POOL_SZ);
    memset(bd, 0, sizeof(msdc_bd_t) * MAX_BD_POOL_SZ);
    memset(priv, 0, sizeof(msdc_priv_t));

    host->id     = id;
    host->base   = base;
    host->f_max  = MSDC_MAX_SCLK;
    host->f_min  = MSDC_MIN_SCLK;
    host->blkbits= MMC_BLOCK_BITS;
    host->blklen = 0;
    host->priv   = (void*)priv;
        
    host->caps   = MMC_CAP_MULTIWRITE;

    if (msdc_cap.flags & MSDC_HIGHSPEED)
        host->caps |= (MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED);
    if (msdc_cap.flags & MSDC_UHS1)
        host->caps |= MMC_CAP_SD_UHS1;
    if (msdc_cap.flags & MSDC_DDR)
        host->caps |= MMC_CAP_DDR;
    if (msdc_cap.data_pins == 4)
        host->caps |= MMC_CAP_4_BIT_DATA;
    if (msdc_cap.data_pins == 8)
        host->caps |= MMC_CAP_8_BIT_DATA;

    host->ocr_avail = MMC_VDD_33_34;  /* TODO: To be customized */
    //host->ocr_avail = MMC_VDD_29_30;  /* TODO: To be customized */
    //host->ocr_avail = MMC_VDD_19_20;  /* TODO: To be customized */

    host->max_hw_segs   = MAX_DMA_TRAN_SIZE/512;
    host->max_phys_segs = MAX_DMA_TRAN_SIZE/512;
    host->max_seg_size  = MAX_DMA_TRAN_SIZE;
    host->max_blk_size  = 2048;
    host->max_blk_count = 65535;
	host->card_detect_flag = 1;

    priv->alloc_bd    = 0;
    priv->alloc_gpd   = 0;
    priv->bd_pool     = (msdc_bd_t *) PHYSADDR(bd); 
    priv->gpd_pool    = (gpd_t *) PHYSADDR(gpd);
    priv->active_head = NULL;
    priv->active_tail = NULL;
    priv->dsmpl       = msdc_cap.data_edge;
    priv->rsmpl       = msdc_cap.cmd_edge;

    cfg->sg      = &priv->sg[0];
    cfg->burstsz = MSDC_BRUST_64B;
    cfg->flags   = DMA_FLAG_NONE;
    cfg->mode    = mode;

#if MSDC_USE_IRQ
    msdc_irq_sts[id]  = 0;
    sdio_irq_handler[id] = NULL;
    REGISTER_MSDC_IRQ_HANDLER(id, isr[id], host);
#endif
	// msdc_power(host, MMC_POWER_OFF);
	// msdc_power(host, MMC_POWER_ON);
	MSDC_SET_FIELD(MSDC_CFG, MSDC_CFG_MODE, MSDC_SDMMC);
    MSDC_RESET();
    MSDC_CLR_FIFO();
    MSDC_CLR_INT();

    /* reset tuning parameter */
    MSDC_WRITE32(MSDC_PAD_CTL0, 0x0090000);
    MSDC_WRITE32(MSDC_PAD_CTL1, 0x00A0000);
    MSDC_WRITE32(MSDC_PAD_CTL2, 0x00A0000);
    //MSDC_WRITE32(MSDC_PAD_TUNE, 0x00000000);
    MSDC_WRITE32(MSDC_PAD_TUNE, 0x84101010);		// for MT7620E2 and afterward
    // MSDC_WRITE32(MSDC_DAT_RDDLY0, 0x00000000);
    MSDC_WRITE32(MSDC_DAT_RDDLY0, 0x10101010);		// for MT7620E2 and afterward
     MSDC_WRITE32(MSDC_DAT_RDDLY1, 0x00000000);
    MSDC_WRITE32(MSDC_IOCON, 0x00000000);
#if 0 // use MT7620 default value: 0x403c004f
    MSDC_WRITE32(MSDC_PATCH_BIT0, 0x003C000F);
#endif
	MSDC_WRITE32(MSDC_PATCH_BIT1, 0x00000011);
					//MSDC_PATCH_BIT1YD:WRDAT_CRCS_TA_CNTR need fix to 3'002 by default,(<50MHz) (>=50MHz set 3'002 as initial value is OK for tunning)
					//YD:CMD_RSP_TA_CNTR need fix to 3'001 by default(<50MHz)(>=50MHz set 3'001as initial value is OK for tunning)
    /* set to SD/MMC mode */
    MSDC_SET_BIT32(MSDC_CFG, MSDC_CFG_PIO);

    /* enable SDIO mode. it's must otherwise sdio command failed */
    MSDC_SET_BIT32(SDC_CFG, SDC_CFG_SDIO);

    /* enable wake up events */
    MSDC_SET_BIT32(SDC_CFG, SDC_CFG_INSWKUP);

    /* eneable SMT for glitch filter */
    MSDC_SET_BIT32(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKSMT);
    MSDC_SET_BIT32(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDSMT);
    MSDC_SET_BIT32(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATSMT);

    /* set clk, cmd, dat pad driving */
    MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVN, msdc_cap.clk_drv);
    MSDC_SET_FIELD(MSDC_PAD_CTL0, MSDC_PAD_CTL0_CLKDRVP, msdc_cap.clk_drv);
    MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVN, msdc_cap.cmd_drv);
    MSDC_SET_FIELD(MSDC_PAD_CTL1, MSDC_PAD_CTL1_CMDDRVP, msdc_cap.cmd_drv);
    MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVN, msdc_cap.dat_drv);
    MSDC_SET_FIELD(MSDC_PAD_CTL2, MSDC_PAD_CTL2_DATDRVP, msdc_cap.dat_drv);

    /* set sampling edge */
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, priv->rsmpl);
    MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, priv->dsmpl);

    /* write crc timeout detection */
    MSDC_SET_FIELD(MSDC_PATCH_BIT0, 1 << 30, 1);

    /* MSDC debug pints enable */
#if MSDC_HW_DEBUG
    MSDC_SET_FIELD(0xc0005c50, 0x7 << 9, 6);  /* DPIG3(GPIO28) */
    MSDC_SET_FIELD(0xc0005c60, 0x7 << 9, 6);  /* DPIG2(GPIO33) */
    MSDC_SET_FIELD(0xc0005c60, 0x7 << 12, 6); /* DPIG7(GPIO34) */
    MSDC_SET_FIELD(0xc0005c70, 0x7 << 6, 6);  /* DPIG0(GPIO37) */
    MSDC_SET_FIELD(0xc0005c70, 0x7 << 9, 6);  /* DPIG6(GPIO38) */
    MSDC_SET_FIELD(0xc0005c70, 0x7 << 12, 6); /* DPIG4(GPIO39) */
    MSDC_SET_FIELD(0xc0005c80, 0x7 << 9, 6);  /* DPIG1(GPIO43) */
    MSDC_SET_FIELD(0xc0005c90, 0x7 << 3, 6);  /* DPIG5(GPIO46) */
    MSDC_WRITE32(0xc00018b0, 0x2005); /* MSDC1 debug enable */    
    MSDC_WRITE32(MSDC_HW_DBG, 0x3A3A3A3A);
    MSDC_CLR_BIT32(MSDC_PATCH_BIT0, 1 << 2);
#endif
    
    msdc_config_clksrc(host, clksrc);
    msdc_config_bus(host, HOST_BUS_WIDTH_1);
    msdc_config_clock(host, 0, MSDC_MIN_SCLK);
    /* disable sdio interrupt by default. sdio interrupt enable upon request */
    msdc_intr_unmask(host, 0x0001FF7B);
    msdc_set_dmode(host, mode);
    msdc_set_pio_bits(host, 32);
    msdc_set_timeout(host, 100000000, 0);
    msdc_card_detect(host, 1);

    return 0;
}

int msdc_deinit(struct mmc_host *host)
{
    u32 base = host->base;

    msdc_card_detect(host, 0);
    msdc_intr_mask(host, 0x0001FFFB);
#if MSDC_USE_IRQ
    UNREGISTER_MSDC_IRQ_HANDLER(host->id);
#endif
    MSDC_RESET();
    MSDC_CLR_FIFO();
    MSDC_CLR_INT();
    msdc_power(host, MMC_POWER_OFF);

    return 0;
}

