/*
    interface.c -- parted support amiga file systems
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


#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>
#include <parted/endian.h>

extern PedFileSystemType _affs0_type;
extern PedFileSystemType _affs1_type;
extern PedFileSystemType _affs2_type;
extern PedFileSystemType _affs3_type;
extern PedFileSystemType _affs4_type;
extern PedFileSystemType _affs5_type;
extern PedFileSystemType _affs6_type;
extern PedFileSystemType _affs7_type;
extern PedFileSystemType _amufs_type;
extern PedFileSystemType _amufs0_type;
extern PedFileSystemType _amufs1_type;
extern PedFileSystemType _amufs2_type;
extern PedFileSystemType _amufs3_type;
extern PedFileSystemType _amufs4_type;
extern PedFileSystemType _amufs5_type;
extern PedFileSystemType _asfs_type;
extern PedFileSystemType _apfs1_type;
extern PedFileSystemType _apfs2_type;

void ped_file_system_amiga_init ()
{
	ped_file_system_type_register (&_affs0_type);
	ped_file_system_type_register (&_affs1_type);
	ped_file_system_type_register (&_affs2_type);
	ped_file_system_type_register (&_affs3_type);
	ped_file_system_type_register (&_affs4_type);
	ped_file_system_type_register (&_affs5_type);
	ped_file_system_type_register (&_affs6_type);
	ped_file_system_type_register (&_affs7_type);
	ped_file_system_type_register (&_amufs_type);
	ped_file_system_type_register (&_amufs0_type);
	ped_file_system_type_register (&_amufs1_type);
	ped_file_system_type_register (&_amufs2_type);
	ped_file_system_type_register (&_amufs3_type);
	ped_file_system_type_register (&_amufs4_type);
	ped_file_system_type_register (&_amufs5_type);
	ped_file_system_type_register (&_asfs_type);
	ped_file_system_type_register (&_apfs1_type);
	ped_file_system_type_register (&_apfs2_type);
}

void ped_file_system_amiga_done ()
{
	ped_file_system_type_unregister (&_affs0_type);
	ped_file_system_type_unregister (&_affs1_type);
	ped_file_system_type_unregister (&_affs2_type);
	ped_file_system_type_unregister (&_affs3_type);
	ped_file_system_type_unregister (&_affs4_type);
	ped_file_system_type_unregister (&_affs5_type);
	ped_file_system_type_unregister (&_affs6_type);
	ped_file_system_type_unregister (&_affs7_type);
	ped_file_system_type_unregister (&_amufs_type);
	ped_file_system_type_unregister (&_amufs0_type);
	ped_file_system_type_unregister (&_amufs1_type);
	ped_file_system_type_unregister (&_amufs2_type);
	ped_file_system_type_unregister (&_amufs3_type);
	ped_file_system_type_unregister (&_amufs4_type);
	ped_file_system_type_unregister (&_amufs5_type);
	ped_file_system_type_unregister (&_asfs_type);
	ped_file_system_type_unregister (&_apfs1_type);
	ped_file_system_type_unregister (&_apfs2_type);
}
