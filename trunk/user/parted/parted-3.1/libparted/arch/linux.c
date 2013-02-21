/* libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define PROC_DEVICES_BUFSIZ 16384

#include <config.h>
#include <arch/linux.h>
#include <linux/blkpg.h>
#include <parted/parted.h>
#include <parted/debug.h>
#if defined __s390__ || defined __s390x__
#include <parted/fdasd.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>        /* for uname() */
#include <scsi/scsi.h>
#include <assert.h>
#ifdef ENABLE_DEVICE_MAPPER
#include <libdevmapper.h>
#endif

#include "../architecture.h"
#include "dirname.h"
#include "xstrtol.h"

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

/* The __attribute__ feature is available in gcc versions 2.5 and later.
   The __-protected variants of the attributes 'format' and 'printf' are
   accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
# define _GL_ATTRIBUTE_FORMAT(spec) __attribute__ ((__format__ spec))
#else
# define _GL_ATTRIBUTE_FORMAT(spec) /* empty */
#endif

#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

#ifndef __NR__llseek
#define __NR__llseek 140
#endif

#ifndef SCSI_IOCTL_SEND_COMMAND
#define SCSI_IOCTL_SEND_COMMAND 1
#endif

/* from <linux/hdreg.h> */
#define HDIO_GETGEO             0x0301  /* get device geometry */
#define HDIO_GET_IDENTITY       0x030d  /* get IDE identification info */

#define RD_MODE (O_RDONLY)
#define WR_MODE (O_WRONLY)
#define RW_MODE (O_RDWR)

struct hd_geometry {
        unsigned char heads;
        unsigned char sectors;
        unsigned short cylinders;
        unsigned long start;
};

struct ata7_sectinfo {
        int valid1:1;
        int valid2:1;
        int rsv:26;
        int multiplier:4;
};

/* structure returned by HDIO_GET_IDENTITY, as per ANSI ATA2 rev.2f spec */
struct hd_driveid {
        unsigned short  config;         /* lots of obsolete bit flags */
        unsigned short  cyls;           /* "physical" cyls */
        unsigned short  reserved2;      /* reserved (word 2) */
        unsigned short  heads;          /* "physical" heads */
        unsigned short  track_bytes;    /* unformatted bytes per track */
        unsigned short  sector_bytes;   /* unformatted bytes per sector */
        unsigned short  sectors;        /* "physical" sectors per track */
        unsigned short  vendor0;        /* vendor unique */
        unsigned short  vendor1;        /* vendor unique */
        unsigned short  vendor2;        /* vendor unique */
        unsigned char   serial_no[20];  /* 0 = not_specified */
        unsigned short  buf_type;
        unsigned short  buf_size;       /* 512 byte increments;
                                                         0 = not_specified */
        unsigned short  ecc_bytes;      /* for r/w long cmds;
                                                         0 = not_specified */
        unsigned char   fw_rev[8];      /* 0 = not_specified */
        char            model[40];      /* 0 = not_specified */
        unsigned char   max_multsect;   /* 0=not_implemented */
        unsigned char   vendor3;        /* vendor unique */
        unsigned short  dword_io;       /* 0=not_implemented; 1=implemented */
        unsigned char   vendor4;        /* vendor unique */
        unsigned char   capability;     /* bits 0:DMA 1:LBA 2:IORDYsw
                                                3:IORDYsup*/
        unsigned short  reserved50;     /* reserved (word 50) */
        unsigned char   vendor5;        /* vendor unique */
        unsigned char   tPIO;           /* 0=slow, 1=medium, 2=fast */
        unsigned char   vendor6;        /* vendor unique */
        unsigned char   tDMA;           /* 0=slow, 1=medium, 2=fast */
        unsigned short  field_valid;    /* bits 0:cur_ok 1:eide_ok */
        unsigned short  cur_cyls;       /* logical cylinders */
        unsigned short  cur_heads;      /* logical heads */
        unsigned short  cur_sectors;    /* logical sectors per track */
        unsigned short  cur_capacity0;  /* logical total sectors on drive */
        unsigned short  cur_capacity1;  /*  (2 words, misaligned int)     */
        unsigned char   multsect;       /* current multiple sector count */
        unsigned char   multsect_valid; /* when (bit0==1) multsect is ok */
        unsigned int    lba_capacity;   /* total number of sectors */
        unsigned short  dma_1word;      /* single-word dma info */
        unsigned short  dma_mword;      /* multiple-word dma info */
        unsigned short  eide_pio_modes; /* bits 0:mode3 1:mode4 */
        unsigned short  eide_dma_min;   /* min mword dma cycle time (ns) */
        unsigned short  eide_dma_time;  /* recommended mword dma cycle
                                           time (ns) */
        unsigned short  eide_pio;       /* min cycle time (ns), no IORDY  */
        unsigned short  eide_pio_iordy; /* min cycle time (ns), with IORDY */
        unsigned short  words69_70[2];  /* reserved words 69-70 */
        /* HDIO_GET_IDENTITY currently returns only words 0 through 70 */
        unsigned short  words71_74[4];  /* reserved words 71-74 */
        unsigned short  queue_depth;    /*  */
        unsigned short  words76_79[4];  /* reserved words 76-79 */
        unsigned short  major_rev_num;  /*  */
        unsigned short  minor_rev_num;  /*  */
        unsigned short  command_set_1;  /* bits 0:Smart 1:Security 2:Removable
                                                3:PM */
        unsigned short  command_set_2;  /* bits 14:Smart Enabled 13:0 zero */
        unsigned short  cfsse;          /* command set-feature supported
                                           extensions */
        unsigned short  cfs_enable_1;   /* command set-feature enabled */
        unsigned short  cfs_enable_2;   /* command set-feature enabled */
        unsigned short  csf_default;    /* command set-feature default */
        unsigned short  dma_ultra;      /*  */
        unsigned short  word89;         /* reserved (word 89) */
        unsigned short  word90;         /* reserved (word 90) */
        unsigned short  CurAPMvalues;   /* current APM values */
        unsigned short  word92;         /* reserved (word 92) */
        unsigned short  hw_config;      /* hardware config */
        unsigned short  words94_105[12];/* reserved words 94-105 */
        struct ata7_sectinfo ata7_sectinfo; /* ATAPI/ATA7 physical and logical
                                               sector size */
        unsigned short  words107_116[10];/* reserved words 107-116 */
        unsigned int    logical_sectsize;/* ATAPI/ATA7 logical sector size */
        unsigned short  words119_125[7];/* reserved words 119-125 */
        unsigned short  last_lun;       /* reserved (word 126) */
        unsigned short  word127;        /* reserved (word 127) */
        unsigned short  dlf;            /* device lock function
                                         * 15:9 reserved
                                         * 8    security level 1:max 0:high
                                         * 7:6  reserved
                                         * 5    enhanced erase
                                         * 4    expire
                                         * 3    frozen
                                         * 2    locked
                                         * 1    en/disabled
                                         * 0    capability
                                         */
        unsigned short  csfo;           /* current set features options
                                         * 15:4 reserved
                                         * 3    auto reassign
                                         * 2    reverting
                                         * 1    read-look-ahead
                                         * 0    write cache
                                         */
        unsigned short  words130_155[26];/* reserved vendor words 130-155 */
        unsigned short  word156;
        unsigned short  words157_159[3]; /* reserved vendor words 157-159 */
        unsigned short  words160_255[95];/* reserved words 160-255 */
};

/* from <linux/fs.h> */
#define BLKRRPART  _IO(0x12,95) /* re-read partition table */
#define BLKGETSIZE _IO(0x12,96) /* return device size */
#define BLKFLSBUF  _IO(0x12,97) /* flush buffer cache */
#define BLKSSZGET  _IO(0x12,104) /* get block device sector size */
#define BLKGETLASTSECT  _IO(0x12,108) /* get last sector of block device */
#define BLKSETLASTSECT  _IO(0x12,109) /* set last sector of block device */

/* return device size in bytes (u64 *arg) */
#define BLKGETSIZE64 _IOR(0x12,114,size_t)

struct blkdev_ioctl_param {
        unsigned int block;
        size_t content_length;
        char * block_contents;
};

/* from <linux/major.h> */
#define IDE0_MAJOR              3
#define IDE1_MAJOR              22
#define IDE2_MAJOR              33
#define IDE3_MAJOR              34
#define IDE4_MAJOR              56
#define IDE5_MAJOR              57
#define SCSI_CDROM_MAJOR        11
#define SCSI_DISK0_MAJOR        8
#define SCSI_DISK1_MAJOR        65
#define SCSI_DISK2_MAJOR        66
#define SCSI_DISK3_MAJOR        67
#define SCSI_DISK4_MAJOR        68
#define SCSI_DISK5_MAJOR        69
#define SCSI_DISK6_MAJOR        70
#define SCSI_DISK7_MAJOR        71
#define SCSI_DISK8_MAJOR        128
#define SCSI_DISK9_MAJOR        129
#define SCSI_DISK10_MAJOR       130
#define SCSI_DISK11_MAJOR       131
#define SCSI_DISK12_MAJOR       132
#define SCSI_DISK13_MAJOR       133
#define SCSI_DISK14_MAJOR       134
#define SCSI_DISK15_MAJOR       135
#define COMPAQ_SMART2_MAJOR     72
#define COMPAQ_SMART2_MAJOR1    73
#define COMPAQ_SMART2_MAJOR2    74
#define COMPAQ_SMART2_MAJOR3    75
#define COMPAQ_SMART2_MAJOR4    76
#define COMPAQ_SMART2_MAJOR5    77
#define COMPAQ_SMART2_MAJOR6    78
#define COMPAQ_SMART2_MAJOR7    79
#define COMPAQ_SMART_MAJOR      104
#define COMPAQ_SMART_MAJOR1     105
#define COMPAQ_SMART_MAJOR2     106
#define COMPAQ_SMART_MAJOR3     107
#define COMPAQ_SMART_MAJOR4     108
#define COMPAQ_SMART_MAJOR5     109
#define COMPAQ_SMART_MAJOR6     110
#define COMPAQ_SMART_MAJOR7     111
#define DAC960_MAJOR            48
#define ATARAID_MAJOR           114
#define I2O_MAJOR1              80
#define I2O_MAJOR2              81
#define I2O_MAJOR3              82
#define I2O_MAJOR4              83
#define I2O_MAJOR5              84
#define I2O_MAJOR6              85
#define I2O_MAJOR7              86
#define I2O_MAJOR8              87
#define UBD_MAJOR               98
#define DASD_MAJOR              94
#define VIODASD_MAJOR           112
#define AOE_MAJOR               152
#define SX8_MAJOR1              160
#define SX8_MAJOR2              161
#define XVD_MAJOR               202
#define SDMMC_MAJOR             179
#define LOOP_MAJOR              7
#define MD_MAJOR                9

#define SCSI_BLK_MAJOR(M) (                                             \
                (M) == SCSI_DISK0_MAJOR                                 \
                || (M) == SCSI_CDROM_MAJOR                              \
                || ((M) >= SCSI_DISK1_MAJOR && (M) <= SCSI_DISK7_MAJOR) \
                || ((M) >= SCSI_DISK8_MAJOR && (M) <= SCSI_DISK15_MAJOR))

/* Maximum number of partitions supported by linux. */
#define MAX_NUM_PARTS		64

static char* _device_get_part_path (PedDevice* dev, int num);
static int _partition_is_mounted_by_path (const char* path);

static int
_read_fd (int fd, char **buf)
{
        char* p;
        size_t size = PROC_DEVICES_BUFSIZ;
        int s, filesize = 0;

        *buf = malloc (size * sizeof (char));
        if (*buf == 0) {
                return -1;
        }

        do {
                p = &(*buf) [filesize];
                s = read (fd, p, PROC_DEVICES_BUFSIZ);
                /* exit if there is an error or EOF is reached */
                if (s <= 0)
                        break;
                filesize += s;
                size += s;
                char *new_buf = realloc (*buf, size);
                if (new_buf == NULL) {
                        int saved_errno = errno;
                        free (*buf);
                        errno = saved_errno;
                        return -1;
                }
                *buf = new_buf;
        } while (1);

        if (filesize == 0 && s < 0) {
                free (*buf);
                *buf = NULL;
                return -1;
        } else {
                char *new_buf = realloc (*buf, filesize + 1);
                if (new_buf == NULL) {
                        int saved_errno = errno;
                        free (*buf);
                        errno = saved_errno;
                        return -1;
                }
                *buf = new_buf;
                (*buf)[filesize] = '\0';
        }

        return filesize;
}

static int
_major_type_in_devices (int major, const char* type)
{
        int fd;
        char* buf = NULL;
        char* line;
        char* end;
        int bd = 0;
        char c;

        fd = open ("/proc/devices", O_RDONLY);
        if (fd < 0)
                return 0;

        if (_read_fd(fd, &buf) < 0) {
                close(fd);
                return 0;
        }

        line = buf;
        end = strchr(line, '\n');
        while (end) {
                char *name;
                int maj;

                c = *end;
                *end = '\0';

                if (!bd) {
                        if (!strncmp(line, "Block devices:", 14))
                                bd = 1;
                        goto next;
                }

                name = strrchr(line, ' ');
                if (!name || strcmp(name+1, type))
                        goto next;

                maj = strtol(line, &name, 10);
                if (maj == major) {
                        free(buf);
                        close(fd);
                        return 1;
                }

next:
                *end = c;
                line = end+1;
                end = strchr(line, '\n');
        }
        free(buf);
        close(fd);
        return 0;
}

static int
_is_ide_major (int major)
{
        switch (major) {
                case IDE0_MAJOR:
                case IDE1_MAJOR:
                case IDE2_MAJOR:
                case IDE3_MAJOR:
                case IDE4_MAJOR:
                case IDE5_MAJOR:
                        return 1;

                default:
                        return 0;
        }
}

static int
_is_cpqarray_major (int major)
{
        return ((COMPAQ_SMART2_MAJOR <= major && major <= COMPAQ_SMART2_MAJOR7)
             || (COMPAQ_SMART_MAJOR <= major && major <= COMPAQ_SMART_MAJOR7));
}

static int
_is_i2o_major (int major)
{
        return (I2O_MAJOR1 <= major && major <= I2O_MAJOR8);
}

static int
_is_sx8_major (int major)
{
        return (SX8_MAJOR1 <= major && major <= SX8_MAJOR2);
}

static int
_is_virtblk_major (int major)
{
        return _major_type_in_devices (major, "virtblk");
}

#ifdef ENABLE_DEVICE_MAPPER
static int
_is_dm_major (int major)
{
        return _major_type_in_devices (major, "device-mapper");
}

static int
_dm_maptype (PedDevice *dev)
{
        LinuxSpecific*  arch_specific = LINUX_SPECIFIC (dev);
        struct dm_task *dmt;
        uint64_t start, length;
        char *target_type = NULL;
        char *params;
        int r = -1;
        const char* dev_dir = getenv ("DM_DEV_DIR");

        if (dev_dir && *dev_dir && !dm_set_dev_dir(dev_dir))
                return r;

        if (!(dmt = dm_task_create(DM_DEVICE_TABLE)))
                return r;

        if (!dm_task_set_major_minor(dmt, arch_specific->major,
                                     arch_specific->minor, 0))
                goto bad;

        dm_task_no_open_count(dmt);

        if (!dm_task_run(dmt))
                goto bad;

        dm_get_next_target(dmt, NULL, &start, &length, &target_type, &params);

        arch_specific->dmtype = strdup(target_type ? target_type : "NO-TARGET");
        if (arch_specific->dmtype == NULL)
                goto bad;
        r = 0;
bad:
        dm_task_destroy(dmt);
        return r;
}


static int
_probe_dm_devices ()
{
       DIR*            mapper_dir;
       struct dirent*  dent;
       char            buf [512];      /* readdir(3) claims d_name[256] */
       struct stat     st;

       mapper_dir = opendir ("/dev/mapper");
       if (!mapper_dir)
               return 0;

       /* Search the /dev/mapper directory for devices w/ the same major
        * number that was returned from _probe_lvm_major().
        */
       while ((dent = readdir (mapper_dir))) {
               if (strcmp (dent->d_name, ".")  == 0 ||
                   strcmp (dent->d_name, "..") == 0)
                       continue;

               snprintf (buf, sizeof (buf), "/dev/mapper/%s", dent->d_name);

               if (stat (buf, &st) != 0)
                       continue;

               if (_is_dm_major(major(st.st_rdev)))
                       _ped_device_probe (buf);
       }
       closedir (mapper_dir);

       return 1;
}
#endif

static int
_device_stat (PedDevice* dev, struct stat * dev_stat)
{
        PED_ASSERT (dev != NULL);
        PED_ASSERT (!dev->external_mode);

        while (1) {
                if (!stat (dev->path, dev_stat)) {
                        return 1;
                } else {
                        if (ped_exception_throw (
                                PED_EXCEPTION_ERROR,
                                PED_EXCEPTION_RETRY_CANCEL,
                                _("Could not stat device %s - %s."),
                                dev->path,
                                strerror (errno))
                                        != PED_EXCEPTION_RETRY)
                                return 0;
                }
        }
}

static int
_device_probe_type (PedDevice* dev)
{
        struct stat             dev_stat;
        int                     dev_major;
        int                     dev_minor;
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);

        if (!_device_stat (dev, &dev_stat))
                return 0;

        if (!S_ISBLK(dev_stat.st_mode)) {
                dev->type = PED_DEVICE_FILE;
                return 1;
        }

        arch_specific->major = dev_major = major (dev_stat.st_rdev);
        arch_specific->minor = dev_minor = minor (dev_stat.st_rdev);

        if (SCSI_BLK_MAJOR (dev_major) && (dev_minor % 0x10 == 0)) {
                dev->type = PED_DEVICE_SCSI;
        } else if (_is_ide_major (dev_major) && (dev_minor % 0x40 == 0)) {
                dev->type = PED_DEVICE_IDE;
        } else if (dev_major == DAC960_MAJOR && (dev_minor % 0x8 == 0)) {
                dev->type = PED_DEVICE_DAC960;
        } else if (dev_major == ATARAID_MAJOR && (dev_minor % 0x10 == 0)) {
                dev->type = PED_DEVICE_ATARAID;
        } else if (dev_major == AOE_MAJOR && (dev_minor % 0x10 == 0)) {
                dev->type = PED_DEVICE_AOE;
        } else if (dev_major == DASD_MAJOR && (dev_minor % 0x4 == 0)) {
                dev->type = PED_DEVICE_DASD;
        } else if (dev_major == VIODASD_MAJOR && (dev_minor % 0x8 == 0)) {
                dev->type = PED_DEVICE_VIODASD;
        } else if (_is_sx8_major(dev_major) && (dev_minor % 0x20 == 0)) {
                dev->type = PED_DEVICE_SX8;
        } else if (_is_i2o_major (dev_major) && (dev_minor % 0x10 == 0)) {
                dev->type = PED_DEVICE_I2O;
        } else if (_is_cpqarray_major (dev_major) && (dev_minor % 0x10 == 0)) {
                dev->type = PED_DEVICE_CPQARRAY;
        } else if (dev_major == UBD_MAJOR && (dev_minor % 0x10 == 0)) {
                dev->type = PED_DEVICE_UBD;
#ifdef ENABLE_DEVICE_MAPPER
        } else if (_is_dm_major(dev_major)) {
                dev->type = PED_DEVICE_DM;
                if (_dm_maptype(dev)) {
                        ped_exception_throw (
                                PED_EXCEPTION_BUG,
                                PED_EXCEPTION_CANCEL,
                                _("Unable to determine the dm type of %s."),
                                dev->path);
                }
#endif
        } else if (dev_major == XVD_MAJOR && (dev_minor % 0x10 == 0)) {
                dev->type = PED_DEVICE_XVD;
        } else if (dev_major == SDMMC_MAJOR && (dev_minor % 0x08 == 0)) {
                dev->type = PED_DEVICE_SDMMC;
        } else if (_is_virtblk_major(dev_major)) {
                dev->type = PED_DEVICE_VIRTBLK;
        } else if (dev_major == LOOP_MAJOR) {
                dev->type = PED_DEVICE_LOOP;
        } else if (dev_major == MD_MAJOR) {
                dev->type = PED_DEVICE_MD;
        } else {
                dev->type = PED_DEVICE_UNKNOWN;
        }

        return 1;
}

static int
_get_linux_version ()
{
        static int kver = -1;

        struct utsname uts;
        int major = 0;
        int minor = 0;
        int teeny = 0;

        if (kver != -1)
                return kver;

        if (uname (&uts))
                return kver = 0;
        int n = sscanf (uts.release, "%u.%u.%u", &major, &minor, &teeny);
        assert (n == 2 || n == 3);
        return kver = KERNEL_VERSION (major, minor, teeny);
}

static int
_have_kern26 ()
{
        static int have_kern26 = -1;
        int kver;

        if (have_kern26 != -1)
                return have_kern26;

        kver = _get_linux_version();
        return have_kern26 = kver >= KERNEL_VERSION (2,6,0) ? 1 : 0;
}

#if USE_BLKID
static void
get_blkid_topology (LinuxSpecific *arch_specific)
{
        arch_specific->probe = blkid_new_probe ();
        if (!arch_specific->probe)
                return;

        if (blkid_probe_set_device(arch_specific->probe,
                                   arch_specific->fd, 0, 0))
                return;

        arch_specific->topology =
                blkid_probe_get_topology(arch_specific->probe);
}
#endif

static void
_device_set_sector_size (PedDevice* dev)
{
        LinuxSpecific*  arch_specific = LINUX_SPECIFIC (dev);
        int sector_size;

        dev->sector_size = PED_SECTOR_SIZE_DEFAULT;
        dev->phys_sector_size = PED_SECTOR_SIZE_DEFAULT;

        PED_ASSERT (dev->open_count);

        if (_get_linux_version() < KERNEL_VERSION (2,3,0)) {
                dev->sector_size = PED_SECTOR_SIZE_DEFAULT;
                return;
        }

        if (ioctl (arch_specific->fd, BLKSSZGET, &sector_size)) {
                ped_exception_throw (
                        PED_EXCEPTION_WARNING,
                        PED_EXCEPTION_OK,
                        _("Could not determine sector size for %s: %s.\n"
                          "Using the default sector size (%lld)."),
                        dev->path, strerror (errno), PED_SECTOR_SIZE_DEFAULT);
        } else {
                dev->sector_size = (long long)sector_size;
                dev->phys_sector_size = dev->sector_size;
        }

#if USE_BLKID
        get_blkid_topology(arch_specific);
        if (!arch_specific->topology) {
                dev->phys_sector_size = 0;
        } else {
                dev->phys_sector_size =
                        blkid_topology_get_physical_sector_size(
                                arch_specific->topology);
        }
        if (dev->phys_sector_size == 0) {
                ped_exception_throw (
                        PED_EXCEPTION_WARNING,
                        PED_EXCEPTION_OK,
                        _("Could not determine physical sector size for %s.\n"
                          "Using the logical sector size (%lld)."),
                        dev->path, dev->sector_size);
                dev->phys_sector_size = dev->sector_size;
        }
#endif

#if defined __s390__ || defined __s390x__
        /* Return PED_SECTOR_SIZE_DEFAULT for DASDs. */
        if (dev->type == PED_DEVICE_DASD) {
                arch_specific->real_sector_size = dev->sector_size;
                dev->sector_size = PED_SECTOR_SIZE_DEFAULT;
        }
#endif
}

static int
_kernel_has_blkgetsize64(void)
{
        int version = _get_linux_version();

        if (version >= KERNEL_VERSION (2,5,4)) return 1;
        if (version <  KERNEL_VERSION (2,5,0) &&
            version >= KERNEL_VERSION (2,4,18)) return 1;
        return 0;
}

/* TODO: do a binary search if BLKGETSIZE doesn't work?! */
static PedSector
_device_get_length (PedDevice* dev)
{
        unsigned long           size;
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);
        uint64_t bytes=0;
        const char*             test_str;
        PedSector               test_size;


        PED_ASSERT (dev->open_count > 0);
        PED_ASSERT (dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

        test_str = getenv ("PARTED_TEST_DEVICE_LENGTH");
        if (test_str
            && xstrtoll (test_str, NULL, 10, &test_size, NULL) == LONGINT_OK)
                return test_size;

        if (_kernel_has_blkgetsize64()) {
                if (ioctl(arch_specific->fd, BLKGETSIZE64, &bytes) == 0) {
                        return bytes / dev->sector_size;
                }
        }

        if (ioctl (arch_specific->fd, BLKGETSIZE, &size)) {
                ped_exception_throw (
                        PED_EXCEPTION_BUG,
                        PED_EXCEPTION_CANCEL,
                        _("Unable to determine the size of %s (%s)."),
                        dev->path,
                        strerror (errno));
                return 0;
        }

        return size;
}

static int
_device_probe_geometry (PedDevice* dev)
{
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);
        struct stat             dev_stat;
        struct hd_geometry      geometry;

        if (!_device_stat (dev, &dev_stat))
                return 0;
        PED_ASSERT (S_ISBLK (dev_stat.st_mode));

        _device_set_sector_size (dev);

        dev->length = _device_get_length (dev);
        if (!dev->length)
                return 0;

        /* The GETGEO ioctl is no longer useful (as of linux 2.6.x).  We could
         * still use it in 2.4.x, but this is contentious.  Perhaps we should
         * move to EDD. */
        dev->bios_geom.sectors = 63;
        dev->bios_geom.heads = 255;
        dev->bios_geom.cylinders
                = dev->length / (63 * 255);

        /* FIXME: what should we put here?  (TODO: discuss on linux-kernel) */
        if (!ioctl (arch_specific->fd, HDIO_GETGEO, &geometry)
                        && geometry.sectors && geometry.heads) {
                dev->hw_geom.sectors = geometry.sectors;
                dev->hw_geom.heads = geometry.heads;
                dev->hw_geom.cylinders
                        = dev->length / (dev->hw_geom.heads
                                         * dev->hw_geom.sectors);
        } else {
                dev->hw_geom = dev->bios_geom;
        }

        return 1;
}

static char*
strip_name(char* str)
{
        int     i;
        int     end = 0;

        for (i = 0; str[i] != 0; i++) {
                if (!isspace (str[i])
                    || (isspace (str[i]) && !isspace (str[i+1]) && str[i+1])) {
                        str [end] = str[i];
                        end++;
                }
        }
        str[end] = 0;
        return strdup (str);
}

static int
init_ide (PedDevice* dev)
{
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);
        struct stat             dev_stat;
        struct hd_driveid       hdi;
        PedExceptionOption      ex_status;
        char                    hdi_buf[41];
        int                     sector_multiplier = 0;

        if (!_device_stat (dev, &dev_stat))
                goto error;

        if (!ped_device_open (dev))
                goto error;

        if (ioctl (arch_specific->fd, HDIO_GET_IDENTITY, &hdi)) {
                ex_status = ped_exception_throw (
                                PED_EXCEPTION_WARNING,
                                PED_EXCEPTION_IGNORE_CANCEL,
                                _("Could not get identity of device %s - %s"),
                                dev->path, strerror (errno));
                switch (ex_status) {
                        case PED_EXCEPTION_CANCEL:
                                goto error_close_dev;

                        case PED_EXCEPTION_UNHANDLED:
                                ped_exception_catch ();
                        case PED_EXCEPTION_IGNORE:
                                dev->model = strdup(_("Generic IDE"));
                                break;
                        default:
                                PED_ASSERT (0);
                                break;
                }
        } else {
                /* hdi.model is not guaranteed to be NULL terminated */
                memcpy (hdi_buf, hdi.model, 40);
                hdi_buf[40] = '\0';
                dev->model = strip_name (hdi_buf);

                if (!hdi.ata7_sectinfo.valid1 && hdi.ata7_sectinfo.valid2)
                        sector_multiplier = hdi.ata7_sectinfo.multiplier;
                else
                        sector_multiplier = 1;

                if (sector_multiplier != 1) {
                        ex_status = ped_exception_throw (
                                PED_EXCEPTION_WARNING,
                                PED_EXCEPTION_IGNORE_CANCEL,
                                _("Device %s has multiple (%d) logical sectors "
                                  "per physical sector.\n"
                                  "GNU Parted supports this EXPERIMENTALLY for "
                                  "some special disk label/file system "
                                  "combinations, e.g. GPT and ext2/3.\n"
                                  "Please consult the web site for up-to-date "
                                  "information."),
                                dev->path, sector_multiplier);

                        switch (ex_status) {
                                case PED_EXCEPTION_CANCEL:
                                        goto error_close_dev;

                                case PED_EXCEPTION_UNHANDLED:
                                        ped_exception_catch ();
                                case PED_EXCEPTION_IGNORE:
                                        break;
                                default:
                                        PED_ASSERT (0);
                                        break;
                        }
                }

                /* XXX sector_size has not been set yet! */
                /* dev->phys_sector_size = dev->sector_size
                   * sector_multiplier;*/
                dev->phys_sector_size = PED_SECTOR_SIZE_DEFAULT;
        }

        if (!_device_probe_geometry (dev))
                goto error_close_dev;

        ped_device_close (dev);
        return 1;

error_close_dev:
        ped_device_close (dev);
error:
        return 0;
}

/* This function reads the /sys entry named "file" for device "dev". */
static char *
read_device_sysfs_file (PedDevice *dev, const char *file)
{
        FILE *f;
        char name_buf[128];
        char buf[256];

        snprintf (name_buf, 127, "/sys/block/%s/device/%s",
                  last_component (dev->path), file);

        if ((f = fopen (name_buf, "r")) == NULL)
                return NULL;

        if (fgets (buf, 255, f) == NULL) {
                fclose (f);
                return NULL;
        }

        fclose (f);
        return strip_name (buf);
}

/* This function sends a query to a SCSI device for vendor and product
 * information.  It uses the deprecated SCSI_IOCTL_SEND_COMMAND to
 * issue this query.
 */
static int
scsi_query_product_info (PedDevice* dev, char **vendor, char **product)
{
        /* The following are defined by the SCSI-2 specification. */
        typedef struct _scsi_inquiry_cmd
        {
                uint8_t op;
                uint8_t lun;          /* bits 5-7 denote the LUN */
                uint8_t page_code;
                uint8_t reserved;
                uint8_t alloc_length;
                uint8_t control;
        } __attribute__((packed)) scsi_inquiry_cmd_t;

        typedef struct _scsi_inquiry_data
        {
                uint8_t peripheral_info;
                uint8_t device_info;
                uint8_t version_info;
                uint8_t _field1;
                uint8_t additional_length;
                uint8_t _reserved1;
                uint8_t _reserved2;
                uint8_t _field2;
                uint8_t vendor_id[8];
                uint8_t product_id[16];
                uint8_t product_revision[4];
                uint8_t vendor_specific[20];
                uint8_t _reserved3[40];
        } __attribute__((packed)) scsi_inquiry_data_t;

        struct scsi_arg
        {
                unsigned int inlen;
                unsigned int outlen;

                union arg_data
                {
                        scsi_inquiry_data_t out;
                        scsi_inquiry_cmd_t  in;
                } data;
        } arg;

        LinuxSpecific* arch_specific = LINUX_SPECIFIC (dev);
        char    buf[32];

        *vendor = NULL;
        *product = NULL;

        memset (&arg, 0x00, sizeof(struct scsi_arg));
        arg.inlen  = 0;
        arg.outlen = sizeof(scsi_inquiry_data_t);
        arg.data.in.op  = INQUIRY;
        arg.data.in.lun = dev->host << 5;
        arg.data.in.alloc_length = sizeof(scsi_inquiry_data_t);
        arg.data.in.page_code = 0;
        arg.data.in.reserved = 0;
        arg.data.in.control = 0;

        if (ioctl (arch_specific->fd, SCSI_IOCTL_SEND_COMMAND, &arg) < 0)
                return 0;

        memcpy (buf, arg.data.out.vendor_id, 8);
        buf[8] = '\0';
        *vendor = strip_name (buf);

        memcpy (buf, arg.data.out.product_id, 16);
        buf[16] = '\0';
        *product = strip_name (buf);

        return 1;
}

/* This function provides the vendor and product name for a SCSI device.
 * It supports both the modern /sys interface and direct queries
 * via the deprecated ioctl, SCSI_IOCTL_SEND_COMMAND.
 */
static int
scsi_get_product_info (PedDevice* dev, char **vendor, char **product)
{
        *vendor = read_device_sysfs_file (dev, "vendor");
        *product = read_device_sysfs_file (dev, "model");
        if (*vendor && *product)
                return 1;

        return scsi_query_product_info (dev, vendor, product);
}

static int
init_scsi (PedDevice* dev)
{
        struct scsi_idlun
        {
                uint32_t dev_id;
                uint32_t host_unique_id;
        } idlun;

        LinuxSpecific* arch_specific = LINUX_SPECIFIC (dev);
        char* vendor;
        char* product;

        if (!ped_device_open (dev))
                goto error;

        if (ioctl (arch_specific->fd, SCSI_IOCTL_GET_IDLUN, &idlun) < 0) {
                dev->host = 0;
                dev->did = 0;
                if (ped_exception_throw (
                        PED_EXCEPTION_ERROR, PED_EXCEPTION_IGNORE_CANCEL,
                        _("Error initialising SCSI device %s - %s"),
                        dev->path, strerror (errno))
                                != PED_EXCEPTION_IGNORE)
                        goto error_close_dev;
                if (!_device_probe_geometry (dev))
                        goto error_close_dev;
                ped_device_close (dev);
                return 1;
        }

        dev->host = idlun.host_unique_id;
        dev->did  = idlun.dev_id;

        dev->model = (char*) ped_malloc (8 + 16 + 2);
        if (!dev->model)
                goto error_close_dev;

        if (scsi_get_product_info (dev, &vendor, &product)) {
                sprintf (dev->model, "%.8s %.16s", vendor, product);
                free (vendor);
                free (product);
        } else {
                strcpy (dev->model, "Generic SCSI");
        }

        if (!_device_probe_geometry (dev))
                goto error_close_dev;

        ped_device_close (dev);
        return 1;

error_close_dev:
        ped_device_close (dev);
error:
        return 0;
}

static int
init_file (PedDevice* dev)
{
        struct stat     dev_stat;

        if (!_device_stat (dev, &dev_stat))
                goto error;
        if (!ped_device_open (dev))
                goto error;

        dev->sector_size = PED_SECTOR_SIZE_DEFAULT;
        char *p = getenv ("PARTED_SECTOR_SIZE");
        if (p) {
                int s = atoi (p);
                if (0 < s && s % 512 == 0)
                        dev->sector_size = s;
        }
        dev->phys_sector_size = dev->sector_size;

        if (S_ISBLK(dev_stat.st_mode))
                dev->length = _device_get_length (dev);
        else
                dev->length = dev_stat.st_size / dev->sector_size;
        if (dev->length <= 0) {
                ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_CANCEL,
                        _("The device %s is so small that it cannot possibly "
                          "store a file system or partition table.  Perhaps "
                          "you selected the wrong device?"),
                        dev->path);
                goto error_close_dev;
        }

        ped_device_close (dev);

        dev->bios_geom.cylinders = dev->length / 4 / 32;
        dev->bios_geom.heads = 4;
        dev->bios_geom.sectors = 32;
        dev->hw_geom = dev->bios_geom;
        dev->model = strdup ("");

        return 1;

error_close_dev:
        ped_device_close (dev);
error:
        return 0;
}

#if defined __s390__ || defined __s390x__
static int
init_dasd (PedDevice* dev, const char* model_name)
{
        struct stat             dev_stat;
        struct hd_geometry      geo;
        dasd_information_t dasd_info;

        if (!_device_stat (dev, &dev_stat))
                goto error;

        if (!ped_device_open (dev))
                goto error;

        LinuxSpecific* arch_specific = LINUX_SPECIFIC (dev);

        PED_ASSERT (S_ISBLK (dev_stat.st_mode));

        _device_set_sector_size (dev);
        if (!dev->sector_size)
                goto error_close_dev;

        dev->length = _device_get_length (dev);
        if (!dev->length)
                goto error_close_dev;

        if (!ioctl (arch_specific->fd, HDIO_GETGEO, &geo)) {
                dev->hw_geom.sectors = geo.sectors;
                dev->hw_geom.heads = geo.heads;
                dev->hw_geom.cylinders = dev->length
                        / (dev->hw_geom.heads * dev->hw_geom.sectors)
                        / (dev->sector_size / PED_SECTOR_SIZE_DEFAULT);
                dev->bios_geom = dev->hw_geom;
        } else {
                dev->bios_geom.sectors = 12;
                dev->bios_geom.heads = 15;
                dev->bios_geom.cylinders = dev->length
                        / (dev->hw_geom.heads * dev->hw_geom.sectors)
                        / (dev->sector_size / PED_SECTOR_SIZE_DEFAULT);
                dev->hw_geom = dev->bios_geom;
        }

        if (!ioctl(arch_specific->fd, BIODASDINFO, &dasd_info)) {
                arch_specific->devno = dasd_info.devno;
        } else {
                arch_specific->devno = arch_specific->major * 256 +
                                       arch_specific->minor;
        }

        dev->model = strdup (model_name);

        ped_device_close (dev);
        return 1;

error_close_dev:
        ped_device_close (dev);
error:
        return 0;
}
#endif

static int
init_generic (PedDevice* dev, const char* model_name)
{
        struct stat             dev_stat;
        PedExceptionOption      ex_status;

        if (!_device_stat (dev, &dev_stat))
                goto error;

        if (!ped_device_open (dev))
                goto error;

        ped_exception_fetch_all ();
        if (_device_probe_geometry (dev)) {
                ped_exception_leave_all ();
        } else {
		if (!_device_get_length (dev)) {
			ped_exception_catch ();
			ped_exception_leave_all ();
			goto error_close_dev;
		}

                /* hack to allow use of files, for testing */
                ped_exception_catch ();
                ped_exception_leave_all ();

                ex_status = ped_exception_throw (
                                PED_EXCEPTION_WARNING,
                                PED_EXCEPTION_IGNORE_CANCEL,
                                _("Unable to determine geometry of "
                                "file/device %s.  You should not use Parted "
                                "unless you REALLY know what you're doing!"),
                                dev->path);
                switch (ex_status) {
                        case PED_EXCEPTION_CANCEL:
                                goto error_close_dev;

                        case PED_EXCEPTION_UNHANDLED:
                                ped_exception_catch ();
                        case PED_EXCEPTION_IGNORE:
                                break;
                        default:
                                PED_ASSERT (0);
                                break;
                }

                /* what should we stick in here? */
                dev->length = dev_stat.st_size / PED_SECTOR_SIZE_DEFAULT;
                dev->bios_geom.cylinders = dev->length / 4 / 32;
                dev->bios_geom.heads = 4;
                dev->bios_geom.sectors = 32;
                dev->sector_size = PED_SECTOR_SIZE_DEFAULT;
                dev->phys_sector_size = PED_SECTOR_SIZE_DEFAULT;
        }

        dev->model = strdup (model_name);

        ped_device_close (dev);
        return 1;

error_close_dev:
        ped_device_close (dev);
error:
        return 0;
}

static int
sdmmc_get_product_info (PedDevice* dev, char **type, char **name)
{
        *type = read_device_sysfs_file (dev, "type");
        *name = read_device_sysfs_file (dev, "name");
        if (*type && *name)
                return 1;

        return 0;
}

static int
init_sdmmc (PedDevice* dev)
{
        char id[128];
        char *type, *name;

        if (sdmmc_get_product_info (dev, &type, &name)) {
                snprintf (id, sizeof(id) - 1, "%s %s", type, name);
                free (type);
                free (name);
        } else {
                snprintf (id, sizeof(id) - 1, "%s",
                          _("Generic SD/MMC Storage Card"));
        }
        return init_generic(dev, id);
}

static PedDevice*
linux_new (const char* path)
{
        PedDevice*      dev;
        LinuxSpecific*  arch_specific;

        PED_ASSERT (path != NULL);

        dev = (PedDevice*) ped_malloc (sizeof (PedDevice));
        if (!dev)
                goto error;

        dev->path = strdup (path);
        if (!dev->path)
                goto error_free_dev;

        dev->arch_specific
                = (LinuxSpecific*) ped_malloc (sizeof (LinuxSpecific));
        if (!dev->arch_specific)
                goto error_free_path;
        arch_specific = LINUX_SPECIFIC (dev);
        arch_specific->dmtype = NULL;
#if USE_BLKID
        arch_specific->probe = NULL;
        arch_specific->topology = NULL;
#endif

        dev->open_count = 0;
        dev->read_only = 0;
        dev->external_mode = 0;
        dev->dirty = 0;
        dev->boot_dirty = 0;

        if (!_device_probe_type (dev))
                goto error_free_arch_specific;

        switch (dev->type) {
        case PED_DEVICE_IDE:
                if (!init_ide (dev))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_SCSI:
                if (!init_scsi (dev))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_DAC960:
                if (!init_generic (dev, _("DAC960 RAID controller")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_SX8:
                if (!init_generic (dev, _("Promise SX8 SATA Device")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_AOE:
                if (!init_generic (dev, _("ATA over Ethernet Device")))
                    goto error_free_arch_specific;
                break;

#if defined __s390__ || defined __s390x__
        case PED_DEVICE_DASD:
                if (!init_dasd (dev, _("IBM S390 DASD drive")))
                        goto error_free_arch_specific;
                break;
#endif

        case PED_DEVICE_VIODASD:
                if (!init_generic (dev, _("IBM iSeries Virtual DASD")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_CPQARRAY:
                if (!init_generic (dev, _("Compaq Smart Array")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_ATARAID:
                if (!init_generic (dev, _("ATARAID Controller")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_I2O:
                if (!init_generic (dev, _("I2O Controller")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_UBD:
                if (!init_generic (dev, _("User-Mode Linux UBD")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_FILE:
                if (!init_file (dev))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_LOOP:
                if (!init_generic (dev, _("Loopback device")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_DM:
                {
                  char* type;
                  if (arch_specific->dmtype == NULL
                      || asprintf(&type, _("Linux device-mapper (%s)"),
                                  arch_specific->dmtype) == -1)
                        goto error_free_arch_specific;
                  bool ok = init_generic (dev, type);
                  free (type);
                  if (!ok)
                    goto error_free_arch_specific;
                  break;
                }

        case PED_DEVICE_XVD:
                if (!init_generic (dev, _("Xen Virtual Block Device")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_UNKNOWN:
                if (!init_generic (dev, _("Unknown")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_SDMMC:
                if (!init_sdmmc (dev))
                        goto error_free_arch_specific;
                break;
        case PED_DEVICE_VIRTBLK:
                if (!init_generic(dev, _("Virtio Block Device")))
                        goto error_free_arch_specific;
                break;

        case PED_DEVICE_MD:
                if (!init_generic(dev, _("Linux Software RAID Array")))
                        goto error_free_arch_specific;
                break;

        default:
                ped_exception_throw (PED_EXCEPTION_NO_FEATURE,
                                PED_EXCEPTION_CANCEL,
                                _("ped_device_new()  Unsupported device type"));
                goto error_free_arch_specific;
        }
        return dev;

error_free_arch_specific:
        free (dev->arch_specific);
error_free_path:
        free (dev->path);
error_free_dev:
        free (dev);
error:
        return NULL;
}

static void
linux_destroy (PedDevice* dev)
{
        LinuxSpecific *arch_specific = LINUX_SPECIFIC(dev);
        void *p = arch_specific->dmtype;

#if USE_BLKID
        if (arch_specific->probe)
                blkid_free_probe(arch_specific->probe);
#endif
        free (p);
        free (dev->arch_specific);
        free (dev->path);
        free (dev->model);
        free (dev);
}

static int
linux_is_busy (PedDevice* dev)
{
        int     i;
        char*   part_name;

        if (_partition_is_mounted_by_path (dev->path))
                return 1;

        for (i = 0; i < 32; i++) {
                int status;

                part_name = _device_get_part_path (dev, i);
                if (!part_name)
                        return 1;
                status = _partition_is_mounted_by_path (part_name);
                free (part_name);

                if (status)
                        return 1;
        }

        return 0;
}

/* we need to flush the master device, and with kernel < 2.6 all the partition
 * devices, because there is no coherency between the caches with old kernels.
 * We should only flush unmounted partition devices, because:
 *  - there is never a need to flush them (we're not doing IO there)
 *  - flushing a device that is mounted causes unnecessary IO, and can
 * even screw journaling & friends up.  Even cause oopsen!
 */
static void
_flush_cache (PedDevice* dev)
{
        LinuxSpecific*  arch_specific = LINUX_SPECIFIC (dev);
        int             i;

        if (dev->read_only)
                return;
        dev->dirty = 0;

        ioctl (arch_specific->fd, BLKFLSBUF);

        /* With linux-2.6.0 and newer, we're done.  */
        if (_have_kern26())
                return;

        for (i = 1; i < 16; i++) {
                char*           name;
                int             fd;

                name = _device_get_part_path (dev, i);
                if (!name)
                        break;
                if (!_partition_is_mounted_by_path (name)) {
                        fd = open (name, WR_MODE, 0);
                        if (fd > 0) {
                                ioctl (fd, BLKFLSBUF);
retry:
                                if (fsync (fd) < 0 || close (fd) < 0)
					if (ped_exception_throw (
						PED_EXCEPTION_WARNING,
						PED_EXCEPTION_RETRY +
							PED_EXCEPTION_IGNORE,
						_("Error fsyncing/closing %s: %s"),
						name, strerror (errno))
							== PED_EXCEPTION_RETRY)
						goto retry;
                        }
                }
                free (name);
        }
}

static int
linux_open (PedDevice* dev)
{
        LinuxSpecific*  arch_specific = LINUX_SPECIFIC (dev);

retry:
        arch_specific->fd = open (dev->path, RW_MODE);

        if (arch_specific->fd == -1) {
                char*   rw_error_msg = strerror (errno);

                arch_specific->fd = open (dev->path, RD_MODE);

                if (arch_specific->fd == -1) {
                        if (ped_exception_throw (
                                PED_EXCEPTION_ERROR,
                                PED_EXCEPTION_RETRY_CANCEL,
                                _("Error opening %s: %s"),
                                dev->path, strerror (errno))
                                        != PED_EXCEPTION_RETRY) {
                                return 0;
                        } else {
                                goto retry;
                        }
                } else {
                        ped_exception_throw (
                                PED_EXCEPTION_WARNING,
                                PED_EXCEPTION_OK,
                                _("Unable to open %s read-write (%s).  %s has "
                                  "been opened read-only."),
                                dev->path, rw_error_msg, dev->path);
                        dev->read_only = 1;
                }
        } else {
                dev->read_only = 0;
        }

        /* With kernels < 2.6 flush cache for cache coherence issues */
        if (!_have_kern26())
                _flush_cache (dev);

        return 1;
}

static int
linux_refresh_open (PedDevice* dev)
{
        return 1;
}

static int
linux_close (PedDevice* dev)
{
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);

        if (dev->dirty)
                _flush_cache (dev);
retry:
        if (fsync (arch_specific->fd) < 0 || close (arch_specific->fd) < 0)
		if (ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_RETRY + PED_EXCEPTION_IGNORE,
			_("Error fsyncing/closing %s: %s"),
			dev->path, strerror (errno))
				== PED_EXCEPTION_RETRY)
			goto retry;
        return 1;
}

static int
linux_refresh_close (PedDevice* dev)
{
        if (dev->dirty)
                _flush_cache (dev);
        return 1;
}

#if SIZEOF_OFF_T < 8

static _syscall5(int,_llseek,
                 unsigned int, fd,
                 unsigned long, offset_high,
                 unsigned long, offset_low,
                 loff_t*, result,
                 unsigned int, origin)

loff_t
llseek (unsigned int fd, loff_t offset, unsigned int whence)
{
        loff_t result;
        int retval;

        retval = _llseek(fd,
                         ((unsigned long long)offset) >> 32,
                         ((unsigned long long)offset) & 0xffffffff,
                         &result,
                         whence);
        return (retval==-1 ? (loff_t) retval : result);
}

#endif /* SIZEOF_OFF_T < 8 */

static int
_device_seek (const PedDevice* dev, PedSector sector)
{
        LinuxSpecific*  arch_specific;

        PED_ASSERT (dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);
        PED_ASSERT (dev != NULL);
        PED_ASSERT (!dev->external_mode);

        arch_specific = LINUX_SPECIFIC (dev);

#if SIZEOF_OFF_T < 8
        if (sizeof (off_t) < 8) {
                loff_t  pos = (loff_t)(sector * dev->sector_size);
                return llseek (arch_specific->fd, pos, SEEK_SET) == pos;
        } else
#endif
        {
                off_t   pos = sector * dev->sector_size;
                return lseek (arch_specific->fd, pos, SEEK_SET) == pos;
        }
}

static int
_read_lastoddsector (const PedDevice* dev, void* buffer)
{
        LinuxSpecific*                  arch_specific;
        struct blkdev_ioctl_param       ioctl_param;

        PED_ASSERT(dev != NULL);
        PED_ASSERT(buffer != NULL);

        arch_specific = LINUX_SPECIFIC (dev);

retry:
        ioctl_param.block = 0; /* read the last sector */
        ioctl_param.content_length = dev->sector_size;
        ioctl_param.block_contents = buffer;

        if (ioctl(arch_specific->fd, BLKGETLASTSECT, &ioctl_param) == -1) {
                PedExceptionOption      opt;
                opt = ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_RETRY_IGNORE_CANCEL,
                        _("%s during read on %s"),
                        strerror (errno), dev->path);

                if (opt == PED_EXCEPTION_CANCEL)
                        return 0;
                if (opt == PED_EXCEPTION_RETRY)
                        goto retry;
        }

        return 1;
}

static int
linux_read (const PedDevice* dev, void* buffer, PedSector start,
            PedSector count)
{
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);
        PedExceptionOption      ex_status;
        void*                   diobuf = NULL;

        PED_ASSERT (dev != NULL);
        PED_ASSERT (dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

        if (_get_linux_version() < KERNEL_VERSION (2,6,0)) {
                /* Kludge.  This is necessary to read/write the last
                   block of an odd-sized disk, until Linux 2.5.x kernel fixes.
                */
                if (dev->type != PED_DEVICE_FILE && (dev->length & 1)
                    && start + count - 1 == dev->length - 1)
                        return ped_device_read (dev, buffer, start, count - 1)
                                && _read_lastoddsector (
                                        dev, (char *) buffer + (count-1) * 512);
        }
        while (1) {
                if (_device_seek (dev, start))
                        break;

                ex_status = ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_RETRY_IGNORE_CANCEL,
                        _("%s during seek for read on %s"),
                        strerror (errno), dev->path);

                switch (ex_status) {
                        case PED_EXCEPTION_IGNORE:
                                return 1;

                        case PED_EXCEPTION_RETRY:
                                break;

                        case PED_EXCEPTION_UNHANDLED:
                                ped_exception_catch ();
                        case PED_EXCEPTION_CANCEL:
                                return 0;
                        default:
                                PED_ASSERT (0);
                                break;
                }
        }

        size_t read_length = count * dev->sector_size;
        if (posix_memalign (&diobuf, dev->sector_size, read_length) != 0)
                return 0;

        while (1) {
                ssize_t status = read (arch_specific->fd, diobuf, read_length);
                if (status > 0)
                        memcpy(buffer, diobuf, status);
                if (status == (ssize_t) read_length)
                        break;
                if (status > 0) {
                        read_length -= status;
                        buffer = (char *) buffer + status;
                        continue;
                }

                ex_status = ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_RETRY_IGNORE_CANCEL,
                        (status == 0
                         ? _("%0.0send of file while reading %s")
                         : _("%s during read on %s")),
                        strerror (errno),
                        dev->path);

                switch (ex_status) {
                        case PED_EXCEPTION_IGNORE:
                                free(diobuf);
                                return 1;

                        case PED_EXCEPTION_RETRY:
                                break;

                        case PED_EXCEPTION_UNHANDLED:
                                ped_exception_catch ();
                        case PED_EXCEPTION_CANCEL:
                                free(diobuf);
                                return 0;
                        default:
                                PED_ASSERT (0);
                                break;
                }
        }

        free (diobuf);

        return 1;
}

static int
_write_lastoddsector (PedDevice* dev, const void* buffer)
{
        LinuxSpecific*                  arch_specific;
        struct blkdev_ioctl_param       ioctl_param;

        PED_ASSERT(dev != NULL);
        PED_ASSERT(buffer != NULL);

        arch_specific = LINUX_SPECIFIC (dev);

retry:
        ioctl_param.block = 0; /* write the last sector */
        ioctl_param.content_length = dev->sector_size;
        ioctl_param.block_contents = (void*) buffer;

        if (ioctl(arch_specific->fd, BLKSETLASTSECT, &ioctl_param) == -1) {
                PedExceptionOption      opt;
                opt = ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_RETRY_IGNORE_CANCEL,
                        _("%s during write on %s"),
                        strerror (errno), dev->path);

                if (opt == PED_EXCEPTION_CANCEL)
                        return 0;
                if (opt == PED_EXCEPTION_RETRY)
                        goto retry;
        }

        return 1;
}

static int
linux_write (PedDevice* dev, const void* buffer, PedSector start,
             PedSector count)
{
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);
        PedExceptionOption      ex_status;
        void*                   diobuf;
        void*                   diobuf_start;

        PED_ASSERT(dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

        if (dev->read_only) {
                if (ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_IGNORE_CANCEL,
                        _("Can't write to %s, because it is opened read-only."),
                        dev->path)
                                != PED_EXCEPTION_IGNORE)
                        return 0;
                else
                        return 1;
        }

        if (_get_linux_version() < KERNEL_VERSION (2,6,0)) {
                /* Kludge.  This is necessary to read/write the last
                   block of an odd-sized disk, until Linux 2.5.x kernel fixes.
                */
                if (dev->type != PED_DEVICE_FILE && (dev->length & 1)
                    && start + count - 1 == dev->length - 1)
                        return ped_device_write (dev, buffer, start, count - 1)
                                && _write_lastoddsector (
                                        dev, ((char*) buffer
                                              + (count-1) * dev->sector_size));
        }
        while (1) {
                if (_device_seek (dev, start))
                        break;

                ex_status = ped_exception_throw (
                        PED_EXCEPTION_ERROR, PED_EXCEPTION_RETRY_IGNORE_CANCEL,
                        _("%s during seek for write on %s"),
                        strerror (errno), dev->path);

                switch (ex_status) {
                        case PED_EXCEPTION_IGNORE:
                                return 1;

                        case PED_EXCEPTION_RETRY:
                                break;

                        case PED_EXCEPTION_UNHANDLED:
                                ped_exception_catch ();
                        case PED_EXCEPTION_CANCEL:
                                return 0;
                        default:
                                PED_ASSERT (0);
                                break;
                }
        }

#ifdef READ_ONLY
        printf ("ped_device_write (\"%s\", %p, %d, %d)\n",
                dev->path, buffer, (int) start, (int) count);
#else
        size_t write_length = count * dev->sector_size;
        dev->dirty = 1;
        if (posix_memalign(&diobuf, dev->sector_size, write_length) != 0)
                return 0;
        memcpy(diobuf, buffer, write_length);
        diobuf_start = diobuf;
        while (1) {
                ssize_t status = write (arch_specific->fd, diobuf, write_length);
                if (status == write_length) break;
                if (status > 0) {
                        write_length -= status;
                        diobuf = (char *) diobuf + status;
                        continue;
                }

                ex_status = ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_RETRY_IGNORE_CANCEL,
                        _("%s during write on %s"),
                        strerror (errno), dev->path);

                switch (ex_status) {
                        case PED_EXCEPTION_IGNORE:
                                free(diobuf_start);
                                return 1;

                        case PED_EXCEPTION_RETRY:
                                break;

                        case PED_EXCEPTION_UNHANDLED:
                                ped_exception_catch ();
                        case PED_EXCEPTION_CANCEL:
                                free(diobuf_start);
                                return 0;
                        default:
                                PED_ASSERT (0);
                                break;
                }
        }
        free(diobuf_start);
#endif /* !READ_ONLY */
        return 1;
}

/* returns the number of sectors that are ok.
 */
static PedSector
linux_check (PedDevice* dev, void* buffer, PedSector start, PedSector count)
{
        LinuxSpecific*  arch_specific = LINUX_SPECIFIC (dev);
        PedSector       done = 0;
        int             status;
        void*           diobuf;

        PED_ASSERT(dev != NULL);

        if (!_device_seek (dev, start))
                return 0;

        if (posix_memalign(&diobuf, PED_SECTOR_SIZE_DEFAULT,
                           count * PED_SECTOR_SIZE_DEFAULT) != 0)
                return 0;

        for (done = 0; done < count; done += status / dev->sector_size) {
                status = read (arch_specific->fd, diobuf,
                               (size_t) ((count-done) * dev->sector_size));
                if (status > 0)
                        memcpy(buffer, diobuf, status);
                if (status < 0)
                        break;
        }
        free(diobuf);

        return done;
}

static int
_do_fsync (PedDevice* dev)
{
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);
        int                     status;
        PedExceptionOption      ex_status;

        while (1) {
                status = fsync (arch_specific->fd);
                if (status >= 0) break;

                ex_status = ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_RETRY_IGNORE_CANCEL,
                        _("%s during write on %s"),
                        strerror (errno), dev->path);

                switch (ex_status) {
                        case PED_EXCEPTION_IGNORE:
                                return 1;

                        case PED_EXCEPTION_RETRY:
                                break;

                        case PED_EXCEPTION_UNHANDLED:
                                ped_exception_catch ();
                        case PED_EXCEPTION_CANCEL:
                                return 0;
                        default:
                                PED_ASSERT (0);
                                break;
                }
        }
        return 1;
}

static int
linux_sync (PedDevice* dev)
{
        PED_ASSERT (dev != NULL);
        PED_ASSERT (!dev->external_mode);

        if (dev->read_only)
                return 1;
        if (!_do_fsync (dev))
                return 0;
        _flush_cache (dev);
        return 1;
}

static int
linux_sync_fast (PedDevice* dev)
{
        PED_ASSERT (dev != NULL);
        PED_ASSERT (!dev->external_mode);

        if (dev->read_only)
                return 1;
        if (!_do_fsync (dev))
                return 0;
        /* no cache flush... */
        return 1;
}

static inline int
_compare_digit_state (char ch, int need_digit)
{
        return !!isdigit (ch) == need_digit;
}

/* matches the regexp "[^0-9]+[0-9]+[^0-9]+[0-9]+$".
 * Motivation: accept devices looking like /dev/rd/c0d0, but
 * not looking like /dev/hda1 and /dev/rd/c0d0p1
 */
static int _GL_ATTRIBUTE_PURE
_match_rd_device (const char* name)
{
        const char* pos;
        int state;

        /* exclude directory names from test */
        pos = strrchr(name, '/') ?: name;

        /* states:
         *      0       non-digits
         *      1       digits
         *      2       non-digits
         *      3       digits
         */
        for (state = 0; state < 4; state++) {
                int want_digits = (state % 2 == 1);
                do {
                        if (!*pos)
                                return 0;
                        if (!_compare_digit_state (*pos, want_digits))
                                return 0;
                        pos++;
                } while (_compare_digit_state (*pos, want_digits));
        }

        return *pos == 0;
}

static int
_probe_proc_partitions ()
{
        FILE*           proc_part_file;
        int             major, minor, size;
        char            buf [512];
        char            part_name [256];
        char            dev_name [256];
        int ok = 0;

        proc_part_file = fopen ("/proc/partitions", "r");
        if (!proc_part_file)
                return 0;

        if (fgets (buf, 256, proc_part_file) == NULL)
                goto done;

        if (fgets (buf, 256, proc_part_file) == NULL)
                goto done;

        while (fgets (buf, 512, proc_part_file)
               && sscanf (buf, "%d %d %d %255s", &major, &minor, &size,
                          part_name) == 4) {
                /* Heuristic for telling partitions and devices apart
                 * Probably needs to be improved
                 */
                if (!_match_rd_device (part_name)
                    && isdigit (part_name [strlen (part_name) - 1]))
                        continue;

                strcpy (dev_name, "/dev/");
                strcat (dev_name, part_name);
                _ped_device_probe (dev_name);
        }

        ok = 1;
 done:
        fclose (proc_part_file);
        return ok;
}

struct _entry {
	const char *name;
	size_t len;
};

static int _GL_ATTRIBUTE_PURE
_skip_entry (const char *name)
{
	struct _entry *i;
	static struct _entry entries[] = {
		{ ".",		sizeof (".") - 1	},
		{ "..",		sizeof ("..") - 1	},
		{ "dm-",	sizeof ("dm-") - 1	},
		{ "loop",	sizeof ("loop") - 1	},
		{ "ram",	sizeof ("ram") - 1	},
		{ "fd",		sizeof ("fd") - 1	},
		{ 0, 0 },
	};

	for (i = entries; i->name != 0; i++) {
		if (strncmp (name, i->name, i->len) == 0)
			return 1;
	}

	return 0;
}

static int
_probe_sys_block ()
{
	DIR *blockdir;
	struct dirent *dirent;
	char dev_name [256];
	char *ptr;

	if (!(blockdir = opendir ("/sys/block")))
		return 0;
	while ((dirent = readdir (blockdir))) {
		if (_skip_entry (dirent->d_name))
			continue;

		if (strlen (dirent->d_name) > sizeof (dev_name) - 6)
			continue; /* device name too long! */

		strcpy (dev_name, "/dev/");
		strcat (dev_name, dirent->d_name);
		/* in /sys/block, '/'s are replaced with '!' or '.' */
		for (ptr = dev_name; *ptr != '\0'; ptr++) {
			if (*ptr == '!' || *ptr == '.')
				*ptr = '/';
		}
		_ped_device_probe (dev_name);
	}

	closedir (blockdir);
	return 1;
}

static int
_probe_standard_devices ()
{
        _ped_device_probe ("/dev/hda");
        _ped_device_probe ("/dev/hdb");
        _ped_device_probe ("/dev/hdc");
        _ped_device_probe ("/dev/hdd");
        _ped_device_probe ("/dev/hde");
        _ped_device_probe ("/dev/hdf");
        _ped_device_probe ("/dev/hdg");
        _ped_device_probe ("/dev/hdh");

        _ped_device_probe ("/dev/sda");
        _ped_device_probe ("/dev/sdb");
        _ped_device_probe ("/dev/sdc");
        _ped_device_probe ("/dev/sdd");
        _ped_device_probe ("/dev/sde");
        _ped_device_probe ("/dev/sdf");

        return 1;
}

static void
linux_probe_all ()
{
        /* we should probe the standard devs too, even with /proc/partitions,
         * because /proc/partitions might return devfs stuff, and we might not
         * have devfs available
         */
        _probe_standard_devices ();

#ifdef ENABLE_DEVICE_MAPPER
        /* device-mapper devices aren't listed in /proc/partitions; or, if
         * they are, they're listed as dm-X.  So, instead of relying on that,
         * we do our own checks.
         */
        _probe_dm_devices ();
#endif

        /* /sys/block is more reliable and consistent; fall back to using
         * /proc/partitions if the former is unavailable, however.
         */
        if (!_probe_sys_block ())
                _probe_proc_partitions ();
}

static char * _GL_ATTRIBUTE_FORMAT ((__printf__, 1, 2))
zasprintf (const char *format, ...)
{
  va_list args;
  char *resultp;
  va_start (args, format);
  int r = vasprintf (&resultp, format, args);
  va_end (args);
  return r < 0 ? NULL : resultp;
}

static char*
_device_get_part_path (PedDevice *dev, int num)
{
        size_t path_len = strlen (dev->path);

        char *result;
        /* Check for devfs-style /disc => /partN transformation
           unconditionally; the system might be using udev with devfs rules,
           and if not the test is harmless. */
        if (5 < path_len && !strcmp (dev->path + path_len - 5, "/disc")) {
                /* replace /disc with /part%d */
                result = zasprintf ("%.*s/part%d",
                                    (int) (path_len - 5), dev->path, num);
        } else {
                char const *p = (dev->type == PED_DEVICE_DAC960
                                 || dev->type == PED_DEVICE_CPQARRAY
                                 || dev->type == PED_DEVICE_ATARAID
                                 || isdigit (dev->path[path_len - 1])
                                 ? "p" : "");
                result = zasprintf ("%s%s%d", dev->path, p, num);
        }

        return result;
}

static char*
linux_partition_get_path (const PedPartition* part)
{
        return _device_get_part_path (part->disk->dev, part->num);
}

static int
_mount_table_search (const char* file_name, dev_t dev)
{
        struct stat part_stat;
        char line[512];
        char part_name[512];
        FILE* file;

        file = fopen (file_name, "r");
        if (!file)
                return 0;
        while (fgets (line, 512, file)) {
                if (sscanf (line, "%s", part_name) == 1
                    && stat (part_name, &part_stat) == 0) {
                        if (part_stat.st_rdev == dev) {
                                fclose (file);
                                return 1;
                        }
                }
        }
        fclose (file);
        return 0;
}

static int
_partition_is_mounted_by_dev (dev_t dev)
{
        return  _mount_table_search( "/proc/mounts", dev)
                || _mount_table_search( "/proc/swaps", dev)
                || _mount_table_search( "/etc/mtab", dev);
}

static int
_partition_is_mounted_by_path (const char *path)
{
        struct stat part_stat;
        if (stat (path, &part_stat) != 0)
                return 0;
        if (!S_ISBLK(part_stat.st_mode))
                return 0;
        return _partition_is_mounted_by_dev (part_stat.st_rdev);
}

/* If partition PART is mounted, or if we encounter an out-of-memory error
   while trying to determine its status, return 1.  Otherwise, return 0.  */
static int
_partition_is_mounted (const PedPartition *part)
{
	if (!ped_partition_is_active (part))
		return 0;
	char *part_name = _device_get_part_path (part->disk->dev, part->num);
	if (!part_name)
		return 1;
	int status = _partition_is_mounted_by_path (part_name);
	free (part_name);
	return !!status;
}

static int
linux_partition_is_busy (const PedPartition* part)
{
        PedPartition*   walk;

        PED_ASSERT (part != NULL);

        if (_partition_is_mounted (part))
                return 1;
        if (part->type == PED_PARTITION_EXTENDED) {
                for (walk = part->part_list; walk; walk = walk->next) {
                        if (linux_partition_is_busy (walk))
                                return 1;
                }
        }
        return 0;
}

static int
_blkpg_part_command (PedDevice* dev, struct blkpg_partition* part, int op)
{
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);
        struct blkpg_ioctl_arg  ioctl_arg;

        ioctl_arg.op = op;
        ioctl_arg.flags = 0;
        ioctl_arg.datalen = sizeof (struct blkpg_partition);
        ioctl_arg.data = (void*) part;

        return ioctl (arch_specific->fd, BLKPG, &ioctl_arg) == 0;
}

static int
_blkpg_add_partition (PedDisk* disk, const PedPartition *part)
{
        struct blkpg_partition  linux_part;
        const char*             vol_name;
        char*                   dev_name;

        PED_ASSERT(disk != NULL);
        PED_ASSERT(disk->dev->sector_size % PED_SECTOR_SIZE_DEFAULT == 0);

        if (ped_disk_type_check_feature (disk->type,
                                         PED_DISK_TYPE_PARTITION_NAME))
                vol_name = ped_partition_get_name (part);
        else
                vol_name = NULL;

        dev_name = _device_get_part_path (disk->dev, part->num);
        if (!dev_name)
                return 0;

        memset (&linux_part, 0, sizeof (linux_part));
        linux_part.start = part->geom.start * disk->dev->sector_size;
        /* see fs/partitions/msdos.c:msdos_partition(): "leave room for LILO" */
        if (part->type & PED_PARTITION_EXTENDED)
                linux_part.length = part->geom.length == 1 ? 512 : 1024;
        else
                linux_part.length = part->geom.length * disk->dev->sector_size;
        linux_part.pno = part->num;
        strncpy (linux_part.devname, dev_name, BLKPG_DEVNAMELTH);
        if (vol_name)
                strncpy (linux_part.volname, vol_name, BLKPG_VOLNAMELTH);

        free (dev_name);

        if (!_blkpg_part_command (disk->dev, &linux_part,
                                  BLKPG_ADD_PARTITION)) {
                return ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_IGNORE_CANCEL,
                        _("Error informing the kernel about modifications to "
                          "partition %s -- %s.  This means Linux won't know "
                          "about any changes you made to %s until you reboot "
                          "-- so you shouldn't mount it or use it in any way "
                          "before rebooting."),
                        linux_part.devname,
                        strerror (errno),
                        linux_part.devname)
                                == PED_EXCEPTION_IGNORE;
        }

        return 1;
}

static int
_blkpg_remove_partition (PedDisk* disk, int n)
{
        struct blkpg_partition  linux_part;

        memset (&linux_part, 0, sizeof (linux_part));
        linux_part.pno = n;
        return _blkpg_part_command (disk->dev, &linux_part,
                                    BLKPG_DEL_PARTITION);
}

/* Read the integer from /sys/block/DEV_BASE/ENTRY and set *VAL
   to that value, where DEV_BASE is the last component of DEV->path.
   Upon success, return true.  Otherwise, return false. */
static bool
_sysfs_int_entry_from_dev(PedDevice const* dev, const char *entry, int *val)
{
        char        path[128];
        int r = snprintf(path, sizeof(path), "/sys/block/%s/%s",
			 last_component(dev->path), entry);
        if (r < 0 || r >= sizeof(path))
                return false;

        FILE *fp = fopen(path, "r");
        if (!fp)
                return false;

        bool ok = fscanf(fp, "%d", val) == 1;
        fclose(fp);

        return ok;
}

/* Read the unsigned long long from /sys/block/DEV_BASE/PART_BASE/ENTRY
   and set *VAL to that value, where DEV_BASE is the last component of path to
   block device corresponding to PART and PART_BASE is the sysfs name of PART.
   Upon success, return true. Otherwise, return false. */
static bool
_sysfs_ull_entry_from_part(PedPartition const* part, const char *entry,
                           unsigned long long *val)
{
        char path[128];
        char *part_name = linux_partition_get_path(part);
        if (!part_name)
                return false;

        int r = snprintf(path, sizeof(path), "/sys/block/%s/%s/%s",
                last_component(part->disk->dev->path),
                last_component(part_name), entry);
        free(part_name);
        if (r < 0 || r >= sizeof(path))
                return false;

        FILE *fp = fopen(path, "r");
        if (!fp)
                return false;

        bool ok = fscanf(fp, "%llu", val) == 1;
        fclose(fp);

        return ok;
}


/* Get the starting sector and length of a partition PART within a block device
   Use blkpg if available, then check sysfs and then use HDIO_GETGEO and
   BLKGETSIZE64 ioctls as fallback.  Upon success, return true.  Otherwise,
   return false. */
static bool
_kernel_get_partition_start_and_length(PedPartition const *part,
                                       unsigned long long *start,
                                       unsigned long long *length)
{
        PED_ASSERT(part);
        PED_ASSERT(start);
        PED_ASSERT(length);

        char *dev_name = linux_partition_get_path (part);
        if (!dev_name)
                return false;

        int ok = _sysfs_ull_entry_from_part (part, "start", start);
        if (!ok) {
                struct hd_geometry geom;
                int dev_fd = open (dev_name, O_RDONLY);
                if (dev_fd != -1 && ioctl (dev_fd, HDIO_GETGEO, &geom)) {
                        *start = geom.start;
                        ok = true;
                } else {
                        if (dev_fd != -1)
                                close(dev_fd);
                        free (dev_name);
                        return false;
                }
        }
        *start = (*start * 512) / part->disk->dev->sector_size;
        ok = _sysfs_ull_entry_from_part (part, "size", length);

        int fd;
        if (!ok) {
                fd = open (dev_name, O_RDONLY);
                if (fd != -1 && ioctl (fd, BLKGETSIZE64, length))
                        ok = true;
        } else {
                fd = -1;
                *length *= 512;
        }
        *length /= part->disk->dev->sector_size;
        if (fd != -1)
                close (fd);

        if (!ok)
                ped_exception_throw (
                        PED_EXCEPTION_BUG,
                        PED_EXCEPTION_CANCEL,
                        _("Unable to determine the start and length of %s."),
                        dev_name);
        free (dev_name);
        return ok;
}


/*
 * The number of partitions that a device can have depends on the kernel.
 * If we don't find this value in /sys/block/DEV/ext_range, we will use our own
 * value.
 */
static unsigned int
_device_get_partition_range(PedDevice const* dev)
{
        int range;
        bool ok = _sysfs_int_entry_from_dev(dev, "ext_range", &range);

        if (!ok)
                return MAX_NUM_PARTS;
        /* both 0 and 1 mean no partitions */
        return range > 1 ? range : 0;
}

/*
 * Sync the partition table in two step process:
 * 1. Remove all of the partitions from the kernel's tables, but do not attempt
 *    removal of any partition for which the corresponding ioctl call fails.
 * 2. Add all the partitions that we hold in disk, throwing a warning
 *    if we cannot because step 1 failed to remove it and it is not being
 *    added back with the same start and length.
 *
 * To achieve this two step process we must calculate the minimum number of
 * maximum possible partitions between what linux supports and what the label
 * type supports. EX:
 *
 * number=MIN(max_parts_supported_in_linux,max_parts_supported_in_msdos_tables)
 */
static int
_disk_sync_part_table (PedDisk* disk)
{
        PED_ASSERT(disk != NULL);
        PED_ASSERT(disk->dev != NULL);
        int lpn;

        unsigned int part_range = _device_get_partition_range(disk->dev);

        /* lpn = largest partition number. */
        if (ped_disk_get_max_supported_partition_count(disk, &lpn))
                lpn = PED_MIN(lpn, part_range);
        else
                lpn = part_range;

        /* Its not possible to support largest_partnum < 0.
         * largest_partnum == 0 would mean does not support partitions.
         * */
        if (lpn < 1)
                return 0;
        int ret = 0;
        int *ok = calloc (lpn, sizeof *ok);
        if (!ok)
                return 0;
        int *errnums = ped_malloc(sizeof(int) * lpn);
        if (!errnums)
                goto cleanup;

        /* Attempt to remove each and every partition, retrying for
           up to max_sleep_seconds upon any failure due to EBUSY. */
        unsigned int sleep_microseconds = 10000;
        unsigned int max_sleep_seconds = 1;
        unsigned int n_sleep = (max_sleep_seconds
                                * 1000000 / sleep_microseconds);
        int i;
        for (i = 0; i < n_sleep; i++) {
	    if (i)
		usleep (sleep_microseconds);
            bool busy = false;
            int j;
            for (j = 0; j < lpn; j++) {
                if (!ok[j]) {
                    ok[j] = _blkpg_remove_partition (disk, j + 1);
                    errnums[j] = errno;
                    if (!ok[j] && errnums[j] == EBUSY)
                        busy = true;
                }
            }
            if (!busy)
                break;
        }

        for (i = 1; i <= lpn; i++) {
                PedPartition *part = ped_disk_get_partition (disk, i);
                if (part) {
                        if (!ok[i - 1] && errnums[i - 1] == EBUSY) {
                                unsigned long long length;
                                unsigned long long start;
                                /* get start and length of existing partition */
                                if (!_kernel_get_partition_start_and_length(part,
                                                                &start, &length))
                                        goto cleanup;
                                if (start == part->geom.start
				    && length == part->geom.length)
                                        ok[i - 1] = 1;
                                /* If the new partition is unchanged and the
				   existing one was not removed because it was
				   in use, then reset the error flag and do not
				   try to add it since it is already there.  */
                                continue;
                        }

                        /* add the (possibly modified or new) partition */
                        if (!_blkpg_add_partition (disk, part)) {
                                ped_exception_throw (
                                        PED_EXCEPTION_ERROR,
                                        PED_EXCEPTION_RETRY_CANCEL,
                                        _("Failed to add partition %d (%s)"),
                                        i, strerror (errno));
                                goto cleanup;
                        }
                }
        }

        char *bad_part_list = NULL;
        /* now warn about any errors */
        for (i = 1; i <= lpn; i++) {
		if (ok[i - 1] || errnums[i - 1] == ENXIO)
			continue;
		if (bad_part_list == NULL) {
			  bad_part_list = malloc (lpn * 5);
			  if (!bad_part_list)
				  goto cleanup;
			  bad_part_list[0] = 0;
		}
		sprintf (bad_part_list + strlen (bad_part_list), "%d, ", i);
	}
        if (bad_part_list == NULL)
		ret = 1;
	else {
                bad_part_list[strlen (bad_part_list) - 2] = 0;
                if (ped_exception_throw (
                        PED_EXCEPTION_ERROR,
                        PED_EXCEPTION_IGNORE_CANCEL,
                        _("Partition(s) %s on %s have been written, but we have "
			  "been unable to inform the kernel of the change, "
			  "probably because it/they are in use.  As a result, "
                          "the old partition(s) will remain in use.  You "
                          "should reboot now before making further changes."),
                        bad_part_list, disk->dev->path) == PED_EXCEPTION_IGNORE)
                        ret = 1;
		free (bad_part_list);
        }
 cleanup:
        free (errnums);
        free (ok);
        return ret;
}

#ifdef ENABLE_DEVICE_MAPPER
static int
_dm_remove_map_name(char *name)
{
        struct dm_task  *task = NULL;
        int             rc;

        task = dm_task_create(DM_DEVICE_REMOVE);
        if (!task)
                return 1;

        dm_task_set_name (task, name);

        rc = dm_task_run(task);
        dm_task_update_nodes();
        dm_task_destroy(task);
        if (!rc)
                return 1;

        return 0;
}

static int
_dm_is_part (struct dm_info *this, char *name)
{
        struct dm_task* task = NULL;
        struct dm_info* info = alloca(sizeof *info);
        struct dm_deps* deps = NULL;
        int             rc = 0;
        unsigned int    i;

        task = dm_task_create(DM_DEVICE_DEPS);
        if (!task)
                return 0;

        dm_task_set_name(task, name);
        if (!dm_task_run(task))
                goto err;

        memset(info, '\0', sizeof *info);
        dm_task_get_info(task, info);
        if (!info->exists)
                goto err;

        deps = dm_task_get_deps(task);
        if (!deps)
                goto err;

        for (i = 0; i < deps->count; i++) {
                unsigned int ma = major(deps->device[i]),
                             mi = minor(deps->device[i]);

                if (ma == this->major && mi == this->minor)
                        rc = 1;
        }

err:
        dm_task_destroy(task);
        return rc;
}

static int
_dm_remove_parts (PedDevice* dev)
{
        struct dm_task*         task = NULL;
        struct dm_info*         info = alloca(sizeof *info);
        struct dm_names*        names = NULL;
        unsigned int            next = 0;
        int                     rc;
        LinuxSpecific*          arch_specific = LINUX_SPECIFIC (dev);

        task = dm_task_create(DM_DEVICE_LIST);
        if (!task)
                goto err;

        if (!dm_task_set_major_minor (task, arch_specific->major,
                                      arch_specific->minor, 0))
                goto err;

        if (!dm_task_run(task))
                goto err;

        memset(info, '\0', sizeof *info);
        dm_task_get_info(task, info);
        if (!info->exists)
                goto err;

        names = dm_task_get_names(task);
        if (!names)
                goto err;

        rc = 0;
        do {
                names = (void *) ((char *) names + next);

                if (_dm_is_part(info, names->name))
                        rc += _dm_remove_map_name(names->name);

                next = names->next;
        } while (next);

        dm_task_update_nodes();
        dm_task_destroy(task);
        task = NULL;

        if (!rc)
                return 1;
err:
        if (task)
                dm_task_destroy(task);
        ped_exception_throw (PED_EXCEPTION_WARNING, PED_EXCEPTION_IGNORE,
                _("parted was unable to re-read the partition "
                  "table on %s (%s).  This means Linux won't know "
                  "anything about the modifications you made. "),
                dev->path, strerror (errno));
        return 0;
}

static int
_dm_add_partition (PedDisk* disk, PedPartition* part)
{
        char*           vol_name = NULL;
        const char*     dev_name = NULL;
        char*           params = NULL;
        LinuxSpecific*  arch_specific = LINUX_SPECIFIC (disk->dev);

        /* Get map name from devicemapper */
        struct dm_task *task = dm_task_create (DM_DEVICE_INFO);
        if (!task)
                goto err;

        if (!dm_task_set_major_minor (task, arch_specific->major,
                                      arch_specific->minor, 0))
                goto err;

        if (!dm_task_run(task))
                goto err;

        dev_name = dm_task_get_name (task);

        if (isdigit (dev_name[strlen (dev_name) - 1])) {
                if ( ! (vol_name = zasprintf ("%sp%d", dev_name, part->num)))
                        goto err;
        } else if ( ! (vol_name = zasprintf ("%s%d", dev_name, part->num)))
                goto err;

        /* Caution: dm_task_destroy frees dev_name.  */
        dm_task_destroy (task);
        task = NULL;

        if ( ! (params = zasprintf ("%d:%d %lld", arch_specific->major,
                                    arch_specific->minor, part->geom.start)))
                goto err;

        task = dm_task_create (DM_DEVICE_CREATE);
        if (!task)
                goto err;

        dm_task_set_name (task, vol_name);
        dm_task_add_target (task, 0, part->geom.length,
                "linear", params);
        if (dm_task_run (task)) {
                //printf("0 %ld linear %s\n", part->geom.length, params);
                dm_task_update_nodes();
                dm_task_destroy(task);
                free(params);
                free(vol_name);
                return 1;
        } else {
                _dm_remove_map_name(vol_name);
        }
err:
        dm_task_update_nodes();
        if (task)
                dm_task_destroy (task);
        free (params);
        free (vol_name);
        return 0;
}

static int
_dm_reread_part_table (PedDisk* disk)
{
        int largest_partnum = ped_disk_get_last_partition_num (disk);
        if (largest_partnum <= 0)
          return 1;

        int     rc = 1;
        int     last = PED_MIN (largest_partnum, 16);
        int     i;

        sync();
        if (!_dm_remove_parts(disk->dev))
                rc = 0;

        for (i = 1; i <= last; i++) {
                PedPartition*      part;

                part = ped_disk_get_partition (disk, i);
                if (!part)
                        continue;

                if (!_dm_add_partition (disk, part))
                        rc = 0;
        }
        return rc;
}
#endif

static int
_have_blkpg ()
{
        static int have_blkpg = -1;
        int kver;

        if (have_blkpg != -1)
                return have_blkpg;

        kver = _get_linux_version();
        return have_blkpg = kver >= KERNEL_VERSION (2,4,0) ? 1 : 0;
}

/* Return nonzero upon success, 0 if something fails.  */
static int
linux_disk_commit (PedDisk* disk)
{
#ifdef ENABLE_DEVICE_MAPPER
        if (disk->dev->type == PED_DEVICE_DM)
                return _dm_reread_part_table (disk);
#endif
        if (disk->dev->type != PED_DEVICE_FILE) {

		/* We now require BLKPG support.  If this assertion fails,
		   please write to the mailing list describing your system.
		   Assuming it's never triggered, ...
		   FIXME: remove this assertion in 2012.  */
		assert (_have_blkpg ());

		if (!_disk_sync_part_table (disk))
			return 0;
        }

        return 1;
}

#if USE_BLKID
static PedAlignment*
linux_get_minimum_alignment(const PedDevice *dev)
{
        blkid_topology tp = LINUX_SPECIFIC(dev)->topology;
        if (!tp)
                return NULL;

        if (blkid_topology_get_minimum_io_size(tp) == 0)
                return ped_alignment_new(
                        blkid_topology_get_alignment_offset(tp) /
                                dev->sector_size,
                        dev->phys_sector_size / dev->sector_size);

        return ped_alignment_new(
                blkid_topology_get_alignment_offset(tp) / dev->sector_size,
                blkid_topology_get_minimum_io_size(tp) / dev->sector_size);
}

static PedAlignment*
linux_get_optimum_alignment(const PedDevice *dev)
{
        blkid_topology tp = LINUX_SPECIFIC(dev)->topology;
        if (!tp)
                return NULL;

        /* When PED_DEFAULT_ALIGNMENT is divisible by the *_io_size or
	   there are no *_io_size values, use the PED_DEFAULT_ALIGNMENT
           If one or the other will not divide evenly, fall through to
           previous logic. */
        unsigned long optimal_io = blkid_topology_get_optimal_io_size(tp);
        unsigned long minimum_io = blkid_topology_get_minimum_io_size(tp);
        if (
            (!optimal_io && !minimum_io)
	    || (optimal_io && PED_DEFAULT_ALIGNMENT % optimal_io == 0
		&& minimum_io && PED_DEFAULT_ALIGNMENT % minimum_io == 0)
	    || (!minimum_io && optimal_io
		&& PED_DEFAULT_ALIGNMENT % optimal_io == 0)
	    || (!optimal_io && minimum_io
		&& PED_DEFAULT_ALIGNMENT % minimum_io == 0)
           ) {
            /* DASD needs to use minimum alignment */
            if (dev->type == PED_DEVICE_DASD)
                return linux_get_minimum_alignment(dev);

            return ped_alignment_new(
                    blkid_topology_get_alignment_offset(tp) / dev->sector_size,
                    PED_DEFAULT_ALIGNMENT / dev->sector_size);
        }

        /* If optimal_io_size is 0 and we don't meet the other criteria
           for using the device.c default, return the minimum alignment. */
        if (blkid_topology_get_optimal_io_size(tp) == 0)
                return linux_get_minimum_alignment(dev);

        return ped_alignment_new(
                blkid_topology_get_alignment_offset(tp) / dev->sector_size,
                blkid_topology_get_optimal_io_size(tp) / dev->sector_size);
}
#endif

static PedDeviceArchOps linux_dev_ops = {
        _new:           linux_new,
        destroy:        linux_destroy,
        is_busy:        linux_is_busy,
        open:           linux_open,
        refresh_open:   linux_refresh_open,
        close:          linux_close,
        refresh_close:  linux_refresh_close,
        read:           linux_read,
        write:          linux_write,
        check:          linux_check,
        sync:           linux_sync,
        sync_fast:      linux_sync_fast,
        probe_all:      linux_probe_all,
#if USE_BLKID
        get_minimum_alignment:	linux_get_minimum_alignment,
        get_optimum_alignment:	linux_get_optimum_alignment,
#endif
};

PedDiskArchOps linux_disk_ops =  {
        partition_get_path:     linux_partition_get_path,
        partition_is_busy:      linux_partition_is_busy,
        disk_commit:            linux_disk_commit
};

PedArchitecture ped_linux_arch = {
        dev_ops:        &linux_dev_ops,
        disk_ops:       &linux_disk_ops
};
