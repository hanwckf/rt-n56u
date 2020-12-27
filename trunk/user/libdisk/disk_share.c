/*
 *  * This program is free software; you can redistribute it and/or
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
#include <limits.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <nvram_linux.h>
#include <shutils.h>

#include "disk_io_tools.h"
#include "disk_initial.h"
#include "disk_share.h"

static void nvram_commit_safe()
{
	if (nvram_get_int("nvram_manual") == 1)
		return;

	nvram_commit();
}

int get_account_list(int *acc_num, char ***account_list)
{
	char nvram_name[24], *nvram_value;
	char **tmp_account_list, **tmp_account;
	int acc_max, len, i, j;

	*acc_num = 0;

	acc_max = nvram_get_int("acc_num");
	if (acc_max > MAX_ACCOUNT_NUM)
		acc_max = MAX_ACCOUNT_NUM;
	if (acc_max <= 0)
		return 0;

	tmp_account_list = NULL;
	for (i = 0; i < acc_max; ++i) {
		sprintf(nvram_name, "acc_username%d", i);
		nvram_value = nvram_safe_get(nvram_name);
		len = strlen(nvram_value);
		if (len < 1)
			continue;
		
		tmp_account = (char **)malloc(sizeof(char *)*(i+1));
		if (tmp_account == NULL) {
			continue;
		}
		
		tmp_account[i] = (char *)malloc(sizeof(char)*(len+1));
		if (tmp_account[i] == NULL) {
			free(tmp_account);
			continue;
		}
		strcpy(tmp_account[i], nvram_value);
		tmp_account[i][len] = 0;
		
		if (i != 0) {
			for (j = 0; j < i; ++j)
				tmp_account[j] = tmp_account_list[j];
			
			free(tmp_account_list);
			tmp_account_list = tmp_account;
		}
		else
			tmp_account_list = tmp_account;
		
		*acc_num += 1;
	}

	if (*acc_num > 0)
		*account_list = tmp_account_list;

	return 0;
}

int get_folder_list_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)
{
	char **tmp_folder_list, target[20];
	int len, i;
	char *list_file, *list_info;
	char *follow_info, *follow_info_end, backup;

	// 1. get list file
	len = strlen(mount_path)+strlen("/.__folder_list.txt");
	list_file = (char *)malloc(sizeof(char)*(len+1));
	if (list_file == NULL) {
		return -1;
	}
	sprintf(list_file, "%s/.__folder_list.txt", mount_path);
	list_file[len] = 0;

	// 2. read if the list file is existed
	if (!test_if_file(list_file))
		initial_folder_list_in_mount_path(mount_path);

	list_info = read_whole_file(list_file);
	if (list_info == NULL) {
		free(list_file);
		return -1;
	}
	free(list_file);

	// 3. find sh_num
	follow_info = strstr(list_info, "sh_num=");
	if (follow_info == NULL) {
		free(list_info);
		return -1;
	}

	follow_info += strlen("sh_num=");
	follow_info_end = follow_info;
	while (*follow_info_end != 0 && *follow_info_end != '\n')
		++follow_info_end;
	if (*follow_info_end == 0) {
		free(list_info);
		return -1;
	}
	backup = *follow_info_end;
	*follow_info_end = 0;

	*sh_num = atoi(follow_info);
	*follow_info_end = backup;

	if (*sh_num <= 0)
		return 0;

	// 4. get folder list from the folder list file
	tmp_folder_list = (char **)malloc(sizeof(char *)*((*sh_num)+1));
	if (tmp_folder_list == NULL) {
		free(list_info);
		return -1;
	}

	for (i = 0; i < *sh_num; ++i) {
		// 5. get folder name
		memset(target, 0, 20);
		sprintf(target, "\nsh_name%d=", i);
		follow_info = strstr(list_info, target);
		if (follow_info == NULL) {
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		
		follow_info += strlen(target);
		follow_info_end = follow_info;
		while (*follow_info_end != 0 && *follow_info_end != '\n')
			++follow_info_end;
		if (*follow_info_end == 0) {
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		backup = *follow_info_end;
		*follow_info_end = 0;
		
		len = strlen(follow_info);
		tmp_folder_list[i] = (char *)malloc(sizeof(char)*(len+1));
		if (tmp_folder_list == NULL) {
			*follow_info_end = backup;
			free(list_info);
			free_2_dimension_list(sh_num, &tmp_folder_list);
			return -1;
		}
		strcpy(tmp_folder_list[i], follow_info);
		tmp_folder_list[i][len] = 0;
		
		*follow_info_end = backup;
	}

	if (*sh_num > 0)
		*folder_list = tmp_folder_list;

	return 0;
}

int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)
{
	DIR *pool_to_open;
	struct dirent *dp;
	char *testdir;
	char **tmp_folder_list, **tmp_folder;
	int len, i;

	tmp_folder_list = NULL;

	pool_to_open = opendir(mount_path);
	if (!pool_to_open)
		return -1;

	*sh_num = 0;
	while ((dp = readdir(pool_to_open)) != NULL) {
		if (dp->d_name[0] == '.')
			continue;
		
		if (test_if_System_folder(dp->d_name) == 1)
			continue;
		
		len = strlen(mount_path)+strlen("/")+strlen(dp->d_name);
		testdir = (char *)malloc(sizeof(char)*(len+1));
		if (testdir == NULL) {
			closedir(pool_to_open);
			return -1;
		}
		sprintf(testdir, "%s/%s", mount_path, dp->d_name);
		testdir[len] = 0;
		if (!test_if_dir(testdir)) {
			free(testdir);
			continue;
		}
		free(testdir);
		
		tmp_folder = (char **)malloc(sizeof(char *)*(*sh_num+1));
		if (tmp_folder == NULL) {
			return -1;
		}
		
		len = strlen(dp->d_name);
		tmp_folder[*sh_num] = (char *)malloc(sizeof(char)*(len+1));
		if (tmp_folder[*sh_num] == NULL) {
			free(tmp_folder);
			
			return -1;
		}
		strcpy(tmp_folder[*sh_num], dp->d_name);
		if (*sh_num != 0) {
			for (i = 0; i < *sh_num; ++i)
				tmp_folder[i] = tmp_folder_list[i];
			free(tmp_folder_list);
			tmp_folder_list = tmp_folder;
		}
		else
			tmp_folder_list = tmp_folder;
		
		++(*sh_num);
	}
	closedir(pool_to_open);

	*folder_list = tmp_folder_list;

	return 0;
}

void free_2_dimension_list(int *num, char ***list)
{
	int i;
	char **target = *list;

	if (*num <= 0 || target == NULL)
		return;

	for (i = 0; i < *num; ++i)
		if (target[i] != NULL)
			free(target[i]);

	if (target != NULL)
		free(target);

	*num = 0;
}

int initial_folder_list_in_mount_path(const char *const mount_path)
{
	int sh_num = 0;
	char **folder_list = NULL;
	FILE *fp;
	char *list_file;
	int result, len, i;

	if (mount_path == NULL || strlen(mount_path) <= 0)
		return -1;

	// 2. get the folder number and folder_list
	result = get_all_folder_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	// 3. get the list_file
	len = strlen(mount_path)+strlen("/.__folder_list.txt");
	list_file = (char *)malloc(sizeof(char)*(len+1));
	if (list_file == NULL) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}
	sprintf(list_file, "%s/.__folder_list.txt", mount_path);
	list_file[len] = 0;

	// 4. write the folder info
	fp = fopen(list_file, "w");
	if (fp == NULL) {
		free_2_dimension_list(&sh_num, &folder_list);
		free(list_file);
		
		return -1;
	}
	free(list_file);

	fprintf(fp, "sh_num=%d\n", sh_num);
	for (i = 0; i < sh_num; ++i)
		fprintf(fp, "sh_name%d=%s\n", i, folder_list[i]);
	fclose(fp);

	free_2_dimension_list(&sh_num, &folder_list);
	return 0;
}

int initial_one_var_file_in_mount_path(const char *const account, const char *const mount_path)
{
	FILE *fp;
	char var_file[256];
	int result, i;
	int sh_num = 0;
	char **folder_list = NULL;
	int samba_right, ftp_right;

	if (account == NULL || strlen(account) <= 0)
		return -1;
	if (mount_path == NULL || strlen(mount_path) <= 0)
		return -1;

	// get the folder number and folder_list
	result = get_all_folder_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	// get the samba right and ftp right
	if (strcmp(account, FTP_ANONYMOUS_USER) == 0/* || strcmp(account, SMB_GUEST_USER) == 0*/) {
		samba_right = 0;
		ftp_right = 1;
	}
	else {
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
	}

	// write the default content in the var file
	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account);
	if ((fp = fopen(var_file, "w")) == NULL) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}
	for (i = 0; i < sh_num; ++i) {
		fprintf(fp, "*");
		
		fprintf(fp, "%s", folder_list[i]);
		fprintf(fp, "=%d%d%d\n", samba_right, ftp_right, DEFAULT_DMS_RIGHT);
	}

	fclose(fp);
	free_2_dimension_list(&sh_num, &folder_list);

	return 0;
}

int initial_all_var_file_in_mount_path(const char *const mount_path)
{
	int i;
	int acc_num = 0;
	char **account_list = NULL;

	if (mount_path == NULL || strlen(mount_path) <= 0)
		return -1;

	// delete all var files
	doSystem("rm -f /%s/.__*.txt", mount_path);

	// initial the var file for ftp anonymous
	initial_one_var_file_in_mount_path(FTP_ANONYMOUS_USER, mount_path);

	// get the account number and account_list
	get_account_list(&acc_num, &account_list);

	// initial the var file
	for (i = 0; i < acc_num; ++i)
		initial_one_var_file_in_mount_path(account_list[i], mount_path);
	free_2_dimension_list(&acc_num, &account_list);

	// initial the folder list
	initial_folder_list_in_mount_path(mount_path);

	return 0;
}

int create_if_no_var_files(const char *const mount_path)
{
	int acc_num = 0;
	char **account_list = NULL;
	int i;
	char var_file[256];

	// process aninymous user
	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, FTP_ANONYMOUS_USER);
	if (!test_if_file(var_file))
		initial_one_var_file_in_mount_path(FTP_ANONYMOUS_USER, mount_path);
	else
		modify_if_exist_new_folder(FTP_ANONYMOUS_USER, mount_path);

	// get the account number and account_list
	get_account_list(&acc_num, &account_list);

	for (i = 0; i < acc_num; ++i) {
		snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account_list[i]);
		if (!test_if_file(var_file))
			initial_one_var_file_in_mount_path(account_list[i], mount_path);
		else
			modify_if_exist_new_folder(account_list[i], mount_path);
	}

	free_2_dimension_list(&acc_num, &account_list);

	return 0;
}

int modify_if_exist_new_folder(const char *const account, const char *const mount_path)
{
	int sh_num = 0;
	char **folder_list = NULL, *target;
	int result, i, len;
	char var_file[256];
	FILE *fp;
	int samba_right, ftp_right;

	// 1. get all folder in mount_path
	result = get_all_folder_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	for (i = 0; i < sh_num; ++i) {
		result = test_if_exist_folder_in_mount_path(mount_path, folder_list[i]);
		if (result != 0) {
			continue;
		}
		
		// get the target
		len = strlen("*")+strlen(folder_list[i])+strlen("=");
		target = (char *)malloc(sizeof(char)*(len+1));
		if (target == NULL) {
			free_2_dimension_list(&sh_num, &folder_list);
			
			return -1;
		}
		sprintf(target, "*%s=", folder_list[i]);
		target[len] = 0;
		
		// get the samba right and ftp right
		if (strcmp(account, FTP_ANONYMOUS_USER) == 0) {
			samba_right = 1;
			ftp_right = 1;
		}
		else {
			samba_right = DEFAULT_SAMBA_RIGHT;
			ftp_right = DEFAULT_FTP_RIGHT;
		}
		
		// add the information of the new folder
		snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account);
		fp = fopen(var_file, "a+");
		if (fp == NULL) {
			free_2_dimension_list(&sh_num, &folder_list);
			free(target);
			return -1;
		}
		
		fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, DEFAULT_DMS_RIGHT);
		free(target);
		fclose(fp);
	}
	free_2_dimension_list(&sh_num, &folder_list);

	return 0;
}

int get_permission(const char *account,
		   const char *mount_path,
		   const char *folder,
		   const char *protocol)
{
	char var_file[256];
	char *target, *follow_info, *var_info;
	int len, result;

	if (!mount_path || !folder)
		return -1;

	// 2. get the content of the var_file of the account
	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account);
	var_info = read_whole_file(var_file);
	if (!var_info)
		return 0;

	// 3. get the target in the content
	len = strlen(folder)+strlen("*=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (!target) {
		free(var_info);
		return -1;
	}
	sprintf(target, "*%s=", folder);
	target[len] = 0;

	follow_info = upper_strstr(var_info, target);
	free(target);
	if (follow_info == NULL) {
		free(var_info);
		return -1;
	}

	follow_info += len;
	if (follow_info[3] != '\n') {
		free(var_info);
		return -1;
	}

	// 4. get the right of folder
	if (!strcmp(protocol, "cifs"))
		result = follow_info[0]-'0';
	else if (!strcmp(protocol, "ftp"))
		result = follow_info[1]-'0';
	else if (!strcmp(protocol, "dms"))
		result = follow_info[2]-'0';
	else{
		free(var_info);
		return -1;
	}
	free(var_info);

	if (result < 0 || result > 3)
		return -1;

	return result;
}

int set_permission(const char *account,
		   const char *mount_path,
		   const char *folder,
		   const char *protocol,
		   const int flag)
{
	FILE *fp;
	char var_file[256];
	char *target, *follow_info, *var_info;
	int len, result;

	if (flag < 0 || flag > 3)
		return -1;

	// 2. get the content of the var_file of the account
	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account);
	var_info = read_whole_file(var_file);
	if (!var_info) {
		initial_one_var_file_in_mount_path(account, mount_path);
		sleep(1);
		var_info = read_whole_file(var_file);
		if (!var_info)
			return -1;
	}

	// 3. get the target in the content
	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (!target) {
		free(var_info);
		return -1;
	}
	sprintf(target, "*%s=", folder);
	target[len] = 0;

	// 4. judge if the target is in the var file.
	follow_info = upper_strstr(var_info, target);
	if (!follow_info) {
		free(var_info);
		
		result = initial_folder_list_in_mount_path(mount_path);
		
		fp = fopen(var_file, "a+");
		if (!fp) {
			free(target);
			return -1;
		}
		
		fprintf(fp, "%s", target);
		free(target);
		
		// 5.1 change the right of folder
		if (!strcmp(protocol, "cifs"))
			fprintf(fp, "%d%d%d\n", flag, 0, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, "ftp"))
			fprintf(fp, "%d%d%d\n", 0, flag, DEFAULT_DMS_RIGHT);
		else if (!strcmp(protocol, "dms"))
			fprintf(fp, "%d%d%d\n", 0, 0, flag);
		else{
			fclose(fp);
			return -1;
		}
		fclose(fp);
		
		if (!strcmp(protocol, "cifs"))
			system("/sbin/run_samba");
		
		return 0;
	}
	free(target);

	follow_info += len;
	if (follow_info[3] != '\n') {
		free(var_info);
		return -1;
	}

	// 5.2. change the right of folder
	if (!strcmp(protocol, "cifs")) {
		if (follow_info[0] == '0'+flag) {
			free(var_info);
			return 0;
		}
		
		follow_info[0] = '0'+flag;
	}
	else if (!strcmp(protocol, "ftp")) {
		if (follow_info[1] == '0'+flag) {
			free(var_info);
			return 0;
		}
		
		follow_info[1] = '0'+flag;
	}
	else if (!strcmp(protocol, "dms")) {
		if (follow_info[2] == '0'+flag) {
			free(var_info);
			return 0;
		}
		
		follow_info[2] = '0'+flag;
	}
	else{
		free(var_info);
		return -1;
	}

	// 6. rewrite the var file.
	fp = fopen(var_file, "w");
	if (fp == NULL) {
		free(var_info);
		return -1;
	}
	fprintf(fp, "%s", var_info);
	fclose(fp);

	free(var_info);

	if (!strcmp(protocol, "cifs"))
		system("/sbin/run_samba");

	return 0;
}

int add_account(const char *const account, const char *const password)
{
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int i, acc_num = 0;
	char **account_list = NULL;
	char nvram_name[32];

	if (account == NULL || strlen(account) <= 0)
		return -1;
	if (password == NULL || strlen(password) <= 0)
		return -1;

	// 1. check if can create the account
	get_account_list(&acc_num, &account_list);
	if (acc_num >= MAX_ACCOUNT_NUM) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account, account_list[i])) {
			free_2_dimension_list(&acc_num, &account_list);
			return -1;
		}

	if (!strcmp(account, FTP_ANONYMOUS_USER)) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	// 2. create nvram value about the new account
	nvram_set_int("acc_num", acc_num+1);

	sprintf(nvram_name, "acc_username%d", acc_num);
	nvram_set(nvram_name, account);

	sprintf(nvram_name, "acc_password%d", acc_num);
	nvram_set(nvram_name, password);

	nvram_commit_safe();

	free_2_dimension_list(&acc_num, &account_list);

	// 3. find every pool
	disk_list = read_disk_data();
	if (!disk_list)
		return 0;

	for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {			
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;
			
			// 4. initial the var file of the account
			initial_one_var_file_in_mount_path(account, follow_partition->mount_point);
		}
	}
	free_disk_data(disk_list);

	// 6. re-run samba
	system("/sbin/run_samba");

	return 0;
}

int del_account(const char *const account)
{
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num = 0, target;
	char **account_list = NULL;
	int result, i;
	char nvram_name[16], nvram_value[128];
	char var_file[256];

	if (account == NULL || strlen(account) <= 0)
		return -1;

	// 1. check if can create the account
	get_account_list(&acc_num, &account_list);
	if (acc_num <= 0)
		return -1;

	result = 0;
	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account_list[i], account)) {
			result = 1;
			target = i;
			break;
		}
	if (result == 0) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	// 2. delete the nvram value about the deleted account
	--acc_num;
	nvram_set_int("acc_num", acc_num);

	for (i = target; i < acc_num; ++i) {
		sprintf(nvram_name, "acc_username%d", i);
		nvram_set(nvram_name, account_list[i+1]);
		
		sprintf(nvram_value, "acc_password%d", i+1);
		
		sprintf(nvram_name, "acc_password%d", i);
		nvram_set(nvram_name, nvram_safe_get(nvram_value));
	}

	// 3. change to the share mode when no account
	if (acc_num < 1) {
		int i_mode;
		
		i_mode = nvram_get_int("st_samba_mode");
		if (i_mode != 1 && i_mode != 3)
			nvram_set_int("st_samba_mode", 1);
		
		i_mode = nvram_get_int("st_ftp_mode");
		if (i_mode != 1 && i_mode != 3)
			nvram_set_int("st_ftp_mode", 1);
	}

	nvram_commit_safe();

	free_2_dimension_list(&acc_num, &account_list);

	// 4. find every pool
	disk_list = read_disk_data();
	if (!disk_list)
		return 0;

	for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;
			
			// 5. delete the var file of the deleted account
			snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", follow_partition->mount_point, account);
			
			unlink(var_file);
		}
	}
	free_disk_data(disk_list);

	// 6. re-run ftp and samba
	system("/sbin/run_ftpsamba");

	return 0;
}

// "new_account" can be the same with "account"!
int mod_account(const char *const account, const char *const new_account, const char *const new_password)
{
	disk_info_t *disk_list, *follow_disk;
	partition_info_t *follow_partition;
	int i, acc_num = 0;
	char **account_list = NULL;
	char nvram_name[16];
	int account_order = -1;
	char var_file[256], new_var_file[256];

	if (account == NULL || strlen(account) <= 0)
		return -1;
	if ((new_account == NULL || strlen(new_account) <= 0) && (new_password == NULL || strlen(new_password) <= 0))
		return -1;

	// 1. check if can modify the account
	get_account_list(&acc_num, &account_list);

	for (i = 0; i < acc_num; ++i) {
		if (new_account != NULL && strcmp(new_account, account) && strlen(new_account) > 0) {
			if (!strcmp(account_list[i], new_account)) {
				free_2_dimension_list(&acc_num, &account_list);
				
				return -1;
			}
		}
		
		if (!strcmp(account, account_list[i]))
			account_order = i;
	}
	free_2_dimension_list(&acc_num, &account_list);

	if (account_order == -1)
		return -1;

	if (!strcmp(new_account, FTP_ANONYMOUS_USER))
		return -1;

	// 2. modify nvram value about the new account
	if (new_account != NULL && strcmp(new_account, account) && strlen(new_account) > 0) {
		sprintf(nvram_name, "acc_username%d", account_order);
		nvram_set(nvram_name, new_account);
	}

	if (new_password != NULL && strlen(new_password) > 0) {
		sprintf(nvram_name, "acc_password%d", account_order);
		nvram_set(nvram_name, new_password);
	}

	nvram_commit_safe();

	// 3. find every pool
	if (new_account == NULL || strlen(new_account) <= 0/* ||
			(new_account != NULL && !strcmp(new_account, account))*/)
		return 0;

	if ((new_account != NULL && !strcmp(new_account, account)))
		goto rerun;

	disk_list = read_disk_data();
	if (!disk_list)
		return 0;

	for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;
			
			// get the var_file and new_var_file
			snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", follow_partition->mount_point, account);
			snprintf(new_var_file, sizeof(new_var_file), "%s/.__%s_var.txt", follow_partition->mount_point, new_account);
			
			// rename the var file
			rename(var_file, new_var_file);
		}
	}
	free_disk_data(disk_list);

rerun:
	// 6. re-run ftp and samba
	system("/sbin/run_ftpsamba");

	return 0;
}

int add_folder(const char *const mount_path, const char *const folder)
{
	int result, i, len;
	int acc_num = 0;
	char **account_list = NULL;
	char var_file[256];
	char *target, *var_info;
	FILE *fp;
	int samba_right, ftp_right;
	char *full_path;

	if (mount_path == NULL || strlen(mount_path) <= 0)
		return -1;
	if (folder == NULL || strlen(folder) <= 0)
		return -1;

	// test if creatting the folder
	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result != 0)
		return -1;

	// create the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if (!full_path)
		return -1;
	sprintf(full_path, "%s/%s", mount_path, folder);
	full_path[len] = 0;

	umask(0000);
	result = mkdir(full_path, 0777);
	free(full_path);
	if (result != 0)
		return -1;

	// get the samba right, ftp right and target
	samba_right = DEFAULT_SAMBA_RIGHT;
	ftp_right = DEFAULT_FTP_RIGHT;

	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (!target)
		return -1;
	sprintf(target, "*%s=", folder);
	target[len] = 0;

	//  add folder's right to every var file
	get_account_list(&acc_num, &account_list);

	for (i = 0; i < acc_num; ++i) {
		// check if the created target is exist in the var file
		snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account_list[i]);
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			;
		}
		else if (strstr(var_info, target) != NULL) {
			free(var_info);
			
			continue;
		}
		else
			free(var_info);
		
		// 7. add the folder's info in the var file
		fp = fopen(var_file, "a+");
		if (fp) {
			fprintf(fp, "%s%d%d%d\n", target, samba_right, ftp_right, DEFAULT_DMS_RIGHT);
			fclose(fp);
		}
	}

	free_2_dimension_list(&acc_num, &account_list);

	// get the var_file for anonymous ftp
	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, FTP_ANONYMOUS_USER);
	fp = fopen(var_file, "a+");
	if (fp) {
		fprintf(fp, "%s%d%d%d\n", target, 1, 1, 1);
		fclose(fp);
	}

	free(target);

	// 9. add the folder's info in the folder list
	initial_folder_list_in_mount_path(mount_path);

	return 0;
}

int del_folder(const char *const mount_path, const char *const folder)
{
	int result, i, len;
	int acc_num = 0;
	char **account_list = NULL;
	char var_file[256];
	char *follow_info, backup;
	char *target, *var_info;
	FILE *fp;
	char *full_path;

	if (mount_path == NULL || strlen(mount_path) <= 0)
		return -1;
	if (folder == NULL || strlen(folder) <= 0)
		return -1;

	// 1. test if deleting the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if (!full_path)
		return -1;
	sprintf(full_path, "%s/%s", mount_path, folder);
	full_path[len] = 0;

	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result == 0) {
		result = test_if_dir(full_path);
		
		if (result != 1) {
			free(full_path);
			
			return -1;
		}
	}

	// 2. delete the folder
	result = rmdir(full_path);
	free(full_path);
	if (result != 0)
		return -1;

	// 4. get the target which is deleted in every var file
	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (!target)
		return -1;
	sprintf(target, "*%s=", folder);
	target[len] = 0;

	// 3. del folder's right to every var file
	get_account_list(&acc_num, &account_list);

	for (i = 0; i < acc_num; ++i) {
		// delete the content about the folder
		snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account_list[i]);
		var_info = read_whole_file(var_file);
		if (!var_info)
			continue;
		
		follow_info = upper_strstr(var_info, target);
		if (!follow_info) {
			free(var_info);
			continue;
		}
		backup = *follow_info;
		*follow_info = 0;
		
		fp = fopen(var_file, "w");
		if (!fp) {
			*follow_info = backup;
			free(var_info);
			continue;
		}
		fprintf(fp, "%s", var_info);
		
		*follow_info = backup;
		while (*follow_info != 0 && *follow_info != '\n')
			++follow_info;
		if (*follow_info != 0 && *(follow_info+1) != 0) {
			++follow_info;
			fprintf(fp, "%s", follow_info);
		}
		fclose(fp);
		
		free(var_info);
	}

	free_2_dimension_list(&acc_num, &account_list);

	// get the var_file for anonymous ftp
	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, FTP_ANONYMOUS_USER);
	var_info = read_whole_file(var_file);
	if (var_info == NULL) {
		goto MOD_FOLDER_END;
	}

	follow_info = upper_strstr(var_info, target);
	if (follow_info == NULL) {
		free(var_info);
		
		goto MOD_FOLDER_END;
	}
	backup = *follow_info;
	*follow_info = 0;

	fp = fopen(var_file, "w");
	if (fp == NULL) {
		*follow_info = backup;
		free(var_info);
		
		goto MOD_FOLDER_END;
	}
	fprintf(fp, "%s", var_info);

	*follow_info = backup;
	while (*follow_info != 0 && *follow_info != '\n')
		++follow_info;
	if (*follow_info != 0 && *(follow_info+1) != 0) {
		++follow_info;
		fprintf(fp, "%s", follow_info);
	}
	fclose(fp);

	free(var_info);

MOD_FOLDER_END:

	free(target);

	// 9. modify the folder's info in the folder list
	initial_folder_list_in_mount_path(mount_path);

	return 0;
}

int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)
{
	int result, i, len;
	int acc_num = 0;
	char **account_list = NULL;
	char var_file[256];
	char *target, *new_target, *var_info;
	FILE *fp;
	char *follow_info, backup;
	char *full_path, *new_full_path;

	if (mount_path == NULL || strlen(mount_path) <= 0)
		return -1;
	if (folder == NULL || strlen(folder) <= 0)
		return -1;
	if (new_folder == NULL || strlen(new_folder) <= 0)
		return -1;

	// 1. test if modifying the folder
	len = strlen(mount_path)+strlen("/")+strlen(folder);
	full_path = (char *)malloc(sizeof(char)*(len+1));
	if (!full_path)
		return -1;
	sprintf(full_path, "%s/%s", mount_path, folder);
	full_path[len] = 0;

	len = strlen(mount_path)+strlen("/")+strlen(new_folder);
	new_full_path = (char *)malloc(sizeof(char)*(len+1));
	if (!new_full_path)
		return -1;
	sprintf(new_full_path, "%s/%s", mount_path, new_folder);
	new_full_path[len] = 0;

	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result == 0) {
		result = test_if_dir(full_path);
		
		if (result != 1) {
			free(full_path);
			free(new_full_path);
			
			return -1;
		}
		
		// the folder is existed but not in .__folder_list.txt
		add_folder(mount_path, folder);
	}

	//  modify the folder
	result = rename(full_path, new_full_path);
	free(full_path);
	free(new_full_path);
	if (result != 0)
		return -1;

	len = strlen("*")+strlen(folder)+strlen("=");
	target = (char *)malloc(sizeof(char)*(len+1));
	if (!target)
		return -1;
	sprintf(target, "*%s=", folder);
	target[len] = 0;

	len = strlen("*")+strlen(new_folder)+strlen("=");
	new_target = (char *)malloc(sizeof(char)*(len+1));
	if (!new_target) {
		free(target);
		return -1;
	}
	sprintf(new_target, "*%s=", new_folder);
	new_target[len] = 0;

	// 3. add folder's right to every var file
	get_account_list(&acc_num, &account_list);

	for (i = 0; i < acc_num; ++i) {
		// check if the created target is exist in the var file
		snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account_list[i]);
		var_info = read_whole_file(var_file);
		if (var_info == NULL) {
			continue;
		}
		
		if ((follow_info = strstr(var_info, target)) == NULL) {
			free(var_info);
			continue;
		}
		
		// 7. modify the folder's info in the var file
		fp = fopen(var_file, "w");
		if (fp) {
			// write the info before target
			backup = *follow_info;
			*follow_info = 0;
			fprintf(fp, "%s", var_info);
			*follow_info = backup;
			
			// write the info before new_target
			fprintf(fp, "%s", new_target);
			
			// write the info after target
			follow_info += strlen(target);
			fprintf(fp, "%s", follow_info);
			
			fclose(fp);
		}
		
		free(var_info);
	}

	free_2_dimension_list(&acc_num, &account_list);

	// get the var_file for anonymous ftp
	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, FTP_ANONYMOUS_USER);
	var_info = read_whole_file(var_file);
	if (var_info) {
		if ((follow_info = strstr(var_info, target))) {
			fp = fopen(var_file, "w");
			if (fp) {
				// write the info before target
				backup = *follow_info;
				*follow_info = 0;
				fprintf(fp, "%s", var_info);
				*follow_info = backup;
				
				// write the info before new_target
				fprintf(fp, "%s", new_target);
				
				// write the info after target
				follow_info += strlen(target);
				fprintf(fp, "%s", follow_info);
				
				fclose(fp);
			}
		}
		
		free(var_info);
	}

	free(target);
	free(new_target);

	// 9. modify the folder's info in the folder list
	initial_folder_list_in_mount_path(mount_path);

	return 0;
}

int test_if_exist_account(const char *const account)
{
	int acc_num = 0;
	char **account_list = NULL;
	int result, i;

	if (!strcmp(account, FTP_ANONYMOUS_USER))
		return 1;

	get_account_list(&acc_num, &account_list);

	result = 0;
	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account, account_list[i])) {
			result = 1;
			break;
		}

	free_2_dimension_list(&acc_num, &account_list);
	return result;
}

int test_if_exist_folder_in_mount_path(const char *const mount_path, const char *const folder)
{
	int sh_num = 0;
	char **folder_list = NULL;
	int result, i;

	result = get_folder_list_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	result = 0;
	for (i = 0; i < sh_num; ++i)
		if (!upper_strcmp(folder, folder_list[i])) {
			result = 1;
			break;
		}

	free_2_dimension_list(&sh_num, &folder_list);
	return result;
}

// for FTP: root dir is POOL_MOUNT_ROOT(/media)
int how_many_layer(int is_chroot, const char *basedir, char **mount_path, char **share)
{
	char *follow_info, *follow_info_end;
	int layer = 0, len = 0;

	if (is_chroot)
		layer = BASE_LAYER;

	if (!strcmp(basedir, "/"))
		return layer;

	len = strlen(basedir);
	if (len > 1)
		layer++;

	follow_info = (char *)basedir;
	while (*follow_info != 0 && (follow_info = index(follow_info+1, '/')) != NULL)
		++layer;

	if (layer >= MOUNT_LAYER) {
		follow_info = (char *)basedir;
		if (!is_chroot)
			follow_info += strlen(POOL_MOUNT_ROOT);
		follow_info = index(follow_info+1, '/');
		if (follow_info == NULL)
			len = strlen(basedir);
		else
			len = strlen(basedir)-strlen(follow_info);
		*mount_path = (char *)malloc(sizeof(char)*(len+1));
		if (*mount_path == NULL)
			return -1;
		strncpy(*mount_path, basedir, len);
		(*mount_path)[len] = 0;
	}

	if (layer >= SHARE_LAYER) {
		++follow_info;
		follow_info_end = index(follow_info, '/');
		if (follow_info_end == NULL)
			len = strlen(follow_info);
		else
			len = strlen(follow_info)-strlen(follow_info_end);
		*share = (char *)malloc(sizeof(char)*(len+1));
		if (*share == NULL)
			return -1;
		strncpy(*share, follow_info, len);
		(*share)[len] = 0;
	}

	return layer;
}

