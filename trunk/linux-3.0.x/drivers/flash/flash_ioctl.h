#ifndef	__FLASH_IOCTL_H__
#define	__FLASH_IOCTL_H__

#define FLASH_IOCTL_READ		(0x01)
#define FLASH_IOCTL_WRITE		(0x02)
#define FLASH_IOCTL_ERASE		(0x03) 
#define FLASH_IOCTL_MII_MGR_GET		(0x04) 
#define FLASH_IOCTL_MII_MGR_SET		(0x05) 

#if defined (CONFIG_RT2880_FLASH_32M)
#define FLASH_MAX_RW_SIZE		(0x20000)
#else
#define FLASH_MAX_RW_SIZE		(0x10000)
#endif
#define FLASH_DEVNAME			"flash0"

enum op_status {
    OUT_OF_SCOPE=10,
};

struct flash_opt {
    unsigned int    *src;
    unsigned int    *dest;
    unsigned int    bytes;
    unsigned int    start_addr;
    unsigned int    end_addr;
    enum op_status  result;
};

#endif
