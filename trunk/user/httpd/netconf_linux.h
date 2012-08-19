/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Network configuration layer (Linux)
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: netconf_linux.h,v 1.1.1.1 2007/02/15 12:14:15 jiahao Exp $
 */

#ifndef _netconf_linux_h_
#define _netconf_linux_h_

/* Debug malloc() */
#ifdef DMALLOC
#include <dmalloc.h>
#endif /* DMALLOC */

/* iptables definitions */
#include <libiptc/libiptc.h>
#include <iptables.h>
#include <linux/types.h>
#include <net/netfilter/nf_nat.h>
#include <linux/netfilter/nf_conntrack_common.h>

//#define ETH_ALEN ETHER_ADDR_LEN

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_mac.h>
#include <linux/netfilter_ipv4/ipt_state.h>
#include <linux/netfilter_ipv4/ipt_time.h>
#include <linux/netfilter_ipv4/ipt_TCPMSS.h>
#include <linux/netfilter_ipv4/ipt_LOG.h>
#include <linux/netfilter_ipv4/ipt_autofw.h>

/* ipt_entry alignment attribute */
#define IPT_ALIGNED ((aligned (__alignof__ (struct ipt_entry))))

/* TCP flags */
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20

#define ETHER_ISNULLADDR(ea) ((((uint8_t *)(ea))[0] |		\
			    ((uint8_t *)(ea))[1] |		\
			    ((uint8_t *)(ea))[2] |		\
			    ((uint8_t *)(ea))[3] |		\
			    ((uint8_t *)(ea))[4] |		\
			    ((uint8_t *)(ea))[5]) == 0)

/* 
 * Functions that work on arrays take a pointer to the array byte
 * length. If the length is zero, the function will set the length to
 * the number of bytes that must be provided to fulfill the
 * request. If the length is non-zero, then the array will be
 * constructed until the buffer length is exhausted or the request is
 * fulfilled.
 */

/*
 * Add a firewall entry
 * @param	fw	firewall entry
 * @return	0 on success and errno on failure
 */
int netconf_add_fw(netconf_fw_t *fw);

/*
 * Delete a firewall entry
 * @param	fw	firewall entry
 * @return	0 on success and errno on failure
 */
int netconf_del_fw(netconf_fw_t *fw);

/*
 * Get a list of the current firewall entries
 * @param	fw_list		list of firewall entries
 * @return	0 on success and errno on failure
 */
int netconf_get_fw(netconf_fw_t *fw_list);

/*
 * See if a given firewall entry already exists
 * @param	nat	NAT entry to look for
 * @return	whether NAT entry exists
 */
unsigned char netconf_fw_exists(netconf_fw_t *fw);

/*
 * Reset the firewall to a sane state
 * @return 0 on success and errno on failure */
int netconf_reset_fw(void);

/*
 * Add a NAT entry or list of NAT entries
 * @param	nat_list	NAT entry or list of NAT entries
 * @return	0 on success and errno on failure
 */
int netconf_add_nat(netconf_nat_t *nat_list);

/*
 * Delete a NAT entry or list of NAT entries
 * @param	nat_list	NAT entry or list of NAT entries
 * @return	0 on success and errno on failure
 */
int netconf_del_nat(netconf_nat_t *nat_list);

/*
 * Get an array of the current NAT entries
 * @param	nat_array	array of NAT entries
 * @param	space		Pointer to size of nat_array in bytes
 * @return 0 on success and errno on failure
 */
int netconf_get_nat(netconf_nat_t *nat_array, int *space);

/*
 * Add a filter entry or list of filter entries
 * @param	filter_list	filter entry or list of filter entries
 * @return	0 on success and errno on failure
 */
int netconf_add_filter(netconf_filter_t *filter_list);

/*
 * Delete a filter entry or list of filter entries
 * @param	filter_list	filter entry or list of filter entries
 * @return	0 on success and errno on failure
 */
int netconf_del_filter(netconf_filter_t *filter_list);

/*
 * Get an array of the current filter entries
 * @param	filter_array	array of filter entries
 * @param	space		Pointer to size of filter_array in bytes
 * @return 0 on success and errno on failure
 */
int netconf_get_filter(netconf_filter_t *filter_array, int *space);


#endif /* _netconf_linux_h_ */
