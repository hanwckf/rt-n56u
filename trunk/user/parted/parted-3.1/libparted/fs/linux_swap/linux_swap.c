/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2000, 2002, 2007-2012 Free Software Foundation, Inc.

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

/* It's a bit silly calling a swap partition a file system.  Oh well...  */

#include <config.h>

#include <parted/parted.h>
#include <parted/endian.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include <unistd.h>
#include <uuid/uuid.h>

#define SWAP_SPECIFIC(fs) ((SwapSpecific*) (fs->type_specific))
#define BUFFER_SIZE 128

#define LINUXSWAP_BLOCK_SIZES       ((int[2]){512, 0})

typedef struct {
	char		page_map[1];
} SwapOldHeader;

/* ripped from mkswap */
typedef struct {
        char            bootbits[1024];    /* Space for disklabel etc. */
        uint32_t        version;
        uint32_t        last_page;
        uint32_t        nr_badpages;
        unsigned char   sws_uuid[16];
        unsigned char   sws_volume[16];
        uint32_t        padding[117];
        uint32_t        badpages[1];
} SwapNewHeader;

typedef struct {
	union {
		SwapNewHeader	new;
		SwapOldHeader	old;
	}* header;

	void*		buffer;
	int		buffer_size;

	PedSector	page_sectors;
	unsigned int	page_count;
	unsigned int	version;
	unsigned int	max_bad_pages;
} SwapSpecific;

static PedFileSystemType _swap_v0_type;
static PedFileSystemType _swap_v1_type;
static PedFileSystemType _swap_swsusp_type;

static PedFileSystem* _swap_v0_open (PedGeometry* geom);
static PedFileSystem* _swap_v1_open (PedGeometry* geom);
static PedFileSystem* _swap_swsusp_open (PedGeometry* geom);
static int swap_close (PedFileSystem* fs);

static PedGeometry*
_generic_swap_probe (PedGeometry* geom, int kind)
{
	PedFileSystem*	fs;
	SwapSpecific*	fs_info;
	PedGeometry*	probed_geom;
	PedSector	length;

        /* Fail the swap-file-system-recognizing test when sector size
           is not the default.  */
	if (geom->dev->sector_size != PED_SECTOR_SIZE_DEFAULT)
		return NULL;

        switch (kind) {
        /* Check for old style swap partitions. */
                case 0:
                        fs = _swap_v0_open(geom);
                        break;
        /* Check for new style swap partitions. */
                case 1:
                        fs = _swap_v1_open(geom);
                        break;
        /* Check for swap partitions containing swsusp data. */
                case -1:
                        fs = _swap_swsusp_open(geom);
                        break;
        /* Not reached. */
                default:
                        goto error;
        }

	if (!fs)
		goto error;
	fs_info = SWAP_SPECIFIC (fs);

	if (fs_info->version)
		length = fs_info->page_sectors * fs_info->page_count;
	else
	        length = geom->length;

	probed_geom = ped_geometry_new (geom->dev, geom->start, length);
	if (!probed_geom)
		goto error_close_fs;
	swap_close (fs);
	return probed_geom;

error_close_fs:
	swap_close (fs);
error:
	return NULL;
}


static int
swap_init (PedFileSystem* fs, int fresh)
{
	SwapSpecific*	fs_info = SWAP_SPECIFIC (fs);

	fs_info->page_sectors = getpagesize () / 512;
	fs_info->page_count = fs->geom->length / fs_info->page_sectors;
	fs_info->version = 1;
	fs_info->max_bad_pages = (getpagesize()
					- sizeof (SwapNewHeader)) / 4;

	if (fresh) {
		uuid_t uuid_dat;

		memset (fs_info->header, 0, getpagesize());

		/* version is always 1 here */
		uuid_generate (uuid_dat);
		memcpy (fs_info->header->new.sws_uuid, uuid_dat,
			sizeof (fs_info->header->new.sws_uuid));
                return 1;
        }
	else
                return ped_geometry_read (fs->geom, fs_info->header,
                                          0, fs_info->page_sectors);
}


static PedFileSystem*
swap_alloc (PedGeometry* geom)
{
	PedFileSystem*	fs;
	SwapSpecific*	fs_info;

	fs = (PedFileSystem*) ped_malloc (sizeof (PedFileSystem));
	if (!fs)
		goto error;

	fs->type_specific = (SwapSpecific*) ped_malloc (sizeof (SwapSpecific));
	if (!fs->type_specific)
		goto error_free_fs;

	fs_info = SWAP_SPECIFIC (fs);
	fs_info->header = ped_malloc (getpagesize());
	if (!fs_info->header)
		goto error_free_type_specific;

	fs_info = SWAP_SPECIFIC (fs);
	fs_info->buffer_size = getpagesize() * BUFFER_SIZE;
	fs_info->buffer = ped_malloc (fs_info->buffer_size);
	if (!fs_info->buffer)
		goto error_free_header;

	fs->geom = ped_geometry_duplicate (geom);
	if (!fs->geom)
		goto error_free_buffer;
	fs->type = &_swap_v1_type;
	return fs;

error_free_buffer:
	free (fs_info->buffer);
error_free_header:
	free (fs_info->header);
error_free_type_specific:
	free (fs->type_specific);
error_free_fs:
	free (fs);
error:
	return NULL;
}

static void
swap_free (PedFileSystem* fs)
{
	SwapSpecific*	fs_info = SWAP_SPECIFIC (fs);

	free (fs_info->buffer);
	free (fs_info->header);
	free (fs->type_specific);

	ped_geometry_destroy (fs->geom);
	free (fs);
}

static PedFileSystem*
_swap_v0_open (PedGeometry* geom)
{
	PedFileSystem*		fs;
	SwapSpecific*		fs_info;
	const char*		sig;

	fs = swap_alloc (geom);
	if (!fs)
		goto error;
	swap_init (fs, 0);

	fs_info = SWAP_SPECIFIC (fs);
	if (!ped_geometry_read (fs->geom, fs_info->header, 0,
				fs_info->page_sectors))
		goto error_free_fs;

	sig = ((char*) fs_info->header) + getpagesize() - 10;
	if (strncmp (sig, "SWAP-SPACE", 10) == 0) {
		fs_info->version = 0;
		fs_info->page_count
			= PED_MIN (fs->geom->length / fs_info->page_sectors,
				   8 * (getpagesize() - 10));
	} else {
		char	_sig [11];

		memcpy (_sig, sig, 10);
		_sig [10] = 0;
 		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Unrecognised old style linux swap signature '%10s'."), _sig);
 		goto error_free_fs;
	}

	fs->checked = 1;
	return fs;

error_free_fs:
	swap_free (fs);
error:
	return NULL;
}

static PedFileSystem*
_swap_v1_open (PedGeometry* geom)
{
	PedFileSystem*		fs;
	SwapSpecific*		fs_info;
	const char*		sig;

	fs = swap_alloc (geom);
	if (!fs)
		goto error;
/* 	swap_init (fs, 0); */

/* 	fs_info = SWAP_SPECIFIC (fs); */
/* 	if (!ped_geometry_read (fs->geom, fs_info->header, 0, */
/* 				fs_info->page_sectors)) */
        if (!swap_init(fs, 0))
		goto error_free_fs;

        fs_info = SWAP_SPECIFIC (fs);

	sig = ((char*) fs_info->header) + getpagesize() - 10;
	if (strncmp (sig, "SWAPSPACE2", 10) == 0) {
		fs_info->version = 1;
		fs_info->page_count = fs_info->header->new.last_page;
	} else {
		char	_sig [11];

		memcpy (_sig, sig, 10);
		_sig [10] = 0;
 		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Unrecognised new style linux swap signature '%10s'."), _sig);
 		goto error_free_fs;
	}

	fs->checked = 1;
	return fs;

error_free_fs:
	swap_free (fs);
error:
	return NULL;
}

static PedFileSystem*
_swap_swsusp_open (PedGeometry* geom)
{
	PedFileSystem*		fs;
	SwapSpecific*		fs_info;
	const char*		sig;

	fs = swap_alloc (geom);
	if (!fs)
		goto error;
        fs->type = &_swap_swsusp_type;
	swap_init (fs, 0);

	fs_info = SWAP_SPECIFIC (fs);
	if (!ped_geometry_read (fs->geom, fs_info->header, 0,
				fs_info->page_sectors))
		goto error_free_fs;

	sig = ((char*) fs_info->header) + getpagesize() - 10;
       	if (strncmp (sig, "S1SUSPEND", 9) == 0) {
	        fs_info->version = -1;
	} else {
		char	_sig [10];

		memcpy (_sig, sig, 9);
		_sig [9] = 0;
 		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Unrecognised swsusp linux swap signature '%9s'."), _sig);
 		goto error_free_fs;
	}

	fs->checked = 1;
	return fs;

error_free_fs:
	swap_free (fs);
error:
	return NULL;
}

static int
swap_close (PedFileSystem* fs)
{
	swap_free (fs);
	return 1;
}

static PedGeometry*
_swap_v0_probe (PedGeometry* geom) {
        return _generic_swap_probe (geom, 0);
}

static PedGeometry*
_swap_v1_probe (PedGeometry* geom) {
        return _generic_swap_probe (geom, 1);
}

static PedGeometry*
_swap_swsusp_probe (PedGeometry* geom) {
        return _generic_swap_probe (geom, -1);
}

static PedFileSystemOps _swap_v0_ops = {
	probe:		_swap_v0_probe,
};

static PedFileSystemOps _swap_v1_ops = {
	probe:		_swap_v1_probe,
};

static PedFileSystemOps _swap_swsusp_ops = {
  probe:		_swap_swsusp_probe,
};

static PedFileSystemType _swap_v0_type = {
	next:	NULL,
	ops:	&_swap_v0_ops,
	name:	"linux-swap(v0)",
	block_sizes: LINUXSWAP_BLOCK_SIZES
};

static PedFileSystemType _swap_v1_type = {
	next:	NULL,
	ops:	&_swap_v1_ops,
	name:	"linux-swap(v1)",
	block_sizes: LINUXSWAP_BLOCK_SIZES
};

static PedFileSystemType _swap_swsusp_type = {
        next:   NULL,
	ops:    &_swap_swsusp_ops,
	name:   "swsusp",
        block_sizes: LINUXSWAP_BLOCK_SIZES
};

void
ped_file_system_linux_swap_init ()
{
	ped_file_system_type_register (&_swap_v0_type);
	ped_file_system_type_register (&_swap_v1_type);
	ped_file_system_type_register (&_swap_swsusp_type);

	ped_file_system_alias_register (&_swap_v0_type, "linux-swap(old)", 1);
	ped_file_system_alias_register (&_swap_v1_type, "linux-swap(new)", 1);
	ped_file_system_alias_register (&_swap_v1_type, "linux-swap", 0);
}

void
ped_file_system_linux_swap_done ()
{
	ped_file_system_alias_unregister (&_swap_v0_type, "linux-swap(old)");
	ped_file_system_alias_unregister (&_swap_v1_type, "linux-swap(new)");
	ped_file_system_alias_unregister (&_swap_v1_type, "linux-swap");

	ped_file_system_type_unregister (&_swap_v0_type);
	ped_file_system_type_unregister (&_swap_v1_type);
	ped_file_system_type_unregister (&_swap_swsusp_type);
}
