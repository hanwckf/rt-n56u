#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/sched.h>
#include "ralink_gdma.h"

#define ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))
#define ra_aor(addr, a_mask, o_value)  ra_outl(addr, (ra_inl(addr) & (a_mask)) | (o_value))


dma_addr_t dma_handle;
unsigned long dma_addr = 0;
int chnum = -1;

static int set_gdma_ch(GdmaReqEntry *req)
{
	int ret;

	if (_GdmaGetFreeCh(&chnum) != 1) {
		printk("no free dma_ch\n");
		return -1;
	}

	req->ChNum = chnum;
	ret = _GdmaReqEntryIns(req);
	
	if (ret != 1)
		return -1;
	else
		return 0;
}




int nand_dma_release(void)
{
	if (chnum >= 0 && chnum < MAX_GDMA_CHANNEL) {
		_GdmaPutFreeCh(chnum);
		chnum = -1;
	}

	if (dma_addr != 0) {
		dma_free_coherent(NULL, PAGE_SIZE, dma_addr, dma_handle);
		dma_addr = 0;
	}

	return 0;
}


int nand_dma_alloc(GdmaReqEntry *req)
{
	int ret;
	int mask;

	BUG_ON (req->SrcReqNum == DMA_MEM_REQ && (req->Dst != 0xb0000818 || req->Src != 0));
	BUG_ON (req->DstReqNum == DMA_MEM_REQ && (req->Src != 0xb0000818 || req->Dst != 0));


	if (dma_addr == 0) {
		dma_addr = dma_alloc_coherent(NULL, PAGE_SIZE, &dma_handle, GFP_ATOMIC);
		//printk("dma alloc: %x, phy:%x \n", dma_addr, dma_handle);
	}
	if (dma_addr == 0) {
		printk("can not alloc dma buffer \n");
		return -1;
	}

	if (req->DstReqNum == DMA_MEM_REQ) {
		req->Dst = dma_addr;
	}
	else if (req->SrcReqNum == DMA_MEM_REQ) {
		req->Src = dma_addr;
	}
	else {
		nand_dma_release();
		printk("can not set DMA address \n");
		return -1;
	}
		
	// sanity check...
	req->Dst &= 0x1fffffff;
	req->Src &= 0x1fffffff;
	mask = req->ChMask;
	req->ChMask = 1;
	ret = set_gdma_ch(req);
	if (ret < 0) {
		printk("get free DMA channel fail \n");
		nand_dma_release();
		return -1;
	}


	return 0;
}


int nand_dma_sync(unsigned int virt)
{	

	dma_cache_wback(virt, PAGE_SIZE);

	GdmaUnMaskChannel(chnum);
	// sync status
	while(!(GDMA_READ_REG(RALINK_GDMAISTS) & (1<<chnum))) {
		if (signal_pending(current))
			return -EINTR;
		
		// do nothing
	}

	
	GDMA_WRITE_REG(RALINK_GDMAISTS, 1<<chnum);
	dma_cache_inv(virt, PAGE_SIZE);
	
	return 0;
}

unsigned int nand_dma_buf(void)
{
	return dma_addr;
}

EXPORT_SYMBOL(nand_dma_buf);
EXPORT_SYMBOL(nand_dma_alloc);
EXPORT_SYMBOL(nand_dma_release);
EXPORT_SYMBOL(nand_dma_sync);

MODULE_VERSION(MOD_VERSION);
