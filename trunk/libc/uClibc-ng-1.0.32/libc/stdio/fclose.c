/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


int fclose(register FILE *stream)
{
	int rv = 0;
	__STDIO_AUTO_THREADLOCK_VAR;

#ifdef __STDIO_HAS_OPENLIST
#if !defined(__UCLIBC_HAS_THREADS__) || !defined(__STDIO_BUFFERS)
	/* First, remove the file from the open file list. */
	{
		FILE *ptr;

		__STDIO_THREADLOCK_OPENLIST_DEL;
		__STDIO_THREADLOCK_OPENLIST_ADD;
		ptr = _stdio_openlist;
		if ((ptr = _stdio_openlist) == stream) {
			_stdio_openlist = stream->__nextopen;
		} else {
			while (ptr) {
				if (ptr->__nextopen == stream) {
					ptr->__nextopen = stream->__nextopen;
					break;
				}
				ptr = ptr->__nextopen;
			}
		}
		__STDIO_THREADUNLOCK_OPENLIST_ADD;
		__STDIO_THREADUNLOCK_OPENLIST_DEL;
	}
#endif
#endif

	__STDIO_AUTO_THREADLOCK(stream);

	__STDIO_STREAM_VALIDATE(stream);

#ifdef __STDIO_BUFFERS
	/* Write any pending buffered chars. */
	if (__STDIO_STREAM_IS_WRITING(stream)) {
		rv = fflush_unlocked(stream);
	}
#endif

	if (__CLOSE(stream) < 0) {	/* Must close even if fflush failed. */
		rv = EOF;
	}

	stream->__filedes = -1;

	/* We need a way for freopen to know that a file has been closed.
	 * Since a file can't be both readonly and writeonly, that makes
	 * an effective signal.  It also has the benefit of disabling
	 * transitions to either reading or writing. */
#if defined(__UCLIBC_HAS_THREADS__) && defined(__STDIO_BUFFERS)
	/* Before we mark the file as closed, make sure we increment the openlist use count
	 * so it isn't freed under us while still cleaning up. */
	__STDIO_OPENLIST_INC_USE;
#endif
	stream->__modeflags &= (__FLAG_FREEBUF|__FLAG_FREEFILE);
	stream->__modeflags |= (__FLAG_READONLY|__FLAG_WRITEONLY);

#ifndef NDEBUG
	/* Reinitialize everything (including putc since fflush could fail). */
	__STDIO_STREAM_DISABLE_GETC(stream);
	__STDIO_STREAM_DISABLE_PUTC(stream);
	__STDIO_STREAM_INIT_BUFREAD_BUFPOS(stream);

# ifdef __UCLIBC_HAS_WCHAR__
	stream->__ungot_width[0] = 0;
# endif
# ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(stream->__state));
# endif
#endif

	__STDIO_AUTO_THREADUNLOCK(stream);

	__STDIO_STREAM_FREE_BUFFER(stream);
#if defined(__UCLIBC_HAS_THREADS__) && defined(__STDIO_BUFFERS)
	/* inefficient - locks/unlocks twice and walks whole list */
	__STDIO_OPENLIST_INC_DEL_CNT;
	__STDIO_OPENLIST_DEC_USE;	/* This with free the file if necessary. */
#else
	__STDIO_STREAM_FREE_FILE(stream);
#endif

	return rv;
}
libc_hidden_def(fclose)
