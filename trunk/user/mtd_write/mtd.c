/*
 * mtd - simple memory technology device manipulation tool
 *
 * Copyright (C) 2005 Waldemar Brodkorb <wbx@dass-it.de>,
 *	                  Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: mtd.c,v 1.1 2009-03-17 09:48:42 steven Exp $
 *
 * The code is based on the linux-mtd examples.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <linux/reboot.h>

#include <mtd-abi.h>

#define BUFSIZE		(1024)
#define MAX_ARGS	3

#ifndef ROUNDUP
#define ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

static int quiet = 0;
static int no_erase = 0;
static int write_check = 1;

static int
mtd_open(const char *mtd, int flags, struct mtd_info_user *p_mi)
{
	FILE *fp;
	char line[128], bnm[64], *p;
	int i, idx, fd;

	idx = -1;

	if ((fp = fopen("/proc/mtd", "r"))) {
		fgets(line, sizeof(line), fp); //skip the 1st line
		while (fgets(line, sizeof(line), fp)) {
			if (sscanf(line, "mtd%d: %*s %*s \"%s\"", &i, bnm) > 1) {
				/* strip tailed " character, if present. */
				if ((p = strchr(bnm, '"')) != NULL)
					*p = '\0';
				if (!strcmp(bnm, mtd)) {
					idx = i;
					break;
				}
			}
		}
		fclose(fp);
	}

	if (idx < 0)
		snprintf(bnm, sizeof(bnm), "%s", mtd);
	else
		snprintf(bnm, sizeof(bnm), "/dev/mtd%d", idx);

	fd = open(bnm, flags);
	if (fd < 0)
		return -1;

	if (ioctl(fd, MEMGETINFO, p_mi) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

static int
mtd_unlock(const char *mtd)
{
	int fd;
	struct mtd_info_user mi;
	struct erase_info_user ei;

	fd = mtd_open(mtd, O_RDWR | O_SYNC, &mi);
	if (fd < 0)
		return -1;

	if (quiet < 2)
		fprintf(stderr, "Unlocking '%s' ...\n", mtd);

	ei.start = 0;
	ei.length = mi.size;
	ioctl(fd, MEMUNLOCK, &ei);
	close(fd);

	return 0;
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

static int
mtd_erase(const char *mtd)
{
	int fd, ret;
	struct mtd_info_user mi;
	struct erase_info_user ei;

	fd = mtd_open(mtd, O_RDWR | O_SYNC, &mi);
	if (fd < 0) {
		fprintf(stderr, "Could not open MTD device: %s\n", mtd);
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "Erasing '%s' ...\n", mtd);

	ret = 0;

	ei.length = mi.erasesize;
	for (ei.start = 0; ei.start < mi.size; ei.start += mi.erasesize) {
		if (mtd_block_is_bad(fd, mi.type, ei.start)) {
			if (!quiet)
				fprintf(stderr, "Skipping bad block at 0x%x\n", ei.start);
			continue;
		}
		if (ioctl(fd, MEMERASE, &ei) < 0) {
			fprintf(stderr, "Erasing MTD (%s) failed at 0x%x\n", mtd, ei.start);
			ret = 1;
			break;
		}
	}

	close(fd);

	if (ret)
		exit(ret);

	return 0;
}

static ssize_t
read_safe(int image_fd, unsigned char *buf, size_t buf_len)
{
	size_t r_len = 0;

	while (r_len < buf_len) {
		ssize_t ret = read(image_fd, buf + r_len, buf_len - r_len);
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

static int
mtd_write(const char *mtd, size_t part_ofs, const char *image_file, int image_fd, off_t image_len)
{
	int fd, ret;
	ssize_t r_len, w_len, w_align;
	size_t buf_len, src_len, dst_len, ers_ptr, bad_shift;
	struct mtd_info_user mi;
	struct erase_info_user ei;
	unsigned char *buf = NULL, *buf_vf = NULL;

	ret = 0;

	fd = mtd_open(mtd, O_RDWR | O_SYNC, &mi);
	if (fd < 0) {
		fprintf(stderr, "Could not open MTD device: %s\n", mtd);
		ret = 1;
		goto out_err;
	}

	if (part_ofs > 0) {
		if ((part_ofs % mi.erasesize) != 0) {
			fprintf(stderr, "Partition offset (0x%x) must be multiple of erasesize (0x%x)!\n", part_ofs, mi.erasesize);
			ret = 1;
			goto out_err;
		}
	}

	if (image_fd > 0) {
		off_t image_len_align = ROUNDUP(image_len, (off_t)mi.erasesize);
		if (image_len_align > (off_t)(mi.size - part_ofs)) {
			fprintf(stderr, "Image file '%s' is too big to fit in MTD '%s'\n", image_file, mtd);
			ret = 1;
			goto out_err;
		}
	}

	if (part_ofs > 0) {
		if (lseek(fd, part_ofs, SEEK_SET) < 0) {
			fprintf(stderr, "Unable seek to partition offset 0x%x!\n", part_ofs);
			ret = 1;
			goto out_err;
		}
	}

	ers_ptr = 0;
	dst_len = 0;
	src_len = (size_t)image_len;
	buf_len = MAX(BUFSIZE, mi.writesize);
	bad_shift = 0;

	buf = (unsigned char *)malloc(buf_len);
	if (!buf) {
		fprintf(stderr, "malloc of %d bytes failed\n", buf_len);
		ret = 1;
		goto out_err;
	}

	buf_vf = (unsigned char *)malloc(buf_len);
	if (!buf_vf) {
		fprintf(stderr, "malloc of %d bytes failed\n", buf_len);
		ret = 1;
		goto out_err;
	}

	if (quiet < 2)
		fprintf(stderr, "Writing from '%s' to MTD '%s' ... ", image_file, mtd);

	if (!quiet)
		fprintf(stderr, " [ ]");

	while (src_len > 0) {
		r_len = read_safe(image_fd, buf, MIN(src_len, buf_len));
		
		/* check EOF */
		if (r_len <= 0)
			break;
		
		src_len -= r_len;
		dst_len += r_len;
		
		/* need to erase the next block before writing data to it */
		while (dst_len > ers_ptr) {
			ei.start = ers_ptr;
			ei.start += part_ofs;
			ei.start += bad_shift;
			ei.length = mi.erasesize;
			
			/* check bad block before erase */
			if (mtd_block_is_bad(fd, mi.type, ei.start)) {
				if (!quiet)
					fprintf(stderr, "\nSkipping bad block at 0x%08x   ", ei.start);
				if (lseek(fd, mi.erasesize, SEEK_CUR) < 0) {
					fprintf(stderr, "\n");
					fprintf(stderr, "Skipping bad block on MTD (%s) failed\n", mtd);
					ret = 1;
					goto out_err;
				}
				bad_shift += mi.erasesize;
			} else {
				if (!no_erase) {
					if (!quiet)
						fprintf(stderr, "\b\b\b[e]");
					if (ioctl(fd, MEMERASE, &ei) < 0) {
						fprintf(stderr, "\n");
						fprintf(stderr, "Erasing MTD (%s) failed at 0x%x\n", mtd, ei.start);
						ret = 1;
						goto out_err;
					}
				}
				ers_ptr += mi.erasesize;
			}
		}
		
		if (!quiet)
			fprintf(stderr, "\b\b\b[w]");
		
		/* align write to MTD writesize (UBIVOL/NAND) */
		w_align = r_len;
		if (r_len < (ssize_t)buf_len && r_len < (ssize_t)mi.writesize) {
			memset(buf + r_len, 0xff, (buf_len - r_len));
			w_align = mi.writesize;
		}
		
		w_len = write(fd, buf, w_align);
		if (w_len < w_align) {
			fprintf(stderr, "\nWrite failed ");
			if (w_len < 0)
				fprintf(stderr, "(errno: %d)!\n", errno);
			else
				fprintf(stderr, "- insufficient space on MTD!\n");
			ret = 1;
			goto out_err;
		}
		
		/*
		 * Post-FlashWrite check (for file only & not UBI)
		 */
		if (image_fd > 0 && write_check && mi.type != MTD_UBIVOLUME) {
			/* trigger filesystem to sync with everything! */
			sync();
			
			if (!quiet)
				fprintf(stderr, "\b\b\b[v]");
			
			if (lseek(fd, -(w_align), SEEK_CUR) < 0) {
				fprintf(stderr, "\nPost-write verify failed - %s!\n", "seek error");
				ret = 1;
				goto out_err;
			}
			if (read(fd, buf_vf, w_align) != w_align) {
				fprintf(stderr, "\nPost-write verify failed - %s!\n", "read error");
				ret = 1;
				goto out_err;
			}
			if (memcmp(buf, buf_vf, r_len)) {
				fprintf(stderr, "\nPost-write verify failed - %s!\n", "data mismatch");
				ret = 1;
				goto out_err;
			}
		}
	}

	if (!quiet)
		fprintf(stderr, "\b\b\b[ok]\n");

out_err:

	fflush(stderr);

	if (buf_vf)
		free(buf_vf);
	if (buf)
		free(buf);
	if (fd > 0)
		close(fd);
	if (image_fd > 0)
		close(image_fd);

	if (ret)
		exit(ret);

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "Usage: mtd [<options> ...] <command> [<arguments> ...] <device>\n\n"
	"The device is in the format of mtdX (eg: mtd4) or its label.\n"
	"mtd recognizes these commands:\n"
	"        unlock                  unlock the device\n"
	"        erase                   erase all data on device\n"
	"        write <imagefile>|-     write <imagefile> (use - for stdin) to device\n"
	"Following options are available:\n"
	"        -q                      quiet mode (once: no [w] on writing,\n"
	"                                           twice: no status messages)\n"
	"        -r                      reboot after successful command\n"
	"        -e <device>             erase <device> before executing the command\n"
	"        -o <offset>             file offset\n"
	"        -l <length>             length in file\n"
	"        -p <offset>             write beginning at partition offset\n"
	"        -n                      write without first erasing the blocks\n"
	"        -w                      do not verify after write action\n"
	"Example: To write linux.trx to mtd4 labeled as linux and reboot afterwards\n"
	"         mtd -r write linux.trx linux\n\n");
	exit(1);
}

int main (int argc, char **argv)
{
	int ch, i, do_reboot, unlocked, image_fd;
	off_t image_ofs, image_len;
	size_t part_ofs;
	char *erase[MAX_ARGS], *device, *image_file;
	enum {
		CMD_ERASE,
		CMD_WRITE,
		CMD_UNLOCK
	} cmd;

	cmd = CMD_UNLOCK;
	do_reboot = 0;
	quiet = 0;
	erase[0] = NULL;

	device = "";
	image_ofs = 0;
	image_len = 0;
	image_fd = 0;
	image_file = "<stdin>";

	part_ofs = 0;

	while ((ch = getopt(argc, argv, "wrnqe:o:l:p:")) != -1) {
		switch (ch) {
			case 'w':
				write_check = 0;
				break;
			case 'r':
				do_reboot = 1;
				break;
			case 'n':
				no_erase = 1;
				break;
			case 'q':
				quiet++;
				break;
			case 'e':
				i = 0;
				while ((erase[i] != NULL) && ((i + 1) < MAX_ARGS))
					i++;
					
				erase[i++] = optarg;
				erase[i] = NULL;
				break;
			case 'o':
				image_ofs = strtoll(optarg, NULL, 0);
				break;
			case 'l':
				image_len = strtoll(optarg, NULL, 0);
				break;
			case 'p':
				part_ofs = strtoul(optarg, 0, 0);
				break;
			case '?':
			default:
				usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	if ((strcmp(argv[0], "unlock") == 0) && (argc == 2)) {
		cmd = CMD_UNLOCK;
		device = argv[1];
	} else if ((strcmp(argv[0], "erase") == 0) && (argc == 2)) {
		cmd = CMD_ERASE;
		device = argv[1];
	} else if ((strcmp(argv[0], "write") == 0) && (argc == 3)) {
		cmd = CMD_WRITE;
		device = argv[2];
		
		if (strcmp(argv[1], "-") != 0) {
			image_file = argv[1];
			if ((image_fd = open(image_file, O_RDONLY)) < 0) {
				fprintf(stderr, "Couldn't open image file: '%s'!\n", image_file);
				exit(1);
			}
			
			if (image_len == 0) {
				struct stat st;
				if (fstat(image_fd, &st) < 0) {
					fprintf(stderr, "Couldn't get stat of image file '%s' (errno: %d)!\n", image_file, errno);
					close(image_fd);
					exit(1);
				}
				if (st.st_size < 1) {
					fprintf(stderr, "Image file '%s' is empty!\n", image_file);
					close(image_fd);
					exit(1);
				}
				
				image_len = st.st_size;
				if (image_ofs > 0 && image_len > image_ofs) {
					if (lseek(image_fd, image_ofs, SEEK_SET) < 0) {
						fprintf(stderr, "Unable seek to offset 0x%llx in file '%s' (errno: %d)!\n", image_ofs, image_file, errno);
						close(image_fd);
						exit(1);
					}
					image_len -= image_ofs;
				}
			}
		}
	} else {
		usage();
	}

	sync();

	i = 0;
	unlocked = 0;
	while (erase[i] != NULL) {
		mtd_unlock(erase[i]);
		mtd_erase(erase[i]);
		if (strcmp(erase[i], device) == 0)
			unlocked = 1;
		i++;
	}

	if (!unlocked)
		mtd_unlock(device);

	switch (cmd) {
		case CMD_UNLOCK:
			break;
		case CMD_ERASE:
			mtd_erase(device);
			break;
		case CMD_WRITE:
			mtd_write(device, part_ofs, image_file, image_fd, image_len);
			break;
	}

	sync();

	if (do_reboot) {
		fprintf(stderr, "Rebooting ...\n");
		fflush(stderr);
		syscall(SYS_reboot,LINUX_REBOOT_MAGIC1,LINUX_REBOOT_MAGIC2,LINUX_REBOOT_CMD_RESTART,NULL);
	}

	return 0;
}

