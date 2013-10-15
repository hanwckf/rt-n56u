 /*
  ***************************************************************************
  * Ralink Tech Inc.
  * 4F, No. 2 Technology 5th Rd.
  * Science-based Industrial Park
  * Hsin-chu, Taiwan, R.O.C.
  *
  * (c) Copyright 2002-2005, Ralink Technology, Inc.
  *
  * All rights reserved. Ralink's source code is an unpublished work and the
  * use of a copyright notice does not imply otherwise. This source code
  * contains confidential trade secret material of Ralink Tech. Any attempt
  * or participation in deciphering, decoding, reverse engineering or in any
  * way altering the source code is stricitly prohibited, unless the prior
  * written consent of Ralink Technology, Inc. is obtained.
  ***************************************************************************
 
	 Module Name:
	 p2p_inf.h
 
	 Abstract:
 
	 Revision History:
	 Who		 When			 What
	 --------	 ---------- 	 ----------------------------------------------
 
 */

#ifndef __P2P_INF_H__
#define __P2P_INF_H__

#define	P2P_DISABLE 0x0
#define P2P_GO_UP	0x1
#define P2P_CLI_UP	0x2
/*#define P2P_GO_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_P2P_GO)) */
/*#define P2P_CLI_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_P2P_CLI)) */



#define P2P_GO_ON(_pAd) \
	(((_pAd)->flg_p2p_init) \
		&& ((_pAd)->flg_p2p_OpStatusFlags == P2P_GO_UP))


#define P2P_CLI_ON(_pAd) \
	(((_pAd)->flg_p2p_init) \
		&& ((_pAd)->flg_p2p_OpStatusFlags == P2P_CLI_UP))

 /* P2P interface hook function definition */
 VOID RTMP_P2P_Init(
		 IN PRTMP_ADAPTER ad_p,
		 IN PNET_DEV main_dev_p);
 
 INT P2P_VirtualIF_Open(
	 IN  PNET_DEV	 dev_p);
 
 INT P2P_VirtualIF_Close(
	 IN  PNET_DEV	 dev_p);
 
 INT P2P_VirtualIF_PacketSend(
	 IN PNDIS_PACKET	 skb_p, 
	 IN PNET_DEV		 dev_p);
 
 INT P2P_VirtualIF_Ioctl(
	 IN PNET_DEV			 dev_p,
	 IN OUT struct ifreq	 *rq_p,
	 IN INT cmd);
 
 VOID RTMP_P2P_Remove(
	 IN PRTMP_ADAPTER pAd);

 #endif /* __P2P_INF_H__ */

