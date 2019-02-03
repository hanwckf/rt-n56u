/*
 * Copyright (c) EMC Corporation 2008
 * Copyright (c) Mark Lord 2008
 *
 * You may use/distribute this freely, under the terms of either
 * (your choice) the GNU General Public License version 2,
 * or a BSD style license.
 */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <sys/mman.h>

#include "hdparm.h"
#include "sgio.h"

/* glibc-2.2 / linux-2.4 don't have MAP_POPULATE */
#ifndef MAP_POPULATE
#define MAP_POPULATE	0
#endif

extern int verbose;

/* Download a firmware segment to the drive */
static int send_firmware (int fd, unsigned int xfer_mode, unsigned int offset,
			  const void *data, unsigned int bytecount)
{
	int err = 0;
	struct hdio_taskfile *r;
	unsigned int blockcount = bytecount / 512;
	unsigned int timeout_secs = 120;
	__u64 lba;

	lba = ((offset / 512) << 8) | ((blockcount >> 8) & 0xff);
	r = malloc(sizeof(struct hdio_taskfile) + bytecount);
	if (!r) {
		if (xfer_mode == 3 || xfer_mode == 0x0e) {
			putchar('\n');
			fflush(stdout);
		}
		err = errno;
		perror("malloc()");
		return err;
	}
	init_hdio_taskfile(r, ATA_OP_DOWNLOAD_MICROCODE, RW_WRITE, LBA28_OK, lba, blockcount & 0xff, bytecount);

	r->lob.feat = xfer_mode;
	r->oflags.bits.lob.feat  = 1;
	r->iflags.bits.lob.nsect = 1;

	if (data && bytecount)
		memcpy(r->data, data, bytecount);

	if (do_taskfile_cmd(fd, r, timeout_secs)) {
		err = errno;
		if (xfer_mode == 3 || xfer_mode == 0x0e) {
			putchar('\n');
			fflush(stdout);
		}
		perror("FAILED");
	} else {
		if (xfer_mode == 3 || xfer_mode == 0x0e) {
			if (!verbose) {
				putchar('.');
				fflush(stdout);
			}
			switch (r->lob.nsect) {
				case 1:	// drive wants more data
				case 2:	// drive thinks it is all done
					err = - r->lob.nsect;
					break;
				default: // no status indication
					err = 0;
					break;
			}
		}
	}
	free(r);
	return err;
}

int fwdownload (int fd, __u16 *id, const char *fwpath, int xfer_mode)
{
	int fwfd, err = 0;
	struct stat st;
	const char *fw = NULL;
	const int max_bytes = 0xffff * 512;
	int xfer_min = 1, xfer_max = 0xffff, xfer_size;
	ssize_t offset;

	if ((fwfd = open(fwpath, O_RDONLY)) == -1 || fstat(fwfd, &st) == -1) {
		err = errno;
		perror(fwpath);
		return err;
	}

	if (!S_ISREG(st.st_mode)) {
		fprintf(stderr, "%s: not a regular file\n", fwpath);
		err = EINVAL;
		goto done;
	}

	if (st.st_size > max_bytes) {
	    	fprintf(stderr, "%s: file size too large, max=%u bytes\n", fwpath, max_bytes);
		err = EINVAL;
		goto done;
	}

	if (st.st_size == 0 || st.st_size % 512) {
	    	fprintf(stderr, "%s: file size (%llu) not a multiple of 512\n",
			fwpath, (unsigned long long) st.st_size);
		err = EINVAL;
		goto done;
	}

	if (verbose)
		printf("%s: %llu bytes\n", fwpath, (unsigned long long)st.st_size);

	fw = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED|MAP_POPULATE|MAP_LOCKED, fwfd, 0);
	if (fw == MAP_FAILED) {
		err = errno;
		perror(fwpath);
		goto done;
	}

	/* Check drive for fwdownload support */
	if (!((id[83] & 1) && (id[86] & 1))) {
		fprintf(stderr, "DOWNLOAD_MICROCODE: not supported by device\n");
		err = ENOTSUP;
		goto done;
	}

	if (xfer_mode == 0) {
		if ((id[119] & 0x10) && (id[120] & 0x10))
			xfer_mode = 3;
		else
			xfer_mode = 7;
	}

	if (xfer_mode == 3 || xfer_mode == 0x30  || xfer_mode == 0x0e) {
		/* the newer, segmented transfer mode */
		xfer_min = id[234];
		if (xfer_min == 0 || xfer_min == 0xffff)
			xfer_min = 1;
		xfer_max = id[235];
		if (xfer_max == 0 || xfer_max == 0xffff)
			xfer_max = xfer_min;
	}

	if (xfer_mode == 0x30) {	// mode-3, using xfer_max
		xfer_mode = 3;
		xfer_size = xfer_max;
	} else if (xfer_mode == 3) {
		xfer_size = xfer_min;
	} else if (xfer_mode == 0x0e) {
#if 0
		xfer_size = xfer_max;
	} else if (xfer_mode == 0xe0) {
		xfer_size = xfer_min;
#else
		xfer_size = xfer_min;
	} else if (xfer_mode == 0xe0) {
		xfer_mode = 0x0e;
		xfer_size = xfer_max;
#endif
	} else {
		xfer_size = st.st_size / 512;
		if (xfer_size > 0xffff) {
			fprintf(stderr, "Error: file size (%llu) too large for mode7 transfers\n", (__u64)st.st_size);
			err = EINVAL;
			goto done;
		}
	}

	xfer_size *= 512;	/* bytecount */

	fprintf(stderr, "%s: xfer_mode=%d min=%u max=%u size=%u\n",
		__func__, xfer_mode, xfer_min, xfer_max, xfer_size);

	/* Perform the fwdownload, in segments if appropriate */
	for (offset = 0; !err && offset < st.st_size;) {
		if ((offset + xfer_size) >= st.st_size)
			xfer_size = st.st_size - offset;
		err = send_firmware(fd, xfer_mode, offset, fw + offset, xfer_size);
		offset += xfer_size;

		if (err == -2) {	// drive has had enough?
			if (offset >= st.st_size) { // transfer complete?
				err = 0;
			} else {
				if (xfer_mode == 3 || xfer_mode == 0x0e) {
					putchar('\n');
					fflush(stdout);
				}
				fprintf(stderr, "Error: drive completed transfer at %llu/%llu bytes\n",
						(unsigned long long)offset, (unsigned long long)st.st_size);
				err = EIO;
			}
		} else if (err == -1) {
			if (offset >= st.st_size) { // no more data?
#if 0
				/* Try sending an empty segment before complaining */
				err = send_firmware(fd, xfer_mode, offset, NULL, 0);
				if (err == 0 || err == -2) {
					err = 0;
					break;
				}
#endif
				if (xfer_mode == 3 || xfer_mode == 0x0e) {
					putchar('\n');
					fflush(stdout);
				}
				fprintf(stderr, "Error: drive expects more data than provided,\n");
				fprintf(stderr, "but the transfer may have worked regardless.\n");
				err = EIO;
			} else {
				err = 0;
			}
		}
	}
	if (!err)
		printf(" Done.\n");
done:
	munlock(fw, st.st_size);
	close (fwfd);
	return err;
}
