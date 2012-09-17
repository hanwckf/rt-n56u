/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <errno.h>

extern const char _string_syserrmsgs[] attribute_hidden;

#ifdef __UCLIBC_HAS_SYS_ERRLIST__

link_warning(_sys_errlist, "sys_nerr and sys_errlist are obsolete and uClibc support for them (in at least some configurations) will probably be unavailable in the near future.")

const char *const sys_errlist[] = {
	[0] =				_string_syserrmsgs + 0,
	[EPERM] =			_string_syserrmsgs + 8,
	[ENOENT] =			_string_syserrmsgs + 32,
	[ESRCH] =			_string_syserrmsgs + 58,
	[EINTR] =			_string_syserrmsgs + 74,
	[EIO] =				_string_syserrmsgs + 98,
	[ENXIO] =			_string_syserrmsgs + 117,
	[E2BIG] =			_string_syserrmsgs + 143,
	[ENOEXEC] =			_string_syserrmsgs + 166,
	[EBADF] =			_string_syserrmsgs + 184,
	[ECHILD] =			_string_syserrmsgs + 204,
	[EAGAIN] =			_string_syserrmsgs + 223,
	[ENOMEM] =			_string_syserrmsgs + 256,
	[EACCES] =			_string_syserrmsgs + 279,
	[EFAULT] =			_string_syserrmsgs + 297,
	[ENOTBLK] =			_string_syserrmsgs + 309,
	[EBUSY] =			_string_syserrmsgs + 331,
	[EEXIST] =			_string_syserrmsgs + 355,
	[EXDEV] =			_string_syserrmsgs + 367,
	[ENODEV] =			_string_syserrmsgs + 393,
	[ENOTDIR] =			_string_syserrmsgs + 408,
	[EISDIR] =			_string_syserrmsgs + 424,
	[EINVAL] =			_string_syserrmsgs + 439,
	[ENFILE] =			_string_syserrmsgs + 456,
	[EMFILE] =			_string_syserrmsgs + 486,
	[ENOTTY] =			_string_syserrmsgs + 506,
	[ETXTBSY] =			_string_syserrmsgs + 537,
	[EFBIG] =			_string_syserrmsgs + 552,
	[ENOSPC] =			_string_syserrmsgs + 567,
	[ESPIPE] =			_string_syserrmsgs + 591,
	[EROFS] =			_string_syserrmsgs + 604,
	[EMLINK] =			_string_syserrmsgs + 626,
	[EPIPE] =			_string_syserrmsgs + 641,
	[EDOM] =			_string_syserrmsgs + 653,
	[ERANGE] =			_string_syserrmsgs + 686,
	[EDEADLK] =			_string_syserrmsgs + 716,
	[ENAMETOOLONG] =		_string_syserrmsgs + 742,
	[ENOLCK] =			_string_syserrmsgs + 761,
	[ENOSYS] =			_string_syserrmsgs + 780,
	[ENOTEMPTY] =			_string_syserrmsgs + 805,
	[ELOOP] =			_string_syserrmsgs + 825,
	/*  	_string_syserrmsgs + 859, */
	[ENOMSG] =			_string_syserrmsgs + 860,
	[EIDRM] =			_string_syserrmsgs + 887,
	[ECHRNG] =			_string_syserrmsgs + 906,
	[EL2NSYNC] =			_string_syserrmsgs + 934,
	[EL3HLT] =			_string_syserrmsgs + 959,
	[EL3RST] =			_string_syserrmsgs + 974,
	[ELNRNG] =			_string_syserrmsgs + 988,
	[EUNATCH] =			_string_syserrmsgs + 1013,
	[ENOCSI] =			_string_syserrmsgs + 1042,
	[EL2HLT] =			_string_syserrmsgs + 1069,
	[EBADE] =			_string_syserrmsgs + 1084,
	[EBADR] =			_string_syserrmsgs + 1101,
	[EXFULL] =			_string_syserrmsgs + 1128,
	[ENOANO] =			_string_syserrmsgs + 1142,
	[EBADRQC] =			_string_syserrmsgs + 1151,
	[EBADSLT] =			_string_syserrmsgs + 1172,
	/*  	_string_syserrmsgs + 1185, */
	[EBFONT] =			_string_syserrmsgs + 1186,
	[ENOSTR] =			_string_syserrmsgs + 1207,
	[ENODATA] =			_string_syserrmsgs + 1227,
	[ETIME] =			_string_syserrmsgs + 1245,
	[ENOSR] =			_string_syserrmsgs + 1259,
	[ENONET] =			_string_syserrmsgs + 1284,
	[ENOPKG] =			_string_syserrmsgs + 1314,
	[EREMOTE] =			_string_syserrmsgs + 1336,
	[ENOLINK] =			_string_syserrmsgs + 1353,
	[EADV] =			_string_syserrmsgs + 1375,
	[ESRMNT] =			_string_syserrmsgs + 1391,
	[ECOMM] =			_string_syserrmsgs + 1405,
	[EPROTO] =			_string_syserrmsgs + 1433,
	[EMULTIHOP] =			_string_syserrmsgs + 1448,
	[EDOTDOT] =			_string_syserrmsgs + 1467,
	[EBADMSG] =			_string_syserrmsgs + 1486,
	[EOVERFLOW] =			_string_syserrmsgs + 1498,
	[ENOTUNIQ] =			_string_syserrmsgs + 1536,
	[EBADFD] =			_string_syserrmsgs + 1563,
	[EREMCHG] =			_string_syserrmsgs + 1592,
	[ELIBACC] =			_string_syserrmsgs + 1615,
	[ELIBBAD] =			_string_syserrmsgs + 1654,
	[ELIBSCN] =			_string_syserrmsgs + 1691,
	[ELIBMAX] =			_string_syserrmsgs + 1723,
	[ELIBEXEC] =			_string_syserrmsgs + 1771,
	[EILSEQ] =			_string_syserrmsgs + 1809,
	[ERESTART] =			_string_syserrmsgs + 1859,
	[ESTRPIPE] =			_string_syserrmsgs + 1903,
	[EUSERS] =			_string_syserrmsgs + 1922,
	[ENOTSOCK] =			_string_syserrmsgs + 1937,
	[EDESTADDRREQ] =		_string_syserrmsgs + 1968,
	[EMSGSIZE] =			_string_syserrmsgs + 1997,
	[EPROTOTYPE] =			_string_syserrmsgs + 2014,
	[ENOPROTOOPT] =			_string_syserrmsgs + 2045,
	[EPROTONOSUPPORT] =		_string_syserrmsgs + 2068,
	[ESOCKTNOSUPPORT] =		_string_syserrmsgs + 2091,
	[EOPNOTSUPP] =			_string_syserrmsgs + 2117,
	[EPFNOSUPPORT] =		_string_syserrmsgs + 2141,
	[EAFNOSUPPORT] =		_string_syserrmsgs + 2171,
	[EADDRINUSE] =			_string_syserrmsgs + 2212,
	[EADDRNOTAVAIL] =		_string_syserrmsgs + 2235,
	[ENETDOWN] =			_string_syserrmsgs + 2267,
	[ENETUNREACH] =			_string_syserrmsgs + 2283,
	[ENETRESET] =			_string_syserrmsgs + 2306,
	[ECONNABORTED] =		_string_syserrmsgs + 2342,
	[ECONNRESET] =			_string_syserrmsgs + 2375,
	[ENOBUFS] =			_string_syserrmsgs + 2400,
	[EISCONN] =			_string_syserrmsgs + 2426,
	[ENOTCONN] =			_string_syserrmsgs + 2466,
	[ESHUTDOWN] =			_string_syserrmsgs + 2502,
	[ETOOMANYREFS] =		_string_syserrmsgs + 2548,
	[ETIMEDOUT] =			_string_syserrmsgs + 2583,
	[ECONNREFUSED] =		_string_syserrmsgs + 2604,
	[EHOSTDOWN] =			_string_syserrmsgs + 2623,
	[EHOSTUNREACH] =		_string_syserrmsgs + 2636,
	[EALREADY] =			_string_syserrmsgs + 2653,
	[EINPROGRESS] =			_string_syserrmsgs + 2683,
	[ESTALE] =			_string_syserrmsgs + 2709,
	[EUCLEAN] =			_string_syserrmsgs + 2731,
	[ENOTNAM] =			_string_syserrmsgs + 2756,
	[ENAVAIL] =			_string_syserrmsgs + 2784,
	[EISNAM] =			_string_syserrmsgs + 2814,
	[EREMOTEIO] =			_string_syserrmsgs + 2835,
	[EDQUOT] =			_string_syserrmsgs + 2852,
	[ENOMEDIUM] =			_string_syserrmsgs + 2872,
	[EMEDIUMTYPE] =			_string_syserrmsgs + 2888,

#if EDEADLOCK != EDEADLK
	[EDEADLOCK] =			_string_syserrmsgs + 2906,
#endif

#if EWOULDBLOCK != EAGAIN
#error EWOULDBLOCK does not equal EAGAIN
#endif

	/* For now, ignore the other arch-specific errors.  glibc only maps EPROCLIM. */

	/* some other mips errors */
#ifdef ECANCELED
#endif
#ifdef EINIT
#endif
#ifdef EREMDEV
#endif

	/* some other sparc errors */
#ifdef EPROCLIM
#endif
#ifdef ERREMOTE
#endif
};

int sys_nerr = sizeof(sys_errlist)/sizeof(sys_errlist[0]);

#endif
