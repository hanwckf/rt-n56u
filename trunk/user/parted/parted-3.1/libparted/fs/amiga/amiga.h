/*
    util.h -- amiga partition table headers.
    Copyright (C) 1998-2000, 2007, 2009-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

struct PartitionBlock {
    uint32_t	pb_ID;			/* Identifier 32 bit word : 'PART' */
    uint32_t	pb_SummedLongs;		/* Size of the structure for checksums */
    int32_t	pb_ChkSum;		/* Checksum of the structure */
    uint32_t	pb_HostID;		/* SCSI Target ID of host, not really used */
    uint32_t	pb_Next;		/* Block number of the next PartitionBlock */
    uint32_t	pb_Flags;		/* Part Flags (NOMOUNT and BOOTABLE) */
    uint32_t	pb_Reserved1[2];
    uint32_t	pb_DevFlags;		/* Preferred flags for OpenDevice */
    uint8_t	pb_DriveName[32];	/* Preferred DOS device name: BSTR form */
    uint32_t	pb_Reserved2[15];
    uint32_t	de_TableSize;		/* Size of Environment vector */
    uint32_t	de_SizeBlock;		/* Size of the blocks in 32 bit words, usually 128 */
    uint32_t	de_SecOrg;	     	/* Not used; must be 0 */
    uint32_t	de_Surfaces;		/* Number of heads (surfaces) */
    uint32_t	de_SectorPerBlock;	/* Disk sectors per block, used with SizeBlock, usually 1 */
    uint32_t	de_BlocksPerTrack;	/* Blocks per track. drive specific */
    uint32_t	de_Reserved;		/* DOS reserved blocks at start of partition. */
    uint32_t	de_PreAlloc;		/* DOS reserved blocks at end of partition */
    uint32_t	de_Interleave;		/* Not used, usually 0 */
    uint32_t	de_LowCyl;		/* First cylinder of the partition */
    uint32_t	de_HighCyl;		/* Last cylinder of the partition */
    uint32_t	de_NumBuffers;		/* Initial # DOS of buffers.  */
    uint32_t	de_BufMemType;		/* Type of mem to allocate for buffers */
    uint32_t	de_MaxTransfer;		/* Max number of bytes to transfer at a time */
    uint32_t	de_Mask;		/* Address Mask to block out certain memory */
    int32_t	de_BootPri;		/* Boot priority for autoboot */
    uint32_t	de_DosType;		/* Dostype of the file system */
    uint32_t	de_Baud;		/* Baud rate for serial handler */
    uint32_t	de_Control;		/* Control word for handler/filesystem */
    uint32_t	de_BootBlocks;		/* Number of blocks containing boot code */
    uint32_t	pb_EReserved[12];
};

#define PART(pos) ((struct PartitionBlock *)(pos))

#define	PBFB_BOOTABLE	0	/* this partition is intended to be bootable */
#define	PBFF_BOOTABLE	1L	/*   (expected directories and files exist) */
#define	PBFB_NOMOUNT	1	/* do not mount this partition (e.g. manually */
#define	PBFF_NOMOUNT	2L	/*   mounted, but space reserved here) */

struct PartitionBlock * amiga_find_part (PedGeometry *geom, struct PartitionBlock *part);

struct AmigaIds {
	uint32_t ID;
	struct AmigaIds *next;
};

struct AmigaIds * _amiga_add_id (uint32_t id, struct AmigaIds *ids);
void _amiga_free_ids (struct AmigaIds *ids);
int _amiga_id_in_list (uint32_t id, struct AmigaIds *ids) _GL_ATTRIBUTE_PURE;
