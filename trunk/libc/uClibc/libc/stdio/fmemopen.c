/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifndef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
#error no custom streams!
#endif

typedef struct {
	size_t pos;
	size_t len;
	size_t eof;
	int dynbuf;
	unsigned char *buf;
	FILE *fp;
} __fmo_cookie;

#define COOKIE ((__fmo_cookie *) cookie)

static ssize_t fmo_read(register void *cookie, char *buf, size_t bufsize)
{
	size_t count = COOKIE->len - COOKIE->pos;

	/* Note: 0 < bufsize < SSIZE_MAX because of _stdio_READ. */
	if (!count) {				/* EOF! */
		return 0;
	}

	if (bufsize > count) {
		bufsize = count;
	}

	memcpy(buf, COOKIE->buf + COOKIE->pos, bufsize);
	COOKIE->pos += bufsize;

	return bufsize;
}

static ssize_t fmo_write(register void *cookie, const char *buf, size_t bufsize)
{
	size_t count;

	/* Note: bufsize < SSIZE_MAX because of _stdio_WRITE. */

	/* If appending, need to seek to end of file!!!! */
	if (COOKIE->fp->__modeflags & __FLAG_APPEND) {
		COOKIE->pos = COOKIE->eof;
	}

	count = COOKIE->len - COOKIE->pos;

	if (bufsize > count) {
		bufsize = count;
		if (count == 0) {		/* We're at the end of the buffer... */
			__set_errno(EFBIG);
			return -1;
		}
	}

	memcpy(COOKIE->buf + COOKIE->pos, buf, bufsize);
	COOKIE->pos += bufsize;

	if (COOKIE->pos > COOKIE->eof) {
		COOKIE->eof = COOKIE->pos;
		if (bufsize < count) {	/* New eof and still room in buffer? */
			*(COOKIE->buf + COOKIE->pos) = 0;
		}
	}

	return bufsize;
}

/* glibc doesn't allow seeking, but it has in-buffer seeks... we don't. */
static int fmo_seek(register void *cookie, __offmax_t *pos, int whence)
{
	__offmax_t p = *pos;

	/* Note: fseek already checks that whence is legal, so don't check here
	 * unless debugging. */
	assert(((unsigned int) whence) <= 2);

	if (whence != SEEK_SET) {
		p += (whence == SEEK_CUR) ? COOKIE->pos : /* SEEK_END */ COOKIE->eof;
	}

	/* Note: glibc only allows seeking in the buffer.  We'll actually restrict
	 * to the data. */
	/* Check for offset < 0, offset > eof, or offset overflow... */
	if (((uintmax_t) p) > COOKIE->eof) {
		return -1;
	}

	COOKIE->pos = *pos = p;
	return 0;
}

static int fmo_close(register void *cookie)
{
	if (COOKIE->dynbuf) {
		free(COOKIE->buf);
	}
	free(cookie);
	return 0;
}

#undef COOKIE

static const cookie_io_functions_t _fmo_io_funcs = {
	fmo_read, fmo_write, fmo_seek, fmo_close
};

/* TODO: If we have buffers enabled, it might be worthwile to add a pointer
 * to the FILE in the cookie and have read, write, and seek operate directly
 * on the buffer itself (ie replace the FILE buffer with the cookie buffer
 * and update FILE bufstart, etc. whenever we seek). */

FILE *fmemopen(void *s, size_t len, const char *modes)
{
	FILE *fp;
	register __fmo_cookie *cookie;
	size_t i;

	if ((cookie = malloc(sizeof(__fmo_cookie))) != NULL) {
		cookie->len = len;
		cookie->eof = cookie->pos = 0; /* pos and eof adjusted below. */
		cookie->dynbuf = 0;
		if (((cookie->buf = s) == NULL) && (len > 0)) {
			if ((cookie->buf = malloc(len)) == NULL) {
				goto EXIT_cookie;
			}
			cookie->dynbuf = 1;
			*cookie->buf = 0;	/* If we're appending, treat as empty file. */
		}
		
#ifndef __BCC__
		fp = fopencookie(cookie, modes, _fmo_io_funcs);
#else
		fp = fopencookie(cookie, modes, &_fmo_io_funcs);
#endif
		/* Note: We don't need to worry about locking fp in the thread case
		 * as the only possible access would be a close or flush with
		 * nothing currently in the FILE's write buffer. */

		if (fp != NULL) {
			cookie->fp = fp;
			if (fp->__modeflags & __FLAG_READONLY) {
				cookie->eof = len;
			}
			if ((fp->__modeflags & __FLAG_APPEND) && (len > 0)) {
				for (i = 0 ; i < len ; i++) {
					if (cookie->buf[i] == 0) {
						break;
					}
				}
				cookie->eof = cookie->pos = i; /* Adjust eof and pos. */
			}

			__STDIO_STREAM_VALIDATE(fp);

			return fp;
		}
	}

	if (!s) {
		free(cookie->buf);
	}
 EXIT_cookie:
	free(cookie);

	return NULL;
}
