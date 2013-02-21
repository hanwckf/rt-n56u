/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2001, 2005, 2007, 2009-2012 Free Software Foundation,
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

#include <config.h>

#include <parted/parted.h>
#include <parted/debug.h>

#include <errno.h>
#include <hurd.h>
#include <hurd/fs.h>
#include <hurd/store.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "../architecture.h"

#define GNU_SPECIFIC(dev)	((GNUSpecific*) (dev)->arch_specific)

typedef	struct _GNUSpecific	GNUSpecific;

struct _GNUSpecific {
	struct store*	store;
	int consume;
};

/* Initialize a PedDevice using SOURCE.  The SOURCE will NOT be destroyed;
   the caller created it, it is the caller's responsilbility to free it
   after it calls ped_device_destory.  SOURCE is not registered in Parted's
   list of devices.  */
PedDevice* ped_device_new_from_store (struct store *source);

static int
_device_get_sector_size (PedDevice* dev)
{
	GNUSpecific*	arch_specific = GNU_SPECIFIC (dev);
	size_t		store_block_size = arch_specific->store->block_size;

	return PED_SECTOR_SIZE_DEFAULT;
}

static PedSector
_device_get_length (PedDevice* dev)
{
	GNUSpecific*	arch_specific = GNU_SPECIFIC (dev);
	size_t		store_blocks = arch_specific->store->blocks;
	size_t		store_block_size = arch_specific->store->block_size;

	return ((long long) store_blocks * store_block_size) / PED_SECTOR_SIZE_DEFAULT;
}

static int
_device_probe_geometry (PedDevice* dev)
{
	PedSector cyl_size;

	dev->length = _device_get_length (dev);
	if (!dev->length)
		return 0;

	dev->sector_size = _device_get_sector_size (dev);
	if (!dev->sector_size)
		return 0;

	/* XXX: We have no way to get this!  */
	dev->bios_geom.sectors = 63;
	dev->bios_geom.heads = 255;
	cyl_size = dev->bios_geom.sectors * dev->bios_geom.heads;
	dev->bios_geom.cylinders = dev->length / cyl_size
					* (dev->sector_size / PED_SECTOR_SIZE_DEFAULT);
	dev->hw_geom = dev->bios_geom;

	return 1;
}

static int
init_file (PedDevice* dev)
{
	PedExceptionOption	ex_status;

retry_open:
	if (!ped_device_open (dev)) {
		ex_status = ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_RETRY_CANCEL,
				_("Unable to open %s."),
				dev->path);
		switch (ex_status) {
			case PED_EXCEPTION_RETRY:
				goto retry_open;

			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				goto error;
		}

		return 0;
	}

retry_probe:
	if (!_device_probe_geometry (dev)) {
		ex_status = ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_RETRY_CANCEL,
				_("Unable to probe store."));
		switch (ex_status) {
			case PED_EXCEPTION_RETRY:
				goto retry_probe;

			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				goto error_close_dev;
		}

		return 0;
	}

	dev->model = "";

	ped_device_close (dev);
	return 1;

error_close_dev:
	ped_device_close (dev);
error:
	return 0;
}

static void
_flush_cache (PedDevice* dev)
{
	GNUSpecific*	arch_specific = GNU_SPECIFIC (dev);

	if (dev->read_only)
		return;

	/* Wait for a complete sync to finish.  */
	file_sync (arch_specific->store->source, 1, 0);
}

/* Initialize by allocating memory and filling in a few defaults, a
   PedDevice structure.  */
static PedDevice*
_init_device (const char *path)
{
	PedDevice *dev;
	GNUSpecific*	arch_specific;

	dev = (PedDevice*) ped_malloc (sizeof (PedDevice));
	if (!dev)
		goto error;

	dev->path = strdup (path);
	if (!dev->path)
		goto error_free_dev;

	dev->arch_specific
		= (GNUSpecific*) ped_malloc (sizeof (GNUSpecific));
	if (!dev->arch_specific)
		goto error_free_path;

	dev->type = PED_DEVICE_FILE;	/* FIXME? */
	dev->open_count = 0;
	dev->read_only = 0;
	dev->external_mode = 0;
	dev->dirty = 0;
	dev->boot_dirty = 0;

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

static int
_kernel_reread_part_table (PedDevice* dev)
{
	/* XXX: We must wait for partfs to be finished.  */
	return 1;
}

/* Free the memory associated with a PedDevice structure.  */
static void
_done_device (PedDevice *dev)
{
        free (dev->arch_specific);
	free (dev->path);
	free (dev);
}

/* Release all resources that libparted owns in DEV.  */
static void
gnu_destroy (PedDevice* dev)
{
	GNUSpecific*	arch_specific = GNU_SPECIFIC (dev);

	if (arch_specific->consume)
	        store_free (arch_specific->store);

	_done_device (dev);
}

static PedDevice*
gnu_new (const char* path)
{
	PedDevice*	dev;
	GNUSpecific*	arch_specific;
	error_t         ro_err, rw_err;
	int             ispath;

	PED_ASSERT (path != NULL);

	dev = _init_device (path);
	if (!dev)
	        return NULL;

	arch_specific = GNU_SPECIFIC (dev);
	arch_specific->consume = 1;

 retry_open:
	/* Try read-write. */
	if (strchr (dev->path, '/') != NULL) {
	        /* We set this to prevent having to use strchr more then once. */
	        ispath = 1;

	        rw_err = store_open (dev->path, 0, NULL, &arch_specific->store);
	} else {
	        rw_err = store_typed_open (dev->path, 0, NULL, &arch_specific->store);
	}

	/* Try readonly. */
	if (rw_err) {
	  if (ispath) {
	        ro_err = store_open (dev->path, STORE_READONLY, NULL,
				 &arch_specific->store);
	  } else {
	        ro_err = store_typed_open (dev->path, STORE_READONLY, NULL,
				       &arch_specific->store);
	  }

	if (ro_err) {
	        if (ped_exception_throw (
			        PED_EXCEPTION_ERROR,
				PED_EXCEPTION_RETRY_CANCEL,
				_("Error opening %s: %s"),
				dev->path, strerror (ro_err))
					!= PED_EXCEPTION_RETRY) {
				return NULL;
			} else
				goto retry_open;
		} else {
			ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_OK,
				_("Unable to open %s read-write (%s).  %s has "
				  "been opened read-only."),
				dev->path, strerror (rw_err), dev->path);
			dev->read_only = 1;
		}
	} else {
		dev->read_only = 0;
	}

	_flush_cache (dev);

	if (!init_file (dev)) {
		gnu_destroy(dev);
		return NULL;
	}

	return dev;
}

PedDevice*
ped_device_new_from_store (struct store *source)
{
        PedDevice*      dev;
	GNUSpecific*    arch_specific;

	PED_ASSERT (source != NULL);

	dev = _init_device (source->name ?: "(unknown)");
	if (!dev)
	        return NULL;

	arch_specific = GNU_SPECIFIC (dev);
	arch_specific->store = source;
	arch_specific->consume = 0;

	dev->read_only = source->flags & (STORE_READONLY|STORE_HARD_READONLY);

	if (!init_file (dev)) {
	        _done_device (dev);
		return NULL;
	}

	return dev;
}

static int
gnu_is_busy (PedDevice* dev)
{
	return 0;
}

static int
gnu_open (PedDevice* dev)
{
        return 1;
}

static int
gnu_refresh_open (PedDevice* dev)
{
	return 1;
}

static int
gnu_close (PedDevice* dev)
{
	GNUSpecific*	arch_specific = GNU_SPECIFIC (dev);

	_flush_cache (dev);

	if (dev->dirty && dev->type != PED_DEVICE_FILE) {
		if (_kernel_reread_part_table (dev))
			dev->dirty = 0;
	}

#if 0
	if (dev->dirty && dev->boot_dirty && dev->type != PED_DEVICE_FILE) {
		/* ouch! */
		ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_OK,
			_("The partition table cannot be re-read.  This means "
			  "you need to reboot before mounting any "
			  "modified partitions.  You also need to reinstall "
			  "your boot loader before you reboot (which may "
			  "require mounting modified partitions).  It is "
			  "impossible do both things!  So you'll need to "
			  "boot off a rescue disk, and reinstall your boot "
			  "loader from the rescue disk.  Read section 4 of "
			  "the Parted User documentation for more "
			  "information."));
		return 1;
	}

	if (dev->dirty && dev->type != PED_DEVICE_FILE) {
		ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_IGNORE,
			_("The partition table on %s cannot be re-read "
			  "(%s).  This means the Hurd knows nothing about any "
			  "modifications you made.  You should reboot your "
			  "computer before doing anything with %s."),
			dev->path, strerror (errno), dev->path);
	}

	if (dev->boot_dirty && dev->type != PED_DEVICE_FILE) {
		ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_OK,
			_("You should reinstall your boot loader before "
			  "rebooting.  Read section 4 of the Parted User "
			  "documentation for more information."));
	}
#endif

	return 1;
}

static int
gnu_refresh_close (PedDevice* dev)
{
	_flush_cache (dev);
	return 1;
}

static int
gnu_read (const PedDevice* dev, void* user_buffer, PedSector device_start,
          PedSector count)
{
	GNUSpecific*		arch_specific = GNU_SPECIFIC (dev);
	error_t			err;
	PedExceptionOption	ex_status;
	size_t			start;
	size_t			store_start_block;
	/* In bytes.  This can be larger than COUNT when store pages are
	   larger than PED_SECTOR_SIZE_DEFAULT.  */
	size_t			store_read_length;
	char			local_buffer[PED_SECTOR_SIZE_DEFAULT];
	void *			store_read_buffer;
	size_t			have_read;
	size_t			read_offset;
	size_t			device_read_length = count * PED_SECTOR_SIZE_DEFAULT;

	start = device_start * PED_SECTOR_SIZE_DEFAULT;
	if (PED_SECTOR_SIZE_DEFAULT != arch_specific->store->block_size) {
		store_start_block = start / arch_specific->store->block_size;
		store_read_length = (device_read_length
				     + arch_specific->store->block_size - 1)
				    / arch_specific->store->block_size;
	} else {
		store_start_block = device_start;
		store_read_length = device_read_length;
	}

	read_offset = start
		      - store_start_block * arch_specific->store->block_size;

	if (store_read_length % arch_specific->store->block_size != 0)
		store_read_length = store_read_length
				    + arch_specific->store->block_size
				    - store_read_length % arch_specific->store->block_size;

retry:
	have_read = 0;
	while (1) {
		size_t	did_read;
		size_t	offset;

		store_read_buffer = local_buffer;
		did_read = sizeof (local_buffer);

		err = store_read (arch_specific->store, store_start_block,
				  store_read_length - have_read,
			  	  &store_read_buffer, &did_read);
		if (err) {
			ex_status = ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_RETRY_IGNORE_CANCEL,
				_("%s during read on %s"),
				strerror (err),
				dev->path);

			switch (ex_status) {
				case PED_EXCEPTION_IGNORE:
					return 1;

				case PED_EXCEPTION_RETRY:
					goto retry;

				case PED_EXCEPTION_UNHANDLED:
					ped_exception_catch ();
				case PED_EXCEPTION_CANCEL:
					return 0;
			}
		}

		memcpy (user_buffer + have_read - read_offset,
			store_read_buffer
			+ (have_read >= read_offset
				? 0 : read_offset - have_read),
			have_read + did_read > device_read_length + read_offset
				? device_read_length + read_offset - have_read
				: did_read);

		if (store_read_buffer != local_buffer)
			vm_deallocate (mach_task_self (),
				       (long) store_read_buffer, did_read);

		have_read += did_read;
		store_start_block += did_read
		       			/ arch_specific->store->block_size;

		if (have_read >= device_read_length)
			break;
	}

	return 1;
}

static int
gnu_write (PedDevice* dev, const void* buffer, PedSector start, PedSector count)
{
	GNUSpecific*		arch_specific = GNU_SPECIFIC (dev);
	error_t			err;
	PedExceptionOption	ex_status;
	void *			temp;
	char			local_buffer[PED_SECTOR_SIZE_DEFAULT];
	size_t			did_read;
	size_t			did_write;

	/* Map a disk sector to a store sector.  */
	#define PED_TO_STORE(store, sector) (((sector) * PED_SECTOR_SIZE_DEFAULT) \
				      	/ (store)->block_size)

	if (dev->read_only) {
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

#ifdef READ_ONLY
	printf ("ped_device_write (\"%s\", %p, %d, %d)\n",
		dev->path, buffer, (int) start, (int) count);
#else
	dev->dirty = 1;

	/* If the first ``device'' block (PedSector) is not aligned on a
	   store block, then we need to fetch the old block, copy in the
	   overlaping area and finally, write the modified data out to the
	   store.  */
	if ((PED_SECTOR_SIZE_DEFAULT * start) % arch_specific->store->block_size
	    != 0) {
		size_t 		write_offset;
		size_t 		flushing;

doggy_first_block_read:
		/* We do not bother looping as we are only reading a
		   single block.  */
		temp = local_buffer;
		did_read = sizeof (local_buffer);
		err = store_read (arch_specific->store,
				  PED_TO_STORE (arch_specific->store, start),
				  arch_specific->store->block_size, &temp,
				  &did_read);
		if (! err && did_read != arch_specific->store->block_size)
			err = EIO;

		if (err) {
			ex_status = ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_RETRY_IGNORE_CANCEL,
				_("%s during read on %s"),
				strerror (err), dev->path);

			switch (ex_status) {
				case PED_EXCEPTION_IGNORE:
					break;

				case PED_EXCEPTION_RETRY:
					goto doggy_first_block_read;

				case PED_EXCEPTION_UNHANDLED:
					ped_exception_catch ();
				case PED_EXCEPTION_CANCEL:
					return 0;
			}
		}

		write_offset = (start * PED_SECTOR_SIZE_DEFAULT)
			       % arch_specific->store->block_size;
		flushing = arch_specific->store->block_size - write_offset;
		if (flushing > count * PED_SECTOR_SIZE_DEFAULT)
			flushing = count * PED_SECTOR_SIZE_DEFAULT;

		memcpy (temp + write_offset, buffer, flushing);

doggy_first_block_write:
		err = store_write (arch_specific->store,
		    		   PED_TO_STORE (arch_specific->store, start),
				   temp, arch_specific->store->block_size,
				   &did_write);
		if (! err && did_write != arch_specific->store->block_size)
			err = EIO;

		if (err) {
			ex_status = ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_RETRY_IGNORE_CANCEL,
				_("%s during write on %s"),
				strerror (err), dev->path);

			switch (ex_status) {
				case PED_EXCEPTION_IGNORE:
					break;

				case PED_EXCEPTION_RETRY:
					goto doggy_first_block_write;

				case PED_EXCEPTION_UNHANDLED:
					ped_exception_catch ();
				case PED_EXCEPTION_CANCEL:
					if (temp != local_buffer)
						vm_deallocate (
							mach_task_self (),
							(long) temp,
							did_read);
					return 0;
			}
		}

		start += flushing / PED_SECTOR_SIZE_DEFAULT;
		count -= flushing / PED_SECTOR_SIZE_DEFAULT;
		buffer += write_offset;

		if (temp != local_buffer)
			vm_deallocate (mach_task_self (), (long) temp,
				       did_read);

		if (count == 0)
			return 1;
	}

	while (count > 0
	   && count >= arch_specific->store->block_size / PED_SECTOR_SIZE_DEFAULT) {
		err = store_write (arch_specific->store,
				   PED_TO_STORE (arch_specific->store, start),
				   buffer, count * PED_SECTOR_SIZE_DEFAULT,
				   &did_write);

		if (err) {
			ex_status = ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_RETRY_IGNORE_CANCEL,
				_("%s during write on %s"),
				strerror (err), dev->path);

			switch (ex_status) {
				case PED_EXCEPTION_IGNORE:
					break;

				case PED_EXCEPTION_RETRY:
					continue;

				case PED_EXCEPTION_UNHANDLED:
					ped_exception_catch ();
				case PED_EXCEPTION_CANCEL:
					return 0;
			}
		}

		start += did_write / PED_SECTOR_SIZE_DEFAULT;
		count -= did_write / PED_SECTOR_SIZE_DEFAULT;
		buffer += did_write;
	}

	if (count == 0)
		return 1;

	/* We are now left with (strictly) less then a store block to write
	   to disk.  Thus, we read the block, overlay the buffer and flush.  */
	PED_ASSERT (count * PED_SECTOR_SIZE_DEFAULT
		    < arch_specific->store->block_size);

doggy_last_block_read:
	/* We do not bother looping as we are only reading a
	   single block.  */
	temp = local_buffer;
	did_read = sizeof (local_buffer);
	err = store_read (arch_specific->store,
			  PED_TO_STORE (arch_specific->store, start),
			  arch_specific->store->block_size, &temp,
			  &did_read);
	if (! err && did_read != arch_specific->store->block_size)
		err = EIO;

	if (err) {
		ex_status = ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_RETRY_IGNORE_CANCEL,
			_("%s during read on %s"),
			strerror (err), dev->path);

		switch (ex_status) {
			case PED_EXCEPTION_IGNORE:
				break;

			case PED_EXCEPTION_RETRY:
				goto doggy_last_block_read;

			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				return 0;
		}
	}

	memcpy (temp, buffer, count * PED_SECTOR_SIZE_DEFAULT);

doggy_last_block_write:
	err = store_write (arch_specific->store,
	    		   PED_TO_STORE (arch_specific->store, start),
			   temp, arch_specific->store->block_size,
			   &did_write);
	if (! err && did_write != arch_specific->store->block_size)
		err = EIO;

	if (err) {
		ex_status = ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_RETRY_IGNORE_CANCEL,
			_("%s during write on %s"),
			strerror (err), dev->path);

		switch (ex_status) {
			case PED_EXCEPTION_IGNORE:
				break;

			case PED_EXCEPTION_RETRY:
				goto doggy_last_block_write;

			case PED_EXCEPTION_UNHANDLED:
				ped_exception_catch ();
			case PED_EXCEPTION_CANCEL:
				if (temp != local_buffer)
					vm_deallocate (mach_task_self (),
					       	       (long) temp,
					       	       did_read);
				return 0;
		}
	}

#endif /* !READ_ONLY */
	return 1;
}

/* TODO: returns the number of sectors that are ok.
 */
static PedSector
gnu_check (PedDevice* dev, void* buffer, PedSector start, PedSector count)
{
	int			status;
	int			done = 0;

	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);
	PED_ASSERT (buffer != NULL);

	return count;
}

static int
gnu_sync (PedDevice* dev)
{
	GNUSpecific*		arch_specific;
	error_t			err;
	PedExceptionOption	ex_status;
	static char *last_failure = NULL;

	PED_ASSERT (dev != NULL);
	PED_ASSERT (!dev->external_mode);

	arch_specific = GNU_SPECIFIC (dev);

	if (dev->read_only || ! dev->dirty)
		return 1;

	while (1) {
		err = file_sync (arch_specific->store->source, 1, 0);
		if (! err || err == EOPNOTSUPP || err == EPERM
		    || (last_failure && strcmp (last_failure, dev->path) == 0))
		  break;

		ex_status = ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_RETRY_IGNORE_CANCEL,
			_("%s trying to sync %s to disk"),
			strerror (errno), dev->path);

		switch (ex_status) {
			case PED_EXCEPTION_IGNORE:
				free (last_failure);
				last_failure = strdup (dev->path);
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
probe_standard_devices ()
{
	_ped_device_probe ("/dev/sd0");
	_ped_device_probe ("/dev/sd1");
	_ped_device_probe ("/dev/sd2");
	_ped_device_probe ("/dev/sd3");
	_ped_device_probe ("/dev/sd4");
	_ped_device_probe ("/dev/sd5");

	_ped_device_probe ("/dev/hd0");
	_ped_device_probe ("/dev/hd1");
	_ped_device_probe ("/dev/hd2");
	_ped_device_probe ("/dev/hd3");
	_ped_device_probe ("/dev/hd4");
	_ped_device_probe ("/dev/hd5");
	_ped_device_probe ("/dev/hd6");
	_ped_device_probe ("/dev/hd7");

	return 1;
}

static void
gnu_probe_all ()
{
	probe_standard_devices ();
}

static char*
gnu_partition_get_path (const PedPartition* part)
{
	const char*	dev_path = part->disk->dev->path;
	int		result_len = strlen (dev_path) + 16;
	char*		result;

	result = (char*) ped_malloc (result_len);
	if (!result)
		return NULL;
	snprintf (result, result_len, "%s%d", dev_path, part->num);
	return result;
}

static int
gnu_partition_is_busy (const PedPartition* part)
{
	return 0;
}

static int
gnu_disk_commit (PedDisk* disk)
{
	return 1;
}

static PedDeviceArchOps gnu_dev_ops = {
	_new:		gnu_new,
	destroy:	gnu_destroy,
	is_busy:	gnu_is_busy,
	open:		gnu_open,
	refresh_open:	gnu_refresh_open,
	close:		gnu_close,
	refresh_close:	gnu_refresh_close,
	read:		gnu_read,
	write:		gnu_write,
	check:		gnu_check,
	sync:		gnu_sync,
	sync_fast:	gnu_sync,
	probe_all:	gnu_probe_all
};

static PedDiskArchOps gnu_disk_ops = {
	partition_get_path:	gnu_partition_get_path,
	partition_is_busy:	gnu_partition_is_busy,
	disk_commit:		gnu_disk_commit
};

PedArchitecture ped_gnu_arch = {
	dev_ops:	&gnu_dev_ops,
	disk_ops:	&gnu_disk_ops
};
