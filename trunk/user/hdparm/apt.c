#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <sys/types.h>
#include <errno.h>
#include "hdparm.h"
#include "sgio.h"

/*
 * apt - Support for ATA PASS THROUGH devices. Currently supported only
 *       JMicron devices.
 *
 * Copyright (c) 2009	Jan Friesse <jfriesse@gmail.com>
 *
 * Magic numbers are taken from smartmontools source code
 * (http://smartmontools.sourceforge.net/)
 *
 * You may use/distribute this freely, under the terms of either
 * (your choice) the GNU General Public License version 2,
 * or a BSD style license.
 */

#ifdef SG_IO

/* Device initialization functions */
static int apt_jmicron_int_init(int fd);

/* Device sg16 functions*/
static int apt_jmicron_sg16(int fd, int rw, int dma, struct ata_tf *tf,
	    void *data, unsigned int data_bytes, unsigned int timeout_secs);

/* Structs */
struct apt_usb_id_entry {
	int vendor_id;
	int product_id;
	int version;
	const char *type;
	int (*init_func)(int fd);
	int (*sg16_func)(int fd, int rw, int dma, struct ata_tf *tf,
	    void *data, unsigned int data_bytes, unsigned int timeout_secs);

};

struct apt_data_struct {
	int is_apt;
	struct apt_usb_id_entry id;
	int verbose;
	union {
		struct {
			int port;
		} jmicron;
	};
};

static struct apt_data_struct apt_data;

const char apt_ds_jmicron[] = "jmicron";
const char apt_ds_unsup[]   = "unsupported";

const struct apt_usb_id_entry apt_usb_id_map[] = {
	{0x152d, 0x2329, 0x0100, apt_ds_jmicron,
	    apt_jmicron_int_init, apt_jmicron_sg16}, /* JMicron JM20329 (USB->SATA) */
	{0x152d, 0x2336, 0x0100, apt_ds_jmicron,
	    apt_jmicron_int_init, apt_jmicron_sg16}, /* JMicron JM20336 (USB+SATA->SATA, USB->2xSATA) */
	{0x152d, 0x2338, 0x0100, apt_ds_jmicron,
	    apt_jmicron_int_init, apt_jmicron_sg16}, /* JMicron JM20337/8 (USB->SATA+PATA, USB+SATA->PATA) */
	{0x152d, 0x2339, 0x0100, apt_ds_jmicron,
	    apt_jmicron_int_init, apt_jmicron_sg16}  /* JMicron JM20339 (USB->SATA) */
};

int apt_detect (int fd, int verbose)
{
	int err;
	unsigned int i;

	apt_data.is_apt = 0;

	err = sysfs_get_attr_recursive(fd, "idVendor", "%x", &apt_data.id.vendor_id, NULL, verbose);
	if (err) {
		if (verbose) printf("APT: No idVendor found -> not USB bridge device\n");
		return 0;
	}

	err = sysfs_get_attr_recursive(fd, "idProduct", "%x", &apt_data.id.product_id, NULL, verbose);
	if (err) return 0;

	err = sysfs_get_attr_recursive(fd, "bcdDevice", "%x", &apt_data.id.version, NULL, verbose);
	if (err) return 0;

	if (verbose)
		printf("APT: USB ID = 0x%04x:0x%04x (0x%03x)\n", apt_data.id.vendor_id, apt_data.id.product_id,
		  apt_data.id.version);

	/* We have all needed informations, let's find if we support that device*/
	for (i = 0; i <	sizeof(apt_usb_id_map)/sizeof(*apt_usb_id_map); i++) {
		if (apt_data.id.vendor_id == apt_usb_id_map[i].vendor_id &&
		    apt_data.id.product_id == apt_usb_id_map[i].product_id) {
			/* Maybe two devices with same vendor and product id -> use version*/
			if (apt_usb_id_map[i].version > 0 && apt_data.id.type &&
			    apt_usb_id_map[i].version == apt_data.id.version) {
				apt_data.id.type = apt_usb_id_map[i].type;
				apt_data.id.init_func = apt_usb_id_map[i].init_func;
				apt_data.id.sg16_func = apt_usb_id_map[i].sg16_func;
			}

			/* We don't have type -> set it (don't care about version) */
			if (!apt_data.id.type) {
				apt_data.id.type = apt_usb_id_map[i].type;
				apt_data.id.init_func = apt_usb_id_map[i].init_func;
				apt_data.id.sg16_func = apt_usb_id_map[i].sg16_func;
			}
		}
	}

	if (!apt_data.id.type || apt_data.id.type == apt_ds_unsup) {
		if (verbose)
			printf("APT: Unsupported device\n");

		return 0;
	}

	apt_data.is_apt = 1;
	if (verbose)
		printf("APT: Found supported device %s\n", apt_data.id.type);

	apt_data.verbose = verbose;

	return (apt_data.id.init_func(fd));
}

int apt_is_apt (void)
{
	return apt_data.is_apt;
}

int apt_sg16(int fd, int rw, int dma, struct ata_tf *tf,
	    void *data, unsigned int data_bytes, unsigned int timeout_secs)
{
	return apt_data.id.sg16_func(fd, rw, dma, tf, data, data_bytes, timeout_secs);
}

static void dump_bytes (const char *prefix, unsigned char *p, int len)
{
	int i;

	if (prefix)
		fprintf(stderr, "%s: ", prefix);
	for (i = 0; i < len; ++i)
		fprintf(stderr, " %02x", p[i]);
	fprintf(stderr, "\n");
}

/***** JMicron support ********/
static int apt_jmicron_int_sg(int fd, int rw, int dma, struct ata_tf *tf,
        void *data, unsigned int data_bytes, unsigned int timeout_secs,
        int port)
{
	unsigned char cdb[12];
	struct scsi_sg_io_hdr io_hdr;

	if (dma && apt_data.verbose)
		printf("APT: JMicron doesn't support DMA\n");

	if (tf->is_lba48) {
		if (apt_data.verbose)
			fprintf(stderr, "APT: JMicron doesn't support 48-bit ATA commands\n");
                errno = EBADE;
                return -1;
	}

	memset(&cdb, 0, sizeof(cdb));
	memset(&io_hdr, 0, sizeof(struct scsi_sg_io_hdr));

	// Build pass through command
	cdb[ 0] = 0xdf;
	cdb[ 1] = (rw ? 0x00 : 0x10);
	cdb[ 2] = 0x00;
	cdb[ 3] = (unsigned char)((data ? data_bytes : 0) >> 8);
	cdb[ 4] = (unsigned char)((data ? data_bytes : 0) );
	cdb[ 5] = tf->lob.feat;
	cdb[ 6] = tf->lob.nsect;
	cdb[ 7] = tf->lob.lbal;
	cdb[ 8] = tf->lob.lbam;
	cdb[ 9] = tf->lob.lbah;
	cdb[10] = (port ? port : apt_data.jmicron.port);
	cdb[11] =  tf->command;

	io_hdr.interface_id	= 'S';
	io_hdr.mx_sb_len	= 0;
	io_hdr.dxfer_direction	= data ? (rw ? SG_DXFER_TO_DEV : SG_DXFER_FROM_DEV) : SG_DXFER_NONE;
	io_hdr.dxfer_len	= data ? data_bytes : 0;
	io_hdr.dxferp		= data;
	io_hdr.cmdp		= cdb;
	io_hdr.pack_id		= tf_to_lba(tf);
	io_hdr.timeout		= (timeout_secs ? timeout_secs : 5) * 1000; /* msecs */
	io_hdr.cmd_len 		= sizeof(cdb);

	if (apt_data.verbose)
		dump_bytes("outgoing cdb", cdb, sizeof(cdb));
	if (ioctl(fd, SG_IO, &io_hdr) == -1) {
		if (apt_data.verbose)
			perror("ioctl(fd,SG_IO)");
		return -1;      /* SG_IO not supported */
        }
	if (apt_data.verbose)
		fprintf(stderr, "SG_IO: ATA_%u status=0x%x, host_status=0x%x, driver_status=0x%x\n",
		    io_hdr.cmd_len, io_hdr.status, io_hdr.host_status, io_hdr.driver_status);

	if (io_hdr.host_status || io_hdr.driver_status) {
		errno = EBADE;
		return -1;
	}

	return 0;
}

static int apt_jmicron_int_get_registers(int fd, unsigned short addr,
				    unsigned char * buf, unsigned short size)
{
	struct ata_tf tf;

	memset(&tf, 0, sizeof(tf));

	tf.lob.feat	= 0x00;
	tf.lob.nsect	= (unsigned char)(addr >> 8);
	tf.lob.lbal	= (unsigned char)(addr);
	tf.lob.lbam	= 0x00;
	tf.lob.lbah	= 0x00;
	tf.command	= 0xfd;

	return apt_jmicron_int_sg(fd, 0, 0, &tf, buf, (unsigned int)size, 0, 0x00);
}

static int apt_jmicron_int_init(int fd)
{
	unsigned char regbuf = 0;
	int res;

	if ((res = apt_jmicron_int_get_registers(fd, 0x720F, &regbuf, 1)) == -1) {
		return res;
	}

	if (regbuf & 0x04) {
		apt_data.jmicron.port = 0xa0;
	} else if (regbuf & 0x40) {
		apt_data.jmicron.port = 0xb0;
	} else {
		perror("APT: No JMicron device connected");
		errno = ENODEV;
		return -1;
	}

	if (apt_data.verbose)
		printf("APT: JMicron Port: 0x%X\n", apt_data.jmicron.port);
	return 0;
}

static int apt_jmicron_sg16(int fd, int rw, int dma, struct ata_tf *tf,
        void *data, unsigned int data_bytes, unsigned int timeout_secs)
{

	return apt_jmicron_int_sg(fd, rw, dma, tf, data, data_bytes, timeout_secs, 0);
}

#else
/* No SGIO -> no support*/
int apt_detect (int fd, int verbose)
{
	if (verbose)
		printf("APT: SGIO Support needed for fd %d\n", fd);
	return 0;
}

int apt_is_apt (void)
{
	return 0;
}

int apt_sg16(int fd, int rw, int dma, struct ata_tf *tf,
	    void *data, unsigned int data_bytes, unsigned int timeout_secs)
{
	printf("APT: SG16 fd %d rw %d dma %d tf %p data %p data_bytes %d timeout %d need SGIO\n",
		fd, rw, dma, tf, data, data_bytes, timeout_secs);
	return -1;
}
#endif
