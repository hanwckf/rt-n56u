/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All related IEEE802.11r Key Distribution Protocol (FT KDP) body.

	Overview:

	1. A station associates to us, send out a broadcast ADD-Notify packet.

		ASSOC -->
		FT_KDP_EVENT_INFORM(FT_KDP_SIG_FT_ASSOCIATION) -->
		FT_KDP_EventInform(FT_KDP_SIG_FT_ASSOCIATION) -->
		Notify IAPP daemon, IAPP_RcvHandlerRawDrv(), IAPP_SIG_Process() -->
		Send ADD-Notify packet, IAPP_UDP_PacketSend(), IAPP_L2UpdateFrameSend()

	2. When receiving a ADD-Notify packet, send a unicast SSB packet to
		request PMK-R1 key for the station with our R0KH.

		IAPP daemon, IAPP_RcvHandlerUdp() -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		FT_KDP_StationInform() -->
		Notify IAPP daemon, FT_KDP_EventInform(FT_KDP_SIG_KEY_REQ_AUTO) -->
		Notify IAPP daemon, IAPP_RcvHandlerRawDrv(), IAPP_SIG_Process() -->
		Send SSB packet with R0KHID = 0, by using TCP or UDP based on peerIP

	3. A station reassociates to us, send out a MOVE-Request packet.

		REASSOC -->
		FT_KDP_EVENT_INFORM(FT_KDP_SIG_FT_REASSOCIATION) -->
		FT_KDP_EventInform(FT_KDP_SIG_FT_REASSOCIATION) -->
		Notify IAPP daemon, IAPP_RcvHandlerRawDrv(), IAPP_SIG_Process() -->
		Send MOVE-Request packet by using TCP or UDP, IAPP_L2UpdateFrameSend()

	4. When receiving a MOVE-Request packet, delete the STA MAC entry.

		IAPP daemon, IAPP_RcvHandlerUdp()/ IAPP_RcvHandlerTcp() -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		RT_SET_DEL_MAC_ENTRY -->
		Send MOVE-Response packet by using TCP, FT_KDP_MoveFrameSend()

	5. When receiving a MOVE-Response packet, nothing to do.

	6. When receiving a SSB packet (i.e. key request), send a unicast SAB
		packet to response the key to the R1KH.

		IAPP daemon -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		FT_KDP_IOCTL_KEY_REQ() -->
		Notify IAPP daemon, FT_KDP_EventInform(FT_KDP_SIG_KEY_RSP_AUTO) -->
		Send SAB packet with my R0KHID, FT_KDP_SecurityBlockSend() by using TCP

	7. When receiving a SAB packet (i.e. key response), set the PMK-R1 key.

		IAPP daemon -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		FT_KDP_KeyResponseToUs()

	8. Send a information broadcast to the LAN periodically.

***************************************************************************/

