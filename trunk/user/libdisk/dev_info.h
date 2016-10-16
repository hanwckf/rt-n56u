#ifndef __DEV_INFO__
#define __DEV_INFO__

#define SYS_MODULE	"/sys/module"
#define SYS_BLOCK	"/sys/block"

enum {
	DEVICE_TYPE_UNKNOWN=0,
	DEVICE_TYPE_MMC,
	DEVICE_TYPE_SCSI_DISK,
	DEVICE_TYPE_SG,
	DEVICE_TYPE_CD,
	DEVICE_TYPE_USB_HUB,
	DEVICE_TYPE_PRINTER,
	DEVICE_TYPE_MODEM_TTY,
	DEVICE_TYPE_MODEM_ETH
};

extern int   get_device_type_by_device(const char *device_name);
extern char *get_blk_sd_path_by_device(const char *device_name, char *buf, int buf_size);


#endif
