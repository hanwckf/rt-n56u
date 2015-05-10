/*
 * XZ decoder as a single file for uncompressing the kernel and initramfs
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

/*
 * Important notes about in-place decompression
 *
 * At least on x86, the kernel is decompressed in place: the compressed data
 * is placed to the end of the output buffer, and the decompressor overwrites
 * most of the compressed data. There must be enough safety margin to
 * guarantee that the write position is always behind the read position.
 * The optimal safety margin for XZ with LZMA2 or BCJ+LZMA2 is calculated
 * below. Note that the margin with XZ is bigger than with Deflate (gzip)!
 *
 * The worst case for in-place decompression is that the beginning of
 * the file is compressed extremely well, and the rest of the file is
 * uncompressible. Thus, we must look for worst-case expansion when the
 * compressor is encoding uncompressible data.
 *
 * The structure of the .xz file in case of a compresed kernel is as follows.
 * Sizes (as bytes) of the fields are in parenthesis.
 *
 *    Stream Header (12)
 *    Block Header:
 *      Block Header (8-12)
 *      Compressed Data (N)
 *      Block Padding (0-3)
 *      CRC32 (4)
 *    Index (8-20)
 *    Stream Footer (12)
 *
 * Normally there is exactly one Block, but let's assume that there are
 * 2-4 Blocks just in case. Because Stream Header and also Block Header
 * of the first Block don't make the decompressor produce any uncompressed
 * data, we can ignore them from our calculations. Block Headers of possible
 * additional Blocks have to be taken into account still. With these
 * assumptions, it is safe to assume that the total header overhead is
 * less than 128 bytes.
 *
 * Compressed Data contains LZMA2 or BCJ+LZMA2 encoded data. Since BCJ
 * doesn't change the size of the data, it is enough to calculate the
 * safety margin for LZMA2.
 *
 * LZMA2 stores the data in chunks. Each chunk has a header whose size is
 * at maximum of 6 bytes, but to get round 2^n numbers, let's assume that
 * the maximum chunk header size is 8 bytes. After the chunk header, there
 * may be up to 64 KiB of actual payload in the chunk. Often the payload is
 * quite a bit smaller though; to be safe, let's assume that an average
 * chunk has only 32 KiB of payload.
 *
 * The maximum uncompressed size of the payload is 2 MiB. The minimum
 * uncompressed size of the payload is in practice never less than the
 * payload size itself. The LZMA2 format would allow uncompressed size
 * to be less than the payload size, but no sane compressor creates such
 * files. LZMA2 supports storing uncompressible data in uncompressed form,
 * so there's never a need to create payloads whose uncompressed size is
 * smaller than the compressed size.
 *
 * The assumption, that the uncompressed size of the payload is never
 * smaller than the payload itself, is valid only when talking about
 * the payload as a whole. It is possible that the payload has parts where
 * the decompressor consumes more input than it produces output. Calculating
 * the worst case for this would be tricky. Instead of trying to do that,
 * let's simply make sure that the decompressor never overwrites any bytes
 * of the payload which it is currently reading.
 *
 * Now we have enough information to calculate the safety margin. We need
 *   - 128 bytes for the .xz file format headers;
 *   - 8 bytes per every 32 KiB of uncompressed size (one LZMA2 chunk header
 *     per chunk, each chunk having average payload size of 32 KiB); and
 *   - 64 KiB (biggest possible LZMA2 chunk payload size) to make sure that
 *     the decompressor never overwrites anything from the LZMA2 chunk
 *     payload it is currently reading.
 *
 * We get the following formula:
 *
 *    safety_margin = 128 + uncompressed_size * 8 / 32768 + 65536
 *                  = 128 + (uncompressed_size >> 12) + 65536
 *
 * For comparision, according to arch/x86/boot/compressed/misc.c, the
 * equivalent formula for Deflate is this:
 *
 *    safety_margin = 18 + (uncompressed_size >> 12) + 32768
 *
 * Thus, when updating Deflate-only in-place kernel decompressor to
 * support XZ, the fixed overhead has to be increased from 18+32768 bytes
 * to 128+65536 bytes.
 */

/*
 * STATIC is defined to "static" if we are being built for kernel
 * decompression (pre-boot code). <linux/decompress/mm.h> below will
 * define STATIC to empty if it wasn't already defined. Since we will
 * need to know if we are being used for kernel decompression, we define
 * XZ_PREBOOT here.
 */

#define XZ_PREBOOT

#define STATIC static

/*
 * Set the linkage of normally extern functions to static. The only
 * function that we might make extern is unxz(), and even that will
 * depend on the STATIC macro.
 */
#define XZ_EXTERN static

/*
 * Use INIT defined in <linux/decompress/mm.h> to possibly add __init
 * to every function.
 */
#define XZ_FUNC

/*
 * Use the internal CRC32 code instead of kernel's CRC32 module, which
 * is not available in early phase of booting.
 */
#define XZ_INTERNAL_CRC32 1

/*
 * Ignore the configuration specified in the kernel config for the xz_dec
 * module. For boot time use, we enable only the BCJ filter of the current
 * architecture, or none if no BCJ filter is available for the architecture.
 */
#define XZ_IGNORE_KCONFIG
#ifdef CONFIG_X86
#	define XZ_DEC_X86
#endif
#ifdef CONFIG_PPC
#	define XZ_DEC_POWERPC
#endif
#ifdef CONFIG_ARM
#	define XZ_DEC_ARM
#endif
#ifdef CONFIG_IA64
#	define XZ_DEC_IA64
#endif
#ifdef CONFIG_SPARC
#	define XZ_DEC_SPARC
#endif

#include "xz_private.h"

#ifdef XZ_PREBOOT
/*
 * Replace the normal allocation functions with the versions
 * from <linux/decompress/mm.h>.
 */
#undef kmalloc
#undef kfree
#undef vmalloc
#undef vfree
#define kmalloc(size, flags) malloc(size)
#define kfree(ptr) free(ptr)
#define vmalloc(size) malloc(size)
#define vfree(ptr) free(ptr)

#endif /* XZ_PREBOOT */

#include "xz_dec_stream.c"
#include "xz_dec_lzma2.c"
#include "xz_dec_bcj.c"

/*
 * Maximum LZMA2 dictionary size. This matters only in multi-call mode.
 * If you change this, remember to update also the error message in
 * "case XZ_MEMLIMIT_ERROR".
 */
#define DICT_MAX (1024 * 1024)

/* Size of the input and output buffers in multi-call mode */
#define XZ_IOBUF_SIZE 4096

/*
 * This function implements the API defined in <linux/decompress/generic.h>.
 *
 * This wrapper will automatically choose single-call or multi-call mode
 * of the native XZ decoder API. The single-call mode can be used only when
 * both input and output buffers are available as a single chunk, i.e. when
 * fill() and flush() won't be used.
 *
 * This API doesn't provide a way to specify the maximum dictionary size
 * for the multi-call mode of the native XZ decoder API. We will use
 * DICT_MAX bytes, which will be allocated with vmalloc().
 */
int XZ_FUNC unxz(/*const*/ unsigned char *in, int in_size,
		unsigned char *out, int *in_used)
{
	struct xz_buf b;
	struct xz_dec *s;
	enum xz_ret ret;

	if (in != NULL && out != NULL)
		s = xz_dec_init(XZ_SINGLE, 0);
	else
		return -1;

	if (s == NULL)
		return -1;

	b.in = in;
	b.in_pos = 0;
	b.in_size = in_size;
	b.out_pos = 0;

	if (in_used != NULL)
		*in_used = 0;

	b.out = out;
	b.out_size = (size_t)-1;
	ret = xz_dec_run(s, &b);

	if (in_used != NULL)
		*in_used += b.in_pos;

	xz_dec_end(s);

	return ret==XZ_STREAM_END?0:ret;
}
