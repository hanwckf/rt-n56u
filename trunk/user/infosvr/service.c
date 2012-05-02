/*
 * Miscellaneous services
 *
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.		
 *				     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.			    
 *
 * $Id: service.c,v 1.1.1.1 2008/07/21 09:17:45 james26_jang Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>

#define IFUP (IFF_UP|IFF_RUNNING|IFF_BROADCAST|IFF_MULTICAST)

#ifdef WCLIENT
int is_dhcpd_exist(void)
{
	// todo
	return 1;
}

int
start_dhcpd(void)
{
	// todo
	return 0;
}

int
stop_dhcpd(void)
{
	// todo
	return 0;
}
#endif

