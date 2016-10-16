
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#include <shutils.h>

#if defined (USE_USB_SUPPORT)
#include <netutils.h>
#include "usb_info.h"
#endif
#include "dev_info.h"

int get_device_type_by_device(const char *device_name)
{
	if (!device_name || strlen(device_name) < 2)
		return DEVICE_TYPE_UNKNOWN;

#if defined (USE_MMC_SUPPORT)
	if (!strncmp(device_name, "mmcblk", 6))
		return DEVICE_TYPE_MMC;
#endif

	if (!strncmp(device_name, "sd", 2))
		return DEVICE_TYPE_SCSI_DISK;

	if (!strncmp(device_name, "sg", 2))
		return DEVICE_TYPE_SG;

	if (!strncmp(device_name, "sr", 2))
		return DEVICE_TYPE_CD;

#if defined (USE_USB_SUPPORT)
	if (!strncmp(device_name, "lp", 2))
		return DEVICE_TYPE_PRINTER;

	if (isSerialNode(device_name) || isACMNode(device_name))
		return DEVICE_TYPE_MODEM_TTY;

	if (is_usbnet_interface(device_name))
		return DEVICE_TYPE_MODEM_ETH;
#endif

	return DEVICE_TYPE_UNKNOWN;
}

char *get_blk_sd_path_by_device(const char *device_name, char *buf, int buf_size)
{
	char sysblock_n[] = "/sys/block/sda/device";
	int len;

	if (strlen(device_name) < 3)
		return NULL;

	sysblock_n[13] = device_name[2];

	len = readlink(sysblock_n, buf, buf_size);
	if (len > 0 && len < buf_size) {
		buf[len] = '\0';
		return buf;
	}

	return NULL;
}

