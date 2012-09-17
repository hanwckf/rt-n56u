/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

wint_t __getwchar_unlocked(void);

#ifdef __DO_UNLOCKED

weak_alias(__getwchar_unlocked,getwchar_unlocked);
#ifndef __UCLIBC_HAS_THREADS__
weak_alias(__getwchar_unlocked,getwchar);
#endif

wint_t __getwchar_unlocked(void)
{
	return __fgetwc_unlocked(stdin);
}

#elif defined __UCLIBC_HAS_THREADS__

wint_t getwchar(void)
{
	return fgetwc(stdin);
}

#endif
