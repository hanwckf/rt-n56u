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
#include "httpd.h"

char *
initial_disk_pool_mapping_info(void)
{
	char *desc = "function pool_names() { return [];}\n\
function pool_types() { return [];}\n\
function pool_status() { return [];}\n\
function pool_kilobytes_in_use() { return [];}\n\
function pool_usage_kilobytes() { return [];}\n\
function per_pane_pool_usage_kilobytes(pool_num, disk_num) { return [];}\n";

	return desc;
}

char *
initial_blank_disk_names_and_sizes(void)
{
	char *desc = "function blank_disks() { return [];}\n\
function blank_disk_interface_names() { return [];}\n\
function blank_disk_device_names() { return [];}\n\
function blank_disk_model_info() { return [];}\n\
function blank_disk_total_size() { return [];}\n\
function blank_disk_total_mounted_number() { return [];}\n";

	return desc;
}

char *
initial_available_disk_names_and_sizes(void)
{
	char *desc = "function foreign_disks() { return [];}\n\
function foreign_disk_interface_names() { return [];}\n\
function foreign_disk_model_info() { return [];}\n\
function foreign_disk_total_size() { return [];}\n\
function foreign_disk_total_mounted_number() { return [];}\n";

	return desc;
}

