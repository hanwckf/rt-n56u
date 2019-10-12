/*
 * random.h - NTFS random number generator wrapper.
 * Part of the Linux-NTFS project.
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
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _LINUX_NTFS_RANDOM_H
#define _LINUX_NTFS_RANDOM_H

#include <linux/random.h>

static inline long int random(void)
{
	return prandom_u32();
}

static inline void srandom(unsigned int seed)
{
	prandom_seed(seed);
}

#endif /* _LINUX_NTFS_RANDOM_H */
