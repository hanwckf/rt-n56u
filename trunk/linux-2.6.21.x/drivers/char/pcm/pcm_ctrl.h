#ifndef __RALINK_PCM_H_
#define __RALINK_PCM_H_

#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <asm/rt2880/rt_mmap.h>

#include "./codec_lib/codec_api.h"

#define PCM_MOD_VERSION 			"1.0"

#if defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6352)
#define CONFIG_RALINK_PCMFRACDIV	1
#endif


#define phys_to_bus(a) (a & 0x1FFFFFFF)

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u8
#define u8 unsigned char
#endif

#ifndef REGBIT
#define REGBIT(x, n)		(x << n)
#endif

#define Virtual2Physical(x)             (((int)x) & 0x1fffffff)
#define Physical2Virtual(x)             (((int)x) | 0x80000000)
#define Virtual2NonCache(x)             (((int)x) | 0x20000000)
#define Physical2NonCache(x)            (((int)x) | 0xa0000000)

#define pcm_outw(address, value)	*((volatile uint32_t *)(address)) = cpu_to_le32(value)
#define pcm_inw(address)			le32_to_cpu(*(volatile u32 *)(address))

#define PCM_DEBUG
#ifdef PCM_DEBUG
#define MSG(fmt, args...) printk("PCM_API: " fmt, ## args)
#else
#define MSG(fmt, args...) { }
#endif

/* Register Map, Ref to RT3052 Data Sheet */

/* System Controller bit field */
#define PCM_CLK_EN			7
#define PCM_CLK_SEL			6
#define	PCM_CLK_DIV			0

/* Register Map Detail */
#define PCM_GLBCFG				(RALINK_PCM_BASE+0x0000)
#define PCM_PCMCFG				(RALINK_PCM_BASE+0x0004)
#define PCM_INT_STATUS			(RALINK_PCM_BASE+0x0008)
#define PCM_INT_EN				(RALINK_PCM_BASE+0x000C)
#define PCM_FF_STATUS			(RALINK_PCM_BASE+0x0010)
#define PCM_CH0_CFG				(RALINK_PCM_BASE+0x0020)
#define PCM_CH1_CFG				(RALINK_PCM_BASE+0x0024)
#define PCM_RSV_REG16			(RALINK_PCM_BASE+0x0030)
#define PCM_FSYNC_CFG			(RALINK_PCM_BASE+0x0030)
#define PCM_CH_CFG2				(RALINK_PCM_BASE+0x0034)	
#define PCM_DIVINT_CFG			(RALINK_PCM_BASE+0x0054)
#define PCM_DIVCOMP_CFG			(RALINK_PCM_BASE+0x0050)
#define PCM_DIGDELAY_CFG		(RALINK_PCM_BASE+0x0060)
#define PCM_CH0_FIFO			(RALINK_PCM_BASE+0x0080)
#define PCM_CH1_FIFO			(RALINK_PCM_BASE+0x0084)

#define PCM_CHFF_STATUS(i)		(((i==0)||(i==1))? (RALINK_PCM_BASE+0x0010+((i)<<2)):\
								(RALINK_PCM_BASE+0x0110+((i-2)<<2)))
#define PCM_CH_CFG(i)			(((i==0)||(i==1))? (RALINK_PCM_BASE+0x0020+((i)<<2)):\
								(RALINK_PCM_BASE+0x0120+((i-2)<<2)))
#define PCM_CH_FIFO(i)			(RALINK_PCM_BASE+0x0080+((i)<<2))

/* PCMCFG bit field */
#define PCM_EXT_CLK_EN			31
#define PCM_CLKOUT				30
#define PCM_EXT_FSYNC			27
#define PCM_LONG_FSYNC			26
#define PCM_FSYNC_POL			25
#define PCM_DRX_TRI				24
#define PCM_SLOTMODE			0

/* GLBCFG bit field */
#define PCM_EN				31
#define DMA_EN				30
#if defined(CONFIG_RALINK_RT6352)
#define PCM_LBK             29
#define PCM_EXT_LBK         28
#endif
#define RFF_THRES			20
#define TFF_THRES			16
#define CH1_TX_EN			9
#define CH0_TX_EN			8
#define CH1_RX_EN			1
#if defined(CONFIG_RALINK_RT6352)
#define CH_EN               0
#else
#define CH0_RX_EN			0
#endif

/* CH0/1_CFG bit field */
#if !defined(CONFIG_RALINK_RT6352)
#define PCM_LBK					31
#define PCM_EXT_LBK				30
#endif
#if defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6352)
#define PCM_CMP_MODE			27
#else
#define PCM_CMP_MODE			28
#endif
#define PCM_TS_START			0

/* INT_STATUS bit field */
#define CH1T_DMA_FAULT			15
#define CH1T_OVRUN				14
#define CH1T_UNRUN				13
#define CH1T_THRES				12
#define CH1R_DMA_FAULT			11
#define CH1R_OVRUN				10
#define CH1R_UNRUN				9
#define CH1R_THRES				8
#define CH0T_DMA_FAULT			7
#define CH0T_OVRUN				6
#define CH0T_UNRUN				5
#define CH0T_THRES				4
#define CH0R_DMA_FAULT			3
#define CH0R_OVRUN				2
#define CH0R_UNRUN				1
#define CH0R_THRES				0

/* INT_EN bit field */
#define INT15_EN				15
#define INT14_EN				14
#define INT13_EN				13
#define INT12_EN				12
#define INT11_EN				11
#define INT10_EN				10
#define INT9_EN					9
#define INT8_EN					8
#define INT7_EN					7
#define INT6_EN					6
#define INT5_EN					5
#define INT4_EN					4
#define INT3_EN					3
#define INT2_EN					2
#define INT1_EN					1
#define INT0_EN					0

/* FF_STATUS bit field */
#define CH1RFF_AVCNT			12
#define CH1TFF_AVCNT			8
#define CH0RFF_AVCNT			4
#define CH0TFF_AVCNT			0

/* PCM_CFG_FSYNC bit field */
#define CFG_FSYNC_EN			31
#define POP_SAMPLE				30
#define FSYNC_START				12
#define FSYNC_INTV				0

/* PCM_DIVCOMP_CFG bit field */
#define CLK_EN					31

/* Test scenario */
#define PCM_IN_CLK
//#define PCM_SLIC_LOOP
#define PCM_INLOOP
//#define PCM_EXLOOP
#define PCM_STATISTIC

#define PCM_LINEAR
//#define PCM_ULAW
//#define PCM_ALAW
//#define PCM_U2L2U
//#define PCM_A2L2A
//#define PCM_L2U2L
//#define PCM_L2A2L
//#define PCM_SW_L2U
//#define PCM_SW_L2A
#define PCM_TASKLET
#define PCM_RECORD
#define PCM_PLAYBACK
//#define PCM_SLIC_CLOCK
//#define PCM_SW_G729AB
//#define PCM_SW_CODEC
/* Constant definition */
#if defined(CONFIG_GDMA_PCM_I2S_OTHERS)
#define MAX_PCM_CH				1
#else
#define MAX_PCM_CH				CONFIG_RALINK_PCMCHNUM
#endif
#define NTFF_THRES				4
#define NRFF_THRES				4

#define MAX_CODEC_CH			MAX_PCM_CH

#define MAX_PCMMMAP_PAGE		6

#define CONFIG_PCM_CH					MAX_PCM_CH

#ifdef PCM_INLOOP
#define CONFIG_PCM_LBK					1
#define CONFIG_PCM_EXT_LBK				0
#else
#define CONFIG_PCM_LBK					0
#define CONFIG_PCM_EXT_LBK				0
#endif

#ifdef PCM_IN_CLK
#define CONFIG_PCM_EXT_CLK_EN			0
#define CONFIG_PCM_CLKOUT_EN			1	/* It should be always one */
#define CONFIG_PCM_EXT_FSYNC			0
#else
#define CONFIG_PCM_EXT_CLK_EN			0
#define CONFIG_PCM_CLKOUT_EN			1	/* It should be always one */
#define CONFIG_PCM_EXT_FSYNC			0
#endif

/* CMP_MODE setup */
#if defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6352)

#ifdef PCM_LINEAR
#define CONFIG_PCM_CMP_MODE			0
#endif
#if	defined(PCM_ULAW)||defined(PCM_ALAW)
#define CONFIG_PCM_CMP_MODE			2
#endif
#ifdef PCM_L2U2L
#define CONFIG_PCM_CMP_MODE			4
#endif
#ifdef PCM_L2A2L
#define CONFIG_PCM_CMP_MODE			6
#endif
#ifdef PCM_U2L2U
#define CONFIG_PCM_CMP_MODE			5
#endif
#ifdef PCM_A2L2A
#define CONFIG_PCM_CMP_MODE			7
#endif

#else

#ifdef PCM_LINEAR
#define CONFIG_PCM_CMP_MODE			0
#endif
#if	defined(PCM_ULAW)||defined(PCM_ALAW)
#define CONFIG_PCM_CMP_MODE			1
#endif
#ifdef PCM_U2L2U
#define CONFIG_PCM_CMP_MODE			2
#endif
#ifdef PCM_A2L2A
#define CONFIG_PCM_CMP_MODE			3
#endif

#endif

#define CONFIG_PCM_LONG_FSYNC			0
#define CONFIG_PCM_FSYNC_POL			1
#define CONFIG_PCM_DRX_TRI			1
#define CONFIG_PCM_TS_START			1

#define CONFIG_PCM_TFF_THRES			NTFF_THRES
#define CONFIG_PCM_RFF_THRES			NRFF_THRES

typedef struct pcm_status_t
{
	u32 ch0txdmafault;
	u32 ch0txovrun;
	u32 ch0txunrun;
	u32 ch0txthres;
	u32 ch0rxdmafault;
	u32 ch0rxovrun;
	u32 ch0rxunrun;
	u32 ch0rxthres;

	u32 ch1txdmafault;
	u32 ch1txovrun;
	u32 ch1txunrun;
	u32 ch1txthres;
	u32 ch1rxdmafault;
	u32 ch1rxovrun;
	u32 ch1rxunrun;
	u32 ch1rxthres;

}pcm_status_type;

typedef struct pcm_config_t
{
	u32 pcm_ch_num;
	u32 codec_ch_num;
	u32 nch_active;
	int curchid,txcurchid;
	int txfifo_rd_idx[MAX_PCM_CH];
	int txfifo_wt_idx[MAX_PCM_CH];
	int rxfifo_rd_idx[MAX_PCM_CH];
	int rxfifo_wt_idx[MAX_PCM_CH];
	
	int bsfifo_rd_idx[MAX_PCM_CH];
	int bsfifo_wt_idx[MAX_PCM_CH];
	
	int rx_isr_cnt;
	int tx_isr_cnt;
	int pos;
	char* mmapbuf;
	int mmappos;
	int	mmap_rd_idx;
	int	mmap_wt_idx;
	int bStartRecord;
	int bStartPlayback;
	int iRecordCH;
	int	iPlaybackCH;
	int codec_type[MAX_CODEC_CH];
	
	
	u32 extclk_en;
	u32 clkout_en;
	u32 ext_fsync;
	u32 long_fynsc;
	u32 fsync_pol;
	u32 drx_tri;
	u32 slot_mode;

	u32 tff_thres;
	u32 rff_thres;

	u32	lbk[MAX_PCM_CH];
	u32 ext_lbk[MAX_PCM_CH];
	u32 ts_start[MAX_PCM_CH];
	u32 cmp_mode[MAX_PCM_CH];
#ifdef __KERNEL__	
	spinlock_t lock;
	spinlock_t txlock;
	spinlock_t rxlock;
	wait_queue_head_t	pcm_qh;
#endif	
	union {
		short* TxPage0Buf16Ptr[MAX_PCM_CH];	
		char* TxPage0Buf8Ptr[MAX_PCM_CH];
	};
	union {
		short* TxPage1Buf16Ptr[MAX_PCM_CH];	
		char* TxPage1Buf8Ptr[MAX_PCM_CH];
	};
	union {
		short* RxPage0Buf16Ptr[MAX_PCM_CH];	
		char* RxPage0Buf8Ptr[MAX_PCM_CH];	
	};
	union {
		short* RxPage1Buf16Ptr[MAX_PCM_CH];	
		char* RxPage1Buf8Ptr[MAX_PCM_CH];	
	};
	
	union {
		short* TxFIFOBuf16Ptr[MAX_PCM_CH];	
		char* TxFIFOBuf8Ptr[MAX_PCM_CH];	
	};
	
	union {
		short* RxFIFOBuf16Ptr[MAX_PCM_CH];	
		char* RxFIFOBuf8Ptr[MAX_PCM_CH];
	};
	
	union {
		short* BSFIFOBuf16Ptr[MAX_PCM_CH];	
		char* BSFIFOBuf8Ptr[MAX_PCM_CH];
	};

}pcm_config_type;


extern pcm_config_type* ppcm_config;
extern pcm_status_type* ppcm_status;

void pcm_rx_task(unsigned long pData);
void pcm_tx_task(unsigned long pData);

#endif /* __KERNEL__ */

#define MAX_PCM_PROC_UNIT		3
#if	defined(PCM_ULAW)||defined(PCM_ALAW)||defined(PCM_U2L2U)||defined(PCM_A2L2A)
#define PCM_SAMPLE_SIZE			1
#else
#define PCM_SAMPLE_SIZE			2
#endif

#define PCM_8KHZ_SAMPLES		80

#define MAX_PCM_FIFO			12
#define PCM_FIFO_SAMPLES		(PCM_8KHZ_SAMPLES)
#define PCM_FIFO_SIZE			PCM_FIFO_SAMPLES*PCM_SAMPLE_SIZE

#define MAX_PCM_BSFIFO			12
#define PCM_BS_SIZE				166
#define PCM_BSFIFO_SIZE			(PCM_BS_SIZE*MAX_PCM_BSFIFO)

#define MAX_PCM_PAGE			1
#define PCM_PAGE_SAMPLES		(PCM_8KHZ_SAMPLES*MAX_PCM_PROC_UNIT)
#define PCM_PAGE_SIZE			PCM_PAGE_SAMPLES*PCM_SAMPLE_SIZE

/* driver status definition */
#define PCM_OK					0
#define PCM_OUTOFMEM				0x01
#define PCM_GDMAFAILED				0x02
#define PCM_REQUEST_IRQ_FAILED			0x04

/* driver i/o control command */
#define PCM_SET_RECORD			0
#define PCM_SET_UNRECORD		1	
#define PCM_READ_PCM			2
#define PCM_START				3
#define PCM_STOP				4
#define PCM_SET_PLAYBACK		5
#define PCM_SET_UNPLAYBACK		6
#define PCM_WRITE_PCM			7
#define PCM_SET_CODEC_TYPE		8
#define PCM_EXT_LOOPBACK_ON		9
#define PCM_EXT_LOOPBACK_OFF	10
#define PCM_PUTDATA				11
#define PCM_GETDATA				12

/* Qwert : Add for slic access */
#define PCM_SLIC_DRREAD		30
#define PCM_SLIC_IRREAD		31
#define PCM_SLIC_DRWRITE	32
#define PCM_SLIC_IRWRITE	33

typedef struct pcm_buffer_t
{
	char* pcmbuf;
	int size;
}pcm_record_type, pcm_playback_type;


#define RALINK_PCM_VERSION	"v1.00"
#define PCMDRV_DEVNAME		"pcm0"
#endif

