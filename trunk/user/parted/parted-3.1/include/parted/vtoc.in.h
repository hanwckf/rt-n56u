/*
 * File...........: s390-tools/dasdview/vtoc.h
 * Author(s)......: Horst Hummel <horst.hummel@de.ibm.com>
 * Bugreports.to..: <Linux390@de.ibm.com>
 *
 * This is a user-space copy of the kernel vtoc,h.
 *
 * (C) IBM Corporation, IBM Deutschland Entwicklung GmbH, 2002
 *
 * History of changes (starts March 2002)
 * 2002-03-12 initial
 */

#ifndef VTOC_H
#define VTOC_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/ioctl.h>

#define LINE_LENGTH 80
#define VTOC_START_CC 0x0
#define VTOC_START_HH 0x1
#define FIRST_USABLE_CYL 1
#define FIRST_USABLE_TRK 2

#define DASD_3380_TYPE 13148
#define DASD_3390_TYPE 13200
#define DASD_9345_TYPE 37701

#define DASD_3380_VALUE 0xbb60
#define DASD_3390_VALUE 0xe5a2
#define DASD_9345_VALUE 0xbc98

#define VOLSER_LENGTH 6
#define BIG_DISK_SIZE 0x10000


typedef struct ttr              ttr_t;
typedef struct cchhb            cchhb_t;
typedef struct cchh             cchh_t;
typedef struct labeldate        labeldate_t;
typedef struct volume_label     volume_label_t;
typedef struct cms_volume_label cms_volume_label_t;
typedef struct ldl_volume_label ldl_volume_label_t;
typedef struct extent           extent_t;
typedef struct dev_const        dev_const_t;
typedef struct format1_label    format1_label_t;
typedef struct format4_label    format4_label_t;
typedef struct ds5ext           ds5ext_t;
typedef struct format5_label    format5_label_t;
typedef struct ds7ext           ds7ext_t;
typedef struct format7_label    format7_label_t;

struct __attribute__ ((packed)) ttr {
        u_int16_t tt;
        u_int8_t  r;
};

struct  __attribute__ ((packed)) cchhb {
        u_int16_t cc;
        u_int16_t hh;
        u_int8_t b;
};

struct __attribute__ ((packed)) cchh {
        u_int16_t cc;
        u_int16_t hh;
};

struct __attribute__ ((packed)) labeldate {
        u_int8_t  year;
        u_int16_t day;
};

struct __attribute__ ((packed)) volume_label {
	char volkey[4];         /* volume key = volume label                 */
	char vollbl[4];	        /* volume label ("VOL1" in EBCDIC)           */
	char volid[6];	        /* volume identifier                         */
	u_int8_t security;	/* security byte                             */
	cchhb_t vtoc;           /* VTOC address                              */
	char res1[5];	        /* reserved                                  */
	char cisize[4];	        /* CI-size for FBA,...                       */
                                /* ...blanks for CKD                         */
	char blkperci[4];       /* no of blocks per CI (FBA), blanks for CKD */
	char labperci[4];       /* no of labels per CI (FBA), blanks for CKD */
	char res2[4];	        /* reserved                                  */
	char lvtoc[14];	        /* owner code for LVTOC                      */
	char res3[29];	        /* reserved                                  */
        char fudge[4];          /* filler to match length of ldl label       */
};

struct __attribute__ ((packed)) ldl_volume_label {
	char vollbl[4];         /* Label identifier ("LNX1" in EBCDIC)       */
        char volid[6];          /* Volume identifier                         */
        char res3[69];          /* Reserved field                            */
        char ldl_version[1];    /* Version number, valid for ldl format      */
        u_int64_t formatted_blocks;  /* valid when ldl_version >= "2" (in
                                        EBCDIC)                              */
};

/*
 * See:
 *	z/VM V5R2.0 CMS Planning and Administration
 *	SC24-6078-01
 *	What CMS Does / Disk and File Management / Disk File Format
 * http://publib.boulder.ibm.com/infocenter/zvm/v5r4/topic/com.ibm.zvm.v54.dmsd1/hcsg2b1018.htm
 */
struct __attribute__ ((packed)) cms_volume_label {
	char label_id[4];       /* Label identifier ("CMS1" in EBCDIC)       */
	char vol_id[6];         /* Volume identifier                         */
	char version_id[2];     /* Version identifier ("\0\0")               */
	u_int32_t block_size;   /* Disk block size (512, 1024, 2048 or 4096) */
	u_int32_t origin_ptr;   /* Disk origin pointer (4 or 5)              */
	u_int32_t usable_count; /* Number of usable cylinders/blocks         */
	u_int32_t formatted_count; /* Max # of formatted cylinders/blocks    */
	u_int32_t block_count;  /* Disk size in CMS blocks                   */
	u_int32_t used_count;   /* Number of CMS blocks in use               */
	u_int32_t fst_size;     /* File Status Table (FST) size (64)         */
	u_int32_t fst_count;    /* Number of FSTs per CMS block              */
	char format_date[6];    /* Disk FORMAT date (YYMMDDhhmmss)           */
	char reserved1[2];      /* Reserved fields.
	                           The low-order bit of the first byte is a
	                           century flag.  0 = 1900s, 1 = 2000s.
	                           It is used in conjunction with
	                           "format_date" to determine the
	                           four-digit year.                          */
	u_int32_t disk_offset;  /* Offset in blocks to the start of the
	                           reserved file when the disk is reserved.
	                           This is the number of blocks to skip
	                           before the partition starts.              */
	u_int32_t map_block;    /* Allocation map block with next hole       */
	u_int32_t hblk_disp;    /* Displacement in HBLK data of next hole    */
	u_int32_t user_disp;    /* Disp into user part of allocation map     */
	u_int32_t open_files;   /* Count of SFS open files for this ADT.
	                           open_files is not really part of the
	                           volume label.  It is not used for
	                           minidisks.                                */
	char segment_name[8];   /* Name of the shared segment.
	                           segment_name is not really part of the
	                           volume label.  It is not stored on disk.  */
};

struct __attribute__ ((packed)) extent {
        u_int8_t  typeind;      /* extent type indicator */
        u_int8_t  seqno;        /* extent sequence number */
        cchh_t llimit;          /* starting point of this extent */
        cchh_t ulimit;          /* ending point of this extent */
};

struct __attribute__ ((packed)) dev_const {
        u_int16_t DS4DSCYL;     /* number of logical cyls */
        u_int16_t DS4DSTRK;     /* number of tracks in a logical cylinder  */
        u_int16_t DS4DEVTK;     /* device track length */
        u_int8_t  DS4DEVI;      /* non-last keyed record overhead */
        u_int8_t  DS4DEVL;      /* last keyed record overhead */
        u_int8_t  DS4DEVK;      /* non-keyed record overhead differential */
        u_int8_t  DS4DEVFG;     /* flag byte */
        u_int16_t DS4DEVTL;     /* device tolerance */
        u_int8_t  DS4DEVDT;     /* number of DSCB's per track */
        u_int8_t  DS4DEVDB;     /* number of directory blocks per track */
};

struct __attribute__ ((packed)) format1_label {
	char  DS1DSNAM[44];     /* data set name                           */
	u_int8_t  DS1FMTID;     /* format identifier                       */
	char  DS1DSSN[6];       /* data set serial number                  */
	u_int16_t DS1VOLSQ;     /* volume sequence number                  */
	labeldate_t DS1CREDT;   /* creation date: ydd                      */
	labeldate_t DS1EXPDT;   /* expiration date                         */
	u_int8_t  DS1NOEPV;     /* number of extents on volume             */
	u_int8_t  DS1NOBDB;     /* no. of bytes used in last direction blk */
	u_int8_t  DS1FLAG1;     /* flag 1                                  */
	char  DS1SYSCD[13];     /* system code                             */
	labeldate_t DS1REFD;    /* date last referenced                    */
	u_int8_t  DS1SMSFG;     /* system managed storage indicators       */
	u_int8_t  DS1SCXTF;     /* sec. space extension flag byte          */
	u_int16_t DS1SCXTV;     /* secondary space extension value         */
	u_int8_t  DS1DSRG1;     /* data set organisation byte 1            */
	u_int8_t  DS1DSRG2;     /* data set organisation byte 2            */
  	u_int8_t  DS1RECFM;     /* record format                           */
	u_int8_t  DS1OPTCD;     /* option code                             */
	u_int16_t DS1BLKL;      /* block length                            */
	u_int16_t DS1LRECL;     /* record length                           */
	u_int8_t  DS1KEYL;      /* key length                              */
	u_int16_t DS1RKP;       /* relative key position                   */
	u_int8_t  DS1DSIND;     /* data set indicators                     */
	u_int8_t  DS1SCAL1;     /* secondary allocation flag byte          */
  	char DS1SCAL3[3];       /* secondary allocation quantity           */
	ttr_t DS1LSTAR;         /* last used track and block on track      */
	u_int16_t DS1TRBAL;     /* space remaining on last used track      */
	u_int16_t res1;         /* reserved                                */
	extent_t DS1EXT1;       /* first extent description                */
	extent_t DS1EXT2;       /* second extent description               */
	extent_t DS1EXT3;       /* third extent description                */
	cchhb_t DS1PTRDS;       /* possible pointer to f2 or f3 DSCB       */
};

struct __attribute__ ((packed)) format4_label {
	char  DS4KEYCD[44];     /* key code for VTOC labels: 44 times 0x04 */
	u_int8_t  DS4IDFMT;     /* format identifier                       */
	cchhb_t DS4HPCHR;       /* highest address of a format 1 DSCB      */
	u_int16_t DS4DSREC;     /* number of available DSCB's              */
	cchh_t DS4HCCHH;        /* CCHH of next available alternate track  */
	u_int16_t DS4NOATK;     /* number of remaining alternate tracks    */
	u_int8_t  DS4VTOCI;     /* VTOC indicators                         */
	u_int8_t  DS4NOEXT;     /* number of extents in VTOC               */
	u_int8_t  DS4SMSFG;     /* system managed storage indicators       */
	u_int8_t  DS4DEVAC;     /* number of alternate cylinders.
                                   Subtract from first two bytes of
                                   DS4DEVSZ to get number of usable
	                           cylinders. can be zero. valid
	                           only if DS4DEVAV on.                    */
	dev_const_t DS4DEVCT;   /* device constants                        */
	char DS4AMTIM[8];       /* VSAM time stamp                         */
	char DS4AMCAT[3];       /* VSAM catalog indicator                  */
	char DS4R2TIM[8];       /* VSAM volume/catalog match time stamp    */
	char res1[5];           /* reserved                                */
	char DS4F6PTR[5];       /* pointer to first format 6 DSCB          */
	extent_t DS4VTOCE;      /* VTOC extent description                 */
	char res2[10];          /* reserved                                */
	u_int8_t DS4EFLVL;      /* extended free-space management level    */
	cchhb_t DS4EFPTR;       /* pointer to extended free-space info     */
	char res3[9];           /* reserved                                */
};

struct __attribute__ ((packed)) ds5ext {
	u_int16_t t;            /* RTA of the first track of free extent   */
	u_int16_t fc;           /* number of whole cylinders in free ext.  */
	u_int8_t  ft;           /* number of remaining free tracks         */
};

struct __attribute__ ((packed)) format5_label {
	char DS5KEYID[4];       /* key identifier                          */
	ds5ext_t DS5AVEXT;      /* first available (free-space) extent.    */
	ds5ext_t DS5EXTAV[7];   /* seven available extents                 */
	u_int8_t DS5FMTID;      /* format identifier                       */
	ds5ext_t DS5MAVET[18];  /* eighteen available extents              */
	cchhb_t DS5PTRDS;       /* pointer to next format5 DSCB            */
};

struct __attribute__ ((packed)) ds7ext {
	u_int32_t a;            /* starting RTA value                      */
	u_int32_t b;            /* ending RTA value + 1                    */
};

struct __attribute__ ((packed)) format7_label {
	char DS7KEYID[4];       /* key identifier                          */
	ds7ext_t DS7EXTNT[5];   /* space for 5 extent descriptions         */
	u_int8_t DS7FMTID;      /* format identifier                       */
	ds7ext_t DS7ADEXT[11];  /* space for 11 extent descriptions        */
	char res1[2];           /* reserved                                */
	cchhb_t DS7PTRDS;       /* pointer to next FMT7 DSCB               */
};

char *vtoc_ebcdic_enc (char const *source, char *target, int l);
char *vtoc_ebcdic_dec (char const *source, char *target, int l);
void vtoc_set_extent (extent_t *ext, u_int8_t typeind, u_int8_t seqno,
                      cchh_t *lower, cchh_t *upper);
void vtoc_set_cchh (cchh_t *addr, u_int16_t cc, u_int16_t hh);
void vtoc_set_cchhb (cchhb_t *addr, u_int16_t cc, u_int16_t hh, u_int8_t b);
void vtoc_set_date (labeldate_t *d, u_int8_t year, u_int16_t day);

void vtoc_volume_label_init (volume_label_t *vlabel);

int vtoc_read_volume_label (int fd, unsigned long vlabel_start,
                            volume_label_t *vlabel);

int vtoc_write_volume_label (int fd, unsigned long vlabel_start,
                             volume_label_t const *vlabel);

void vtoc_volume_label_set_volser (volume_label_t *vlabel, char const *volser);

char *vtoc_volume_label_get_volser (volume_label_t *vlabel, char *volser);

void vtoc_volume_label_set_key (volume_label_t *vlabel, char const *key);

void vtoc_volume_label_set_label (volume_label_t *vlabel, char const *lbl);

char *vtoc_volume_label_get_label (volume_label_t *vlabel, char *lbl);

void vtoc_read_label (int fd, unsigned long position, format1_label_t *f1,
                      format4_label_t *f4, format5_label_t *f5,
                      format7_label_t *f7);

void vtoc_write_label (int fd, unsigned long position,
		       format1_label_t const *f1,
                       format4_label_t const *f4,
		       format5_label_t const *f5,
                       format7_label_t const *f7);

void vtoc_init_format1_label (char *volid, unsigned int blksize,
                              extent_t *part_extent, format1_label_t *f1);

void vtoc_init_format4_label (format4_label_t *f4lbl,
                              unsigned int usable_partitions,
                              unsigned int cylinders,
                              unsigned int tracks,
                              unsigned int blocks,
                              unsigned int blksize,
                              u_int16_t dev_type);

void vtoc_update_format4_label (format4_label_t *f4, cchhb_t *highest_f1,
                                u_int16_t unused_update);

void vtoc_init_format5_label (format5_label_t *f5);

void vtoc_update_format5_label_add (format5_label_t *f5, int verbose, int cyl,
                                    int trk, u_int16_t a, u_int16_t b,
                                    u_int8_t c);

void vtoc_update_format5_label_del (format5_label_t *f5, int verbose, int cyl,
                                    int trk, u_int16_t a, u_int16_t b,
                                    u_int8_t c);

void vtoc_init_format7_label (format7_label_t *f7);

void vtoc_update_format7_label_add (format7_label_t *f7, int verbose,
                                    u_int32_t a, u_int32_t b);

void vtoc_update_format7_label_del (format7_label_t *f7, int verbose,
                                    u_int32_t a, u_int32_t b);

void vtoc_set_freespace(format4_label_t *f4, format5_label_t *f5,
                        format7_label_t *f7, char ch, int verbose,
                        u_int32_t start, u_int32_t stop, int cyl, int trk);

#endif /* VTOC_H */
