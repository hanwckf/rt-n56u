/* vi: set sw=4 ts=4: */
/*
 * cancellation macros for uClibc
 *
 * Copyright (C) 2011 Peter S. Mazinger <ps.m at gmx.net>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _CANCEL_H
#define _CANCEL_H

/*
 * Usage of this header:
 * 1. define a static or hidden function __NC(NAME) - expands to __NAME_nocancel
 * 2. if it is hidden, add the prototype to the appropiate header where NAME has
 * it's prototype (guarded by _LIBC)
 * 3. add a CANCELLABLE_SYSCALL(...) line at the end, this will create the function
 * NAME (as weak) with enabled cancellation for NPTL (and later for new LT), for
 * LT_OLD it will also create a strong_alias to __libc_NAME to be used in libpthread
 * 4. if you need libc_hidden_(weak|def) line, use instead lt_libc_hidden, this will
 * take care of the correct type, weak or strong depending on the THREADS type
 * 5. If the implementation can't be done using CANCELLABLE_SYSCALL (like for fcntl)
 * you need to manually add lt_strong_alias() line too, to optionally create the
 * __libc_NAME alias
 * 6. if functions are needed to implement __NC(NAME), that themselves are cancellable,
 * decide how the cancellation should be solved, two variants are possible:
 *  a. use the other function as __NC(FUNC), this way you access the non-cancellable
 *  variant and provide by CANCELLABLE_SYSCALL(...) the dedicated cancellation for NAME.
 *  be aware, that for this case __NC(FUNC) has to be hidden (not static)
 *  b. use the other function with it's name (FUNC) and add LIBC_CANCEL_HANDLED(); at
 *  the end of file with a comment telling us which function took care of the cancellation
 * Note: LIBC_CANCEL_HANDLED() is noop on uClibc, glibc uses it only for tests, we use
 * it only for "documentation".
 *
 * For now the use of this file is limited to libc, will expand later to support libpthread
 * and librt as well.
 */

#include <features.h>

#ifndef NOT_IN_libc

#define __NC(name) _NC(name)
#define _NC(name) __##name##_nocancel

#define __NC_OLD(name) _NC_OLD(name)
#define _NC_OLD(name) __libc_##name

#define __NC_PROTO(name) extern __typeof(name) __NC(name) attribute_hidden;
#define __NC_OLD_PROTO(name) extern __typeof(name) __NC_OLD(name);

#if defined __UCLIBC_HAS_THREADS__ && !defined __LINUXTHREADS_OLD__
# define __NEW_THREADS 1
#else
# define SINGLE_THREAD_P 1
#endif

#ifdef __NEW_THREADS
# include <sysdep-cancel.h>

# define CANCELLABLE_SYSCALL(res_type, name, param_list, params)	\
res_type weak_function name param_list					\
{									\
	if (SINGLE_THREAD_P)						\
		return __NC(name) params;				\
	int oldtype = LIBC_CANCEL_ASYNC();				\
	res_type result = __NC(name) params;				\
	LIBC_CANCEL_RESET(oldtype);					\
	return result;							\
}

# define lt_strong_alias(name)
# define lt_libc_hidden(name) libc_hidden_def(name)

#elif defined __LINUXTHREADS_OLD__

# define CANCELLABLE_SYSCALL(res_type, name, param_list, params)	\
weak_alias(__NC(name),name)						\
lt_strong_alias(name)

# define lt_strong_alias(name)						\
__NC_OLD_PROTO(name)							\
strong_alias(name,__NC_OLD(name))
# define lt_libc_hidden(name) libc_hidden_weak(name)

#else

# define CANCELLABLE_SYSCALL(res_type, name, param_list, params)	\
strong_alias(__NC(name),name)

# define lt_strong_alias(name)
# define lt_libc_hidden(name) libc_hidden_def(name)

#endif

/* disable it, useless, glibc uses it only for tests */
# undef LIBC_CANCEL_HANDLED
# define LIBC_CANCEL_HANDLED()

#endif /* NOT_IN_libc */

#endif
