#include <linux/delay.h>
#include <linux/dma-mapping.h>
#define PHYSADDR(a)		virt_to_phys((void*)(a))
#include <asm/io.h>
#include <asm/rt2880/rt_mmap.h>
#include "gdma.h"

#include "ralink_nand.h"

#define DMA_CHNUM (0)

int _nand_dma_sync(void)
{
	//unmask to start dma
	unsigned long data;
	int retry = 1000000; //fixme

	data = GDMA_READ_REG(GDMA_CTRL_REG1(DMA_CHNUM));
	data &= ~( 0x01 << CH_MASK_OFFSET); 
	GDMA_WRITE_REG(GDMA_CTRL_REG1(DMA_CHNUM), data);
	
#if defined (CONFIG_RALINK_RT3052)
	// sync status
	while(!(GDMA_READ_REG(RALINK_GDMAISTS) & (1<<DMA_CHNUM)) && retry--) {
		ndelay(1);
		// do nothing
	}
	if (!(GDMA_READ_REG(RALINK_GDMAISTS) & (1<<DMA_CHNUM))) {
		return -1;
	}
	GDMA_WRITE_REG(RALINK_GDMAISTS, 1<<DMA_CHNUM);
#elif defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
	while(!(GDMA_READ_REG(RALINK_GDMA_DONEINT) & (1<<DMA_CHNUM)) && retry--) {
		ndelay(1);
	}
	if (!(GDMA_READ_REG(RALINK_GDMA_DONEINT) & (1<<DMA_CHNUM))) {
		return -1;
	}
	GDMA_WRITE_REG(RALINK_GDMA_DONEINT, 1<<DMA_CHNUM);
#endif
	
	return 0;
}

void _release_dma_buf(void)
{
	unsigned long data;

	data = GDMA_READ_REG(GDMA_CTRL_REG(DMA_CHNUM));
	data &= ~( 0x01 << CH_EBL_OFFSET);
	GDMA_WRITE_REG(GDMA_CTRL_REG(DMA_CHNUM), data);

}

int _set_gdma_ch(unsigned long dst, 
		 unsigned long src, unsigned int len, int burst_size,
		 int soft_mode, int src_req_type, int dst_req_type,
		 int src_burst_mode, int dst_burst_mode)
{
	unsigned long data;

	//src
	GDMA_WRITE_REG(GDMA_SRC_REG(DMA_CHNUM), (src & 0x1fffffff));

	//dst
	GDMA_WRITE_REG(GDMA_DST_REG(DMA_CHNUM), (dst & 0x1fffffff));

	//control 1, 
	data = 0;
//    data = (0 << CH_UNMASK_INTEBL_OFFSET); 
	data |= ( DMA_CHNUM << NEXT_UNMASK_CH_OFFSET); 
	data |= ( (soft_mode == 0) << CH_MASK_OFFSET); 
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
	data |= (src_req_type << SRC_DMA_REQ_OFFSET); 
	data |= (dst_req_type << DST_DMA_REQ_OFFSET); 
#endif
	GDMA_WRITE_REG(GDMA_CTRL_REG1(DMA_CHNUM), data);

	// control 
	data = (len << TRANS_CNT_OFFSET); 
#ifdef CONFIG_RALINK_RT3052
	data |= (src_req_type << SRC_DMA_REQ_OFFSET); 
	data |= (dst_req_type << DST_DMA_REQ_OFFSET); 
#endif
	data |= (src_burst_mode << SRC_BRST_MODE_OFFSET); 
	data |= (dst_burst_mode << DST_BRST_MODE_OFFSET); 
	data |= (burst_size << BRST_SIZE_OFFSET); 
//    data |= (0 << INT_EBL_OFFSET); 
	data |= ((soft_mode != 0) << MODE_SEL_OFFSET); 
	data |= (0x01<<CH_EBL_OFFSET); 
	GDMA_WRITE_REG(GDMA_CTRL_REG(DMA_CHNUM), data);

	return 1;
}


int _ra_nand_prepare_dma_pull(unsigned long dst, int len)
{
	_set_gdma_ch(PHYSADDR(dst), NFC_DATA, len,  
		     BURST_SIZE_4B, HW_MODE, DMA_NAND_REQ, DMA_REQMEM, 
		     TRN_FIX, TRN_INC);
	return 0;
}

#if 0
int _ra_nor_dma_pull(char *dst, char *src, int len)
{
	int ret = 0;

	//fixme, take care about alignment issues
	while (len > 0) {
		int size = (len > ((1<<16) - 4)) ? ((1<<16) - 4) : len;	// limitation is DMA buffer
		// set GDMA 
		_set_gdma_ch(PHYSADDR(dst), PHYSADDR(src), size,  
			    BURST_SIZE_32B, SW_MODE, DMA_REQMEM, DMA_REQMEM, 
			    TRN_INC, TRN_INC);

		// start and wait dma done
		if (_nand_dma_sync()) {
			printk("%s: gdma: fail, dst:%lx, len:%x \n", __func__, dst, len);
			ret = -1;
		}
	
		// disable dma
		_release_dma_buf();

		len -= size;
		dst += size;
		src += size;
	}
	return ret;


}
#endif

// this is "data moving" from nand to memory.
int _ra_nand_dma_pull(unsigned long dst, int len)
{
	int ret =0;

	dma_cache_inv(dst, len);

	// set GDMA 
	_set_gdma_ch(PHYSADDR(dst), NFC_DATA, len,  
		     BURST_SIZE_4B, HW_MODE, DMA_NAND_REQ, DMA_REQMEM, 
		     TRN_FIX, TRN_INC);

	// start and wait dma done
	if (_nand_dma_sync()) {
		printk("%s: gdma: fail, dst:%lx, len:%x \n", __func__, dst, len);
		ret = -1;
	}
		
	// disable dma
	_release_dma_buf();

	return ret;
}

// this is data moving from memory to nand.
int _ra_nand_dma_push(unsigned long src, int len)
{
	int ret = 0;

	dma_cache_wback(src, len);

	// set GDMA 
	_set_gdma_ch(NFC_DATA, PHYSADDR((void*)src), len,  
		     BURST_SIZE_4B, HW_MODE, DMA_REQMEM, DMA_NAND_REQ, 
		     TRN_INC, TRN_FIX);

	// start and wait dma done
	if (_nand_dma_sync()) {
		printk("%s: gdma: fail, dst:%lx, len:%x \n", __func__, src, len);
		ret = -1;
	}

	// disable dma
	_release_dma_buf();

	return ret;
}

void gdma_reset(void)
{
	ra_aor(RALINK_SYSCTL_BASE + 0x34, ~(1<<14) , (1<<14));
	udelay(1);
	ra_aor(RALINK_SYSCTL_BASE + 0x34, ~(1<<14) , 0);
}
