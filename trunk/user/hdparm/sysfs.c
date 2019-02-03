/*
 * Access helpers for sysfs.
 * Copyright (c) Mark Lord 2008
 *
 * You may use/distribute this freely, under the terms of either
 * (your choice) the GNU General Public License version 2,
 * or a BSD style license.
 */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <linux/types.h>

#include "hdparm.h"

static char *path_append (char *path, const char *new)
{
	char *pathtail = path + strlen(path);

	*pathtail = '/';
	strcpy(pathtail+1, new);
	return pathtail;
}

static int sysfs_write_attr (char *path, const char *attr, const char *fmt, void *val, int verbose)
{
	FILE *fp;
	int count = -1, err = 0;
	char *pathtail = path_append(path, attr);

	fp = fopen(path, "w");
	if (!fp) {
		err = errno;
	} else if (fmt[0] != '%') {
		err = EINVAL;
	} else {
		switch (fmt[1]) {
			case 's':
				count = fprintf(fp, fmt, val);
				break;
			case 'd':
			case 'u':
				count = fprintf(fp, fmt, *(unsigned int *)val);
				break;
			case 'l':
				if (fmt[2] == 'l')
					count = fprintf(fp, fmt, *(unsigned long long *)val);
				else
					count = fprintf(fp, fmt, *(unsigned long *)val);
				break;
			default:
				errno = EINVAL;
		}
		if (count < 0)
			err = errno;
		fclose(fp);
	}
	if (err && verbose) perror(path);
	*pathtail = '\0';
	return err;
}

static int sysfs_read_attr (char *path, const char *attr, const char *fmt, void *val1, void *val2, int verbose)
{
	FILE *fp;
	int count, err = 0;
	char *pathtail = path_append(path, attr);

	fp = fopen(path, "r");
	if (!fp) {
		err = errno;
	} else {
		count = fscanf(fp, fmt, val1, val2);
		if (count != (val2 ? 2 : 1))
			err = (count == EOF) ? errno : EINVAL;
		fclose(fp);
	}
	if (err && verbose) perror(path);
	*pathtail = '\0';
	return err;
}

static int sysfs_find_dev2 (char *path, dev_t dev, int recurse, int verbose)
{
	DIR *dp;
	struct dirent *entry;
	char *pathtail;

	if (!(dp = opendir(path))) {
		int err = errno;
		if (verbose) perror(path);
		return err;
	}
	pathtail = path + strlen(path);
	while ((entry = readdir(dp)) != NULL) {
		if ((entry->d_type == DT_DIR || entry->d_type == DT_LNK) && entry->d_name[0] != '.') {
			unsigned int maj, min;
			sprintf(pathtail, "/%s", entry->d_name);
			if (sysfs_read_attr(path, "/dev", "%u:%u", &maj, &min, verbose))
				min = ~minor(dev);
			else if (maj != (unsigned)major(dev))
				continue;
			if (min == (unsigned)minor(dev)
			 || (recurse && sysfs_find_dev2(path, dev, recurse - 1, verbose) == 0)) {
				closedir(dp);
				return 0;
			}
		}
	}
	closedir(dp);
	*pathtail = '\0';
	if (verbose)
		fprintf(stderr, "%u,%u: device not found in /sys\n", major(dev), minor(dev));
	return ENOENT;
}

static int sysfs_find_dev (dev_t dev, char *path, int verbose)
{
	int err, recurse = 1;

	strcpy(path, "/sys/block");
	err = sysfs_find_dev2(path, dev, recurse, 0);
	if (err && verbose)
		fprintf(stderr, "%s(%u:%u): %s\n", __func__,
			major(dev), minor(dev), strerror(err));
	return err;
}

static int get_dev_from_fd (int fd, dev_t *dev, int verbose)
{
	struct stat st;

	if (0 != fstat(fd, &st)) {
		int err = errno;
		if (verbose) perror(" fstat() failed");
		return err;
	}
	if (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode))
		*dev = st.st_rdev;
	else
		*dev = st.st_dev;
	return 0;
}

static int sysfs_find_fd (int fd, char **path_p, int verbose)
{
	static int have_prev = 0;
	static dev_t prev;
	static char path[PATH_MAX];
	dev_t dev;
	int err;

	memset(&dev, 0, sizeof(dev));
	err = get_dev_from_fd(fd, &dev, verbose);
	if (!err) {
		if (have_prev && 0 == memcmp(&dev, &prev, sizeof(dev))) {
			/*re-use previous path, since dev was unchanged from before */
		} else {
			prev = dev;
			have_prev = 1;
			err = sysfs_find_dev(dev, path, verbose);
		}
	}
	if (err)
		have_prev = 0;
	else
		*path_p = path;
	return err;
}

int sysfs_get_attr (int fd, const char *attr, const char *fmt, void *val1, void *val2, int verbose)
{
	char *path;
	int err;

	err = sysfs_find_fd(fd, &path, verbose);
	if (!err)
		err = sysfs_read_attr(path, attr, fmt, val1, val2, verbose);
	return err;
}

int sysfs_set_attr (int fd, const char *attr, const char *fmt, void *val_p, int verbose)
{
	char *path;
	int err;

	err = sysfs_find_fd(fd, &path, verbose);
	if (!err)
		err = sysfs_write_attr(path, attr, fmt, val_p, verbose);
	return err;
}

static int sysfs_find_attr_file_path (const char *start_path, char **dest_path, const char *attr)
{
	static char path[PATH_MAX];
	static char have_prev = 0;
	char file_path[PATH_MAX + FILENAME_MAX];
	struct stat st;
	ino_t stop_inode;
	int depth = 0;

	if (have_prev) {
		*dest_path = path;

		return 0;
	}

	stat("/sys/devices", &st);
	stop_inode = st.st_ino;

	strcpy(path, start_path);

	while (depth++ < 20) {
		strcat(path, "/..");

		if (stat(path, &st) != 0)
			return errno;

		if (st.st_ino == stop_inode)
			return EINVAL;

		strcpy(file_path, path);
		strcat(file_path, "/");
		strcat(file_path, attr);

		if (access(file_path, F_OK | R_OK) == 0) {
			*dest_path = path;

			return 0;
		}
	}

	return EINVAL;
}

int sysfs_get_attr_recursive (int fd, const char *attr, const char *fmt, void *val1, void *val2, int verbose)
{
	char *path;
	char *attr_path;
	int err;

	err = sysfs_find_fd(fd, &path, verbose);
	if (!err) {
		err = sysfs_find_attr_file_path(path, &attr_path, attr);

		if (!err) {
			err = sysfs_read_attr(attr_path, attr, fmt, val1, val2, verbose);
		}
	}

	return err;
}
