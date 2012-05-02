/* Some prototypes for extern functions. */

//#undef __KERNEL_STRICT_NAMES
#include <linux/types.h>

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

#define lba28_limit ((__u64)(1<<28) - 1)

void identify (__u16 *id_supplied);
void usage_error(int out) __attribute__((noreturn));
void no_scsi (void);
void no_xt (void);
void process_dev (char *devname);
int sysfs_get_attr (int fd, const char *attr, const char *fmt, void *val1, void *val2, int verbose);
int sysfs_set_attr (int fd, const char *attr, const char *fmt, void *val_p, int verbose);
int get_dev_geometry (int fd, __u32 *cyls, __u32 *heads, __u32 *sects, __u64 *start_lba, __u64 *nsectors);
int get_dev_t_geometry (dev_t dev, __u32 *cyls, __u32 *heads, __u32 *sects, __u64 *start_lba, __u64 *nsectors);
int do_filemap(const char *file_name);
int do_fallocate_syscall (const char *name, __u64 bytecount);
int fwdownload(int fd, __u16 *id, const char *fwpath, int xfer_mode);
void dco_identify_print (__u16 *dco);
int set_dvdspeed(int fd, int speed);
int fd_is_raid (int fd);

extern const char *BuffType[4];

struct local_hd_big_geometry {
       unsigned char	heads;
       unsigned char	sectors;
       unsigned int	cylinders;
       unsigned long	start;
};

struct local_hd_geometry {
      unsigned char	heads;
      unsigned char	sectors;
      unsigned short	cylinders;
      unsigned long	start;		/* mmm.. on 32-bit, this limits us to LBA32, 2TB max offset */
};

enum {	/* ioctl() numbers */
	HDIO_DRIVE_CMD		= 0x031f,
	HDIO_DRIVE_RESET	= 0x031c,
	HDIO_DRIVE_TASK		= 0x031e,
	HDIO_DRIVE_TASKFILE	= 0x031d,
	HDIO_GETGEO		= 0x0301,
	HDIO_GETGEO_BIG		= 0x0330,
	HDIO_GET_32BIT		= 0x0309,
	HDIO_GET_ACOUSTIC	= 0x030f,
	HDIO_GET_BUSSTATE	= 0x031a,
	HDIO_GET_DMA		= 0x030b,
	HDIO_GET_IDENTITY	= 0x030d,
	HDIO_GET_KEEPSETTINGS	= 0x0308,
	HDIO_GET_MULTCOUNT	= 0x0304,
	HDIO_GET_NOWERR		= 0x030a,
	HDIO_GET_QDMA		= 0x0305,
	HDIO_GET_UNMASKINTR	= 0x0302,
	HDIO_OBSOLETE_IDENTITY	= 0x0307,
	HDIO_SCAN_HWIF		= 0x0328,
	HDIO_SET_32BIT		= 0x0324,
	HDIO_SET_ACOUSTIC	= 0x032c,
	HDIO_SET_BUSSTATE	= 0x032d,
	HDIO_SET_DMA		= 0x0326,
	HDIO_SET_KEEPSETTINGS	= 0x0323,
	HDIO_SET_MULTCOUNT	= 0x0321,
	HDIO_SET_NOWERR		= 0x0325,
	HDIO_SET_PIO_MODE	= 0x0327,
	HDIO_SET_QDMA		= 0x032e,
	HDIO_SET_UNMASKINTR	= 0x0322,
	HDIO_SET_WCACHE		= 0x032b,
	HDIO_TRISTATE_HWIF	= 0x031b,
	HDIO_UNREGISTER_HWIF	= 0x032a,
	CDROM__SPEED		= 0x5322,
};

#define START_LBA_UNKNOWN	(~0ull)

/* Some systems define BLKGETSIZE64 with a "u64" arg,
 * but without supplying a typedef for u64.
 * The only real workaround here is to define it locally,
 * instead of using the system def from <linux/fs.h>
 */
#ifdef BLKGETSIZE64
#undef BLKGETSIZE64
#endif
#define BLKGETSIZE64 _IOR(0x12,114,__u64)

