/*
 * Thread-local storage handling in statically linked binaries.
 * Copyright (C) 2009 Free Software Foundation, Inc.
 *
 * Based on GNU C Library (file: libc/sysdeps/sh/libc-tls.c)
 *
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 * Copyright (C) 2010 STMicroelectronics Ltd.
 *
 * Author: Filippo Arcidiacono <filippo.arcidiacono@st.com>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdeps/generic/libc-tls.c>
#include <dl-tls.h>

#if defined(USE_TLS) && USE_TLS

void *
__tls_get_addr (tls_index *ti)
{
  dtv_t *dtv = THREAD_DTV ();
  return (char *) dtv[1].pointer.val + ti->ti_offset;
}

#endif
