#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "usb_info.h"

#include <nvram/bcmnvram.h>
#include <shutils.h>

#include <fcntl.h>
#include <errno.h>

extern int file_lock(char *tag)
{
	char fn[64];
	struct flock lock;
	int lockfd = -1;
	pid_t lockpid;

	sprintf(fn, "/var/lock/%s.lock", tag);
	if ((lockfd = open(fn, O_CREAT | O_RDWR, 0666)) < 0)
		goto lock_error;

	pid_t pid = getpid();
	if (read(lockfd, &lockpid, sizeof(pid_t))) {
		// check if we already hold a lock
		if (pid == lockpid) {
			// don't close the file here as that will release all locks
			return -1;
		}
	}

	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_pid = pid;

	if (fcntl(lockfd, F_SETLKW, &lock) < 0) {
		close(lockfd);
		goto lock_error;
	}

	lseek(lockfd, 0, SEEK_SET);
	write(lockfd, &pid, sizeof(pid_t));
	return lockfd;
lock_error:
	// No proper error processing
	usb_dbg("Error %d locking %s, proceeding anyway", errno, fn);
	return -1;
}

extern void file_unlock(int lockfd)
{
	if (lockfd >= 0) {
		ftruncate(lockfd, 0);
		close(lockfd);
	}
}

char ehci_string[32];
char ohci_string[32];

extern char *get_usb_ehci_port(int port){
	char word[100], *next;
	char *ports = nvram_safe_get("ehci_ports");
	int i=0;

	strcpy(ehci_string, "xxxxxxxx");

	foreach(word, ports, next) {
		if(i==port) {
			strcpy(ehci_string, word);
		}		
		i++;
	}
	return ehci_string;
}

extern char *get_usb_ohci_port(int port){
	char word[100], *next;
	char *ports = nvram_safe_get("ohci_ports");
	int i=0;

	strcpy(ohci_string, "xxxxxxxx");

	foreach(word, ports, next) {
		if(i==port) {
			strcpy(ohci_string, word);
		}		
		i++;
	}
	return ohci_string;
}

extern int get_usb_port_number(const char *usb_port){
	char word[100], *next;
	char *ports;
	int port_num, i;

	port_num = 0;
	ports = nvram_safe_get("ehci_ports");
	i = 0;
	foreach(word, ports, next){
		++i;
		if(!strcmp(usb_port, word)){
			port_num = i;
			break;
		}
	}

	ports = nvram_safe_get("ohci_ports");
	i = 0;
	if(port_num == 0){
		foreach(word, ports, next){
			++i;
			if(!strcmp(usb_port, word)){
				port_num = i;
				break;
			}
		}
	}

	return port_num;
}

extern int get_device_type_by_device(const char *device_name){
	if(device_name == NULL || strlen(device_name) <= 0){
		usb_dbg("(%s): The device name is not correct.\n", device_name);
		return DEVICE_TYPE_UNKNOWN;
	}

	if(!strncmp(device_name, "sd", 2)
#ifdef SUPPORT_IDE_SATA_DISK
			|| !strncmp(device_name, "hd", 2)
#endif
			){
		return DEVICE_TYPE_DISK;
	}
#ifdef RTCONFIG_USB_PRINTER
	if(!strncmp(device_name, "lp", 2)){
		return DEVICE_TYPE_PRINTER;
	}
#endif
	if(!strncmp(device_name, "sg", 2)){
		return DEVICE_TYPE_SG;
	}
	if(!strncmp(device_name, "sr", 2)){
		return DEVICE_TYPE_CD;
	}
	if(isSerialNode(device_name) || isACMNode(device_name) || isWDMNode(device_name)){
		return DEVICE_TYPE_MODEM;
	}
	if(isUsbNetIf(device_name)){
		return DEVICE_TYPE_USBETH;
	}

	return DEVICE_TYPE_UNKNOWN;
}

extern char *get_usb_port_by_string(const char *target_string, char *buf, const int buf_size){
	memset(buf, 0, buf_size);

	if(strstr(target_string, USB_EHCI_PORT_1))
		strcpy(buf, USB_EHCI_PORT_1);
	else if(strstr(target_string, USB_EHCI_PORT_2))
		strcpy(buf, USB_EHCI_PORT_2);
	else if(strstr(target_string, USB_OHCI_PORT_1))
		strcpy(buf, USB_OHCI_PORT_1);
	else if(strstr(target_string, USB_OHCI_PORT_2))
		strcpy(buf, USB_OHCI_PORT_2);
	else
		return NULL;

	return buf;
}

extern char *get_usb_port_by_device(const char *device_name, char *buf, const int buf_size){
	int device_type = get_device_type_by_device(device_name);
	char device_path[128], usb_path[PATH_MAX];
	char disk_name[4];

	if(device_type == DEVICE_TYPE_UNKNOWN)
		return NULL;

	memset(device_path, 0, 128);
	memset(usb_path, 0, PATH_MAX);

	if(device_type == DEVICE_TYPE_DISK){
		memset(disk_name, 0, 4);
		strncpy(disk_name, device_name, 3);
		sprintf(device_path, "%s/%s/device", SYS_BLOCK, disk_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#ifdef RTCONFIG_USB_PRINTER
	if(device_type == DEVICE_TYPE_PRINTER){
		sprintf(device_path, "%s/%s/device", SYS_PRINTER, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#endif
	if(device_type == DEVICE_TYPE_SG){
		sprintf(device_path, "%s/%s/device", SYS_SG, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_CD){
		sprintf(device_path, "%s/%s/device", SYS_BLOCK, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_MODEM){
		sprintf(device_path, "%s/%s/device", SYS_TTY, device_name);
		if(realpath(device_path, usb_path) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.
			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
	if(device_type == DEVICE_TYPE_USBETH){
		sprintf(device_path, "%s/%s/device", SYS_NET, device_name);
		if(realpath(device_path, usb_path) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.
			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
		return NULL;

	if(get_usb_port_by_string(usb_path, buf, buf_size) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}

extern char *get_interface_by_string(const char *target_string, char *buf, const int buf_size){
	char *ptr, *ptr_end;
	int len;

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

	++ptr;
	if((ptr_end = strchr(ptr, '/')) == NULL)
		ptr_end = ptr+strlen(ptr);

	if((len = ptr_end-ptr) < 0)
		len = ptr-ptr_end;

	memset(buf, 0, buf_size);
	strncpy(buf, ptr, len); // ex: 1-1:1.0/~

	return buf;
}

extern char *get_interface_by_device(const char *device_name, char *buf, const int buf_size){
	int device_type = get_device_type_by_device(device_name);
	char device_path[128], usb_path[PATH_MAX];
	char disk_name[4];

	if(device_type == DEVICE_TYPE_UNKNOWN)
		return NULL;

	memset(device_path, 0, 128);
	memset(usb_path, 0, PATH_MAX);

	if(device_type == DEVICE_TYPE_DISK){
		memset(disk_name, 0, 4);
		strncpy(disk_name, device_name, 3);
		sprintf(device_path, "%s/%s/device", SYS_BLOCK, disk_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#ifdef RTCONFIG_USB_PRINTER
	if(device_type == DEVICE_TYPE_PRINTER){
		sprintf(device_path, "%s/%s/device", SYS_PRINTER, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
#endif
	if(device_type == DEVICE_TYPE_SG){
		sprintf(device_path, "%s/%s/device", SYS_SG, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_CD){
		sprintf(device_path, "%s/%s/device", SYS_BLOCK, device_name);
		if(realpath(device_path, usb_path) == NULL){
			usb_dbg("(%s): Fail to get link: %s.\n", device_name, device_path);
			return NULL;
		}
	}
	else
	if(device_type == DEVICE_TYPE_MODEM){
		sprintf(device_path, "%s/%s/device", SYS_TTY, device_name);
		if(realpath(device_path, usb_path) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.
			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
	if(device_type == DEVICE_TYPE_USBETH){
		sprintf(device_path, "%s/%s/device", SYS_NET, device_name);
		if(realpath(device_path, usb_path) == NULL){
			sleep(1); // Sometimes link would be built slowly, so try again.
			if(realpath(device_path, usb_path) == NULL){
				usb_dbg("(%s)(2/2): Fail to get link: %s.\n", device_name, device_path);
				return NULL;
			}
		}
	}
	else
		return NULL;

	if(get_interface_by_string(usb_path, buf, buf_size) == NULL){
		usb_dbg("(%s): Fail to get usb port: %s.\n", device_name, usb_path);
		return NULL;
	}

	return buf;
}

extern char *get_usb_vid(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[8], target_file[128], *ptr;
	int len;

	if(usb_port == NULL || get_usb_port_by_string(usb_port, check_usb_port, sizeof(check_usb_port)) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/idVendor", USB_DEVICE_PATH, usb_port);
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

extern char *get_usb_pid(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[8], target_file[128], *ptr;
	int len;

	if(usb_port == NULL || get_usb_port_by_string(usb_port, check_usb_port, sizeof(check_usb_port)) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/idProduct", USB_DEVICE_PATH, usb_port);
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

extern char *get_usb_manufacturer(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[8], target_file[128], *ptr;
	int len;

	if(usb_port == NULL || get_usb_port_by_string(usb_port, check_usb_port, sizeof(check_usb_port)) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/manufacturer", USB_DEVICE_PATH, usb_port);
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

extern char *get_usb_product(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[8], target_file[128], *ptr;
	int len;

	if(usb_port == NULL || get_usb_port_by_string(usb_port, check_usb_port, sizeof(check_usb_port)) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/product", USB_DEVICE_PATH, usb_port);
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

extern char *get_usb_serial(const char *usb_port, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[8], target_file[128], *ptr;
	int len;

	if(usb_port == NULL || get_usb_port_by_string(usb_port, check_usb_port, sizeof(check_usb_port)) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return NULL;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/serial", USB_DEVICE_PATH, usb_port);
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

extern int get_usb_interface_number(const char *usb_port){
	FILE *fp;
	char target_file[128], buf[8], *ptr;

	if(usb_port == NULL || get_usb_port_by_string(usb_port, buf, sizeof(buf)) == NULL || strlen(usb_port) != strlen(USB_EHCI_PORT_1))
		return 0;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/bNumInterfaces", USB_DEVICE_PATH, usb_port);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, sizeof(buf));
	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if(ptr == NULL)
		return 0;

	return atoi(buf);
}

extern char *get_usb_interface_class(const char *interface_name, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[8], target_file[128], *ptr;
	int retry, len;

	if(interface_name == NULL || get_usb_port_by_string(interface_name, check_usb_port, sizeof(check_usb_port)) == NULL)
		return NULL;

	snprintf(target_file, sizeof(target_file), "%s/%s/bInterfaceClass", USB_DEVICE_PATH, interface_name);
	retry = 0;
	while((fp = fopen(target_file, "r")) == NULL && retry < MAX_WAIT_FILE){
		++retry;
		sleep(1); // Sometimes the class file would be built slowly, so try again.
	}

	if(fp == NULL){
		return NULL;
	}

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

extern char *get_usb_interface_subclass(const char *interface_name, char *buf, const int buf_size){
	FILE *fp;
	char check_usb_port[8], target_file[128], *ptr;
	int retry, len;

	if(interface_name == NULL || get_usb_port_by_string(interface_name, check_usb_port, sizeof(check_usb_port)) == NULL)
		return NULL;

	snprintf(target_file, sizeof(target_file), "%s/%s/bInterfaceSubClass", USB_DEVICE_PATH, interface_name);
	retry = 0;
	while((fp = fopen(target_file, "r")) == NULL && retry < MAX_WAIT_FILE){
		++retry;
		sleep(1); // Sometimes the class file would be built slowly, so try again.
	}

	if(fp == NULL){
		return NULL;
	}

	memset(buf, 0, buf_size);
	ptr = fgets(buf, buf_size, fp);
	fclose(fp);
	if(ptr == NULL)
		return NULL;

	len = strlen(buf);
	buf[len-1] = 0;

	return buf;
}

extern int get_interface_numendpoints(const char *interface_name){
	FILE *fp;
	char target_file[128], buf[8], *ptr;

	if(interface_name == NULL || get_usb_port_by_string(interface_name, buf, sizeof(buf)) == NULL)
		return 0;

	memset(target_file, 0, 128);
	sprintf(target_file, "%s/%s/bNumEndpoints", USB_DEVICE_PATH, interface_name);
	if((fp = fopen(target_file, "r")) == NULL)
		return 0;

	memset(buf, 0, sizeof(buf));
	ptr = fgets(buf, sizeof(buf), fp);
	fclose(fp);
	if(ptr == NULL)
		return 0;

	return atoi(buf);
}

extern int get_interface_Int_endpoint(const char *interface_name){
	FILE *fp;
	char interface_path[128], bmAttributes_file[128], buf[8], *ptr;
	DIR *interface_dir = NULL;
	struct dirent *end_name;
	int bNumEndpoints, end_count, got_Int = 0;

	if(interface_name == NULL || get_usb_port_by_string(interface_name, buf, sizeof(buf)) == NULL){
		usb_dbg("(%s): The device is not a interface.\n", interface_name);
		return 0;
	}

	snprintf(interface_path, sizeof(interface_path), "%s/%s", USB_DEVICE_PATH, interface_name);
	if((interface_dir = opendir(interface_path)) == NULL){
		usb_dbg("(%s): Fail to open dir: %s.\n", interface_name, interface_path);
		return 0;
	}

	// Get bNumEndpoints.
	bNumEndpoints = get_interface_numendpoints(interface_name);
	if(bNumEndpoints <= 0){
		usb_dbg("(%s): No endpoints: %d.\n", interface_name, bNumEndpoints);
		return 0;
	}

	end_count = 0;
	while((end_name = readdir(interface_dir)) != NULL){
		if(strncmp(end_name->d_name, "ep_", 3))
			continue;

		++end_count;

		memset(bmAttributes_file, 0, 128);
		sprintf(bmAttributes_file, "%s/%s/bmAttributes", interface_path, end_name->d_name);

		if((fp = fopen(bmAttributes_file, "r")) == NULL){
			usb_dbg("(%s): Fail to open file: %s.\n", interface_name, bmAttributes_file);
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

extern int isSerialNode(const char *device_name){
	if(strncmp(device_name, "ttyUSB", 6))
		return 0;

	return 1;
}

extern int isACMNode(const char *device_name){
	if(strncmp(device_name, "ttyACM", 6))
		return 0;

	return 1;
}

extern int isWDMNode(const char *device_name){
	if(strncmp(device_name, "cdc-wdm", 7))
		return 0;

	return 1;
}

extern int isUsbNetIf(const char *device_name) {
	if(!strncmp(device_name, "usb", 3))
		return 1;
	if(!strncmp(device_name, "wwan", 4))
		return 1;
	return 0;
}

extern int isSerialInterface(const char *interface_class){
	if(strcmp(interface_class, "ff") == 0)
		return 1;

	return 0;
}

extern int isACMInterface(const char *interface_class, const char *interface_subclass){
	if(strcmp(interface_class, "02") == 0 && strcmp(interface_subclass, "02") == 0)
		return 1;

	return 0;
}

extern int isRNDISInterface(const char *interface_class, const char *interface_subclass){
	// tethering
	if(strcmp(interface_class, "e0") == 0 && strcmp(interface_subclass, "01") == 0)
		return 1;

	// WM5
	if(strcmp(interface_class, "ef") == 0 && strcmp(interface_subclass, "01") == 0)
		return 1;

	return 0;
}

extern int isCDCEthInterface(const char *interface_class, const char *interface_subclass){
	// cdc-ether
	if(strcmp(interface_class, "02") == 0 && strcmp(interface_subclass, "06") == 0)
		return 1;

	// cdc-mdlm
	if(strcmp(interface_class, "02") == 0 && strcmp(interface_subclass, "0a") == 0)
		return 1;

	return 0;
}

extern int isCDCNCMInterface(const char *interface_class, const char *interface_subclass){
	// cdc-ncm
	if(strcmp(interface_class, "02") == 0 && strcmp(interface_subclass, "0d") == 0)
		return 1;

	return 0;
}

extern int isCDCMBIMInterface(const char *interface_class, const char *interface_subclass){
	// cdc-mbim
	if(strcmp(interface_class, "02") == 0 && strcmp(interface_subclass, "0e") == 0)
		return 1;

	return 0;
}

extern int is_usb_modem_ready(){
	if ((!strcmp(nvram_safe_get("usb_path1"), "modem") && strcmp(nvram_safe_get("usb_path1_act"), "")) ||
	    (!strcmp(nvram_safe_get("usb_path2"), "modem") && strcmp(nvram_safe_get("usb_path2_act"), "")))
		return 1;
	else
		return 0;
}

extern int get_usb_modem_state(){
	if(nvram_get_int("modem_running") == 1)
		return 1;
	else
		return 0;
}

extern int set_usb_modem_state(const int flag){
	if(flag != 1 && flag != 0)
		return 0;

	if(flag){
		nvram_set("modem_running", "1");
		return 1;
	}
	else{
		nvram_set("modem_running", "0");
		return 0;
	}
}

#ifdef RTCONFIG_USB_PRINTER
extern int hadPrinterModule(){
	char target_file[128];
	DIR *module_dir;

	sprintf(target_file, "%s/usblp", SYS_MODULE);
	if((module_dir = opendir(target_file)) != NULL){
		closedir(module_dir);
		return 1;
	}
	else
		return 0;
}

extern int hadPrinterInterface(const char *usb_port){
	char check_usb_port[8], device_name[8];
	int printer_order, got_printer = 0;

	for(printer_order = 0; printer_order < SCAN_PRINTER_NODE; ++printer_order){
		sprintf(device_name, "lp%d", printer_order);

		if(get_usb_port_by_device(device_name, check_usb_port, sizeof(check_usb_port)) == NULL)
			continue;

		if(!strcmp(usb_port, check_usb_port)){
			got_printer = 1;
			break;
		}
	}

	return got_printer;
}

extern int isPrinterInterface(const char *interface_class){
	if(strcmp(interface_class, "07") == 0)
		return 1;

	return 0;
}
#endif // RTCONFIG_USB_PRINTER

extern int isStorageInterface(const char *interface_class){
	if(strcmp(interface_class, "08") == 0)
		return 1;

	return 0;
}
