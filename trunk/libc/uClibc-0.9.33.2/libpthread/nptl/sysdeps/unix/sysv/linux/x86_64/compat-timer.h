/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <signal.h>
#include <time.h>
#include <sys/types.h>

#define OLD_TIMER_MAX	256

extern timer_t __compat_timer_list[OLD_TIMER_MAX] attribute_hidden;


extern int __timer_create_new (clockid_t clock_id, struct sigevent *evp,
			       timer_t *timerid);
extern int __timer_delete_new (timer_t timerid);
extern int __timer_getoverrun_new (timer_t timerid);
extern int __timer_gettime_new (timer_t timerid, struct itimerspec *value);
extern int __timer_settime_new (timer_t timerid, int flags,
				const struct itimerspec *value,
				struct itimerspec *ovalue);


extern int __timer_create_old (clockid_t clock_id, struct sigevent *evp,
			       int *timerid);
extern int __timer_delete_old (int timerid);
extern int __timer_getoverrun_old (int timerid);
extern int __timer_gettime_old (int timerid, struct itimerspec *value);
extern int __timer_settime_old (int timerid, int flags,
				const struct itimerspec *value,
				struct itimerspec *ovalue);
