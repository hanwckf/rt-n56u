/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	ral_omac_usb.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RAL_OMAC_USB_H__
#define __RAL_OMAC_USB_H__


#define USB_DMA_CFG 0x02a0
#ifdef RT_BIG_ENDIAN
typedef	union _USB_DMA_CFG_STRUC {
	struct {
		UINT32 TxBusy:1;   	/*USB DMA TX FSM busy. debug only */
		UINT32 RxBusy:1;        /*USB DMA RX FSM busy. debug only */
		UINT32 EpoutValid:6;        /*OUT endpoint data valid. debug only */
		UINT32 TxBulkEn:1;        /*Enable USB DMA Tx */
		UINT32 RxBulkEn:1;        /*Enable USB DMA Rx */
		UINT32 RxBulkAggEn:1;        /*Enable Rx Bulk Aggregation */
		UINT32 TxopHalt:1;        /*Halt TXOP count down when TX buffer is full. */
		UINT32 TxClear:1;        /*Clear USB DMA TX path */
		UINT32 rsv:2;        
		UINT32 phyclear:1;        		/*phy watch dog enable. write 1 */
		UINT32 RxBulkAggLmt:8;        /*Rx Bulk Aggregation Limit  in unit of 1024 bytes */
		UINT32 RxBulkAggTOut:8;        /*Rx Bulk Aggregation TimeOut  in unit of 33ns */
	} field;
	UINT32 word;
} USB_DMA_CFG_STRUC, *PUSB_DMA_CFG_STRUC;
#else
typedef	union _USB_DMA_CFG_STRUC {
	struct {
		UINT32 RxBulkAggTOut:8;        /*Rx Bulk Aggregation TimeOut  in unit of 33ns */
		UINT32 RxBulkAggLmt:8;        /*Rx Bulk Aggregation Limit  in unit of 256 bytes */
		UINT32 phyclear:1;        		/*phy watch dog enable. write 1 */
		UINT32 rsv:2;
		UINT32 TxClear:1;        /*Clear USB DMA TX path */
		UINT32 TxopHalt:1;        /*Halt TXOP count down when TX buffer is full. */
		UINT32 RxBulkAggEn:1;        /*Enable Rx Bulk Aggregation */
		UINT32 RxBulkEn:1;        /*Enable USB DMA Rx */
		UINT32 TxBulkEn:1;        /*Enable USB DMA Tx */
		UINT32 EpoutValid:6;        /*OUT endpoint data valid */
		UINT32 RxBusy:1;        /*USB DMA RX FSM busy */
		UINT32 TxBusy:1;   	/*USB DMA TX FSM busy */
	} field;
	UINT32 word;
} USB_DMA_CFG_STRUC, *PUSB_DMA_CFG_STRUC;
#endif

#endif /*__RAL_OMAC_USB_H__ */

