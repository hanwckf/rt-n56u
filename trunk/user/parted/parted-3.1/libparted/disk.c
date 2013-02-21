 /*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2003, 2005, 2007-2012 Free Software Foundation, Inc.

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

/** \file disk.c */

/**
 * \addtogroup PedDisk
 *
 * \brief Disk label access.
 *
 * Most programs will need to use ped_disk_new() or ped_disk_new_fresh() to get
 * anything done.  A PedDisk is always associated with a device and has a
 * partition table.  There are different types of partition tables (or disk
 * labels).  These are represented by the PedDiskType enumeration.
 *
 * @{
 */

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <stdbool.h>

#include "architecture.h"
#include "labels/pt-tools.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#  define N_(String) (String)
#else
#  define _(String) (String)
#  define N_(String) (String)
#endif /* ENABLE_NLS */

/* UPDATE MODE functions */
#ifdef DEBUG
static int _disk_check_sanity (PedDisk* disk);
#endif
static int _disk_push_update_mode (PedDisk* disk);
static int _disk_pop_update_mode (PedDisk* disk);
static int _disk_raw_insert_before (PedDisk* disk, PedPartition* loc,
				    PedPartition* part);
static int _disk_raw_insert_after (PedDisk* disk, PedPartition* loc,
				   PedPartition* part);
static int _disk_raw_remove (PedDisk* disk, PedPartition* part);
static int _disk_raw_add (PedDisk* disk, PedPartition* part);

static PedDiskType*	disk_types = NULL;

void
ped_disk_type_register (PedDiskType* disk_type)
{
	PED_ASSERT (disk_type != NULL);
	PED_ASSERT (disk_type->ops != NULL);
	PED_ASSERT (disk_type->name != NULL);

        disk_type->next = disk_types;
        disk_types =  disk_type;
}

void
ped_disk_type_unregister (PedDiskType* disk_type)
{
	PedDiskType*	walk;
	PedDiskType*	last = NULL;

	PED_ASSERT (disk_types != NULL);
	PED_ASSERT (disk_type != NULL);

	for (walk = disk_types; walk && walk != disk_type;
                last = walk, walk = walk->next);

	PED_ASSERT (walk != NULL);
	if (last)
		((struct _PedDiskType*) last)->next = disk_type->next;
	else
		disk_types = disk_type->next;
}

/**
 * Return the next disk type registers, after "type".  If "type" is
 * NULL, returns the first disk type.
 *
 * \return Next disk; NULL if "type" is the last registered disk type.
 */
PedDiskType*
ped_disk_type_get_next (PedDiskType const *type)
{
	if (type)
		return type->next;
	else
		return disk_types;
}

/**
 * Return the disk type with a name of "name".
 *
 * \return Disk type; NULL if no match.
 */
PedDiskType*
ped_disk_type_get (const char* name)
{
	PedDiskType*	walk = NULL;

	PED_ASSERT (name != NULL);

	for (walk = ped_disk_type_get_next (NULL); walk;
	     walk = ped_disk_type_get_next (walk))
			if (strcasecmp (walk->name, name) == 0)
					break;

	return walk;
}

/**
 * Return the type of partition table detected on "dev".
 *
 * \return Type; NULL if none was detected.
 */
PedDiskType*
ped_disk_probe (PedDevice* dev)
{
        PedDiskType* walk = NULL;

        PED_ASSERT (dev != NULL);

        if (!ped_device_open (dev))
                return NULL;

        ped_exception_fetch_all ();
        for (walk = ped_disk_type_get_next (NULL); walk;
             walk = ped_disk_type_get_next (walk))
          {
                if (getenv ("PARTED_DEBUG")) {
                        fprintf (stderr, "probe label: %s\n",
                                 walk->name);
                        fflush (stderr);
                }
                if (walk->ops->probe (dev))
                        break;
          }

        if (ped_exception)
                ped_exception_catch ();
        ped_exception_leave_all ();

        ped_device_close (dev);
        return walk;
}

/**
 * Read the partition table off a device (if one is found).
 *
 * \warning May modify \p dev->cylinders, \p dev->heads and \p dev->sectors
 *      if the partition table indicates that the existing values
 *      are incorrect.
 *
 * \return A new \link _PedDisk PedDisk \endlink object;
 *         NULL on failure (e.g. partition table not detected).
 */
PedDisk*
ped_disk_new (PedDevice* dev)
{
	PedDiskType*	type;
	PedDisk*	disk;

	PED_ASSERT (dev != NULL);

	if (!ped_device_open (dev))
		goto error;

	type = ped_disk_probe (dev);
	if (!type) {
		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("%s: unrecognised disk label"),
			dev->path);
		goto error_close_dev;
	}
	disk = ped_disk_new_fresh (dev, type);
	if (!disk)
		goto error_close_dev;
	if (!type->ops->read (disk))
		goto error_destroy_disk;
	disk->needs_clobber = 0;
	ped_device_close (dev);
	return disk;

error_destroy_disk:
	ped_disk_destroy (disk);
error_close_dev:
	ped_device_close (dev);
error:
	return NULL;
}

static int
_add_duplicate_part (PedDisk* disk, PedPartition* old_part)
{
	PedPartition*	new_part;
	int ret;

	new_part = disk->type->ops->partition_duplicate (old_part);
	if (!new_part)
		goto error;
	new_part->disk = disk;

	if (!_disk_push_update_mode (disk))
		goto error_destroy_new_part;
	ret = _disk_raw_add (disk, new_part);
	if (!_disk_pop_update_mode (disk))
		goto error_destroy_new_part;
	if (!ret)
		goto error_destroy_new_part;
#ifdef DEBUG
	if (!_disk_check_sanity (disk))
		goto error_destroy_new_part;
#endif
	return 1;

error_destroy_new_part:
	ped_partition_destroy (new_part);
error:
	return 0;
}

/**
 * Clone a \link _PedDisk PedDisk \endlink object.
 *
 * \return Deep copy of \p old_disk, NULL on failure.
 */
PedDisk*
ped_disk_duplicate (const PedDisk* old_disk)
{
	PedDisk*	new_disk;
	PedPartition*	old_part;

	PED_ASSERT (old_disk != NULL);
	PED_ASSERT (!old_disk->update_mode);
	PED_ASSERT (old_disk->type->ops->duplicate != NULL);
	PED_ASSERT (old_disk->type->ops->partition_duplicate != NULL);

	new_disk = old_disk->type->ops->duplicate (old_disk);
	if (!new_disk)
		goto error;

	if (!_disk_push_update_mode (new_disk))
		goto error_destroy_new_disk;
	for (old_part = ped_disk_next_partition (old_disk, NULL); old_part;
	     old_part = ped_disk_next_partition (old_disk, old_part)) {
		if (ped_partition_is_active (old_part)) {
			if (!_add_duplicate_part (new_disk, old_part)){
				_disk_pop_update_mode (new_disk);
				goto error_destroy_new_disk;
			}
		}
	}
	if (!_disk_pop_update_mode (new_disk))
		goto error_destroy_new_disk;

        new_disk->needs_clobber = old_disk->needs_clobber;

	return new_disk;

error_destroy_new_disk:
	ped_disk_destroy (new_disk);
error:
	return NULL;
}

/* Given a partition table type NAME, e.g., "gpt", return its PedDiskType
   handle.  If no known type has a name matching NAME, return NULL.  */
static PedDiskType const * _GL_ATTRIBUTE_PURE
find_disk_type (char const *name)
{
  PedDiskType const *t;
  for (t = ped_disk_type_get_next (NULL); t; t = ped_disk_type_get_next (t))
    {
      if (strcmp (t->name, name) == 0)
        return t;
    }
  return NULL;
}

/**
 * Remove all identifying signatures of a partition table,
 *
 * \return 0 on error, 1 otherwise.
 *
 * \sa ped_disk_clobber()
 */
int
ped_disk_clobber (PedDevice* dev)
{
	PED_ASSERT (dev != NULL);

	if (!ped_device_open (dev))
		goto error;

        PedDiskType const *gpt = find_disk_type ("gpt");
	PED_ASSERT (gpt != NULL);

        /* If there is a GPT table, don't clobber the protective MBR.  */
        bool is_gpt = gpt->ops->probe (dev);
        PedSector first_sector = (is_gpt ? 1 : 0);

	/* How many sectors to zero out at each end.
	   This must be large enough to zero out the magic bytes
	   starting at offset 8KiB on a DASD partition table.
	   Doing the same from the end of the disk is probably
	   overkill, but at least on GPT, we do need to zero out
	   the final sector.  */
	const PedSector n_sectors = 9 * 1024 / dev->sector_size + 1;

	/* Clear the first few.  */
	PedSector n = n_sectors;
	if (dev->length < first_sector + n_sectors)
	  n = dev->length - first_sector;
        if (!ptt_clear_sectors (dev, first_sector, n))
          goto error_close_dev;

	/* Clear the last few.  */
	PedSector t = (dev->length -
		       (n_sectors < dev->length ? n_sectors : 1));

        /* Don't clobber the pMBR if we have a pathologically small disk.  */
        if (t < first_sector)
          t = first_sector;
        if (!ptt_clear_sectors (dev, t, dev->length - t))
          goto error_close_dev;

	ped_device_close (dev);
	return 1;

error_close_dev:
	ped_device_close (dev);
error:
	return 0;
}

/**
 * Create a new partition table on \p dev.
 *
 * This new partition table is only created in-memory, and nothing is written
 * to disk until ped_disk_commit_to_dev() is called.
 *
 * \return The newly constructed \link _PedDisk PedDisk \endlink,
 *      NULL on failure.
 */
PedDisk*
ped_disk_new_fresh (PedDevice* dev, const PedDiskType* type)
{
	PedDisk*	disk;

	PED_ASSERT (dev != NULL);
	PED_ASSERT (type != NULL);
	PED_ASSERT (type->ops->alloc != NULL);
	PedCHSGeometry*	bios_geom = &dev->bios_geom;
	PED_ASSERT (bios_geom->sectors != 0);
	PED_ASSERT (bios_geom->heads != 0);

	disk = type->ops->alloc (dev);
	if (!disk)
       		goto error;
        if (!_disk_pop_update_mode (disk))
                goto error_destroy_disk;
	PED_ASSERT (disk->update_mode == 0);

	disk->needs_clobber = 1;
	return disk;

error_destroy_disk:
        ped_disk_destroy (disk);
error:
	return NULL;
}

PedDisk*
_ped_disk_alloc (const PedDevice* dev, const PedDiskType* disk_type)
{
	PedDisk*	disk;

	disk = (PedDisk*) ped_malloc (sizeof (PedDisk));
	if (!disk)
		goto error;

	disk->dev = (PedDevice*)dev;
	disk->type = disk_type;
	disk->update_mode = 1;
	disk->part_list = NULL;
	return disk;

error:
	return NULL;
}

void
_ped_disk_free (PedDisk* disk)
{
	_disk_push_update_mode (disk);
	ped_disk_delete_all (disk);
	free (disk);
}

/**
 * Close \p disk.
 *
 * What this function does depends on the PedDiskType of \p disk,
 * but you can generally assume that outstanding writes are flushed
 * (this mainly means that _ped_disk_free is called).
 */
void
ped_disk_destroy (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (!disk->update_mode);

	disk->type->ops->free (disk);
}

/**
 * Tell the operating system kernel about the partition table layout
 * of \p disk.
 *
 * This is rather loosely defined: for example, on old versions of Linux,
 * it simply calls the BLKRRPART ioctl, which tells the kernel to
 * reread the partition table. On newer versions (2.4.x), it will
 * use the new blkpg interface to tell Linux where each partition
 * starts/ends, etc. In this case, Linux does not need to have support for
 * a specific type of partition table.
 *
 * \return 0 on failure, 1 otherwise.
 */
int
ped_disk_commit_to_os (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);

	if (!ped_device_open (disk->dev))
		goto error;
	if (!ped_architecture->disk_ops->disk_commit (disk))
		goto error_close_dev;
	ped_device_close (disk->dev);
	return 1;

error_close_dev:
	ped_device_close (disk->dev);
error:
	return 0;
}

/**
 * Write the changes made to the in-memory description
 * of a partition table to the device.
 *
 * \return 0 on failure, 1 otherwise.
 */
int
ped_disk_commit_to_dev (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (!disk->update_mode);

	if (!disk->type->ops->write) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("This libparted doesn't have write support for "
			  "%s.  Perhaps it was compiled read-only."),
			disk->type->name);
		goto error;
	}

	if (!ped_device_open (disk->dev))
		goto error;

	if (disk->needs_clobber) {
		if (!ped_disk_clobber (disk->dev))
			goto error_close_dev;
		disk->needs_clobber = 0;
	}
	if (!disk->type->ops->write (disk))
		goto error_close_dev;
	ped_device_close (disk->dev);
	return 1;

error_close_dev:
	ped_device_close (disk->dev);
error:
	return 0;
}

/*
 * This function writes the in-memory changes to a partition table to
 * disk and informs the operating system of the changes.
 *
 * \note Equivalent to calling first ped_disk_commit_to_dev(), then
 *      ped_disk_commit_to_os().
 *
 * \return 0 on failure, 1 otherwise.
 */
int
ped_disk_commit (PedDisk* disk)
{
        /* Open the device here, so that the underlying fd is not closed
           between commit_to_dev and commit_to_os (closing causes unwanted
           udev events to be sent under Linux). */
	if (!ped_device_open (disk->dev))
		goto error;

	if (!ped_disk_commit_to_dev (disk))
		goto error_close_dev;

	if (!ped_disk_commit_to_os (disk))
		goto error_close_dev;

	ped_device_close (disk->dev);
	return 1;

error_close_dev:
	ped_device_close (disk->dev);
error:
	return 0;
}

/**
 * \addtogroup PedPartition
 *
 * @{
 */

/**
 * Check whether a partition is mounted or busy in some
 * other way.
 *
 * \note An extended partition is busy if any logical partitions are mounted.
 *
 * \return \c 1 if busy.
 */
int
ped_partition_is_busy (const PedPartition* part)
{
	PED_ASSERT (part != NULL);

	return ped_architecture->disk_ops->partition_is_busy (part);
}

/**
 * Return a path that can be used to address the partition in the
 * operating system.
 */
char*
ped_partition_get_path (const PedPartition* part)
{
	PED_ASSERT (part != NULL);

	return ped_architecture->disk_ops->partition_get_path (part);
}

/** @} */

/**
 * \addtogroup PedDisk
 *
 * @{
 */

/**
 * Perform a sanity check on a partition table.
 *
 * \note The check performed is generic (i.e. it does not depends on the label
 *      type of the disk.
 *
 * \throws PED_EXCEPTION_WARNING if a partition type ID does not match the file
 *      system on it.
 *
 * \return 0 if the check fails, 1 otherwise.
 */
int
ped_disk_check (const PedDisk* disk)
{
	PedPartition*	walk;

	PED_ASSERT (disk != NULL);

	for (walk = disk->part_list; walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		const PedFileSystemType*	fs_type = walk->fs_type;
		PedGeometry*			geom;
		PedSector			length_error;
		PedSector			max_length_error;

		if (!ped_partition_is_active (walk) || !fs_type)
			continue;

		geom = ped_file_system_probe_specific (fs_type, &walk->geom);
		if (!geom)
			continue;

		length_error = abs (walk->geom.length - geom->length);
		max_length_error = PED_MAX (4096, walk->geom.length / 100);
                bool ok = (ped_geometry_test_inside (&walk->geom, geom)
                           && length_error <= max_length_error);
                char *fs_size = ped_unit_format (disk->dev, geom->length);
                ped_geometry_destroy (geom);
                if (!ok) {
			char* part_size = ped_unit_format (disk->dev,
                                                           walk->geom.length);
			PedExceptionOption choice;
			choice = ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_IGNORE_CANCEL,
				_("Partition %d is %s, but the file system is "
				  "%s."),
				walk->num, part_size, fs_size);

			free (part_size);

			free (fs_size);
			fs_size = NULL;

			if (choice != PED_EXCEPTION_IGNORE)
				return 0;
		}
		free (fs_size);
	}

	return 1;
}

/**
 * This function checks if a particular type of partition table supports
 * a feature.
 *
 * \return 1 if \p disk_type supports \p feature, 0 otherwise.
 */
int
ped_disk_type_check_feature (const PedDiskType* disk_type,
			     PedDiskTypeFeature feature)
{
	return (disk_type->features & feature) != 0;
}

/**
 * Get the number of primary partitions.
 */
int
ped_disk_get_primary_partition_count (const PedDisk* disk)
{
	PedPartition*	walk;
	int		count = 0;

	PED_ASSERT (disk != NULL);

	for (walk = disk->part_list; walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		if (ped_partition_is_active (walk)
				&& ! (walk->type & PED_PARTITION_LOGICAL))
			count++;
	}

	return count;
}

/**
 * Get the highest available partition number on \p disk.
 */
int
ped_disk_get_last_partition_num (const PedDisk* disk)
{
	PedPartition*	walk;
	int		highest = -1;

	PED_ASSERT (disk != NULL);

	for (walk = disk->part_list; walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		if (walk->num > highest)
			highest = walk->num;
	}

	return highest;
}

/**
 * Get the highest supported partition number on \p disk.
 *
 * \return 0 if call fails. 1 otherwise.
 */
bool
ped_disk_get_max_supported_partition_count(const PedDisk* disk, int* supported)
{
	PED_ASSERT(disk != NULL);
	PED_ASSERT(disk->type->ops->get_max_supported_partition_count != NULL);

	return disk->type->ops->get_max_supported_partition_count(disk, supported);
}

/**
 * Get the alignment needed for partition boundaries on this disk.
 * The returned alignment describes the alignment for the start sector of the
 * partition, for all disklabel types which require alignment, except Sun
 * disklabels, the end sector must be aligned too. To get the end sector
 * alignment decrease the PedAlignment offset by 1.
 *
 * \return NULL on error, otherwise a pointer to a dynamically allocated
 *         alignment.
 */
PedAlignment*
ped_disk_get_partition_alignment(const PedDisk *disk)
{
        /* disklabel handlers which don't need alignment don't define this */
        if (!disk->type->ops->get_partition_alignment)
                return ped_alignment_duplicate(ped_alignment_any);

        return disk->type->ops->get_partition_alignment(disk);
}

/**
 * Get the maximum number of (primary) partitions the disk label supports.
 *
 * For example, MacIntosh partition maps can have different sizes,
 * and accordingly support a different number of partitions.
 */
int
ped_disk_get_max_primary_partition_count (const PedDisk* disk)
{
	PED_ASSERT (disk->type != NULL);
	PED_ASSERT (disk->type->ops->get_max_primary_partition_count != NULL);

	return disk->type->ops->get_max_primary_partition_count (disk);
}

/**
 * Set the state (\c 1 or \c 0) of a flag on a disk.
 *
 * \note It is an error to call this on an unavailable flag -- use
 * ped_disk_is_flag_available() to determine which flags are available
 * for a given disk label.
 *
 * \throws PED_EXCEPTION_ERROR if the requested flag is not available for this
 *      label.
 */
int
ped_disk_set_flag(PedDisk *disk, PedDiskFlag flag, int state)
{
        int ret;

        PED_ASSERT (disk != NULL);

        PedDiskOps *ops = disk->type->ops;

        if (!_disk_push_update_mode(disk))
                return 0;

        if (!ped_disk_is_flag_available(disk, flag)) {
                ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_CANCEL,
                        "The flag '%s' is not available for %s disk labels.",
                        ped_disk_flag_get_name(flag),
                        disk->type->name);
                _disk_pop_update_mode(disk);
                return 0;
        }

        ret = ops->disk_set_flag(disk, flag, state);

        if (!_disk_pop_update_mode (disk))
                return 0;

        return ret;
}

/**
 * Get the state (\c 1 or \c 0) of a flag on a disk.
 */
int
ped_disk_get_flag(const PedDisk *disk, PedDiskFlag flag)
{
        PED_ASSERT (disk != NULL);

        PedDiskOps *ops = disk->type->ops;

        if (!ped_disk_is_flag_available(disk, flag))
                return 0;

        return ops->disk_get_flag(disk, flag);
}

/**
 * Check whether a given flag is available on a disk.
 *
 * \return \c 1 if the flag is available.
 */
int
ped_disk_is_flag_available(const PedDisk *disk, PedDiskFlag flag)
{
        PED_ASSERT (disk != NULL);

        PedDiskOps *ops = disk->type->ops;

        if (!ops->disk_is_flag_available)
                return 0;

        return ops->disk_is_flag_available(disk, flag);
}

/**
 * Returns a name for a \p flag, e.g. PED_DISK_CYLINDER_ALIGNMENT will return
 * "cylinder_alignment".
 *
 * \note The returned string will be in English.  However,
 * translations are provided, so the caller can call
 * dgettext("parted", RESULT) on the result.
 */
const char *
ped_disk_flag_get_name(PedDiskFlag flag)
{
        switch (flag) {
        case PED_DISK_CYLINDER_ALIGNMENT:
                return N_("cylinder_alignment");
        case PED_DISK_GPT_PMBR_BOOT:
                return N_("pmbr_boot");
        default:
                ped_exception_throw (
                        PED_EXCEPTION_BUG,
                        PED_EXCEPTION_CANCEL,
                        _("Unknown disk flag, %d."),
                        flag);
                return NULL;
        }
}

/**
 * Returns the flag associated with \p name.
 *
 * \p name can be the English
 * string, or the translation for the native language.
 */
PedDiskFlag
ped_disk_flag_get_by_name(const char *name)
{
        PedDiskFlag flag;

        for (flag = ped_disk_flag_next(0); flag;
             flag = ped_disk_flag_next(flag)) {
                const char *flag_name = ped_disk_flag_get_name(flag);
                if (strcasecmp(name, flag_name) == 0
                    || strcasecmp(name, _(flag_name)) == 0)
                        return flag;
        }

        return 0;
}

/**
 * Iterates through all disk flags.
 *
 * ped_disk_flag_next(0) returns the first flag
 *
 * \return the next flag, or 0 if there are no more flags
 */
PedDiskFlag
ped_disk_flag_next(PedDiskFlag flag)
{
        return (flag + 1) % (PED_DISK_LAST_FLAG + 1);
}

/**
 * \internal We turned a really nasty bureaucracy problem into an elegant maths
 * problem :-)  Basically, there are some constraints to a partition's
 * geometry:
 *
 * (1) it must start and end on a "disk" block, determined by the disk label
 * (not the hardware).  (constraint represented by a PedAlignment)
 *
 * (2) if we're resizing a partition, we MIGHT need to keep each block aligned.
 * Eg: if an ext2 file system has 4k blocks, then we can only move the start
 * by a multiple of 4k.  (constraint represented by a PedAlignment)
 *
 * (3) we need to keep the start and end within the device's physical
 * boundaries.  (constraint represented by a PedGeometry)
 *
 * Satisfying (1) and (2) simultaneously required a bit of fancy maths ;-)  See
 * ped_alignment_intersect()
 *
 * The application of these constraints is in disk_*.c's *_partition_align()
 * function.
 */
static int
_partition_align (PedPartition* part, const PedConstraint* constraint)
{
	const PedDiskType*	disk_type;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->num != -1);
	PED_ASSERT (part->disk != NULL);
	disk_type = part->disk->type;
	PED_ASSERT (disk_type != NULL);
	PED_ASSERT (disk_type->ops->partition_align != NULL);
	PED_ASSERT (part->disk->update_mode);

	return disk_type->ops->partition_align (part, constraint);
}

static int
_partition_enumerate (PedPartition* part)
{
	const PedDiskType*	disk_type;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	disk_type = part->disk->type;
	PED_ASSERT (disk_type != NULL);
	PED_ASSERT (disk_type->ops->partition_enumerate != NULL);

	return disk_type->ops->partition_enumerate (part);
}

/**
 * Gives all the (active) partitions a number.  It should preserve the numbers
 * and orders as much as possible.
 */
static int
ped_disk_enumerate_partitions (PedDisk* disk)
{
	PedPartition*	walk;
	int		i;
	int		end;

	PED_ASSERT (disk != NULL);

/* first "sort" already-numbered partitions.  (e.g. if a logical partition
 * is removed, then all logical partitions that were number higher MUST be
 * renumbered)
 */
	end = ped_disk_get_last_partition_num (disk);
	for (i=1; i<=end; i++) {
		walk = ped_disk_get_partition (disk, i);
		if (walk) {
			if (!_partition_enumerate (walk))
				return 0;
		}
	}

/* now, number un-numbered partitions */
	for (walk = disk->part_list; walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		if (ped_partition_is_active (walk) && walk->num == -1) {
			if (!_partition_enumerate (walk))
				return 0;
		}
	}

	return 1;
}

static int
_disk_remove_metadata (PedDisk* disk)
{
	PedPartition*	walk = NULL;
	PedPartition*	next;

	PED_ASSERT (disk != NULL);

	next = ped_disk_next_partition (disk, walk);

	while (next) {
		walk = next;
		while (1) {
			next = ped_disk_next_partition (disk, next);
			if (!next || next->type & PED_PARTITION_METADATA)
				break;
		}
		if (walk->type & PED_PARTITION_METADATA)
			ped_disk_delete_partition (disk, walk);
	}
	return 1;
}

static int
_disk_alloc_metadata (PedDisk* disk)
{
	PED_ASSERT (disk != NULL);

	if (!disk->update_mode)
		_disk_remove_metadata (disk);

	return disk->type->ops->alloc_metadata (disk);
}

static int
_disk_remove_freespace (PedDisk* disk)
{
	PedPartition*	walk;
	PedPartition*	next;

	walk = ped_disk_next_partition (disk, NULL);
	for (; walk; walk = next) {
		next = ped_disk_next_partition (disk, walk);

		if (walk->type & PED_PARTITION_FREESPACE) {
			_disk_raw_remove (disk, walk);
			ped_partition_destroy (walk);
		}
	}

	return 1;
}

static int
_alloc_extended_freespace (PedDisk* disk)
{
	PedSector	last_end;
	PedPartition*	walk;
	PedPartition*	last;
	PedPartition*	free_space;
	PedPartition*	extended_part;

	extended_part = ped_disk_extended_partition (disk);
	if (!extended_part)
		return 1;

	last_end = extended_part->geom.start;
	last = NULL;

	for (walk = extended_part->part_list; walk; walk = walk->next) {
		if (walk->geom.start > last_end + 1) {
			free_space = ped_partition_new (
					disk,
					PED_PARTITION_FREESPACE
						| PED_PARTITION_LOGICAL,
					NULL,
					last_end + 1, walk->geom.start - 1);
			_disk_raw_insert_before (disk, walk, free_space);
		}

		last = walk;
		last_end = last->geom.end;
	}

	if (last_end < extended_part->geom.end) {
		free_space = ped_partition_new (
				disk,
				PED_PARTITION_FREESPACE | PED_PARTITION_LOGICAL,
				NULL,
				last_end + 1, extended_part->geom.end);

		if (last)
			return _disk_raw_insert_after (disk, last, free_space);
		else
			extended_part->part_list = free_space;
	}

	return 1;
}

static int
_disk_alloc_freespace (PedDisk* disk)
{
	PedSector	last_end;
	PedPartition*	walk;
	PedPartition*	last;
	PedPartition*	free_space;

	if (!_disk_remove_freespace (disk))
		return 0;
	if (!_alloc_extended_freespace (disk))
		return 0;

	last = NULL;
	last_end = -1;

	for (walk = disk->part_list; walk; walk = walk->next) {
		if (walk->geom.start > last_end + 1) {
			free_space = ped_partition_new (disk,
					PED_PARTITION_FREESPACE, NULL,
					last_end + 1, walk->geom.start - 1);
			_disk_raw_insert_before (disk, walk, free_space);
		}

		last = walk;
		last_end = last->geom.end;
	}

	if (last_end < disk->dev->length - 1) {
		free_space = ped_partition_new (disk,
					PED_PARTITION_FREESPACE, NULL,
					last_end + 1, disk->dev->length - 1);
		if (last)
			return _disk_raw_insert_after (disk, last, free_space);
		else
			disk->part_list = free_space;
	}

	return 1;
}

/**
 * Update mode: used when updating the internal representation of the partition
 * table.  In update mode, the metadata and freespace placeholder/virtual
 * partitions are removed, making it much easier for various manipulation
 * routines...
 */
static int
_disk_push_update_mode (PedDisk* disk)
{
	if (!disk->update_mode) {
#ifdef DEBUG
		if (!_disk_check_sanity (disk))
			return 0;
#endif

		_disk_remove_freespace (disk);
		disk->update_mode++;
		_disk_remove_metadata (disk);

#ifdef DEBUG
		if (!_disk_check_sanity (disk))
			return 0;
#endif
	} else {
		disk->update_mode++;
	}
	return 1;
}

static int
_disk_pop_update_mode (PedDisk* disk)
{
	PED_ASSERT (disk->update_mode);

	if (disk->update_mode == 1) {
	/* re-allocate metadata BEFORE leaving update mode, to prevent infinite
	 * recursion (metadata allocation requires update mode)
	 */
#ifdef DEBUG
		if (!_disk_check_sanity (disk))
			return 0;
#endif

		_disk_alloc_metadata (disk);
		disk->update_mode--;
		_disk_alloc_freespace (disk);

#ifdef DEBUG
		if (!_disk_check_sanity (disk))
			return 0;
#endif
	} else {
		disk->update_mode--;
	}
	return 1;
}

/** @} */

/**
 * \addtogroup PedPartition
 *
 * \brief Partition access.
 *
 * @{
 */

PedPartition*
_ped_partition_alloc (const PedDisk* disk, PedPartitionType type,
		      const PedFileSystemType* fs_type,
		      PedSector start, PedSector end)
{
	PedPartition*	part;

	PED_ASSERT (disk != NULL);

	part = (PedPartition*) ped_malloc (sizeof (PedPartition));
	if (!part)
		goto error;

	part->prev = NULL;
	part->next = NULL;

	part->disk = (PedDisk*) disk;
	if (!ped_geometry_init (&part->geom, disk->dev, start, end - start + 1))
		goto error_free_part;

	part->num = -1;
	part->type = type;
	part->part_list = NULL;
	part->fs_type = fs_type;

	return part;

error_free_part:
	free (part);
error:
	return NULL;
}

void
_ped_partition_free (PedPartition* part)
{
	free (part);
}

int
_ped_partition_attempt_align (PedPartition* part,
			      const PedConstraint* external,
			      PedConstraint* internal)
{
	PedConstraint*		intersection;
	PedGeometry*		solution;

	intersection = ped_constraint_intersect (external, internal);
	ped_constraint_destroy (internal);
	if (!intersection)
		goto fail;

	solution = ped_constraint_solve_nearest (intersection, &part->geom);
	if (!solution)
		goto fail_free_intersection;
	ped_geometry_set (&part->geom, solution->start, solution->length);
	ped_geometry_destroy (solution);
	ped_constraint_destroy (intersection);
	return 1;

fail_free_intersection:
	ped_constraint_destroy (intersection);
fail:
	return 0;
}

/**
 * Create a new \link _PedPartition PedPartition \endlink on \p disk.
 *
 * \param type One of \p PED_PARTITION_NORMAL, \p PED_PARTITION_EXTENDED,
 *      \p PED_PARTITION_LOGICAL.
 *
 * \note The constructed partition is not added to <tt>disk</tt>'s
 *      partition table. Use ped_disk_add_partition() to do this.
 *
 * \return A new \link _PedPartition PedPartition \endlink object,
 *      NULL on failure.
 *
 * \throws PED_EXCEPTION_ERROR if \p type is \p EXTENDED or \p LOGICAL but the
 *      label does not support this concept.
 */
PedPartition*
ped_partition_new (const PedDisk* disk, PedPartitionType type,
		   const PedFileSystemType* fs_type, PedSector start,
		   PedSector end)
{
	int		supports_extended;
	PedPartition*	part;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (disk->type->ops->partition_new != NULL);

	supports_extended = ped_disk_type_check_feature (disk->type,
			    	PED_DISK_TYPE_EXTENDED);

	if (!supports_extended
	    && (type == PED_PARTITION_EXTENDED
			|| type == PED_PARTITION_LOGICAL)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s disk labels do not support extended "
			  "partitions."),
			disk->type->name);
		goto error;
	}

	part = disk->type->ops->partition_new (disk, type, fs_type, start, end);
	if (!part)
		goto error;

	if (fs_type || part->type == PED_PARTITION_EXTENDED) {
		if (!ped_partition_set_system (part, fs_type))
			goto error_destroy_part;
	}
	return part;

error_destroy_part:
	ped_partition_destroy (part);
error:
	return NULL;
}

/**
 * Destroy a \link _PedPartition PedPartition \endlink object.
 *
 * \note Should not be called on a partition that is in a partition table.
 *      Use ped_disk_delete_partition() instead.
 */
void
ped_partition_destroy (PedPartition* part)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (part->disk->type->ops->partition_new != NULL);

	part->disk->type->ops->partition_destroy (part);
}


/**
 * Return whether or not the partition is "active".
 *
 * A partition is active if \p part->type is neither \p PED_PARTITION_METADATA
 * nor \p PED_PARTITION_FREE.
 */
int
ped_partition_is_active (const PedPartition* part)
{
	PED_ASSERT (part != NULL);

	return !(part->type & PED_PARTITION_FREESPACE
		 || part->type & PED_PARTITION_METADATA);
}

/**
 * Set the state (\c 1 or \c 0) of a flag on a partition.
 *
 * Flags are disk label specific, although they have a global
 * "namespace": the flag PED_PARTITION_BOOT, for example, roughly means
 * "this" partition is bootable". But this means different things on different
 * disk labels (and may not be defined on some disk labels). For example,
 * on MS-DOS disk labels, there can only be one boot partition, and this
 * refers to the partition that will be booted from on startup. On PC98
 * disk labels, the user can choose from any bootable partition on startup.
 *
 * \note It is an error to call this on an unavailable flag -- use
 * ped_partition_is_flag_available() to determine which flags are available
 * for a given disk label.
 *
 * \throws PED_EXCEPTION_ERROR if the requested flag is not available for this
 *      label.
 */
int
ped_partition_set_flag (PedPartition* part, PedPartitionFlag flag, int state)
{
	PedDiskOps*	ops;

	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (ped_partition_is_active (part));

	ops = part->disk->type->ops;
	PED_ASSERT (ops->partition_set_flag != NULL);
	PED_ASSERT (ops->partition_is_flag_available != NULL);

	if (!ops->partition_is_flag_available (part, flag)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			"The flag '%s' is not available for %s disk labels.",
			ped_partition_flag_get_name (flag),
			part->disk->type->name);
		return 0;
	}

	return ops->partition_set_flag (part, flag, state);
}

/**
 * Get the state (\c 1 or \c 0) of a flag on a partition.
 *
 * See ped_partition_set_flag() for conditions that must hold.
 *
 * \todo Where's the check for flag availability?
 */
int
ped_partition_get_flag (const PedPartition* part, PedPartitionFlag flag)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (part->disk->type->ops->partition_get_flag != NULL);
	PED_ASSERT (ped_partition_is_active (part));

	return part->disk->type->ops->partition_get_flag (part, flag);
}

/**
 * Check whether a given flag is available on a partition.
 *
 * \return \c 1 if the flag is available.
 */
int
ped_partition_is_flag_available (const PedPartition* part,
	       			 PedPartitionFlag flag)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (part->disk->type->ops->partition_is_flag_available != NULL);
	PED_ASSERT (ped_partition_is_active (part));

	return part->disk->type->ops->partition_is_flag_available (part, flag);
}

/**
 * Sets the system type on the partition to \p fs_type.
 *
 * \note The file system may be opened, to get more information about the
 * file system, e.g. to determine if it's FAT16 or FAT32.
 *
 * \return \c 0 on failure.
 */
int
ped_partition_set_system (PedPartition* part, const PedFileSystemType* fs_type)
{
	const PedDiskType*	disk_type;

	PED_ASSERT (part != NULL);
	PED_ASSERT (ped_partition_is_active (part));
	PED_ASSERT (part->disk != NULL);
	disk_type = part->disk->type;
	PED_ASSERT (disk_type != NULL);
	PED_ASSERT (disk_type->ops != NULL);
	PED_ASSERT (disk_type->ops->partition_set_system != NULL);

	return disk_type->ops->partition_set_system (part, fs_type);
}

static int
_assert_partition_name_feature (const PedDiskType* disk_type)
{
	if (!ped_disk_type_check_feature (
			disk_type, PED_DISK_TYPE_PARTITION_NAME)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			"%s disk labels do not support partition names.",
			disk_type->name);
		return 0;
	}
	return 1;
}

/**
 * Sets the name of a partition.
 *
 * \note This will only work if the disk label supports it.
 *      You can use
 *      \code
 * ped_disk_type_check_feature (part->disk->type, PED_DISK_TYPE_PARTITION_NAME);
 *      \endcode
 *      to check whether this feature is enabled for a label.
 *
 * \note \p name will not be modified by libparted. It can be freed
 *      by the caller immediately after ped_partition_set_name() is called.
 *
 * \return \c 1 on success, \c 0 otherwise.
 */
int
ped_partition_set_name (PedPartition* part, const char* name)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (ped_partition_is_active (part));
	PED_ASSERT (name != NULL);

	if (!_assert_partition_name_feature (part->disk->type))
		return 0;

	PED_ASSERT (part->disk->type->ops->partition_set_name != NULL);
	part->disk->type->ops->partition_set_name (part, name);
	return 1;
}

/**
 * Returns the name of a partition \p part.  This will only work if the disk
 * label supports it.
 *
 * \note The returned string should not be modified.  It should
 *	not be referenced after the partition is destroyed.
 */
const char*
ped_partition_get_name (const PedPartition* part)
{
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk != NULL);
	PED_ASSERT (ped_partition_is_active (part));

	if (!_assert_partition_name_feature (part->disk->type))
		return NULL;

	PED_ASSERT (part->disk->type->ops->partition_get_name != NULL);
	return part->disk->type->ops->partition_get_name (part);
}

/** @} */

/**
 * \addtogroup PedDisk
 *
 * @{
 */

PedPartition*
ped_disk_extended_partition (const PedDisk* disk)
{
	PedPartition*		walk;

	PED_ASSERT (disk != NULL);

	for (walk = disk->part_list; walk; walk = walk->next) {
		if (walk->type == PED_PARTITION_EXTENDED)
			break;
	}
	return walk;
}

/**
 * Return the next partition after \p part on \p disk. If \p part is \c NULL,
 * return the first partition. If \p part is the last partition, returns
 * \c NULL. If \p part is an extended partition, returns the first logical
 * partition. If this is called repeatedly passing the return value as \p part,
 * a depth-first traversal is executed.
 *
 * \return The next partition, \c NULL if no more partitions left.
 */
PedPartition*
ped_disk_next_partition (const PedDisk* disk, const PedPartition* part)
{
	PED_ASSERT (disk != NULL);

	if (!part)
		return disk->part_list;
	if (part->type == PED_PARTITION_EXTENDED)
		return part->part_list ? part->part_list : part->next;
	if (part->next)
		return part->next;
	if (part->type & PED_PARTITION_LOGICAL)
		return ped_disk_extended_partition (disk)->next;
	return NULL;
}

/** @} */

#ifdef DEBUG
static int _GL_ATTRIBUTE_PURE
_disk_check_sanity (PedDisk* disk)
{
	PedPartition*	walk;

	PED_ASSERT (disk != NULL);

	for (walk = disk->part_list; walk; walk = walk->next) {
		PED_ASSERT (!(walk->type & PED_PARTITION_LOGICAL));
		PED_ASSERT (!walk->prev || walk->prev->next == walk);
	}

	if (!ped_disk_extended_partition (disk))
		return 1;

	for (walk = ped_disk_extended_partition (disk)->part_list; walk;
	     walk = walk->next) {
		PED_ASSERT (walk->type & PED_PARTITION_LOGICAL);
		if (walk->prev)
			PED_ASSERT (walk->prev->next == walk);
	}
	return 1;
}
#endif

/**
 * Returns the partition numbered \p num.
 *
 * \return \c NULL if the specified partition does not exist.
 */
PedPartition*
ped_disk_get_partition (const PedDisk* disk, int num)
{
	PedPartition*	walk;

	PED_ASSERT (disk != NULL);

	for (walk = disk->part_list; walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		if (walk->num == num && !(walk->type & PED_PARTITION_FREESPACE))
			return walk;
	}

	return NULL;
}

/**
 * Returns the partition that contains sect.  If sect lies within a logical
 * partition, then the logical partition is returned (not the extended
 * partition).
 */
PedPartition*
ped_disk_get_partition_by_sector (const PedDisk* disk, PedSector sect)
{
	PedPartition*	walk;

	PED_ASSERT (disk != NULL);

	for (walk = disk->part_list; walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		if (ped_geometry_test_sector_inside (&walk->geom, sect)
		    && walk->type != PED_PARTITION_EXTENDED)
			return walk;
	}

	/* should never get here, unless sect is outside of disk's useable
	 * part, or we're in "update mode", and the free space place-holders
	 * have been removed with _disk_remove_freespace()
	 */
	return NULL;
}

/**
 * Return the maximum representable length (in sectors) of a
 * partition on disk \disk.
 */
PedSector
ped_disk_max_partition_length (const PedDisk* disk)
{
  return disk->type->ops->max_length ();
}

/**
 * Return the maximum representable start sector of a
 * partition on disk \disk.
 */
PedSector
ped_disk_max_partition_start_sector (const PedDisk* disk)
{
  return disk->type->ops->max_start_sector ();
}

/* I'm beginning to agree with Sedgewick :-/ */
static int
_disk_raw_insert_before (PedDisk* disk, PedPartition* loc, PedPartition* part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (loc != NULL);
	PED_ASSERT (part != NULL);

	part->prev = loc->prev;
	part->next = loc;
	if (part->prev) {
		part->prev->next = part;
	} else {
		if (loc->type & PED_PARTITION_LOGICAL)
			ped_disk_extended_partition (disk)->part_list = part;
		else
			disk->part_list = part;
	}
	loc->prev = part;

	return 1;
}

static int
_disk_raw_insert_after (PedDisk* disk, PedPartition* loc, PedPartition* part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (loc != NULL);
	PED_ASSERT (part != NULL);

	part->prev = loc;
	part->next = loc->next;
	if (loc->next)
		loc->next->prev = part;
	loc->next = part;

	return 1;
}

static int
_disk_raw_remove (PedDisk* disk, PedPartition* part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (part != NULL);

	if (part->prev) {
		part->prev->next = part->next;
		if (part->next)
			part->next->prev = part->prev;
	} else {
		if (part->type & PED_PARTITION_LOGICAL) {
			ped_disk_extended_partition (disk)->part_list
				= part->next;
		} else {
			disk->part_list = part->next;
		}
		if (part->next)
			part->next->prev = NULL;
	}

	return 1;
}

/*
 *UPDATE MODE ONLY
 */
static int
_disk_raw_add (PedDisk* disk, PedPartition* part)
{
	PedPartition*	walk;
	PedPartition*	last;
	PedPartition*	ext_part;

	PED_ASSERT (disk->update_mode);

	ext_part = ped_disk_extended_partition (disk);

	last = NULL;
	walk = (part->type & PED_PARTITION_LOGICAL) ?
			ext_part->part_list : disk->part_list;

	for (; walk; last = walk, walk = walk->next) {
		if (walk->geom.start > part->geom.end)
			break;
	}

	if (walk) {
		return _disk_raw_insert_before (disk, walk, part);
	} else {
		if (last) {
			return _disk_raw_insert_after (disk, last, part);
		} else {
			if (part->type & PED_PARTITION_LOGICAL)
				ext_part->part_list = part;
			else
				disk->part_list = part;
		}
	}

	return 1;
}

static PedConstraint*
_partition_get_overlap_constraint (PedPartition* part, PedGeometry* geom)
{
	PedSector	min_start;
	PedSector	max_end;
	PedPartition*	walk;
	PedGeometry	free_space;

	PED_ASSERT (part->disk->update_mode);
	PED_ASSERT (part->geom.dev == geom->dev);

	if (part->type & PED_PARTITION_LOGICAL) {
		PedPartition* ext_part;

		ext_part = ped_disk_extended_partition (part->disk);
		PED_ASSERT (ext_part != NULL);

		min_start = ext_part->geom.start;
		max_end = ext_part->geom.end;
		walk = ext_part->part_list;
	} else {
		min_start = 0;
		max_end = part->disk->dev->length - 1;
		walk = part->disk->part_list;
	}

	while (walk != NULL
	       && (walk->geom.start < geom->start
			    || min_start >= walk->geom.start)) {
		if (walk != part)
			min_start = walk->geom.end + 1;
		walk = walk->next;
	}

	if (walk == part)
		walk = walk->next;

	if (walk)
		max_end = walk->geom.start - 1;

	if (min_start >= max_end)
		return NULL;

	ped_geometry_init (&free_space, part->disk->dev,
			   min_start, max_end - min_start + 1);
	return ped_constraint_new_from_max (&free_space);
}

/*
 * Returns \c 0 if the partition, \p part overlaps with any partitions on the
 * \p disk.  The geometry of \p part is taken to be \p geom, NOT \p part->geom
 * (the idea here is to check if \p geom is valid, before changing \p part).
 *
 * This is useful for seeing if a resized partitions new geometry is going to
 * fit, without the existing geomtry getting in the way.
 *
 * Note: overlap with an extended partition is also allowed, provided that
 * \p geom lies completely inside the extended partition.
 */
static int _GL_ATTRIBUTE_PURE
_disk_check_part_overlaps (PedDisk* disk, PedPartition* part)
{
	PedPartition*	walk;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (part != NULL);

	for (walk = ped_disk_next_partition (disk, NULL); walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		if (walk->type & PED_PARTITION_FREESPACE)
			continue;
		if (walk == part)
			continue;
		if (part->type & PED_PARTITION_EXTENDED
		    && walk->type & PED_PARTITION_LOGICAL)
			continue;

		if (ped_geometry_test_overlap (&walk->geom, &part->geom)) {
			if (walk->type & PED_PARTITION_EXTENDED
			    && part->type & PED_PARTITION_LOGICAL
			    && ped_geometry_test_inside (&walk->geom,
							 &part->geom))
				continue;
			return 0;
		}
	}

	return 1;
}

static int
_partition_check_basic_sanity (PedDisk* disk, PedPartition* part)
{
	PedPartition*	ext_part = ped_disk_extended_partition (disk);

	PED_ASSERT (part->disk == disk);

	PED_ASSERT (part->geom.start >= 0);
	PED_ASSERT (part->geom.end < disk->dev->length);
	PED_ASSERT (part->geom.start <= part->geom.end);

	if (!ped_disk_type_check_feature (disk->type, PED_DISK_TYPE_EXTENDED)
	    && (part->type == PED_PARTITION_EXTENDED
		    || part->type == PED_PARTITION_LOGICAL)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("%s disk labels don't support logical or extended "
			  "partitions."),
			disk->type->name);
		return 0;
	}

	if (ped_partition_is_active (part)
			&& ! (part->type & PED_PARTITION_LOGICAL)) {
		if (ped_disk_get_primary_partition_count (disk) + 1
		    > ped_disk_get_max_primary_partition_count (disk)) {
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Too many primary partitions."));
			return 0;
		}
	}

	if ((part->type & PED_PARTITION_LOGICAL) && !ext_part) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't add a logical partition to %s, because "
			"there is no extended partition."),
			disk->dev->path);
		return 0;
	}

	return 1;
}

static int
_check_extended_partition (PedDisk* disk, PedPartition* part)
{
	PedPartition*		walk;
	PedPartition*		ext_part;

	PED_ASSERT (disk != NULL);
	ext_part = ped_disk_extended_partition (disk);
	if (!ext_part) ext_part = part;
	PED_ASSERT (ext_part != NULL);

	if (part != ext_part) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't have more than one extended partition on %s."),
			disk->dev->path);
		return 0;
	}

	for (walk = ext_part->part_list; walk; walk = walk->next) {
		if (!ped_geometry_test_inside (&ext_part->geom, &walk->geom)) {
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Can't have logical partitions outside of "
				  "the extended partition."));
			return 0;
		}
	}
	return 1;
}

static int
_check_partition (PedDisk* disk, PedPartition* part)
{
	PedPartition*	ext_part = ped_disk_extended_partition (disk);

	PED_ASSERT (part->geom.start <= part->geom.end);

	if (part->type == PED_PARTITION_EXTENDED) {
		if (!_check_extended_partition (disk, part))
			return 0;
	}

	if (part->type & PED_PARTITION_LOGICAL
	    && !ped_geometry_test_inside (&ext_part->geom, &part->geom)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't have a logical partition outside of the "
			  "extended partition on %s."),
			disk->dev->path);
		return 0;
	}

	if (!_disk_check_part_overlaps (disk, part)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't have overlapping partitions."));
		return 0;
	}

	if (! (part->type & PED_PARTITION_LOGICAL)
	    && ext_part && ext_part != part
	    && ped_geometry_test_inside (&ext_part->geom, &part->geom)) {
		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Can't have a primary partition inside an extended "
			 "partition."));
		return 0;
	}

	if (!(part->type & PED_PARTITION_METADATA))
		if (!disk->type->ops->partition_check(part))
			return 0;

	return 1;
}

/**
 * Adds PedPartition \p part to PedPartition \p disk.
 *
 * \warning The partition's geometry may be changed, subject to \p constraint.
 * You could set \p constraint to <tt>ped_constraint_exact(&part->geom)</tt>,
 * but many partition table schemes have special requirements on the start
 * and end of partitions.  Therefore, having an overly strict constraint
 * will probably mean that this function will fail (in which
 * case \p part will be left unmodified)
 * \p part is assigned a number (\p part->num) in this process.
 *
 * \return \c 0 on failure.
 */
int
ped_disk_add_partition (PedDisk* disk, PedPartition* part,
			const PedConstraint* constraint)
{
	PedConstraint*	overlap_constraint = NULL;
	PedConstraint*	constraints = NULL;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (part != NULL);

	if (!_partition_check_basic_sanity (disk, part))
		return 0;

	if (!_disk_push_update_mode (disk))
		return 0;

	if (ped_partition_is_active (part)) {
		overlap_constraint
			= _partition_get_overlap_constraint (part, &part->geom);
		constraints = ped_constraint_intersect (overlap_constraint,
							constraint);

		if (!constraints && constraint) {
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Can't have overlapping partitions."));
			goto error;
		}

		if (!_partition_enumerate (part))
			goto error;
		if (!_partition_align (part, constraints))
			goto error;
	}
        /* FIXME: when _check_partition fails, we end up leaking PART
           at least for DVH partition tables.  Simply calling
           ped_partition_destroy(part) here fixes it for DVH, but
           causes trouble for other partition types.  Similarly,
           reordering these two checks, putting _check_partition after
           _disk_raw_add induces an infinite loop.  */
	if (!_check_partition (disk, part))
		goto error;
	if (!_disk_raw_add (disk, part))
		goto error;

	ped_constraint_destroy (overlap_constraint);
	ped_constraint_destroy (constraints);
	if (!_disk_pop_update_mode (disk))
		return 0;
#ifdef DEBUG
	if (!_disk_check_sanity (disk))
		return 0;
#endif
	return 1;

error:
	ped_constraint_destroy (overlap_constraint);
	ped_constraint_destroy (constraints);
	_disk_pop_update_mode (disk);
	return 0;
}

/**
 * Removes PedPartition \p part from PedDisk \p disk.
 *
 * If \p part is an extended partition, it must not contain any logical
 * partitions. \p part is *NOT* destroyed. The caller must call
 * ped_partition_destroy(), or use ped_disk_delete_partition() instead.
 *
 * \return \c 0 on error.
 */
int
ped_disk_remove_partition (PedDisk* disk, PedPartition* part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (part != NULL);

	if (!_disk_push_update_mode (disk))
		return 0;
	PED_ASSERT (part->part_list == NULL);
	_disk_raw_remove (disk, part);
	if (!_disk_pop_update_mode (disk))
		return 0;
	ped_disk_enumerate_partitions (disk);
	return 1;
}

static int
ped_disk_delete_all_logical (PedDisk* disk);

/**
 * Removes \p part from \p disk, and destroys \p part.
 *
 * \return \c 0 on failure.
 */
int
ped_disk_delete_partition (PedDisk* disk, PedPartition* part)
{
	PED_ASSERT (disk != NULL);
	PED_ASSERT (part != NULL);

	if (!_disk_push_update_mode (disk))
		return 0;
	if (part->type == PED_PARTITION_EXTENDED)
		ped_disk_delete_all_logical (disk);
	ped_disk_remove_partition (disk, part);
	ped_partition_destroy (part);
	if (!_disk_pop_update_mode (disk))
		return 0;

	return 1;
}

static int
ped_disk_delete_all_logical (PedDisk* disk)
{
	PedPartition*		walk;
	PedPartition*		next;
	PedPartition*		ext_part;

	PED_ASSERT (disk != NULL);
	ext_part = ped_disk_extended_partition (disk);
	PED_ASSERT (ext_part != NULL);

	for (walk = ext_part->part_list; walk; walk = next) {
		next = walk->next;

		if (!ped_disk_delete_partition (disk, walk))
			return 0;
	}
	return 1;
}

/**
 * Removes and destroys all partitions on \p disk.
 *
 * \return \c 0 on failure.
 */
int
ped_disk_delete_all (PedDisk* disk)
{
	PedPartition*		walk;
	PedPartition*		next;

	PED_ASSERT (disk != NULL);

	if (!_disk_push_update_mode (disk))
		return 0;

	for (walk = disk->part_list; walk; walk = next) {
		next = walk->next;

		if (!ped_disk_delete_partition (disk, walk)) {
		        _disk_pop_update_mode(disk);
			return 0;
                }
	}

	if (!_disk_pop_update_mode (disk))
		return 0;

	return 1;
}

/**
 * Sets the geometry of \p part (i.e. change a partitions location). This can
 * fail for many reasons, e.g. can't overlap with other partitions. If it
 * does fail, \p part will remain unchanged. Returns \c 0 on failure. \p part's
 * geometry may be set to something different from \p start and \p end subject
 * to \p constraint.
 *
 * \warning The constraint warning from ped_disk_add_partition() applies.
 *
 * \note this function does not modify the contents of the partition.  You need
 *       to call ped_file_system_resize() separately.
 */
int
ped_disk_set_partition_geom (PedDisk* disk, PedPartition* part,
			     const PedConstraint* constraint,
			     PedSector start, PedSector end)
{
	PedConstraint*	overlap_constraint = NULL;
	PedConstraint*	constraints = NULL;
	PedGeometry	old_geom;
	PedGeometry	new_geom;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (part != NULL);
	PED_ASSERT (part->disk == disk);

	old_geom = part->geom;
	if (!ped_geometry_init (&new_geom, part->geom.dev, start, end - start + 1))
		return 0;

	if (!_disk_push_update_mode (disk))
		return 0;

	overlap_constraint
		= _partition_get_overlap_constraint (part, &new_geom);
	constraints = ped_constraint_intersect (overlap_constraint, constraint);
	if (!constraints && constraint) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Can't have overlapping partitions."));
		goto error_pop_update_mode;
	}

	part->geom = new_geom;
	if (!_partition_align (part, constraints))
		goto error_pop_update_mode;
	if (!_check_partition (disk, part))
		goto error_pop_update_mode;

	/* remove and add, to ensure the ordering gets updated if necessary */
	_disk_raw_remove (disk, part);
	_disk_raw_add (disk, part);

	if (!_disk_pop_update_mode (disk))
		goto error;

	ped_constraint_destroy (overlap_constraint);
	ped_constraint_destroy (constraints);
	return 1;

error_pop_update_mode:
	_disk_pop_update_mode (disk);
error:
	ped_constraint_destroy (overlap_constraint);
	ped_constraint_destroy (constraints);
	part->geom = old_geom;
	return 0;
}

/**
 * Grow PedPartition \p part geometry to the maximum possible subject to
 * \p constraint.  The new geometry will be a superset of the old geometry.
 *
 * \return 0 on failure
 */
int
ped_disk_maximize_partition (PedDisk* disk, PedPartition* part,
			     const PedConstraint* constraint)
{
	PedGeometry	old_geom;
	PedSector	global_min_start;
	PedSector	global_max_end;
	PedSector	new_start;
	PedSector	new_end;
	PedPartition*	ext_part = ped_disk_extended_partition (disk);
	PedConstraint*	constraint_any;

	PED_ASSERT (disk != NULL);
	PED_ASSERT (part != NULL);

	if (part->type & PED_PARTITION_LOGICAL) {
		PED_ASSERT (ext_part != NULL);
		global_min_start = ext_part->geom.start;
		global_max_end = ext_part->geom.end;
	} else {
		global_min_start = 0;
		global_max_end = disk->dev->length - 1;
	}

	old_geom = part->geom;

	if (!_disk_push_update_mode (disk))
		return 0;

	if (part->prev)
		new_start = part->prev->geom.end + 1;
	else
		new_start = global_min_start;

	if (part->next)
		new_end = part->next->geom.start - 1;
	else
		new_end = global_max_end;

	if (!ped_disk_set_partition_geom (disk, part, constraint, new_start,
					  new_end))
		goto error;

	if (!_disk_pop_update_mode (disk))
		return 0;
	return 1;

error:
	constraint_any = ped_constraint_any (disk->dev);
	ped_disk_set_partition_geom (disk, part, constraint_any,
				     old_geom.start, old_geom.end);
	ped_constraint_destroy (constraint_any);
	_disk_pop_update_mode (disk);
	return 0;
}

/**
 * Get the maximum geometry \p part can be grown to, subject to
 * \p constraint.
 *
 * \return \c NULL on failure.
 */
PedGeometry*
ped_disk_get_max_partition_geometry (PedDisk* disk, PedPartition* part,
				     const PedConstraint* constraint)
{
	PedGeometry	old_geom;
	PedGeometry*	max_geom;
	PedConstraint*	constraint_exact;

	PED_ASSERT(disk != NULL);
	PED_ASSERT(part != NULL);
	PED_ASSERT(ped_partition_is_active (part));

	old_geom = part->geom;
	if (!ped_disk_maximize_partition (disk, part, constraint))
		return NULL;
	max_geom = ped_geometry_duplicate (&part->geom);

	constraint_exact = ped_constraint_exact (&old_geom);
	ped_disk_set_partition_geom (disk, part, constraint_exact,
				     old_geom.start, old_geom.end);
	ped_constraint_destroy (constraint_exact);

	/* this assertion should never fail, because the old
	 * geometry was valid
	 */
	PED_ASSERT (ped_geometry_test_equal (&part->geom, &old_geom));

	return max_geom;
}

/**
 * Reduce the size of the extended partition to a minimum while still wrapping
 * its logical partitions.  If there are no logical partitions, remove the
 * extended partition.
 *
 * \return 0 on failure.
 */
int
ped_disk_minimize_extended_partition (PedDisk* disk)
{
	PedPartition*		first_logical;
	PedPartition*		last_logical;
	PedPartition*		walk;
	PedPartition*		ext_part;
	PedConstraint*		constraint;
	int			status;

	PED_ASSERT (disk != NULL);

	ext_part = ped_disk_extended_partition (disk);
	if (!ext_part)
		return 1;

	if (!_disk_push_update_mode (disk))
		return 0;

	first_logical = ext_part->part_list;
	if (!first_logical) {
		if (!_disk_pop_update_mode (disk))
			return 0;
		return ped_disk_delete_partition (disk, ext_part);
	}

	for (walk = first_logical; walk->next; walk = walk->next);
	last_logical = walk;

	constraint = ped_constraint_any (disk->dev);
	status = ped_disk_set_partition_geom (disk, ext_part, constraint,
					      first_logical->geom.start,
					      last_logical->geom.end);
	ped_constraint_destroy (constraint);

	if (!_disk_pop_update_mode (disk))
		return 0;
	return status;
}

/**
 * @}
 */

/**
 * \addtogroup PedPartition
 *
 * @{
 */

/**
 * Returns a name that seems mildly appropriate for a partition type \p type.
 *
 * Eg, if you pass (PED_PARTITION_LOGICAL & PED_PARTITION_FREESPACE), it
 * will return "free".  This isn't to be taken too seriously - it's just
 * useful for user interfaces, so you can show the user something ;-)
 *
 * \note The returned string will be in English.  However,
 * translations are provided, so the caller can call
 * dgettext("parted", RESULT) on the result.
 *
 */
const char*
ped_partition_type_get_name (PedPartitionType type)
{
	if (type & PED_PARTITION_METADATA)
		return N_("metadata");
	else if (type & PED_PARTITION_FREESPACE)
		return N_("free");
	else if (type & PED_PARTITION_EXTENDED)
		return N_("extended");
	else if (type & PED_PARTITION_LOGICAL)
		return N_("logical");
	else
		return N_("primary");
}


/**
 * Returns a name for a \p flag, e.g. PED_PARTITION_BOOT will return "boot".
 *
 * \note The returned string will be in English.  However,
 * translations are provided, so the caller can call
 * dgettext("parted", RESULT) on the result.
 */
const char*
ped_partition_flag_get_name (PedPartitionFlag flag)
{
	switch (flag) {
	case PED_PARTITION_BOOT:
		return N_("boot");
	case PED_PARTITION_BIOS_GRUB:
		return N_("bios_grub");
	case PED_PARTITION_ROOT:
		return N_("root");
	case PED_PARTITION_SWAP:
		return N_("swap");
	case PED_PARTITION_HIDDEN:
		return N_("hidden");
	case PED_PARTITION_RAID:
		return N_("raid");
	case PED_PARTITION_LVM:
		return N_("lvm");
	case PED_PARTITION_LBA:
		return N_("lba");
	case PED_PARTITION_HPSERVICE:
		return N_("hp-service");
	case PED_PARTITION_PALO:
		return N_("palo");
	case PED_PARTITION_PREP:
		return N_("prep");
	case PED_PARTITION_MSFT_RESERVED:
		return N_("msftres");
        case PED_PARTITION_APPLE_TV_RECOVERY:
                return N_("atvrecv");
        case PED_PARTITION_DIAG:
                return N_("diag");
        case PED_PARTITION_LEGACY_BOOT:
                return N_("legacy_boot");

	default:
		ped_exception_throw (
			PED_EXCEPTION_BUG,
			PED_EXCEPTION_CANCEL,
			_("Unknown partition flag, %d."),
			flag);
		return NULL;
	}
}

/**
 * Iterates through all flags.
 *
 * ped_partition_flag_next(0) returns the first flag
 *
 * \return the next flag, or 0 if there are no more flags
 */
PedPartitionFlag
ped_partition_flag_next (PedPartitionFlag flag)
{
	return (flag + 1) % (PED_PARTITION_LAST_FLAG + 1);
}

/**
 * Returns the flag associated with \p name.
 *
 * \p name can be the English
 * string, or the translation for the native language.
 */
PedPartitionFlag
ped_partition_flag_get_by_name (const char* name)
{
	PedPartitionFlag	flag;
	const char*		flag_name;

	for (flag = ped_partition_flag_next (0); flag;
	     		flag = ped_partition_flag_next (flag)) {
		flag_name = ped_partition_flag_get_name (flag);
		if (strcasecmp (name, flag_name) == 0
		    || strcasecmp (name, _(flag_name)) == 0)
			return flag;
	}

	return 0;
}

static void
ped_partition_print (const PedPartition* part)
{
	PED_ASSERT (part != NULL);

	printf ("  %-10s %02d  (%d->%d)\n",
		ped_partition_type_get_name (part->type),
		part->num,
		(int) part->geom.start, (int) part->geom.end);
}

/** @} */

/**
 * \addtogroup PedDisk
 *
 * @{
 */

/**
 * Prints a summary of disk's partitions.  Useful for debugging.
 */
void
ped_disk_print (const PedDisk* disk)
{
	PedPartition*	part;

	PED_ASSERT (disk != NULL);

	for (part = disk->part_list; part;
	     part = ped_disk_next_partition (disk, part))
		ped_partition_print (part);
}

/** @} */
