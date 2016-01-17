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
#include <dirent.h>
#include <unistd.h>
#include <sys/vfs.h>

#include <shutils.h>	// for eval()

#include "disk_io_tools.h"
#include "disk_initial.h"
#include "usb_info.h"

int main(int argc, char *argv[])
{
	disk_info_t *follow_disk, *disks_info;
	partition_info_t *follow_partition;
	usb_info_t *follow_info, *usb_info;

	disks_info = read_disk_data();
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		csprintf("              port: %u.\n", follow_disk->port_root);
		csprintf("               tag: %s.\n", follow_disk->tag);
		csprintf("            vendor: %s.\n", follow_disk->vendor);
		csprintf("             model: %s.\n", follow_disk->model);
		csprintf("            device: %s.\n", follow_disk->device);
		csprintf("             major: %u.\n", follow_disk->major);
		csprintf("             minor: %u.\n", follow_disk->minor);
		csprintf("  partition_number: %u.\n", follow_disk->partition_number);
		csprintf("    mounted_number: %u.\n", follow_disk->mounted_number);
		csprintf("     swapon_number: %u.\n", follow_disk->swapon_number);
		csprintf(" size_in_kilobytes: %llu.\n", follow_disk->size_in_kilobytes);
		
		if (follow_disk->partitions == NULL) {
			csprintf("\n");
			continue;
		}
		
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			csprintf("		           device: %s.\n", follow_partition->device);
			csprintf("		           swapon: %d.\n", follow_partition->swapon);
			csprintf("		      mount_point: %s.\n", follow_partition->mount_point);
			csprintf("		      file_system: %s.\n", follow_partition->file_system);
			csprintf("		size_in_kilobytes: %llu.\n", follow_partition->size_in_kilobytes);
			csprintf("		   used_kilobytes: %llu.\n", follow_partition->used_kilobytes);
		}
		
		csprintf("\n");
	}

	free_disk_data(disks_info);

	usb_info = get_usb_info();
	for (follow_info = usb_info; follow_info != NULL; follow_info = follow_info->next) {
		csprintf("-----------------------------------------------\n");
		csprintf("         port_root: %u\n", follow_info->port_root);
		csprintf("          dev_type: %u\n", follow_info->dev_type);
		csprintf("      manufacturer: %s\n", follow_info->manuf);
		csprintf("           product: %s\n", follow_info->product);
		csprintf("            id_dev: %u\n", follow_info->id_devnum);
		csprintf("         id_parent: %u\n", follow_info->id_parent);
		csprintf("           id_port: %u\n", follow_info->id_port);
		csprintf("             class: %02X\n", follow_info->dev_cls);
		csprintf("          subclass: %02X\n", follow_info->dev_sub);
		csprintf("          protocol: %02X\n", follow_info->dev_prt);
		csprintf("               vid: %04X\n", follow_info->dev_vid);
		csprintf("               pid: %04X\n", follow_info->dev_pid);
	}
	free_usb_info(usb_info);

	if (argc > 1 && strcmp(argv[1], "-u") == 0)
		umount_all_storage();

	return 0;
}

