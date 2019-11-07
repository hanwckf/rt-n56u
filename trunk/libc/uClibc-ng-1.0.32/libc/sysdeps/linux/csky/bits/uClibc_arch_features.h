/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

/*
 * Track misc arch-specific features that aren't config options
 */

#ifndef _BITS_UCLIBC_ARCH_FEATURES_H
#define _BITS_UCLIBC_ARCH_FEATURES_H

# define __UCLIBC_ABORT_INSTRUCTION__ ".long 0xffffffff"

/* can your target use syscall6() for mmap ? */
#undef __UCLIBC_MMAP_HAS_6_ARGS__

#ifdef __CSKYABIV2__
#undef __UCLIBC_SYSCALL_ALIGN_64BIT__
#else
#define __UCLIBC_SYSCALL_ALIGN_64BIT__
#endif

/* does your target have a broken create_module() ? */
#define __UCLIBC_BROKEN_CREATE_MODULE__

/* does your target have to worry about older [gs]etrlimit() ? */
#define __UCLIBC_HANDLE_OLDER_RLIMIT__

/* does your target have an asm .set ? */
#define __UCLIBC_HAVE_ASM_SET_DIRECTIVE__

/* define if target doesn't like .global */
#undef __UCLIBC_ASM_GLOBAL_DIRECTIVE__

/* define if target supports .weak */
#define __UCLIBC_HAVE_ASM_WEAK_DIRECTIVE__

/* define if target supports .weakext */
#undef __UCLIBC_HAVE_ASM_WEAKEXT_DIRECTIVE__

/* needed probably only for ppc64 */
#undef __UCLIBC_HAVE_ASM_GLOBAL_DOT_NAME__

/* define if target supports IEEE signed zero floats */
#define __UCLIBC_HAVE_SIGNED_ZERO__

/* define if target supports CFI pseudo ops */
#define __UCLIBC_HAVE_ASM_CFI_DIRECTIVES__

#endif /* _BITS_UCLIBC_ARCH_FEATURES_H */
