/* Some prototypes for extern functions. */

//#undef __KERNEL_STRICT_NAMES
#include <sys/types.h>

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

#define lba28_limit ((__u64)(1<<28) - 1)

void identify (int fd, __u16 *id_supplied);
void usage_error(int out) __attribute__((noreturn));
void no_scsi (void);
void no_xt (void);
void process_dev (char *devname);
int sysfs_get_attr (int fd, const char *attr, const char *fmt, void *val1, void *val2, int verbose);
int sysfs_set_attr (int fd, const char *attr, const char *fmt, void *val_p, int verbose);
int sysfs_get_attr_recursive (int fd, const char *attr, const char *fmt, void *val1, void *val2, int verbose);

int get_dev_geometry (int fd, __u32 *cyls, __u32 *heads, __u32 *sects, __u64 *start_lba, __u64 *nsectors);
int get_dev_t_geometry (dev_t dev, __u32 *cyls, __u32 *heads, __u32 *sects,
				__u64 *start_lba, __u64 *nsectors, unsigned int *sector_bytes);
int do_filemap(const char *file_name);
int do_fallocate_syscall (const char *name, __u64 bytecount);
int fwdownload (int fd, __u16 *id, const char *fwpath, int xfer_mode);
void dco_identify_print (__u16 *dco);
int set_dvdspeed(int fd, int speed);
int fd_is_raid (int fd);

int  wdidle3_set_timeout (int fd, unsigned char timeout);
int  wdidle3_get_timeout (int fd, unsigned char *timeout);
void wdidle3_print_timeout (unsigned char timeout);
unsigned char wdidle3_msecs_to_timeout (unsigned int msecs);

int get_log_page_data (int fd, __u8 log_address, __u8 pagenr, __u8 *buf);
int get_current_sector_size (int fd);

/* APT Functions */
int apt_detect (int fd, int verbose);
int apt_is_apt (void);

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


/*
 * Sanitize Device FEATURE field values
 */
enum {
	SANITIZE_STATUS_EXT 			= 0x0000,
	SANITIZE_CRYPTO_SCRAMBLE_EXT 	= 0x0011,
	SANITIZE_BLOCK_ERASE_EXT		= 0x0012,
	SANITIZE_OVERWRITE_EXT			= 0x0014,
	SANITIZE_FREEZE_LOCK_EXT		= 0x0020,
	SANITIZE_ANTIFREEZE_LOCK_EXT	= 0x0040,
};

/*
 * Sanitize commands keys
 */
enum {
	SANITIZE_FREEZE_LOCK_KEY		= 0x46724C6B,	/* "FrLk" */
	SANITIZE_ANTIFREEZE_LOCK_KEY	= 0x416E7469,	/* "Anti" */
	SANITIZE_CRYPTO_SCRAMBLE_KEY	= 0x43727970,	/* "Cryp" */
	SANITIZE_BLOCK_ERASE_KEY		= 0x426B4572,	/* "BkEr" */
	SANITIZE_OVERWRITE_KEY			= 0x00004F57,	/* "OW"   */
};

/*
 * Sanitize states
 */
enum {
	SANITIZE_IDLE_STATE_SD0,
	SANITIZE_FROZEN_STATE_SD1,
	SANITIZE_OPERATION_IN_PROGRESS_SD2,
	SANITIZE_OPERATION_FAILED_SD3,
	SANITIZE_OPERATION_SUCCEEDED_SD4,
	SANITIZE_STATE_NUMBER
};

/*
 * Sanitize outputs flags
 */
enum {
	SANITIZE_FLAG_OPERATION_SUCCEEDED	= (1 << 7),
	SANITIZE_FLAG_OPERATION_IN_PROGRESS	= (1 << 6),
	SANITIZE_FLAG_DEVICE_IN_FROZEN		= (1 << 5),
	SANITIZE_FLAG_ANTIFREEZE_BIT		= (1 << 4),
};
/*
 * Sanitize devise error reason
 */
enum {
	SANITIZE_ERR_NO_REASON,
	SANITIZE_ERR_CMD_UNSUCCESSFUL,
	SANITIZE_ERR_CMD_UNSUPPORTED,
	SANITIZE_ERR_DEVICE_IN_FROZEN,
	SANITIZE_ERR_ANTIFREEZE_LOCK,
	SANITIZE_ERR_NUMBER
};
