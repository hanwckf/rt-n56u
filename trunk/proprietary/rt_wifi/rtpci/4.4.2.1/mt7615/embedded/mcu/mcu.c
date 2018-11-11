/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_mcu.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

INT MCUBurstWrite(PRTMP_ADAPTER pAd, UINT32 Offset, UINT32 *Data, UINT32 Cnt)
{
	return 0;
}


INT MCURandomWrite(PRTMP_ADAPTER pAd, RTMP_REG_PAIR *RegPair, UINT32 Num)
{
	UINT32 Index;
	
	for (Index = 0; Index < Num; Index++)
		RTMP_IO_WRITE32(pAd, RegPair->Register, RegPair->Value);

	return 0;
}


INT32 MCUSysPrepare(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	MCU_CTRL_INIT(pAd);
#ifdef MT_MAC
	if (pAd->chipOps.FwInit && (pAd->chipCap.hif_type == HIF_MT))
		pAd->chipOps.FwInit(pAd);
	
#endif /*MT_MAC*/

	return Ret;
}


INT32 MCULoadRomPatch(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC
#if defined(MT7636) || defined(MT7637) || defined(MT7615) || defined(MT7622)
	if (IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7615(pAd) || IS_MT7622(pAd)) {
		if (load_patch(pAd) != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("load patch failed!\n"));
			return FALSE;
		}	
	}
#endif /* MT7636 || MT7637 || MT7615 || MT7622 */
#endif /* MT_MAC*/
	return TRUE;
}



INT32 MCUSysInit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

#ifdef INTERNAL_CAPTURE_SUPPORT    
	UINT32 Value;
#endif /* INTERNAL_CAPTURE_SUPPORT */

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	
	MCU_CTRL_INIT(pAd);
#ifdef MT_MAC
	if (pAd->chipOps.FwInit && (pAd->chipCap.hif_type == HIF_MT))
		pAd->chipOps.FwInit(pAd);
	

#endif /* MT_MAC */

#ifdef INTERNAL_CAPTURE_SUPPORT//AKai	
	/* Refer to profile setting to decide the sysram partition format */
    MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
	    ("%s: Before NICLoadFirmware, check IcapMode=%d\n", __FUNCTION__, pAd->IcapMode));

    if (pAd->IcapMode == 2)/* Wifi-spectrum */
    {
        HW_IO_READ32(pAd,CONFG_COM_REG3, &Value);

        Value = Value | CONFG_COM_REG3_FWOPMODE;

        HW_IO_WRITE32(pAd,CONFG_COM_REG3, Value);
        
    }
    else
    {
        HW_IO_READ32(pAd,CONFG_COM_REG3, &Value);

        Value = Value & (~CONFG_COM_REG3_FWOPMODE);

        HW_IO_WRITE32(pAd,CONFG_COM_REG3, Value);      
    }
#endif /* INTERNAL_CAPTURE_SUPPORT */

	Ret = NICLoadFirmware(pAd);
	
	if (Ret != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: NICLoadFirmware failed, Status[=0x%08x]\n", __FUNCTION__, Ret));
		return -1;
	}
#ifdef INTERNAL_CAPTURE_SUPPORT	
	/* Refer to profile setting to decide the sysram partition format */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: NICLoadFirmware OK, Check IcapMode=%d\n", __FUNCTION__, pAd->IcapMode));
	if (pAd->IcapMode == 1) /* Internal capture */
	{
		MtCmdRfTestSwitchMode(pAd, OPERATION_ICAP_MODE, 0, 
								RF_TEST_DEFAULT_RESP_LEN);
	}
#endif /* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}


INT32 MCUSysExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

	if (pAd->chipOps.FwExit)
		pAd->chipOps.FwExit(pAd);

	MCU_CTRL_EXIT(pAd);

	return Ret;
}


VOID ChipOpsMCUHook(PRTMP_ADAPTER pAd, enum MCU_TYPE MCUType)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	printk("%s\n",__FUNCTION__);

#ifdef CONFIG_ANDES_SUPPORT
	if ((MCUType & ANDES) == ANDES) 
	{
		RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) 
		{
#ifdef RTMP_PCI_SUPPORT
			if (pChipCap->need_load_rom_patch) 
			{
				pChipOps->load_rom_patch = AndesRltPciLoadRomPatch;
				pChipOps->erase_rom_patch = AndesRltPciEraseRomPatch;
			}

			if (pChipCap->need_load_fw) 
			{
				pChipOps->loadFirmware = AndesRltPciLoadFw;
				pChipOps->eraseFirmware = AndesRltPciEraseFw;
				pChipOps->pci_kick_out_cmd_msg = AndesRltPciKickOutCmdMsg;
			}

			pChipOps->FwInit = AndesRltPciFwInit;
#endif /* RTMP_PCI_SUPPORT */
			if (pChipCap->need_load_fw) 
			{
				//pChipOps->sendCommandToMcu = andes_send_cmd_msg;
				pChipOps->Calibration = AndesRltCalibration;
				pChipOps->BurstWrite =  AndesRltBurstWrite;
				pChipOps->BurstRead = AndesRltBurstRead;
				pChipOps->RandomRead = AndesRltRandomRead;
				pChipOps->RFRandomRead = AndesRltRfRandomRead;
				pChipOps->ReadModifyWrite = AndesRltReadModifyWrite;
				pChipOps->RFReadModifyWrite = AndesRltRfReadModifyWrite;
				pChipOps->RandomWrite = AndesRltRandomWrite;
				pChipOps->RFRandomWrite = AndesRltRfRandomWrite;
#ifdef CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT
				pChipOps->BBPRandomWrite = AndesBbpRandomWrite;
#endif /* CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT */
				pChipOps->sc_random_write = AndesRltScRandomWrite;
				pChipOps->sc_rf_random_write = AndesRltScRfRandomWrite;
				pChipOps->PwrSavingOP = AndesRltPwrSaving;
				pChipOps->andes_fill_cmd_header = AndesRltFillCmdHeader;
			}
		}
#endif /* RLT_MAC */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
			if (IS_PCIE_INF(pAd) || IS_RBUS_INF(pAd))
			{
				pChipOps->FwInit = AndesMTPciFwInit;
				pChipOps->FwExit = AndesMTPciFwExit;

				if (pChipCap->need_load_fw) {
#if defined(MT7615) || defined(MT7622)
					if (IS_MT7615(pAd) || IS_MT7622(pAd))
						pChipOps->pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsgFwDlRing;
					else
#endif
						pChipOps->pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsg;
				}
			}
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */


			if (pChipCap->need_load_fw) {
				pChipOps->loadFirmware = AndesMTLoadFw;
				pChipOps->eraseFirmware = AndesMTEraseFw;
			}

			if (pChipCap->need_load_rom_patch) {
				pChipOps->load_rom_patch = AndesMTLoadRomPatch;
				pChipOps->erase_rom_patch = AndesMTEraseRomPatch;
			}

			FwdlHookInit(pAd);
				
			pChipOps->andes_fill_cmd_header = AndesMTFillCmdHeader;
			pChipOps->rx_event_handler = AndesMTRxEventHandler;
		}
#endif /* MT_MAC */

		pChipOps->MtCmdTx = AndesSendCmdMsg;
		pChipOps->MCUCtrlInit = AndesCtrlInit;
		pChipOps->MCUCtrlExit = AndesCtrlExit;
	}
#endif /* CONFIG_ANDES_SUPPORT */
}

