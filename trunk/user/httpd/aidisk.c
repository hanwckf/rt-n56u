/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <disk_initial.h>
#include <disk_share.h>
#include <dev_info.h>
#include <usb_info.h>

#include "httpd.h"

#if defined (USE_USB_SUPPORT)
int
ej_get_usb_ports_info(int eid, webs_t wp, int argc, char **argv)
{
	int i, idx, first, usb_dev_type[2] = {0};
	char *usb_dev_string[2];
	usb_info_t *usb_info, *follow_usb;

	usb_info = get_usb_info();
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		idx = follow_usb->port_root;
		if (idx == 1 || idx == 2) {
			if (follow_usb->dev_type == DEVICE_TYPE_USB_HUB)
				usb_dev_type[idx-1] |= 0x01;
			else if (follow_usb->dev_type == DEVICE_TYPE_SCSI_DISK)
				usb_dev_type[idx-1] |= 0x02;
			else if (follow_usb->dev_type == DEVICE_TYPE_PRINTER)
				usb_dev_type[idx-1] |= 0x04;
			else if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY)
				usb_dev_type[idx-1] |= 0x08;
			else if (follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH)
				usb_dev_type[idx-1] |= 0x10;
		}
	}

	for (i = 0; i < 2; i++) {
		if (usb_dev_type[i] & 0x01)
			usb_dev_string[i] = "hub";
		else if (usb_dev_type[i] & 0x04)
			usb_dev_string[i] = "printer";
		else if (usb_dev_type[i] & 0x10)
			usb_dev_string[i] = "modem_eth";
		else if (usb_dev_type[i] & 0x08)
			usb_dev_string[i] = "modem_tty";
		else if (usb_dev_type[i] & 0x02)
			usb_dev_string[i] = "storage";
		else
			usb_dev_string[i] = "unknown";
	}

	/* usb ports num */
	websWrite(wp, "function get_usb_ports_num(){\n");
	websWrite(wp, "    return %u;\n", BOARD_NUM_USB_PORTS);
	websWrite(wp, "}\n\n");

	/* usb device types */
	websWrite(wp, "function get_device_type_usb(port_num){\n");
	websWrite(wp, "    if (port_num == 1)\n");
	websWrite(wp, "        return \"%s\";\n", usb_dev_string[0]);
	websWrite(wp, "    else if (port_num == 2)\n");
	websWrite(wp, "        return \"%s\";\n", usb_dev_string[1]);
	websWrite(wp, "    else\n");
	websWrite(wp, "        return \"%s\";\n", "unknown");
	websWrite(wp, "}\n\n");

	/* printers */
	websWrite(wp, "function printer_ports() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_PRINTER) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%u\"", follow_usb->port_root);
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function printer_manufacts() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_PRINTER) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->manuf) ? follow_usb->manuf : "Unknown");
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function printer_models() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_PRINTER) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->product) ? follow_usb->product : "Unknown" );
		}
	}
	websWrite(wp, "];\n}\n\n");

	/* modems */
	websWrite(wp, "function modem_ports() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%u\"", follow_usb->port_root);
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_devnum() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%u\"", follow_usb->id_devnum);
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_types() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY) ? "RAS" : "NDIS/RNDIS");
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_manufacts() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->manuf) ? follow_usb->manuf : "Unknown");
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_models() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->product) ? follow_usb->product : "Unknown" );
		}
	}
	websWrite(wp, "];\n}\n\n");

	free_usb_info(usb_info);

	return 0;
}
#endif

int
ej_disk_pool_mapping_info(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	int first;
	char *Ptr;

	disks_info = read_disk_data();
	if (!disks_info) {
		websWrite(wp, "%s", initial_disk_pool_mapping_info());
		return -1;
	}

	websWrite(wp, "function pool_names() {\n");
	websWrite(wp, "    return [");

	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			
			if (follow_partition->mount_point == NULL) {
				websWrite(wp, "\"unknown\"");
				continue;
			}
			
			Ptr = rindex(follow_partition->mount_point, '/');
			if (Ptr == NULL) {
				websWrite(wp, "\"unknown\"");
				continue;
			}
			
			if (strncmp(follow_partition->mount_point, "/media/", 7) != 0) {
				websWrite(wp, "\"unknown\"");
				continue;
			}
			++Ptr;
			websWrite(wp, "\"%s\"", Ptr);
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function pool_types() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			if (!follow_partition->file_system) {
				websWrite(wp, "\"unknown\"");
				continue;
			}

			websWrite(wp, "\"%s\"", follow_partition->file_system);
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function pool_status() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			if (!follow_partition->mount_point) {
				websWrite(wp, "\"unmounted\"");
				continue;
			}

			if (follow_partition->read_only)
				websWrite(wp, "\"ro\"");
			else
				websWrite(wp, "\"rw\"");
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function pool_kilobytes_in_use() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "%llu", follow_partition->used_kilobytes);
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	disk_info_t *follow_disk2;
	u32 disk_num, pool_num;
	websWrite(wp, "function per_pane_pool_usage_kilobytes(pool_num, disk_num) {\n");
	for (follow_disk = disks_info, pool_num = 0; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++pool_num) {
			websWrite(wp, "    if (pool_num == %d) {\n", pool_num);
			if (follow_partition->mount_point != NULL)
				for (follow_disk2 = disks_info, disk_num = 0; follow_disk2 != NULL; follow_disk2 = follow_disk2->next, ++disk_num) {
					websWrite(wp, "	if (disk_num == %d) {\n", disk_num);
					if (follow_disk2->major == follow_disk->major && follow_disk2->minor == follow_disk->minor)
						websWrite(wp, "	    return [%llu];\n", follow_partition->size_in_kilobytes);
					else
						websWrite(wp, "	    return [0];\n");
					websWrite(wp, "	}\n");
				}
			else
				websWrite(wp, "	return [0];\n");
			websWrite(wp, "    }\n");
		}
	}
	websWrite(wp, "}\n\n");
	free_disk_data(disks_info);

	return 0;
}

int
ej_available_disk_names_and_sizes(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	int first;

	websWrite(wp, "%s", initial_blank_disk_names_and_sizes());

	disks_info = read_disk_data();
	if (!disks_info) {
		websWrite(wp, "%s", initial_available_disk_names_and_sizes());
		return -1;
	}

	/* show name of the foreign disks */
	websWrite(wp, "function foreign_disks() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%s\"", follow_disk->tag);
	}
	websWrite(wp, "];\n}\n\n");

	/* show interface of the foreign disks */
	websWrite(wp, "function foreign_disk_interface_names() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%u\"", follow_disk->port_root);
	}
	websWrite(wp, "];\n}\n\n");

	/* show device name of the foreign disks */
	websWrite(wp, "function foreign_disk_device_names() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%s\"", follow_disk->device);
	}
	websWrite(wp, "];\n}\n\n");

	/* show model info of the foreign disks */
	websWrite(wp, "function foreign_disk_model_info() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"");
		if (follow_disk->vendor && strlen(follow_disk->vendor) > 0)
			websWrite(wp, "%s", follow_disk->vendor);
		if (follow_disk->model) {
			if (follow_disk->vendor && strlen(follow_disk->vendor) > 0)
				websWrite(wp, " ");
			websWrite(wp, "%s", follow_disk->model);
		}
		websWrite(wp, "\"");
	}
	websWrite(wp, "];\n}\n\n");

	/* show total_size of the foreign disks */
	websWrite(wp, "function foreign_disk_total_size() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%llu\"", follow_disk->size_in_kilobytes);
	}
	websWrite(wp, "];\n}\n\n");

	/* show total number of the mounted partition in this foreign disk */
	websWrite(wp, "function foreign_disk_total_mounted_number() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%u\"", (follow_disk->mounted_number + follow_disk->swapon_number));
	}
	websWrite(wp, "];\n}\n\n");

	free_disk_data(disks_info);

	return 0;
}

int
ej_get_storage_share_list(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	int first_pool, ret;

	disks_info = read_disk_data();
	if (!disks_info)
		return 0;

	ret = 0;
	first_pool = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 1) {
				if (first_pool == 1)
					first_pool = 0;
				else
					ret += websWrite(wp, ", ");
				
				ret += websWrite(wp, "[\"%s\",\"%s\",\"%s\"]", 
					follow_partition->device, follow_partition->mount_point, follow_partition->file_system);
			}
		}
	}

	free_disk_data(disks_info);

	return ret;
}

int
ej_get_AiDisk_status(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	char *follow_info;
	int sh_num;
	char **folder_list = NULL;
	int first_pool, first_folder, result, i;

	websWrite(wp, "function get_cifs_status() {\n");
	websWrite(wp, "    return %d;\n", nvram_get_int("enable_samba"));
	websWrite(wp, "}\n\n");

	websWrite(wp, "function get_ftp_status() {\n");
	websWrite(wp, "    return %d;\n", nvram_get_int("enable_ftp"));
	websWrite(wp, "}\n\n");

	websWrite(wp, "function get_share_management_status(protocol) {\n");
	websWrite(wp, "    if (protocol == \"cifs\")\n");
	websWrite(wp, "	return %d;\n", nvram_get_int("st_samba_mode"));
	websWrite(wp, "    else if (protocol == \"ftp\")\n");
	websWrite(wp, "	return %d;\n", nvram_get_int("st_ftp_mode"));
	websWrite(wp, "    else\n");
	websWrite(wp, "	return -1;\n");
	websWrite(wp, "}\n\n");

	disks_info = read_disk_data();
	if (!disks_info) {
		websWrite(wp, "function get_sharedfolder_in_pool(poolName) {}\n");
		return -1;
	}

	first_pool = 1;
	websWrite(wp, "function get_sharedfolder_in_pool(poolName) {\n");
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				websWrite(wp, "    ");

				if (first_pool == 1)
					first_pool = 0;
				else
					websWrite(wp, "else ");

				follow_info = rindex(follow_partition->mount_point, '/');
				websWrite(wp, "if (poolName == \"%s\") {\n", follow_info+1);
				websWrite(wp, "	return [");

				result = get_all_folder_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list);
				if (result < 0) {
					websWrite(wp, "];\n");
					websWrite(wp, "    }\n");
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}

				first_folder = 1;
				for (i = 0; i < sh_num; ++i) {
					if (first_folder == 1)
						first_folder = 0;
					else
						websWrite(wp, ", ");
					websWrite(wp, "\"%s\"", folder_list[i]);
				}

				websWrite(wp, "];\n");
				websWrite(wp, "    }\n");
			}
		}

	websWrite(wp, "}\n\n");

	if (disks_info != NULL) {
		free_2_dimension_list(&sh_num, &folder_list);
		free_disk_data(disks_info);
	}

	return 0;
}

int
ej_set_AiDisk_status(int eid, webs_t wp, int argc, char **argv)
{
	char *protocol = websGetVar(wp, "protocol", "");
	char *flag = websGetVar(wp, "flag", "");
	int result = 0;

	if (strlen(protocol) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", get_alert_msg_from_dict("Input1"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (strlen(flag) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", get_alert_msg_from_dict("Input18"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (!strcmp(protocol, "cifs")) {
		if (!strcmp(flag, "on")) {
			nvram_set_int("enable_samba", 1);
			nvram_commit_safe();
			result = system("/sbin/run_samba");
		}
		else if (!strcmp(flag, "off")) {
			nvram_set_int("enable_samba", 0);
			nvram_commit_safe();
			if (!pids("smbd"))
				goto SET_AIDISK_STATUS_SUCCESS;

			result = system("/sbin/stop_samba");
		}
		else {
			websWrite(wp, "<script>\n");
			websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", get_alert_msg_from_dict("Input19"));
			websWrite(wp, "</script>\n");
			return -1;
		}
	}
	else if (!strcmp(protocol, "ftp")) {
		if (!strcmp(flag, "on")) {
			nvram_set_int("enable_ftp", 1);
			nvram_commit_safe();
			result = system("run_ftp");
		}
		else if (!strcmp(flag, "off")) {
			nvram_set_int("enable_ftp", 0);
			nvram_commit_safe();
			if (!pids("vsftpd"))
				goto SET_AIDISK_STATUS_SUCCESS;
			result = system("/sbin/stop_ftp");
		}
		else {
			websWrite(wp, "<script>\n");
			websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", get_alert_msg_from_dict("Input19"));
			websWrite(wp, "</script>\n");
			return -1;
		}
	}
	else {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", get_alert_msg_from_dict("Input2"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (result != 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", get_alert_msg_from_dict("Action8"));
		websWrite(wp, "</script>\n");
		return -1;
	}

SET_AIDISK_STATUS_SUCCESS:
	websWrite(wp, "<script>\n");
	websWrite(wp, "parent.resultOfSwitchAppStatus();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_get_all_accounts(int eid, webs_t wp, int argc, char **argv)
{
	int acc_num = 0;
	char **account_list = NULL;
	char *acc_mode = "ftp";
	int i, first;

	ejArgs(argc, argv, "%s", &acc_mode);

	first = 1;
	if (strcmp(acc_mode, "ftp") == 0) {
		int st_ftp_mode = nvram_get_int("st_ftp_mode");
		if (st_ftp_mode == 3 || st_ftp_mode == 4) {
			first = 0;
			websWrite(wp, "\"%s\"", FTP_ANONYMOUS_USER);
		}
	}

	get_account_list(&acc_num, &account_list);
	for (i = 0; i < acc_num; ++i) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%s\"", account_list[i]);
	}
	free_2_dimension_list(&acc_num, &account_list);

	return 0;
}

int
ej_safely_remove_disk(int eid, webs_t wp, int argc, char **argv)
{
	int result, port_num;
	char *disk_port = websGetVar(wp, "port", "");
	char *disk_devn = websGetVar(wp, "devn", "");

	port_num = atoi(disk_port);
	if (port_num < 0)
		port_num = 0;

#if defined (USE_ATA_SUPPORT)
	if (port_num == ATA_VIRT_PORT_ID)
		result = doSystem("/sbin/ejata %s", disk_devn);
	else
#endif
#if defined (USE_MMC_SUPPORT)
	if (port_num == MMC_VIRT_PORT_ID)
		result = system("/sbin/ejmmc");
	else
#endif
		result = doSystem("/sbin/ejusb %d %s", port_num, disk_devn);

	if (result != 0) {
		websWrite(wp, "<script>safely_remove_disk_error(\'%s\');</script>\n", get_alert_msg_from_dict("Action9"));
	} else {
		websWrite(wp, "<script>safely_remove_disk_success();</script>\n");
	}

	return 0;
}

int
ej_get_permissions_of_account(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num = 0;
	char **account_list = NULL;
	char *acc_mode = "ftp";
	int samba_right, ftp_right;
	int result, anonym, i, j;
	int first_pool, first_account, first_folder;

	ejArgs(argc, argv, "%s", &acc_mode);

	disks_info = read_disk_data();
	if (!disks_info) {
		websWrite(wp, "function get_account_permissions_in_pool(account, pool) {return [];}\n");
		return -1;
	}

	get_account_list(&acc_num, &account_list);

	websWrite(wp, "function get_account_permissions_in_pool(account, pool) {\n");

	anonym = 0;
	if (strcmp(acc_mode, "ftp") == 0) {
		int st_ftp_mode = nvram_get_int("st_ftp_mode");
		if (st_ftp_mode == 3 || st_ftp_mode == 4)
			anonym = 1;
	}

	if ((acc_num + anonym) <= 0)
		websWrite(wp, "    return [];\n");

	first_account = 1;
	for (i = 0; i < (acc_num + anonym); ++i) {
		char *acc_value;
		websWrite(wp, "    ");
		if (first_account == 1)
			first_account = 0;
		else
			websWrite(wp, "else ");
		
		if (i < acc_num)
			acc_value = account_list[i];
		else
			acc_value = FTP_ANONYMOUS_USER;

		websWrite(wp, "if (account == \"%s\") {\n", acc_value);

		first_pool = 1;
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
					int sh_num = 0;
					char **folder_list = NULL;
					websWrite(wp, "	");
					if (first_pool == 1)
						first_pool = 0;
					else
						websWrite(wp, "else ");

					websWrite(wp, "if (pool == \"%s\") {\n", rindex(follow_partition->mount_point, '/')+1);

					websWrite(wp, "	    return [");

					result = get_all_folder_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list);
					if (result != 0) {
						websWrite(wp, "];\n");
						websWrite(wp, "	}\n");
						free_2_dimension_list(&sh_num, &folder_list);
						continue;
					}
					first_folder = 1;
					for (j = 0; j < sh_num; ++j) {
						samba_right = get_permission(acc_value,
												 follow_partition->mount_point,
												 folder_list[j],
												 "cifs");
						ftp_right = get_permission(acc_value,
												 follow_partition->mount_point,
												 folder_list[j],
												 "ftp");
						if (samba_right < 0 || samba_right > 3) {
//							samba_right = DEFAULT_SAMBA_RIGHT;	// J++
							samba_right = 0;	
						}

						if (ftp_right < 0 || ftp_right > 3) {
//							ftp_right = DEFAULT_FTP_RIGHT;		// J++
							ftp_right = 0;
						}

						if (first_folder == 1) {
							first_folder = 0;
							websWrite(wp, "[\"%s\", %d, %d]", folder_list[j], samba_right, ftp_right);
						}
						else
							websWrite(wp, "		    [\"%s\", %d, %d]", folder_list[j], samba_right, ftp_right);

						if (j != sh_num-1)
							websWrite(wp, ",\n");
					}
					websWrite(wp, "];\n");
					websWrite(wp, "	}\n");
					
					free_2_dimension_list(&sh_num, &folder_list);
				}
			}
		}

		websWrite(wp, "    }\n");
	}

	websWrite(wp, "}\n\n");

	free_2_dimension_list(&acc_num, &account_list);

	if (disks_info != NULL)
		free_disk_data(disks_info);

	return 0;
}

int
ej_get_folder_tree(int eid, webs_t wp, int argc, char **argv)
{
	char *layer_order = websGetVar(wp, "layer_order", ""), folder_code[1024];
	char *follow_info, *follow_info_end, backup;
	int layer = 0, first;
	int disk_count, partition_count, folder_count1, folder_count2;
	int disk_order = -1, partition_order = -1, folder_order = -1;
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	char *pool_mount_dir;
	DIR *dir1, *dir2;
	struct dirent *dp1, *dp2;
	char dir1_Path[4096], dir2_Path[4096];

	if (strlen(layer_order) <= 0)
		return -1;

	follow_info = index(layer_order, '_');
	while (follow_info != NULL && *follow_info != 0) {
		++layer;

		++follow_info;
		if (*follow_info == 0)
			break;
		follow_info_end = follow_info;
		while (*follow_info_end != 0 && isdigit(*follow_info_end))
			++follow_info_end;
		backup = *follow_info_end;
		*follow_info_end = 0;

		if (layer == 1)
			disk_order = atoi(follow_info);
		else if (layer == 2)
			partition_order = atoi(follow_info);
		else if (layer == 3)
			folder_order = atoi(follow_info);
		*follow_info_end = backup;

		if (layer == 3) {
			memset(folder_code, 0, 1024);
			strcpy(folder_code, follow_info);
		}

		follow_info = follow_info_end;
	}
	follow_info = folder_code;

	disks_info = read_disk_data();
	if (disks_info == NULL)
		return -1;

	first = 1;
	disk_count = 0;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next, ++disk_count) {
		if (layer == 0) { // get disks.
			partition_count = 0;
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++partition_count)
				;

			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "\"%s#%u#%u\"", follow_disk->tag, disk_count, partition_count);

			continue;
		}
		if (disk_count != disk_order)
			continue;

		partition_count = 0;
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++partition_count) {
			if (follow_partition->mount_point == NULL || strlen(follow_partition->mount_point) <= 0)
				continue;

			pool_mount_dir = rindex(follow_partition->mount_point, '/')+1;

			if (layer == 1) { // get pools.
				dir2 = opendir(follow_partition->mount_point);
				/* pool_mount_dir isn't file. */
				if (dir2 == NULL)
					continue;

				folder_count2 = 0;
				while ((dp2 = readdir(dir2)) != NULL) {
					if (dp2->d_name[0] == '.')
						continue;

					++folder_count2;
				}
				closedir(dir2);

				if (first == 1)
					first = 0;
				else
					websWrite(wp, ", ");

				websWrite(wp, "\"%s#%u#%u\"", pool_mount_dir, partition_count, folder_count2);

				continue;
			}
			if (partition_count != partition_order)
				continue;

			snprintf(dir1_Path, sizeof(dir1_Path), "%s", follow_partition->mount_point);
			dir1 = opendir(dir1_Path);
			if (dir1 == NULL) {
				free_disk_data(disks_info);
				return -1;
			}

			folder_count1 = -1;
			while ((dp1 = readdir(dir1)) != NULL) {
				if (dp1->d_name[0] == '.')
					continue;

				++folder_count1;

				if (layer == 2) { // get L1's folders.
					snprintf(dir2_Path, sizeof(dir2_Path), "%s/%s", dir1_Path, dp1->d_name);
					dir2 = opendir(dir2_Path);

					folder_count2 = 0;
					if (dir2 != NULL) {
						while ((dp2 = readdir(dir2)) != NULL) {
							if (dp2->d_name[0] == '.')
								continue;

							++folder_count2;
						}
						closedir(dir2);
					}
					if (first == 1)
						first = 0;
					else
						websWrite(wp, ", ");

					websWrite(wp, "\"%s#%u#%u\"", dp1->d_name, folder_count1, folder_count2);

					continue;
				}

				if (folder_count1 == folder_order)
					snprintf(dir1_Path, sizeof(dir1_Path), "%s/%s", dir1_Path, dp1->d_name);
			}
			closedir(dir1);
		}
	}
	free_disk_data(disks_info);
	layer -= 3;

	while (layer >= 0) {      // get Ln's folders.
		/* get the current folder_code and folder_order. */
		follow_info_end = index(follow_info, '_');
		if (follow_info_end != NULL)
			follow_info = follow_info_end+1;
		else
			backup = -1;
		folder_order = atoi(follow_info);

		dir1 = opendir(dir1_Path);
		if (dir1 == NULL)
			return -1;
		
		folder_count1 = -1;
		while ((dp1 = readdir(dir1)) != NULL) {
			if (dp1->d_name[0] == '.')
				continue;

			++folder_count1;

			if (layer == 0) {
				snprintf(dir2_Path, sizeof(dir2_Path), "%s/%s", dir1_Path, dp1->d_name);
				dir2 = opendir(dir2_Path);
				folder_count2 = 0;
				if (dir2 != NULL) {
					while ((dp2 = readdir(dir2)) != NULL) {
						if (dp2->d_name[0] == '.')
							continue;

						++folder_count2;
					}
					closedir(dir2);
				}

				if (first == 1)
					first = 0;
				else
					websWrite(wp, ", ");

				websWrite(wp, "\"%s#%u#%u\"", dp1->d_name, folder_count1, folder_count2);

				continue;
			}

			if (folder_count1 == folder_order)
				snprintf(dir1_Path, sizeof(dir1_Path), "%s/%s", dir1_Path, dp1->d_name);
		}
		closedir(dir1);
		--layer;
	}

	return 0;
}

int
ej_get_share_tree(int eid, webs_t wp, int argc, char **argv)
{
	char *layer_order = websGetVar(wp, "layer_order", "");
	char *follow_info, *follow_info_end, backup;
	int layer = 0, first;
	int disk_count, partition_count, share_count;
	int disk_order = -1, partition_order = -1;
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;

	if (strlen(layer_order) <= 0)
		return -1;

	follow_info = index(layer_order, '_');
	while (follow_info != NULL && *follow_info != 0) {
		++layer;
		++follow_info;
		if (*follow_info == 0)
			break;
		follow_info_end = follow_info;
		while (*follow_info_end != 0 && isdigit(*follow_info_end))
			++follow_info_end;
		backup = *follow_info_end;
		*follow_info_end = 0;

		if (layer == 1)
			disk_order = atoi(follow_info);
		else if (layer == 2)
			partition_order = atoi(follow_info);
		else {
			*follow_info_end = backup;
			printf("Input \"%s\" is incorrect!\n", layer_order);
			return -1;
		}

		*follow_info_end = backup;
		follow_info = follow_info_end;
	}

	disks_info = read_disk_data();
	if (!disks_info)
		return -1;

	first = 1;
	disk_count = 0;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next, ++disk_count) {
		partition_count = 0;
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++partition_count) {
			if (layer != 0 && follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				int i;
				char **folder_list;
				int result;
				result = get_all_folder_in_mount_path(follow_partition->mount_point, &share_count, &folder_list);
				if (result < 0)
					share_count = 0;

				if (layer == 2 && partition_count == partition_order && disk_count == disk_order) {
					for (i = 0; i < share_count; ++i) {
						if (first == 1)
							first = 0;
						else
							websWrite(wp, ", ");

						websWrite(wp, "\"%s#%u#0\"", folder_list[i], i);
					}
				}
				else if (layer == 1 && disk_count == disk_order) {
					if (first == 1)
						first = 0;
					else
						websWrite(wp, ", ");

					follow_info = rindex(follow_partition->mount_point, '/');
					websWrite(wp, "\"%s#%u#%u\"", follow_info+1, partition_count, share_count);
				}

				free_2_dimension_list(&share_count, &folder_list);
			}
		}
		if (layer == 0) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "\"%s#%u#%u\"", follow_disk->tag, disk_count, partition_count);
		}

		if (layer > 0 && disk_count == disk_order)
			break;
	}

	free_disk_data(disks_info);

	return 0;
}

static void
not_ej_initial_folder_var_file(void)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;

	disks_info = read_disk_data();
	if (!disks_info)
		return;

	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				initial_folder_list_in_mount_path(follow_partition->mount_point);
//				initial_all_var_file_in_mount_path(follow_partition->mount_point);
			}
		}
	}

	free_disk_data(disks_info);
}

int
ej_set_share_mode(int eid, webs_t wp, int argc, char **argv)
{
	int samba_mode = nvram_get_int("st_samba_mode");
	int ftp_mode = nvram_get_int("st_ftp_mode");
	char *dummyShareway = websGetVar(wp, "dummyShareway", "");
	char *protocol = websGetVar(wp, "protocol", "");
	char *mode = websGetVar(wp, "mode", "");
	int result;

	if (strlen(dummyShareway) > 0)
		nvram_set_temp("dummyShareway", dummyShareway);
	else
		nvram_set_int_temp("dummyShareway", 0);

	if (strlen(protocol) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", get_alert_msg_from_dict("Input1"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (strlen(mode) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", get_alert_msg_from_dict("Input3"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (!strcmp(mode, "share")) {
		if (!strcmp(protocol, "cifs")) {
			if (samba_mode == 1 || samba_mode == 3)
				goto SET_SHARE_MODE_SUCCESS;
			nvram_set_int("st_samba_mode", 1);
		}
		else if (!strcmp(protocol, "ftp")) {
			if (ftp_mode == 1)
				goto SET_SHARE_MODE_SUCCESS;
			nvram_set_int("st_ftp_mode", 1);
		}
		else {
			websWrite(wp, "<script>\n");
			websWrite(wp, "set_share_mode_error(\'%s\');\n", get_alert_msg_from_dict("Input2"));
			websWrite(wp, "</script>\n");
			return -1;
		}
	}
	else if (!strcmp(mode, "account")) {
		if (!strcmp(protocol, "cifs")) {
			if (samba_mode == 2 || samba_mode == 4)
				goto SET_SHARE_MODE_SUCCESS;
			nvram_set_int("st_samba_mode", 4);
		}
		else if (!strcmp(protocol, "ftp")) {
			if (ftp_mode == 2)
				goto SET_SHARE_MODE_SUCCESS;
			nvram_set_int("st_ftp_mode", 2);
		}
		else {
			websWrite(wp, "<script>\n");
			websWrite(wp, "set_share_mode_error(\'%s\');\n", get_alert_msg_from_dict("Input2"));
			websWrite(wp, "</script>\n");
			return -1;
		}
	}
	else if (!strcmp(mode, "anonym") && !strcmp(protocol, "ftp")) {
		if (ftp_mode == 3)
			goto SET_SHARE_MODE_SUCCESS;
		nvram_set_int("st_ftp_mode", 3);
	}
	else if (!strcmp(mode, "account_anonym") && !strcmp(protocol, "ftp")) {
		if (ftp_mode == 4)
			goto SET_SHARE_MODE_SUCCESS;
		nvram_set_int("st_ftp_mode", 4);
	}
	else {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", get_alert_msg_from_dict("Input4"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	nvram_commit_safe();

	not_ej_initial_folder_var_file();	// J++

	if (!strcmp(protocol, "cifs"))
		result = eval("/sbin/run_samba");
	else if (!strcmp(protocol, "ftp"))
		result = eval("/sbin/run_ftp");
	else {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", get_alert_msg_from_dict("Input2"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (result != 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", get_alert_msg_from_dict("Action8"));
		websWrite(wp, "</script>\n");
		return -1;
	}

SET_SHARE_MODE_SUCCESS:
	websWrite(wp, "<script>\n");
	websWrite(wp, "set_share_mode_success();\n");
	websWrite(wp, "</script>\n");
	return 0;
}

static void
restart_ftp_samba_on_change_shared_folder(void)
{
#if defined(APP_SMBD) || defined(APP_FTPD)
	int st_share_mode;
#endif
#if defined(APP_SMBD)
	st_share_mode = nvram_get_int("st_samba_mode");
	if (st_share_mode == 4)
		eval("/sbin/run_samba");
#endif
#if defined(APP_FTPD)
	st_share_mode = nvram_get_int("st_ftp_mode");
	if (st_share_mode == 2 || st_share_mode == 4)
		eval("/sbin/run_ftp");
#endif
}

int
ej_modify_sharedfolder(int eid, webs_t wp, int argc, char **argv)
{
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *new_folder = websGetVar(wp, "new_folder", "");
	char *mount_path;

	if (strlen(pool) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Input7"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (strlen(folder) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Input9"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (strlen(new_folder) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Input17"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (get_mount_path(pool, &mount_path) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("System1"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (mod_folder(mount_path, folder, new_folder) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Action7"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}
	free(mount_path);

	restart_ftp_samba_on_change_shared_folder();

	websWrite(wp, "<script>\n");
	websWrite(wp, "modify_sharedfolder_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_delete_sharedfolder(int eid, webs_t wp, int argc, char **argv)
{
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *mount_path;

	if (strlen(pool) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Input7"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (strlen(folder) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Input9"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (get_mount_path(pool, &mount_path) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("System1"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (del_folder(mount_path, folder) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Action6"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}
	free(mount_path);

	restart_ftp_samba_on_change_shared_folder();

	websWrite(wp, "<script>\n");
	websWrite(wp, "delete_sharedfolder_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_create_sharedfolder(int eid, webs_t wp, int argc, char **argv)
{
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *mount_path;

	if (strlen(pool) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Input7"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (strlen(folder) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Input9"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (get_mount_path(pool, &mount_path) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("System1"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (add_folder(mount_path, folder) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", get_alert_msg_from_dict("Action5"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}
	free(mount_path);

	restart_ftp_samba_on_change_shared_folder();

	websWrite(wp, "<script>\n");
	websWrite(wp, "create_sharedfolder_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_modify_account(int eid, webs_t wp, int argc, char **argv)
{
	char *account = websGetVar(wp, "account", "");
	char *new_account = websGetVar(wp, "new_account", "");
	char *new_password = websGetVar(wp, "new_password", "");

	if (strlen(account) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_account_error(\'%s\');\n", get_alert_msg_from_dict("Input5"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (strlen(new_account) <= 0 && strlen(new_password) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_account_error(\'%s\');\n", get_alert_msg_from_dict("Input16"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (mod_account(account, new_account, new_password) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_account_error(\'%s\');\n", get_alert_msg_from_dict("Action4"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	websWrite(wp, "<script>\n");
	websWrite(wp, "modify_account_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_delete_account(int eid, webs_t wp, int argc, char **argv)
{
	char *account = websGetVar(wp, "account", "");

	if (strlen(account) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_account_error(\'%s\');\n", get_alert_msg_from_dict("Input5"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (del_account(account) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_account_error(\'%s\');\n", get_alert_msg_from_dict("Action3"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	websWrite(wp, "<script>\n");
	websWrite(wp, "delete_account_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_initial_account(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	char *command;
	int len, result;

	nvram_set_int("acc_num", 0);
	nvram_commit_safe();

	disks_info = read_disk_data();
	if (!disks_info) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "initial_account_error(\'%s\');\n", get_alert_msg_from_dict("System2"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next)
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				len = strlen("rm -f ")+strlen(follow_partition->mount_point)+strlen("/.__*");
				command = (char *)malloc(sizeof(char)*(len+1));
				if (command == NULL) {
					websWrite(wp, "<script>\n");
					websWrite(wp, "initial_account_error(\'%s\');\n", get_alert_msg_from_dict("System1"));
					websWrite(wp, "</script>\n");
					return -1;
				}
				sprintf(command, "rm -f %s/.__*", follow_partition->mount_point);
				command[len] = 0;

				result = system(command);
				free(command);

				initial_folder_list_in_mount_path(follow_partition->mount_point);
				initial_all_var_file_in_mount_path(follow_partition->mount_point);
			}

	free_disk_data(disks_info);

	eval("/sbin/run_ftpsamba");

	websWrite(wp, "<script>\n");
	websWrite(wp, "initial_account_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_create_account(int eid, webs_t wp, int argc, char **argv)
{
	char *account = websGetVar(wp, "account", "");
	char *password = websGetVar(wp, "password", "");

	if (strlen(account) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "create_account_error(\'%s\');\n", get_alert_msg_from_dict("Input5"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (strlen(password) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "create_account_error(\'%s\');\n", get_alert_msg_from_dict("Input14"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (add_account(account, password) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "create_account_error(\'%s\');\n", get_alert_msg_from_dict("Action2"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	websWrite(wp, "<script>\n");
	websWrite(wp, "create_account_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int
ej_set_account_permission(int eid, webs_t wp, int argc, char **argv)
{
	char *mount_path;
	char *account = websGetVar(wp, "account", "");
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *protocol = websGetVar(wp, "protocol", "");
	char *permission = websGetVar(wp, "permission", "");
	int right;

	if (strlen(account) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input5"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (test_if_exist_account(account) != 1) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input6"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (strlen(pool) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input7"));
		websWrite(wp, "</script>\n");
		return -1;
	}
	if (get_mount_path(pool, &mount_path) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("System1"));
		websWrite(wp, "</script>\n");
		return -1;
	}

	if (strlen(folder) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input9"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}
	if (strlen(protocol) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input1"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}
	if (strcmp(protocol, "cifs") && strcmp(protocol, "ftp") && strcmp(protocol, "dms")) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input2"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}

	if (strlen(permission) <= 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input12"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}
	right = atoi(permission);
	if (right < 0 || right > 3) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Input13"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}

	if (set_permission(account, mount_path, folder, protocol, right) < 0) {
		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", get_alert_msg_from_dict("Action1"));
		websWrite(wp, "</script>\n");
		free(mount_path);
		return -1;
	}

	free(mount_path);

	websWrite(wp, "<script>\n");
	websWrite(wp, "set_account_permission_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

