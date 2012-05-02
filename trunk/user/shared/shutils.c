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
/*
 * Shell-like utility functions
 *
 * Copyright 2005, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <ctype.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>


/* Linux specific headers */
#ifdef linux
#include <error.h>
#include <termios.h>
#include <sys/time.h>
#include <net/ethernet.h>
#endif /* linux */

// 0912 add
#include <assert.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <dirent.h>
#include <time.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>
/* Must be before netinet/ip.h. Found on FreeBSD, Solaris */
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <syslog.h>
#include <utime.h>

#define PRIVATE_HANDS_OFF_syscall_retval syscall_retval
#define PRIVATE_HANDS_OFF_exit_status exit_status
#include "sysutil.h"
#include "utility.h"
#include "tunables.h"
#include "str.h"
#include "oneprocess.h"
#include "twoprocess.h"

#include <sys/sysinfo.h>

/* Activate 64-bit file support on Linux/32bit plus others */
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1

/* @@@ Ugly code starts here! */
char *ugly_global_username = NULL;
char *ugly_global_password = NULL;

enum access_kind
{
  AC_READ, AC_CREATE, AC_DELETE
};

void
die(const char *str)
{
	printf("%s\n", str);
	exit(-1);
}

void 
bug(const char *str)
{
	printf("%s\n", str);
}


#define MAX_NVPARSE 255

#ifdef linux
/*
 * Reads file and returns contents
 * @param	fd	file descriptor
 * @return	contents of file or NULL if an error occurred
 */
char *
fd2str(int fd)
{
	char *buf = NULL;
	size_t count = 0, n;

	do {
		buf = realloc(buf, count + 512);
		n = read(fd, buf + count, 512);
		if (n < 0) {
			free(buf);
			buf = NULL;
		}
		count += n;
	} while (n == 512);

	close(fd);
	if (buf)
		buf[count] = '\0';
	return buf;
}

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
char *
file2str(const char *path)
{
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1) {
		perror(path);
		return NULL;
	}

	return fd2str(fd);
}

/* 
 * Waits for a file descriptor to change status or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
int
waitfor (int fd, int timeout)
{
	fd_set rfds;
	struct timeval tv = { timeout, 0 };

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	return select(fd + 1, &rfds, NULL, NULL, (timeout > 0) ? &tv : NULL);
}

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */

int
simple_eval(char *const argv[], char *path, int timeout, int *ppid)
{
	pid_t pid;
	int status;
	int fd;
	int flags;
	int sig;
	int i;

	switch (pid = fork()) {
	case -1:	// error
		perror("fork");
		return errno;
	case 0:		// child
		// Reset signal handlers set for parent process
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		// Clean up
		//ioctl(0, TIOCNOTTY, 0);
		//close(STDIN_FILENO);
		setsid();

		for (i=3; i<256; ++i)    // close un-needed fd
			close(i);

		// Redirect stdout to <path>
		if (path) {
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2)) {
				// append to <path>
				flags |= O_APPEND;
				path += 2;
			} else if (!strncmp(path, ">", 1)) {
				// overwrite <path>
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else {
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
		}

		// execute command 
		dprintf("%s\n", argv[0]);
		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
		alarm(timeout);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default:	// parent
		if (ppid) {
			*ppid = pid;
			return 0;
		} else {
			printf("parent wait\n");	// tmp test
			if (waitpid(pid, &status, 0) == -1) {
				perror("waitpid");
				return errno;
			}
			printf("parent after wait\n");	// tmp test
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

int
s_eval2(char *const argv[], char *path, int timeout, int *ppid)
{
	pid_t pid;
	int status;
	int sig;
	int i;

	switch (pid = fork()) {
	case -1:	// error
		perror("fork");
		return errno;
	case 0:	 // child
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		for (i=3; i<256; ++i)    // close un-needed fd
			close(i);

		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default:	// parent
		if (ppid) {
			*ppid = pid;
			return 0;
		} else {
			printf("parent wait\n");	// tmp test
			if (waitpid(pid, &status, 0) == -1) {
				perror("waitpid");
				return errno;
			}
			printf("parent after wait\n");	// tmp test
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}


void time_zone_x_mapping()
{
	char tmpstr[32];
	char *ptr;

	/* pre mapping */
	if (nvram_match("time_zone", "KST-9KDT"))
		nvram_set("time_zone", "UCT-9_1");

	strcpy(tmpstr, nvram_safe_get("time_zone"));
	/* replace . with : */
	if ((ptr=strchr(tmpstr, '.'))!=NULL) *ptr = ':';
	/* remove *_? */
	if ((ptr=strchr(tmpstr, '_'))!=NULL) *ptr = 0x0;

	/* special mapping */
	if (nvram_match("time_zone", "JST"))
		nvram_set("time_zone_x", "UCT-9");
	else if (nvram_match("time_zone", "TST-10TDT"))
		nvram_set("time_zone_x", "UCT-10");
	else if (nvram_match("time_zone", "CST-9:30CDT"))
		nvram_set("time_zone_x", "UCT-9:30");
	else
		nvram_set("time_zone_x", tmpstr);

	doSystem("echo %s > /etc/TZ", nvram_safe_get("time_zone_x"));
}

void change_passwd_unix(char *user, char *pass)
{
	char cmdbuf[128];
	
	if (!user || !*user)
		user = "admin";
	
	if (!pass)
		pass = "admin";
	
	sprintf(cmdbuf, "chpasswd.sh %s %s", user, pass);
	system(cmdbuf);
}

void recreate_passwd_unix(int force_create)
{
	FILE *fp1, *fp2;
	int i, uid, sh_num;
	char tmpusernm[32];
	char *rootnm, *usernm;
	
	rootnm = nvram_safe_get("http_username");
	if (!*rootnm) rootnm = "admin";
	
	fp1 = fopen("/etc/passwd.orig", "w");
	fp2 = fopen("/etc/group.orig", "w");
	if (fp1 && fp2)
	{
		fprintf(fp1, "%s::0:0::/home/admin:/bin/sh\n", rootnm);
		fprintf(fp1, "nobody:x:99:99::/media:/bin/false\n");
		fprintf(fp2, "%s:x:0:%s\n", rootnm, rootnm);
		fprintf(fp2, "nobody:x:99:\n");
		
		sh_num = atoi(nvram_safe_get("acc_num"));
		if (sh_num > 100) sh_num = 100;
		memset(tmpusernm, 0, sizeof(tmpusernm));
		uid=1000;
		for (i=0; i<sh_num; i++)
		{
			sprintf(tmpusernm, "acc_username%d", i);
			usernm = nvram_safe_get(tmpusernm);
			if (*usernm && strcmp(usernm, "root") && strcmp(usernm, "nobody") && strcmp(usernm, rootnm))
			{
				fprintf(fp1, "%s:x:%d:%d:::\n", usernm, uid, uid);
				fprintf(fp2, "%s:x:%d:\n", usernm, uid);
				uid++;
			}
		}
	}
	
	if (fp1) fclose(fp1);
	if (fp2) fclose(fp2);
	
	if (force_create || system("md5sum -cs /tmp/hashes/passwd_md5") != 0)
	{
		system("md5sum /etc/passwd.orig /etc/group.orig > /tmp/hashes/passwd_md5");
		system("cp -f /etc/passwd.orig /etc/passwd");
		system("cp -f /etc/group.orig /etc/group");
		change_passwd_unix(rootnm, nvram_safe_get("http_passwd"));
	}
	
	unlink("/etc/passwd.orig");
	unlink("/etc/group.orig");
}


int
_eval(char *const argv[], char *path, int timeout, int *ppid)
{
	sigset_t set;
	pid_t pid, ret;
	int status;
	int fd;
	int flags;
	int sig, i;

	switch (pid = fork()) {
	case -1:	/* error */
		perror("fork");
		return errno;
	case 0:	 /* child */
		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		/* Unblock signals if called from signal handler */
		sigemptyset(&set);
		sigprocmask(SIG_SETMASK, &set, NULL);

		/* Clean up */
		for (i=3; i<256; i++)    // close un-needed fd
			close(i);
		ioctl(0, TIOCNOTTY, 0);	// detach from current process
		setsid();
		
		/* Redirect stdout to <path> */
		if (path) {
			flags = O_WRONLY | O_CREAT;
			if (!strncmp(path, ">>", 2)) {
				/* append to <path> */
				flags |= O_APPEND;
				path += 2;
			} else if (!strncmp(path, ">", 1)) {
				/* overwrite <path> */
				flags |= O_TRUNC;
				path += 1;
			}
			if ((fd = open(path, flags, 0644)) < 0)
				perror(path);
			else {
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
		}
		
		/* execute command */
		dprintf("%s\n", argv[0]);
		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
		alarm(timeout);
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(errno);
	default:	/* parent */
		if (ppid) {
			*ppid = pid;
			return 0;
		} else {
			do
				ret = waitpid(pid, &status, 0);
			while ((ret == -1) && (errno == EINTR));
			
			if (ret != pid) {
				perror("waitpid");
				return errno;
			}
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @return	stdout of executed command or NULL if an error occurred
 */
char *
_backtick(char *const argv[])
{
	int filedes[2];
	pid_t pid;
	int status;
	char *buf = NULL;

	/* create pipe */
	if (pipe(filedes) == -1) {
		perror(argv[0]);
		return NULL;
	}

	switch (pid = fork()) {
	case -1:	/* error */
		return NULL;
	case 0:		/* child */
		close(filedes[0]);	/* close read end of pipe */
		dup2(filedes[1], 1);	/* redirect stdout to write end of pipe */
		close(filedes[1]);	/* close write end of pipe */
		execvp(argv[0], argv);
		exit(errno);
		break;
	default:	/* parent */
		close(filedes[1]);	/* close write end of pipe */
		buf = fd2str(filedes[0]);
		waitpid(pid, &status, 0);
		break;
	}
	
	return buf;
}

/* 
 * Kills process whose PID is stored in plaintext in pidfile
 * @param	pidfile	PID file
 * @return	0 on success and errno on failure
 */
int
kill_pidfile(char *pidfile)
{
	FILE *fp = fopen(pidfile, "r");
	char buf[256];

	if (fp && fgets(buf, sizeof(buf), fp)) {
		pid_t pid = strtoul(buf, NULL, 0);
		fclose(fp);
		return kill(pid, SIGTERM);
  	} else
		return errno;
}

/*
 * fread() with automatic retry on syscall interrupt
 * @param	ptr	location to store to
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully read
 */
int
safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fread((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param	ptr	location to read from
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully written
 */
int
safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t ret = 0;

	do {
		clearerr(stream);
		ret += fwrite((char *)ptr + (ret * size), size, nmemb - ret, stream);
	} while (ret < nmemb && ferror(stream) && errno == EINTR);

	return ret;
}

#endif /* linux */
/*
 * Convert Ethernet address string representation to binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @param	e	binary data
 * @return	TRUE if conversion was successful and FALSE otherwise
 */
int
ether_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, ETHER_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}

/*
 * Convert Ethernet address binary data to string representation
 * @param	e	binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @return	a
 */
char *
ether_etoa(const unsigned char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}

/*
 * Convert Ethernet address binary data to string representation
 * @param       e       binary data
 * @param       a       string in xxxxxxxxxxxx notation
 * @return      a
 */
char *
ether_etoa2(const unsigned char *e, char *a)
{
	char *c = a;
	int i;
													       
	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		c += sprintf(c, "%02x", e[i] & 0xff);
	}
	return a;
}

/*
 * Get the ip configuration index if it exists given the 
 * eth name.
 * 
 * @param	wl_ifname 	pointer to eth interface name
 * @return	index or -1 if not found
 */ 
int 
get_ipconfig_index(char *eth_ifname)
{
	char varname[64];
	char varval[64];
	char *ptr;
	char wl_ifname[64];
	int index;
	
	/* Bail if we get a NULL or empty string */
	
	if (!eth_ifname) return -1;
	if (!*eth_ifname) return -1;
	
	/* Look up wl name from the eth name */
	if ( osifname_to_nvifname( eth_ifname, wl_ifname, sizeof(wl_ifname)) != 0 )
		return -1;
		
	snprintf(varname,sizeof(varname),"%s_ipconfig_index",wl_ifname);

	ptr = nvram_get(varname);
	
	if (ptr) {
	/* Check ipconfig_index pointer to see if it is still pointing 
	   the correct lan config block */
		if (*ptr) {
			int index;
			char *ifname;
			char buf[64];
			index = atoi(ptr);
			
			snprintf(buf,sizeof(buf),"lan%d_ifname",index);
			
			ifname = nvram_get(buf);
			
			if (ifname) {
				if  (!(strcmp(ifname,wl_ifname)))
					return index;
			}
			nvram_unset(varname);
		}
	}
	
	/* The index pointer may not have been configured if the
	 * user enters the variables manually. Do a brute force search 
	 *  of the lanXX_ifname variables
	 */
	for (index=0 ; index < MAX_NVPARSE; index++) {
		snprintf(varname,sizeof(varname),"lan%d_ifname",index);
		if ( nvram_match(varname,wl_ifname)) {
			/* if a match is found set up a corresponding index pointer for wlXX */
			snprintf(varname,sizeof(varname),"%s_ipconfig_index",wl_ifname);
			snprintf(varval,sizeof(varval),"%d",index);
			nvram_set(varname,varval);
			nvram_commit();
			return index;
		};
	}
	return -1;
}
	
/*
 * Set the ip configuration index given the eth name
 * Updates both wlXX_ipconfig_index and lanYY_ifname.
 * 
 * @param	eth_ifname 	pointer to eth interface name
 * @return	0 if successful -1 if not.
 */
int 
set_ipconfig_index(char *eth_ifname,int index)
{
	char varname[255];
	char varval[16];
	char wl_ifname[64];
	
	/* Bail if we get a NULL or empty string */
	
	if (!eth_ifname) return -1;
	if (!*eth_ifname) return -1;
	
	if (index >= MAX_NVPARSE) return -1;
	
	/* Look up wl name from the eth name only if the name contains
	   eth
	*/

	if ( osifname_to_nvifname( eth_ifname, wl_ifname, sizeof(wl_ifname)) != 0 )
		return -1;
	
	snprintf(varname,sizeof(varname),"%s_ipconfig_index",wl_ifname);
	snprintf(varval,sizeof(varval),"%d",index);
	nvram_set(varname,varval);
	
	snprintf(varname,sizeof(varname),"lan%d_ifname",index);
	nvram_set(varname,wl_ifname);
	
	nvram_commit();
	
	return 0;
}
	
/*
 * Get interfaces belonging to a specific bridge.
 * 
 * @param	bridge_name 	pointer to bridge interface name
 * @return	list of interfaces belonging to the bridge or NULL
 *	      if not found/empty
 */	
char *
get_bridged_interfaces(char *bridge_name)
{
	static char interfaces[255] ;	
	char *ifnames=NULL;
	char bridge[64];
	
	if (!bridge_name) return NULL;
	
	memset(interfaces,0,sizeof(interfaces));
	snprintf(bridge,sizeof(bridge),"%s_ifnames",bridge_name);
	
	ifnames=nvram_get(bridge);
	
	if (ifnames) 
		strncpy(interfaces,ifnames,sizeof(interfaces));
	else
		return NULL;
		
	return  interfaces;
		
}

//#if 0
/*
 * Search a string backwards for a set of characters
 * This is the reverse version of strspn()
 *
 * @param	s	string to search backwards
 * @param	accept	set of chars for which to search
 * @return	number of characters in the trailing segment of s 
 *		which consist only of characters from accept.
 */
static size_t
sh_strrspn(const char *s, const char *accept)
{
	const char *p;
	size_t accept_len = strlen(accept);
	int i;

	
	if (s[0] == '\0')
		return 0;
	
	p = s + (strlen(s) - 1);
	i = 0;
	
	do {
		if (memchr(accept, *p, accept_len) == NULL)
			break;
		p--; i++;
	} while (p != s);

	return i;
}

/*
 * Parse the unit and subunit from an interface string such as wlXX or wlXX.YY
 *
 * @param	ifname	interface string to parse
 * @param	unit	pointer to return the unit number, may pass NULL
 * @param	subunit	pointer to return the subunit number, may pass NULL
 * @return	Returns 0 if the string ends with digits or digits.digits, -1 otherwise.
 *		If ifname ends in digits.digits, then unit and subuint are set
 *		to the first and second values respectively. If ifname ends 
 *		in just digits, unit is set to the value, and subunit is set
 *		to -1. On error both unit and subunit are -1. NULL may be passed
 *		for unit and/or subuint to ignore the value.
 */
int
get_ifname_unit(const char* ifname, int *unit, int *subunit)
{
	const char digits[] = "0123456789";
	char str[64];
	char *p;
	size_t ifname_len = strlen(ifname);
	size_t len;
	long val;

	if (unit)
		*unit = -1;
	if (subunit)
		*subunit = -1;
	
	if (ifname_len + 1 > sizeof(str)) 
		return -1;

	strcpy(str, ifname);

	/* find the trailing digit chars */
	len = sh_strrspn(str, digits);
	
	/* fail if there were no trailing digits */
	if (len == 0)
		return -1;

	/* point to the beginning of the last integer and convert */
	p = str + (ifname_len - len);
	val = strtol(p, NULL, 10);
	
	/* if we are at the beginning of the string, or the previous
	 * character is not a '.', then we have the unit number and
	 * we are done parsing
	 */
	if (p == str || p[-1] != '.') {
		if (unit)
			*unit = val;
		return 0;
	} else {
		if (subunit)
			*subunit = val;
	}

	/* chop off the '.NNN' and get the unit number */
	p--;
	p[0] = '\0';
	
	/* find the trailing digit chars */
	len = sh_strrspn(str, digits);

	/* fail if there were no trailing digits */
	if (len == 0)
		return -1;

	/* point to the beginning of the last integer and convert */
	p = p - len;
	val = strtol(p, NULL, 10);

	/* save the unit number */
	if (unit)
		*unit = val;

	return 0;
}

/** 
		remove_from_list
		Remove the specified word from the list.

		@param name word to be removed from the list
		@param list Space separated list to modify
		@param listsize Max size the list can occupy

		@return	error code
*/
int remove_from_list( char *name, char *list, int listsize )
{
	int listlen = 0;
	int namelen = 0;
	char *occurrence = list;
	
	if ( !list || !name || (listsize <= 0) )
		return EINVAL;

	listlen = strlen( list );
	namelen = strlen( name );

	while ( occurrence != NULL && ( occurrence - list < listlen ))
	{
		occurrence = strstr( occurrence, name );
		
		if ( !occurrence )
			return EINVAL;
		
		/* last item in list? */
		if ( occurrence[namelen] == 0 )
		{
			/* only item in list? */
			if ( occurrence != list )
				occurrence--;
			occurrence[0] = 0;
			break;
		}
		else if ( occurrence[namelen] == ' ' )
		{
			strncpy( occurrence, &occurrence[namelen+1/*space*/], 
							 strlen( &occurrence[namelen+1/*space*/]) +1/*terminate*/ );
			break;
		}
		occurrence++;
	}

	return 0;
}

/** 
		add_to_list
		Add the specified interface(string) to the list as long as
		it will fit in the space left in the list.

		NOTE: If item is already in list, it won't be added again.

		@param name Name of interface to be added to the list
		@param list List to modify
		@param listsize Max size the list can occupy

		@return	error code
*/
int add_to_list( char *name, char *list, int listsize )
{
	int listlen = 0;
	int namelen = 0;
	char *temp = NULL;

	if ( !list || !name || (listsize <= 0) )
		return EINVAL;

	listlen = strlen( list );
	namelen = strlen( name );

	/* is the item already in the list? */
	temp = strstr( list, name );
	if ( temp && ( temp[namelen] == ' ' || temp[namelen] == 0))
		return 0;


	if ( listsize <= listlen + namelen + 1/*space*/ )
		return EMSGSIZE;

	/* add a space if the list isn't empty */
	if ( list[0] != 0 )
	{
		list[listlen++] = 0x20;
	}
	
	listlen += (int)strncpy( &list[listlen], name, namelen+1/*terminate*/ );

	return 0;
}

#define WLMBSS_DEV_NAME	"wlmbss"
#define WL_DEV_NAME "wl"
#define WDS_DEV_NAME	"wds"

#if defined(linux)
/**
	 nvifname_to_osifname() 
	 The intent here is to provide a conversion between the OS interface name
	 and the device name that we keep in NVRAM.  
	 This should eventually be placed in a Linux specific file with other 
	 OS abstraction functions.

	 @param nvifname pointer to ifname to be converted
	 @param osifname_buf storage for the converted osifname
	 @param osifname_buf_len length of storage for osifname_buf
*/
int 
nvifname_to_osifname( const char *nvifname, char *osifname_buf, 
											int osifname_buf_len )
{
	char varname[NVRAM_MAX_PARAM_LEN];
	char *ptr;
	
	memset( osifname_buf, 0, osifname_buf_len );

	/* Bail if we get a NULL or empty string */
	if ((!nvifname) || (!*nvifname) || (!osifname_buf)) {
		return -1;
	}
	
	if (strstr(nvifname,"eth") || strstr(nvifname,".")) {
		strncpy( osifname_buf, nvifname, osifname_buf_len);
		return 0;
	}	
	
	snprintf( varname, sizeof(varname), "%s_ifname", nvifname);
	ptr = nvram_get(varname);
	if (ptr) {
		/* Bail if the string is empty */
		if (!*ptr) return -1;
		strncpy( osifname_buf, ptr, osifname_buf_len);
		return 0;
	}

	return -1;
}


/* osifname_to_nvifname()

   Convert the OS interface name to the name we use internally(NVRAM,GUI,etc.)

	 This is the Linux version of this function 

	 @param osifname pointer to osifname to be converted
	 @param nvifname_buf storage for the converted ifname
	 @param nvifname_buf_len length of storage for nvifname_buf
*/

int
osifname_to_nvifname( const char *osifname, char *nvifname_buf, int nvifname_buf_len )
{
	char varname[NVRAM_MAX_PARAM_LEN];
	int pri,sec;
	
	/* Bail if we get a NULL or empty string */
	
	if ((!osifname) || (!*osifname) || (!nvifname_buf))
	{
		return -1;
	}
	
	memset(nvifname_buf,nvifname_buf_len,0);
	
	if (strstr(osifname,"wl")) {
		strncpy(nvifname_buf,osifname,nvifname_buf_len);
		return 0;
	}	
	
	/* look for interface name on the primary interfaces first */
	for (pri=0;pri < MAX_NVPARSE; pri++) {
		snprintf(varname,sizeof(varname),
					"wl%d_ifname",pri);
		if (nvram_match(varname,(char *)osifname)) {
					snprintf(nvifname_buf,nvifname_buf_len,"wl%d",pri);
					return 0;
				}
	}
	
	/* look for interface name on the multi-instance interfaces */
	for (pri=0;pri < MAX_NVPARSE; pri++)
		for (sec=0;sec< MAX_NVPARSE; sec++) {
			snprintf(varname,sizeof(varname),
					"wl%d.%d_ifname",pri,sec);
			if (nvram_match(varname,(char *)osifname)) {
					snprintf(nvifname_buf,nvifname_buf_len,"wl%d.%d",pri,sec);
					return 0;
				}
		}
	
	return -1;

}
//#endif

#endif

#define	T(x)				__TXT(x)
#define	__TXT(s)			L ## s

#ifndef B_L
#define B_L				T(__FILE__), __LINE__
#define B_ARGS_DEC		char *file, int line
#define B_ARGS			file, line
#endif /* B_L */

#define bfree(B_ARGS, p) free(p)
#define balloc(B_ARGS, num) malloc(num)
#define brealloc(B_ARGS, p, num) realloc(p, num)

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif /* min */

#define STR_REALLOC		0x1				/* Reallocate the buffer as required */
#define STR_INC			64				/* Growth increment */

typedef struct {
	char		*s;						/* Pointer to buffer */
	int		size;						/* Current buffer size */
	int		max;						/* Maximum buffer size */
	int		count;						/* Buffer count */
	int		flags;						/* Allocation flags */
} strbuf_t;

/*
 *	Sprintf formatting flags
 */
enum flag {
	flag_none = 0,
	flag_minus = 1,
	flag_plus = 2,
	flag_space = 4,
	flag_hash = 8,
	flag_zero = 16,
	flag_short = 32,
	flag_long = 64
};

/******************************************************************************/
/*
 *	Return the length of a string limited by a given length
 */

static int strnlen(char *s, unsigned int n)
{
	unsigned int 	len;

	len = strlen(s);
	return min(len, n);
}

/******************************************************************************/
/*
 *	Add a character to a string buffer
 */

static void put_char(strbuf_t *buf, char c)
{
	if (buf->count >= (buf->size - 1)) {
		if (! (buf->flags & STR_REALLOC)) {
			return;
		}
		buf->size += STR_INC;
		if (buf->size > buf->max && buf->size > STR_INC) {
/*
 *			Caller should increase the size of the calling buffer
 */
			buf->size -= STR_INC;
			return;
		}
		if (buf->s == NULL) {
			buf->s = balloc(B_L, buf->size * sizeof(char));
		} else {
			buf->s = brealloc(B_L, buf->s, buf->size * sizeof(char));
		}
	}
	buf->s[buf->count] = c;
	if (c != '\0') {
		++buf->count;
	}
}

/******************************************************************************/
/*
 *	Add a string to a string buffer
 */

static void put_string(strbuf_t *buf, char *s, int len, int width,
		int prec, enum flag f)
{
	int		i;

	if (len < 0) { 
		len = strnlen(s, prec >= 0 ? prec : ULONG_MAX); 
	} else if (prec >= 0 && prec < len) { 
		len = prec; 
	}
	if (width > len && !(f & flag_minus)) {
		for (i = len; i < width; ++i) { 
			put_char(buf, ' '); 
		}
	}
	for (i = 0; i < len; ++i) { 
		put_char(buf, s[i]); 
	}
	if (width > len && f & flag_minus) {
		for (i = len; i < width; ++i) { 
			put_char(buf, ' '); 
		}
	}
}

/******************************************************************************/
/*
 *	Add a long to a string buffer
 */

static void put_ulong(strbuf_t *buf, unsigned long int value, int base,
		int upper, char *prefix, int width, int prec, enum flag f)
{
	unsigned long	x, x2;
	int				len, zeros, i;

	for (len = 1, x = 1; x < ULONG_MAX / base; ++len, x = x2) {
		x2 = x * base;
		if (x2 > value) { 
			break; 
		}
	}
	zeros = (prec > len) ? prec - len : 0;
	width -= zeros + len;
	if (prefix != NULL) { 
		width -= strnlen(prefix, ULONG_MAX); 
	}
	if (!(f & flag_minus)) {
		if (f & flag_zero) {
			for (i = 0; i < width; ++i) { 
				put_char(buf, '0'); 
			}
		} else {
			for (i = 0; i < width; ++i) { 
				put_char(buf, ' '); 
			}
		}
	}
	if (prefix != NULL) { 
		put_string(buf, prefix, -1, 0, -1, flag_none); 
	}
	for (i = 0; i < zeros; ++i) { 
		put_char(buf, '0'); 
	}
	for ( ; x > 0; x /= base) {
		int digit = (value / x) % base;
		put_char(buf, (char) ((digit < 10 ? '0' : (upper ? 'A' : 'a') - 10) +
			digit));
	}
	if (f & flag_minus) {
		for (i = 0; i < width; ++i) { 
			put_char(buf, ' '); 
		}
	}
}

/******************************************************************************/
/*
 *	Dynamic sprintf implementation. Supports dynamic buffer allocation.
 *	This function can be called multiple times to grow an existing allocated
 *	buffer. In this case, msize is set to the size of the previously allocated
 *	buffer. The buffer will be realloced, as required. If msize is set, we
 *	return the size of the allocated buffer for use with the next call. For
 *	the first call, msize can be set to -1.
 */

static int dsnprintf(char **s, int size, char *fmt, va_list arg, int msize)
{
	strbuf_t	buf;
	char		c;

	assert(s);
	assert(fmt);

	memset(&buf, 0, sizeof(buf));
	buf.s = *s;

	if (*s == NULL || msize != 0) {
		buf.max = size;
		buf.flags |= STR_REALLOC;
		if (msize != 0) {
			buf.size = max(msize, 0);
		}
		if (*s != NULL && msize != 0) {
			buf.count = strlen(*s);
		}
	} else {
		buf.size = size;
	}

	while ((c = *fmt++) != '\0') {
		if (c != '%' || (c = *fmt++) == '%') {
			put_char(&buf, c);
		} else {
			enum flag f = flag_none;
			int width = 0;
			int prec = -1;
			for ( ; c != '\0'; c = *fmt++) {
				if (c == '-') { 
					f |= flag_minus; 
				} else if (c == '+') { 
					f |= flag_plus; 
				} else if (c == ' ') { 
					f |= flag_space; 
				} else if (c == '#') { 
					f |= flag_hash; 
				} else if (c == '0') { 
					f |= flag_zero; 
				} else {
					break;
				}
			}
			if (c == '*') {
				width = va_arg(arg, int);
				if (width < 0) {
					f |= flag_minus;
					width = -width;
				}
				c = *fmt++;
			} else {
				for ( ; isdigit((int)c); c = *fmt++) {
					width = width * 10 + (c - '0');
				}
			}
			if (c == '.') {
				f &= ~flag_zero;
				c = *fmt++;
				if (c == '*') {
					prec = va_arg(arg, int);
					c = *fmt++;
				} else {
					for (prec = 0; isdigit((int)c); c = *fmt++) {
						prec = prec * 10 + (c - '0');
					}
				}
			}
			if (c == 'h' || c == 'l') {
				f |= (c == 'h' ? flag_short : flag_long);
				c = *fmt++;
			}
			if (c == 'd' || c == 'i') {
				long int value;
				if (f & flag_short) {
					value = (short int) va_arg(arg, int);
				} else if (f & flag_long) {
					value = va_arg(arg, long int);
				} else {
					value = va_arg(arg, int);
				}
				if (value >= 0) {
					if (f & flag_plus) {
						put_ulong(&buf, value, 10, 0, T("+"), width, prec, f);
					} else if (f & flag_space) {
						put_ulong(&buf, value, 10, 0, T(" "), width, prec, f);
					} else {
						put_ulong(&buf, value, 10, 0, NULL, width, prec, f);
					}
				} else {
					put_ulong(&buf, -value, 10, 0, T("-"), width, prec, f);
				}
			} else if (c == 'o' || c == 'u' || c == 'x' || c == 'X') {
				unsigned long int value;
				if (f & flag_short) {
					value = (unsigned short int) va_arg(arg, unsigned int);
				} else if (f & flag_long) {
					value = va_arg(arg, unsigned long int);
				} else {
					value = va_arg(arg, unsigned int);
				}
				if (c == 'o') {
					if (f & flag_hash && value != 0) {
						put_ulong(&buf, value, 8, 0, T("0"), width, prec, f);
					} else {
						put_ulong(&buf, value, 8, 0, NULL, width, prec, f);
					}
				} else if (c == 'u') {
					put_ulong(&buf, value, 10, 0, NULL, width, prec, f);
				} else {
					if (f & flag_hash && value != 0) {
						if (c == 'x') {
							put_ulong(&buf, value, 16, 0, T("0x"), width, 
								prec, f);
						} else {
							put_ulong(&buf, value, 16, 1, T("0X"), width, 
								prec, f);
						}
					} else {
                  /* 04 Apr 02 BgP -- changed so that %X correctly outputs
                   * uppercase hex digits when requested.
						put_ulong(&buf, value, 16, 0, NULL, width, prec, f);
                   */
						put_ulong(&buf, value, 16, ('X' == c) , NULL, width, prec, f);
					}
				}

			} else if (c == 'c') {
				char value = va_arg(arg, int);
				put_char(&buf, value);

			} else if (c == 's' || c == 'S') {
				char *value = va_arg(arg, char *);
				if (value == NULL) {
					put_string(&buf, T("(null)"), -1, width, prec, f);
				} else if (f & flag_hash) {
					put_string(&buf,
						value + 1, (char) *value, width, prec, f);
				} else {
					put_string(&buf, value, -1, width, prec, f);
				}
			} else if (c == 'p') {
				void *value = va_arg(arg, void *);
				put_ulong(&buf,
					(unsigned long int) value, 16, 0, T("0x"), width, prec, f);
			} else if (c == 'n') {
				if (f & flag_short) {
					short int *value = va_arg(arg, short int *);
					*value = buf.count;
				} else if (f & flag_long) {
					long int *value = va_arg(arg, long int *);
					*value = buf.count;
				} else {
					int *value = va_arg(arg, int *);
					*value = buf.count;
				}
			} else {
				put_char(&buf, c);
			}
		}
	}
	if (buf.s == NULL) {
		put_char(&buf, '\0');
	}

/*
 *	If the user requested a dynamic buffer (*s == NULL), ensure it is returned.
 */
	if (*s == NULL || msize != 0) {
		*s = buf.s;
	}

	if (*s != NULL && size > 0) {
		if (buf.count < size) {
			(*s)[buf.count] = '\0';
		} else {
			(*s)[buf.size - 1] = '\0';
		}
	}

	if (msize != 0) {
		return buf.size;
	}
	return buf.count;
}

/******************************************************************************/
/*
 *	sprintf and vsprintf are bad, ok. You can easily clobber memory. Use
 *	fmtAlloc and fmtValloc instead! These functions do _not_ support floating
 *	point, like %e, %f, %g...
 */

int fmtAlloc(char **s, int n, char *fmt, ...)
{
	va_list	ap;
	int		result;

	assert(s);
	assert(fmt);

	*s = NULL;
	va_start(ap, fmt);
	result = dsnprintf(s, n, fmt, ap, 0);
	va_end(ap);
	return result;
}

/******************************************************************************/
/*
 *	A vsprintf replacement.
 */

int fmtValloc(char **s, int n, char *fmt, va_list arg)
{
	assert(s);
	assert(fmt);

	*s = NULL;
	return dsnprintf(s, n, fmt, arg, 0);
}

#define CMD_BUFSIZE	256

/*
 *  * description: parse va and do system
 *   */
int doSystem(char *fmt, ...)
{
	va_list		vargs;
	char		*cmd = NULL;
	int		rc = 0;

	va_start(vargs, fmt);
	if (fmtValloc(&cmd, CMD_BUFSIZE, fmt, vargs) >= CMD_BUFSIZE) {
		fprintf(stderr, "doSystem: lost data, buffer overflow\n");
	}
	va_end(vargs);

	if (cmd) {
		if (!strncmp(cmd, "iwpriv", 6))
			fprintf(stderr, "[doSystem] %s\n", cmd);

		rc = system(cmd);
		bfree(B_L, cmd);
	}

	return rc;
}

//int
unsigned long
swap_check()
{
	struct sysinfo info;

	sysinfo(&info);
	if (info.totalswap > 0)
	{
		return info.totalswap;
	}
	else
	{
		return 0;
	}
}

/* 
 *  * Kills process whose PID is stored in plaintext in pidfile
 *   * @param       pidfile PID file, signal
 *    * @return      0 on success and errno on failure
 *     */
int
kill_pidfile_s(char *pidfile, int sig)
{
	FILE *fp = fopen(pidfile, "r");
	char buf[256];

	if (fp && fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		pid_t pid = strtoul(buf, NULL, 0);
		return kill(pid, sig);
	} else
		return errno;
}

