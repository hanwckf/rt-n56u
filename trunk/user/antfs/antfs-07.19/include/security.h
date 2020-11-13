/*
 * security.h - Exports for handling security/ACLs in NTFS.
 *              Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2004      Anton Altaparmakov
 * Copyright (c) 2005-2006 Szabolcs Szakacsits
 * Copyright (c) 2007-2010 Jean-Pierre Andre
 * Copyright (C) 2019      AVM GmbH
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

#ifndef _NTFS_SECURITY_H
#define _NTFS_SECURITY_H

#include "types.h"
#include "layout.h"
#include "inode.h"
#include "dir.h"
#include "endians.h"

extern int ntfs_sd_add_everyone(struct ntfs_inode *ni);
#endif /* defined _NTFS_SECURITY_H */
