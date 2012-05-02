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
    Copyright 2001, ASUSTeK Inc.
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of ASUSTeK Inc..
*/
/*
 * BCM43XX Sonics SiliconBackplane PCMCIA core hardware definitions.
 *
 * $Id: sbpcmcia.h,v 1.1 2007/06/08 10:20:42 arthur Exp $
 * Copyright(c) 2001 ASUSTeK Inc.
 */

#ifndef	_SBPCMCIA_H
#define	_SBPCMCIA_H


/* All the addresses that are offsets in attribute space are divided
 * by two to account for the fact that odd bytes are invalid in
 * attribute space and our read/write routines make the space appear
 * as if they didn't exist. Still we want to show the original numbers
 * as documented in the hnd_pcmcia core manual.
 */

/* PCMCIA Function Configuration Registers */
#define	PCMCIA_FCR		(0x700 / 2)

#define	FCR0_OFF		0
#define	FCR1_OFF		(0x40 / 2)
#define	FCR2_OFF		(0x80 / 2)
#define	FCR3_OFF		(0xc0 / 2)

#define	PCMCIA_FCR0		(0x700 / 2)
#define	PCMCIA_FCR1		(0x740 / 2)
#define	PCMCIA_FCR2		(0x780 / 2)
#define	PCMCIA_FCR3		(0x7c0 / 2)

/* Standard PCMCIA FCR registers */

#define	PCMCIA_COR		0

#define	COR_RST			0x80
#define	COR_LEV			0x40
#define	COR_IRQEN		0x04
#define	COR_BLREN		0x01
#define	COR_FUNEN		0x01


#define	PCICIA_FCSR		(2 / 2)
#define	PCICIA_PRR		(4 / 2)
#define	PCICIA_SCR		(6 / 2)
#define	PCICIA_ESR		(8 / 2)


#define PCM_MEMOFF		0x0000
#define F0_MEMOFF		0x1000
#define F1_MEMOFF		0x2000
#define F2_MEMOFF		0x3000
#define F3_MEMOFF		0x4000

/* Memory base in the function fcr's */
#define MEM_ADDR0		(0x728 / 2)
#define MEM_ADDR1		(0x72a / 2)
#define MEM_ADDR2		(0x72c / 2)

/* PCMCIA base plus Srom access in fcr0: */
#define PCMCIA_ADDR0		(0x072e / 2)
#define PCMCIA_ADDR1		(0x0730 / 2)
#define PCMCIA_ADDR2		(0x0732 / 2)

#define MEM_SEG			(0x0734 / 2)
#define SROM_CS			(0x0736 / 2)
#define SROM_DATAL		(0x0738 / 2)
#define SROM_DATAH		(0x073a / 2)
#define SROM_ADDRL		(0x073c / 2)
#define SROM_ADDRH		(0x073e / 2)

/*  Values for srom_cs: */
#define SROM_IDLE		0
#define SROM_WRITE		1
#define SROM_READ		2
#define SROM_WEN		4
#define SROM_WDS		7
#define SROM_DONE		8

/* CIS stuff */

/* The CIS stops where the FCRs start */
#define	CIS_SIZE		PCMCIA_FCR

/* Standard tuples we know about */

#define	CISTPL_MANFID		0x20		/* Manufacturer and device id */
#define	CISTPL_FUNCE		0x22		/* Function extensions */

/* Function extensions for LANs */

#define	LAN_TECH		1		/* Technology type */
#define	LAN_SPEED		2		/* Raw bit rate */
#define	LAN_MEDIA		3		/* Transmission media */
#define	LAN_NID			4		/* Node identification (aka MAC addr) */
#define	LAN_CONN		5		/* Connector standard */

/* Vendor unique tuples are 0x80-0x8f. Within Broadcom we'll
 * take one for HNBU, and use "extensions" (a la FUNCE) within it.
 */

#define	CISTPL_BRCM_HNBU	0x80

/* Subtypes of BRCM_HNBU: */

#define	HNBU_CHIPID		0x01		/* Six bytes with PCI vendor &
						 * device id and chiprev
						 */
#define	HNBU_BOARDREV		0x02		/* Two bytes board revision */
#define	HNBU_PAPARMS		0x03		/* Eleven bytes PA parameters */
#define	HNBU_OEM		0x04		/* Eight bytes OEM data */
#define	HNBU_CC			0x05		/* Default country code */
#define	HNBU_AA			0x06		/* Antennas available */
#define	HNBU_AG			0x07		/* Antenna gain */
#define HNBU_BOARDFLAGS		0x08		/* board flags */
#define HNBU_LED		0x09		/* LED set */

#endif	/* _SBPCMCIA_H */
