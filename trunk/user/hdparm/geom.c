/*
 * Device geometry helpers for hdparm and friends.
 * Copyright (c) Mark Lord 2008
 *
 * You may use/distribute this freely, under the terms of either
 * (your choice) the GNU General Public License version 2,
 * or a BSD style license.
 */
#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "hdparm.h"

static int get_driver_major (const char *driver, unsigned int *major)
{
	static const char proc_devices[] = "/proc/devices";
	char buf[256];
	int err = 0;
	FILE *fp = fopen(proc_devices, "r");

	if (fp == NULL) {
		err = EIO;
	} else {
		while (fgets(buf, sizeof(buf) - 1, fp)) {
			int len = strlen(buf);
			if (len > 5 && buf[len - 1] == '\n') {
				buf[len - 1] = '\0';
				if (buf[3] == ' ' && 0 == strcmp(buf + 4, driver)) {
					*major = atoi(buf);
					break;
				}
			}
		}
	}
	if (err)
		perror(proc_devices);
	if (fp)
		fclose(fp);
	return err;
}

static unsigned int md_major (void)
{
	static unsigned int maj = 0;

	if (!maj) {
		unsigned int val;
		if (0 == get_driver_major("md", &val))
			maj = val;
	}
	return maj;
}

int fd_is_raid (int fd)
{
	struct stat st;

	if (!md_major())
		return 0;  /* not a RAID device */
	if (fstat(fd, &st)) {
		perror("fstat()");
		return 0;  /* ugh.. shouldn't happen */
	}
	return (major(st.st_rdev) == md_major());
}

static int get_sector_count (int fd, __u64 *nsectors)
{
	int		err;
	unsigned int	nsects32 = 0;
	__u64		nbytes64 = 0;

	if (0 == sysfs_get_attr(fd, "size", "%llu", nsectors, NULL, 0))
		return 0;
#ifdef BLKGETSIZE64
	if (0 == ioctl(fd, BLKGETSIZE64, &nbytes64)) {	// returns bytes
		*nsectors = nbytes64 / 512;
		return 0;
	}
#endif
	err = ioctl(fd, BLKGETSIZE, &nsects32);	// returns sectors
	if (err == 0) {
		*nsectors = nsects32;
	} else {
		err = errno;
		perror(" BLKGETSIZE failed");
	}
	return err;
}

/*
 * "md" (RAID) devices have per-member "start" offsets.
 * Realistically, we can only support raid1 arrays here,
 * and only then when all members have the same "start" offsets.
 */
static int get_raid1_start_lba (int fd, __u64 *start_lba)
{
	char buf[32];
	unsigned int member, raid_disks;
	__u64 start = 0, offset = 0;

	if (sysfs_get_attr(fd, "md/level",      "%s", buf,         NULL, 0)
	 || sysfs_get_attr(fd, "md/raid_disks", "%u", &raid_disks, NULL, 0))
		return ENODEV;
	if (strcmp(buf, "raid1") || !raid_disks)
		return EINVAL;
	for (member = 0; member < raid_disks; ++member) {
		__u64 member_start, member_offset;
		char member_path[32];
		sprintf(member_path, "md/rd%u/offset", member);
		if (sysfs_get_attr(fd, member_path, "%llu", &member_offset, NULL, 0))
			member_offset = 0;
		sprintf(member_path, "md/rd%u/block/dev", member);
		if (sysfs_get_attr(fd, member_path, "%s", buf, NULL, 0))
			return EINVAL;
		if (md_major() == (unsigned)atoi(buf))  /* disallow recursive RAIDs */
			return EINVAL;
		sprintf(member_path, "md/rd%u/block/start", member);
		if (sysfs_get_attr(fd, member_path, "%llu", &member_start, NULL, 0))
			return ENODEV;
		if (member == 0) {
			start  = member_start;
			offset = member_offset;
		} else if (member_start != start || member_offset != offset)
			return EINVAL;
		/* FIXME?  Should --fibmap should account for member_offset in calculations? */
	}
	*start_lba = start;
	return 0;
}

int get_dev_geometry (int fd, __u32 *cyls, __u32 *heads, __u32 *sects,
				__u64 *start_lba, __u64 *nsectors)
{
	static struct local_hd_geometry      g;
	static struct local_hd_big_geometry bg;
	int err = 0, try_getgeo_big_first = 1;
	int sector_bytes = get_current_sector_size(fd);

	if (nsectors) {
		err = get_sector_count(fd, nsectors);
		if (err)
			return err;
	}

	if (start_lba) {
		/*
		 * HDIO_GETGEO uses 32-bit fields on 32-bit architectures,
		 * so it cannot be relied upon for start_lba with very large drives >= 2TB.
		 */
		__u64 result;
		if (0 == sysfs_get_attr(fd, "start", "%llu", &result, NULL, 0)) {
			result /= (sector_bytes / 512);   /* sysfs entry is broken for non-512byte sectors */
			*start_lba = result;
			start_lba = NULL;
			try_getgeo_big_first = 0;	/* if kernel has sysfs, it probably lacks GETGEO_BIG */
		} else if (0 == get_raid1_start_lba(fd, &result)) {
			*start_lba = result;
			 start_lba = NULL;
			try_getgeo_big_first = 0;	/* if kernel has sysfs, it probably lacks GETGEO_BIG */
		} else if (fd_is_raid(fd)) {
			*start_lba = START_LBA_UNKNOWN;	/* RAID: no such thing as a "start_lba" */
			 start_lba = NULL;
			try_getgeo_big_first = 0;	/* no point even trying it on RAID */
		}
	}

	if (cyls || heads || sects || start_lba) {
		/* Skip HDIO_GETGEO_BIG (doesn't exist) on kernels with sysfs (>= 2.6.xx) */
		if (try_getgeo_big_first && !ioctl(fd, HDIO_GETGEO_BIG, &bg)) {
			if (cyls)	*cyls  = bg.cylinders;
			if (heads)	*heads = bg.heads;
			if (sects)	*sects = bg.sectors;
			if (start_lba)	*start_lba = bg.start;
		} else if (!ioctl(fd, HDIO_GETGEO, &g)) {
			if (cyls)	*cyls  = g.cylinders;
			if (heads)	*heads = g.heads;
			if (sects)	*sects = g.sectors;
			if (start_lba)	*start_lba = g.start;
		} else if (!try_getgeo_big_first && !ioctl(fd, HDIO_GETGEO_BIG, &bg)) {
			if (cyls)	*cyls  = bg.cylinders;
			if (heads)	*heads = bg.heads;
			if (sects)	*sects = bg.sectors;
			if (start_lba)	*start_lba = bg.start;
		} else {
			err = errno;
			perror(" HDIO_GETGEO failed");
			return err;
		}
		/*
		 * On all (32 and 64 bit) systems, the cyls value is bit-limited.
		 * So try and correct it using other info we have at hand.
		 */
		if (nsectors && cyls && heads && sects
		 && *nsectors && *cyls && *heads && *sects) {
			__u64 hs  = (*heads) * (*sects);
			__u64 cyl = (*cyls);
			__u64 chs = cyl * hs;
			if (chs < (*nsectors))
				*cyls = (*nsectors) / hs;
		}
	}

	return 0;
}

static int find_dev_in_directory (dev_t dev, const char *dir, char *path, int verbose)
{
	DIR *dp;
	struct dirent *entry;
	unsigned int maj = major(dev), min = minor(dev);

	*path = '\0';
	if (!(dp = opendir(dir))) {
		int err = errno;
		if (verbose)
			perror(dir);
		return err;
	}
	while ((entry = readdir(dp)) != NULL) {
		if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_BLK) {
			struct stat st;
			sprintf(path, "%s/%s", dir, entry->d_name);
			if (stat(path, &st)) {
				if (verbose)
					perror(path);
			} else if (S_ISBLK(st.st_mode)) {
				if (maj == (unsigned)major(st.st_rdev) && min == (unsigned)minor(st.st_rdev)) {
					closedir(dp);
					return 0;
				}
			}
		}
	}
	closedir(dp);
	*path = '\0';
	if (verbose)
		fprintf(stderr, "%d,%d: device not found in %s\n", major(dev), minor(dev), dir);
	return ENOENT;
}

int get_dev_t_geometry (dev_t dev, __u32 *cyls, __u32 *heads, __u32 *sects,
				__u64 *start_lba, __u64 *nsectors, unsigned int *sector_bytes)
{
	char path[PATH_MAX];
	int fd, err;

	err = find_dev_in_directory (dev, "/dev", path, 1);
	if (err)
		return err;

	fd = open(path, O_RDONLY|O_NONBLOCK);
	if (fd == -1) {
		err = errno;
		perror(path);
		return err;
	}
	*sector_bytes = get_current_sector_size(fd);

	err = get_dev_geometry(fd, cyls, heads, sects, start_lba, nsectors);
	close(fd);
	return err;
}

