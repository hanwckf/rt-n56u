/*
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYS_USER_H
#define _SYS_USER_H	1

/* The whole purpose of this file is for GDB and GDB only.  Don't read
   too much into it.  Don't use it for anything other than GDB unless
   you know what you are doing.  */


/* Actually apps like strace also expect a struct user, so it's better to
 * have a dummy implementation
 */

struct user {
	int dummy;
};

#endif  /* _SYS_USER_H */
