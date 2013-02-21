/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2000, 2007-2012 Free Software Foundation, Inc.

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
 * \addtogroup PedAlignment
 * @{
 */

/** \file natmath.h */

#ifndef PED_NATMATH_H_INCLUDED
#define PED_NATMATH_H_INCLUDED


typedef struct _PedAlignment	PedAlignment;

#include <parted/disk.h>
#include <parted/device.h>
#include <parted/geom.h>

#define PED_MIN(a, b)	( ((a)<(b)) ? (a) : (b) )
#define PED_MAX(a, b)	( ((a)>(b)) ? (a) : (b) )

/* this is weird (I'm still not sure I should be doing this!)
 *
 * For the functions: new, destroy, duplicate and merge: the following values
 * for align are valid:
 * 	* align == NULL  (!)  		represents no solution
 * 	* align->grain_size == 0	represents a single solution
 * 					(align->offset)
 * 	* align->grain_size > 0		represents a set of solutions
 *
 * These are invalid:
 * 	* align->offset < 0		Note: this gets "normalized"
 * 	* align->grain_size < 0
 *
 * For the align_* operations, there must be a solution.  i.e. align != NULL
 * All solutions must be greater than zero.
 */

struct _PedAlignment {
	PedSector	offset;
	PedSector	grain_size;
};

extern PedSector ped_round_up_to (PedSector sector, PedSector grain_size)
  _GL_ATTRIBUTE_CONST;
extern PedSector ped_round_down_to (PedSector sector, PedSector grain_size)
  _GL_ATTRIBUTE_CONST;
extern PedSector ped_round_to_nearest (PedSector sector, PedSector grain_size)
  _GL_ATTRIBUTE_CONST;
extern PedSector ped_greatest_common_divisor (PedSector a, PedSector b)
  _GL_ATTRIBUTE_PURE;

extern int ped_alignment_init (PedAlignment* align, PedSector offset,
			       PedSector grain_size);
extern PedAlignment* ped_alignment_new (PedSector offset, PedSector grain_size);
extern void ped_alignment_destroy (PedAlignment* align);
extern PedAlignment* ped_alignment_duplicate (const PedAlignment* align);
extern PedAlignment* ped_alignment_intersect (const PedAlignment* a,
					      const PedAlignment* b);

extern PedSector
ped_alignment_align_up (const PedAlignment* align, const PedGeometry* geom,
			PedSector sector) _GL_ATTRIBUTE_PURE;
extern PedSector
ped_alignment_align_down (const PedAlignment* align, const PedGeometry* geom,
			  PedSector sector) _GL_ATTRIBUTE_PURE;
extern PedSector
ped_alignment_align_nearest (const PedAlignment* align, const PedGeometry* geom,
			     PedSector sector) _GL_ATTRIBUTE_PURE;

extern int
ped_alignment_is_aligned (const PedAlignment* align, const PedGeometry* geom,
			  PedSector sector) _GL_ATTRIBUTE_PURE;

extern const PedAlignment* ped_alignment_any;
extern const PedAlignment* ped_alignment_none;

static inline PedSector
ped_div_round_up (PedSector numerator, PedSector divisor)
{
	return (numerator + divisor - 1) / divisor;
}


static inline PedSector
ped_div_round_to_nearest (PedSector numerator, PedSector divisor)
{
	return (numerator + divisor/2) / divisor;
}

#endif /* PED_NATMATH_H_INCLUDED */

/**
 * @}
 */
