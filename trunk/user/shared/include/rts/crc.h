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
 * $Id: crc.h,v 1.1.1.1 2007/01/25 12:52:12 jiahao_jhou Exp $
 * Copyright 2006, ASUSTeK Inc.      
 * All Rights Reserved.      
 *       
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
 * crc.h - a function to compute crc for iLine10 headers
 ******************************************************************************/

#ifndef _RTS_CRC_H_
#define _RTS_CRC_H_ 1

#include <nvram/typedefs.h>

#ifdef __cplusplus
extern "C" {
#endif


#define CRC8_INIT_VALUE  0xff       /* Initial CRC8 checksum value */
#define CRC8_GOOD_VALUE  0x9f       /* Good final CRC8 checksum value */
#define HCS_GOOD_VALUE   0x39       /* Good final header checksum value */

#define CRC16_INIT_VALUE 0xffff     /* Initial CRC16 checksum value */
#define CRC16_GOOD_VALUE 0xf0b8     /* Good final CRC16 checksum value */

#define CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define CRC32_GOOD_VALUE 0xdebb20e3 /* Good final CRC32 checksum value */

void   hcs(uint8 *, uint);
uint8  crc8(uint8 *, uint, uint8);
uint16 crc16(uint8 *, uint, uint16);
uint32 crc32(uint8 *, uint, uint32);

/* macros for common usage */

#define APPEND_CRC8(pbytes, nbytes)                           \
do {                                                          \
    uint8 tmp = crc8(pbytes, nbytes, CRC8_INIT_VALUE) ^ 0xff; \
    (pbytes)[(nbytes)] = tmp;                                 \
    (nbytes) += 1;                                            \
} while (0)

#define APPEND_CRC16(pbytes, nbytes)                               \
do {                                                               \
    uint16 tmp = crc16(pbytes, nbytes, CRC16_INIT_VALUE) ^ 0xffff; \
    (pbytes)[(nbytes) + 0] = (tmp >> 0) & 0xff;                    \
    (pbytes)[(nbytes) + 1] = (tmp >> 8) & 0xff;                    \
    (nbytes) += 2;                                                 \
} while (0)

#define APPEND_CRC32(pbytes, nbytes)                                   \
do {                                                                   \
    uint32 tmp = crc32(pbytes, nbytes, CRC32_INIT_VALUE) ^ 0xffffffff; \
    (pbytes)[(nbytes) + 0] = (tmp >>  0) & 0xff;                       \
    (pbytes)[(nbytes) + 1] = (tmp >>  8) & 0xff;                       \
    (pbytes)[(nbytes) + 2] = (tmp >> 16) & 0xff;                       \
    (pbytes)[(nbytes) + 3] = (tmp >> 24) & 0xff;                       \
    (nbytes) += 4;                                                     \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* _RTS_CRC_H_ */
