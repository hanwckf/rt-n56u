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
#include <nvram/bcmnvram.h>

#include "disk_io_tools.h"
#include "disk_initial.h"

int main(int argc, char *argv[]) {
	disk_info_t *follow_disk, *disks_info = read_disk_data();
	partition_info_t *follow_partition;
	
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		csprintf("	      tag: %s.\n", follow_disk->tag);
		csprintf("	   vendor: %s.\n", follow_disk->vendor);
		csprintf("	    model: %s.\n", follow_disk->model);
		csprintf("	   device: %s.\n", follow_disk->device);
		csprintf("	    major: %u.\n", follow_disk->major);
		csprintf("	    minor: %u.\n", follow_disk->minor);
		csprintf("   mounted_number: %u.\n", follow_disk->mounted_number);
		csprintf("     device_order: %u.\n", follow_disk->device_order);
		csprintf("size_in_kilobytes: %llu.\n", follow_disk->size_in_kilobytes);
		if (follow_disk->partitions == NULL) {
			csprintf("\n");
			continue;
		}
		
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			csprintf("     Partition %u:\n", follow_partition->partition_number);
			csprintf("parent disk's tag: %s.\n", follow_partition->disk->tag);
			csprintf("	   device: %s.\n", follow_partition->device);
			csprintf("      mount_point: %s.\n", follow_partition->mount_point);
			csprintf("      file_system: %s.\n", follow_partition->file_system);
			csprintf("size_in_kilobytes: %llu.\n", follow_partition->size_in_kilobytes);
			csprintf("   used_kilobytes: %llu.\n", follow_partition->used_kilobytes);
		}
		
		csprintf("\n");
	}//*/
	
	free_disk_data(&disks_info);
	
	return 0;
}
