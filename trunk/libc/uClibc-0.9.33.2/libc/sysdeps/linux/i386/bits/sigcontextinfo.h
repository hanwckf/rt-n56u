/* Copyright (C) 1998, 1999, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define SIGCONTEXT struct sigcontext
#define SIGCONTEXT_EXTRA_ARGS
#define GET_PC(ctx)	((void *) ctx.eip)
#define GET_FRAME(ctx)	((void *) ctx.ebp)
#define GET_STACK(ctx)	((void *) ctx.esp_at_signal)
#define CALL_SIGHANDLER(handler, signo, ctx) \
do {									      \
  int __tmp1, __tmp2, __tmp3, __tmp4;					      \
  __asm__ __volatile__ ("movl\t%%esp, %%edi\n\t"				      \
		    "andl\t$-16, %%esp\n\t"				      \
		    "subl\t%8, %%esp\n\t"				      \
		    "movl\t%%edi, %c8-4(%%esp)\n\t"			      \
		    "movl\t%1, 0(%%esp)\n\t"				      \
		    "leal\t4(%%esp), %%edi\n\t"				      \
		    "cld\n\t"						      \
		    "rep\tmovsl\n\t"					      \
		    "call\t*%0\n\t"					      \
		    "cld\n\t"						      \
		    "movl\t%9, %%ecx\n\t"				      \
		    "subl\t%%edi, %%esi\n\t"				      \
		    "leal\t4(%%esp,%%esi,1), %%edi\n\t"			      \
		    "leal\t4(%%esp), %%esi\n\t"				      \
		    "rep\tmovsl\n\t"					      \
		    "movl\t%c8-4(%%esp), %%esp\n\t"			      \
		    : "=a" (__tmp1), "=d" (__tmp2), "=S" (__tmp3),	      \
		      "=c" (__tmp4)					      \
		    : "0" (handler), "1" (signo), "2" (&ctx),		      \
		      "3" (sizeof (struct sigcontext) / 4),		      \
		      "n" ((sizeof (struct sigcontext) + 19) & ~15),	      \
		      "i" (sizeof (struct sigcontext) / 4)		      \
		    : "cc", "edi");					      \
} while (0)
