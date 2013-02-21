/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2001, 2006-2007, 2009-2012 Free Software Foundation,
    Inc.

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
 * \addtogroup PedFileSystem
 * @{
 */

/** \file filesys.h */

#ifndef PED_FILESYS_H_INCLUDED
#define PED_FILESYS_H_INCLUDED

typedef struct _PedFileSystem		PedFileSystem;
typedef struct _PedFileSystemType	PedFileSystemType;
typedef struct _PedFileSystemAlias	PedFileSystemAlias;
typedef const struct _PedFileSystemOps	PedFileSystemOps;

#include <parted/geom.h>
#include <parted/constraint.h>
#include <parted/timer.h>

struct _PedFileSystemOps {
	PedGeometry* (*probe) (PedGeometry* geom);
};

/**
 * Structure describing type of file system
 */
struct _PedFileSystemType {
	PedFileSystemType*	next;
	const char* const	name;		/**< name of the file system type */
        const int*              block_sizes;
	PedFileSystemOps* const	ops;
};

/**
 * Structure describing a file system alias. This is separate from
 * PedFileSystemType because probing only looks through the list of types,
 * and does not probe aliases separately.
 */
struct _PedFileSystemAlias {
	PedFileSystemAlias*	next;
	PedFileSystemType*	fs_type;
	const char*		alias;
	int			deprecated;
};


/**
 * Structure describing file system
 */
struct _PedFileSystem {
	PedFileSystemType*	type;		/**< the file system type */
	PedGeometry*		geom;		/**< where the file system actually is */
	int			checked;	/**< 1 if the file system has been checked.
						      0 otherwise. */

	void*			type_specific;

};

extern void ped_file_system_type_register (PedFileSystemType* type);
extern void ped_file_system_type_unregister (PedFileSystemType* type);

extern void ped_file_system_alias_register (PedFileSystemType* type,
					    const char* alias, int deprecated);
extern void ped_file_system_alias_unregister (PedFileSystemType* type,
					      const char* alias);

extern PedFileSystemType* ped_file_system_type_get (const char* name);
extern PedFileSystemType*
ped_file_system_type_get_next (const PedFileSystemType* fs_type)
  _GL_ATTRIBUTE_PURE;

extern PedFileSystemAlias*
ped_file_system_alias_get_next (const PedFileSystemAlias* fs_alias)
  _GL_ATTRIBUTE_PURE;

extern PedFileSystemType* ped_file_system_probe (PedGeometry* geom);
extern PedGeometry* ped_file_system_probe_specific (
			const PedFileSystemType* fs_type,
			PedGeometry* geom);

PedFileSystem *ped_file_system_open (PedGeometry *geom);
int ped_file_system_close (PedFileSystem *fs);
int ped_file_system_resize (PedFileSystem *fs, PedGeometry *geom,
			    PedTimer *timer);
PedConstraint *ped_file_system_get_resize_constraint (const PedFileSystem *fs);

#endif /* PED_FILESYS_H_INCLUDED */

/** @} */
