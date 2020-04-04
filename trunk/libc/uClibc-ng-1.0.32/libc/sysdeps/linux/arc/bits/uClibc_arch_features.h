/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/*
 * Track misc arch-specific features that aren't config options
 */

#ifndef _BITS_UCLIBC_ARCH_FEATURES_H
#define _BITS_UCLIBC_ARCH_FEATURES_H

/* instruction used when calling abort() to kill yourself */
#define __UCLIBC_ABORT_INSTRUCTION__ "flag 0"

/* can your target use syscall6() for mmap ? */
#undef __UCLIBC_MMAP_HAS_6_ARGS__

/* does your target have a broken create_module() ? */
#undef __UCLIBC_BROKEN_CREATE_MODULE__

/* does your target have to worry about older [gs]etrlimit() ? */
#undef __UCLIBC_HANDLE_OLDER_RLIMIT__

/* does your target have an asm .set ? */
#define __UCLIBC_HAVE_ASM_SET_DIRECTIVE__

/* define if target supports .weak */
#define __UCLIBC_HAVE_ASM_WEAK_DIRECTIVE__

/* define if target supports .weakext */
#undef __UCLIBC_HAVE_ASM_WEAKEXT_DIRECTIVE__

/* define if target supports CFI pseudo ops */
#undef __UCLIBC_HAVE_ASM_CFI_DIRECTIVES__

/* define if target supports IEEE signed zero floats */
#define __UCLIBC_HAVE_SIGNED_ZERO__

/* The default ';' is a comment on ARC. */
#define __UCLIBC_ASM_LINE_SEP__ `

/* does your target align 64bit values in register pairs ? (32bit arches only)
 *  - ARC700 never had any constraint on reg pairs (even if ABI v3)
 *  - Inital HS ABI (v3: non upstream gcc) had 64-bit data aligned in even-odd
 *     reg pairs (thus allowed reg holes when passing such args to calls)
 *  - Upstream gcc (6.x) HS ABI doesn't have that restriction
 */

#if defined(__A7__) || (__GNUC__ > 4)
#undef __UCLIBC_SYSCALL_ALIGN_64BIT__
#else
#define __UCLIBC_SYSCALL_ALIGN_64BIT__
#endif

#endif /* _BITS_UCLIBC_ARCH_FEATURES_H */
