#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/memory.h>

#include <asm/rt2880/surfboardint.h>
#include "rt_dma.h"

/************************ DMA engine API functions ****************************/

#define MEMCPY_DMA_CH	8
#define to_rt_dma_chan(chan)            \
	container_of(chan, struct rt_dma_chan, common)

static dma_cookie_t rt_dma_tx_submit(struct dma_async_tx_descriptor *tx)
{
	dma_cookie_t cookie;
	
	cookie = tx->chan->cookie;

	return cookie;
}

#define MIN_RTDMA_PKT_LEN	128
static struct dma_async_tx_descriptor *
rt_dma_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
		size_t len, unsigned long flags)
{
	struct rt_dma_chan *rt_chan = to_rt_dma_chan(chan);
	unsigned long mid_offset;

//	printk("%x->%x len=%d ch=%d\n", src, dest, len, chan->chan_id);

	spin_lock_bh(&rt_chan->lock);

	if(len < MIN_RTDMA_PKT_LEN) {
		memcpy(phys_to_virt(dest), phys_to_virt(src), len);	
	} else {
		mid_offset = len/2;

		/* Lower parts are transferred  by GDMA.
		 * Upper parts are transferred by CPU.
		 */
		RT_DMA_WRITE_REG(RT_DMA_SRC_REG(MEMCPY_DMA_CH), src);
		RT_DMA_WRITE_REG(RT_DMA_DST_REG(MEMCPY_DMA_CH), dest);
		RT_DMA_WRITE_REG(RT_DMA_CTRL_REG(MEMCPY_DMA_CH), (mid_offset << 16) | (3 << 3) | (3 << 0));

		memcpy(phys_to_virt(dest)+mid_offset, phys_to_virt(src)+mid_offset, len-mid_offset);	
		
		dma_async_tx_descriptor_init(&rt_chan->txd, chan);
		
		while((RT_DMA_READ_REG(RT_DMA_DONEINT) & (0x1<<MEMCPY_DMA_CH))==0);
		RT_DMA_WRITE_REG(RT_DMA_DONEINT, (1<<MEMCPY_DMA_CH));
	}

	spin_unlock_bh(&rt_chan->lock);

	return &rt_chan->txd;
}



/**
 * rt_dma_status - poll the status of an XOR transaction
 * @chan: XOR channel handle
 * @cookie: XOR transaction identifier
 * @txstate: XOR transactions state holder (or NULL)
 */
static enum dma_status rt_dma_status(struct dma_chan *chan,
					  dma_cookie_t cookie,
					  struct dma_tx_state *txstate)
{
	return 0;
}


static irqreturn_t rt_dma_interrupt_handler(int irq, void *data)
{

	return IRQ_HANDLED;
}

static void rt_dma_issue_pending(struct dma_chan *chan)
{
}

static int rt_dma_alloc_chan_resources(struct dma_chan *chan)
{
	return 0;
}

static void rt_dma_free_chan_resources(struct dma_chan *chan)
{
}

static int rt_dma_probe(struct platform_device *pdev)
{
	struct dma_device *dma_dev;
	struct rt_dma_chan *rt_chan;
	int err;
	int ret;
	int reg;

	printk("%s\n",__FUNCTION__);
	
	dma_dev = devm_kzalloc(&pdev->dev, sizeof(*dma_dev), GFP_KERNEL);
	if (!dma_dev)
		return -ENOMEM;


	INIT_LIST_HEAD(&dma_dev->channels);
	dma_cap_zero(dma_dev->cap_mask);
	dma_cap_set(DMA_MEMCPY, dma_dev->cap_mask);
	dma_cap_set(DMA_SLAVE, dma_dev->cap_mask);
	dma_dev->device_alloc_chan_resources = rt_dma_alloc_chan_resources;
	dma_dev->device_free_chan_resources = rt_dma_free_chan_resources;
	dma_dev->device_tx_status = rt_dma_status;
	dma_dev->device_issue_pending = rt_dma_issue_pending;
	dma_dev->device_prep_dma_memcpy = rt_dma_prep_dma_memcpy;
	dma_dev->dev = &pdev->dev;

	rt_chan = devm_kzalloc(&pdev->dev, sizeof(*rt_chan), GFP_KERNEL);
        if (!rt_chan) {
		return -ENOMEM;
	}

	spin_lock_init(&rt_chan->lock);
        INIT_LIST_HEAD(&rt_chan->chain);
	INIT_LIST_HEAD(&rt_chan->completed_slots);
	INIT_LIST_HEAD(&rt_chan->all_slots);
	rt_chan->common.device = dma_dev;
	rt_chan->txd.tx_submit = rt_dma_tx_submit;

	list_add_tail(&rt_chan->common.device_node, &dma_dev->channels);
	
	err = dma_async_device_register(dma_dev);
	if (0 != err) {
		pr_err("ERR_MDMA:device_register failed: %d\n", err);
		return 1;
	}

	ret = request_irq(SURFBOARDINT_DMA, rt_dma_interrupt_handler, IRQF_DISABLED, "Ralink_DMA", NULL);
	if(ret){
		pr_err("IRQ %d is not free.\n", SURFBOARDINT_DMA);
		return 1;
	}

	//set GDMA register in advance.
	reg = (32 << 16) | (32 << 8) | (MEMCPY_DMA_CH << 3);
	RT_DMA_WRITE_REG(RT_DMA_CTRL_REG1(MEMCPY_DMA_CH), reg);
	
	return 0;
}


static int rt_dma_remove(struct platform_device *dev)
{
	struct dma_device *dma_dev = platform_get_drvdata(dev);

	dma_async_device_unregister(dma_dev);

	return 0;
}

static struct platform_driver rt_dma_driver = {
	.probe		= rt_dma_probe,
	.remove		= rt_dma_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= RT_DMA_NAME,
	},
};

static int __init rt_dma_init(void)
{
	return platform_driver_register(&rt_dma_driver);
}

module_init(rt_dma_init);


MODULE_AUTHOR("Steven Liu <steven_liu@mediatek.com>");
MODULE_DESCRIPTION("DMA engine driver for Ralink DMA engine");
MODULE_LICENSE("GPL");
