/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define __FORCE_GLIBC
#include <features.h>
#include <errno.h>
#include <syscall.h>
#include <sys/socket.h>

#ifdef __NR_socketcall
extern int __socketcall(int call, unsigned long *args) attribute_hidden;

/* Various socketcall numbers */
#define SYS_SOCKET      1
#define SYS_BIND        2
#define SYS_CONNECT     3
#define SYS_LISTEN      4
#define SYS_ACCEPT      5
#define SYS_GETSOCKNAME 6
#define SYS_GETPEERNAME 7
#define SYS_SOCKETPAIR  8
#define SYS_SEND        9
#define SYS_RECV        10
#define SYS_SENDTO      11
#define SYS_RECVFROM    12
#define SYS_SHUTDOWN    13
#define SYS_SETSOCKOPT  14
#define SYS_GETSOCKOPT  15
#define SYS_SENDMSG     16
#define SYS_RECVMSG     17
#define SYS_ACCEPT4     18
#endif

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#include <pthreadP.h>
#else
#define SINGLE_THREAD_P 1
#endif

#ifdef L_accept
extern __typeof(accept) __libc_accept;
#ifdef __NR_accept
#define __NR___sys_accept  __NR_accept
static
_syscall3(int, __sys_accept, int, call, struct sockaddr *, addr, socklen_t *,addrlen)
int __libc_accept(int s, struct sockaddr *addr, socklen_t * addrlen)
{
	if (SINGLE_THREAD_P)
		return __sys_accept(s, addr, addrlen);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_accept(s, addr, addrlen);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_socketcall)
int __libc_accept(int s, struct sockaddr *addr, socklen_t * addrlen)
{
	unsigned long args[3];

	args[0] = s;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) addrlen;

	if (SINGLE_THREAD_P)
		return __socketcall(SYS_ACCEPT, args);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_ACCEPT, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#endif
weak_alias(__libc_accept,accept)
libc_hidden_weak(accept)
#endif

#ifdef L_accept4
#ifdef __NR_accept4
# define __NR___sys_accept4  __NR_accept4
static _syscall4(int, __sys_accept4, int, fd, struct sockaddr *, addr, socklen_t *, addrlen, int, flags)
int accept4(int fd, struct sockaddr *addr, socklen_t * addrlen, int flags)
{
	if (SINGLE_THREAD_P)
		return __sys_accept4(fd, addr, addrlen, flags);
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	else {
		int oldtype = LIBC_CANCEL_ASYNC ();
		int result = __sys_accept4(fd, addr, addrlen, flags);
		LIBC_CANCEL_RESET (oldtype);
		return result;
	}
#endif
}
#elif defined(__NR_socketcall)
int accept4(int fd, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
	unsigned long args[4];

	args[0] = fd;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) addrlen;
	args[3] = flags;
	if (SINGLE_THREAD_P)
		return __socketcall(SYS_ACCEPT4, args);
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	else {
		int oldtype = LIBC_CANCEL_ASYNC ();
		int result = __socketcall(SYS_ACCEPT4, args);
		LIBC_CANCEL_RESET (oldtype);
		return result;
	}
#endif
}
#endif
#endif

#ifdef L_bind
#ifdef __NR_bind
_syscall3(int, bind, int, sockfd, const struct sockaddr *, myaddr, socklen_t, addrlen)
#elif defined(__NR_socketcall)
int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) myaddr;
	args[2] = addrlen;
	return __socketcall(SYS_BIND, args);
}
#endif
libc_hidden_def(bind)
#endif

#ifdef L_connect
extern __typeof(connect) __libc_connect;
#ifdef __NR_connect
#define __NR___sys_connect __NR_connect
static
_syscall3(int, __sys_connect, int, sockfd, const struct sockaddr *, saddr, socklen_t, addrlen)
int __libc_connect(int sockfd, const struct sockaddr *saddr, socklen_t addrlen)
{
	if (SINGLE_THREAD_P)
		return __sys_connect(sockfd, saddr, addrlen);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_connect(sockfd, saddr, addrlen);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_socketcall)
int __libc_connect(int sockfd, const struct sockaddr *saddr, socklen_t addrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) saddr;
	args[2] = addrlen;

	if (SINGLE_THREAD_P)
		return __socketcall(SYS_CONNECT, args);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_CONNECT, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#endif
weak_alias(__libc_connect,connect)
libc_hidden_weak(connect)
#endif

#ifdef L_getpeername
#ifdef __NR_getpeername
_syscall3(int, getpeername, int, sockfd, struct sockaddr *, addr, socklen_t *,paddrlen)
#elif defined(__NR_socketcall)
int getpeername(int sockfd, struct sockaddr *addr, socklen_t * paddrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) paddrlen;
	return __socketcall(SYS_GETPEERNAME, args);
}
#endif
#endif

#ifdef L_getsockname
#ifdef __NR_getsockname
_syscall3(int, getsockname, int, sockfd, struct sockaddr *, addr, socklen_t *,paddrlen)
#elif defined(__NR_socketcall)
int getsockname(int sockfd, struct sockaddr *addr, socklen_t * paddrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) paddrlen;
	return __socketcall(SYS_GETSOCKNAME, args);
}
#endif
libc_hidden_def(getsockname)
#endif

#ifdef L_getsockopt
#ifdef __NR_getsockopt
_syscall5(int, getsockopt, int, fd, int, level, int, optname, __ptr_t, optval, socklen_t *, optlen)
#elif defined(__NR_socketcall)
int getsockopt(int fd, int level, int optname, __ptr_t optval,
		   socklen_t * optlen)
{
	unsigned long args[5];

	args[0] = fd;
	args[1] = level;
	args[2] = optname;
	args[3] = (unsigned long) optval;
	args[4] = (unsigned long) optlen;
	return (__socketcall(SYS_GETSOCKOPT, args));
}
#endif
#endif

#ifdef L_listen
#ifdef __NR_listen
_syscall2(int, listen, int, sockfd, int, backlog)
#elif defined(__NR_socketcall)
int listen(int sockfd, int backlog)
{
	unsigned long args[2];

	args[0] = sockfd;
	args[1] = backlog;
	return __socketcall(SYS_LISTEN, args);
}
#endif
libc_hidden_def(listen)
#endif

#ifdef L_recv
extern __typeof(recv) __libc_recv;
#ifdef __NR_recv
#define __NR___sys_recv __NR_recv
static
_syscall4(ssize_t, __sys_recv, int, sockfd, __ptr_t, buffer, size_t, len,
	int, flags)
ssize_t __libc_recv(int sockfd, __ptr_t buffer, size_t len, int flags)
{
	if (SINGLE_THREAD_P)
		return __sys_recv(sockfd, buffer, len, flags);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_recv(sockfd, buffer, len, flags);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_recvfrom)
ssize_t __libc_recv(int sockfd, __ptr_t buffer, size_t len, int flags)
{
	return (recvfrom(sockfd, buffer, len, flags, NULL, NULL));
}
#elif defined(__NR_socketcall)
/* recv, recvfrom added by bir7@leland.stanford.edu */
ssize_t __libc_recv(int sockfd, __ptr_t buffer, size_t len, int flags)
{
	unsigned long args[4];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;

	if (SINGLE_THREAD_P)
		return (__socketcall(SYS_RECV, args));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_RECV, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#endif
weak_alias(__libc_recv,recv)
libc_hidden_weak(recv)
#endif

#ifdef L_recvfrom
extern __typeof(recvfrom) __libc_recvfrom;
#ifdef __NR_recvfrom
#define __NR___sys_recvfrom __NR_recvfrom
static
_syscall6(ssize_t, __sys_recvfrom, int, sockfd, __ptr_t, buffer, size_t, len,
	int, flags, struct sockaddr *, to, socklen_t *, tolen)
ssize_t __libc_recvfrom(int sockfd, __ptr_t buffer, size_t len, int flags,
		 struct sockaddr *to, socklen_t * tolen)
{
	if (SINGLE_THREAD_P)
		return __sys_recvfrom(sockfd, buffer, len, flags, to, tolen);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_recvfrom(sockfd, buffer, len, flags, to, tolen);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_socketcall)
/* recv, recvfrom added by bir7@leland.stanford.edu */
ssize_t __libc_recvfrom(int sockfd, __ptr_t buffer, size_t len, int flags,
		 struct sockaddr *to, socklen_t * tolen)
{
	unsigned long args[6];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;
	args[4] = (unsigned long) to;
	args[5] = (unsigned long) tolen;

	if (SINGLE_THREAD_P)
		return (__socketcall(SYS_RECVFROM, args));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_RECVFROM, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#endif
weak_alias(__libc_recvfrom,recvfrom)
libc_hidden_weak(recvfrom)
#endif

#ifdef L_recvmsg
extern __typeof(recvmsg) __libc_recvmsg;
#ifdef __NR_recvmsg
#define __NR___sys_recvmsg __NR_recvmsg
static
_syscall3(ssize_t, __sys_recvmsg, int, sockfd, struct msghdr *, msg, int, flags)
ssize_t __libc_recvmsg(int sockfd, struct msghdr *msg, int flags)
{
	if (SINGLE_THREAD_P)
		return __sys_recvmsg(sockfd, msg, flags);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_recvmsg(sockfd, msg, flags);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_socketcall)
ssize_t __libc_recvmsg(int sockfd, struct msghdr *msg, int flags)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) msg;
	args[2] = flags;

	if (SINGLE_THREAD_P)
		return (__socketcall(SYS_RECVMSG, args));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_RECVMSG, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#endif
weak_alias(__libc_recvmsg,recvmsg)
libc_hidden_weak(recvmsg)
#endif

#ifdef L_send
extern __typeof(send) __libc_send;
#ifdef __NR_send
#define __NR___sys_send    __NR_send
static
_syscall4(ssize_t, __sys_send, int, sockfd, const void *, buffer, size_t, len, int, flags)
ssize_t __libc_send(int sockfd, const void *buffer, size_t len, int flags)
{
	if (SINGLE_THREAD_P)
		return __sys_send(sockfd, buffer, len, flags);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_send(sockfd, buffer, len, flags);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_sendto)
ssize_t __libc_send(int sockfd, const void *buffer, size_t len, int flags)
{
	return (sendto(sockfd, buffer, len, flags, NULL, 0));
}
#elif defined(__NR_socketcall)
/* send, sendto added by bir7@leland.stanford.edu */
ssize_t __libc_send(int sockfd, const void *buffer, size_t len, int flags)
{
	unsigned long args[4];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;

	if (SINGLE_THREAD_P)
		return (__socketcall(SYS_SEND, args));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_SEND, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}

#endif
weak_alias(__libc_send,send)
libc_hidden_weak(send)
#endif

#ifdef L_sendmsg
extern __typeof(sendmsg) __libc_sendmsg;
#ifdef __NR_sendmsg
#define __NR___sys_sendmsg __NR_sendmsg
static
_syscall3(ssize_t, __sys_sendmsg, int, sockfd, const struct msghdr *, msg, int, flags)
ssize_t __libc_sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
	if (SINGLE_THREAD_P)
		return __sys_sendmsg(sockfd, msg, flags);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_sendmsg(sockfd, msg, flags);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_socketcall)
ssize_t __libc_sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) msg;
	args[2] = flags;

	if (SINGLE_THREAD_P)
		return (__socketcall(SYS_SENDMSG, args));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_SENDMSG, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#endif
weak_alias(__libc_sendmsg,sendmsg)
libc_hidden_weak(sendmsg)
#endif

#ifdef L_sendto
extern __typeof(sendto) __libc_sendto;
#ifdef __NR_sendto
#define __NR___sys_sendto  __NR_sendto
static
_syscall6(ssize_t, __sys_sendto, int, sockfd, const void *, buffer,
	size_t, len, int, flags, const struct sockaddr *, to, socklen_t, tolen)
ssize_t __libc_sendto(int sockfd, const void *buffer, size_t len, int flags,const struct sockaddr *to, socklen_t tolen)
{
	if (SINGLE_THREAD_P)
		return __sys_sendto(sockfd, buffer, len, flags, to, tolen);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __sys_sendto(sockfd, buffer, len, flags, to, tolen);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#elif defined(__NR_socketcall)
/* send, sendto added by bir7@leland.stanford.edu */
ssize_t __libc_sendto(int sockfd, const void *buffer, size_t len, int flags,
	   const struct sockaddr *to, socklen_t tolen)
{
	unsigned long args[6];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;
	args[4] = (unsigned long) to;
	args[5] = tolen;

	if (SINGLE_THREAD_P)
		return (__socketcall(SYS_SENDTO, args));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __socketcall(SYS_SENDTO, args);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
#endif
weak_alias(__libc_sendto,sendto)
libc_hidden_weak(sendto)
#endif

#ifdef L_setsockopt
#ifdef __NR_setsockopt
_syscall5(int, setsockopt, int, fd, int, level, int, optname, const void *, optval, socklen_t, optlen)
#elif defined(__NR_socketcall)
/* [sg]etsockoptions by bir7@leland.stanford.edu */
int setsockopt(int fd, int level, int optname, const void *optval,
		   socklen_t optlen)
{
	unsigned long args[5];

	args[0] = fd;
	args[1] = level;
	args[2] = optname;
	args[3] = (unsigned long) optval;
	args[4] = optlen;
	return (__socketcall(SYS_SETSOCKOPT, args));
}
#endif
libc_hidden_def(setsockopt)
#endif

#ifdef L_shutdown
#ifdef __NR_shutdown
_syscall2(int, shutdown, int, sockfd, int, how)
#elif defined(__NR_socketcall)
/* shutdown by bir7@leland.stanford.edu */
int shutdown(int sockfd, int how)
{
	unsigned long args[2];

	args[0] = sockfd;
	args[1] = how;
	return (__socketcall(SYS_SHUTDOWN, args));
}
#endif
#endif

#ifdef L_socket
#ifdef __NR_socket
_syscall3(int, socket, int, family, int, type, int, protocol)
#elif defined(__NR_socketcall)
int socket(int family, int type, int protocol)
{
	unsigned long args[3];

	args[0] = family;
	args[1] = type;
	args[2] = (unsigned long) protocol;
	return __socketcall(SYS_SOCKET, args);
}
#endif
libc_hidden_def(socket)
#endif

#ifdef L_socketpair
#ifdef __NR_socketpair
_syscall4(int, socketpair, int, family, int, type, int, protocol, int *, sockvec)
#elif defined(__NR_socketcall)
int socketpair(int family, int type, int protocol, int sockvec[2])
{
	unsigned long args[4];

	args[0] = family;
	args[1] = type;
	args[2] = protocol;
	args[3] = (unsigned long) sockvec;
	return __socketcall(SYS_SOCKETPAIR, args);
}
#endif
#endif

