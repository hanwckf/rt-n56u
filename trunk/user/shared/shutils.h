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

#ifndef _shutils_h_
#define _shutils_h_

#include <string.h>

#include "defaults.h"

#define MAX_FILE_LINE_SIZE	2048
#define ARRAY_SIZE(x)		((unsigned)(sizeof(x) / sizeof((x)[0])))

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
extern char * file2str(const char *path, size_t chunk_size);

/* 
 * Waits for a file descriptor to become available for reading or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
extern int waitfor(int fd, int timeout);

extern void time_zone_x_mapping();

extern int doSystem(const char *fmt, ...);

extern unsigned long get_swap_size(void);
extern unsigned int get_mtd_size(const char *mtd);

extern void change_passwd_unix(char *user, char *pass);
extern void recreate_passwd_unix(int force_create);

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
extern int _eval(char *const argv[], char *path, int timeout, pid_t *ppid);

/* 
 * Kills process whose PID is stored in plaintext in pidfile
 * @param	pidfile	PID file
 * @return	0 on success and errno on failure
 */
extern int kill_pidfile(char *pidfile);
extern int kill_pidfile_s(char *pidfile, int sig);

extern int pids(char *appname);
extern int pids_main(char *appname);

/*
 * fread() with automatic retry on syscall interrupt
 * @param	ptr	location to store to
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully read
 */
extern int safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param	ptr	location to read from
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully written
 */
extern int safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * Convert Ethernet address string representation to binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @param	e	binary data
 * @return	TRUE if conversion was successful and FALSE otherwise
 */
extern int ether_atoe(const char *a, unsigned char *e);

/*
 * Convert Ethernet address binary data to string representation
 * @param	e	binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @return	a
 */
extern char * ether_etoa(const unsigned char *e, char *a);
extern char * ether_etoa2(const unsigned char *e, char *a);
extern char * ether_etoa3(const unsigned char *e, char *a);

/*
 * Concatenate two strings together into a caller supplied buffer
 * @param	s1	first string
 * @param	s2	second string
 * @param	buf	buffer large enough to hold both strings
 * @return	buf
 */
static inline char * strcat_r(const char *s1, const char *s2, char *buf)
{
	strcpy(buf, s1);
	strcat(buf, s2);
	return buf;
}

extern char *trim_r(char *str);

extern int get_param_int(char *line, const char *param, int base, int defval);
extern char *get_param_str(char *line, const char *param, int dups);

extern int fput_string(const char *name, const char *value);
extern int fput_int(const char *name, int value);

extern int compare_text_files(const char* file1, const char* file2);

extern void logmessage(char *logheader, char *fmt, ...);

/* Check for a blank character; that is, a space or a tab */
#ifndef isblank
# define isblank(c) ((c) == ' ' || (c) == '\t')
#endif

/* Strip trailing CR/NL from string <s> */
#define chomp(s) ({ \
	char *c = (s) + strlen((s)) - 1; \
	while ((c > (s)) && (*c == '\n' || *c == '\r')) \
		*c-- = '\0'; \
	s; \
})

/* Simple version of _eval() (no timeout and wait for child termination) */
#define eval(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, NULL, 0, NULL); \
})

#define eval_dumb(cmd, args...) ({ \
	char *argv[] = {cmd, ## args, NULL}; \
	_eval(argv, ">/dev/null", 0, NULL); \
})

#define eval2file(path, cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	_eval(argv, path, 0, NULL); \
})

/* Copy each token in wordlist delimited by space into word */
#define foreach(word, wordlist, next) \
	for (next = &wordlist[strspn(wordlist, " ")], \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '); \
	     strlen(word); \
	     next = next ? &next[strspn(next, " ")] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ' '))

/* Return NUL instead of NULL if undefined */
#define safe_getenv(s) (getenv(s) ? : "")

/* Print directly to the console */
//#define dbg(fmt, args...) do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } } while (0)
#define dbg(fmt, args...) do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } else fprintf(stderr, fmt, ## args); } while (0)
#define dbG(fmt, args...) dbg("%s(0x%04x): " fmt , __FUNCTION__ , __LINE__, ## args)
#define cprintf(fmt, args...) //do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } } while (0)

/* Debug print */
#ifdef DEBUG
#define dprintf(fmt, args...) //cprintf("%s: " fmt, __FUNCTION__, ## args)
#else
#define dprintf(fmt, args...) //cprintf(fmt, ## args);
#endif

#endif /* _shutils_h_ */
