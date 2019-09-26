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
#ifndef _DISK_SHARE_
#define _DISK_SHARE_

#define MAX_ACCOUNT_NUM		50
#define DEFAULT_SAMBA_RIGHT	3
#define DEFAULT_FTP_RIGHT	3
#define DEFAULT_DMS_RIGHT	1

#define BASE_LAYER		1
#define MOUNT_LAYER		(BASE_LAYER+1)
#define SHARE_LAYER		(MOUNT_LAYER+1)

#define FTP_ANONYMOUS_USER	"anonymous"
#define SMB_GUEST_USER		FTP_ANONYMOUS_USER

extern int get_account_list(int *, char ***);
extern int get_folder_list_in_mount_path(const char *const, int *, char ***);
extern int get_all_folder_in_mount_path(const char *const, int *, char ***);
extern void free_2_dimension_list(int *, char ***);
extern int initial_folder_list_in_mount_path(const char *const);
extern int initial_one_var_file_in_mount_path(const char *const, const char *const);
extern int initial_all_var_file_in_mount_path(const char *const);

extern int create_if_no_var_files(const char *const);
extern int modify_if_exist_new_folder(const char *const, const char *const);

extern int get_permission(const char *, const char *, const char *, const char *);
extern int set_permission(const char *, const char *, const char *, const char *, const int);
extern int add_account(const char *const, const char *const);
extern int del_account(const char *const);
extern int mod_account(const char *const, const char *const, const char *const);
extern int add_folder(const char *const, const char *const);
extern int del_folder(const char *const, const char *const);
extern int mod_folder(const char *const, const char *const, const char *const);

extern int test_if_exist_account(const char *const);
extern int test_if_exist_folder_in_mount_path(const char *const, const char *const);

extern int how_many_layer(int, const char *, char **, char **);

#endif // _DISK_SHARE_
