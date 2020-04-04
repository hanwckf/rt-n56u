/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

/* This is pretty much straight from uClibc, but with one important
 * difference.
 *
 * We now initialize the locking flag to user locking instead of
 * auto locking (i.e. FSETLOCKING_BYCALLER vs FSETLOCKING_INTERNAL).
 * This greatly benefits non-threading applications linked to a
 * shared thread-enabled library.  In threading applications, we
 * walk the stdio open file list and reset the locking mode
 * appropriately when the thread subsystem is initialized.
 */

/**********************************************************************/

#ifdef __UCLIBC_HAS_WCHAR__
#define __STDIO_FILE_INIT_WUNGOT		{ 0, 0 },
#else
#define __STDIO_FILE_INIT_WUNGOT
#endif

#ifdef __STDIO_GETC_MACRO
# define __STDIO_FILE_INIT_BUFGETC(x) x,
#else
# define __STDIO_FILE_INIT_BUFGETC(x)
#endif

#ifdef __STDIO_PUTC_MACRO
# define __STDIO_FILE_INIT_BUFPUTC(x) x,
#else
# define __STDIO_FILE_INIT_BUFPUTC(x)
#endif

#ifdef __STDIO_HAS_OPENLIST
#define __STDIO_FILE_INIT_NEXT(next)	(next),
#else
#define __STDIO_FILE_INIT_NEXT(next)
#endif

#ifdef __STDIO_BUFFERS
#define __STDIO_FILE_INIT_BUFFERS(buf,bufsize) \
	(buf), (buf)+(bufsize), (buf), (buf),
#else
#define __STDIO_FILE_INIT_BUFFERS(buf,bufsize)
#endif

#ifdef __STDIO_MBSTATE
#define __STDIO_FILE_INIT_MBSTATE \
	{ 0, 0 },
#else
#define __STDIO_FILE_INIT_MBSTATE
#endif

#ifdef __UCLIBC_HAS_XLOCALE__
#define __STDIO_FILE_INIT_UNUSED \
	NULL,
#else
#define __STDIO_FILE_INIT_UNUSED
#endif

#ifdef __UCLIBC_HAS_THREADS__
#ifdef __USE_STDIO_FUTEXES__
#define __STDIO_FILE_INIT_THREADSAFE \
	2, _LIBC_LOCK_RECURSIVE_INITIALIZER,
#else
#define __STDIO_FILE_INIT_THREADSAFE \
	2, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP,
#endif
#else
#define __STDIO_FILE_INIT_THREADSAFE
#endif

#define __STDIO_INIT_FILE_STRUCT(stream, flags, filedes, next, buf, bufsize) \
	{ (flags), \
	{ 0, 0 }, /* ungot[2] (no wchar) or ungot_width[2] (wchar)*/ \
	(filedes), \
	__STDIO_FILE_INIT_BUFFERS(buf,bufsize) \
	__STDIO_FILE_INIT_BUFGETC((buf)) \
	__STDIO_FILE_INIT_BUFPUTC((buf)) \
	__STDIO_FILE_INIT_NEXT(next) \
	__STDIO_FILE_INIT_WUNGOT \
	__STDIO_FILE_INIT_MBSTATE \
	__STDIO_FILE_INIT_UNUSED \
	__STDIO_FILE_INIT_THREADSAFE \
} /* TODO: builtin buf */

/**********************************************************************/
/* First we need the standard files. */

#ifdef __STDIO_BUFFERS
static unsigned char _fixed_buffers[2 * BUFSIZ];
#endif

static FILE _stdio_streams[] = {
	__STDIO_INIT_FILE_STRUCT(_stdio_streams[0], \
							 __FLAG_LBF|__FLAG_READONLY, \
							 0, \
							 _stdio_streams + 1, \
							 _fixed_buffers, \
							 BUFSIZ ),
	__STDIO_INIT_FILE_STRUCT(_stdio_streams[1], \
							 __FLAG_LBF|__FLAG_WRITEONLY, \
							 1, \
							 _stdio_streams + 2, \
							 _fixed_buffers + BUFSIZ, \
							 BUFSIZ ),
	__STDIO_INIT_FILE_STRUCT(_stdio_streams[2], \
							 __FLAG_NBF|__FLAG_WRITEONLY, \
							 2, \
							 NULL, \
							 NULL, \
							 0 )
};

FILE *stdin  = _stdio_streams;
FILE *stdout = _stdio_streams + 1;
FILE *stderr = _stdio_streams + 2;

#ifdef __STDIO_GETC_MACRO
FILE *__stdin = _stdio_streams;		 /* For getchar() macro. */
#endif
#ifdef __STDIO_PUTC_MACRO
FILE *__stdout = _stdio_streams + 1; /* For putchar() macro. */
/* FILE *__stderr = _stdio_streams + 2; */
#endif

/**********************************************************************/
#ifdef __STDIO_HAS_OPENLIST

/* In certain configurations, we need to keep a list of open files.
 * 1) buffering enabled - We need to initialize the buffering mode
 *       (full or line buffering) of stdin and stdout.  We also
 *       need to flush all write buffers prior to normal termination.
 * 2) custom streams - Even if we aren't buffering in the library
 *       itself, we need to fclose() all custom streams when terminating
 *       so that any special cleanup is done.
 * 3) threads enabled - We need to be able to reset the locking mode
 *       of all open streams when the threading system is initialized.
 */

FILE *_stdio_openlist = _stdio_streams;

# ifdef __UCLIBC_HAS_THREADS__
__UCLIBC_IO_MUTEX_INIT(_stdio_openlist_add_lock);
#  ifdef __STDIO_BUFFERS
__UCLIBC_IO_MUTEX_INIT(_stdio_openlist_del_lock);
volatile int _stdio_openlist_use_count = 0;
int _stdio_openlist_del_count = 0;
#  endif
# endif
#endif
/**********************************************************************/
#ifdef __UCLIBC_HAS_THREADS__

/* 2 if threading not initialized and 0 otherwise; */
int _stdio_user_locking = 2;

#ifndef __USE_STDIO_FUTEXES__
void attribute_hidden __stdio_init_mutex(__UCLIBC_MUTEX_TYPE *m)
{
	const __UCLIBC_MUTEX_STATIC(__stdio_mutex_initializer,
		PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);

	memcpy(m, &__stdio_mutex_initializer, sizeof(__stdio_mutex_initializer));
}
#endif

#endif
/**********************************************************************/

/* We assume here that we are the only remaining thread. */
void _stdio_term(void)
{
#if defined(__STDIO_BUFFERS) || defined(__UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__)
	register FILE *ptr;

#ifdef __UCLIBC_HAS_THREADS__
	/* First, make sure the open file list is unlocked.  If it was
	 * locked, then I suppose there is a chance that a pointer in the
	 * chain might be corrupt due to a partial store.
	 */
	STDIO_INIT_MUTEX(_stdio_openlist_add_lock);
#ifdef __STDIO_BUFFERS
	STDIO_INIT_MUTEX(_stdio_openlist_del_lock);
#endif

	/* Next we need to worry about the streams themselves.  If a stream
	 * is currently locked, then it may be in an invalid state.  So we
	 * 'disable' it in case a custom stream is stacked on top of it.
	 * Then we reinitialize the locks.
	 */
	for (ptr = _stdio_openlist ; ptr ; ptr = ptr->__nextopen ) {
		if (__STDIO_ALWAYS_THREADTRYLOCK_CANCEL_UNSAFE(ptr)) {
			/* The stream is already locked, so we don't want to touch it.
			 * However, if we have custom streams, we can't just close it
			 * or leave it locked since a custom stream may be stacked
			 * on top of it.  So we do unlock it, while also disabling it.
			 */
			ptr->__modeflags = (__FLAG_READONLY|__FLAG_WRITEONLY);
			__STDIO_STREAM_DISABLE_GETC(ptr);
			__STDIO_STREAM_DISABLE_PUTC(ptr);
			__STDIO_STREAM_INIT_BUFREAD_BUFPOS(ptr);
		}

		ptr->__user_locking = 1; /* Set locking mode to "by caller". */
		STDIO_INIT_MUTEX(ptr->__lock); /* Shouldn't be necessary, but... */
	}
#endif

	/* Finally, flush all writing streams and shut down all custom streams.
	 * NOTE: We assume that any stacking by custom streams is done on top
	 *       of streams previously allocated, and hence further down the
	 *       list.  Otherwise we have no way of knowing the order in which
	 *       to shut them down.
	 *       Remember that freopen() counts as a new allocation here, even
	 *       though the stream is reused.  That's because it moves the
	 *       stream to the head of the list.
	 */
	for (ptr = _stdio_openlist ; ptr ; ptr = ptr->__nextopen ) {
#ifdef __STDIO_BUFFERS
		/* Write any pending buffered chars. */
		if (__STDIO_STREAM_IS_WRITING(ptr)) {
			__STDIO_COMMIT_WRITE_BUFFER(ptr);
		}
#endif
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
		/* Actually close all custom streams to perform any special cleanup. */
		if (__STDIO_STREAM_IS_CUSTOM(ptr)) {
			__CLOSE(ptr);
		}
#endif
	}

#endif
}

#if defined __STDIO_BUFFERS || !defined __UCLIBC__
void _stdio_init(void)
{
#ifdef __STDIO_BUFFERS
	int old_errno = errno;
	/* stdin and stdout uses line buffering when connected to a tty. */
	if (!isatty(0))
		_stdio_streams[0].__modeflags ^= __FLAG_LBF;
	if (!isatty(1))
		_stdio_streams[1].__modeflags ^= __FLAG_LBF;
	__set_errno(old_errno);
#endif
#ifndef __UCLIBC__
	/* _stdio_term is done automatically when exiting if stdio is used.
	 * See misc/internals/__uClibc_main.c and and stdlib/atexit.c. */
	atexit(_stdio_term);
#endif
}
#endif

/**********************************************************************/

#if !(__MASK_READING & __FLAG_UNGOT)
#error Assumption violated about __MASK_READING and __FLAG_UNGOT
#endif

#ifndef NDEBUG

void attribute_hidden _stdio_validate_FILE(const FILE *stream)
{
#ifdef __UCLIBC_HAS_THREADS__
	assert(((unsigned int)(stream->__user_locking)) <= 2);
#endif

#warning Define a constant for minimum possible valid __filedes?
	assert(stream->__filedes >= -4);

	if (stream->__filedes < 0) {
/* 		assert((stream->__filedes == -1) || __STDIO_STREAM_IS_FBF(stream)); */

		assert(!__STDIO_STREAM_IS_FAKE_VSNPRINTF(stream)
			   || __STDIO_STREAM_IS_NARROW(stream));
		assert(!__STDIO_STREAM_IS_FAKE_VSSCANF(stream)
			   || __STDIO_STREAM_IS_NARROW(stream));
#ifdef __STDIO_STREAM_IS_FAKE_VSWPRINTF
		assert(!__STDIO_STREAM_IS_FAKE_VSWPRINTF(stream)
			   || __STDIO_STREAM_IS_WIDE(stream));
#endif
#ifdef __STDIO_STREAM_IS_FAKE_VSWSCANF
		assert(!__STDIO_STREAM_IS_FAKE_VSWSCANF(stream)
			   || __STDIO_STREAM_IS_WIDE(stream));
#endif
	}

	/* Can not be both narrow and wide oriented at the same time. */
	assert(!(__STDIO_STREAM_IS_NARROW(stream)
			 && __STDIO_STREAM_IS_WIDE(stream)));


	/* The following impossible case is used to disable a stream. */
	if ((stream->__modeflags & (__FLAG_READONLY|__FLAG_WRITEONLY))
		== (__FLAG_READONLY|__FLAG_WRITEONLY)
		) {
		assert(stream->__modeflags == (__FLAG_READONLY|__FLAG_WRITEONLY));
		assert(stream->__filedes == -1);
#ifdef __STDIO_BUFFERS
		assert(stream->__bufpos == stream->__bufstart);
		assert(stream->__bufread == stream->__bufstart);
# ifdef __UCLIBC_HAS_STDIO_PUTC_MACRO__
		assert(stream->__bufputc_u == stream->__bufstart);
# endif
# ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
		assert(stream->__bufgetc_u == stream->__bufstart);
# endif
#endif
	}

	if (__STDIO_STREAM_IS_READONLY(stream)) {
/* 		assert(!__STDIO_STREAM_IS_WRITEONLY(stream)); */
		assert(!__STDIO_STREAM_IS_WRITING(stream));
		if (stream->__modeflags & __FLAG_UNGOT) {
			assert(((unsigned)(stream->__ungot[1])) <= 1);
			assert(!__FEOF_UNLOCKED(stream));
		}
	}

	if (__STDIO_STREAM_IS_WRITEONLY(stream)) {
/* 		assert(!__STDIO_STREAM_IS_READONLY(stream)); */
		assert(!__STDIO_STREAM_IS_READING(stream));
		assert(!(stream->__modeflags & __FLAG_UNGOT));
	}

	if (__STDIO_STREAM_IS_NBF(stream)) {
		/* We require that all non buffered streams have no buffer. */
		assert(!__STDIO_STREAM_BUFFER_SIZE(stream));
	}

	assert((stream->__modeflags & __MASK_BUFMODE) <= __FLAG_NBF);

#ifdef __STDIO_BUFFERS
	/* Ensure __bufstart <= __bufpos <= __bufend. */
	assert(stream->__bufpos >= stream->__bufstart);
	assert(stream->__bufpos <= stream->__bufend);
	/* Ensure __bufstart <= __bufread <= __bufend. */
	assert(stream->__bufread >= stream->__bufstart);
	assert(stream->__bufread <= stream->__bufend);
#endif

	/* If EOF, then we must have no buffered readable or ungots. */
	if (__FEOF_UNLOCKED(stream)) {
#ifdef __STDIO_BUFFERS
		assert(stream->__bufpos == stream->__bufread);
#endif
		assert(!(stream->__modeflags & __FLAG_UNGOT));
	}


	if (!__STDIO_STREAM_IS_WRITING(stream)) {
#ifdef __STDIO_BUFFERS
		/* If not writing, then putc macro must be disabled. */
# ifdef __UCLIBC_HAS_STDIO_PUTC_MACRO__
		assert(stream->__bufputc_u == stream->__bufstart);
# endif
#endif
	}

	if (!__STDIO_STREAM_IS_READING(stream)) {
		/* If not reading, then can not have ungots. */
		assert(!(stream->__modeflags & __FLAG_UNGOT));
#ifdef __STDIO_BUFFERS
		/* Ensure __bufread == __bufstart. */
		assert(stream->__bufread == stream->__bufstart);
		/* If not reading, then getc macro must be disabled. */
# ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
		assert(stream->__bufgetc_u == stream->__bufstart);
# endif
#endif
	}

	if (__STDIO_STREAM_IS_READING(stream)) {
		assert(!__STDIO_STREAM_IS_WRITING(stream));
#ifdef __STDIO_BUFFERS
		/* Ensure __bufpos <= __bufread. */
		assert(stream->__bufpos <= stream->__bufread);

		/* Ensure __bufgetc_u is valid. */
# ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
		assert(stream->__bufgetc_u >= stream->__bufstart);
		assert(stream->__bufgetc_u <= stream->__bufread);
# endif

#endif
	}

	if (__STDIO_STREAM_IS_WRITING(stream)) {
		assert(!__STDIO_STREAM_IS_READING(stream));
#ifdef __STDIO_BUFFERS
# ifdef __UCLIBC_HAS_STDIO_PUTC_MACRO__
		assert(stream->__bufputc_u >= stream->__bufstart);
		assert(stream->__bufputc_u <= stream->__bufend);
# endif
#endif
	}

	/* If have an ungotten char, then getc (and putc) must be disabled. */
	/* Also, wide streams must have the getc/putc macros disabled. */
	if ((stream->__modeflags & __FLAG_UNGOT)
		|| __STDIO_STREAM_IS_WIDE(stream)
		) {
#ifdef __STDIO_BUFFERS
# ifdef __UCLIBC_HAS_STDIO_PUTC_MACRO__
		assert(stream->__bufputc_u == stream->__bufstart);
# endif
# ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
		assert(stream->__bufgetc_u == stream->__bufstart);
# endif
#endif
	}

	/* TODO -- filepos?  ungot_width?  filedes?  nextopen? */
}

#endif
