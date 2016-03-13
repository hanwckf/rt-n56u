#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>

#include <asm/rt2880/surfboardint.h>

#include "mcrypto_aes_api.h"
#include "aes_engine.h"

static struct AesReqEntry AES_Entry;

static int mtk_aes_poll_done(void);

int mtk_aes_process_sg(struct scatterlist* sg_src,
		struct scatterlist* sg_dst,
		struct mcrypto_ctx *ctx,
		unsigned int nbytes,
		unsigned int mode)
{
	struct scatterlist *next_dst, *next_src;
	struct AES_txdesc* txdesc;
	struct AES_rxdesc* rxdesc;
	u32 aes_txd_info4;
	u32 aes_size_total, aes_size_chunk, aes_free_desc;
	u32 aes_tx_scatter = 0;
	u32 aes_rx_gather = 0;
	u32 i = 1, j = 1;
	unsigned long flags = 0;

	next_src = sg_src;
	next_dst = sg_dst;

	while (sg_dma_len(next_src) == 0) {
		if (sg_is_last(next_src))
			return -EINVAL;
		next_src = sg_next(next_src);
	}

	while (sg_dma_len(next_dst) == 0) {
		if (sg_is_last(next_dst))
			return -EINVAL;
		next_dst = sg_next(next_dst);
	}

	if (ctx->keylen == AES_KEYSIZE_256)
		aes_txd_info4 = TX4_DMA_AES_256;
	else if (ctx->keylen == AES_KEYSIZE_192)
		aes_txd_info4 = TX4_DMA_AES_192;
	else
		aes_txd_info4 = TX4_DMA_AES_128;

	if (mode & MCRYPTO_MODE_ENC)
		aes_txd_info4 |= TX4_DMA_ENC;

	if (mode & MCRYPTO_MODE_CBC)
		aes_txd_info4 |= TX4_DMA_CBC | TX4_DMA_IVR;

	spin_lock_irqsave(&AES_Entry.page_lock, flags);

	DBGPRINT(DBG_HIGH, "\nStart new scater, TX [front=%u rear=%u]; RX [front=%u rear=%u]\n",
			AES_Entry.aes_tx_front_idx, AES_Entry.aes_tx_rear_idx,
			AES_Entry.aes_rx_front_idx, AES_Entry.aes_rx_rear_idx);

	aes_size_total = nbytes;

	if (AES_Entry.aes_tx_front_idx > AES_Entry.aes_tx_rear_idx)
		aes_free_desc = NUM_AES_TX_DESC - (AES_Entry.aes_tx_front_idx - AES_Entry.aes_tx_rear_idx);
	else
		aes_free_desc = AES_Entry.aes_tx_rear_idx - AES_Entry.aes_tx_front_idx;

	/* TX descriptor */
	while (1) {
		if (i > aes_free_desc) {
			spin_unlock_irqrestore(&AES_Entry.page_lock, flags);
			return -EAGAIN;
		}
		
		aes_tx_scatter = (AES_Entry.aes_tx_rear_idx + i) % NUM_AES_TX_DESC;
		txdesc = &AES_Entry.AES_tx_ring0[aes_tx_scatter];
		
		if (sg_dma_len(next_src) == 0)
			goto next_desc_tx;
		
		aes_size_chunk = min(aes_size_total, sg_dma_len(next_src));
		
		DBGPRINT(DBG_HIGH, "AES set TX Desc[%u] Src=%08X, len=%d, Key=%08X, klen=%d\n",
			aes_tx_scatter, (u32)sg_virt(next_src), aes_size_chunk, (u32)ctx->key, ctx->keylen);
		
		if ((mode & MCRYPTO_MODE_CBC) && (i == 1)) {
			if (!ctx->iv)
				memset((void*)txdesc->IV, 0xFF, sizeof(uint32_t)*4);
			else
				memcpy((void*)txdesc->IV, ctx->iv, sizeof(uint32_t)*4);
			txdesc->txd_info4 = aes_txd_info4 | TX4_DMA_KIU;
		} else {
			txdesc->txd_info4 = aes_txd_info4;
		}
		
		if (i == 1) {
			txdesc->SDP0 = (u32)dma_map_single(NULL, ctx->key, ctx->keylen, DMA_TO_DEVICE);
			txdesc->txd_info2 = TX2_DMA_SDL0_SET(ctx->keylen);
		} else {
			txdesc->txd_info2 = 0;
		}
		
		txdesc->SDP1 = (u32)dma_map_single(NULL, sg_virt(next_src), aes_size_chunk, DMA_TO_DEVICE);
		txdesc->txd_info2 |= TX2_DMA_SDL1_SET(aes_size_chunk);
		
		i++;
		aes_size_total -= aes_size_chunk;
next_desc_tx:
		if (!aes_size_total || sg_is_last(next_src)) {
			txdesc->txd_info2 |= TX2_DMA_LS1;
			break;
		}
		
		next_src = sg_next(next_src);
	}

	aes_size_total = nbytes;

	if (AES_Entry.aes_rx_front_idx > AES_Entry.aes_rx_rear_idx)
		aes_free_desc = NUM_AES_RX_DESC - (AES_Entry.aes_rx_front_idx - AES_Entry.aes_rx_rear_idx);
	else
		aes_free_desc = AES_Entry.aes_rx_rear_idx - AES_Entry.aes_rx_front_idx;

	/* RX descriptor */
	while (1) {
		if (j > aes_free_desc) {
			spin_unlock_irqrestore(&AES_Entry.page_lock, flags);
			return -EAGAIN;
		}
		
		aes_rx_gather = (AES_Entry.aes_rx_rear_idx + j) % NUM_AES_RX_DESC;
		rxdesc = &AES_Entry.AES_rx_ring0[aes_rx_gather];
		
		if (sg_dma_len(next_dst) == 0)
			goto next_desc_rx;
		
		aes_size_chunk = min(aes_size_total, sg_dma_len(next_dst));
		
		DBGPRINT(DBG_HIGH, "AES set RX Desc[%u] Dst=%08X, len=%d\n",
			aes_rx_gather, (u32)sg_virt(next_dst), aes_size_chunk);
		
		rxdesc->SDP0 = dma_map_single(NULL, sg_virt(next_dst), aes_size_chunk, DMA_FROM_DEVICE);
		rxdesc->rxd_info2 = RX2_DMA_SDL0_SET(aes_size_chunk);
		
		j++;
		aes_size_total -= aes_size_chunk;
next_desc_rx:
		if (!aes_size_total || sg_is_last(next_dst)) {
			rxdesc->rxd_info2 |= RX2_DMA_LS0;
			break;
		}
		
		next_dst = sg_next(next_dst);
	}

	AES_Entry.aes_tx_rear_idx = aes_tx_scatter;
	AES_Entry.aes_rx_rear_idx = aes_rx_gather;

	DBGPRINT(DBG_MID, "TT [front=%u rear=%u]; RR [front=%u rear=%u]\n",
		AES_Entry.aes_tx_front_idx, AES_Entry.aes_tx_rear_idx,
		AES_Entry.aes_rx_front_idx, AES_Entry.aes_rx_rear_idx);

#if defined (CONFIG_CRYPTO_DEV_MTK_AES_INT)
	INIT_COMPLETION(AES_Entry.op_complete);
#endif

	wmb();

	aes_tx_scatter = (aes_tx_scatter + 1) % NUM_AES_TX_DESC;
	sysRegWrite(AES_TX_CTX_IDX0, cpu_to_le32(aes_tx_scatter));

	spin_unlock_irqrestore(&AES_Entry.page_lock, flags);

#if defined (CONFIG_CRYPTO_DEV_MTK_AES_INT)
	if (wait_for_completion_timeout(&AES_Entry.op_complete, msecs_to_jiffies(200)) == 0) {
		printk("\n%s: PDMA timeout!\n", AES_MODNAME);
		return -ETIMEDOUT;
	}
#endif

	return mtk_aes_poll_done();
}

static int mtk_aes_poll_done(void)
{
	struct AES_txdesc* txdesc;
	struct AES_rxdesc* rxdesc;
	u32 k, m, regVal;
	int try_count = 0;
	unsigned long flags = 0;

	do {
		regVal = sysRegRead(AES_GLO_CFG);
		if ((regVal & (AES_RX_DMA_EN | AES_TX_DMA_EN)) != (AES_RX_DMA_EN | AES_TX_DMA_EN))
			return -EIO;
		if (!(regVal & (AES_RX_DMA_BUSY | AES_TX_DMA_BUSY)))
			break;
#if !defined (CONFIG_CRYPTO_DEV_MTK_AES_INT)
		if (++try_count > 1000000) {
			printk("\n%s: PDMA timeout! (AES_GLO_CFG: 0x%08X)\n", AES_MODNAME, regVal);
			return -ETIMEDOUT;
		}
#endif
		cpu_relax();
	} while(1);

	DBGPRINT(DBG_HIGH, "\nPoll done, times: %d\n", try_count);

	spin_lock_irqsave(&AES_Entry.page_lock, flags);

	k = AES_Entry.aes_rx_front_idx;
	m = AES_Entry.aes_tx_front_idx;

	try_count = 0;

	do
	{
		rxdesc = &AES_Entry.AES_rx_ring0[k];
		
		if (!(rxdesc->rxd_info2 & RX2_DMA_DONE)) {
			try_count++;
			cpu_relax();
			continue;
		}
		
		DBGPRINT(DBG_HIGH, "Rx Desc[%d] Done\n", k);
		
		rxdesc->rxd_info2 &= ~RX2_DMA_DONE;
		
		if (rxdesc->rxd_info2 & RX2_DMA_LS0) {
			/* last RX, release correspond TX */
			do
			{
				txdesc = &AES_Entry.AES_tx_ring0[m];
				if (!(txdesc->txd_info2 & TX2_DMA_DONE))
					break;
				
				if (txdesc->txd_info2 & TX2_DMA_LS1)
					break;
				
				m = (m+1) % NUM_AES_TX_DESC;
			} while (1);
			
			AES_Entry.aes_tx_front_idx = (m+1) % NUM_AES_TX_DESC;
			
			if (m == AES_Entry.aes_tx_rear_idx) {
				DBGPRINT(DBG_HIGH, "Tx Desc[%d] Clean\n", AES_Entry.aes_tx_rear_idx);
			}
			
			AES_Entry.aes_rx_front_idx = (k+1) % NUM_AES_RX_DESC;
			
			if (k == AES_Entry.aes_rx_rear_idx) {
				DBGPRINT(DBG_HIGH, "Rx Desc[%d] Clean\n", AES_Entry.aes_rx_rear_idx);
				break;
			}
		}
		
		k = (k+1) % NUM_AES_RX_DESC;
	} while(1);

	AES_Entry.aes_rx_rear_idx = k;

	wmb();
	sysRegWrite(AES_RX_CALC_IDX0, cpu_to_le32(k));

	spin_unlock_irqrestore(&AES_Entry.page_lock, flags);

	return 0;
}

#if defined (CONFIG_CRYPTO_DEV_MTK_AES_INT)
irqreturn_t AesEngineIrqHandler(int irq, void *irqaction)
{
	sysRegWrite(AES_INT_STATUS, AES_MASK_INT_ALL);

	complete(&AES_Entry.op_complete);

	return IRQ_HANDLED;
}
#endif

static void aes_engine_reset(void)
{
	u32 val;

	val = sysRegRead(REG_CLKCTRL);
	val |= RALINK_CRYPTO_CLK_EN;
	sysRegWrite(REG_CLKCTRL, val);

	udelay(10);

	val = sysRegRead(REG_RSTCTRL);
	val |= RALINK_CRYPTO_RST;
	sysRegWrite(REG_RSTCTRL, val);

	udelay(10);

	val &= ~(RALINK_CRYPTO_RST);
	sysRegWrite(REG_RSTCTRL, val);

	udelay(100);
}

static void aes_engine_stop(void)
{
	int i;
	u32 regValue;

	regValue = sysRegRead(AES_GLO_CFG);
	regValue &= ~(AES_TX_WB_DDONE | AES_RX_DMA_EN | AES_TX_DMA_EN);
	sysRegWrite(AES_GLO_CFG, regValue);

	/* wait AES stopped */
	for (i = 0; i < 50; i++) {
		msleep(1);
		regValue = sysRegRead(AES_GLO_CFG);
		if (!(regValue & (AES_RX_DMA_BUSY | AES_TX_DMA_BUSY)))
			break;
	}

	/* disable AES interrupt */
	sysRegWrite(AES_INT_MASK, 0);
}

static void aes_engine_start(void)
{
	u32 AES_glo_cfg = AES_TX_DMA_EN | AES_RX_DMA_EN | AES_TX_WB_DDONE | AES_DESC_5DW_INFO_EN | AES_RX_ANYBYTE_ALIGN;

	sysRegWrite(AES_DLY_INT_CFG, AES_DLY_INIT_VALUE);
	sysRegWrite(AES_INT_STATUS, 0xffffffff);
#if defined (CONFIG_CRYPTO_DEV_MTK_AES_INT)
	sysRegWrite(AES_INT_MASK, AES_MASK_INT_ALL);
#endif

	AES_glo_cfg |= AES_BT_SIZE_16DWORDS;
	sysRegWrite(AES_GLO_CFG, AES_glo_cfg);
}

static void aes_engine_desc_free(void)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&AES_Entry.page_lock, flags);

	sysRegWrite(AES_TX_BASE_PTR0, 0);
	sysRegWrite(AES_RX_BASE_PTR0, 0);

	if (AES_Entry.AES_tx_ring0) {
		dma_free_coherent(NULL, NUM_AES_TX_DESC * sizeof(struct AES_txdesc), AES_Entry.AES_tx_ring0, AES_Entry.phy_aes_tx_ring0);
		AES_Entry.AES_tx_ring0 = NULL;
		AES_Entry.phy_aes_tx_ring0 = 0;
	}

	if (AES_Entry.AES_rx_ring0) {
		dma_free_coherent(NULL, NUM_AES_RX_DESC * sizeof(struct AES_rxdesc), AES_Entry.AES_rx_ring0, AES_Entry.phy_aes_rx_ring0);
		AES_Entry.AES_rx_ring0 = NULL;
		AES_Entry.phy_aes_rx_ring0 = 0;
	}

	spin_unlock_irqrestore(&AES_Entry.page_lock, flags);
}

static int aes_engine_desc_init(void)
{
	int i;
	u32 regVal;

	AES_Entry.AES_tx_ring0 = dma_alloc_coherent(NULL, NUM_AES_TX_DESC * sizeof(struct AES_txdesc), &AES_Entry.phy_aes_tx_ring0, GFP_KERNEL);
	if (!AES_Entry.AES_tx_ring0)
		goto err_cleanup;

	AES_Entry.AES_rx_ring0 = dma_alloc_coherent(NULL, NUM_AES_RX_DESC * sizeof(struct AES_rxdesc), &AES_Entry.phy_aes_rx_ring0, GFP_KERNEL);
	if (!AES_Entry.AES_rx_ring0)
		goto err_cleanup;

	for (i = 0; i < NUM_AES_TX_DESC; i++) {
		memset(&AES_Entry.AES_tx_ring0[i], 0, sizeof(struct AES_txdesc));
		AES_Entry.AES_tx_ring0[i].txd_info2 |= TX2_DMA_DONE;
	}

	for (i = 0; i < NUM_AES_RX_DESC; i++) {
		memset(&AES_Entry.AES_rx_ring0[i], 0, sizeof(struct AES_rxdesc));
	}

	AES_Entry.aes_tx_front_idx = 0;
	AES_Entry.aes_tx_rear_idx = NUM_AES_TX_DESC-1;

	AES_Entry.aes_rx_front_idx = 0;
	AES_Entry.aes_rx_rear_idx = NUM_AES_RX_DESC-1;

	wmb();

	regVal = sysRegRead(AES_GLO_CFG);
	regVal &= 0x00000ff0;
	sysRegWrite(AES_GLO_CFG, regVal);
	regVal = sysRegRead(AES_GLO_CFG);

	sysRegWrite(AES_TX_BASE_PTR0, phys_to_bus((u32)AES_Entry.phy_aes_tx_ring0));
	sysRegWrite(AES_TX_MAX_CNT0, cpu_to_le32((u32)NUM_AES_TX_DESC));
	sysRegWrite(AES_TX_CTX_IDX0, 0);
	sysRegWrite(AES_RST_CFG, AES_PST_DTX_IDX0);

	sysRegWrite(AES_RX_BASE_PTR0, phys_to_bus((u32)AES_Entry.phy_aes_rx_ring0));
	sysRegWrite(AES_RX_MAX_CNT0, cpu_to_le32((u32)NUM_AES_RX_DESC));
	sysRegWrite(AES_RX_CALC_IDX0, cpu_to_le32((u32)(NUM_AES_RX_DESC - 1)));
	regVal = sysRegRead(AES_RX_CALC_IDX0);
	sysRegWrite(AES_RST_CFG, AES_PST_DRX_IDX0);

	return 0;

err_cleanup:
	aes_engine_desc_free();
	return -ENOMEM;
}

static void aes_engine_uninit(void)
{
	aes_engine_stop();
	aes_engine_desc_free();

#if defined (CONFIG_CRYPTO_DEV_MTK_AES_INT)
	free_irq(SURFBOARDINT_AESENGINE, NULL);
#endif
}

static int __init AesEngineInit(void)
{
	int err;

	spin_lock_init(&AES_Entry.page_lock);

	aes_engine_reset();

	printk("MTK AES Engine Module, HW verson: %02X\n", sysRegRead(AES_INFO) >> 28);

	err = aes_engine_desc_init();
	if (err != 0) {
		printk(KERN_WARNING "%s: ring_alloc FAILED!\n", AES_MODNAME);
		return err;
	}

#if defined (CONFIG_CRYPTO_DEV_MTK_AES_INT)
	init_completion(&AES_Entry.op_complete);

	err = request_irq(SURFBOARDINT_AESENGINE, AesEngineIrqHandler, IRQF_DISABLED, "aes_engine", NULL);
	if (err) {
		printk("%s: IRQ %d is not free!\n", AES_MODNAME, SURFBOARDINT_AESENGINE);
		aes_engine_desc_free();
		return err;
	}
#endif

	aes_engine_start();

	printk("%s: register %s crypto api\n", AES_MODNAME, mcrypto_aes_cbc_alg.cra_name);
	err = crypto_register_alg(&mcrypto_aes_cbc_alg);
	if (err) {
		printk("%s: register %s crypto api failed!\n", AES_MODNAME, mcrypto_aes_cbc_alg.cra_name);
		goto init_failed;
	}

	printk("%s: register %s crypto api\n", AES_MODNAME, mcrypto_aes_ecb_alg.cra_name);
	err = crypto_register_alg(&mcrypto_aes_ecb_alg);
	if (err) {
		printk("%s: register %s crypto api failed!\n", AES_MODNAME, mcrypto_aes_ecb_alg.cra_name);
		crypto_unregister_alg(&mcrypto_aes_cbc_alg);
		goto init_failed;
	}

	return 0;

init_failed:

	aes_engine_uninit();

	return err;
}

static void __exit AesEngineExit(void)
{
	crypto_unregister_alg(&mcrypto_aes_ecb_alg);
	crypto_unregister_alg(&mcrypto_aes_cbc_alg);

	aes_engine_uninit();
}

module_init(AesEngineInit);
module_exit(AesEngineExit);

MODULE_DESCRIPTION("MTK AES Engine Module");
MODULE_AUTHOR("Qwert");
MODULE_LICENSE("GPL");
