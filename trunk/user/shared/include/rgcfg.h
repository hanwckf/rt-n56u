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
/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* 
* FILENAME:    $Workfile: rgcfg.h $ 
* REVISION:    $Revision: 1.1 $ 
* LAST UPDATE: $Modtime: 12/25/02 5:48p $ 
* 
* rgcfg.h
*
* DESCRIPTION:
*       Header file for the rgcfg application
*
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef _RGCFG_H_
#define _RGCFG_H_

#ifdef REMOVE
#include <msApi.h>
#include <msIoctl.h>
#include <mv_platform.h>

/*
typedef enum {
  false =0,
  true
} bool;

typedef void VOID;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef unsigned int DWORD;
typedef char WCHAR;
typedef char TCHAR;
*/
#endif

/* mfg data constants */
#define SZ_PHY_ADDR         6   /* Number of bytes in ethernet MAC address */
#define SZ_BOARD_NAME       8
#define SZ_BOOT_VERSION     12
#define SZ_PRODUCT_ID       58
#define SZ_INTERNAL_PA_CFG  14
#define SZ_EXTERNAL_PA_CFG  1
#define SZ_CCA_CFG          8
#define SZ_LED              4
#define SZ_DEVICE_NAME      16
#define SZ_SSID             32

/* Manufature data structure */
struct mvwlan_mfg_param {
	unsigned char board_descr[SZ_BOARD_NAME];
	unsigned char revision;
	unsigned char pa_options;
	unsigned char ext_pa[SZ_EXTERNAL_PA_CFG];
	unsigned char antenna_cfg;
	unsigned short int_pa[SZ_INTERNAL_PA_CFG];
	unsigned char cca[SZ_CCA_CFG];
	unsigned short domain;
	unsigned short customer_options;
	unsigned char led[SZ_LED];
	unsigned short xosc;
	unsigned char reserved_1[2];
	unsigned short magic;
	unsigned short check_sum;
	unsigned char mfg_mac_addr[SZ_PHY_ADDR];
	unsigned char reserved_2[4];
	unsigned char pid[SZ_PRODUCT_ID];
	unsigned char boot_version[SZ_BOOT_VERSION];
};

#endif   /* _RGCFG_H_ */

