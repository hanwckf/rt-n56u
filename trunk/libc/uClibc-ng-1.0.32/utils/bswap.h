/*
 * Lame bswap replacements as we can't assume the host is sane and provides
 * working versions of these.
 */

#ifndef _BSWAP_H
#define	_BSWAP_H 1

#ifdef __linux__
# include <byteswap.h>
#else

static __inline__ uint16_t bswap_16(uint16_t x)
{
	return ((((x) & 0xff00) >> 8) | \
	        (((x) & 0x00ff) << 8));
}
static __inline__ uint32_t bswap_32(uint32_t x)
{
	return ((((x) & 0xff000000) >> 24) | \
	        (((x) & 0x00ff0000) >>  8) | \
	        (((x) & 0x0000ff00) <<  8) | \
	        (((x) & 0x000000ff) << 24));
}
static __inline__ uint64_t bswap_64(uint64_t x)
{
#define _uswap_64(x, sfx) \
	return ((((x) & 0xff00000000000000##sfx) >> 56) | \
	        (((x) & 0x00ff000000000000##sfx) >> 40) | \
	        (((x) & 0x0000ff0000000000##sfx) >> 24) | \
	        (((x) & 0x000000ff00000000##sfx) >>  8) | \
	        (((x) & 0x00000000ff000000##sfx) <<  8) | \
	        (((x) & 0x0000000000ff0000##sfx) << 24) | \
	        (((x) & 0x000000000000ff00##sfx) << 40) | \
	        (((x) & 0x00000000000000ff##sfx) << 56));
#if defined(__GNUC__)
	_uswap_64(x, ull)
#else
	_uswap_64(x, )
#endif
#undef _uswap_64
}
#endif

#endif
