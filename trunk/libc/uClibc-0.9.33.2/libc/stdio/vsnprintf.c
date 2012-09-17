/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <stdarg.h>


#ifdef __UCLIBC_MJN3_ONLY__
#warning WISHLIST: Implement vsnprintf for non-buffered and no custom stream case.
#endif /* __UCLIBC_MJN3_ONLY__ */

#ifdef __STDIO_BUFFERS
/* NB: we can still have __USE_OLD_VFPRINTF__ defined in this case! */

int vsnprintf(char *__restrict buf, size_t size,
			  const char * __restrict format, va_list arg)
{
	FILE f;
	int rv;

/* 	__STDIO_STREAM_RESET_GCS(&f); */
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
	f.__cookie = &(f.__filedes);
	f.__gcs.read = NULL;
	f.__gcs.write = NULL;
	f.__gcs.seek = NULL;
	f.__gcs.close = NULL;
#endif

	f.__filedes = __STDIO_STREAM_FAKE_VSNPRINTF_FILEDES;
	f.__modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __UCLIBC_HAS_WCHAR__
	f.__ungot_width[0] = 0;
#endif /* __UCLIBC_HAS_WCHAR__ */
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.__state));
#endif /* __STDIO_MBSTATE */

#if (defined(__STDIO_BUFFERS) || defined(__USE_OLD_VFPRINTF__)) && defined(__UCLIBC_HAS_THREADS__)
	f.__user_locking = 1;		/* Set user locking. */
	STDIO_INIT_MUTEX(f.__lock);
#endif
	f.__nextopen = NULL;

	if (size > SIZE_MAX - (size_t) buf) {
		size = SIZE_MAX - (size_t) buf;
	}

/* TODO: this comment seems to be wrong */
	/* Set these last since __bufputc initialization depends on
	 * __user_locking and only gets set if user locking is on. */
	f.__bufstart = (unsigned char *) buf;
	f.__bufend = (unsigned char *) buf + size;
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(&f);
	__STDIO_STREAM_DISABLE_GETC(&f);
	__STDIO_STREAM_ENABLE_PUTC(&f);

#ifdef __USE_OLD_VFPRINTF__
	rv = vfprintf(&f, format, arg);
#else
	rv = _vfprintf_internal(&f, format, arg);
#endif
	if (size) {
		if (f.__bufpos == f.__bufend) {
			--f.__bufpos;
		}
		*f.__bufpos = 0;
	}
	return rv;
}
libc_hidden_def(vsnprintf)

#elif defined(__USE_OLD_VFPRINTF__)

typedef struct {
	FILE f;
	unsigned char *bufend;		/* pointer to 1 past end of buffer */
	unsigned char *bufpos;
} __FILE_vsnprintf;

int vsnprintf(char *__restrict buf, size_t size,
			  const char * __restrict format, va_list arg)
{
	__FILE_vsnprintf f;
	int rv;

	f.bufpos = buf;

	if (size > SIZE_MAX - (size_t) buf) {
		size = SIZE_MAX - (size_t) buf;
	}
	f.bufend = buf + size;

/* 	__STDIO_STREAM_RESET_GCS(&f.f); */
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
	f.f.__cookie = &(f.f.__filedes);
	f.f.__gcs.read = NULL;
	f.f.__gcs.write = NULL;
	f.f.__gcs.seek = NULL;
	f.f.__gcs.close = NULL;
#endif

	f.f.__filedes = __STDIO_STREAM_FAKE_VSNPRINTF_FILEDES_NB;
	f.f.__modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __UCLIBC_HAS_WCHAR__
	f.f.__ungot_width[0] = 0;
#endif /* __UCLIBC_HAS_WCHAR__ */
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.f.__state));
#endif /* __STDIO_MBSTATE */

#ifdef __UCLIBC_HAS_THREADS__
	f.f.__user_locking = 1;		/* Set user locking. */
	STDIO_INIT_MUTEX(f.f.__lock);
#endif
	f.f.__nextopen = NULL;

	rv = vfprintf((FILE *) &f, format, arg);
	if (size) {
		if (f.bufpos == f.bufend) {
			--f.bufpos;
		}
		*f.bufpos = 0;
	}
	return rv;
}
libc_hidden_def(vsnprintf)

#elif defined(__UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__)

typedef struct {
	size_t pos;
	size_t len;
	unsigned char *buf;
	FILE *fp;
} __snpf_cookie;

#define COOKIE ((__snpf_cookie *) cookie)

static ssize_t snpf_write(register void *cookie, const char *buf,
						  size_t bufsize)
{
	size_t count;
	register char *p;

	/* Note: bufsize < SSIZE_MAX because of _stdio_WRITE. */

	if (COOKIE->len > COOKIE->pos) {
		count = COOKIE->len - COOKIE->pos - 1; /* Leave space for nul. */
		if (count > bufsize) {
			count = bufsize;
		}

		p = COOKIE->buf + COOKIE->pos;
		while (count) {
			*p++ = *buf++;
			--count;
		}
		*p = 0;
	}

	COOKIE->pos += bufsize;

	return bufsize;
}

#undef COOKIE

int vsnprintf(char *__restrict buf, size_t size,
			  const char * __restrict format, va_list arg)
{
	FILE f;
	__snpf_cookie cookie;
	int rv;

	cookie.buf = buf;
	cookie.len = size;
	cookie.pos = 0;
	cookie.fp = &f;

	f.__cookie = &cookie;
	f.__gcs.write = snpf_write;
	f.__gcs.read = NULL;
	f.__gcs.seek = NULL;
	f.__gcs.close = NULL;

	f.__filedes = -1;			/* For debugging. */
	f.__modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __UCLIBC_HAS_WCHAR__
	f.__ungot_width[0] = 0;
#endif /* __UCLIBC_HAS_WCHAR__ */
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.__state));
#endif /* __STDIO_MBSTATE */

	f.__nextopen = NULL;

	rv = _vfprintf_internal(&f, format, arg);

	return rv;
}
libc_hidden_def(vsnprintf)

#else
#warning Skipping vsnprintf since no buffering, no custom streams, and not old vfprintf!
#ifdef __STDIO_HAS_VSNPRINTF
#error WHOA! __STDIO_HAS_VSNPRINTF is defined!
#endif
#endif
