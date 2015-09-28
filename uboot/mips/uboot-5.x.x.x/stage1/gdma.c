#include <common.h>
#include "rt_mmap.h"
/*
 * DEFINITIONS AND MACROS
 */
#define MAX_GDMA_CHANNEL		8
#define MOD_VERSION 			"0.3"
#define RALINK_GDMA_CTRL_BASE		(RALINK_GDMA_BASE)
#define RALINK_GDMAISTS			(RALINK_GDMA_BASE + 0x80)
#define RALINK_GDMASSTS			(RALINK_GDMA_BASE + 0x84)
#define RALINK_GDMAGCT			(RALINK_GDMA_BASE + 0x88)

#define GDMA_READ_REG(addr) 		le32_to_cpu(*(volatile u32 *)(addr))
#define GDMA_WRITE_REG(addr, val)  	*((volatile uint32_t *)(addr)) = cpu_to_le32(val)

#define RALINK_IRQ_ADDR                 RALINK_INTCL_BASE
#define RALINK_REG_INTENA               (RALINK_IRQ_ADDR + 0x34)
#define RALINK_REG_INTDIS               (RALINK_IRQ_ADDR + 0x38)

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
#define TX_DONE_INT_STATUS_OFFSET	0
#define UMASK_INT_STATUS_OFFSET		16

//Control Reg1
#define CH_UNMASK_INTEBL_OFFSET		4
#define NEXT_UNMASK_CH_OFFSET		1
#define CH_MASK_OFFSET			0

//Control Reg
#define MODE_SEL_OFFSET			0
#define CH_EBL_OFFSET			1
#define INT_EBL_OFFSET			2
#define BRST_SIZE_OFFSET		3
#define DST_BRST_MODE_OFFSET		6
#define SRC_BRST_MODE_OFFSET		7
#define DST_DMA_REQ_OFFSET		8
#define SRC_DMA_REQ_OFFSET		12
#define TRANS_CNT_OFFSET		16

#define GDMA_PCM0_RX0			0
#define GDMA_PCM0_RX1			1
#define GDMA_PCM0_TX0			2
#define GDMA_PCM0_TX1			3

#define GDMA_PCM1_RX0			4
#define GDMA_PCM1_RX1			5
#define GDMA_PCM1_TX0			6
#define GDMA_PCM1_TX1			7

#define GDMA_I2S_TX0			4
#define GDMA_I2S_TX1			5

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
	DMA_REQ0=0,
	DMA_NAND_FLASH_REQ=1,
	DMA_I2S_REQ=2,
	DMA_PCM_RX0_REQ=3,
	DMA_PCM_RX1_REQ=4,
	DMA_PCM_TX0_REQ=5,
	DMA_PCM_TX1_REQ=6,
	DMA_REQ7=7,
	DMA_MEM_REQ=8
};


#define DMA_CHNUM (0)    

inline int nand_dma_sync(void)
{	
	//unmask to start dma
	uint32_t data;
	data = GDMA_READ_REG(GDMA_CTRL_REG1(DMA_CHNUM));
	data &= ~( 0x01 << CH_MASK_OFFSET); 
	GDMA_WRITE_REG(GDMA_CTRL_REG1(DMA_CHNUM), data);
	
	// sync status
	while(!(GDMA_READ_REG(RALINK_GDMAISTS) & (1<<DMA_CHNUM))) {
		udelay(1);
		// do nothing
	}
	
	GDMA_WRITE_REG(RALINK_GDMAISTS, 1<<DMA_CHNUM);
	
	return 0;
}

inline void release_dma_buf()
{
	uint32_t data;

	data = GDMA_READ_REG(GDMA_CTRL_REG(DMA_CHNUM));
	data &= ~( 0x01 << CH_EBL_OFFSET);
	GDMA_WRITE_REG(GDMA_CTRL_REG(DMA_CHNUM), data);

}

inline int set_gdma_ch(unsigned long dst, 
		unsigned long src, unsigned int len, int burst_size,
		int soft_mode, int src_req_type, int dst_req_type,
		int src_burst_mode, int dst_burst_mode)
{
    uint32_t data;

    //src
    GDMA_WRITE_REG(GDMA_SRC_REG(DMA_CHNUM), (src & 0x1fffffff));

    //dst
    GDMA_WRITE_REG(GDMA_DST_REG(DMA_CHNUM), (dst & 0x1fffffff));

    /* set_gdma_ch(p,
	    NFC_DATA, len,  BURST_SIZE_4B,
	    HW_MODE, DMA_REQ1, DMA_REQMEM, 
			TRN_FIX, TRN_INC);
		*/

    //control 1, 
    data = 0;
//    data = (0 << CH_UNMASK_INTEBL_OFFSET); 
    data |= ( DMA_CHNUM << NEXT_UNMASK_CH_OFFSET); 
    data |= ( (soft_mode == 0) << CH_MASK_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG1(DMA_CHNUM), data);

    // control 
    data = (len << TRANS_CNT_OFFSET); 
    data |= (src_req_type << SRC_DMA_REQ_OFFSET); 
    data |= (dst_req_type << DST_DMA_REQ_OFFSET); 
    data |= (src_burst_mode << SRC_BRST_MODE_OFFSET); 
    data |= (dst_burst_mode << DST_BRST_MODE_OFFSET); 
    data |= (burst_size << BRST_SIZE_OFFSET); 
//    data |= (0 << INT_EBL_OFFSET); 
    data |= ((soft_mode != 0) << MODE_SEL_OFFSET); 
    data |= (0x01<<CH_EBL_OFFSET); 
    GDMA_WRITE_REG(GDMA_CTRL_REG(DMA_CHNUM), data);


    return 1;

}




