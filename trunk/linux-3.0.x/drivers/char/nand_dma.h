#ifndef NAND_DMA_H
#define NAND_DMA_H

#include "ralink_gdma.h"

int nand_dma_release(void);
int nand_dma_alloc(GdmaReqEntry *req);
int nand_dma_sync(unsigned int virt);
unsigned int nand_dma_buf(void);



#endif
