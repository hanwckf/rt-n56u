/*
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

.text
.global ___vfork
.hidden ___vfork
.type  ___vfork,STT_FUNC;
.align 4
___vfork:
       p0 = __NR_vfork;
       excpt 0;
       rts;
.size ___vfork,.-___vfork
weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
