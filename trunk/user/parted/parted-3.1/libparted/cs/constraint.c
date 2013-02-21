/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2000-2001, 2007, 2009-2012 Free Software Foundation, Inc.

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
 * \addtogroup PedConstraint
 *
 * \brief Constraint solver interface.
 *
 * Constraints are used to communicate restrictions on operations Constraints
 * are restrictions on the location and alignment of the start and end of a
 * partition, and the minimum and maximum size.
 *
 * Constraints are closed under intersection (for the proof see the source
 * code).  For background information see the Chinese Remainder Theorem.
 *
 * This interface consists of construction constraints, finding the intersection
 * of constraints, and finding solutions to constraints.
 *
 * The constraint solver allows you to specify constraints on where a partition
 * or file system (or any PedGeometry) may be placed/resized/etc. For example,
 * you might want to make sure that a file system is at least 10 Gb, or that it
 * starts at the beginning of new cylinder.
 *
 * The constraint solver in this file unifies solver in geom.c (which allows you
 * to specify constraints on ranges) and natmath.c (which allows you to specify
 * alignment constraints).
 *
 * @{
 */

#include <config.h>
#include <parted/parted.h>
#include <parted/debug.h>
#include <assert.h>

/**
 * Initializes a pre-allocated piece of memory to contain a constraint
 * with the supplied default values.
 *
 * \return \c 0 on failure.
 */
int
ped_constraint_init (
	PedConstraint* constraint,
	const PedAlignment* start_align,
	const PedAlignment* end_align,
	const PedGeometry* start_range,
	const PedGeometry* end_range,
	PedSector min_size,
	PedSector max_size)
{
	PED_ASSERT (constraint != NULL);
	PED_ASSERT (start_range != NULL);
	PED_ASSERT (end_range != NULL);
	PED_ASSERT (min_size > 0);
	PED_ASSERT (max_size > 0);

	constraint->start_align = ped_alignment_duplicate (start_align);
	constraint->end_align = ped_alignment_duplicate (end_align);
	constraint->start_range = ped_geometry_duplicate (start_range);
	constraint->end_range = ped_geometry_duplicate (end_range);
	constraint->min_size = min_size;
	constraint->max_size = max_size;

	return 1;
}

/**
 * Convenience wrapper for ped_constraint_init().
 *
 * Allocates a new piece of memory and initializes the constraint.
 *
 * \return \c NULL on failure.
 */
PedConstraint*
ped_constraint_new (
	const PedAlignment* start_align,
	const PedAlignment* end_align,
	const PedGeometry* start_range,
	const PedGeometry* end_range,
	PedSector min_size,
	PedSector max_size)
{
	PedConstraint*	constraint;

	constraint = (PedConstraint*) ped_malloc (sizeof (PedConstraint));
	if (!constraint)
		goto error;
	if (!ped_constraint_init (constraint, start_align, end_align,
			          start_range, end_range, min_size, max_size))
		goto error_free_constraint;
	return constraint;

error_free_constraint:
	free (constraint);
error:
	return NULL;
}

/**
 * Return a constraint that requires a region to be entirely contained inside
 * \p max, and to entirely contain \p min.
 *
 * \return \c NULL on failure.
 */
PedConstraint*
ped_constraint_new_from_min_max (
	const PedGeometry* min,
	const PedGeometry* max)
{
	PedGeometry	start_range;
	PedGeometry	end_range;

	PED_ASSERT (min != NULL);
	PED_ASSERT (max != NULL);
	PED_ASSERT (ped_geometry_test_inside (max, min));

	ped_geometry_init (&start_range, min->dev, max->start,
			   min->start - max->start + 1);
	ped_geometry_init (&end_range, min->dev, min->end,
			   max->end - min->end + 1);

	return ped_constraint_new (
			ped_alignment_any, ped_alignment_any,
			&start_range, &end_range,
			min->length, max->length);
}

/**
 * Return a constraint that requires a region to entirely contain \p min.
 *
 * \return \c NULL on failure.
 */
PedConstraint*
ped_constraint_new_from_min (const PedGeometry* min)
{
	PedGeometry	full_dev;

	PED_ASSERT (min != NULL);

	ped_geometry_init (&full_dev, min->dev, 0, min->dev->length);
	return ped_constraint_new_from_min_max (min, &full_dev);
}

/**
 * Return a constraint that requires a region to be entirely contained inside
 * \p max.
 *
 * \return \c NULL on failure.
 */
PedConstraint*
ped_constraint_new_from_max (const PedGeometry* max)
{
	PED_ASSERT (max != NULL);

	return ped_constraint_new (
			ped_alignment_any, ped_alignment_any,
			max, max, 1, max->length);
}

/**
 * Duplicate a constraint.
 *
 * \return \c NULL on failure.
 */
PedConstraint*
ped_constraint_duplicate (const PedConstraint* constraint)
{
	PED_ASSERT (constraint != NULL);

	return ped_constraint_new (
		constraint->start_align,
		constraint->end_align,
		constraint->start_range,
		constraint->end_range,
		constraint->min_size,
		constraint->max_size);
}

/**
 * Return a constraint that requires a region to satisfy both \p a and \p b.
 *
 * Moreover, any region satisfying \p a and \p b will also satisfy the returned
 * constraint.
 *
 * \return \c NULL if no solution could be found (note that \c NULL is a valid
 *         PedConstraint).
 */
PedConstraint*
ped_constraint_intersect (const PedConstraint* a, const PedConstraint* b)
{
	PedAlignment*	start_align;
	PedAlignment*	end_align;
	PedGeometry*	start_range;
	PedGeometry*	end_range;
	PedSector	min_size;
	PedSector	max_size;
	PedConstraint*	constraint;

	if (!a || !b)
		return NULL;

	start_align = ped_alignment_intersect (a->start_align, b->start_align);
	if (!start_align)
		goto empty;
	end_align = ped_alignment_intersect (a->end_align, b->end_align);
	if (!end_align)
		goto empty_destroy_start_align;
	start_range = ped_geometry_intersect (a->start_range, b->start_range);
	if (!start_range)
		goto empty_destroy_end_align;
	end_range = ped_geometry_intersect (a->end_range, b->end_range);
	if (!end_range)
		goto empty_destroy_start_range;
	min_size = PED_MAX (a->min_size, b->min_size);
	max_size = PED_MIN (a->max_size, b->max_size);

	constraint = ped_constraint_new (
			start_align, end_align, start_range, end_range,
			min_size, max_size);
	if (!constraint)
		goto empty_destroy_end_range;

	ped_alignment_destroy (start_align);
	ped_alignment_destroy (end_align);
	ped_geometry_destroy (start_range);
	ped_geometry_destroy (end_range);
	return constraint;

empty_destroy_end_range:
	ped_geometry_destroy (end_range);
empty_destroy_start_range:
	ped_geometry_destroy (start_range);
empty_destroy_end_align:
	ped_alignment_destroy (end_align);
empty_destroy_start_align:
	ped_alignment_destroy (start_align);
empty:
	return NULL;
}

/**
 * Release the memory allocated for a PedConstraint constructed with
 * ped_constraint_init().
 */
void
ped_constraint_done (PedConstraint* constraint)
{
	PED_ASSERT (constraint != NULL);

	ped_alignment_destroy (constraint->start_align);
	ped_alignment_destroy (constraint->end_align);
	ped_geometry_destroy (constraint->start_range);
	ped_geometry_destroy (constraint->end_range);
}

/**
 * Release the memory allocated for a PedConstraint constructed with
 * ped_constraint_new().
 */
void
ped_constraint_destroy (PedConstraint* constraint)
{
	if (constraint) {
		ped_constraint_done (constraint);
		free (constraint);
	}
}

/*
 * Return the region within which the start must lie
 * in order to satisfy a constriant.  It takes into account
 * constraint->start_range, constraint->min_size and constraint->max_size.
 * All sectors in this range that also satisfy alignment requirements have
 * an end, such that the (start, end) satisfy the constraint.
 */
static PedGeometry*
_constraint_get_canonical_start_range (const PedConstraint* constraint)
{
	PedSector	first_end_soln;
	PedSector	last_end_soln;
	PedSector	min_start;
	PedSector	max_start;
	PedGeometry	start_min_max_range;

	if (constraint->min_size > constraint->max_size)
		return NULL;

	first_end_soln = ped_alignment_align_down (
			constraint->end_align, constraint->end_range,
			constraint->end_range->start);
	last_end_soln = ped_alignment_align_up (
			constraint->end_align, constraint->end_range,
			constraint->end_range->end);
	if (first_end_soln == -1 || last_end_soln == -1
	    || first_end_soln > last_end_soln
	    || last_end_soln < constraint->min_size)
		return NULL;

	min_start = first_end_soln - constraint->max_size + 1;
	if (min_start < 0)
		min_start = 0;
	max_start = last_end_soln - constraint->min_size + 1;
	if (max_start < 0)
		return NULL;

	ped_geometry_init (
		&start_min_max_range, constraint->start_range->dev,
		min_start, max_start - min_start + 1);

	return ped_geometry_intersect (&start_min_max_range,
				       constraint->start_range);
}

/*
 * Return the nearest start that will have at least one other end that
 * together satisfy the constraint.
 */
static PedSector
_constraint_get_nearest_start_soln (const PedConstraint* constraint,
				    PedSector start)
{
	PedGeometry*	start_range;
	PedSector	result;

	start_range = _constraint_get_canonical_start_range (constraint);
	if (!start_range)
		return -1;
	result = ped_alignment_align_nearest (
			constraint->start_align, start_range, start);
	ped_geometry_destroy (start_range);
	return result;
}

/*
 * Given a constraint and a start ("half of the solution"), find the
 * range of all possible ends, such that all (start, end) are solutions
 * to constraint (subject to additional alignment requirements).
 */
static PedGeometry*
_constraint_get_end_range (const PedConstraint* constraint, PedSector start)
{
	PedDevice*	dev = constraint->end_range->dev;
	PedSector	first_min_max_end;
	PedSector	last_min_max_end;
	PedGeometry	end_min_max_range;

	if (start + constraint->min_size - 1 > dev->length - 1)
		return NULL;

	first_min_max_end = start + constraint->min_size - 1;
	last_min_max_end = start + constraint->max_size - 1;
	if (last_min_max_end > dev->length - 1)
		last_min_max_end = dev->length - 1;

	ped_geometry_init (&end_min_max_range, dev,
			   first_min_max_end,
			   last_min_max_end - first_min_max_end + 1);

	return ped_geometry_intersect (&end_min_max_range,
		       		       constraint->end_range);
}

/*
 * Given "constraint" and "start", find the end that is nearest to
 * "end", such that ("start", the end) together form a solution to
 * "constraint".
 */
static PedSector
_constraint_get_nearest_end_soln (const PedConstraint* constraint,
				  PedSector start, PedSector end)
{
	PedGeometry*	end_range;
	PedSector	result;

	end_range = _constraint_get_end_range (constraint, start);
	if (!end_range)
		return -1;

	result = ped_alignment_align_nearest (constraint->end_align, end_range,
		       			      end);
	ped_geometry_destroy (end_range);
	return result;
}

/**
 * Return the nearest region to \p geom that satisfy a \p constraint.
 *
 * Note that "nearest" is somewhat ambiguous.  This function makes
 * no guarantees about how this ambiguity is resovled.
 *
 * \return PedGeometry, or NULL when a \p constrain cannot be satisfied
 */
PedGeometry*
ped_constraint_solve_nearest (
	const PedConstraint* constraint, const PedGeometry* geom)
{
	PedSector	start;
	PedSector	end;
	PedGeometry*	result;

	if (constraint == NULL)
		return NULL;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (constraint->start_range->dev == geom->dev);

	start = _constraint_get_nearest_start_soln (constraint, geom->start);
	if (start == -1)
		return NULL;
	end = _constraint_get_nearest_end_soln (constraint, start, geom->end);
	if (end == -1)
		return NULL;

	result = ped_geometry_new (geom->dev, start, end - start + 1);
	if (!result)
		return NULL;
	PED_ASSERT (ped_constraint_is_solution (constraint, result));
	return result;
}

/**
 * Find the largest region that satisfies a constraint.
 *
 * There might be more than one solution.  This function makes no
 * guarantees about which solution it will choose in this case.
 */
PedGeometry*
ped_constraint_solve_max (const PedConstraint* constraint)
{
	PedDevice*	dev;
	PedGeometry	full_dev;

	if (!constraint)
		return NULL;
	dev = constraint->start_range->dev;
	ped_geometry_init (&full_dev, dev, 0, dev->length);
	return ped_constraint_solve_nearest (constraint, &full_dev);
}

/**
 * Check whether \p geom satisfies the given constraint.
 *
 * \return \c 1 if it does.
 **/
int
ped_constraint_is_solution (const PedConstraint* constraint,
	       		    const PedGeometry* geom)
{
	PED_ASSERT (constraint != NULL);
	PED_ASSERT (geom != NULL);

	if (!ped_alignment_is_aligned (constraint->start_align, NULL,
				       geom->start))
		return 0;
	if (!ped_alignment_is_aligned (constraint->end_align, NULL, geom->end))
		return 0;
	if (!ped_geometry_test_sector_inside (constraint->start_range,
					      geom->start))
		return 0;
	if (!ped_geometry_test_sector_inside (constraint->end_range, geom->end))
		return 0;
	if (geom->length < constraint->min_size)
		return 0;
	if (geom->length > constraint->max_size)
		return 0;
	return 1;
}

/**
 * Return a constraint that any region on the given device will satisfy.
 */
PedConstraint*
ped_constraint_any (const PedDevice* dev)
{
	PedGeometry	full_dev;

	if (!ped_geometry_init (&full_dev, dev, 0, dev->length))
		return NULL;

	return ped_constraint_new (
			ped_alignment_any,
		       	ped_alignment_any,
			&full_dev,
			&full_dev,
		       	1,
			dev->length);
}

/**
 * Return a constraint that only the given region will satisfy.
 */
PedConstraint*
ped_constraint_exact (const PedGeometry* geom)
{
	PedAlignment	start_align;
	PedAlignment	end_align;
	PedGeometry	start_sector;
	PedGeometry	end_sector;
	int ok;

	/* With grain size of 0, it always succeeds.  */
	ok = ped_alignment_init (&start_align, geom->start, 0);
	assert (ok);
	ok = ped_alignment_init (&end_align, geom->end, 0);
	assert (ok);

	ok = ped_geometry_init (&start_sector, geom->dev, geom->start, 1);
	if (!ok)
	  return NULL;
	ok = ped_geometry_init (&end_sector, geom->dev, geom->end, 1);
	if (!ok)
	  return NULL;

	return ped_constraint_new (&start_align, &end_align,
				   &start_sector, &end_sector, 1,
				   geom->dev->length);
}

/**
 * @}
 */
