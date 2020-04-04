/*
 * Copyright (C) 2006 by Steven J. Hill <sjhill@realitydiluted.com>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */
/* libpthread sets _dl_error_catch_tsd to point to this function.
   We define it here instead of in libpthread so that it doesn't
   need to have a TLS segment of its own just for this one pointer.  */

#include <features.h>

void **__libc_dl_error_tsd(void) __attribute__ ((const));
void ** __attribute__ ((const))
__libc_dl_error_tsd (void)
{
  static __thread void *__tsd_data attribute_tls_model_ie;
  return &__tsd_data;
}
