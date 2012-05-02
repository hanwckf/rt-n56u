/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    util.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-01-25      Initial version
*/

#ifndef _UTIL_WANTED
#define _UTIL_WANTED

#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <asm/checksum.h>
#include <linux/pci.h>
#include <linux/etherdevice.h>
#include "foe_fdb.h"
#include "common.h"

/*
 * DEFINITIONS AND MACROS
 */
#define PHYS_TO_K1(physaddr) KSEG1ADDR(physaddr)
#define RegRead(phys) (*(volatile uint32_t *)PHYS_TO_K1(phys))
#define RegWrite(phys, val)  ((*(volatile uint32_t *)PHYS_TO_K1(phys)) = (val))

/*
 * TYPEDEFS AND STRUCTURES
 */
#ifndef CONFIG_RA_HW_NAT_MINIMAL
void MacReverse(uint8_t *Mac);
void CalIpRange(uint32_t StartIp, uint32_t EndIp, uint8_t *M, uint8_t *E);
#endif
void RegModifyBits( uint32_t Addr, uint32_t Data, uint32_t  Offset, uint32_t Len);
void FoeToOrgTcpHdr(IN struct FoeEntry *foe_entry, IN struct iphdr *iph, OUT struct tcphdr *th);
void FoeToOrgUdpHdr(IN struct FoeEntry *foe_entry, IN struct iphdr *iph, OUT struct udphdr *uh);
void FoeToOrgIpHdr(IN struct FoeEntry *foe_entry, OUT struct iphdr *iph);
#ifdef HWNAT_DEBUG
uint8_t *Ip2Str(uint32_t ip);
unsigned int Str2Ip(IN char *str);
#endif
#endif
