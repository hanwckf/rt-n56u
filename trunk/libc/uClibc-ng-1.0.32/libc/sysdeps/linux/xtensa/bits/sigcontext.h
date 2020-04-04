/* Copyright (C) 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

#ifndef _BITS_SIGCONTEXT_H
#define _BITS_SIGCONTEXT_H 1

struct sigcontext
{
  unsigned long sc_pc;
  unsigned long sc_ps;
  unsigned long sc_lbeg;
  unsigned long sc_lend;
  unsigned long sc_lcount;
  unsigned long sc_sar;
  unsigned long sc_acclo;
  unsigned long sc_acchi;
  unsigned long sc_a[16];
  void *sc_xtregs;
};

#endif /* _BITS_SIGCONTEXT_H */

