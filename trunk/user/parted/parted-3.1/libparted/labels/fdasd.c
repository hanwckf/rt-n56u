/*
 * File...........: arch/s390/tools/fdasd.c
 * Author(s)......: Volker Sameske <sameske@de.ibm.com>
 * Bugreports.to..: <Linux390@de.ibm.com>
 * (C) IBM Corporation, IBM Deutschland Entwicklung GmbH, 2001
 *
 * History of changes (starts March 2001)
 * 2001-04-11 possibility to change volume serial added
 *            possibility to change partition type added
 *            some changes to DS4HPCHR and DS4DSREC
 * 2001-05-03 check for invalid partition numbers added
 *            wrong free_space calculation bug fixed
 * 2001-06-26 '-a' option added, it is now possible to add a single
 *            partition in non-interactive mode
 * 2001-06-26 long parameter support added
 *
 */

#include <config.h>
#include <arch/linux.h>
#include <parted/vtoc.h>
#include <parted/device.h>
#include <parted/fdasd.h>

#include <parted/parted.h>

#include <libintl.h>
#if ENABLE_NLS
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

static int
getpos (fdasd_anchor_t *anc, int dsn)
{
	PDEBUG
	return anc->partno[dsn];
}

static int
getdsn (fdasd_anchor_t *anc, int pos)
{
	PDEBUG
	int i;

	for (i=0; i<USABLE_PARTITIONS; i++) {
		if (anc->partno[i] == pos)
			return i;
	}

	return -1;
}

static void
setpos (fdasd_anchor_t *anc, int dsn, int pos)
{
	PDEBUG
	anc->partno[dsn] = pos;
}

void
fdasd_cleanup (fdasd_anchor_t *anchor)
{
	PDEBUG
	int i;
	partition_info_t *p, *q;

	if (anchor == NULL)
		return;

        free(anchor->f4);
        free(anchor->f5);
        free(anchor->f7);
        free(anchor->vlabel);

	p = anchor->first;
	if (p == NULL)
		return;

	for (i=1; i <= USABLE_PARTITIONS; i++) {
		if (p == NULL)
			return;
		q = p->next;
		free(p);
		p = q;
	}
}

static void
fdasd_error (fdasd_anchor_t *anc, enum fdasd_failure why, char const *str)
{
	PDEBUG
	char error[2*LINE_LENGTH], *message = error;

	switch (why) {
		case unable_to_open_disk:
			sprintf(error, "fdasd: %s -- %s\n", _("open error"), str);
			break;
		case unable_to_seek_disk:
			sprintf(error, "fdasd: %s -- %s\n", _("seek error"), str);
			break;
		case unable_to_read_disk:
			sprintf(error, "fdasd: %s -- %s\n", _("read error"), str);
			break;
		case read_only_disk:
			sprintf(error, "fdasd: %s -- %s\n", _("write error"), str);
			break;
		case unable_to_ioctl:
			sprintf(error, "fdasd: %s -- %s\n", _("ioctl() error"), str);
			break;
		case api_version_mismatch:
			sprintf(error, "fdasd: %s -- %s\n",
				_("API version mismatch"), str);
			break;
		case wrong_disk_type:
			sprintf(error, "fdasd: %s -- %s\n",
				_("Unsupported disk type"), str);
			break;
		case wrong_disk_format:
			sprintf(error, "fdasd: %s -- %s\n",
				_("Unsupported disk format"), str);
			break;
		case disk_in_use:
			sprintf(error, "fdasd: %s -- %s\n",
				_("Disk is in use"), str);
			break;
		case config_syntax_error:
			sprintf(error, "fdasd: %s -- %s\n",
				_("Syntax error in config file"), str);
			break;
		case vlabel_corrupted:
			sprintf(error, "fdasd: %s -- %s\n",
				_("Volume label is corrupted"), str);
			break;
		case dsname_corrupted:
			sprintf(error, "fdasd: %s -- %s\n",
				_("A data set name is corrupted"), str);
			break;
		case malloc_failed:
			sprintf(error, "fdasd: %s -- %s\n",
				_("Memory allocation failed"), str);
			break;
		case device_verification_failed:
			sprintf(error, "fdasd: %s -- %s\n",
				_("Device verification failed"),
				_("The specified device is not a valid DASD device"));
			break;
		default:
			sprintf(error, "fdasd: %s: %s\n", _("Fatal error"), str);
	}

	ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL, message);
}

/*
 * converts cyl-cyl-head-head-blk to blk
 */
static unsigned long
cchhb2blk (cchhb_t *p, struct fdasd_hd_geometry *geo)
{
	PDEBUG
	return (unsigned long) (p->cc * geo->heads * geo->sectors
	                        + p->hh * geo->sectors + p->b);
}

/*
 * initializes the anchor structure and allocates some
 * memory for the labels
 */
void
fdasd_initialize_anchor (fdasd_anchor_t * anc)
{
	PDEBUG
	int i;
	volume_label_t *v;
	partition_info_t *p = NULL;
	partition_info_t *q = NULL;

	anc->devno             = 0;
	anc->dev_type          = 0;
	anc->used_partitions   = 0;

	anc->silent            = 0;
	anc->verbose           = 0;
	anc->big_disk          = 0;
	anc->volid_specified   = 0;
	anc->config_specified  = 0;
	anc->auto_partition    = 0;
	anc->devname_specified = 0;
	anc->print_table       = 0;

	anc->option_reuse      = 0;
	anc->option_recreate   = 0;

	anc->vlabel_changed    = 0;
	anc->vtoc_changed      = 0;
	anc->blksize           = 0;
	anc->fspace_trk        = 0;
	anc->label_pos         = 0;

	for (i=0; i<USABLE_PARTITIONS; i++)
		setpos(anc, i, -1);

	bzero(anc->confdata, sizeof(config_data_t));

	anc->f4 = malloc(sizeof(format4_label_t));
	if (anc->f4 == NULL)
		fdasd_error(anc, malloc_failed, "FMT4 DSCB.");

	anc->f5 = malloc(sizeof(format5_label_t));
	if (anc->f5 == NULL)
		fdasd_error(anc, malloc_failed, "FMT5 DSCB.");

	anc->f7 = malloc(sizeof(format7_label_t));
	if (anc->f7 == NULL)
		fdasd_error(anc, malloc_failed, "FMT7 DSCB.");

	bzero(anc->f4, sizeof(format4_label_t));
	bzero(anc->f5, sizeof(format5_label_t));
	bzero(anc->f7, sizeof(format7_label_t));

	v = malloc(sizeof(volume_label_t));
	if (v == NULL)
		fdasd_error(anc, malloc_failed,
			    _("No room for volume label."));
	bzero(v, sizeof(volume_label_t));
	anc->vlabel = v;

	for (i=1; i<=USABLE_PARTITIONS; i++) {
        p = malloc(sizeof(partition_info_t));
		if (p == NULL)
			fdasd_error(anc, malloc_failed,
				    _("No room for partition info."));
		p->used       = 0x00;
		p->len_trk    = 0;
		p->start_trk  = 0;
		p->fspace_trk = 0;
		p->type       = 0;

		/* add p to double pointered list */
		if (i == 1) {
	        anc->first = p;
			p->prev = NULL;
		} else if (i == USABLE_PARTITIONS) {
	        anc->last = p;
	        p->next = NULL;
			p->prev = q;
			q->next = p;
		} else {
	        p->prev = q;
	        q->next = p;
		}

		p->f1 = malloc(sizeof(format1_label_t));
		if (p->f1 == NULL)
			fdasd_error(anc, malloc_failed, "FMT1 DSCB.");
		bzero(p->f1, sizeof(format1_label_t));

		q = p;
	}
}

/*
 * writes all changes to dasd
 */
static void
fdasd_write_vtoc_labels (fdasd_anchor_t * anc, int fd)
{
	PDEBUG
	partition_info_t *p;
	unsigned long b;
	char dsno[6], s1[7], s2[45], *c1, *c2, *ch;
	int i = 0, k = 0;

	b = (cchhb2blk (&anc->vlabel->vtoc, &anc->geo) - 1) * anc->blksize;
	if (b <= 0)
		fdasd_error (anc, vlabel_corrupted, "");

	/* write FMT4 DSCB */
	vtoc_write_label (fd, b, NULL, anc->f4, NULL, NULL);

	/* write FMT5 DSCB */
	b += anc->blksize;
	vtoc_write_label (fd, b, NULL, NULL, anc->f5, NULL);

	/* write FMT7 DSCB */
	if (anc->big_disk) {
		b += anc->blksize;
		vtoc_write_label (fd, b, NULL, NULL, NULL, anc->f7);
	}

	/* loop over all FMT1 DSCBs */
	p = anc->first;
	for (i = 0; i < USABLE_PARTITIONS; i++) {
		b += anc->blksize;

		if (p->used != 0x01) {
			vtoc_write_label (fd, b, p->f1, NULL, NULL, NULL);
			continue;
		}

		strncpy (p->f1->DS1DSSN, anc->vlabel->volid, 6);

		ch = p->f1->DS1DSNAM;
		vtoc_ebcdic_dec (ch, ch, 44);
		c1 = ch + 7;

		if (getdsn (anc, i) > -1) {
			/* re-use the existing data set name */
			c2 = strchr (c1, '.');
			if (c2 != NULL)
				strncpy (s2, c2, 31);
			else
				fdasd_error (anc, dsname_corrupted, "");

			strncpy (s1, anc->vlabel->volid, 6);
			vtoc_ebcdic_dec (s1, s1, 6);
			s1[6] = ' ';
			strncpy (c1, s1, 7);
			c1 = strchr (ch, ' ');
			strncpy (c1, s2, 31);
		} else {
			/* create a new data set name */
			while (getpos (anc, k) > -1)
				k++;

			setpos (anc, k, i);

			strncpy (s2, ch, 44);
			s2[44] = 0;
			vtoc_ebcdic_dec (s2, s2, 44);

			strncpy (ch, "LINUX.V               " "                      ", 44);

			strncpy (s1, anc->vlabel->volid, 6);
			vtoc_ebcdic_dec (s1, s1, 6);
			strncpy (c1, s1, 6);

			c1 = strchr (ch, ' ');
			strncpy (c1, ".PART", 5);
			c1 += 5;

			sprintf (dsno, "%04d.", k + 1);
			strncpy (c1, dsno, 5);

			c1 += 5;
			switch(p->type) {
				case PARTITION_LINUX_LVM:
					strncpy(c1, PART_TYPE_LVM, 6);
					break;
				case PARTITION_LINUX_RAID:
					strncpy(c1, PART_TYPE_RAID, 6);
					break;
				case PARTITION_LINUX:
					strncpy(c1, PART_TYPE_NATIVE, 6);
					break;
				case PARTITION_LINUX_SWAP:
					strncpy(c1, PART_TYPE_SWAP, 6);
					break;
				default:
					strncpy(c1, PART_TYPE_NATIVE, 6);
					break;
			}
		}

		vtoc_ebcdic_enc (ch, ch, 44);

		vtoc_write_label (fd, b, p->f1, NULL, NULL, NULL);
		p = p->next;
	}
}

/*
 * writes all changes to dasd
 */
int
fdasd_write_labels (fdasd_anchor_t * anc, int fd)
{
	PDEBUG
	if (anc->vlabel_changed)
		vtoc_write_volume_label (fd, anc->label_pos, anc->vlabel);

	if (anc->vtoc_changed)
		fdasd_write_vtoc_labels (anc, fd);

	return 1;
}

/*
 * writes all changes to dasd
 */
int
fdasd_prepare_labels (fdasd_anchor_t *anc, int fd)
{
	PDEBUG
	partition_info_t *p = anc->first;
	char dsno[6], s1[7], s2[45], *c1, *c2, *ch;
	int i = 0, k = 0;

	/* loop over all FMT1 DSCBs */
	p = anc->first;
	for (i = 0; i < USABLE_PARTITIONS; i++) {
		strncpy (p->f1->DS1DSSN, anc->vlabel->volid, 6);

		ch = p->f1->DS1DSNAM;
		vtoc_ebcdic_dec (ch, ch, 44);
		c1 = ch + 7;

		if (getdsn (anc, i) > -1) {
			/* re-use the existing data set name */
			c2 = strchr (c1, '.');
			if (c2 != NULL)
				strncpy (s2, c2, 31);
			else
				fdasd_error (anc, dsname_corrupted, "");

			strncpy (s1, anc->vlabel->volid, 6);
			vtoc_ebcdic_dec (s1, s1, 6);
			s1[6] = ' ';
			strncpy (c1, s1, 7);
			c1 = strchr (ch, ' ');
			strncpy (c1, s2, 31);
		} else {
			/* create a new data set name */
			while (getpos (anc, k) > -1)
				k++;

			setpos (anc, k, i);

			strncpy (s2, ch, 44);
			s2[44] = 0;
			vtoc_ebcdic_dec (s2, s2, 44);

			strncpy (ch, "LINUX.V               " "                      ", 44);

			strncpy (s1, anc->vlabel->volid, 6);
			vtoc_ebcdic_dec (s1, s1, 6);
			strncpy (c1, s1, 6);

			c1 = strchr (ch, ' ');
			strncpy (c1, ".PART", 5);
			c1 += 5;

			sprintf (dsno, "%04d.", k + 1);
			strncpy (c1, dsno, 5);

			c1 += 5;
			switch(p->type) {
				case PARTITION_LINUX_LVM:
					strncpy(c1, PART_TYPE_LVM, 6);
					break;
				case PARTITION_LINUX_RAID:
					strncpy(c1, PART_TYPE_RAID, 6);
					break;
				case PARTITION_LINUX:
					strncpy(c1, PART_TYPE_NATIVE, 6);
					break;
				case PARTITION_LINUX_SWAP:
					strncpy(c1, PART_TYPE_SWAP, 6);
					break;
				default:
					strncpy(c1, PART_TYPE_NATIVE, 6);
					break;
			}
		}

		vtoc_ebcdic_enc (ch, ch, 44);
		p = p->next;
	}

	return 1;
}

void
fdasd_recreate_vtoc (fdasd_anchor_t *anc)
{
	PDEBUG
	partition_info_t *p = anc->first;
	int i;

	vtoc_init_format4_label(anc->f4,
							USABLE_PARTITIONS,
							anc->geo.cylinders,
							anc->geo.heads,
							anc->geo.sectors,
							anc->blksize,
							anc->dev_type);

	vtoc_init_format5_label(anc->f5);
	vtoc_init_format7_label(anc->f7);
	vtoc_set_freespace(anc->f4, anc->f5, anc->f7,
					   '+', anc->verbose,
					   FIRST_USABLE_TRK,
					   anc->geo.cylinders * anc->geo.heads - 1,
					   anc->geo.cylinders, anc->geo.heads);

	for (i = 0; i < USABLE_PARTITIONS; i++) {
		bzero(p->f1, sizeof(format1_label_t));
		p->used       = 0x00;
		p->start_trk  = 0;
		p->end_trk    = 0;
		p->len_trk    = 0;
		p->fspace_trk = 0;
		p->type       = 0;
		p = p->next;
	}

	anc->used_partitions = 0;
	anc->fspace_trk = anc->geo.cylinders * anc->geo.heads - FIRST_USABLE_TRK;

	for (i=0; i<USABLE_PARTITIONS; i++)
		setpos(anc, i, -1);

	anc->vtoc_changed++;
}

/*
 * sets some important partition data
 * (like used, start_trk, end_trk, len_trk)
 * by calculating these values with the
 * information provided in the labels
 */
static void
fdasd_update_partition_info (fdasd_anchor_t *anc)
{
	PDEBUG
	partition_info_t *q = NULL, *p = anc->first;
	unsigned int h = anc->geo.heads;
	unsigned long max = anc->geo.cylinders * h - 1;
	int i;
	char *ch;

	anc->used_partitions = anc->geo.sectors - 2 - anc->f4->DS4DSREC;

	for (i = 1; i <= USABLE_PARTITIONS; i++) {
		if (p->f1->DS1FMTID != 0xf1) {
			if (i == 1)
				/* there is no partition at all */
				anc->fspace_trk = max - FIRST_USABLE_TRK + 1;
			else
				/* previous partition was the last one */
				q->fspace_trk = max - q->end_trk;
			break;
		}

		/* this is a valid format 1 label */
		p->used = 0x01;
		p->start_trk = p->f1->DS1EXT1.llimit.cc * h + p->f1->DS1EXT1.llimit.hh;
		p->end_trk   = p->f1->DS1EXT1.ulimit.cc * h + p->f1->DS1EXT1.ulimit.hh;
		p->len_trk   = p->end_trk - p->start_trk + 1;

		if (i == 1) {
			/* first partition, there is at least one */
			anc->fspace_trk = p->start_trk - FIRST_USABLE_TRK;
		} else {
			if (i == USABLE_PARTITIONS)
				/* last possible partition */
				p->fspace_trk = max - p->end_trk;

			/* set free space values of previous partition */
			q->fspace_trk = p->start_trk - q->end_trk - 1;
		}

		ch = p->f1->DS1DSNAM;
		vtoc_ebcdic_dec (ch, ch, 44);
		if (strstr(ch, PART_TYPE_LVM))
			p->type = PARTITION_LINUX_LVM;
		else if (strstr(ch, PART_TYPE_RAID))
			p->type = PARTITION_LINUX_RAID;
		else if (strstr(ch, PART_TYPE_NATIVE))
			p->type = PARTITION_LINUX;
		else if (strstr(ch, PART_TYPE_SWAP))
			p->type = PARTITION_LINUX_SWAP;
		else
			p->type = PARTITION_LINUX;
		vtoc_ebcdic_enc (ch, ch, 44);

		q = p;
		p = p->next;
	}
}

/*
 * reorganizes all FMT1s, after that all used FMT1s should be right in
 * front of all unused FMT1s
 */
static void
fdasd_reorganize_FMT1s (fdasd_anchor_t *anc)
{
	PDEBUG
	int i, j;
	format1_label_t *ltmp;
	partition_info_t *ptmp;

	for (i=1; i<=USABLE_PARTITIONS - 1; i++) {
		ptmp = anc->first;

		for (j=1; j<=USABLE_PARTITIONS - i; j++) {
			if (ptmp->f1->DS1FMTID < ptmp->next->f1->DS1FMTID) {
				ltmp = ptmp->f1;
				ptmp->f1 = ptmp->next->f1;
				ptmp->next->f1 = ltmp;
			}

			ptmp=ptmp->next;
		}
	}
}

static void
fdasd_process_valid_vtoc (fdasd_anchor_t * anc, unsigned long b, int fd)
{
	PDEBUG
	int f5_counter = 0, f7_counter = 0, f1_counter = 0, oldfmt = 0;
	int i, n, f1size = sizeof (format1_label_t);
	partition_info_t *p = anc->first;
	format1_label_t q;
	char s[5], *ch;

	b += anc->blksize;

	for (i = 1; i <= anc->geo.sectors; i++) {
		bzero (&q, f1size);
		vtoc_read_label (fd, b, &q, NULL, NULL, NULL);

		switch (q.DS1FMTID) {
			case 0xf1:
				if (p == NULL)
					break;
				memcpy (p->f1, &q, f1size);

				n = -1;
				vtoc_ebcdic_dec (p->f1->DS1DSNAM, p->f1->DS1DSNAM, 44);
				ch = strstr (p->f1->DS1DSNAM, "PART");
				if (ch != NULL) {
					strncpy (s, ch + 4, 4);
					s[4] = '\0';
					n = atoi (s) - 1;
				}

				vtoc_ebcdic_enc (p->f1->DS1DSNAM, p->f1->DS1DSNAM, 44);

				/* this dasd has data set names 0000-0002
				but we use now 0001-0003 */
				if (n == -1)
					oldfmt++;

				if (((oldfmt == 0) && (n < 0)) || (n >= USABLE_PARTITIONS)) {
					/* no op */
				} else {
					if (oldfmt) {
						/* correct +1 */
						setpos (anc, n + 1, f1_counter);
					} else {
						setpos (anc, n, f1_counter);
					}
				}

				p = p->next;
				f1_counter++;
				break;
			case 0xf5:
				memcpy (anc->f5, &q, f1size);
				f5_counter++;
				break;
			case 0xf7:
				if (f7_counter == 0)
					memcpy (anc->f7, &q, f1size);
				f7_counter++;
				break;
		}

		b += anc->blksize;
	}

	if (oldfmt > 0) {
		/* this is the old format PART0000 - PART0002 */
		anc->vtoc_changed++;
	}

	if ((f5_counter == 0) || (anc->big_disk))
		vtoc_init_format5_label (anc->f5);

	if (f7_counter == 0)
		vtoc_init_format7_label (anc->f7);

	fdasd_reorganize_FMT1s (anc);
	fdasd_update_partition_info (anc);
}

static int
fdasd_valid_vtoc_pointer(fdasd_anchor_t *anc, unsigned long b, int fd)
{
	PDEBUG
	char str[LINE_LENGTH];

	/* VOL1 label contains valid VTOC pointer */
	vtoc_read_label (fd, b, NULL, anc->f4, NULL, NULL);

	if (anc->f4->DS4IDFMT == 0xf4) {
		fdasd_process_valid_vtoc (anc, b, fd);
		return 0;
	}
	if (strncmp(anc->vlabel->volkey, vtoc_ebcdic_enc("LNX1",str,4),4) == 0 ||
	    strncmp(anc->vlabel->volkey, vtoc_ebcdic_enc("CMS1",str,4),4) == 0)
		return 0;

	fdasd_error(anc, wrong_disk_format, _("Invalid VTOC."));
	return 1;
}

/*
 * check the dasd for a volume label
 */
int
fdasd_check_volume (fdasd_anchor_t *anc, int fd)
{
	PDEBUG
	volume_label_t *v = anc->vlabel;
	unsigned long b = -1;
	char str[LINE_LENGTH];

	vtoc_read_volume_label (fd, anc->label_pos, v);

	if (strncmp(v->vollbl, vtoc_ebcdic_enc ("VOL1", str, 4), 4) == 0) {
		/* found VOL1 volume label */
		b = (cchhb2blk (&v->vtoc, &anc->geo) - 1) * anc->blksize;

		if (b > 0) {
			int rc;
			rc = fdasd_valid_vtoc_pointer (anc, b, fd);

			if (rc < 0)
				return 1;
			else
				return 0;
		} else {
			return 1;
		}
	} else if (strncmp (v->volkey, vtoc_ebcdic_enc ("LNX1", str, 4), 4) == 0 ||
	           strncmp (v->volkey, vtoc_ebcdic_enc ("CMS1", str, 4), 4) == 0) {
		return 0;
	}

	return 1;
}

/*
 * checks the current API version with the API version of the dasd driver
 */
void
fdasd_check_api_version (fdasd_anchor_t *anc, int f)
{
	PDEBUG
	int api;
	char s[2*LINE_LENGTH];

        struct stat st;
        if (fstat (f, &st) == 0 && S_ISREG (st.st_mode)) {
		/* skip these tests when F is a regular file.  */
	}
	else {
		if (ioctl(f, DASDAPIVER, &api) != 0)
			fdasd_error(anc, unable_to_ioctl,
				    _("Could not retrieve API version."));

		if (api != DASD_MIN_API_VERSION) {
			sprintf(s, _("The current API version '%d' doesn't " \
					"match dasd driver API version " \
					"'%d'!"), api, DASD_MIN_API_VERSION);
			fdasd_error(anc, api_version_mismatch, s);
		}
	}
}

/*
 * reads dasd geometry data
 */
void
fdasd_get_geometry (const PedDevice *dev, fdasd_anchor_t *anc, int f)
{
	PDEBUG
	int blksize = 0;
	dasd_information_t dasd_info;

	/* We can't get geometry from a regular file,
	   so simulate something usable, for the sake of testing.  */
	struct stat st;
	if (fstat (f, &st) == 0 && S_ISREG (st.st_mode)) {
	    PedSector n_sectors = st.st_size / dev->sector_size;
	    anc->geo.heads = 15;
	    anc->geo.sectors = 12;
	    anc->geo.cylinders
	      = (n_sectors / (anc->geo.heads * anc->geo.sectors
			      * (dev->sector_size / dev->phys_sector_size)));
	    anc->geo.start = 0;
	    blksize = 4096;
	    memcpy (dasd_info.type, "ECKD", 4);
	    dasd_info.dev_type = 13200;
	    dasd_info.label_block = 2;
	    dasd_info.devno = 513;
	} else {
		if (ioctl(f, HDIO_GETGEO, &anc->geo) != 0)
			fdasd_error(anc, unable_to_ioctl,
			    _("Could not retrieve disk geometry information."));

		if (ioctl(f, BLKSSZGET, &blksize) != 0)
			fdasd_error(anc, unable_to_ioctl,
			    _("Could not retrieve blocksize information."));

		/* get disk type */
		if (ioctl(f, BIODASDINFO, &dasd_info) != 0)
			fdasd_error(anc, unable_to_ioctl,
				    _("Could not retrieve disk information."));
	}

	anc->dev_type   = dasd_info.dev_type;
	anc->blksize    = blksize;
	anc->label_pos  = dasd_info.label_block * blksize;
	anc->devno      = dasd_info.devno;
	anc->fspace_trk = anc->geo.cylinders * anc->geo.heads - FIRST_USABLE_TRK;
}

/*
 * returns unused partition info pointer if there
 * is a free partition, otherwise NULL
 */
static partition_info_t *
fdasd_get_empty_f1_label (fdasd_anchor_t * anc)
{
	PDEBUG
	if (anc->used_partitions < USABLE_PARTITIONS)
		return anc->last;
	else
		return NULL;
}

/*
 * asks for and sets some important partition data
 */
static int
fdasd_get_partition_data (fdasd_anchor_t *anc, extent_t *part_extent,
                          partition_info_t *p, unsigned int *start_ptr,
                          unsigned int *stop_ptr)
{
	PDEBUG
	unsigned int limit, cc, hh;
	cchh_t llimit, ulimit;
	partition_info_t *q;
	u_int8_t b1, b2;
	u_int16_t c, h;
	unsigned int start = *start_ptr, stop = *stop_ptr;
	int i;
	char *ch;

	if (anc->f4->DS4DEVCT.DS4DEVFG & ALTERNATE_CYLINDERS_USED)
		c = anc->f4->DS4DEVCT.DS4DSCYL - (u_int16_t) anc->f4->DS4DEVAC;
	else
		c = anc->f4->DS4DEVCT.DS4DSCYL;

	h = anc->f4->DS4DEVCT.DS4DSTRK;
	limit = (h * c - 1);

	/* check start value from user */
	q = anc->first;
	for (i = 0; i < USABLE_PARTITIONS; i++) {
		if ( q->next == NULL )
			break;

		if (start >= q->start_trk && start <= q->end_trk) {
			/* start is within another partition */
			start = q->end_trk + 1;

			if (start > limit) {
				start = FIRST_USABLE_TRK;
				q = anc->first;
			}
		}

		if (start < q->start_trk) {
			limit = q->start_trk - 1;
			break;
		}

		q = q->next;
	}

	if (start == limit)
		stop = start;

	/* update partition info */
	p->len_trk    = stop - start + 1;
	p->start_trk  = start;
	p->end_trk    = stop;

	cc = start / anc->geo.heads;
	hh = start - (cc * anc->geo.heads);
	vtoc_set_cchh(&llimit, cc, hh);

	/* check for cylinder boundary */
	if (hh == 0)
		b1 = 0x81;
	else
		b1 = 0x01;

	cc = stop / anc->geo.heads;
	hh = stop - cc * anc->geo.heads;
	vtoc_set_cchh(&ulimit, cc, hh);

	/* it is always the 1st extent */
	b2 = 0x00;

	vtoc_set_extent(part_extent, b1, b2, &llimit, &ulimit);

	*start_ptr = start;
	*stop_ptr = stop;

	ch = p->f1->DS1DSNAM;
	vtoc_ebcdic_dec (ch, ch, 44);

	if (strstr(ch, PART_TYPE_LVM))
		p->type = PARTITION_LINUX_LVM;
	else if (strstr(ch, PART_TYPE_RAID))
		p->type = PARTITION_LINUX_RAID;
	else if (strstr(ch, PART_TYPE_NATIVE))
		p->type = PARTITION_LINUX;
	else if (strstr(ch, PART_TYPE_SWAP))
		p->type = PARTITION_LINUX_SWAP;
	else
		p->type = PARTITION_LINUX;

	vtoc_ebcdic_enc (ch, ch, 44);

	return 0;
}

static void
fdasd_enqueue_new_partition (fdasd_anchor_t *anc)
{
	PDEBUG
	partition_info_t *q = anc->first, *p = anc->last;
	int i, k=0;

	for (i=1; i<USABLE_PARTITIONS; i++) {
		if ((q->end_trk == 0) || (p->start_trk < q->start_trk)) {
			break;
		} else {
			q = q->next;
			k++;
		}
	}

	if (anc->first == q)
		anc->first = p;

	if (p != q) {
		anc->last->prev->next = NULL;
		anc->last = anc->last->prev;

		p->next = q;
		p->prev = q->prev;
		q->prev = p;

		if (p->prev != NULL)
			p->prev->next = p;
	}

	p->used = 0x01;
	p->type = PARTITION_LINUX;

	for (i=0; i<USABLE_PARTITIONS; i++) {
		int j = getpos(anc, i);
		if (j >= k)
			setpos(anc, i, j + 1);
	}

	/* update free-space counters */
	if (anc->first == p) {
		/* partition is the first used partition */
		if (p->start_trk == FIRST_USABLE_TRK) {
			/* partition starts right behind VTOC */
			p->fspace_trk = anc->fspace_trk - p->len_trk;
			anc->fspace_trk = 0;
		} else {
			/* there is some space between VTOC and partition */
			p->fspace_trk = anc->fspace_trk - p->len_trk - p->start_trk
							+ FIRST_USABLE_TRK;
			anc->fspace_trk = p->start_trk - FIRST_USABLE_TRK;
		}
	} else {
		/* there are partitions in front of the new one */
		if (p->start_trk == p->prev->end_trk + 1) {
			/* new partition is right behind the previous one */
			p->fspace_trk = p->prev->fspace_trk - p->len_trk;
			p->prev->fspace_trk = 0;
		} else {
			/* there is some space between new and prev. part. */
			p->fspace_trk = p->prev->fspace_trk - p->len_trk
							- p->start_trk + p->prev->end_trk + 1;
			p->prev->fspace_trk = p->start_trk - p->prev->end_trk - 1;
		}
	}
}

/*
 * adds a new partition to the 'partition table'
 */
partition_info_t *
fdasd_add_partition (fdasd_anchor_t *anc, unsigned int start,
                     unsigned int stop)
{
	PDEBUG
	cchhb_t hf1;
	partition_info_t *p;
	extent_t ext;
	int i;

	PDEBUG;

	if ((p = fdasd_get_empty_f1_label(anc)) == NULL) {
		PDEBUG;
		return 0;
	}

	PDEBUG;
	if (fdasd_get_partition_data(anc, &ext, p, &start, &stop) != 0)
		return 0;

	PDEBUG;
	vtoc_init_format1_label(anc->vlabel->volid, anc->blksize, &ext, p->f1);

	PDEBUG;
	fdasd_enqueue_new_partition(anc);

	PDEBUG;
	anc->used_partitions += 1;

	i = anc->used_partitions + 2;
	if (anc->big_disk)
		i++;
	PDEBUG;

	vtoc_set_cchhb(&hf1, VTOC_START_CC, VTOC_START_HH, i);

	vtoc_update_format4_label(anc->f4, &hf1, anc->f4->DS4DSREC - 1);

	PDEBUG;

	start = ext.llimit.cc * anc->geo.heads + ext.llimit.hh;
	stop  = ext.ulimit.cc * anc->geo.heads + ext.ulimit.hh;

	PDEBUG;
	vtoc_set_freespace(anc->f4, anc->f5, anc->f7, '-', anc->verbose,
					   start, stop, anc->geo.cylinders, anc->geo.heads);

	anc->vtoc_changed++;

	PDEBUG;
	return p;
}

/* vim:set tabstop=4 shiftwidth=4 softtabstop=4: */
