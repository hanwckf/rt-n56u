/*  Copyright (C) 2002     Manuel Novoa III
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  Besides uClibc, I'm using this code in my libc for elks, which is
 *  a 16-bit environment with a fairly limited compiler.  It would make
 *  things much easier for me if this file isn't modified unnecessarily.
 *  In particular, please put any new or replacement functions somewhere
 *  else, and modify the makefile to use your version instead.
 *  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */

/* Nov 21, 2002
 *
 * Reimplement fputwc and fputws in terms of internal function _wstdio_fwrite.
 */




/*
 * ANSI/ISO C99 says

 9 Although both text and binary wide­oriented streams are conceptually sequences of wide
 characters, the external file associated with a wide­oriented stream is a sequence of
 multibyte characters, generalized as follows:
 --- Multibyte encodings within files may contain embedded null bytes (unlike multibyte
 encodings valid for use internal to the program).
 --- A file need not begin nor end in the initial shift state. 225)

 * How do we deal with this?

 * Should auto_wr_transition init the mbstate object?
*/


#define _GNU_SOURCE
#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

#ifndef __UCLIBC_HAS_THREADS__

#ifdef __BCC__
#define UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,STREAM) \
asm(".text\nexport _" "NAME" "_unlocked\n_" "NAME" "_unlocked = _" "NAME"); \
RETURNTYPE NAME PARAMS
#else
#define UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,STREAM) \
strong_alias(NAME,NAME##_unlocked) \
RETURNTYPE NAME PARAMS
#endif

#define UNLOCKED(RETURNTYPE,NAME,PARAMS,ARGS) \
	UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,stream)

#ifdef __BCC__
#define UNLOCKED_VOID_RETURN(NAME,PARAMS,ARGS) \
asm(".text\nexport _" "NAME" "_unlocked\n_" "NAME" "_unlocked = _" "NAME"); \
void NAME PARAMS
#else
#define UNLOCKED_VOID_RETURN(NAME,PARAMS,ARGS) \
strong_alias(NAME,NAME##_unlocked) \
void NAME PARAMS
#endif

#else  /* __UCLIBC_HAS_THREADS__ */

#include <pthread.h>

#define UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,STREAM) \
RETURNTYPE NAME PARAMS \
{ \
	RETURNTYPE retval; \
	__STDIO_THREADLOCK(STREAM); \
	retval = NAME##_unlocked ARGS ; \
	__STDIO_THREADUNLOCK(STREAM); \
	return retval; \
} \
RETURNTYPE NAME##_unlocked PARAMS

#define UNLOCKED(RETURNTYPE,NAME,PARAMS,ARGS) \
	UNLOCKED_STREAM(RETURNTYPE,NAME,PARAMS,ARGS,stream)

#define UNLOCKED_VOID_RETURN(NAME,PARAMS,ARGS) \
void NAME PARAMS \
{ \
	__STDIO_THREADLOCK(stream); \
	NAME##_unlocked ARGS ; \
	__STDIO_THREADUNLOCK(stream); \
} \
void NAME##_unlocked PARAMS

#endif /* __UCLIBC_HAS_THREADS__ */

#ifndef __STDIO_BUFFERS
#error stdio buffers are currently required for wide i/o
#endif

/**********************************************************************/
#ifdef L_fwide

/* TODO: According to SUSv3 should return EBADF if invalid stream. */

int fwide(register FILE *stream, int mode)
{
	__STDIO_THREADLOCK(stream);

	if (mode && !(stream->modeflags & (__FLAG_WIDE|__FLAG_NARROW))) {
		stream->modeflags |= ((mode > 0) ? __FLAG_WIDE : __FLAG_NARROW);
	}

	mode = (stream->modeflags & __FLAG_WIDE)
		- (stream->modeflags & __FLAG_NARROW);

	__STDIO_THREADUNLOCK(stream);

	return mode;
}

#endif
/**********************************************************************/
#ifdef L_fgetwc

static void munge_stream(register FILE *stream, unsigned char *buf)
{
#ifdef __STDIO_GETC_MACRO
	stream->bufgetc =
#endif
#ifdef __STDIO_PUTC_MACRO
	stream->bufputc =
#endif
	stream->bufpos = stream->bufread = stream->bufend = stream->bufstart = buf;
}

UNLOCKED(wint_t,fgetwc,(register FILE *stream),(stream))
{
	wint_t wi;
	wchar_t wc[1];
	int n;
	size_t r;
	unsigned char c[1];
	unsigned char sbuf[1];

	wi = WEOF;					/* Prepare for failure. */

	if (stream->modeflags & __FLAG_NARROW) {
		stream->modeflags |= __FLAG_ERROR;
		__set_errno(EBADF);
		goto DONE;
	}
	stream->modeflags |= __FLAG_WIDE;

	if (stream->modeflags & __MASK_UNGOT) {/* Any ungetwc()s? */

		assert(stream->modeflags & __FLAG_READING);

/* 		assert( (stream->modeflags & (__FLAG_READING|__FLAG_ERROR)) */
/* 				== __FLAG_READING); */

		if ((((stream->modeflags & __MASK_UNGOT) > 1) || stream->ungot[1])) {
			stream->ungot_width[0] = 0;	/* Application ungot... */
		} else {
			stream->ungot_width[0] = stream->ungot_width[1]; /* scanf ungot */
		}

		wi = stream->ungot[(--stream->modeflags) & __MASK_UNGOT];
		stream->ungot[1] = 0;
		goto DONE;
	}

	if (!stream->bufstart) {	/* Ugh... stream isn't buffered! */
		/* Munge the stream temporarily to use a 1-byte buffer. */
		munge_stream(stream, sbuf);
		++stream->bufend;
	}

	if (stream->state.mask == 0) { /* If last was a complete char */
		stream->ungot_width[0] = 0;	/* then reset the width. */
	}

 LOOP:
	if ((n = stream->bufread - stream->bufpos) == 0) {
		goto FILL_BUFFER;
	}

	r = mbrtowc(wc, stream->bufpos, n, &stream->state);
	if (((ssize_t) r) >= 0) {	/* Success... */
		if (r == 0) {			/* Nul wide char... means 0 byte for us so */
			++r;				/* increment r and handle below as single. */
		}
		stream->bufpos += r;
		stream->ungot_width[0] += r;
		wi = *wc;
		goto DONE;
	}

	if (r == ((size_t) -2)) {
		/* Potentially valid but incomplete and no more buffered. */
		stream->bufpos += n;	/* Update bufpos for stream. */
		stream->ungot_width[0] += n;
	FILL_BUFFER:
		if (_stdio_fread(c, (size_t) 1, stream) > 0) {
			assert(stream->bufpos == stream->bufstart + 1);
			*--stream->bufpos = *c;	/* Insert byte into buffer. */
			goto LOOP;
		}
		if (!__FERROR(stream)) { /* EOF with no error. */
			if (!stream->state.mask) {	/* No partially complete wchar. */
				goto DONE;
			}
			/* EOF but partially complete wchar. */
			/* TODO: should EILSEQ be set? */
			__set_errno(EILSEQ);
		}
	}

	/* If we reach here, either r == ((size_t)-1) and mbrtowc set errno
	 * to EILSEQ, or r == ((size_t)-2) and stream is in an error state
	 * or at EOF with a partially complete wchar.  Make sure stream's
	 * error indicator is set. */
	stream->modeflags |= __FLAG_ERROR;

 DONE:
	if (stream->bufstart == sbuf) {	/* Need to un-munge the stream. */
		munge_stream(stream, NULL);
	}

	return wi;
}

strong_alias(fgetwc_unlocked,getwc_unlocked);
strong_alias(fgetwc,getwc);

#endif
/**********************************************************************/
#ifdef L_getwchar

UNLOCKED_STREAM(wint_t,getwchar,(void),(),stdin)
{
	register FILE *stream = stdin; /* This helps bcc optimize. */

	return fgetwc_unlocked(stream);
}

#endif
/**********************************************************************/
#ifdef L_fgetws

UNLOCKED(wchar_t *,fgetws,(wchar_t *__restrict ws, int n,
						   FILE *__restrict stream),(ws, n, stream))
{
	register wchar_t *p = ws;
	wint_t wi;

	while ((n > 1)
		   && ((wi = fgetwc_unlocked(stream)) != WEOF)
		   && ((*p++ = wi) != '\n')
		   ) {
		--n;
	}
	if (p == ws) {
		/* TODO -- should we set errno? */
/*  		if (n <= 0) { */
/*  			errno = EINVAL; */
/*  		} */
		return NULL;
	}
	*p = 0;
	return ws;
}

#endif
/**********************************************************************/
#ifdef L_fputwc

UNLOCKED(wint_t,fputwc,(wchar_t wc, FILE *stream),(wc, stream))
{
#if 1
	return _wstdio_fwrite(&wc, 1, stream) ? wc : WEOF;
#else
	size_t n;
	char buf[MB_LEN_MAX];

	if (stream->modeflags & __FLAG_NARROW) {
		stream->modeflags |= __FLAG_ERROR;
		__set_errno(EBADF);
		return WEOF;
	}
	stream->modeflags |= __FLAG_WIDE;

	return (((n = wcrtomb(buf, wc, &stream->state)) != ((size_t)-1)) /* !EILSEQ */
			&& (_stdio_fwrite(buf, n, stream) == n))/* and wrote everything. */
		? wc : WEOF;
#endif
}

strong_alias(fputwc_unlocked,putwc_unlocked);
strong_alias(fputwc,putwc);

#endif
/**********************************************************************/
#ifdef L_putwchar

UNLOCKED_STREAM(wint_t,putwchar,(wchar_t wc),(wc),stdout)
{
	register FILE *stream = stdout; /* This helps bcc optimize. */

	return fputwc_unlocked(wc, stream);
}

#endif
/**********************************************************************/
#ifdef L_fputws

UNLOCKED(int,fputws,(const wchar_t *__restrict ws,
					 register FILE *__restrict stream),(ws, stream))
{
#if 1
	size_t n = wcslen(ws);

	return (_wstdio_fwrite(ws, n, stream) == n) ? 0 : -1;
#else
	size_t n;
	char buf[64];

	if (stream->modeflags & __FLAG_NARROW) {
		stream->modeflags |= __FLAG_ERROR;
		__set_errno(EBADF);
		return -1;
	}
	stream->modeflags |= __FLAG_WIDE;

	while ((n = wcsrtombs(buf, &ws, sizeof(buf), &stream->state)) != 0) {
		/* Wasn't an empty wide string. */
		if ((n == ((size_t) -1))/* Encoding error! */
			 || (_stdio_fwrite(buf, n, stream) != n)/* Didn't write everything. */
			 ) {
			return -1;
		}
		if (!ws) {				/* Done? */
			break;
		}
	}

	return 1;
#endif
}

#endif
/**********************************************************************/
#ifdef L_ungetwc
/*
 * Note: This is the application-callable ungetwc.  If wscanf calls this, it
 * should also set stream->ungot[1] to 0 if this is the only ungot, as well
 * as reset stream->ungot_width[1] for use by _stdio_adjpos().
 */

/* Reentrant. */

wint_t ungetwc(wint_t c, register FILE *stream)
{
	__STDIO_THREADLOCK(stream);

	__stdio_validate_FILE(stream); /* debugging only */

	if (stream->modeflags & __FLAG_NARROW) {
		stream->modeflags |= __FLAG_ERROR;
		c = WEOF;
		goto DONE;
	}
	stream->modeflags |= __FLAG_WIDE;

	/* If can't read or c == WEOF or ungot slots already filled, then fail. */
	if ((stream->modeflags
		 & (__MASK_UNGOT2|__FLAG_WRITEONLY
#ifndef __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__
			|__FLAG_WRITING		/* Note: technically no, but yes in spirit */
#endif /* __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__ */
			))
		|| ((stream->modeflags & __MASK_UNGOT1) && (stream->ungot[1]))
		|| (c == WEOF) ) {
		c = WEOF;
		goto DONE;;
	}

/*  ungot_width */

#ifdef __STDIO_BUFFERS
#ifdef __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__
	if (stream->modeflags & __FLAG_WRITING) {
		fflush_unlocked(stream); /* Commit any write-buffered chars. */
	}
#endif /* __UCLIBC_HAS_STDIO_AUTO_RW_TRANSITION__ */
#endif /* __STDIO_BUFFERS */

	/* Clear EOF and WRITING flags, and set READING FLAG */
	stream->modeflags &= ~(__FLAG_EOF|__FLAG_WRITING);
#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Is setting the reading flag after an ungetwc necessary?
#endif /* __UCLIBC_MJN3_ONLY__ */
	stream->modeflags |= __FLAG_READING;
	stream->ungot[1] = 1;		/* Flag as app ungetc call; wscanf fixes up. */
	stream->ungot[(stream->modeflags++) & __MASK_UNGOT] = c;

	__stdio_validate_FILE(stream); /* debugging only */

 DONE:
	__STDIO_THREADUNLOCK(stream);

	return c;
}

#endif
/**********************************************************************/
