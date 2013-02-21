/* libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2001, 2007-2012 Free Software Foundation, Inc.

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

/** \file filesys.c */

/**
 * \addtogroup PedFileSystem
 *
 * \note File systems exist on a PedGeometry - NOT a PedPartition.
 *
 * @{
 */

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include "pt-tools.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#define STREQ(a, b) (strcmp (a, b) == 0)

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef PedFileSystem * (*open_fn_t) (PedGeometry *);
extern PedFileSystem *hfsplus_open (PedGeometry *);
extern PedFileSystem *hfs_open (PedGeometry *);
extern PedFileSystem *fat_open (PedGeometry *);

typedef int (*close_fn_t) (PedFileSystem *);
extern int hfsplus_close (PedFileSystem *);
extern int hfs_close (PedFileSystem *);
extern int fat_close (PedFileSystem *);

typedef int (*resize_fn_t) (PedFileSystem *fs, PedGeometry *geom,
			    PedTimer *timer);
extern int hfsplus_resize (PedFileSystem *fs, PedGeometry *geom,
			   PedTimer *timer);
extern int hfs_resize (PedFileSystem *fs, PedGeometry *geom,
		       PedTimer *timer);
extern int fat_resize (PedFileSystem *fs, PedGeometry *geom,
		       PedTimer *timer);

typedef PedConstraint * (*resize_constraint_fn_t) (PedFileSystem const *fs);
extern PedConstraint *hfsplus_get_resize_constraint (PedFileSystem const *fs);
extern PedConstraint *hfs_get_resize_constraint (PedFileSystem const *fs);
extern PedConstraint *fat_get_resize_constraint (PedFileSystem const *fs);

static bool
is_hfs_plus (char const *fs_type_name)
{
  return STREQ (fs_type_name, "hfsx") || STREQ (fs_type_name, "hfs+");
}

static open_fn_t
open_fn (char const *fs_type_name)
{
  if (is_hfs_plus (fs_type_name))
    return hfsplus_open;
  if (STREQ (fs_type_name, "hfs"))
    return hfs_open;
  if (strncmp (fs_type_name, "fat", 3) == 0)
    return fat_open;
  return NULL;
}

static close_fn_t
close_fn (char const *fs_type_name)
{
  if (is_hfs_plus (fs_type_name))
    return hfsplus_close;
  if (STREQ (fs_type_name, "hfs"))
    return hfs_close;
  if (strncmp (fs_type_name, "fat", 3) == 0)
    return fat_close;
  return NULL;
}

static resize_fn_t
resize_fn (char const *fs_type_name)
{
  if (is_hfs_plus (fs_type_name))
    return hfsplus_resize;
  if (STREQ (fs_type_name, "hfs"))
    return hfs_resize;
  if (strncmp (fs_type_name, "fat", 3) == 0)
    return fat_resize;
  return NULL;
}

static resize_constraint_fn_t
resize_constraint_fn (char const *fs_type_name)
{
  if (is_hfs_plus (fs_type_name))
    return hfsplus_get_resize_constraint;
  if (STREQ (fs_type_name, "hfs"))
    return hfs_get_resize_constraint;
  if (strncmp (fs_type_name, "fat", 3) == 0)
    return fat_get_resize_constraint;
  return NULL;
}

/**
 * This function opens the file system stored on \p geom, if it
 * can find one.
 * It is often called in the following manner:
 * \code
 *     fs = ped_file_system_open (&part.geom)
 * \endcode
 *
 * \throws PED_EXCEPTION_ERROR if file system could not be detected
 * \throws PED_EXCEPTION_ERROR if the file system is bigger than its volume
 * \throws PED_EXCEPTION_NO_FEATURE if opening of a file system stored on
 *     \p geom is not implemented
 *
 * \return a PedFileSystem on success, \c NULL on failure.
 */
PedFileSystem *
ped_file_system_open (PedGeometry* geom)
{
       PED_ASSERT (geom != NULL);

       if (!ped_device_open (geom->dev))
               goto error;

       PedFileSystemType *type = ped_file_system_probe (geom);
       if (!type) {
               ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
                                    _("Could not detect file system."));
               goto error_close_dev;
       }

       open_fn_t open_f = open_fn (type->name);
       if (open_f == NULL) {
	   ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("resizing %s file systems is not supported"),
				type->name);
	   goto error_close_dev;
       }

       PedGeometry *probed_geom = ped_file_system_probe_specific (type, geom);
       if (!probed_geom)
               goto error_close_dev;
       if (!ped_geometry_test_inside (geom, probed_geom)) {
               if (ped_exception_throw (
                       PED_EXCEPTION_ERROR,
                       PED_EXCEPTION_IGNORE_CANCEL,
                       _("The file system is bigger than its volume!"))
                               != PED_EXCEPTION_IGNORE)
                       goto error_destroy_probed_geom;
       }

       PedFileSystem *fs = (*open_f) (probed_geom);
       if (!fs)
               goto error_destroy_probed_geom;
       ped_geometry_destroy (probed_geom);
       fs->type = type;
       return fs;

error_destroy_probed_geom:
       ped_geometry_destroy (probed_geom);
error_close_dev:
       ped_device_close (geom->dev);
error:
       return NULL;
}

/**
 * Close file system \p fs.
 *
 * \return \c 1 on success, \c 0 on failure
 */
int
ped_file_system_close (PedFileSystem* fs)
{
       PED_ASSERT (fs != NULL);
       PedDevice *dev = fs->geom->dev;

       if (!(close_fn (fs->type->name) (fs)))
               goto error_close_dev;
       ped_device_close (dev);
       return 1;

error_close_dev:
       ped_device_close (dev);
       return 0;
}

/**
 * This function erases all file system signatures that indicate that a
 * file system occupies a given region described by \p geom.
 * After this operation ped_file_system_probe() won't detect any file system.
 *
 * \note ped_file_system_create() calls this before creating a new file system.
 *
 * \return \c 1 on success, \c 0 on failure
 */
static int
ped_file_system_clobber (PedGeometry* geom)
{
  PED_ASSERT (geom != NULL);

  if (!ped_device_open (geom->dev))
    return 0;

  /* Clear the first three and the last two sectors, albeit fewer
     when GEOM is too small.  */
  PedSector len = MIN (geom->length, geom->dev->length);

  int ok = (len <= 5
	    ? ptt_geom_clear_sectors (geom, 0, len)
	    : (ptt_geom_clear_sectors (geom, 0, 3)
	       && ptt_geom_clear_sectors (geom, geom->dev->length - 2, 2)));

  ped_device_close (geom->dev);
  return !!ok;
}

/* This function erases all signatures that indicate the presence of
 * a file system in a particular region, without erasing any data
 * contained inside the "exclude" region.
 */
static int
ped_file_system_clobber_exclude (PedGeometry* geom,
				 const PedGeometry* exclude)
{
	PedGeometry*    clobber_geom;
	int             status;

	if (ped_geometry_test_sector_inside (exclude, geom->start))
		return 1;

	clobber_geom = ped_geometry_duplicate (geom);
	if (ped_geometry_test_overlap (clobber_geom, exclude))
		ped_geometry_set_end (clobber_geom, exclude->start - 1);

	status = ped_file_system_clobber (clobber_geom);
	ped_geometry_destroy (clobber_geom);
	return status;
}

/**
 * Resize \p fs to new geometry \p geom.
 *
 * \p geom should satisfy the ped_file_system_get_resize_constraint().
 * (This isn't asserted, so it's not a bug not to... just it's likely
 * to fail ;)  If \p timer is non-NULL, it is used as the progress meter.
 *
 * \throws PED_EXCEPTION_NO_FEATURE if resizing of file system \p fs
 *     is not implemented yet
 *
 * \return \c 0 on failure
 */
int
ped_file_system_resize (PedFileSystem *fs, PedGeometry *geom, PedTimer *timer)
{
       PED_ASSERT (fs != NULL);
       PED_ASSERT (geom != NULL);

       resize_fn_t resize_f = resize_fn (fs->type->name);
       if (resize_f == NULL) {
	   ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("resizing %s file systems is not supported"),
				fs->type->name);
	   return 0;
       }

       if (!ped_file_system_clobber_exclude (geom, fs->geom))
               return 0;

       return resize_f (fs, geom, timer);
}

/**
 * Return a constraint that represents all of the possible ways the
 * file system \p fs can be resized with ped_file_system_resize().
 * This takes into account the amount of used space on
 * the filesystem \p fs and the capabilities of the resize algorithm.
 * Hints:
 * -# if constraint->start_align->grain_size == 0, or
 *    constraint->start_geom->length == 1, then the start cannot be moved
 * -# constraint->min_size is the minimum size you can resize the partition
 *    to.  You might want to tell the user this ;-).
 *
 * \return a PedConstraint on success, \c NULL on failure
 */
PedConstraint *
ped_file_system_get_resize_constraint (const PedFileSystem *fs)
{
	PED_ASSERT (fs != NULL);

	resize_constraint_fn_t resize_constraint_f =
	  resize_constraint_fn (fs->type->name);
	if (resize_constraint_f == NULL)
	  return NULL;

	return resize_constraint_f (fs);
}
