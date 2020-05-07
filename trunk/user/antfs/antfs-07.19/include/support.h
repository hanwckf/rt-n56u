/*
 * support.h - Useful definitions and macros.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2004 Anton Altaparmakov
 * Copyright (c) 2016 Jens Krieg
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

#ifndef _NTFS_SUPPORT_H
#define _NTFS_SUPPORT_H

#include <linux/bitops.h>

/*
 * Our mailing list. Use this define to prevent typos in email address.
 */
#define NTFS_DEV_LIST	"jkrieg@avm.de"

/*
 * Generic macro to convert pointers to values for comparison purposes.
 */
#ifndef p2n
#define p2n(p)		((ptrdiff_t)((ptrdiff_t *)(p)))
#endif

#endif /* defined _NTFS_SUPPORT_H */
