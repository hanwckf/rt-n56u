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
 * \file natmath.c
 */

/**
 * \addtogroup PedAlignment
 *
 * \brief Alignment constraint model.
 *
 * This part of libparted models alignment constraints.
 *
 * @{
 */

#include <config.h>
#include <stdlib.h>
#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/natmath.h>

/* Arrrghhh!  Why doesn't C have tuples? */
typedef struct {
	PedSector	gcd;		/* "converges" to the gcd */
	PedSector	x;
	PedSector	y;
} EuclidTriple;

static const PedAlignment _any = {
	offset:		0,
	grain_size:	1
};

const PedAlignment* ped_alignment_any = &_any;
const PedAlignment* ped_alignment_none = NULL;

/* This function returns "a mod b", the way C should have done it!
 * Mathematicians prefer -3 mod 4 to be 3.  Reason: division by N
 * is all about adding or subtracting N, and we like our remainders
 * to be between 0 and N - 1.
 */
static PedSector
abs_mod (PedSector a, PedSector b)
{
	if (a < 0)
		return a % b + b;
	else
		return a % b;
}

/* Rounds a number down to the closest number that is a multiple of
 * grain_size.
 */
PedSector
ped_round_down_to (PedSector sector, PedSector grain_size)
{
	return sector - abs_mod (sector, grain_size);
}

/* Rounds a number up to the closest number that is a multiple of
 * grain_size.
 */
PedSector
ped_round_up_to (PedSector sector, PedSector grain_size)
{
	if (sector % grain_size)
		return ped_round_down_to (sector, grain_size) + grain_size;
	else
		return sector;
}

/* Rounds a number to the closest number that is a multiple of grain_size. */
PedSector
ped_round_to_nearest (PedSector sector, PedSector grain_size)
{
	if (sector % grain_size > grain_size/2)
		return ped_round_up_to (sector, grain_size);
	else
		return ped_round_down_to (sector, grain_size);
}

/* This function returns the largest number that divides both a and b.
 * It uses the ancient Euclidean algorithm.
 */
PedSector
ped_greatest_common_divisor (PedSector a, PedSector b)
{
	PED_ASSERT (a >= 0);
	PED_ASSERT (b >= 0);

	/* Put the arguments in the "right" format.  (Recursive calls made by
	 * this function are always in the right format.)
	 */
	if (b > a)
		return ped_greatest_common_divisor (b, a);

	if (b)
		return ped_greatest_common_divisor (b, a % b);
	else
		return a;
}

/**
 * Initialize a preallocated piece of memory for an alignment object
 * (used by PedConstraint).
 *
 * The object will represent all sectors \e s for which the equation
 * <tt>s = offset + X * grain_size</tt> holds.
 */
int
ped_alignment_init (PedAlignment* align, PedSector offset, PedSector grain_size)
{
	PED_ASSERT (align != NULL);

	if (grain_size < 0)
		return 0;

	if (grain_size)
		align->offset = abs_mod (offset, grain_size);
	else
		align->offset = offset;
	align->grain_size = grain_size;

	return 1;
}

/**
 * Return an alignment object (used by PedConstraint), representing all
 * PedSector's that are of the form <tt>offset + X * grain_size</tt>.
 */
PedAlignment*
ped_alignment_new (PedSector offset, PedSector grain_size)
{
	PedAlignment*	align;

	align = (PedAlignment*) ped_malloc (sizeof (PedAlignment));
	if (!align)
		goto error;

	if (!ped_alignment_init (align, offset, grain_size))
		goto error_free_align;

	return align;

error_free_align:
	free (align);
error:
	return NULL;
}

/**
 * Free up memory associated with \p align.
 */
void
ped_alignment_destroy (PedAlignment* align)
{
	free (align);
}

/**
 * Return a duplicate of \p align.
 */
PedAlignment*
ped_alignment_duplicate (const PedAlignment* align)
{
	if (!align)
		return NULL;
	return ped_alignment_new (align->offset, align->grain_size);
}

/* the extended Euclid algorithm.
 *
 * input:
 * 	a and b, a > b
 *
 * output:
 * 	gcd, x and y, such that:
 *
 * 	gcd = greatest common divisor of a and b
 * 	gcd = x*a + y*b
 */
static EuclidTriple _GL_ATTRIBUTE_PURE
extended_euclid (int a, int b)
{
	EuclidTriple	result;
	EuclidTriple	tmp;

	if (b == 0) {
		result.gcd = a;
		result.x = 1;
		result.y = 0;
		return result;
	}

	tmp = extended_euclid (b, a % b);
	result.gcd = tmp.gcd;
	result.x = tmp.y;
	result.y = tmp.x - (a/b) * tmp.y;
	return result;
}

/**
 * This function computes a PedAlignment object that describes the
 * intersection of two alignments.  That is, a sector satisfies the
 * new alignment object if and only if it satisfies both of the original
 * ones.  (See ped_alignment_is_aligned() for the meaning of "satisfies")
 *
 * Apart from the trivial cases (where one or both of the alignment objects
 * constraints have no sectors that satisfy them), this is what we're trying to
 * do:
 *  - two input constraints: \p a and \p b.
 *  - the new grain_size is going to be the lowest common multiple of
 *  \p a->grain_size and \p b->grain_size
 *  - hard part - solve the simultaneous equations, for offset, where offset,
 *  X and Y are variables.  (Note: offset can be obtained from either X or Y,
 *  by substituing into either equation)
 *
 * \code
 *  	offset = \p a->offset + X * \p a->grain_size		(1)
 *  	offset = \p b->offset + Y * \p b->grain_size		(2)
 * \endcode
 *
 * or, abbreviated:
 *
 * \code
 *  	o = Ao + X*Ag		(1)
 *  	o = Bo + Y*Bg		(2)
 *
 *  =>	Ao + X*Ag    = Bo + Y*Bg     (1) = (2)
 *  	X*Ag - Y*Bg  = Bo - Ao  (3)
 * \endcode
 *
 * As it turns out, there only exists a solution if (Bo - Ao) is a multiple
 * of the GCD of Ag and Bg.  Reason: all linear combinations of Ag and Bg are
 * multiples of the GCD.
 *
 * Proof:
 *
 * \code
 *	A * Ag + B * Bg
 *	= A * (\p a * gcd) + B * (\p b * gcd)
 *	= gcd * (A * \p a + B * \p b)
 * \endcode
 *
 * gcd is a factor of the linear combination.  QED
 *
 * Anyway, \p a * Ag + \p b * Bg = gcd can be solved (for \p a, \p b and gcd)
 * with Euclid's extended algorithm.  Then, we just multiply through by
 * (Bo - Ao) / gcd to get (3).
 *
 * i.e.
 * \code
 * 	A * Ag + B * Bg				= gcd
 * 	A*(Bo-Ao)/gcd * Ag + B(Bo-Ao)/gcd * Bg	= gcd * (Bo-Ao)/gcd
 * 	X*Ag - Y*Bg				= Bo - Ao		(3)
 *
 * 	X = A*(Bo-Ao)/gcd
 * 	Y = - B*(Bo-Ao)/gcd
 * \endcode
 *
 * then:
 * \code
 *  	o = Ao + X*Ag			(1)
 *	  = Ao + A*(Bo-Ao)/gcd*Ag
 *  	o = Bo + Y*Bg			(2)
 *	  = Bo - B*(Bo-Ao)/gcd*Ag
 * \endcode
 *
 * Thanks go to Nathan Hurst (njh@hawthorn.csse.monash.edu.au) for figuring
 * this algorithm out :-)
 *
 * \note Returned \c NULL is a valid PedAlignment object, and can be used
	for ped_alignment_*() function.
 *
 * \return a PedAlignment on success, \c NULL on failure
 */
PedAlignment*
ped_alignment_intersect (const PedAlignment* a, const PedAlignment* b)
{
	PedSector	new_grain_size;
	PedSector	new_offset;
	PedSector	delta_on_gcd;
	EuclidTriple	gcd_factors;


	if (!a || !b)
		return NULL;

        /*PED_DEBUG (0x10, "intersecting alignments (%d,%d) and (%d,%d)",
                        a->offset, a->grain_size, b->offset, b->grain_size);
        */

	if (a->grain_size < b->grain_size) {
		const PedAlignment*	tmp;
	        tmp = a; a = b; b = tmp;
	}

	/* weird/trivial case: where the solution space for "a" or "b" is
	 * either empty or contains exactly one solution
	 */
	if (a->grain_size == 0 && b->grain_size == 0) {
		if (a->offset == b->offset)
			return ped_alignment_duplicate (a);
		else
			return NULL;
	}

	/* general case */
	gcd_factors = extended_euclid (a->grain_size, b->grain_size);

	delta_on_gcd = (b->offset - a->offset) / gcd_factors.gcd;
	new_offset = a->offset + gcd_factors.x * delta_on_gcd * a->grain_size;
	new_grain_size = a->grain_size * b->grain_size / gcd_factors.gcd;

	/* inconsistency => no solution */
	if (new_offset
	    != b->offset - gcd_factors.y * delta_on_gcd * b->grain_size)
		return NULL;

	return ped_alignment_new (new_offset, new_grain_size);
}

/* This function returns the sector closest to "sector" that lies inside
 * geom and satisfies the alignment constraint.
 */
static PedSector _GL_ATTRIBUTE_PURE
_closest_inside_geometry (const PedAlignment* align, const PedGeometry* geom,
			  PedSector sector)
{
	PED_ASSERT (align != NULL);

	if (!align->grain_size) {
		if (ped_alignment_is_aligned (align, geom, sector)
		    && (!geom || ped_geometry_test_sector_inside (geom,
				    				  sector)))
			return sector;
		else
			return -1;
	}

	if (sector < geom->start)
		sector += ped_round_up_to (geom->start - sector,
					   align->grain_size);
	if (sector > geom->end)
		sector -= ped_round_up_to (sector - geom->end,
					   align->grain_size);

	if (!ped_geometry_test_sector_inside (geom, sector))
		return -1;
	return sector;
}

/**
 * This function returns the closest sector to \p sector that lies inside
 * \p geom that satisfies the given alignment constraint \p align.  It prefers
 * sectors that are beyond \p sector (are not smaller than \p sector),
 * but does not guarantee that this.
 *
 * \return a PedSector on success, \c -1 on failure
 */
PedSector
ped_alignment_align_up (const PedAlignment* align, const PedGeometry* geom,
			PedSector sector)
{
	PedSector	result;

	PED_ASSERT (align != NULL);

	if (!align->grain_size)
		result = align->offset;
	else
		result = ped_round_up_to (sector - align->offset,
			       		  align->grain_size)
			 + align->offset;

	if (geom)
		result = _closest_inside_geometry (align, geom, result);
	return result;
}

/**
 * This function returns the closest sector to \p sector that lies inside
 * \p geom that satisfies the given alignment constraint \p align.  It prefers
 * sectors that are before \p sector (are not larger than \p sector),
 * but does not guarantee that this.
 *
 * \return a PedSector on success, \c -1 on failure
 */
PedSector
ped_alignment_align_down (const PedAlignment* align, const PedGeometry* geom,
			  PedSector sector)
{
	PedSector	result;

	PED_ASSERT (align != NULL);

	if (!align->grain_size)
		result = align->offset;
	else
		result = ped_round_down_to (sector - align->offset,
			      		    align->grain_size)
			 + align->offset;

	if (geom)
		result = _closest_inside_geometry (align, geom, result);
	return result;
}

/* Returns either a or b, depending on which is closest to "sector". */
static PedSector
closest (PedSector sector, PedSector a, PedSector b)
{
	if (a == -1)
		return b;
	if (b == -1)
		return a;

	if (abs (sector - a) < abs (sector - b))
		return a;
	else
		return b;
}

/**
 * This function returns the sector that is closest to \p sector,
 * satisfies the \p align constraint and lies inside \p geom.
 *
 * \return a PedSector on success, \c -1 on failure
 */
PedSector
ped_alignment_align_nearest (const PedAlignment* align, const PedGeometry* geom,
			     PedSector sector)
{
	PED_ASSERT (align != NULL);

	return closest (sector, ped_alignment_align_up (align, geom, sector),
			ped_alignment_align_down (align, geom, sector));
}

/**
 * This function returns 1 if \p sector satisfies the alignment
 * constraint \p align and lies inside \p geom.
 *
 * \return \c 1 on success, \c 0 on failure
 */
int
ped_alignment_is_aligned (const PedAlignment* align, const PedGeometry* geom,
			  PedSector sector)
{
	if (!align)
		return 0;

	if (geom && !ped_geometry_test_sector_inside (geom, sector))
		return 0;

	if (align->grain_size)
		return (sector - align->offset) % align->grain_size == 0;
	else
		return sector == align->offset;
}

/**
 * @}
 */
