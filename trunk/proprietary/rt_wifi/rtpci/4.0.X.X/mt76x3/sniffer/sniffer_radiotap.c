/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sniffer_radiotap.c
*/
#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rtmp_osabl.h"
#include "rt_os_util.h"

#define ETH_P_ECONET 0x0018
#define ETH_P_80211_RAW (ETH_P_ECONET + 1)


void send_radiotap_monitor_packets(
						PNET_DEV pNetDev,
			  			PNDIS_PACKET pRxPacket,
			  			VOID *dot11_hdr,
			  			UCHAR *pData,
			  			USHORT DataSize,
			  			UCHAR L2PAD,
			  			UCHAR PHYMODE,
			  			UCHAR BW,
			  			UCHAR ShortGI,
			  			UCHAR MCS,
			  			UCHAR LDPC,
						UCHAR LDPC_EX_SYM,
			  			UCHAR AMPDU,
			  			UCHAR STBC,
			  			UCHAR RSSI1,
			  			UCHAR *pDevName,
			  			UCHAR Channel,
			  			UCHAR CentralChannel,
						UCHAR sideband_index,
			  			UINT32 MaxRssi) {
	struct sk_buff *pOSPkt;
	int rate_index = 0;
	USHORT header_len = 0;
	UCHAR temp_header[40] = {0};
	struct mtk_radiotap_header *mtk_rt_hdr;
	UINT32 varlen = 0, padding_len = 0;
	UINT64 tmp64;
	UINT32 tmp32;
	UINT16 tmp16;
	UCHAR *pos;
	DOT_11_HDR *pHeader = (DOT_11_HDR *)dot11_hdr;

	MEM_DBG_PKT_FREE_INC(pRxPacket);

	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;
	if (pHeader->FC.Type == 0x2 /* FC_TYPE_DATA */) {
		DataSize -= LENGTH_802_11;
		if ((pHeader->FC.ToDs == 1) && (pHeader->FC.FrDs == 1))
			header_len = LENGTH_802_11_WITH_ADDR4;
		else
			header_len = LENGTH_802_11;

		/* QOS */
		if (pHeader->FC.SubType & 0x08) {
			header_len += 2;
			/* Data skip QOS contorl field */
			DataSize -= 2;
		}

		/* Order bit: A-Ralink or HTC+ */
		if (pHeader->FC.Order) {
			header_len += 4;
			/* Data skip HTC contorl field */
			DataSize -= 4;
		}

		/* Copy Header */
		if (header_len <= 40)
			NdisMoveMemory(temp_header, pData, header_len);

		/* skip HW padding */
		if (L2PAD)
			pData += (header_len + 2);
		else
			pData += header_len;
	}

	if (DataSize < pOSPkt->len) {
		skb_trim(pOSPkt, DataSize);
	} else {
		skb_put(pOSPkt, (DataSize - pOSPkt->len));
	}

	if ((pData - pOSPkt->data) > 0) {
		skb_put(pOSPkt, (pData - pOSPkt->data));
		skb_pull(pOSPkt, (pData - pOSPkt->data));
	}

	if (skb_headroom(pOSPkt) < (sizeof(*mtk_rt_hdr) + header_len)) {
		if (pskb_expand_head(pOSPkt, (sizeof(*mtk_rt_hdr) + header_len), 0, GFP_ATOMIC)) {
			DBGPRINT(RT_DEBUG_ERROR,
				 ("%s : Reallocate header size of sk_buff fail!\n",
				  __FUNCTION__));
			goto err_free_sk_buff;
		}
	}

	if (header_len > 0)
		NdisMoveMemory(skb_push(pOSPkt, header_len), temp_header, header_len);

	/* tsf */
	padding_len = ((varlen % 8) == 0) ? 0 : (8 - (varlen % 8));
	varlen += (8 + padding_len);

	/* flags */
	varlen += 1;

	/* rate */
	if (PHYMODE < MODE_HTMIX)
		varlen += 1;

	/* channel frequency */
	padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
	varlen += (2 + padding_len);

	/* channel flags */
	varlen += 2;

	/* MCS */
	if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)) {
		/* known */
		varlen += 1;

		/* flags */
		varlen += 1;

		/* index */
		varlen += 1;
	}

	/* A-MPDU */
	if (AMPDU) {
		/* reference number */
		padding_len = ((varlen % 4) == 0) ? 0 : (4 - (varlen % 4));
		varlen += (4 + padding_len);
	
		/* flags */
		varlen += 2;

		/* delimiter crc value */
		varlen += 1;

		/* reserved */
		varlen += 1;
	}

	/* VHT */
	if (PHYMODE == MODE_VHT) {
		/* known */
		padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
		varlen += (2 + padding_len);

		/* flags */
		varlen += 1;

		/* bandwidth */
		varlen += 1;

		/* mcs_nss */
		varlen += 4;

		/* coding */
		varlen += 1;

		/* group_id */
		varlen += 1;

		/* partial_aid */
		varlen += 2;
	}

	mtk_rt_hdr = (struct mtk_radiotap_header *)skb_push(pOSPkt, sizeof(*mtk_rt_hdr) + varlen);
	NdisZeroMemory(mtk_rt_hdr, sizeof(*mtk_rt_hdr) + varlen);

	mtk_rt_hdr->rt_hdr.it_version = PKTHDR_RADIOTAP_VERSION;
	mtk_rt_hdr->rt_hdr.it_pad = 0;
	mtk_rt_hdr->rt_hdr.it_len = cpu2le16(sizeof(*mtk_rt_hdr) + varlen);
	mtk_rt_hdr->rt_hdr.it_present = cpu2le32(
		(1 << IEEE80211_RADIOTAP_TSFT) |
		(1 << IEEE80211_RADIOTAP_FLAGS));

	if (PHYMODE < MODE_HTMIX) {
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_RATE);
	}

	mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_CHANNEL);

	if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)) {
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_MCS);	
	}

	if (AMPDU) {
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_AMPDU_STATUS);
	}

	if (PHYMODE == MODE_VHT)
		mtk_rt_hdr->rt_hdr.it_present |= cpu2le32(1 << IEEE80211_RADIOTAP_VHT); 

	varlen = 0;
	pos = mtk_rt_hdr->variable;
	
	padding_len = ((varlen % 8) == 0) ? 0 : (8 - (varlen % 8));
	pos += padding_len;
	varlen += padding_len;

	/* tsf */
	tmp64 = 0;
	NdisMoveMemory(pos, &tmp64, 8);
	pos += 8;
	varlen += 8;
	
	/* flags */
	*pos = 0;
	pos++;
	varlen++;

	/* rate */
	if (PHYMODE == MODE_OFDM) {
		rate_index = (UCHAR)(MCS) + 4;
		*pos = ralinkrate[rate_index];
		pos++;
		varlen++;
	} else if (PHYMODE == MODE_CCK) {
		rate_index = (UCHAR)(MCS);
		*pos = ralinkrate[rate_index];
		pos++;
		varlen++;
	}
	
	/* channel frequency */
	padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
	pos += padding_len;
	varlen += padding_len;

#define ieee80211chan2mhz(x)	\
	(((x) <= 14) ? \
	(((x) == 14) ? 2484 : ((x) * 5) + 2407) : \
	 ((x) + 1000) * 5)

	tmp16 = cpu2le16(ieee80211chan2mhz(Channel));
	NdisMoveMemory(pos, &tmp16, 2);
	pos += 2;
	varlen += 2;

	if (Channel > 14) {
		tmp16 = cpu2le16((IEEE80211_CHAN_OFDM | IEEE80211_CHAN_5GHZ));
	} else {
		if (PHYMODE == MODE_CCK) {
			tmp16 = cpu2le16(IEEE80211_CHAN_CCK | IEEE80211_CHAN_2GHZ);
		} else {
			tmp16 = cpu2le16(IEEE80211_CHAN_OFDM | IEEE80211_CHAN_2GHZ);
		}
	}
	
	NdisMoveMemory(pos, &tmp16, 2);
	pos += 2;
	varlen += 2;

	/* HT MCS */
	if ((PHYMODE == MODE_HTMIX) || (PHYMODE == MODE_HTGREENFIELD)) {

		*pos = (IEEE80211_RADIOTAP_MCS_HAVE_BW |
				IEEE80211_RADIOTAP_MCS_HAVE_MCS |	
				IEEE80211_RADIOTAP_MCS_HAVE_GI |
				IEEE80211_RADIOTAP_MCS_HAVE_FMT |
				IEEE80211_RADIOTAP_MCS_HAVE_FEC);

		pos++;
		varlen++;

		/* BW */
		if (BW == 0) {
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_20);	
		} else {
			*pos = HT_BW(IEEE80211_RADIOTAP_MCS_BW_40);
		}

		/* HT GI */
		*pos |= HT_GI(ShortGI); 

		/* HT format */
		if (PHYMODE == MODE_HTMIX)
			*pos |= HT_FORMAT(0); 
		else if (PHYMODE == MODE_HTGREENFIELD)
			*pos |= HT_FORMAT(1);

		/* HT FEC type */
		*pos |= HT_FEC_TYPE(LDPC);

		pos++;
		varlen++;

		/* HT mcs index */
		*pos = MCS;

		pos++;
		varlen++;
	}

	if (AMPDU) {
		/* reference number */
		padding_len = ((varlen % 4) == 0) ? 0 : (4 - (varlen % 4));
		varlen += padding_len;
		pos += padding_len;
		tmp32 = 0;
		NdisMoveMemory(pos, &tmp32, 4);
		pos += 4;
		varlen += 2;

		/* flags */
		tmp16 = 0;
		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;

		/* delimiter CRC value */
		*pos = 0;
		pos++;
		varlen++;

		/* reserved */
		*pos = 0;
		pos++;
		varlen++;
	}

#ifdef DOT11_VHT_AC
	/* VHT */
	if (PHYMODE == MODE_VHT) {

		/* known */
		padding_len = ((varlen % 2) == 0) ? 0 : (2 - (varlen % 2));
		varlen += padding_len;
		pos += padding_len;
		
		tmp16 = cpu2le16(IEEE80211_RADIOTAP_VHT_KNOWN_STBC |
						IEEE80211_RADIOTAP_VHT_KNOWN_GI |
						IEEE80211_RADIOTAP_VHT_KNOWN_LDPC_EXTRA_OFDM_SYM |
						IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH);
	
		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;

		/* flags */
		*pos = (STBC?IEEE80211_RADIOTAP_VHT_FLAG_STBC:0);
		*pos |= (ShortGI?IEEE80211_RADIOTAP_VHT_FLAG_SGI:0);
		*pos |= (LDPC_EX_SYM?IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM:0);

		pos++;
		varlen++;

		/* bandwidth */
		if (BW == 0) {
			*pos = 0;
		} else if (BW == 1) {
				*pos = 1;
		} else if (BW == 2) {
				*pos = 4;

		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:unknow bw(%d)\n", __FUNCTION__, BW));
		}

		/* mcs_nss */
		pos++;
		varlen++;

		/* vht_mcs_nss[0] */
		*pos = (GET_VHT_NSS(MCS) & 0x0f);
		*pos |= ((GET_VHT_MCS(MCS) & 0x0f) << 4);
		pos++;
		varlen++;

		/* vht_mcs_nss[1] */
		*pos = 0;
		pos++;
		varlen++;

		/* vht_mcs_nss[2] */
		*pos = 0;
		pos++;
		varlen++;

		/* vht_mcs_nss[3] */
		*pos = 0;
		pos++;
		varlen++;

		/* coding */
		if (LDPC)
			*pos = 1;
		else
			*pos = 0;

		pos++;
		varlen++;
		
		/* group_id */
		*pos = 0;
		pos++;
		varlen++;

		/* partial aid */
		tmp16 = 0;
		NdisMoveMemory(pos, &tmp16, 2);
		pos += 2;
		varlen += 2;
	}
#endif /* DOT11_VHT_AC */

	pOSPkt->dev = pOSPkt->dev;
	pOSPkt->mac_header = pOSPkt->data;
	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->protocol = __constant_htons(ETH_P_80211_RAW);
	pOSPkt->ip_summed = CHECKSUM_NONE;
	netif_rx_ni(pOSPkt);
	return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(NULL, pRxPacket, NDIS_STATUS_FAILURE);
	return;
}

