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
#include <netutils.h>

#include "dev_info.h"
#include "usb_info.h"
#include "disk_io_tools.h"

int get_usb_root_port_by_sd_device(const char *device_name)
{
	char usb_path[PATH_MAX];

	if (!get_blk_sd_path_by_device(device_name, usb_path, sizeof(usb_path)))
		return -1;

	// ../../devices/platform/xhci-hcd/usb2/2-1/2-1:1.0/host0/target0:0:0/0:0:0:0

	if (strstr(usb_path, USB_EHCI_PORT_1) || strstr(usb_path, USB_OHCI_PORT_1))
		return 1;

	if (strstr(usb_path, USB_EHCI_PORT_2) || strstr(usb_path, USB_OHCI_PORT_2))
		return 2;

	return -1;
}

char *get_usb_path_by_device(const char *device_name, char *buf, const int buf_size)
{
	int device_type;
	char device_path[128];

	device_type = get_device_type_by_device(device_name);
	if(device_type == DEVICE_TYPE_UNKNOWN ||
	   device_type == DEVICE_TYPE_MMC)
		return NULL;

	memset(buf, 0, sizeof(buf_size));

	if(device_type == DEVICE_TYPE_SCSI_DISK){
		return get_blk_sd_path_by_device(device_name, buf, buf_size);
	}
	else
	if(device_type == DEVICE_TYPE_PRINTER){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_PRINTER, device_name);
		if(realpath(device_path, buf) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_SG){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_SG, device_name);
		if(realpath(device_path, buf) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_CD){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_BLOCK, device_name);
		if(realpath(device_path, buf) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_MODEM_TTY){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_TTY, device_name);
		if(realpath(device_path, buf) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.
			if(realpath(device_path, buf) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
	if(device_type == DEVICE_TYPE_MODEM_ETH){
		snprintf(device_path, sizeof(device_path), "%s/%s/device", SYS_NET, device_name);
		if(realpath(device_path, buf) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.
			if(realpath(device_path, buf) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
		return NULL;

	return buf;
}

char *get_usb_port_by_interface_string(const char *target_string, char *buf, const int buf_size)
{
	char *ptr;

	snprintf(buf, buf_size, "%s", target_string);
	ptr = strchr(buf, ':');
	if (ptr) {
		*ptr = '\0';
		return buf;
	}

	return NULL;
}

char *get_usb_port_by_string(const char *target_string, char *buf, const int buf_size)
{
	// example 1: device in root port
	// ../usb1/1-1/1-1:1.0/net/usb0

	// example 2: device in external hub port
	// ../usb1/1-2/1-2.4/1-2.4:1.0/net/usb0
	
	// example 3: device in second external hub port
	// ../usb1/1-2/1-2.3/1-2.3.1/1-2.3.1:1.2/net/usb0

	if (get_usb_interface_by_string(target_string, buf, buf_size)) {
		char *ptr = strchr(buf, ':');
		if (ptr) {
			*ptr = '\0';
			return buf;
		}
	}

	return NULL;
}

char *get_usb_port_by_device(const char *device_name, char *buf, const int buf_size)
{
	char usb_path[PATH_MAX];

	if (!get_usb_path_by_device(device_name, usb_path, sizeof(usb_path)))
		return NULL;

	if (!get_usb_port_by_string(usb_path, buf, buf_size)) {
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}

char *get_usb_interface_by_string(const char *target_string, char *buf, const int buf_size)
{
	char *ptr, *ptr2, *ptr_start, *ptr_end;
	int len;

	// example 1: device in root port
	// ../usb1/1-1/1-1:1.0/net/usb0

	// example 2: device in external hub port
	// ../usb1/1-2/1-2.4/1-2.4:1.0/net/usb0
	
	// example 3: device in second external hub port
	// ../usb1/1-2/1-2.3/1-2.3.1/1-2.3.1:1.2/net/usb0

	if((ptr = strstr(target_string, USB_EHCI_PORT_1)) != NULL)
		ptr += strlen(USB_EHCI_PORT_1);
	else if((ptr = strstr(target_string, USB_EHCI_PORT_2)) != NULL)
		ptr += strlen(USB_EHCI_PORT_2);
	else if((ptr = strstr(target_string, USB_OHCI_PORT_1)) != NULL)
		ptr += strlen(USB_OHCI_PORT_1);
	else if((ptr = strstr(target_string, USB_OHCI_PORT_2)) != NULL)
		ptr += strlen(USB_OHCI_PORT_2);
	else
		return NULL;

	ptr++;
	ptr2 = ptr;

	while (*ptr && *ptr != ':')
		ptr++;

	if (*ptr != ':')
		return NULL;

	while ((ptr_start = strchr(ptr2, '/')) && ((ptr_start+1) < ptr)) {
		ptr_start++;
		ptr2 = ptr_start;
	}

	if (!ptr_start || (ptr_start+1) >= ptr)
		ptr_start = ptr2;

	if((ptr_end = strchr(ptr, '/')) == NULL)
		ptr_end = ptr+strlen(ptr);

	len = ptr_end-ptr_start;
	if (len < 2)
		return NULL;

	memset(buf, 0, buf_size);
	strncpy(buf, ptr_start, len); // ex: 1-1:1.0/~

	return buf;
}

char *get_usb_interface_by_device(const char *device_name, char *buf, const int buf_size)
{
	char usb_path[PATH_MAX];

	if (!get_usb_path_by_device(device_name, usb_path, sizeof(usb_path)))
		return NULL;

	if (!get_usb_interface_by_string(usb_path, buf, buf_size)) {
		usb_dbg("(%s): Fail to get interface: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}

char *get_usb_param(const char *usb_string, const char *param, char *buf, const int buf_size)
{
	FILE *fp;
	char target_file[128], *ptr;
	int len;

	if (!usb_string)
		return NULL;

	snprintf(target_file, sizeof(target_file), "%s/%s/%s", USB_DEVICE_PATH, usb_string, param);
	if((fp = fopen(target_file, "r")) == NULL)
		return NULL;

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

char *get_usb_vid(const char *usb_port_id, char *buf, const int buf_size)
{
	return get_usb_param(usb_port_id, "idVendor", buf, buf_size);
}

char *get_usb_pid(const char *usb_port_id, char *buf, const int buf_size)
{
	return get_usb_param(usb_port_id, "idProduct", buf, buf_size);
}

char *get_usb_manufact(const char *usb_port_id, char *buf, const int buf_size)
{
	return get_usb_param(usb_port_id, "manufacturer", buf, buf_size);
}

char *get_usb_product(const char *usb_port_id, char *buf, const int buf_size)
{
	return get_usb_param(usb_port_id, "product", buf, buf_size);
}

char *get_usb_serial(const char *usb_port_id, char *buf, const int buf_size)
{
	return get_usb_param(usb_port_id, "serial", buf, buf_size);
}

int get_usb_devnum(const char *usb_port_id)
{
	char buf[32];
	if (!get_usb_param(usb_port_id, "devnum", buf, sizeof(buf)))
		return 0;

	return atoi(buf);
}

int get_usb_interface_number(const char *usb_port_id)
{
	char buf[16];
	if (!get_usb_param(usb_port_id, "bNumInterfaces", buf, sizeof(buf)))
		return 0;

	return atoi(buf);
}

int get_usb_interface_numendpoints(const char *usb_interface_id)
{
	char buf[16];
	if (!get_usb_param(usb_interface_id, "bNumEndpoints", buf, sizeof(buf)))
		return 0;

	return atoi(buf);
}

char *get_usb_interface_class(const char *usb_interface_id, char *buf, const int buf_size)
{
	return get_usb_param(usb_interface_id, "bInterfaceClass", buf, buf_size);
}

char *get_usb_interface_subclass(const char *usb_interface_id, char *buf, const int buf_size)
{
	return get_usb_param(usb_interface_id, "bInterfaceSubClass", buf, buf_size);
}

int get_usb_interface_Int_endpoint(const char *usb_interface_id)
{
	FILE *fp;
	char interface_path[128], bmAttributes_file[270], buf[8], *ptr;
	DIR *interface_dir = NULL;
	struct dirent *end_name;
	int bNumEndpoints, end_count, got_Int = 0;

	// Get bNumEndpoints.
	bNumEndpoints = get_usb_interface_numendpoints(usb_interface_id);
	if (bNumEndpoints <= 0){
		usb_dbg("(%s): No endpoints: %d.\n", usb_interface_id, bNumEndpoints);
		return 0;
	}

	snprintf(interface_path, sizeof(interface_path), "%s/%s", USB_DEVICE_PATH, usb_interface_id);
	if((interface_dir = opendir(interface_path)) == NULL){
		usb_dbg("(%s): Fail to open dir: %s.\n", usb_interface_id, interface_path);
		return 0;
	}

	end_count = 0;
	while((end_name = readdir(interface_dir)) != NULL){
		if(strncmp(end_name->d_name, "ep_", 3))
			continue;

		++end_count;

		snprintf(bmAttributes_file,sizeof(bmAttributes_file), "%s/%s/bmAttributes", interface_path, end_name->d_name);

		if((fp = fopen(bmAttributes_file, "r")) == NULL){
			usb_dbg("(%s): Fail to open file: %s.\n", usb_interface_id, bmAttributes_file);
			continue;
		}

		memset(buf, 0, sizeof(buf));
		ptr = fgets(buf, sizeof(buf), fp);
		fclose(fp);
		if(ptr == NULL)
			return 0;

		if(!strncmp(buf, "03", 2)){
			got_Int = 1;
			break;
		}
		else if(end_count == bNumEndpoints)
			break;
	}
	closedir(interface_dir);

	return got_Int;
}

int isSerialNode(const char *device_name)
{
	if(strncmp(device_name, "ttyUSB", 6))
		return 0;

	return 1;
}

int isACMNode(const char *device_name)
{
	if(strncmp(device_name, "ttyACM", 6))
		return 0;

	return 1;
}

int isWDMNode(const char *device_name)
{
	if(strncmp(device_name, "cdc-wdm", 7))
		return 0;

	return 1;
}

int isSerialInterface(const char *interface_class)
{
	if(strcmp(interface_class, "ff") == 0)
		return 1;

	return 0;
}

int isACMInterface(const char *interface_class, const char *interface_subclass)
{
	if(strcmp(interface_class, "02") == 0 && strcmp(interface_subclass, "02") == 0)
		return 1;

	return 0;
}

int isStorageInterface(const char *interface_class)
{
	if(strcmp(interface_class, "08") == 0)
		return 1;

	return 0;
}

int has_usb_devices(void)
{
	FILE *fp;
	int id_parent, dev_found;
	char line[128];

	fp = fopen(USB_BUS_PATH, "r");
	if (!fp)
		return 0;

	dev_found = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] != 'T')
			continue;
		
		id_parent = get_param_int(line, "Prnt=", 10, -1);
#if defined (BOARD_GPIO_LED_USB2)
		if (id_parent == 1) {
			int id_port = get_param_int(line, "Port=", 10, 0);
#if BOARD_USB_PORT_SWAP
			id_port = (id_port) ? 0 : 1;
#endif
			switch (id_port)
			{
			case 0:
				dev_found |= 0x1;
				break;
			case 1:
				dev_found |= 0x2;
				break;
			}
			if ((dev_found & 0x3) == 0x3)
				break;
		}
#else
		if (id_parent > 0) {
			dev_found = 1;
			break;
		}
#endif
	}

	fclose(fp);

	return dev_found;
}

usb_info_t *get_usb_info(void)
{
	FILE *fp;
	char line[256];
	int skip;
	usb_info_t *usb_info_list, *new_dev, *old_dev;

	fp = fopen(USB_BUS_PATH, "r");
	if (!fp)
		return NULL;

	if (!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return NULL;
	}

	skip = 0;
	old_dev = NULL;
	new_dev = NULL;
	usb_info_list = NULL;
	while (fgets(line, sizeof(line), fp)) {
		char *ptr;
		int id_parent;
		
		if ((ptr = strchr(line, '\n')))
			*ptr = 0;

		if (skip && (line[0] != 'I' || line[0] != 'E'))
			continue;
		else
			skip = 0;

		if (!new_dev && line[0] != 'T')
			continue;

		switch (line[0])
		{
		case 'T':
			/* skip internal hubs */
			id_parent = get_param_int(line, "Prnt=", 10, 0);
			if (id_parent == 0) {
				new_dev = NULL;
				continue;
			}
			new_dev = malloc(sizeof(usb_info_t));
			if (!new_dev)
				goto usb_info_exit;
			
			memset(new_dev, 0, sizeof(usb_info_t));
			new_dev->dev_type = DEVICE_TYPE_UNKNOWN;
			new_dev->id_parent = id_parent;
			new_dev->id_port = get_param_int(line, "Port=", 10, 0);
			new_dev->id_devnum = get_param_int(line, "Dev#=", 10, 0);
			new_dev->port_speed = get_param_int(line, "Spd=", 10, 0);
			if (id_parent == 1) {
#if BOARD_USB_PORT_SWAP
				new_dev->id_port = (new_dev->id_port) ? 0 : 1;
#endif
				new_dev->port_root = (new_dev->id_port + 1);
			}
			if (!usb_info_list)
				usb_info_list = new_dev;
			else {
				old_dev->next = new_dev;
				new_dev->prev = old_dev;
			}
			old_dev = new_dev;
			break;
		case 'D':
			new_dev->dev_cls = get_param_int(line, "Cls=", 16, 0);
			new_dev->dev_sub = get_param_int(line, "Sub=", 16, 0);
			new_dev->dev_prt = get_param_int(line, "Prot=", 16, 0);
			if (new_dev->dev_cls == 0x09 && new_dev->dev_sub == 0)
				new_dev->dev_type = DEVICE_TYPE_USB_HUB;
			else if (new_dev->dev_cls == 0x07)
				new_dev->dev_type = DEVICE_TYPE_PRINTER;
			break;
		case 'P':
			new_dev->dev_vid = get_param_int(line, "Vendor=", 16, 0);
			new_dev->dev_pid = get_param_int(line, "ProdID=", 16, 0);
			break;
		case 'S':
			if (!new_dev->manuf)
				new_dev->manuf = get_param_str(line, "Manufacturer=", 1);
			if (!new_dev->product)
				new_dev->product = get_param_str(line, "Product=", 1);
			if (new_dev->manuf) {
				strntrim(new_dev->manuf);
				sanity_name(new_dev->manuf);
			}
			if (new_dev->product) {
				strntrim(new_dev->product);
				sanity_name(new_dev->product);
			}
			break;
		case 'C':
			if (line[2] != '*')
				skip = 1;
			break;
		case 'I':
			if (new_dev->dev_type != DEVICE_TYPE_UNKNOWN &&
			    new_dev->dev_type != DEVICE_TYPE_MODEM_TTY &&
			    new_dev->dev_type != DEVICE_TYPE_SCSI_DISK)
				continue;
			ptr = get_param_str(line, "Driver=", 0);
			if (strcmp(ptr, "usb-storage") == 0) {
				/* for combined devices (modem+storage) use modem as primary */
				if (new_dev->dev_type != DEVICE_TYPE_MODEM_TTY)
					new_dev->dev_type = DEVICE_TYPE_SCSI_DISK;
			}
			else if (strcmp(ptr, "usblp") == 0)
				new_dev->dev_type = DEVICE_TYPE_PRINTER;
			else if (strcmp(ptr, "option") == 0 ||
				 strcmp(ptr, "sierra") == 0 ||
				 strcmp(ptr, "qcserial") == 0 ||
				 strcmp(ptr, "cdc_acm") == 0 ||
				 strcmp(ptr, "cdc-acm") == 0)
				new_dev->dev_type = DEVICE_TYPE_MODEM_TTY;
			else if (strcmp(ptr, "rndis_host") == 0 ||
				 strcmp(ptr, "qmi_wwan") == 0 ||
				 strcmp(ptr, "cdc_mbim") == 0 ||
				 strcmp(ptr, "huawei_cdc_ncm") == 0 ||
				 strcmp(ptr, "cdc_ncm") == 0 ||
				 strcmp(ptr, "sierra_net") == 0 ||
				 strcmp(ptr, "cdc_ether") == 0)
				new_dev->dev_type = DEVICE_TYPE_MODEM_ETH;
			break;
		default:
			break;
		}
	}

usb_info_exit:
	fclose(fp);

	for (new_dev = usb_info_list; new_dev != NULL; new_dev = new_dev->next) {
		if (!new_dev->port_root) {
			usb_info_t *tmp = new_dev->prev;
			while (tmp && tmp->id_devnum != new_dev->id_parent)
				tmp = tmp->prev;
			if (tmp && tmp->port_root)
				new_dev->port_root = tmp->port_root;
		}
	}

	return usb_info_list;
}

void free_usb_info(usb_info_t *usb_info_list)
{
	usb_info_t *follow_dev, *old_dev;

	if(!usb_info_list)
		return;

	follow_dev = usb_info_list;
	while (follow_dev) {
		if(follow_dev->manuf)
			free(follow_dev->manuf);
		if(follow_dev->product)
			free(follow_dev->product);
		old_dev = follow_dev;
		follow_dev = follow_dev->next;
		free(old_dev);
	}
}

