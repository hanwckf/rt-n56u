/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1998-2000, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef PED_CONSTRAINT_H_INCLUDED
#define PED_CONSTRAINT_H_INCLUDED

typedef struct _PedConstraint	PedConstraint;

#include <parted/device.h>
#include <parted/geom.h>
#include <parted/natmath.h>

struct _PedConstraint {
	PedAlignment*	start_align;
	PedAlignment*	end_align;
	PedGeometry*	start_range;
	PedGeometry*	end_range;
	PedSector	min_size;
	PedSector	max_size;
};

extern int
ped_constraint_init (
	PedConstraint* constraint,
	const PedAlignment* start_align,
	const PedAlignment* end_align,
	const PedGeometry* start_range,
	const PedGeometry* end_range,
	PedSector min_size,
	PedSector max_size);

extern PedConstraint*
ped_constraint_new (
	const PedAlignment* start_align,
	const PedAlignment* end_align,
	const PedGeometry* start_range,
	const PedGeometry* end_range,
	PedSector min_size,
	PedSector max_size);

extern PedConstraint*
ped_constraint_new_from_min_max (
	const PedGeometry* min,
	const PedGeometry* max);

extern PedConstraint*
ped_constraint_new_from_min (const PedGeometry* min);

extern PedConstraint*
ped_constraint_new_from_max (const PedGeometry* max);

extern PedConstraint*
ped_constraint_duplicate (const PedConstraint* constraint);

extern void
ped_constraint_done (PedConstraint* constraint);

extern void
ped_constraint_destroy (PedConstraint* constraint);

extern PedConstraint*
ped_constraint_intersect (const PedConstraint* a, const PedConstraint* b);

extern PedGeometry*
ped_constraint_solve_max (const PedConstraint* constraint);

extern PedGeometry*
ped_constraint_solve_nearest (
	const PedConstraint* constraint, const PedGeometry* geom);

extern int
ped_constraint_is_solution (const PedConstraint* constraint,
			    const PedGeometry* geom)
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;

extern PedConstraint*
ped_constraint_any (const PedDevice* dev);

extern PedConstraint*
ped_constraint_exact (const PedGeometry* geom);

#endif /* PED_CONSTRAINT_H_INCLUDED */
