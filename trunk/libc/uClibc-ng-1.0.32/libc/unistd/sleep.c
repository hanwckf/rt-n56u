/* Implementation of the POSIX sleep function using nanosleep.
   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>


/* version perusing nanosleep */
#if defined __UCLIBC_HAS_REALTIME__

/* I am unable to reproduce alleged "Linux quirk".
 * I used the following test program:
#include <unistd.h>
#include <time.h>
#include <signal.h>
static void dummy(int sig) {}
int main() {
    struct timespec t = { 2, 0 };
    if (fork() == 0) {
	sleep(1);
	return 0;
    }
    signal(SIGCHLD, SIG_DFL); //
    signal(SIGCHLD, dummy);   // Pick one
    signal(SIGCHLD, SIG_IGN); //
    nanosleep(&t, &t);
    return 0;
}
 * Testing on 2.4.20 and on 2.6.35-rc4:
 * With SIG_DFL, nanosleep is not interrupted by SIGCHLD. Ok.
 * With dummy handler, nanosleep is interrupted by SIGCHLD. Ok.
 * With SIG_IGN, nanosleep is NOT interrupted by SIGCHLD.
 * It looks like sleep's workaround for SIG_IGN is no longer needed?
 * The only emails I can find are from 1998 (!):
 * ----------
 *  Subject: Re: sleep ignore sigchld
 *  From: Linus Torvalds <torvalds@transmeta.com>
 *  Date: Mon, 16 Nov 1998 11:02:15 -0800 (PST)
 *
 *  On Mon, 16 Nov 1998, H. J. Lu wrote:
 *  > That is a kernel bug. SIGCHLD is a special one. Usually it cannot
 *  > be ignored. [snip...]
 *
 *  No can do.
 *
 *  "nanosleep()" is implemented in a bad way that makes it impossible to
 *  restart it cleanly. It was done that way because glibc wanted it that way,
 *  not because it's a good idea. [snip...]
 * ----------
 * I assume that in the passed twelve+ years, nanosleep got fixed,
 * but the hack in sleep to work around broken nanosleep was never removed.
 */

# if 0

/* This is a quick and dirty, but not 100% compliant with
 * the stupid SysV SIGCHLD vs. SIG_IGN behaviour.  It is
 * fine unless you are messing with SIGCHLD...  */
unsigned int sleep (unsigned int sec)
{
	unsigned int res;
	struct timespec ts = { .tv_sec = (long int) seconds, .tv_nsec = 0 };
	res = nanosleep(&ts, &ts);
	if (res) res = (unsigned int) ts.tv_sec + (ts.tv_nsec >= 500000000L);
	return res;
}

# else

/* We are going to use the `nanosleep' syscall of the kernel.  But the
   kernel does not implement the sstupid SysV SIGCHLD vs. SIG_IGN
   behaviour for this syscall.  Therefore we have to emulate it here.  */
unsigned int sleep (unsigned int seconds)
{
    struct timespec ts = { .tv_sec = (long int) seconds, .tv_nsec = 0 };
    sigset_t set;
    struct sigaction oact;
    unsigned int result;

    /* This is not necessary but some buggy programs depend on this.  */
    if (seconds == 0) {
#  ifdef CANCELLATION_P
	int cancelhandling;
	CANCELLATION_P (THREAD_SELF);
#  endif
	return 0;
    }

    /* Linux will wake up the system call, nanosleep, when SIGCHLD
       arrives even if SIGCHLD is ignored.  We have to deal with it
       in libc.  */

    __sigemptyset (&set);
    __sigaddset (&set, SIGCHLD);

    /* Is SIGCHLD set to SIG_IGN? */
    sigaction (SIGCHLD, NULL, &oact); /* never fails */
    if (oact.sa_handler == SIG_IGN) {
	/* Yes.  Block SIGCHLD, save old mask.  */
	sigprocmask (SIG_BLOCK, &set, &set); /* never fails */
    }

    /* Run nanosleep, with SIGCHLD blocked if SIGCHLD is SIG_IGNed.  */
    result = nanosleep (&ts, &ts);
    if (result != 0) {
	/* Got EINTR. Return remaining time.  */
	result = (unsigned int) ts.tv_sec + (ts.tv_nsec >= 500000000L);
    }

    if (!__sigismember (&set, SIGCHLD)) {
	/* We did block SIGCHLD, and old mask had no SIGCHLD bit.
	   IOW: we need to unblock SIGCHLD now. Do it.  */
	/* this sigprocmask call never fails, thus never updates errno,
	   and therefore we don't need to save/restore it.  */
	sigprocmask (SIG_SETMASK, &set, NULL); /* never fails */
    }

    return result;
}

# endif

#else /* __UCLIBC_HAS_REALTIME__ */

/* no nanosleep, use signals and alarm() */
static void sleep_alarm_handler(int attribute_unused sig)
{
}
unsigned int sleep (unsigned int seconds)
{
    struct sigaction act, oact;
    sigset_t set, oset;
    unsigned int result, remaining;
    time_t before, after;
    int old_errno = errno;

    /* This is not necessary but some buggy programs depend on this.  */
    if (seconds == 0)
	return 0;

    /* block SIGALRM */
    __sigemptyset (&set);
    __sigaddset (&set, SIGALRM);
    sigprocmask (SIG_BLOCK, &set, &oset); /* can't fail */

    act.sa_handler = sleep_alarm_handler;
    act.sa_flags = 0;
    act.sa_mask = oset;
    sigaction(SIGALRM, &act, &oact); /* never fails */

    before = time(NULL);
    remaining = alarm(seconds);
    if (remaining && remaining > seconds) {
	/* restore user's alarm */
	sigaction(SIGALRM, &oact, NULL);
	alarm(remaining); /* restore old alarm */
	sigsuspend(&oset);
	after = time(NULL);
    } else {
	sigsuspend (&oset);
	after = time(NULL);
	sigaction (SIGALRM, &oact, NULL);
    }
    result = after - before;
    alarm(remaining > result ? remaining - result : 0);
    sigprocmask (SIG_SETMASK, &oset, NULL);

    __set_errno(old_errno);

    return result > seconds ? 0 : seconds - result;
}

#endif /* __UCLIBC_HAS_REALTIME__ */

libc_hidden_def(sleep)
