/* partition table tools
   Copyright (C) 2008-2012 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <config.h>

#include <string.h>
#include <stdlib.h>

#include <parted/parted.h>
#include <parted/debug.h>

#include "pt-tools.h"

#if ENABLE_NLS
# include <libintl.h>
# define _(String) dgettext (PACKAGE, String)
#else
# define _(String) (String)
#endif /* ENABLE_NLS */

static char zero[16 * 1024];

/* Write a single sector to DISK, filling the first BUFLEN
   bytes of that sector with data from BUF, and NUL-filling
   any remaining bytes.  Return nonzero to indicate success,
   zero otherwise.  */
int
ptt_write_sector (PedDisk const *disk, void const *buf, size_t buflen)
{
  PED_ASSERT (buflen <= disk->dev->sector_size);
  /* Allocate a big enough buffer for ped_device_write.  */
  char *s0 = ped_malloc (disk->dev->sector_size);
  if (s0 == NULL)
    return 0;
  /* Copy boot_code into the first part.  */
  memcpy (s0, buf, buflen);
  char *p = s0 + buflen;
  /* Fill the rest with zeros.  */
  memset (p, 0, disk->dev->sector_size - buflen);
  int write_ok = ped_device_write (disk->dev, s0, 0, 1);
  free (s0);

  return write_ok;
}

/* Read N sectors, starting with sector SECTOR_NUM (which has length
   DEV->sector_size) into malloc'd storage.  If the read fails, free
   the memory and return zero without modifying *BUF.  Otherwise, set
   *BUF to the new buffer and return 1.  */
int
ptt_read_sectors (PedDevice const *dev, PedSector start_sector,
		  PedSector n_sectors, void **buf)
{
  char *b = ped_malloc (n_sectors * dev->sector_size);
  PED_ASSERT (b != NULL);
  if (!ped_device_read (dev, b, start_sector, n_sectors)) {
    free (b);
    return 0;
  }
  *buf = b;
  return 1;
}

/* Read sector, SECTOR_NUM (which has length DEV->sector_size) into malloc'd
   storage.  If the read fails, free the memory and return zero without
   modifying *BUF.  Otherwise, set *BUF to the new buffer and return 1.  */
int
ptt_read_sector (PedDevice const *dev, PedSector sector_num, void **buf)
{
  return ptt_read_sectors (dev, sector_num, 1, buf);
}

/* Zero N sectors of DEV, starting with START.
   Return nonzero to indicate success, zero otherwise.  */
int
ptt_clear_sectors (PedDevice *dev, PedSector start, PedSector n)
{
  PED_ASSERT (dev->sector_size <= sizeof zero);
  PedSector n_z_sectors = sizeof zero / dev->sector_size;
  PedSector n_full = n / n_z_sectors;
  PedSector i;
  for (i = 0; i < n_full; i++)
    {
      if (!ped_device_write (dev, zero, start + n_z_sectors * i, n_z_sectors))
        return 0;
    }

  PedSector rem = n - n_z_sectors * i;
  return (rem == 0
          ? 1 : ped_device_write (dev, zero, start + n_z_sectors * i, rem));
}

/* Zero N sectors of GEOM->dev, starting with GEOM->start + START.
   Return nonzero to indicate success, zero otherwise.  */
int
ptt_geom_clear_sectors (PedGeometry *geom, PedSector start, PedSector n)
{
  return ptt_clear_sectors (geom->dev, geom->start + start, n);
}

#include "pt-limit.c"

/* Throw an exception and return 0 if PART's starting sector number or
   its length is greater than the maximum allowed value for LABEL_TYPE.
   Otherwise, return 1.  */
int
ptt_partition_max_start_len (char const *pt_type, const PedPartition *part)
{
  struct partition_limit const *pt_lim
    = pt_limit_lookup (pt_type, strlen (pt_type));

  /* If we don't have info on the type, return "true".  */
  if (pt_lim == NULL)
    return 1;

  /* If the length in sectors exceeds the limit, you lose.  */
  if (part->geom.length > pt_lim->max_length)
    {
      ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			   _("partition length of %jd sectors exceeds"
			     " the %s-partition-table-imposed maximum"
			     " of %jd"),
			   part->geom.length,
			   pt_type,
			   pt_lim->max_length);
      return 0;
    }

  /* If the starting sector exceeds the limit, you lose.  */
  if (part->geom.start > pt_lim->max_start_sector) {
    ped_exception_throw (
			 PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			 _("starting sector number, %jd exceeds"
			   " the %s-partition-table-imposed maximum"
			   " of %jd"),
			 part->geom.start,
			 pt_type,
			 pt_lim->max_start_sector);
    return 0;
  }

  return 1;
}

/* Set *MAX to the largest representation-imposed starting sector number
   of a partition of type PT_TYPE and return 0.  If PT_TYPE is not
   recognized, return -1.  */
int
ptt_partition_max_start_sector (char const *pt_type, PedSector *max)
{
  struct partition_limit const *pt_lim
    = pt_limit_lookup (pt_type, strlen (pt_type));
  if (pt_lim == NULL)
    return -1;

  *max = pt_lim->max_start_sector;
  return 0;
}

/* Set *MAX to the maximum representable length of a partition of type
   PT_TYPE and return 0.  If PT_TYPE is not recognized, return -1.  */
int
ptt_partition_max_length (char const *pt_type, PedSector *max)
{
  struct partition_limit const *pt_lim
    = pt_limit_lookup (pt_type, strlen (pt_type));
  if (pt_lim == NULL)
    return -1;

  *max = pt_lim->max_length;
  return 0;
}
