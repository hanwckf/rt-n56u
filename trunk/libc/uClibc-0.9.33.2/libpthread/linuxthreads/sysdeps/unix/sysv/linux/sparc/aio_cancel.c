#include <shlib-compat.h>

#define aio_cancel64 XXX
#include <aio.h>
#undef aio_cancel64
#include <errno.h>

extern __typeof (aio_cancel) __new_aio_cancel;
extern __typeof (aio_cancel) __old_aio_cancel;

#define aio_cancel	__new_aio_cancel

#include <sysdeps/pthread/aio_cancel.c>

#undef aio_cancel
strong_alias (__new_aio_cancel, __new_aio_cancel64);
versioned_symbol (librt, __new_aio_cancel, aio_cancel, GLIBC_2_3);
versioned_symbol (librt, __new_aio_cancel64, aio_cancel64, GLIBC_2_3);

#if SHLIB_COMPAT (librt, GLIBC_2_1, GLIBC_2_3)

#undef ECANCELED
#define aio_cancel	__old_aio_cancel
#define ECANCELED	125

#include <sysdeps/pthread/aio_cancel.c>

#undef aio_cancel
strong_alias (__old_aio_cancel, __old_aio_cancel64);
compat_symbol (librt, __old_aio_cancel, aio_cancel, GLIBC_2_1);
compat_symbol (librt, __old_aio_cancel64, aio_cancel64, GLIBC_2_1);

#endif
