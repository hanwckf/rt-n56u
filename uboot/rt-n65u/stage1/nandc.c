#include "configs/rt2880.h"
#include <common.h>
#include "rt_mmap.h"

#define ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))
#define ra_aor(addr, a_mask, o_value)  ra_outl(addr, (ra_inl(addr) & (a_mask)) | (o_value))

//#define CONFIG_BADBLOCK_CHECK


#define NFC_BASE 	RALINK_NAND_CTRL_BASE
#define NFC_CTRL	(NFC_BASE + 0x0)
#define NFC_CONF	(NFC_BASE + 0x4)
#define NFC_CMD1	(NFC_BASE + 0x8)
#define NFC_CMD2	(NFC_BASE + 0xc)
#define NFC_CMD3	(NFC_BASE + 0x10)
#define NFC_ADDR	(NFC_BASE + 0x14)
#define NFC_DATA	(NFC_BASE + 0x18)
#define NFC_ECC		(NFC_BASE + 0x1c)
#define NFC_STATUS	(NFC_BASE + 0x20)
#define NFC_INT_EN	(NFC_BASE + 0x24)
#define NFC_INT_ST	(NFC_BASE + 0x28)


#define DMA_REQ1 1 //NAND_CONTROLLER
#define DMA_NAND_FLASH_REQ DMA_REQ1
#define BURST_SIZE_4B 0 	/* 1 transfer */
#define BURST_SIZE_8B 1 	/* 2 transfer */
#define	BURST_SIZE_16B 2  	/* 4 transfer */
#define	BURST_SIZE_32B 3  	/* 8 transfer */
#define BURST_SIZE_64B 4  	/* 16 transfer */
#define SW_MODE 1
#define HW_MODE 0
#define DMA_REQMEM 0x08
#define TRN_FIX  1
#define TRN_INC 0



// chip geometry
#define NAND_BYTE_SIZE (25) 	//! (1<<NAND_SIZE_BYTE) MB
#define PAGE_SIZE (9)   	//! (1<<PAGE_SIZE) MB
#define PAGE_SHIFT 1		//! these bits will be compensate by command cycle

#define PAGE_SIZE_BYTE (1<<PAGE_SIZE)
#define COLUMN_ADDR_MASK ((1<<(PAGE_SIZE-PAGE_SHIFT)) - 1)
#define COLUMN_ADDR_CYCLE (((PAGE_SIZE-PAGE_SHIFT) + 7)/8) 
#define ROW_ADDR_CYCLE ((NAND_BYTE_SIZE - PAGE_SIZE + 7)/8) 
#define ADDR_CYCLE (COLUMN_ADDR_CYCLE + ROW_ADDR_CYCLE)
#if 0
extern char _gp; 
#define RELOCATE_OFFSET ({unsigned long offset, got = (unsigned long)&_gp; \
				 asm volatile ("subu %0, $gp, %1\n\t": "=r"(offset) :"r"(&_gp)); offset; })
#define RELOCATE_CALL(func, arg...) ({typeof(func) *f =(unsigned long)(func) + RELOCATE_OFFSET; f(arg);})
#endif


void nand_dma_pull(char *dst, int len)
{
	while (len > 0) {
#define NFC_FIFO_SIZE_A (60)
#define NFC_FIFO_SIZE_B (512)
		int size = (len > (NFC_FIFO_SIZE_A)) ? (NFC_FIFO_SIZE_A) : len;	// limitation is NFC fifo size

		// set GDMA 
		set_gdma_ch(dst, NFC_DATA, size,  
			    BURST_SIZE_4B, HW_MODE, DMA_REQ1, DMA_REQMEM, 
			    TRN_FIX, TRN_INC);

		// start and wait dma done
		nand_dma_sync();
	
		// disable dma
		release_dma_buf();

		len -= size;
		dst += size;
	}
	return ;
}

int nand_init(void)
{
	int retry;

	// reset nand flash
	ra_outl(NFC_CMD3, 0xff);
	ra_outl(NFC_ADDR, 0xfffffff);
	ra_outl(NFC_CONF, 0x1021);
	udelay(1);

	// reset controller
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) | 0x02); //clear data buffer
	ra_outl(NFC_CTRL, ra_inl(NFC_CTRL) & ~0x02); //clear data buffer
	ra_outl(NFC_INT_ST, ra_inl(NFC_INT_ST));
	retry = 100;
	while ((ra_inl(NFC_INT_ST) & 0x02) != 0x02 && retry--);
	if (retry <= 0) {
		return -1;
	}
	retry = 100;
	while ((ra_inl(NFC_STATUS) & 0x1) != 0x0 && retry--) { //fixme, controller is busy ?
		udelay(1);
	}
	
	return 0;
}

#ifdef CONFIG_BADBLOCK_CHECK
int nand_block_isbad(loff_t offs)
{
	unsigned int cmd1, bus_addr, conf;
	long p;
	unsigned char *c;

	p = 0x80500000L;
	// set gdma
	set_gdma_ch(p, NFC_DATA, 1,  
			BURST_SIZE_4B, HW_MODE, DMA_REQ1, DMA_REQMEM, 
			TRN_FIX, TRN_INC);

	// set NFC
	ra_outl(NFC_INT_ST, ra_inl(NFC_INT_ST));
	cmd1 = 0x50;
	ra_outl(NFC_CMD1, cmd1); 	
	bus_addr = ((offs >> PAGE_SIZE) << (COLUMN_ADDR_CYCLE*8)) | (4 & COLUMN_ADDR_MASK);
	ra_outl(NFC_ADDR, bus_addr);
	conf = 0x000141 | (ADDR_CYCLE << 16) | (1 << 20);
	conf |= (1<<2); //gdma
	ra_outl(NFC_CONF, conf);

	// start and wait dma done
	nand_dma_sync();

	// disable dma
	release_dma_buf();

	c = (unsigned char *)p;
	if ((*c & 0xff) == 0xff)
		return 0;
	return 1;
}
#endif

#define WORK_AROUND_RXB_OV 1
int nand_read(uint32_t dst, uint32_t addr, uint32_t size)
{
	unsigned int cmd1, conf;
	long p = dst;
	unsigned int bus_addr;
	unsigned len;

	ra_outl(NFC_INT_ST, ra_inl(NFC_INT_ST));

	while (size > 0) {
		bus_addr = ((addr >> PAGE_SIZE) << (COLUMN_ADDR_CYCLE*8));

#ifdef CONFIG_BADBLOCK_CHECK
		if (nand_block_isbad(addr)) {
			addr += 0x4000;
			continue;
		}
#endif

		bus_addr |= (addr & COLUMN_ADDR_MASK);

		cmd1 = 0x00 | ((addr >> (PAGE_SIZE - PAGE_SHIFT)) & ((1 << PAGE_SHIFT)-1));

		// read command is limited within a page range.
		len = PAGE_SIZE_BYTE - (addr % PAGE_SIZE_BYTE) ; 
		len = (len > size) ? size : len;

#if defined (WORK_AROUND_RXB_OV)
		// to workaround about NFC's rx buffer overrun error 
		len = (len > 60) ? 60 : len;
#endif

		// set gdma
		set_gdma_ch(p, NFC_DATA, len,  
				BURST_SIZE_4B, HW_MODE, DMA_REQ1, DMA_REQMEM, 
				TRN_FIX, TRN_INC);

		// set NFC
		ra_outl(NFC_INT_ST, ra_inl(NFC_INT_ST));
		ra_outl(NFC_CMD1, cmd1); 	
		ra_outl(NFC_ADDR, bus_addr);
		conf = 0x000141| ((ADDR_CYCLE)<<16) | ((len) << 20);
		conf |= (1<<2); //gdma
		ra_outl(NFC_CONF, conf); 	//fixme, note the length of addr 

		// start and wait dma done
		nand_dma_sync();

		// disable dma
		release_dma_buf();

		addr += len;
		size -= len;
		p += len;
		len -= len;
	}

	return 0;
}

#if 0
void nor_dma_pull(char *dst, char *src, int len)
{
	while (len > 0) {
		int size = (len > ((1<<16) - 4)) ? ((1<<16) - 4) : len;	// limitation is DMA buffer
		// set GDMA 
		set_gdma_ch(dst, src, size,  
			    BURST_SIZE_32B, SW_MODE, DMA_REQMEM, DMA_REQMEM, 
			    TRN_INC, TRN_INC);

		// start and wait dma done
		nand_dma_sync();
	
		// disable dma
		release_dma_buf();

		len -= size;
		dst += size;
		src += size;
	}
	return ;
}
#endif
