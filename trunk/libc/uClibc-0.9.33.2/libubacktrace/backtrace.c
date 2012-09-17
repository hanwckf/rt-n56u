/*
 * Perform stack unwinding by using the _Unwind_Backtrace.
 *
 * User application that wants to use backtrace needs to be
 * compiled with -fasynchronous-unwind-tables option and -rdynamic to get full
 * symbols printed.
 *
 * Copyright (C) 2009, 2010 STMicroelectronics Ltd.
 *
 * Author(s): Giuseppe Cavallaro <peppe.cavallaro@st.com>
 * - Initial implementation for glibc
 *
 * Author(s): Carmelo Amoroso <carmelo.amoroso@st.com>
 * - Reworked for uClibc
 *   - use dlsym/dlopen from libdl
 *   - rewrite initialisation to not use libc_once
 *   - make it available in static link too
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

#include <execinfo.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unwind.h>
#include <assert.h>
#include <stdio.h>

struct trace_arg
{
  void **array;
  int cnt, size;
};

static _Unwind_Reason_Code (*unwind_backtrace) (_Unwind_Trace_Fn, void *);
static _Unwind_Ptr (*unwind_getip) (struct _Unwind_Context *);

static void backtrace_init (void)
{
	void *handle = dlopen ("libgcc_s.so.1", RTLD_LAZY);

	if (handle == NULL
		|| ((unwind_backtrace = dlsym (handle, "_Unwind_Backtrace")) == NULL)
		|| ((unwind_getip = dlsym (handle, "_Unwind_GetIP")) == NULL)) {
		printf("libgcc_s.so.1 must be installed for backtrace to work\n");
		abort();
	}
}

static _Unwind_Reason_Code
backtrace_helper (struct _Unwind_Context *ctx, void *a)
{
	struct trace_arg *arg = a;

	assert (unwind_getip != NULL);

	/* We are first called with address in the __backtrace function. Skip it. */
	if (arg->cnt != -1)
		arg->array[arg->cnt] = (void *) unwind_getip (ctx);
	if (++arg->cnt == arg->size)
		return _URC_END_OF_STACK;
	return _URC_NO_REASON;
}

/*
 * Perform stack unwinding by using the _Unwind_Backtrace.
 *
 */
int backtrace (void **array, int size)
{
	struct trace_arg arg = { .array = array, .size = size, .cnt = -1 };

	if (unwind_backtrace == NULL)
		backtrace_init();

	if (size >= 1)
		unwind_backtrace (backtrace_helper, &arg);

	return arg.cnt != -1 ? arg.cnt : 0;
}
