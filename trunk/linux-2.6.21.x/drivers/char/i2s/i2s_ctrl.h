#ifndef __RALINK_I2S_H_
#define __RALINK_I2S_H_

#ifdef __KERNEL__
#include <asm/rt2880/rt_mmap.h>
#endif

#define I2S_MAX_DEV			1
#define I2S_MOD_VERSION			"0.1"
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
#define NonCache2Virtual(x)             (((int)x) & 0xDFFFFFFF)

#define I2S_DEBUG_PRN
#ifdef I2S_DEBUG_PRN
#define MSG(fmt, args...) printk("I2S: " fmt, ## args)
#else
#define MSG(fmt, args...) { }
#endif

#define i2s_outw(address, value)	do{printk("0x%08X = 0x%08X\n",(u32)address,(u32)value);*((volatile uint32_t *)(address)) = cpu_to_le32(value);}while(0)
#define i2s_inw(address)			le32_to_cpu(*(volatile u32 *)(address))

/* HW feature definiations */
#if defined(CONFIG_RALINK_RT3883)
#define CONFIG_I2S_TXRX			1
#define CONFIG_I2S_IN_MCLK		1
//#define CONFIG_I2S_WS_EDGE		1
#define CONFIG_I2S_FRAC_DIV		1
#define CONFIG_I2S_EXTENDCFG	1
#define CONFIG_I2S_IN_CLK		1
#define CONFIG_I2S_MS_MODE		1
#endif

#if defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) \
	|| defined(CONFIG_RALINK_RT6855A) || defined(CONFIG_RALINK_RT6352)
#define CONFIG_I2S_TXRX			1
//#define CONFIG_I2S_IN_MCLK		1
//#define CONFIG_I2S_WS_EDGE		1
#define CONFIG_I2S_FRAC_DIV		1
#define CONFIG_I2S_EXTENDCFG	1
#define CONFIG_I2S_IN_CLK		1
#endif

#if defined(CONFIG_RALINK_RT3350)
#define CONFIG_I2S_IN_MCLK	1
#endif

#if defined(CONFIG_RALINK_RT3052)
#define CONFIG_I2S_MS_MODE		1
#endif

#define CONFIG_I2S_MMAP           1

/* Register Map, Ref to RT3052 Data Sheet */

/* Register Map Detail */
#define I2S_I2SCFG				(RALINK_I2S_BASE+0x0000)
#define I2S_INT_STATUS			(RALINK_I2S_BASE+0x0004)
#define I2S_INT_EN				(RALINK_I2S_BASE+0x0008)
#define I2S_FF_STATUS			(RALINK_I2S_BASE+0x000c)
#define I2S_FIFO_WREG			(RALINK_I2S_BASE+0x0010)
#define I2S_TX_FIFO_WREG		I2S_FIFO_WREG
#define I2S_RX_FIFO_RREG		(RALINK_I2S_BASE+0x0014)
#define I2S_I2SCFG1				(RALINK_I2S_BASE+0x0018)
#define I2S_DIVINT_CFG			(RALINK_I2S_BASE+0x0024)
#define I2S_DIVCOMP_CFG			(RALINK_I2S_BASE+0x0020)



/* I2SCFG bit field */
#define I2S_EN				31
#define I2S_DMA_EN			30
#define I2S_BYTE_SWAP			28
#define I2S_CLK_OUT_DIS		8
#define I2S_FF_THRES		4
#define I2S_CH_SWAP			3
#define I2S_CH1_OFF			2
#define I2S_CH0_OFF			1
#define I2S_SLAVE_EN		0
#define I2S_TX_EN			24
#define I2S_RX_EN			20
#define I2S_SLAVE_MODE		16
#define I2S_RX_FF_THRES		12
#define I2S_RX_CH_SWAP		11
#define I2S_RX_CH1_OFF		10
#define I2S_RX_CH0_OFF		9
#define I2S_TX_FF_THRES		4
#define I2S_TX_CH_SWAP		3
#define I2S_TX_CH1_OFF		2
#define I2S_TX_CH0_OFF		1
#define I2S_WS_INV			0
/* INT_EN bit field */
#define I2S_RX_INT3_EN			7
#define I2S_RX_INT2_EN			6
#define I2S_RX_INT1_EN			5
#define I2S_RX_INT0_EN			4
#define I2S_TX_INT3_EN			3
#define I2S_TX_INT2_EN			2
#define I2S_TX_INT1_EN			1
#define I2S_TX_INT0_EN			0

/* INT_STATUS bit field */
#define I2S_RX_DMA_FAULT		7
#define I2S_RX_OVRUN			6
#define I2S_RX_UNRUN			5
#define I2S_RX_THRES			4
#define I2S_TX_DMA_FAULT		3
#define I2S_TX_OVRUN			2
#define I2S_TX_UNRUN			1
#define I2S_TX_THRES			0

/* FF_STATUS bit field */
#define I2S_RX_EPCNT			4
#define I2S_TX_EPCNT			0
/* I2S_DIVCOMP_CFG bit field */
#define I2S_CLKDIV_EN				31

/* I2S_CFG1 bit field */
#define I2S_LBK_EN				31
#define I2S_EXT_LBK_EN			30
#define I2S_DATA_FMT			0

/* FIFO_WREG bit field */
#define I2S_FIFO_WDATA		0

/* Constant definition */
#define NFF_THRES			4
#define I2S_PAGE_SIZE		(4*4096)//(1152*2*2*2)
#define MAX_I2S_PAGE		8

#define MAX_SRATE_HZ			96000
#define MIN_SRATE_HZ			8000

#define MAX_VOL_DB				+0			
#define MIN_VOL_DB				-127

/* I2S I/O command */
#define I2S_SRATE				0
#define I2S_VOL					1
#define I2S_ENABLE				2
#define I2S_DISABLE				3
#define I2S_TX_ENABLE				2
#define I2S_TX_DISABLE				3
#define I2S_GET_WBUF			4
#define I2S_PUT_WBUF			5
#define I2S_RX_ENABLE			6
#define I2S_RX_DISABLE			7
#define I2S_PUT_AUDIO		4
#define I2S_GET_AUDIO			5
#define I2S_TX_VOL				1
#define I2S_RX_VOL				8

#define I2S_DEBUG				20
#define I2S_DEBUG_CLKGEN		20
#define I2S_DEBUG_INLBK			21
#define I2S_DEBUG_EXLBK			22
#define I2S_DEBUG_FMT			23
#define I2S_DEBUG_RESET			24
#define I2S_DEBUG_CODECBYPASS	25

/* configuration */
#define CONFIG_I2S_TFF_THRES					NFF_THRES
#define CONFIG_I2S_CH_SWAP					0
#if defined(CONFIG_I2S_MS_MODE) 
#define CONFIG_I2S_SLAVE_EN					0
#else
#define CONFIG_I2S_SLAVE_EN					1
#endif


#define CONFIG_I2S_INLBK					0
#define CONFIG_I2S_EXLBK					0
#define CONFIG_I2S_FMT						0

/* driver status definition */
#define I2S_OK						0
#define I2S_OUTOFMEM				0x01
#define I2S_GDMAFAILED				0x02
#define I2S_REQUEST_IRQ_FAILED		0x04
#define I2S_REG_SETUP_FAILED		0x08

//#define I2S_FIFO_MODE		1
#define I2S_STATISTIC
#define I2S_MAJOR			234

typedef struct i2s_status_t
{
	u32 txdmafault;
	u32 txovrun;
	u32 txunrun;
	u32 txthres;
	int txbuffer_unrun;
	int txbuffer_ovrun;
	int txbuffer_len;
	
	u32 rxdmafault;
	u32 rxovrun;
	u32 rxunrun;
	u32 rxthres;
	int rxbuffer_unrun;
	int rxbuffer_ovrun;
	int rxbuffer_len;
}i2s_status_type;


typedef struct i2s_config_t
{

	int srate;
	int txvol;
	int rxvol;
	u32	pos;
	u32 flag;
	u32	tx_isr_cnt;
	u32	rx_isr_cnt;
	int bSleep;
	int bTxDMAEnable;
	int bRxDMAEnable;
#ifdef __KERNEL__		
	spinlock_t lock;
	wait_queue_head_t i2s_tx_qh, i2s_rx_qh;
#endif
	u32 dmach;
	u32 tx_ff_thres;
	u32 tx_ch_swap;
	u32 rx_ff_thres;
	u32 rx_ch_swap;
	u32 slave_en;
	
	/* for I2S_CFG1 */
	u32 lbk;
	u32	extlbk;
	u32 fmt;
	
	int w_idx;
	int r_idx;
	
	int tx_w_idx;
	int tx_r_idx;
	int rx_w_idx;
	int rx_r_idx;
	int mmap_index;
	
	u8* buf8ptr;	
	char* pMMAPBufPtr[MAX_I2S_PAGE*2];	
	char* pMMAPTxBufPtr[MAX_I2S_PAGE];
	char* pMMAPRxBufPtr[MAX_I2S_PAGE];
	
	union {
		u16* pPage0TxBuf16Ptr;	
		u8* pPage0TxBuf8ptr;	
	};
	union {
		u16* pPage1TxBuf16Ptr;	
		u8* pPage1TxBuf8ptr;	
	};
		
	union {
		u16* pPage0RxBuf16Ptr;	
		u8* pPage0RxBuf8ptr;	
	};
	union {
		u16* pPage1RxBuf16Ptr;	
		u8* pPage1RxBuf8ptr;	
	};

}i2s_config_type;


int i2s_reset_tx_config(i2s_config_type* ptri2s_config);
int i2s_reset_rx_config(i2s_config_type* ptri2s_config);
int i2s_tx_config(i2s_config_type* ptri2s_config);
int i2s_rx_config(i2s_config_type* ptri2s_config);
int i2s_tx_enable(i2s_config_type* ptri2s_config);
int i2s_tx_disable(i2s_config_type* ptri2s_config);
int i2s_rx_enable(i2s_config_type* ptri2s_config);
int i2s_rx_disable(i2s_config_type* ptri2s_config);
int i2s_codec_enable(i2s_config_type* ptri2s_config);
int i2s_codec_disable(i2s_config_type* ptri2s_config);
int i2s_clock_enable(i2s_config_type* ptri2s_config);
int i2s_clock_disable(i2s_config_type* ptri2s_config);
void i2s_dma_tx_handler(u32 dma_ch);
void i2s_dma_rx_handler(u32 dma_ch);
void i2s_unmask_handler(u32 dma_ch);

#if !defined(CONFIG_I2S_TXRX)
#define GdmaI2sRx	//GdmaI2sRx
#endif

#define RALINK_I2S_VERSION	"1.0"
#define I2SDRV_DEVNAME		"i2s0"

#endif /* __RALINK_I2S_H_ */

