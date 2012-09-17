/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


#ifdef __DO_UNLOCKED

#ifdef __UCLIBC_MJN3_ONLY__
#warning WISHLIST: Add option to test for undefined behavior of fflush.
#endif /* __UCLIBC_MJN3_ONLY__ */

/* Even if the stream is set to user-locking, we still need to lock
 * when all (lbf) writing streams are flushed. */

#define __MY_STDIO_THREADLOCK(__stream)					\
        __UCLIBC_IO_MUTEX_CONDITIONAL_LOCK((__stream)->__lock,		\
	(_stdio_user_locking != 2))

#define __MY_STDIO_THREADUNLOCK(__stream)				\
        __UCLIBC_IO_MUTEX_CONDITIONAL_UNLOCK((__stream)->__lock,		\
	(_stdio_user_locking != 2))

#if defined(__UCLIBC_HAS_THREADS__) && defined(__STDIO_BUFFERS)
void attribute_hidden _stdio_openlist_dec_use(void)
{
	__STDIO_THREADLOCK_OPENLIST_DEL;
	if ((_stdio_openlist_use_count == 1) && (_stdio_openlist_del_count > 0)) {
		FILE *p = NULL;
		FILE *n;
		FILE *stream;

#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: As an optimization, we could unlock after we move past the head.
#endif
		/* Grab the openlist add lock since we might change the head of the list. */
		__STDIO_THREADLOCK_OPENLIST_ADD;
		for (stream = _stdio_openlist; stream; stream = n) {
			n = stream->__nextopen;
#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: fix for nonatomic
#endif
			if ((stream->__modeflags & (__FLAG_READONLY|__FLAG_WRITEONLY|__FLAG_FAILED_FREOPEN))
				== (__FLAG_READONLY|__FLAG_WRITEONLY)
				) {		 /* The file was closed and should be removed from the list. */
				if (!p) {
					_stdio_openlist = n;
				} else {
					p->__nextopen = n;
				}
				__STDIO_STREAM_FREE_FILE(stream);
			} else {
				p = stream;
			}
		}
		__STDIO_THREADUNLOCK_OPENLIST_ADD;
		_stdio_openlist_del_count = 0; /* Should be clean now. */
	}
	--_stdio_openlist_use_count;
	__STDIO_THREADUNLOCK_OPENLIST_DEL;
}
#endif

int fflush_unlocked(register FILE *stream)
{
#ifdef __STDIO_BUFFERS

	int retval = 0;
#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: should probably define a modeflags type
#endif
	unsigned short bufmask = __FLAG_LBF;

#ifndef NDEBUG
	if ((stream != NULL) && (stream != (FILE *) &_stdio_openlist)) {
		__STDIO_STREAM_VALIDATE(stream); /* debugging only */
	}
#endif

	if (stream == (FILE *) &_stdio_openlist) { /* Flush all lbf streams. */
		stream = NULL;
		bufmask = 0;
	}

	if (!stream) {				/* Flush all (lbf) writing streams. */

		__STDIO_OPENLIST_INC_USE;

		__STDIO_THREADLOCK_OPENLIST_ADD;
		stream = _stdio_openlist;
		__STDIO_THREADUNLOCK_OPENLIST_ADD;

		while(stream) {
			/* We only care about currently writing streams and do not want to
			 * block trying to obtain mutexes on non-writing streams. */
#warning fix for nonatomic
#warning unnecessary check if no threads
			if (__STDIO_STREAM_IS_WRITING(stream)) { /* ONLY IF ATOMIC!!! */
				__MY_STDIO_THREADLOCK(stream);
				/* Need to check again once we have the lock. */
				if (!(((stream->__modeflags | bufmask)
					   ^ (__FLAG_WRITING|__FLAG_LBF)
					   ) & (__FLAG_WRITING|__MASK_BUFMODE))
					) {
					if (!__STDIO_COMMIT_WRITE_BUFFER(stream)) {
						__STDIO_STREAM_DISABLE_PUTC(stream);
						__STDIO_STREAM_CLEAR_WRITING(stream);
					} else {
						retval = EOF;
					}
				}
				__MY_STDIO_THREADUNLOCK(stream);
			}
			stream = stream->__nextopen;
		}

		__STDIO_OPENLIST_DEC_USE;

	} else if (__STDIO_STREAM_IS_WRITING(stream)) {
		if (!__STDIO_COMMIT_WRITE_BUFFER(stream)) {
			__STDIO_STREAM_DISABLE_PUTC(stream);
			__STDIO_STREAM_CLEAR_WRITING(stream);
		} else {
			retval = EOF;
		}
	}
#if 0
	else if (stream->__modeflags & (__MASK_READING|__FLAG_READONLY)) {
		/* ANSI/ISO says behavior in this case is undefined but also says you
		 * shouldn't flush a stream you were reading from.  As usual, glibc
		 * caters to broken programs and simply ignores this. */
		__UNDEFINED_OR_NONPORTABLE;
		__STDIO_STREAM_SET_ERROR(stream);
		__set_errno(EBADF);
		retval = EOF;
	}
#endif

#ifndef NDEBUG
	if ((stream != NULL) && (stream != (FILE *) &_stdio_openlist)) {
		__STDIO_STREAM_VALIDATE(stream); /* debugging only */
	}
#endif

	return retval;

#else  /* __STDIO_BUFFERS --------------------------------------- */

#ifndef NDEBUG
	if ((stream != NULL)
#ifdef __STDIO_HAS_OPENLIST
		&& (stream != (FILE *) &_stdio_openlist)
#endif
		) {
		__STDIO_STREAM_VALIDATE(stream); /* debugging only */
	}
#endif

#if 0
	if (stream && (stream->__modeflags & (__MASK_READING|__FLAG_READONLY))) {
		/* ANSI/ISO says behavior in this case is undefined but also says you
		 * shouldn't flush a stream you were reading from.  As usual, glibc
		 * caters to broken programs and simply ignores this. */
		__UNDEFINED_OR_NONPORTABLE;
		__STDIO_STREAM_SET_ERROR(stream);
		__set_errno(EBADF);
		return EOF;
	}
#endif

	return 0;
#endif /* __STDIO_BUFFERS */
}
libc_hidden_def(fflush_unlocked)

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(fflush_unlocked,fflush)
libc_hidden_def(fflush)
#endif

#elif defined __UCLIBC_HAS_THREADS__

int fflush(register FILE *stream)
{
	int retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	if (stream
#ifdef __STDIO_HAS_OPENLIST
		&& (stream != (FILE *) &_stdio_openlist)
#endif
		) {

		__STDIO_AUTO_THREADLOCK(stream);

		retval = fflush_unlocked(stream);

		__STDIO_AUTO_THREADUNLOCK(stream);
	} else {
		retval = fflush_unlocked(stream);
	}

	return retval;
}
libc_hidden_def(fflush)

#endif
