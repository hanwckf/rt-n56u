#ifndef ASUS_EXT_H
#define ASUS_EXT_H

struct mystr;
struct passwd;
struct vsf_session;

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

//#define DEBUG_LOG 1

#ifdef DEBUG_LOG
#define dbg_log(fmt, args...) do{ \
		FILE *fp = fopen("/vsftpd.log", "a+"); \
		if(fp){ \
			fprintf(fp, "[vsftpd: %s] ", __FUNCTION__); \
			fprintf(fp, fmt, ## args); \
			fclose(fp); \
		} \
	}while(0)
#else
#define dbg_log(fmt, args...)
#endif

#define PERM_READ 1
#define PERM_WRITE 2
#define PERM_DELETE 3

int asus_check_permission(struct vsf_session* p_sess, int perm);
int asus_check_file_visible(struct vsf_session* p_sess, const struct mystr* p_filename_str);
int asus_check_auth(struct mystr* p_user_str, const struct mystr* p_pass_str);
struct passwd *asus_getpwnam(const char *name);


#endif /* ASUS_EXT_H */

