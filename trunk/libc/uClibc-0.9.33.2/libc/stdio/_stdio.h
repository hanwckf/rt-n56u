/* Copyright (C) 2004-2005 Manuel Novoa III    <mjn3@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include <features.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#ifdef __UCLIBC_HAS_WCHAR__
#include <wchar.h>
#endif

#include <bits/uClibc_mutex.h>

#define __STDIO_THREADLOCK_OPENLIST_ADD			\
        __UCLIBC_IO_MUTEX_LOCK(_stdio_openlist_add_lock)

#define __STDIO_THREADUNLOCK_OPENLIST_ADD		\
        __UCLIBC_IO_MUTEX_UNLOCK(_stdio_openlist_add_lock)

#ifdef __STDIO_BUFFERS

#define __STDIO_THREADLOCK_OPENLIST_DEL			\
        __UCLIBC_IO_MUTEX_LOCK(_stdio_openlist_del_lock)

#define __STDIO_THREADUNLOCK_OPENLIST_DEL		\
        __UCLIBC_IO_MUTEX_UNLOCK(_stdio_openlist_del_lock)


#ifdef __UCLIBC_HAS_THREADS__
#define __STDIO_OPENLIST_INC_USE			\
do {							\
	__STDIO_THREADLOCK_OPENLIST_DEL;		\
	++_stdio_openlist_use_count;			\
	__STDIO_THREADUNLOCK_OPENLIST_DEL;		\
} while (0)

extern void _stdio_openlist_dec_use(void) attribute_hidden;

#define __STDIO_OPENLIST_DEC_USE			\
	_stdio_openlist_dec_use()

#define __STDIO_OPENLIST_INC_DEL_CNT			\
do {							\
	__STDIO_THREADLOCK_OPENLIST_DEL;		\
	++_stdio_openlist_del_count;			\
	__STDIO_THREADUNLOCK_OPENLIST_DEL;		\
} while (0)

#define __STDIO_OPENLIST_DEC_DEL_CNT			\
do { \
	__STDIO_THREADLOCK_OPENLIST_DEL;		\
	--_stdio_openlist_del_count;			\
	__STDIO_THREADUNLOCK_OPENLIST_DEL;		\
} while (0)

#endif /* __UCLIBC_HAS_THREADS__ */
#endif /* __STDIO_BUFFERS */

#ifndef __STDIO_THREADLOCK_OPENLIST_DEL
#define	__STDIO_THREADLOCK_OPENLIST_DEL     ((void)0)
#endif
#ifndef __STDIO_THREADUNLOCK_OPENLIST_DEL
#define	__STDIO_THREADUNLOCK_OPENLIST_DEL   ((void)0)
#endif
#ifndef __STDIO_OPENLIST_INC_USE
#define __STDIO_OPENLIST_INC_USE            ((void)0)
#endif
#ifndef __STDIO_OPENLIST_DEC_USE
#define __STDIO_OPENLIST_DEC_USE            ((void)0)
#endif
#ifndef __STDIO_OPENLIST_INC_DEL_CNT
#define __STDIO_OPENLIST_INC_DEL_CNT        ((void)0)
#endif
#ifndef __STDIO_OPENLIST_DEC_DEL_CNT
#define __STDIO_OPENLIST_DEC_DEL_CNT        ((void)0)
#endif

#define __UNDEFINED_OR_NONPORTABLE ((void)0)

/**********************************************************************/
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__

extern __ssize_t _cs_read(void *cookie, char *buf, size_t bufsize) attribute_hidden;
extern __ssize_t _cs_write(void *cookie, const char *buf, size_t bufsize) attribute_hidden;
extern int _cs_seek(void *cookie, __offmax_t *pos, int whence) attribute_hidden;
extern int _cs_close(void *cookie) attribute_hidden;

#define __STDIO_STREAM_RESET_GCS(S) \
	(S)->__cookie = &((S)->__filedes); \
	(S)->__gcs.read = _cs_read; \
	(S)->__gcs.write = _cs_write; \
	(S)->__gcs.seek = _cs_seek; \
	(S)->__gcs.close = _cs_close


#define __READ(STREAMPTR,BUF,SIZE) \
	((((STREAMPTR)->__gcs.read) == NULL) ? -1 : \
	(((STREAMPTR)->__gcs.read)((STREAMPTR)->__cookie,(BUF),(SIZE))))
#define __WRITE(STREAMPTR,BUF,SIZE) \
	((((STREAMPTR)->__gcs.write) == NULL) ? -1 : \
	(((STREAMPTR)->__gcs.write)((STREAMPTR)->__cookie,(BUF),(SIZE))))
#define __SEEK(STREAMPTR,PPOS,WHENCE) \
	((((STREAMPTR)->__gcs.seek) == NULL) ? -1 : \
	(((STREAMPTR)->__gcs.seek)((STREAMPTR)->__cookie,(PPOS),(WHENCE))))
#define __CLOSE(STREAMPTR) \
	((((STREAMPTR)->__gcs.close) == NULL) ? 0 : \
	(((STREAMPTR)->__gcs.close)((STREAMPTR)->__cookie)))

#else  /* __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__ */

extern int __stdio_seek(FILE *stream, register __offmax_t *pos, int whence) attribute_hidden;

#define __STDIO_STREAM_RESET_GCS(S) ((void)0)

#define __READ(STREAMPTR,BUF,SIZE) \
	(read((STREAMPTR)->__filedes,(BUF),(SIZE)))
#define __WRITE(STREAMPTR,BUF,SIZE) \
	(write((STREAMPTR)->__filedes,(BUF),(SIZE)))
#define __SEEK(STREAMPTR,PPOS,WHENCE) \
	(__stdio_seek((STREAMPTR),(PPOS),(WHENCE)))
#define __CLOSE(STREAMPTR) \
	(close((STREAMPTR)->__filedes))

#endif /* __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__ */

/**********************************************************************/
#ifdef __UCLIBC_HAS_WCHAR__

#define __STDIO_STREAM_TRANS_TO_WRITE(S,O)	__stdio_trans2w_o((S), (O))
#define __STDIO_STREAM_TRANS_TO_READ(S,O)	__stdio_trans2r_o((S), (O))

#else

#define __STDIO_STREAM_TRANS_TO_WRITE(S,O)	__stdio_trans2w((S))
#define __STDIO_STREAM_TRANS_TO_READ(S,O)	__stdio_trans2r((S))

#endif
/**********************************************************************/

#define __STDIO_STREAM_IS_READING(S) ((S)->__modeflags & __MASK_READING)
#define __STDIO_STREAM_IS_WRITING(S) ((S)->__modeflags & __FLAG_WRITING)

#define __STDIO_STREAM_SET_READING(S) ((S)->__modeflags |= __FLAG_READING)
#define __STDIO_STREAM_SET_WRITING(S) ((S)->__modeflags |= __FLAG_WRITING)

#define __STDIO_STREAM_IS_READING_OR_READONLY(S) \
	((S)->__modeflags & (__MASK_READING|__FLAG_READONLY))

#define __STDIO_STREAM_IS_WRITING_OR_WRITEONLY(S) \
	((S)->__modeflags & (__FLAG_WRITING|__FLAG_WRITEONLY))

#define __STDIO_STREAM_IS_READONLY(S) ((S)->__modeflags & __FLAG_READONLY)
#define __STDIO_STREAM_IS_WRITEONLY(S) ((S)->__modeflags & __FLAG_WRITEONLY)


/**********************************************************************/
#ifdef __UCLIBC_HAS_WCHAR__

#define __STDIO_STREAM_IS_NARROW_WRITING(S)			\
	(((S)->__modeflags & (__FLAG_WRITING|__FLAG_NARROW))	\
	 == (__FLAG_WRITING|__FLAG_NARROW))

#define __STDIO_STREAM_IS_WIDE_WRITING(S)			\
	(((S)->__modeflags & (__FLAG_WRITING|__FLAG_WIDE))	\
	 == (__FLAG_WRITING|__FLAG_WIDE))

#if (__FLAG_NARROW <= __MASK_READING)
#error assumption violated regarding __FLAG_NARROW
#endif

#define __STDIO_STREAM_IS_NARROW_READING(S)			\
	(((S)->__modeflags & (__MASK_READING|__FLAG_NARROW)) > __FLAG_NARROW)

#define __STDIO_STREAM_IS_WIDE_READING(S)			\
	(((S)->__modeflags & (__MASK_READING|__FLAG_WIDE)) > __FLAG_WIDE)

#define __STDIO_STREAM_IS_NARROW(S)		((S)->__modeflags & __FLAG_NARROW)
#define __STDIO_STREAM_IS_WIDE(S)		((S)->__modeflags & __FLAG_WIDE)

#define __STDIO_STREAM_SET_NARROW(S)				\
	((void)((S)->__modeflags |= __FLAG_NARROW))
#define __STDIO_STREAM_SET_WIDE(S)				\
	((void)((S)->__modeflags |= __FLAG_WIDE))

#else

#define __STDIO_STREAM_IS_NARROW_WRITING(S)  __STDIO_STREAM_IS_WRITING(S)

#define __STDIO_STREAM_IS_NARROW_READING(S)  __STDIO_STREAM_IS_READING(S)

#define __STDIO_STREAM_IS_NARROW(S)			(1)
#define __STDIO_STREAM_IS_WIDE(S)			(0)

#define __STDIO_STREAM_SET_NARROW(S)			((void)0)
#define __STDIO_STREAM_SET_WIDE(S)			((void)0)

#endif
/**********************************************************************/

#define __STDIO_STREAM_SET_EOF(S) \
	((void)((S)->__modeflags |= __FLAG_EOF))
#define __STDIO_STREAM_SET_ERROR(S) \
	((void)((S)->__modeflags |= __FLAG_ERROR))

#define __STDIO_STREAM_CLEAR_EOF(S) \
	((void)((S)->__modeflags &= ~__FLAG_EOF))
#define __STDIO_STREAM_CLEAR_ERROR(S) \
	((void)((S)->__modeflags &= ~__FLAG_ERROR))

#define __STDIO_STREAM_CLEAR_READING_AND_UNGOTS(S) \
	((void)((S)->__modeflags &= ~__MASK_READING))
#define __STDIO_STREAM_CLEAR_WRITING(S) \
	((void)((S)->__modeflags &= ~__FLAG_WRITING))

#ifdef __UCLIBC_HAS_STDIO_GETC_MACRO__
# define __STDIO_STREAM_DISABLE_GETC(S) \
	((void)((S)->__bufgetc_u = (S)->__bufstart))
# define __STDIO_STREAM_ENABLE_GETC(S) \
	((void)((S)->__bufgetc_u = (S)->__bufread))
# define __STDIO_STREAM_CAN_USE_BUFFER_GET(S) \
	((S)->__bufpos < (S)->__bufgetc_u)
#else
# define __STDIO_STREAM_DISABLE_GETC(S)			((void)0)
# define __STDIO_STREAM_ENABLE_GETC(S)			((void)0)
# define __STDIO_STREAM_CAN_USE_BUFFER_GET(S)		(0)
#endif

#ifdef __UCLIBC_HAS_STDIO_PUTC_MACRO__
# define __STDIO_STREAM_DISABLE_PUTC(S) \
	((void)((S)->__bufputc_u = (S)->__bufstart))
# define __STDIO_STREAM_ENABLE_PUTC(S) \
	((void)((S)->__bufputc_u = (S)->__bufend))
# define __STDIO_STREAM_CAN_USE_BUFFER_ADD(S) \
	((S)->__bufpos < (S)->__bufputc_u)
#else
# define __STDIO_STREAM_DISABLE_PUTC(S)			((void)0)
# define __STDIO_STREAM_ENABLE_PUTC(S)			((void)0)
# define __STDIO_STREAM_CAN_USE_BUFFER_ADD(S)		(0)
#endif

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
#define __STDIO_STREAM_IS_CUSTOM(S)		((S)->__cookie != &((S)->__filedes))
#else
#define __STDIO_STREAM_IS_CUSTOM(S)			(0)
#endif

/**********************************************************************/

#ifdef __STDIO_BUFFERS
#define __STDIO_STREAM_FREE_BUFFER(S)			\
	do { if ((S)->__modeflags & __FLAG_FREEBUF)	\
	free((S)->__bufstart); } while (0)
#else
#define __STDIO_STREAM_FREE_BUFFER(S) ((void)0)
#endif

#define __STDIO_STREAM_FREE_FILE(S) \
	do { if ((S)->__modeflags & __FLAG_FREEFILE)	\
	free((S)); } while (0)


#ifdef __UCLIBC_HAS_LFS__
#define __STDIO_WHEN_LFS(E) E
#else
#define __STDIO_WHEN_LFS(E) ((void)0)
#endif

/**********************************************************************/
/* The following return 0 on success. */

#ifdef __STDIO_BUFFERS
/* Assume stream in valid writing state.  Do not reset writing flag
 * or disble putc macro unless error. */
/* Should we assume that buffer is not empty to avoid a check? */
extern size_t __stdio_wcommit(FILE *__restrict stream) attribute_hidden;

/* Remember to fail if at EOF! */
extern size_t __stdio_rfill(FILE *__restrict stream) attribute_hidden;

extern size_t __stdio_fwrite(const unsigned char *__restrict buffer,
		size_t bytes, FILE *__restrict stream) attribute_hidden;
#else

#define __stdio_fwrite(B,N,S)  __stdio_WRITE((S),(B),(N))

#endif

extern size_t __stdio_WRITE(FILE *stream, const unsigned char *buf,
		size_t bufsize) attribute_hidden;
extern size_t __stdio_READ(FILE *stream, unsigned char *buf,
		size_t bufsize) attribute_hidden;

extern int __stdio_trans2r(FILE *__restrict stream) attribute_hidden;
extern int __stdio_trans2w(FILE *__restrict stream) attribute_hidden;

extern int __stdio_trans2r_o(FILE *__restrict stream, int oflag) attribute_hidden;
extern int __stdio_trans2w_o(FILE *__restrict stream, int oflag) attribute_hidden;

/**********************************************************************/
#ifdef __STDIO_BUFFERS

#define __STDIO_STREAM_IS_FBF(S)		(!((S)->__modeflags & __MASK_BUFMODE))
#define __STDIO_STREAM_IS_LBF(S)		((S)->__modeflags & __FLAG_LBF)
#define __STDIO_STREAM_IS_NBF(S)		((S)->__modeflags & __FLAG_NBF)

#define __STDIO_STREAM_BUFFER_SIZE(S)		((S)->__bufend - (S)->__bufstart)

/* Valid when writing... */
#define __STDIO_STREAM_BUFFER_ADD(S,C)		(*(S)->__bufpos++ = (C))
#define __STDIO_STREAM_BUFFER_UNADD(S)		(--(S)->__bufpos)
#define __STDIO_STREAM_BUFFER_WAVAIL(S)		((S)->__bufend - (S)->__bufpos)
#define __STDIO_STREAM_BUFFER_WUSED(S)		((S)->__bufpos - (S)->__bufstart)
#define __STDIO_COMMIT_WRITE_BUFFER(S)		__stdio_wcommit((S))
#ifdef __UCLIBC_HAS_WCHAR__
#define __STDIO_STREAM_IS_NARROW_FBF(S) \
	(!((S)->__modeflags & (__MASK_BUFMODE|__FLAG_WIDE)))
#else
#define __STDIO_STREAM_IS_NARROW_FBF(S)		__STDIO_STREAM_IS_FBF((S))
#endif

/* Valid when reading... */
#define __STDIO_STREAM_BUFFER_RAVAIL(S)		((S)->__bufread - (S)->__bufpos)
#define __STDIO_STREAM_BUFFER_GET(S)		(*(S)->__bufpos++)
#define __STDIO_FILL_READ_BUFFER(S)		__stdio_rfill((S))

#define __STDIO_STREAM_INIT_BUFREAD_BUFPOS(S)		\
	(S)->__bufread = (S)->__bufpos = (S)->__bufstart


#define __STDIO_STREAM_FAKE_VSNPRINTF_FILEDES		(-2)
#define __STDIO_STREAM_FAKE_VSSCANF_FILEDES		(-2)
#define __STDIO_STREAM_FAKE_VSWPRINTF_FILEDES		(-3)
#define __STDIO_STREAM_FAKE_VSWSCANF_FILEDES		(-3)

#define __STDIO_STREAM_IS_FAKE_VSNPRINTF(S) \
	((S)->__filedes == __STDIO_STREAM_FAKE_VSNPRINTF_FILEDES)
#define __STDIO_STREAM_IS_FAKE_VSSCANF(S) \
	((S)->__filedes == __STDIO_STREAM_FAKE_VSSCANF_FILEDES)
#define __STDIO_STREAM_IS_FAKE_VSWPRINTF(S) \
	((S)->__filedes == __STDIO_STREAM_FAKE_VSWPRINTF_FILEDES)
#define __STDIO_STREAM_IS_FAKE_VSWSCANF(S) \
	((S)->__filedes == __STDIO_STREAM_FAKE_VSWSCANF_FILEDES)

#else  /* __STDIO_BUFFERS */

#define __STDIO_STREAM_IS_FBF(S)				(0)
#define __STDIO_STREAM_IS_LBF(S)				(0)
#define __STDIO_STREAM_IS_NBF(S)				(1)

#define __STDIO_STREAM_BUFFER_SIZE(S)				(0)
#define __STDIO_STREAM_BUFFER_ADD(S,C)				((void)0)
#define __STDIO_STREAM_BUFFER_UNADD(S)				((void)0)
#define __STDIO_STREAM_BUFFER_WAVAIL(S)				(0)
#define __STDIO_STREAM_BUFFER_WUSED(S)				(0)
#define __STDIO_COMMIT_WRITE_BUFFER(S)				(0)
#define __STDIO_STREAM_IS_NARROW_FBF(S)				(0)

#define __STDIO_STREAM_BUFFER_RAVAIL(S)				(0)
#define __STDIO_STREAM_BUFFER_GET(S)				(EOF)
#define __STDIO_FILL_READ_BUFFER(S)				(0)
#define __STDIO_STREAM_INIT_BUFREAD_BUFPOS(S)			((void)0)

#undef __STDIO_STREAM_FAKE_VSNPRINTF_FILEDES
#undef __STDIO_STREAM_FAKE_VSSCANF_FILEDES
#undef __STDIO_STREAM_FAKE_VSWPRINTF_FILEDES

#define __STDIO_STREAM_IS_FAKE_VSNPRINTF(S)			(0)
#define __STDIO_STREAM_IS_FAKE_VSSCANF(S)			(0)
#undef __STDIO_STREAM_IS_FAKE_VSWPRINTF

# ifdef __USE_OLD_VFPRINTF__
#  define __STDIO_STREAM_FAKE_VSNPRINTF_FILEDES_NB		(-2)
#  define __STDIO_STREAM_IS_FAKE_VSNPRINTF_NB(S)		\
	((S)->__filedes == __STDIO_STREAM_FAKE_VSNPRINTF_FILEDES_NB)
# endif

# ifndef __UCLIBC_HAS_WCHAR__
#  define __STDIO_STREAM_FAKE_VSSCANF_FILEDES_NB		(-2)
#  define __STDIO_STREAM_IS_FAKE_VSSCANF_NB(S)			\
	((S)->__filedes == __STDIO_STREAM_FAKE_VSSCANF_FILEDES_NB)
# endif

#endif /* __STDIO_BUFFERS */
/**********************************************************************/

extern int __stdio_adjust_position(FILE *__restrict stream, __offmax_t *pos) attribute_hidden;

#ifdef __STDIO_HAS_OPENLIST
	/* Uses an implementation hack!!! */
#define __STDIO_FLUSH_LBF_STREAMS		\
	fflush_unlocked((FILE *) &_stdio_openlist)
#else
#define __STDIO_FLUSH_LBF_STREAMS		((void)0)
#endif

#ifdef NDEBUG
#define __STDIO_STREAM_VALIDATE(S)		((void)0)
#else
extern void _stdio_validate_FILE(const FILE *stream) attribute_hidden;
#define __STDIO_STREAM_VALIDATE(S)		_stdio_validate_FILE((S))
#endif

#ifdef __STDIO_MBSTATE
#define __COPY_MBSTATE(dest,src) \
	((void)((dest)->__mask = (src)->__mask, (dest)->__wc = (src)->__wc))
#define __INIT_MBSTATE(dest)			((void)((dest)->__mask = 0))
#else
#define __COPY_MBSTATE(dest,src)		((void)0)
#define __INIT_MBSTATE(dest)			((void)0)
#endif

/**********************************************************************/

extern FILE *_stdio_fopen(intptr_t fname_or_mode, const char *__restrict mode,
		FILE *__restrict stream, int filedes) attribute_hidden;

#ifdef __UCLIBC_HAS_WCHAR__
extern size_t _wstdio_fwrite(const wchar_t *__restrict ws,
		size_t n, FILE *__restrict stream) attribute_hidden;
#endif

/**********************************************************************/

extern int _vfprintf_internal (FILE * __restrict stream,
			const char * __restrict format,
			va_list arg) attribute_hidden;

#ifdef __UCLIBC_HAS_WCHAR__
extern int _vfwprintf_internal (FILE * __restrict stream,
			const wchar_t * __restrict format,
			va_list arg) attribute_hidden;
#endif

/**********************************************************************/
/* Only use the macro below if you know fp is a valid FILE for a valid fd.
 * This is _not_ true for custom streams! */
#define __FILENO_UNLOCKED(fp)	((fp)->__filedes)

#define __FEOF_OR_FERROR_UNLOCKED(stream) \
	((stream)->__modeflags & (__FLAG_EOF|__FLAG_ERROR))

#if defined(__STDIO_BUFFERS) || defined(__USE_OLD_VFPRINTF__) || defined(__UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__)
#define __STDIO_HAS_VSNPRINTF 1
#endif
