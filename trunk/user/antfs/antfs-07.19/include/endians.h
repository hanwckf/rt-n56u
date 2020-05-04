/*
 * endians.h - Definitions related to handling of byte ordering.
 *             Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2005 Anton Altaparmakov
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _NTFS_ENDIANS_H
#define _NTFS_ENDIANS_H

/*
 * Notes:
 *	We define the conversion functions including typecasts since the
 * defaults don't necessarily perform appropriate typecasts.
 *	Also, using our own functions means that we can change them if it
 * turns out that we do need to use the unaligned access macros on
 * architectures requiring aligned memory accesses...
 */

#include <linux/kernel.h>

/* Signed from LE to CPU conversion. */

#define sle16_to_cpu(x)		((s16)__le16_to_cpu((s16)(x)))
#define sle32_to_cpu(x)		((s32)__le32_to_cpu((s32)(x)))
#define sle64_to_cpu(x)		((s64)__le64_to_cpu((s64)(x)))

#define sle16_to_cpup(x)	((s16)__le16_to_cpu(*(s16 *)(x)))
#define sle32_to_cpup(x)	((s32)__le32_to_cpu(*(s32 *)(x)))
#define sle64_to_cpup(x)	((s64)__le64_to_cpu(*(s64 *)(x)))

/* Signed from CPU to LE conversion. */

#define cpu_to_sle16(x)		((s16)__cpu_to_le16((s16)(x)))
#define cpu_to_sle32(x)		((s32)__cpu_to_le32((s32)(x)))
#define cpu_to_sle64(x)		((s64)__cpu_to_le64((s64)(x)))

#define cpu_to_sle16p(x)	((s16)__cpu_to_le16(*(s16 *)(x)))
#define cpu_to_sle32p(x)	((s32)__cpu_to_le32(*(s32 *)(x)))
#define cpu_to_sle64p(x)	((s64)__cpu_to_le64(*(s64 *)(x)))

/* Signed from BE to CPU conversion. */

#define sbe16_to_cpu(x)		((s16)__be16_to_cpu((s16)(x)))
#define sbe32_to_cpu(x)		((s32)__be32_to_cpu((s32)(x)))
#define sbe64_to_cpu(x)		((s64)__be64_to_cpu((s64)(x)))

#define sbe16_to_cpup(x)	((s16)__be16_to_cpu(*(s16 *)(x)))
#define sbe32_to_cpup(x)	((s32)__be32_to_cpu(*(s32 *)(x)))
#define sbe64_to_cpup(x)	((s64)__be64_to_cpu(*(s64 *)(x)))

/* Signed from CPU to BE conversion. */

#define cpu_to_sbe16(x)		((s16)__cpu_to_be16((s16)(x)))
#define cpu_to_sbe32(x)		((s32)__cpu_to_be32((s32)(x)))
#define cpu_to_sbe64(x)		((s64)__cpu_to_be64((s64)(x)))

#define cpu_to_sbe16p(x)	((s16)__cpu_to_be16(*(s16 *)(x)))
#define cpu_to_sbe32p(x)	((s32)__cpu_to_be32(*(s32 *)(x)))
#define cpu_to_sbe64p(x)	((s64)__cpu_to_be64(*(s64 *)(x)))

/* Constant endianness conversion defines. */

#define const_le16_to_cpu(x)	((u16) __constant_le16_to_cpu(x))
#define const_le32_to_cpu(x)	((u32) __constant_le32_to_cpu(x))
#define const_le64_to_cpu(x)	((u64) __constant_le64_to_cpu(x))

#define const_cpu_to_le16(x)	((le16) __constant_cpu_to_le16(x))
#define const_cpu_to_le32(x)	((le32) __constant_cpu_to_le32(x))
#define const_cpu_to_le64(x)	((le64) __constant_cpu_to_le64(x))

#define const_sle16_to_cpu(x)	((s16) __constant_le16_to_cpu((le16) x))
#define const_sle32_to_cpu(x)	((s32) __constant_le32_to_cpu((le32) x))
#define const_sle64_to_cpu(x)	((s64) __constant_le64_to_cpu((le64) x))

#define const_cpu_to_sle16(x)	((sle16) __constant_cpu_to_le16((u16) x))
#define const_cpu_to_sle32(x)	((sle32) __constant_cpu_to_le32((u32) x))
#define const_cpu_to_sle64(x)	((sle64) __constant_cpu_to_le64((u64) x))

#define const_be16_to_cpu(x)	((u16) __constant_be16_to_cpu(x))
#define const_be32_to_cpu(x)	((u32) __constant_be32_to_cpu(x))
#define const_be64_to_cpu(x)	((u64) __constant_be64_to_cpu(x))

#define const_cpu_to_be16(x)	((be16) __constant_cpu_to_be16(x))
#define const_cpu_to_be32(x)	((be32) __constant_cpu_to_be32(x))
#define const_cpu_to_be64(x)	((be64) __constant_cpu_to_be64(x))

#define const_sbe16_to_cpu(x)	((s16) __constant_be16_to_cpu((be16) x))
#define const_sbe32_to_cpu(x)	((s32) __constant_be32_to_cpu((be32) x))
#define const_sbe64_to_cpu(x)	((s64) __constant_be64_to_cpu((be64) x))

#define const_cpu_to_sbe16(x)	((sbe16) __constant_cpu_to_be16((u16) x))
#define const_cpu_to_sbe32(x)	((sbe32) __constant_cpu_to_be32((u32) x))
#define const_cpu_to_sbe64(x)	((sbe64) __constant_cpu_to_be64((u64) x))

#endif /* defined _NTFS_ENDIANS_H */
