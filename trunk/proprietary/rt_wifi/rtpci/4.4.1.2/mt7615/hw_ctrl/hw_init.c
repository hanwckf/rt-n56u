#ifdef MTK_LICENSE
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
	hw_init.c
*/
#endif /* MTK_LICENSE */
#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "hw_init.tmh"
#endif
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif



/*Local function*/
#ifdef RTMP_MAC_SDIO
#if CFG_SDIO_BIST
static INT mt_sdio_bist(RTMP_ADAPTER *pAd)
{
	INT32 bistlen = 1536*20;
	INT32 u4Ret = 0;
	INT32 u4idx = 0;
	INT32 u4bist = 0;
	INT32 bistlimit = 50;
	INT32 prbslimit = 100;
	UCHAR *pBuf;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, 1536*21);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n\n============ BIST Start ============\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): PRBS pattern seed=37\n", __FUNCTION__));
	HIF_IO_WRITE32(pAd, WTMCR, 0x00370002);

	//u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
	//for(u4idx=0;u4idx<200;u4idx++){
	//	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %x", *(pBuf+u4idx)));
	//}
	for(u4bist=0;u4bist<prbslimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	for(u4bist=0;u4bist<prbslimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	for(u4bist=0;u4bist<prbslimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	//-----------------------------------------------------------------------------
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): PRBS pattern seed=ec\n", __FUNCTION__));
	HIF_IO_WRITE32(pAd, WTMCR, 0x00ec0002);

	//u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
	//for(u4idx=0;u4idx<200;u4idx++){
	//	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %x", *(pBuf+u4idx)));
	//}
	for(u4bist=0;u4bist<prbslimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	for(u4bist=0;u4bist<prbslimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	for(u4bist=0;u4bist<prbslimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}

	//-----------------------------------------------------------------------------
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): 0x5a 0x5a 0x5a 0x5a pattern\n", __FUNCTION__));
	HIF_IO_WRITE32(pAd, WTMCR, 0x00000000);
	HIF_IO_WRITE32(pAd, WTMDPCR0, 0x5A5A5A5A);

	for(u4bist=0;u4bist<bistlimit;u4bist++){
	u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
	if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}

	for(u4idx=0;u4idx<bistlen;u4idx++){
		if(*(pBuf+u4idx)!=0x5A){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error ====> %x\n", *(pBuf+u4idx)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
	}
	}
	for(u4bist=0;u4bist<bistlimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	//-----------------------------------------------------------------------------
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): 0xf0 0xf0 0xf0 0xf0 pattern\n", __FUNCTION__));
	HIF_IO_WRITE32(pAd, WTMDPCR0, 0xF0F0F0F0);
	for(u4bist=0;u4bist<bistlimit;u4bist++){
	u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
	if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}

	for(u4idx=0;u4idx<bistlen;u4idx++){
		if(*(pBuf+u4idx)!=0xf0){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error ====> %x\n", *(pBuf+u4idx)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
	}
	}
	for(u4bist=0;u4bist<bistlimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	//-----------------------------------------------------------------------------

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): 21 43 65 87 pattern\n", __FUNCTION__));
	HIF_IO_WRITE32(pAd, WTMDPCR0, 0x87654321);

	for(u4bist=0;u4bist<bistlimit;u4bist++){
	u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
	if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}

	for(u4idx=0;u4idx<bistlen;u4idx=u4idx+4){
		if(*(pBuf+u4idx+0)!=0x21){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=21 read= %x\n", *(pBuf+u4idx+0)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
		if(*(pBuf+u4idx+1)!=0x43){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=43 read= %x\n", *(pBuf+u4idx+1)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
		if(*(pBuf+u4idx+2)!=0x65){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=65 read= %x\n", *(pBuf+u4idx+2)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
		if(*(pBuf+u4idx+3)!=0x87){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=87 read= %x\n", *(pBuf+u4idx+3)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
	}
	}
	for(u4bist=0;u4bist<bistlimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	//-----------------------------------------------------------------------------
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): 36 76 10 00 pattern\n", __FUNCTION__));
	HIF_IO_WRITE32(pAd, WTMDPCR0, 0x00107636);

	for(u4bist=0;u4bist<bistlimit;u4bist++){
	u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
	if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}

	for(u4idx=0;u4idx<bistlen;u4idx=u4idx+4){
		if(*(pBuf+u4idx+0)!=0x36){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=36 read= %x\n", *(pBuf+u4idx+0)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
		if(*(pBuf+u4idx+1)!=0x76){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=76 read= %x\n", *(pBuf+u4idx+1)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
		if(*(pBuf+u4idx+2)!=0x10){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=10 read= %x\n", *(pBuf+u4idx+2)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
		if(*(pBuf+u4idx+3)!=0x00){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error exp=00 read= %x\n", *(pBuf+u4idx+3)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
	}
	}
	for(u4bist=0;u4bist<bistlimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	//-----------------------------------------------------------------------------

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): 0x01 0x01 0x01 0x01 pattern\n", __FUNCTION__));
	HIF_IO_WRITE32(pAd, WTMDPCR0, 0x01010101);

	for(u4bist=0;u4bist<bistlimit;u4bist++){
	u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
	if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}

	for(u4idx=0;u4idx<bistlen;u4idx++){
		if(*(pBuf+u4idx)!=0x1){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" error ====> %x\n", *(pBuf+u4idx)));
			u4idx=bistlen;
			u4bist=bistlimit;
		}
	}
	}
	for(u4bist=0;u4bist<bistlimit;u4bist++){
		u4Ret = MTSDIOMultiRead(pAd, WTMDR, pBuf, bistlen);
		if (u4Ret != 0){MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): error MTSDIOMultiRead u4Ret=%x\n", __FUNCTION__, u4Ret));}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============ BIST End ============\n\n\n"));
	os_free_mem(pBuf);
	return NDIS_STATUS_SUCCESS;
}
#endif

#if CFG_SDIO_DRIVING_TUNE
static INT mt_sdio_driving_tune(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

            //===== driving
	HIF_IO_READ32(pAd, CMDIOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): CMDIOCR 1: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_WRITE32(pAd, CMDIOCR, 0x00000022);
	HIF_IO_READ32(pAd, CMDIOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): CMDIOCR 2: Value:%x\n", __FUNCTION__, Value));

	HIF_IO_READ32(pAd, DAT0IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT0IOCR 1: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_WRITE32(pAd, DAT0IOCR, 0x00000022);
	HIF_IO_READ32(pAd, DAT0IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT0IOCR 2: Value:%x\n", __FUNCTION__, Value));

	HIF_IO_READ32(pAd, DAT1IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT1IOCR 1: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_WRITE32(pAd, DAT1IOCR, 0x00000022);
	HIF_IO_READ32(pAd, DAT1IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT1IOCR 2: Value:%x\n", __FUNCTION__, Value));

	HIF_IO_READ32(pAd, DAT2IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT2IOCR 1: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_WRITE32(pAd, DAT2IOCR, 0x00000022);
	HIF_IO_READ32(pAd, DAT2IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT2IOCR 2: Value:%x\n", __FUNCTION__, Value));

	HIF_IO_READ32(pAd, DAT3IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT3IOCR 1: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_WRITE32(pAd, DAT3IOCR, 0x0000002a);
	HIF_IO_READ32(pAd, DAT3IOCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DAT3IOCR 2: Value:%x\n", __FUNCTION__, Value));

	//===== DAT output delay
	HIF_IO_READ32(pAd, ODATDLYCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ODATDLYCR 1: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_WRITE32(pAd, ODATDLYCR, 0x00000000);
	HIF_IO_READ32(pAd, ODATDLYCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ODATDLYCR 2: Value:%x\n", __FUNCTION__, Value));

	//===== cmdR output delay
	HIF_IO_READ32(pAd, CMDDLYCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): CMDDLYCR 1: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_WRITE32(pAd, CMDDLYCR, 0x00ff0000);
	HIF_IO_READ32(pAd, CMDDLYCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): CMDDLYCR 2: Value:%x\n", __FUNCTION__, Value));
}
#endif


static INT mt_hif_sys_sdio_init(RTMP_ADAPTER *pAd)
{
	UINT32 Value;
	UINT32 counter=0;

	HIF_IO_WRITE32(pAd, WHLPCR, W_INT_EN_CLR);
	HIF_IO_READ32(pAd, WHLPCR, &Value);

	HIF_IO_READ32(pAd, WCIR, &Value);

	if(GET_POR_INDICATOR(Value)) {// POR
		HIF_IO_WRITE32(pAd, WCIR, POR_INDICATOR);
		HIF_IO_READ32(pAd, WCIR, &Value);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MCR_WCIR: Value:%x\n", __FUNCTION__, Value));
	}
	os_msec_delay(100);

	//		HIF_IO_WRITE32(pAd, WHIER, 0x0);
	HIF_IO_WRITE32(pAd, WHLPCR, W_INT_EN_CLR);

	//Poll W_FUNC for FW own back
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Request FW-Own back\n",__FUNCTION__));

	HIF_IO_READ32(pAd, WHLPCR, &Value);
	HIF_IO_WRITE32(pAd, WHLPCR, W_FW_OWN_REQ_CLR);
	while(!GET_W_FW_OWN_REQ_SET(Value)) {
		HIF_IO_READ32(pAd, WHLPCR, &Value);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Request FW-Own processing: %x\n",__FUNCTION__,Value));
		counter++;
		os_msec_delay(50);
		if(counter >100){
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:  FW-Own back Faiure\n",__FUNCTION__));
			break;
		}
	}

	HIF_IO_WRITE32(pAd, WHLPCR, W_INT_EN_CLR);
	HIF_IO_READ32(pAd, WHLPCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MCR_WHLPCR: Value:%x\n", __FUNCTION__, Value));

	HIF_IO_WRITE32(pAd, WHIER, 0x46);
	HIF_IO_READ32(pAd, WASR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): WASR: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_READ32(pAd, WHIER, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MCR_WHIER: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_READ32(pAd, WHISR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MCR_WHISR: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_READ32(pAd, WASR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): WASR: Value:%x\n", __FUNCTION__, Value));
	HIF_IO_READ32(pAd, WCIR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): MCR_WCIR: Value:%x\n", __FUNCTION__, Value));

	HIF_IO_READ32(pAd, WHCR, &Value);

#if CFG_SDIO_RX_ENHANCE
	Value |= RX_ENHANCE_MODE;
#else
	Value &= ~RX_ENHANCE_MODE;
#endif /* CFG_SDIO_RX_AGG */

	HIF_IO_WRITE32(pAd, WHCR, Value);
	HIF_IO_READ32(pAd, WHCR, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s(): ==================>WHCR= %x\n", __FUNCTION__,Value));
	HIF_IO_READ32(pAd, WHIER, &Value);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s(): ==================>WHIER= %x\n", __FUNCTION__,Value));

#if(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 0)
	SDIO_CFG_MAX_HIF_RX_LEN_NUM(pAd, 1);

#elif(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 1)
	SDIO_CFG_MAX_HIF_RX_LEN_NUM(pAd, 16);

#elif (CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1)
	SDIO_CFG_MAX_HIF_RX_LEN_NUM(pAd, 16);
#endif

#if CFG_SDIO_DRIVING_TUNE
	mt_sdio_driving_tune(pAd);
#endif

#if CFG_SDIO_BIST
	mt_sdio_bist(pAd);
#endif

    return 0;
}
#endif /*RTMP_MAC_SDIO*/




#ifdef RTMP_MAC_PCI
static INT mt_hif_sys_pci_init(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		rlt_asic_init_txrx_ring(pAd);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		rtmp_asic_init_txrx_ring(pAd);
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT){
		mt_asic_init_txrx_ring(pAd);
		if (pAd->chipOps.hif_set_pcie_read_params
			#ifdef ERR_RECOVERY
			&& (IsErrRecoveryInIdleStat(pAd) == FALSE)
			#endif /* ERR_RECOVERY*/
			)
			pAd->chipOps.hif_set_pcie_read_params(pAd);
	}
#endif /* MT_MAC */

	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &mac_val);
	//mac_val |= 0xb0; // bit 7/5~4 => 1
	if(IS_MT7637(pAd)){
		mac_val = 0x52001055; //workaround PDMA issue for WHQA_00022606
	}
	else if(IS_MT7615(pAd) || IS_MT7622(pAd)){
		mac_val = 0x10001870;
	}
	else{
		mac_val = 0x52000850;
	}
	HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, mac_val);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef RTMP_MAC_PCI
#ifndef COMPOS_TESTMODE_WIN
#if defined(MT7615) 
static void patch_for_reload_fw_issue(RTMP_ADAPTER *pAd)
{
    UINT32 value = 0;
    
    /* do pdma0 hw reset(bit 24) */
    HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &value);
    value |= SW_RST;
    HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, value);

}
#endif
#endif
#endif /* RTMP_MAC_PCI */


static INT mt_hif_sys_init(RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo)
{

#ifdef RTMP_MAC_PCI
#ifndef COMPOS_TESTMODE_WIN
	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
	{
#if defined(MT7615)   
	    patch_for_reload_fw_issue(pAd);
#endif
        mt_hif_sys_pci_init(pAd);
	}
#endif
#endif /* RTMP_MAC_PCI */


#ifdef RTMP_MAC_SDIO
	if (IS_SDIO_INF(pAd))
	{
		mt_hif_sys_sdio_init(pAd);
	}
#endif

	return NDIS_STATUS_SUCCESS;
}



/*HW related init*/

INT32 WfHifHwInit(RTMP_ADAPTER *pAd,HIF_INFO_T *pHifInfo)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		ret = rt_hif_sys_init(pAd);
	}
#endif /*  defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		ret = mt_hif_sys_init(pAd,pHifInfo);
	}
#endif /* MT_MAC */
	return ret;
}

static INT32 WfTopHwInit(RTMP_ADAPTER *pAd)
{
	
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP ||pAd->chipCap.hif_type == HIF_RLT)
		return RtAsicTOPInit(pAd);
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		return MtAsicTOPInit(pAd);
#endif	

	return FALSE;
}

static INT32 WfMcuHwInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
    
#ifdef INTERNAL_CAPTURE_SUPPORT    
	UINT32 Value;
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef COMPOS_WIN
//#elif defined (COMPOS_TESTMODE_WIN)
#else

	if (pAd->chipOps.fw_prepare)
	{
		pAd->chipOps.fw_prepare(pAd);
	}
    #ifdef MT7615
        #if defined(RTMP_PCI_SUPPORT)
retry_dl_fw:
        #endif 
    #endif 
	ret = NICLoadRomPatch(pAd);
	if (ret != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
            ("%s: NICLoadRomPatch failed, Status[=0x%08x]\n", __FUNCTION__, ret));
    #ifdef MT7615
        #if defined(RTMP_PCI_SUPPORT)
        if (pAd->chipOps.resetFirmware)
        {
            pAd->chipOps.resetFirmware(pAd);
        }
        goto retry_dl_fw;
        #endif        
    #else
        return NDIS_STATUS_FAILURE;
    #endif
    }
#endif

#ifdef INTERNAL_CAPTURE_SUPPORT	
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
    
	ret = NICLoadFirmware(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
            ("%s: NICLoadFirmware failed, Status[=0x%08x]\n", __FUNCTION__, ret));
#ifdef MT7615
    #if defined(RTMP_PCI_SUPPORT)
        if (pAd->chipOps.resetFirmware)
        {
            pAd->chipOps.resetFirmware(pAd);
        }
        goto retry_dl_fw;
    #endif  
#else 
		return NDIS_STATUS_FAILURE;
#endif
	}

	/*After fw download should disalbe dma schedule bypass mode*/
#ifdef DMA_SCH_SUPPORT
	if(AsicWaitPDMAIdle(pAd, 100, 1000) != TRUE)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			ret =  NDIS_STATUS_FAILURE;
			return ret;
		}
	}

	AsicDMASchedulerInit(pAd, DMA_SCH_LMAC);
#endif

#ifdef INTERNAL_CAPTURE_SUPPORT	
	/* Refer to profile setting to decide the sysram partition format */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
	    ("%s: NICLoadFirmware OK, Check IcapMode=%d\n", __FUNCTION__, pAd->IcapMode));
	if (pAd->IcapMode == 1) /* Internal capture */
	{
		MtCmdRfTestSwitchMode(pAd, OPERATION_ICAP_MODE, 0, 
								RF_TEST_DEFAULT_RESP_LEN);
	}	
#endif /* INTERNAL_CAPTURE_SUPPORT */

	return ret;
}


static INT32 WfEPROMHwInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#if defined(COMPOS_WIN)  || defined (COMPOS_TESTMODE_WIN)
#else
	NICInitAsicFromEEPROM(pAd);
#endif
	return ret;
}


/*Common Part for externl*/

INT32 WfTopInit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;
	
	if(WfTopHwInit(pAd)!=TRUE)
	{
		ret = NDIS_STATUS_FAILURE;
	}
	
	return ret;

}

INT32 WfHifInit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;
	HIF_INFO_T hifInfo;

	os_zero_mem(&hifInfo,sizeof(HIF_INFO_T));
	
	if((ret = WfHifSysInit(pAd,&hifInfo))!=NDIS_STATUS_SUCCESS)
	{
		goto err;
	}

	WfHifHwInit(pAd,&hifInfo);

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Success!\n", __FUNCTION__));
	return 0;
err:
	WfHifSysExit(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Err! status=%d\n", __FUNCTION__, ret));
	return ret;

}

INT32 WfMcuInit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;
	
	if((ret = WfMcuSysInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err;
	}

#ifdef MTK_UART_SUPPORT  
/* UART not support load firmware */
// Do nohting
#else
	if((ret = WfMcuHwInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err;
	}
#endif

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Success!\n", __FUNCTION__));
	return ret;
err:
	WfMcuSysExit(pAd);	
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Err! status=%d\n", __FUNCTION__, ret));

	return ret;
}

INT32 WfMacInit(RTMP_ADAPTER *pAd)
{
	UINT32 ret = NDIS_STATUS_SUCCESS;
#if defined(COMPOS_TESTMODE_WIN) && defined(RTMP_SDIO_SUPPORT)
	//todo:  This function cause 7637 FPGA SDIO load FW failed, need to check. 
	return ret;
#endif

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		ret = rtmp_nic_asic_init(pAd);
	}
#endif /*  defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		ret = mt_nic_asic_init(pAd);
	}
#endif /* MT_MAC */

	/* Clear raw counters*/
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)
	NicResetRawCounters(pAd);
#endif

	return ret;
}

INT32 WfEPROMInit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;

//#if  defined(COMPOS_WIN) || defined (COMPOS_TESTMODE_WIN)
#if  defined (COMPOS_TESTMODE_WIN)
#else
	if ((ret = WfEPROMSysInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err;
	}

	WfEPROMHwInit(pAd);
	
	return ret;
err:
	WfEPROMSysExit(pAd);
#endif
	return ret;
}

INT32 WfPhyInit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;
#if  defined(COMPOS_WIN)  || defined (COMPOS_TESTMODE_WIN)	
#else
	NICInitBBP(pAd);
#endif
	return ret;
}

INT32 WfInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;	

	if((ret = WfTopInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err0;
	}
	
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Top Init Done!\n"));
	
	if((ret = WfHifInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err0;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Hif Init Done!\n"));

	if((ret = WfMcuInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err1;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCU Init Done!\n"));
	
#ifdef RLM_CAL_CACHE_SUPPORT
	rlmCalCacheApply(pAd, pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */
	
	/*Adjust eeprom + config => apply to HW*/
	if((ret = WfEPROMInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err2;
	}

#ifdef SINGLE_SKU_V2
    /* Load SKU table to Host Driver */
    RTMPSetSingleSKUParameters(pAd);
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
    /* Load BF Backoff table to Host Driver */
    RTMPSetBfBackOffParameters(pAd);
#endif /* defined(MT_MAC) && defined(TXBF_SUPPORT) */
#endif /* SINGLE_SKU_V2 */
	
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EEPROM Init Done!\n"));

	if((ret = WfMacInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err3;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC Init Done!\n"));

	if((ret = WfPhyInit(pAd))!=NDIS_STATUS_SUCCESS)
	{
		goto err3;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PHY Init Done!\n"));

	return ret;
#if  defined(COMPOS_WIN)  || defined (COMPOS_TESTMODE_WIN)
err3:
err2:
err1:
err0:
#else
err3:
	WfEPROMSysExit(pAd);
err2:
	WfMcuSysExit(pAd);
err1:
	WfHifSysExit(pAd);
err0:

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): initial faild!! ret=%d\n",__FUNCTION__, ret));

#endif
	return ret;
}


/*SW related init*/

INT32 WfSysPreInit(RTMP_ADAPTER *pAd)
{











#ifdef MT7615
	if (IS_MT7615(pAd)) {
		mt7615_init(pAd);
	}
#endif /* MT7615 */


#ifdef MT7622
	if (IS_MT7622(pAd)) {
		mt7622_init(pAd);
	}
#endif /* MT7622 */

	/* We depends on RfICType and MACVersion to assign the corresponding operation callbacks. */


	return 0;
}


INT32 WfSysPosExit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;

#if  defined(COMPOS_WIN)  || defined (COMPOS_TESTMODE_WIN)
#else
	WfEPROMSysExit(pAd);
	WfMcuSysExit(pAd);	
	WfHifSysExit(pAd);
#endif

	return ret;
}

INT32 WfSysCfgInit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;
	return ret;

}


INT32 WfSysCfgExit(RTMP_ADAPTER *pAd)
{
	INT32 ret=NDIS_STATUS_SUCCESS;
	return ret;

}
