/*
 * param.h - Parameter values for ntfs-3g
 *
 * Copyright (c) 2009-2010 Jean-Pierre Andre
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

#ifndef _NTFS_PARAM_H
#define _NTFS_PARAM_H

/*
 *		Parameters for compression
 */

	/* default option for compression */
#define DEFAULT_COMPRESSION 1
	/* (log2 of) number of clusters in a compression block for new files */
#define STANDARD_COMPRESSION_UNIT 4
	/* maximum cluster size for allowing compression for new files */
#define MAX_COMPRESSION_CLUSTER_SIZE 4096

/*
 *		Parameters for runlists
 */

	/* only update the final extent of a runlist when appending data */
#define PARTIAL_RUNLIST_UPDATING 1

/*
 *		Parameters for upper-case table
 */

	/* Create upper-case tables as defined by Windows 6.1 (Win7) */
#define UPCASE_MAJOR 6
#define UPCASE_MINOR 1

#endif /* defined _NTFS_PARAM_H */
