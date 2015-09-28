/*
 * Private includes and definitions
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#ifndef XZ_PRIVATE_H
#define XZ_PRIVATE_H

#include <common.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include <xz.h>

#ifndef __always_inline
#define __always_inline inline
#endif

#ifndef GFP_KERNEL
#define GFP_KERNEL	0
#endif

#ifndef memzero
#define memzero(ptr, len) memset(ptr, 0, len)
#endif

#ifndef memeq
#define memeq(s1, s2, n) (memcmp(s1, s2, n) == 0)
#endif

#ifndef get_le32
#define get_le32(x) get_unaligned_le32(x)
#endif

#ifndef bool
typedef unsigned int bool;
#define false	(0)
#define true	(!false)
#endif

#ifndef min_t
#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#endif
#ifndef max_t
#define max_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })
#endif

/* If no specific decoding mode is requested, enable support for all modes. */
#if !defined(XZ_DEC_SINGLE) && !defined(XZ_DEC_PREALLOC) \
		&& !defined(XZ_DEC_DYNALLOC)
#	define XZ_DEC_SINGLE
#	define XZ_DEC_PREALLOC
#	define XZ_DEC_DYNALLOC
#endif

/*
 * The DEC_IS_foo(mode) macros are used in "if" statements. If only some
 * of the supported modes are enabled, these macros will evaluate to true or
 * false at compile time and thus allow the compiler to omit unneeded code.
 */
#ifdef XZ_DEC_SINGLE
#	define DEC_IS_SINGLE(mode) ((mode) == XZ_SINGLE)
#else
#	define DEC_IS_SINGLE(mode) (false)
#endif

#ifdef XZ_DEC_PREALLOC
#	define DEC_IS_PREALLOC(mode) ((mode) == XZ_PREALLOC)
#else
#	define DEC_IS_PREALLOC(mode) (false)
#endif

#ifdef XZ_DEC_DYNALLOC
#	define DEC_IS_DYNALLOC(mode) ((mode) == XZ_DYNALLOC)
#else
#	define DEC_IS_DYNALLOC(mode) (false)
#endif

#if !defined(XZ_DEC_SINGLE)
#	define DEC_IS_MULTI(mode) (true)
#elif defined(XZ_DEC_PREALLOC) || defined(XZ_DEC_DYNALLOC)
#	define DEC_IS_MULTI(mode) ((mode) != XZ_SINGLE)
#else
#	define DEC_IS_MULTI(mode) (false)
#endif

/*
 * If any of the BCJ filter decoders are wanted, define XZ_DEC_BCJ.
 * XZ_DEC_BCJ is used to enable generic support for BCJ decoders.
 */
#ifndef XZ_DEC_BCJ
#	if defined(XZ_DEC_X86) || defined(XZ_DEC_POWERPC) \
			|| defined(XZ_DEC_IA64) || defined(XZ_DEC_ARM) \
			|| defined(XZ_DEC_ARM) || defined(XZ_DEC_ARMTHUMB) \
			|| defined(XZ_DEC_SPARC)
#		define XZ_DEC_BCJ
#	endif
#endif

/*
 * Allocate memory for LZMA2 decoder. xz_dec_lzma2_reset() must be used
 * before calling xz_dec_lzma2_run().
 */
XZ_EXTERN struct xz_dec_lzma2 * XZ_FUNC xz_dec_lzma2_create(
		enum xz_mode mode, uint32_t dict_max);

/*
 * Decode the LZMA2 properties (one byte) and reset the decoder. Return
 * XZ_OK on success, XZ_MEMLIMIT_ERROR if the preallocated dictionary is not
 * big enough, and XZ_OPTIONS_ERROR if props indicates something that this
 * decoder doesn't support.
 */
XZ_EXTERN enum xz_ret XZ_FUNC xz_dec_lzma2_reset(
		struct xz_dec_lzma2 *s, uint8_t props);

/* Decode raw LZMA2 stream from b->in to b->out. */
XZ_EXTERN enum xz_ret XZ_FUNC xz_dec_lzma2_run(
		struct xz_dec_lzma2 *s, struct xz_buf *b);

/* Free the memory allocated for the LZMA2 decoder. */
XZ_EXTERN void XZ_FUNC xz_dec_lzma2_end(struct xz_dec_lzma2 *s);

#ifdef XZ_DEC_BCJ
/*
 * Allocate memory for BCJ decoders. xz_dec_bcj_reset() must be used before
 * calling xz_dec_bcj_run().
 */
XZ_EXTERN struct xz_dec_bcj * XZ_FUNC xz_dec_bcj_create(bool single_call);

/*
 * Decode the Filter ID of a BCJ filter. This implementation doesn't
 * support custom start offsets, so no decoding of Filter Properties
 * is needed. Returns XZ_OK if the given Filter ID is supported.
 * Otherwise XZ_OPTIONS_ERROR is returned.
 */
XZ_EXTERN enum xz_ret XZ_FUNC xz_dec_bcj_reset(
		struct xz_dec_bcj *s, uint8_t id);

/*
 * Decode raw BCJ + LZMA2 stream. This must be used only if there actually is
 * a BCJ filter in the chain. If the chain has only LZMA2, xz_dec_lzma2_run()
 * must be called directly.
 */
XZ_EXTERN enum xz_ret XZ_FUNC xz_dec_bcj_run(struct xz_dec_bcj *s,
		struct xz_dec_lzma2 *lzma2, struct xz_buf *b);

/* Free the memory allocated for the BCJ filters. */
#define xz_dec_bcj_end(s) kfree(s)
#endif

#endif
