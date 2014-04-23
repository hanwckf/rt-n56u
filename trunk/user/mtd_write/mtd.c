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

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <linux/reboot.h>

#include "mtd.h"

#define BUFSIZE (1 * 1024)
#define MAX_ARGS 3

char *buf;
int buflen;
int quiet;
int verbose;
int write_check=1;

int
mtd_open(const char *mtd, int flags)
{
	FILE *fp;
	char line[128], bnm[64];
	int i, ret;

	if ((fp = fopen("/proc/mtd", "r"))) {
		fgets(line, sizeof(line), fp); //skip the 1st line
		while (fgets(line, sizeof(line), fp)) {
			if (sscanf(line, "mtd%d: %*s %*s \"%s\"", &i, bnm) > 1) {
				/* strip tailed " character, if present. */
				char *p = strchr(bnm, '"');
				if (p)
					*p = '\0';
				if (!strcmp(bnm, mtd)) {
					snprintf(bnm, sizeof(bnm), "/dev/mtd%d", i);
					ret = open(bnm, flags);
					fclose(fp);
					return ret;
				}
			}
		}
		fclose(fp);
	}

	return open(mtd, flags);
}

int mtd_check(char *mtd)
{
	struct mtd_info_user mtdInfo;
	int fd;

	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		return 0;
	}

	if(ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		close(fd);
		return 0;
	}

	close(fd);
	return 1;
}

int
mtd_unlock(const char *mtd)
{
	int fd;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdLockInfo;

	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if(ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		close(fd);
		exit(1);
	}

	mtdLockInfo.start = 0;
	mtdLockInfo.length = mtdInfo.size;
	if(ioctl(fd, MEMUNLOCK, &mtdLockInfo)) {
		close(fd);
		return 0;
	}
		
	close(fd);
	return 0;
}


int
mtd_erase(const char *mtd)
{
	int fd, i;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdEraseInfo;
	unsigned int test_char = 256;
	unsigned char *test_buf;

	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if(ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		close(fd);
		exit(1);
	}

	mtdEraseInfo.length = mtdInfo.erasesize;
	test_buf = malloc(sizeof(unsigned char) * mtdInfo.erasesize);
	if(!test_buf)
		exit(1);

	for (mtdEraseInfo.start = 0;
		 mtdEraseInfo.start < mtdInfo.size;
		 mtdEraseInfo.start += mtdInfo.erasesize) {

		ioctl(fd, MEMUNLOCK, &mtdEraseInfo);
		if(ioctl(fd, MEMERASE, &mtdEraseInfo)){
			fprintf(stderr, "Failed to erase block on %s at 0x%x\n", mtd, mtdEraseInfo.start);
			close(fd);
			exit(1);
		}

		/*
		 * Post-Erase Check
		 */
		sync();
		if(read(fd, test_buf, mtdEraseInfo.length) != mtdEraseInfo.length){
				fprintf(stderr, "Failed to erase block, read() failed\n");
				close(fd);
				exit(1);
		}
		if(test_char == 256){
			test_char = test_buf[0];
			printf("Erase char is %d\n", test_char);
		}
		for(i=0; i<mtdEraseInfo.length; i++){
			if(test_buf[i] != test_char){
				fprintf(stderr, "Failed to erase block, dismatch\n");
				close(fd);
				exit(1);
			}
		}

	}		

	free(test_buf);
	close(fd);
	return 0;

}

int
mtd_write(int imagefd, int offset, int len, const char *mtd)
{
	int fd, result;
	size_t r, w, e;
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdEraseInfo;
	int statistic = 0;
	unsigned char *test_buf;

	fd = mtd_open(mtd, O_RDWR | O_SYNC);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if(ioctl(fd, MEMGETINFO, &mtdInfo)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd);
		close(fd);
		exit(1);
	}
		
	r = w = e = 0;
	if (!quiet)
		fprintf(stderr, " [ ]");

	//YYHuang
	lseek(imagefd, offset, SEEK_SET);

	buf = (char *)malloc(BUFSIZE);
	if (NULL == buf) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	for (; len;) {
		/* buffer may contain data already (from trx check) */
		r = 0;
		if( (BUFSIZE) > len ){
			r = read(imagefd, buf , len);		
		}else
			r = read(imagefd, buf , BUFSIZE);

		w += r;

		/* EOF */
		if (r <= 0) break;

		len = len - r;

		/* need to erase the next block before writing data to it */
		while (w > e) {
			mtdEraseInfo.start = e;
			mtdEraseInfo.length = mtdInfo.erasesize;

			if (!quiet)
				fprintf(stderr, "\b\b\b[e]");
			/* erase the chunk */
			if (ioctl (fd,MEMERASE,&mtdEraseInfo) < 0) {
				fprintf(stderr, "Erasing mtd failed: %s\n", mtd);
				free(buf);
				exit(1);
			}
			e += mtdInfo.erasesize;
		}

		if (!quiet)
			fprintf(stderr, "\b\b\b[w]");

		if ((result = write(fd, buf, r)) < r) {
			if (result < 0) {
				fprintf(stderr, "Error writing image.\n");
				free(buf);
				exit(1);
			} else {
				fprintf(stderr, "Insufficient space.\n");
				free(buf);
				exit(1);
			}
		}

#if 1
		/*
		 * Post-FlashWrite check
		 */
		if(write_check){
 			/* trigger filesystem to sync with everything! */ 
 			sync();

 			if( lseek(fd, -r, SEEK_CUR) == -1){
 				fprintf(stderr, "lseek failed.\n");
				free(buf);
 				exit(1);
 			}

 			if(!(test_buf = malloc(sizeof(char) * r))){
 				fprintf(stderr, "Out of memory.\n");
				free(buf);
 				exit(1);
 			}
 			if( read(fd, test_buf, r) != r){
 				fprintf(stderr, "Error reading mtd.\n");
				free(buf);
 				exit(1);
 			}
 			if(memcmp(buf, test_buf, r)){
 				fprintf(stderr, "Post-Write check failed.\n");
				free(buf);
 				exit(1);
 			}else{
 				/*
 				 * Matched.
 				 */
 			//	printf("mtd_write:verify Write action success\n");
 			}
 			free(test_buf);
 		}
#endif

		if(verbose == 1){
			fprintf(stdout, "%d\n", statistic);
		}else if(verbose == 2){	// output HTML format
			fprintf(stdout, "%d... ", statistic);
		}
		fflush(stdout);
		statistic += result;
	}
	if (!quiet)
		fprintf(stderr, "\b\b\b\b");

	free(buf);
	close(fd);
	return 0;
}

void usage(void)
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
	"        -v                      output writing info. (1 more -v would output HTML format.)\n"
	"        -o <num>                file offset \n"
	"        -l <num>                length in file\n"
	"        -w                      read after write action to check\n"
	"Example: To write linux.trx to mtd4 labeled as linux and reboot afterwards\n"
	"         mtd -r write linux.trx linux\n\n");
	exit(1);
}

int getFileSize(char *filename)
{
	int fd;
	struct stat StatBuf;
	if( (fd = open(filename, O_RDONLY, 0666)) == -1){
		perror("getFileSize");
	}
	if (fstat(fd, &StatBuf) == -1){
		close(fd);
		return -1;	  
	}
	close(fd);
	return StatBuf.st_size;
}

int main (int argc, char **argv)
{
	int ch, i, boot, imagefd, unlocked, offset, len;
	char *erase[MAX_ARGS], *device, *imagefile;
	enum {
		CMD_ERASE,
		CMD_WRITE,
		CMD_UNLOCK
	} cmd;
	
	erase[0] = NULL;
	boot = 0;
	buflen = 0;
	quiet = 0;
	verbose = 0;
	offset = 0;
	len = 0;
	cmd = CMD_UNLOCK;
	imagefd = 0;
	imagefile = "<stdin>";
	device = "";

	while ((ch = getopt(argc, argv, "vwrqe:o:l:")) != -1)
		switch (ch) {
			case 'r':
				boot = 1;
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
			case 'v':
				verbose++;
				break;
			case 'o':
				offset = atoi(optarg);
				break;
			case 'l':
				len = atoi(optarg);
				break;
			case 'w':
				write_check++;
				break;
			case '?':
			default:
				usage();
		}
	argc -= optind;
	argv += optind;
	
	if (argc < 2)
		usage();

	if(len == 0){
		len = getFileSize(argv[1]);
	}

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
			imagefile = argv[1];
			if ((imagefd = open(argv[1], O_RDONLY)) < 0) {
				fprintf(stderr, "Couldn't open image file: %s!\n", imagefile);
				exit(1);
			}
		}
	
		if (!mtd_check(device)) {
			fprintf(stderr, "Can't open device for writing!\n");
			exit(1);
		}
	} else {
		usage();
	}

	sync();
	
	i = 0;
	unlocked = 0;
	while (erase[i] != NULL) {
		if (quiet < 2)
			fprintf(stderr, "Unlocking %s ...\n", erase[i]);
		mtd_unlock(erase[i]);
		if (quiet < 2)
			fprintf(stderr, "Erasing %s ...\n", erase[i]);
		mtd_erase(erase[i]);
		if (strcmp(erase[i], device) == 0)
			unlocked = 1;
		i++;
	}
	
	if (!unlocked) {
		if (quiet < 2) 
			fprintf(stderr, "Unlocking %s ...\n", device);
		mtd_unlock(device);
	}
	
	switch (cmd) {
		case CMD_UNLOCK:
			break;
		case CMD_ERASE:
			if (quiet < 2)
				fprintf(stderr, "Erasing %s ...\n", device);
			mtd_erase(device);
			break;
		case CMD_WRITE:
			if (quiet < 2)
				fprintf(stderr, "Writing from %s to %s ... ", imagefile, device);
			mtd_write(imagefd, offset, len,  device);

			if (quiet < 2)
				fprintf(stderr, "\n");
			break;
	}

	sync();
	
	if (boot) {
		fflush(stdout);
		syscall(SYS_reboot,LINUX_REBOOT_MAGIC1,LINUX_REBOOT_MAGIC2,LINUX_REBOOT_CMD_RESTART,NULL);
	}
	return 0;
}

