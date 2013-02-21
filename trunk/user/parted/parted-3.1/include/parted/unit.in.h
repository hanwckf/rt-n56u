/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2005, 2007, 2009-2012 Free Software Foundation, Inc.

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

/**
 * \addtogroup PedUnit
 * @{
 */

/** \file unit.h */

#ifndef PED_UNIT_H_INCLUDED
#define PED_UNIT_H_INCLUDED

#include <parted/device.h>

#include <stdarg.h>
#include <stdio.h>

#define PED_SECTOR_SIZE_DEFAULT   512LL
#define PED_KILOBYTE_SIZE 1000LL
#define PED_MEGABYTE_SIZE 1000000LL
#define PED_GIGABYTE_SIZE 1000000000LL
#define PED_TERABYTE_SIZE 1000000000000LL
#define PED_KIBIBYTE_SIZE 1024LL
#define PED_MEBIBYTE_SIZE 1048576LL
#define PED_GIBIBYTE_SIZE 1073741824LL
#define PED_TEBIBYTE_SIZE 1099511627776LL

/**
 * Human-friendly unit for representation of a location within device
 */
typedef enum {
	PED_UNIT_SECTOR,
	PED_UNIT_BYTE,
	PED_UNIT_KILOBYTE,
	PED_UNIT_MEGABYTE,
	PED_UNIT_GIGABYTE,
	PED_UNIT_TERABYTE,
	PED_UNIT_COMPACT,
	PED_UNIT_CYLINDER,
	PED_UNIT_CHS,
	PED_UNIT_PERCENT,
	PED_UNIT_KIBIBYTE,
	PED_UNIT_MEBIBYTE,
	PED_UNIT_GIBIBYTE,
	PED_UNIT_TEBIBYTE
} PedUnit;

#define PED_UNIT_FIRST PED_UNIT_SECTOR
#define PED_UNIT_LAST PED_UNIT_TEBIBYTE

extern long long ped_unit_get_size (const PedDevice* dev, PedUnit unit);
extern const char *ped_unit_get_name (PedUnit unit)
  _GL_ATTRIBUTE_PURE _GL_ATTRIBUTE_CONST;
extern PedUnit ped_unit_get_by_name (const char* unit_name) _GL_ATTRIBUTE_PURE;

extern void ped_unit_set_default (PedUnit unit);
extern PedUnit ped_unit_get_default ();

extern char* ped_unit_format_byte (const PedDevice* dev, PedSector byte);
extern char* ped_unit_format_custom_byte (const PedDevice* dev, PedSector byte,
					  PedUnit unit);

extern char* ped_unit_format (const PedDevice* dev, PedSector sector);
extern char* ped_unit_format_custom (const PedDevice* dev, PedSector sector,
				     PedUnit unit);

extern int ped_unit_parse (const char* str, const PedDevice* dev,
                           PedSector* sector,
			   PedGeometry** range);
extern int ped_unit_parse_custom (const char* str, const PedDevice* dev,
				  PedUnit unit, PedSector* sector,
				  PedGeometry** range);

#endif /* PED_UNIT_H_INCLUDED */

/** @} */
