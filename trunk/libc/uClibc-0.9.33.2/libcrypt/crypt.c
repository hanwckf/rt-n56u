/* vi: set sw=4 ts=4: */
/*
 * crypt() for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define __FORCE_GLIBC
#include <unistd.h>
#include <crypt.h>
#include "libcrypt.h"

char *crypt(const char *key, const char *salt)
{
	const unsigned char *ukey = (const unsigned char *)key;
	const unsigned char *usalt = (const unsigned char *)salt;

	if (salt[0] == '$' && salt[2] == '$') {
		if (*++salt == '1')
			return __md5_crypt(ukey, usalt);
#ifdef __UCLIBC_HAS_SHA256_CRYPT_IMPL__
		else if (*salt == '5')
			return __sha256_crypt(ukey, usalt);
#endif
#ifdef __UCLIBC_HAS_SHA512_CRYPT_IMPL__
		else if (*salt == '6')
			return __sha512_crypt(ukey, usalt);
#endif
	}
	return __des_crypt(ukey, usalt);
}
