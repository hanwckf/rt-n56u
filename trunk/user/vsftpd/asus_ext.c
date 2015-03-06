
#include "asus_ext.h"
#include "session.h"
#include "ls.h"
#include "tunables.h"
#include "defs.h"
#include "str.h"
#include "sysstr.h"
#include "sysutil.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <nvram_linux.h>
#include <disk_io_tools.h>
#include <disk_initial.h>
#include <disk_share.h>

#define HIDE_PARTITION_FILE ".hidden"

int asus_share_mode_read(void)
{
	return nvram_get_int("st_ftp_mode");
}

int test_if_dir_stat(const char *path)
{
	struct stat st;
	return (stat(path, &st) == 0) && (S_ISDIR(st.st_mode));
}

int test_if_hidden_share(const char *share_path)
{
	struct stat st;
	static char s_share_test[256];

	snprintf(s_share_test, sizeof(s_share_test), "%s/%s", share_path, HIDE_PARTITION_FILE);

	return (stat(s_share_test, &st) == 0) && (S_ISREG(st.st_mode));
}

int get_mount_layer(const char *basedir, char **mount_path, char **share)
{
	char *follow_info, *follow_info_end;
	int layer = BASE_LAYER, len = 0;

	if (!basedir)
		return -1;

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

char* read_file_to_buffer(const char *target_file)
{
	FILE *fp;
	char *buffer;
	struct stat stat_buf;
	int i_readed, i_chunk, i_readed_total;

	memset(&stat_buf, 0, sizeof(stat_buf));
	stat(target_file, &stat_buf);

	fp = fopen(target_file, "r");
	if (!fp)
		return NULL;

	if (stat_buf.st_size < 1 || stat_buf.st_size > 16*1024*1024) {
		fclose(fp);
		return NULL;
	}

	buffer = (char *)malloc(stat_buf.st_size + 1);
	if (!buffer) {
		fclose(fp);
		return NULL;
	}

	i_chunk = MIN(1024, stat_buf.st_size);
	i_readed_total = 0;
	while ((i_readed = fread(buffer+i_readed_total, sizeof(char), i_chunk, fp)) > 0) {
		i_readed_total += i_readed;
		i_chunk = MIN(1024, stat_buf.st_size - i_readed_total);
		if (i_chunk < 1)
			break;
	}

	fclose(fp);

	buffer[i_readed_total] = 0;

	return buffer;
}

int get_permission_ftp(const char *account, const char *mount_path, const char *folder)
{
	char var_file[256], *var_info;
	char *target, *follow_info;
	int len, result;

	if (!mount_path || !folder)
		return -1;

	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account);
	var_info = read_file_to_buffer(var_file);
	if (!var_info)
		return 0;

	len = strlen(folder)+strlen("*=");
	target = (char *)malloc(len+1);
	if (!target) {
		free(var_info);
		return -1;
	}
	sprintf(target, "*%s=", folder);

	follow_info = strstr(var_info, target);
	free(target);

	if (!follow_info) {
		free(var_info);
		return -1;
	}

	follow_info += len;
	if (follow_info[3] != '\n') {
		free(var_info);
		return -1;
	}

	result = follow_info[1]-'0';
	free(var_info);

	if (result < 0 || result > 3)
		return -1;

	return result;
}

int asus_check_permission(struct vsf_session* p_sess, int perm)
{
	int i_layer, i_user_right, i_result;
	char *mount_path = NULL, *share_name = NULL;

	if (!tunable_chroot_local_user)
		return 1;

	str_getcwd(&p_sess->cwd_str);

	i_result = 1;
	i_layer = get_mount_layer(str_getbuf(&p_sess->cwd_str), &mount_path, &share_name);

	if (mount_path && test_if_hidden_share(mount_path)) {
		i_result = 0;
		goto free_and_exit;
	}

	if(i_layer < SHARE_LAYER){
		if (perm == PERM_DELETE || perm == PERM_WRITE){
			if (i_layer < MOUNT_LAYER || !p_sess->is_anonymous || p_sess->st_ftp_mode != 1){
				i_result = 0;
				goto free_and_exit;
			}
		}
	}
	else{
		const char *account_ftp;
		
		if (p_sess->is_anonymous){
			if (p_sess->st_ftp_mode == 1)
				goto free_and_exit;
			account_ftp = FTP_ANONYMOUS_USER;
		}
		else{
			account_ftp = str_getbuf(&p_sess->user_str);
		}
		
		i_user_right = get_permission_ftp(account_ftp, mount_path, share_name);
		if (perm == PERM_DELETE){
			if (i_user_right < 3){
				i_result = 0;
				goto free_and_exit;
			}
		}
		else if (perm == PERM_WRITE){
			if (i_user_right < 2){
				i_result = 0;
				goto free_and_exit;
			}
		}
		else if (perm == PERM_READ){
			if (i_user_right < 1 || i_user_right == 2){
				i_result = 0;
				goto free_and_exit;
			}
		}
	}

free_and_exit:

	if (mount_path)
		free(mount_path);
	if (share_name)
		free(share_name);

	return i_result;
}

int asus_check_file_visible(struct vsf_session* p_sess, const struct mystr* p_filename_str)
{
	int i_layer, i_user_right, i_result;
	char *mount_path = NULL, *share_name = NULL;
	const char *p_filename, *p_fullpath;

	if (str_isempty(p_filename_str))
		return 0;

	if (!tunable_chroot_local_user)
		return 1;

	p_filename = str_getbuf(p_filename_str);
	if (!strcmp(p_filename, ".") || !strcmp(p_filename, ".."))
		return 1;

	str_getcwd(&p_sess->cwd_str);
	if (!str_equal_text(&p_sess->cwd_str, "/"))
		str_append_char(&p_sess->cwd_str, '/');
	str_append_str(&p_sess->cwd_str, p_filename_str);
	p_fullpath = str_getbuf(&p_sess->cwd_str);

	i_result = 1;
	i_layer = get_mount_layer(p_fullpath, &mount_path, &share_name);

	if (mount_path && test_if_hidden_share(mount_path)) {
		i_result = 0;
		goto free_and_exit;
	}

	if(i_layer == MOUNT_LAYER){
		if(!test_if_dir_stat(p_fullpath)){
			i_result = 0;
			goto free_and_exit;
		}
	}
	else if(i_layer == SHARE_LAYER){
		const char *account_ftp;
		
		if(test_if_System_folder(p_filename)){
			i_result = 0;
			goto free_and_exit;
		}
		
		if(!strncmp(p_filename, ".__", 3)){
			i_result = 0;
			goto free_and_exit;
		}
		
		if(p_sess->is_anonymous){
			if (p_sess->st_ftp_mode == 1)
				goto free_and_exit;
			account_ftp = FTP_ANONYMOUS_USER;
		}
		else{
			account_ftp = str_getbuf(&p_sess->user_str);
		}
		
		if(!test_if_dir_stat(p_fullpath)){
			i_result = 0;
			goto free_and_exit;
		}
		
		i_user_right = get_permission_ftp(account_ftp, mount_path, share_name);
		if (i_user_right < 1) {
			i_result = 0;
			goto free_and_exit;
		}
	}

free_and_exit:

	if (mount_path)
		free(mount_path);
	if (share_name)
		free(share_name);

	return i_result;
}

struct passwd *asus_getpwnam(const char *name)
{
	static struct passwd resultbuf;
	static char pw_name[VSFTP_USERNAME_MAX];
	static char pw_passwd[VSFTP_PASSWORD_MAX];
	char nvram_value[32];
	char *tmp_user, *tmp_pass;
	int acc_num, i;

	if (strcmp(name, "root") == 0){
		strcpy(pw_name, name);
		strcpy(pw_passwd, "*");
		resultbuf.pw_name = pw_name;
		resultbuf.pw_passwd = pw_passwd;
		resultbuf.pw_uid = 0;
		resultbuf.pw_gid = 0;
		resultbuf.pw_gecos = pw_name;
		resultbuf.pw_dir = POOL_MOUNT_ROOT;
		resultbuf.pw_shell = "/bin/false";
		return &resultbuf;
	}
	else if(strcmp(name, "ftp") == 0){
		strcpy(pw_name, name);
		strcpy(pw_passwd, "*");
		resultbuf.pw_name = pw_name;
		resultbuf.pw_passwd = pw_passwd;
		resultbuf.pw_uid = 99; // nobody
		resultbuf.pw_gid = 99; // nogroup
		resultbuf.pw_gecos = pw_name;
		resultbuf.pw_dir = POOL_MOUNT_ROOT;
		resultbuf.pw_shell = "/bin/false";
		return &resultbuf;
	}
	else {
		acc_num = nvram_get_int("acc_num");
		if (acc_num > MAX_ACCOUNT_NUM) acc_num = MAX_ACCOUNT_NUM;
		for(i = 0; i < acc_num; i++){
			sprintf(nvram_value, "acc_username%d", i);
			tmp_user = nvram_safe_get(nvram_value);
			sprintf(nvram_value, "acc_password%d", i);
			tmp_pass = nvram_safe_get(nvram_value);
			if(strcmp(name, tmp_user) == 0){
				strcpy(pw_name, name);
				strcpy(pw_passwd, tmp_pass);
				resultbuf.pw_name = pw_name;
				resultbuf.pw_passwd = pw_passwd;
				resultbuf.pw_uid = 99; // nobody
				resultbuf.pw_gid = 99; // nogroup
				resultbuf.pw_gecos = pw_name;
				resultbuf.pw_dir = POOL_MOUNT_ROOT;
				resultbuf.pw_shell = "/bin/sh";
				return &resultbuf;
			}
		}
	}

	return NULL;
}

int
asus_check_auth(struct mystr* p_user_str, const struct mystr* p_pass_str)
{
	const char* p_pass;
	const struct passwd* p_pwd = asus_getpwnam(str_getbuf(p_user_str));
	if (!p_pwd)
	{
		return 0;
	}
	p_pass = str_getbuf(p_pass_str);
	if ( vsf_sysutil_strcmp(p_pass, "*") &&
	    !vsf_sysutil_strcmp(p_pass, p_pwd->pw_passwd))
	{
		return 1;
	}
	return 0;
}

