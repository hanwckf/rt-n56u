/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* get rid of REDIRECT */
#define strerror_r __hide_strerror_r

#include <features.h>
#include <errno.h>
#include <string.h>
#include "_syserrmsg.h"

#undef strerror_r

#ifdef __UCLIBC_HAS_ERRNO_MESSAGES__

extern const char _string_syserrmsgs[] attribute_hidden;

#if defined(__alpha__) || defined(__mips__) || defined(__sparc__)

static const unsigned char estridx[] = {
	0,							/* success is always 0 */
	EPERM,
	ENOENT,
	ESRCH,
	EINTR,
	EIO,
	ENXIO,
	E2BIG,
	ENOEXEC,
	EBADF,
	ECHILD,
	EAGAIN,
	ENOMEM,
	EACCES,
	EFAULT,
	ENOTBLK,
	EBUSY,
	EEXIST,
	EXDEV,
	ENODEV,
	ENOTDIR,
	EISDIR,
	EINVAL,
	ENFILE,
	EMFILE,
	ENOTTY,
	ETXTBSY,
	EFBIG,
	ENOSPC,
	ESPIPE,
	EROFS,
	EMLINK,
	EPIPE,
	EDOM,
	ERANGE,
	EDEADLK,
	ENAMETOOLONG,
	ENOLCK,
	ENOSYS,
	ENOTEMPTY,
	ELOOP,
	0,
	ENOMSG,
	EIDRM,
	ECHRNG,
	EL2NSYNC,
	EL3HLT,
	EL3RST,
	ELNRNG,
	EUNATCH,
	ENOCSI,
	EL2HLT,
	EBADE,
	EBADR,
	EXFULL,
	ENOANO,
	EBADRQC,
	EBADSLT,
	0,
	EBFONT,
	ENOSTR,
	ENODATA,
	ETIME,
	ENOSR,
	ENONET,
	ENOPKG,
	EREMOTE,
	ENOLINK,
	EADV,
	ESRMNT,
	ECOMM,
	EPROTO,
	EMULTIHOP,
	EDOTDOT,
	EBADMSG,
	EOVERFLOW,
	ENOTUNIQ,
	EBADFD,
	EREMCHG,
	ELIBACC,
	ELIBBAD,
	ELIBSCN,
	ELIBMAX,
	ELIBEXEC,
	EILSEQ,
	ERESTART,
	ESTRPIPE,
	EUSERS,
	ENOTSOCK,
	EDESTADDRREQ,
	EMSGSIZE,
	EPROTOTYPE,
	ENOPROTOOPT,
	EPROTONOSUPPORT,
	ESOCKTNOSUPPORT,
	EOPNOTSUPP,
	EPFNOSUPPORT,
	EAFNOSUPPORT,
	EADDRINUSE,
	EADDRNOTAVAIL,
	ENETDOWN,
	ENETUNREACH,
	ENETRESET,
	ECONNABORTED,
	ECONNRESET,
	ENOBUFS,
	EISCONN,
	ENOTCONN,
	ESHUTDOWN,
	ETOOMANYREFS,
	ETIMEDOUT,
	ECONNREFUSED,
	EHOSTDOWN,
	EHOSTUNREACH,
	EALREADY,
	EINPROGRESS,
	ESTALE,
	EUCLEAN,
	ENOTNAM,
	ENAVAIL,
	EISNAM,
	EREMOTEIO,
#if EDQUOT > 200			/* mips has an outrageous value for this... */
	0,
#else
	EDQUOT,
#endif
	ENOMEDIUM,
	EMEDIUMTYPE,
#if EDEADLOCK != EDEADLK
	EDEADLOCK,
#endif
};

#endif

int __xpg_strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
    register char *s;
    int i, retval;
    char buf[_STRERROR_BUFSIZE];
    static const char unknown[] = {
		'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 'e', 'r', 'r', 'o', 'r', ' '
    };

    retval = EINVAL;


#ifdef __UCLIBC_HAS_ERRNO_MESSAGES__

#if defined(__alpha__) || defined(__mips__) || defined(__sparc__)
	/* Need to translate errno to string index. */
	for (i = 0 ; i < sizeof(estridx)/sizeof(estridx[0]) ; i++) {
		if (estridx[i] == errnum) {
			goto GOT_ESTRIDX;
		}
	}
	i = INT_MAX;	/* Failed, but may need to check mips special case. */
#if EDQUOT > 200	/* Deal with large EDQUOT value on mips */
	if (errnum == EDQUOT)
		i = 122;
#endif
 GOT_ESTRIDX:
#else
	/* No errno to string index translation needed. */
	i = errnum;
#endif

    if (((unsigned int) i) < _SYS_NERR) {
		/* Trade time for space.  This function should rarely be called
		 * so rather than keeping an array of pointers for the different
		 * messages, just run through the buffer until we find the
		 * correct string. */
		for (s = (char *) _string_syserrmsgs ; i ; ++s) {
			if (!*s) {
				--i;
			}
		}
		if (*s) {		/* Make sure we have an actual message. */
			retval = 0;
			goto GOT_MESG;
		}
    }

#endif /* __UCLIBC_HAS_ERRNO_MESSAGES__ */

    s = _int10tostr(buf+sizeof(buf)-1, errnum) - sizeof(unknown);
    memcpy(s, unknown, sizeof(unknown));

 GOT_MESG:
    if (!strerrbuf) {		/* SUSv3  */
		buflen = 0;
    }
    i = strlen(s) + 1;
    if (i > buflen) {
		i = buflen;
		retval = ERANGE;
    }

    if (i) {
		memcpy(strerrbuf, s, i);
		strerrbuf[i-1] = 0;	/* In case buf was too small. */
    }

    if (retval) {
		__set_errno(retval);
    }

    return retval;
}

#else  /* __UCLIBC_HAS_ERRNO_MESSAGES__ */

int __xpg_strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
    register char *s;
    int i, retval;
    char buf[_STRERROR_BUFSIZE];
    static const char unknown[] = {
		'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 'e', 'r', 'r', 'o', 'r', ' '
    };

    s = _int10tostr(buf+sizeof(buf)-1, errnum) - sizeof(unknown);
    memcpy(s, unknown, sizeof(unknown));

    if (!strerrbuf) {		/* SUSv3  */
		buflen = 0;
    }

    retval = EINVAL;

	i = buf + sizeof(buf) - s;

    if (i > buflen) {
		i = buflen;
		retval = ERANGE;
    }

    if (i) {
		memcpy(strerrbuf, s, i);
		strerrbuf[i-1] = 0;	/* In case buf was too small. */
    }

	__set_errno(retval);

    return retval;
}

#endif /* __UCLIBC_HAS_ERRNO_MESSAGES__ */
libc_hidden_def(__xpg_strerror_r)
#if defined __USE_XOPEN2K && !defined __USE_GNU
strong_alias(__xpg_strerror_r,strerror_r)
#endif
