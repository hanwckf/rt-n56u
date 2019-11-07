/*
 * system call not available stub
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Please keep the list sorted alphabetically, in ascending order
 * of the stub name!  */

#include <errno.h>
#include <bits/wordsize.h>
#include <sys/syscall.h>

#ifdef __UCLIBC_HAS_STUBS__

__attribute_used__ static int enosys_stub(void)
{
	__set_errno(ENOSYS);
	return -1;
}

__attribute_used__ static int ret_enosys_stub(void)
{
	return ENOSYS;
}

#define make_stub(stub) \
	link_warning(stub, #stub ": this function is not implemented") \
	strong_alias(enosys_stub, stub)

#define make_ret_stub(stub) \
	link_warning(stub, #stub ": this function is not implemented") \
	strong_alias(ret_enosys_stub, stub)

#ifndef __ARCH_USE_MMU__
# undef __NR_fork
#endif

#ifdef __arm__
# define __NR_fadvise64_64 __NR_arm_fadvise64_64
# define __NR_fadvise64 __NR_arm_fadvise64_64
/* ARM always provides funcs w/out syscalls; disable the stubs */
# define __NR_ioperm 0
# define __NR_iopl 0
#endif

#ifdef __mips__
# define __NR_fadvise64_64 __NR_fadvise64
#endif

#ifdef __xtensa__
# define __NR_fadvise64 __NR_fadvise64_64
#endif

#if !defined __NR_accept && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(accept)
#endif

#if !defined __NR_accept4 && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__ && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(accept4)
#endif

#if !defined __NR_arch_prctl && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(arch_prctl)
#endif

#if !defined __NR_bdflush && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(bdflush)
#endif

#if !defined __NR_bind && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(bind)
#endif

#if !defined __NR_capget && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(capget)
#endif

#if !defined __NR_capset && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(capset)
#endif

#if !defined __NR_connect && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(connect)
#endif

#if !defined __NR_epoll_create && defined __UCLIBC_HAS_EPOLL__ \
	&& !defined __NR_epoll_create1
make_stub(epoll_create)
#endif

#if !defined __NR_epoll_ctl && defined __UCLIBC_HAS_EPOLL__
make_stub(epoll_ctl)
#endif

#if !defined __NR_epoll_pwait && defined __UCLIBC_HAS_EPOLL__
make_stub(epoll_pwait)
#endif

#if !defined __NR_epoll_wait && defined __UCLIBC_HAS_EPOLL__ \
		&& !defined __NR_epoll_pwait
make_stub(epoll_wait)
#endif

#if !defined __NR_eventfd && !defined __NR_eventfd2 && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(eventfd)
#endif

#if !defined __NR_fdatasync && !defined __NR_osf_fdatasync
make_stub(fdatasync)
#endif

#ifndef __NR_fgetxattr
make_stub(fgetxattr)
#endif

#ifndef __NR_flistxattr
make_stub(flistxattr)
#endif

#if !defined __NR_fork && !defined __NR_clone
make_stub(fork)
#endif

#ifndef __NR_fremovexattr
make_stub(fremovexattr)
#endif

#ifndef __NR_fsetxattr
make_stub(fsetxattr)
#endif

#if !defined __NR_fstatfs && !defined __NR_fstatfs64 \
	&& defined __UCLIBC_LINUX_SPECIFIC__
make_stub(fstatfs)
#endif

#ifndef __NR_get_kernel_syms
make_stub(get_kernel_syms)
#endif

#if !defined __NR_getpeername && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(getpeername)
#endif

#if !defined __NR_getpgrp && !defined __NR_getpgid
make_stub(getpgrp)
#endif

#if !defined __NR_getrandom && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(getrandom)
#endif

#if !defined __NR_getsockname && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(getsockname)
#endif

#if !defined __NR_getsockopt && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(getsockopt)
#endif

#ifndef __NR_getxattr
make_stub(getxattr)
#endif

#if !defined __NR_inotify_add_watch && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(inotify_add_watch)
#endif

#if !defined __NR_inotify_init && defined __UCLIBC_LINUX_SPECIFIC__ \
		&& !defined __NR_inotify_init1
make_stub(inotify_init)
#endif

#if !defined __NR_inotify_init1 && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(inotify_init1)
#endif

#if !defined __NR_inotify_rm_watch && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(inotify_rm_watch)
#endif

#if !defined __NR_ioperm && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(ioperm)
#endif

#if !defined __NR_iopl && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(iopl)
#endif

#ifndef __NR_lgetxattr
make_stub(lgetxattr)
#endif

#if !defined __NR_listen && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(listen)
#endif

#ifndef __NR_listxattr
make_stub(listxattr)
#endif

#ifndef __NR_llistxattr
make_stub(llistxattr)
#endif

#ifndef __NR_lremovexattr
make_stub(lremovexattr)
#endif

#ifndef __NR_lsetxattr
make_stub(lsetxattr)
#endif

#if !defined __NR_madvise && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(madvise)
#endif

#if !defined __NR_modify_ldt && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(modify_ldt)
#endif

#ifndef __NR_openat
make_stub(openat)
make_stub(openat64)
#endif

#if !defined __NR_personality && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(personality)
#endif

#if !defined __NR_pipe2 && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(pipe2)
#endif

#if !defined __NR_pivot_root && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(pivot_root)
#endif

#if !defined __NR_fadvise64
make_ret_stub(posix_fadvise)
#endif

#if !defined __NR_fadvise64_64 && __WORDSIZE == 32
make_ret_stub(posix_fadvise64)
#endif

#ifndef __NR_madvise
make_ret_stub(posix_madvise)
#endif

#if !defined __NR_ppoll && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(ppoll)
#endif

#if !defined __NR_prctl && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(prctl)
#endif

#if !defined __NR_readahead && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(readahead)
#endif

#if !defined __NR_reboot && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(reboot)
#endif

#if !defined __NR_recv && !defined __NR_socketcall && !defined __NR_recvfrom && defined __UCLIBC_HAS_SOCKET__
make_stub(recv)
#endif

#if !defined __NR_recvfrom && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(recvfrom)
#endif

#if !defined __NR_recvmsg && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(recvmsg)
#endif

#if !defined __NR_remap_file_pages && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(remap_file_pages)
#endif

#ifndef __NR_removexattr
make_stub(removexattr)
#endif

#ifndef __NR_renameat
make_stub(renameat)
#endif

#if !defined __NR_sched_getaffinity && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(sched_getaffinity)
#endif

#if !defined __NR_getcpu && defined __UCLIBC_LINUX_SPECIFIC__ && ((defined __x86_64__ && !defined __UCLIBC_HAS_TLS__) || !defined __x86_64__)
make_stub(sched_getcpu)
#endif

#if !defined __NR_sched_setaffinity && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(sched_setaffinity)
#endif

#if !defined __NR_send && !defined __NR_socketcall && !defined __NR_sendto && defined __UCLIBC_HAS_SOCKET__
make_stub(send)
#endif

#if !defined __NR_sendfile && !defined __NR_sendfile64 \
	&& defined __UCLIBC_LINUX_SPECIFIC__
make_stub(sendfile)
#endif

#if !defined __NR_sendfile64 && !defined __NR_sendfile && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(sendfile64)
#endif

#if !defined __NR_sendmsg && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(sendmsg)
#endif

#if !defined __NR_sendto && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(sendto)
#endif

#if ((__WORDSIZE == 32 && (!defined __NR_setfsgid32 && !defined __NR_setfsgid)) || (__WORDSIZE == 64 && !defined __NR_setfsgid)) && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(setfsgid)
#endif

#if ((__WORDSIZE == 32 && (!defined __NR_setfsuid32 && !defined __NR_setfsuid)) || (__WORDSIZE == 64 && !defined __NR_setfsuid)) && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(setfsuid)
#endif

#if !defined __NR_setns && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(setns)
#endif

#if !defined __NR_setresgid32 && !defined __NR_setresgid && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(setresgid)
#endif

#if !defined __NR_setresuid32 && !defined __NR_setresuid && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(setresuid)
#endif

#if !defined __NR_setsockopt && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(setsockopt)
#endif

#ifndef __NR_setxattr
make_stub(setxattr)
#endif

#if !defined __NR_shutdown && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(shutdown)
#endif

#if !defined __NR_signalfd4 && !defined __NR_signalfd && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(signalfd)
#endif

#ifndef __NR_rt_sigtimedwait
make_stub(sigtimedwait)
make_stub(sigwaitinfo)
#endif

#ifndef __NR_rt_sigqueueinfo
make_stub(sigqueue)
#endif

#if !defined __NR_socket && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(socket)
#endif

#if !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(socketcall)
#endif

#if !defined __NR_socketpair && !defined __NR_socketcall && defined __UCLIBC_HAS_SOCKET__
make_stub(socketpair)
#endif

#if !defined __NR_stime && !defined __NR_settimeofday
make_stub(stime)
make_stub(settimeofday)
#endif

#if !defined __NR_splice && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(splice)
#endif

#if !defined __NR_swapoff && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(swapoff)
#endif

#if !defined __NR_swapon && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(swapon)
#endif

#if !defined __NR_symlink && !defined __NR_symlinkat
make_stub(symlink)
#endif

#if !defined __NR_sync_file_range && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(sync_file_range)
#endif

#if !defined __NR__sysctl && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(_sysctl)
#endif

#if !defined __NR_sysinfo && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(sysinfo)
#endif

#if !defined __NR_tee && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(tee)
#endif

#if !defined __NR_timerfd_create && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(timerfd_create)
#endif

#if !defined __NR_timerfd_settime && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(timerfd_settime)
#endif

#if !defined __NR_timerfd_gettime && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(timerfd_gettime)
#endif

#ifndef __NR_utimensat
make_stub(futimens)
make_stub(utimensat)
# ifndef __NR_lutimes
make_stub(lutimes)
# endif
#endif

#ifndef __NR_utimensat
#if !defined __NR_utime && !defined __NR_utimes
/*make_stub(utime) obsoleted */
make_stub(utimes)
#endif
#endif

#if !defined __NR_umount && !defined __NR_umount2 && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(umount)
#endif

#if !defined __NR_umount2 && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(umount2)
#endif

#if !defined __NR_unshare && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(unshare)
#endif

#if defined __UCLIBC_SV4_DEPRECATED__ && !defined __NR_ustat
make_stub(ustat)
#endif

#if !defined __NR_vhangup && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(vhangup)
#endif

#if !defined __NR_vmsplice && defined __UCLIBC_LINUX_SPECIFIC__
make_stub(vmsplice)
#endif

#endif
