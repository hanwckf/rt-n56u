/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sysdep.h>

PSEUDO_ERRVAL(__csky_clone, clone, 5)
  cmpnei a0, 0
  bf start_thread
  rts

start_thread:
#ifdef  __CSKYABIV2__
  subi	sp, 4
#endif
  ld.w	a0, (sp, 0x4)
  ld.w	a1, (sp, 0x0)
  addi	sp, 8
  jsr   a1

  DO_CALL(exit, 0)

END(__csky_clone)

