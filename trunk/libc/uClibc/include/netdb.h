/* Copyright (C) 1996,1997,1998,1999,2000,2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* All data returned by the network data base library are supplied in
   host order and returned in network order (suitable for use in
   system calls).  */

#ifndef	_NETDB_H
#define	_NETDB_H	1

#include <features.h>

#include <netinet/in.h>
#include <stdint.h>
#ifdef __USE_MISC
/* This is necessary to make this include file properly replace the
   Sun version.  */
# include <rpc/netdb.h>
#endif

#ifdef __USE_GNU
# define __need_sigevent_t
# include <bits/siginfo.h>
# define __need_timespec
# include <time.h>
#endif

#include <bits/netdb.h>

/* Absolute file name for network data base files.  */
#define	_PATH_HEQUIV		"/etc/hosts.equiv"
#define	_PATH_HOSTS		"/etc/hosts"
#define	_PATH_NETWORKS		"/etc/networks"
#define	_PATH_NSSWITCH_CONF	"/etc/nsswitch.conf"
#define	_PATH_PROTOCOLS		"/etc/protocols"
#define	_PATH_SERVICES		"/etc/services"


__BEGIN_DECLS

/* Error status for non-reentrant lookup functions.  */
extern int h_errno;

/* Function to get address of global `h_errno' variable.  */
extern int *__h_errno_location (void) __THROW __attribute__ ((__const__));

#ifdef _LIBC
# define __set_h_errno(x) (h_errno = (x))
#endif

#if defined __UCLIBC_HAS_THREADS__
/* Use a macro to access always the thread specific `h_errno' variable.  */
# define h_errno (*__h_errno_location ())
#endif


/* Possible values left in `h_errno'.  */
#define	NETDB_INTERNAL	-1	/* See errno.  */
#define	NETDB_SUCCESS	0	/* No problem.  */
#define	HOST_NOT_FOUND	1	/* Authoritative Answer Host not found.  */
#define	TRY_AGAIN	2	/* Non-Authoritative Host not found,
				   or SERVERFAIL.  */
#define	NO_RECOVERY	3	/* Non recoverable errors, FORMERR, REFUSED,
				   NOTIMP.  */
#define	NO_DATA		4	/* Valid name, no data record of requested
				   type.  */
#define	NO_ADDRESS	NO_DATA	/* No address, look for MX record.  */

#ifdef __USE_XOPEN2K
/* Highest reserved Internet port number.  */
# define IPPORT_RESERVED	1024
#endif

#ifdef __USE_GNU
/* Scope delimiter for getaddrinfo(), getnameinfo().  */
# define SCOPE_DELIMITER	'%'
#endif

/* Print error indicated by `h_errno' variable on standard error.  STR
   if non-null is printed before the error string.  */
extern void herror (__const char *__str) __THROW;

/* Return string associated with error ERR_NUM.  */
extern __const char *hstrerror (int __err_num) __THROW;



/* Description of data base entry for a single host.  */
struct hostent
{
  char *h_name;			/* Official name of host.  */
  char **h_aliases;		/* Alias list.  */
  int h_addrtype;		/* Host address type.  */
  int h_length;			/* Length of address.  */
  char **h_addr_list;		/* List of addresses from name server.  */
#define	h_addr	h_addr_list[0]	/* Address, for backward compatibility.  */
};

/* Open host data base files and mark them as staying open even after
   a later search if STAY_OPEN is non-zero.  */
extern void sethostent (int __stay_open) __THROW;

/* Close host data base files and clear `stay open' flag.  */
extern void endhostent (void) __THROW;

/* Get next entry from host data base file.  Open data base if
   necessary.  */
extern struct hostent *gethostent (void) __THROW;

/* Return entry from host data base which address match ADDR with
   length LEN and type TYPE.  */
extern struct hostent *gethostbyaddr (__const void *__addr, __socklen_t __len,
				      int __type) __THROW;

/* Return entry from host data base for host with NAME.  */
extern struct hostent *gethostbyname (__const char *__name) __THROW;

#ifdef __USE_MISC
/* Return entry from host data base for host with NAME.  AF must be
   set to the address type which is `AF_INET' for IPv4 or `AF_INET6'
   for IPv6.  */
extern struct hostent *gethostbyname2 (__const char *__name, int __af) __THROW;

/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  The last
   argument is a pointer to a variable which gets the value which
   would be stored in the global variable `herrno' by the
   non-reentrant functions.  */
extern int gethostent_r (struct hostent *__restrict __result_buf,
			 char *__restrict __buf, size_t __buflen,
			 struct hostent **__restrict __result,
			 int *__restrict __h_errnop) __THROW;

extern int gethostbyaddr_r (__const void *__restrict __addr, __socklen_t __len,
			    int __type,
			    struct hostent *__restrict __result_buf,
			    char *__restrict __buf, size_t __buflen,
			    struct hostent **__restrict __result,
			    int *__restrict __h_errnop) __THROW;

extern int gethostbyname_r (__const char *__restrict __name,
			    struct hostent *__restrict __result_buf,
			    char *__restrict __buf, size_t __buflen,
			    struct hostent **__restrict __result,
			    int *__restrict __h_errnop) __THROW;

extern int gethostbyname2_r (__const char *__restrict __name, int __af,
			     struct hostent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct hostent **__restrict __result,
			     int *__restrict __h_errnop) __THROW;
#endif	/* misc */


/* Open network data base files and mark them as staying open even
   after a later search if STAY_OPEN is non-zero.  */
extern void setnetent (int __stay_open) __THROW;

/* Close network data base files and clear `stay open' flag.  */
extern void endnetent (void) __THROW;

/* Get next entry from network data base file.  Open data base if
   necessary.  */
extern struct netent *getnetent (void) __THROW;

/* Return entry from network data base which address match NET and
   type TYPE.  */
extern struct netent *getnetbyaddr (uint32_t __net, int __type)
     __THROW;

/* Return entry from network data base for network with NAME.  */
extern struct netent *getnetbyname (__const char *__name) __THROW;

#if 0
/* FIXME */
#ifdef	__USE_MISC
/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  The last
   argument is a pointer to a variable which gets the value which
   would be stored in the global variable `herrno' by the
   non-reentrant functions.  */
extern int getnetent_r (struct netent *__restrict __result_buf,
			char *__restrict __buf, size_t __buflen,
			struct netent **__restrict __result,
			int *__restrict __h_errnop) __THROW;

extern int getnetbyaddr_r (uint32_t __net, int __type,
			   struct netent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct netent **__restrict __result,
			   int *__restrict __h_errnop) __THROW;

extern int getnetbyname_r (__const char *__restrict __name,
			   struct netent *__restrict __result_buf,
			   char *__restrict __buf, size_t __buflen,
			   struct netent **__restrict __result,
			   int *__restrict __h_errnop) __THROW;
#endif	/* misc */
#endif


/* Description of data base entry for a single service.  */
struct servent
{
  char *s_name;			/* Official service name.  */
  char **s_aliases;		/* Alias list.  */
  int s_port;			/* Port number.  */
  char *s_proto;		/* Protocol to use.  */
};

/* Open service data base files and mark them as staying open even
   after a later search if STAY_OPEN is non-zero.  */
extern void setservent (int __stay_open) __THROW;

/* Close service data base files and clear `stay open' flag.  */
extern void endservent (void) __THROW;

/* Get next entry from service data base file.  Open data base if
   necessary.  */
extern struct servent *getservent (void) __THROW;

/* Return entry from network data base for network with NAME and
   protocol PROTO.  */
extern struct servent *getservbyname (__const char *__name,
				      __const char *__proto) __THROW;

/* Return entry from service data base which matches port PORT and
   protocol PROTO.  */
extern struct servent *getservbyport (int __port, __const char *__proto)
     __THROW;


#ifdef	__USE_MISC
/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  */
extern int getservent_r (struct servent *__restrict __result_buf,
			 char *__restrict __buf, size_t __buflen,
			 struct servent **__restrict __result) __THROW;

extern int getservbyname_r (__const char *__restrict __name,
			    __const char *__restrict __proto,
			    struct servent *__restrict __result_buf,
			    char *__restrict __buf, size_t __buflen,
			    struct servent **__restrict __result) __THROW;

extern int getservbyport_r (int __port, __const char *__restrict __proto,
			    struct servent *__restrict __result_buf,
			    char *__restrict __buf, size_t __buflen,
			    struct servent **__restrict __result) __THROW;
#endif	/* misc */


/* Description of data base entry for a single service.  */
struct protoent
{
  char *p_name;			/* Official protocol name.  */
  char **p_aliases;		/* Alias list.  */
  int p_proto;			/* Protocol number.  */
};

/* Open protocol data base files and mark them as staying open even
   after a later search if STAY_OPEN is non-zero.  */
extern void setprotoent (int __stay_open) __THROW;

/* Close protocol data base files and clear `stay open' flag.  */
extern void endprotoent (void) __THROW;

/* Get next entry from protocol data base file.  Open data base if
   necessary.  */
extern struct protoent *getprotoent (void) __THROW;

/* Return entry from protocol data base for network with NAME.  */
extern struct protoent *getprotobyname (__const char *__name) __THROW;

/* Return entry from protocol data base which number is PROTO.  */
extern struct protoent *getprotobynumber (int __proto) __THROW;


#ifdef	__USE_MISC
/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  */
extern int getprotoent_r (struct protoent *__restrict __result_buf,
			  char *__restrict __buf, size_t __buflen,
			  struct protoent **__restrict __result) __THROW;

extern int getprotobyname_r (__const char *__restrict __name,
			     struct protoent *__restrict __result_buf,
			     char *__restrict __buf, size_t __buflen,
			     struct protoent **__restrict __result) __THROW;

extern int getprotobynumber_r (int __proto,
			       struct protoent *__restrict __result_buf,
			       char *__restrict __buf, size_t __buflen,
			       struct protoent **__restrict __result) __THROW;
#endif	/* misc */


#ifdef __USE_BSD
/* Call `rshd' at port RPORT on remote machine *AHOST to execute CMD.
   The local user is LOCUSER, on the remote machine the command is
   executed as REMUSER.  In *FD2P the descriptor to the socket for the
   connection is returned.  The caller must have the right to use a
   reserved port.  When the function returns *AHOST contains the
   official host name.  */
extern int rcmd (char **__restrict __ahost, unsigned short int __rport,
		 __const char *__restrict __locuser,
		 __const char *__restrict __remuser,
		 __const char *__restrict __cmd, int *__restrict __fd2p)
     __THROW;

#if 0
/* FIXME */
/* This is the equivalent function where the protocol can be selected
   and which therefore can be used for IPv6.  */
extern int rcmd_af (char **__restrict __ahost, unsigned short int __rport,
		    __const char *__restrict __locuser,
		    __const char *__restrict __remuser,
		    __const char *__restrict __cmd, int *__restrict __fd2p,
		    sa_family_t __af) __THROW;
#endif

/* Call `rexecd' at port RPORT on remote machine *AHOST to execute
   CMD.  The process runs at the remote machine using the ID of user
   NAME whose cleartext password is PASSWD.  In *FD2P the descriptor
   to the socket for the connection is returned.  When the function
   returns *AHOST contains the official host name.  */
extern int rexec (char **__restrict __ahost, int __rport,
		  __const char *__restrict __name,
		  __const char *__restrict __pass,
		  __const char *__restrict __cmd, int *__restrict __fd2p)
     __THROW;

/* This is the equivalent function where the protocol can be selected
   and which therefore can be used for IPv6.  */
extern int rexec_af (char **__restrict __ahost, int __rport,
		     __const char *__restrict __name,
		     __const char *__restrict __pass,
		     __const char *__restrict __cmd, int *__restrict __fd2p,
		     sa_family_t __af) __THROW;

/* Check whether user REMUSER on system RHOST is allowed to login as LOCUSER.
   If SUSER is not zero the user tries to become superuser.  Return 0 if
   it is possible.  */
extern int ruserok (__const char *__rhost, int __suser,
		    __const char *__remuser, __const char *__locuser) __THROW;

#if 0
/* FIXME */
/* This is the equivalent function where the protocol can be selected
   and which therefore can be used for IPv6.  */
extern int ruserok_af (__const char *__rhost, int __suser,
		       __const char *__remuser, __const char *__locuser,
		       sa_family_t __af) __THROW;
#endif

/* Try to allocate reserved port, returning a descriptor for a socket opened
   at this port or -1 if unsuccessful.  The search for an available port
   will start at ALPORT and continues with lower numbers.  */
extern int rresvport (int *__alport) __THROW;

#if 0
/* FIXME */
/* This is the equivalent function where the protocol can be selected
   and which therefore can be used for IPv6.  */
extern int rresvport_af (int *__alport, sa_family_t __af) __THROW;
#endif
#endif


/* Extension from POSIX.1g.  */
#ifdef	__USE_POSIX
/* Structure to contain information about address of a service provider.  */
struct addrinfo
{
  int ai_flags;			/* Input flags.  */
  int ai_family;		/* Protocol family for socket.  */
  int ai_socktype;		/* Socket type.  */
  int ai_protocol;		/* Protocol for socket.  */
  socklen_t ai_addrlen;		/* Length of socket address.  */
  struct sockaddr *ai_addr;	/* Socket address for socket.  */
  char *ai_canonname;		/* Canonical name for service location.  */
  struct addrinfo *ai_next;	/* Pointer to next in list.  */
};

/* Possible values for `ai_flags' field in `addrinfo' structure.  */
# define AI_PASSIVE	0x0001	/* Socket address is intended for `bind'.  */
# define AI_CANONNAME	0x0002	/* Request for canonical name.  */
# define AI_NUMERICHOST	0x0004	/* Don't use name resolution.  */

/* Error values for `getaddrinfo' function.  */
# define EAI_BADFLAGS	  -1	/* Invalid value for `ai_flags' field.  */
# define EAI_NONAME	  -2	/* NAME or SERVICE is unknown.  */
# define EAI_AGAIN	  -3	/* Temporary failure in name resolution.  */
# define EAI_FAIL	  -4	/* Non-recoverable failure in name res.  */
# define EAI_NODATA	  -5	/* No address associated with NAME.  */
# define EAI_FAMILY	  -6	/* `ai_family' not supported.  */
# define EAI_SOCKTYPE	  -7	/* `ai_socktype' not supported.  */
# define EAI_SERVICE	  -8	/* SERVICE not supported for `ai_socktype'.  */
# define EAI_ADDRFAMILY	  -9	/* Address family for NAME not supported.  */
# define EAI_MEMORY	  -10	/* Memory allocation failure.  */
# define EAI_SYSTEM	  -11	/* System error returned in `errno'.  */
# ifdef __USE_GNU
#  define EAI_INPROGRESS  -100	/* Processing request in progress.  */
#  define EAI_CANCELED	  -101	/* Request canceled.  */
#  define EAI_NOTCANCELED -102	/* Request not canceled.  */
#  define EAI_ALLDONE	  -103	/* All requests done.  */
#  define EAI_INTR	  -104	/* Interrupted by a signal.  */
# endif

# define NI_MAXHOST      1025
# define NI_MAXSERV      32

# define NI_NUMERICHOST	1	/* Don't try to look up hostname.  */
# define NI_NUMERICSERV 2	/* Don't convert port number to name.  */
# define NI_NOFQDN	4	/* Only return nodename portion.  */
# define NI_NAMEREQD	8	/* Don't return numeric addresses.  */
# define NI_DGRAM	16	/* Look up UDP service rather than TCP.  */

/* Translate name of a service location and/or a service name to set of
   socket addresses.  */
extern int getaddrinfo (__const char *__restrict __name,
			__const char *__restrict __service,
			__const struct addrinfo *__restrict __req,
			struct addrinfo **__restrict __pai) __THROW;

/* Free `addrinfo' structure AI including associated storage.  */
extern void freeaddrinfo (struct addrinfo *__ai) __THROW;

/* Convert error return from getaddrinfo() to a string.  */
extern __const char *gai_strerror (int __ecode) __THROW;

/* Translate a socket address to a location and service name.  */
extern int getnameinfo (__const struct sockaddr *__restrict __sa,
			socklen_t __salen, char *__restrict __host,
			socklen_t __hostlen, char *__restrict __serv,
			socklen_t __servlen, unsigned int __flags) __THROW;

#endif	/* POSIX */

__END_DECLS

#endif	/* netdb.h */
