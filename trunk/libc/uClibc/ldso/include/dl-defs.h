/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

#ifndef _LD_DEFS_H
#define _LD_DEFS_H

#define FLAG_ANY             -1
#define FLAG_TYPE_MASK       0x00ff
#define FLAG_LIBC4           0x0000
#define FLAG_ELF             0x0001
#define FLAG_ELF_LIBC5       0x0002
#define FLAG_ELF_LIBC6       0x0003
#define FLAG_ELF_UCLIBC      0x0004
#define FLAG_REQUIRED_MASK   0xff00
#define FLAG_SPARC_LIB64     0x0100
#define FLAG_IA64_LIB64      0x0200
#define FLAG_X8664_LIB64     0x0300
#define FLAG_S390_LIB64      0x0400
#define FLAG_POWERPC_LIB64   0x0500
#define FLAG_MIPS64_LIBN32   0x0600
#define FLAG_MIPS64_LIBN64   0x0700

#define LIB_ANY	     -1
#define LIB_DLL       0
#define LIB_ELF       1
#define LIB_ELF64     0x80
#define LIB_ELF_LIBC5 2
#define LIB_ELF_LIBC6 3
#define LIB_ELF_LIBC0 4

#if defined(__LDSO_PRELOAD_FILE_SUPPORT__) || defined(__LDSO_CACHE_SUPPORT__)
#ifndef __LDSO_BASE_FILENAME__
#define __LDSO_BASE_FILENAME__ "ld.so"
#endif
#define LDSO_BASE_PATH UCLIBC_RUNTIME_PREFIX "etc/" __LDSO_BASE_FILENAME__

#ifdef __LDSO_PRELOAD_FILE_SUPPORT__
#define LDSO_PRELOAD LDSO_BASE_PATH ".preload"
#endif

#ifdef __LDSO_CACHE_SUPPORT__
#define LDSO_CONF    LDSO_BASE_PATH ".conf"
#define LDSO_CACHE   LDSO_BASE_PATH ".cache"

#define LDSO_CACHE_MAGIC "ld.so-"
#define LDSO_CACHE_MAGIC_LEN (sizeof LDSO_CACHE_MAGIC -1)
#define LDSO_CACHE_VER "1.7.0"
#define LDSO_CACHE_VER_LEN (sizeof LDSO_CACHE_VER -1)

typedef struct {
	char magic   [LDSO_CACHE_MAGIC_LEN];
	char version [LDSO_CACHE_VER_LEN];
	int nlibs;
} header_t;

typedef struct {
	int flags;
	int sooffset;
	int liboffset;
} libentry_t;
#endif	/* __LDSO_CACHE_SUPPORT__ */

#endif

#endif	/* _LD_DEFS_H */
