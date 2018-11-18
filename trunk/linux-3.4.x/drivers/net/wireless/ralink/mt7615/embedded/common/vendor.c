#include "rt_config.h"


ULONG build_vendor_ie(struct _RTMP_ADAPTER *pAd, 
        struct wifi_dev *wdev, UCHAR *frame_buffer
#ifdef WH_EZ_SETUP
		, UCHAR SubType
#endif
		)
{
    struct _ralink_ie ra_ie;
    ULONG ra_ie_len = 0;
#ifdef MT_MAC
    struct _mediatek_ie mtk_ie;
    ULONG mtk_ie_len = 0;
    struct _mediatek_vht_ie mtk_vht_ie;
    ULONG mtk_vht_ie_len = 0;
    UCHAR mtk_vht_cap[] = {
        0xBF, // EID
        0x0C, // LEN
        0xB1, 0x01, 0xC0, 0x33, // VHT Cap. Info.
        0x2A, 0xFF, 0x92, 0x04, 0x2A, 0xFF, 0x92, 0x04 //Supported MCS and Nss
    };
    UCHAR mtk_vht_op[] = {
        0xC0, // EID
        0x05, // LEN
        0x0, 0x0, 0x0, // VHT Op. Info.
        0x2A, 0xFF // Basic MCS and Nss
    };
    UCHAR mtk_vht_txpwr_env[] = {
        0xC3, // EID
        0x03, // LEN
        0x01, // TX PWR Info.
        0x02, 0x02 // Max. Power for 20 & 40Mhz
    };
#endif /* MT_MAC */
    ULONG vendor_ie_len = 0;

    NdisZeroMemory(&ra_ie, sizeof(struct _ralink_ie));

    ra_ie.ie_hdr.eid = IE_VENDOR_SPECIFIC;
    ra_ie.ie_hdr.len = 0x7;
    ra_ie.oui[0] = 0x00;
    ra_ie.oui[1] = 0x0C;
    ra_ie.oui[2] = 0x43;

    if (pAd->CommonCfg.bAggregationCapable) {
        ra_ie.cap0 |= RALINK_AGG_CAP;
    }

    if (pAd->CommonCfg.bPiggyBackCapable) {
        ra_ie.cap0 |= RALINK_PIGGY_CAP;
    }

    if (pAd->CommonCfg.bRdg) {
        ra_ie.cap0 |= RALINK_RDG_CAP;
    }

    if (pAd->CommonCfg.g_band_256_qam && pAd->chipCap.g_band_256_qam 
            && WMODE_CAP(wdev->PhyMode, WMODE_GN)) {
        ra_ie.cap0 |= RALINK_256QAM_CAP;
    }

    MakeOutgoingFrame(frame_buffer,
            &ra_ie_len, (ra_ie.ie_hdr.len + 2), &ra_ie,
            END_OF_ARGS);

    //hex_dump ("build vendor_ie: Ralink_OUI", frame_buffer, (ra_ie.ie_hdr.len + 2));

    vendor_ie_len = ra_ie_len;

#ifdef MT_MAC
    if (pAd->chipCap.hif_type == HIF_MT) {

#ifdef WH_EZ_SETUP
		/*
			To prevent old device has trouble to parse MTK vendor IE,
			insert easy setup IE first.
		*/
		if (IS_EZ_SETUP_ENABLED(wdev) && 
			((SubType == SUBTYPE_BEACON) || (SubType == SUBTYPE_PROBE_RSP))) {
			vendor_ie_len += ez_build_beacon_ie(wdev, (frame_buffer + vendor_ie_len));
		} else {
			if (ez_is_triband())
			{
				if (!IS_EZ_SETUP_ENABLED(wdev) && ((SubType == SUBTYPE_BEACON) || (SubType == SUBTYPE_PROBE_RSP)))
				{
					ULONG tmp_len = 0;
#ifdef EZ_MOD_SUPPORT
					ez_triband_insert_tlv(wdev->sys_handle, EZ_TAG_NON_EZ_BEACON, 
						frame_buffer + vendor_ie_len,
						&tmp_len);
#else
					ez_insert_tlv(EZ_TAG_NON_EZ_BEACON, 
						NULL, 0, frame_buffer + vendor_ie_len, &tmp_len);
#endif
					vendor_ie_len += tmp_len;
				}
			}
		}
#endif /* WH_EZ_SETUP */

        NdisZeroMemory(&mtk_ie, sizeof(struct _mediatek_ie));
        NdisZeroMemory(&mtk_vht_ie, sizeof(struct _mediatek_vht_ie));
        mtk_vht_ie_len = sizeof(mtk_vht_cap) + sizeof(mtk_vht_op) + sizeof(mtk_vht_txpwr_env);

        mtk_ie.ie_hdr.eid = IE_VENDOR_SPECIFIC;
        mtk_ie.ie_hdr.len = (0x7 + mtk_vht_ie_len);
        mtk_ie.oui[0] = 0x00;
        mtk_ie.oui[1] = 0x0C;
        mtk_ie.oui[2] = 0xE7;
        //MTK VHT CAP IE
        memcpy(&mtk_vht_ie.vht_cap, (VHT_CAP *)mtk_vht_cap, sizeof(VHT_CAP));
        //MTK VHT OP IE
        memcpy(&mtk_vht_ie.vht_op, (VHT_OP *)mtk_vht_op, sizeof(VHT_OP));
        //MTK VHT TX PWR ENV IE
        memcpy(&mtk_vht_ie.vht_txpwr_env, (VHT_TX_PWR_ENV *)mtk_vht_txpwr_env,
                sizeof(VHT_TX_PWR_ENV));

        if (pAd->CommonCfg.g_band_256_qam && pAd->chipCap.g_band_256_qam
                && WMODE_CAP(wdev->PhyMode, WMODE_GN)) {
            mtk_ie.cap0 |= MEDIATEK_256QAM_CAP;
        }
        
#ifdef MWDS
        if(wdev->bSupportMWDS) {
            mtk_ie.cap0 |= MEDIATEK_MWDS_CAP;
        }
#endif /* MWDS */
#ifdef STA_FORCE_ROAM_SUPPORT
				if(pAd->en_force_roam_supp)
					mtk_ie.cap0 |= MEDIATEK_CLI_ENTRY;
#endif /* MWDS */
        MakeOutgoingFrame((frame_buffer + vendor_ie_len),
                &mtk_ie_len, sizeof(struct _mediatek_ie), &mtk_ie,
                END_OF_ARGS);

        vendor_ie_len += mtk_ie_len;

        MakeOutgoingFrame((frame_buffer + vendor_ie_len),
                &mtk_vht_ie_len,
                (sizeof(mtk_vht_cap) + sizeof(mtk_vht_op) + sizeof(mtk_vht_txpwr_env)),
                &mtk_vht_ie,
                END_OF_ARGS);

        //hex_dump ("build vendor_ie: MediaTek_OUI",
         //       (frame_buffer+vendor_ie_len-mtk_ie_len), (mtk_ie.ie_hdr.len + 2));

        vendor_ie_len += mtk_vht_ie_len;

#ifdef WH_EVENT_NOTIFIER
        if(wdev->custom_vie.ie_hdr.len > 0)
        {
            ULONG custom_vie_len;
            ULONG total_custom_vie_len = sizeof(struct Custom_IE_Header) + wdev->custom_vie.ie_hdr.len;
            MakeOutgoingFrame((frame_buffer + vendor_ie_len),
                        &custom_vie_len, total_custom_vie_len, (UCHAR*)&wdev->custom_vie, END_OF_ARGS);
            vendor_ie_len += custom_vie_len;
        }
#endif /* WH_EVENT_NOTIFIER */
    }
#endif /* MT_MAC */


    return vendor_ie_len;
}


VOID check_vendor_ie(struct _RTMP_ADAPTER *pAd,
        UCHAR *ie_buffer, struct _vendor_ie_cap *vendor_ie)
{
    PEID_STRUCT info_elem = (PEID_STRUCT)ie_buffer;
    UCHAR ralink_oui[] = {0x00, 0x0c, 0x43};
    UCHAR mediatek_oui[] = {0x00, 0x0c, 0xe7};
    UCHAR broadcom_oui[] = {0x00, 0x90, 0x4c};
    UCHAR quantenna_oui[] = {0x00, 0x26, 0x86};
    //UCHAR broadcom_fixed_pattern[] = {0x04, 0x08};

    if (NdisEqualMemory(info_elem->Octet, ralink_oui, 3) 
            && (info_elem->Len == 7))
    {
        vendor_ie->ra_cap = (ULONG)info_elem->Octet[3];
		vendor_ie->is_rlt = TRUE;
        vendor_ie->ldpc = TRUE;
        vendor_ie->sgi = TRUE;
        //hex_dump ("recv. vendor_ie: Ralink_OUI", (UCHAR *)info_elem, (info_elem->Len + 2));
    }
    else if (NdisEqualMemory(info_elem->Octet, mediatek_oui, 3)
            && (info_elem->Len >= 7))
    {
		vendor_ie->mtk_cap = (ULONG)info_elem->Octet[3];
		vendor_ie->is_mtk = TRUE;

#ifdef WH_EZ_SETUP        // Rakesh: Parse Easy setup IE only if easy enabled on own, acceptable??
	    if ((IS_ADPTR_EZ_SETUP_ENABLED(pAd)) && (info_elem->Octet[3] & MEDIATEK_EASY_SETUP)) {
			ez_vendor_ie_parse(vendor_ie, info_elem);
	    }
        else 
#endif /* WH_EZ_SETUP */
		if (info_elem->Len > 7) {

            /* have MTK VHT IEs */
            vendor_ie->ldpc = TRUE;
            vendor_ie->sgi = TRUE;
#ifdef MWDS
            /* We can't be covered by easy setup customized mtk ie. */
            vendor_ie->mtk_cap_found = TRUE;
            if (MWDS_SUPPORT(vendor_ie->mtk_cap)) {
                vendor_ie->support_mwds = TRUE;
            } else {
                vendor_ie->support_mwds = FALSE;
            }
#endif /* MWDS */
        }
        else {
            vendor_ie->ldpc = FALSE;
            vendor_ie->sgi = FALSE;
        }
        //hex_dump ("recv. vendor_ie: MediaTek_OUI", (UCHAR *)info_elem, (info_elem->Len + 2));
    }
    else if (NdisEqualMemory(info_elem->Octet, broadcom_oui, 3)
            //&& NdisEqualMemory(info_elem->Octet+3, broadcom_fixed_pattern, 2))
        )
    {
        vendor_ie->brcm_cap = BROADCOM_256QAM_CAP;
        vendor_ie->ldpc = TRUE;
        vendor_ie->sgi = TRUE;
        //hex_dump ("recv. vendor_ie: Broadcom_OUI", (UCHAR *)info_elem, (info_elem->Len + 2));
    }
    else if (NdisEqualMemory(info_elem->Octet, quantenna_oui, 3))
    {
        vendor_ie->is_quant = TRUE;
    }
#ifdef WH_EVENT_NOTIFIER
    else if(pAd->ApCfg.EventNotifyCfg.CustomOUILen && 
            (info_elem->Len >= pAd->ApCfg.EventNotifyCfg.CustomOUILen) &&
            NdisEqualMemory(info_elem->Octet, pAd->ApCfg.EventNotifyCfg.CustomOUI,pAd->ApCfg.EventNotifyCfg.CustomOUILen))
    {
        vendor_ie->custom_ie_len = info_elem->Len;
        NdisMoveMemory(vendor_ie->custom_ie, info_elem->Octet, info_elem->Len);
    }
#endif /* WH_EVENT_NOTIFIER */
    else 
    {
        //printk ("Other Vendor IE: OUI: %02x %02x %02x\n", 
         //       info_elem->Octet[0], info_elem->Octet[1], info_elem->Octet[2]);
        //hex_dump ("recv. vendor_ie: xxx_OUI", (UCHAR *)info_elem, (info_elem->Len + 2));
    }
}
