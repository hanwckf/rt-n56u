/*
    libparted - a library for manipulating disk partitions
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

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#define BUFFER_SIZE	4096		/* in sectors */

static PedFileSystemType*	fs_types = NULL;
static PedFileSystemAlias*	fs_aliases = NULL;

void
ped_file_system_type_register (PedFileSystemType* fs_type)
{
	PED_ASSERT (fs_type != NULL);
	PED_ASSERT (fs_type->ops != NULL);
	PED_ASSERT (fs_type->name != NULL);

        fs_type->next = fs_types;
        fs_types = fs_type;
}

void
ped_file_system_type_unregister (PedFileSystemType* fs_type)
{
	PedFileSystemType*	walk;
	PedFileSystemType*	last = NULL;

	PED_ASSERT (fs_types != NULL);
	PED_ASSERT (fs_type != NULL);

	for (walk = fs_types; walk && walk != fs_type;
                last = walk, walk = walk->next);

	PED_ASSERT (walk != NULL);
	if (last)
		((struct _PedFileSystemType*) last)->next = fs_type->next;
	else
		fs_types = fs_type->next;
}

void
ped_file_system_alias_register (PedFileSystemType* fs_type, const char* alias,
				int deprecated)
{
	PedFileSystemAlias*	fs_alias;

	PED_ASSERT (fs_type != NULL);
	PED_ASSERT (alias != NULL);

	fs_alias = ped_malloc (sizeof *fs_alias);
	if (!fs_alias)
		return;

	fs_alias->next = fs_aliases;
	fs_alias->fs_type = fs_type;
	fs_alias->alias = alias;
	fs_alias->deprecated = deprecated;
	fs_aliases = fs_alias;
}

void
ped_file_system_alias_unregister (PedFileSystemType* fs_type,
				  const char* alias)
{
	PedFileSystemAlias*	walk;
	PedFileSystemAlias*	last = NULL;

	PED_ASSERT (fs_aliases != NULL);
	PED_ASSERT (fs_type != NULL);
	PED_ASSERT (alias != NULL);

	for (walk = fs_aliases; walk; last = walk, walk = walk->next) {
		if (walk->fs_type == fs_type && !strcmp (walk->alias, alias))
			break;
	}

	PED_ASSERT (walk != NULL);
	if (last)
		last->next = walk->next;
	else
		fs_aliases = walk->next;
	free (walk);
}

/**
 * Get a PedFileSystemType by its @p name.
 *
 * @return @c NULL if none found.
 */
PedFileSystemType*
ped_file_system_type_get (const char* name)
{
	PedFileSystemType*	walk;
	PedFileSystemAlias*	alias_walk;

	PED_ASSERT (name != NULL);

	for (walk = fs_types; walk != NULL; walk = walk->next) {
		if (!strcasecmp (walk->name, name))
			break;
	}
	if (walk != NULL)
		return walk;

	for (alias_walk = fs_aliases; alias_walk != NULL;
	     alias_walk = alias_walk->next) {
		if (!strcasecmp (alias_walk->alias, name))
			break;
	}
	if (alias_walk != NULL) {
		if (alias_walk->deprecated)
			PED_DEBUG (0, "File system alias %s is deprecated",
				   name);
		return alias_walk->fs_type;
	}

	return NULL;
}

/**
 * Get the next PedFileSystemType after @p fs_type.
 *
 * @return @c NULL if @p fs_type is the last item in the list.
 */
PedFileSystemType*
ped_file_system_type_get_next (const PedFileSystemType* fs_type)
{
	if (fs_type)
		return fs_type->next;
	else
		return fs_types;
}

/**
 * Get the next PedFileSystemAlias after @p fs_alias.
 *
 * @return @c NULL if @p fs_alias is the last item in the list.
 */
PedFileSystemAlias*
ped_file_system_alias_get_next (const PedFileSystemAlias* fs_alias)
{
	if (fs_alias)
		return fs_alias->next;
	else
		return fs_aliases;
}

/**
 * Attempt to find a file system and return the region it occupies.
 *
 * @param fs_type The file system type to probe for.
 * @param geom The region to be searched.
 *
 * @return @p NULL if @p fs_type file system wasn't detected
 */
PedGeometry*
ped_file_system_probe_specific (
		const PedFileSystemType* fs_type, PedGeometry* geom)
{
	PedGeometry*	result;

	PED_ASSERT (fs_type != NULL);
	PED_ASSERT (fs_type->ops->probe != NULL);
	PED_ASSERT (geom != NULL);

        /* Fail all fs-specific probe-related tests when sector size
           is not the default.  */
	if (geom->dev->sector_size != PED_SECTOR_SIZE_DEFAULT)
		return 0;

	if (!ped_device_open (geom->dev))
		return 0;
	result = fs_type->ops->probe (geom);
	ped_device_close (geom->dev);
	return result;
}

static int
_geometry_error (const PedGeometry* a, const PedGeometry* b)
{
	PedSector	start_delta = a->start - b->start;
	PedSector	end_delta = a->end - b->end;

	return abs (start_delta) + abs (end_delta);
}

static PedFileSystemType*
_best_match (const PedGeometry* geom, PedFileSystemType* detected [],
	     const int detected_error [], int detected_count)
{
	int		best_match = 0;
	int		i;
	PedSector	min_error;

	min_error = PED_MAX (4096, geom->length / 100);

	for (i = 1; i < detected_count; i++) {
		if (detected_error [i] < detected_error [best_match])
			best_match = i;
	}

	/* make sure the best match is significantly better than all the
	 * other matches
	 */
	for (i = 0; i < detected_count; i++) {
		if (i == best_match)
			continue;

		if (abs (detected_error [best_match] - detected_error [i])
				< min_error)
			return NULL;
	}

	return detected [best_match];
}


/**
 * Attempt to detect a file system in region \p geom.
 * This function tries to be clever at dealing with ambiguous
 * situations, such as when one file system was not completely erased before a
 * new file system was created on top of it.
 *
 * \return a new PedFileSystem on success, \c NULL on failure
 */
PedFileSystemType*
ped_file_system_probe (PedGeometry* geom)
{
	PedFileSystemType*	detected[32];
	int			detected_error[32];
	int			detected_count = 0;
	PedFileSystemType*	walk = NULL;

	PED_ASSERT (geom != NULL);

	if (!ped_device_open (geom->dev))
		return NULL;

	ped_exception_fetch_all ();
	while ( (walk = ped_file_system_type_get_next (walk)) ) {
		PedGeometry*	probed;

		probed = ped_file_system_probe_specific (walk, geom);
		if (probed) {
			detected [detected_count] = walk;
			detected_error [detected_count]
				= _geometry_error (geom, probed);
			detected_count++;
			ped_geometry_destroy (probed);
		} else {
			ped_exception_catch ();
		}
	}
	ped_exception_leave_all ();

	ped_device_close (geom->dev);

	if (!detected_count)
		return NULL;
	walk = _best_match (geom, detected, detected_error, detected_count);
	if (walk)
		return walk;
	return NULL;
}
