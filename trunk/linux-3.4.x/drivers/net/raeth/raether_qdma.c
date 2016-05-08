/*
 * QDMA implementation (MT7621/MT7623 only)
 * Note: this file is part of raether.c
 */

u8  M2Q_table[64] = {0};
int M2Q_wan_lan = 0;
EXPORT_SYMBOL(M2Q_table);
EXPORT_SYMBOL(M2Q_wan_lan);

static void
fe_dma_ring_free(END_DEVICE *ei_local)
{
	u32 i;

	/* free PDMA (or QDMA) RX buffers */
	for (i = 0; i < NUM_RX_DESC; i++) {
		if (ei_local->rxd_buff[i]) {
			dev_kfree_skb(ei_local->rxd_buff[i]);
			ei_local->rxd_buff[i] = NULL;
		}
	}

	/* free PDMA (or QDMA) RX descriptors */
	if (ei_local->rxd_ring) {
		dma_free_coherent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), ei_local->rxd_ring, ei_local->rxd_ring_phy);
		ei_local->rxd_ring = NULL;
	}

#if !defined (CONFIG_RAETH_QDMATX_QDMARX)
	/* free QDMA RX stub buffer */
	if (ei_local->qrx_buff) {
		dev_kfree_skb(ei_local->qrx_buff);
		ei_local->qrx_buff = NULL;
	}

	/* free QDMA RX stub descriptors */
	if (ei_local->qrx_ring) {
		dma_free_coherent(NULL, NUM_QRX_DESC * sizeof(struct PDMA_rxdesc), ei_local->qrx_ring, ei_local->qrx_ring_phy);
		ei_local->qrx_ring = NULL;
	}
#endif

	/* free QDMA SW TX descriptors */
	if (ei_local->txd_pool) {
		dma_free_coherent(NULL, NUM_TX_DESC * sizeof(struct QDMA_txdesc), ei_local->txd_pool, ei_local->txd_pool_phy);
		ei_local->txd_pool = NULL;
	}

	/* free QDMA HW TX descriptors */
	if (ei_local->fq_head) {
		dma_free_coherent(NULL, NUM_QDMA_PAGE * sizeof(struct QDMA_txdesc), ei_local->fq_head, ei_local->fq_head_phy);
		ei_local->fq_head = NULL;
	}

	/* free QDMA HW TX pool */
	if (ei_local->fq_head_page) {
		dma_free_coherent(NULL, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, ei_local->fq_head_page, ei_local->fq_head_page_phy);
		ei_local->fq_head_page = NULL;
	}
}

static int
fe_dma_ring_alloc(END_DEVICE *ei_local)
{
	u32 i;

	/* allocate QDMA HW TX pool */
	ei_local->fq_head_page = dma_alloc_coherent(NULL, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, &ei_local->fq_head_page_phy, GFP_KERNEL);
	if (!ei_local->fq_head_page)
		goto err_cleanup;

	/* allocate QDMA HW TX descriptors */
	ei_local->fq_head = dma_alloc_coherent(NULL, NUM_QDMA_PAGE * sizeof(struct QDMA_txdesc), &ei_local->fq_head_phy, GFP_KERNEL);
	if (!ei_local->fq_head)
		goto err_cleanup;

	/* allocate QDMA SW TX descriptors */
	ei_local->txd_pool = dma_alloc_coherent(NULL, NUM_TX_DESC * sizeof(struct QDMA_txdesc), &ei_local->txd_pool_phy, GFP_KERNEL);
	if (!ei_local->txd_pool)
		goto err_cleanup;

	/* allocate PDMA (or QDMA) RX descriptors */
	ei_local->rxd_ring = dma_alloc_coherent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->rxd_ring_phy, GFP_KERNEL);
	if (!ei_local->rxd_ring)
		goto err_cleanup;

#if !defined (CONFIG_RAETH_QDMATX_QDMARX)
	/* allocate QDMA RX stub descriptors */
	ei_local->qrx_ring = dma_alloc_coherent(NULL, NUM_QRX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->qrx_ring_phy, GFP_KERNEL);
	if (!ei_local->qrx_ring)
		goto err_cleanup;

	/* allocate QDMA RX stub buffer */
	ei_local->qrx_buff = __dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN, GFP_KERNEL);
	if (!ei_local->qrx_buff)
		goto err_cleanup;
#endif

	/* allocate PDMA (or QDMA) RX buffers */
	for (i = 0; i < NUM_RX_DESC; i++) {
		ei_local->rxd_buff[i] = __dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN, GFP_KERNEL);
		if (!ei_local->rxd_buff[i])
			goto err_cleanup;
	}

	return 0;

err_cleanup:
	fe_dma_ring_free(ei_local);
	return -ENOMEM;
}

static inline u32
get_txd_offset(END_DEVICE *ei_local, dma_addr_t txd_ptr_phy)
{
	return (txd_ptr_phy - ei_local->txd_pool_phy) / sizeof(struct QDMA_txdesc);
}

static inline dma_addr_t
get_txd_ptr_phy(END_DEVICE *ei_local, u32 txd_offset)
{
	return (ei_local->txd_pool_phy + (txd_offset * sizeof(struct QDMA_txdesc)));
}

/* must be spinlock protected */
static u32
get_free_txd(END_DEVICE *ei_local)
{
	u32 txd_idx = ei_local->txd_pool_free_head;

	ei_local->txd_pool_free_head = ei_local->txd_pool_info[txd_idx];
	ei_local->txd_pool_free_num--;

	return txd_idx;
}

/* must be spinlock protected */
static inline void
put_free_txd(END_DEVICE *ei_local, u32 free_txd_idx)
{
	ei_local->txd_pool_info[ei_local->txd_pool_free_tail] = free_txd_idx;
	ei_local->txd_pool_free_tail = free_txd_idx;
	ei_local->txd_pool_free_num++;
}

/* must be spinlock protected */
static void
fe_dma_init(END_DEVICE *ei_local)
{
	u32 i, txd_idx, regVal;
	dma_addr_t rxd_buf_phy, fq_tail_phy, txd_free_phy;

	/* init QDMA HW TX pool */
	for (i = 0; i < NUM_QDMA_PAGE; i++) {
		struct QDMA_txdesc *txd = &ei_local->fq_head[i];
		dma_addr_t fq_buf_phy, fq_ndp_phy;
		
		fq_buf_phy = ei_local->fq_head_page_phy + (i * QDMA_PAGE_SIZE);
		if (i < (NUM_QDMA_PAGE-1))
			fq_ndp_phy = ei_local->fq_head_phy + ((i+1) * sizeof(struct QDMA_txdesc));
		else
			fq_ndp_phy = ei_local->fq_head_phy;
		
		ACCESS_ONCE(txd->txd_info1) = (u32)fq_buf_phy;
		ACCESS_ONCE(txd->txd_info2) = (u32)fq_ndp_phy;
		ACCESS_ONCE(txd->txd_info3) = TX3_QDMA_SDL(QDMA_PAGE_SIZE);
		ACCESS_ONCE(txd->txd_info4) = 0;
	}

	fq_tail_phy = ei_local->fq_head_phy + ((NUM_QDMA_PAGE-1) * sizeof(struct QDMA_txdesc));

	/* init QDMA SW TX pool */
	for (i = 0; i < NUM_TX_DESC; i++) {
		struct QDMA_txdesc *txd = &ei_local->txd_pool[i];
		
		ei_local->txd_buff[i] = NULL;
		ei_local->txd_pool_info[i] = i + 1;
		
		ACCESS_ONCE(txd->txd_info1) = 0;
		ACCESS_ONCE(txd->txd_info2) = 0;
		ACCESS_ONCE(txd->txd_info3) = TX3_QDMA_LS | TX3_QDMA_OWN;
		ACCESS_ONCE(txd->txd_info4) = 0;
	}

	ei_local->txd_pool_free_head = 0;
	ei_local->txd_pool_free_tail = NUM_TX_DESC - 1;
	ei_local->txd_pool_free_num = NUM_TX_DESC;

	/* init PDMA (or QDMA) RX ring */
	for (i = 0; i < NUM_RX_DESC; i++) {
		struct PDMA_rxdesc *rxd = &ei_local->rxd_ring[i];
		
		rxd_buf_phy = dma_map_single(NULL, ei_local->rxd_buff[i]->data, MAX_RX_LENGTH + NET_IP_ALIGN, DMA_FROM_DEVICE);
		
		ACCESS_ONCE(rxd->rxd_info1) = (u32)rxd_buf_phy;
		ACCESS_ONCE(rxd->rxd_info2) = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
		ACCESS_ONCE(rxd->rxd_info3) = 0;
		ACCESS_ONCE(rxd->rxd_info4) = 0;
	}

#if !defined (CONFIG_RAETH_QDMATX_QDMARX)
	/* init QDMA RX stub ring (map one buffer to all RXD) */
	rxd_buf_phy = dma_map_single(NULL, ei_local->qrx_buff->data, MAX_RX_LENGTH + NET_IP_ALIGN, DMA_FROM_DEVICE);

	for (i = 0; i < NUM_QRX_DESC; i++) {
		struct PDMA_rxdesc *rxd = &ei_local->qrx_ring[i];
		
		ACCESS_ONCE(rxd->rxd_info1) = (u32)rxd_buf_phy;
		ACCESS_ONCE(rxd->rxd_info2) = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
		ACCESS_ONCE(rxd->rxd_info3) = 0;
		ACCESS_ONCE(rxd->rxd_info4) = 0;
	}
#endif

	wmb();

	/* clear QDMA */
	regVal = sysRegRead(QDMA_GLO_CFG);
	regVal &= ~(CSR_CLKGATE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(QDMA_GLO_CFG, regVal);

	/* clear PDMA */
	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= ~(CSR_CLKGATE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regVal);

	/* PPE QoS -> QDMA HW TX pool */
	sysRegWrite(QDMA_FQ_HEAD, (u32)ei_local->fq_head_phy);
	sysRegWrite(QDMA_FQ_TAIL, (u32)fq_tail_phy);
	sysRegWrite(QDMA_FQ_CNT,  cpu_to_le32((NUM_TX_DESC << 16) | NUM_QDMA_PAGE));
	sysRegWrite(QDMA_FQ_BLEN, cpu_to_le32(QDMA_PAGE_SIZE << 16));

#if defined (CONFIG_RAETH_QDMATX_QDMARX)
	/* GDMA1/2 -> QDMA RX ring #0 */
	sysRegWrite(QRX_BASE_PTR0, phys_to_bus((u32)ei_local->rxd_ring_phy));
	sysRegWrite(QRX_MAX_CNT0, cpu_to_le32(NUM_RX_DESC));
	sysRegWrite(QRX_CRX_IDX0, cpu_to_le32(NUM_RX_DESC - 1));
#else
	/* GDMA1/2 -> PDMA RX ring #0 */
	sysRegWrite(RX_BASE_PTR0, phys_to_bus((u32)ei_local->rxd_ring_phy));
	sysRegWrite(RX_MAX_CNT0,  cpu_to_le32(NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX0, cpu_to_le32(NUM_RX_DESC - 1));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX0);

	/* GDMA1/2 -> QDMA RX stub ring #0 (not used, but RX DMA started) */
	sysRegWrite(QRX_BASE_PTR0, phys_to_bus((u32)ei_local->qrx_ring_phy));
	sysRegWrite(QRX_MAX_CNT0, cpu_to_le32(NUM_QRX_DESC));
	sysRegWrite(QRX_CRX_IDX0, cpu_to_le32(NUM_QRX_DESC-1));
#endif
	sysRegWrite(QDMA_RST_CFG, PST_DRX_IDX0);

	/* Reserve 4 TXD for each physical queue */
	for (i = 0; i < 16; i++)
		sysRegWrite(QTX_CFG_0 + 0x10*i, ((NUM_PQ_RESV << 8) | NUM_PQ_RESV));

	/* get free txd from pool for RLS (release) */
	txd_idx = get_free_txd(ei_local);
	txd_free_phy = get_txd_ptr_phy(ei_local, txd_idx);
	sysRegWrite(QTX_CRX_PTR, (u32)txd_free_phy);
	sysRegWrite(QTX_DRX_PTR, (u32)txd_free_phy);

	/* get free txd from pool for FWD (forward) */
	txd_idx = get_free_txd(ei_local);
	txd_free_phy = get_txd_ptr_phy(ei_local, txd_idx);
	ei_local->txd_last_idx = txd_idx;
	sysRegWrite(QTX_CTX_PTR, (u32)txd_free_phy);
	sysRegWrite(QTX_DTX_PTR, (u32)txd_free_phy);

	/* reset TX indexes for queues 0~15 */
	sysRegWrite(QDMA_RST_CFG, 0xffff);

	/* enable random early drop and set drop threshold automatically */
	sysRegWrite(QDMA_FC_THRES, 0x174444);
	sysRegWrite(QDMA_HRED2, 0x0);

	/* config DLY interrupt */
	sysRegWrite(DLY_INT_CFG, FE_DLY_INIT_VALUE);
	sysRegWrite(QDMA_DELAY_INT, FE_DLY_INIT_VALUE);
}

static void
fe_dma_uninit(END_DEVICE *ei_local)
{
	int i;

	spin_lock_bh(&ei_local->page_lock);

	/* free uncompleted QDMA TX buffers */
	for (i = 0; i < NUM_TX_DESC; i++) {
		if (ei_local->txd_buff[i]) {
			dev_kfree_skb(ei_local->txd_buff[i]);
			ei_local->txd_buff[i] = NULL;
		}
	}

	spin_unlock_bh(&ei_local->page_lock);
}

static void
fe_dma_clear_addr(void)
{
	/* clear adapter QDMA HW TX pool */
	sysRegWrite(QDMA_FQ_HEAD, 0);
	sysRegWrite(QDMA_FQ_TAIL, 0);
	sysRegWrite(QDMA_FQ_CNT, 0);

	/* clear adapter QDMA SW TX pool */
	sysRegWrite(QTX_CTX_PTR, 0);
	sysRegWrite(QTX_CRX_PTR, 0);

	/* clear adapter QDMA RX ring */
	sysRegWrite(QRX_BASE_PTR0, 0);
	sysRegWrite(QRX_MAX_CNT0, 0);

	/* clear adapter PDMA RX ring */
	sysRegWrite(RX_BASE_PTR0, 0);
	sysRegWrite(RX_MAX_CNT0,  0);
}

static inline void
qdma_write_skb_fragment(END_DEVICE *ei_local,
			dma_addr_t frag_addr,
			u32 frag_size,
			const u32 txd_info3,
			const u32 txd_info4,
			struct sk_buff *skb,
			const bool ls)
{
	u32 fwd_idx = ei_local->txd_last_idx;

	while (frag_size > 0) {
		const u32 part_addr = (u32)frag_addr;
		const u32 part_size = min_t(u32, frag_size, TXD_MAX_SEG_SIZE);
		struct QDMA_txdesc *txd;
		u32 ndp_idx, info3;
		
		frag_addr += part_size;
		frag_size -= part_size;
		
		info3 = txd_info3 | TX3_QDMA_SDL(part_size);
		if (ls && frag_size == 0) {
			info3 |= TX3_QDMA_LS;
			
			/* store skb with LS bit */
			ei_local->txd_buff[fwd_idx] = skb;
		}
		
		ndp_idx = get_free_txd(ei_local);
		
		txd = &ei_local->txd_pool[fwd_idx];
		
		ACCESS_ONCE(txd->txd_info1) = part_addr;
		ACCESS_ONCE(txd->txd_info2) = (u32)get_txd_ptr_phy(ei_local, ndp_idx);
		ACCESS_ONCE(txd->txd_info4) = txd_info4;
		ACCESS_ONCE(txd->txd_info3) = info3;
		
		fwd_idx = ndp_idx;
	}

	ei_local->txd_last_idx = fwd_idx;
}

static inline int
dma_xmit(struct sk_buff *skb, struct net_device *dev, END_DEVICE *ei_local, int gmac_no)
{
	struct netdev_queue *txq;
	dma_addr_t frag_addr;
	u32 frag_size, nr_desc;
	u32 txd_info3, txd_info4;
#if defined (CONFIG_RAETH_SG_DMA_TX)
	u32 i, nr_frags;
	const skb_frag_t *tx_frag;
	const struct skb_shared_info *shinfo;
#else
#define nr_frags 0
#endif

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if (ra_sw_nat_hook_tx != NULL) {
#if defined (CONFIG_RA_HW_NAT_WIFI) || defined (CONFIG_RA_HW_NAT_PCI)
		if (IS_DPORT_PPE_VALID(skb))
			gmac_no = PSE_PORT_PPE;
		else
#endif
		if (ra_sw_nat_hook_tx(skb, gmac_no) == 0) {
			dev_kfree_skb(skb);
			return NETDEV_TX_OK;
		}
	}
#endif

	txd_info3 = TX3_QDMA_SWC;
	if (gmac_no != PSE_PORT_PPE) {
		u32 QID = M2Q_table[(skb->mark & 0x3f)];
		if (QID < 8 && M2Q_wan_lan) {
#if defined (CONFIG_PSEUDO_SUPPORT)
			if (gmac_no == PSE_PORT_GMAC2)
				QID += 8;
#elif defined (CONFIG_RAETH_HW_VLAN_TX)
			if ((skb_vlan_tag_get(skb) & VLAN_VID_MASK) > 1)
				QID += 8;
#endif
		}
		txd_info3 |= TX3_QDMA_QID(QID);
	}

	txd_info4 = TX4_DMA_FPORT(gmac_no);

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD)
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		txd_info4 |= TX4_DMA_TUI_CO(7);
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX)
	if (skb_vlan_tag_present(skb))
		txd_info4 |= (0x10000 | skb_vlan_tag_get(skb));
#endif

#if defined (CONFIG_RAETH_SG_DMA_TX)
	shinfo = skb_shinfo(skb);
#endif

#if defined (CONFIG_RAETH_TSO)
	/* fill MSS info in tcp checksum field */
	if (shinfo->gso_size) {
		u32 hdr_len;
		
		if (!(shinfo->gso_type & (SKB_GSO_TCPV4|SKB_GSO_TCPV6))) {
			dev_kfree_skb(skb);
			return NETDEV_TX_OK;
		}
		
		if (skb_header_cloned(skb)) {
			if (pskb_expand_head(skb, 0, 0, GFP_ATOMIC)) {
				dev_kfree_skb(skb);
				return NETDEV_TX_OK;
			}
		}
		
		hdr_len = (skb_transport_offset(skb) + tcp_hdrlen(skb));
		if (hdr_len >= skb->len) {
			dev_kfree_skb(skb);
			return NETDEV_TX_OK;
		}
		
		tcp_hdr(skb)->check = htons(shinfo->gso_size);
		txd_info4 |= TX4_DMA_TSO;
	}
#endif

	nr_desc = DIV_ROUND_UP(skb_headlen(skb), TXD_MAX_SEG_SIZE);
#if defined (CONFIG_RAETH_SG_DMA_TX)
	nr_frags = (u32)shinfo->nr_frags;

	for (i = 0; i < nr_frags; i++) {
		tx_frag = &shinfo->frags[i];
		nr_desc += DIV_ROUND_UP(skb_frag_size(tx_frag), TXD_MAX_SEG_SIZE);
	}
#endif

	txq = netdev_get_tx_queue(dev, 0);

	/* flush main skb part before spin_lock() */
	frag_size = (u32)skb_headlen(skb);
	frag_addr = dma_map_single(NULL, skb->data, frag_size, DMA_TO_DEVICE);

	/* protect TX ring access (from eth2/eth3 queues) */
	spin_lock(&ei_local->page_lock);

	/* check nr_desc+2 free descriptors (2 need to prevent head/tail overlap) */
	if (ei_local->txd_pool_free_num < (nr_desc+2)) {
		spin_unlock(&ei_local->page_lock);
		netif_tx_stop_queue(txq);
#if defined (CONFIG_RAETH_DEBUG)
		if (net_ratelimit())
			printk("%s: QDMA TX pool is run out! (GMAC: %d)\n", RAETH_DEV_NAME, gmac_no);
#endif
		return NETDEV_TX_BUSY;
	}

	qdma_write_skb_fragment(ei_local, frag_addr, frag_size,
				txd_info3, txd_info4, skb, nr_frags == 0);
#if defined (CONFIG_RAETH_SG_DMA_TX)
	for (i = 0; i < nr_frags; i++) {
		tx_frag = &shinfo->frags[i];
		frag_size = skb_frag_size(tx_frag);
		frag_addr = skb_frag_dma_map(NULL, tx_frag, 0, frag_size, DMA_TO_DEVICE);
		qdma_write_skb_fragment(ei_local, frag_addr, frag_size,
					txd_info3, txd_info4, skb, i == nr_frags - 1);
	}
#endif

#if defined (CONFIG_RAETH_BQL)
	netdev_tx_sent_queue(txq, skb->len);
#endif

#if !defined (CONFIG_RAETH_BQL) || !defined (CONFIG_SMP)
	/* smp_mb() already inlined in netdev_tx_sent_queue */
	wmb();
#endif

	/* kick the QDMA TX */
	sysRegWrite(QTX_CTX_PTR, (u32)get_txd_ptr_phy(ei_local, ei_local->txd_last_idx));

	spin_unlock(&ei_local->page_lock);

	return NETDEV_TX_OK;
}

static inline void
dma_xmit_clean(struct net_device *dev, END_DEVICE *ei_local)
{
	struct netdev_queue *txq;
	int cpu, clean_done = 0;
	u32 cpu_ptr, dma_ptr, cpu_idx;
#if defined (CONFIG_RAETH_BQL)
	u32 bytes_sent_ge1 = 0;
#if defined (CONFIG_PSEUDO_SUPPORT)
	u32 bytes_sent_ge2 = 0;
#endif
#endif

	spin_lock(&ei_local->page_lock);

	cpu_ptr = sysRegRead(QTX_CRX_PTR);
	dma_ptr = sysRegRead(QTX_DRX_PTR);

	/* get current CPU TXD index */
	cpu_idx = get_txd_offset(ei_local, cpu_ptr);

	while (cpu_ptr != dma_ptr) {
		struct QDMA_txdesc *txd;
		struct sk_buff *skb;
		
		txd = &ei_local->txd_pool[cpu_idx];
		
		/* check TXD not owned by DMA */
		if (!(ACCESS_ONCE(txd->txd_info3) & TX3_QDMA_OWN))
			break;
		
		/* hold next TXD ptr */
		cpu_ptr = ACCESS_ONCE(txd->txd_info2);
		
		/* release current TXD */
		put_free_txd(ei_local, cpu_idx);
		
		/* get next TXD index */
		cpu_idx = get_txd_offset(ei_local, cpu_ptr);
		
		/* free skb */
		skb = ei_local->txd_buff[cpu_idx];
		if (skb) {
#if defined (CONFIG_RAETH_BQL)
#if defined (CONFIG_PSEUDO_SUPPORT)
			if (skb->dev == ei_local->PseudoDev)
				bytes_sent_ge2 += skb->len;
			else
#endif
				bytes_sent_ge1 += skb->len;
#endif
			ei_local->txd_buff[cpu_idx] = NULL;
			dev_kfree_skb(skb);
		}
		
		clean_done++;
		
		/* prevent infinity loop when something wrong */
		if (clean_done > (NUM_TX_DESC-4))
			break;
	}

	if (clean_done)
		sysRegWrite(QTX_CRX_PTR, cpu_ptr);

	spin_unlock(&ei_local->page_lock);

	if (!clean_done)
		return;

	cpu = smp_processor_id();

	if (netif_running(dev)) {
		txq = netdev_get_tx_queue(dev, 0);
		__netif_tx_lock(txq, cpu);
#if defined (CONFIG_RAETH_BQL)
		netdev_tx_completed_queue(txq, 0, bytes_sent_ge1);
#endif
		if (netif_tx_queue_stopped(txq))
			netif_tx_wake_queue(txq);
		__netif_tx_unlock(txq);
	}

#if defined (CONFIG_PSEUDO_SUPPORT)
	if (netif_running(ei_local->PseudoDev)) {
		txq = netdev_get_tx_queue(ei_local->PseudoDev, 0);
		__netif_tx_lock(txq, cpu);
#if defined (CONFIG_RAETH_BQL)
		netdev_tx_completed_queue(txq, 0, bytes_sent_ge2);
#endif
		if (netif_tx_queue_stopped(txq))
			netif_tx_wake_queue(txq);
		__netif_tx_unlock(txq);
	}
#endif
}

