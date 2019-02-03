/* prototypes and stuff for ATA command ioctls */

#include <linux/types.h>

enum {
	ATA_OP_DSM			= 0x06, // Data Set Management (TRIM)
	ATA_OP_READ_PIO			= 0x20,
	ATA_OP_READ_PIO_ONCE		= 0x21,
	ATA_OP_READ_LONG		= 0x22,
	ATA_OP_READ_LONG_ONCE		= 0x23,
	ATA_OP_READ_PIO_EXT		= 0x24,
	ATA_OP_READ_DMA_EXT		= 0x25,
	ATA_OP_READ_LOG_EXT		= 0x2f,
	ATA_OP_READ_FPDMA		= 0x60,	// NCQ
	ATA_OP_WRITE_PIO		= 0x30,
	ATA_OP_WRITE_LONG		= 0x32,
	ATA_OP_WRITE_LONG_ONCE		= 0x33,
	ATA_OP_WRITE_PIO_EXT		= 0x34,
	ATA_OP_WRITE_DMA_EXT		= 0x35,
	ATA_OP_WRITE_FPDMA		= 0x61,	// NCQ
	ATA_OP_READ_VERIFY		= 0x40,
	ATA_OP_READ_VERIFY_ONCE		= 0x41,
	ATA_OP_READ_VERIFY_EXT		= 0x42,
	ATA_OP_WRITE_UNC_EXT		= 0x45,	// lba48, no data, uses feat reg
	ATA_OP_FORMAT_TRACK		= 0x50,
	ATA_OP_DOWNLOAD_MICROCODE	= 0x92,
	ATA_OP_STANDBYNOW2		= 0x94,
	ATA_OP_CHECKPOWERMODE2		= 0x98,
	ATA_OP_SLEEPNOW2		= 0x99,
	ATA_OP_PIDENTIFY		= 0xa1,
	ATA_OP_READ_NATIVE_MAX		= 0xf8,
	ATA_OP_READ_NATIVE_MAX_EXT	= 0x27,
	ATA_OP_GET_NATIVE_MAX_EXT	= 0x78,
	ATA_OP_SMART			= 0xb0,
	ATA_OP_DCO			= 0xb1,
	ATA_OP_SET_SECTOR_CONFIGURATION = 0xb2,
	ATA_OP_SANITIZE			= 0xb4,
	ATA_OP_ERASE_SECTORS		= 0xc0,
	ATA_OP_READ_DMA			= 0xc8,
	ATA_OP_WRITE_DMA		= 0xca,
	ATA_OP_DOORLOCK			= 0xde,
	ATA_OP_DOORUNLOCK		= 0xdf,
	ATA_OP_STANDBYNOW1		= 0xe0,
	ATA_OP_IDLEIMMEDIATE		= 0xe1,
	ATA_OP_SETIDLE			= 0xe3,
	ATA_OP_SET_MAX			= 0xf9,
	ATA_OP_SET_MAX_EXT		= 0x37,
	ATA_OP_SET_MULTIPLE		= 0xc6,
	ATA_OP_CHECKPOWERMODE1		= 0xe5,
	ATA_OP_SLEEPNOW1		= 0xe6,
	ATA_OP_FLUSHCACHE		= 0xe7,
	ATA_OP_FLUSHCACHE_EXT		= 0xea,
	ATA_OP_IDENTIFY			= 0xec,
	ATA_OP_SETFEATURES		= 0xef,
	ATA_OP_SECURITY_SET_PASS	= 0xf1,
	ATA_OP_SECURITY_UNLOCK		= 0xf2,
	ATA_OP_SECURITY_ERASE_PREPARE	= 0xf3,
	ATA_OP_SECURITY_ERASE_UNIT	= 0xf4,
	ATA_OP_SECURITY_FREEZE_LOCK	= 0xf5,
	ATA_OP_SECURITY_DISABLE		= 0xf6,
	ATA_OP_VENDOR_SPECIFIC_0x80	= 0x80,
};

/*
 * Some useful ATA register bits
 */
enum {
	ATA_USING_LBA		= (1 << 6),
	ATA_STAT_DRQ		= (1 << 3),
	ATA_STAT_ERR		= (1 << 0),
};

/*
 * Useful parameters for init_hdio_taskfile():
 */
enum {	RW_READ			= 0,
	RW_WRITE		= 1,
	LBA28_OK		= 0,
	LBA48_FORCE		= 1,
};

/*
 * Definitions and structures for use with SG_IO + ATA_16:
 */
struct ata_lba_regs {
	__u8	feat;
	__u8	nsect;
	__u8	lbal;
	__u8	lbam;
	__u8	lbah;
};
struct ata_tf {
	__u8			dev;
	__u8			command;
	__u8			error;
	__u8			status;
	__u8			is_lba48;
	struct ata_lba_regs	lob;
	struct ata_lba_regs	hob;
};

/*
 * Definitions and structures for use with HDIO_DRIVE_TASKFILE:
 */

enum {
	/*
	 * These (redundantly) specify the category of the request
	 */
	TASKFILE_CMD_REQ_NODATA	= 0,	/* ide: IDE_DRIVE_TASK_NO_DATA */
	TASKFILE_CMD_REQ_IN	= 2,	/* ide: IDE_DRIVE_TASK_IN */
	TASKFILE_CMD_REQ_OUT	= 3,	/* ide: IDE_DRIVE_TASK_OUT */
	TASKFILE_CMD_REQ_RAW_OUT= 4,	/* ide: IDE_DRIVE_TASK_RAW_WRITE */
	/*
	 * These specify the method of transfer (pio, dma, multi, ..)
	 */
	TASKFILE_DPHASE_NONE	= 0,	/* ide: TASKFILE_IN */
	TASKFILE_DPHASE_PIO_IN	= 1,	/* ide: TASKFILE_IN */
	TASKFILE_DPHASE_PIO_OUT	= 4,	/* ide: TASKFILE_OUT */
};

union reg_flags {
	unsigned all				:16;
	struct {
	union {
		unsigned lob_all		: 8;
		struct {
			unsigned data		: 1;
			unsigned feat		: 1;
			unsigned lbal		: 1;
			unsigned nsect		: 1;
			unsigned lbam		: 1;
			unsigned lbah		: 1;
			unsigned dev		: 1;
			unsigned command	: 1;
		} lob;
	};
	union {
		unsigned hob_all		: 8;
		struct {
			unsigned data		: 1;
			unsigned feat		: 1;
			unsigned lbal		: 1;
			unsigned nsect		: 1;
			unsigned lbam		: 1;
			unsigned lbah		: 1;
			unsigned dev		: 1;
			unsigned command	: 1;
		} hob;
	};
	} bits;
} __attribute__((packed));

struct taskfile_regs {
	__u8	data;
	__u8	feat;
	__u8	nsect;
	__u8	lbal;
	__u8	lbam;
	__u8	lbah;
	__u8	dev;
	__u8	command;
};

struct hdio_taskfile {
	struct taskfile_regs	lob;
	struct taskfile_regs	hob;
	union reg_flags		oflags;
	union reg_flags		iflags;
	int			dphase;
	int			cmd_req;     /* IDE command_type */
	unsigned long		obytes;
	unsigned long		ibytes;
	__u16			data[0];
};

struct scsi_sg_io_hdr {
	int			interface_id;
	int			dxfer_direction;
	unsigned char		cmd_len;
	unsigned char		mx_sb_len;
	unsigned short		iovec_count;
	unsigned int		dxfer_len;
	void *			dxferp;
	unsigned char *		cmdp;
	void *			sbp;
	unsigned int		timeout;
	unsigned int		flags;
	int			pack_id;
	void *			usr_ptr;
	unsigned char		status;
	unsigned char		masked_status;
	unsigned char		msg_status;
	unsigned char		sb_len_wr;
	unsigned short		host_status;
	unsigned short		driver_status;
	int			resid;
	unsigned int		duration;
	unsigned int		info;
};

#ifndef SG_DXFER_NONE
	#define SG_DXFER_NONE		-1
	#define SG_DXFER_TO_DEV		-2
	#define SG_DXFER_FROM_DEV	-3
	#define SG_DXFER_TO_FROM_DEV	-4
#endif

#define SG_READ			0
#define SG_WRITE		1

#define SG_PIO			0
#define SG_DMA			1

#define SG_CHECK_CONDITION	0x02
#define SG_DRIVER_SENSE		0x08

#define SG_ATA_16		0x85
#define SG_ATA_16_LEN		16

#define SG_ATA_12		0xa1
#define SG_ATA_12_LEN		12

#define SG_ATA_LBA48		1
#define SG_ATA_PROTO_NON_DATA	( 3 << 1)
#define SG_ATA_PROTO_PIO_IN	( 4 << 1)
#define SG_ATA_PROTO_PIO_OUT	( 5 << 1)
#define SG_ATA_PROTO_DMA	( 6 << 1)
#define SG_ATA_PROTO_UDMA_IN	(11 << 1) /* not yet supported in libata */
#define SG_ATA_PROTO_UDMA_OUT	(12 << 1) /* not yet supported in libata */

void tf_init (struct ata_tf *tf, __u8 ata_op, __u64 lba, unsigned int nsect);
__u64 tf_to_lba (struct ata_tf *tf);
int sg16 (int fd, int rw, int dma, struct ata_tf *tf, void *data, unsigned int data_bytes, unsigned int timeout_secs);
int do_drive_cmd (int fd, unsigned char *args, unsigned int timeout);
int do_taskfile_cmd (int fd, struct hdio_taskfile *r, unsigned int timeout_secs);
int dev_has_sgio (int fd);
void init_hdio_taskfile (struct hdio_taskfile *r, __u8 ata_op, int rw, int force_lba48,
				__u64 lba, unsigned int nsect, int data_bytes);

/* APT */
int apt_sg16(int fd, int rw, int dma, struct ata_tf *tf,
		void *data, unsigned int data_bytes, unsigned int timeout_secs);
