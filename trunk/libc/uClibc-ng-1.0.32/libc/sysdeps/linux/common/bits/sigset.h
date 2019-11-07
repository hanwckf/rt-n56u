/* __sig_atomic_t, __sigset_t, and related definitions.  Linux version.
   Copyright (C) 1991, 1992, 1994, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef	_SIGSET_H_types
# define _SIGSET_H_types	1

typedef int __sig_atomic_t;

/* A 'sigset_t' has a bit for each signal.
 * glibc has space for 1024 signals (!), but most arches supported
 * by Linux have 64 signals, and only MIPS has 128.
 * There seems to be some historical baggage in sparc[64]
 * where they might have (or had in the past) 32 signals only,
 * I hope it's irrelevant now.
 * Signal 0 does not exist, so we have signals 1..64, not 0..63.
 * In uclibc, kernel and userspace sigset_t is always the same.
 * BTW, struct sigaction is also the same on kernel and userspace side.
 */
#if defined(__mips__)
# define _SIGSET_NWORDS	(128 / (8 * sizeof (unsigned long)))
#else
# define _SIGSET_NWORDS	(64 / (8 * sizeof (unsigned long)))
#endif
typedef struct {
	unsigned long __val[_SIGSET_NWORDS];
} __sigset_t;

#endif


/* We only want to define these functions if <signal.h> was actually
   included; otherwise we were included just to define the types.  Since we
   are namespace-clean, it wouldn't hurt to define extra macros.  But
   trouble can be caused by functions being defined (e.g., any global
   register vars declared later will cause compilation errors).  */

#if !defined _SIGSET_H_fns && defined _SIGNAL_H
# define _SIGSET_H_fns 1

# ifndef _EXTERN_INLINE
#  define _EXTERN_INLINE __extern_inline
# endif

/* Return a mask that includes the bit for SIG only.  */
/* Unsigned cast ensures shift/mask insns are used.  */
# define __sigmask(sig) \
  (((unsigned long) 1) << ((unsigned)((sig) - 1) % (8 * sizeof (unsigned long))))

/* Return the word index for SIG.  */
# define __sigword(sig)	((unsigned)((sig) - 1) / (8 * sizeof (unsigned long)))

/* gcc 4.3.1 is not clever enough to optimize for _SIGSET_NWORDS == 1 and 2,
 * which are about the only values which can be there */

# if defined __GNUC__ && __GNUC__ >= 2
#  define __sigemptyset(set) \
(__extension__ ({ \
	sigset_t *__set = (set);					\
	if (_SIGSET_NWORDS <= 2) {					\
		__set->__val[0] = 0;					\
		if (_SIGSET_NWORDS == 2)				\
			__set->__val[1] = 0;				\
	} else {							\
		int __cnt = _SIGSET_NWORDS;				\
		while (--__cnt >= 0) __set->__val[__cnt] = 0;		\
	}								\
	0;								\
}))
#  define __sigfillset(set) \
(__extension__ ({ \
	sigset_t *__set = (set);					\
	if (_SIGSET_NWORDS <= 2) {					\
		__set->__val[0] = ~0UL;					\
		if (_SIGSET_NWORDS == 2)				\
			__set->__val[1] = ~0UL;				\
	} else {							\
		int __cnt = _SIGSET_NWORDS;				\
		while (--__cnt >= 0) __set->__val[__cnt] = ~0UL;	\
	}								\
	0;								\
}))

#  ifdef __USE_GNU
/* The POSIX does not specify for handling the whole signal set in one
   command.  This is often wanted and so we define three more functions
   here.  */
#   define __sigisemptyset(set) \
(__extension__ ({ \
	long __ret;							\
	const sigset_t *__set = (set);					\
	if (_SIGSET_NWORDS == 1) {					\
		__ret = __set->__val[0];				\
	} else if (_SIGSET_NWORDS == 2) {				\
		__ret = __set->__val[0] | __set->__val[1];		\
	} else {							\
		int __cnt = _SIGSET_NWORDS;				\
		__ret = __set->__val[--__cnt];				\
		while (!__ret && --__cnt >= 0)				\
			__ret = __set->__val[__cnt];			\
	}								\
	__ret == 0;							\
}))
#   define __sigandset(dest, left, right) \
(__extension__ ({ \
	sigset_t *__dest = (dest);					\
	const sigset_t *__left = (left);				\
	const sigset_t *__right = (right);				\
	if (_SIGSET_NWORDS <= 2) {					\
		__dest->__val[0] = __left->__val[0] & __right->__val[0];\
		if (_SIGSET_NWORDS == 2)				\
			__dest->__val[1] = __left->__val[1] & __right->__val[1];\
	} else {							\
		int __cnt = _SIGSET_NWORDS;				\
		while (--__cnt >= 0)					\
			__dest->__val[__cnt] = (__left->__val[__cnt]	\
					& __right->__val[__cnt]);	\
	}								\
	0;								\
}))
#   define __sigorset(dest, left, right) \
(__extension__ ({ \
	sigset_t *__dest = (dest);					\
	const sigset_t *__left = (left);				\
	const sigset_t *__right = (right);				\
	if (_SIGSET_NWORDS <= 2) {					\
		__dest->__val[0] = __left->__val[0] | __right->__val[0];\
		if (_SIGSET_NWORDS == 2)				\
			__dest->__val[1] = __left->__val[1] | __right->__val[1];\
	} else {							\
		int __cnt = _SIGSET_NWORDS;				\
		while (--__cnt >= 0)					\
			__dest->__val[__cnt] = (__left->__val[__cnt]	\
					| __right->__val[__cnt]);	\
	}								\
	0;								\
}))
#  endif
# endif

/* These functions needn't check for a bogus signal number -- error
   checking is done in the non __ versions.  */

extern int __sigismember (const __sigset_t *, int);
libc_hidden_proto(__sigismember)
extern void __sigaddset (__sigset_t *, int);
libc_hidden_proto(__sigaddset)
extern void __sigdelset (__sigset_t *, int);
libc_hidden_proto(__sigdelset)

# ifdef __USE_EXTERN_INLINES
#  define __SIGSETFN(RET_TYPE, NAME, BODY, CONST)			\
_EXTERN_INLINE RET_TYPE							\
NAME (CONST __sigset_t *__set, int __sig)				\
{									\
	unsigned long __mask = __sigmask (__sig);			\
	unsigned __word = __sigword (__sig);				\
	BODY;							\
}

__SIGSETFN (int, __sigismember, return (__set->__val[__word] & __mask) ? 1 : 0,
	const)
__SIGSETFN (void, __sigaddset, (__set->__val[__word] |= __mask), )
__SIGSETFN (void, __sigdelset, (__set->__val[__word] &= ~__mask), )

#  undef __SIGSETFN
# endif

# ifdef _LIBC
/* It's far too much PITA to __USE_EXTERN_INLINES from within libc.
 * Especially since we want to inline only calls with const sig,
 * but __USE_EXTERN_INLINES will inline all calls!
 */
static __always_inline unsigned long
const_sigismember(const __sigset_t *set, int sig)
{
	unsigned long mask = __sigmask(sig);
	unsigned word = __sigword(sig);
	return (set->__val[word] & mask);
}
#  define __sigismember(set, sig) \
	(__builtin_constant_p(sig) ? (const_sigismember(set, sig) != 0) : __sigismember(set, sig))
static __always_inline void
const_sigaddset(__sigset_t *set, int sig)
{
	unsigned long mask = __sigmask(sig);
	unsigned word = __sigword(sig);
	set->__val[word] |= mask;
}
#  define __sigaddset(set, sig) \
	(__builtin_constant_p(sig) ? const_sigaddset(set, sig)  : __sigaddset(set, sig))
static __always_inline void
const_sigdelset(__sigset_t *set, int sig)
{
	unsigned long mask = __sigmask(sig);
	unsigned word = __sigword(sig);
	set->__val[word] &= ~mask;
}
#  define __sigdelset(set, sig) \
	(__builtin_constant_p(sig) ? const_sigdelset(set, sig) : __sigdelset(set, sig))
# endif

#endif /* ! _SIGSET_H_fns.  */
