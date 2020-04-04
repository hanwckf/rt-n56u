/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


/*
 * Cases:
 *  fopen64  : filename != NULL, stream == NULL, filedes == -2
 *  fopen    : filename != NULL, stream == NULL, filedes == -1
 *  freopen  : filename != NULL, stream != NULL, filedes == -1
 *  fdopen   : filename == NULL, stream == NULL, filedes valid
 *
 *  fsfopen  : filename != NULL, stream != NULL, filedes == -1
 */

#if (O_ACCMODE != 3) || (O_RDONLY != 0) || (O_WRONLY != 1) || (O_RDWR != 2)
#error Assumption violated - mode constants
#endif

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

/* Internal function -- reentrant (locks open file list) */

FILE attribute_hidden *_stdio_fopen(intptr_t fname_or_mode,
				   register const char * __restrict mode,
				   register FILE * __restrict stream, int filedes)
{
	__mode_t open_mode;
	int i;

	/* Parse the specified mode. */
	open_mode = O_RDONLY;
	if (*mode != 'r') {			/* Not read... */
		open_mode = (O_WRONLY | O_CREAT | O_TRUNC);
		if (*mode != 'w') {		/* Not write (create or truncate)... */
			open_mode = (O_WRONLY | O_CREAT | O_APPEND);
			if (*mode != 'a') {	/* Not write (create or append)... */
			DO_EINVAL:
				__set_errno(EINVAL); /* So illegal mode. */
				if (stream) {
				FREE_STREAM:
					assert(!(stream->__modeflags & __FLAG_FREEBUF));
					__STDIO_STREAM_FREE_FILE(stream);
				}
				return NULL;
			}
		}
	}

	if ((mode[1] == 'b')) {		/* Binary mode (NO-OP currently). */
		++mode;
	}

	if (mode[1] == '+') {			/* Read and Write. */
		++mode;
		open_mode |= (O_RDONLY | O_WRONLY);
		open_mode += (O_RDWR - (O_RDONLY | O_WRONLY));
	}

#if defined(__UCLIBC_HAS_FOPEN_EXCLUSIVE_MODE__) || defined(__UCLIBC_HAS_FOPEN_LARGEFILE_MODE__) || \
    defined(__UCLIBC_HAS_FOPEN_CLOSEEXEC_MODE__)

	while (*++mode) {
# ifdef __UCLIBC_HAS_FOPEN_EXCLUSIVE_MODE__
		if (*mode == 'x') {	   /* Open exclusive (a glibc extension). */
			open_mode |= O_EXCL;
			continue;
		}
# endif
# ifdef __UCLIBC_HAS_FOPEN_LARGEFILE_MODE__
		if (*mode == 'F') {		/* Open as large file (uClibc extension). */
			open_mode |= O_LARGEFILE;
			continue;
		}
# endif
# ifdef __UCLIBC_HAS_FOPEN_CLOSEEXEC_MODE__
		if (*mode == 'e') {		/* Close on exec (a glibc extension). */
			open_mode |= O_CLOEXEC;
			continue;
		}
# endif
 	}

#endif

	if (!stream) {		  /* Need to allocate a FILE (not freopen). */
		if ((stream = malloc(sizeof(FILE))) == NULL) {
			return stream;
		}
		stream->__modeflags = __FLAG_FREEFILE;
#ifdef __STDIO_BUFFERS
		stream->__bufstart = NULL; /* We allocate a buffer below. */
#endif
#ifdef __UCLIBC_HAS_THREADS__
		/* We only initialize the mutex in the non-freopen case. */
		/* stream->__user_locking = _stdio_user_locking; */
		STDIO_INIT_MUTEX(stream->__lock);
#endif
	}

	if (filedes >= 0) {			/* Handle fdopen trickery. */
		stream->__filedes = filedes;
		/* NOTE: it is insufficient to just check R/W/RW agreement.
		 * We must also check largefile compatibility if applicable.
		 * Also, if append mode is desired for fdopen but O_APPEND isn't
		 * currently set, then set it as recommended by SUSv3.  However,
		 * if append mode is not specified for fdopen but O_APPEND is set,
		 * leave it set (glibc compat). */
		i = (open_mode & (O_ACCMODE|O_LARGEFILE)) + 1;

		/* NOTE: fopencookie needs changing if the basic check changes! */
		if ((i & ((int)fname_or_mode + 1)) != i) /* Basic agreement? */
			goto DO_EINVAL;
		if ((open_mode & ~(__mode_t)fname_or_mode) & O_APPEND) {
			if (fcntl(filedes, F_SETFL, O_APPEND))	/* Need O_APPEND. */
				goto DO_EINVAL;
		}
		/* For later... to reflect largefile setting in stream flags. */
		__STDIO_WHEN_LFS( open_mode |= (((__mode_t) fname_or_mode)
										& O_LARGEFILE) );
	} else {
		__STDIO_WHEN_LFS( if (filedes < -1) open_mode |= O_LARGEFILE );
		if ((stream->__filedes = open(((const char *) fname_or_mode),
									  open_mode, 0666)) < 0) {
			goto FREE_STREAM;
		}
	}

	stream->__modeflags &= __FLAG_FREEFILE;
/* 	stream->__modeflags &= ~(__FLAG_READONLY|__FLAG_WRITEONLY); */

	stream->__modeflags |=		/* Note: Makes assumptions about flag vals! */
#if (O_APPEND != __FLAG_APPEND) || ((O_LARGEFILE != __FLAG_LARGEFILE) && (O_LARGEFILE != 0))
# if (O_APPEND != __FLAG_APPEND)
		((open_mode & O_APPEND) ? __FLAG_APPEND : 0) |
# else
		(open_mode & O_APPEND) |
# endif
# if (O_LARGEFILE != __FLAG_LARGEFILE) && (O_LARGEFILE != 0)
		((open_mode & O_LARGEFILE) ? __FLAG_LARGEFILE : 0) |
# else
		(open_mode & O_LARGEFILE) |
# endif
#else
		(open_mode & (O_APPEND|O_LARGEFILE)) | /* i386 linux and elks */
#endif
		((((open_mode & O_ACCMODE) + 1) ^ 0x03) * __FLAG_WRITEONLY);

#ifdef __STDIO_BUFFERS
	if (stream->__filedes != INT_MAX) {
		/* NB: fopencookie uses bogus filedes == INT_MAX,
		 * avoiding isatty() in that case.
		 */
		i = errno; /* preserve errno against isatty call. */
		if (isatty(stream->__filedes))
			stream->__modeflags |= __FLAG_LBF;
		__set_errno(i);
	}

	if (!stream->__bufstart) {
		if ((stream->__bufstart = malloc(BUFSIZ)) != NULL) {
			stream->__bufend = stream->__bufstart + BUFSIZ;
			stream->__modeflags |= __FLAG_FREEBUF;
		} else {
# if __STDIO_BUILTIN_BUF_SIZE > 0
			stream->__bufstart = stream->__builtinbuf;
			stream->__bufend = stream->__builtinbuf + sizeof(stream->__builtinbuf);
# else  /* __STDIO_BUILTIN_BUF_SIZE > 0 */
			stream->__bufend = stream->__bufstart;
# endif /* __STDIO_BUILTIN_BUF_SIZE > 0 */
		}
	}

	__STDIO_STREAM_DISABLE_GETC(stream);
	__STDIO_STREAM_DISABLE_PUTC(stream);
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(stream);
#endif

#ifdef __UCLIBC_HAS_WCHAR__
	stream->__ungot_width[0] = 0;
#endif
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(stream->__state));
#endif

#ifdef __UCLIBC_HAS_THREADS__
	/* Even in the freopen case, we reset the user locking flag. */
	stream->__user_locking = _stdio_user_locking;
	/* STDIO_INIT_MUTEX(stream->__lock); */
#endif

#ifdef __STDIO_HAS_OPENLIST
#if defined(__UCLIBC_HAS_THREADS__) && defined(__STDIO_BUFFERS)
	if (!(stream->__modeflags & __FLAG_FREEFILE))
	{
		/* An freopen call so the file was never removed from the list. */
	}
	else
#endif
	{
		/* We have to lock the del mutex in case another thread wants to fclose()
		 * the last file. */
		__STDIO_THREADLOCK_OPENLIST_DEL;
		__STDIO_THREADLOCK_OPENLIST_ADD;
		stream->__nextopen = _stdio_openlist; /* New files are inserted at */
		_stdio_openlist = stream;			  /*   the head of the list. */
		__STDIO_THREADUNLOCK_OPENLIST_ADD;
		__STDIO_THREADUNLOCK_OPENLIST_DEL;
	}
#endif

	__STDIO_STREAM_VALIDATE(stream);

	return stream;
}
