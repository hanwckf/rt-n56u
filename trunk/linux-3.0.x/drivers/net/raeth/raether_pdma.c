/*
 * PDMA implementation
 * Note: this file is part of raether.c
 */

static void
fe_dma_ring_free(END_DEVICE *ei_local)
{
	int i;

	/* free RX buffers */
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

	/* free PDMA TX descriptors */
	if (ei_local->txd_ring) {
		dma_free_coherent(NULL, NUM_TX_DESC * sizeof(struct PDMA_txdesc), ei_local->txd_ring, ei_local->txd_ring_phy);
		ei_local->txd_ring = NULL;
	}
}

static int
fe_dma_ring_alloc(END_DEVICE *ei_local)
{
	int i;

	/* allocate PDMA TX descriptors */
	ei_local->txd_ring = dma_alloc_coherent(NULL, NUM_TX_DESC * sizeof(struct PDMA_txdesc), &ei_local->txd_ring_phy, GFP_KERNEL);
	if (!ei_local->txd_ring)
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
#if !defined (RAETH_PDMA_V2)
		skb_reserve(ei_local->rxd_buff[i], NET_IP_ALIGN);
#endif
	}

	return 0;

err_cleanup:
	fe_dma_ring_free(ei_local);
	return -ENOMEM;
}

/* must be spinlock protected */
static void
fe_dma_init(END_DEVICE *ei_local)
{
	int i;
	u32 regVal;

	/* init PDMA TX ring */
	for (i = 0; i < NUM_TX_DESC; i++) {
		struct PDMA_txdesc *txd = &ei_local->txd_ring[i];
		
		ei_local->txd_buff[i] = NULL;
		
		ACCESS_ONCE(txd->txd_info1) = 0;
		ACCESS_ONCE(txd->txd_info2) = TX2_DMA_DONE;
#if defined (RAETH_PDMA_V2)
		ACCESS_ONCE(txd->txd_info4) = 0;
#else
		ACCESS_ONCE(txd->txd_info4) = TX4_DMA_QN(3);
#endif
		ACCESS_ONCE(txd->txd_info3) = 0;
	}

	/* init PDMA RX ring */
	for (i = 0; i < NUM_RX_DESC; i++) {
		struct PDMA_rxdesc *rxd = &ei_local->rxd_ring[i];
#if defined (RAETH_PDMA_V2)
		ACCESS_ONCE(rxd->rxd_info1) = (u32)dma_map_single(NULL, ei_local->rxd_buff[i]->data, MAX_RX_LENGTH + NET_IP_ALIGN, DMA_FROM_DEVICE);
		ACCESS_ONCE(rxd->rxd_info2) = RX2_DMA_SDL0_SET(MAX_RX_LENGTH);
#else
		ACCESS_ONCE(rxd->rxd_info1) = (u32)dma_map_single(NULL, ei_local->rxd_buff[i]->data, MAX_RX_LENGTH, DMA_FROM_DEVICE);
		ACCESS_ONCE(rxd->rxd_info2) = RX2_DMA_LS0;
#endif
		ACCESS_ONCE(rxd->rxd_info3) = 0;
		ACCESS_ONCE(rxd->rxd_info4) = 0;
	}

	wmb();

	/* clear PDMA */
	regVal = sysRegRead(PDMA_GLO_CFG);
	regVal &= ~(CSR_CLKGATE | RX_DMA_EN | TX_DMA_EN);
	sysRegWrite(PDMA_GLO_CFG, regVal);

	/* GDMA1/2 <- TX Ring #0 */
	sysRegWrite(TX_BASE_PTR0, phys_to_bus((u32)ei_local->txd_ring_phy));
	sysRegWrite(TX_MAX_CNT0, cpu_to_le32(NUM_TX_DESC));
	sysRegWrite(TX_CTX_IDX0, 0);
	sysRegWrite(PDMA_RST_CFG, PST_DTX_IDX0);
	ei_local->txd_last_idx = le32_to_cpu(sysRegRead(TX_CTX_IDX0));
	ei_local->txd_free_idx = ei_local->txd_last_idx;

	/* GDMA1/2 -> RX Ring #0 */
	sysRegWrite(RX_BASE_PTR0, phys_to_bus((u32)ei_local->rxd_ring_phy));
	sysRegWrite(RX_MAX_CNT0, cpu_to_le32(NUM_RX_DESC));
	sysRegWrite(RX_CALC_IDX0, cpu_to_le32(NUM_RX_DESC - 1));
	sysRegWrite(PDMA_RST_CFG, PST_DRX_IDX0);

	/* only the following chipset need to set it */
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
	// set 1us timer count in unit of clock cycle
	regVal = sysRegRead(FE_GLO_CFG);
	regVal &= ~(0xff << 8); //clear bit8-bit15
	regVal |= (((get_surfboard_sysclk()/1000000)) << 8);
	sysRegWrite(FE_GLO_CFG, regVal);
#endif

	/* config DLY interrupt */
	sysRegWrite(DLY_INT_CFG, FE_DLY_INIT_VALUE);
}

static void
fe_dma_uninit(END_DEVICE *ei_local)
{
	int i;
	struct sk_buff *skb;

	spin_lock_bh(&ei_local->page_lock);

	/* free uncompleted PDMA TX buffers */
	for (i = 0; i < NUM_TX_DESC; i++) {
		skb = ei_local->txd_buff[i];
		if (skb) {
			if (skb != (struct sk_buff *)0xFFFFFFFF)
				dev_kfree_skb(skb);
			ei_local->txd_buff[i] = NULL;
		}
	}

	spin_unlock_bh(&ei_local->page_lock);
}

static void
fe_dma_clear_addr(void)
{
	/* clear adapter PDMA TX ring */
	sysRegWrite(TX_BASE_PTR0, 0);
	sysRegWrite(TX_MAX_CNT0, 0);

	/* clear adapter PDMA RX ring */
	sysRegWrite(RX_BASE_PTR0, 0);
	sysRegWrite(RX_MAX_CNT0,  0);
}

static inline void
pdma_write_skb_fragment(END_DEVICE *ei_local,
			dma_addr_t frag_addr,
			u32 frag_size,
			u32 *desc_odd,
			u32 *txd_info2,
			const u32 txd_info4,
			struct sk_buff *skb,
			const bool ls)
{
	u32 desc_idx = ei_local->txd_last_idx;

	while (frag_size > 0) {
		const u32 part_addr = (u32)frag_addr;
		const u32 part_size = min_t(u32, frag_size, TXD_MAX_SEG_SIZE);
		struct PDMA_txdesc *txd;
		
		frag_size -= part_size;
		frag_addr += part_size;
		
		txd = &ei_local->txd_ring[desc_idx];
		
		if (*desc_odd == 0) {
			*txd_info2 = TX2_DMA_SDL0(part_size);
			
			ACCESS_ONCE(txd->txd_info1) = part_addr;
			ACCESS_ONCE(txd->txd_info4) = txd_info4;
			
			if (ls && frag_size == 0) {
				*txd_info2 |= TX2_DMA_LS0;
				
				ACCESS_ONCE(txd->txd_info3) = 0;
				ACCESS_ONCE(txd->txd_info2) = *txd_info2;
				
				/* store skb with LS0 bit */
				ei_local->txd_buff[desc_idx] = skb;
				
				desc_idx = (desc_idx + 1) % NUM_TX_DESC;
			} else
				*desc_odd = 1;
		} else {
			*txd_info2 |= TX2_DMA_SDL1(part_size);
			
			if (ls && frag_size == 0) {
				*txd_info2 |= TX2_DMA_LS1;
				
				/* store skb with LS1 bit */
				ei_local->txd_buff[desc_idx] = skb;
			} else
				ei_local->txd_buff[desc_idx] = (struct sk_buff *)0xFFFFFFFF; //MAGIC ID
			
			ACCESS_ONCE(txd->txd_info3) = part_addr;
			ACCESS_ONCE(txd->txd_info2) = *txd_info2;
			
			desc_idx = (desc_idx + 1) % NUM_TX_DESC;
			*desc_odd = 0;
		}
	}

	ei_local->txd_last_idx = desc_idx;
}

static inline int
dma_xmit(struct sk_buff* skb, struct net_device *dev, END_DEVICE *ei_local, int gmac_no)
{
	struct netdev_queue *txq;
	dma_addr_t frag_addr;
	u32 frag_size, nr_desc;
	u32 next_idx, desc_odd = 0;
	u32 txd_info2 = 0, txd_info4;
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

#if !defined (RAETH_HW_PADPKT)
	if (skb->len < ei_local->min_pkt_len) {
		if (skb_padto(skb, ei_local->min_pkt_len)) {
#if defined (CONFIG_RAETH_DEBUG)
			if (net_ratelimit())
				printk(KERN_ERR "%s: skb_padto failed\n", RAETH_DEV_NAME);
#endif
			return NETDEV_TX_OK;
		}
		skb_put(skb, ei_local->min_pkt_len - skb->len);
	}
#endif

#if defined (CONFIG_RALINK_MT7620)
	if (gmac_no == PSE_PORT_PPE)
		txd_info4 = TX4_DMA_FP_BMAP(0x80); /* P7 */
	else
#if defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_ESW)
		txd_info4 = TX4_DMA_FP_BMAP(0x20); /* P5 */
#elif defined (CONFIG_RAETH_HAS_PORT4) && !defined (CONFIG_RAETH_HAS_PORT5) && !defined (CONFIG_RAETH_ESW)
		txd_info4 = TX4_DMA_FP_BMAP(0x10); /* P4 */
#else
		txd_info4 = 0; /* routing by DA */
#endif
#elif defined (CONFIG_RALINK_MT7621)
	txd_info4 = TX4_DMA_FPORT(gmac_no);
#else
	txd_info4 = (TX4_DMA_QN(3) | TX4_DMA_PN(gmac_no));
#endif

#if defined (CONFIG_RAETH_CHECKSUM_OFFLOAD) && !defined (RAETH_SDMA)
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		txd_info4 |= TX4_DMA_TUI_CO(7);
#endif

#if defined (CONFIG_RAETH_HW_VLAN_TX)
	if (skb_vlan_tag_present(skb)) {
#if defined (RAETH_HW_VLAN4K)
		txd_info4 |= (0x10000 | skb_vlan_tag_get(skb));
#else
		u32 vlan_tci = skb_vlan_tag_get(skb);
		txd_info4 |= (TX4_DMA_INSV | TX4_DMA_VPRI(vlan_tci));
		txd_info4 |= (u32)ei_local->vlan_4k_map[(vlan_tci & VLAN_VID_MASK)];
#endif
	}
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
	nr_desc = DIV_ROUND_UP(nr_desc, 2);

	txq = netdev_get_tx_queue(dev, 0);

	/* flush main skb part before spin_lock() */
	frag_size = (u32)skb_headlen(skb);
	frag_addr = dma_map_single(NULL, skb->data, frag_size, DMA_TO_DEVICE);

	/* protect TX ring access (from eth2/eth3 queues) */
	spin_lock(&ei_local->page_lock);

	/* check nr_desc+1 free descriptors */
	next_idx = (ei_local->txd_last_idx + nr_desc) % NUM_TX_DESC;
	if (ei_local->txd_buff[ei_local->txd_last_idx] || ei_local->txd_buff[next_idx]) {
		spin_unlock(&ei_local->page_lock);
		netif_tx_stop_queue(txq);
#if defined (CONFIG_RAETH_DEBUG)
		if (net_ratelimit())
			printk("%s: PDMA TX ring is full! (GMAC: %d)\n", RAETH_DEV_NAME, gmac_no);
#endif
		return NETDEV_TX_BUSY;
	}

	pdma_write_skb_fragment(ei_local, frag_addr, frag_size, &desc_odd,
				&txd_info2, txd_info4, skb, nr_frags == 0);
#if defined (CONFIG_RAETH_SG_DMA_TX)
	for (i = 0; i < nr_frags; i++) {
		tx_frag = &shinfo->frags[i];
		frag_size = skb_frag_size(tx_frag);
		frag_addr = skb_frag_dma_map(NULL, tx_frag, 0, frag_size, DMA_TO_DEVICE);
		pdma_write_skb_fragment(ei_local, frag_addr, frag_size, &desc_odd,
					&txd_info2, txd_info4, skb, i == nr_frags - 1);
	}
#endif

#if defined (CONFIG_RAETH_BQL)
	netdev_tx_sent_queue(txq, skb->len);
#endif

#if !defined (CONFIG_RAETH_BQL) || !defined (CONFIG_SMP)
	/* smp_mb() already inlined in netdev_tx_sent_queue */
	wmb();
#endif

	/* kick the DMA TX */
	sysRegWrite(TX_CTX_IDX0, cpu_to_le32(ei_local->txd_last_idx));

	spin_unlock(&ei_local->page_lock);

	return NETDEV_TX_OK;
}

static inline void
dma_xmit_clean(struct net_device *dev, END_DEVICE *ei_local)
{
	struct netdev_queue *txq;
	int cpu, clean_done = 0;
	u32 txd_free_idx;
#if defined (CONFIG_RAETH_BQL)
	u32 bytes_sent_ge1 = 0;
#if defined (CONFIG_PSEUDO_SUPPORT)
	u32 bytes_sent_ge2 = 0;
#endif
#endif

	spin_lock(&ei_local->page_lock);

	txd_free_idx = ei_local->txd_free_idx;

	while (clean_done < (NUM_TX_DESC-2)) {
		struct PDMA_txdesc *txd;
		struct sk_buff *skb;
		
		skb = ei_local->txd_buff[txd_free_idx];
		if (!skb)
			break;
		
		txd = &ei_local->txd_ring[txd_free_idx];
		
		/* check TXD not owned by DMA */
		if (!(ACCESS_ONCE(txd->txd_info2) & TX2_DMA_DONE))
			break;
		
		if (skb != (struct sk_buff *)0xFFFFFFFF) {
#if defined (CONFIG_RAETH_BQL)
#if defined (CONFIG_PSEUDO_SUPPORT)
			if (skb->dev == ei_local->PseudoDev)
				bytes_sent_ge2 += skb->len;
			else
#endif
				bytes_sent_ge1 += skb->len;
#endif
			dev_kfree_skb(skb);
		}
		
		ei_local->txd_buff[txd_free_idx] = NULL;
		
		txd_free_idx = (txd_free_idx + 1) % NUM_TX_DESC;
		
		clean_done++;
	}

	if (ei_local->txd_free_idx != txd_free_idx)
		ei_local->txd_free_idx = txd_free_idx;

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

