/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************
 */

#ifndef __RALINK_DMA_CTRL_H__
#define __RALINK_DMA_CTRL_H__

#include <asm/rt2880/rt_mmap.h>

/*
 * DEFINITIONS AND MACROS
 */
#define MOD_VERSION 			"0.4"

#if defined (CONFIG_RALINK_MT7621)
#define MAX_GDMA_CHANNEL		16
#elif defined (CONFIG_RALINK_RT3052)
#define MAX_GDMA_CHANNEL		8
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
#define MAX_GDMA_CHANNEL		16
#else
#error Please Choose System Type
#endif


#define RALINK_GDMA_CTRL_BASE		(RALINK_GDMA_BASE)
#if defined (CONFIG_RALINK_RT3052)
#define RALINK_GDMAISTS			(RALINK_GDMA_BASE + 0x80)
#define RALINK_GDMAGCT			(RALINK_GDMA_BASE + 0x88)
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
#define RALINK_GDMA_UNMASKINT		(RALINK_GDMA_BASE + 0x200)
#define RALINK_GDMA_DONEINT		(RALINK_GDMA_BASE + 0x204)
#define RALINK_GDMA_GCT			(RALINK_GDMA_BASE + 0x220)
#endif

#define GDMA_READ_REG(addr) 		(le32_to_cpu(*(volatile u32 *)(addr)))
#define GDMA_WRITE_REG(addr, val)  	*((volatile uint32_t *)(addr)) = cpu_to_le32(val)
#define GET_GDMA_IP_VER			(GDMA_READ_REG(RALINK_GDMA_GCT) & 0x6) >> 1 //GDMA_GCT[2:1]

/* 
 * 12bytes=GDMA Channel n Source Address(4) +
 *         GDMA Channel n Destination Address(4) +
 *         GDMA Channel n Control Register(4)
 *
 */
#define GDMA_SRC_REG(ch)		(RALINK_GDMA_BASE + ch*16)
#define GDMA_DST_REG(ch)		(GDMA_SRC_REG(ch) + 4)
#define GDMA_CTRL_REG(ch)		(GDMA_DST_REG(ch) + 4)
#define GDMA_CTRL_REG1(ch)		(GDMA_CTRL_REG(ch) + 4)

//GDMA Interrupt Status Register
#if defined (CONFIG_RALINK_RT3052)
#define UNMASK_INT_STATUS(ch)           (ch+16)
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
#define UNMASK_INT_STATUS(ch)           (ch)
#endif
#define TXDONE_INT_STATUS(ch)           (ch)

//Control Reg0
#define MODE_SEL_OFFSET			0
#define CH_EBL_OFFSET			1
#define CH_DONEINT_EBL_OFFSET		2
#define BRST_SIZE_OFFSET		3
#define DST_BRST_MODE_OFFSET		6
#define SRC_BRST_MODE_OFFSET		7
#define TRANS_CNT_OFFSET		16

//Control Reg1
#if defined (CONFIG_RALINK_RT3052)
#define CH_UNMASKINT_EBL_OFFSET		4
#define NEXT_UNMASK_CH_OFFSET		1
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
#define CH_UNMASKINT_EBL_OFFSET		1
#define NEXT_UNMASK_CH_OFFSET		3
#endif
#define COHERENT_INT_EBL_OFFSET		2
#define CH_MASK_OFFSET			0


#if defined (CONFIG_RALINK_RT3052)
//Control Reg0
#define DST_DMA_REQ_OFFSET		8
#define SRC_DMA_REQ_OFFSET		12
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7620)  ||  defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
//Control Reg1
#define DST_DMA_REQ_OFFSET		8
#define SRC_DMA_REQ_OFFSET		16
#endif

#define GDMA_PCM0_RX0			0
#define GDMA_PCM0_RX1			1
#define GDMA_PCM0_TX0			2
#define GDMA_PCM0_TX1			3

#define GDMA_PCM1_RX0			4
#define GDMA_PCM1_RX1			5
#define GDMA_PCM1_TX0			6
#define GDMA_PCM1_TX1			7

#define GDMA_PCM_RX(i,j)		(0+((i)<<2)+j)
#define GDMA_PCM_TX(i,j)        (2+((i)<<2)+j)

#define GDMA_I2S_TX0			4
#define GDMA_I2S_TX1			5
#define GDMA_I2S_RX0			6
#define GDMA_I2S_RX1			7

#define GDMA_SPI_TX       13
#define GDMA_SPI_RX       12


//#define GDMA_DEBUG
#ifdef GDMA_DEBUG
#define GDMA_PRINT(fmt, args...) printk(KERN_INFO "GDMA: " fmt, ## args)
#else
#define GDMA_PRINT(fmt, args...) { }
#endif

/*
 * TYPEDEFS AND STRUCTURES
 */

enum GdmaBusterMode {
	INC_MODE=0,
	FIX_MODE=1
};

enum GdmaBusterSize {
	BUSTER_SIZE_4B=0, 	/* 1 transfer */
	BUSTER_SIZE_8B=1, 	/* 2 transfer */
	BUSTER_SIZE_16B=2,  	/* 4 transfer */
	BUSTER_SIZE_32B=3,  	/* 8 transfer */
	BUSTER_SIZE_64B=4  	/* 16 transfer */
};

enum GdmaDmaReqNum {
#if defined (CONFIG_RALINK_RT3052)
	DMA_REQ0=0,
	DMA_NAND_REQ=1,
	DMA_I2S_TX_REQ=2,
	DMA_PCM_RX0_REQ=3,
	DMA_PCM_RX1_REQ=4,
	DMA_PCM_TX0_REQ=5,
	DMA_PCM_TX1_REQ=6,
	DMA_REG7=7,
	DMA_MEM_REQ=8
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) 
	DMA_REQ0=0,
	DMA_NAND_REQ=1,
	DMA_I2S_TX_REQ=2,
	DMA_I2S_RX_REQ=3,
	DMA_PCM_RX0_REQ=4,
	DMA_PCM_RX1_REQ=5,
	DMA_PCM_TX0_REQ=6,
	DMA_PCM_TX1_REQ=7,
	DMA_CODEC0_REQ8=8,
	DMA_CODEC1_REQ9=9,
	DMA_REQ10=10,
	DMA_REQ11=11,
	DMA_REQ12=12,
	DMA_REQ13=13,
	DMA_REQ14=14,
	DMA_REQ15=15,
	
	#if defined (CONFIG_RALINK_RT3883)
		DMA_MEM_REQ=16
	#elif defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) 
		DMA_MEM_REQ=32
	#endif
	
#elif defined(CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	DMA_REQ0=0,
	DMA_NAND_REQ=1,
	DMA_I2S_TX_REQ=2,
	DMA_I2S_RX_REQ=3,
	DMA_PCM_RX0_REQ=4,
	DMA_PCM_RX1_REQ=5,
	DMA_PCM_TX0_REQ=6,
	DMA_PCM_TX1_REQ=7,
	DMA_PCM_RX2_REQ=8,
	DMA_PCM_RX3_REQ=9,
	DMA_PCM_TX2_REQ=10,
	DMA_PCM_TX3_REQ=11,
	DMA_SPI_RX_REQ=12,
	DMA_SPI_TX_REQ=13,	
	DMA_MEM_REQ=32

#elif defined (CONFIG_RALINK_RT6855A)
	DMA_NAND_REQ=0,
	DMA_I2S_TX_REQ=1,
	DMA_I2S_RX_REQ=2,
	DMA_REQ0=3,
	DMA_PCM_RX0_REQ=4,
	DMA_PCM_RX1_REQ=5,
	DMA_PCM_TX0_REQ=6,
	DMA_PCM_TX1_REQ=7,
	DMA_CODEC0_REQ8=8,
	DMA_CODEC1_REQ9=9,
	DMA_REQ10=10,
	DMA_REQ11=11,
	DMA_REQ12=12,
	DMA_REQ13=13,
	DMA_REQ14=14,
	DMA_REQ15=15,
	DMA_MEM_REQ=32
#else
#error Please Choose System Type
#endif
};



typedef struct {
	uint32_t Src;
	uint32_t Dst;
	uint16_t TransCount;
	uint8_t  SoftMode;
	uint8_t  NextUnMaskCh;
	uint8_t  ChMask;
	uint8_t  CoherentIntEbl;
	uint32_t  ChNum;
	enum GdmaDmaReqNum SrcReqNum;
	enum GdmaDmaReqNum DstReqNum;
	enum GdmaBusterMode SrcBurstMode;
	enum GdmaBusterMode DstBurstMode;
	enum GdmaBusterSize BurstSize;
	void (*DoneIntCallback)(uint32_t);
	void (*UnMaskIntCallback)(uint32_t);
} GdmaReqEntry;

/*
 * EXPORT FUNCTION
 */
int GdmaI2sTx(uint32_t Src, uint32_t Dst, uint8_t TxNo, uint16_t TransCount,
		void (*DoneIntCallback)(uint32_t data), 
		void (*UnMaskIntCallback)(uint32_t data));

int GdmaI2sRx(uint32_t Src, uint32_t Dst, uint8_t RxNo, uint16_t TransCount,
		void (*DoneIntCallback)(uint32_t data), 
		void (*UnMaskIntCallback)(uint32_t data));

int GdmaPcmRx(uint32_t Src, uint32_t Dst, uint8_t PcmNo, uint8_t RxNo, uint16_t TransCount,
		void (*DoneIntCallback)(uint32_t data), 
		void (*UnMaskIntCallback)(uint32_t data));

int GdmaPcmTx(uint32_t Src, uint32_t Dst, uint8_t PcmNo, uint8_t TxNo, uint16_t TransCount,
		void (*DoneIntCallback)(uint32_t data), 
		void (*UnMaskIntCallback)(uint32_t data));
		
int GdmaSpiTx(uint32_t Src, uint32_t Dst, uint16_t TransCount,
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data));

int GdmaSpiRx(uint32_t Src, uint32_t Dst, uint16_t TransCount,
	void (*DoneIntCallback)(uint32_t data),
	void (*UnMaskIntCallback)(uint32_t data));	


int GdmaMem2Mem(uint32_t Src, uint32_t Dst, uint16_t TransCount, 
		void (*DoneIntCallback)(uint32_t data)); 

int GdmaMaskChannel(uint32_t ChNum);

int GdmaUnMaskChannel(uint32_t ChNum);

int GdmaReqQuickIns(uint32_t ChNum);


#endif
