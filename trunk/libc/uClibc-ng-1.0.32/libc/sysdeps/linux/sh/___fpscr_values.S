/* Startup code for SH & ELF.
   Copyright (C) 1999 Free Software Foundation, Inc.
   Copyright (C) 2001 Hewlett-Packard Australia
   Copyright (C) 2002 Stefan Allius

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Library General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option) any
 later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 details.

 You should have received a copy of the GNU Library General Public License
 along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <features.h>

.data
#ifdef __SH4__
.global ___fpscr_values
.type ___fpscr_values,@object
.size ___fpscr_values,8
___fpscr_values:
	.long   0
	.long   0x80000

.weak __fpscr_values
	__fpscr_values = ___fpscr_values
#endif
