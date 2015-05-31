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
#include <errno.h>

#include <mtd-abi.h>
#include "flash_mtd.h"

#ifndef ROUNDUP
#define ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))
#endif

#ifndef ROUNDDOWN
#define ROUNDDOWN(x, y)		((x) - (x%y))
#endif

struct mtd_info {
	char dev[8];
	unsigned int size;
	unsigned int erasesize;
	unsigned int writesize;
	int type;
};

int
mtd_dev_idx(const char *mtd_part)
{
	FILE *fp;
	char line[128], bnm[64], *p;
	int idx, ret_idx = -1;

	if (!(fp = fopen("/proc/mtd", "r")))
		return -1;

	fgets(line, sizeof(line), fp); //skip the 1st line
	while (fgets(line, sizeof(line), fp)) {
		if (sscanf(line, "mtd%d: %*s %*s \"%63s\"", &idx, bnm) != 2)
			continue;
		
		/* strip tailed " character, if present. */
		if ((p = strchr(bnm, '"')) != NULL)
			*p = '\0';
		
		if (strcmp(mtd_part, bnm) == 0) {
			ret_idx = idx;
			break;
		}
	}

	fclose(fp);

	return ret_idx;
}

static int
mtd_dev_open(const char *mtd_part, int flags, struct mtd_info *p_mi)
{
	struct mtd_info_user miu;
	char mtd_dev[16];
	int idx, fd;

	idx = mtd_dev_idx(mtd_part);
	if (idx < 0)
		return -1;

	snprintf(mtd_dev, sizeof(mtd_dev), "/dev/mtd%d", idx);
	fd = open(mtd_dev, flags);
	if (fd < 0)
		return fd;

	if (ioctl(fd, MEMGETINFO, &miu) < 0) {
		close(fd);
		return -1;
	}

	sprintf(p_mi->dev, "mtd%d", idx);
	p_mi->size = miu.size;
	p_mi->erasesize = miu.erasesize;
	p_mi->writesize = miu.writesize;
	p_mi->type = miu.type;

	return fd;
}

static int
mtd_block_is_bad(int fd, int mtd_type, loff_t offset)
{
	if (mtd_type != MTD_NANDFLASH && mtd_type != MTD_MLCNANDFLASH)
		return 0;

	if (ioctl(fd, MEMGETBADBLOCK, &offset) > 0)
		return 1;

	return 0;
}

static ssize_t
read_safe(int fd, unsigned char *buf, size_t buf_len)
{
	size_t r_len = 0;

	while (r_len < buf_len) {
		ssize_t ret = read(fd, buf + r_len, buf_len - r_len);
		if (ret < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return -1;
		}
		
		if (ret == 0)
			break;
		
		r_len += ret;
	}

	return (ssize_t)r_len;
}

int
flash_mtd_read(const char *mtd_part, int offset, unsigned char *buf, size_t count)
{
	int fd, ret;
	ssize_t r_len;
	struct mtd_info mi;

	if (!mtd_part || !buf || offset < 0 || count < 1)
		return -1;

	memset(&mi, 0, sizeof(mi));
	fd = mtd_dev_open(mtd_part, O_RDONLY|O_SYNC, &mi);
	if (fd < 0) {
		fprintf(stderr, "%s: failed to open MTD partition %s\n",
			__func__, mtd_part);
		return -2;
	}

	ret = 0;

	if (((size_t)offset + count) > mi.size) {
		fprintf(stderr, "%s: out of MTD partition (offset: 0x%x, count: 0x%x, %s size: 0x%x)\n",
			__func__, offset, count, mi.dev, mi.size);
		ret = -3;
		goto out_err;
	}

	if (lseek(fd, offset, SEEK_SET) == -1) {
		fprintf(stderr, "%s: failed to seek %s to offset 0x%x (errno: %d)\n",
			__func__, mi.dev, offset, errno);
		ret = 3;
		goto out_err;
	}

	r_len = read_safe(fd, buf, count);
	if (r_len < 0) {
		fprintf(stderr, "%s: failed to read %u bytes from %s (errno: %d)\n",
			__func__, count, mi.dev, errno);
		ret = -4;
		goto out_err;
	}

	if (r_len != count)
		ret = 1;

out_err:
	fflush(stderr);

	close(fd);

	return ret;
}

int
flash_mtd_write(const char *mtd_part, int offset, unsigned char *buf, size_t count)
{
	int fd, ret, cnt, len, ofs, ofs_align, bad_shift;
	unsigned char *ptr, *tmp = NULL, *tmp_ubi = NULL;
	struct mtd_info mi;
	struct erase_info_user ei;

	if (!mtd_part || !buf || offset < 0 || count < 1)
		return -1;

	memset(&mi, 0, sizeof(mi));
	fd = mtd_dev_open(mtd_part, O_RDWR|O_SYNC, &mi);
	if (fd < 0) {
		fprintf(stderr, "%s: failed to open MTD partition %s\n",
			__func__, mtd_part);
		return -2;
	}

	ret = 0;

	if (((size_t)offset + count) > mi.size) {
		fprintf(stderr, "%s: out of MTD partition (offset: 0x%x, count: 0x%x, %s size: 0x%x)\n",
			__func__, offset, count, mi.dev, mi.size);
		ret = -3;
		goto out_err;
	}

	ptr = (unsigned char*) buf;

	/* align offset and length to write size of UBI volume backed MTD device. */
	if (mi.type == MTD_UBIVOLUME) {
		int old_offset = offset;
		size_t tmp_count, old_count = count;
		
		tmp_count = ROUNDUP((size_t)offset + count, mi.writesize);
		offset = ROUNDDOWN(offset, (int)mi.writesize);
		count = tmp_count - offset;
		
		if (offset != old_offset || count != old_count) {
			if (((size_t)offset + count) > mi.size) {
				fprintf(stderr, "%s: out of UBI partition. (aligned offset: 0x%x, count: 0x%x, %s size: 0x%x)\n",
					__func__, offset, count, mi.dev, mi.size);
				ret = -3;
				goto out_err;
			}
			
			tmp_ubi = malloc(count);
			if (!tmp_ubi) {
				fprintf(stderr, "%s: failed to alloc memory for %u bytes\n",
					__func__, count);
				ret = -1;
				goto out_err;
			}
			if (flash_mtd_read(mtd_part, offset, tmp_ubi, count) < 0) {
				ret = -4;
				goto out_err;
			}
			memcpy(tmp_ubi + (old_offset - offset), buf, old_count);
			ptr = tmp_ubi;
		}
	}

	tmp = (unsigned char *)malloc(mi.erasesize);
	if (!tmp) {
		fprintf(stderr, "%s: failed to alloc memory for %u bytes\n",
			__func__, mi.erasesize);
		ret = -1;
		goto out_err;
	}

	cnt = (int)count;
	ofs = offset;
	bad_shift = 0;

	while (cnt > 0) {
		ofs_align = ofs & ~(mi.erasesize - 1);	/* aligned to erase boundary */
		len = (int)mi.erasesize - (ofs - ofs_align);
		if (cnt < len)
			len = cnt;
		
		ei.start = ofs_align;
		ei.start += bad_shift;
		ei.length = mi.erasesize;
		
		/* check bad block */
		if (mtd_block_is_bad(fd, mi.type, ei.start)) {
			bad_shift += mi.erasesize;
			continue;
		}
		
		/* backup */
		if (lseek(fd, ei.start, SEEK_SET) < 0) {
			fprintf(stderr, "%s: failed to seek 0x%x on %s (errno: %d)\n",
				__func__, ei.start, mi.dev, errno);
			ret = -4;
			break;
		}
		if (read(fd, tmp, mi.erasesize) != mi.erasesize) {
			fprintf(stderr, "%s: failed to read %u bytes from %s (errno: %d)\n",
				__func__, mi.erasesize, mi.dev, errno);
			ret = -4;
			break;
		}
		
		/* erase */
		ioctl(fd, MEMUNLOCK, &ei);
		if (ioctl(fd, MEMERASE, &ei) < 0) {
			fprintf(stderr, "%s: failed to erase %s at start 0x%x, length 0x%x (errno: %d)\n",
				__func__, mi.dev, ei.start, ei.length, errno);
			ret = -5;
			break;
		}
		
		/* write */
		if (lseek(fd, ei.start, SEEK_SET) < 0) {
			fprintf(stderr, "%s: failed to seek 0x%x on %s (errno: %d)\n",
				__func__, ei.start, mi.dev, errno);
			ret = -6;
			break;
		}
		memcpy(tmp + (ofs - ofs_align), ptr, len);
		if (write(fd, tmp, mi.erasesize) != mi.erasesize) {
			fprintf(stderr, "%s: failed to write %u bytes to %s (errno: %d)\n",
				__func__, mi.erasesize, mi.dev, errno);
			ret = -6;
			break;
		}
		ptr += len;
		ofs += len;
		cnt -= len;
	}

	free(tmp);

	/* verify after write */
	if (cnt == 0 && mi.type != MTD_UBIVOLUME) {
		tmp = (unsigned char *)malloc(count);
		if (tmp) {
			if (flash_mtd_read(mtd_part, offset, tmp, count) == 0) {
				if (memcmp(tmp, buf, count) != 0) {
					fprintf(stderr, "%s: failed to verify after write to %s - data mismatch!\n",
						__func__, mi.dev);
					ret = -7;
				}
			}
			free(tmp);
		}
	}

out_err:

	if (tmp_ubi)
		free(tmp_ubi);

	close(fd);

	return ret;
}

