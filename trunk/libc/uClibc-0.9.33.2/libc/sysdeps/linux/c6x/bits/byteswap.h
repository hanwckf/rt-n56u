/*
 * Port of uClibc for TMS320C6000 DSP architecture
 * Copyright (C) 2004 Texas Instruments Incorporated
 * Author of TMS320C6000 port: Aurelien Jacquiot
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

#if !defined _BYTESWAP_H && !defined _NETINET_IN_H
# error "Never use <bits/byteswap.h> directly; include <byteswap.h> instead."
#endif

#ifdef __GNUC__
#define __bswap_non_constant_32(x) __builtin_bswap32(x)
#endif

#include <bits/byteswap-common.h>

#endif
