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

#include <config.h>
#include "architecture.h"

const PedArchitecture* ped_architecture;

void
ped_set_architecture ()
{
	/* Set just once */
	if (ped_architecture)
		return;

#ifdef linux
	extern PedArchitecture ped_linux_arch;
	const PedArchitecture* arch = &ped_linux_arch;
#elif defined(__BEOS__)
	extern PedArchitecture ped_beos_arch;
	const PedArchitecture* arch = &ped_beos_arch;
#else
	extern PedArchitecture ped_gnu_arch;
	const PedArchitecture* arch = &ped_gnu_arch;
#endif

	ped_architecture = arch;
}
