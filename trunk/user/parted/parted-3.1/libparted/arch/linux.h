/* libparted - a library for manipulating disk partitions
    Copyright (C) 2009-2012 Free Software Foundation, Inc.

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

#ifndef PED_ARCH_LINUX_H_INCLUDED
#define PED_ARCH_LINUX_H_INCLUDED

#if HAVE_BLKID_BLKID_H
#  include <blkid/blkid.h>
#endif

#define LINUX_SPECIFIC(dev)	((LinuxSpecific*) (dev)->arch_specific)

typedef	struct _LinuxSpecific	LinuxSpecific;

struct _LinuxSpecific {
	int	fd;
	int	major;
	int	minor;
	char*	dmtype;         /**< device map target type */
#if defined __s390__ || defined __s390x__
	unsigned int real_sector_size;
	unsigned int devno;
#endif
#if USE_BLKID
        blkid_probe probe;
        blkid_topology topology;
#endif
};

#endif /* PED_ARCH_LINUX_H_INCLUDED */
