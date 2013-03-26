#ifndef ASUS_EXT_H
#define ASUS_EXT_H

struct mystr;
struct passwd;
struct vsf_session;

#define PERM_READ 1
#define PERM_WRITE 2
#define PERM_DELETE 3

int asus_share_mode_read(void);
int asus_check_permission(struct vsf_session* p_sess, int perm);
int asus_check_file_visible(struct vsf_session* p_sess, const struct mystr* p_filename_str);
int asus_check_auth(struct mystr* p_user_str, const struct mystr* p_pass_str);
struct passwd *asus_getpwnam(const char *name);


#endif /* ASUS_EXT_H */

