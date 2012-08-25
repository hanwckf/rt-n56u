/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * NVRAM variable manipulation (Linux user mode half)
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram_linux.c,v 1.1 2007/06/12 02:28:08 arthur Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <nvram/bcmnvram.h>

#define PATH_DEV_NVRAM "/dev/nvram"

/* Globals */
static int nvram_fd = -1;
static char *nvram_buf = NULL;
static char nvram_empty[4] = {0};

int
nvram_init(void)
{
	if ((nvram_fd = open(PATH_DEV_NVRAM, O_RDWR)) < 0)
		goto err;

	/* Map kernel string buffer into user space */
	if ((nvram_buf = mmap(NULL, NVRAM_SPACE, PROT_READ, MAP_SHARED, nvram_fd, 0)) == MAP_FAILED) {
		fprintf (stderr, "%s() mmap fail, return 0x%p\n", __FUNCTION__, nvram_buf); //eric++
		close(nvram_fd);
		nvram_fd = -1;
		goto err;
	}

	return 0;

err:
	perror(PATH_DEV_NVRAM);
	return errno;
}

void
nvram_exit(void)
{
	if (nvram_fd) {
		close(nvram_fd);
		nvram_fd = -1;
	}
}

char *
nvram_get_(const char *name)
{
	size_t count = strlen(name) + 1;
	char tmp[NVRAM_MAX_PARAM_LEN], *value;
	unsigned long *off = (unsigned long *) tmp;

	if (nvram_fd < 0)
		if (nvram_init())
			return NULL;

	if (count > sizeof(tmp)) {
		if (!(off = malloc(count)))
			return NULL;
	}

	/* Get offset into mmap() space */
	strcpy((char *) off, name);
	count = read(nvram_fd, off, count);

	if (count == sizeof(unsigned long))
		value = &nvram_buf[*off];
	else
		value = NULL;

	if (count < 0)
		perror(PATH_DEV_NVRAM);

	if (off != (unsigned long *) tmp)
		free(off);

	return value;
}

char *
nvram_get(const char *name)
{
	char *value = nvram_get_(name);
	if (value > 0)
		return value;
	else
		return NULL;
}

char *
nvram_safe_get(const char *name)
{
	char *value = nvram_get_(name);
	if (value > 0)
		return value;
	else
		return nvram_empty;
}

int
nvram_get_int(const char *name)
{
	char *value = nvram_get_(name);
	if (value > 0)
		return atoi(value);
	else
		return 0;
}

int
nvram_getall(char *buf, int count)
{
	int ret;

	if (nvram_fd < 0)
		if ((ret = nvram_init()))
			return ret;
	
	if (!buf || count == 0)
		return 0;

	/* Get all variables */
	*buf = '\0';

	ret = read(nvram_fd, buf, count);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	return (ret == count) ? 0 : ret;
}

int
_nvram_set(const char *name, const char *value)
{
	size_t count = strlen(name) + 1;
	char tmp[256], *buf = tmp;
	int ret;

	if (nvram_fd < 0)
		if ((ret = nvram_init()))
			return ret;

	/* Unset if value is NULL */
	if (value)
		count += strlen(value) + 1;

	if (count > sizeof(tmp)) {
		if (!(buf = malloc(count)))
			return -ENOMEM;
	}

	if (value)
		sprintf(buf, "%s=%s", name, value);
	else
		strcpy(buf, name);

	ret = write(nvram_fd, buf, count);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	if (buf != tmp)
		free(buf);

	return (ret == count) ? 0 : ret;
}

int
nvram_set(const char *name, const char *value)
{
	return _nvram_set(name, value);
}

int nvram_set_int(const char *name, int value)
{
	char int_str[16];
	sprintf(int_str, "%d", value);
	return _nvram_set(name, int_str);
}

int
nvram_unset(const char *name)
{
	return _nvram_set(name, NULL);
}

int
nvram_match(const char *name, char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

int
nvram_invmatch(const char *name, char *invmatch)
{
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}

int
nvram_commit(void)
{
	int ret;

	if (nvram_fd < 0)
		if ((ret = nvram_init()))
			return ret;

	ret = ioctl(nvram_fd, NVRAM_MAGIC, 0);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	return ret;
}

int
nvram_clear(void)
{
	int ret;

	if (nvram_fd < 0)
		if ((ret = nvram_init()))
			return ret;

	ret = ioctl(nvram_fd, NVRAM_MAGIC, 1);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	return ret;
}
