 /*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2007, 2009-2012 Free Software Foundation, Inc.

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

/*
 * WARNING: This shouldn't be exported to the API
 */

#ifndef _LIBPARTED_ARCH_H_INCLUDED
#define _LIBPARTED_ARCH_H_INCLUDED

#include <parted/disk.h>

struct _PedArchitecture {
	PedDiskArchOps*		disk_ops;
	PedDeviceArchOps*	dev_ops;
};
typedef struct _PedArchitecture PedArchitecture;

extern const PedArchitecture*	ped_architecture;

extern void ped_set_architecture ();

#endif /* _LIBPARTED_ARCH_H_INCLUDED */
