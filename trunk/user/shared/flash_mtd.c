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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "mtd-abi.h"
#include "flash_mtd.h"

#define NUM_INFO 8

struct mtd_info {
	char dev[8];
	int size;
	int erasesize;
	char name[16];
};

static struct mtd_info info[NUM_INFO];

static int
mtd_init_info(void)
{
	FILE *fp;
	char line[128], bnm[64];
	int i, bsz, esz;

	memset(info, 0, sizeof(info));

	if ((fp = fopen("/proc/mtd", "r"))) {
		fgets(line, sizeof(line), fp); //skip the 1st line
		while (fgets(line, sizeof(line), fp)) {
			if (sscanf(line, "mtd%d: %x %x \"%s\"", &i, &bsz, &esz, bnm) > 3) {
				/* strip tailed " character, if present. */
				char *p = strchr(bnm, '"');
				if (p)
					*p = '\0';
				
				if (i >= NUM_INFO)
					printf("please enlarge 'NUM_INFO'\n");
				else {
					sprintf(info[i].dev, "mtd%d", i);
					info[i].size = bsz;
					info[i].erasesize = esz;
					snprintf(info[i].name, sizeof(info[i].name), "%s", bnm);
				}
			}
		}
		fclose(fp);
	} else {
		fprintf(stderr, "failed to open %s\n", "/proc/mtd");
		return -1;
	}

	return 0;
}

static int
mtd_dev_open(int num, int flags)
{
	char dev[16];
	snprintf(dev, sizeof(dev), "/dev/mtd%d", num);
	return open(dev, flags);
}

int FRead(unsigned char *dst, int src, int count)
{
	int i, o, off, cnt, addr, fd, len;
	unsigned char *buf, *p;

	if (mtd_init_info())
		return -1;

	buf = (unsigned char *)malloc(count);
	if (buf == NULL) {
		fprintf(stderr, "fail to alloc memory for %d bytes\n", count);
		return -1;
	}
	p = buf;
	cnt = count;
	off = src;

	for (i = 0, addr = 0; i < NUM_INFO; i++) {
		if (addr <= off && off < addr + info[i].size) {
			o = off - addr;
			fd = mtd_dev_open(i, O_RDONLY | O_SYNC);
			if (fd < 0) {
				fprintf(stderr, "failed to open mtd%d\n", i);
				free(buf);
				return -1;
			}
			lseek(fd, o, SEEK_SET);
			len = ((o + cnt) < info[i].size)? cnt : (info[i].size - o);
			read(fd, p, len);
			close(fd);
			cnt -= len;
			if (cnt == 0)
				break;
			off += len;
			p += len;
		}
		addr += info[i].size;
	}

	memcpy(dst, buf, count);
	free(buf);

	return 0;
}

int FWrite(unsigned char *src, int offset, int count)
{
	int i, o, fd, off, addr, sz;
	unsigned char *buf;
	struct erase_info_user ei;

	if (mtd_init_info())
		return -1;

	off = offset;

	for (i = 0, addr = 0; i < NUM_INFO; i++) {
		if (addr <= off && off < addr + info[i].size) {
			sz = info[i].erasesize;
			buf = (unsigned char *)malloc(sz);
			if (buf == NULL) {
				fprintf(stderr, "failed to alloc memory for %d bytes\n",
						sz);
				return -1;
			}
			fd = mtd_dev_open(i, O_RDWR | O_SYNC);
			if (fd < 0) {
				fprintf(stderr, "failed to open mtd%d\n", i);
				free(buf);
				return -1;
			}
			off -= addr;
			o = (off / sz) * sz;
			lseek(fd, o, SEEK_SET);
			//backup
			if (read(fd, buf, sz) != sz) {
				fprintf(stderr, "failed to read %d bytes from mtd%d\n",
						sz, i);
				free(buf);
				close(fd);
				return -1;
			}
			//erase
			ei.start = o;
			ei.length = sz;
			if (ioctl(fd, MEMERASE, &ei) < 0) {
				fprintf(stderr, "failed to erase mtd%d\n", i);
				free(buf);
				close(fd);
				return -1;
			}
			//write
			lseek(fd, o, SEEK_SET);
			memcpy(buf + (off - o), src, count);
			if (write(fd, buf, sz) == -1) {
				fprintf(stderr, "failed to write mtd%d\n", i);
				free(buf);
				close(fd);
				return -1;
			}
			free(buf);
			close(fd);
			break;
		}
		addr += info[i].size;
	}
	buf = (unsigned char *)malloc(count);
	FRead(buf, offset, count);
	free(buf);

	return 0;
}

