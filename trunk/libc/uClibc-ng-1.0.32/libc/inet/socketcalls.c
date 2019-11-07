/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stddef.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <cancel.h>
#include <bits/kernel-features.h>

#ifdef __NR_socketcall
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
#define SYS_RECVMMSG    19
#define SYS_SENDMMSG    20
#endif

/* exposed on x86 since Linux commit 9dea5dc921b5f4045a18c63eb92e84dc274d17eb */
#if defined(__sparc__) || defined(__i386__)
#undef __NR_accept
#undef __NR_accept4
#undef __NR_bind
#undef __NR_connect
#undef __NR_getpeername
#undef __NR_getsockname
#undef __NR_getsockopt
#undef __NR_listen
#undef __NR_recv
#undef __NR_recvfrom
#undef __NR_recvmsg
#undef __NR_send
#undef __NR_sendmsg
#undef __NR_sendto
#undef __NR_setsockopt
#undef __NR_shutdown
#undef __NR_socket
#undef __NR_socketpair
#endif

#ifdef L_accept
static int __NC(accept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
# ifdef __NR_accept
	return INLINE_SYSCALL(accept, 3, sockfd, addr, addrlen);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) addrlen;

	return __socketcall(SYS_ACCEPT, args);
# endif
}
CANCELLABLE_SYSCALL(int, accept, (int sockfd, struct sockaddr *addr, socklen_t *addrlen),
		    (sockfd, addr, addrlen))
lt_libc_hidden(accept)
#endif

#ifdef L_accept4
static int __NC(accept4)(int fd, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
# ifdef __NR_accept4
	return INLINE_SYSCALL(accept4, 4, fd, addr, addrlen, flags);
# elif defined(__NR_socketcall)
	unsigned long args[4];

	args[0] = fd;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) addrlen;
	args[3] = flags;

	return __socketcall(SYS_ACCEPT4, args);
#endif
}
CANCELLABLE_SYSCALL(int, accept4, (int fd, struct sockaddr *addr, socklen_t *addrlen, int flags),
		    (fd, addr, addrlen, flags))
lt_libc_hidden(accept4)
#endif

#ifdef L_bind
int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen)
{
# ifdef __NR_bind
	return INLINE_SYSCALL(bind, 3, sockfd, myaddr, addrlen);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) myaddr;
	args[2] = addrlen;
	return __socketcall(SYS_BIND, args);
# endif
}
libc_hidden_def(bind)
#endif

#ifdef L_connect
static int __NC(connect)(int sockfd, const struct sockaddr *saddr, socklen_t addrlen)
{
# ifdef __NR_connect
	return INLINE_SYSCALL(connect, 3, sockfd, saddr, addrlen);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) saddr;
	args[2] = addrlen;
	return __socketcall(SYS_CONNECT, args);
# endif
}
CANCELLABLE_SYSCALL(int, connect, (int sockfd, const struct sockaddr *saddr, socklen_t addrlen),
		    (sockfd, saddr, addrlen))
lt_libc_hidden(connect)
#endif

#ifdef L_getpeername
int getpeername(int sockfd, struct sockaddr *addr, socklen_t *paddrlen)
{
# ifdef __NR_getpeername
	return INLINE_SYSCALL(getpeername, 3, sockfd, addr, paddrlen);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) paddrlen;
	return __socketcall(SYS_GETPEERNAME, args);
# endif
}
#endif

#ifdef L_getsockname
int getsockname(int sockfd, struct sockaddr *addr, socklen_t * paddrlen)
{
# ifdef __NR_getsockname
	return INLINE_SYSCALL(getsockname, 3, sockfd, addr, paddrlen);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) addr;
	args[2] = (unsigned long) paddrlen;
	return __socketcall(SYS_GETSOCKNAME, args);
# endif
}
libc_hidden_def(getsockname)
#endif

#ifdef L_getsockopt
int getsockopt(int fd, int level, int optname, void *optval,
	       socklen_t *optlen)
{
# ifdef __NR_getsockopt
	return INLINE_SYSCALL(getsockopt, 5, fd, level, optname, optval, optlen);
# elif defined(__NR_socketcall)
	unsigned long args[5];

	args[0] = fd;
	args[1] = level;
	args[2] = optname;
	args[3] = (unsigned long) optval;
	args[4] = (unsigned long) optlen;
	return (__socketcall(SYS_GETSOCKOPT, args));
# endif
}
#endif

#ifdef L_listen
int listen(int sockfd, int backlog)
{
# ifdef __NR_listen
	return INLINE_SYSCALL(listen, 2, sockfd, backlog);
# elif defined(__NR_socketcall)
	unsigned long args[2];

	args[0] = sockfd;
	args[1] = backlog;
	return __socketcall(SYS_LISTEN, args);
# endif
}
libc_hidden_def(listen)
#endif

#ifdef L_recv
static ssize_t __NC(recv)(int sockfd, void *buffer, size_t len, int flags)
{
# ifdef __NR_recv
	return (ssize_t)INLINE_SYSCALL(recv, 4, sockfd, buffer, len, flags);
# elif defined __NR_recvfrom && defined _syscall6
	return __NC(recvfrom)(sockfd, buffer, len, flags, NULL, NULL);
# elif defined(__NR_socketcall)
	unsigned long args[4];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;
	return (ssize_t)__socketcall(SYS_RECV, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, recv, (int sockfd, void *buffer, size_t len, int flags),
		    (sockfd, buffer, len, flags))
lt_libc_hidden(recv)
#endif

#ifdef L_recvfrom
ssize_t __NC(recvfrom)(int sockfd, void *buffer, size_t len, int flags,
		       struct sockaddr *to, socklen_t *tolen)
{
# if defined __NR_recvfrom && defined _syscall6
	return (ssize_t)INLINE_SYSCALL(recvfrom, 6, sockfd, buffer, len,
				       flags, to, tolen);
# elif defined(__NR_socketcall)
	unsigned long args[6];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;
	args[4] = (unsigned long) to;
	args[5] = (unsigned long) tolen;
	return (ssize_t)__socketcall(SYS_RECVFROM, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, recvfrom, (int sockfd, void *buffer, size_t len,
					int flags, struct sockaddr *to, socklen_t *tolen),
		    (sockfd, buffer, len, flags, to, tolen))
lt_libc_hidden(recvfrom)
#endif

#ifdef L_recvmsg
static ssize_t __NC(recvmsg)(int sockfd, struct msghdr *msg, int flags)
{
# ifdef __NR_recvmsg
	return (ssize_t)INLINE_SYSCALL(recvmsg, 3, sockfd, msg, flags);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) msg;
	args[2] = flags;
	return (ssize_t)__socketcall(SYS_RECVMSG, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, recvmsg, (int sockfd, struct msghdr *msg, int flags),
		    (sockfd, msg, flags))
lt_libc_hidden(recvmsg)
#endif

#ifdef L_recvmmsg
#ifdef __ASSUME_RECVMMSG_SYSCALL
static ssize_t __NC(recvmmsg)(int sockfd, struct mmsghdr *msg, size_t vlen,
			      int flags, struct timespec *tmo)
{
# ifdef __NR_recvmmsg
	return (ssize_t)INLINE_SYSCALL(recvmmsg, 5, sockfd, msg, vlen, flags, tmo);
# elif __NR_socketcall
	unsigned long args[5];

	args[0] = sockfd;
	args[1] = (unsigned long) msg;
	args[2] = vlen;
	args[3] = flags;
	args[4] = (unsigned long) tmo;
	return (ssize_t)__socketcall(SYS_RECVMMSG, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, recvmmsg,
		    (int sockfd, struct mmsghdr *msg, size_t vlen, int flags,
		     struct timespec *tmo),
		    (sockfd, msg, vlen, flags, tmo))
lt_libc_hidden(recvmmsg)
#endif
#endif

#ifdef L_send
static ssize_t __NC(send)(int sockfd, const void *buffer, size_t len, int flags)
{
# ifdef __NR_send
	return (ssize_t)INLINE_SYSCALL(send, 4, sockfd, buffer, len, flags);
# elif defined __NR_sendto && defined _syscall6
	return __NC(sendto)(sockfd, buffer, len, flags, NULL, 0);
# elif defined(__NR_socketcall)
	unsigned long args[4];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;
	return (ssize_t)__socketcall(SYS_SEND, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, send, (int sockfd, const void *buffer, size_t len, int flags),
		    (sockfd, buffer, len, flags))
lt_libc_hidden(send)
#endif

#ifdef L_sendmsg
static ssize_t __NC(sendmsg)(int sockfd, const struct msghdr *msg, int flags)
{
# ifdef __NR_sendmsg
	return (ssize_t)INLINE_SYSCALL(sendmsg, 3, sockfd, msg, flags);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long) msg;
	args[2] = flags;
	return (ssize_t)__socketcall(SYS_SENDMSG, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, sendmsg, (int sockfd, const struct msghdr *msg, int flags),
		    (sockfd, msg, flags))
lt_libc_hidden(sendmsg)
#endif

#ifdef L_sendmmsg
#ifdef __ASSUME_SENDMMSG_SYSCALL
static ssize_t __NC(sendmmsg)(int sockfd, struct mmsghdr *msg, size_t vlen,
			      int flags)
{
# ifdef __NR_sendmmsg
	return (ssize_t)INLINE_SYSCALL(sendmmsg, 4, sockfd, msg, vlen, flags);
# elif __NR_socketcall
	unsigned long args[4];

	args[0] = sockfd;
	args[1] = (unsigned long) msg;
	args[2] = vlen;
	args[3] = flags;
	return (ssize_t)__socketcall(SYS_SENDMMSG, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, sendmmsg,
		    (int sockfd, struct mmsghdr *msg, size_t vlen, int flags),
		    (sockfd, msg, vlen, flags))
lt_libc_hidden(sendmmsg)
#endif
#endif

#ifdef L_sendto
ssize_t __NC(sendto)(int sockfd, const void *buffer, size_t len, int flags,
		     const struct sockaddr *to, socklen_t tolen)
{
# if defined __NR_sendto && defined _syscall6
	return (ssize_t)INLINE_SYSCALL(sendto, 6, sockfd, buffer, len, flags, to, tolen);
# elif defined(__NR_socketcall)
	unsigned long args[6];

	args[0] = sockfd;
	args[1] = (unsigned long) buffer;
	args[2] = len;
	args[3] = flags;
	args[4] = (unsigned long) to;
	args[5] = tolen;
	return (ssize_t)__socketcall(SYS_SENDTO, args);
# endif
}
CANCELLABLE_SYSCALL(ssize_t, sendto, (int sockfd, const void *buffer, size_t len,
				      int flags, const struct sockaddr *to, socklen_t tolen),
		    (sockfd, buffer, len, flags, to, tolen))
lt_libc_hidden(sendto)
#endif

#ifdef L_setsockopt
int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
# ifdef __NR_setsockopt
	return INLINE_SYSCALL(setsockopt, 5, fd, level, optname, optval, optlen);
# elif defined(__NR_socketcall)
	unsigned long args[5];

	args[0] = fd;
	args[1] = level;
	args[2] = optname;
	args[3] = (unsigned long) optval;
	args[4] = optlen;
	return __socketcall(SYS_SETSOCKOPT, args);
# endif
}
libc_hidden_def(setsockopt)
#endif

#ifdef L_shutdown
int shutdown(int sockfd, int how)
{
# ifdef __NR_shutdown
	return INLINE_SYSCALL(shutdown, 2, sockfd, how);
# elif defined(__NR_socketcall)
	unsigned long args[2];

	args[0] = sockfd;
	args[1] = how;
	return __socketcall(SYS_SHUTDOWN, args);
# endif
}
#endif

#ifdef L_socket
int socket(int family, int type, int protocol)
{
# ifdef __NR_socket
	return INLINE_SYSCALL(socket, 3, family, type, protocol);
# elif defined(__NR_socketcall)
	unsigned long args[3];

	args[0] = family;
	args[1] = type;
	args[2] = (unsigned long) protocol;
	return __socketcall(SYS_SOCKET, args);
# endif
}
libc_hidden_def(socket)
#endif

#ifdef L_socketpair
int socketpair(int family, int type, int protocol, int sockvec[2])
{
# ifdef __NR_socketpair
	return INLINE_SYSCALL(socketpair, 4, family, type, protocol, sockvec);
# elif defined(__NR_socketcall)
	unsigned long args[4];

	args[0] = family;
	args[1] = type;
	args[2] = protocol;
	args[3] = (unsigned long) sockvec;
	return __socketcall(SYS_SOCKETPAIR, args);
# endif
}
#endif
