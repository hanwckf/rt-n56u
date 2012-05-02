#ifndef __LIBATA_COMPAT_H__
#define __LIBATA_COMPAT_H__

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/slab.h>

typedef u32 __le32;
typedef u64 __le64;

#define DMA_64BIT_MASK 0xffffffffffffffffULL
#define DMA_32BIT_MASK 0x00000000ffffffffULL

#define MODULE_VERSION(ver_str)

struct device {
	struct pci_dev pdev;
};

static inline struct pci_dev *to_pci_dev(struct device *dev)
{
	return (struct pci_dev *) dev;
}

#define pci_set_consistent_dma_mask(pdev,mask) (0)

#define DMA_FROM_DEVICE PCI_DMA_FROMDEVICE

/* NOTE: dangerous! we ignore the 'gfp' argument */
#define dma_alloc_coherent(dev,sz,dma,gfp) \
	pci_alloc_consistent(to_pci_dev(dev),(sz),(dma))
#define dma_free_coherent(dev,sz,addr,dma_addr) \
	pci_free_consistent(to_pci_dev(dev),(sz),(addr),(dma_addr))

#define dma_map_sg(dev,a,b,c) \
	pci_map_sg(to_pci_dev(dev),(a),(b),(c))
#define dma_unmap_sg(dev,a,b,c) \
	pci_unmap_sg(to_pci_dev(dev),(a),(b),(c))

#define dma_map_single(dev,a,b,c) \
	pci_map_single(to_pci_dev(dev),(a),(b),(c))
#define dma_unmap_single(dev,a,b,c) \
	pci_unmap_single(to_pci_dev(dev),(a),(b),(c))

#define dma_mapping_error(addr) (0)

#define dev_get_drvdata(dev) \
	pci_get_drvdata(to_pci_dev(dev))
#define dev_set_drvdata(dev,ptr) \
	pci_set_drvdata(to_pci_dev(dev),(ptr))

static inline void *kcalloc(size_t nmemb, size_t size, int flags)
{
	size_t total = nmemb * size;
	void *mem = kmalloc(total, flags);
	if (mem)
		memset(mem, 0, total);
	return mem;
}

#endif /* __LIBATA_COMPAT_H__ */
