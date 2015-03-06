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
#include <unistd.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <sys/swap.h>

#include <shutils.h>	// for eval()
#include <nvram_linux.h>

#include "disk_io_tools.h"
#include "disk_swap.h"

int start_swap(const char *disk_port, const int swap_mega_size) {
	int disk_index = -1, pool_number = 0;
	int result;
	char nvram_path[16], swap_path[32], size_str[32];
	//char *command;
	char *command[6];
	
	if (nvram_match("swap_on", "1")) {
		csprintf("Already swap on.\n");
		return 1;
	}
	
	if (nvram_match("usb_disc0_port", (char *)disk_port)) {
		disk_index = 0;
		pool_number = atoi(nvram_safe_get("usb_disc0_index"));
		result = test_mounted_disk_size_status(nvram_safe_get("usb_disc0_path0"));
	}
	else if (nvram_match("usb_disc1_port", (char *)disk_port)) {
		disk_index = 1;
		pool_number = atoi(nvram_safe_get("usb_disc1_index"));
		result = test_mounted_disk_size_status(nvram_safe_get("usb_disc1_path0"));
	}
	else{
		csprintf("No disk in USB port '%s'.\n", disk_port);
		return 0;
	}
	
	if (pool_number == 0) {
		csprintf("No mounted partition in USB port '%s'.\n", disk_port);
		csprintf("So can't swap on.\n");
		return 0;
	}
	
	if (result == 1) {
		csprintf("The swap file is not added for free space is less than 33MB.\n");
		return 0;
	}
	else if (result == 3) {
		csprintf("The swap file is not added for partition size is less than 1024MB.\n");
		return 0;
	}
	else if (result != 2) {
		csprintf("The swap file is not added for unknown reasons.\n");
		return 0;
	}
	
	if (swap_mega_size < SWAP_MIN || swap_mega_size > SWAP_MAX) {
		csprintf("Swap size is over the range.\n");
		csprintf("The range is between %d ~ %d.\n", SWAP_MIN, SWAP_MAX);
		csprintf("1 is set to 1Kb.\n");
		return 0;
	}
	else{
		memset(size_str, 0, 32);
		sprintf(size_str, "count=%d", swap_mega_size);
	}
	
	memset(nvram_path, 0, 16);
	sprintf(nvram_path, "usb_disc%d_path0", disk_index);
	
	memset(swap_path, 0, 32);
	sprintf(swap_path, "of=%s/.swap", nvram_safe_get(nvram_path));
	
	/*len = strlen("dd if=/dev/zero of=")+strlen(swap_path)+strlen(" bs=1048576 count=")+strlen(size_str);
	command = (char *)malloc(sizeof(char)*(len+1));
	if (command == NULL) {
		csprintf("start_swap:\n");
		csprintf("\tNo memory!!(command for dd)\n");
		return 0;
	}
	sprintf(command, "dd if=/dev/zero of=%s bs=1048576 count=%s", swap_path, size_str);
	command[len] = 0;
	result = system(command);
	free(command);//*/
	command[0] = "/bin/dd";
	command[1] = "if=/dev/zero";
	command[2] = swap_path;
	command[3] = "bs=1048576";
	command[4] = size_str;
	command[5] = NULL;
	result = _eval(command, "/dev/console", 0, NULL);
	sleep(1);
	
	if (result < 0) {
		csprintf("\tFailed to dd swap file!\n");
		return 0;
	}
	
	/*len = strlen("mkswap ")+strlen(swap_path);
	command = (char *)malloc(sizeof(char)*(len+1));
	if (command == NULL) {
		csprintf("start_swap:\n");
		csprintf("\tNo memory!!(command for mkswap)\n");
		return 0;
	}
	sprintf(command, "mkswap %s", swap_path);
	command[len] = 0;
	result = system(command);
	free(command);//*/
	command[0] = "/sbin/mkswap";
	command[1] = swap_path+3;
	command[2] = NULL;
	result = _eval(command, "/dev/console", 0, NULL);
	sleep(1);
	
	if (result < 0) {
		csprintf("\tFailed to mkswap swap file!\n");
		return 0;
	}
	
	result = swapon(swap_path, 0);
	sleep(1);
	if (result == -1) {
		csprintf("\tFailed to swap on the swap file, '%s'!\n", swap_path);
		return 0;
	}
	
	nvram_set_int_temp("swap_on", 1);
	return 1;
}

int stop_swap_from_proc() {
	char *swap_info;
	char *follow_info, *follow_info_end, backup;
	int result, ever_failed = 0;
	FILE *fp;
	
	swap_info = (char *)read_whole_file("/proc/swaps");
	if (swap_info == NULL) {
		csprintf("Can't read '/proc/swaps' out!\n");
		return -1;
	}
	
	if ((follow_info_end = strstr(swap_info, "swap")) == NULL) {
		csprintf("No swap file!\n");
		free(swap_info);
		return 1;
	}
	
	do{
		follow_info = follow_info_end;
		while (*(follow_info-1) != 0 && !isspace(*(follow_info-1)))
			--follow_info;
		
		follow_info_end += 4;
		backup = *follow_info_end;
		*follow_info_end = 0;
		
		if ((fp = fopen(follow_info, "rb")) == NULL) {
			csprintf("No swap file '%s'.\n", follow_info);
			continue;
		}
		fclose(fp);
		
		csprintf("swap '%s' off.\n", follow_info);
		result = swapoff(follow_info);
		sleep(1);
		unlink(follow_info);

		if (result == -1) {
			csprintf("\tFailed to swap off the swap file, '%s'!\n", follow_info);
			csprintf("\tMust reboot the system...!\n");
			ever_failed = 1;
			continue;
		}
		
		*follow_info_end = backup;
	}while ((follow_info_end = strstr(follow_info_end, "swap")) != NULL);
	free(swap_info);
	
	if (ever_failed) {
		csprintf("*** Failed to stop_swap_from_proc()! ***\n");
		nvram_set_int_temp("swapoff_failed", 1);
		
		return 0;
	}
	else{
		csprintf("*** Succeeded to stop_swap_from_proc(). ***\n");
		nvram_set_int_temp("swap_on", 0);
		nvram_set_int_temp("swapoff_failed", 0);
		
		return 1;
	}
}

int stop_swap(const char *disk_port) {
	FILE *fp;
	int disk_index = -1, pool_number = 0;
	int result;
	char nvram_path[16], swap_path[32];
	
	if (disk_port == NULL)
		return stop_swap_from_proc();
		
	if (!swap_check())
		return 1;
	
	if (nvram_match("usb_disc0_port", (char *)disk_port)) {
		disk_index = 0;
		pool_number = atoi(nvram_safe_get("usb_disc0_index"));
	}
	else if (nvram_match("usb_disc1_port", (char *)disk_port)) {
		disk_index = 1;
		pool_number = atoi(nvram_safe_get("usb_disc1_index"));
	}
	else{
		csprintf("No disk in USB port '%s'.\n", disk_port);
		return 0;
	}
	
	if (pool_number == 0) {
		csprintf("No mounted partition in USB port '%s'.\n", disk_port);
		csprintf("So can't swap off.\n");
		return 0;
	}
	
	memset(nvram_path, 0, 16);
	sprintf(nvram_path, "usb_disc%d_path0", disk_index);
	
	memset(swap_path, 0, 32);
	sprintf(swap_path, "%s/.swap", nvram_safe_get(nvram_path));
	
	if ((fp = fopen(swap_path, "rb")) == NULL) {
		csprintf("No swap file in USB port '%s'.\n", disk_port);
		return 1;
	}
	fclose(fp);
	
	result = swapoff(swap_path);
	sleep(1);
	unlink(swap_path);
	
	if (result == -1) {
		csprintf("\tFailed to swap off the swap file, '%s'!\n", swap_path);
		csprintf("\tMust reboot the system...!\n");
		nvram_set_int_temp("swapoff_failed", 1);
		return 0;
	}
	
	nvram_set_int_temp("swap_on", 0);
	nvram_set_int_temp("swapoff_failed", 0);
	return 1;
}

// swap area needs to be at least 40 Kb
int run_swap(const int swap_mega_size) {
	int result = 0;

	if (nvram_match("swap_on", "1"))
		return result;
	
	if (nvram_invmatch("usb_disc0_port", "")) {
		csprintf("swap on in disc0.\n");
		stop_swap(nvram_safe_get("usb_disc0_port"));
		result = start_swap(nvram_safe_get("usb_disc0_port"), swap_mega_size);
	}
	else if (nvram_invmatch("usb_disc1_port", "")) {
		csprintf("swap on in disc1.\n");
		stop_swap(nvram_safe_get("usb_disc1_port"));
		result = start_swap(nvram_safe_get("usb_disc1_port"), swap_mega_size);
	}
	else
		csprintf("Can't swap on when no disk is pluged.\n");
	
	if (result == 1)
		result = swap_check();
	
	return result;
}
/*
int swap_check() {
	struct sysinfo info;

	sysinfo(&info);
	if (info.totalswap > 0)
		return 1;
	else
		return 0;
}
*/
int do_swap_for_format(const char *pool_Kb_size) {
	unsigned long long size_number, swap_Mb_size_number;
	int result = 0;
	char *command[2];
	
	if (pool_Kb_size == NULL) {
		csprintf("Don't input 'pool_Kb_size' in do_swap_for_format().\n");
		return 0;
	}
	
	size_number = (unsigned long long)atoll(pool_Kb_size);
	swap_Mb_size_number = size_number/2049/1024+16;	// It's safer to add morely 16Mb.
	
	//result = system("/sbin/stop_ftpsamba");
	command[0] = "/sbin/stop_ftpsamba";
	command[1] = NULL;
	result = _eval(command, "/dev/console", 0, NULL);
	if (result == -1) {
		csprintf("Failed to stop_ftpsamba - in do_swap_for_format()\n");
		return -1;
	}
	
	result = stop_swap_from_proc();
	if (result == -1) {
		csprintf("Failed to stop_swap - in stop_swap_from_proc()\n");
		return -1;
	}
	
	run_swap((int)swap_Mb_size_number);
	if (result == -1) {
		csprintf("Failed to run_swap - in do_swap_for_format()\n");
		return -1;
	}
	
	return 1;
}
