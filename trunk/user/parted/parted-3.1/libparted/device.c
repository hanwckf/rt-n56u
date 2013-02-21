/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999 - 2001, 2005, 2007-2010 Free Software Foundation, Inc.

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

/** \file device.c */

/**
 * \addtogroup PedDevice
 *
 * \brief Device access.
 *
 * When ped_device_probe_all() is called, libparted attempts to detect all
 * devices.  It constructs a list which can be accessed with
 * ped_device_get_next().
 *
 * If you want to use a device that isn't on the list, use
 * ped_device_get().  Also, there may be OS-specific constructors, for creating
 * devices from file descriptors, stores, etc.  For example,
 * ped_device_new_from_store().
 *
 * @{
 */

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "architecture.h"

static PedDevice*	devices; /* legal advice says: initialized to NULL,
				    under section 6.7.8 part 10
				    of ISO/EIC 9899:1999 */

static void
_device_register (PedDevice* dev)
{
	PedDevice*	walk;
	for (walk = devices; walk && walk->next; walk = walk->next);
	if (walk)
		walk->next = dev;
	else
		devices = dev;
	dev->next = NULL;
}

static void
_device_unregister (PedDevice* dev)
{
	PedDevice*	walk;
	PedDevice*	last = NULL;

	for (walk = devices; walk != NULL; last = walk, walk = walk->next) {
		if (walk == dev) break;
	}

	/* This function may be called twice for the same device if a
	   libparted user explictly removes the device from the cache using
	   ped_device_cache_remove(), we get called and it then becomes the
	   user's responsibility to free the PedDevice by calling
	   ped_device_destroy().
	   ped_device_destroy() will then call us a second time, so if the
	   device is not found in the list do nothing. */
	if (walk == NULL)
		return;

	if (last)
		last->next = dev->next;
	else
		devices = dev->next;
}

/**
 * Returns the next device that was detected by ped_device_probe_all(), or
 * calls to ped_device_get_next().
 * If dev is NULL, returns the first device.
 *
 * \return NULL if dev is the last device.
 */
PedDevice*
ped_device_get_next (const PedDevice* dev)
{
	if (dev)
		return dev->next;
	else
		return devices;
}

void
_ped_device_probe (const char* path)
{
	PedDevice*	dev;

	PED_ASSERT (path != NULL);

	ped_exception_fetch_all ();
	dev = ped_device_get (path);
	if (!dev)
		ped_exception_catch ();
	ped_exception_leave_all ();
}

/**
 * Attempts to detect all devices.
 */
void
ped_device_probe_all ()
{
	ped_architecture->dev_ops->probe_all ();
}

/**
 * Close/free all devices.
 * Called by ped_done(), so you do not need to worry about it.
 */
void
ped_device_free_all ()
{
	while (devices)
		ped_device_destroy (devices);
}

/**
 * Gets the device "name", where name is usually the block device, e.g.
 * /dev/sdb.  If the device wasn't detected with ped_device_probe_all(),
 * an attempt will be made to detect it again.  If it is found, it will
 * be added to the list.
 */
PedDevice*
ped_device_get (const char* path)
{
	PedDevice*	walk;
	char*		normal_path = NULL;

	PED_ASSERT (path != NULL);
	/* Don't canonicalize /dev/mapper paths, see tests/symlink.c */
	if (strncmp (path, "/dev/mapper/", 12))
		normal_path = canonicalize_file_name (path);
	if (!normal_path)
		/* Well, maybe it is just that the file does not exist.
		 * Try it anyway.  */
		normal_path = strdup (path);
	if (!normal_path)
		return NULL;

	for (walk = devices; walk != NULL; walk = walk->next) {
		if (!strcmp (walk->path, normal_path)) {
			free (normal_path);
			return walk;
		}
	}

	walk = ped_architecture->dev_ops->_new (normal_path);
	free (normal_path);
	if (!walk)
		return NULL;
	_device_register (walk);
	return walk;
}

/**
 * Destroys a device and removes it from the device list, and frees
 * all resources associated with the device (all resources allocated
 * when the device was created).
 */
void
ped_device_destroy (PedDevice* dev)
{
	_device_unregister (dev);

	while (dev->open_count) {
		if (!ped_device_close (dev))
			break;
	}

	ped_architecture->dev_ops->destroy (dev);
}

void
ped_device_cache_remove(PedDevice *dev)
{
	_device_unregister (dev);
}

int
ped_device_is_busy (PedDevice* dev)
{
	return ped_architecture->dev_ops->is_busy (dev);
}

/**
 * Attempt to open a device to allow use of read, write and sync functions.
 *
 * The meaning of "open" is architecture-dependent.  Apart from requesting
 * access to the device from the operating system, it does things like flushing
 * caches.
 * \note May allocate resources.  Any resources allocated here will
 * be freed by a final ped_device_close().  (ped_device_open() may be
 * called multiple times -- it's a ref-count-like mechanism)
 *
 * \return zero on failure
 */
int
ped_device_open (PedDevice* dev)
{
	int	status;

	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);

	if (dev->open_count)
		status = ped_architecture->dev_ops->refresh_open (dev);
	else
		status = ped_architecture->dev_ops->open (dev);
	if (status)
		dev->open_count++;
	return status;
}

/**
 * Close dev.
 * If this is the final close, then resources allocated by
 * ped_device_open() are freed.
 *
 * \return zero on failure
 */
int
ped_device_close (PedDevice* dev)
{
	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);
	PED_ASSERT (dev->open_count > 0);

	if (--dev->open_count)
		return ped_architecture->dev_ops->refresh_close (dev);
	else
		return ped_architecture->dev_ops->close (dev);
}

/**
 * Begins external access mode.  External access mode allows you to
 * safely do IO on the device.  If a PedDevice is open, then you should
 * not do any IO on that device, e.g. by calling an external program
 * like e2fsck, unless you put it in external access mode.  You should
 * not use any libparted commands that do IO to a device, e.g.
 * ped_file_system_{open|resize|copy}, ped_disk_{read|write}), while
 * a device is in external access mode.
 * Also, you should not ped_device_close() a device, while it is
 * in external access mode.
 * Note: ped_device_begin_external_access_mode() does things like
 * tell the kernel to flush its caches.
 *
 * Close a device while pretending it is still open.
 * This is useful for temporarily suspending libparted access to the device
 * in order for an external program to access it.
 * (Running external programs while the device is open can cause cache
 * coherency problems.)
 *
 * In particular, this function keeps track of dev->open_count, so that
 * reference counting isn't screwed up.
 *
 * \return zero on failure.
 */
int
ped_device_begin_external_access (PedDevice* dev)
{
	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);

	dev->external_mode = 1;
	if (dev->open_count)
		return ped_architecture->dev_ops->close (dev);
	else
		return 1;
}

/**
 * \brief Complementary function to ped_device_begin_external_access.
 *
 * \note does things like tell the kernel to flush the device's cache.
 *
 * \return zero on failure.
 */
int
ped_device_end_external_access (PedDevice* dev)
{
	PED_ASSERT (dev != NULL);
	PED_ASSERT (dev->external_mode);

	dev->external_mode = 0;
	if (dev->open_count)
		return ped_architecture->dev_ops->open (dev);
	else
		return 1;
}

/**
 * \internal Read count sectors from dev into buffer, beginning with sector
 * start.
 *
 * \return zero on failure.
 */
int
ped_device_read (const PedDevice* dev, void* buffer, PedSector start,
                 PedSector count)
{
        PED_ASSERT (dev != NULL);
        PED_ASSERT (buffer != NULL);
        PED_ASSERT (!dev->external_mode);
        PED_ASSERT (dev->open_count > 0);

        return (ped_architecture->dev_ops->read) (dev, buffer, start, count);
}

/**
 * \internal Write count sectors from buffer to dev, starting at sector
 * start.
 *
 * \return zero on failure.
 *
 * \sa PedDevice::sector_size
 * \sa PedDevice::phys_sector_size
 */
int
ped_device_write (PedDevice* dev, const void* buffer, PedSector start,
		  PedSector count)
{
	PED_ASSERT (dev != NULL);
	PED_ASSERT (buffer != NULL);
	PED_ASSERT (!dev->external_mode);
	PED_ASSERT (dev->open_count > 0);

	return (ped_architecture->dev_ops->write) (dev, buffer, start, count);
}

PedSector
ped_device_check (PedDevice* dev, void* buffer, PedSector start,
		  PedSector count)
{
	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);
	PED_ASSERT (dev->open_count > 0);

	return (ped_architecture->dev_ops->check) (dev, buffer, start, count);
}

/**
 * \internal Flushes all write-behind caches that might be holding up
 * writes.
 * It is slow because it guarantees cache coherency among all relevant caches.
 *
 * \return zero on failure
 */
int
ped_device_sync (PedDevice* dev)
{
	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);
	PED_ASSERT (dev->open_count > 0);

	return ped_architecture->dev_ops->sync (dev);
}

/**
 * \internal Flushes all write-behind caches that might be holding writes.
 * \warning Does NOT ensure cache coherency with other caches.
 * If you need cache coherency, use ped_device_sync() instead.
 *
 * \return zero on failure
 */
int
ped_device_sync_fast (PedDevice* dev)
{
	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);
	PED_ASSERT (dev->open_count > 0);

	return ped_architecture->dev_ops->sync_fast (dev);
}

/**
 * Get a constraint that represents hardware requirements on geometry.
 * This function will return a constraint representing the limits imposed
 * by the size of the disk, it will *not* provide any alignment constraints.
 *
 * Alignment constraints may be desirable when using media that have a physical
 * sector size that is a multiple of the logical sector size, as in this case
 * proper partition alignment can benefit disk performance signigicantly.
 * When you want a constraint with alignment info, use
 * ped_device_get_minimal_aligned_constraint() or
 * ped_device_get_optimal_aligned_constraint().
 *
 * \return NULL on error, otherwise a pointer to a dynamically allocated
 *         constraint.
 */
PedConstraint*
ped_device_get_constraint (const PedDevice* dev)
{
        PedGeometry *s, *e;
        PedConstraint* c = ped_constraint_new (
                                ped_alignment_any, ped_alignment_any,
                                s = ped_geometry_new (dev, 0, dev->length),
                                e = ped_geometry_new (dev, 0, dev->length),
                                1, dev->length);

        free (s);
        free (e);
        return c;
}

static PedConstraint*
_ped_device_get_aligned_constraint(const PedDevice *dev,
                                   PedAlignment* start_align)
{
        PedAlignment *end_align = NULL;
        PedGeometry *whole_dev_geom = NULL;
        PedConstraint *c = NULL;

        if (start_align) {
                end_align = ped_alignment_new(start_align->offset - 1,
                                              start_align->grain_size);
                if (!end_align)
                        goto free_start_align;
        }

        whole_dev_geom = ped_geometry_new (dev, 0, dev->length);

        if (start_align)
                c =  ped_constraint_new (start_align, end_align,
                                         whole_dev_geom, whole_dev_geom,
                                         1, dev->length);
        else
                c =  ped_constraint_new (ped_alignment_any, ped_alignment_any,
                                         whole_dev_geom, whole_dev_geom,
                                         1, dev->length);

        free (whole_dev_geom);
        free (end_align);
free_start_align:
        free (start_align);
        return c;
}

/**
 * Get a constraint that represents hardware requirements on geometry and
 * alignment.
 *
 * This function will return a constraint representing the limits imposed
 * by the size of the disk and the minimal alignment requirements for proper
 * performance of the disk.
 *
 * \return NULL on error, otherwise a pointer to a dynamically allocated
 *         constraint.
 */
PedConstraint*
ped_device_get_minimal_aligned_constraint(const PedDevice *dev)
{
        return _ped_device_get_aligned_constraint(dev,
                                         ped_device_get_minimum_alignment(dev));
}

/**
 * Get a constraint that represents hardware requirements on geometry and
 * alignment.
 *
 * This function will return a constraint representing the limits imposed
 * by the size of the disk and the alignment requirements for optimal
 * performance of the disk.
 *
 * \return NULL on error, otherwise a pointer to a dynamically allocated
 *         constraint.
 */
PedConstraint*
ped_device_get_optimal_aligned_constraint(const PedDevice *dev)
{
        return _ped_device_get_aligned_constraint(dev,
                                         ped_device_get_optimum_alignment(dev));
}

/**
 * Get an alignment that represents minimum hardware requirements on alignment.
 * When for example using media that has a physical sector size that is a
 * multiple of the logical sector size, it is desirable to have disk accesses
 * (and thus partitions) properly aligned. Having partitions not aligned to
 * the minimum hardware requirements may lead to a performance penalty.
 *
 * The returned alignment describes the alignment for the start sector of the
 * partition, the end sector should be aligned too, to get the end sector
 * alignment decrease the returned alignment's offset by 1.
 *
 * \return the minimum alignment of partition start sectors, or NULL if this
 *         information is not available.
 */
PedAlignment*
ped_device_get_minimum_alignment(const PedDevice *dev)
{
        PedAlignment *align = NULL;

        if (ped_architecture->dev_ops->get_minimum_alignment)
                align = ped_architecture->dev_ops->get_minimum_alignment(dev);

        if (align == NULL)
                align = ped_alignment_new(0,
                                dev->phys_sector_size / dev->sector_size);

        return align;
}

/**
 * Get an alignment that represents the hardware requirements for optimal
 * performance.
 *
 * The returned alignment describes the alignment for the start sector of the
 * partition, the end sector should be aligned too, to get the end sector
 * alignment decrease the returned alignment's offset by 1.
 *
 * \return the optimal alignment of partition start sectors, or NULL if this
 *         information is not available.
 */
PedAlignment*
ped_device_get_optimum_alignment(const PedDevice *dev)
{
        PedAlignment *align = NULL;

        if (ped_architecture->dev_ops->get_optimum_alignment)
                align = ped_architecture->dev_ops->get_optimum_alignment(dev);

        /* If the arch specific code could not give as an alignment
           return a default value based on the type of device. */
        if (align == NULL) {
                switch (dev->type) {
                case PED_DEVICE_DASD:
                        align = ped_device_get_minimum_alignment(dev);
                        break;
                default:
                        /* Align to a grain of 1MiB (like vista / win7) */
                        align = ped_alignment_new(0,
                                                  (PED_DEFAULT_ALIGNMENT
						   / dev->sector_size));
                }
        }

        return align;
}

/** @} */
