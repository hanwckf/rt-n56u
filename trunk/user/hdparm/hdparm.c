/*
 * hdparm.c - Command line interface to get/set hard disk parameters.
 *          - by Mark Lord (C) 1994-2018 -- freely distributable.
 */
#define HDPARM_VERSION "v9.58"

#define _LARGEFILE64_SOURCE /*for lseek64*/
#define _BSD_SOURCE	/* for strtoll() */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#define __USE_GNU	/* for O_DIRECT */
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <endian.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <endian.h>
#include <asm/byteorder.h>

#include "hdparm.h"
#include "sgio.h"

static int    argc;
static char **argv;
static char  *argp;
static int    num_flags_processed = 0;

extern const char *minor_str[];

#ifndef O_DIRECT
#define O_DIRECT	040000	/* direct disk access, not easily obtained from headers */
#endif

#ifndef CDROM_SELECT_SPEED	/* already defined in 2.3.xx kernels and above */
#define CDROM_SELECT_SPEED	0x5322
#endif

#define TIMING_BUF_MB		2
#define TIMING_BUF_BYTES	(TIMING_BUF_MB * 1024 * 1024)

char *progname;
int verbose = 0;
int prefer_ata12 = 0;
static int do_defaults = 0, do_flush = 0, do_ctimings, do_timings = 0;
static int do_identity = 0, get_geom = 0, noisy = 1, quiet = 0;
static int do_flush_wcache = 0;

static int set_wdidle3  = 0, get_wdidle3 = 0, wdidle3 = 0;
static int   set_timings_offset = 0;
static __u64 timings_offset = 0;
static int set_fsreadahead= 0, get_fsreadahead= 0, fsreadahead= 0;
static int set_readonly = 0, get_readonly = 0, readonly = 0;
static int set_unmask   = 0, get_unmask   = 0, unmask   = 0;
static int set_mult     = 0, get_mult     = 0, mult     = 0;
static int set_dma      = 0, get_dma      = 0, dma      = 0;
static int set_dma_q	  = 0, get_dma_q    = 0, dma_q	  = 0;
static int set_nowerr   = 0, get_nowerr   = 0, nowerr   = 0;
static int set_keep     = 0, get_keep     = 0, keep     = 0;
static int set_io32bit  = 0, get_io32bit  = 0, io32bit  = 0;
static int set_piomode  = 0, get_piomode= 0, piomode = 0;
static int set_dkeep    = 0, get_dkeep    = 0, dkeep    = 0;
static int set_standby  = 0, get_standby  = 0, standby= 0;
#if !defined (HDPARM_MINI)
static int set_xfermode = 0, get_xfermode = 0;
static int xfermode_requested= 0;
#endif
static int set_lookahead= 0, get_lookahead= 0, lookahead= 0;
static int set_prefetch = 0, get_prefetch = 0, prefetch = 0;
static int set_defects  = 0, get_defects  = 0, defects  = 0;
static int set_wcache   = 0, get_wcache   = 0, wcache   = 0;
static int set_doorlock = 0, get_doorlock = 0, doorlock = 0;
static int set_seagate  = 0, get_seagate  = 0;
static int get_idleimmediate = 0, set_idleimmediate = 0;
static int get_idleunload = 0, set_idleunload = 0;
static int set_standbynow = 0, get_standbynow = 0;
static int set_sleepnow   = 0, get_sleepnow   = 0;
static int set_powerup_in_standby = 0, get_powerup_in_standby = 0, powerup_in_standby = 0;
static int get_hitachi_temp = 0, set_hitachi_temp = 0;
#if !defined (HDPARM_MINI)
static int security_prompt_for_password = 0;
#endif
static int security_freeze   = 0;
static int security_master = 0, security_mode = 0;
static int enhanced_erase = 0;
static int set_security   = 0;
#if !defined (HDPARM_MINI)
static int do_dco_freeze = 0, do_dco_restore = 0, do_dco_identify = 0, do_dco_setmax = 0;
#endif
static unsigned int security_command = ATA_OP_SECURITY_UNLOCK;

static char security_password[33], *fwpath, *raw_identify_path;

static int do_sanitize = 0;
static __u16 sanitize_feature = 0;
static __u32 ow_pattern = 0;
static const char *sanitize_states_str[SANITIZE_STATE_NUMBER] = {
	"SD0 Sanitize Idle",
	"SD1 Sanitize Frozen",
	"SD2 Sanitize operation In Process",
	"SD3 Sanitize Operation Failed",
	"SD4 Sanitize Operation succeeded"
};
static const char *sanitize_err_reason_str[SANITIZE_ERR_NUMBER] = {
	"Reason not reported",
	"Last Sanitize Command completed unsuccessfully",
	"Unsupported command",
	"Device in FROZEN state",
	"Antifreeze lock enabled"
};

static int get_powermode  = 0, set_powermode = 0;
static int set_apmmode = 0, get_apmmode= 0, apmmode = 0;
#if !defined (HDPARM_MINI)
static int get_cdromspeed = 0, set_cdromspeed = 0, cdromspeed = 0;
static int do_fwdownload = 0, xfer_mode = 0;
static int drq_hsm_error = 0;
#endif
static int do_IDentity = 0;
static int	set_busstate = 0, get_busstate = 0, busstate = 0;
static int	set_reread_partn = 0, get_reread_partn;
static int	set_acoustic = 0, get_acoustic = 0, acoustic = 0;
static int write_read_verify = 0, get_write_read_verify = 0, set_write_read_verify = 0;

static int   make_bad_sector = 0, make_bad_sector_flagged;
static __u64 make_bad_sector_addr = ~0ULL;

#ifdef FORMAT_AND_ERASE
static int   format_track = 0;
static __u64 format_track_addr = ~0ULL;

static int   erase_sectors = 0;
static __u64 erase_sectors_addr = ~0ULL;
#endif

static struct sector_range_s *trim_sector_ranges = NULL;
static int   trim_sector_ranges_count = 0;
static int   trim_from_stdin = 0;
static int   do_set_sector_size = 0;
static __u64 new_sector_size = 0;
#define SET_SECTOR_SIZE "set-sector-size"

static int   write_sector = 0;
static __u64 write_sector_addr = ~0ULL;

static int   read_sector = 0;
static __u64 read_sector_addr = ~0ULL;

#if !defined (HDPARM_MINI)
static int   set_max_sectors = 0, set_max_permanent, get_native_max_sectors = 0;
static __u64 set_max_addr = 0;
#endif

static int	get_doreset = 0, set_doreset = 0;
static int	i_know_what_i_am_doing = 0;
static int	please_destroy_my_drive = 0;

const int timeout_15secs = 15;
const int timeout_60secs = 60;
const int timeout_5mins  = (5 * 60);
const int timeout_2hrs   = (2 * 60 * 60);

static int open_flags = O_RDONLY|O_NONBLOCK;

// Historically, if there was no HDIO_OBSOLETE_IDENTITY, then
// then the HDIO_GET_IDENTITY only returned 142 bytes.
// Otherwise, HDIO_OBSOLETE_IDENTITY returns 142 bytes,
// and HDIO_GET_IDENTITY returns 512 bytes.  But the latest
// 2.5.xx kernels no longer define HDIO_OBSOLETE_IDENTITY
// (which they should, but they should just return -EINVAL).
//
// So.. we must now assume that HDIO_GET_IDENTITY returns 512 bytes.
// On a really old system, it will not, and we will be confused.
// Too bad, really.

const char *cfg_str[] =
{	"",	        " HardSect",   " SoftSect",  " NotMFM",
	" HdSw>15uSec", " SpinMotCtl", " Fixed",     " Removable",
	" DTR<=5Mbs",   " DTR>5Mbs",   " DTR>10Mbs", " RotSpdTol>.5%",
	" dStbOff",     " TrkOff",     " FmtGapReq", " nonMagnetic"
};

const char *SlowMedFast[]	= {"slow", "medium", "fast", "eide", "ata"};
const char *BuffType[4]		= {"unknown", "1Sect", "DualPort", "DualPortCache"};

#define YN(b)	(((b)==0)?"no":"yes")

static void on_off (unsigned int value)
{
	printf(value ? " (on)\n" : " (off)\n");
}

#ifndef ENOIOCTLCMD
#define ENOIOCTLCMD ENOTTY
#endif

#define DCO_CHECKSUM_WORDS	154
// the DCO spec says that the checksum is the 2's compelement of the sum of all bytes in words 0-154 + byte 511. 
static __u8 dco_verify_checksum(__u16 *dcobuffer)
{
	__u8 csum = 0;
	int i;

	for(i = 0; i < DCO_CHECKSUM_WORDS; i++) {
		csum += (dcobuffer[i] & 0xFF);
		csum += (dcobuffer[i] >> 8);
	}
	// The INTEL drives have a byte OUTSIDE of the valid checksum area,
	//  and they erroneously include it in the checksum! WARNING: KLUDGE!
	if (dcobuffer[208] != 0) {
		csum += (dcobuffer[208] & 0xFF);
		csum += (dcobuffer[208] >> 8);
	}
	// get the signature byte
	csum += (dcobuffer[255] & 0xFF);
	return (0-csum);
}

static void flush_buffer_cache (int fd)
{
	sync();
	fsync(fd);				/* flush buffers */
	fdatasync(fd);				/* flush buffers */
	sync();
	if (ioctl(fd, BLKFLSBUF, NULL))		/* do it again, big time */
		perror("BLKFLSBUF failed");
	else
		do_drive_cmd(fd, NULL, 0);	/* IDE: await completion */
	sync();
}

static int seek_to_zero (int fd)
{
	if (lseek(fd, (off_t) 0, SEEK_SET)) {
		perror("lseek() failed");
		return 1;
	}
	return 0;
}

static int read_big_block (int fd, char *buf)
{
	int i, rc;
	if ((rc = read(fd, buf, TIMING_BUF_BYTES)) != TIMING_BUF_BYTES) {
		if (rc) {
			if (rc == -1)
				perror("read() failed");
			else
				fprintf(stderr, "read(%u) returned %u bytes\n", TIMING_BUF_BYTES, rc);
		} else {
			fputs ("read() hit EOF - device too small\n", stderr);
		}
		return EIO;
	}
	/* access all sectors of buf to ensure the read fully completed */
	for (i = 0; i < TIMING_BUF_BYTES; i += 512)
		buf[i] &= 1;
	return 0;
}

static void *prepare_timing_buf (unsigned int len)
{
	unsigned int i;
	__u8 *buf;

	buf = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (buf == MAP_FAILED) {
		perror("could not allocate timing buf");
		return NULL;
	}
	for (i = 0; i < len; i += 4096)
		buf[i] = 0; /* guarantee memory is present/assigned */
	if (-1 == mlock(buf, len)) {
		perror("mlock() failed on timing buf");
		munmap(buf, len);
		return NULL;
	}
	mlockall(MCL_CURRENT|MCL_FUTURE); // don't care if this fails on low-memory machines
	sync();

	/* give time for I/O to settle */
	sleep(3);
	return buf;
}

static void time_cache (int fd)
{
	char *buf;
	struct itimerval e1, e2;
	double elapsed, elapsed2;
	unsigned int iterations, total_MB;

	buf = prepare_timing_buf(TIMING_BUF_BYTES);
	if (!buf)
		return;

	/*
	 * getitimer() is used rather than gettimeofday() because
	 * it is much more consistent (on my machine, at least).
	 */
	setitimer(ITIMER_REAL, &(struct itimerval){{1000,0},{1000,0}}, NULL);
	if (seek_to_zero (fd)) return;
	if (read_big_block (fd, buf)) return;
	printf(" Timing %scached reads:   ", (open_flags & O_DIRECT) ? "O_DIRECT " : "");
	fflush(stdout);

	/* Clear out the device request queues & give them time to complete */
	flush_buffer_cache(fd);
	sleep(1);

	/* Now do the timing */
	iterations = 0;
	getitimer(ITIMER_REAL, &e1);
	do {
		++iterations;
		if (seek_to_zero (fd) || read_big_block (fd, buf))
			goto quit;
		getitimer(ITIMER_REAL, &e2);
		elapsed = (e1.it_value.tv_sec - e2.it_value.tv_sec)
		 + ((e1.it_value.tv_usec - e2.it_value.tv_usec) / 1000000.0);
	} while (elapsed < 2.0);
	total_MB = iterations * TIMING_BUF_MB;

	elapsed = (e1.it_value.tv_sec - e2.it_value.tv_sec)
	 + ((e1.it_value.tv_usec - e2.it_value.tv_usec) / 1000000.0);

	/* Now remove the lseek() and getitimer() overheads from the elapsed time */
	getitimer(ITIMER_REAL, &e1);
	do {
		if (seek_to_zero (fd))
			goto quit;
		getitimer(ITIMER_REAL, &e2);
		elapsed2 = (e1.it_value.tv_sec - e2.it_value.tv_sec)
		 + ((e1.it_value.tv_usec - e2.it_value.tv_usec) / 1000000.0);
	} while (--iterations);

	elapsed -= elapsed2;

	if (total_MB >= elapsed)  /* more than 1MB/s */
		printf("%3u MB in %5.2f seconds = %6.2f MB/sec\n",
			total_MB, elapsed,
			total_MB / elapsed);
	else
		printf("%3u MB in %5.2f seconds = %6.2f kB/sec\n",
			total_MB, elapsed,
			total_MB / elapsed * 1024);

	flush_buffer_cache(fd);
	sleep(1);
quit:
	munlockall();
	munmap(buf, TIMING_BUF_BYTES);
}

static int time_device (int fd)
{
	char *buf;
	double elapsed;
	struct itimerval e1, e2;
	int err = 0;
	unsigned int max_iterations = 1024, total_MB, iterations;

	/*
	 * get device size
	 */
	if (do_ctimings || do_timings) {
		__u64 nsectors;
		do_flush = 1;
		err = get_dev_geometry(fd, NULL, NULL, NULL, NULL, &nsectors);
		if (!err)
			max_iterations = nsectors / (2 * 1024) / TIMING_BUF_MB;
	}
	buf = prepare_timing_buf(TIMING_BUF_BYTES);
	if (!buf)
		err = ENOMEM;
	if (err)
		goto quit;

	printf(" Timing %s disk reads", (open_flags & O_DIRECT) ? "O_DIRECT" : "buffered");
	if (set_timings_offset)
		printf(" (offset %llu GB)", timings_offset / 0x40000000ULL);
	printf(": ");
	fflush(stdout);

	if (set_timings_offset && lseek64(fd, timings_offset, SEEK_SET) == (off64_t)-1) {
		err = errno;
		perror("lseek() failed");
		goto quit;
	}

	/*
	 * getitimer() is used rather than gettimeofday() because
	 * it is much more consistent (on my machine, at least).
	 */
	setitimer(ITIMER_REAL, &(struct itimerval){{1000,0},{1000,0}}, NULL);

	/* Now do the timings for real */
	iterations = 0;
	getitimer(ITIMER_REAL, &e1);
	do {
		++iterations;
		if ((err = read_big_block(fd, buf)))
			goto quit;
		getitimer(ITIMER_REAL, &e2);
		elapsed = (e1.it_value.tv_sec - e2.it_value.tv_sec)
		 + ((e1.it_value.tv_usec - e2.it_value.tv_usec) / 1000000.0);
	} while (elapsed < 3.0 && iterations < max_iterations);

	total_MB = iterations * TIMING_BUF_MB;
	if ((total_MB / elapsed) > 1.0)  /* more than 1MB/s */
		printf("%3u MB in %5.2f seconds = %6.2f MB/sec\n",
			total_MB, elapsed, total_MB / elapsed);
	else
		printf("%3u MB in %5.2f seconds = %6.2f kB/sec\n",
			total_MB, elapsed, total_MB / elapsed * 1024);
quit:
	munlockall();
	if (buf)
		munmap(buf, TIMING_BUF_BYTES);
	return err;
}

static void dmpstr (const char *prefix, unsigned int i, const char *s[], unsigned int maxi)
{
	if (i > maxi)
		printf("%s%u", prefix, i);
	else
		printf("%s%s", prefix, s[i]);
}

static __u16 *id;

#define SUPPORTS_ACS3(id) ((id)[80] & 0x400)
#define SUPPORTS_AMAX_ADDR(id) (SUPPORTS_ACS3(id) && ((id)[119] & (1u << 8)))
#define SUPPORTS_48BIT_ADDR(id) ((((id)[83] & 0xc400) == 0x4400) && ((id)[86] & 0x0400))

static void get_identify_data (int fd);

static __u64 get_lba_capacity (__u16 *idw)
{
	__u64 nsects = ((__u32)idw[58] << 16) | idw[57];

	if (idw[49] & 0x200) {
		nsects = ((__u32)idw[61] << 16) | idw[60];
		if ((idw[83] & 0xc000) == 0x4000 && (idw[86] & 0x0400)) {
			nsects = (__u64)idw[103] << 48 | (__u64)idw[102] << 32 |
			         (__u64)idw[101] << 16 | idw[100];
		}
	}
	return nsects;
}

static char *strip (char *s)
{
	char *e;

	while (*s == ' ') ++s;
	if (*s)
		for (e = s + strlen(s); *--e == ' '; *e = '\0');
	return s;
}

static void dump_identity (__u16 *idw)
{
	int i;
	char pmodes[64] = {0,}, dmodes[128]={0,}, umodes[128]={0,};
	char *model = strip(strndup((char *)&idw[27], 40));
	char *fwrev = strip(strndup((char *)&idw[23],  8));
	char *serno = strip(strndup((char *)&idw[10], 20));
	__u8 tPIO;

	printf("\n Model=%.40s, FwRev=%.8s, SerialNo=%.20s", model, fwrev, serno);
	printf("\n Config={");
	for (i = 0; i <= 15; i++) {
		if (idw[0] & (1<<i))
			printf("%s", cfg_str[i]);
	}
	printf(" }\n");
	printf(" RawCHS=%u/%u/%u, TrkSize=%u, SectSize=%u, ECCbytes=%u\n",
		idw[1], idw[3], idw[6], idw[4], idw[5], idw[22]);
	dmpstr(" BuffType=", idw[20], BuffType, 3);
	if (idw[21] && idw[21] != 0xffff)
		printf(", BuffSize=%ukB", idw[21] / 2);
	else
		printf(", BuffSize=unknown");
	printf(", MaxMultSect=%u", idw[47] & 0xff);
	if ((idw[47] & 0xff)) {
		printf(", MultSect=");
		if (!(idw[59] & 0x100))
			printf("?%u?", idw[59] & 0xff);
		else if (idw[59] & 0xff)
			printf("%u", idw[59] & 0xff);
		else
			printf("off");
	}
	putchar('\n');
	tPIO = idw[51] >> 8;
	if (tPIO <= 5) {
		strcat(pmodes, "pio0 ");
		if (tPIO >= 1) strcat(pmodes, "pio1 ");
		if (tPIO >= 2) strcat(pmodes, "pio2 ");
	}
	if (!(idw[53] & 1))
		printf(" (maybe):");
	printf(" CurCHS=%u/%u/%u, CurSects=%u", idw[54], idw[55], idw[56], idw[57] | (idw[58] << 16));
	printf(", LBA=%s", YN(idw[49] & 0x200));
	if (idw[49] & 0x200)
 		printf(", LBAsects=%llu", get_lba_capacity(idw));

	if (idw[49] & 0x100) {
		if (idw[62] | idw[63]) {
			if (idw[62] & 0x100)	strcat(dmodes,"*");
			if (idw[62] & 1)	strcat(dmodes,"sdma0 ");
			if (idw[62] & 0x200)	strcat(dmodes,"*");
			if (idw[62] & 2)	strcat(dmodes,"sdma1 ");
			if (idw[62] & 0x400)	strcat(dmodes,"*");
			if (idw[62] & 4)	strcat(dmodes,"sdma2 ");
			if (idw[62] & 0xf800)	strcat(dmodes,"*");
			if (idw[62] & 0xf8)	strcat(dmodes,"sdma? ");
			if (idw[63] & 0x100)	strcat(dmodes,"*");
			if (idw[63] & 1)	strcat(dmodes,"mdma0 ");
			if (idw[63] & 0x200)	strcat(dmodes,"*");
			if (idw[63] & 2)	strcat(dmodes,"mdma1 ");
			if (idw[63] & 0x400)	strcat(dmodes,"*");
			if (idw[63] & 4)	strcat(dmodes,"mdma2 ");
			if (idw[63] & 0xf800)	strcat(dmodes,"*");
			if (idw[63] & 0xf8)	strcat(dmodes,"mdma? ");
		}
	}
	printf("\n IORDY=");
	if (idw[49] & 0x800)
		printf((idw[49] & 0x400) ? "on/off" : "yes");
	else
		printf("no");
	if ((idw[49] & 0x800) || (idw[53] & 2)) {
		if ((idw[53] & 2)) {
			printf(", tPIO={min:%u,w/IORDY:%u}", idw[67], idw[68]);
			if (idw[64] & 1)	strcat(pmodes, "pio3 ");
			if (idw[64] & 2)	strcat(pmodes, "pio4 ");
			if (idw[64] &~3)	strcat(pmodes, "pio? ");
		}
		if (idw[53] & 4) {
			if (idw[88] & 0x100)	strcat(umodes,"*");
			if (idw[88] & 0x001)	strcat(umodes,"udma0 ");
			if (idw[88] & 0x200)	strcat(umodes,"*");
			if (idw[88] & 0x002)	strcat(umodes,"udma1 ");
			if (idw[88] & 0x400)	strcat(umodes,"*");
			if (idw[88] & 0x004)	strcat(umodes,"udma2 ");
			if (idw[88] & 0x800)	strcat(umodes,"*");
			if (idw[88] & 0x008)	strcat(umodes,"udma3 ");
			if (idw[88] & 0x1000)	strcat(umodes,"*");
			if (idw[88] & 0x010)	strcat(umodes,"udma4 ");
			if (idw[88] & 0x2000)	strcat(umodes,"*");
			if (idw[88] & 0x020)	strcat(umodes,"udma5 ");
			if (idw[88] & 0x4000)	strcat(umodes,"*");
			if (idw[88] & 0x040)	strcat(umodes,"udma6 ");
		}
	}
	if ((idw[49] & 0x100) && (idw[53] & 2))
		printf(", tDMA={min:%u,rec:%u}", idw[65], idw[66]);
	printf("\n PIO modes:  %s", pmodes);
	if (*dmodes)
		printf("\n DMA modes:  %s", dmodes);
	if (*umodes)
		printf("\n UDMA modes: %s", umodes);

	printf("\n AdvancedPM=%s",YN(idw[83]&8));
	if (idw[83] & 8) {
		if (!(idw[86]&8))
			printf(": disabled (255)");
		else if ((idw[91]&0xFF00)!=0x4000)
			printf(": unknown setting");
		else
			printf(": mode=0x%02X (%u)",idw[91]&0xFF,idw[91]&0xFF);
	}
	if (idw[82]&0x20)
		printf(" WriteCache=%s",(idw[85]&0x20) ? "enabled" : "disabled");
	if (idw[81] || idw[80]) {
		printf("\n Drive conforms to: ");
		if (idw[81] <= 31)
			printf("%s: ", minor_str[idw[81]]);
		else
			printf("unknown: ");
		if (idw[80] != 0x0000 &&  /* NOVAL_0 */
		    idw[80] != 0xFFFF) {  /* NOVAL_1 */
			int count = 0;
			for (i=0; i <= 7; i++) {
				if (idw[80] & (1<<i))
					printf("%s%u", count++ ? "," : " ATA/ATAPI-", i);
			}
		}
	}
	printf("\n");
	printf("\n * signifies the current active mode\n");
	printf("\n");
}

static const char *busstate_str (unsigned int value)
{
	static const char *states[4] = {"off", "on", "tristate", "unknown"};

	if (value > 3)
		value = 3;
	return states[value];
}

static void interpret_standby (void)
{
	printf(" (");
	switch(standby) {
		case 0:		printf("off");
				break;
		case 252:	printf("21 minutes");
				break;
		case 253:	printf("vendor-specific");
				break;
		case 254:	printf("?reserved");
				break;
		case 255:	printf("21 minutes + 15 seconds");
				break;
		default:
			if (standby <= 240) {
				unsigned int secs = standby * 5;
				unsigned int mins = secs / 60;
				secs %= 60;
				if (mins)	  printf("%u minutes", mins);
				if (mins && secs) printf(" + ");
				if (secs)	  printf("%u seconds", secs);
			} else if (standby <= 251) {
				unsigned int mins = (standby - 240) * 30;
				unsigned int hrs  = mins / 60;
				mins %= 60;
				if (hrs)	  printf("%u hours", hrs);
				if (hrs && mins)  printf(" + ");
				if (mins)	  printf("%u minutes", mins);
			} else {
				printf("illegal value");
			}
			break;
	}
	printf(")\n");
}

#if !defined (HDPARM_MINI)
struct xfermode_entry {
	int val;
	const char *name;
};

static const struct xfermode_entry xfermode_table[] = {
	{ 8,    "pio0" },
	{ 9,    "pio1" },
	{ 10,   "pio2" },
	{ 11,   "pio3" },
	{ 12,   "pio4" },
	{ 13,   "pio5" },
	{ 14,   "pio6" },
	{ 15,   "pio7" },
	{ 16,   "sdma0" },
	{ 17,   "sdma1" },
	{ 18,   "sdma2" },
	{ 19,   "sdma3" },
	{ 20,   "sdma4" },
	{ 21,   "sdma5" },
	{ 22,   "sdma6" },
	{ 23,   "sdma7" },
	{ 32,   "mdma0" },
	{ 33,   "mdma1" },
	{ 34,   "mdma2" },
	{ 35,   "mdma3" },
	{ 36,   "mdma4" },
	{ 37,   "mdma5" },
	{ 38,   "mdma6" },
	{ 39,   "mdma7" },
	{ 64,   "udma0" },
	{ 65,   "udma1" },
	{ 66,   "udma2" },
	{ 67,   "udma3" },
	{ 68,   "udma4" },
	{ 69,   "udma5" },
	{ 70,   "udma6" },
	{ 71,   "udma7" },
	{ 0, NULL }
};

static int translate_xfermode(char * name)
{
	const struct xfermode_entry *tmp;
	char *endptr;
	int val = -1;

	for (tmp = xfermode_table; tmp->name != NULL; ++tmp) {
		if (!strcmp(name, tmp->name))
			return tmp->val;
	}
	val = strtol(name, &endptr, 10);
	if (*endptr == '\0')
		return val;
	return -1;
}

static void interpret_xfermode (unsigned int xfermode)
{
	printf(" (");
	switch(xfermode) {
		case 0:		printf("default PIO mode");
				break;
		case 1:		printf("default PIO mode, disable IORDY");
				break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:	printf("PIO flow control mode%u", xfermode-8);
				break;
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:	printf("singleword DMA mode%u", xfermode-16);
				break;
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:	printf("multiword DMA mode%u", xfermode-32);
				break;
		case 64:
		case 65:
		case 66:
		case 67:
		case 68:
		case 69:
		case 70:
		case 71:	printf("UltraDMA mode%u", xfermode-64);
				break;
		default:
				printf("unknown, probably not valid");
				break;
	}
	printf(")\n");
}
#endif

static unsigned int get_erase_timeout_secs (int fd, int enhanced)
{
	// Grab ID Data
	get_identify_data(fd);

	if (id == NULL) {
		// ID pointer is invalid, return a default of twelve hours
		return 12 * 60 * 60;
	}

	// Build an estimate at 1 second per 30MB of capacity (Norman Diamond suggestion)
	// Add 30 mins of uncertainty
	__u64 const lba_limit = get_lba_capacity(id);
	__u64 const estimate  = ((lba_limit / 2048ULL) / 30ULL) + (30 * 60);

	// Grab the timeout field from the ID
	// If enhanced is non-zero then look at word 90, otherwise look at word 89
	// If bit 15 is set, then the time field is bits [14:0]
	// Otherwise the time field is bits [7:0] (ACS-3)
	unsigned int const idx      = (enhanced != 0) ? 90: 89;
	unsigned int       timeout  = id[idx];
	unsigned int const ext_time = (timeout & (1 << 15)) != 0;

	// Mask off reserved bits
	timeout = ext_time ? timeout & 0x7FFF: timeout & 0x00FF;
	if (timeout == 0) {
		// Value is not specified, return the estimate
		return estimate;
	}

	// Decode timeout (Add some wiggle room)
	timeout = ( ext_time && (timeout == 0x7FFF)) ? 65532 + 90:         // Max ext time is > 65532 minutes
	          (!ext_time && (timeout == 0x00FF)) ? 508 + 90:           // Max non-ext time is > 508 minutes
	                                               (timeout * 2) + 60; // Time is id value * 2 mins
	timeout *= 60; // Convert timeout to seconds

	// Return the larger value between timeout and estimate
	return (timeout < estimate) ? estimate: timeout;
}
static int
get_sanitize_state(__u8 nsect)
{
	int state = SANITIZE_IDLE_STATE_SD0;
	if (nsect & SANITIZE_FLAG_DEVICE_IN_FROZEN) {
		state = SANITIZE_FROZEN_STATE_SD1;
	} else if (nsect & SANITIZE_FLAG_OPERATION_IN_PROGRESS) {
		state = SANITIZE_OPERATION_IN_PROGRESS_SD2;
	}
	return state;
}

static void
sanitize_normal_output(int sanitize_state, struct hdio_taskfile * r)
{
	printf("    State:    %s\n", sanitize_states_str[sanitize_state]);
	if (sanitize_state == SANITIZE_OPERATION_IN_PROGRESS_SD2) {
		int progress = (r->lob.lbam << 8) | r->lob.lbal;
		int percent = (progress == 0xFFFF) ? (100) : ((100 * (progress + 1)) / 0xFFFF);
		printf("    Progress: 0x%x (%d%%)\n", progress, percent);
	}
	if (r->hob.nsect & SANITIZE_FLAG_OPERATION_SUCCEEDED) {
		printf("    Last Sanitize Operation Completed Without Error\n");
	}
	if (r->hob.nsect & SANITIZE_FLAG_ANTIFREEZE_BIT)
		printf("    Antifreeze bit set\n");
}

static void
sanitize_error_output(struct hdio_taskfile * r)
{
	int err_reason = (r->lob.lbal >= SANITIZE_ERR_NUMBER) ? (SANITIZE_ERR_NO_REASON) : (r->lob.lbal);
	fprintf(stderr, "SANITIZE device error reason: %s\n", sanitize_err_reason_str[err_reason]);
	if (err_reason == SANITIZE_ERR_CMD_UNSUCCESSFUL)
		fprintf(stderr, "Drive in %s state\n", sanitize_states_str[SANITIZE_OPERATION_FAILED_SD3]);
}

static void
do_sanitize_cmd (int fd)
{
	int err = 0;
	__u64 lba = 0;
	const char *description;
	int sanitize_state;
	struct hdio_taskfile r;

	get_identify_data(fd);
	if (!id)
		exit(EIO);
	if (id[59] & 0x1000) {

		switch (sanitize_feature) {
			case SANITIZE_STATUS_EXT:
				description = "SANITIZE_STATUS";
				break;
			case SANITIZE_CRYPTO_SCRAMBLE_EXT:
				lba = SANITIZE_CRYPTO_SCRAMBLE_KEY;
				description = "SANITIZE_CRYPTO_SCRAMBLE";
				break;
			case SANITIZE_BLOCK_ERASE_EXT:
				lba = SANITIZE_BLOCK_ERASE_KEY;
				description = "SANITIZE_BLOCK_ERASE";
				break;
			case SANITIZE_OVERWRITE_EXT:
				lba = ((__u64)(SANITIZE_OVERWRITE_KEY) << 32) | ow_pattern;
				description = "SANITIZE_OVERWRITE";
				break;
			case SANITIZE_FREEZE_LOCK_EXT:
				lba = SANITIZE_FREEZE_LOCK_KEY;
				description = "SANITIZE_FREEZE_LOCK";
				break;
			case SANITIZE_ANTIFREEZE_LOCK_EXT:
				lba = SANITIZE_ANTIFREEZE_LOCK_KEY;
				description = "SANITIZE_ANTIFREEZE_LOCK";
				break;
			default:
				fprintf(stderr, "BUG in do_sanitize_cmd(), feat=0x%x\n", sanitize_feature);
				exit(EINVAL);
		}

		memset(&r, 0, sizeof(r));
		r.cmd_req = TASKFILE_CMD_REQ_NODATA;
		r.dphase  = TASKFILE_DPHASE_NONE;

		r.oflags.bits.lob.command = 1;
		r.oflags.bits.lob.feat    = 1;
		r.oflags.bits.lob.lbal    = 1;
		r.oflags.bits.lob.lbam    = 1;
		r.oflags.bits.lob.lbah    = 1;
		r.oflags.bits.hob.lbal    = 1;
		r.oflags.bits.hob.lbam    = 1;
		r.oflags.bits.hob.lbah    = 1;

		r.lob.command = ATA_OP_SANITIZE;
		r.lob.feat    = sanitize_feature;
		r.lob.lbal    = lba;
		r.lob.lbam    = lba >>  8;
		r.lob.lbah    = lba >> 16;
		r.hob.lbal    = lba >> 24;
		r.hob.lbam    = lba >> 32;
		r.hob.lbah    = lba >> 40;

		r.iflags.bits.lob.lbal    = 1;
		r.iflags.bits.lob.lbam    = 1;
		r.iflags.bits.hob.nsect   = 1;

		printf("Issuing %s command\n", description);
		if (do_taskfile_cmd(fd, &r, 10)) {
			err = errno;
			perror("SANITIZE failed");
			sanitize_error_output(&r);
		}
		else {
			switch (sanitize_feature) {
				case SANITIZE_STATUS_EXT:
					printf("Sanitize status:\n");
					sanitize_state = get_sanitize_state(r.hob.nsect);
					sanitize_normal_output(sanitize_state, &r);
					break;
				case SANITIZE_BLOCK_ERASE_EXT:
				case SANITIZE_OVERWRITE_EXT:
				case SANITIZE_CRYPTO_SCRAMBLE_EXT:
					printf("Operation started in background\n");
					printf("You may use `--sanitize-status` to check progress\n");
					break;
				default:
					//nothing here
					break;
			}
		}
	} else {
		fprintf(stderr, "SANITIZE feature set is not supported\n");
		exit(EINVAL);
	}
	if (err)
		exit(err);
}

static void
do_set_security (int fd)
{
	int err = 0;
	const char *description;
	struct hdio_taskfile *r;
	__u8 *data;

	r = malloc(sizeof(struct hdio_taskfile) + 512);
	if (!r) {
		err = errno;
		perror("malloc()");
		exit(err);
	}

	memset(r, 0, sizeof(struct hdio_taskfile) + 512);
	r->cmd_req	= TASKFILE_CMD_REQ_OUT;
	r->dphase	= TASKFILE_DPHASE_PIO_OUT;
	r->obytes	= 512;
	r->lob.command	= security_command;
	r->oflags.bits.lob.nsect = 1;
	r->lob.nsect    = 1;
	data		= (__u8*)r->data;
	data[0]		= security_master & 0x01;
	memcpy(data+2, security_password, 32);

	r->oflags.bits.lob.command = 1;
	r->oflags.bits.lob.feat    = 1;

	switch (security_command) {
		case ATA_OP_SECURITY_ERASE_UNIT:
			description = "SECURITY_ERASE";
			data[0] |= enhanced_erase ? 0x02 : 0;
			break;
		case ATA_OP_SECURITY_DISABLE:
			description = "SECURITY_DISABLE";
			break;
		case ATA_OP_SECURITY_UNLOCK:
			description = "SECURITY_UNLOCK";
			break;
		case ATA_OP_SECURITY_SET_PASS:
			description = "SECURITY_SET_PASS";
			data[1] = (security_mode & 0x01);
			if (security_master) {
				/* increment master-password revision-code */
				__u16 revcode;
				get_identify_data(fd);
				if (!id)
					exit(EIO);
				revcode = id[92];
				if (revcode == 0xfffe)
					revcode = 0;
				revcode += 1;
				data[34] = revcode;
				data[35] = revcode >> 8;
			}
			break;
		default:
			fprintf(stderr, "BUG in do_set_security(), command1=0x%x\n", security_command);
			exit(EINVAL);
	}
	if (!quiet) {
		printf(" Issuing %s command, password=\"%s\", user=%s",
			description, security_password, (data[0] & 1) ? "master" : "user");
		if (security_command == ATA_OP_SECURITY_SET_PASS)
			printf(", mode=%s", data[1] ? "max" : "high");
		printf("\n");
	}

	/*
	 * The Linux kernel IDE driver (until at least 2.6.12) segfaults on the first
	 * command when issued on a locked drive, and the actual erase is never issued.
	 * One could patch the code to issue separate commands for erase prepare and
	 * erase to erase a locked drive.
	 *
	 * We would like to issue these commands consecutively, but since the Linux
	 * kernel until at least 2.6.12 segfaults on each command issued the second will
	 * never be executed.
	 *
	 * One is at least able to issue the commands consecutively in two hdparm invocations,
	 * assuming the segfault isn't followed by an oops.
	 */
	if (security_command == ATA_OP_SECURITY_ERASE_UNIT) {
		unsigned int timeout = get_erase_timeout_secs(fd, enhanced_erase);
		__u8 args[4] = {ATA_OP_SECURITY_ERASE_PREPARE,0,0,0};
		if (do_drive_cmd(fd, args, 0)) {   
			err = errno;
			perror("ERASE_PREPARE");
		} else {
			if ((do_taskfile_cmd(fd, r, timeout))) {
				err = errno;
				perror("SECURITY_ERASE");
			}
		}
	} else if (security_command == ATA_OP_SECURITY_DISABLE) {
		/* First attempt an unlock  */
		r->lob.command = ATA_OP_SECURITY_UNLOCK;
		if ((do_taskfile_cmd(fd, r, timeout_15secs))) {
			err = errno;
			perror("SECURITY_UNLOCK");
		} else {
			/* Then the security disable */
			r->lob.command = security_command;
			if ((do_taskfile_cmd(fd, r, timeout_15secs))) {
				err = errno;
				perror("SECURITY_DISABLE");
			}
		}
	} else if (security_command == ATA_OP_SECURITY_UNLOCK) {
		if ((do_taskfile_cmd(fd, r, timeout_15secs))) {
			err = errno;
			perror("SECURITY_UNLOCK");
		}
	} else if (security_command == ATA_OP_SECURITY_SET_PASS) {
		if ((do_taskfile_cmd(fd, r, timeout_15secs))) {
			err = errno;
			perror("SECURITY_SET_PASS");
		}
	} else {
		fprintf(stderr, "BUG in do_set_security(), command2=0x%x\n", security_command);
		err = EINVAL;
	}
	free(r);
	if (err)
		exit(err);
}

static __u8 last_identify_op = 0;

static void get_identify_data (int fd)
{
	static __u8 args[4+512];
	int i;

	if (id)
		return;
	memset(args, 0, sizeof(args));
	last_identify_op = ATA_OP_IDENTIFY;
	args[0] = last_identify_op;
	args[3] = 1;	/* sector count */
	if (do_drive_cmd(fd, args, 0)) {
		prefer_ata12 = 0;
		memset(args, 0, sizeof(args));
		last_identify_op = ATA_OP_PIDENTIFY;
		args[0] = last_identify_op;
		args[3] = 1;	/* sector count */
		if (do_drive_cmd(fd, args, 0)) {
			perror(" HDIO_DRIVE_CMD(identify) failed");
			return;
		}
	}
	/* byte-swap the little-endian IDENTIFY data to match byte-order on host CPU */
	id = (void *)(args + 4);
	for (i = 0; i < 0x100; ++i) {
		unsigned char *b = (unsigned char *)&id[i];
		id[i] = b[0] | (b[1] << 8);	/* le16_to_cpu() */
	}
}

static int do_read_log (int fd, __u8 log_address, __u8 pagenr, void *buf)
{
	struct hdio_taskfile *r;
	int err = 0;

	r = malloc(sizeof(struct hdio_taskfile) + 512);
	if (!r) {
		err = errno;
		perror("malloc()");
		return err;
	}

	init_hdio_taskfile(r, ATA_OP_READ_LOG_EXT, RW_READ, LBA48_FORCE, log_address + (pagenr << 8), 1, 512);
	if (do_taskfile_cmd(fd, r, timeout_15secs)) {
		err = errno;
	} else {
		memcpy(buf, r->data, 512);
	}
	free(r);
	return err;
}


int get_log_page_data (int fd, __u8 log_address, __u8 pagenr, __u8 *buf)
{
	static __u16 *page0 = NULL, page0_buf[512] = {0,};
	int err;

	get_identify_data(fd);
	if (!id)
		exit(EIO);
	if ((id[84] && (1<<5)) == 0)
		return -ENOENT;  /* READ_LOG_EXT not supported */
	if (!page0) {
		err = do_read_log(fd, 0, 0, page0_buf);
		if (err) {
			fprintf(stderr, "READ_LOG_EXT(0,0) failed: %s\n", strerror(err));
			return -ENOENT;
		}
		page0 = page0_buf;
	}
	if (page0[log_address] <= pagenr)
		return -ENOENT;
	err = do_read_log(fd, log_address, pagenr, buf);
	if (err) {
		fprintf(stderr, "READ_LOG_EXT(0x%02x, %u) failed: %s\n", log_address, pagenr, strerror(err));
		return -ENOENT;
	}
	return 0;
}

static void confirm_i_know_what_i_am_doing (const char *opt, const char *explanation)
{
	if (!i_know_what_i_am_doing) {
		fprintf(stderr, "Use of %s is VERY DANGEROUS.\n%s\n"
		"Please supply the --yes-i-know-what-i-am-doing flag if you really want this.\n"
		"Program aborted.\n", opt, explanation);
		exit(EPERM);
	}
}

static void confirm_please_destroy_my_drive (const char *opt, const char *explanation)
{
	if (!please_destroy_my_drive) {
		fprintf(stderr, "Use of %s is EXTREMELY DANGEROUS.\n%s\n"
		"Please also supply the --please-destroy-my-drive flag if you really want this.\n"
		"Program aborted.\n", opt, explanation);
		exit(EPERM);
	}
}

static int flush_wcache (int fd)
{
	__u8 args[4] = {ATA_OP_FLUSHCACHE,0,0,0};
	int err = 0;

	get_identify_data(fd);
	if (id && (id[83] & 0xe000) == 0x6000)
		args[0] = ATA_OP_FLUSHCACHE_EXT;
	if (do_drive_cmd(fd, args, timeout_60secs)) {
		err = errno;
		perror (" HDIO_DRIVE_CMD(flushcache) failed");
	}
	return err;
}

static void dump_sectors (__u16 *w, unsigned int count, int raw, unsigned int sector_bytes)
{
	unsigned int i;

	for (i = 0; i < (count*(sector_bytes/2)/8); ++i) {
		if (raw) {
			printf("%04x %04x %04x %04x %04x %04x %04x %04x\n",
				w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7]);
			w += 8;
		} else {
			int word;
			for (word = 0; word < 8; ++word) {
				unsigned char *b = (unsigned char *)w++;
				printf("%02x%02x", b[0], b[1]);
				putchar(word == 7 ? '\n' : ' ');
			}
		}
	}
}

static int abort_if_not_full_device (int fd, __u64 lba, const char *devname, const char *msg)
{
	struct stat stat;
	__u64 start_lba;
	int i, err, shortened = 0;
	char *fdevname = strdup(devname);

	if (0 == fstat(fd, &stat) && S_ISCHR(stat.st_mode))
		return 0; /* skip geometry test for character (non-block) devices; eg. /dev/sg* */
	err = get_dev_geometry(fd, NULL, NULL, NULL, &start_lba, NULL);
	if (err)
		exit(err);
	for (i = strlen(fdevname); --i > 2 && (fdevname[i] >= '0' && fdevname[i] <= '9');) {
		fdevname[i] = '\0';
		shortened = 1;
	}

	if (!shortened)
		fdevname = strdup("the full disk");

	if (start_lba == 0ULL)
		return 0;
	if (start_lba == START_LBA_UNKNOWN || fd_is_raid(fd)) {
		fprintf(stderr, "%s is a RAID device: please specify an absolute LBA of a raw member device instead (raid1 only)\n", devname);
	} else if (msg) {
		fprintf(stderr, "%s\n", msg);
	} else {
		fprintf(stderr, "Device %s has non-zero LBA starting offset of %llu.\n", devname, start_lba);
		fprintf(stderr, "Please use an absolute LBA with the /dev/ entry for the raw device, rather than a partition or raid name.\n");
		fprintf(stderr, "%s is probably a partition of %s (?)\n", devname, fdevname);
		fprintf(stderr, "The absolute LBA of sector %llu from %s should be %llu\n", lba, devname, start_lba + lba);
	}
	fprintf(stderr, "Aborting.\n");
	exit(EINVAL);
}

#if !defined (HDPARM_MINI)
static __u16 *get_dco_identify_data (int fd, int quietly)
{
	static __u8 args[4+512];
	__u16 *dco = (void *)(args + 4);
	int i;
	
	memset(args, 0, sizeof(args));
	args[0] = ATA_OP_DCO;
	args[2] = 0xc2;
	args[3] = 1;
	if (do_drive_cmd(fd, args, 0)) {
		if (!quietly)
			perror(" HDIO_DRIVE_CMD(dco_identify) failed");
		return NULL;
	} else {
		/* byte-swap the little-endian DCO data to match byte-order on host CPU */
		for (i = 0; i < 0x100; ++i) {
			unsigned char *b = (unsigned char *)&dco[i];
			dco[i] = b[0] | (b[1] << 8);	/* le16_to_cpu */
		}
		//dump_sectors(dco, 1, 0, 512);
		return dco;
	}
}

static void
do_dco_setmax_cmd (int fd)
{
	int err = 0;
	struct hdio_taskfile *r;
	__u8 *data;
	__u16 *dco = (__u16 *) NULL;

	r = malloc(sizeof(struct hdio_taskfile) + 512);
	if (!r) {
		err = errno;
		perror("malloc()");
		exit(err);
	}

	// first, get the dco data
	dco = get_dco_identify_data(fd, 0);
	if (dco != ((__u16 *) NULL)) {
		__u64 *maxlba = (__u64 *) &dco[3];

		// first, check DCO checksum
		if (dco_verify_checksum(dco) != (dco[255] >> 8)) {
			printf("DCO Checksum FAILED!\n");
			exit(1);
		}
		if (verbose) {
			printf("Original DCO:\n");
			dump_sectors(dco, 1, 0, 512);
		}
		// set the new MAXLBA to the requested sectors - 1
		*maxlba = set_max_addr - 1;
		// recalculate the checksum
		dco[255] = (dco[255] & 0xFF) | ((__u16) dco_verify_checksum(dco) << 8);
		if (verbose) {
			printf("New DCO:\n");
			dump_sectors(dco, 1, 0, 512);
		}

	} else {
		printf("DCO data is NULL!\n");
		exit(1);
	}
	memset(r, 0, sizeof(struct hdio_taskfile) + 512);
	r->cmd_req	= TASKFILE_CMD_REQ_OUT;
	r->dphase	= TASKFILE_DPHASE_PIO_OUT;
	r->obytes	= 512;
	r->lob.command	= ATA_OP_DCO;
	r->oflags.bits.lob.command = 1;
	r->lob.feat        = 0xc3;
	r->oflags.bits.lob.feat = 1;
	data		= (__u8*)r->data;
	// copy data from new dco to output buffer
	memcpy(data, (__u8*) dco, 512);
	if ((do_taskfile_cmd(fd, r, timeout_15secs))) {
		err = errno;
		perror("DEVICE CONFIGURATION SET");
	} 
	free(r);
	if (err)
		exit(err);
}

static __u64 do_get_native_max_sectors (int fd)
{
	int err = 0;
	__u64 max = 0;
	struct hdio_taskfile r;

	get_identify_data(fd);
	if (!id)
		exit(EIO);
	memset(&r, 0, sizeof(r));
	r.cmd_req = TASKFILE_CMD_REQ_NODATA;
	r.dphase  = TASKFILE_DPHASE_NONE;
	r.oflags.bits.lob.command  = 1;
	r.iflags.bits.lob.command  = 1;
	r.iflags.bits.lob.lbal     = 1;
	r.iflags.bits.lob.lbam     = 1;
	r.iflags.bits.lob.lbah     = 1;

	if (SUPPORTS_AMAX_ADDR(id)) {
		/* ACS3 supported, no 28-bit variant defined in spec */
		r.iflags.bits.hob.lbal 	= 1;
		r.iflags.bits.hob.lbam 	= 1;
		r.iflags.bits.hob.lbah	= 1;
		r.oflags.bits.lob.feat  = 1;
		r.lob.command = ATA_OP_GET_NATIVE_MAX_EXT;
		r.lob.feat = 0x00; //GET NATIVE MAX ADDRESS EXT is 78h/0000h
		//bit 6 of DEVICE field is defined as "N/A"
		if (do_taskfile_cmd(fd, &r, 0)) {
			err = errno;
			perror (" GET_NATIVE_MAX_ADDRESS_EXT failed");
		} else {
			if (verbose)
				printf("GET_NATIVE_MAX_ADDRESS_EXT response: hob={%02x %02x %02x} lob={%02x %02x %02x}\n",
					   r.hob.lbah, r.hob.lbam, r.hob.lbal, r.lob.lbah, r.lob.lbam, r.lob.lbal);
			max = (((__u64)((r.hob.lbah << 16) | (r.hob.lbam << 8) | r.hob.lbal) << 24)
				   | ((r.lob.lbah << 16) | (r.lob.lbam << 8) | r.lob.lbal)) + 1;
		}
	} else { // ACS2 or below, or optional AMAX not present
		if (SUPPORTS_48BIT_ADDR(id)) {
			r.iflags.bits.hob.lbal  = 1;
			r.iflags.bits.hob.lbam  = 1;
			r.iflags.bits.hob.lbah  = 1;
			r.oflags.bits.lob.dev   = 1;
			r.lob.command = ATA_OP_READ_NATIVE_MAX_EXT;
			r.lob.dev = 0x40;
			if (do_taskfile_cmd(fd, &r, timeout_15secs)) { //timeout for pre-ACS3 case of do_set_max_sectors
				err = errno;
				perror (" READ_NATIVE_MAX_ADDRESS_EXT failed");
			} else {
				if (verbose)
					printf("READ_NATIVE_MAX_ADDRESS_EXT response: hob={%02x %02x %02x} lob={%02x %02x %02x}\n",
						r.hob.lbah, r.hob.lbam, r.hob.lbal, r.lob.lbah, r.lob.lbam, r.lob.lbal);
				max = (((__u64)((r.hob.lbah << 16) | (r.hob.lbam << 8) | r.hob.lbal) << 24)
				     	| ((r.lob.lbah << 16) | (r.lob.lbam << 8) | r.lob.lbal)) + 1;
			}
		} else {
			/* DEVICE (3:0) / LBA (27:24) "remap" does NOT apply in ATA Status Return */
			r.iflags.bits.hob.lbal = 1;
			r.lob.command = ATA_OP_READ_NATIVE_MAX;
			//bit 7:5 of DEVICE field is defined as "Obsolete"
			if (do_taskfile_cmd(fd, &r, timeout_15secs)) { //timeout for pre-ACS3 case of do_set_max_sectors
				err = errno;
				perror (" READ_NATIVE_MAX_ADDRESS failed");
			} else {
				max = (((r.hob.lbal & 0x0f) << 24) | (r.lob.lbah << 16) | (r.lob.lbam << 8) | r.lob.lbal) + 1;
			}
		}
	}

	errno = err;
	return max;
	
}
#endif

static int do_make_bad_sector (int fd, __u64 lba, const char *devname)
{
	int err = 0, has_write_unc = 0;
	struct hdio_taskfile *r;
	const char *flagged;

	abort_if_not_full_device(fd, lba, devname, NULL);
	r = malloc(sizeof(struct hdio_taskfile) + 520);
	if (!r) {
		err = errno;
		perror("malloc()");
		return err;
	}

	get_identify_data(fd);
	if (id)
		has_write_unc = (id[ 83] & 0xc000) == 0x4000 && (id[ 86] & 0x8000) == 0x8000
			     && (id[119] & 0xc004) == 0x4004 && (id[120] & 0xc000) == 0x4000;

	if (has_write_unc || make_bad_sector_flagged || lba >= lba28_limit) {
		if (!has_write_unc) {
			printf("Device does not claim to implement the required WRITE_UNC_EXT command\n"
				"This operation will probably fail (continuing regardless).\n");
		}
		init_hdio_taskfile(r, ATA_OP_WRITE_UNC_EXT, RW_READ, LBA48_FORCE, lba, 1, 0);
		r->oflags.bits.lob.feat = 1;
		r->lob.feat = make_bad_sector_flagged ? 0xaa : 0x55;
		flagged     = make_bad_sector_flagged ? "flagged" : "pseudo";
		printf("Corrupting sector %llu (WRITE_UNC_EXT as %s): ", lba, flagged);
	} else {
		init_hdio_taskfile(r, ATA_OP_WRITE_LONG_ONCE, RW_WRITE, LBA28_OK, lba, 1, 520);
		memset(r->data, 0xa5, 520);
		printf("Corrupting sector %llu (WRITE_LONG): ", lba);
	}
	fflush(stdout);

	/* Try and ensure that the system doesn't have our sector in cache */
	flush_buffer_cache(fd);

	if (do_taskfile_cmd(fd, r, timeout_60secs)) {
		err = errno;
		perror("FAILED");
	} else {
		printf("succeeded\n");
	}
	free(r);
	return err;
}

#ifdef FORMAT_AND_ERASE
static int do_format_track (int fd, __u64 lba, const char *devname)
{
	int err = 0;
	struct hdio_taskfile *r;

	abort_if_not_full_device(fd, lba, devname, NULL);
	r = malloc(sizeof(struct hdio_taskfile) + 512);
	if (!r) {
		err = errno;
		perror("malloc()");
		return err;
	}
	init_hdio_taskfile(r, ATA_OP_FORMAT_TRACK, RW_WRITE, LBA28_OK, lba, 1, 512);
	r->lob.nsect = 0;

	printf("re-formatting lba %llu: ", lba);
	fflush(stdout);

	if (do_taskfile_cmd(fd, r, timeout_60secs)) {
		err = errno;
		perror("FAILED");
	} else {
		printf("succeeded\n");
	}

	// Try and ensure that the system doesn't have our sector in cache:
	flush_buffer_cache(fd);

	free(r);
	return err;
}

static int do_erase_sectors (int fd, __u64 lba, const char *devname)
{
	int err = 0;
	struct hdio_taskfile *r;

	abort_if_not_full_device(fd, lba, devname, NULL);
	r = malloc(sizeof(struct hdio_taskfile) + 0);
	if (!r) {
		err = errno;
		perror("malloc()");
		return err;
	}
	init_hdio_taskfile(r, ATA_OP_ERASE_SECTORS, RW_READ, LBA28_OK, lba, 256, 0);

	printf("erasing sectors %llu-%llu: ", lba, lba + 255);
	fflush(stdout);

	if (do_taskfile_cmd(fd, r, timeout_60secs)) {
		err = errno;
		perror("FAILED");
	} else {
		printf("succeeded\n");
	}
	free(r);
	return err;
}
#endif /* FORMAT_AND_ERASE */

struct sector_range_s {
	__u64	lba;
	__u64	nsectors;
};

static int trim_sectors (int fd, const char *devname, int nranges, void *data, __u64 nsectors)
{
	struct ata_tf tf;
	int err = 0;
	unsigned int data_bytes = nranges * sizeof(__u64);
	unsigned int data_sects = (data_bytes + 511) / 512;

	data_bytes = data_sects * 512;

	abort_if_not_full_device(fd, 0, devname, NULL);
	printf("trimming %llu sectors from %d ranges\n", nsectors, nranges);
	fflush(stdout);

	// Try and ensure that the system doesn't have the to-be-trimmed sectors in cache:
	flush_buffer_cache(fd);

	tf_init(&tf, ATA_OP_DSM, 0, data_sects);
	tf.lob.feat = 0x01;	/* DSM/TRIM */

	if (sg16(fd, SG_WRITE, SG_DMA, &tf, data, data_bytes, 300 /* seconds */)) {
		err = errno;
		perror("FAILED");
	} else {
		printf("succeeded\n");
	}
	return err;
}

static void do_trim_sector_ranges (int fd, const char *devname, int nranges, struct sector_range_s *sr)
{
	__u64 range, *data, nsectors = 0;
	unsigned int data_sects, data_bytes;
	int i, err = 0;

	abort_if_not_full_device(fd, 0, devname, NULL);

	data_sects = ((nranges * sizeof(range)) + 511) / 512;
	data_bytes = data_sects * 512;

	data = mmap(NULL, data_bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (data == MAP_FAILED) {
		err = errno;
		perror("mmap(MAP_ANONYMOUS)");
		exit(err);
	}
	// FIXME: handle counts > 65535 here!
	for (i = 0; i < nranges; ++i) {
		nsectors += sr->nsectors;
		range = sr->nsectors;
		range = (range << 48) | sr->lba;
		data[i] = __cpu_to_le64(range);
		++sr;
	}

	err = trim_sectors(fd, devname, nranges, data, nsectors);
	munmap(data, data_bytes);
	exit(err);
}

static void
extract_id_string (__u16 *idw, int words, char *dst)
{
	char *e;
	int bytes = words * 2;

	memcpy(dst, idw, bytes);
	dst[bytes] = '\0';
	for (e = dst + bytes; --e != dst;) {
		if (*e && *e != ' ')
			break;
		*e = '\0';
	}
}

static int
get_trim_dev_limit (void)
{
	char model[41];

	if (id[105] && id[105] != 0xffff)
		return id[105];
	extract_id_string(id + 27, 20, model);
	if (0 == strcmp(model, "OCZ VERTEX-LE"))
		return 8;
	if (0 == strcmp(model, "OCZ-VERTEX"))
		return 64;
	return 1;  /* all other drives, including Intel SSDs */
}

int get_current_sector_size (int fd)
{
	unsigned int words = 256;

	get_identify_data(fd);
	if(id && (id[106] & 0xc000) == 0x4000) {
		if (id[106] & (1<<12))
			words = (id[118] << 16) | id[117];
	}
	return 2 * words;
}

static int
get_set_sector_index (int fd, unsigned int wanted_sector_size, int *checkword)
{
	__u8 d[512] = {0,};
	const int SECTOR_CONFIG = 0x2f;
	int i, rc;

	rc = get_log_page_data(fd, SECTOR_CONFIG, 0, d);
	if (rc) {
		fprintf(stderr, "READ_LOG_EXT(SECTOR_CONFIGURATION) failed: %s\n", strerror(rc));
		exit(1);
	}
	for (i = 0; i < 128; i += 16) {
		unsigned int lss;
		if ((d[i] & 0x80) == 0)  /* Is this descriptor valid? */
			continue;  /* not valid */
		lss = d[i + 4] | (d[i + 5] << 8) | (d[i + 6] << 16) | (d[i + 7] << 24);  /* logical sector size */
		if (lss == wanted_sector_size) {
			*checkword = d[i + 2] | (d[i + 3] << 8);
			return i / 16;  /* descriptor index */
		}
	}
	fprintf(stderr, "ERROR: unsupported sector size: %d\n", wanted_sector_size);
	exit (-1);
}

static int do_set_sector_size_cmd (int fd, const char *devname)
{
	int index, err = 0;
	__u8 ata_op;
	struct hdio_taskfile *r;
	int checkword = 0;

	abort_if_not_full_device(fd, 0, devname, NULL);
	r = malloc(sizeof(struct hdio_taskfile) + 512);
	if (!r) {
		err = errno;
		perror("malloc()");
		return err;
	}
	ata_op = ATA_OP_SET_SECTOR_CONFIGURATION;
	init_hdio_taskfile(r, ata_op, RW_WRITE, LBA48_FORCE, 0, 0, 0);

	index = get_set_sector_index(fd, new_sector_size, &checkword);
	r->hob.feat  = checkword >> 8;
	r->lob.feat  = checkword;
	r->hob.nsect = 0;
	r->lob.nsect = index;
	r->oflags.bits.hob.feat = 1;

	printf("changing sector size configuration to %llu: ", new_sector_size);
	fflush(stdout);

	if (do_taskfile_cmd(fd, r, timeout_60secs)) {
		err = errno;
		perror("FAILED");
	} else {
		printf("succeeded\n");
	}

	free(r);
	return err;
}

static int
do_trim_from_stdin (int fd, const char *devname)
{
	__u64 *data, range, nsectors = 0, lba_limit;
	unsigned int max_kb, data_sects, data_bytes;
	unsigned int total_ranges = 0, nranges = 0, max_ranges, dev_limit;
	int err = 0;

	get_identify_data(fd);
	if (!id)
		exit(EIO);
	lba_limit = get_lba_capacity(id);
	dev_limit = get_trim_dev_limit();

	err = sysfs_get_attr(fd, "queue/max_sectors_kb", "%u", &max_kb, NULL, 0);
	if (err || max_kb == 0)
		data_sects = 128;	/* "safe" default for most controllers */
	else
		data_sects = max_kb * 2;
	if (data_sects > dev_limit)
		data_sects = dev_limit;
	data_bytes = data_sects * 512;

	data = mmap(NULL, data_bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	if (data == MAP_FAILED) {
		err = errno;
		perror("mmap(MAP_ANONYMOUS)");
		exit(err);
	}
	memset(data, 0, data_bytes);
	max_ranges = data_bytes / sizeof(range);

	do {
		__u64 lba, nsect;
		int args;

		if (nranges == 0)
			memset(data, 0, data_bytes);
		errno = EINVAL;
		args = scanf("%llu:%llu", &lba, &nsect);
		if (args == EOF)
			break;
		if (args != 2 || nsect > 0xffff || lba >= lba_limit) {
			if (args == 2)
				errno = ERANGE;
			err = errno;
			fprintf(stderr, "stdin: error at lba:count pair #%d: %s\n", (total_ranges + 1), strerror(err));
		} else {
			range = (nsect << 48) | lba;
			nsectors += nsect;
			data[nranges++] = __cpu_to_le64(range);
			if (nranges == max_ranges) {
				err = trim_sectors(fd, devname, nranges, data, nsectors);
				memset(data, 0, data_bytes);
				nranges = 0;
				nsectors = 0;
			}
			++total_ranges;
		}
	} while (!err);
	if (!err && nranges)
		err = trim_sectors(fd, devname, nranges, data, nsectors);
	munmap(data, data_bytes);
	return err;
}

static int do_write_sector (int fd, __u64 lba, const char *devname)
{
	int err = 0;
	__u8 ata_op;
	struct hdio_taskfile *r;
	int sector_bytes = get_current_sector_size(fd);

	abort_if_not_full_device(fd, lba, devname, NULL);
	r = malloc(sizeof(struct hdio_taskfile) + sector_bytes);
	if (!r) {
		err = errno;
		perror("malloc()");
		return err;
	}
	ata_op = (lba >= lba28_limit) ? ATA_OP_WRITE_PIO_EXT : ATA_OP_WRITE_PIO;
	init_hdio_taskfile(r, ata_op, RW_WRITE, LBA28_OK, lba, 1, sector_bytes);

	printf("re-writing sector %llu: ", lba);
	fflush(stdout);

	// Try and ensure that the system doesn't have our sector in cache:
	flush_buffer_cache(fd);

	if (do_taskfile_cmd(fd, r, timeout_60secs)) {
		err = errno;
		perror("FAILED");
	} else {
		printf("succeeded\n");
	}

	free(r);
	return err;
}

static int do_read_sector (int fd, __u64 lba, const char *devname)
{
	int err = 0;
	__u8 ata_op;
	struct hdio_taskfile *r;
	int sector_bytes = get_current_sector_size(fd);

	abort_if_not_full_device(fd, lba, devname, NULL);
	r = malloc(sizeof(struct hdio_taskfile) + sector_bytes);
	if (!r) {
		err = errno;
		perror("malloc()");
		return err;
	}
	ata_op = (lba >= lba28_limit) ? ATA_OP_READ_PIO_EXT : ATA_OP_READ_PIO;
	init_hdio_taskfile(r, ata_op, RW_READ, LBA28_OK, lba, 1, sector_bytes);

	printf("reading sector %llu: ", lba);
	fflush(stdout);

	if (do_taskfile_cmd(fd, r, timeout_60secs)) {
		err = errno;
		perror("FAILED");
	} else {
		printf("succeeded\n");
		dump_sectors(r->data, 1, 0, sector_bytes);
	}
	free(r);
	return err;
}

static int do_idleunload (int fd, const char *devname)
{
	int err = 0;
	struct hdio_taskfile r;

	abort_if_not_full_device(fd, 0, devname, NULL);
	init_hdio_taskfile(&r, ATA_OP_IDLEIMMEDIATE, RW_READ, LBA28_OK, 0x0554e4c, 0, 0);
	r.oflags.bits.lob.feat = 1;
	r.lob.feat = 0x44;

	if (do_taskfile_cmd(fd, &r, 0)) {
		err = errno;
		perror("TASKFILE(idle_immediate_unload) failed");
	}
	return err;
}

#if !defined (HDPARM_MINI)
static int do_set_max_sectors (int fd, __u64 max_lba, int permanent)
{
	int err = 0;
	struct hdio_taskfile r;
	__u8 nsect = permanent ? 1 : 0;
	
	get_identify_data(fd);
	if (!id)
		exit(EIO);
	
	if (SUPPORTS_AMAX_ADDR(id)) {
		/* ACS3 supported, no 28-bit variant defined in spec */
		init_hdio_taskfile(&r, ATA_OP_GET_NATIVE_MAX_EXT, RW_READ, LBA48_FORCE, max_lba, nsect, 0);
		r.oflags.bits.lob.feat = 1;
		r.lob.feat = 0x01; //SET ACCESSIBLE MAX ADDRESS EXT is 78h/0001h
		//bit 6 of DEVICE field is defined as "N/A"

		/* No more "racey" in ACS3+AMAX case? */
		if (do_taskfile_cmd(fd, &r, timeout_15secs)) {
			err = errno;
			perror(" SET_ACCESSIBLE_MAX_ADDRESS_EXT failed");
		}
	} else {
		if ((max_lba >= lba28_limit) || SUPPORTS_48BIT_ADDR(id)) {
			init_hdio_taskfile(&r, ATA_OP_SET_MAX_EXT, RW_READ, LBA48_FORCE, max_lba, nsect, 0);
			r.oflags.bits.lob.dev = 1;
			r.lob.dev = 0x40;
		} else {
			init_hdio_taskfile(&r, ATA_OP_SET_MAX, RW_READ, LBA28_OK, max_lba, nsect, 0);
			//bit 7:5 of DEVICE field is defined as "Obsolete"
		}

		/* spec requires that we do this immediately in front.. racey */
		if (!do_get_native_max_sectors(fd))
			return errno;

		/* now set the new value */
		if (do_taskfile_cmd(fd, &r, 0)) {
			err = errno;
			perror(" SET_MAX_ADDRESS(_EXT) failed");
		}
	}

	return err;
}
#endif

static void usage_help (int clue, int rc)
{
	FILE *desc = rc ? stderr : stdout;

	fprintf(desc,"\n%s - get/set hard disk parameters - version " HDPARM_VERSION ", by Mark Lord.\n\n", progname);
	if (1) if (rc) fprintf(desc, "clue=%d\n", clue);
	fprintf(desc,"Usage:  %s  [options] [device ...]\n\n", progname);
	fprintf(desc,"Options:\n"
	" -a   Get/set fs readahead\n"
	" -A   Get/set the drive look-ahead flag (0/1)\n"
	" -b   Get/set bus state (0 == off, 1 == on, 2 == tristate)\n"
	" -B   Set Advanced Power Management setting (1-255)\n"
	" -c   Get/set IDE 32-bit IO setting\n"
	" -C   Check drive power mode status\n"
	" -d   Get/set using_dma flag\n"
	" -D   Enable/disable drive defect management\n"
#if !defined (HDPARM_MINI)
	" -E   Set cd/dvd drive speed\n"
#endif
	" -f   Flush buffer cache for device on exit\n"
	" -F   Flush drive write cache\n"
	" -g   Display drive geometry\n"
	" -h   Display terse usage information\n"
	" -H   Read temperature from drive (Hitachi only)\n"
	" -i   Display drive identification\n"
	" -I   Detailed/current information directly from drive\n"
	" -J   Get/set Western DIgital \"Idle3\" timeout for a WDC \"Green\" drive (DANGEROUS)\n"
	" -k   Get/set keep_settings_over_reset flag (0/1)\n"
	" -K   Set drive keep_features_over_reset flag (0/1)\n"
	" -L   Set drive doorlock (0/1) (removable harddisks only)\n"
	" -m   Get/set multiple sector count\n"
	" -M   Get/set acoustic management (0-254, 128: quiet, 254: fast)\n"
	" -n   Get/set ignore-write-errors flag (0/1)\n"
#if !defined (HDPARM_MINI)
	" -N   Get/set max visible number of sectors (HPA) (VERY DANGEROUS)\n"
	" -p   Set PIO mode on IDE interface chipset (0,1,2,3,4,...)\n"
#endif
	" -P   Set drive prefetch count\n"
	" -q   Change next setting quietly\n"
	" -Q   Get/set DMA queue_depth (if supported)\n"
	" -r   Get/set device readonly flag (DANGEROUS to set)\n"
	" -R   Get/set device write-read-verify flag\n"
	" -s   Set power-up in standby flag (0/1) (DANGEROUS)\n"
	" -S   Set standby (spindown) timeout\n"
	" -t   Perform device read timings\n"
	" -T   Perform cache read timings\n"
	" -u   Get/set unmaskirq flag (0/1)\n"
	" -U   Obsolete\n"
	" -v   Use defaults; same as -acdgkmur for IDE drives\n"
	" -V   Display program version and exit immediately\n"
	" -w   Perform device reset (DANGEROUS)\n"
	" -W   Get/set drive write-caching flag (0/1)\n"
	" -x   Obsolete\n"
#if !defined (HDPARM_MINI)
	" -X   Set IDE xfer mode (DANGEROUS)\n"
#endif
	" -y   Put drive in standby mode\n"
	" -Y   Put drive to sleep\n"
	" -z   Re-read partition table\n"
	" -Z   Disable Seagate auto-powersaving mode\n"
#if !defined (HDPARM_MINI)
	" --dco-freeze      Freeze/lock current device configuration until next power cycle\n"
	" --dco-identify    Read/dump device configuration identify data\n"
	" --dco-restore     Reset device configuration back to factory defaults\n"
	" --dco-setmax      Use DCO to set maximum addressable sectors\n"
	" --direct          Use O_DIRECT to bypass page cache for timings\n"
	" --drq-hsm-error   Crash system with a \"stuck DRQ\" error (VERY DANGEROUS)\n"
	" --fallocate       Create a file without writing data to disk\n"
	" --fibmap          Show device extents (and fragmentation) for a file\n"
	" --fwdownload            Download firmware file to drive (EXTREMELY DANGEROUS)\n"
	" --fwdownload-mode3      Download firmware using min-size segments (EXTREMELY DANGEROUS)\n"
	" --fwdownload-mode3-max  Download firmware using max-size segments (EXTREMELY DANGEROUS)\n"
	" --fwdownload-mode7      Download firmware using a single segment (EXTREMELY DANGEROUS)\n"
	" --fwdownload-modee      Download firmware using mode E (min-size segments) (EXTREMELY DANGEROUS)\n"
	" --fwdownload-modee-max  Download firmware using mode E (max-size segments) (EXTREMELY DANGEROUS)\n"
	" --idle-immediate  Idle drive immediately\n"
	" --idle-unload     Idle immediately and unload heads\n"
  " --Iraw filename   Write raw binary identify data to the specfied file\n"
	" --Istdin          Read identify data from stdin as ASCII hex\n"
	" --Istdout         Write identify data to stdout as ASCII hex\n"
	" --make-bad-sector Deliberately corrupt a sector directly on the media (VERY DANGEROUS)\n"
	" --offset          use with -t, to begin timings at given offset (in GiB) from start of drive\n"
	" --prefer-ata12    Use 12-byte (instead of 16-byte) SAT commands when possible\n"
	" --read-sector     Read and dump (in hex) a sector directly from the media\n"
	" --repair-sector   Alias for the --write-sector option (VERY DANGEROUS)\n"
	" --sanitize-antifreeze-lock  Block sanitize-freeze-lock command until next power cycle\n"
	" --sanitize-block-erase      Start block erase operation\n"
	" --sanitize-crypto-scramble  Change the internal encryption keys that used for used data\n"
	" --sanitize-freeze-lock      Lock drive's sanitize features until next power cycle\n"
	" --sanitize-overwrite  PATTERN  Overwrite the internal media with constant PATTERN\n"
	" --sanitize-status           Show sanitize status information\n"
	" --security-help             Display help for ATA security commands\n"
	" --set-sector-size           Change logical sector size of drive\n"
	" --trim-sector-ranges        Tell SSD firmware to discard unneeded data sectors: lba:count ..\n"
	" --trim-sector-ranges-stdin  Same as above, but reads lba:count pairs from stdin\n"
	" --verbose                   Display extra diagnostics from some commands\n"
	" --write-sector              Repair/overwrite a (possibly bad) sector directly on the media (VERY DANGEROUS)\n"
#endif
	"\n");
	exit(rc);
}

#if !defined (HDPARM_MINI)
static void security_help (int rc)
{
	FILE *desc = rc ? stderr : stdout;

	fprintf(desc, "\n"
	"ATA Security Commands:\n"
	" Most of these are VERY DANGEROUS and can destroy all of your data!\n"
	" Due to bugs in older Linux kernels, use of these commands may even\n"
	" trigger kernel segfaults or worse.  EXPERIMENT AT YOUR OWN RISK!\n"
	"\n"
	" --security-freeze           Freeze security settings until reset.\n"
	"\n"
	" --security-set-pass PASSWD  Lock drive, using password PASSWD:\n"
	"                                  Use 'NULL' to set empty password.\n"
	"                                  Drive gets locked if user-passwd is selected.\n"
	" --security-prompt-for-password   Prompt user to enter the drive password.\n"
	"\n"
	" --security-unlock   PASSWD  Unlock drive.\n"
	" --security-disable  PASSWD  Disable drive locking.\n"
	" --security-erase    PASSWD  Erase a (locked) drive.\n"
	" --security-erase-enhanced PASSWD   Enhanced-erase a (locked) drive.\n"
	"\n"
	" The above four commands may optionally be preceded by these options:\n"
	" --security-mode  LEVEL      Use LEVEL to select security level:\n"
	"                                  h   high security (default).\n"
	"                                  m   maximum security.\n"
	" --user-master    WHICH      Use WHICH to choose password type:\n"
	"                                  u   user-password (default).\n"
	"                                  m   master-password\n"
	);
	exit(rc);
}
#endif

void process_dev (char *devname)
{
	int fd;
	int err = 0;
	static long parm, multcount;

	id = NULL;
	fd = open(devname, open_flags);
	if (fd < 0) {
		err = errno;
		perror(devname);
		exit(err);
	}
	if (!quiet)
		printf("\n%s:\n", devname);

	if (apt_detect(fd, verbose) == -1) {
		err = errno;
		perror(devname);
		close(fd);
		exit(err);
	}

	if (do_set_sector_size) {
		if (num_flags_processed > 1 || argc)
			usage_help(16,EINVAL);
		confirm_please_destroy_my_drive("--" SET_SECTOR_SIZE, "This will likely destroy all data on the drive.");
		exit(do_set_sector_size_cmd(fd, devname));
	}

	if (trim_from_stdin) {
		if (num_flags_processed > 1 || argc)
			usage_help(12,EINVAL);
		confirm_please_destroy_my_drive("--trim-sector-ranges-stdin", "This might destroy the drive and/or all data on it.");
		exit(do_trim_from_stdin(fd, devname));
	}

	if (set_wdidle3) {
		unsigned char timeout = wdidle3_msecs_to_timeout(wdidle3);
		confirm_please_destroy_my_drive("-J", "This implementation is not as thorough as the official WDIDLE3.EXE. Use at your own risk!");
		if (get_wdidle3) {
			printf(" setting wdidle3 to ");
			wdidle3_print_timeout(timeout);
			putchar('\n');
		}
		err = wdidle3_set_timeout(fd, timeout);
	}
	if (set_fsreadahead) {
		if (get_fsreadahead)
			printf(" setting fs readahead to %d\n", fsreadahead);
		if (ioctl(fd, BLKRASET, fsreadahead)) {
			err = errno;
			perror(" BLKRASET failed");
		}
	}
	if (set_piomode) {
		if (get_piomode) {
			if (piomode == 255)
				printf(" attempting to auto-tune PIO mode\n");
			else if (piomode < 100)
				printf(" attempting to set PIO mode to %d\n", piomode);
			else if (piomode < 200)
				printf(" attempting to set MDMA mode to %d\n", (piomode-100));
			else
				printf(" attempting to set UDMA mode to %d\n", (piomode-200));
		}
		if (ioctl(fd, HDIO_SET_PIO_MODE, piomode)) {
			err = errno;
			perror(" HDIO_SET_PIO_MODE failed");
		}
	}
	if (set_io32bit) {
		if (get_io32bit)
			printf(" setting 32-bit IO_support flag to %d\n", io32bit);
		if (ioctl(fd, HDIO_SET_32BIT, io32bit)) {
			err = errno;
			perror(" HDIO_SET_32BIT failed");
		}
	}
	if (set_mult) {
		if (get_mult)
			printf(" setting multcount to %d\n", mult);
		if (ioctl(fd, HDIO_SET_MULTCOUNT, mult))
#if 0
			perror(" HDIO_SET_MULTCOUNT failed");
#else /* for libata */
		{
			if (errno != ENOTTY) {
				perror(" HDIO_SET_MULTCOUNT failed");
			} else {
				__u8 args[4] = {ATA_OP_SET_MULTIPLE,mult,0,0};
				confirm_i_know_what_i_am_doing("-m", "Only the old IDE drivers work correctly with -m with kernels up to at least 2.6.29.\nlibata drives may fail and get hung if you set this flag.");
				if (do_drive_cmd(fd, args, 0)) {
					err = errno;
					perror(" HDIO_DRIVE_CMD(set_multi_count) failed");
				}
			}
		}
#endif
	}
	if (set_readonly) {
		if (get_readonly) {
			printf(" setting readonly to %d", readonly);
			on_off(readonly);
		}
		if (ioctl(fd, BLKROSET, &readonly)) {
			err = errno;
			perror(" BLKROSET failed");
		}
	}
	if (set_unmask) {
		if (get_unmask) {
			printf(" setting unmaskirq to %d", unmask);
			on_off(unmask);
		}
		if (ioctl(fd, HDIO_SET_UNMASKINTR, unmask)) {
			err = errno;
			perror(" HDIO_SET_UNMASKINTR failed");
		}
	}
	if (set_dma) {
		if (get_dma) {
			printf(" setting using_dma to %d", dma);
			on_off(dma);
		}
		if (ioctl(fd, HDIO_SET_DMA, dma)) {
			err = errno;
			perror(" HDIO_SET_DMA failed");
		}
	}
	if (set_dma_q) {
		if (get_dma_q)
			printf(" setting queue_depth to %d\n", dma_q);
		err = sysfs_set_attr(fd, "device/queue_depth", "%u", &dma_q, 1);
	}
	if (set_nowerr) {
		if (get_nowerr) {
			printf(" setting nowerr to %d", nowerr);
			on_off(nowerr);
		}
		if (ioctl(fd, HDIO_SET_NOWERR, nowerr)) {
			err = errno;
			perror(" HDIO_SET_NOWERR failed");
		}
	}
	if (set_keep) {
		if (get_keep) {
			printf(" setting keep_settings to %d", keep);
			on_off(keep);
		}
		if (ioctl(fd, HDIO_SET_KEEPSETTINGS, keep)) {
			err = errno;
			perror(" HDIO_SET_KEEPSETTINGS failed");
		}
	}
	if (set_doorlock) {
		__u8 args[4] = {0,0,0,0};
		args[0] = doorlock ? ATA_OP_DOORLOCK : ATA_OP_DOORUNLOCK;
		if (get_doorlock) {
			printf(" setting drive doorlock to %d", doorlock);
			on_off(doorlock);
		}
		if (do_drive_cmd(fd, args, timeout_15secs)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(doorlock) failed");
		}
	}
	if (set_dkeep) {
		/* lock/unlock the drive's "feature" settings */
		__u8 args[4] = {ATA_OP_SETFEATURES,0,0,0};
		if (get_dkeep) {
			printf(" setting drive keep features to %d", dkeep);
			on_off(dkeep);
		}
		args[2] = dkeep ? 0x66 : 0xcc;
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(keepsettings) failed");
		}
	}
	if (set_defects) {
		__u8 args[4] = {ATA_OP_SETFEATURES,0,0,0};
		args[2] = defects ? 0x04 : 0x84;
		if (get_defects)
			printf(" setting drive defect management to %d\n", defects);
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(defectmgmt) failed");
		}
	}
	if (set_prefetch) {
		__u8 args[4] = {ATA_OP_SETFEATURES,0,0xab,0};
		args[1] = prefetch;
		if (get_prefetch)
			printf(" setting drive prefetch to %d\n", prefetch);
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(setprefetch) failed");
		}
	}
#if !defined (HDPARM_MINI)
	if (set_xfermode) {
		__u8 args[4] = {ATA_OP_SETFEATURES,0,3,0};
		args[1] = xfermode_requested;
		if (get_xfermode) {
			printf(" setting xfermode to %d", xfermode_requested);
			interpret_xfermode(xfermode_requested);
		}
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(setxfermode) failed");
		}
	}
#endif
	if (set_lookahead) {
		__u8 args[4] = {ATA_OP_SETFEATURES,0,0,0};
		args[2] = lookahead ? 0xaa : 0x55;
		if (get_lookahead) {
			printf(" setting drive read-lookahead to %d", lookahead);
			on_off(lookahead);
		}
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(setreadahead) failed");
		}
	}
	if (set_powerup_in_standby) {
		__u8 args[4] = {ATA_OP_SETFEATURES,0,0,0};
		if (powerup_in_standby == 0) {
			__u8 args1[4] = {ATA_OP_SETFEATURES,0,0x07,0}; /* spinup from standby */
			printf(" spin-up:");
			fflush(stdout);
			(void) do_drive_cmd(fd, args1, 0);
		} else {
			confirm_i_know_what_i_am_doing("-s1",
				"This requires BIOS and kernel support to recognize/boot the drive.");
		}
		if (get_powerup_in_standby) {
			printf(" setting power-up in standby to %d", powerup_in_standby);
			fflush(stdout);
			on_off(powerup_in_standby);
		}
		args[0] = ATA_OP_SETFEATURES;
		args[2] = powerup_in_standby ? 0x06 : 0x86;
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(powerup_in_standby) failed");
		}
	}
	if (set_apmmode) {
		__u8 args[4] = {ATA_OP_SETFEATURES,0,0,0};
		if (get_apmmode)
			printf(" setting Advanced Power Management level to");
		if (apmmode==255) {
			/* disable Advanced Power Management */
			args[2] = 0x85; /* feature register */
			if (get_apmmode) printf(" disabled\n");
		} else {
			/* set Advanced Power Management mode */
			args[2] = 0x05; /* feature register */
			args[1] = apmmode; /* sector count register */
			if (get_apmmode)
				printf(" 0x%02x (%d)\n",apmmode,apmmode);
		}
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD failed");
		}
	}
#if !defined (HDPARM_MINI)
	if (set_cdromspeed) {
		int err1, err2;
		/* The CDROM_SELECT_SPEED ioctl
		 * actually issues GPCMD_SET_SPEED to the drive.
		 * But many newer DVD drives want GPCMD_SET_STREAMING instead,
		 * which we now do afterwards.
		 */
		if (get_cdromspeed)
			printf ("setting cd/dvd speed to %d\n", cdromspeed);
		err1 = set_dvdspeed(fd, cdromspeed);
		err2 = ioctl(fd, CDROM_SELECT_SPEED, cdromspeed);
		if (err1 && err2) {
			err = errno;
			perror(" SET_STREAMING/CDROM_SELECT_SPEED both failed");
		}
	}
#endif
	if (set_acoustic) {
		__u8 args[4];
		if (get_acoustic)
			printf(" setting acoustic management to %d\n", acoustic);
		args[0] = ATA_OP_SETFEATURES;
		args[1] = acoustic;
		args[2] = acoustic ? 0x42 : 0xc2;
		args[3] = 0;
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD:ACOUSTIC failed");
		}
	}
	if (set_write_read_verify) {
		__u8 args[4];
		if (get_write_read_verify)
			printf(" setting write-read-verify to %d\n", write_read_verify);
		args[0] = ATA_OP_SETFEATURES;
		args[1] = write_read_verify;
		args[2] = write_read_verify ? 0x0b : 0x8b;
		args[3] = 0;
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD:WRV failed");
		}
	}
	if (set_wcache) {
		if (get_wcache) {
			printf(" setting drive write-caching to %d", wcache);
			on_off(wcache);
		}
		if (!wcache)
			err = flush_wcache(fd);
		if (ioctl(fd, HDIO_SET_WCACHE, wcache)) {
			__u8 setcache[4] = {ATA_OP_SETFEATURES,0,0,0};
			setcache[2] = wcache ? 0x02 : 0x82;
			if (do_drive_cmd(fd, setcache, 0)) {
				err = errno;
				perror(" HDIO_DRIVE_CMD(setcache) failed");
			}
		}
		if (!wcache)
			err = flush_wcache(fd);
	}
	if (set_standby) {
		__u8 args[4] = {ATA_OP_SETIDLE,standby,0,0};
		if (get_standby) {
			printf(" setting standby to %u", standby);
			interpret_standby();
		}
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(setidle) failed");
		}
	}
	if (set_security) {
		do_set_security(fd);
	}
	if (do_sanitize) {
		if (do_sanitize > 1) {
			confirm_i_know_what_i_am_doing("--sanitize", "This sanitize command destroys all user data.");
		}
		do_sanitize_cmd(fd);
	}
#if !defined (HDPARM_MINI)
	if (do_dco_identify) {
		__u16 *dco = get_dco_identify_data(fd, 0);
		if (dco) {
			if (dco_verify_checksum(dco) == (dco[255] >> 8))
				printf("DCO Checksum verified.\n");
			else
				printf("DCO Checksum FAILED!\n");
			dco_identify_print(dco);
		}
	}
	if (do_dco_restore) {
		__u8 args[4] = {ATA_OP_DCO,0,0xc0,0};
		confirm_i_know_what_i_am_doing("--dco-restore", "You are trying to deliberately reset your drive configuration back to the factory defaults.\nThis may change the apparent capacity and feature set of the drive, making all data on it inaccessible.\nYou could lose *everything*.");
		if (!quiet)
			printf(" issuing DCO restore command\n");
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(dco_restore) failed");
		}
	}
	if (do_dco_freeze) {
		__u8 args[4] = {ATA_OP_DCO,0,0xc1,0};
		if (!quiet)
			printf(" issuing DCO freeze command\n");
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(dco_freeze) failed");
		}
	}
	if (do_dco_setmax) {
		get_identify_data(fd);
		if (id) {
			if (set_max_addr < get_lba_capacity(id))
				confirm_i_know_what_i_am_doing("--dco-setmax", "You have requested reducing the apparent size of the drive.\nThis is a BAD idea, and can easily destroy all of the drive's contents.");

			// set max sectors with DCO set command
			if (!quiet)
				printf("issuing DCO set command (sectors = %llu)\n", set_max_addr);
			do_dco_setmax_cmd(fd);

			// invalidate current IDENTIFY data
			id = NULL; 
		}
	}
#endif
	if (security_freeze) {
		__u8 args[4] = {ATA_OP_SECURITY_FREEZE_LOCK,0,0,0};
		if (!quiet)
			printf(" issuing security freeze command\n");
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(security_freeze) failed");
		}
	}
	if (set_seagate) {
		__u8 args[4] = {0xfb,0,0,0};
		if (!quiet && get_seagate)
			printf(" disabling Seagate auto powersaving mode\n");
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(seagatepwrsave) failed");
		}
	}
	if (set_busstate) {
		if (!quiet && get_busstate)
			printf(" setting bus state to %d (%s)\n", busstate, busstate_str(busstate));
		if (ioctl(fd, HDIO_SET_BUSSTATE, busstate)) {
			err = errno;
			perror(" HDIO_SET_BUSSTATE failed");
		}
	}
#if !defined (HDPARM_MINI)
	if (set_max_sectors) {
		if (!quiet && get_native_max_sectors)
			printf(" setting max visible sectors to %llu (%s)\n", set_max_addr, set_max_permanent ? "permanent" : "temporary");
		get_identify_data(fd);
		if (id) {
			if (set_max_addr < get_lba_capacity(id))
				confirm_i_know_what_i_am_doing("-Nnnnnn", "You have requested reducing the apparent size of the drive.\nThis is a BAD idea, and can easily destroy all of the drive's contents.");
			err = do_set_max_sectors(fd, set_max_addr - 1, set_max_permanent);
			id = NULL; /* invalidate existing identify data */
		}
	}
#endif
	if (make_bad_sector) {
		get_identify_data(fd);
		if (id) {
			confirm_i_know_what_i_am_doing("--make-bad-sector", "You are trying to deliberately corrupt a low-level sector on the media.\nThis is a BAD idea, and can easily result in total data loss.");
			err = do_make_bad_sector(fd, make_bad_sector_addr, devname);
		}
	}
#ifdef FORMAT_AND_ERASE
	if (format_track) {
		confirm_i_know_what_i_am_doing("--format-track", "This flag is still under development and probably does not work correctly yet.\nYou are trying to deliberately destroy your device.\nThis is a BAD idea, and can easily result in total data loss.");
		confirm_please_destroy_my_drive("--format-track", "This might destroy the drive and/or all data on it.");
		err = do_format_track(fd, format_track_addr, devname);
	}
	if (erase_sectors) {
		confirm_i_know_what_i_am_doing("--erase-sectors", "This flag is still under development and probably does not work correctly yet.\nYou are trying to deliberately destroy your device.\nThis is a BAD idea, and can easily result in total data loss.");
		confirm_please_destroy_my_drive("--erase-sectors", "This might destroy the drive and/or all data on it.");
		err = do_erase_sectors(fd, erase_sectors_addr, devname);
	}
#endif /* FORMAT_AND_ERASE */
	if (trim_sector_ranges_count) {
		if (num_flags_processed > 1 || argc)
			usage_help(13,EINVAL);
		confirm_please_destroy_my_drive("--trim-sector-ranges", "This might destroy the drive and/or all data on it.");
		do_trim_sector_ranges(fd, devname, trim_sector_ranges_count, trim_sector_ranges);
	}
	if (write_sector) {
		if (num_flags_processed > 1 || argc)
			usage_help(14,EINVAL);
		confirm_i_know_what_i_am_doing("--write-sector", "You are trying to deliberately overwrite a low-level sector on the media.\nThis is a BAD idea, and can easily result in total data loss.");
		err = do_write_sector(fd, write_sector_addr, devname);
	}
#if !defined (HDPARM_MINI)
	if (do_fwdownload) {
		if (num_flags_processed > 1 || argc)
			usage_help(15,EINVAL);
		abort_if_not_full_device (fd, 0, devname, "--fwdownload requires the raw device, not a partition.");
		confirm_i_know_what_i_am_doing("--fwdownload", "This flag has not been tested with many drives to date.\nYou are trying to deliberately overwrite the drive firmware with the contents of the specified file.\nIf this fails, your drive could be toast.");
		confirm_please_destroy_my_drive("--fwdownload", "This might destroy the drive and well as all of the data on it.");
		get_identify_data(fd);
		if (id) {
			err = fwdownload(fd, id, fwpath, xfer_mode);
			if (err)
				exit(err);
		}
	}
#endif
	if (read_sector)
		err = do_read_sector(fd, read_sector_addr, devname);
#if !defined (HDPARM_MINI)
	if (drq_hsm_error) {
		get_identify_data(fd);
		if (id) {
			__u8 args[4] = {0,0,0,0};
			args[0] = last_identify_op;
			printf(" triggering \"stuck DRQ\" host state machine error\n");
			flush_buffer_cache(fd);
			sleep(1);
			do_drive_cmd(fd, args, timeout_60secs);
			err = errno;
			perror("drq_hsm_error");
			fprintf(stderr, "ata status=0x%02x ata error=0x%02x\n", args[0], args[1]);
		}
	}
#endif
	id = NULL; /* force re-IDENTIFY in case something above modified settings */
	if (get_hitachi_temp) {
		__u8 args[4] = {0xf0,0,0x01,0}; /* "Sense Condition", vendor-specific */
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(hitachisensecondition) failed");
		} else {
			printf(" drive temperature (celsius) is:  ");
			if (args[2]==0)
				printf("under -20");
			else if (args[2]==0xFF)
				printf("over 107");
			else
				printf("%d", args[2]/2-20);
			printf("\n drive temperature in range:  %s\n", YN(!(args[1]&0x10)) );
		}
	}
	if (do_defaults || get_mult || do_identity) {
		multcount = -1;
		err = 0;
		if (ioctl(fd, HDIO_GET_MULTCOUNT, &multcount)) {
			err = errno;
			get_identify_data(fd);
			if (id) {
				err = 0;
				if ((id[59] & 0xff00) == 0x100)
					multcount = id[59] & 0xff;
				else
					multcount = 0;
			}
			if (err && get_mult) {
				errno = err;
				perror(" HDIO_GET_MULTCOUNT failed");
			}
		}
		if (!err && (do_defaults || get_mult)) {
			printf(" multcount     = %2ld", multcount);
			on_off(multcount);
		}
	}
	if (do_defaults || get_io32bit) {
		if (0 == ioctl(fd, HDIO_GET_32BIT, &parm)) {
			printf(" IO_support    =%3ld (", parm);
			switch (parm) {
				case 0:	printf("default) \n");
					break;
				case 2: printf("16-bit)\n");
					break;
				case 1:	printf("32-bit)\n");
					break;
				case 3:	printf("32-bit w/sync)\n");
					break;
				case 8:	printf("Request-Queue-Bypass)\n");
					break;
				default:printf("\?\?\?)\n");
			}
               } else if (get_io32bit) {
                       err = errno;
                       perror(" HDIO_GET_32BIT failed");
		}
	}
	if (do_defaults || get_unmask) {
		if (0 == ioctl(fd, HDIO_GET_UNMASKINTR, &parm)) {
			printf(" unmaskirq     = %2ld", parm);
			on_off(parm);
               } else if (get_unmask) {
                       err = errno;
                       perror(" HDIO_GET_UNMASKINTR failed");
		}
	}

	if (do_defaults || get_dma) {
		if (0 == ioctl(fd, HDIO_GET_DMA, &parm)) {
			printf(" using_dma     = %2ld", parm);
			if (parm == 8)
				printf(" (DMA-Assisted-PIO)\n");
			else
				on_off(parm);
                } else if (get_dma) {
                       err = errno;
                       perror(" HDIO_GET_DMA failed");
		}
	}
	if (get_dma_q) {
		err = sysfs_get_attr(fd, "device/queue_depth", "%u", &dma_q, NULL, 1);
		if (!err)
			printf(" queue_depth   = %2u\n", dma_q);
	}
	if (do_defaults || get_keep) {
		if (0 == ioctl(fd, HDIO_GET_KEEPSETTINGS, &parm)) {
			printf(" keepsettings  = %2ld", parm);
			on_off(parm);
		} else if (get_keep) {
			err = errno;
                        perror(" HDIO_GET_KEEPSETTINGS failed");
		}
	}
	if (get_nowerr) {
		if (ioctl(fd, HDIO_GET_NOWERR, &parm)) {
			err = errno;
			perror(" HDIO_GET_NOWERR failed");
		} else {
			printf(" nowerr        = %2ld", parm);
			on_off(parm);
		}
	}
	if (do_defaults || get_readonly) {
		if (ioctl(fd, BLKROGET, &parm)) {
			err = errno;
			perror(" BLKROGET failed");
		} else {
			printf(" readonly      = %2ld", parm);
			on_off(parm);
		}
	}
	if (do_defaults || get_fsreadahead) {
		if (ioctl(fd, BLKRAGET, &parm)) {
			err = errno;
			perror(" BLKRAGET failed");
		} else {
			printf(" readahead     = %2ld", parm);
			on_off(parm);
		}
	}
	if (do_defaults || get_geom) {
		__u32 cyls = 0, heads = 0, sects = 0;
		__u64 start_lba = 0, nsectors = 0;
		err = get_dev_geometry (fd, &cyls, &heads, &sects, &start_lba, &nsectors);
		if (!err) {
			printf(" geometry      = %u/%u/%u, sectors = %lld, start = ", cyls, heads, sects, nsectors);
			if (start_lba == START_LBA_UNKNOWN)
				printf("unknown\n");
			else
				printf("%lld\n", start_lba);
		}
	}
	if (get_wdidle3) {
		unsigned char timeout = 0;
		err = wdidle3_get_timeout(fd, &timeout);
		if (!err) {
			printf(" wdidle3      = ");
			wdidle3_print_timeout(timeout);
			putchar('\n');
		}
	}
	if (get_powermode) {
		__u8 args[4] = {ATA_OP_CHECKPOWERMODE1,0,0,0};
		const char *state = "unknown";
		if (do_drive_cmd(fd, args, 0)
		 && (args[0] = ATA_OP_CHECKPOWERMODE2) /* (single =) try again with 0x98 */
		 && do_drive_cmd(fd, args, 0)) {
			err = errno;
		} else {
			switch (args[2]) {
				case 0x00: state = "standby";		break;
				case 0x40: state = "NVcache_spindown";	break;
				case 0x41: state = "NVcache_spinup";	break;
				case 0x80: state = "idle";		break;
				case 0xff: state = "active/idle";	break;
			}
		}
		printf(" drive state is:  %s\n", state);
	}
	if (do_identity) {
		__u16 id2[256];

		if (!ioctl(fd, HDIO_GET_IDENTITY, id2)) {
			if (multcount != -1) {
				id2[59] = multcount | 0x100;
			} else {
				id2[59] &= ~0x100;
			}
			dump_identity(id2);
		} else if (errno == -ENOMSG) {
			printf(" no identification info available\n");
		} else {
			err = errno;
			perror(" HDIO_GET_IDENTITY failed");
		}
	}
	if (do_IDentity) {
		get_identify_data(fd);
		if (id) {
			if (do_IDentity == 2) {
				dump_sectors(id, 1, 1, 512);
			} else if (do_IDentity == 3) {
				/* Write raw binary IDENTIFY DEVICE data to the specified file */
				int rfd = open(raw_identify_path, O_WRONLY|O_TRUNC|O_CREAT, 0644);
				if (rfd == -1) {
					err = errno;
					perror(raw_identify_path);
					exit(err);
				}
				err = write(rfd, id, 0x200);
				if (err == -1) {
					err = errno;
					perror(raw_identify_path);
					exit(err);
				} else if (err != 0x200) {
					fprintf(stderr, "Error writing IDENTIFY DEVICE data to \"%s\"\n", raw_identify_path);
					exit(EIO);
				} else {
					fprintf(stderr, "Wrote IDENTIFY DEVICE data to \"%s\"\n", raw_identify_path);
					close(rfd);
				}
      			} else {
				identify(fd, (void *)id);
			}
		}
	}
	if (get_lookahead) {
		get_identify_data(fd);
		if (id) {
			int supported = id[82] & 0x0040;
			if (supported) {
				lookahead = !!(id[85] & 0x0040);
				printf(" look-ahead    = %2d", lookahead);
				on_off(lookahead);
			} else {
				printf(" look-ahead    = not supported\n");
			}
		}
	}
	if (get_wcache) {
		get_identify_data(fd);
		if (id) {
			int supported = id[82] & 0x0020;
			if (supported) {
				wcache = !!(id[85] & 0x0020);
				printf(" write-caching = %2d", wcache);
				on_off(wcache);
			} else {
				printf(" write-caching = not supported\n");
			}
		}
	}
	if (get_apmmode) {
		get_identify_data(fd);
		if (id) {
			printf(" APM_level	= ");
			if ((id[83] & 0xc008) == 0x4008) {
				if (id[86] & 0x0008)
					printf("%u\n", id[91] & 0xff);
				else
					printf("off\n");
			} else
				printf("not supported\n");
		}
	}
	if (get_acoustic) {
		get_identify_data(fd);
		if (id) {
			int supported = id[83] & 0x200;
			if (supported) 
				printf(" acoustic      = %2u (128=quiet ... 254=fast)\n", id[94] & 0xff);
			else
				printf(" acoustic      = not supported\n");
		}
	}
	if (get_write_read_verify) {
		get_identify_data(fd);
		if (id) {
				int supported = id[119] & 0x2;
				if (supported)
					printf(" write-read-verify = %2u\n", id[120] & 0x2);
				else
					printf(" write-read-verify = not supported\n");
		}
	}
	if (get_busstate) {
		if (ioctl(fd, HDIO_GET_BUSSTATE, &parm)) {
			err = errno;
			perror(" HDIO_GET_BUSSTATE failed");
		} else {
			printf(" busstate      = %2ld (%s)\n", parm, busstate_str(parm));
		}
	}
#if !defined (HDPARM_MINI)
	if (get_native_max_sectors) {
		get_identify_data(fd);
		if (id) {
			__u64 visible = get_lba_capacity(id);
			__u64 native  = do_get_native_max_sectors(fd);
			if (!native) {
				err = errno;
			} else {
				printf(" max sectors   = %llu/%llu", visible, native);
				if (visible < native){
					if (SUPPORTS_AMAX_ADDR(id)) {
						printf(", ACCESSIBLE MAX ADDRESS enabled\n");
						printf("Power cycle your device after every ACCESSIBLE MAX ADDRESS\n");
					}
					else
						printf(", HPA is enabled\n");
				}
				else if (visible == native){
					if (SUPPORTS_AMAX_ADDR(id))
						printf(", ACCESSIBLE MAX ADDRESS disabled\n");
					else
						printf(", HPA is disabled\n");
				}
				else {
					__u16 *dco = get_dco_identify_data(fd, 1);
					if (dco) {
						__u64 dco_max = dco[5];
						dco_max = ((((__u64)dco[5]) << 32) | (dco[4] << 16) | dco[3]) + 1;
						printf("(%llu?)", dco_max);
					}
					printf(", HPA setting seems invalid");
					if ((native & 0xffffff000000ull) == 0)
						printf(" (buggy kernel device driver?)");
					putchar('\n');
					}
				}
			}
		
	}	
#endif

	if (do_ctimings)
		time_cache(fd);
	if (do_flush_wcache)
		err = flush_wcache(fd);
	if (do_timings)
		err = time_device(fd);
	if (do_flush)
		flush_buffer_cache(fd);
	if (set_reread_partn) {
		if (get_reread_partn)
			printf(" re-reading partition table\n");
		if (ioctl(fd, BLKRRPART, NULL)) {
			err = errno;
			perror(" BLKRRPART failed");
		}
	}
	if (set_idleimmediate) {
		__u8 args[4] = {ATA_OP_IDLEIMMEDIATE,0,0,0};
		if (get_idleimmediate)
			printf(" issuing idle_immediate command\n");
		if (do_drive_cmd(fd, args, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(idle_immediate) failed");
		}
	}
	if (set_standbynow) {
		__u8 args1[4] = {ATA_OP_STANDBYNOW1,0,0,0};
		__u8 args2[4] = {ATA_OP_STANDBYNOW2,0,0,0};
		if (get_standbynow)
			printf(" issuing standby command\n");
		if (do_drive_cmd(fd, args1, 0) && do_drive_cmd(fd, args2, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(standby) failed");
		}
	}
	if (set_idleunload) {
		if (get_idleunload)
			printf(" issuing idle_immediate_unload command\n");
		err = do_idleunload(fd, devname);
	}
	if (set_sleepnow) {
		__u8 args1[4] = {ATA_OP_SLEEPNOW1,0,0,0};
		__u8 args2[4] = {ATA_OP_SLEEPNOW2,0,0,0};
		if (get_sleepnow)
			printf(" issuing sleep command\n");
		if (do_drive_cmd(fd, args1, 0) && do_drive_cmd(fd, args2, 0)) {
			err = errno;
			perror(" HDIO_DRIVE_CMD(sleep) failed");
		}
	}
	if (set_doreset) {
		if (get_doreset)
			printf(" resetting drive\n");
		if (ioctl(fd, HDIO_DRIVE_RESET, NULL)) {
			err = errno;
			perror(" HDIO_DRIVE_RESET failed");
		}
	}
	close (fd);
	if (err)
		exit (err);
}

#if !defined (HDPARM_MINI)
#define GET_XFERMODE(flag, num)					\
	do {							\
		char *tmpstr = name;				\
		tmpstr[0] = '\0';				\
		if (!*argp && argc && isalnum(**argv))		\
			argp = *argv++, --argc;			\
		while (isalnum(*argp) && (tmpstr - name) < 31) {\
			tmpstr[0] = *argp++;			\
			tmpstr[1] = '\0';			\
			++tmpstr;				\
		}						\
		num = translate_xfermode(name);			\
		if (num == -1)					\
			flag = 0;				\
		else						\
			flag = 1;				\
	} while (0)

static int fromhex (__u8 c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	fprintf(stderr, "bad char: '%c' 0x%02x\n", c, c);
	exit(EINVAL);
}

static int ishex (char c)
{
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static void
identify_from_stdin (void)
{
	__u16 sbuf[512];
	int err, wc = 0;

	do {
		int digit;
		int d[4];

		if (ishex(d[digit=0] = getchar())
		 && ishex(d[++digit] = getchar())
		 && ishex(d[++digit] = getchar())
		 && ishex(d[++digit] = getchar())) {
		 	sbuf[wc] = (fromhex(d[0]) << 12) | (fromhex(d[1]) << 8) | (fromhex(d[2]) << 4) | fromhex(d[3]);
			++wc;
		} else if (d[digit] == EOF) {
			goto eof;
		} else if (wc == 0) {
			/* skip over leading lines of cruft */
			while (d[digit] != '\n') {
				if (d[digit] == EOF)
					goto eof;
				d[digit=0] = getchar();
			};
		}
	} while (wc < 256);
	putchar('\n');
	identify(-1, sbuf);
	return;
eof:
	err = errno;
	fprintf(stderr, "read only %u/256 IDENTIFY words from stdin: %s\n", wc, strerror(err));
	exit(err);
}
#endif

static void
numeric_parm (char c, const char *name, int *val, int *setparm, int *getparm, int min, int max, int set_only)
{
	int got_digit = 0;

	*val = 0;
	*getparm = noisy;
	noisy = 1;
	if (!*argp && argc && isdigit(**argv))
		argp = *argv++, --argc;
	while (isdigit(*argp)) {
		*setparm = 1;
		*val = (*val * 10) + (*argp++ - '0');
		got_digit = 1;
	}
	if ((set_only && !got_digit) || *val < min || *val > max) {
		fprintf(stderr, "  -%c: bad/missing %s value (%d..%d)\n", c, name, min, max);
		exit(EINVAL);
	}
}

#define NUMERIC_PARM(CH,NAME,VAR,MIN,MAX,GETSET) numeric_parm(CH,NAME,&VAR,&set_##VAR,&get_##VAR,MIN,MAX,GETSET)
#define GET_SET_PARM(CH,NAME,VAR,MIN,MAX) CH:NUMERIC_PARM(CH,NAME,VAR,MIN,MAX,0);break
#define     SET_PARM(CH,NAME,VAR,MIN,MAX) CH:NUMERIC_PARM(CH,NAME,VAR,MIN,MAX,1);break
#define     SET_FLAG1(VAR)                get_##VAR=noisy;noisy=1;set_##VAR=1
#define     SET_FLAG(CH,VAR)              CH:SET_FLAG1(VAR);break
#define      DO_FLAG(CH,VAR)              CH:VAR=1;noisy=1;break
#define    INCR_FLAG(CH,VAR)              CH:VAR++;noisy=1;break

#if !defined (HDPARM_MINI)
static void get_security_password (int handle_NULL)
{
	unsigned int maxlen = sizeof(security_password) - 1;
	unsigned int binary_passwd = 0;

	if (security_prompt_for_password) {
		const char *passwd = getpass("Please enter the drive password: ");
		if (passwd == NULL) {
			fprintf(stderr, "failed to read a password, errno=%d\n", errno);
			exit(EINVAL);
		}
		if (strlen(passwd) >= sizeof(security_password)) {
			fprintf(stderr, "password is too long (%u chars max)\n", (int)sizeof(security_password) - 1);
			exit(EINVAL);
		}
		strcpy(security_password, passwd);
		return;
	}

	argp = *argv++, --argc;
	if (!argp || argc < 1) {
		fprintf(stderr, "missing PASSWD\n");
		exit(EINVAL);
	}
	memset(security_password, 0, maxlen + 1);
	if (0 == strncmp(argp, "hex:", 4)) {
		argp += 4;
		if (strlen(argp) != (maxlen * 2)) {
			fprintf(stderr, "invalid PASSWD length (hex string must be exactly %d chars)\n", maxlen*2);
			exit(EINVAL);
		}
		char *cur = security_password;
		while (*argp) {
			int d[2];
			d[0] = fromhex(*argp++);
			d[1] = fromhex(*argp++);
			*(cur++) = d[0] << 4 | d[1];
		}
		binary_passwd = 1;
	} else if (strlen(argp) > maxlen) {
		fprintf(stderr, "PASSWD too long (must be %d chars max)\n", maxlen);
		exit(EINVAL);
	} else if (!handle_NULL || strcmp(argp, "NULL")) {
		strcpy(security_password, argp);
	}
	printf("security_password:");
	if (!binary_passwd) {
		printf(" \"%s\"\n", security_password);
	} else {
		unsigned int i;
		for (i = 0; i < maxlen; ++i) {
			unsigned char c = security_password[i];
			printf(" %02x", c);
		}
		putchar('\n');
	}
	while (*argp)
		++argp;
}

static void get_ow_pattern (void)
{
	unsigned int maxlen = sizeof(ow_pattern);

	argp = *argv++, --argc;
	if (!argp || argc < 1) {
		fprintf(stderr, "missing PATTERN\n");
		exit(EINVAL);
	}
	if (0 == strncmp(argp, "hex:", 4)) {
		argp += 4;
		if (strlen(argp) != (maxlen * 2)) {
			fprintf(stderr, "invalid PATTERN length (hex string must be exactly %d chars)\n", maxlen*2);
			exit(EINVAL);
		}
		int i = 28;
		ow_pattern = 0;
		while (*argp) {
			ow_pattern |= fromhex(*argp++) << i;
			i -= 4;
		}

	} else {
		fprintf(stderr, "invalid PATTERN format (must be hex:XXXXXXXX)\n");
		exit(EINVAL);
	}
}

static const char *sector_size_emsg = "sector size out of range";
static const char *lba_emsg = "bad/missing sector value";
static const char *count_emsg = "bad/missing sector count";
static const __u64 lba_limit = (1ULL << 48) - 1;

static int
get_u64_parm (int optional, const char flag_c, int *flag_p, __u64 *value_p,
		unsigned int min_value, __u64 limit, const char *eprefix, const char *emsg)
{
	int got_value = 0;
	__u64 value = *value_p;
	char *endp = NULL;

	if (!*argp && argc && (isdigit(**argv) || (flag_p && flag_c == **argv)))
		argp = *argv++, --argc;

	if (flag_p) {
		*flag_p = 0;
		if (*argp == flag_c) {
			*flag_p = 1;
			argp++;
		}
	}

	errno = 0;
	value = strtoll(argp, &endp, 0);
	if (errno != 0 || (endp != argp && ((__s64)value < (__s64)min_value || value > limit))) {
		fprintf(stderr, "  %s: %s\n", eprefix, emsg);
		exit(EINVAL);
	}
	if (endp != argp) {
		got_value = 1;
		*value_p = value;
		argp = endp;
	}
	if (!optional && !got_value) {
		fprintf(stderr, "  %s: %s\n", eprefix, emsg);
		exit(EINVAL);
	}
	return got_value;
}

static void
get_set_max_sectors_parms (void)
{
	get_native_max_sectors = noisy;
	noisy = 1;
	set_max_sectors = get_u64_parm(1, 'p', &set_max_permanent, &set_max_addr, 1, lba_limit, "-N", lba_emsg);
}

static void
get_set_max_sectors_parms_dco (void)
{
	do_dco_setmax = get_u64_parm(0, 0, NULL, &set_max_addr, 1, lba_limit, "--dco-setmax", lba_emsg);
}

static int
handle_standalone_longarg (char *name)
{
	if (num_flags_processed) {
		if (verbose)
			fprintf(stderr, "%s: num_flags_processed == %d\n", __func__, num_flags_processed);
		usage_help(1,EINVAL);
	}
	/* --Istdin is special: no filename arg(s) wanted here */
	if (0 == strcasecmp(name, "Istdin")) {
		if (argc > 0) {
			if (verbose)
				fprintf(stderr, "%s: argc(%d) > 0\n", __func__, argc);
			usage_help(2,EINVAL);
		}
		identify_from_stdin();
		exit(0);
	}
	if (0 == strcasecmp(name, "dco-restore")) {
		do_dco_restore = 1;
	} else if (0 == strcasecmp(name, "dco-setmax")) {
		get_set_max_sectors_parms_dco();
	} else if (0 == strcasecmp(name, "security-help")) {
		security_help(0);
		exit(0);
	} else if (0 == strcasecmp(name, "security-unlock")) {
		set_security = 1;
		security_command = ATA_OP_SECURITY_UNLOCK;
		get_security_password(1);
	} else if (0 == strcasecmp(name, "security-set-pass")) {
		set_security = 1;
		security_command = ATA_OP_SECURITY_SET_PASS;
		get_security_password(1);
	} else if (0 == strcasecmp(name, "security-disable")) {
		set_security = 1;
		security_command = ATA_OP_SECURITY_DISABLE;
		get_security_password(1);
	} else if (0 == strcasecmp(name, "security-erase")) {
		set_security = 1;
		security_command = ATA_OP_SECURITY_ERASE_UNIT;
		get_security_password(1);
	} else if (0 == strcasecmp(name, "security-erase-enhanced")) {
		set_security = 1;
		enhanced_erase = 1;
		security_command = ATA_OP_SECURITY_ERASE_UNIT;
		get_security_password(1);
	} else if (0 == strcasecmp(name, "sanitize-status")) {
		do_sanitize = 1;
		sanitize_feature = SANITIZE_STATUS_EXT;
	} else if (0 == strcasecmp(name, "sanitize-freeze-lock")) {
		do_sanitize = 1;
		sanitize_feature = SANITIZE_FREEZE_LOCK_EXT;
	} else if (0 == strcasecmp(name, "sanitize-antifreeze-lock")) {
		do_sanitize = 1;
		sanitize_feature = SANITIZE_ANTIFREEZE_LOCK_EXT;
	} else if (0 == strcasecmp(name, "sanitize-block-erase")) {
		do_sanitize = 2;
		sanitize_feature = SANITIZE_BLOCK_ERASE_EXT;
	} else if (0 == strcasecmp(name, "sanitize-crypto-scramble")) {
		do_sanitize = 2;
		sanitize_feature = SANITIZE_CRYPTO_SCRAMBLE_EXT;
	} else if (0 == strcasecmp(name, "sanitize-overwrite")) {
		do_sanitize = 2;
		get_ow_pattern();
		sanitize_feature = SANITIZE_OVERWRITE_EXT;
	}
	else {
		fprintf(stderr, "%s: unknown flag\n", name);
		exit(EINVAL);
		//usage_help(3,EINVAL);
	}
	return 1;  // no more flags allowed
}

static void
get_filename_parm (char **result, const char *emsg)
{
	if (!*argp && argc)
		argp = *argv++, --argc;
	if (!argp || !*argp) {
		fprintf(stderr, "  %s: bad/missing filename parameter\n", emsg);
		exit(EINVAL);
	}
	*result = argp;
	argp += strlen(argp);
	// if (argc) argp = *argv++, --argc;
}

static void
do_fallocate (const char *name)
{
	char *path;
	__u64 blkcount;

	get_u64_parm(0, 0, NULL, &blkcount, 0, (1ULL << 53), name, "bad/missing block-count");
	get_filename_parm(&path, name);
	if (num_flags_processed || argc)
		usage_help(4,EINVAL);
	exit(do_fallocate_syscall(path, blkcount * 1024));
}

static void
do_fibmap_file (const char *name)
{
	int err;
	char *path;

	get_filename_parm(&path, name);
	if (num_flags_processed || argc)
		usage_help(5,EINVAL);
	err = do_filemap(path);
	exit(err);
}

static int
get_longarg (void)
{
	char *name = argp;

	while (*argp)
		++argp;
	if (0 == strcasecmp(name, "verbose")) {
		verbose = 1;
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "prefer-ata12")) {
		prefer_ata12 = 1;
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "offset")) {
		set_timings_offset = 1;
		get_u64_parm(0, 0, NULL, &timings_offset, 0, ~0, name, "GB offset for -t flag");
		timings_offset *= 0x40000000ULL;
	} else if (0 == strcasecmp(name, "yes-i-know-what-i-am-doing")) {
		i_know_what_i_am_doing = 1;
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "please-destroy-my-drive")) {
		please_destroy_my_drive = 1;
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "direct")) {
		open_flags |= O_DIRECT;
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "drq-hsm-error")) {
		drq_hsm_error = 1;
	} else if (0 == strcasecmp(name, "dco-freeze")) {
		do_dco_freeze = 1;
	} else if (0 == strcasecmp(name, "dco-identify")) {
		do_dco_identify = 1;
	} else if (0 == strcasecmp(name, "fallocate")) {
		do_fallocate(name);
	} else if (0 == strcasecmp(name, "fibmap")) {
		do_fibmap_file(name);
	} else if (0 == strcasecmp(name, "fwdownload-mode3")) {
		get_filename_parm(&fwpath, name);
		do_fwdownload = 1;
		xfer_mode = 3;
	} else if (0 == strcasecmp(name, "fwdownload-modee")) {
		get_filename_parm(&fwpath, name);
		do_fwdownload = 1;
		xfer_mode = 0xe;
	} else if (0 == strcasecmp(name, "fwdownload-modee-max")) {
		get_filename_parm(&fwpath, name);
		do_fwdownload = 1;
		xfer_mode = 0xe0;
	} else if (0 == strcasecmp(name, "fwdownload-mode3-max")) {
		get_filename_parm(&fwpath, name);
		do_fwdownload = 1;
		xfer_mode = 0x30;
	} else if (0 == strcasecmp(name, "fwdownload-mode7")) {
		get_filename_parm(&fwpath, name);
		do_fwdownload = 1;
		xfer_mode = 7;
	} else if (0 == strcasecmp(name, "fwdownload")) {
		get_filename_parm(&fwpath, name);
		do_fwdownload = 1;
		xfer_mode = 0;
	} else if (0 == strcasecmp(name, "idle-immediate")) {
		SET_FLAG1(idleimmediate);
	} else if (0 == strcasecmp(name, "idle-unload")) {
		SET_FLAG1(idleunload);
	} else if (0 == strcasecmp(name, "make-bad-sector")) {
		make_bad_sector = 1;
		get_u64_parm(0, 'f', &make_bad_sector_flagged, &make_bad_sector_addr, 0, lba_limit, name, lba_emsg);
#ifdef FORMAT_AND_ERASE
	} else if (0 == strcasecmp(name, "format-track")) {
		format_track = 1;
		get_u64_parm(0, 0, NULL, &format_track_addr, 0, lba_limit, name, lba_emsg);
	} else if (0 == strcasecmp(name, "erase-sectors")) {
		erase_sectors = 1;
		get_u64_parm(0, 0, NULL, &erase_sectors_addr, 0, lba_limit, name, lba_emsg);
#endif
	} else if (0 == strcasecmp(name, SET_SECTOR_SIZE)) {
		if (get_u64_parm(0, 0, NULL, &new_sector_size, 0x200, 0x1080, "--" SET_SECTOR_SIZE, sector_size_emsg))
			do_set_sector_size = 1;
	} else if (0 == strcasecmp(name, "trim-sector-ranges-stdin")) {
		trim_from_stdin = 1;
	} else if (0 == strcasecmp(name, "trim-sector-ranges")) {
		int i, optional = 0, max_ranges = argc;
		trim_sector_ranges = malloc(sizeof(struct sector_range_s) * max_ranges);
		if (!trim_sector_ranges) {
			int err = errno;
			perror("malloc()");
			exit(err);
		}
		open_flags |= O_RDWR;
		for (i = 0; i < max_ranges; ++i) {
			char err_prefix[64];
			struct sector_range_s *p = &trim_sector_ranges[i];
			sprintf(err_prefix, "%s[%u]", name, i);
			if (!get_u64_parm(optional, 0, NULL, &(p->lba), 0, lba_limit, err_prefix, lba_emsg))
				break;
			if (*argp++ != ':' || !isdigit(*argp)) {
				fprintf(stderr, "%s: %s\n", err_prefix, count_emsg);
				exit(EINVAL);
			}
			get_u64_parm(0, 0, NULL, &(p->nsectors), 1, 0xffff, err_prefix, count_emsg);
			optional = 1;
			trim_sector_ranges_count = i + 1;
		}
	} else if (0 == strcasecmp(name, "write-sector") || 0 == strcasecmp(name, "repair-sector")) {
		write_sector = 1;
		get_u64_parm(0, 0, NULL, &write_sector_addr, 0, lba_limit, name, lba_emsg);
	} else if (0 == strcasecmp(name, "read-sector")) {
		read_sector = 1;
		get_u64_parm(0, 0, NULL, &read_sector_addr, 0, lba_limit, name, lba_emsg);
	} else if (0 == strcasecmp(name, "Istdout")) {
		do_IDentity = 2;
	} else if (0 == strcasecmp(name, "Iraw")) {
		do_IDentity = 3;
		get_filename_parm(&raw_identify_path, name);
	} else if (0 == strcasecmp(name, "security-mode")) {
		if (argc && isalpha(**argv)) {
			argp = *argv++, --argc;
			if (*argp == 'm')	/* max */
				security_mode = 1;
			else if (*argp == 'h')	/* high */
				security_mode = 0;
			else
				security_help(EINVAL);
			while (*argp) ++argp;
		}
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "security-prompt-for-password")) {
		security_prompt_for_password = 1;
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "user-master")) {
		if (argc && isalpha(**argv)) {
			argp = *argv++, --argc;
			if (*argp == 'u')	/* user */
				security_master = 0;
			else if (*argp == 'm')	/* master */
				security_master = 1;
			else
				security_help(EINVAL);
			while (*argp) ++argp;
		}
		--num_flags_processed;	/* doesn't count as an action flag */
	} else if (0 == strcasecmp(name, "security-freeze")) {
		security_freeze = 1;
	} else {
		return handle_standalone_longarg(name);
	}
	return 0; /* additional flags allowed */
}
#endif

int main (int _argc, char **_argv)
{
	int no_more_flags = 0, disallow_flags = 0;
	char c;

	argc = _argc;
	argv = _argv;
	argp = NULL;

	if  ((progname = (char *) strrchr(*argv, '/')) == NULL)
		progname = *argv;
	else
		progname++;
	++argv;

	if (!--argc)
		usage_help(6,EINVAL);
	while (argc--) {
		argp = *argv++;
		if (no_more_flags || argp[0] != '-') {
			if (!num_flags_processed)
				do_defaults = 1;
			process_dev(argp);
			continue;
		}
		if (0 == strcmp(argp, "--")) {
			no_more_flags = 1;
			continue;
		}
		if (disallow_flags) {
			fprintf(stderr, "Excess flags given.\n");
			usage_help(7,EINVAL);
		}
		if (!*++argp)
			usage_help(8,EINVAL);
		while (argp && (c = *argp++)) {
			switch (c) {
				case GET_SET_PARM('a',"filesystem-read-ahead",fsreadahead,0,2048);
				case GET_SET_PARM('A',"look-ahead",lookahead,0,1);
				case GET_SET_PARM('b',"bus-state",busstate,0,2);
				case GET_SET_PARM('B',"power-management-mode",apmmode,0,255);
				case GET_SET_PARM('c',"32-bit-IO",io32bit,0,3);
				case     SET_FLAG('C',powermode);
				case GET_SET_PARM('d',"dma-enable",dma,0,1);
				case     SET_PARM('D',"defects-management",defects,0,1);
#if !defined (HDPARM_MINI)
				case     SET_PARM('E',"CDROM/DVD-speed",cdromspeed,0,255);
#endif
				case      DO_FLAG('f',do_flush);
				case      DO_FLAG('F',do_flush_wcache);
				case      DO_FLAG('g',get_geom);
				case              'h': usage_help(9,0); break;
				case     SET_FLAG('H',hitachi_temp);
				case      DO_FLAG('i',do_identity);
				case      DO_FLAG('I',do_IDentity);
				case GET_SET_PARM('J',"WDC-idle3-timeout",wdidle3,0,300);
				case GET_SET_PARM('k',"kernel-keep-settings",keep,0,1);
				case     SET_PARM('K',"drive-keep-settings",dkeep,0,1);
				case     SET_PARM('L',"door-lock",doorlock,0,1);
				case GET_SET_PARM('m',"multmode-count",mult,0,64);
				case GET_SET_PARM('M',"acoustic-management",acoustic,0,255);
				case GET_SET_PARM('n',"ignore-write-errors",nowerr,0,1);
#if !defined (HDPARM_MINI)
				case              'N': get_set_max_sectors_parms(); break;
#endif
				case     SET_PARM('P',"prefetch",prefetch,0,255);
				case              'q': quiet = 1; noisy = 0; break;
				case GET_SET_PARM('Q',"queue-depth",dma_q,0,1024);
				case     SET_PARM('s',"powerup-in-standby",powerup_in_standby,0,1);
				case     SET_PARM('S',"standby-interval",standby,0,255);
				case GET_SET_PARM('r',"read-only",readonly,0,1);
				case GET_SET_PARM('R',"write-read-verify",write_read_verify,0,3);
				case      DO_FLAG('t',do_timings);
				case      DO_FLAG('T',do_ctimings);
				case GET_SET_PARM('u',"unmask-irq",unmask,0,1);
				case      DO_FLAG('v',do_defaults);
				case              'V': fprintf(stdout, "%s %s\n", progname, HDPARM_VERSION); exit(0);
				case     SET_FLAG('w',doreset);
				case GET_SET_PARM('W',"write-cache",wcache,0,1);
				case     SET_FLAG('y',standbynow);
				case     SET_FLAG('Y',sleepnow);

				case     SET_FLAG('z',reread_partn);
				case     SET_FLAG('Z',seagate);

#if !defined (HDPARM_MINI)
				case '-':
					if (get_longarg())
						disallow_flags = 1;
					break;

				case 'p':
					get_piomode = noisy;
					noisy = 1;
					GET_XFERMODE(set_piomode,piomode);
					break;

				case 'X':
					get_xfermode = noisy;
					noisy = 1;
					GET_XFERMODE(set_xfermode,xfermode_requested);
					if (!set_xfermode)
						fprintf(stderr, "-X: missing value\n");
					break;
#endif

				default:
					usage_help(10,EINVAL);
			}
			num_flags_processed++;
		}
		if (!argc)
			usage_help(11,EINVAL);
	}
	return 0;
}
