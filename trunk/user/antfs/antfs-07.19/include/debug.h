/*
 * debug.h - Debugging output functions. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2002-2004 Anton Altaparmakov
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

#ifndef _NTFS_DEBUG_H
#define _NTFS_DEBUG_H

#include "antfs.h"
#include "attrib.h"

struct runlist_element;
extern const char *ntfs_lcn_str[5];

#if ANTFS_LOGLEVEL >= ANTFS_LOGLEVEL_DBG
/**
 * ntfs_debug_runlist_dump - Dump a runlist.
 * @rl:
 *
 * Description...
 *
 * Returns:
 */
static inline void ntfs_debug_runlist_dump(const struct runlist_element *rl)
{
	int i = 0;

	antfs_log_debug("NTFS-fs DEBUG: Dumping runlist (values in hex):");
	if (!rl) {
		antfs_log_debug("Run list not present.");
		return;
	}
	antfs_log_debug("VCN              LCN               Run length");
	do {
		LCN lcn = (rl + i)->lcn;

		if (lcn < (LCN) 0) {
			int idx = -lcn - 1;

			if (idx > -LCN_EINVAL - 1)
				idx = 4;
			antfs_log_debug("%-16lld %s %-16lld%s",
					(long long)rl[i].vcn, ntfs_lcn_str[idx],
					(long long)rl[i].length,
					rl[i].length ? "" : " (runlist end)");
		} else
			antfs_log_debug("%-16lld %-16lld  %-16lld%s",
					(long long)rl[i].vcn,
					(long long)rl[i].lcn,
					(long long)rl[i].length,
					rl[i].length ? "" : " (runlist end)");
	} while (rl[i++].length);
}
#else
static inline void ntfs_debug_runlist_dump(const struct runlist_element *rl
					   __attribute__ ((unused)))
{
};
#endif

#if ANTFS_LOGLEVEL >= ANTFS_LOGLEVEL_ERR
/**
 * ntfs_debug_runlist_dump - Dump a runlist.
 * @rl:
 *
 * Description...
 *
 * Returns:
 */
static inline void ntfs_err_runlist_dump(const struct runlist_element *rl)
{
	int i = 0;

	antfs_log_error("NTFS-fs DEBUG: Dumping runlist (values in hex):");
	if (!rl) {
		antfs_log_error("Run list not present.");
		return;
	}
	antfs_log_error("VCN              LCN               Run length");
	do {
		LCN lcn = (rl + i)->lcn;

		if (lcn < (LCN) 0) {
			int idx = -lcn - 1;

			if (idx > -LCN_EINVAL - 1)
				idx = 4;
			antfs_log_error("%-16lld %s %-16lld%s",
					(long long)rl[i].vcn, ntfs_lcn_str[idx],
					(long long)rl[i].length,
					rl[i].length ? "" : " (runlist end)");
		} else
			antfs_log_error("%-16lld %-16lld  %-16lld%s",
					(long long)rl[i].vcn,
					(long long)rl[i].lcn,
					(long long)rl[i].length,
					rl[i].length ? "" : " (runlist end)");
	} while (rl[i++].length);
}
#else
static inline void ntfs_err_runlist_dump(const struct runlist_element *rl
					   __attribute__ ((unused)))
{
};
#endif

#define NTFS_BUG(msg)							\
do {									\
	antfs_log_critical("Bug: %s\n", msg);				\
	BUG();								\
} while (0)

#endif /* defined _NTFS_DEBUG_H */
