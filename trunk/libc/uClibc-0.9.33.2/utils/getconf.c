/* Copyright (C) 1991, 92, 1995-2008, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#define _GNU_SOURCE 1
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PACKAGE "getconf regression test"
#define VERSION ""
#define _(x) x
#define error(status, errnum, fmt, ...) \
	{fprintf(stderr, fmt "\n", ## __VA_ARGS__); exit(status);}


struct conf
  {
    const char *name;
    const int call_name;
    const enum { SYSCONF, CONFSTR, PATHCONF } call;
  };

static const struct conf vars[] =
  {
#ifdef _PC_LINK_MAX
    { "LINK_MAX", _PC_LINK_MAX, PATHCONF },
#endif
#ifdef _PC_LINK_MAX
    { "_POSIX_LINK_MAX", _PC_LINK_MAX, PATHCONF },
#endif
#ifdef _PC_MAX_CANON
    { "MAX_CANON", _PC_MAX_CANON, PATHCONF },
#endif
#ifdef _PC_MAX_CANON
    { "_POSIX_MAX_CANON", _PC_MAX_CANON, PATHCONF },
#endif
#ifdef _PC_MAX_INPUT
    { "MAX_INPUT", _PC_MAX_INPUT, PATHCONF },
#endif
#ifdef _PC_MAX_INPUT
    { "_POSIX_MAX_INPUT", _PC_MAX_INPUT, PATHCONF },
#endif
#ifdef _PC_NAME_MAX
    { "NAME_MAX", _PC_NAME_MAX, PATHCONF },
#endif
#ifdef _PC_NAME_MAX
    { "_POSIX_NAME_MAX", _PC_NAME_MAX, PATHCONF },
#endif
#ifdef _PC_PATH_MAX
    { "PATH_MAX", _PC_PATH_MAX, PATHCONF },
#endif
#ifdef _PC_PATH_MAX
    { "_POSIX_PATH_MAX", _PC_PATH_MAX, PATHCONF },
#endif
#ifdef _PC_PIPE_BUF
    { "PIPE_BUF", _PC_PIPE_BUF, PATHCONF },
#endif
#ifdef _PC_PIPE_BUF
    { "_POSIX_PIPE_BUF", _PC_PIPE_BUF, PATHCONF },
#endif
#ifdef _PC_SOCK_MAXBUF
    { "SOCK_MAXBUF", _PC_SOCK_MAXBUF, PATHCONF },
#endif
#ifdef _PC_ASYNC_IO
    { "_POSIX_ASYNC_IO", _PC_ASYNC_IO, PATHCONF },
#endif
#ifdef _PC_CHOWN_RESTRICTED
    { "_POSIX_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED, PATHCONF },
#endif
#ifdef _PC_NO_TRUNC
    { "_POSIX_NO_TRUNC", _PC_NO_TRUNC, PATHCONF },
#endif
#ifdef _PC_PRIO_IO
    { "_POSIX_PRIO_IO", _PC_PRIO_IO, PATHCONF },
#endif
#ifdef _PC_SYNC_IO
    { "_POSIX_SYNC_IO", _PC_SYNC_IO, PATHCONF },
#endif
#ifdef _PC_VDISABLE
    { "_POSIX_VDISABLE", _PC_VDISABLE, PATHCONF },
#endif

#ifdef _SC_ARG_MAX
    { "ARG_MAX", _SC_ARG_MAX, SYSCONF },
#endif
#ifdef _SC_ATEXIT_MAX
    { "ATEXIT_MAX", _SC_ATEXIT_MAX, SYSCONF },
#endif
#ifdef _SC_CHAR_BIT
    { "CHAR_BIT", _SC_CHAR_BIT, SYSCONF },
#endif
#ifdef _SC_CHAR_MAX
    { "CHAR_MAX", _SC_CHAR_MAX, SYSCONF },
#endif
#ifdef _SC_CHAR_MIN
    { "CHAR_MIN", _SC_CHAR_MIN, SYSCONF },
#endif
#ifdef _SC_CHILD_MAX
    { "CHILD_MAX", _SC_CHILD_MAX, SYSCONF },
#endif
#ifdef _SC_CLK_TCK
    { "CLK_TCK", _SC_CLK_TCK, SYSCONF },
#endif
#ifdef _SC_INT_MAX
    { "INT_MAX", _SC_INT_MAX, SYSCONF },
#endif
#ifdef _SC_INT_MIN
    { "INT_MIN", _SC_INT_MIN, SYSCONF },
#endif
#ifdef _SC_UIO_MAXIOV
    { "IOV_MAX", _SC_UIO_MAXIOV, SYSCONF },
#endif
#ifdef _SC_LOGIN_NAME_MAX
    { "LOGNAME_MAX", _SC_LOGIN_NAME_MAX, SYSCONF },
#endif
#ifdef _SC_LONG_BIT
    { "LONG_BIT", _SC_LONG_BIT, SYSCONF },
#endif
#ifdef _SC_MB_LEN_MAX
    { "MB_LEN_MAX", _SC_MB_LEN_MAX, SYSCONF },
#endif
#ifdef _SC_NGROUPS_MAX
    { "NGROUPS_MAX", _SC_NGROUPS_MAX, SYSCONF },
#endif
#ifdef _SC_NL_ARGMAX
    { "NL_ARGMAX", _SC_NL_ARGMAX, SYSCONF },
#endif
#ifdef _SC_NL_LANGMAX
    { "NL_LANGMAX", _SC_NL_LANGMAX, SYSCONF },
#endif
#ifdef _SC_NL_MSGMAX
    { "NL_MSGMAX", _SC_NL_MSGMAX, SYSCONF },
#endif
#ifdef _SC_NL_NMAX
    { "NL_NMAX", _SC_NL_NMAX, SYSCONF },
#endif
#ifdef _SC_NL_SETMAX
    { "NL_SETMAX", _SC_NL_SETMAX, SYSCONF },
#endif
#ifdef _SC_NL_TEXTMAX
    { "NL_TEXTMAX", _SC_NL_TEXTMAX, SYSCONF },
#endif
#ifdef _SC_GETGR_R_SIZE_MAX
    { "NSS_BUFLEN_GROUP", _SC_GETGR_R_SIZE_MAX, SYSCONF },
#endif
#ifdef _SC_GETPW_R_SIZE_MAX
    { "NSS_BUFLEN_PASSWD", _SC_GETPW_R_SIZE_MAX, SYSCONF },
#endif
#ifdef _SC_NZERO
    { "NZERO", _SC_NZERO, SYSCONF },
#endif
#ifdef _SC_OPEN_MAX
    { "OPEN_MAX", _SC_OPEN_MAX, SYSCONF },
#endif
#ifdef _SC_PAGESIZE
    { "PAGESIZE", _SC_PAGESIZE, SYSCONF },
#endif
#ifdef _SC_PAGESIZE
    { "PAGE_SIZE", _SC_PAGESIZE, SYSCONF },
#endif
#ifdef _SC_PASS_MAX
    { "PASS_MAX", _SC_PASS_MAX, SYSCONF },
#endif
#ifdef _SC_THREAD_DESTRUCTOR_ITERATIONS
    { "PTHREAD_DESTRUCTOR_ITERATIONS", _SC_THREAD_DESTRUCTOR_ITERATIONS, SYSCONF },
#endif
#ifdef _SC_THREAD_KEYS_MAX
    { "PTHREAD_KEYS_MAX", _SC_THREAD_KEYS_MAX, SYSCONF },
#endif
#ifdef _SC_THREAD_STACK_MIN
    { "PTHREAD_STACK_MIN", _SC_THREAD_STACK_MIN, SYSCONF },
#endif
#ifdef _SC_THREAD_THREADS_MAX
    { "PTHREAD_THREADS_MAX", _SC_THREAD_THREADS_MAX, SYSCONF },
#endif
#ifdef _SC_SCHAR_MAX
    { "SCHAR_MAX", _SC_SCHAR_MAX, SYSCONF },
#endif
#ifdef _SC_SCHAR_MIN
    { "SCHAR_MIN", _SC_SCHAR_MIN, SYSCONF },
#endif
#ifdef _SC_SHRT_MAX
    { "SHRT_MAX", _SC_SHRT_MAX, SYSCONF },
#endif
#ifdef _SC_SHRT_MIN
    { "SHRT_MIN", _SC_SHRT_MIN, SYSCONF },
#endif
#ifdef _SC_SSIZE_MAX
    { "SSIZE_MAX", _SC_SSIZE_MAX, SYSCONF },
#endif
#ifdef _SC_TTY_NAME_MAX
    { "TTY_NAME_MAX", _SC_TTY_NAME_MAX, SYSCONF },
#endif
#ifdef _SC_TZNAME_MAX
    { "TZNAME_MAX", _SC_TZNAME_MAX, SYSCONF },
#endif
#ifdef _SC_UCHAR_MAX
    { "UCHAR_MAX", _SC_UCHAR_MAX, SYSCONF },
#endif
#ifdef _SC_UINT_MAX
    { "UINT_MAX", _SC_UINT_MAX, SYSCONF },
#endif
#ifdef _SC_UIO_MAXIOV
    { "UIO_MAXIOV", _SC_UIO_MAXIOV, SYSCONF },
#endif
#ifdef _SC_ULONG_MAX
    { "ULONG_MAX", _SC_ULONG_MAX, SYSCONF },
#endif
#ifdef _SC_USHRT_MAX
    { "USHRT_MAX", _SC_USHRT_MAX, SYSCONF },
#endif
#ifdef _SC_WORD_BIT
    { "WORD_BIT", _SC_WORD_BIT, SYSCONF },
#endif
#ifdef _SC_AVPHYS_PAGES
    { "_AVPHYS_PAGES", _SC_AVPHYS_PAGES, SYSCONF },
#endif
#ifdef _SC_NPROCESSORS_CONF
    { "_NPROCESSORS_CONF", _SC_NPROCESSORS_CONF, SYSCONF },
#endif
#ifdef _SC_NPROCESSORS_ONLN
    { "_NPROCESSORS_ONLN", _SC_NPROCESSORS_ONLN, SYSCONF },
#endif
#ifdef _SC_PHYS_PAGES
    { "_PHYS_PAGES", _SC_PHYS_PAGES, SYSCONF },
#endif
#ifdef _SC_ARG_MAX
    { "_POSIX_ARG_MAX", _SC_ARG_MAX, SYSCONF },
#endif
#ifdef _SC_ASYNCHRONOUS_IO
    { "_POSIX_ASYNCHRONOUS_IO", _SC_ASYNCHRONOUS_IO, SYSCONF },
#endif
#ifdef _SC_CHILD_MAX
    { "_POSIX_CHILD_MAX", _SC_CHILD_MAX, SYSCONF },
#endif
#ifdef _SC_FSYNC
    { "_POSIX_FSYNC", _SC_FSYNC, SYSCONF },
#endif
#ifdef _SC_JOB_CONTROL
    { "_POSIX_JOB_CONTROL", _SC_JOB_CONTROL, SYSCONF },
#endif
#ifdef _SC_MAPPED_FILES
    { "_POSIX_MAPPED_FILES", _SC_MAPPED_FILES, SYSCONF },
#endif
#ifdef _SC_MEMLOCK
    { "_POSIX_MEMLOCK", _SC_MEMLOCK, SYSCONF },
#endif
#ifdef _SC_MEMLOCK_RANGE
    { "_POSIX_MEMLOCK_RANGE", _SC_MEMLOCK_RANGE, SYSCONF },
#endif
#ifdef _SC_MEMORY_PROTECTION
    { "_POSIX_MEMORY_PROTECTION", _SC_MEMORY_PROTECTION, SYSCONF },
#endif
#ifdef _SC_MESSAGE_PASSING
    { "_POSIX_MESSAGE_PASSING", _SC_MESSAGE_PASSING, SYSCONF },
#endif
#ifdef _SC_NGROUPS_MAX
    { "_POSIX_NGROUPS_MAX", _SC_NGROUPS_MAX, SYSCONF },
#endif
#ifdef _SC_OPEN_MAX
    { "_POSIX_OPEN_MAX", _SC_OPEN_MAX, SYSCONF },
#endif
#ifdef _SC_PII
    { "_POSIX_PII", _SC_PII, SYSCONF },
#endif
#ifdef _SC_PII_INTERNET
    { "_POSIX_PII_INTERNET", _SC_PII_INTERNET, SYSCONF },
#endif
#ifdef _SC_PII_INTERNET_DGRAM
    { "_POSIX_PII_INTERNET_DGRAM", _SC_PII_INTERNET_DGRAM, SYSCONF },
#endif
#ifdef _SC_PII_INTERNET_STREAM
    { "_POSIX_PII_INTERNET_STREAM", _SC_PII_INTERNET_STREAM, SYSCONF },
#endif
#ifdef _SC_PII_OSI
    { "_POSIX_PII_OSI", _SC_PII_OSI, SYSCONF },
#endif
#ifdef _SC_PII_OSI_CLTS
    { "_POSIX_PII_OSI_CLTS", _SC_PII_OSI_CLTS, SYSCONF },
#endif
#ifdef _SC_PII_OSI_COTS
    { "_POSIX_PII_OSI_COTS", _SC_PII_OSI_COTS, SYSCONF },
#endif
#ifdef _SC_PII_OSI_M
    { "_POSIX_PII_OSI_M", _SC_PII_OSI_M, SYSCONF },
#endif
#ifdef _SC_PII_SOCKET
    { "_POSIX_PII_SOCKET", _SC_PII_SOCKET, SYSCONF },
#endif
#ifdef _SC_PII_XTI
    { "_POSIX_PII_XTI", _SC_PII_XTI, SYSCONF },
#endif
#ifdef _SC_POLL
    { "_POSIX_POLL", _SC_POLL, SYSCONF },
#endif
#ifdef _SC_PRIORITIZED_IO
    { "_POSIX_PRIORITIZED_IO", _SC_PRIORITIZED_IO, SYSCONF },
#endif
#ifdef _SC_PRIORITY_SCHEDULING
    { "_POSIX_PRIORITY_SCHEDULING", _SC_PRIORITY_SCHEDULING, SYSCONF },
#endif
#ifdef _SC_REALTIME_SIGNALS
    { "_POSIX_REALTIME_SIGNALS", _SC_REALTIME_SIGNALS, SYSCONF },
#endif
#ifdef _SC_SAVED_IDS
    { "_POSIX_SAVED_IDS", _SC_SAVED_IDS, SYSCONF },
#endif
#ifdef _SC_SELECT
    { "_POSIX_SELECT", _SC_SELECT, SYSCONF },
#endif
#ifdef _SC_SEMAPHORES
    { "_POSIX_SEMAPHORES", _SC_SEMAPHORES, SYSCONF },
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
    { "_POSIX_SHARED_MEMORY_OBJECTS", _SC_SHARED_MEMORY_OBJECTS, SYSCONF },
#endif
#ifdef _SC_SSIZE_MAX
    { "_POSIX_SSIZE_MAX", _SC_SSIZE_MAX, SYSCONF },
#endif
#ifdef _SC_STREAM_MAX
    { "_POSIX_STREAM_MAX", _SC_STREAM_MAX, SYSCONF },
#endif
#ifdef _SC_SYNCHRONIZED_IO
    { "_POSIX_SYNCHRONIZED_IO", _SC_SYNCHRONIZED_IO, SYSCONF },
#endif
#ifdef _SC_THREADS
    { "_POSIX_THREADS", _SC_THREADS, SYSCONF },
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
    { "_POSIX_THREAD_ATTR_STACKADDR", _SC_THREAD_ATTR_STACKADDR, SYSCONF },
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
    { "_POSIX_THREAD_ATTR_STACKSIZE", _SC_THREAD_ATTR_STACKSIZE, SYSCONF },
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
    { "_POSIX_THREAD_PRIORITY_SCHEDULING", _SC_THREAD_PRIORITY_SCHEDULING, SYSCONF },
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
    { "_POSIX_THREAD_PRIO_INHERIT", _SC_THREAD_PRIO_INHERIT, SYSCONF },
#endif
#ifdef _SC_THREAD_PRIO_PROTECT
    { "_POSIX_THREAD_PRIO_PROTECT", _SC_THREAD_PRIO_PROTECT, SYSCONF },
#endif
#ifdef _SC_THREAD_ROBUST_PRIO_INHERIT
    { "_POSIX_THREAD_ROBUST_PRIO_INHERIT", _SC_THREAD_ROBUST_PRIO_INHERIT,
      SYSCONF },
#endif
#ifdef _SC_THREAD_ROBUST_PRIO_PROTECT
    { "_POSIX_THREAD_ROBUST_PRIO_PROTECT", _SC_THREAD_ROBUST_PRIO_PROTECT,
      SYSCONF },
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
    { "_POSIX_THREAD_PROCESS_SHARED", _SC_THREAD_PROCESS_SHARED, SYSCONF },
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
    { "_POSIX_THREAD_SAFE_FUNCTIONS", _SC_THREAD_SAFE_FUNCTIONS, SYSCONF },
#endif
#ifdef _SC_TIMERS
    { "_POSIX_TIMERS", _SC_TIMERS, SYSCONF },
#endif
#ifdef _SC_TIMER_MAX
    { "TIMER_MAX", _SC_TIMER_MAX, SYSCONF },
#endif
#ifdef _SC_TZNAME_MAX
    { "_POSIX_TZNAME_MAX", _SC_TZNAME_MAX, SYSCONF },
#endif
#ifdef _SC_VERSION
    { "_POSIX_VERSION", _SC_VERSION, SYSCONF },
#endif
#ifdef _SC_T_IOV_MAX
    { "_T_IOV_MAX", _SC_T_IOV_MAX, SYSCONF },
#endif
#ifdef _SC_XOPEN_CRYPT
    { "_XOPEN_CRYPT", _SC_XOPEN_CRYPT, SYSCONF },
#endif
#ifdef _SC_XOPEN_ENH_I18N
    { "_XOPEN_ENH_I18N", _SC_XOPEN_ENH_I18N, SYSCONF },
#endif
#ifdef _SC_XOPEN_LEGACY
    { "_XOPEN_LEGACY", _SC_XOPEN_LEGACY, SYSCONF },
#endif
#ifdef _SC_XOPEN_REALTIME
    { "_XOPEN_REALTIME", _SC_XOPEN_REALTIME, SYSCONF },
#endif
#ifdef _SC_XOPEN_REALTIME_THREADS
    { "_XOPEN_REALTIME_THREADS", _SC_XOPEN_REALTIME_THREADS, SYSCONF },
#endif
#ifdef _SC_XOPEN_SHM
    { "_XOPEN_SHM", _SC_XOPEN_SHM, SYSCONF },
#endif
#ifdef _SC_XOPEN_UNIX
    { "_XOPEN_UNIX", _SC_XOPEN_UNIX, SYSCONF },
#endif
#ifdef _SC_XOPEN_VERSION
    { "_XOPEN_VERSION", _SC_XOPEN_VERSION, SYSCONF },
#endif
#ifdef _SC_XOPEN_XCU_VERSION
    { "_XOPEN_XCU_VERSION", _SC_XOPEN_XCU_VERSION, SYSCONF },
#endif
#ifdef _SC_XOPEN_XPG2
    { "_XOPEN_XPG2", _SC_XOPEN_XPG2, SYSCONF },
#endif
#ifdef _SC_XOPEN_XPG3
    { "_XOPEN_XPG3", _SC_XOPEN_XPG3, SYSCONF },
#endif
#ifdef _SC_XOPEN_XPG4
    { "_XOPEN_XPG4", _SC_XOPEN_XPG4, SYSCONF },
#endif
    /* POSIX.2  */
#ifdef _SC_BC_BASE_MAX
    { "BC_BASE_MAX", _SC_BC_BASE_MAX, SYSCONF },
#endif
#ifdef _SC_BC_DIM_MAX
    { "BC_DIM_MAX", _SC_BC_DIM_MAX, SYSCONF },
#endif
#ifdef _SC_BC_SCALE_MAX
    { "BC_SCALE_MAX", _SC_BC_SCALE_MAX, SYSCONF },
#endif
#ifdef _SC_BC_STRING_MAX
    { "BC_STRING_MAX", _SC_BC_STRING_MAX, SYSCONF },
#endif
    { "CHARCLASS_NAME_MAX", _SC_CHARCLASS_NAME_MAX, SYSCONF },
#ifdef _SC_COLL_WEIGHTS_MAX
    { "COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX, SYSCONF },
#endif
#ifdef _SC_EQUIV_CLASS_MAX
    { "EQUIV_CLASS_MAX", _SC_EQUIV_CLASS_MAX, SYSCONF },
#endif
#ifdef _SC_EXPR_NEST_MAX
    { "EXPR_NEST_MAX", _SC_EXPR_NEST_MAX, SYSCONF },
#endif
#ifdef _SC_LINE_MAX
    { "LINE_MAX", _SC_LINE_MAX, SYSCONF },
#endif
#ifdef _SC_BC_BASE_MAX
    { "POSIX2_BC_BASE_MAX", _SC_BC_BASE_MAX, SYSCONF },
#endif
#ifdef _SC_BC_DIM_MAX
    { "POSIX2_BC_DIM_MAX", _SC_BC_DIM_MAX, SYSCONF },
#endif
#ifdef _SC_BC_SCALE_MAX
    { "POSIX2_BC_SCALE_MAX", _SC_BC_SCALE_MAX, SYSCONF },
#endif
#ifdef _SC_BC_STRING_MAX
    { "POSIX2_BC_STRING_MAX", _SC_BC_STRING_MAX, SYSCONF },
#endif
#ifdef _SC_2_CHAR_TERM
    { "POSIX2_CHAR_TERM", _SC_2_CHAR_TERM, SYSCONF },
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
    { "POSIX2_COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX, SYSCONF },
#endif
#ifdef _SC_2_C_BIND
    { "POSIX2_C_BIND", _SC_2_C_BIND, SYSCONF },
#endif
#ifdef _SC_2_C_DEV
    { "POSIX2_C_DEV", _SC_2_C_DEV, SYSCONF },
#endif
#ifdef _SC_2_C_VERSION
    { "POSIX2_C_VERSION", _SC_2_C_VERSION, SYSCONF },
#endif
#ifdef _SC_EXPR_NEST_MAX
    { "POSIX2_EXPR_NEST_MAX", _SC_EXPR_NEST_MAX, SYSCONF },
#endif
#ifdef _SC_2_FORT_DEV
    { "POSIX2_FORT_DEV", _SC_2_FORT_DEV, SYSCONF },
#endif
#ifdef _SC_2_FORT_RUN
    { "POSIX2_FORT_RUN", _SC_2_FORT_RUN, SYSCONF },
#endif
#ifdef _SC_LINE_MAX
    { "_POSIX2_LINE_MAX", _SC_LINE_MAX, SYSCONF },
    { "POSIX2_LINE_MAX", _SC_LINE_MAX, SYSCONF },
#endif
#ifdef _SC_2_LOCALEDEF
    { "POSIX2_LOCALEDEF", _SC_2_LOCALEDEF, SYSCONF },
#endif
#ifdef _SC_RE_DUP_MAX
    { "POSIX2_RE_DUP_MAX", _SC_RE_DUP_MAX, SYSCONF },
#endif
#ifdef _SC_2_SW_DEV
    { "POSIX2_SW_DEV", _SC_2_SW_DEV, SYSCONF },
#endif
#ifdef _SC_2_UPE
    { "POSIX2_UPE", _SC_2_UPE, SYSCONF },
#endif
#ifdef _SC_2_VERSION
    { "POSIX2_VERSION", _SC_2_VERSION, SYSCONF },
#endif
#ifdef _SC_RE_DUP_MAX
    { "RE_DUP_MAX", _SC_RE_DUP_MAX, SYSCONF },
#endif

#ifdef _CS_PATH
    { "PATH", _CS_PATH, CONFSTR },
    { "CS_PATH", _CS_PATH, CONFSTR },
#endif

    /* LFS */
#ifdef _CS_LFS_CFLAGS
    { "LFS_CFLAGS", _CS_LFS_CFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS_LDFLAGS
    { "LFS_LDFLAGS", _CS_LFS_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS_LIBS
    { "LFS_LIBS", _CS_LFS_LIBS, CONFSTR },
#endif
#ifdef _CS_LFS_LINTFLAGS
    { "LFS_LINTFLAGS", _CS_LFS_LINTFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS64_CFLAGS
    { "LFS64_CFLAGS", _CS_LFS64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS64_LDFLAGS
    { "LFS64_LDFLAGS", _CS_LFS64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_LFS64_LIBS
    { "LFS64_LIBS", _CS_LFS64_LIBS, CONFSTR },
#endif
#ifdef _CS_LFS64_LINTFLAGS
    { "LFS64_LINTFLAGS", _CS_LFS64_LINTFLAGS, CONFSTR },
#endif

    /* Programming environments.  */
#ifdef _CS_V5_WIDTH_RESTRICTED_ENVS
    { "_XBS5_WIDTH_RESTRICTED_ENVS", _CS_V5_WIDTH_RESTRICTED_ENVS, CONFSTR },
    { "XBS5_WIDTH_RESTRICTED_ENVS", _CS_V5_WIDTH_RESTRICTED_ENVS, CONFSTR },
#endif

#ifdef _SC_XBS5_ILP32_OFF32
    { "_XBS5_ILP32_OFF32", _SC_XBS5_ILP32_OFF32, SYSCONF },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_CFLAGS
    { "XBS5_ILP32_OFF32_CFLAGS", _CS_XBS5_ILP32_OFF32_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LDFLAGS
    { "XBS5_ILP32_OFF32_LDFLAGS", _CS_XBS5_ILP32_OFF32_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LIBS
    { "XBS5_ILP32_OFF32_LIBS", _CS_XBS5_ILP32_OFF32_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LINTFLAGS
    { "XBS5_ILP32_OFF32_LINTFLAGS", _CS_XBS5_ILP32_OFF32_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_XBS5_ILP32_OFFBIG
    { "_XBS5_ILP32_OFFBIG", _SC_XBS5_ILP32_OFFBIG, SYSCONF },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_CFLAGS
    { "XBS5_ILP32_OFFBIG_CFLAGS", _CS_XBS5_ILP32_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LDFLAGS
    { "XBS5_ILP32_OFFBIG_LDFLAGS", _CS_XBS5_ILP32_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LIBS
    { "XBS5_ILP32_OFFBIG_LIBS", _CS_XBS5_ILP32_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LINTFLAGS
    { "XBS5_ILP32_OFFBIG_LINTFLAGS", _CS_XBS5_ILP32_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_XBS5_LP64_OFF64
    { "_XBS5_LP64_OFF64", _SC_XBS5_LP64_OFF64, SYSCONF },
#endif
#ifdef _CS_XBS5_LP64_OFF64_CFLAGS
    { "XBS5_LP64_OFF64_CFLAGS", _CS_XBS5_LP64_OFF64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LDFLAGS
    { "XBS5_LP64_OFF64_LDFLAGS", _CS_XBS5_LP64_OFF64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LIBS
    { "XBS5_LP64_OFF64_LIBS", _CS_XBS5_LP64_OFF64_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LINTFLAGS
    { "XBS5_LP64_OFF64_LINTFLAGS", _CS_XBS5_LP64_OFF64_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_XBS5_LPBIG_OFFBIG
    { "_XBS5_LPBIG_OFFBIG", _SC_XBS5_LPBIG_OFFBIG, SYSCONF },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_CFLAGS
    { "XBS5_LPBIG_OFFBIG_CFLAGS", _CS_XBS5_LPBIG_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LDFLAGS
    { "XBS5_LPBIG_OFFBIG_LDFLAGS", _CS_XBS5_LPBIG_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LIBS
    { "XBS5_LPBIG_OFFBIG_LIBS", _CS_XBS5_LPBIG_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS
    { "XBS5_LPBIG_OFFBIG_LINTFLAGS", _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V6_ILP32_OFF32
    { "_POSIX_V6_ILP32_OFF32", _SC_V6_ILP32_OFF32, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_CFLAGS
    { "POSIX_V6_ILP32_OFF32_CFLAGS", _CS_POSIX_V6_ILP32_OFF32_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LDFLAGS
    { "POSIX_V6_ILP32_OFF32_LDFLAGS", _CS_POSIX_V6_ILP32_OFF32_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LIBS
    { "POSIX_V6_ILP32_OFF32_LIBS", _CS_POSIX_V6_ILP32_OFF32_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS
    { "POSIX_V6_ILP32_OFF32_LINTFLAGS", _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS, CONFSTR },
#endif

#ifdef _CS_V6_WIDTH_RESTRICTED_ENVS
    { "_POSIX_V6_WIDTH_RESTRICTED_ENVS", _CS_V6_WIDTH_RESTRICTED_ENVS, CONFSTR },
    { "POSIX_V6_WIDTH_RESTRICTED_ENVS", _CS_V6_WIDTH_RESTRICTED_ENVS, CONFSTR },
#endif

#ifdef _SC_V6_ILP32_OFFBIG
    { "_POSIX_V6_ILP32_OFFBIG", _SC_V6_ILP32_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS
    { "POSIX_V6_ILP32_OFFBIG_CFLAGS", _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS
    { "POSIX_V6_ILP32_OFFBIG_LDFLAGS", _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_LIBS
    { "POSIX_V6_ILP32_OFFBIG_LIBS", _CS_POSIX_V6_ILP32_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS
    { "POSIX_V6_ILP32_OFFBIG_LINTFLAGS", _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V6_LP64_OFF64
    { "_POSIX_V6_LP64_OFF64", _SC_V6_LP64_OFF64, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_CFLAGS
    { "POSIX_V6_LP64_OFF64_CFLAGS", _CS_POSIX_V6_LP64_OFF64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LDFLAGS
    { "POSIX_V6_LP64_OFF64_LDFLAGS", _CS_POSIX_V6_LP64_OFF64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LIBS
    { "POSIX_V6_LP64_OFF64_LIBS", _CS_POSIX_V6_LP64_OFF64_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LP64_OFF64_LINTFLAGS
    { "POSIX_V6_LP64_OFF64_LINTFLAGS", _CS_POSIX_V6_LP64_OFF64_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V6_LPBIG_OFFBIG
    { "_POSIX_V6_LPBIG_OFFBIG", _SC_V6_LPBIG_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS
    { "POSIX_V6_LPBIG_OFFBIG_CFLAGS", _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS
    { "POSIX_V6_LPBIG_OFFBIG_LDFLAGS", _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LIBS
    { "POSIX_V6_LPBIG_OFFBIG_LIBS", _CS_POSIX_V6_LPBIG_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS
    { "POSIX_V6_LPBIG_OFFBIG_LINTFLAGS", _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V7_ILP32_OFF32
    { "_POSIX_V7_ILP32_OFF32", _SC_V7_ILP32_OFF32, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_CFLAGS
    { "POSIX_V7_ILP32_OFF32_CFLAGS", _CS_POSIX_V7_ILP32_OFF32_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_LDFLAGS
    { "POSIX_V7_ILP32_OFF32_LDFLAGS", _CS_POSIX_V7_ILP32_OFF32_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_LIBS
    { "POSIX_V7_ILP32_OFF32_LIBS", _CS_POSIX_V7_ILP32_OFF32_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS
    { "POSIX_V7_ILP32_OFF32_LINTFLAGS", _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS, CONFSTR },
#endif

#ifdef _CS_V7_WIDTH_RESTRICTED_ENVS
    { "_POSIX_V7_WIDTH_RESTRICTED_ENVS", _CS_V7_WIDTH_RESTRICTED_ENVS, CONFSTR },
    { "POSIX_V7_WIDTH_RESTRICTED_ENVS", _CS_V7_WIDTH_RESTRICTED_ENVS, CONFSTR },
#endif

#ifdef _SC_V7_ILP32_OFFBIG
    { "_POSIX_V7_ILP32_OFFBIG", _SC_V7_ILP32_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS
    { "POSIX_V7_ILP32_OFFBIG_CFLAGS", _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS
    { "POSIX_V7_ILP32_OFFBIG_LDFLAGS", _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_LIBS
    { "POSIX_V7_ILP32_OFFBIG_LIBS", _CS_POSIX_V7_ILP32_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS
    { "POSIX_V7_ILP32_OFFBIG_LINTFLAGS", _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V7_LP64_OFF64
    { "_POSIX_V7_LP64_OFF64", _SC_V7_LP64_OFF64, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_CFLAGS
    { "POSIX_V7_LP64_OFF64_CFLAGS", _CS_POSIX_V7_LP64_OFF64_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_LDFLAGS
    { "POSIX_V7_LP64_OFF64_LDFLAGS", _CS_POSIX_V7_LP64_OFF64_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_LIBS
    { "POSIX_V7_LP64_OFF64_LIBS", _CS_POSIX_V7_LP64_OFF64_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LP64_OFF64_LINTFLAGS
    { "POSIX_V7_LP64_OFF64_LINTFLAGS", _CS_POSIX_V7_LP64_OFF64_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_V7_LPBIG_OFFBIG
    { "_POSIX_V7_LPBIG_OFFBIG", _SC_V7_LPBIG_OFFBIG, SYSCONF },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS
    { "POSIX_V7_LPBIG_OFFBIG_CFLAGS", _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS
    { "POSIX_V7_LPBIG_OFFBIG_LDFLAGS", _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_LIBS
    { "POSIX_V7_LPBIG_OFFBIG_LIBS", _CS_POSIX_V7_LPBIG_OFFBIG_LIBS, CONFSTR },
#endif
#ifdef _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS
    { "POSIX_V7_LPBIG_OFFBIG_LINTFLAGS", _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS, CONFSTR },
#endif

#ifdef _SC_ADVISORY_INFO
    { "_POSIX_ADVISORY_INFO", _SC_ADVISORY_INFO, SYSCONF },
#endif
#ifdef _SC_BARRIERS
    { "_POSIX_BARRIERS", _SC_BARRIERS, SYSCONF },
#endif
#ifdef _SC_BASE
    { "_POSIX_BASE", _SC_BASE, SYSCONF },
#endif
#ifdef _SC_C_LANG_SUPPORT
    { "_POSIX_C_LANG_SUPPORT", _SC_C_LANG_SUPPORT, SYSCONF },
#endif
#ifdef _SC_C_LANG_SUPPORT_R
    { "_POSIX_C_LANG_SUPPORT_R", _SC_C_LANG_SUPPORT_R, SYSCONF },
#endif
#ifdef _SC_CLOCK_SELECTION
    { "_POSIX_CLOCK_SELECTION", _SC_CLOCK_SELECTION, SYSCONF },
#endif
#ifdef _SC_CPUTIME
    { "_POSIX_CPUTIME", _SC_CPUTIME, SYSCONF },
#endif
#ifdef _SC_THREAD_CPUTIME
    { "_POSIX_THREAD_CPUTIME", _SC_THREAD_CPUTIME, SYSCONF },
#endif
#ifdef _SC_DEVICE_SPECIFIC
    { "_POSIX_DEVICE_SPECIFIC", _SC_DEVICE_SPECIFIC, SYSCONF },
#endif
#ifdef _SC_DEVICE_SPECIFIC_R
    { "_POSIX_DEVICE_SPECIFIC_R", _SC_DEVICE_SPECIFIC_R, SYSCONF },
#endif
#ifdef _SC_FD_MGMT
    { "_POSIX_FD_MGMT", _SC_FD_MGMT, SYSCONF },
#endif
#ifdef _SC_FIFO
    { "_POSIX_FIFO", _SC_FIFO, SYSCONF },
#endif
#ifdef _SC_PIPE
    { "_POSIX_PIPE", _SC_PIPE, SYSCONF },
#endif
#ifdef _SC_FILE_ATTRIBUTES
    { "_POSIX_FILE_ATTRIBUTES", _SC_FILE_ATTRIBUTES, SYSCONF },
#endif
#ifdef _SC_FILE_LOCKING
    { "_POSIX_FILE_LOCKING", _SC_FILE_LOCKING, SYSCONF },
#endif
#ifdef _SC_FILE_SYSTEM
    { "_POSIX_FILE_SYSTEM", _SC_FILE_SYSTEM, SYSCONF },
#endif
#ifdef _SC_MONOTONIC_CLOCK
    { "_POSIX_MONOTONIC_CLOCK", _SC_MONOTONIC_CLOCK, SYSCONF },
#endif
#ifdef _SC_MULTI_PROCESS
    { "_POSIX_MULTI_PROCESS", _SC_MULTI_PROCESS, SYSCONF },
#endif
#ifdef _SC_SINGLE_PROCESS
    { "_POSIX_SINGLE_PROCESS", _SC_SINGLE_PROCESS, SYSCONF },
#endif
#ifdef _SC_NETWORKING
    { "_POSIX_NETWORKING", _SC_NETWORKING, SYSCONF },
#endif
#ifdef _SC_READER_WRITER_LOCKS
    { "_POSIX_READER_WRITER_LOCKS", _SC_READER_WRITER_LOCKS, SYSCONF },
#endif
#ifdef _SC_SPIN_LOCKS
    { "_POSIX_SPIN_LOCKS", _SC_SPIN_LOCKS, SYSCONF },
#endif
#ifdef _SC_REGEXP
    { "_POSIX_REGEXP", _SC_REGEXP, SYSCONF },
#endif
#ifdef _SC_REGEX_VERSION
    { "_REGEX_VERSION", _SC_REGEX_VERSION, SYSCONF },
#endif
#ifdef _SC_SHELL
    { "_POSIX_SHELL", _SC_SHELL, SYSCONF },
#endif
#ifdef _SC_SIGNALS
    { "_POSIX_SIGNALS", _SC_SIGNALS, SYSCONF },
#endif
#ifdef _SC_SPAWN
    { "_POSIX_SPAWN", _SC_SPAWN, SYSCONF },
#endif
#ifdef _SC_SPORADIC_SERVER
    { "_POSIX_SPORADIC_SERVER", _SC_SPORADIC_SERVER, SYSCONF },
#endif
#ifdef _SC_THREAD_SPORADIC_SERVER
    { "_POSIX_THREAD_SPORADIC_SERVER", _SC_THREAD_SPORADIC_SERVER, SYSCONF },
#endif
#ifdef _SC_SYSTEM_DATABASE
    { "_POSIX_SYSTEM_DATABASE", _SC_SYSTEM_DATABASE, SYSCONF },
#endif
#ifdef _SC_SYSTEM_DATABASE_R
    { "_POSIX_SYSTEM_DATABASE_R", _SC_SYSTEM_DATABASE_R, SYSCONF },
#endif
#ifdef _SC_TIMEOUTS
    { "_POSIX_TIMEOUTS", _SC_TIMEOUTS, SYSCONF },
#endif
#ifdef _SC_TYPED_MEMORY_OBJECTS
    { "_POSIX_TYPED_MEMORY_OBJECTS", _SC_TYPED_MEMORY_OBJECTS, SYSCONF },
#endif
#ifdef _SC_USER_GROUPS
    { "_POSIX_USER_GROUPS", _SC_USER_GROUPS, SYSCONF },
#endif
#ifdef _SC_USER_GROUPS_R
    { "_POSIX_USER_GROUPS_R", _SC_USER_GROUPS_R, SYSCONF },
#endif
#ifdef _SC_2_PBS
    { "POSIX2_PBS", _SC_2_PBS, SYSCONF },
#endif
#ifdef _SC_2_PBS_ACCOUNTING
    { "POSIX2_PBS_ACCOUNTING", _SC_2_PBS_ACCOUNTING, SYSCONF },
#endif
#ifdef _SC_2_PBS_LOCATE
    { "POSIX2_PBS_LOCATE", _SC_2_PBS_LOCATE, SYSCONF },
#endif
#ifdef _SC_2_PBS_TRACK
    { "POSIX2_PBS_TRACK", _SC_2_PBS_TRACK, SYSCONF },
#endif
#ifdef _SC_2_PBS_MESSAGE
    { "POSIX2_PBS_MESSAGE", _SC_2_PBS_MESSAGE, SYSCONF },
#endif
#ifdef _SC_SYMLOOP_MAX
    { "SYMLOOP_MAX", _SC_SYMLOOP_MAX, SYSCONF },
#endif
#ifdef _SC_STREAM_MAX
    { "STREAM_MAX", _SC_STREAM_MAX, SYSCONF },
#endif
#ifdef _SC_AIO_LISTIO_MAX
    { "AIO_LISTIO_MAX", _SC_AIO_LISTIO_MAX, SYSCONF },
#endif
#ifdef _SC_AIO_MAX
    { "AIO_MAX", _SC_AIO_MAX, SYSCONF },
#endif
#ifdef _SC_AIO_PRIO_DELTA_MAX
    { "AIO_PRIO_DELTA_MAX", _SC_AIO_PRIO_DELTA_MAX, SYSCONF },
#endif
#ifdef _SC_DELAYTIMER_MAX
    { "DELAYTIMER_MAX", _SC_DELAYTIMER_MAX, SYSCONF },
#endif
#ifdef _SC_HOST_NAME_MAX
    { "HOST_NAME_MAX", _SC_HOST_NAME_MAX, SYSCONF },
#endif
#ifdef _SC_LOGIN_NAME_MAX
    { "LOGIN_NAME_MAX", _SC_LOGIN_NAME_MAX, SYSCONF },
#endif
#ifdef _SC_MQ_OPEN_MAX
    { "MQ_OPEN_MAX", _SC_MQ_OPEN_MAX, SYSCONF },
#endif
#ifdef _SC_MQ_PRIO_MAX
    { "MQ_PRIO_MAX", _SC_MQ_PRIO_MAX, SYSCONF },
#endif
#ifdef _SC_DEVICE_IO
    { "_POSIX_DEVICE_IO", _SC_DEVICE_IO, SYSCONF },
#endif
#ifdef _SC_TRACE
    { "_POSIX_TRACE", _SC_TRACE, SYSCONF },
#endif
#ifdef _SC_TRACE_EVENT_FILTER
    { "_POSIX_TRACE_EVENT_FILTER", _SC_TRACE_EVENT_FILTER, SYSCONF },
#endif
#ifdef _SC_TRACE_INHERIT
    { "_POSIX_TRACE_INHERIT", _SC_TRACE_INHERIT, SYSCONF },
#endif
#ifdef _SC_TRACE_LOG
    { "_POSIX_TRACE_LOG", _SC_TRACE_LOG, SYSCONF },
#endif
#ifdef _SC_RTSIG_MAX
    { "RTSIG_MAX", _SC_RTSIG_MAX, SYSCONF },
#endif
#ifdef _SC_SEM_NSEMS_MAX
    { "SEM_NSEMS_MAX", _SC_SEM_NSEMS_MAX, SYSCONF },
#endif
#ifdef _SC_SEM_VALUE_MAX
    { "SEM_VALUE_MAX", _SC_SEM_VALUE_MAX, SYSCONF },
#endif
#ifdef _SC_SIGQUEUE_MAX
    { "SIGQUEUE_MAX", _SC_SIGQUEUE_MAX, SYSCONF },
#endif
#ifdef _PC_FILESIZEBITS
    { "FILESIZEBITS", _PC_FILESIZEBITS, PATHCONF },
#endif
#ifdef _PC_ALLOC_SIZE_MIN
    { "POSIX_ALLOC_SIZE_MIN", _PC_ALLOC_SIZE_MIN, PATHCONF },
#endif
#ifdef _PC_REC_INCR_XFER_SIZE
    { "POSIX_REC_INCR_XFER_SIZE", _PC_REC_INCR_XFER_SIZE, PATHCONF },
#endif
#ifdef _PC_REC_MAX_XFER_SIZE
    { "POSIX_REC_MAX_XFER_SIZE", _PC_REC_MAX_XFER_SIZE, PATHCONF },
#endif
#ifdef _PC_REC_MIN_XFER_SIZE
    { "POSIX_REC_MIN_XFER_SIZE", _PC_REC_MIN_XFER_SIZE, PATHCONF },
#endif
#ifdef _PC_REC_XFER_ALIGN
    { "POSIX_REC_XFER_ALIGN", _PC_REC_XFER_ALIGN, PATHCONF },
#endif
#ifdef _PC_SYMLINK_MAX
    { "SYMLINK_MAX", _PC_SYMLINK_MAX, PATHCONF },
#endif
#ifdef _CS_GNU_LIBC_VERSION
    { "GNU_LIBC_VERSION", _CS_GNU_LIBC_VERSION, CONFSTR },
#endif
#ifdef _CS_GNU_LIBPTHREAD_VERSION
    { "GNU_LIBPTHREAD_VERSION", _CS_GNU_LIBPTHREAD_VERSION, CONFSTR },
#endif
#ifdef _PC_2_SYMLINKS
    { "POSIX2_SYMLINKS", _PC_2_SYMLINKS, PATHCONF },
#endif

#ifdef _SC_LEVEL1_ICACHE_SIZE
    { "LEVEL1_ICACHE_SIZE", _SC_LEVEL1_ICACHE_SIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL1_ICACHE_ASSOC
    { "LEVEL1_ICACHE_ASSOC", _SC_LEVEL1_ICACHE_ASSOC, SYSCONF },
#endif
#ifdef _SC_LEVEL1_ICACHE_LINESIZE
    { "LEVEL1_ICACHE_LINESIZE", _SC_LEVEL1_ICACHE_LINESIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL1_DCACHE_SIZE
    { "LEVEL1_DCACHE_SIZE", _SC_LEVEL1_DCACHE_SIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL1_DCACHE_ASSOC
    { "LEVEL1_DCACHE_ASSOC", _SC_LEVEL1_DCACHE_ASSOC, SYSCONF },
#endif
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
    { "LEVEL1_DCACHE_LINESIZE", _SC_LEVEL1_DCACHE_LINESIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL2_CACHE_SIZE
    { "LEVEL2_CACHE_SIZE", _SC_LEVEL2_CACHE_SIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL2_CACHE_ASSOC
    { "LEVEL2_CACHE_ASSOC", _SC_LEVEL2_CACHE_ASSOC, SYSCONF },
#endif
#ifdef _SC_LEVEL2_CACHE_LINESIZE
    { "LEVEL2_CACHE_LINESIZE", _SC_LEVEL2_CACHE_LINESIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL3_CACHE_SIZE
    { "LEVEL3_CACHE_SIZE", _SC_LEVEL3_CACHE_SIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL3_CACHE_ASSOC
    { "LEVEL3_CACHE_ASSOC", _SC_LEVEL3_CACHE_ASSOC, SYSCONF },
#endif
#ifdef _SC_LEVEL3_CACHE_LINESIZE
    { "LEVEL3_CACHE_LINESIZE", _SC_LEVEL3_CACHE_LINESIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL4_CACHE_SIZE
    { "LEVEL4_CACHE_SIZE", _SC_LEVEL4_CACHE_SIZE, SYSCONF },
#endif
#ifdef _SC_LEVEL4_CACHE_ASSOC
    { "LEVEL4_CACHE_ASSOC", _SC_LEVEL4_CACHE_ASSOC, SYSCONF },
#endif
#ifdef _SC_LEVEL4_CACHE_LINESIZE
    { "LEVEL4_CACHE_LINESIZE", _SC_LEVEL4_CACHE_LINESIZE, SYSCONF },
#endif

#ifdef _SC_IPV6
    { "IPV6", _SC_IPV6, SYSCONF },
#endif
#ifdef _SC_RAW_SOCKETS
    { "RAW_SOCKETS", _SC_RAW_SOCKETS, SYSCONF },
#endif

    { NULL, 0, SYSCONF }
  };


static const struct { const char *name; int num; } specs[] =
  {
#ifdef _SC_XBS5_ILP32_OFF32
    { "XBS5_ILP32_OFF32", _SC_XBS5_ILP32_OFF32 },
#endif
#ifdef _SC_XBS5_ILP32_OFFBIG
    { "XBS5_ILP32_OFFBIG", _SC_XBS5_ILP32_OFFBIG },
#endif
#ifdef _SC_XBS5_LP64_OFF64
    { "XBS5_LP64_OFF64", _SC_XBS5_LP64_OFF64 },
#endif
#ifdef _SC_XBS5_LPBIG_OFFBIG
    { "XBS5_LPBIG_OFFBIG", _SC_XBS5_LPBIG_OFFBIG },
#endif
#ifdef _SC_V6_ILP32_OFF32
    { "POSIX_V6_ILP32_OFF32", _SC_V6_ILP32_OFF32 },
#endif
#ifdef _SC_V6_ILP32_OFFBIG
    { "POSIX_V6_ILP32_OFFBIG", _SC_V6_ILP32_OFFBIG },
#endif
#ifdef _SC_V6_LP64_OFF64
    { "POSIX_V6_LP64_OFF64", _SC_V6_LP64_OFF64 },
#endif
#ifdef _SC_V6_LPBIG_OFFBIG
    { "POSIX_V6_LPBIG_OFFBIG", _SC_V6_LPBIG_OFFBIG },
#endif
#ifdef _SC_V7_ILP32_OFF32
    { "POSIX_V7_ILP32_OFF32", _SC_V7_ILP32_OFF32 },
#endif
#ifdef _SC_V7_ILP32_OFFBIG
    { "POSIX_V7_ILP32_OFFBIG", _SC_V7_ILP32_OFFBIG },
#endif
#ifdef _SC_V7_LP64_OFF64
    { "POSIX_V7_LP64_OFF64", _SC_V7_LP64_OFF64 },
#endif
#ifdef _SC_V7_LPBIG_OFFBIG
    { "POSIX_V7_LPBIG_OFFBIG", _SC_V7_LPBIG_OFFBIG },
#endif
  };
static const int nspecs = sizeof (specs) / sizeof (specs[0]);

#ifdef __UCLIBC_HAS___PROGNAME__
extern const char *__progname;
#else
#define __progname "foo"
#endif

static void
usage (void)
{
  fprintf (stderr,
	   _("Usage: %s [-v specification] variable_name [pathname]\n"),
	   __progname);
  fprintf (stderr,
	   _("       %s -a [pathname]\n"), __progname);
  exit (2);
}


static void
print_all (const char *path)
{
  register const struct conf *c;
  size_t clen;
  long int value;
  char *cvalue;
  for (c = vars; c->name != NULL; ++c) {
    printf("%-35s", c->name);
    switch (c->call) {
      case PATHCONF:
	value = pathconf (path, c->call_name);
	if (value != -1) {
	  printf("%ld", value);
	}
	printf("\n");
	break;
      case SYSCONF:
	value = sysconf (c->call_name);
	if (value == -1l) {
	  if (c->call_name == _SC_UINT_MAX
	    || c->call_name == _SC_ULONG_MAX)
	    printf ("%lu", value);
	}
	else {
	  printf ("%ld", value);
	}
	printf ("\n");
	break;
      case CONFSTR:
	clen = confstr (c->call_name, (char *) NULL, 0);
	cvalue = (char *) malloc (clen);
	if (cvalue == NULL)
	  error (3, 0, _("memory exhausted"));
	if (confstr (c->call_name, cvalue, clen) != clen)
	  error (3, errno, "confstr");
	printf ("%.*s\n", (int) clen, cvalue);
	free (cvalue);
	break;
    }
  }
  exit (0);
}

int
main (int argc, char *argv[])
{
  register const struct conf *c;

  /* Set locale.  Do not set LC_ALL because the other categories must
     not be affected (according to POSIX.2).  */

  /* Initialize the message catalog.  */

  if (argc > 1 && strcmp (argv[1], "--version") == 0)
    {
      printf ("getconf (GNU %s) %s\n", PACKAGE, VERSION);
      printf ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
", "2009");
      printf ("Written by %s.\n", "Roland McGrath");
      return 0;
    }

  if (argc > 1 && strcmp (argv[1], "--help") == 0)
    {
      printf ("\
Usage: getconf [-v SPEC] VAR\n\
  or:  getconf [-v SPEC] PATH_VAR PATH\n\
\n\
Get the configuration value for variable VAR, or for variable PATH_VAR\n\
for path PATH.  If SPEC is given, give values for compilation\n\
environment SPEC.\n\n");
      fputs ("For bug reporting instructions, please see:\n\
<http://www.gnu.org/software/libc/bugs.html>.\n", stdout);
      return 0;
    }

  const char *getconf_dir = getenv ("GETCONF_DIR") ?: GETCONF_DIR;
  size_t getconf_dirlen = strlen (getconf_dir);

  const char *spec = NULL;
  char buf[sizeof "POSIX_V6_LPBIG_OFFBIG"];
  char *argv0 = argv[0];
  if (argc > 1 && strncmp (argv[1], "-v", 2) == 0)
    {
      if (argv[1][2] == '\0')
	{
	  if (argc < 3)
	    usage ();

	  spec = argv[2];
	  argv += 2;
	  argc -= 2;
	}
      else
	{
	  spec = &argv[1][2];
	  argv += 1;
	  argc += 1;
	}
    }
  else
    {
      char default_name[getconf_dirlen + sizeof "/default"];
      memcpy (mempcpy (default_name, getconf_dir, getconf_dirlen),
	      "/default", sizeof "/default");
      int len = readlink (default_name, buf, sizeof buf - 1);
      if (len > 0)
	{
	  buf[len] = '\0';
	  spec = buf;
	}
    }

  /* Check for the specifications we know.  */
  if (spec != NULL)
    {
      int i;
      for (i = 0; i < nspecs; ++i)
	if (strcmp (spec, specs[i].name) == 0)
	  break;

      if (i == nspecs)
	error (2, 0, _("unknown specification \"%s\""), spec);

      switch (specs[i].num)
	{
#if !defined(_XBS5_ILP32_OFF32) && defined(_SC_XBS5_ILP32_OFF32)
	case _SC_XBS5_ILP32_OFF32:
#endif
#if !defined(_XBS5_ILP32_OFFBIG) && defined(_SC_XBS5_ILP32_OFFBIG)
	case _SC_XBS5_ILP32_OFFBIG:
#endif
#if !defined(_XBS5_LP64_OFF64) && defined(_SC_XBS5_LP64_OFF64)
	case _SC_XBS5_LP64_OFF64:
#endif
#if !defined(_XBS5_LPBIG_OFFBIG) && defined(_SC_XBS5_LPBIG_OFFBIG)
	case _SC_XBS5_LPBIG_OFFBIG:
#endif
#if !defined(_POSIX_V6_ILP32_OFF32) && defined(_SC_V6_ILP32_OFF32)
	case _SC_V6_ILP32_OFF32:
#endif
#if !defined(_POSIX_V6_ILP32_OFFBIG) && defined(_SC_V6_ILP32_OFFBIG)
	case _SC_V6_ILP32_OFFBIG:
#endif
#if !defined(_POSIX_V6_LP64_OFF64) && defined(_SC_V6_LP64_OFF64)
	case _SC_V6_LP64_OFF64:
#endif
#if !defined(_POSIX_V6_LPBIG_OFFBIG) && defined(_SC_V6_LPBIG_OFFBIG)
	case _SC_V6_LPBIG_OFFBIG:
#endif
#if !defined(_POSIX_V7_ILP32_OFF32) && defined(_SC_V7_ILP32_OFF32)
	case _SC_V7_ILP32_OFF32:
#endif
#if !defined(_POSIX_V7_ILP32_OFFBIG) && defined(_SC_V7_ILP32_OFFBIG)
	case _SC_V7_ILP32_OFFBIG:
#endif
#if !defined(_POSIX_V7_LP64_OFF64) && defined(_SC_V7_LP64_OFF64)
	case _SC_V7_LP64_OFF64:
#endif
#if !defined(_POSIX_V7_LPBIG_OFFBIG) && defined(_SC_V7_LPBIG_OFFBIG)
	case _SC_V7_LPBIG_OFFBIG:
#endif
	    {
	      const char *args[argc + 3];
	      size_t spec_len = strlen (spec);
	      char getconf_name[getconf_dirlen + 1 + spec_len + 1];
	      memcpy (mempcpy (mempcpy (getconf_name, getconf_dir,
					getconf_dirlen),
			       "/", 1), spec, spec_len + 1);
	      args[0] = argv0;
	      args[1] = "-v";
	      args[2] = spec;
	      memcpy (&args[3], &argv[1], argc * sizeof (argv[1]));
	      execv (getconf_name, (char * const *) args);
	      error (4, errno, _("Couldn't execute %s"), getconf_name);
	    }
	  default:
	    break;
	}
    }

  if (argc > 1 && strcmp (argv[1], "-a") == 0)
    {
      if (argc == 2)
	print_all ("/");
      else if (argc == 3)
	print_all (argv[2]);
      else
	usage ();
    }

  int ai = 1;
  if (argc > ai && strcmp (argv[ai], "--") == 0)
    ++ai;

  if (argc - ai < 1 || argc - ai > 2)
    usage ();

  for (c = vars; c->name != NULL; ++c)
    if (strcmp (c->name, argv[ai]) == 0
	|| (strncmp (c->name, "_POSIX_", 7) == 0
	    && strcmp (c->name + 7, argv[ai]) == 0))
      {
	long int value;
	size_t clen;
	char *cvalue;
	switch (c->call)
	  {
	  case PATHCONF:
	    if (argc - ai < 2)
	      usage ();
	    errno = 0;
	    value = pathconf (argv[ai + 1], c->call_name);
	    if (value == -1)
	      {
		if (errno) {
		  error (3, errno, "pathconf: %s", argv[ai + 1]);
		} else
		  puts (_("undefined"));
	      }
	    else
	      printf ("%ld\n", value);
	    exit (0);

	  case SYSCONF:
	    if (argc - ai > 1)
	      usage ();
	    value = sysconf (c->call_name);
	    if (value == -1l)
	      {
		if (c->call_name == _SC_UINT_MAX
		    || c->call_name == _SC_ULONG_MAX)
		  printf ("%lu\n", value);
		else
		  puts (_("undefined"));
	      }
	    else
	      printf ("%ld\n", value);
	    exit (0);

	  case CONFSTR:
	    if (argc - ai > 1)
	      usage ();
	    clen = confstr (c->call_name, (char *) NULL, 0);
	    cvalue = (char *) malloc (clen);
	    if (cvalue == NULL)
	      error (3, 0, _("memory exhausted"));

	    if (confstr (c->call_name, cvalue, clen) != clen)
	      error (3, errno, "confstr");

	    printf ("%.*s\n", (int) clen, cvalue);
	    exit (0);
	  }
      }

  error (2, 0, _("Unrecognized variable `%s'"), argv[ai]);
  /* NOTREACHED */
  return 2;
}
