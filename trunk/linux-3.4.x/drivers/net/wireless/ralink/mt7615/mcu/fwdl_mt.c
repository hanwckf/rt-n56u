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
	fwdl.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "fwdl_mt.tmh"
#endif

struct MCU_CTRL;

#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif


static INT32 MtCmdFwScatters(RTMP_ADAPTER *ad, UINT8 *image, UINT32 image_len)
{
	UINT32 sent_len;
	UINT32 cur_len = 0, count = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	while (1)
	{
		UINT32 sent_len_max = MT_UPLOAD_FW_UNIT - cap->cmd_header_len;
		sent_len = (image_len - cur_len) >=  sent_len_max ? 
                        sent_len_max : (image_len - cur_len);

		if (sent_len > 0) 
        {
			ret = MtCmdFwScatter(ad, image + cur_len, sent_len, count);
			count++;
			if (ret)
            {
				goto error;
            }
            cur_len += sent_len;
		} 
        else 
        {
			break;
		}
	}

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
                ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}




static UINT32 AndesMTLoadFwMethod1(RTMP_ADAPTER *ad)
{
	UINT32 value, loop, dl_len;
	UINT32 ret;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (cap->load_code_method == BIN_FILE_METHOD) 
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                ("load fw image from /lib/firmware/%s\n", 
                                cap->fw_bin_file_name));
		os_load_code_from_bin(ad,&cap->FwImgBuf, 
                cap->fw_bin_file_name, &cap->fw_len);
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("load fw image from fw_header_image\n"));
		cap->FwImgBuf = cap->fw_header_image;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
        ("%s(%d)::pChipCap->fw_len(%d)\n", __FUNCTION__, __LINE__, cap->fw_len));

	if (!cap->FwImgBuf)
	{
		if (cap->load_code_method == BIN_FILE_METHOD) 
        {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", 
                    __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} 
        else 
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s:Please assign a fw image(fw_header_image), load_method(%d)\n",
				                        __FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = FW_DOWNLOAD;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FW Version:"));
	for (loop = 0; loop < 10; loop++)
	{	
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%c", *(cap->FwImgBuf + cap->fw_len - 29 + loop)));
    }
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FW Build Date:"));
	for (loop = 0; loop < 15; loop++)
    {   
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%c", *(cap->FwImgBuf + cap->fw_len - 19 + loop)));
    }
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

    dl_len = (*(cap->FwImgBuf + cap->fw_len - 1) << 24) |
             (*(cap->FwImgBuf + cap->fw_len - 2) << 16) |
             (*(cap->FwImgBuf + cap->fw_len - 3) <<  8) |
              *(cap->FwImgBuf + cap->fw_len - 4);

	dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, 
                    ("\ndownload len = %d\n", dl_len));

#ifdef DMA_SCH_SUPPORT
	MtAsicSetDmaByPassMode(ad, TRUE);
#endif /* DMA_SCH_SUPPORT */

	/* optional CMD procedure */
	/* CMD restart download flow request */
	if (MtAsicGetMcuStatus(ad, METH1_RAM_CODE))
	{
		ret = MtCmdRestartDLReq(ad);
		if (ret)
        {      
			goto done;
        }    
	}

	/* check rom code if ready */
	loop = 0;

	do
	{
		if ((value = MtAsicGetMcuStatus(ad,METH1_ROM_CODE)))
        {	
            break;
        }
        os_msec_delay(1);
		loop++;
	} while (loop <= 500);

	if (loop > 500) 
    {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
            ("%s: rom code is not ready(TOP_MISC2 = %d)\n", __FUNCTION__, value));
		goto done;
	}

	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = MtCmdAddressLenReq(ad, FW_CODE_START_ADDRESS1, 
	                dl_len, TARGET_ADDR_LEN_NEED_RSP);
	if (ret)
	{
		goto done;
	}

	/* 2. Loading firmware image phase */
	ret = MtCmdFwScatters(ad, cap->FwImgBuf, dl_len);
	if (ret)
	{
		goto done;
	}
	/* 3. Firmware start negotiation phase */
	ret = MtCmdFwStartReq(ad, 1, FW_CODE_START_ADDRESS1);

	/* 4. check Firmware running */
	for (loop = 0; loop < 500; loop++)
	{
		if (MtAsicGetMcuStatus(ad, METH1_RAM_CODE))
        {	
            break;
        }
		os_msec_delay(1);
	}

	if (loop == 500)
	{
		ret = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                        ("firmware loading failure\n"));
		Ctl->Stage = FW_NO_INIT;
	}
	else
	{
		Ctl->Stage = FW_RUN_TIME;
	}

done:
#ifdef DMA_SCH_SUPPORT
	MtAsicSetDmaByPassMode(ad, FALSE);
#endif

    if (cap->FwImgBuf)
    {
        NICEraseFirmware(ad);
    }
	return ret;
}


static UINT32 AndesMTLoadFwMethod2(RTMP_ADAPTER *ad)
{

	UINT32 value, loop, ilm_dl_len, dlm_dl_len;
	UINT8 ilm_feature_set, dlm_feature_set;
	UINT8 ilm_chip_info, dlm_chip_info;
	UINT32 ilm_target_addr, dlm_target_addr;
	UINT32 ret = 0;
	NTSTATUS status;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
    struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (cap->load_code_method == BIN_FILE_METHOD) 
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("load fw image from /lib/firmware/%s\n", 
                            cap->fw_bin_file_name));
		os_load_code_from_bin(ad,&cap->FwImgBuf, 
            cap->fw_bin_file_name, &cap->fw_len);

	} 
    else
	{
		cap->FwImgBuf = cap->fw_header_image;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
        ("%s(%d), cap->fw_len(%d)\n", __FUNCTION__, __LINE__, cap->fw_len));

	if (!cap->FwImgBuf)
	{
		if (cap->load_code_method == BIN_FILE_METHOD) 
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
            ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", 
                __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} 
        else
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s:Please assign a fw image, load_method(%d)\n",
				        __FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = FW_DOWNLOAD;

	ilm_target_addr =   (*(cap->FwImgBuf + cap->fw_len - (33 + 36)) << 24) |
				        (*(cap->FwImgBuf + cap->fw_len - (34 + 36)) << 16) |
                        (*(cap->FwImgBuf + cap->fw_len - (35 + 36)) <<  8) |
                         *(cap->FwImgBuf + cap->fw_len - (36 + 36));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
        ("ILM target address = %x\n", ilm_target_addr));

	ilm_chip_info = *(cap->FwImgBuf + cap->fw_len - (32 + 36));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
        ("\nILM chip information = %x\n", ilm_chip_info));

	ilm_feature_set = *(cap->FwImgBuf + cap->fw_len - (31 + 36));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
        ("\nILM feature set = %x\n", ilm_feature_set));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nILM Build Date:"));

	for (loop = 0; loop < 8; loop++)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%c", *(cap->FwImgBuf + cap->fw_len - (20 + 36) + loop)));
	}
	
	ilm_dl_len = (*(cap->FwImgBuf + cap->fw_len - (1 + 36)) << 24) |
				 (*(cap->FwImgBuf + cap->fw_len - (2 + 36)) << 16) |
				 (*(cap->FwImgBuf + cap->fw_len - (3 + 36)) <<  8) |
				  *(cap->FwImgBuf + cap->fw_len - (4 + 36));

	ilm_dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
            ("\nILM download len = %d\n", ilm_dl_len));

	dlm_target_addr = (*(cap->FwImgBuf + cap->fw_len - 33) << 24) |
				      (*(cap->FwImgBuf + cap->fw_len - 34) << 16) |
				      (*(cap->FwImgBuf + cap->fw_len - 35) <<  8) |
				       *(cap->FwImgBuf + cap->fw_len - 36);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
        ("DLM target address = %x\n", dlm_target_addr));

	dlm_chip_info = *(cap->FwImgBuf + cap->fw_len - 32);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
        ("\nDLM chip information = %x\n", dlm_chip_info));

	dlm_feature_set = *(cap->FwImgBuf + cap->fw_len - 31);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
            ("\nDLM feature set = %x\n", dlm_feature_set));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DLM Build Date:"));

	for (loop = 0; loop < 8; loop++)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%c", *(cap->FwImgBuf + cap->fw_len - 20 + loop)));
	}
	
	dlm_dl_len = (*(cap->FwImgBuf + cap->fw_len - 1) << 24) |
				 (*(cap->FwImgBuf + cap->fw_len - 2) << 16) |
				 (*(cap->FwImgBuf + cap->fw_len - 3) <<  8) |
				  *(cap->FwImgBuf + cap->fw_len - 4);

	dlm_dl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
            ("\nDLM download len = %d\n", dlm_dl_len));

#ifdef DMA_SCH_SUPPORT
	MtAsicSetDmaByPassMode(ad, TRUE);
#endif /* DMA_SCH_SUPPORT */

	/* optional CMD procedure */
	/* CMD restart download flow request */
	/* check if SW_SYN0 at init. state */
	
	value = MtAsicGetFwSyncValue(ad);

	if (value == 0x0)
	{
		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		    ("%s: 1. power on WiFi SYS\n", __FUNCTION__));
		status = MtCmdPowerOnWiFiSys(ad);

		if (status)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: failed to power on WiFi SYS\n", __FUNCTION__));
			goto done;
		}

		/* poll SW_SYN0 == 1 */
		loop = 0;
		do
		{
			value = MtAsicGetFwSyncValue(ad);
			
			if (value == 0x1)
			{	
			    break;
            }
			os_msec_delay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 1. SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", 
                                                __FUNCTION__, value));
			goto done;
		}
	}
	else if (value == 0x3)
	{
		/* restart cmd*/
		ret = MtCmdRestartDLReq(ad);

		if (ret)
        {	
            goto done;
        }
		/* poll SW_SYN0 == 0 */
		loop = 0;
		do
		{
			value = MtAsicGetFwSyncValue(ad);
			if (value == 0x0)
            {	
                break;
            }
            os_msec_delay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 2. SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", 
                                                __FUNCTION__, value));
			goto done;
		}

		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		    ("%s: 2. power on WiFi SYS\n", __FUNCTION__));
		status = MtCmdPowerOnWiFiSys(ad);
		if (status)
        {	
            goto done;
        }
		/* poll SW_SYN0 == 1*/
		loop = 0;
		do
		{
			
			value = MtAsicGetFwSyncValue(ad);			
			if (value == 0x1)
            {	
                break;
            }
            os_msec_delay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", 
                                            __FUNCTION__, value));
			goto done;
		}
	} 
    else 
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
            ("%s: (SW_SYN0 = %d), ready to ILM/DLM DL...\n", 
                                    __FUNCTION__, value));
	}

	/*  ILM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
    ret = MtCmdAddressLenReq(ad, ilm_target_addr, ilm_dl_len,
        ((ilm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV : 0) |
         (ilm_feature_set & FW_FEATURE_SET_ENCRY) | TARGET_ADDR_LEN_NEED_RSP |
                (FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(ilm_feature_set))));

	if (ret)
    {	
        goto done;
    }
	/* 2. Loading ilm firmware image phase */
	Ctl->Stage = FW_DOWNLOAD_SCATTER;
	ret = MtCmdFwScatters(ad, cap->FwImgBuf, ilm_dl_len);
	Ctl->Stage = FW_DOWNLOAD;

	if (ret)
    {	
        goto done;
    }


	/*  DLM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = MtCmdAddressLenReq(ad, dlm_target_addr, dlm_dl_len,
        ((dlm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV : 0) |
         (dlm_feature_set & FW_FEATURE_SET_ENCRY) | TARGET_ADDR_LEN_NEED_RSP |
                (FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(dlm_feature_set))));

	if (ret)
    {	
        goto done;
    }


	/* 2. Loading dlm firmware image phase */
	Ctl->Stage = FW_DOWNLOAD_SCATTER;
	ret = MtCmdFwScatters(ad, cap->FwImgBuf + ilm_dl_len, dlm_dl_len);
	Ctl->Stage = FW_DOWNLOAD;

	if (ret)
    {	
        goto done;
    }


	/* 3. Firmware start negotiation phase */
	ret = MtCmdFwStartReq(ad, 0, 0);


#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* 4. check Firmware running */
	for (loop = 0; loop < 500; loop++)
	{
		value = MtAsicGetFwSyncValue(ad);	
		if ((value & 0x03) == 0x03)
        {	
            break;
        }
		os_msec_delay(1);
	}

#endif
/*Since FW will support can read rom code cmd as soon as possible, currently not modify it.*/

	if (loop == 500)
	{
		ret = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                        ("firmware loading failure\n"));
		Ctl->Stage = FW_NO_INIT;
	}
	else
	{
		Ctl->Stage = FW_RUN_TIME;
	}
done:
	MtAsicSetDmaByPassMode(ad, FALSE);

    if (cap->FwImgBuf)
    {
        NICEraseFirmware(ad);
    }

	return ret;
}

#ifdef MT7615

#ifdef PALLADIUM
#define WAIT_LOOP 500 * 200
#else
#define WAIT_LOOP 1500
#endif

/* count the fw dl fail count */
//static UINT32 patch_download_fail_count = 0;
//static UINT32 n9_fw_download_fail_count = 0;
//static UINT32 total_fw_download_count = 0;



static VOID fw_info(CHAR* fw_image, UINT32 fw_len, UINT32 offset,
    UINT32 *ptarget_addr, UINT8 *pchip_info, UINT8 *pfeature_set, UINT32 *pdl_len)
{
    UINT32 loop;
	*ptarget_addr = (UINT32)(((UINT8)*(fw_image + fw_len - (33 + offset)) << 24) |
				((UINT8)*(fw_image + fw_len - (34 + offset)) << 16) |
				((UINT8)*(fw_image + fw_len -(35 + offset)) << 8) |
				(UINT8)*(fw_image + fw_len - (36 + offset)));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("target address = %x\n", *ptarget_addr));

	*pchip_info = (UINT8)*(fw_image + fw_len - (32 + offset));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("chip information = %x\n", *pchip_info));

	*pfeature_set = (UINT8)*(fw_image + fw_len - (31 + offset));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("feature set = %x\n", *pfeature_set));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Build Date:"));
	for (loop = 0; loop < 13; loop++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", (UINT8)*(fw_image + fw_len - (20 + offset) + loop)));
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	*pdl_len = (UINT32)(((UINT8)*(fw_image + fw_len - (1 + offset)) << 24) |
				((UINT8)*(fw_image + fw_len - (2 + offset)) << 16) |
				((UINT8)*(fw_image + fw_len -(3 + offset)) << 8) |
				(UINT8)*(fw_image + fw_len - (4 + offset)));

	*pdl_len += 4; /* including crc value */

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("download len = %d\n", *pdl_len));

}

VOID MtSetDirectPath(RTMP_ADAPTER *ad)
{
    UINT32 value;
#ifdef RTMP_PCI_SUPPORT
    HW_IO_WRITE32(ad, MCU_PCIE_REMAP_2, 0x82000000);
#endif /* RTMP_PCI_SUPPORT */
    MAC_IO_READ32(ad, 0xec054, &value);
    value &= ~0x00000003;
    value |= 0x00000001;
    MAC_IO_WRITE32(ad, 0xec054, value);
#ifdef RTMP_PCI_SUPPORT
    HW_IO_WRITE32(ad, MCU_PCIE_REMAP_2, 0x80080000);
#endif /* RTMP_PCI_SUPPORT */
}

#ifdef RTMP_PCI_SUPPORT
INT32 AndesResetMcuViaWdt(RTMP_ADAPTER *pAd)
{

#define WDT_SWRST_CR_PA 0x81080044
#define MCU_POWER_ON 0x01
#define HOST_TRIGGER_WDT_SWRST 0x1209

    UINT32 top_clock_gen0_value = 0;
    UINT32 top_clock_gen1_value = 0;
    UINT32 top_misc_value = 0;
    UINT32 reset_count = 0;
    UINT32 origonal_remap_cr_value = 0;
    UINT32 remap_cr_record_base_address = 0;
    UINT32 offset_between_target_and_remap_cr_base = 0;

    /* switch hclk to XTAL source, 0x80021100[1:0] = 2'b00 */
    HW_IO_READ32(pAd, TOP_CKGEN0, &top_clock_gen0_value);
    top_clock_gen0_value &= ~(BIT0 | BIT1);
    HW_IO_WRITE32(pAd, TOP_CKGEN0, top_clock_gen0_value);

    /* Set HCLK divider to 1:1, 0x80021104[1:0] = 2'b00 */
    HW_IO_READ32(pAd, TOP_CKGEN1, &top_clock_gen1_value);
    top_clock_gen1_value &= ~(BIT0 | BIT1);
    HW_IO_WRITE32(pAd, TOP_CKGEN1, top_clock_gen1_value);

    /* disable HIF can be reset by WDT 0x80021130[30]=1'b0 */
    HW_IO_READ32(pAd, TOP_MISC, &top_misc_value);
    top_misc_value &= ~(BIT30);
    HW_IO_WRITE32(pAd, TOP_MISC, top_misc_value);

    /* enable WDT reset mode and trigger WDT reset 0x81080044 = 0x1209 */
    /* keep the origonal remap cr1 value for restore */
    HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &origonal_remap_cr_value);
    /* do PCI-E remap for CR4 PDMA physical base address to 0x40000 */
    HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, WDT_SWRST_CR_PA);

    HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &remap_cr_record_base_address);

    if ((WDT_SWRST_CR_PA  - remap_cr_record_base_address) > REMAP_1_OFFSET_MASK)
    {
        /* restore the origonal remap cr1 value */
        HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, origonal_remap_cr_value);
        
    }
    offset_between_target_and_remap_cr_base =
        ((WDT_SWRST_CR_PA  - remap_cr_record_base_address) & REMAP_1_OFFSET_MASK);

    RTMP_IO_WRITE32(pAd, MT_PCI_REMAP_ADDR_1 + offset_between_target_and_remap_cr_base,
                                                        HOST_TRIGGER_WDT_SWRST);
    /* restore the origonal remap cr1 value */
    HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, origonal_remap_cr_value);

    /* polling the HW ready to FW DL status */
    while (MtAsicGetFwSyncValue(pAd)!= MCU_POWER_ON) {   
        reset_count++;
        os_msec_delay(1);
        if (reset_count == WAIT_LOOP) {   
            MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                ("%s: Reach polling Max count, MtAsicGetFwSyncValue = %d\n", 
                        __FUNCTION__ , MtAsicGetFwSyncValue(pAd)));
            return NDIS_STATUS_FAILURE;
        }
    }
    
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
        ("%s: polling count = %d, MtAsicGetFwSyncValue = %d\n", 
            __FUNCTION__ , reset_count, MtAsicGetFwSyncValue(pAd)));
    
    return NDIS_STATUS_SUCCESS;
}
#endif

INT32 AndesRestartCheck(RTMP_ADAPTER *ad)
{
	UINT32 value,loop;
	INT32 Ret=0;
	NTSTATUS status;

	/* check if TOP_MISC2 at init. state */
	value = MtAsicGetFwSyncValue(ad);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
        ("%s: Current TOP_MISC2(0x%x)\n", __FUNCTION__, value));
    if ((value & (BIT0|BIT1|BIT2)) == (BIT0|BIT1|BIT2))
	{
		/* restart cmd*/
       	//MtSetDirectPath(ad);
		Ret = MtCmdRestartDLReq(ad);
		if (Ret)
		{
		    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                    ("%s: MtCmdRestartDLReq fail(ret = %d)\n", __FUNCTION__,Ret));
			Ret = NDIS_STATUS_FAILURE;
			goto done;
		}

		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		    ("%s: 2. power on WiFi SYS\n", __FUNCTION__));
		status = MtCmdPowerOnWiFiSys(ad);
		if (status)
		{
			Ret = NDIS_STATUS_FAILURE;
			goto done;
		}

		/* poll TOP_MISC2 from 0x7 -> 0x1 */
		loop = 0;
		do
		{
			value = MtAsicGetFwSyncValue(ad);
			if ((value & (BIT0|BIT1|BIT2)) == BIT0)
            {	
                break;
            }
            RtmpOsMsDelay(1);
			loop++;
        	
		} while (loop <= WAIT_LOOP);

		if (loop > WAIT_LOOP)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: TOP_MISC2 is not at init. state (TOP_MISC2 = %d)\n", 
                                                    __FUNCTION__, value));
			Ret = NDIS_STATUS_FAILURE;
			goto done;
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
            ("%s:  TOP_MISC2(%d)\n", __FUNCTION__, value));
	}
    else if ((value & BIT0) == 0)
	{
		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		    ("%s: 1. power on WiFi SYS\n", __FUNCTION__));
		status = MtCmdPowerOnWiFiSys(ad);

		if (status)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: failed to power on WiFi SYS\n", __FUNCTION__));
			Ret = NDIS_STATUS_FAILURE;
			goto done;
		}

		/* poll TOP_MISC2 == 1 */
		loop = 0;
		do
		{
			value = MtAsicGetFwSyncValue(ad);
			if ((value & BIT0) == BIT0)
				break;
			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= WAIT_LOOP);

		if (loop > WAIT_LOOP)
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 1. TOP_MISC2 is not at init. state (TOP_MISC2 = %d)\n", 
                                                __FUNCTION__, value));
			Ret = NDIS_STATUS_FAILURE;
			goto done;
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%s: (TOP_MISC2 = %d), ready to continue...RET(%d)\n", 
                                        __FUNCTION__, value, Ret));
	}
done:
    
	return Ret;
}

static NDIS_STATUS AndesMTLoadFwMethodFwDlRing(RTMP_ADAPTER *ad)
{
    UINT32 value, loop, ilm_dl_len, dlm_dl_len, cr4_dl_len;
    UINT8 ilm_feature_set, dlm_feature_set, cr4_feature_set;
    UINT8 ilm_chip_info, dlm_chip_info, cr4_chip_info;
    UINT32 ilm_target_addr, dlm_target_addr, cr4_target_addr;
    UINT32 Ret = 0;
    RTMP_CHIP_CAP *cap = &ad->chipCap;
    struct MCU_CTRL *Ctl = &ad->MCUCtrl;

    if (cap->load_code_method == BIN_FILE_METHOD) {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
        os_load_code_from_bin(ad,&cap->FwImgBuf, 
                cap->fw_bin_file_name, &cap->fw_len);
    } else {
        cap->FwImgBuf = cap->fw_header_image;
    }

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
        ("%s(%d), cap->fw_len(%d)\n", __FUNCTION__, __LINE__, cap->fw_len));

    if (!cap->FwImgBuf) {
        if (cap->load_code_method == BIN_FILE_METHOD) {
            MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
            ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", 
                    __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
        } else  {
            MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s:Please assign a fw image, load_method(%d)\n",
                        __FUNCTION__, cap->load_code_method));
        }
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }

    Ctl->Stage = FW_DOWNLOAD;

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\niLM:\n"));
    fw_info(cap->FwImgBuf, cap->fw_len, 36, &ilm_target_addr, 
            &ilm_chip_info, &ilm_feature_set, &ilm_dl_len);
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\ndLM:\n"));
    fw_info(cap->FwImgBuf, cap->fw_len, 0, &dlm_target_addr, 
            &dlm_chip_info, &dlm_feature_set, &dlm_dl_len);

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
            ("\nDLM download len = %d\n", dlm_dl_len));

    /*
     * PDMA0-Tx Ring3 will be used of CR4/N9 firmware downloading.
     * Initial FwDwloRing at begining of firmware downloading.
     */
    ad->chipOps.pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsgFwDlRing;

    Ret = AndesRestartCheck(ad);
    if(Ret!=NDIS_STATUS_SUCCESS)
        goto done;
    
    /* this WAR is for E1 HW bug, should be checked when E3 */
#ifdef CONFIG_FPGA_MODE
#ifdef MT7615
    {
        RTMP_CHIP_CAP *pChipCap = &ad->chipCap;

        if (!pChipCap->need_load_rom_patch)
        {
            /*
             * RxRest is a flag which indicates whether filter the resp from firmware.
             * if value is zero, we can receive the resp after sending cmd.
             */
            ad->RxRest = 0;
            ad->RxResetDropCount = 10;
        }
    }
#endif /* MT7615 */
#endif /* CONFIG_FPGA_MODE */

    /*  ILM */
    /* standard CMD procedure */
    /* 1. Config PDA phase */
    Ret = MtCmdAddressLenReq(ad, ilm_target_addr, ilm_dl_len,
                (ilm_feature_set & FW_FEATURE_SET_ENCRY) |
                (FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(ilm_feature_set))) |
                ((ilm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
                TARGET_ADDR_LEN_NEED_RSP);
    if (Ret) {   
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 1. N9 iLm CmdAddressLenReq Command fail (Ret = %d)\n", 
                __FUNCTION__, Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }

    /* 2. Loading ilm firmware image phase */
    Ret = MtCmdFwScatters(ad, cap->FwImgBuf, ilm_dl_len);
    if (Ret) {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 2. N9 iLm MtCmdFwScatters Command fail (Ret = %d)\n",
                __FUNCTION__, Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }

    /*  DLM */
    /* standard CMD procedure */
    /* 3. Config PDA phase */
    Ret = MtCmdAddressLenReq(ad, dlm_target_addr, dlm_dl_len,
                (dlm_feature_set & FW_FEATURE_SET_ENCRY) |
                (FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(dlm_feature_set))) |
                ((dlm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
                TARGET_ADDR_LEN_NEED_RSP);
    if (Ret) {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 3. N9 dLm CmdAddressLenReq Command fail (Ret = %d)\n", 
                __FUNCTION__, Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }

    /* 4. Loading dlm firmware image phase */
    Ret = MtCmdFwScatters(ad, cap->FwImgBuf + ilm_dl_len, dlm_dl_len);
    if (Ret)
    {   
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
            ("%s: 4. N9 dLm MtCmdFwScatters Command fail (Ret = %d)\n",
            __FUNCTION__, Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }
    /* 5. N9 Firmware start negotiation phase */
    /* Ret = MtCmdFwStartReq(ad, 0, 0); */
    Ret = MtCmdFwStartReq(ad, 1, ilm_target_addr);
    if (Ret) {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 5. N9 dLm CmdFwStartReq Fail (Ret = %d)\n", 
                __FUNCTION__,Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }

#ifdef MT7615
    /* 6. download CR4. */
    cap->FwImgBuf = cap->fw_header_image_ext;
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\nCR4:\n"));
    fw_info(cap->FwImgBuf, cap->fw_len_ext, 0, &cr4_target_addr, 
                &cr4_chip_info, &cr4_feature_set, &cr4_dl_len);

    Ret = MtCmdAddressLenReq(ad, cr4_target_addr, cr4_dl_len,
                (cr4_feature_set & FW_FEATURE_SET_ENCRY) |
                (FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(cr4_feature_set))) |
                ((ilm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
                FW_FEATURE_CR4_FW_DOWNLOAD |
                TARGET_ADDR_LEN_NEED_RSP);
    if (Ret)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 6. CR4 CmdAddressLenReq Command fail (Ret = %d)\n", 
                __FUNCTION__, Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }

    /* 7. Loading cr4 firmware image phase */
    Ret = MtCmdFwScatters(ad, cap->FwImgBuf, cr4_dl_len);
    if (Ret)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 7. CR4 MtCmdFwScatters Command fail (Ret = %d)\n", 
                __FUNCTION__, Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }

    /* 8. CR4 Firmware start negotiation phase */
    Ret = MtCmdFwStartReq(ad, 0x4, 0);
    if (Ret)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s: 8. CR4 start Fail (Ret = %d)\n", __FUNCTION__, Ret));
        Ret = NDIS_STATUS_FAILURE;
        goto done;
    }
#endif /* MT7615 */

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
    /* 9. check Firmware running */
    for (loop = 0; loop < WAIT_LOOP; loop++)
    {
        value = MtAsicGetFwSyncValue(ad);
        if ((value & (BIT1 | BIT2)) == (BIT1 | BIT2))
            break;

        RtmpOsMsDelay(1);
    }

    if (loop == WAIT_LOOP)
    {
        Ret = NDIS_STATUS_FAILURE;
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("check firmware running fail (0x%x\n)", value));
        Ret = NDIS_STATUS_FAILURE;
        Ctl->Stage = FW_NO_INIT;
    }
    else
    {
        Ctl->Stage = FW_RUN_TIME;
        /*
         * PDMA0-Tx Ring2 will be used of CR4/N9 command tunnel.
         * redirect pci_kick_out)cmd_msg operation pointer to AndesMTPciKickOutCmdMsg.
         */
        ad->chipOps.pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsg;
    }
#endif

done:

    if (cap->FwImgBuf)
    {
        NICEraseFirmware(ad);
    }

    return Ret;
}


NDIS_STATUS AndesMTLoadRomMethodFwDlRing(RTMP_ADAPTER *ad)
{
    UINT32 value, loop;
    UINT32 ret = 0;
    RTMP_CHIP_CAP *cap = &ad->chipCap;
    RTMP_CHIP_OP *pChipOps = &ad->chipOps;
    struct MCU_CTRL *Ctl = &ad->MCUCtrl;
    UINT32 patch_len = 0, total_checksum = 0;

    if (cap->load_code_method == BIN_FILE_METHOD) {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("load rom patch image from /lib/firmware/%s\n", 
                            cap->rom_patch_bin_file_name));
        os_load_code_from_bin(ad, &cap->rom_patch, 
            cap->rom_patch_bin_file_name, &cap->rom_patch_len);
    } else {
        cap->rom_patch = cap->rom_patch_header_image;
    }

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
        ("%s(%d), cap->rom_patch_len(%d)\n", 
        __FUNCTION__, __LINE__, cap->rom_patch_len));

    if (!cap->rom_patch) {
        if (cap->load_code_method == BIN_FILE_METHOD) {
            MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", 
                __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
        } else {
            MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
                ("%s:Please assign a fw image, load_method(%d)\n",
                __FUNCTION__, cap->load_code_method));
        }
        ret = NDIS_STATUS_FAILURE;
        goto done;
    }

    ad->chipOps.pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsgFwDlRing;

    Ctl->Stage = ROM_PATCH_DOWNLOAD;

    value = MtAsicGetFwSyncValue(ad);
    if (BIT0 != value)
        goto done;
    
    ret = AndesRestartCheck(ad);
    if (ret != 0)
        goto done;
    

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Build Date:"));

    for (loop = 0; loop < 16; loop++)
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->rom_patch + loop)));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("platform = \n"));

    for (loop = 0; loop < 4; loop++)
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->rom_patch + 16 + loop)));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("hw/sw version = \n"));

    for (loop = 0; loop < 4; loop++)
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", *(cap->rom_patch + 20 + loop)));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("patch version = \n"));

    for (loop = 0; loop < 4; loop++)
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", *(cap->rom_patch + 24 + loop)));

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

    total_checksum = *(cap->rom_patch + 28) | (*(cap->rom_patch + 29) << 8);

    patch_len = cap->rom_patch_len - PATCH_INFO_SIZE;

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\ndownload len = %d\n", patch_len));

#ifdef MT7615 
    /* to disable filter dummy PKT action */
    ad->RxRest = 0;    
    ad->RxResetDropCount = 10;
#endif

    ret = MtCmdPatchSemGet(ad, GET_PATCH_SEM);
    if (ret)
        goto release_sem;

    if (Ctl->SemStatus == 0)
    {
        Ctl->Stage = FW_NO_INIT;
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\nPatch is not ready && get semaphore fail, SemStatus(%d)\n", Ctl->SemStatus));
        ret = 1;
        goto release_sem;
    }
    else if (Ctl->SemStatus == 1)
    {
        Ctl->Stage = FW_NO_INIT;
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\nPatch is ready, continue to ILM/DLM DL, SemStatus(%d)\n", Ctl->SemStatus));
        ret = 0;
        goto release_sem;
    }
    else if (Ctl->SemStatus == 2)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\nPatch is not ready && get semaphore success, SemStatus(%d)\n", Ctl->SemStatus));
        ret = 0;
    }
    else if (Ctl->SemStatus == 3)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\nRelease patch semaphore, SemStatus(%d), Error\n", Ctl->SemStatus));
        ret = 1;
    }
    else
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\nUnknown SemStatus(%d), Error\n", Ctl->SemStatus));
        ret = 1;
    }

    if (ret)
        goto release_sem;

    /* standard CMD procedure */
    /* 1. Config PDA phase */
    ret = MtCmdAddressLenReq(ad, cap->rom_patch_offset, patch_len, TARGET_ADDR_LEN_NEED_RSP);
    if (ret)
        goto release_sem;

    Ctl->Stage = ROM_PATCH_DOWNLOAD_SCATTER;
    /* 2. Loading rom patch image phase */
    ret = MtCmdFwScatters(ad, cap->rom_patch + PATCH_INFO_SIZE, patch_len);

    Ctl->Stage = ROM_PATCH_DOWNLOAD;

    if (ret)
        goto release_sem;

    /* 3. ROM patch start negotiation phase */
    ret = MtCmdPatchFinishReq(ad);

    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Send checksum req..\n"));

    if (pChipOps->AndesMTChkCrc &&  pChipOps->AndesMTGetCrc) {
        pChipOps->AndesMTChkCrc(ad, patch_len);

        os_msec_delay(20);

        if (total_checksum != pChipOps->AndesMTGetCrc(ad)) {
            MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
                ("checksum fail!, local(0x%x) <> fw(0x%x)\n", 
                    total_checksum, pChipOps->AndesMTGetCrc(ad)));

            ret = NDIS_STATUS_FAILURE;
        }
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("checksum=0x%x\n", pChipOps->AndesMTGetCrc(ad)));
    }

release_sem:

    ret = MtCmdPatchSemGet(ad, REL_PATCH_SEM);

    if (Ctl->SemStatus == 3)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\nRelease patch semaphore, SemStatus(%d)\n", Ctl->SemStatus));
        ret = 0;
    }
    else
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\nRelease patch semaphore, SemStatus(%d), Error\n", Ctl->SemStatus));
        ret = 1;
    }

done:
#ifdef DMA_SCH_SUPPORT
    MtAsicSetDmaByPassMode(ad,FALSE);
#endif /* DMA_SCH_SUPPORT */

    NICEraseRomPatch(ad);
    return ret;
}



#endif /* MT7615 */


INT32 AndesMTLoadFw(RTMP_ADAPTER *pAd)
{
	INT32 Ret;
	RTMP_CHIP_CAP *Cap = &pAd->chipCap;
	if (Cap->DownLoadType == DownLoadTypeA)
	{
		Ret = AndesMTLoadFwMethod1(pAd);
	}
	else if (Cap->DownLoadType == DownLoadTypeB)
	{
		Ret = AndesMTLoadFwMethod2(pAd);
	}
#ifdef MT7615
	if (Cap->DownLoadType == DownLoadTypeC)
	{		
		Ret = AndesMTLoadFwMethodFwDlRing(pAd);
	}
#endif /* MT7615 */
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s: Unknow Download type(%d)\n", 
					__FUNCTION__, Cap->DownLoadType));
		Ret = -1;
	}

	return Ret;
}



NDIS_STATUS AndesMTLoadRomPatchMath1(RTMP_ADAPTER *ad)
{
	UINT32 value, loop;
	UINT32 ret = 0;
	NTSTATUS status;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	RTMP_CHIP_OP *pChipOps = &ad->chipOps;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;
	UINT32 patch_len = 0, total_checksum = 0;

	if (cap->load_code_method == BIN_FILE_METHOD) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("load rom patch image from /lib/firmware/%s\n", cap->rom_patch_bin_file_name));
		os_load_code_from_bin(ad,&cap->rom_patch, cap->rom_patch_bin_file_name, &cap->rom_patch_len);
	} else {
		cap->rom_patch = cap->rom_patch_header_image;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d), cap->rom_patch_len(%d)\n", __FUNCTION__, __LINE__, cap->rom_patch_len));

	if (!cap->rom_patch) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Please assign a fw image, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = ROM_PATCH_DOWNLOAD;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Build Date:"));

	for (loop = 0; loop < 16; loop++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->rom_patch + loop)));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("platform = \n"));

	for (loop = 0; loop < 4; loop++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(cap->rom_patch + 16 + loop)));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("hw/sw version = \n"));

	for (loop = 0; loop < 4; loop++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", *(cap->rom_patch + 20 + loop)));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("patch version = \n"));

	for (loop = 0; loop < 4; loop++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", *(cap->rom_patch + 24 + loop)));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	total_checksum = *(cap->rom_patch + 28) | (*(cap->rom_patch + 29) << 8);

	patch_len = cap->rom_patch_len - PATCH_INFO_SIZE;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\ndownload len = %d\n", patch_len));

#ifdef DMA_SCH_SUPPORT
	MtAsicSetDmaByPassMode(ad,TRUE);
#endif /* DMA_SCH_SUPPORT */

	/* check if Ready Reg at init. state */
	value = MtAsicGetFwSyncValue(ad);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Current CheckReadyReg(%d)\n", __FUNCTION__, value));

	if (value == 0x0)
	{
		/* power on WiFi SYS*/
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 1. power on WiFi SYS\n", __FUNCTION__));
		status = MtCmdPowerOnWiFiSys(ad);

		if (status) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: failed to power on WiFi SYS\n", __FUNCTION__));
			goto done;
		}

		/* poll CheckReadyReg == 1 */
		loop = 0;
		do
		{
			value = MtAsicGetFwSyncValue(ad);
			if (value == 0x1)
				break;
			os_msec_delay(1);
			loop++;
		} while (loop <= 500);

		if (loop > 500) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: 1. CheckReadyReg is not at init. state (SW_SYN0 = %d)\n", __FUNCTION__, value));
			goto done;
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: (CheckReadyReg = %d)\n", __FUNCTION__, value));
	}

	ret = MtCmdPatchSemGet(ad, GET_PATCH_SEM);

	if (ret)
		goto done;

	if (Ctl->SemStatus == 0)
	{
		Ctl->Stage = FW_NO_INIT;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nPatch is not ready && get semaphore fail, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 1;
		goto done;
	}
	else if (Ctl->SemStatus == 1)
	{
		Ctl->Stage = FW_NO_INIT;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nPatch is ready, continue to ILM/DLM DL, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 0;
		goto done;
	}
	else if (Ctl->SemStatus == 2)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nPatch is not ready && get semaphore success, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 0;
	}
	else if (Ctl->SemStatus == 3)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRelease patch semaphore, SemStatus(%d), Error\n", Ctl->SemStatus));
		ret = 1;
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nUnknown SemStatus(%d), Error\n", Ctl->SemStatus));
		ret = 1;
	}

	if (ret)
		goto done;

	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = MtCmdAddressLenReq(ad, cap->rom_patch_offset, patch_len, TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	Ctl->Stage = ROM_PATCH_DOWNLOAD_SCATTER;
	/* 2. Loading rom patch image phase */
	ret = MtCmdFwScatters(ad, cap->rom_patch + PATCH_INFO_SIZE, patch_len);

	Ctl->Stage = ROM_PATCH_DOWNLOAD;

	if (ret)
		goto done;

	/* 3. ROM patch start negotiation phase */
	ret = MtCmdPatchFinishReq(ad);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Send checksum req..\n"));

	if(pChipOps->AndesMTChkCrc &&  pChipOps->AndesMTGetCrc)
	{
		pChipOps->AndesMTChkCrc(ad, patch_len);

		os_msec_delay(20);

		if (total_checksum != pChipOps->AndesMTGetCrc(ad)) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("checksum fail!, local(0x%x) <> fw(0x%x)\n", total_checksum,
											pChipOps->AndesMTGetCrc(ad)));

			ret = NDIS_STATUS_FAILURE;
		}
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("checksum=0x%x\n", pChipOps->AndesMTGetCrc(ad)));
	}

	ret = MtCmdPatchSemGet(ad, REL_PATCH_SEM);

	if (Ctl->SemStatus == 3)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRelease patch semaphore, SemStatus(%d)\n", Ctl->SemStatus));
		ret = 0;
	}
	else
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nRelease patch semaphore, SemStatus(%d), Error\n", Ctl->SemStatus));
		ret = 1;
	}

	if (ret)
		goto done;

done:
#ifdef DMA_SCH_SUPPORT
	MtAsicSetDmaByPassMode(ad,FALSE);
#endif /* DMA_SCH_SUPPORT */

	NICEraseRomPatch(ad);

	return ret;
}


NDIS_STATUS AndesMTLoadRomPatch(RTMP_ADAPTER *pAd)
{
	UINT32 Ret=0;
	RTMP_CHIP_CAP *Cap = &pAd->chipCap;


#ifdef MT7615
	if (Cap->DownLoadType == DownLoadTypeC)
	{
		Ret = AndesMTLoadRomMethodFwDlRing(pAd);
	}
	else
#else
	if (Cap->DownLoadType == DownLoadTypeA || Cap->DownLoadType == DownLoadTypeB)
	{
		Ret = AndesMTLoadRomPatchMath1(pAd);
	}
    	else
#endif /* MT7615 */
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s:ERROR!!!! Unknow Download type(%d)\n",
					__FUNCTION__, Cap->DownLoadType));
		Ret = -1;
	}

	return Ret;
}


INT32 AndesMTEraseFw(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = &pAd->chipCap;

	if (cap->load_code_method == BIN_FILE_METHOD) 
	{

		if (cap->FwImgBuf)
		{
			os_free_mem(cap->FwImgBuf);
		}	
		cap->FwImgBuf = NULL;
	}

	return 0;
}

INT32 AndesMTEraseRomPatch(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
									("%s\n", __FUNCTION__));

	if (cap->load_code_method == BIN_FILE_METHOD) 
	{
		if (cap->rom_patch)
		{
			os_free_mem(cap->rom_patch);
		}
		cap->rom_patch = NULL;
	}

	return 0;
}





INT AndesMtRestartFw(RTMP_ADAPTER *pAd)
{
	UINT32 ret = 0;
	
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s(%d): ========================>CmdRestartDLReq\n", 
									__FUNCTION__, __LINE__));
		ret = MtCmdRestartDLReq(pAd);
		if (ret) 
		{
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
								("%s(%d): failed to CmdRestartDLReq\n", 
										__FUNCTION__, __LINE__));
		}
	}
	return ret;
}


INT AndesMtFwdlHookInit(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	
	if (pChipCap->need_load_fw) 
	{
		pChipOps->loadFirmware = AndesMTLoadFw;
		pChipOps->eraseFirmware = AndesMTEraseFw;
#ifdef MT7615
		pChipOps->restartFirmware = AndesRestartCheck;
    #if defined(RTMP_PCI_SUPPORT)
        pChipOps->resetFirmware = AndesResetMcuViaWdt;
    #endif
#else
		pChipOps->restartFirmware = AndesMtRestartFw;
#endif /* MT7615 */
	}
	
	if (pChipCap->need_load_rom_patch) 
	{
		pChipOps->load_rom_patch = AndesMTLoadRomPatch;
		pChipOps->erase_rom_patch = AndesMTEraseRomPatch;
	}
	return NDIS_STATUS_SUCCESS;
}

