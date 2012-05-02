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
 * flash.h: Common definitions for flash access.
 *
 * Copyright 2001, ASUSTeK Inc.
 *
 * $Id: flashutl.h,v 1.1 2007/06/08 10:20:42 arthur Exp $
 */

/* Types of flashes we know about */
typedef enum _flash_type {OLD, BSC, SCS, AMD, SST} flash_type_t;

/* Commands to write/erase the flases */
typedef struct _flash_cmds{
	flash_type_t	type;
	bool		need_unlock;
	uint16		pre_erase;
	uint16		erase_block;
	uint16		erase_chip;
	uint16		write_word;
	uint16		write_buf;
	uint16		clear_csr;
	uint16		read_csr;
	uint16		read_id;
	uint16		confirm;
	uint16		read_array;
} flash_cmds_t;

#define	UNLOCK_CMD_WORDS	2

typedef struct _unlock_cmd {
  uint		addr[UNLOCK_CMD_WORDS];
  uint16	cmd[UNLOCK_CMD_WORDS];
} unlock_cmd_t;

/* Flash descriptors */
typedef struct _flash_desc {
	uint16		mfgid;		/* Manufacturer Id */
	uint16		devid;		/* Device Id */
	uint		size;		/* Total size in bytes */
	uint		width;		/* Device width in bytes */
	flash_type_t	type;		/* Device type old, S, J */
	uint		bsize;		/* Block size */
	uint		nb;		/* Number of blocks */
	uint		ff;		/* First full block */
	uint		lf;		/* Last full block */
	uint		nsub;		/* Number of subblocks */
	uint		*subblocks;	/* Offsets for subblocks */
	char		*desc;		/* Description */
} flash_desc_t;


#ifdef	DECLARE_FLASHES

flash_cmds_t flash_cmds[] = {
/*	  type	needu	preera	eraseb	erasech	write	wbuf	clcsr	rdcsr	rdid	confrm	read */
	{ BSC,	0,	0x00,	0x20,	0x00,	0x40,	0x00,	0x50,	0x70,	0x90,	0xd0,	0xff },
	{ SCS,	0,	0x00,	0x20,	0x00,	0x40,	0xe8,	0x50,	0x70,	0x90,	0xd0,	0xff },
	{ AMD,	1,	0x80,	0x30,	0x10,	0xa0,	0x00,	0x00,	0x00,	0x90,	0x00,	0xf0 },
	{ SST,	1,	0x80,	0x50,	0x10,	0xa0,	0x00,	0x00,	0x00,	0x90,	0x00,	0xf0 },
	{ 0 }
};

unlock_cmd_t unlock_cmd_amd = {
#ifdef MIPSEB
/* addr: */	{ 0x0aa8,	0x0556},
#else
/* addr: */	{ 0x0aaa,	0x0554},
#endif
/* data: */	{ 0xaa,		0x55}
};

unlock_cmd_t unlock_cmd_sst = {
#ifdef MIPSEB
/* addr: */	{ 0xaaa8,	0x5556},
#else
/* addr: */	{ 0xaaaa,	0x5554},
#endif
/* data: */	{ 0xaa,		0x55}
};

#define AMD_CMD 0xaaa
#define SST_CMD 0xaaaa

/* intel unlock block cmds */
#define INTEL_UNLOCK1	0x60
#define INTEL_UNLOCK2	0xD0

/* Just eight blocks of 8KB byte each */

uint blk8x8k[] = { 0x00000000,
		   0x00002000,
		   0x00004000,
		   0x00006000,
		   0x00008000,
		   0x0000a000,
		   0x0000c000,
		   0x0000e000,
		   0x00010000
};

/* Funky AMD arrangement for 29xx800's */
uint amd800[] = { 0x00000000,		/* 16KB */
		  0x00004000,		/* 32KB */
		  0x0000c000,		/* 8KB */
		  0x0000e000,		/* 8KB */
		  0x00010000,		/* 8KB */
		  0x00012000,		/* 8KB */
		  0x00014000,		/* 32KB */
		  0x0001c000,		/* 16KB */
		  0x00020000
};

/* AMD arrangement for 29xx160's */
uint amd4112[] = { 0x00000000,		/* 32KB */
		   0x00008000,		/* 8KB */
		   0x0000a000,		/* 8KB */
		   0x0000c000,		/* 16KB */
		   0x00010000
};
uint amd2114[] = { 0x00000000,		/* 16KB */
		   0x00004000,		/* 8KB */
		   0x00006000,		/* 8KB */
		   0x00008000,		/* 32KB */
		   0x00010000
};



flash_desc_t flashes[] = {
	{ 0x00b0, 0x00d0, 0x0200000, 2,	SCS, 0x10000, 32,  0, 31,  0, NULL,    "Intel 28F160S3/5 1Mx16" },
	{ 0x00b0, 0x00d4, 0x0400000, 2,	SCS, 0x10000, 64,  0, 63,  0, NULL,    "Intel 28F320S3/5 2Mx16" },
	{ 0x0089, 0x8890, 0x0200000, 2,	BSC, 0x10000, 32,  0, 30,  8, blk8x8k, "Intel 28F160B3 1Mx16 TopB" },
	{ 0x0089, 0x8891, 0x0200000, 2,	BSC, 0x10000, 32,  1, 31,  8, blk8x8k, "Intel 28F160B3 1Mx16 BotB" },
	{ 0x0089, 0x8896, 0x0400000, 2,	BSC, 0x10000, 64,  0, 62,  8, blk8x8k, "Intel 28F320B3 2Mx16 TopB" },
	{ 0x0089, 0x8897, 0x0400000, 2,	BSC, 0x10000, 64,  1, 63,  8, blk8x8k, "Intel 28F320B3 2Mx16 BotB" },
	{ 0x0089, 0x8898, 0x0800000, 2,	BSC, 0x10000, 128, 0, 126, 8, blk8x8k, "Intel 28F640B3 4Mx16 TopB" },
	{ 0x0089, 0x8899, 0x0800000, 2,	BSC, 0x10000, 128, 1, 127, 8, blk8x8k, "Intel 28F640B3 4Mx16 BotB" },
	{ 0x0089, 0x88C2, 0x0200000, 2,	BSC, 0x10000, 32,  0, 30,  8, blk8x8k, "Intel 28F160C3 1Mx16 TopB" },
	{ 0x0089, 0x88C3, 0x0200000, 2,	BSC, 0x10000, 32,  1, 31,  8, blk8x8k, "Intel 28F160C3 1Mx16 BotB" },
	{ 0x0089, 0x88C4, 0x0400000, 2,	BSC, 0x10000, 64,  0, 62,  8, blk8x8k, "Intel 28F320C3 2Mx16 TopB" },
	{ 0x0089, 0x88C5, 0x0400000, 2,	BSC, 0x10000, 64,  1, 63,  8, blk8x8k, "Intel 28F320C3 2Mx16 BotB" },
	{ 0x0089, 0x88CC, 0x0800000, 2,	BSC, 0x10000, 128, 0, 126, 8, blk8x8k, "Intel 28F640C3 4Mx16 TopB" },
	{ 0x0089, 0x88CD, 0x0800000, 2,	BSC, 0x10000, 128, 1, 127, 8, blk8x8k, "Intel 28F640C3 4Mx16 BotB" },
	{ 0x0089, 0x0014, 0x0400000, 2,	SCS, 0x20000, 32,  0, 31,  0, NULL,    "Intel 28F320J5 2Mx16" },
	{ 0x0089, 0x0015, 0x0800000, 2,	SCS, 0x20000, 64,  0, 63,  0, NULL,    "Intel 28F640J5 4Mx16" },
	{ 0x0089, 0x0016, 0x0400000, 2,	SCS, 0x20000, 32,  0, 31,  0, NULL,    "Intel 28F320J3 2Mx16" },
	{ 0x0089, 0x0017, 0x0800000, 2,	SCS, 0x20000, 64,  0, 63,  0, NULL,    "Intel 28F640J3 4Mx16" },
	{ 0x0089, 0x0018, 0x1000000, 2,	SCS, 0x20000, 128, 0, 127, 0, NULL,    "Intel 28F128J3 8Mx16" },
	{ 0x00b0, 0x00e3, 0x0400000, 2,	BSC, 0x10000, 64,  1, 63,  8, blk8x8k, "Sharp 28F320BJE 2Mx16 BotB" },
	{ 0x0001, 0x224a, 0x0100000, 2,	AMD, 0x10000, 16,  0, 13,  8, amd800,  "AMD 29DL800BT 512Kx16 TopB" },
	{ 0x0001, 0x22cb, 0x0100000, 2,	AMD, 0x10000, 16,  2, 15,  8, amd800,  "AMD 29DL800BB 512Kx16 BotB" },
	{ 0x0001, 0x22c4, 0x0200000, 2,	AMD, 0x10000, 32,  0, 30,  4, amd2114, "AMD 29lv160DT 1Mx16 TopB" },
	{ 0x0001, 0x2249, 0x0200000, 2,	AMD, 0x10000, 32,  1, 31,  4, amd4112, "AMD 29lv160DB 1Mx16 BotB" },
	{ 0x0001, 0x22f6, 0x0400000, 2,	AMD, 0x10000, 64,  0, 62,  8, blk8x8k, "AMD 29lv320DT 2Mx16 TopB" },
	{ 0x0001, 0x22f9, 0x0400000, 2,	AMD, 0x10000, 64,  1, 63,  8, blk8x8k, "AMD 29lv320DB 2Mx16 BotB" },
	{ 0x0020, 0x22CA, 0x0400000, 2,	AMD, 0x10000, 64,  0, 62,  4, amd4112, "ST 29w320DT 2Mx16 TopB" },
	{ 0x0020, 0x22CB, 0x0400000, 2,	AMD, 0x10000, 64,  1, 63,  4, amd2114, "ST 29w320DB 2Mx16 BotB" },
	{ 0x00C2, 0x00A7, 0x0400000, 2,	AMD, 0x10000, 64,  0, 62,  4, amd4112, "MX29LV320T 2Mx16 TopB" },
	{ 0x00C2, 0x00A8, 0x0400000, 2,	AMD, 0x10000, 64,  1, 63,  4, amd2114, "MX29LV320B 2Mx16 BotB" },
	{ 0x0004, 0x22F6, 0x0400000, 2,	AMD, 0x10000, 64,  0, 62,  4, amd4112, "MBM29LV320TE 2Mx16 TopB" },
	{ 0x0004, 0x22F9, 0x0400000, 2,	AMD, 0x10000, 64,  1, 63,  4, amd2114, "MBM29LV320BE 2Mx16 BotB" },
	{ 0x0098, 0x009A, 0x0400000, 2,	AMD, 0x10000, 64,  0, 62,  4, amd4112, "TC58FVT321 2Mx16 TopB" },
	{ 0x0098, 0x009C, 0x0400000, 2,	AMD, 0x10000, 64,  1, 63,  4, amd2114, "TC58FVB321 2Mx16 BotB" }, 
	{ 0x00C2, 0x22A7, 0x0400000, 2,	AMD, 0x10000, 64,  0, 62,  4, amd4112, "MX29LV320T 2Mx16 TopB" },
	{ 0x00C2, 0x22A8, 0x0400000, 2,	AMD, 0x10000, 64,  1, 63,  4, amd2114, "MX29LV320B 2Mx16 BotB" },
	{ 0x00BF, 0x2783, 0x0400000, 2,	SST, 0x10000, 64,  0, 63,  0, NULL,    "SST39VF320 2Mx16" },
	{ 0,      0,      0,         0,	OLD, 0,       0,   0, 0,   0, NULL,    NULL },
};

#else

extern flash_cmds_t flash_cmds[];
extern unlock_cmd_t unlock_cmd;
extern flash_desc_t flashes[];

#endif
