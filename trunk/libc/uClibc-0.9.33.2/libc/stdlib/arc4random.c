/*	$$$: arc4random.c 2005/02/08 robert */
/*	$NetBSD: arc4random.c,v 1.5.2.1 2004/03/26 22:52:50 jmc Exp $	*/
/*	$OpenBSD: arc4random.c,v 1.6 2001/06/05 05:05:38 pvalchev Exp $	*/

/*
 * Arc4 random number generator for OpenBSD.
 * Copyright 1996 David Mazieres <dm@lcs.mit.edu>.
 *
 * Modification and redistribution in source and binary forms is
 * permitted provided that due credit is given to the author and the
 * OpenBSD project by leaving this copyright notice intact.
 */

/*
 * This code is derived from section 17.1 of Applied Cryptography,
 * second edition, which describes a stream cipher allegedly
 * compatible with RSA Labs "RC4" cipher (the actual description of
 * which is a trade secret).  The same algorithm is used as a stream
 * cipher called "arcfour" in Tatu Ylonen's ssh package.
 *
 * Here the stream cipher has been modified always to include the time
 * when initializing the state.  That makes it impossible to
 * regenerate the same random sequence twice, so this can't be used
 * for encryption, but will generate good random numbers.
 *
 * RC4 is a registered trademark of RSA Laboratories.
 */

#include <features.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#ifdef __ARC4RANDOM_USE_ERANDOM__
#include <sys/sysctl.h>
#endif


struct arc4_stream {
	uint8_t i;
	uint8_t j;
	uint8_t s[256];
};

static int    rs_initialized;
static struct arc4_stream rs;

static __inline__ void arc4_init(struct arc4_stream *);
static __inline__ void arc4_addrandom(struct arc4_stream *, u_char *, int);
static void arc4_stir(struct arc4_stream *);
static __inline__ uint8_t arc4_getbyte(struct arc4_stream *);
static __inline__ uint32_t arc4_getword(struct arc4_stream *);

static __inline__ void
arc4_init(struct arc4_stream *as)
{
	int     n;

	for (n = 0; n < 256; n++)
		as->s[n] = n;
	as->i = 0;
	as->j = 0;
}

static __inline__ void
arc4_addrandom(struct arc4_stream *as, u_char *dat, int datlen)
{
	int     n;
	uint8_t si;

	as->i--;
	for (n = 0; n < 256; n++) {
		as->i = (as->i + 1);
		si = as->s[as->i];
		as->j = (as->j + si + dat[n % datlen]);
		as->s[as->i] = as->s[as->j];
		as->s[as->j] = si;
	}
	as->j = as->i;
}

static void
arc4_stir(struct arc4_stream *as)
{
	int     fd;
	struct {
		struct timeval tv;
		uint rnd[(128 - sizeof(struct timeval)) / sizeof(uint)];
	}       rdat;
	int	n;

	gettimeofday(&rdat.tv, NULL);
	fd = open("/dev/urandom", O_RDONLY);
	if (fd != -1) {
		read(fd, rdat.rnd, sizeof(rdat.rnd));
		close(fd);
	}
#ifdef __ARC4RANDOM_USE_ERANDOM__
	else {
		int mib[3];
		uint i;
		size_t len;

		/* Device could not be opened, we might be chrooted, take
		 * randomness from sysctl. */

		mib[0] = CTL_KERN;
		mib[1] = KERN_RANDOM;
		mib[2] = RANDOM_ERANDOM;

		for (i = 0; i < sizeof(rdat.rnd) / sizeof(uint); i++) {
			len = sizeof(uint);
			if (sysctl(mib, 3, &rdat.rnd[i], &len, NULL, 0) == -1)
				break;
		}
	}
#endif

	arc4_addrandom(as, (void *) &rdat, sizeof(rdat));

	/*
	 * Throw away the first N words of output, as suggested in the
	 * paper "Weaknesses in the Key Scheduling Algorithm of RC4"
	 * by Fluher, Mantin, and Shamir.
	 * http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
	 * N = 256 in our case.
	 */
	for (n = 0; n < 256 * 4; n++)
		arc4_getbyte(as);
}

static __inline__ uint8_t
arc4_getbyte(struct arc4_stream *as)
{
	uint8_t si, sj;

	as->i = (as->i + 1);
	si = as->s[as->i];
	as->j = (as->j + si);
	sj = as->s[as->j];
	as->s[as->i] = sj;
	as->s[as->j] = si;
	return (as->s[(si + sj) & 0xff]);
}

static __inline__ uint32_t
arc4_getword(struct arc4_stream *as)
{
	uint32_t val;
	val = arc4_getbyte(as) << 24;
	val |= arc4_getbyte(as) << 16;
	val |= arc4_getbyte(as) << 8;
	val |= arc4_getbyte(as);
	return val;
}

static void
__arc4random_stir(void)
{
	if (!rs_initialized) {
		arc4_init(&rs);
		rs_initialized = 1;
	}
	arc4_stir(&rs);
}
strong_alias(__arc4random_stir,arc4random_stir)

void
arc4random_addrandom(u_char *dat, int datlen)
{
	if (!rs_initialized)
		__arc4random_stir();
	arc4_addrandom(&rs, dat, datlen);
}

uint32_t
arc4random(void)
{
	if (!rs_initialized)
		__arc4random_stir();
	return arc4_getword(&rs);
}

#if 0
/*-------- Test code --------*/
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int random_number;
    random_number = arc4random() % 65536;
    printf("%d\n", random_number);
    return 0;
}
#endif
