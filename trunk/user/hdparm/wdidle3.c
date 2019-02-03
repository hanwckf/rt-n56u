#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "sgio.h"
#include "hdparm.h"

extern int verbose;  /* hdparm.c */

/*
 * The Western Digital (WD) "Green" drive IDLE3 timeout value is 8-bits.
 * Earlier models used a strict value from 8.0 to 25.5 seconds (0x50 to 0xff).
 * Later models use 8.0 to 12.7 secs (0x50 to 0x7f), and 30 to 300 secs (0x81 to 0x8a).
 * Other values are undefined, but have been observed on some drives.
 *
 * Older drives (from WDIDLE3.EXE documentation):
 * 0x00 	timer disabled.
 * 0x50-0xff	timeout in tenths of a second, from 8.0 to 25.5 seconds.
 *
 *
 * Some drives  (found by observation):
 * 0x01-0x50	timeout in seconds, from 1 to 30 seconds (?).
 *
 * Newer drives (from WDIDLE3.EXE documentation):
 * 0x00 	timer disabled.
 * 0x01-0x49	not permitted by WDIDLE3.EXE; reports as 0.1 to 7.9 seconds.
 * 0x50-0x7f	timeout in tenths of a second, from 8.0 to 12.7 seconds.
 * 0x80		not permitted by WDIDLE3.EXE; reports as 12.8 seconds.
 * 0x81-0x8a	timeout in 30s of seconds, from 30 to 300 seconds.
 * 0x90-0xff	not permitted; reports as 330 to 3810 seconds.
 *
 * A WD whitepaper identified some models/versions of drives with the "older" firmware,
 * but no comprehensive list exists.   Models with firmware 02.01B01 are "older",
 * and models with "02.01B02" are "newer".
 *
 * The real WDIDLE3 utility determines the correct value ranges
 * by interogating the drive somehow, or perhaps based on the reported firmware version.
 *
 * WDIDLE3.EXE also writes some scary looking full 512-byte sequences to the drives
 * in addition to what we do here.
 *
 * So this (hdparm) method may be somewhat risky, since it doesn't do everything WD does.
 */

enum {
	WDC_VSC_DISABLED	= 'D',
	WDC_VSC_ENABLED		= 'E',
	WDC_IDLE3_READ		= 1,
	WDC_IDLE3_WRITE		= 2,
	WDC_OP_TIMEOUT_SECS	= 12,
	WDC_TIMEOUT_THRESHOLD	= 128,
};

static int wdidle3_issue (int fd, int rw, struct ata_tf *tf, void *data, const char *msg)
{
	int ret = sg16(fd, rw, SG_PIO, tf, data, data ? 512 : 0, WDC_OP_TIMEOUT_SECS);
	if (ret != 0) {
		ret = errno;
		perror(msg);
	}
	return ret;
}

/*
 * Note: when WD vendor-specific-commands (vsc) are enabled,
 * the ATA_OP_SMART command is "enhanced" to support additional
 * functions to read/write specific data "registers" from the firmware.
 * This may (or not) also cause regular SMART commands to fail (?).
 */
static int wdidle3_vsc_enable_disable (int fd, int setting)
{
	struct ata_tf	tf;
	unsigned long long vendor_WD0 = ('W' << 16) | ('D' << 8) | 0;

	tf_init(&tf, ATA_OP_VENDOR_SPECIFIC_0x80, vendor_WD0, 0);
	tf.lob.feat	= setting;
	tf.dev		= 0xa0;
	return wdidle3_issue(fd, SG_WRITE, &tf, NULL, __func__);
}

/*
 * This sends a key (password? I2C address?) to the drive
 * in preparation for a data_in or data_out command.
 */
static int wdidle3_vsc_send_key (int fd, int rw)
{
	char		data[512];
	struct ata_tf	tf;

	memset(data, 0, sizeof(data));
	tf_init(&tf, ATA_OP_SMART, 0xc24fbe, 1);
	tf.lob.feat	= 0xd6;
	tf.dev		= 0xa0;
	data[ 0]	= 0x2a;
	data[ 2]	= rw;
	data[ 4]	= 0x02;
	data[ 6]	= 0x0d;
	data[ 8]	= 0x16;
	data[10]	= 0x01;
	return wdidle3_issue(fd, SG_WRITE, &tf, data, __func__);
}

/*
 * This reads a data value from the drive.
 */
static int wdidle3_data_in (int fd, unsigned char *timeout)
{
	char		data[512];
	int		ret;
	struct ata_tf	tf;

	memset(data, 0, sizeof(data));
	tf_init(&tf, ATA_OP_SMART, 0xc24fbf, 1);
	tf.lob.feat	= 0xd5;
	tf.dev		= 0xa0;
	ret = wdidle3_issue(fd, SG_READ, &tf, data, __func__);
	if (!ret)
		*timeout = data[0];
	return ret;
}

/*
 * This writes a data value to the drive.
 */
static int wdidle3_vsc_data_out (int fd, unsigned char timeout)
{
	char		data[512];
	struct ata_tf	tf;

	memset(data, 0, sizeof(data));
	tf_init(&tf, ATA_OP_SMART, 0xc24fbf, 1);
	tf.lob.feat	= 0xd6;
	tf.dev		= 0xa0;
	data[0]		= timeout;
	return wdidle3_issue(fd, SG_WRITE, &tf, data, __func__);
}

int wdidle3_set_timeout (int fd, unsigned char timeout)
{
	int ret, ret2;

	ret = wdidle3_vsc_enable_disable(fd, WDC_VSC_ENABLED);
	if (!ret) {
		ret = wdidle3_vsc_send_key(fd, WDC_IDLE3_WRITE);
		if (!ret)
			ret = wdidle3_vsc_data_out(fd, timeout);
		ret2 = wdidle3_vsc_enable_disable(fd, WDC_VSC_DISABLED);
		if (!ret)
			ret = ret2;
	}
	return ret;
}

int wdidle3_get_timeout (int fd, unsigned char *timeout)
{
	int ret, ret2;

	ret = wdidle3_vsc_enable_disable(fd, WDC_VSC_ENABLED);
	if (!ret) {
		ret = wdidle3_vsc_send_key(fd, WDC_IDLE3_READ);
		if (!ret)
			ret = wdidle3_data_in(fd, timeout);
		ret2 = wdidle3_vsc_enable_disable(fd, WDC_VSC_DISABLED);
		if (!ret)
			ret = ret2;
	}
	return ret;
}

unsigned char wdidle3_msecs_to_timeout (unsigned int secs)
{
	unsigned char timeout;

	if (secs == 0)
		timeout = 0;		/* disabled */
	else if (secs < 8)
		timeout = 80;		/* 8.0 seconds minimum */
	else if (secs <= (WDC_TIMEOUT_THRESHOLD / 10))
		timeout = secs * 10;	/* 8.0 to 12.7 seconds */
	else {
		secs += 29;		/* round up to next multiple of 30 */
		if (secs > 300)
			secs = 300;	/* max timeout is 300 secs */
		timeout = (secs / 30) + WDC_TIMEOUT_THRESHOLD;
	}
	return timeout;
}

void wdidle3_print_timeout (unsigned char timeout)
{
	if (verbose)
		printf("[raw=0x%02x] ", timeout);
	if (timeout == 0)
		printf("disabled");
	else if (timeout < 0x50 || timeout == WDC_TIMEOUT_THRESHOLD)
		printf("%u ??", timeout);
	else if (timeout < WDC_TIMEOUT_THRESHOLD)
		printf("%u.%u secs", timeout / 10, timeout % 10);
	else {
		if (timeout > (WDC_TIMEOUT_THRESHOLD + 10))
			printf("%u ??", timeout);
		else
			printf("%u secs", (timeout - WDC_TIMEOUT_THRESHOLD) * 30);
		printf(" (or %u.%u secs for older drives)", timeout / 10, timeout % 10);
	}
}
