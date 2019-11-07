/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


#ifndef __DO_LARGEFILE
# define FILEDES_ARG    (-1)
#endif

FILE *freopen(const char * __restrict filename, const char * __restrict mode,
			  register FILE * __restrict stream)
{
	/*
	 * ANSI/ISO allow (implementation-defined) change of mode for an
	 * existing file if filename is NULL.  It doesn't look like Linux
	 * supports this, so we don't here.
	 *
	 * NOTE: Whether or not the stream is free'd on failure is unclear
	 *       w.r.t. ANSI/ISO.  This implementation chooses to NOT free
	 *       the stream and associated buffer if they were dynamically
	 *       allocated.
	 * NOTE: Previous versions of uClibc did free dynamic storage.
	 *
	 * TODO: Apparently linux allows setting append mode.  Implement?
	 */
	unsigned short dynmode;
	register FILE *fp;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	__STDIO_STREAM_VALIDATE(stream);

	__STDIO_OPENLIST_INC_USE;	/* Do not remove the file from the list. */

	/* First, flush and close, but don't deallocate, the stream. */
	/* This also removes the stream for the open file list. */
	dynmode = (stream->__modeflags & (__FLAG_FREEBUF|__FLAG_FREEFILE));

	stream->__modeflags &= ~(__FLAG_FREEBUF|__FLAG_FREEFILE);

	/* Only call fclose on the stream if it is not already closed. */
	if ((stream->__modeflags & (__FLAG_READONLY|__FLAG_WRITEONLY))
		!= (__FLAG_READONLY|__FLAG_WRITEONLY)
		) {
		fclose(stream);			/* Failures are ignored. */
		/* NOTE: fclose always does __STDIO_OPENLIST_INC_DEL_CNT.  But we don't
		 * want to remove this FILE from the open list, even if the freopen fails.
		 * Consider the case of a failed freopen() on stdin.  You probably still
		 * want to be able to call freopen() again.  Similarly for other "malloc'd"
		 * streams. */
		__STDIO_OPENLIST_DEC_DEL_CNT;
	}

	fp = _stdio_fopen(((intptr_t) filename), mode, stream, FILEDES_ARG);
	if (!fp) {
		/* Don't remove stream from the open file list and (potentially) free it.
		 * See _stdio_openlist_dec_use() in fflush.c. */
		stream->__modeflags = __FLAG_READONLY|__FLAG_WRITEONLY|__FLAG_FAILED_FREOPEN;
	}

	/* Reset the allocation flags. */
	stream->__modeflags |= dynmode;

	__STDIO_OPENLIST_DEC_USE;

	__STDIO_AUTO_THREADUNLOCK(stream);

	return fp;
}
