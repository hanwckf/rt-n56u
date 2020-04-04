/*
 * cacheflush syscall for SUPERH
 *
 * Copyright (C) 2009 STMicroelectronics Ltd
 * Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <sys/syscall.h>

#ifdef __NR_cacheflush
int cacheflush(void *addr, const int nbytes, int op);
_syscall3(int, cacheflush, void *, addr, const int, nbytes, const int, op)
#endif
