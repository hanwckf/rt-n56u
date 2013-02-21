/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2000, 2005, 2007-2012 Free Software Foundation, Inc.

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

/** \file geom.c */


/**
 * \addtogroup PedGeometry
 *
 * \brief PedGeometry represents a continuous region on a device. All addressing
 *      through a PedGeometry object is in terms of the start of the continuous
 *      region.
 *
 * The following conditions are always true on a PedGeometry object manipulated
 * with the GNU Parted API:
 *
 * - <tt>start + length - 1 == end</tt>
 * - <tt>length > 0</tt>
 * - <tt>start >= 0</tt>
 * - <tt>end < dev->length</tt>
 *
 * @{
 */

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

/**
 * Initialize the previously allocated PedGeometry \p geom.
 */
int
ped_geometry_init (PedGeometry* geom, const PedDevice* dev,
		   PedSector start, PedSector length)
{
	PED_ASSERT (geom != NULL);
	PED_ASSERT (dev != NULL);

	geom->dev = (PedDevice*) dev;
	return ped_geometry_set (geom, start, length);
}

/**
 * Create a new PedGeometry object on \p disk, starting at \p start with a
 * size of \p length sectors.
 *
 * \return NULL on failure.
 */
PedGeometry*
ped_geometry_new (const PedDevice* dev, PedSector start, PedSector length)
{
	PedGeometry*	geom;

	PED_ASSERT (dev != NULL);

	geom = (PedGeometry*) ped_malloc (sizeof (PedGeometry));
	if (!geom)
		goto error;
	if (!ped_geometry_init (geom, dev, start, length))
		goto error_free_geom;
	return geom;

error_free_geom:
	free (geom);
error:
	return NULL;
}

/**
 * Duplicate a PedGeometry object.
 *
 * This function constructs a PedGeometry object that is an identical but
 * independent copy of \p geom.  Both the input, \p geom, and the output
 * should be destroyed with ped_geometry_destroy() when they are no
 * longer needed.
 *
 * \return NULL on failure.
 */
PedGeometry*
ped_geometry_duplicate (const PedGeometry* geom)
{
	PED_ASSERT (geom != NULL);
	return ped_geometry_new (geom->dev, geom->start, geom->length);
}

/**
 * Return a PedGeometry object that refers to the intersection of
 * \p a and \p b.
 *
 * This function constructs a PedGeometry object that describes the
 * region that is common to both a and b.  If there is no such common
 * region, it returns NULL.  (This situation is not treated as an
 * error by much of GNU Parted.)
 */
PedGeometry*
ped_geometry_intersect (const PedGeometry* a, const PedGeometry* b)
{
	PedSector	start;
	PedSector	end;

	if (!a || !b || a->dev != b->dev)
		return NULL;

	start = PED_MAX (a->start, b->start);
	end = PED_MIN (a->end, b->end);
	if (start > end)
		return NULL;

	return ped_geometry_new (a->dev, start, end - start + 1);
}

/**
 * Destroy a PedGeometry object.
 */
void
ped_geometry_destroy (PedGeometry* geom)
{
	PED_ASSERT (geom != NULL);

	free (geom);
}

/**
 * Assign a new \p start, \p end (implicitly) and \p length to \p geom.
 *
 * \p geom->end is calculated from \p start and \p length.
 */
int
ped_geometry_set (PedGeometry* geom, PedSector start, PedSector length)
{
	PED_ASSERT (geom != NULL);
	PED_ASSERT (geom->dev != NULL);

	if (length < 1) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't have the end before the start!"
                          " (start sector=%jd length=%jd)"), start, length);
		return 0;
	}
	if (start < 0 || start + length - 1 >= geom->dev->length) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't have a partition outside the disk!"));
		return 0;
 	}

	geom->start = start;
	geom->length = length;
	geom->end = start + length - 1;

	return 1;
}

/**
 * Assign a new start to \p geom without changing \p geom->end.
 *
 * \p geom->length is updated accordingly.
 */
int
ped_geometry_set_start (PedGeometry* geom, PedSector start)
{
	return ped_geometry_set (geom, start, geom->end - start + 1);
}

/**
 * Assign a new end to \p geom without changing \p geom->start.
 *
 * \p geom->length is updated accordingly.
 */
int
ped_geometry_set_end (PedGeometry* geom, PedSector end)
{
	return ped_geometry_set (geom, geom->start, end - geom->start + 1);
}
/**
 * Test if \p a overlaps with \p b.
 *
 * That is, they lie on the same physical device, and they share
 * the same physical region at least partially.
 *
 * \return 1 if \p a and \p b overlap.
 */
int
ped_geometry_test_overlap (const PedGeometry* a, const PedGeometry* b)
{
	PED_ASSERT (a != NULL);
	PED_ASSERT (b != NULL);

	if (a->dev != b->dev)
		return 0;

	if (a->start < b->start)
		return a->end >= b->start;
	else
		return b->end >= a->start;
}

/**
 * Tests if \p b lies completely within \p a.  That is, they lie on the same
 * physical device, and all of the \p b's region is contained inside
 * \p a's.
 *
 * \return 1 if the region \p b describes is contained entirely inside \p a
*/
int
ped_geometry_test_inside (const PedGeometry* a, const PedGeometry* b)
{
	PED_ASSERT (a != NULL);
	PED_ASSERT (b != NULL);

	if (a->dev != b->dev)
		return 0;

	return b->start >= a->start && b->end <= a->end;
}

/**
 * Tests if \a a and \p b refer to the same physical region.
 *
 * \return 1 if \p a and \p b describe the same regions
 *
 */
int
ped_geometry_test_equal (const PedGeometry* a, const PedGeometry* b)
{
	PED_ASSERT (a != NULL);
	PED_ASSERT (b != NULL);

	return a->dev == b->dev
	       && a->start == b->start
	       && a->end == b->end;
}

/**
 * Tests if \p sector is inside \p geom.
 *
 * \return 1 if sector lies within the \p region that \p geom describes
 */
int
ped_geometry_test_sector_inside (const PedGeometry* geom, PedSector sector)
{
	PED_ASSERT (geom != NULL);

	return sector >= geom->start && sector <= geom->end;
}

/**
 * Reads data from the region represented by \p geom.  \p offset is the
 * location from within the region, not from the start of the disk.
 * \p count sectors are read into \p buffer.
 * This is essentially equivalent to:
 * \code
 * 	ped_device_read (geom->disk->dev, buffer, geom->start + offset, count)
 * \endcode
 *
 * \throws PED_EXCEPTION_ERROR when attempting to read sectors outside of
 * partition
 *
 * \return 0 on failure
 */
int
ped_geometry_read (const PedGeometry* geom, void* buffer, PedSector offset,
		   PedSector count)
{
	PedSector	real_start;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (buffer != NULL);
	PED_ASSERT (offset >= 0);
	PED_ASSERT (count >= 0);

	real_start = geom->start + offset;

	if (real_start + count - 1 > geom->end)
		return 0;

	if (!ped_device_read (geom->dev, buffer, real_start, count))
		return 0;
	return 1;
}

/* Like ped_device_read, but read into malloc'd storage.  */
int
ped_geometry_read_alloc (const PedGeometry* geom, void** buffer,
                         PedSector offset, PedSector count)
{
	char *buf = ped_malloc (count * geom->dev->sector_size);
	if (buf == NULL)
		return 0;
	int ok = ped_geometry_read (geom, buf, offset, count);
	if (!ok) {
		free (buf);
		buf = NULL;
	}

	*buffer = buf;
	return ok;
}

/**
 * Flushes the cache on \p geom.
 *
 * This function flushes all write-behind caches that might be holding
 * writes made by ped_geometry_write() to \p geom.  It is slow, because
 * it guarantees cache coherency among all relevant caches.
 *
 * \return 0 on failure
 */
int
ped_geometry_sync (PedGeometry* geom)
{
	PED_ASSERT (geom != NULL);
	return ped_device_sync (geom->dev);
}

/**
 * Flushes the cache on \p geom.
 *
 * This function flushes all write-behind caches that might be holding writes
 * made by ped_geometry_write() to \p geom.  It does NOT ensure cache coherency
 * with other caches that cache data in the region described by \p geom.
 * If you need cache coherency, use ped_geometry_sync() instead.
 *
 * \return 0 on failure
 */
int
ped_geometry_sync_fast (PedGeometry* geom)
{
	PED_ASSERT (geom != NULL);
	return ped_device_sync_fast (geom->dev);
}

/**
 * Writes data into the region represented by \p geom.  \p offset is the
 * location from within the region, not from the start of the disk.
 * \p count sectors are written.
 *
 * \return 0 on failure
 */
int
ped_geometry_write (PedGeometry* geom, const void* buffer, PedSector offset,
		    PedSector count)
{
	int		exception_status;
	PedSector	real_start;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (buffer != NULL);
	PED_ASSERT (offset >= 0);
	PED_ASSERT (count >= 0);

	real_start = geom->start + offset;

	if (real_start + count - 1 > geom->end) {
		exception_status = ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Attempt to write sectors %ld-%ld outside of "
			  "partition on %s."),
			(long) offset, (long) (offset + count - 1),
			geom->dev->path);
		return exception_status == PED_EXCEPTION_IGNORE;
	}

	if (!ped_device_write (geom->dev, buffer, real_start, count))
		return 0;
	return 1;
}

/**
 * Checks for physical disk errors.  \todo use ped_device_check()
 *
 * Checks a region for physical defects on \p geom.  \p buffer is used
 * for temporary storage for ped_geometry_check(), and has an undefined
 * value.  \p buffer is \p buffer_size sectors long.
 * The region checked starts at \p offset sectors inside the
 * region represented by \p geom, and is \p count sectors long.
 * \p granularity specificies how sectors should be grouped
 * together.  The first bad sector to be returned will always be in
 * the form:
 * 	<tt>offset + n * granularity</tt>
 *
 * \return the first bad sector, or 0 if there were no physical errors
 */
PedSector
ped_geometry_check (PedGeometry* geom, void* buffer, PedSector buffer_size,
		    PedSector offset, PedSector granularity, PedSector count,
		    PedTimer* timer)
{
	PedSector	group;
	PedSector	i;
	PedSector	read_len;

	PED_ASSERT (geom != NULL);
	PED_ASSERT (buffer != NULL);

	ped_timer_reset (timer);
	ped_timer_set_state_name (timer, _("checking for bad blocks"));

retry:
	ped_exception_fetch_all();
	for (group = offset; group < offset + count; group += buffer_size) {
		ped_timer_update (timer, 1.0 * (group - offset) / count);
		read_len = PED_MIN (buffer_size, offset + count - group);
		if (!ped_geometry_read (geom, buffer, group, read_len))
			goto found_error;
	}
	ped_exception_leave_all();
	ped_timer_update (timer, 1.0);
	return 0;

found_error:
	ped_exception_catch();
	for (i = group; i + granularity < group + count; i += granularity) {
		if (!ped_geometry_read (geom, buffer, i, granularity)) {
			ped_exception_catch();
			ped_exception_leave_all();
			return i;
		}
	}
	ped_exception_leave_all();
	goto retry;   /* weird: failure on group read, but not individually */
}

/**
 * This function takes a \p sector inside the region described by src, and
 * returns that sector's address inside dst.  This means that
 *
 * \code
 * 	ped_geometry_read (dst, buf, ped_geometry_map(dst, src, sector), 1)
 * \endcode
 *
 * does the same thing as
 *
 * \code
 * 	ped_geometry_read (src, buf, sector, 1)
 * \endcode
 *
 * Clearly, this will only work if \p src and \p dst overlap.
 *
 * \return -1 if \p sector is not within \p dst's space,
 * 	or \p sector's address inside \p dst
 *
 */
PedSector
ped_geometry_map (const PedGeometry* dst, const PedGeometry* src,
		  PedSector sector)
{
	PedSector	result;

	PED_ASSERT (dst != NULL);
	PED_ASSERT (src != NULL);

	if (!ped_geometry_test_sector_inside (src, sector))
		return -1;
	if (dst->dev != src->dev)
		return -1;

	result = src->start + sector - dst->start;
	if (result < 0 || result > dst->length)
		return -1;

	return result;
}

/** @} */
