/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1998-2002, 2007, 2009-2012 Free Software Foundation, Inc.

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

/* should only be #included by files in libparted */

#ifndef PED_ENDIAN_H_INCLUDED
#define PED_ENDIAN_H_INCLUDED

#include <stdint.h>

/* returns the n'th least significant byte */
#define _GET_BYTE(x, n)		( ((x) >> (8 * (n))) & 0xff )

#define _PED_SWAP16(x)		( (_GET_BYTE(x, 0) << 8)	\
				+ (_GET_BYTE(x, 1) << 0) )

#define _PED_SWAP32(x)		( (_GET_BYTE(x, 0) << 24)	\
				+ (_GET_BYTE(x, 1) << 16)	\
				+ (_GET_BYTE(x, 2) << 8)	\
				+ (_GET_BYTE(x, 3) << 0) )

#define _PED_SWAP64(x)		( (_GET_BYTE(x, 0) << 56)	\
				+ (_GET_BYTE(x, 1) << 48)	\
				+ (_GET_BYTE(x, 2) << 40)	\
				+ (_GET_BYTE(x, 3) << 32)	\
				+ (_GET_BYTE(x, 4) << 24)	\
				+ (_GET_BYTE(x, 5) << 16)	\
				+ (_GET_BYTE(x, 6) << 8)	\
				+ (_GET_BYTE(x, 7) << 0) )

#define PED_SWAP16(x)		((uint16_t) _PED_SWAP16( (uint16_t) (x) ))
#define PED_SWAP32(x)		((uint32_t) _PED_SWAP32( (uint32_t) (x) ))
#define PED_SWAP64(x)		((uint64_t) _PED_SWAP64( (uint64_t) (x) ))

#ifdef WORDS_BIGENDIAN

#define PED_CPU_TO_LE16(x)	PED_SWAP16(x)
#define PED_CPU_TO_BE16(x)	(x)
#define PED_CPU_TO_LE32(x)	PED_SWAP32(x)
#define PED_CPU_TO_BE32(x)	(x)
#define PED_CPU_TO_LE64(x)	PED_SWAP64(x)
#define PED_CPU_TO_BE64(x)	(x)

#define PED_LE16_TO_CPU(x)	PED_SWAP16(x)
#define PED_BE16_TO_CPU(x)	(x)
#define PED_LE32_TO_CPU(x)	PED_SWAP32(x)
#define PED_BE32_TO_CPU(x)	(x)
#define PED_LE64_TO_CPU(x)	PED_SWAP64(x)
#define PED_BE64_TO_CPU(x)	(x)

#else /* !WORDS_BIGENDIAN */

#define PED_CPU_TO_LE16(x)	(x)
#define PED_CPU_TO_BE16(x)	PED_SWAP16(x)
#define PED_CPU_TO_LE32(x)	(x)
#define PED_CPU_TO_BE32(x)	PED_SWAP32(x)
#define PED_CPU_TO_LE64(x)	(x)
#define PED_CPU_TO_BE64(x)	PED_SWAP64(x)

#define PED_LE16_TO_CPU(x)	(x)
#define PED_BE16_TO_CPU(x)	PED_SWAP16(x)
#define PED_LE32_TO_CPU(x)	(x)
#define PED_BE32_TO_CPU(x)	PED_SWAP32(x)
#define PED_LE64_TO_CPU(x)	(x)
#define PED_BE64_TO_CPU(x)	PED_SWAP64(x)

#endif /* !WORDS_BIGENDIAN */

#endif /* PED_ENDIAN_H_INCLUDED */
