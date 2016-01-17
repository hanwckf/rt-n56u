#ifndef __USB_INFO__
#define __USB_INFO__

#include <ralink_boards.h>

//#define DEBUG_USB

#ifdef DEBUG_USB
#define usb_dbg(fmt, args...) do{ \
		FILE *fp = fopen("/tmp/usb.log", "a+"); \
		if(fp){ \
			fprintf(fp, "[usb_dbg: %s] ", __FUNCTION__); \
			fprintf(fp, fmt, ## args); \
			fclose(fp); \
		} \
	}while(0)
#else
#define usb_dbg(fmt, args...) do{}while(0)
#endif

#define SYS_TTY			"/sys/class/tty"
#define SYS_NET			"/sys/class/net"
#define SYS_PRINTER		"/sys/class/usb"
#define SYS_SG			"/sys/class/scsi_generic"
#define USB_DEVICE_PATH		"/sys/bus/usb/devices"
#define USB_BUS_PATH		"/proc/bus/usb/devices"

#if BOARD_USB_PORT_SWAP
#define USB_EHCI_PORT_2		"1-1"
#define USB_OHCI_PORT_2		"2-1"
#define USB_EHCI_PORT_1		"1-2"
#define USB_OHCI_PORT_1		"2-2"
#else
#define USB_EHCI_PORT_1		"1-1"
#define USB_OHCI_PORT_1		"2-1"
#define USB_EHCI_PORT_2		"1-2"
#define USB_OHCI_PORT_2		"2-2"
#endif

#define USB_PORT_HAS_MODEM_TTY	0x02
#define USB_PORT_HAS_MODEM_ETH	0x04
#define USB_PORT_HAS_PRINTER	0x08

typedef struct usb_info_t usb_info_t;
struct usb_info_t {
	char *manuf;
	char *product;
	int port_root;
	int port_speed;
	int id_devnum;
	int id_parent;
	int id_port;
	int dev_type;
	int dev_cls;
	int dev_sub;
	int dev_prt;
	int dev_vid;
	int dev_pid;
	usb_info_t *prev;
	usb_info_t *next;
};

extern int  get_usb_root_port_by_sd_device(const char *device_name);
extern char *get_usb_path_by_device(const char *device_name, char *buf, const int buf_size);
extern char *get_usb_port_by_interface_string(const char *target_string, char *buf, const int buf_size);
extern char *get_usb_port_by_string(const char *target_string, char *buf, const int buf_size);
extern char *get_usb_port_by_device(const char *device_name, char *buf, const int buf_size);
extern char *get_usb_interface_by_string(const char *target_string, char *buf, const int buf_size);
extern char *get_usb_interface_by_device(const char *device_name, char *buf, const int buf_size);

extern char *get_usb_param(const char *usb_string, const char *param, char *buf, const int buf_size);
extern char *get_usb_vid(const char *usb_port_id, char *buf, const int buf_size);
extern char *get_usb_pid(const char *usb_port_id, char *buf, const int buf_size);
extern char *get_usb_manufact(const char *usb_port_id, char *buf, const int buf_size);
extern char *get_usb_product(const char *usb_port_id, char *buf, const int buf_size);
extern char *get_usb_serial(const char *usb_port_id, char *buf, const int buf_size);
extern char *get_usb_interface_class(const char *usb_interface_id, char *buf, const int buf_size);
extern char *get_usb_interface_subclass(const char *usb_interface_id, char *buf, const int buf_size);
extern int  get_usb_interface_numendpoints(const char *usb_interface_id);
extern int  get_usb_interface_Int_endpoint(const char *usb_interface_id);
extern int  get_usb_devnum(const char *usb_port_id);
extern int  get_usb_interface_number(const char *usb_port_id);

extern int  isSerialNode(const char *device_name);
extern int  isACMNode(const char *device_name);
extern int  isWDMNode(const char *device_name);
extern int  isSerialInterface(const char *interface_class);
extern int  isACMInterface(const char *interface_class, const char *interface_subclass);
extern int  isStorageInterface(const char *interface_class);

extern int  has_usb_devices(void);
extern usb_info_t *get_usb_info(void);
extern void free_usb_info(usb_info_t *usb_info_list);

#endif
