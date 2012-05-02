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
/* This is the userspace/kernel interface for Generic IP Chains,
   required for libc6. */
#ifndef _FWCHAINS_KERNEL_HEADERS_H
#define _FWCHAINS_KERNEL_HEADERS_H

#include <limits.h>

#if defined(__GLIBC__) && __GLIBC__ == 2
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/types.h>
#else /* libc5 */
#include <sys/socket.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/icmp.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/types.h>
#include <linux/in6.h>
#endif
#endif
