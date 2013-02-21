/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2006-2007, 2009-2012 Free Software Foundation, Inc.

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

/* POSIX headers */
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* BeOS APIs */
#include <drivers/Drivers.h>

/* ZETA R1+ APIs */
#if B_ZETA_VERSION >= B_ZETA_VERSION_1_0_0
#  include <device/ata_info.h>
#endif

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "../architecture.h"

#define BEOS_SPECIFIC(dev)	((BEOSSpecific*) (dev)->arch_specific)

typedef	struct _BEOSSpecific	BEOSSpecific;

struct _BEOSSpecific {
	int	fd;
};

static void
_scan_for_disks(const char* path)
{
	char subdir[PATH_MAX];
	dirent_t* entp;
	size_t pos;
	DIR* dirp;

	if ((dirp=opendir(path)) != NULL)
	{
		/* Build first part of path */
		strcpy(subdir, path);
		strcat(subdir, "/");
		pos = strlen(subdir);

		/* Check all directory entries.. */
		while((entp=readdir(dirp)) != NULL)
		{
			/* If they start with '.' just skip */
			if (entp->d_name[0] == '.')
				continue;

			strcpy(subdir+pos, entp->d_name);

			/* /dev/disk/.../raw are the complete disks
				we're interested in */
			if (strcmp(entp->d_name, "raw") == 0)
				_ped_device_probe(subdir);
			else	/* If not 'raw', it most often will
						be another subdir */
				_scan_for_disks(subdir);
		}

		closedir(dirp);
	}
}

static void
_flush_cache(PedDevice* dev)
{
	int fd;
	if ((fd=open(dev->path, O_RDONLY)) < 0)
	{
		ioctl(fd, B_FLUSH_DRIVE_CACHE);
		close(fd);
	}
}

#if B_ZETA_VERSION >= B_ZETA_VERSION_1_0_0
static int
_device_init_ata(PedDevice* dev)
{
	ata_device_infoblock ide_info;
	int maxlen, len, idx, fd;
	char buf[256];

	/* Try and get information about device from ATA(PI) driver */
	if ((fd=open(dev->path, O_RDONLY)) < 0)
		goto ata_error;

	if (ioctl(fd, B_ATA_GET_DEVICE_INFO, &ide_info, sizeof(ide_info)) < 0)
		goto ata_error_close;

	close(fd);

	/* Copy 'logical' dimensions */
	dev->bios_geom.cylinders = ide_info.cylinders;
	dev->bios_geom.heads = ide_info.heads;
	dev->bios_geom.sectors = ide_info.sectors;

	/* Copy used dimensions */
	dev->hw_geom.cylinders = ide_info.current_cylinders;
	dev->hw_geom.heads = ide_info.current_heads;
	dev->hw_geom.sectors = ide_info.current_sectors;

	/* Copy total number of sectors */
	if (ide_info._48_bit_addresses_supported)
		dev->length = ide_info.LBA48_total_sectors;
	else if (ide_info.LBA_supported)
		dev->length = ide_info.LBA_total_sectors;
	else
		dev->length = ide_info.cylinders *
				ide_info.heads *
				ide_info.sectors;

	dev->sector_size =
	dev->phys_sector_size = PED_SECTOR_SIZE_DEFAULT;

	/* Use sensible model */
	maxlen=sizeof(ide_info.model_number);
	strncpy(buf, ide_info.model_number, maxlen);
	buf[maxlen] = '\0';

	for (len=-1, idx=maxlen-1; idx > 0 && len < 0; idx--)
		if (buf[idx] > 0x20)
			len = idx;

	buf[(len == -1) ? 0 : len+1] = '\0';

	dev->model = strdup(buf);

	return PED_DEVICE_IDE;

ata_error_close:
	close(fd);

ata_error:
	return 0;
}
#endif

static int
_device_init_generic_blkdev(PedDevice* dev)
{
	device_geometry bios, os;
	int got_bios_info = 0;
	int fd;

	/* Try and get information about device from ATA(PI) driver */
	if ((fd=open(dev->path, O_RDONLY)) < 0)
		goto blkdev_error;

	/* B_GET_GEOMETRY is mandatory */
	if (ioctl(fd, B_GET_GEOMETRY, &os, sizeof(os)) < 0)
		goto blkdev_error_close;

	/* B_GET_BIOS_GEOMETRY is optional */
	if (!ioctl(fd, B_GET_BIOS_GEOMETRY, &bios, sizeof(bios)))
		got_bios_info = 1;

	close(fd);

	dev->hw_geom.cylinders = os.cylinder_count;
	dev->hw_geom.heads = os.head_count;
	dev->hw_geom.sectors = os.sectors_per_track;

	dev->sector_size =
	dev->phys_sector_size = os.bytes_per_sector;

	if (got_bios_info)
	{
		dev->bios_geom.cylinders = bios.cylinder_count;
		dev->bios_geom.heads = bios.head_count;
		dev->bios_geom.sectors = bios.sectors_per_track;
	}
	else
		dev->bios_geom = dev->hw_geom;

	dev->model = strdup("");

	return PED_DEVICE_IDE;

blkdev_error_close:
	close(fd);

blkdev_error:
	return 0;
}

static int
_device_init_blkdev(PedDevice* dev)
{
	int type;

#if B_ZETA_VERSION >= B_ZETA_VERSION_1_0_0
	if (!(type=_device_init_ata(dev)))
#endif
	type = _device_init_generic_blkdev(dev);

	return type;
}

static int
_device_init_file(PedDevice* dev, struct stat* dev_statp)
{
	if (!dev_statp->st_size)
		return 0;

	dev->sector_size =
	dev->phys_sector_size = PED_SECTOR_SIZE_DEFAULT;

	dev->length = dev_statp->st_size / PED_SECTOR_SIZE_DEFAULT;

	dev->bios_geom.cylinders = dev->length / (4 * 32);
	dev->bios_geom.heads = 4;
	dev->bios_geom.sectors = 32;
	dev->hw_geom = dev->bios_geom;

	dev->model = strdup(_("Disk Image"));

	return PED_DEVICE_FILE;
}

static int
_device_init(PedDevice* dev)
{
	struct stat dev_stat;
	int type = 0;

	/* Check if we're a regular file */
	if (stat(dev->path, &dev_stat) < 0)
		goto done;

	if (S_ISBLK(dev_stat.st_mode) || S_ISCHR(dev_stat.st_mode))
		type = _device_init_blkdev(dev);
	else if (S_ISREG(dev_stat.st_mode))
		type = _device_init_file(dev,&dev_stat);

done:
	return type;
}


static PedDevice*
beos_new (const char* path)
{
	struct stat stat_info;
	PedDevice* dev;

	PED_ASSERT(path != NULL);

	dev = (PedDevice*) ped_malloc (sizeof (PedDevice));
	if (!dev)
		goto error;

	dev->path = strdup(path);
	if (!dev->path)
		goto error_free_dev;

	dev->arch_specific
		= (BEOSSpecific*) ped_malloc(sizeof(BEOSSpecific));
	if (dev->arch_specific == NULL)
		goto error_free_path;

	dev->open_count = 0;
	dev->read_only = 0;
	dev->external_mode = 0;
	dev->dirty = 0;
	dev->boot_dirty = 0;

	if ((dev->type=_device_init(dev)) <= 0)
		goto error_free_arch_specific;

	/* All OK! */
	return dev;

error_free_arch_specific:
	free (dev->arch_specific);

error_free_path:
	free (dev->path);

error_free_dev:
	free (dev);

error:
	return NULL;
}

static void
beos_destroy (PedDevice* dev)
{
	free (dev->arch_specific);
	free (dev->path);
	free (dev->model);
	free (dev);
}

static int
beos_is_busy (PedDevice* dev)
{
	return 1;
}

static int
beos_open (PedDevice* dev)
{
	BEOSSpecific* arch_specific = BEOS_SPECIFIC(dev);

retry:
	arch_specific->fd = open(dev->path, O_RDWR);
	if (arch_specific->fd == -1) {
		char* rw_error_msg = strerror(errno);

		arch_specific->fd = open (dev->path, O_RDONLY);
		if (arch_specific->fd == -1) {
			if (ped_exception_throw (
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_RETRY_CANCEL,
					_("Error opening %s: %s"),
					dev->path, strerror (errno))
					!= PED_EXCEPTION_RETRY) {
						return 0;
					} else {
						goto retry;
					}
		} else {
			ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_OK,
				_("Unable to open %s read-write (%s).  %s has "
					"been opened read-only."),
				dev->path, rw_error_msg, dev->path);
			dev->read_only = 1;
		}
	} else {
		dev->read_only = 0;
	}

	_flush_cache (dev);

	return 1;
}

static int
beos_refresh_open (PedDevice* dev)
{
	return 1;
}

static int
beos_close (PedDevice* dev)
{
	BEOSSpecific* arch_specific = BEOS_SPECIFIC(dev);

	if (dev->dirty)
		_flush_cache (dev);

	close (arch_specific->fd);

	return 1;
}

static int
beos_refresh_close (PedDevice* dev)
{
	if (dev->dirty)
		_flush_cache (dev);

	return 1;
}

static int
beos_read (const PedDevice* dev, void* buffer, PedSector start, PedSector count)
{
	BEOSSpecific* arch_specific = BEOS_SPECIFIC(dev);
	int status;
	PedExceptionOption ex_status;
	size_t read_length = count * dev->sector_size;

	PED_ASSERT(dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

	/* First, try to seek */
	while(1)
	{
		if (lseek(arch_specific->fd, start * dev->sector_size, SEEK_SET)
			== start * dev->sector_size)
			break;

		ex_status = ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_RETRY_IGNORE_CANCEL,
			_("%s during seek for read on %s"),
			strerror (errno), dev->path);

		switch (ex_status)
		{
			case PED_EXCEPTION_IGNORE:
				return 1;
			case PED_EXCEPTION_RETRY:
				break /* out of switch */;
			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				return 0;
		}
	}

	/* If we seeked ok, now is the time to read */
	while (1)
	{
		status = read(arch_specific->fd, buffer, read_length);
		if (status == count * dev->sector_size)
			break;

		if (status > 0)
		{
			read_length -= status;
			buffer += status;
			continue;
		}

		ex_status = ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_RETRY_IGNORE_CANCEL,
			_("%s during read on %s"),
			strerror (errno),
			dev->path);

		switch (ex_status)
		{
			case PED_EXCEPTION_IGNORE:
				return 1;
			case PED_EXCEPTION_RETRY:
				break;
			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				return 0;
		}
	}

	return 1;
}

static int
beos_write (PedDevice* dev, const void* buffer, PedSector start,
	PedSector count)
{
	BEOSSpecific* arch_specific = BEOS_SPECIFIC(dev);
	int                     status;
	PedExceptionOption      ex_status;
	size_t                  write_length = count * dev->sector_size;

	PED_ASSERT(dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

	if (dev->read_only)
	{
		if (ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Can't write to %s, because it is opened read-only."),
			dev->path)
			!= PED_EXCEPTION_IGNORE)
			return 0;
		else
			return 1;
	}

	while(1)
	{
		if (lseek(arch_specific->fd,start * dev->sector_size,SEEK_SET)
			== start * dev->sector_size)
			break;

		ex_status = ped_exception_throw (
			PED_EXCEPTION_ERROR, PED_EXCEPTION_RETRY_IGNORE_CANCEL,
			_("%s during seek for write on %s"),
			strerror (errno), dev->path);

		switch (ex_status)
		{
			case PED_EXCEPTION_IGNORE:
				return 1;
			case PED_EXCEPTION_RETRY:
				break;
			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				return 0;
		}
	}

#ifdef READ_ONLY
	printf ("ped_device_write (\"%s\", %p, %d, %d)\n",
		dev->path, buffer, (int) start, (int) count);
#else
	dev->dirty = 1;
	while(1)
	{
		status = write (arch_specific->fd, buffer, write_length);
		if (status == count * dev->sector_size)
			break;

		if (status > 0)
		{
			write_length -= status;
			buffer += status;
			continue;
		}

		ex_status = ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_RETRY_IGNORE_CANCEL,
			_("%s during write on %s"),
			strerror (errno), dev->path);

		switch (ex_status)
		{
			case PED_EXCEPTION_IGNORE:
				return 1;
			case PED_EXCEPTION_RETRY:
				break;
			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				return 0;
		}
	}
#endif /* !READ_ONLY */

	return 1;
}

static PedSector
beos_check (PedDevice* dev, void* buffer, PedSector start, PedSector count)
{
	BEOSSpecific*	arch_specific = BEOS_SPECIFIC(dev);
	PedSector		done = 0;
	int				status;

	PED_ASSERT(dev != NULL);

	if (lseek(arch_specific->fd, start * dev->sector_size, SEEK_SET)
		!= start * dev->sector_size)
		return 0;

	for (done = 0; done < count; done += status / dev->sector_size)
	{
		status = read (arch_specific->fd, buffer,
                               (size_t) ((count-done) * dev->sector_size));
		if (status < 0)
			break;
	}

	return done;
}

static int
beos_sync (PedDevice* dev)
{
	return 1;
}

static int
beos_sync_fast (PedDevice* dev)
{
	return 1;
}

/* Probe for all available disks */
static void
beos_probe_all ()
{
	/* For BeOS/ZETA/Haiku, all disks are published under /dev/disk */
	_scan_for_disks("/dev/disk");
}

static char*
beos_partition_get_path (const PedPartition* part)
{
	return NULL;
}

static int
beos_partition_is_busy (const PedPartition* part)
{
	return 0;
}

static int
beos_disk_commit (PedDisk* disk)
{
	return 0;
}

static PedDeviceArchOps beos_dev_ops = {
        _new:           beos_new,
        destroy:        beos_destroy,
        is_busy:        beos_is_busy,
        open:           beos_open,
        refresh_open:   beos_refresh_open,
        close:          beos_close,
        refresh_close:  beos_refresh_close,
        read:           beos_read,
        write:          beos_write,
        check:          beos_check,
        sync:           beos_sync,
        sync_fast:      beos_sync_fast,
        probe_all:      beos_probe_all
};

static PedDiskArchOps beos_disk_ops =  {
        partition_get_path:     beos_partition_get_path,
        partition_is_busy:      beos_partition_is_busy,
        disk_commit:            beos_disk_commit
};

PedArchitecture ped_beos_arch = {
        dev_ops:        &beos_dev_ops,
        disk_ops:       &beos_disk_ops
};
