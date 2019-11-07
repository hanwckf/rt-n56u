/*
 * Track misc arch-specific features that aren't config options
 */

#ifndef _BITS_UCLIBC_ARCH_FEATURES_H
#define _BITS_UCLIBC_ARCH_FEATURES_H

/* instruction used when calling abort() to kill yourself */
#define __UCLIBC_ABORT_INSTRUCTION__ "bl abort"

/* can your target use syscall6() for mmap ? */
#undef __UCLIBC_MMAP_HAS_6_ARGS__

/* does your target align 64bit values in register pairs ? (32bit arches only) */
#ifdef __ARM_EABI__
#define __UCLIBC_SYSCALL_ALIGN_64BIT__
#else
#undef __UCLIBC_SYSCALL_ALIGN_64BIT__
#endif

/* does your target have a broken create_module() ? */
#define __UCLIBC_BROKEN_CREATE_MODULE__

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

/* only weird assemblers generally need this */
#undef __UCLIBC_ASM_LINE_SEP__


/* The __ARM_ARCH define is provided by gcc 4.8.  Construct it otherwise.  */
#ifndef __ARM_ARCH
# ifdef __ARM_ARCH_2__
#  define __ARM_ARCH 2
# elif defined (__ARM_ARCH_3__) || defined (__ARM_ARCH_3M__)
#  define __ARM_ARCH 3
# elif defined (__ARM_ARCH_4__) || defined (__ARM_ARCH_4T__)
#  define __ARM_ARCH 4
# elif defined (__ARM_ARCH_5__) || defined (__ARM_ARCH_5E__) \
       || defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) \
       || defined(__ARM_ARCH_5TEJ__)
#  define __ARM_ARCH 5
# elif defined (__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
       || defined (__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) \
       || defined (__ARM_ARCH_6K__) || defined(__ARM_ARCH_6T2__)
#  define __ARM_ARCH 6
# elif defined (__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) \
       || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) \
       || defined(__ARM_ARCH_7EM__)
#  define __ARM_ARCH 7
# else
#  error unknown arm architecture
# endif
#endif

#ifdef __GNUC__
# define __need_uClibc_config_h
# include <bits/uClibc_config.h>
# undef __need_uClibc_config_h
# if defined __CONFIG_ARM_EABI__ && !defined __ARM_EABI__
#  error Your toolchain does not support EABI
# elif !defined __CONFIG_ARM_EABI__ && defined __ARM_EABI__
#  error Your toolchain was built for EABI, but you have chosen OABI
# endif
#endif

#endif /* _BITS_UCLIBC_ARCH_FEATURES_H */
