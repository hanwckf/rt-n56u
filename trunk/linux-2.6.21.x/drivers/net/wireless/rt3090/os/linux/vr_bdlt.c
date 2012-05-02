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
    vr_brlt.c
 
    Abstract:
    Only for BroadLight 2348 platform.

	For fast path function.
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Sample Lin	01-12-2010    Created

 */

#define MODULE_BDLT

#include "rt_config.h"


#ifdef PLATFORM_BL2348

/* global variables */
int (*pToUpperLayerPktSent)(struct sk_buff *pSkb) = netif_rx ;




/*
========================================================================
Routine Description:
	Assign the briding function.

Arguments:
	xi_destination_ptr	- bridging function

Return Value:
	None

Note:
	The function name must be replace_upper_layer_packet_destination.
========================================================================
*/
VOID replace_upper_layer_packet_destination(VOID *pXiDestination)
{
	DBGPRINT(RT_DEBUG_TRACE, ("ralink broad light> replace_upper_layer_packet_destination\n"));
	pToUpperLayerPktSent = pXiDestination ;
} /* End of replace_upper_layer_packet_destination */


EXPORT_SYMBOL(replace_upper_layer_packet_destination);

#endif // PLATFORM_BL2348 //


/* End of vr_bdlt.c */
