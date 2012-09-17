/* memmove implementation for SH4
 *
 * Copyright (C) 2009 STMicroelectronics Ltd.
 *
 * Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef __SH_FPU_ANY__
#include "../../generic/memmove.c"
#else

#include <string.h>

#define FPSCR_SR	(1 << 20)
#define STORE_FPSCR(x)	__asm__ __volatile__("sts fpscr, %0" : "=r"(x))
#define LOAD_FPSCR(x)	__asm__ __volatile__("lds %0, fpscr" : : "r"(x))

static void fpu_optimised_copy_fwd(void *dest, const void *src, size_t len)
{
	char *d = (char *)dest;
	char *s = (char *)src;

	if (len >= 64) {
		unsigned long fpscr;
		int *s1;
		int *d1;

		/* Align the dest to 4 byte boundary. */
		while ((unsigned)d & 0x7) {
			*d++ = *s++;
			len--;
		}

		s1 = (int *)s;
		d1 = (int *)d;

		/* check if s is well aligned to use FPU */
		if (!((unsigned)s1 & 0x7)) {

			/* Align the dest to cache-line boundary */
			while ((unsigned)d1 & 0x1c) {
				*d1++ = *s1++;
				len -= 4;
			}

			/* Use paired single precision load or store mode for
			 * 64-bit tranfering.*/
			STORE_FPSCR(fpscr);
			LOAD_FPSCR(FPSCR_SR);

			while (len >= 32) {
				__asm__ __volatile__ ("fmov @%0+,dr0":"+r" (s1));
				__asm__ __volatile__ ("fmov @%0+,dr2":"+r" (s1));
				__asm__ __volatile__ ("fmov @%0+,dr4":"+r" (s1));
				__asm__ __volatile__ ("fmov @%0+,dr6":"+r" (s1));
				__asm__
				    __volatile__ ("fmov dr0,@%0"::"r"
					      (d1):"memory");
				d1 += 2;
				__asm__
				    __volatile__ ("fmov dr2,@%0"::"r"
					      (d1):"memory");
				d1 += 2;
				__asm__
				    __volatile__ ("fmov dr4,@%0"::"r"
					      (d1):"memory");
				d1 += 2;
				__asm__
				    __volatile__ ("fmov dr6,@%0"::"r"
					      (d1):"memory");
				d1 += 2;
				len -= 32;
			}
			LOAD_FPSCR(fpscr);
		}
		s = (char *)s1;
		d = (char *)d1;
		/*TODO: other subcases could be covered here?!?*/
	}
	/* Go to per-byte copy */
	while (len > 0) {
		*d++ = *s++;
		len--;
	}
	return;
}

void *memmove(void *dest, const void *src, size_t len)
{
	unsigned long int d = (long int)dest;
	unsigned long int s = (long int)src;
	unsigned long int res;

	if (d >= s)
		res = d - s;
	else
		res = s - d;
	/*
	 * 1) dest and src are not overlap  ==> memcpy (BWD/FDW)
	 * 2) dest and src are 100% overlap ==> memcpy (BWD/FDW)
	 * 3) left-to-right overlap ==>  Copy from the beginning to the end
	 * 4) right-to-left overlap ==>  Copy from the end to the beginning
	 */

	if (res == 0)		/* 100% overlap */
		memcpy(dest, src, len);	/* No overlap */
	else if (res >= len)
		memcpy(dest, src, len);
	else {
		if (d > s)	/* right-to-left overlap */
			memcpy(dest, src, len);	/* memcpy is BWD */
		else		/* cannot use SH4 memcpy for this case */
			fpu_optimised_copy_fwd(dest, src, len);
	}
	return (dest);
}

libc_hidden_def(memmove)
#endif /*__SH_FPU_ANY__ */
