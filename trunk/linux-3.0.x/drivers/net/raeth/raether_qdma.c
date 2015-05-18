/*
 * QDMA implementation (MT7621 only)
 * Note: this file is part of raether.c
 */

u8 M2Q_table[64] = {0};
EXPORT_SYMBOL(M2Q_table);

static void
fe_dma_ring_free(END_DEVICE *ei_local)
{
	u32 i;

	/* free PDMA RX buffers */
	for (i = 0; i < NUM_RX_DESC; i++) {
		if (ei_local->rxd_buff[i]) {
			dev_kfree_skb(ei_local->rxd_buff[i]);
			ei_local->rxd_buff[i] = NULL;
		}
	}

	/* free PDMA RX descriptors */
	if (ei_local->rxd_ring) {
		dma_free_coherent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), ei_local->rxd_ring, ei_local->rxd_ring_phy);
		ei_local->rxd_ring = NULL;
	}

	/* free QDMA TX descriptors */
	if (ei_local->txd_pool) {
		dma_free_coherent(NULL, NUM_TX_DESC * sizeof(struct QDMA_txdesc), ei_local->txd_pool, ei_local->txd_pool_phy);
		ei_local->txd_pool = NULL;
	}

#if defined (CONFIG_RA_HW_NAT_QDMA)
	/* free QDMA FQ descriptors */
	if (ei_local->free_head) {
		dma_free_coherent(NULL, NUM_QDMA_PAGE * sizeof(struct QDMA_txdesc), ei_local->free_head, ei_local->free_head_phy);
		ei_local->free_head = NULL;
	}

	/* free QDMA FQ pool */
	if (ei_local->free_head_page) {
		dma_free_coherent(NULL, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, ei_local->free_head_page, ei_local->free_head_page_phy);
		ei_local->free_head_page = NULL;
	}
#endif
}

static int
fe_dma_ring_alloc(END_DEVICE *ei_local)
{
	u32 i;

#if defined (CONFIG_RA_HW_NAT_QDMA)
	/* allocate QDMA FQ pool */
	ei_local->free_head_page = dma_alloc_coherent(NULL, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, &ei_local->free_head_page_phy, GFP_KERNEL);
	if (!ei_local->free_head_page)
		goto err_cleanup;

	/* allocate QDMA FQ descriptors */
	ei_local->free_head = dma_alloc_coherent(NULL, NUM_QDMA_PAGE * sizeof(struct QDMA_txdesc), &ei_local->free_head_phy, GFP_KERNEL);
	if (!ei_local->free_head)
		goto err_cleanup;
#endif

	/* allocate QDMA TX descriptors */
	ei_local->txd_pool = dma_alloc_coherent(NULL, NUM_TX_DESC * sizeof(struct QDMA_txdesc), &ei_local->txd_pool_phy, GFP_KERNEL);
	if (!ei_local->txd_pool)
		goto err_cleanup;

	/* allocate PDMA RX descriptors */
	ei_local->rxd_ring = dma_alloc_coherent(NULL, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->rxd_ring_phy, GFP_KERNEL);
	if (!ei_local->rxd_ring)
		goto err_cleanup;

	/* allocate PDMA RX buffers */
	for (i = 0; i < NUM_RX_DESC; i++) {
		ei_local->rxd_buff[i] = dev_alloc_skb(MAX_RX_LENGTH + NET_IP_ALIGN);
		if (!ei_local->rxd_buff[i])
			goto err_cleanup;
	}

	return 0;

err_cleanup:
	fe_dma_ring_free(ei_local);
	return -ENOMEM;
}

static inline u32
GET_TXD_OFFSET(END_DEVICE *ei_local, const struct QDMA_txdesc *cpu_ptr)
{
	return (u32)(cpu_ptr - ei_local->txd_pool);
}

/* must be spinlock protected */
static inline u32
get_free_txd(END_DEVICE *ei_local, struct QDMA_txdesc **free_txd)
{
	u32 txd_idx;

	if (!ei_local->txd_pool_free_num)
		return NUM_TX_DESC;

	ei_local->txd_pool_free_num -= 1;
	txd_idx = ei_local->txd_pool_free_head;
	ei_local->txd_pool_free_head = ei_local->txd_pool_info[txd_idx];
	*free_txd = &ei_local->txd_pool[txd_idx];

	return txd_idx;
}

/* must be spinlock protected */
static inline void
put_free_txd(END_DEVICE *ei_local, u32 free_txd_idx)
{
	ei_local->txd_pool_info[ei_local->txd_pool_free_tail] = free_txd_idx;
	ei_local->txd_pool_free_tail = free_txd_idx;
	ei_local->txd_pool_info[free_txd_idx] = NUM_TX_DESC;
	ei_local->txd_pool_free_num += 1;
}

static void
fe_dma_init(END_DEVICE *ei_local)
{
	u32 i, regVal;
	struct QDMA_txdesc *free_txd = NULL;
#if defined (CONFIG_RA_HW_NAT_QDMA)
	dma_addr_t free_tail_phy;

	/* init QDMA FQ pool */
	for (i = 0; i < NUM_QDMA_PAGE; i++) {
		ei_local->free_head[i].txd_info1 = (u32)(ei_local->free_head_page_phy + (i * QDMA_PAGE_SIZE));
		if (i < (NUM_QDMA_PAGE-1))
			ei_local->free_head[i].txd_info2 = (u32)(ei_local->free_head_phy + ((i+1) * sizeof(struct QDMA_txdesc)));
		else
			ei_local->free_head[i].txd_info2 = (u32)(ei_local->free_head_phy);
		ei_local->free_head[i].txd_info4 = 0;
		ei_local->free_head[i].txd_info3 = TX3_QDMA_SDL(QDMA_PAGE_SIZE);
	}

	free_tail_phy = ei_local->free_head_phy + (dma_addr_t)((NUM_QDMA_PAGE-1) * sizeof(struct QDMA_txdesc));
#endif

	/* init QDMA TX pool */
	for (i = 0; i < NUM_TX_DESC; i++) {
		ei_local->txd_buff[i] = NULL;
		ei_local->txd_pool_info[i] = i + 1;
		ei_local->txd_pool[i].txd_info1 = 0;
		if (i < (NUM_TX_DESC-1))
			ei_local->txd_pool[i].txd_info2 = VIRT_TO_PHYS(&ei_local->txd_pool[i+1]);
		else
			ei_local->txd_pool[i].txd_info2 = VIRT_TO_PHYS(&ei_local->txd_pool[0]);
		ei_local->txd_pool[i].txd_info3 = TX3_QDMA_LS | TX3_QDMA_OWN;
		ei_local->txd_pool[i].txd_info4 = 0;
	}

	ei_local->txd_pool_free_head = 0;
	ei_local->txd_pool_free_tail = NUM_TX_DESC - 1;
	ei_local->txd_pool_free_num = NUM_TX_DESC;

	/* init PDMA RX ring */
	for (i = 0; i < NUM_RX_DESC; i++) {
		ei_local->rxd_ring[i].rxd_info1 = (u32)dma_map_single(NULL, ei_local->rxd_buff[i]->data, MAX_RX_LENGTH, DMA_FROM_DEVICE);
		ei_local->rxd_ring[i].rxd_info4 = 0;
		ei_local->rxd_ring[i].rxd_info3 = 0;
		ei_local->rxd_ring[i].rxd_info2 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
	}

	wmb();

	/* clear QDMA */
	regVal = sysRegRead(QDMA_GLO_CFG);
	regVal &= ~(0x000000FF);
	sysRegWrite(QDMA_GLO_CFG, regVal);

	/* clear PDMA */
	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= ~(0x000000FF);
	sysRegWrite(PDMA_GLO_CFG, regVal);

#if defined (CONFIG_RA_HW_NAT_QDMA)
	/* PPE -> QDMA FQ pool */
	sysRegWrite(QDMA_FQ_HEAD, phys_to_bus((u32)ei_local->free_head_phy));
	sysRegWrite(QDMA_FQ_TAIL, phys_to_bus((u32)free_tail_phy));
	sysRegWrite(QDMA_FQ_CNT,  cpu_to_le32((u32)((NUM_TX_DESC << 16) | NUM_QDMA_PAGE)));
	sysRegWrite(QDMA_FQ_BLEN, cpu_to_le32((u32)(QDMA_PAGE_SIZE << 16)));
#else
	sysRegWrite(QDMA_FQ_CNT,  cpu_to_le32((u32)(NUM_TX_DESC << 16)));
#endif

	/* GDMA1/2 -> QRX ring #0 (not used direct forward to P5, only via FQ) */
	sysRegWrite(QRX_BASE_PTR0, 0);
	sysRegWrite(QRX_MAX_CNT0, 0);
	sysRegWrite(QRX_CRX_IDX0, cpu_to_le32((u32)(NUM_QRX_DESC - 1)));
	sysRegWrite(QDMA_RST_CFG, PST_DRX_IDX0);

	/* GDMA1/2 -> RX ring #0 */
	sysRegWrite(RX_BASE_PTR0, phys_to_bus((u32)ei_local->rxd_ring_phy));
	sysRegWrite(RX_MAX_CNT0,  cpu_to_le32((u32)NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX0, cpu_to_le32((u32)(NUM_RX_DESC - 1)));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX0);

	/* get free txd from pool for RLS (release) */
	get_free_txd(ei_local, &free_txd);
	sysRegWrite(QTX_CRX_PTR, VIRT_TO_PHYS(free_txd));
	sysRegWrite(QTX_DRX_PTR, VIRT_TO_PHYS(free_txd));

	/* get free txd from pool for TX */
	get_free_txd(ei_local, &free_txd);
	sysRegWrite(QTX_CTX_PTR, VIRT_TO_PHYS(free_txd));
	sysRegWrite(QTX_DTX_PTR, VIRT_TO_PHYS(free_txd));
	ei_local->txd_cpu_ptr = free_txd;

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

	/* free uncompleted QDMA TX buffers */
	for (i = 0; i < NUM_TX_DESC; i++) {
		if (ei_local->txd_buff[i]) {
#if defined (CONFIG_RAETH_SG_DMA_TX)
			if (ei_local->txd_buff[i] != (struct sk_buff *)0xFFFFFFFF)
#endif
				dev_kfree_skb(ei_local->txd_buff[i]);
			ei_local->txd_buff[i] = NULL;
		}
	}

	/* uninit PDMA RX ring */
	for (i = 0; i < NUM_RX_DESC; i++) {
		if (ei_local->rxd_ring[i].rxd_info1) {
			ei_local->rxd_ring[i].rxd_info1 = 0;
			ei_local->rxd_ring[i].rxd_info2 = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
		}
	}

	wmb();

	/* clear adapter QDMA FQ pool */
	sysRegWrite(QDMA_FQ_HEAD, 0);
	sysRegWrite(QDMA_FQ_TAIL, 0);
	sysRegWrite(QDMA_FQ_CNT, 0);

	/* clear adapter QDMA TX pool */
	sysRegWrite(QTX_CTX_PTR, 0);
	sysRegWrite(QTX_CRX_PTR, 0);

	/* clear adapter QDMA RX ring */
	sysRegWrite(QRX_BASE_PTR0, 0);
	sysRegWrite(QRX_MAX_CNT0, 0);

	/* clear adapter PDMA RX ring */
	sysRegWrite(RX_BASE_PTR0, 0);
	sysRegWrite(RX_MAX_CNT0,  0);
}

static inline int
dma_xmit(struct sk_buff *skb, struct net_device *dev, END_DEVICE *ei_local, int gmac_no)
{
	struct QDMA_txdesc *cpu_ptr;
	struct QDMA_txdesc *free_txd = NULL;
	struct netdev_queue *txq;
	u32 ctx_offset, skb_len;
	u32 txd_info3, txd_info4;
#if defined (CONFIG_RAETH_SG_DMA_TX)
	u32 i, nr_slots, nr_frags;
	const skb_frag_t *tx_frag;
	const struct skb_shared_info *shinfo;
#else
#define nr_slots 1
#define nr_frags 0
#endif

	if (!ei_local->active) {
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

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
	if (gmac_no != PSE_PORT_PPE)
		txd_info3 |= TX3_QDMA_QID(M2Q_table[(skb->mark & 0x3f)]);

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
	nr_frags = shinfo->nr_frags;
	nr_slots = nr_frags + 1;
#endif

	txq = netdev_get_tx_queue(dev, 0);

	/* protect TX ring access (from eth2/eth3 queues) */
	spin_lock(&ei_local->page_lock);

	if (ei_local->txd_pool_free_num < (nr_slots+3)) {
		spin_unlock(&ei_local->page_lock);
		netif_tx_stop_queue(txq);
#if defined (CONFIG_RAETH_DEBUG)
		if (net_ratelimit())
			printk("%s: QDMA TX pool is run out! (GMAC: %d)\n", RAETH_DEV_NAME, gmac_no);
#endif
		return NETDEV_TX_BUSY;
	}

#if defined (CONFIG_RAETH_TSO)
	/* fill MSS info in tcp checksum field */
	if (shinfo->gso_size) {
		if (skb_header_cloned(skb)) {
			if (pskb_expand_head(skb, 0, 0, GFP_ATOMIC)) {
				spin_unlock(&ei_local->page_lock);
				dev_kfree_skb(skb);
#if defined (CONFIG_RAETH_DEBUG)
				if (net_ratelimit())
					printk(KERN_ERR "%s: pskb_expand_head for TSO failed!\n", RAETH_DEV_NAME);
#endif
				return NETDEV_TX_OK;
			}
		}
		if (shinfo->gso_type & (SKB_GSO_TCPV4|SKB_GSO_TCPV6)) {
			u32 hdr_len = (skb_transport_offset(skb) + tcp_hdrlen(skb));
			if (skb->len > hdr_len) {
				tcp_hdr(skb)->check = htons(shinfo->gso_size);
				txd_info4 |= TX4_DMA_TSO;
			}
		}
	}
#endif

	cpu_ptr = ei_local->txd_cpu_ptr;
	ctx_offset = GET_TXD_OFFSET(ei_local, cpu_ptr);
	ei_local->txd_buff[ctx_offset] = skb;

	ctx_offset = get_free_txd(ei_local, &free_txd);
#if defined (CONFIG_RAETH_DEBUG)
	BUG_ON(ctx_offset == NUM_TX_DESC);
#endif

	ei_local->txd_cpu_ptr = free_txd;

#if defined (CONFIG_RAETH_SG_DMA_TX)
	if (nr_frags)
		skb_len = skb_headlen(skb);
	else
#endif
	{
		skb_len = skb->len;
		txd_info3 |= TX3_QDMA_LS;
	}

	/* write QDMA TX desc (QDMA_OWN must be cleared last) */
	cpu_ptr->txd_info1 = (u32)dma_map_single(NULL, skb->data, skb_len, DMA_TO_DEVICE);
	cpu_ptr->txd_info2 = VIRT_TO_PHYS(free_txd);
	cpu_ptr->txd_info4 = txd_info4;
	cpu_ptr->txd_info3 = txd_info3 | TX3_QDMA_SDL(skb_len);

#if defined (CONFIG_RAETH_SG_DMA_TX)
	for (i = 0; i < nr_frags; i++) {
		tx_frag = &shinfo->frags[i];
		cpu_ptr = free_txd;
		
		ei_local->txd_buff[ctx_offset] = (struct sk_buff *)0xFFFFFFFF; //MAGIC ID
		
		ctx_offset = get_free_txd(ei_local, &free_txd);
#if defined (CONFIG_RAETH_DEBUG)
		BUG_ON(ctx_offset == NUM_TX_DESC);
#endif
		
		ei_local->txd_cpu_ptr = free_txd;
		
		if ((i + 1) == nr_frags) // last segment
			txd_info3 |= TX3_QDMA_LS;
		
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
		cpu_ptr->txd_info1 = (u32)skb_frag_dma_map(NULL, tx_frag, 0, skb_frag_size(tx_frag), DMA_TO_DEVICE);
#else
		cpu_ptr->txd_info1 = (u32)dma_map_page(NULL, tx_frag->page, tx_frag->page_offset, tx_frag->size, DMA_TO_DEVICE);
#endif
		cpu_ptr->txd_info2 = VIRT_TO_PHYS(free_txd);
		cpu_ptr->txd_info4 = txd_info4;
		cpu_ptr->txd_info3 = txd_info3 | TX3_QDMA_SDL(tx_frag->size);
	}
#endif

#if defined (CONFIG_RAETH_BQL)
	netdev_tx_sent_queue(txq, skb->len);
#endif

	wmb();

	/* kick the DMA TX */
	sysRegWrite(QTX_CTX_PTR, VIRT_TO_PHYS(ei_local->txd_cpu_ptr));

	spin_unlock(&ei_local->page_lock);

	return NETDEV_TX_OK;
}

static inline void
dma_xmit_clean(struct net_device *dev, END_DEVICE *ei_local)
{
	struct netdev_queue *txq;
	struct sk_buff *txd_buff;
	int cpu, clean_done = 0;
	struct QDMA_txdesc *cpu_ptr, *dma_ptr, *htx_ptr;
	u32 htx_offset = 0;
#if defined (CONFIG_RAETH_BQL)
	u32 bytes_sent_ge1 = 0;
#if defined (CONFIG_PSEUDO_SUPPORT)
	u32 bytes_sent_ge2 = 0;
#endif
#endif

	spin_lock(&ei_local->page_lock);

	cpu_ptr = PHYS_TO_VIRT(sysRegRead(QTX_CRX_PTR));
	dma_ptr = PHYS_TO_VIRT(sysRegRead(QTX_DRX_PTR));

	while (cpu_ptr != dma_ptr && (cpu_ptr->txd_info3 & TX3_QDMA_OWN)) {
		/* keep cpu next TXD */
		htx_ptr = PHYS_TO_VIRT(cpu_ptr->txd_info2);
		htx_offset = GET_TXD_OFFSET(ei_local, htx_ptr);
		txd_buff = ei_local->txd_buff[htx_offset];
		
		/* free skb */
		if (txd_buff
#if defined (CONFIG_RAETH_SG_DMA_TX)
		 && txd_buff != (struct sk_buff *)0xFFFFFFFF
#endif
		    ) {
#if defined (CONFIG_RAETH_BQL)
#if defined (CONFIG_PSEUDO_SUPPORT)
			if (txd_buff->dev == ei_local->PseudoDev)
				bytes_sent_ge2 += txd_buff->len;
			else
#endif
				bytes_sent_ge1 += txd_buff->len;
#endif
			dev_kfree_skb(txd_buff);
		}
		
		ei_local->txd_buff[htx_offset] = NULL;
		
		/* release TXD */
		htx_offset = GET_TXD_OFFSET(ei_local, cpu_ptr);
		put_free_txd(ei_local, htx_offset);
		
		/* update cpu_ptr to next ptr */
		cpu_ptr = htx_ptr;
		
		if (++clean_done > (NUM_TX_DESC-2))
			break;
	}

	if (clean_done)
		sysRegWrite(QTX_CRX_PTR, VIRT_TO_PHYS(cpu_ptr));

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
