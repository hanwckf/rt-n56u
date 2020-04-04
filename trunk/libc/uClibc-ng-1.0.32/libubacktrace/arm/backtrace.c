/*
 * Perform stack unwinding by using the _Unwind_Backtrace.
 *
 * User application that wants to use backtrace needs to be
 * compiled with -fasynchronous-unwid-tables option and -rdynamic i
 * to get full symbols printed.
 *
 * Author(s): Khem Raj <raj.khem@gmail.com>
 * - ARM specific implementation of backtrace
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

#include <libgcc_s.h>
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

#ifdef SHARED
static _Unwind_Reason_Code (*unwind_backtrace) (_Unwind_Trace_Fn, void *);
static _Unwind_VRS_Result (*unwind_vrs_get) (_Unwind_Context *,
					     _Unwind_VRS_RegClass,
					     _uw,
					     _Unwind_VRS_DataRepresentation,
					     void *);

static void backtrace_init (void)
{
	void *handle = dlopen (LIBGCC_S_SO, RTLD_LAZY);
	if (handle == NULL
		|| ((unwind_backtrace = dlsym (handle, "_Unwind_Backtrace")) == NULL)
		|| ((unwind_vrs_get = dlsym (handle, "_Unwind_VRS_Get")) == NULL)) {
		printf(LIBGCC_S_SO " must be installed for backtrace to work\n");
		abort();
	}
}
#else
# define unwind_backtrace _Unwind_Backtrace
# define unwind_vrs_get _Unwind_VRS_Get
#endif
/* This function is identical to "_Unwind_GetGR", except that it uses
   "unwind_vrs_get" instead of "_Unwind_VRS_Get".  */
static inline _Unwind_Word
unwind_getgr (_Unwind_Context *context, int regno)
{
  _uw val;
  unwind_vrs_get (context, _UVRSC_CORE, regno, _UVRSD_UINT32, &val);
  return val;
}

/* This macro is identical to the _Unwind_GetIP macro, except that it
   uses "unwind_getgr" instead of "_Unwind_GetGR".  */
#define unwind_getip(context) \
 (unwind_getgr (context, 15) & ~(_Unwind_Word)1)

static _Unwind_Reason_Code
backtrace_helper (struct _Unwind_Context *ctx, void *a)
{
	struct trace_arg *arg = a;

	assert (unwind_getip(ctx) != NULL);

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

#ifdef SHARED
	if (unwind_backtrace == NULL)
		backtrace_init();
#endif

	if (size >= 1)
		unwind_backtrace (backtrace_helper, &arg);

	return arg.cnt != -1 ? arg.cnt : 0;
}
