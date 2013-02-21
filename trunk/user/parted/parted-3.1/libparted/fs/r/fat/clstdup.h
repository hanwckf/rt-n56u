/*
    libparted
    Copyright (C) 1999, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef PED_FAT_CLSTDUP_H_INCLUDED
#define PED_FAT_CLSTDUP_H_INCLUDED

#include "context.h"

/* the big important one :-) */
extern int fat_duplicate_clusters (FatOpContext* ctx, PedTimer* timer);

#endif /* PED_FAT_CLSTDUP_H_INCLUDED */
