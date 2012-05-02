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

#ifdef ASUS_NVRAM
#include <linux/config.h>
#include <nvram/typedefs.h>
#include <nvram/bcmnvram.h>
#else	// !ASUS_NVRAM
#include <typedefs.h>
#include <bcmnvram.h>
#endif	// ASUS_NVRAM

#define PATH_DEV_NVRAM "/dev/nvram"

/* Globals */
static int nvram_fd = -1;
static char *nvram_buf = NULL;
static char *nvram_xfr_buf = NULL;

#ifdef ASUS_NVRAM
#define MAJ_NUM_STR	"major number"
#define NVRAM_DEV_NODE	"/tmp/dev_nvram"

static char BUFFER[2048];

/* Get major number of nvra driver and create an device node
 * return 0: fail
 *	1: success
 */
static int
create_nvram_dev_node (void)
{
	FILE *f;
	int nvram_major = -1;
	char buf[100];
	dev_t dev;

	// If we can not open /dev/nvram, get major number from /proc/nvram and create one.
	if ((f = fopen ("/proc/nvram", "r")) == NULL)	{
		return 0;
	}
		
	// looking for MAJ_NUM_STR
	while (fscanf (f, "%[^\n]%*c", buf) != EOF && strncmp (buf, MAJ_NUM_STR, strlen (MAJ_NUM_STR)))
		;
	
	// get major number
	fclose (f);
	if (strncmp (buf, MAJ_NUM_STR, strlen (MAJ_NUM_STR)) == 0)	{
		char *p = strstr (buf, ":");
		while (p != NULL && *p != '\0')	{
			if (p != NULL && *p >= '0' && *p <= '9')	{
				nvram_major = atoi (p);
				break;
			}
			++p;
		}
	}

	if (nvram_major <= 0)	{
		return 0;
	}

	// Create an device node under /tmp directory for nvram driver
	dev = makedev (nvram_major, 0);
	if (mknod (NVRAM_DEV_NODE, S_IFCHR | 0666, dev))	{
		return 0;
	}

	return 1;
}
#endif	// ASUS_NVRAM

int
nvram_init(void *unused)
{
#ifdef ASUS_NVRAM
	if ((nvram_fd = open(PATH_DEV_NVRAM, O_RDWR)) < 0)	{
		if ((nvram_fd = open(NVRAM_DEV_NODE, O_RDWR)) < 0)	{
			if (create_nvram_dev_node () == 0)	{
				goto err;
			}

			if ((nvram_fd = open(NVRAM_DEV_NODE, O_RDWR)) < 0)	{
				goto err;
			}
		}
	}
#else	// !ASUS_NVRAM
	if ((nvram_fd = open(PATH_DEV_NVRAM, O_RDWR)) < 0)
		goto err;
#endif	// ASUS_NVRAM

	/* Map kernel string buffer into user space */
	if ((nvram_buf = mmap(NULL, NVRAM_SPACE, PROT_READ, MAP_SHARED, nvram_fd, 0)) == MAP_FAILED) {
		fprintf (stderr, "%s() mmap fail, return 0x%p\n", __FUNCTION__, nvram_buf); //eric++
		close(nvram_fd);
		nvram_fd = -1;
		goto err;
	}
//	else
//		fprintf(stderr, "%s() mmap state, return 0x%p\n", __FUNCTION__, nvram_buf);

	return 0;

err:
	perror(PATH_DEV_NVRAM);
	return errno;
}

char *
nvram_get_(const char *name)
{
	size_t count = strlen(name) + 1;
	char tmp[128], *value;
	unsigned long *off = (unsigned long *) tmp;

	if (nvram_fd < 0)
		if (nvram_init(NULL))
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
		value = (char*)-1;

	if (count < 0)
		perror(PATH_DEV_NVRAM);

	if (off != (unsigned long *) tmp)
		free(off);

	return value;
}

char *
nvram_get(const char *name)
{
	char *value=nvram_get_(name);
	if (value==(char*)-1)
		return NULL;
	else if (value>0)
		return value;
	else
	{
		BUFFER[0] = 0;
		return BUFFER;
	}
}

int
nvram_getall(char *buf, int count)
{
	int ret;

	if (nvram_fd < 0)
		if ((ret = nvram_init(NULL)))
		{
			return ret;
		}
	if (count == 0)
		return 0;

	/* Get all variables */
	*buf = '\0';

	ret = read(nvram_fd, buf, count);

	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	return (ret == count) ? 0 : ret;
}

static int
_nvram_set(const char *name, const char *value)
{
	size_t count = strlen(name) + 1;
	char tmp[100], *buf = tmp;
	int ret;

//	if (name && value && (strstr(name, "hwaddr") || strstr(name, "macaddr")))
//	if (name && value && (strstr(name, "wan0_ipaddr")))
//		fprintf(stderr, "## set %s = %s\n", name, value);

	if (nvram_fd < 0)
		if ((ret = nvram_init(NULL)))
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

char *
nvram_xfr(char *buf)
{
	size_t count = strlen(buf)*2 + 1;       // ham 1120
	int ret;
	char tmpbuf[1024];

	if (nvram_fd < 0)
		if ((ret = nvram_init(NULL)))
			return NULL;

	if (count > sizeof(tmpbuf))
		return NULL;

	strcpy(tmpbuf, buf);

	if (!nvram_xfr_buf)
		nvram_xfr_buf=(char *)malloc(1024+1);
	if (!nvram_xfr_buf)
		return NULL;

	ret = ioctl(nvram_fd, NVRAM_MAGIC, tmpbuf);

	if (ret < 0)
	{
		perror(PATH_DEV_NVRAM);
		return NULL;
	}
	else
	{
		strcpy(nvram_xfr_buf, tmpbuf);
		return nvram_xfr_buf;
	}
}

int
nvram_set(const char *name, const char *value)
{
	return _nvram_set(name, value);
}

int
nvram_unset(const char *name)
{
	return _nvram_set(name, NULL);
}

int
nvram_commit(void)
{
	int ret;

	if (nvram_fd < 0)
		if ((ret = nvram_init(NULL)))
			return ret;

	ret = ioctl(nvram_fd, NVRAM_MAGIC, NULL);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	return ret;
}
