/*
    libparted
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

#ifndef PED_FAT_BOOTSECTOR_H
#define PED_FAT_BOOTSECTOR_H

typedef struct _FatBootSector	FatBootSector;
typedef struct _FatInfoSector	FatInfoSector;

#include "fat.h"

#define FAT32_INFO_MAGIC1	0x41615252
#define FAT32_INFO_MAGIC2	0x61417272
#define FAT32_INFO_MAGIC3	0xaa55

/* stolen from mkdosfs, by Dave Hudson */

#define FAT_BOOT_MESSAGE	\
"This partition does not have an operating system loader installed on it.\n\r"\
"Press a key to reboot..."

#define FAT_BOOT_JUMP	"\xeb\x58\x90"		/* jmp	+5a */

#define FAT_BOOT_CODE	"\x0e"			/* push cs */		\
			"\x1f"			/* pop ds */		\
			"\xbe\x74\x7e"		/* mov si, offset message */ \
					/* write_msg_loop: */		\
			"\xac"			/* lodsb */		\
			"\x22\xc0"		/* and al, al */	\
			"\x74\x06"		/* jz done (+8) */	\
			"\xb4\x0e"		/* mov ah, 0x0e */	\
			"\xcd\x10"		/* int 0x10 */		\
			"\xeb\xf5"		/* jmp write_msg_loop */ \
					/* done: */			\
			"\xb4\x00"		/* mov ah, 0x00 */	\
			"\xcd\x16"		/* int 0x16 */		\
			"\xb4\x00"		/* mov ah, 0x00 */	\
			"\xcd\x19"		/* int 0x19 */		\
			"\xeb\xfe"		/* jmp +0 - in case int 0x19 */ \
						/* doesn't work */	\
					/* message: */			\
			FAT_BOOT_MESSAGE

#define FAT_BOOT_CODE_LENGTH 128

struct __attribute__ ((packed)) _FatBootSector {
        uint8_t		boot_jump[3];	/* 00: Boot strap short or near jump */
        uint8_t		system_id[8];	/* 03: system name */
        uint16_t	sector_size;	/* 0b: bytes per logical sector */
        uint8_t		cluster_size;	/* 0d: sectors/cluster */
        uint16_t	reserved;	/* 0e: reserved sectors */
        uint8_t		fats;		/* 10: number of FATs */
        uint16_t	dir_entries;	/* 11: number of root directory entries */
        uint16_t	sectors;	/* 13: if 0, total_sect supersedes */
        uint8_t		media;		/* 15: media code */
        uint16_t	fat_length;	/* 16: sectors/FAT for FAT12/16 */
        uint16_t	secs_track;	/* 18: sectors per track */
        uint16_t	heads;		/* 1a: number of heads */
        uint32_t	hidden;		/* 1c: hidden sectors (partition start) */
        uint32_t	sector_count;	/* 20: no. of sectors (if sectors == 0) */

        union __attribute__ ((packed)) {
                /* FAT16 fields */
                struct __attribute__ ((packed)) {
                        uint8_t		drive_num;	/* 24: */
                        uint8_t		empty_1;	/* 25: */
                        uint8_t		ext_signature;	/* 26: always 0x29 */
                        uint32_t	serial_number;	/* 27: */
                        uint8_t		volume_name [11];       /* 2b: */
                        uint8_t		fat_name [8];	/* 36: */
                        uint8_t		boot_code[448];	/* 3f: Boot code (or message) */
                } fat16;
                /* FAT32 fields */
                struct __attribute__ ((packed)) {
                        uint32_t	fat_length;	/* 24: size of FAT in sectors */
                        uint16_t	flags;		/* 28: bit8: fat mirroring, low4: active fat */
                        uint16_t	version;        /* 2a: minor * 256 + major */
                        uint32_t	root_dir_cluster;	/* 2c: */
                        uint16_t	info_sector;    /* 30: */
                        uint16_t	backup_sector;	/* 32: */
                        uint8_t		empty_1 [12];	/* 34: */
                        uint16_t	drive_num;	/* 40: */
                        uint8_t		ext_signature;	/* 42: always 0x29 */
                        uint32_t	serial_number;	/* 43: */
                        uint8_t		volume_name [11];	/* 47: */
                        uint8_t		fat_name [8];	/* 52: */
                        uint8_t		boot_code[420];	/* 5a: Boot code (or message) */
                } fat32;
        } u;

	uint16_t	boot_sign;	/* 1fe: always 0xAA55 */
};

struct __attribute__ ((packed)) _FatInfoSector {
        uint32_t	signature_1;	/* should be 0x41615252 */
        uint8_t		unused [480];
        uint32_t	signature_2;	/* should be 0x61417272 */
        uint32_t	free_clusters;
        uint32_t	next_cluster;	/* most recently allocated cluster */
        uint8_t		unused2 [0xe];
        uint16_t	signature_3;	/* should be 0xaa55 */
};

int fat_boot_sector_read (FatBootSector* bs, const PedGeometry* geom);
FatType fat_boot_sector_probe_type (const FatBootSector* bs,
				    const PedGeometry* geom);
int fat_boot_sector_analyse (FatBootSector* bs, PedFileSystem* fs);
int fat_boot_sector_set_boot_code (FatBootSector* bs);
int fat_boot_sector_generate (FatBootSector* bs, const PedFileSystem* fs);
int fat_boot_sector_write (const FatBootSector* bs, PedFileSystem* fs);

int fat_info_sector_read (FatInfoSector* is, const PedFileSystem* fs);
int fat_info_sector_generate (FatInfoSector* is, const PedFileSystem* fs);
int fat_info_sector_write (const FatInfoSector* is, PedFileSystem* fs);

#endif /* PED_FAT_BOOTSECTOR_H */
