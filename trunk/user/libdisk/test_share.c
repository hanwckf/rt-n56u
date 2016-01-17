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
#include "disk_io_tools.h"
#include "disk_share.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s MOUNT_PATH\n", argv[0]);
		return -1;
	}

	create_if_no_var_files(argv[1]);	// According to the old folder_list, add the new folder.
	initial_folder_list_in_mount_path(argv[1]);	// get the new folder_list.
	create_if_no_var_files(argv[1]);	// According to the new folder_list, add the new var file.

	return 0;
}
