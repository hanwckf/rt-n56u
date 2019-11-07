/* prototypes for internal crypt functions
 *
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef __LIBCRYPT_H__
#define __LIBCRYPT_H__

extern char *__md5_crypt(const unsigned char *pw, const unsigned char *salt) attribute_hidden;
extern char *__sha256_crypt(const unsigned char *pw, const unsigned char *salt) attribute_hidden;
extern char *__sha512_crypt(const unsigned char *pw, const unsigned char *salt) attribute_hidden;
extern char *__des_crypt(const unsigned char *pw, const unsigned char *salt) attribute_hidden;

extern char *__sha256_crypt_r (const char *key, const char *salt, char *buffer, int buflen) attribute_hidden;
extern char *__sha512_crypt_r (const char *key, const char *salt, char *buffer, int buflen) attribute_hidden;

/* shut up gcc-4.x signed warnings */
#define strcpy(dst,src) strcpy((char*)dst,(char*)src)
#define strlen(s) strlen((char*)s)
#define strncat(dst,src,n) strncat((char*)dst,(char*)src,n)
#define strncmp(s1,s2,n) strncmp((char*)s1,(char*)s2,n)

#endif
