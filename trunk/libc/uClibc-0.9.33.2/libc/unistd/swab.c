#include <unistd.h>
#include <sys/types.h>
#include <byteswap.h>

/* Updated implementation based on byteswap.h from Miles Bader
 * <miles@gnu.org>.  This should be much faster on arches with machine
 * specific, optimized definitions in include/bits/byteswap.h (i.e. on
 * x86, use the bswap instruction on i486 and better boxes).  For
 * platforms that lack such support, this should be no slower than it
 * was before... */
void swab (const void *source, void *dest, ssize_t count)
{
    const unsigned short *from = source, *from_end = from + (count >> 1);
    unsigned short junk;
    unsigned short *to = dest;

    while (from < from_end) {
	/* Don't put '*from++'into the bswap_16() macros
	 * or mad things will happen on macro expansion */
	junk=*from++;
	*to++ = bswap_16 (junk);
    }
}
