/* vi: set sw=4 ts=4: */
/* resolv.c: DNS Resolver
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *                     The Silver Hammer Group, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */
/*
 * Portions Copyright (c) 1985, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/*
 *  5-Oct-2000 W. Greathouse  wgreathouse@smva.com
 *   Fix memory leak and memory corruption.
 *   -- Every name resolution resulted in
 *      a new parse of resolv.conf and new
 *      copy of nameservers allocated by
 *      strdup.
 *   -- Every name resolution resulted in
 *      a new read of resolv.conf without
 *      resetting index from prior read...
 *      resulting in exceeding array bounds.
 *
 *   Limit nameservers read from resolv.conf.
 *   Add "search" domains from resolv.conf.
 *   Some systems will return a security
 *   signature along with query answer for
 *   dynamic DNS entries -- skip/ignore this answer.
 *   Include arpa/nameser.h for defines.
 *   General cleanup.
 *
 * 20-Jun-2001 Michal Moskal <malekith@pld.org.pl>
 *   partial IPv6 support (i.e. gethostbyname2() and resolve_address2()
 *   functions added), IPv6 nameservers are also supported.
 *
 * 6-Oct-2001 Jari Korva <jari.korva@iki.fi>
 *   more IPv6 support (IPv6 support for gethostbyaddr();
 *   address family parameter and improved IPv6 support for get_hosts_byname
 *   and read_etc_hosts; getnameinfo() port from glibc; defined
 *   defined ip6addr_any and in6addr_loopback)
 *
 * 2-Feb-2002 Erik Andersen <andersen@codepoet.org>
 *   Added gethostent(), sethostent(), and endhostent()
 *
 * 17-Aug-2002 Manuel Novoa III <mjn3@codepoet.org>
 *   Fixed __read_etc_hosts_r to return alias list, and modified buffer
 *   allocation accordingly.  See MAX_ALIASES and ALIAS_DIM below.
 *   This fixes the segfault in the Python 2.2.1 socket test.
 *
 * 04-Jan-2003 Jay Kulpinski <jskulpin@berkshire.rr.com>
 *   Fixed __decode_dotted to count the terminating null character
 *   in a host name.
 *
 * 02-Oct-2003 Tony J. White <tjw@tjw.org>
 *   Lifted dn_expand() and dependent ns_name_uncompress(), ns_name_unpack(),
 *   and ns_name_ntop() from glibc 2.3.2 for compatibility with ipsec-tools
 *   and openldap.
 *
 * 7-Sep-2004 Erik Andersen <andersen@codepoet.org>
 *   Added gethostent_r()
 *
 * 2008, 2009 Denys Vlasenko <vda.linux@googlemail.com>
 *   Cleanups, fixes, readability, more cleanups and more fixes.
 *
 * March 2010 Bernhard Reutner-Fischer
 *   Switch to common config parser
 */
/* Nota bene:
 * The whole resolver code has several (severe) problems:
 * - it doesn't even build without IPv4, i.e. !UCLIBC_HAS_IPV4 but only IPv6
 * - it is way too big
 *
 * Both points above are considered bugs, patches/reimplementations welcome.
 */
/* RFC 1035
...
Whenever an octet represents a numeric quantity, the left most bit
in the diagram is the high order or most significant bit.
That is, the bit labeled 0 is the most significant bit.
...

4.1.1. Header section format
      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   OPCODE  |AA|TC|RD|RA| 0  0  0|   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
ID      16 bit random identifier assigned by querying peer.
        Used to match query/response.
QR      message is a query (0), or a response (1).
OPCODE  0   standard query (QUERY)
        1   inverse query (IQUERY)
        2   server status request (STATUS)
AA      Authoritative Answer - this bit is valid in responses.
        Responding name server is an authority for the domain name
        in question section. Answer section may have multiple owner names
        because of aliases.  The AA bit corresponds to the name which matches
        the query name, or the first owner name in the answer section.
TC      TrunCation - this message was truncated.
RD      Recursion Desired - this bit may be set in a query and
        is copied into the response.  If RD is set, it directs
        the name server to pursue the query recursively.
        Recursive query support is optional.
RA      Recursion Available - this be is set or cleared in a
        response, and denotes whether recursive query support is
        available in the name server.
RCODE   Response code.
        0   No error condition
        1   Format error
        2   Server failure - server was unable to process the query
            due to a problem with the name server.
        3   Name Error - meaningful only for responses from
            an authoritative name server. The referenced domain name
            does not exist.
        4   Not Implemented.
        5   Refused.
QDCOUNT number of entries in the question section.
ANCOUNT number of records in the answer section.
NSCOUNT number of records in the authority records section.
ARCOUNT number of records in the additional records section.

4.1.2. Question section format

The section contains QDCOUNT (usually 1) entries, each of this format:
      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
QNAME   a domain name represented as a sequence of labels, where
        each label consists of a length octet followed by that
        number of octets. The domain name terminates with the
        zero length octet for the null label of the root. Note
        that this field may be an odd number of octets; no
        padding is used.
QTYPE   a two octet type of the query.
          1 a host address [REQ_A const]
          2 an authoritative name server
          3 a mail destination (Obsolete - use MX)
          4 a mail forwarder (Obsolete - use MX)
          5 the canonical name for an alias
          6 marks the start of a zone of authority
          7 a mailbox domain name (EXPERIMENTAL)
          8 a mail group member (EXPERIMENTAL)
          9 a mail rename domain name (EXPERIMENTAL)
         10 a null RR (EXPERIMENTAL)
         11 a well known service description
         12 a domain name pointer [REQ_PTR const]
         13 host information
         14 mailbox or mail list information
         15 mail exchange
         16 text strings
       0x1c IPv6?
        252 a request for a transfer of an entire zone
        253 a request for mailbox-related records (MB, MG or MR)
        254 a request for mail agent RRs (Obsolete - see MX)
        255 a request for all records
QCLASS  a two octet code that specifies the class of the query.
          1 the Internet
        (others are historic only)
        255 any class

4.1.3. Resource record format

The answer, authority, and additional sections all share the same format:
a variable number of resource records, where the number of records
is specified in the corresponding count field in the header.
Each resource record has this format:
      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    /                                               /
    /                      NAME                     /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
NAME    a domain name to which this resource record pertains.
TYPE    two octets containing one of the RR type codes.  This
        field specifies the meaning of the data in the RDATA field.
CLASS   two octets which specify the class of the data in the RDATA field.
TTL     a 32 bit unsigned integer that specifies the time interval
        (in seconds) that the record may be cached.
RDLENGTH a 16 bit integer, length in octets of the RDATA field.
RDATA   a variable length string of octets that describes the resource.
        The format of this information varies according to the TYPE
        and CLASS of the resource record.
        If the TYPE is A and the CLASS is IN, it's a 4 octet IP address.

4.1.4. Message compression

In order to reduce the size of messages, domain names can be compressed.
An entire domain name or a list of labels at the end of a domain name
is replaced with a pointer to a prior occurance of the same name.

The pointer takes the form of a two octet sequence:
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    | 1  1|                OFFSET                   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
The first two bits are ones.  This allows a pointer to be distinguished
from a label, since the label must begin with two zero bits because
labels are restricted to 63 octets or less.  The OFFSET field specifies
an offset from the start of the message (i.e., the first octet
of the ID field in the domain header).
A zero offset specifies the first byte of the ID field, etc.
Domain name in a message can be represented as either:
   - a sequence of labels ending in a zero octet
   - a pointer
   - a sequence of labels ending with a pointer
 */

#define __FORCE_GLIBC
#include <features.h>
#include <string.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <signal.h>
#include <malloc.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <resolv.h>
#include <netdb.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <arpa/nameser.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <bits/uClibc_mutex.h>
#include "internal/parse_config.h"

/* poll() is not supported in kernel <= 2.0, therefore if __NR_poll is
 * not available, we assume an old Linux kernel is in use and we will
 * use select() instead. */
#include <sys/syscall.h>
#ifndef __NR_poll
# define USE_SELECT
#endif

#if defined __UCLIBC_HAS_IPV4__ && defined __UCLIBC_HAS_IPV6__
#define IF_HAS_BOTH(...) __VA_ARGS__
#else
#define IF_HAS_BOTH(...)
#endif


#define MAX_RECURSE    5
#define MAXALIASES  (4)
#define BUFSZ       (80) /* one line */

#define NS_TYPE_ELT					0x40 /*%< EDNS0 extended label type */
#define DNS_LABELTYPE_BITSTRING		0x41

#undef DEBUG
/* #define DEBUG */

#ifdef DEBUG
#define DPRINTF(X,args...) fprintf(stderr, X, ##args)
#else
#define DPRINTF(X,args...)
#endif

/* Make sure the incoming char * buffer is aligned enough to handle our random
 * structures.  This define is the same as we use for malloc alignment (which
 * has same requirements).  The offset is the number of bytes we need to adjust
 * in order to attain desired alignment.
 */
#define ALIGN_ATTR __alignof__(double __attribute_aligned__ (sizeof(size_t)))
#define ALIGN_BUFFER_OFFSET(buf) ((ALIGN_ATTR - ((size_t)buf % ALIGN_ATTR)) % ALIGN_ATTR)


/* Structs */
struct resolv_header {
	int id;
	int qr, opcode, aa, tc, rd, ra, rcode;
	int qdcount;
	int ancount;
	int nscount;
	int arcount;
};

struct resolv_question {
	char *dotted;
	int qtype;
	int qclass;
};

struct resolv_answer {
	char *dotted;
	int atype;
	int aclass;
	int ttl;
	int rdlength;
	const unsigned char *rdata;
	int rdoffset;
	char* buf;
	size_t buflen;
	size_t add_count;
};

enum etc_hosts_action {
	GET_HOSTS_BYNAME = 0,
	GETHOSTENT,
	GET_HOSTS_BYADDR,
};

typedef union sockaddr46_t {
	struct sockaddr sa;
#ifdef __UCLIBC_HAS_IPV4__
	struct sockaddr_in sa4;
#endif
#ifdef __UCLIBC_HAS_IPV6__
	struct sockaddr_in6 sa6;
#endif
} sockaddr46_t;


__UCLIBC_MUTEX_EXTERN(__resolv_lock);

/* Protected by __resolv_lock */
extern void (*__res_sync)(void) attribute_hidden;
/*extern uint32_t __resolv_opts attribute_hidden; */
extern uint8_t __resolv_timeout attribute_hidden;
extern uint8_t __resolv_attempts attribute_hidden;
extern unsigned __nameservers attribute_hidden;
extern unsigned __searchdomains attribute_hidden;
extern sockaddr46_t *__nameserver attribute_hidden;
extern char **__searchdomain attribute_hidden;
#ifdef __UCLIBC_HAS_IPV4__
extern const struct sockaddr_in __local_nameserver attribute_hidden;
 #define GETXX_BUFSIZE 	(sizeof(struct in_addr) + sizeof(struct in_addr *) * 2 + \
			/*sizeof(char *)*ALIAS_DIM */+ 456/*namebuffer*/ + 32/* margin */)
#else
extern const struct sockaddr_in6 __local_nameserver attribute_hidden;
 #define GETXX_BUFSIZE 	(sizeof(struct in6_addr) + sizeof(struct in6_addr *) * 2 + \
			/*sizeof(char *)*ALIAS_DIM */+ 456/*namebuffer*/ + 32/* margin */)
#endif
extern char * static_getXXaddrbuf attribute_hidden;
/* Arbitrary */
#define MAXLEN_searchdomain 128


/* prototypes for internal functions */
extern void endhostent_unlocked(void) attribute_hidden;
extern int __get_hosts_byname_r(const char *name,
		int type,
		struct hostent *result_buf,
		char *buf,
		size_t buflen,
		struct hostent **result,
		int *h_errnop) attribute_hidden;
extern int __get_hosts_byaddr_r(const char *addr,
		int len,
		int type,
		struct hostent *result_buf,
		char *buf,
		size_t buflen,
		struct hostent **result,
		int *h_errnop) attribute_hidden;
extern parser_t *__open_etc_hosts(void) attribute_hidden;
extern int __read_etc_hosts_r(parser_t *parser,
		const char *name,
		int type,
		enum etc_hosts_action action,
		struct hostent *result_buf,
		char *buf,
		size_t buflen,
		struct hostent **result,
		int *h_errnop) attribute_hidden;
extern int __dns_lookup(const char *name,
		int type,
		unsigned char **outpacket,
		struct resolv_answer *a) attribute_hidden;
extern int __encode_dotted(const char *dotted,
		unsigned char *dest,
		int maxlen) attribute_hidden;
extern int __decode_dotted(const unsigned char *packet,
		int offset,
		int packet_len,
		char *dest,
		int dest_len) attribute_hidden;
extern int __encode_header(struct resolv_header *h,
		unsigned char *dest,
		int maxlen) attribute_hidden;
extern void __decode_header(unsigned char *data,
		struct resolv_header *h) attribute_hidden;
extern int __encode_question(const struct resolv_question *q,
		unsigned char *dest,
		int maxlen) attribute_hidden;
extern int __encode_answer(struct resolv_answer *a,
		unsigned char *dest,
		int maxlen) attribute_hidden;
extern void __open_nameservers(void) attribute_hidden;
extern void __close_nameservers(void) attribute_hidden;
extern void __init_getXXaddrbuf(void) attribute_hidden;

/*
 * Theory of operation.
 *
 * gethostbyname, getaddrinfo and friends end up here, and they sometimes
 * need to talk to DNS servers. In order to do this, we need to read /etc/resolv.conf
 * and determine servers' addresses and the like. resolv.conf format:
 *
 * nameserver <IP[v6]>
 *		Address of DNS server. Cumulative.
 *		If not specified, assumed to be on localhost.
 * search <domain1>[ <domain2>]...
 *		Append these domains to unqualified names.
 *		See ndots:n option.
 *		$LOCALDOMAIN (space-separated list) overrides this.
 * domain <domain>
 *		Effectively same as "search" with one domain.
 *		If no "domain" line is present, the domain is determined
 *		from the local host name returned by gethostname();
 *		the domain part is taken to be everything after the first dot.
 *		If there are no dots, there will be no "domain".
 *		The domain and search keywords are mutually exclusive.
 *		If more than one instance of these keywords is present,
 *		the last instance wins.
 * sortlist 130.155.160.0[/255.255.240.0] 130.155.0.0
 *		Allows addresses returned by gethostbyname to be sorted.
 *		Not supported.
 * options option[ option]...
 *		(so far we support timeout:n and attempts:n)
 *		$RES_OPTIONS (space-separated list) is to be added to "options"
 *  debug	sets RES_DEBUG in _res.options
 *  ndots:n	how many dots there should be so that name will be tried
 *		first as an absolute name before any search list elements
 *		are appended to it. Default 1
 *  timeout:n   how long to wait for response. Default 5
 *		(sun seems to have retrans:n synonym)
 *  attempts:n	number of rounds to do before giving up and returning
 *		an error. Default 2
 *		(sun seems to have retry:n synonym)
 *  rotate	sets RES_ROTATE in _res.options, round robin
 *		selection of nameservers. Otherwise try
 *		the first listed server first every time
 *  no-check-names
 *		sets RES_NOCHECKNAME in _res.options, which disables
 *		checking of incoming host names for invalid characters
 *		such as underscore (_), non-ASCII, or control characters
 *  inet6	sets RES_USE_INET6 in _res.options. Try a AAAA query
 *		before an A query inside the gethostbyname(), and map
 *		IPv4 responses in IPv6 "tunnelled form" if no AAAA records
 *		are found but an A record set exists
 *  no_tld_query (FreeBSDism?)
 *		do not attempt to resolve names without dots
 *
 * We will read and analyze /etc/resolv.conf as needed before
 * we do a DNS request. This happens in __dns_lookup.
 * It is reread if its mtime is changed.
 *
 * BSD has res_init routine which is used to initialize resolver state
 * which is held in global structure _res.
 * Generally, programs call res_init, then fiddle with _res.XXX
 * (_res.options and _res.nscount, _res.nsaddr_list[N]
 * are popular targets of fiddling) and expect subsequent calls
 * to gethostbyname, getaddrinfo, etc to use modified information.
 *
 * However, historical _res structure is quite awkward.
 * Using it for storing /etc/resolv.conf info is not desirable,
 * and __dns_lookup does not use it.
 *
 * We would like to avoid using it unless absolutely necessary.
 * If user doesn't use res_init, we should arrange it so that
 * _res structure doesn't even *get linked in* into user's application
 * (imagine static uclibc build here).
 *
 * The solution is a __res_sync function pointer, which is normally NULL.
 * But if res_init is called, it gets set and any subsequent gethostbyname
 * et al "syncronizes" our internal structures with potentially
 * modified _res.XXX stuff by calling __res_sync.
 * The trick here is that if res_init is not used and not linked in,
 * gethostbyname itself won't reference _res and _res won't be linked in
 * either. Other possible methods like
 * if (__res_sync_just_an_int_flag)
 *	__sync_me_with_res()
 * would pull in __sync_me_with_res, which pulls in _res. Bad.
 */


#ifdef L_encodeh

int attribute_hidden __encode_header(struct resolv_header *h, unsigned char *dest, int maxlen)
{
	if (maxlen < HFIXEDSZ)
		return -1;

	dest[0] = (h->id & 0xff00) >> 8;
	dest[1] = (h->id & 0x00ff) >> 0;
	dest[2] = (h->qr ? 0x80 : 0) |
		((h->opcode & 0x0f) << 3) |
		(h->aa ? 0x04 : 0) |
		(h->tc ? 0x02 : 0) |
		(h->rd ? 0x01 : 0);
	dest[3] = (h->ra ? 0x80 : 0) | (h->rcode & 0x0f);
	dest[4] = (h->qdcount & 0xff00) >> 8;
	dest[5] = (h->qdcount & 0x00ff) >> 0;
	dest[6] = (h->ancount & 0xff00) >> 8;
	dest[7] = (h->ancount & 0x00ff) >> 0;
	dest[8] = (h->nscount & 0xff00) >> 8;
	dest[9] = (h->nscount & 0x00ff) >> 0;
	dest[10] = (h->arcount & 0xff00) >> 8;
	dest[11] = (h->arcount & 0x00ff) >> 0;

	return HFIXEDSZ;
}
#endif /* L_encodeh */


#ifdef L_decodeh

void attribute_hidden __decode_header(unsigned char *data,
		struct resolv_header *h)
{
	h->id = (data[0] << 8) | data[1];
	h->qr = (data[2] & 0x80) ? 1 : 0;
	h->opcode = (data[2] >> 3) & 0x0f;
	h->aa = (data[2] & 0x04) ? 1 : 0;
	h->tc = (data[2] & 0x02) ? 1 : 0;
	h->rd = (data[2] & 0x01) ? 1 : 0;
	h->ra = (data[3] & 0x80) ? 1 : 0;
	h->rcode = data[3] & 0x0f;
	h->qdcount = (data[4] << 8) | data[5];
	h->ancount = (data[6] << 8) | data[7];
	h->nscount = (data[8] << 8) | data[9];
	h->arcount = (data[10] << 8) | data[11];
}
#endif /* L_decodeh */


#ifdef L_encoded

/* Encode a dotted string into nameserver transport-level encoding.
   This routine is fairly dumb, and doesn't attempt to compress
   the data */
int attribute_hidden __encode_dotted(const char *dotted, unsigned char *dest, int maxlen)
{
	unsigned used = 0;

	while (dotted && *dotted) {
		char *c = strchr(dotted, '.');
		int l = c ? c - dotted : strlen(dotted);

		/* two consecutive dots are not valid */
		if (l == 0)
			return -1;

		if (l >= (maxlen - used - 1))
			return -1;

		dest[used++] = l;
		memcpy(dest + used, dotted, l);
		used += l;

		if (!c)
			break;
		dotted = c + 1;
	}

	if (maxlen < 1)
		return -1;

	dest[used++] = 0;

	return used;
}
#endif /* L_encoded */


#ifdef L_decoded

/* Decode a dotted string from nameserver transport-level encoding.
   This routine understands compressed data. */
int attribute_hidden __decode_dotted(const unsigned char *packet,
		int offset,
		int packet_len,
		char *dest,
		int dest_len)
{
	unsigned b;
	bool measure = 1;
	unsigned total = 0;
	unsigned used = 0;

	if (!packet)
		return -1;

	while (1) {
		if (offset >= packet_len)
			return -1;
		b = packet[offset++];
		if (b == 0)
			break;

		if (measure)
			total++;

		if ((b & 0xc0) == 0xc0) {
			if (offset >= packet_len)
				return -1;
			if (measure)
				total++;
			/* compressed item, redirect */
			offset = ((b & 0x3f) << 8) | packet[offset];
			measure = 0;
			continue;
		}

		if (used + b + 1 >= dest_len)
			return -1;
		if (offset + b >= packet_len)
			return -1;
		memcpy(dest + used, packet + offset, b);
		offset += b;
		used += b;

		if (measure)
			total += b;

		if (packet[offset] != 0)
			dest[used++] = '.';
		else
			dest[used++] = '\0';
	}

	/* The null byte must be counted too */
	if (measure)
		total++;

	DPRINTF("Total decode len = %d\n", total);

	return total;
}
#endif /* L_decoded */


#ifdef L_encodeq

int attribute_hidden __encode_question(const struct resolv_question *q,
		unsigned char *dest,
		int maxlen)
{
	int i;

	i = __encode_dotted(q->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < 4)
		return -1;

	dest[0] = (q->qtype & 0xff00) >> 8;
	dest[1] = (q->qtype & 0x00ff) >> 0;
	dest[2] = (q->qclass & 0xff00) >> 8;
	dest[3] = (q->qclass & 0x00ff) >> 0;

	return i + 4;
}
#endif /* L_encodeq */


#ifdef L_encodea

int attribute_hidden __encode_answer(struct resolv_answer *a, unsigned char *dest, int maxlen)
{
	int i;

	i = __encode_dotted(a->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < (RRFIXEDSZ + a->rdlength))
		return -1;

	*dest++ = (a->atype & 0xff00) >> 8;
	*dest++ = (a->atype & 0x00ff) >> 0;
	*dest++ = (a->aclass & 0xff00) >> 8;
	*dest++ = (a->aclass & 0x00ff) >> 0;
	*dest++ = (a->ttl & 0xff000000) >> 24;
	*dest++ = (a->ttl & 0x00ff0000) >> 16;
	*dest++ = (a->ttl & 0x0000ff00) >> 8;
	*dest++ = (a->ttl & 0x000000ff) >> 0;
	*dest++ = (a->rdlength & 0xff00) >> 8;
	*dest++ = (a->rdlength & 0x00ff) >> 0;
	memcpy(dest, a->rdata, a->rdlength);

	return i + RRFIXEDSZ + a->rdlength;
}
#endif /* L_encodea */


#ifdef CURRENTLY_UNUSED
#ifdef L_encodep

int __encode_packet(struct resolv_header *h,
		struct resolv_question **q,
		struct resolv_answer **an,
		struct resolv_answer **ns,
		struct resolv_answer **ar,
		unsigned char *dest, int maxlen) attribute_hidden;
int __encode_packet(struct resolv_header *h,
		struct resolv_question **q,
		struct resolv_answer **an,
		struct resolv_answer **ns,
		struct resolv_answer **ar,
		unsigned char *dest, int maxlen)
{
	int i, total = 0;
	unsigned j;

	i = __encode_header(h, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;
	total += i;

	for (j = 0; j < h->qdcount; j++) {
		i = __encode_question(q[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	for (j = 0; j < h->ancount; j++) {
		i = __encode_answer(an[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->nscount; j++) {
		i = __encode_answer(ns[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->arcount; j++) {
		i = __encode_answer(ar[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	return total;
}
#endif /* L_encodep */


#ifdef L_decodep

int __decode_packet(unsigned char *data, struct resolv_header *h) attribute_hidden;
int __decode_packet(unsigned char *data, struct resolv_header *h)
{
	__decode_header(data, h);
	return HFIXEDSZ;
}
#endif /* L_decodep */


#ifdef L_formquery

int __form_query(int id,
		const char *name,
		int type,
		unsigned char *packet,
		int maxlen);
int __form_query(int id,
		const char *name,
		int type,
		unsigned char *packet,
		int maxlen)
{
	struct resolv_header h;
	struct resolv_question q;
	int i, j;

	memset(&h, 0, sizeof(h));
	h.id = id;
	h.qdcount = 1;

	q.dotted = (char *) name;
	q.qtype = type;
	q.qclass = C_IN; /* CLASS_IN */

	i = __encode_header(&h, packet, maxlen);
	if (i < 0)
		return i;

	j = __encode_question(&q, packet + i, maxlen - i);
	if (j < 0)
		return j;

	return i + j;
}
#endif /* L_formquery */
#endif /* CURRENTLY_UNUSED */


#ifdef L_opennameservers

# if __BYTE_ORDER == __LITTLE_ENDIAN
#define NAMESERVER_PORT_N (__bswap_constant_16(NAMESERVER_PORT))
#else
#define NAMESERVER_PORT_N NAMESERVER_PORT
#endif

__UCLIBC_MUTEX_INIT(__resolv_lock, PTHREAD_MUTEX_INITIALIZER);

/* Protected by __resolv_lock */
void (*__res_sync)(void);
/*uint32_t __resolv_opts; */
uint8_t __resolv_timeout = RES_TIMEOUT;
uint8_t __resolv_attempts = RES_DFLRETRY;
unsigned __nameservers;
unsigned __searchdomains;
sockaddr46_t *__nameserver;
char **__searchdomain;
#ifdef __UCLIBC_HAS_IPV4__
const struct sockaddr_in __local_nameserver = {
	.sin_family = AF_INET,
	.sin_port = NAMESERVER_PORT_N,
};
#else
const struct sockaddr_in6 __local_nameserver = {
	.sin6_family = AF_INET6,
	.sin6_port = NAMESERVER_PORT_N,
};
#endif

/* Helpers. Both stop on EOL, if it's '\n', it is converted to NUL first */
extern char *skip_nospace(char *p) attribute_hidden;
char attribute_hidden *skip_nospace(char *p)
{
	while (*p != '\0' && !isspace(*p)) {
		if (*p == '\n') {
			*p = '\0';
			break;
		}
		p++;
	}
	return p;
}
extern char *skip_and_NUL_space(char *p) attribute_hidden;
char attribute_hidden *skip_and_NUL_space(char *p)
{
	/* NB: '\n' is not isspace! */
	while (1) {
		char c = *p;
		if (c == '\0' || !isspace(c))
			break;
		*p = '\0';
		if (c == '\n' || c == '#')
			break;
		p++;
	}
	return p;
}

/* Must be called under __resolv_lock. */
void attribute_hidden __open_nameservers(void)
{
	static uint32_t resolv_conf_mtime;

	char szBuffer[MAXLEN_searchdomain];
	FILE *fp;
	int i;
	sockaddr46_t sa;

	if (!__res_sync) {
		/* Reread /etc/resolv.conf if it was modified.  */
		struct stat sb;
		if (stat("/etc/resolv.conf", &sb) != 0)
			sb.st_mtime = 0;
		if (resolv_conf_mtime != (uint32_t)sb.st_mtime) {
			resolv_conf_mtime = sb.st_mtime;
			__close_nameservers(); /* force config reread */
		}
	}

	if (__nameservers)
		goto sync;

	__resolv_timeout = RES_TIMEOUT;
	__resolv_attempts = RES_DFLRETRY;

	fp = fopen("/etc/resolv.conf", "r");
#ifdef FALLBACK_TO_CONFIG_RESOLVCONF
	if (!fp) {
		/* If we do not have a pre-populated /etc/resolv.conf then
		   try to use the one from /etc/config which exists on numerous
		   systems ranging from some uClinux to IRIX installations and
		   may be the only /etc dir that was mounted rw.  */
		fp = fopen("/etc/config/resolv.conf", "r");
	}
#endif

	if (fp) {
		while (fgets(szBuffer, sizeof(szBuffer), fp) != NULL) {
			void *ptr;
			char *keyword, *p;

			keyword = p = skip_and_NUL_space(szBuffer);
			/* skip keyword */
			p = skip_nospace(p);
			/* find next word */
			p = skip_and_NUL_space(p);

			if (strcmp(keyword, "nameserver") == 0) {
				/* terminate IP addr */
				*skip_nospace(p) = '\0';
				memset(&sa, 0, sizeof(sa));
				if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV6__
				else if (inet_pton(AF_INET6, p, &sa.sa6.sin6_addr) > 0) {
					sa.sa6.sin6_family = AF_INET6;
					sa.sa6.sin6_port = htons(NAMESERVER_PORT);
				}
#endif
#ifdef __UCLIBC_HAS_IPV4__
				else if (inet_pton(AF_INET, p, &sa.sa4.sin_addr) > 0) {
					sa.sa4.sin_family = AF_INET;
					sa.sa4.sin_port = htons(NAMESERVER_PORT);
				}
#endif
				else
					continue; /* garbage on this line */
				ptr = realloc(__nameserver, (__nameservers + 1) * sizeof(__nameserver[0]));
				if (!ptr)
					continue;
				__nameserver = ptr;
				__nameserver[__nameservers++] = sa; /* struct copy */
				continue;
			}
			if (strcmp(keyword, "domain") == 0 || strcmp(keyword, "search") == 0) {
				char *p1;

				/* free old domains ("last 'domain' or 'search' wins" rule) */
				while (__searchdomains)
					free(__searchdomain[--__searchdomains]);
				/*free(__searchdomain);*/
				/*__searchdomain = NULL; - not necessary */
 next_word:
				/* terminate current word */
				p1 = skip_nospace(p);
				/* find next word (maybe) */
				p1 = skip_and_NUL_space(p1);
				/* add it */
				ptr = realloc(__searchdomain, (__searchdomains + 1) * sizeof(__searchdomain[0]));
				if (!ptr)
					continue;
				__searchdomain = ptr;
				/* NB: strlen(p) <= MAXLEN_searchdomain) because szBuffer[] is smaller */
				ptr = strdup(p);
				if (!ptr)
					continue;
				DPRINTF("adding search %s\n", (char*)ptr);
				__searchdomain[__searchdomains++] = (char*)ptr;
				p = p1;
				if (*p)
					goto next_word;
				continue;
			}
			/* if (strcmp(keyword, "sortlist") == 0)... */
			if (strcmp(keyword, "options") == 0) {
				char *p1;
				uint8_t *what;

				if (p == NULL || (p1 = strchr(p, ':')) == NULL)
					continue;
				*p1++ = '\0';
				if (strcmp(p, "timeout") == 0)
					what = &__resolv_timeout;
				else if (strcmp(p, "attempts") == 0)
					what = &__resolv_attempts;
				else
					continue;
				*what = atoi(p1);
				DPRINTF("option %s:%d\n", p, *what);
			}
		}
		fclose(fp);
	}
	if (__nameservers == 0) {
		/* Have to handle malloc failure! What a mess...
		 * And it's not only here, we need to be careful
		 * to never write into __nameserver[0] if it points
		 * to constant __local_nameserver, or free it. */
		__nameserver = malloc(sizeof(__nameserver[0]));
		if (__nameserver)
			memcpy(__nameserver, &__local_nameserver, sizeof(__local_nameserver));
		else
			__nameserver = (void*) &__local_nameserver;
		__nameservers++;
	}
	if (__searchdomains == 0) {
		char buf[256];
		char *p;
		i = gethostname(buf, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';
		if (i == 0 && (p = strchr(buf, '.')) != NULL && p[1]) {
			p = strdup(p + 1);
			if (!p)
				goto err;
			__searchdomain = malloc(sizeof(__searchdomain[0]));
			if (!__searchdomain) {
				free(p);
				goto err;
			}
			__searchdomain[0] = p;
			__searchdomains++;
 err: ;
		}
	}
	DPRINTF("nameservers = %d\n", __nameservers);

 sync:
	if (__res_sync)
		__res_sync();
}
#endif /* L_opennameservers */


#ifdef L_closenameservers

/* Must be called under __resolv_lock. */
void attribute_hidden __close_nameservers(void)
{
	if (__nameserver != (void*) &__local_nameserver)
		free(__nameserver);
	__nameserver = NULL;
	__nameservers = 0;
	while (__searchdomains)
		free(__searchdomain[--__searchdomains]);
	free(__searchdomain);
	__searchdomain = NULL;
	/*__searchdomains = 0; - already is */
}
#endif /* L_closenameservers */


#ifdef L_dnslookup

/* Helpers */
static int __length_question(const unsigned char *data, int maxlen)
{
	const unsigned char *start;
	unsigned b;

	if (!data)
		return -1;

	start = data;
	while (1) {
		if (maxlen <= 0)
			return -1;
		b = *data++;
		if (b == 0)
			break;
		if ((b & 0xc0) == 0xc0) {
			/* It's a "compressed" name. */
			data++; /* skip lsb of redirected offset */
			maxlen -= 2;
			break;
		}
		data += b;
		maxlen -= (b + 1); /* account for data++ above */
	}
	/* Up to here we were skipping encoded name */

	/* Account for QTYPE and QCLASS fields */
	if (maxlen < 4)
		return -1;
	return data - start + 2 + 2;
}

static int __decode_answer(const unsigned char *message, /* packet */
		int offset,
		int len, /* total packet len */
		struct resolv_answer *a)
{
	char temp[256];
	int i;

	DPRINTF("decode_answer(start): off %d, len %d\n", offset, len);
	i = __decode_dotted(message, offset, len, temp, sizeof(temp));
	if (i < 0)
		return i;

	message += offset + i;
	len -= i + RRFIXEDSZ + offset;
	if (len < 0) {
		DPRINTF("decode_answer: off %d, len %d, i %d\n", offset, len, i);
		return len;
	}

/* TODO: what if strdup fails? */
	a->dotted = strdup(temp);
	a->atype = (message[0] << 8) | message[1];
	message += 2;
	a->aclass = (message[0] << 8) | message[1];
	message += 2;
	a->ttl = (message[0] << 24) |
		(message[1] << 16) | (message[2] << 8) | (message[3] << 0);
	message += 4;
	a->rdlength = (message[0] << 8) | message[1];
	message += 2;
	a->rdata = message;
	a->rdoffset = offset + i + RRFIXEDSZ;

	DPRINTF("i=%d,rdlength=%d\n", i, a->rdlength);

	if (len < a->rdlength)
		return -1;
	return i + RRFIXEDSZ + a->rdlength;
}

/* Commonplace for buffer for non-reentrant gethostXX functions */
char *static_getXXaddrbuf = NULL;

void attribute_hidden __init_getXXaddrbuf(void)
{
	if (!static_getXXaddrbuf)
		static_getXXaddrbuf = (char *)__uc_malloc(GETXX_BUFSIZE);
}

/* On entry:
 *  a.buf(len) = auxiliary buffer for IP addresses after first one
 *  a.add_count = how many additional addresses are there already
 *  outpacket = where to save ptr to raw packet? can be NULL
 * On exit:
 *  ret < 0: error, all other data is not valid
 *  ret >= 0: length of reply packet
 *  a.add_count & a.buf: updated
 *  a.rdlength: length of addresses (4 bytes for IPv4)
 *  *outpacket: updated (packet is malloced, you need to free it)
 *  a.rdata: points into *outpacket to 1st IP addr
 *      NB: don't pass outpacket == NULL if you need to use a.rdata!
 *  a.atype: type of query?
 *  a.dotted: which name we _actually_ used. May contain search domains
 *      appended. (why the filed is called "dotted" I have no idea)
 *      This is a malloced string. May be NULL because strdup failed.
 */
int attribute_hidden __dns_lookup(const char *name,
		int type,
		unsigned char **outpacket,
		struct resolv_answer *a)
{
	/* Protected by __resolv_lock: */
	static int last_ns_num = 0;
	static uint16_t last_id = 1;

	int i, j, fd, rc;
	int packet_len;
	int name_len;
#ifdef USE_SELECT
	struct timeval tv;
	fd_set fds;
#else
	struct pollfd fds;
#endif
	struct resolv_header h;
	struct resolv_question q;
	struct resolv_answer ma;
	bool first_answer = 1;
	int retries_left;
	unsigned char *packet = malloc(PACKETSZ);
	char *lookup;
	int variant = -1;  /* search domain to append, -1: none */
	int local_ns_num = -1; /* Nth server to use */
	int local_id = local_id; /* for compiler */
	int sdomains;
	bool ends_with_dot;
	sockaddr46_t sa;

	fd = -1;
	lookup = NULL;
	name_len = strlen(name);
	if ((unsigned)name_len >= MAXDNAME - MAXLEN_searchdomain - 2)
		goto fail; /* paranoia */
	lookup = malloc(name_len + 1/*for '.'*/ + MAXLEN_searchdomain + 1);
	if (!packet || !lookup || !name[0])
		goto fail;
	ends_with_dot = (name[name_len - 1] == '.');
	/* no strcpy! paranoia, user might change name[] under us */
	memcpy(lookup, name, name_len);

	DPRINTF("Looking up type %d answer for '%s'\n", type, name);
	retries_left = 0; /* for compiler */
	do {
		int pos;
		unsigned reply_timeout;

		if (fd != -1) {
			close(fd);
			fd = -1;
		}

		/* Mess with globals while under lock */
		/* NB: even data *pointed to* by globals may vanish
		 * outside the locks. We should assume any and all
		 * globals can completely change between locked
		 * code regions. OTOH, this is rare, so we don't need
		 * to handle it "nicely" (do not skip servers,
		 * search domains, etc), we only need to ensure
		 * we do not SEGV, use freed+overwritten data
		 * or do other Really Bad Things. */
		__UCLIBC_MUTEX_LOCK(__resolv_lock);
		__open_nameservers();
		sdomains = __searchdomains;
		lookup[name_len] = '\0';
		if ((unsigned)variant < sdomains) {
			/* lookup is name_len + 1 + MAXLEN_searchdomain + 1 long */
			/* __searchdomain[] is not bigger than MAXLEN_searchdomain */
			lookup[name_len] = '.';
			strcpy(&lookup[name_len + 1], __searchdomain[variant]);
		}
		/* first time? pick starting server etc */
		if (local_ns_num < 0) {
			local_id = last_id;
/*TODO: implement /etc/resolv.conf's "options rotate"
 (a.k.a. RES_ROTATE bit in _res.options)
			local_ns_num = 0;
			if (_res.options & RES_ROTATE) */
				local_ns_num = last_ns_num;
			retries_left = __nameservers * __resolv_attempts;
		}
		retries_left--;
		if (local_ns_num >= __nameservers)
			local_ns_num = 0;
		local_id++;
		local_id &= 0xffff;
		/* write new values back while still under lock */
		last_id = local_id;
		last_ns_num = local_ns_num;
		/* struct copy */
		/* can't just take a pointer, __nameserver[x]
		 * is not safe to use outside of locks */
		sa = __nameserver[local_ns_num];
		__UCLIBC_MUTEX_UNLOCK(__resolv_lock);

		memset(packet, 0, PACKETSZ);
		memset(&h, 0, sizeof(h));

		/* encode header */
		h.id = local_id;
		h.qdcount = 1;
		h.rd = 1;
		DPRINTF("encoding header\n", h.rd);
		i = __encode_header(&h, packet, PACKETSZ);
		if (i < 0)
			goto fail;

		/* encode question */
		DPRINTF("lookup name: %s\n", lookup);
		q.dotted = lookup;
		q.qtype = type;
		q.qclass = C_IN; /* CLASS_IN */
		j = __encode_question(&q, packet+i, PACKETSZ-i);
		if (j < 0)
			goto fail;
		packet_len = i + j;

		/* send packet */
#ifdef DEBUG
		{
			const socklen_t plen = sa.sa.sa_family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
			char *pbuf = malloc(plen);
			if (pbuf == NULL) ;/* nothing */
#ifdef __UCLIBC_HAS_IPV6__
			else if (sa.sa.sa_family == AF_INET6)
				pbuf = (char*)inet_ntop(AF_INET6, &sa.sa6.sin6_addr, pbuf, plen);
#endif
#ifdef __UCLIBC_HAS_IPV4__
			else if (sa.sa.sa_family == AF_INET)
				pbuf = (char*)inet_ntop(AF_INET, &sa.sa4.sin_addr, pbuf, plen);
#endif
			DPRINTF("On try %d, sending query to %s, port %d\n",
				retries_left, pbuf, NAMESERVER_PORT);
			free(pbuf);
		}
#endif
		fd = socket(sa.sa.sa_family, SOCK_DGRAM, IPPROTO_UDP);
		if (fd < 0) /* paranoia */
			goto try_next_server;
		rc = connect(fd, &sa.sa, sizeof(sa));
		if (rc < 0) {
			/*if (errno == ENETUNREACH) { */
				/* routing error, presume not transient */
				goto try_next_server;
			/*} */
/*For example, what transient error this can be? Can't think of any */
			/* retry */
			/*continue; */
		}
		DPRINTF("Xmit packet len:%d id:%d qr:%d\n", packet_len, h.id, h.qr);
		/* no error check - if it fails, we time out on recv */
		send(fd, packet, packet_len, 0);

#ifdef USE_SELECT
		reply_timeout = __resolv_timeout;
 wait_again:
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		tv.tv_sec = reply_timeout;
		tv.tv_usec = 0;
		if (select(fd + 1, &fds, NULL, NULL, &tv) <= 0) {
			DPRINTF("Timeout\n");
			/* timed out, so retry send and receive
			 * to next nameserver */
			goto try_next_server;
		}
		reply_timeout--;
#else /* !USE_SELECT */
		reply_timeout = __resolv_timeout * 1000;
 wait_again:
		fds.fd = fd;
		fds.events = POLLIN;
		if (poll(&fds, 1, reply_timeout) <= 0) {
			DPRINTF("Timeout\n");
			/* timed out, so retry send and receive
			 * to next nameserver */
			goto try_next_server;
		}
		if (fds.revents & (POLLERR | POLLHUP | POLLNVAL)) {
			DPRINTF("Bad event\n");
			goto try_next_server;
		}
/*TODO: better timeout accounting?*/
		reply_timeout -= 1000;
#endif /* USE_SELECT */

/* vda: a bogus response seen in real world (caused SEGV in uclibc):
 * "ping www.google.com" sending AAAA query and getting
 * response with one answer... with answer part missing!
 * Fixed by thorough checks for not going past the packet's end.
 */
#ifdef DEBUG
		{
			static const char test_query[32] = "\0\2\1\0\0\1\0\0\0\0\0\0\3www\6google\3com\0\0\34\0\1";
			static const char test_respn[32] = "\0\2\201\200\0\1\0\1\0\0\0\0\3www\6google\3com\0\0\34\0\1";
			pos = memcmp(packet + 2, test_query + 2, 30);
		packet_len = recv(fd, packet, PACKETSZ, MSG_DONTWAIT);
			if (pos == 0) {
				packet_len = 32;
				memcpy(packet + 2, test_respn + 2, 30);
			}
		}
#else
		packet_len = recv(fd, packet, PACKETSZ, MSG_DONTWAIT);
#endif

		if (packet_len < HFIXEDSZ) {
			/* too short!
			 * If the peer did shutdown then retry later,
			 * try next peer on error.
			 * it's just a bogus packet from somewhere */
 bogus_packet:
			if (packet_len >= 0 && reply_timeout)
				goto wait_again;
			goto try_next_server;
		}
		__decode_header(packet, &h);
		DPRINTF("len:%d id:%d qr:%d\n", packet_len, h.id, h.qr);
		if (h.id != local_id || !h.qr) {
			/* unsolicited */
			goto bogus_packet;
		}

		DPRINTF("Got response (i think)!\n");
		DPRINTF("qrcount=%d,ancount=%d,nscount=%d,arcount=%d\n",
				h.qdcount, h.ancount, h.nscount, h.arcount);
		DPRINTF("opcode=%d,aa=%d,tc=%d,rd=%d,ra=%d,rcode=%d\n",
				h.opcode, h.aa, h.tc, h.rd, h.ra, h.rcode);

		/* bug 660 says we treat negative response as an error
		 * and retry, which is, eh, an error. :)
		 * We were incurring long delays because of this. */
		if (h.rcode == NXDOMAIN || h.rcode == SERVFAIL) {
			/* if possible, try next search domain */
			if (!ends_with_dot) {
				DPRINTF("variant:%d sdomains:%d\n", variant, sdomains);
				if (variant < sdomains - 1) {
					/* next search domain */
					variant++;
					continue;
				}
				/* no more search domains to try */
			}
			if (h.rcode != SERVFAIL) {
				/* dont loop, this is "no such host" situation */
				h_errno = HOST_NOT_FOUND;
				goto fail1;
			}
		}
		/* Insert other non-fatal errors here, which do not warrant
		 * switching to next nameserver */

		/* Strange error, assuming this nameserver is feeling bad */
		if (h.rcode != 0)
			goto try_next_server;

		/* Code below won't work correctly with h.ancount == 0, so... */
		if (h.ancount <= 0) {
			h_errno = NO_DATA; /* [is this correct code to check for?] */
			goto fail1;
		}
		pos = HFIXEDSZ;
		for (j = 0; j < h.qdcount; j++) {
			DPRINTF("Skipping question %d at %d\n", j, pos);
			i = __length_question(packet + pos, packet_len - pos);
			if (i < 0) {
				DPRINTF("Packet'question section "
					"is truncated, trying next server\n");
				goto try_next_server;
			}
			pos += i;
			DPRINTF("Length of question %d is %d\n", j, i);
		}
		DPRINTF("Decoding answer at pos %d\n", pos);

		first_answer = 1;
		a->dotted = NULL;
		for (j = 0; j < h.ancount; j++) {
			i = __decode_answer(packet, pos, packet_len, &ma);
			if (i < 0) {
				DPRINTF("failed decode %d\n", i);
				/* If the message was truncated but we have
				 * decoded some answers, pretend it's OK */
				if (j && h.tc)
					break;
				goto try_next_server;
			}
			pos += i;

			if (first_answer) {
				ma.buf = a->buf;
				ma.buflen = a->buflen;
				ma.add_count = a->add_count;
				free(a->dotted);
				memcpy(a, &ma, sizeof(ma));
				if (a->atype != T_SIG && (NULL == a->buf || (type != T_A && type != T_AAAA)))
					break;
				if (a->atype != type)
					continue;
				a->add_count = h.ancount - j - 1;
				if ((a->rdlength + sizeof(struct in_addr*)) * a->add_count > a->buflen)
					break;
				a->add_count = 0;
				first_answer = 0;
			} else {
				free(ma.dotted);
				if (ma.atype != type)
					continue;
				if (a->rdlength != ma.rdlength) {
					free(a->dotted);
					DPRINTF("Answer address len(%u) differs from original(%u)\n",
							ma.rdlength, a->rdlength);
					goto try_next_server;
				}
				memcpy(a->buf + (a->add_count * ma.rdlength), ma.rdata, ma.rdlength);
				++a->add_count;
			}
		}

		/* Success! */
		DPRINTF("Answer name = |%s|\n", a->dotted);
		DPRINTF("Answer type = |%d|\n", a->atype);
		if (fd != -1)
			close(fd);
		if (outpacket)
			*outpacket = packet;
		else
			free(packet);
		free(lookup);
		return packet_len;

 try_next_server:
		/* Try next nameserver */
		local_ns_num++;
		variant = -1;
	} while (retries_left > 0);

 fail:
	h_errno = NETDB_INTERNAL;
 fail1:
	if (fd != -1)
		close(fd);
	free(lookup);
	free(packet);
	return -1;
}
#endif /* L_dnslookup */


#ifdef L_read_etc_hosts_r

parser_t * __open_etc_hosts(void)
{
	parser_t *parser;
	parser = config_open("/etc/hosts");
#ifdef FALLBACK_TO_CONFIG_RESOLVCONF
	if (parser == NULL)
		parser = config_open("/etc/config/hosts");
#endif
	return parser;
}

#define MINTOKENS 2 /* ip address + canonical name */
#define MAXTOKENS (MINTOKENS + MAXALIASES)
#define HALISTOFF (sizeof(char*) * MAXTOKENS)
#define INADDROFF (HALISTOFF + 2 * sizeof(char*))

int attribute_hidden __read_etc_hosts_r(
		parser_t * parser,
		const char *name,
		int type,
		enum etc_hosts_action action,
		struct hostent *result_buf,
		char *buf, size_t buflen,
		struct hostent **result,
		int *h_errnop)
{
	char **tok = NULL;
	struct in_addr *h_addr0 = NULL;
	const size_t aliaslen = INADDROFF +
#ifdef __UCLIBC_HAS_IPV6__
							sizeof(struct in6_addr)
#else
							sizeof(struct in_addr)
#endif
							;
	int ret = HOST_NOT_FOUND;

	*h_errnop = NETDB_INTERNAL;
	if (buflen < aliaslen
		|| (buflen - aliaslen) < BUFSZ + 1)
		return ERANGE;
	if (parser == NULL)
		parser = __open_etc_hosts();
	if (parser == NULL) {
		*result = NULL;
		return errno;
	}
	/* Layout in buf:
	 * char *alias[MAXTOKENS]  = {address, name, aliases...}
	 * char **h_addr_list[1]   = {*in[6]_addr, NULL}
	 * struct in[6]_addr
	 * char line_buffer[BUFSZ+];
	 */
	memset(buf, 0, buflen);
	parser->data = buf;
	parser->data_len = aliaslen;
	parser->line_len = buflen - aliaslen;
	*h_errnop = HOST_NOT_FOUND;
	/* <ip>[[:space:]][<aliases>] */
	while (config_read(parser, &tok, MAXTOKENS, MINTOKENS, "# \t", PARSE_NORMAL)) {
		result_buf->h_aliases = tok+1;
		if (action == GETHOSTENT) {
			/* Return whatever the next entry happens to be. */
			break;
		}
		if (action == GET_HOSTS_BYADDR) {
			if (strcmp(name, *tok) != 0)
				continue;
		} else { /* GET_HOSTS_BYNAME */
			int aliases = 0;
			char **alias = tok + 1;
			while (aliases < MAXALIASES) {
				char *tmp = *(alias+aliases++);
				if (tmp && strcasecmp(name, tmp) == 0)
					goto found;
			}
			continue;
		}
found:
		result_buf->h_name = *(result_buf->h_aliases++);
		result_buf->h_addr_list = (char**)(buf + HALISTOFF);
		*(result_buf->h_addr_list + 1) = '\0';
		h_addr0 = (struct in_addr*)(buf + INADDROFF);
		result_buf->h_addr = (char*)h_addr0;
		if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV4__
		else if (type == AF_INET
				&& inet_pton(AF_INET, *tok, h_addr0) > 0) {
			DPRINTF("Found INET\n");
			result_buf->h_addrtype = AF_INET;
			result_buf->h_length = sizeof(struct in_addr);
			*result = result_buf;
			ret = NETDB_SUCCESS;
		}
#endif
#ifdef __UCLIBC_HAS_IPV6__
#define in6 ((struct in6_addr *)buf)
		else if (type == AF_INET6
				&& inet_pton(AF_INET6, *tok, h_addr0) > 0) {
			DPRINTF("Found INET6\n");
			result_buf->h_addrtype = AF_INET6;
			result_buf->h_length = sizeof(struct in6_addr);
			*result = result_buf;
			ret = NETDB_SUCCESS;
		}
#endif
		else {
			/* continue parsing in the hope the user has multiple
			 * host types listed in the database like so:
			 * <ipv4 addr> host
			 * <ipv6 addr> host
			 * If looking for an IPv6 addr, don't bail when we got the IPv4
			 */
			DPRINTF("Error: Found host but different address family\n");
			/* NB: gethostbyname2_r depends on this feature
			 * to avoid looking for IPv6 addr of "localhost" etc */
			ret = TRY_AGAIN;
			continue;
		}
		break;
	}
	if (action != GETHOSTENT)
		config_close(parser);
	return ret;
#undef in6
}
#endif /* L_read_etc_hosts_r */


#ifdef L_get_hosts_byname_r

int attribute_hidden __get_hosts_byname_r(const char *name,
		int type,
		struct hostent *result_buf,
		char *buf,
		size_t buflen,
		struct hostent **result,
		int *h_errnop)
{
	return __read_etc_hosts_r(NULL, name, type, GET_HOSTS_BYNAME,
	                          result_buf, buf, buflen, result, h_errnop);
}
#endif /* L_get_hosts_byname_r */


#ifdef L_get_hosts_byaddr_r

int attribute_hidden __get_hosts_byaddr_r(const char *addr,
		int len,
		int type,
		struct hostent *result_buf,
		char *buf,
		size_t buflen,
		struct hostent **result,
		int *h_errnop)
{
#ifndef __UCLIBC_HAS_IPV6__
	char	ipaddr[INET_ADDRSTRLEN];
#else
	char	ipaddr[INET6_ADDRSTRLEN];
#endif

	switch (type) {
#ifdef __UCLIBC_HAS_IPV4__
		case AF_INET:
			if (len != sizeof(struct in_addr))
				return 0;
			break;
#endif
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
			if (len != sizeof(struct in6_addr))
				return 0;
			break;
#endif
		default:
			return 0;
	}

	inet_ntop(type, addr, ipaddr, sizeof(ipaddr));

	return __read_etc_hosts_r(NULL, ipaddr, type, GET_HOSTS_BYADDR,
				result_buf, buf, buflen, result, h_errnop);
}
#endif /* L_get_hosts_byaddr_r */


#ifdef L_getnameinfo

int getnameinfo(const struct sockaddr *sa,
		socklen_t addrlen,
		char *host,
		socklen_t hostlen,
		char *serv,
		socklen_t servlen,
		unsigned flags)
{
	int serrno = errno;
	unsigned ok;
	struct hostent *hoste = NULL;
	char domain[256];

	if (flags & ~(NI_NUMERICHOST|NI_NUMERICSERV|NI_NOFQDN|NI_NAMEREQD|NI_DGRAM))
		return EAI_BADFLAGS;

	if (sa == NULL || addrlen < sizeof(sa_family_t))
		return EAI_FAMILY;

	ok = sa->sa_family;
	if (ok == AF_LOCAL) /* valid */;
#ifdef __UCLIBC_HAS_IPV4__
	else if (ok == AF_INET) {
		if (addrlen < sizeof(struct sockaddr_in))
			return EAI_FAMILY;
	}
#endif
#ifdef __UCLIBC_HAS_IPV6__
	else if (ok == AF_INET6) {
		if (addrlen < sizeof(struct sockaddr_in6))
			return EAI_FAMILY;
	}
#endif
	else
		return EAI_FAMILY;

	ok = 0;
	if (host != NULL && hostlen > 0)
		switch (sa->sa_family) {
		case AF_INET:
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
#endif
			if (!(flags & NI_NUMERICHOST)) {
				if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV6__
				else if (sa->sa_family == AF_INET6)
					hoste = gethostbyaddr((const void *)
						&(((const struct sockaddr_in6 *) sa)->sin6_addr),
						sizeof(struct in6_addr), AF_INET6);
#endif
#ifdef __UCLIBC_HAS_IPV4__
				else
					hoste = gethostbyaddr((const void *)
						&(((const struct sockaddr_in *)sa)->sin_addr),
						sizeof(struct in_addr), AF_INET);
#endif

				if (hoste) {
					char *c;
#undef min
#define min(x,y) (((x) > (y)) ? (y) : (x))
					if ((flags & NI_NOFQDN)
					 && (getdomainname(domain, sizeof(domain)) == 0)
					 && (c = strstr(hoste->h_name, domain)) != NULL
					 && (c != hoste->h_name) && (*(--c) == '.')
					) {
						strncpy(host, hoste->h_name,
							min(hostlen, (size_t) (c - hoste->h_name)));
						host[min(hostlen - 1, (size_t) (c - hoste->h_name))] = '\0';
					} else {
						strncpy(host, hoste->h_name, hostlen);
					}
					ok = 1;
#undef min
				}
			}

			if (!ok) {
				const char *c = NULL;

				if (flags & NI_NAMEREQD) {
					errno = serrno;
					return EAI_NONAME;
				}
				if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV6__
				else if (sa->sa_family == AF_INET6) {
					const struct sockaddr_in6 *sin6p;

					sin6p = (const struct sockaddr_in6 *) sa;
					c = inet_ntop(AF_INET6,
						(const void *) &sin6p->sin6_addr,
						host, hostlen);
#if 0
					/* Does scope id need to be supported? */
					uint32_t scopeid;
					scopeid = sin6p->sin6_scope_id;
					if (scopeid != 0) {
						/* Buffer is >= IFNAMSIZ+1.  */
						char scopebuf[IFNAMSIZ + 1];
						char *scopeptr;
						int ni_numericscope = 0;
						size_t real_hostlen = strnlen(host, hostlen);
						size_t scopelen = 0;

						scopebuf[0] = SCOPE_DELIMITER;
						scopebuf[1] = '\0';
						scopeptr = &scopebuf[1];

						if (IN6_IS_ADDR_LINKLOCAL(&sin6p->sin6_addr)
						    || IN6_IS_ADDR_MC_LINKLOCAL(&sin6p->sin6_addr)) {
							if (if_indextoname(scopeid, scopeptr) == NULL)
								++ni_numericscope;
							else
								scopelen = strlen(scopebuf);
						} else {
							++ni_numericscope;
						}

						if (ni_numericscope)
							scopelen = 1 + snprintf(scopeptr,
								(scopebuf
								+ sizeof scopebuf
								- scopeptr),
								"%u", scopeid);

						if (real_hostlen + scopelen + 1 > hostlen)
							return EAI_SYSTEM;
						memcpy(host + real_hostlen, scopebuf, scopelen + 1);
					}
#endif
				}
#endif /* __UCLIBC_HAS_IPV6__ */
#if defined __UCLIBC_HAS_IPV4__
				else {
					c = inet_ntop(AF_INET, (const void *)
						&(((const struct sockaddr_in *) sa)->sin_addr),
						host, hostlen);
				}
#endif
				if (c == NULL) {
					errno = serrno;
					return EAI_SYSTEM;
				}
				ok = 1;
			}
			break;

		case AF_LOCAL:
			if (!(flags & NI_NUMERICHOST)) {
				struct utsname utsname;

				if (!uname(&utsname)) {
					strncpy(host, utsname.nodename, hostlen);
					break;
				};
			};

			if (flags & NI_NAMEREQD) {
				errno = serrno;
				return EAI_NONAME;
			}

			strncpy(host, "localhost", hostlen);
			break;
/* Already checked above
		default:
			return EAI_FAMILY;
*/
	}

	if (serv && (servlen > 0)) {
		if (sa->sa_family == AF_LOCAL) {
			strncpy(serv, ((const struct sockaddr_un *) sa)->sun_path, servlen);
		} else { /* AF_INET || AF_INET6 */
			if (!(flags & NI_NUMERICSERV)) {
				struct servent *s;
				s = getservbyport(((const struct sockaddr_in *) sa)->sin_port,
				      ((flags & NI_DGRAM) ? "udp" : "tcp"));
				if (s) {
					strncpy(serv, s->s_name, servlen);
					goto DONE;
				}
			}
			snprintf(serv, servlen, "%d",
				ntohs(((const struct sockaddr_in *) sa)->sin_port));
		}
	}
DONE:
	if (host && (hostlen > 0))
		host[hostlen-1] = 0;
	if (serv && (servlen > 0))
		serv[servlen-1] = 0;
	errno = serrno;
	return 0;
}
libc_hidden_def(getnameinfo)
#endif /* L_getnameinfo */


#ifdef L_gethostbyname_r

/* Bug 671 says:
 * "uClibc resolver's gethostbyname does not return the requested name
 * as an alias, but instead returns the canonical name. glibc's
 * gethostbyname has a similar bug where it returns the requested name
 * with the search domain name appended (to make a FQDN) as an alias,
 * but not the original name itself. Both contradict POSIX, which says
 * that the name argument passed to gethostbyname must be in the alias list"
 * This is fixed now, and we differ from glibc:
 *
 * $ ./gethostbyname_uclibc wer.google.com
 * h_name:'c13-ss-2-lb.cnet.com'
 * h_length:4
 * h_addrtype:2 AF_INET
 * alias:'wer.google.com' <===
 * addr: 0x4174efd8 '216.239.116.65'
 *
 * $ ./gethostbyname_glibc wer.google.com
 * h_name:'c13-ss-2-lb.cnet.com'
 * h_length:4
 * h_addrtype:2 AF_INET
 * alias:'wer.google.com.com' <===
 * addr:'216.239.116.65'
 *
 * When examples were run, /etc/resolv.conf contained "search com" line.
 */
int gethostbyname_r(const char *name,
		struct hostent *result_buf,
		char *buf,
		size_t buflen,
		struct hostent **result,
		int *h_errnop)
{
	struct in_addr **addr_list;
	char **alias;
	char *alias0;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int packet_len;
	int wrong_af = 0;

	*result = NULL;
	if (!name)
		return EINVAL;

	/* do /etc/hosts first */
	{
		int old_errno = errno;  /* save the old errno and reset errno */
		__set_errno(0);         /* to check for missing /etc/hosts. */
		i = __get_hosts_byname_r(name, AF_INET, result_buf,
				buf, buflen, result, h_errnop);
		if (i == NETDB_SUCCESS) {
			__set_errno(old_errno);
			return i;
		}
		switch (*h_errnop) {
			case HOST_NOT_FOUND:
				wrong_af = (i == TRY_AGAIN);
			case NO_ADDRESS:
				break;
			case NETDB_INTERNAL:
				if (errno == ENOENT) {
					break;
				}
				/* else fall through */
			default:
				return i;
		}
		__set_errno(old_errno);
	}

	DPRINTF("Nothing found in /etc/hosts\n");

	*h_errnop = NETDB_INTERNAL;

	/* prepare future h_aliases[0] */
	i = strlen(name) + 1;
	if ((ssize_t)buflen <= i)
		return ERANGE;
	memcpy(buf, name, i); /* paranoia: name might change */
	alias0 = buf;
	buf += i;
	buflen -= i;
	/* make sure pointer is aligned */
	i = ALIGN_BUFFER_OFFSET(buf);
	buf += i;
	buflen -= i;
	/* Layout in buf:
	 * char *alias[2];
	 * struct in_addr* addr_list[NN+1];
	 * struct in_addr* in[NN];
	 */
	alias = (char **)buf;
	buf += sizeof(alias[0]) * 2;
	buflen -= sizeof(alias[0]) * 2;
	addr_list = (struct in_addr **)buf;
	/* buflen may be < 0, must do signed compare */
	if ((ssize_t)buflen < 256)
		return ERANGE;

	/* we store only one "alias" - the name itself */
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO -- generate the full list
#endif
	alias[0] = alias0;
	alias[1] = NULL;

	/* maybe it is already an address? */
	{
		struct in_addr *in = (struct in_addr *)(buf + sizeof(addr_list[0]) * 2);
		if (inet_aton(name, in)) {
			addr_list[0] = in;
			addr_list[1] = NULL;
			result_buf->h_name = alias0;
			result_buf->h_aliases = alias;
			result_buf->h_addrtype = AF_INET;
			result_buf->h_length = sizeof(struct in_addr);
			result_buf->h_addr_list = (char **) addr_list;
			*result = result_buf;
			*h_errnop = NETDB_SUCCESS;
			return NETDB_SUCCESS;
		}
	}

	/* what if /etc/hosts has it but it's not IPv4?
	 * F.e. "::1 localhost6". We don't do DNS query for such hosts -
	 * "ping localhost6" should be fast even if DNS server is down! */
	if (wrong_af) {
		*h_errnop = HOST_NOT_FOUND;
		return TRY_AGAIN;
	}

	/* talk to DNS servers */
	a.buf = buf;
	/* take into account that at least one address will be there,
	 * we'll need space for one in_addr + two addr_list[] elems */
	a.buflen = buflen - ((sizeof(addr_list[0]) * 2 + sizeof(struct in_addr)));
	a.add_count = 0;
	packet_len = __dns_lookup(name, T_A, &packet, &a);
	if (packet_len < 0) {
		*h_errnop = HOST_NOT_FOUND;
		DPRINTF("__dns_lookup returned < 0\n");
		return TRY_AGAIN;
	}

	if (a.atype == T_A) { /* ADDRESS */
		/* we need space for addr_list[] and one IPv4 address */
		/* + 1 accounting for 1st addr (it's in a.rdata),
		 * another + 1 for NULL in last addr_list[]: */
		int need_bytes = sizeof(addr_list[0]) * (a.add_count + 1 + 1)
				/* for 1st addr (it's in a.rdata): */
				+ sizeof(struct in_addr);
		/* how many bytes will 2nd and following addresses take? */
		int ips_len = a.add_count * a.rdlength;

		buflen -= (need_bytes + ips_len);
		if ((ssize_t)buflen < 0) {
			DPRINTF("buffer too small for all addresses\n");
			/* *h_errnop = NETDB_INTERNAL; - already is */
			i = ERANGE;
			goto free_and_ret;
		}

		/* if there are additional addresses in buf,
		 * move them forward so that they are not destroyed */
		DPRINTF("a.add_count:%d a.rdlength:%d a.rdata:%p\n", a.add_count, a.rdlength, a.rdata);
		memmove(buf + need_bytes, buf, ips_len);

		/* 1st address is in a.rdata, insert it  */
		buf += need_bytes - sizeof(struct in_addr);
		memcpy(buf, a.rdata, sizeof(struct in_addr));

		/* fill addr_list[] */
		for (i = 0; i <= a.add_count; i++) {
			addr_list[i] = (struct in_addr*)buf;
			buf += sizeof(struct in_addr);
		}
		addr_list[i] = NULL;

		/* if we have enough space, we can report "better" name
		 * (it may contain search domains attached by __dns_lookup,
		 * or CNAME of the host if it is different from the name
		 * we used to find it) */
		if (a.dotted && buflen > strlen(a.dotted)) {
			strcpy(buf, a.dotted);
			alias0 = buf;
		}

		result_buf->h_name = alias0;
		result_buf->h_aliases = alias;
		result_buf->h_addrtype = AF_INET;
		result_buf->h_length = sizeof(struct in_addr);
		result_buf->h_addr_list = (char **) addr_list;
		*result = result_buf;
		*h_errnop = NETDB_SUCCESS;
		i = NETDB_SUCCESS;
		goto free_and_ret;
	}

	*h_errnop = HOST_NOT_FOUND;
	__set_h_errno(HOST_NOT_FOUND);
	i = TRY_AGAIN;

 free_and_ret:
	free(a.dotted);
	free(packet);
	return i;
}
libc_hidden_def(gethostbyname_r)
link_warning(gethostbyname_r, "gethostbyname_r is obsolescent, use getnameinfo() instead.");
#endif /* L_gethostbyname_r */


#ifdef L_gethostbyname2_r

int gethostbyname2_r(const char *name,
		int family,
		struct hostent *result_buf,
		char *buf,
		size_t buflen,
		struct hostent **result,
		int *h_errnop)
{
#ifndef __UCLIBC_HAS_IPV6__
	return family == (AF_INET)
		? gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop)
		: HOST_NOT_FOUND;
#else
	struct in6_addr **addr_list;
	char **alias;
	char *alias0;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int packet_len;
	int wrong_af = 0;

	if (family == AF_INET)
		return gethostbyname_r(name, result_buf, buf, buflen, result, h_errnop);

	*result = NULL;
	if (family != AF_INET6)
		return EINVAL;

	if (!name)
		return EINVAL;

	/* do /etc/hosts first */
	{
		int old_errno = errno;  /* save the old errno and reset errno */
		__set_errno(0);         /* to check for missing /etc/hosts. */
		i = __get_hosts_byname_r(name, AF_INET6, result_buf,
				buf, buflen, result, h_errnop);
		if (i == NETDB_SUCCESS) {
			__set_errno(old_errno);
			return i;
		}
		switch (*h_errnop) {
			case HOST_NOT_FOUND:
				wrong_af = (i == TRY_AGAIN);
			case NO_ADDRESS:
				break;
			case NETDB_INTERNAL:
				if (errno == ENOENT) {
					break;
				}
				/* else fall through */
			default:
				return i;
		}
		__set_errno(old_errno);
	}

	DPRINTF("Nothing found in /etc/hosts\n");

	*h_errnop = NETDB_INTERNAL;

	/* prepare future h_aliases[0] */
	i = strlen(name) + 1;
	if ((ssize_t)buflen <= i)
		return ERANGE;
	memcpy(buf, name, i); /* paranoia: name might change */
	alias0 = buf;
	buf += i;
	buflen -= i;
	/* make sure pointer is aligned */
	i = ALIGN_BUFFER_OFFSET(buf);
	buf += i;
	buflen -= i;
	/* Layout in buf:
	 * char *alias[2];
	 * struct in6_addr* addr_list[NN+1];
	 * struct in6_addr* in[NN];
	 */
	alias = (char **)buf;
	buf += sizeof(alias[0]) * 2;
	buflen -= sizeof(alias[0]) * 2;
	addr_list = (struct in6_addr **)buf;
	/* buflen may be < 0, must do signed compare */
	if ((ssize_t)buflen < 256)
		return ERANGE;

	/* we store only one "alias" - the name itself */
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO -- generate the full list
#endif
	alias[0] = alias0;
	alias[1] = NULL;

	/* maybe it is already an address? */
	{
		struct in6_addr *in = (struct in6_addr *)(buf + sizeof(addr_list[0]) * 2);
		if (inet_pton(AF_INET6, name, in)) {
			addr_list[0] = in;
			addr_list[1] = NULL;
			result_buf->h_name = alias0;
			result_buf->h_aliases = alias;
			result_buf->h_addrtype = AF_INET6;
			result_buf->h_length = sizeof(struct in6_addr);
			result_buf->h_addr_list = (char **) addr_list;
			*result = result_buf;
			*h_errnop = NETDB_SUCCESS;
			return NETDB_SUCCESS;
		}
	}

	/* what if /etc/hosts has it but it's not IPv6?
	 * F.e. "127.0.0.1 localhost". We don't do DNS query for such hosts -
	 * "ping localhost" should be fast even if DNS server is down! */
	if (wrong_af) {
		*h_errnop = HOST_NOT_FOUND;
		return TRY_AGAIN;
	}

	/* talk to DNS servers */
	a.buf = buf;
	/* take into account that at least one address will be there,
	 * we'll need space of one in6_addr + two addr_list[] elems */
	a.buflen = buflen - ((sizeof(addr_list[0]) * 2 + sizeof(struct in6_addr)));
	a.add_count = 0;
	packet_len = __dns_lookup(name, T_AAAA, &packet, &a);
	if (packet_len < 0) {
		*h_errnop = HOST_NOT_FOUND;
		DPRINTF("__dns_lookup returned < 0\n");
		return TRY_AGAIN;
	}

	if (a.atype == T_AAAA) { /* ADDRESS */
		/* we need space for addr_list[] and one IPv6 address */
		/* + 1 accounting for 1st addr (it's in a.rdata),
		 * another + 1 for NULL in last addr_list[]: */
		int need_bytes = sizeof(addr_list[0]) * (a.add_count + 1 + 1)
				/* for 1st addr (it's in a.rdata): */
				+ sizeof(struct in6_addr);
		/* how many bytes will 2nd and following addresses take? */
		int ips_len = a.add_count * a.rdlength;

		buflen -= (need_bytes + ips_len);
		if ((ssize_t)buflen < 0) {
			DPRINTF("buffer too small for all addresses\n");
			/* *h_errnop = NETDB_INTERNAL; - already is */
			i = ERANGE;
			goto free_and_ret;
		}

		/* if there are additional addresses in buf,
		 * move them forward so that they are not destroyed */
		DPRINTF("a.add_count:%d a.rdlength:%d a.rdata:%p\n", a.add_count, a.rdlength, a.rdata);
		memmove(buf + need_bytes, buf, ips_len);

		/* 1st address is in a.rdata, insert it  */
		buf += need_bytes - sizeof(struct in6_addr);
		memcpy(buf, a.rdata, sizeof(struct in6_addr));

		/* fill addr_list[] */
		for (i = 0; i <= a.add_count; i++) {
			addr_list[i] = (struct in6_addr*)buf;
			buf += sizeof(struct in6_addr);
		}
		addr_list[i] = NULL;

		/* if we have enough space, we can report "better" name
		 * (it may contain search domains attached by __dns_lookup,
		 * or CNAME of the host if it is different from the name
		 * we used to find it) */
		if (a.dotted && buflen > strlen(a.dotted)) {
			strcpy(buf, a.dotted);
			alias0 = buf;
		}

		result_buf->h_name = alias0;
		result_buf->h_aliases = alias;
		result_buf->h_addrtype = AF_INET6;
		result_buf->h_length = sizeof(struct in6_addr);
		result_buf->h_addr_list = (char **) addr_list;
		*result = result_buf;
		*h_errnop = NETDB_SUCCESS;
		i = NETDB_SUCCESS;
		goto free_and_ret;
	}

	*h_errnop = HOST_NOT_FOUND;
	__set_h_errno(HOST_NOT_FOUND);
	i = TRY_AGAIN;

 free_and_ret:
	free(a.dotted);
	free(packet);
	return i;
#endif /* __UCLIBC_HAS_IPV6__ */
}
libc_hidden_def(gethostbyname2_r)
#endif /* L_gethostbyname2_r */


#ifdef L_gethostbyaddr_r

int gethostbyaddr_r(const void *addr, socklen_t addrlen,
		int type,
		struct hostent *result_buf,
		char *buf, size_t buflen,
		struct hostent **result,
		int *h_errnop)

{
	struct in_addr *in;
	struct in_addr **addr_list;
	char **alias;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int packet_len;
	int nest = 0;

	*result = NULL;
	if (!addr)
		return EINVAL;

	switch (type) {
#ifdef __UCLIBC_HAS_IPV4__
		case AF_INET:
			if (addrlen != sizeof(struct in_addr))
				return EINVAL;
			break;
#endif
#ifdef __UCLIBC_HAS_IPV6__
		case AF_INET6:
			if (addrlen != sizeof(struct in6_addr))
				return EINVAL;
			break;
#endif
		default:
			return EINVAL;
	}

	/* do /etc/hosts first */
	i = __get_hosts_byaddr_r(addr, addrlen, type, result_buf,
				buf, buflen, result, h_errnop);
	if (i == 0)
		return i;
	switch (*h_errnop) {
		case HOST_NOT_FOUND:
		case NO_ADDRESS:
			break;
		default:
			return i;
	}

	*h_errnop = NETDB_INTERNAL;

	/* make sure pointer is aligned */
	i = ALIGN_BUFFER_OFFSET(buf);
	buf += i;
	buflen -= i;
	/* Layout in buf:
	 * char *alias[ALIAS_DIM];
	 * struct in[6]_addr* addr_list[2];
	 * struct in[6]_addr in;
	 * char scratch_buffer[256+];
	 */
#define in6 ((struct in6_addr *)in)
	alias = (char **)buf;
	addr_list = (struct in_addr**)buf;
	buf += sizeof(*addr_list) * 2;
	buflen -= sizeof(*addr_list) * 2;
	in = (struct in_addr*)buf;
#ifndef __UCLIBC_HAS_IPV6__
	buf += sizeof(*in);
	buflen -= sizeof(*in);
	if (addrlen > sizeof(*in))
		return ERANGE;
#else
	buf += sizeof(*in6);
	buflen -= sizeof(*in6);
	if (addrlen > sizeof(*in6))
		return ERANGE;
#endif
	if ((ssize_t)buflen < 256)
		return ERANGE;
	alias[0] = buf;
	alias[1] = NULL;
	addr_list[0] = in;
	addr_list[1] = NULL;
	memcpy(in, addr, addrlen);

	if (0) /* nothing */;
#ifdef __UCLIBC_HAS_IPV4__
	else IF_HAS_BOTH(if (type == AF_INET)) {
		unsigned char *tp = (unsigned char *)addr;
		sprintf(buf, "%u.%u.%u.%u.in-addr.arpa",
				tp[3], tp[2], tp[1], tp[0]);
	}
#endif
#ifdef __UCLIBC_HAS_IPV6__
	else {
		char *dst = buf;
		unsigned char *tp = (unsigned char *)addr + addrlen - 1;
		do {
			dst += sprintf(dst, "%x.%x.", tp[0] & 0xf, tp[0] >> 4);
			tp--;
		} while (tp >= (unsigned char *)addr);
		strcpy(dst, "ip6.arpa");
	}
#endif

	memset(&a, '\0', sizeof(a));
	for (;;) {
/* Hmm why we memset(a) to zeros only once? */
		packet_len = __dns_lookup(buf, T_PTR, &packet, &a);
		if (packet_len < 0) {
			*h_errnop = HOST_NOT_FOUND;
			return TRY_AGAIN;
		}

		strncpy(buf, a.dotted, buflen);
		free(a.dotted);
		if (a.atype != T_CNAME)
			break;

		DPRINTF("Got a CNAME in gethostbyaddr()\n");
		if (++nest > MAX_RECURSE) {
			*h_errnop = NO_RECOVERY;
			return -1;
		}
		/* Decode CNAME into buf, feed it to __dns_lookup() again */
		i = __decode_dotted(packet, a.rdoffset, packet_len, buf, buflen);
		free(packet);
		if (i < 0) {
			*h_errnop = NO_RECOVERY;
			return -1;
		}
	}

	if (a.atype == T_PTR) {	/* ADDRESS */
		i = __decode_dotted(packet, a.rdoffset, packet_len, buf, buflen);
		free(packet);
		result_buf->h_name = buf;
		result_buf->h_addrtype = type;
		result_buf->h_length = addrlen;
		result_buf->h_addr_list = (char **) addr_list;
		result_buf->h_aliases = alias;
		*result = result_buf;
		*h_errnop = NETDB_SUCCESS;
		return NETDB_SUCCESS;
	}

	free(packet);
	*h_errnop = NO_ADDRESS;
	return TRY_AGAIN;
#undef in6
}
libc_hidden_def(gethostbyaddr_r)
link_warning(gethostbyaddr_r, "gethostbyaddr_r is obsolescent, use getaddrinfo() instead.");
#endif /* L_gethostbyaddr_r */


#ifdef L_gethostent_r

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

static parser_t *hostp = NULL;
static smallint host_stayopen;

void endhostent_unlocked(void)
{
	if (hostp) {
		config_close(hostp);
		hostp = NULL;
	}
	host_stayopen = 0;
}
void endhostent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	endhostent_unlocked();
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

void sethostent(int stay_open)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (stay_open)
		host_stayopen = 1;
	__UCLIBC_MUTEX_UNLOCK(mylock);
}

int gethostent_r(struct hostent *result_buf, char *buf, size_t buflen,
	struct hostent **result, int *h_errnop)
{
	int ret = HOST_NOT_FOUND;

	__UCLIBC_MUTEX_LOCK(mylock);
	if (hostp == NULL) {
		hostp = __open_etc_hosts();
		if (hostp == NULL) {
			*result = NULL;
			ret = TRY_AGAIN;
			goto DONE;
		}
	}

	ret = __read_etc_hosts_r(hostp, NULL, AF_INET, GETHOSTENT,
		   result_buf, buf, buflen, result, h_errnop);
	if (!host_stayopen)
		endhostent_unlocked();
DONE:
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return ret;
}
libc_hidden_def(gethostent_r)
#endif /* L_gethostent_r */


#ifdef L_gethostent

struct hostent *gethostent(void)
{
	static struct hostent hoste;
	struct hostent *host;

	__init_getXXaddrbuf();
	gethostent_r(&hoste, static_getXXaddrbuf, GETXX_BUFSIZE, &host, &h_errno);
	return host;
}
#endif /* L_gethostent */


#ifdef L_gethostbyname2

struct hostent *gethostbyname2(const char *name, int family)
{
	static struct hostent hoste;
	struct hostent *hp;

	__init_getXXaddrbuf();
#ifndef __UCLIBC_HAS_IPV6__
	if (family != AF_INET)
		return (struct hostent*)NULL;
	gethostbyname_r(name, &hoste, static_getXXaddrbuf, GETXX_BUFSIZE, &hp, &h_errno);
#else
	gethostbyname2_r(name, family, &hoste, static_getXXaddrbuf, GETXX_BUFSIZE, &hp, &h_errno);
#endif /* __UCLIBC_HAS_IPV6__ */

	return hp;
}
libc_hidden_def(gethostbyname2)
#endif /* L_gethostbyname2 */


#ifdef L_gethostbyname

struct hostent *gethostbyname(const char *name)
{
	return gethostbyname2(name, AF_INET);
}
libc_hidden_def(gethostbyname)
link_warning(gethostbyname, "gethostbyname is obsolescent, use getnameinfo() instead.");
#endif /* L_gethostbyname */


#ifdef L_gethostbyaddr

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type)
{
	static struct hostent hoste;
	struct hostent *hp;

	__init_getXXaddrbuf();
	gethostbyaddr_r(addr, len, type, &hoste, static_getXXaddrbuf, GETXX_BUFSIZE, &hp, &h_errno);
	return hp;
}
libc_hidden_def(gethostbyaddr)
link_warning(gethostbyaddr, "gethostbyaddr is obsolescent, use getaddrinfo() instead.");
#endif /* L_gethostbyaddr */


#ifdef L_res_comp

/*
 * Expand compressed domain name 'comp_dn' to full domain name.
 * 'msg' is a pointer to the begining of the message,
 * 'eomorig' points to the first location after the message,
 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
 * Return size of compressed name or -1 if there was an error.
 */
int dn_expand(const u_char *msg, const u_char *eom, const u_char *src,
				char *dst, int dstsiz)
{
	int n = ns_name_uncompress(msg, eom, src, dst, (size_t)dstsiz);

	if (n > 0 && dst[0] == '.')
		dst[0] = '\0';
	return n;
}
libc_hidden_def(dn_expand)

/*
 * Pack domain name 'exp_dn' in presentation form into 'comp_dn'.
 * Return the size of the compressed name or -1.
 * 'length' is the size of the array pointed to by 'comp_dn'.
 */
int
dn_comp(const char *src, u_char *dst, int dstsiz,
		u_char **dnptrs, u_char **lastdnptr)
{
	return ns_name_compress(src, dst, (size_t) dstsiz,
			(const u_char **) dnptrs,
			(const u_char **) lastdnptr);
}
libc_hidden_def(dn_comp)
#endif /* L_res_comp */


#ifdef L_ns_name

/* Thinking in noninternationalized USASCII (per the DNS spec),
 * is this character visible and not a space when printed ?
 */
static int printable(int ch)
{
	return (ch > 0x20 && ch < 0x7f);
}
/* Thinking in noninternationalized USASCII (per the DNS spec),
 * is this characted special ("in need of quoting") ?
 */
static int special(int ch)
{
	switch (ch) {
		case 0x22: /* '"' */
		case 0x2E: /* '.' */
		case 0x3B: /* ';' */
		case 0x5C: /* '\\' */
			/* Special modifiers in zone files. */
		case 0x40: /* '@' */
		case 0x24: /* '$' */
			return 1;
		default:
			return 0;
	}
}

/*
 * ns_name_uncompress(msg, eom, src, dst, dstsiz)
 *      Expand compressed domain name to presentation format.
 * return:
 *      Number of bytes read out of `src', or -1 (with errno set).
 * note:
 *      Root domain returns as "." not "".
 */
int ns_name_uncompress(const u_char *msg, const u_char *eom,
		const u_char *src, char *dst, size_t dstsiz)
{
	u_char tmp[NS_MAXCDNAME];
	int n;

	n = ns_name_unpack(msg, eom, src, tmp, sizeof tmp);
	if (n == -1)
		return -1;
	if (ns_name_ntop(tmp, dst, dstsiz) == -1)
		return -1;
	return n;
}
libc_hidden_def(ns_name_uncompress)

/*
 * ns_name_ntop(src, dst, dstsiz)
 *      Convert an encoded domain name to printable ascii as per RFC1035.
 * return:
 *      Number of bytes written to buffer, or -1 (with errno set)
 * notes:
 *      The root is returned as "."
 *      All other domains are returned in non absolute form
 */
int ns_name_ntop(const u_char *src, char *dst, size_t dstsiz)
{
	const u_char *cp;
	char *dn, *eom;
	u_char c;
	u_int n;

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	while ((n = *cp++) != 0) {
		if ((n & NS_CMPRSFLGS) != 0) {
			/* Some kind of compression pointer. */
			__set_errno(EMSGSIZE);
			return -1;
		}
		if (dn != dst) {
			if (dn >= eom) {
				__set_errno(EMSGSIZE);
				return -1;
			}
			*dn++ = '.';
		}
		if (dn + n >= eom) {
			__set_errno(EMSGSIZE);
			return -1;
		}
		for (; n > 0; n--) {
			c = *cp++;
			if (special(c)) {
				if (dn + 1 >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				*dn++ = '\\';
				*dn++ = (char)c;
			} else if (!printable(c)) {
				if (dn + 3 >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				*dn++ = '\\';
				*dn++ = "0123456789"[c / 100];
				c = c % 100;
				*dn++ = "0123456789"[c / 10];
				*dn++ = "0123456789"[c % 10];
			} else {
				if (dn >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				*dn++ = (char)c;
			}
		}
	}
	if (dn == dst) {
		if (dn >= eom) {
			__set_errno(EMSGSIZE);
			return -1;
		}
		*dn++ = '.';
	}
	if (dn >= eom) {
		__set_errno(EMSGSIZE);
		return -1;
	}
	*dn++ = '\0';
	return (dn - dst);
}
libc_hidden_def(ns_name_ntop)

static int encode_bitstring(const char **bp, const char *end,
							unsigned char **labelp,
							unsigned char ** dst,
							unsigned const char *eom)
{
	int afterslash = 0;
	const char *cp = *bp;
	unsigned char *tp;
	const char *beg_blen;
	int value = 0, count = 0, tbcount = 0, blen = 0;

	beg_blen = NULL;

	/* a bitstring must contain at least 2 characters */
	if (end - cp < 2)
		return EINVAL;

	/* XXX: currently, only hex strings are supported */
	if (*cp++ != 'x')
		return EINVAL;
	if (!isxdigit((unsigned char) *cp)) /*%< reject '\[x/BLEN]' */
		return EINVAL;

	for (tp = *dst + 1; cp < end && tp < eom; cp++) {
		unsigned char c = *cp;

		switch (c) {
		case ']':       /*%< end of the bitstring */
			if (afterslash) {
				char *end_blen;
				if (beg_blen == NULL)
					return EINVAL;
				blen = (int)strtol(beg_blen, &end_blen, 10);
				if (*end_blen != ']')
					return EINVAL;
			}
			if (count)
				*tp++ = ((value << 4) & 0xff);
			cp++;   /*%< skip ']' */
			goto done;
		case '/':
			afterslash = 1;
			break;
		default:
			if (afterslash) {
				if (!__isdigit_char(c))
					return EINVAL;
				if (beg_blen == NULL) {
					if (c == '0') {
						/* blen never begings with 0 */
						return EINVAL;
					}
					beg_blen = cp;
				}
			} else {
				if (!__isdigit_char(c)) {
					c = c | 0x20; /* lowercase */
					c = c - 'a';
					if (c > 5) /* not a-f? */
						return EINVAL;
					c += 10 + '0';
				}
				value <<= 4;
				value += (c - '0');
				count += 4;
				tbcount += 4;
				if (tbcount > 256)
					return EINVAL;
				if (count == 8) {
					*tp++ = value;
					count = 0;
				}
			}
			break;
		}
	}
  done:
	if (cp >= end || tp >= eom)
		return EMSGSIZE;

	/*
	 * bit length validation:
	 * If a <length> is present, the number of digits in the <bit-data>
	 * MUST be just sufficient to contain the number of bits specified
	 * by the <length>. If there are insignificant bits in a final
	 * hexadecimal or octal digit, they MUST be zero.
	 * RFC2673, Section 3.2.
	 */
	if (blen > 0) {
		int traillen;

		if (((blen + 3) & ~3) != tbcount)
			return EINVAL;
		traillen = tbcount - blen; /*%< between 0 and 3 */
		if (((value << (8 - traillen)) & 0xff) != 0)
			return EINVAL;
	}
	else
		blen = tbcount;
	if (blen == 256)
		blen = 0;

	/* encode the type and the significant bit fields */
	**labelp = DNS_LABELTYPE_BITSTRING;
	**dst = blen;

	*bp = cp;
	*dst = tp;

	return 0;
}

int ns_name_pton(const char *src, u_char *dst, size_t dstsiz)
{
	static const char digits[] = "0123456789";
	u_char *label, *bp, *eom;
	int c, n, escaped, e = 0;
	char *cp;

	escaped = 0;
	bp = dst;
	eom = dst + dstsiz;
	label = bp++;

	while ((c = *src++) != 0) {
		if (escaped) {
			if (c == '[') { /*%< start a bit string label */
				cp = strchr(src, ']');
				if (cp == NULL) {
					errno = EINVAL; /*%< ??? */
					return -1;
				}
				e = encode_bitstring(&src, cp + 2,
							 &label, &bp, eom);
				if (e != 0) {
					errno = e;
					return -1;
				}
				escaped = 0;
				label = bp++;
				c = *src++;
				if (c == '\0')
					goto done;
				if (c != '.') {
					errno = EINVAL;
					return -1;
				}
				continue;
			}
			cp = strchr(digits, c);
			if (cp != NULL) {
				n = (cp - digits) * 100;
				c = *src++;
				if (c == '\0')
					goto ret_EMSGSIZE;
				cp = strchr(digits, c);
				if (cp == NULL)
					goto ret_EMSGSIZE;
				n += (cp - digits) * 10;
				c = *src++;
				if (c == '\0')
					goto ret_EMSGSIZE;
				cp = strchr(digits, c);
				if (cp == NULL)
					goto ret_EMSGSIZE;
				n += (cp - digits);
				if (n > 255)
					goto ret_EMSGSIZE;
				c = n;
			}
			escaped = 0;
		} else if (c == '\\') {
			escaped = 1;
			continue;
		} else if (c == '.') {
			c = (bp - label - 1);
			if ((c & NS_CMPRSFLGS) != 0) {  /*%< Label too big. */
				goto ret_EMSGSIZE;
			}
			if (label >= eom) {
				goto ret_EMSGSIZE;
			}
			*label = c;
			/* Fully qualified ? */
			if (*src == '\0') {
				if (c != 0) {
					if (bp >= eom) {
						goto ret_EMSGSIZE;
					}
					*bp++ = '\0';
				}
				if ((bp - dst) > MAXCDNAME) {
					goto ret_EMSGSIZE;
				}

				return 1;
			}
			if (c == 0 || *src == '.') {
				goto ret_EMSGSIZE;
			}
			label = bp++;
			continue;
		}
		if (bp >= eom) {
			goto ret_EMSGSIZE;
		}
		*bp++ = (u_char)c;
	}
	c = (bp - label - 1);
	if ((c & NS_CMPRSFLGS) != 0) {	  /*%< Label too big. */
		goto ret_EMSGSIZE;
	}
 done:
	if (label >= eom) {
		goto ret_EMSGSIZE;
	}
	*label = c;
	if (c != 0) {
		if (bp >= eom) {
			goto ret_EMSGSIZE;
		}
		*bp++ = 0;
	}
	if ((bp - dst) > MAXCDNAME) {   /*%< src too big */
		goto ret_EMSGSIZE;
	}

	return 0;

 ret_EMSGSIZE:
	errno = EMSGSIZE;
	return -1;
}
libc_hidden_def(ns_name_pton)

/*
 * ns_name_unpack(msg, eom, src, dst, dstsiz)
 *      Unpack a domain name from a message, source may be compressed.
 * return:
 *      -1 if it fails, or consumed octets if it succeeds.
 */
int ns_name_unpack(const u_char *msg, const u_char *eom, const u_char *src,
               u_char *dst, size_t dstsiz)
{
	const u_char *srcp, *dstlim;
	u_char *dstp;
	int n, len, checked;

	len = -1;
	checked = 0;
	dstp = dst;
	srcp = src;
	dstlim = dst + dstsiz;
	if (srcp < msg || srcp >= eom) {
		__set_errno(EMSGSIZE);
		return -1;
	}
	/* Fetch next label in domain name. */
	while ((n = *srcp++) != 0) {
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS) {
			case 0:
				/* Limit checks. */
				if (dstp + n + 1 >= dstlim || srcp + n >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				checked += n + 1;
				*dstp++ = n;
				memcpy(dstp, srcp, n);
				dstp += n;
				srcp += n;
				break;

			case NS_CMPRSFLGS:
				if (srcp >= eom) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				if (len < 0)
					len = srcp - src + 1;
				srcp = msg + (((n & 0x3f) << 8) | (*srcp & 0xff));
				if (srcp < msg || srcp >= eom) {  /* Out of range. */
					__set_errno(EMSGSIZE);
					return -1;
				}
				checked += 2;
				/*
				 * Check for loops in the compressed name;
				 * if we've looked at the whole message,
				 * there must be a loop.
				 */
				if (checked >= eom - msg) {
					__set_errno(EMSGSIZE);
					return -1;
				}
				break;

			default:
				__set_errno(EMSGSIZE);
				return -1;                    /* flag error */
		}
	}
	*dstp = '\0';
	if (len < 0)
		len = srcp - src;
	return len;
}
libc_hidden_def(ns_name_unpack)

static int labellen(const unsigned char *lp)
{
	unsigned bitlen;
	unsigned char l = *lp;

	if ((l & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
		/* should be avoided by the caller */
		return -1;
	}

	if ((l & NS_CMPRSFLGS) == NS_TYPE_ELT) {
		if (l == DNS_LABELTYPE_BITSTRING) {
			bitlen = lp[1];
			if (bitlen == 0)
				bitlen = 256;
			return ((bitlen + 7 ) / 8 + 1);
		}

		return -1;    /*%< unknwon ELT */
	}

	return l;
}

static int mklower(int ch)
{
	if (ch >= 0x41 && ch <= 0x5A)
		return (ch + 0x20);

	return ch;
}

static int dn_find(const unsigned char *domain,
				   const unsigned char *msg,
				   const unsigned char * const *dnptrs,
				   const unsigned char * const *lastdnptr)
{
	const unsigned char *dn, *cp, *sp;
	const unsigned char * const *cpp;
	u_int n;

	for (cpp = dnptrs; cpp < lastdnptr; cpp++) {
		sp = *cpp;
		/*
		 * terminate search on:
		 * root label
		 * compression pointer
		 * unusable offset
		 */
		while (*sp != 0 && (*sp & NS_CMPRSFLGS) == 0 &&
				(sp - msg) < 0x4000) {
			dn = domain;
			cp = sp;

			while ((n = *cp++) != 0) {
				/*
				 * check for indirection
				 */
				switch (n & NS_CMPRSFLGS) {
				case 0:	 /*%< normal case, n == len */
					n = labellen(cp - 1); /*%< XXX */
					if (n != *dn++)
						goto next;

					for (; n > 0; n--)
						if (mklower(*dn++) !=
						    mklower(*cp++))
							goto next;
					/* Is next root for both ? */
					if (*dn == '\0' && *cp == '\0')
						return (sp - msg);
					if (*dn)
						continue;
					goto next;
				case NS_CMPRSFLGS:      /*%< indirection */
					cp = msg + (((n & 0x3f) << 8) | *cp);
					break;

				default:	/*%< illegal type */
					errno = EMSGSIZE;
					return -1;
				}
			}
next:
			sp += *sp + 1;
		}
	}

	errno = ENOENT;
	return -1;
}

int ns_name_pack(const unsigned char *src,
				 unsigned char *dst, int dstsiz,
				 const unsigned char **dnptrs,
				 const unsigned char **lastdnptr)
{
	unsigned char *dstp;
	const unsigned char **cpp, **lpp, *eob, *msg;
	const unsigned char *srcp;
	int n, l, first = 1;

	srcp = src;
	dstp = dst;
	eob = dstp + dstsiz;
	lpp = cpp = NULL;

	if (dnptrs != NULL) {
		msg = *dnptrs++;
		if (msg != NULL) {
			for (cpp = dnptrs; *cpp != NULL; cpp++)
				continue;

			lpp = cpp;      /*%< end of list to search */
		}
	} else {
		msg = NULL;
	}

	/* make sure the domain we are about to add is legal */
	l = 0;
	do {
		int l0;

		n = *srcp;
		if ((n & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
			errno = EMSGSIZE;
			return -1;
		}

		l0 = labellen(srcp);
		if (l0 < 0) {
			errno = EINVAL;
			return -1;
		}

		l += l0 + 1;
		if (l > MAXCDNAME) {
			errno = EMSGSIZE;
			return -1;
		}

		srcp += l0 + 1;
	} while (n != 0);

	/* from here on we need to reset compression pointer array on error */
	srcp = src;

	do {
		/* Look to see if we can use pointers. */
		n = *srcp;

		if (n != 0 && msg != NULL) {
			l = dn_find(srcp, msg, (const unsigned char * const *) dnptrs,
						(const unsigned char * const *) lpp);
			if (l >= 0) {
				if (dstp + 1 >= eob) {
					goto cleanup;
				}

				*dstp++ = ((u_int32_t)l >> 8) | NS_CMPRSFLGS;
				*dstp++ = l % 256;
				return (dstp - dst);
			}

			/* Not found, save it. */
			if (lastdnptr != NULL && cpp < lastdnptr - 1 &&
				(dstp - msg) < 0x4000 && first) {
				*cpp++ = dstp;
				*cpp = NULL;
				first = 0;
			}
		}

		/* copy label to buffer */
		if ((n & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
			/* Should not happen. */
			goto cleanup;
		}

		n = labellen(srcp);
		if (dstp + 1 + n >= eob) {
			goto cleanup;
		}

		memcpy(dstp, srcp, (size_t)(n + 1));
		srcp += n + 1;
		dstp += n + 1;
	} while (n != 0);

	if (dstp > eob) {
cleanup:
		if (msg != NULL)
			*lpp = NULL;

			errno = EMSGSIZE;
			return -1;
	}

	return dstp - dst;
}
libc_hidden_def(ns_name_pack)

int ns_name_compress(const char *src,
					 unsigned char *dst, size_t dstsiz,
					 const unsigned char **dnptrs,
					 const unsigned char **lastdnptr)
{
	unsigned char tmp[NS_MAXCDNAME];

	if (ns_name_pton(src, tmp, sizeof(tmp)) == -1)
		return -1;

	return ns_name_pack(tmp, dst, dstsiz, dnptrs, lastdnptr);
}
libc_hidden_def(ns_name_compress)

int ns_name_skip(const unsigned char **ptrptr,
				 const unsigned char *eom)
{
	const unsigned char *cp;
	u_int n;
	int l;

	cp = *ptrptr;
	while (cp < eom && (n = *cp++) != 0) {
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS) {
		case 0:		 /*%< normal case, n == len */
			cp += n;
			continue;
		case NS_TYPE_ELT: /*%< EDNS0 extended label */
			l = labellen(cp - 1);
			if (l < 0) {
				errno = EMSGSIZE; /*%< XXX */
				return -1;
			}
			cp += l;
			continue;
		case NS_CMPRSFLGS:      /*%< indirection */
			cp++;
			break;
		default:		/*%< illegal type */
			errno = EMSGSIZE;
			return -1;
		}

		break;
	}

	if (cp > eom) {
		errno = EMSGSIZE;
		return -1;
	}

	*ptrptr = cp;

	return 0;
}
libc_hidden_def(ns_name_skip)

int dn_skipname(const unsigned char *ptr, const unsigned char *eom)
{
	const unsigned char *saveptr = ptr;

	if (ns_name_skip(&ptr, eom) == -1)
		return -1;

	return ptr - saveptr;
}
libc_hidden_def(dn_skipname)
#endif /* L_ns_name */


#ifdef L_res_init

/* Will be called under __resolv_lock. */
static void res_sync_func(void)
{
	struct __res_state *rp = &(_res);
	int n;

	/* If we didn't get malloc failure earlier... */
	if (__nameserver != (void*) &__local_nameserver) {
		/* TODO:
		 * if (__nameservers < rp->nscount) - try to grow __nameserver[]?
		 */
#ifdef __UCLIBC_HAS_IPV6__
		if (__nameservers > rp->_u._ext.nscount)
			__nameservers = rp->_u._ext.nscount;
		n = __nameservers;
		while (--n >= 0)
			__nameserver[n].sa6 = *rp->_u._ext.nsaddrs[n]; /* struct copy */
#else /* IPv4 only */
		if (__nameservers > rp->nscount)
			__nameservers = rp->nscount;
		n = __nameservers;
		while (--n >= 0)
			__nameserver[n].sa4 = rp->nsaddr_list[n]; /* struct copy */
#endif
	}
	__resolv_timeout = rp->retrans ? : RES_TIMEOUT;
	__resolv_attempts = rp->retry ? : RES_DFLRETRY;
	/* Extend and comment what program is known
	 * to use which _res.XXX member(s).

	   __resolv_opts = rp->options;
	   ...
	 */
}

/* has to be called under __resolv_lock */
static int
__res_vinit(res_state rp, int preinit)
{
	int i, n, options, retrans, retry, ndots;
#ifdef __UCLIBC_HAS_IPV6__
	int m = 0;
#endif

	__close_nameservers();
	__open_nameservers();

	if (preinit) {
		options = rp->options;
		retrans = rp->retrans;
		retry = rp->retry;
		ndots = rp->ndots;
	}

	memset(rp, 0, sizeof(*rp));

	if (!preinit) {
		rp->options = RES_DEFAULT;
		rp->retrans = RES_TIMEOUT;
		rp->retry = RES_DFLRETRY;
		rp->ndots = 1;
	} else {
		rp->options = options;
		rp->retrans = retrans;
		rp->retry = retry;
		rp->ndots = ndots;
	}

#ifdef __UCLIBC_HAS_COMPAT_RES_STATE__
	/* Was: "rp->id = random();" but:
	 * - random() pulls in largish static buffers
	 * - isn't actually random unless, say, srandom(time(NULL)) was called
	 * - is not used by uclibc anyway :)
	 */
	/* rp->id = 0; - memset did it */
#endif
#ifdef __UCLIBC_HAS_EXTRA_COMPAT_RES_STATE__
	rp->_vcsock = -1;
#endif

	n = __searchdomains;
	if (n > ARRAY_SIZE(rp->dnsrch))
		n = ARRAY_SIZE(rp->dnsrch);
	for (i = 0; i < n; i++)
		rp->dnsrch[i] = __searchdomain[i];

	/* copy nameservers' addresses */
	i = 0;
#ifdef __UCLIBC_HAS_IPV4__
	n = 0;
	while (n < ARRAY_SIZE(rp->nsaddr_list) && i < __nameservers) {
		if (__nameserver[i].sa.sa_family == AF_INET) {
			rp->nsaddr_list[n] = __nameserver[i].sa4; /* struct copy */
#ifdef __UCLIBC_HAS_IPV6__
			if (m < ARRAY_SIZE(rp->_u._ext.nsaddrs)) {
				rp->_u._ext.nsaddrs[m] = (void*) &rp->nsaddr_list[n];
				m++;
			}
#endif
			n++;
		}
#ifdef __UCLIBC_HAS_IPV6__
		if (__nameserver[i].sa.sa_family == AF_INET6
		 && m < ARRAY_SIZE(rp->_u._ext.nsaddrs)
		) {
			struct sockaddr_in6 *sa6 = malloc(sizeof(*sa6));
			if (sa6) {
				*sa6 = __nameserver[i].sa6; /* struct copy */
				rp->_u._ext.nsaddrs[m] = sa6;
				m++;
			}
		}
#endif
		i++;
	}
	rp->nscount = n;
#ifdef __UCLIBC_HAS_IPV6__
	rp->_u._ext.nscount = m;
#endif

#else /* IPv6 only */
	while (m < ARRAY_SIZE(rp->_u._ext.nsaddrs) && i < __nameservers) {
		struct sockaddr_in6 *sa6 = malloc(sizeof(*sa6));
		if (sa6) {
			*sa6 = __nameserver[i].sa6; /* struct copy */
			rp->_u._ext.nsaddrs[m] = sa6;
			m++;
		}
		i++;
	}
	rp->_u._ext.nscount = m;
#endif

	rp->options |= RES_INIT;

	return 0;
}

static unsigned int
res_randomid(void)
{
	return 0xffff & getpid();
}

/* Our res_init never fails (always returns 0) */
int
res_init(void)
{
	/*
	 * These three fields used to be statically initialized.  This made
	 * it hard to use this code in a shared library.  It is necessary,
	 * now that we're doing dynamic initialization here, that we preserve
	 * the old semantics: if an application modifies one of these three
	 * fields of _res before res_init() is called, res_init() will not
	 * alter them.  Of course, if an application is setting them to
	 * _zero_ before calling res_init(), hoping to override what used
	 * to be the static default, we can't detect it and unexpected results
	 * will follow.  Zero for any of these fields would make no sense,
	 * so one can safely assume that the applications were already getting
	 * unexpected results.
	 *
	 * _res.options is tricky since some apps were known to diddle the bits
	 * before res_init() was first called. We can't replicate that semantic
	 * with dynamic initialization (they may have turned bits off that are
	 * set in RES_DEFAULT).  Our solution is to declare such applications
	 * "broken".  They could fool us by setting RES_INIT but none do (yet).
	 */

	__UCLIBC_MUTEX_LOCK(__resolv_lock);

	if (!_res.retrans)
		_res.retrans = RES_TIMEOUT;
	if (!_res.retry)
		_res.retry = 4;
	if (!(_res.options & RES_INIT))
		_res.options = RES_DEFAULT;

	/*
	 * This one used to initialize implicitly to zero, so unless the app
	 * has set it to something in particular, we can randomize it now.
	 */
	if (!_res.id)
		_res.id = res_randomid();

	__res_sync = NULL;
	__res_vinit(&_res, 1);
	__res_sync = res_sync_func;

	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);

	return 0;
}
libc_hidden_def(res_init)

static void
__res_iclose(res_state statp)
{
	struct __res_state * rp = statp;
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	if (rp == NULL)
		rp = __res_state();
	__close_nameservers();
	__res_sync = NULL;
#ifdef __UCLIBC_HAS_IPV6__
	{
		char *p1 = (char*) &(rp->nsaddr_list[0]);
		unsigned int m = 0;
		/* free nsaddrs[m] if they do not point to nsaddr_list[x] */
		while (m < ARRAY_SIZE(rp->_u._ext.nsaddrs)) {
			char *p2 = (char*)(rp->_u._ext.nsaddrs[m++]);
			if (p2 < p1 || (p2 - p1) > (signed)sizeof(rp->nsaddr_list))
				free(p2);
		}
	}
#endif
	memset(rp, 0, sizeof(struct __res_state));
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
}

/*
 * This routine is for closing the socket if a virtual circuit is used and
 * the program wants to close it.  This provides support for endhostent()
 * which expects to close the socket.
 *
 * This routine is not expected to be user visible.
 */

void
res_nclose(res_state statp)
{
	__res_iclose(statp);
}

#ifdef __UCLIBC_HAS_BSD_RES_CLOSE__
void res_close(void)
{
	__res_iclose(NULL);
}
#endif

/* This needs to be after the use of _res in res_init, above.  */
#undef _res

#ifndef __UCLIBC_HAS_THREADS__
/* The resolver state for use by single-threaded programs.
   This differs from plain `struct __res_state _res;' in that it doesn't
   create a common definition, but a plain symbol that resides in .bss,
   which can have an alias.  */
struct __res_state _res __attribute__((section (".bss")));
struct __res_state *__resp = &_res;
#else /* __UCLIBC_HAS_THREADS__ */
struct __res_state _res __attribute__((section (".bss"))) attribute_hidden;

# if defined __UCLIBC_HAS_TLS__
#  undef __resp
__thread struct __res_state *__resp = &_res;
/*
 * FIXME: Add usage of hidden attribute for this when used in the shared
 *        library. It currently crashes the linker when doing section
 *        relocations.
 */
extern __thread struct __res_state *__libc_resp
       __attribute__ ((alias ("__resp"))) attribute_hidden;
# else
#  undef __resp
struct __res_state *__resp = &_res;
# endif
#endif /* !__UCLIBC_HAS_THREADS__ */

/*
 * Set up default settings.  If the configuration file exist, the values
 * there will have precedence.  Otherwise, the server address is set to
 * INADDR_ANY and the default domain name comes from the gethostname().
 *
 * An interrim version of this code (BIND 4.9, pre-4.4BSD) used 127.0.0.1
 * rather than INADDR_ANY ("0.0.0.0") as the default name server address
 * since it was noted that INADDR_ANY actually meant ``the first interface
 * you "ifconfig"'d at boot time'' and if this was a SLIP or PPP interface,
 * it had to be "up" in order for you to reach your own name server.  It
 * was later decided that since the recommended practice is to always
 * install local static routes through 127.0.0.1 for all your network
 * interfaces, that we could solve this problem without a code change.
 *
 * The configuration file should always be used, since it is the only way
 * to specify a default domain.  If you are running a server on your local
 * machine, you should say "nameserver 0.0.0.0" or "nameserver 127.0.0.1"
 * in the configuration file.
 *
 * Return 0 if completes successfully, -1 on error
 */
int
res_ninit(res_state statp)
{
	int ret;
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	ret = __res_vinit(statp, 0);
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
	return ret;
}

#endif /* L_res_init */

#ifdef L_res_state
# if defined __UCLIBC_HAS_TLS__
struct __res_state *
__res_state (void)
{
       return __resp;
}
# else
#  undef _res
extern struct __res_state _res;

/* When threaded, _res may be a per-thread variable.  */
struct __res_state *
weak_const_function
__res_state (void)
{
       return &_res;
}
# endif

#endif /* L_res_state */


#ifdef L_res_query

int res_query(const char *dname, int class, int type,
              unsigned char *answer, int anslen)
{
	int i;
	unsigned char *packet = NULL;
	struct resolv_answer a;

	if (!dname || class != 1 /* CLASS_IN */) {
		h_errno = NO_RECOVERY;
		return -1;
	}

	memset(&a, '\0', sizeof(a));
	i = __dns_lookup(dname, type, &packet, &a);

	if (i < 0) {
		if (!h_errno) /* TODO: can this ever happen? */
			h_errno = TRY_AGAIN;
		return -1;
	}

	free(a.dotted);

	i = MIN(anslen, i);
	memcpy(answer, packet, i);

	free(packet);
	return i;
}
libc_hidden_def(res_query)

/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error code, if any, is left in h_errno.
 */
#define __TRAILING_DOT	(1<<0)
#define __GOT_NODATA	(1<<1)
#define __GOT_SERVFAIL	(1<<2)
#define __TRIED_AS_IS	(1<<3)
int res_search(const char *name, int class, int type, u_char *answer,
		int anslen)
{
	const char *cp;
	char **domain;
	HEADER *hp = (HEADER *)(void *)answer;
	unsigned dots;
	unsigned state;
	int ret, saved_herrno;
	uint32_t _res_options;
	unsigned _res_ndots;
	char **_res_dnsrch;

	if (!name || !answer) {
		h_errno = NETDB_INTERNAL;
		return -1;
	}

 again:
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	_res_options = _res.options;
	_res_ndots = _res.ndots;
	_res_dnsrch = _res.dnsrch;
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
	if (!(_res_options & RES_INIT)) {
		res_init(); /* our res_init never fails */
		goto again;
	}

	state = 0;
	errno = 0;
	h_errno = HOST_NOT_FOUND;	/* default, if we never query */
	dots = 0;
	for (cp = name; *cp; cp++)
		dots += (*cp == '.');

	if (cp > name && *--cp == '.')
		state |= __TRAILING_DOT;

	/*
	 * If there are dots in the name already, let's just give it a try
	 * 'as is'.  The threshold can be set with the "ndots" option.
	 */
	saved_herrno = -1;
	if (dots >= _res_ndots) {
		ret = res_querydomain(name, NULL, class, type, answer, anslen);
		if (ret > 0)
			return ret;
		saved_herrno = h_errno;
		state |= __TRIED_AS_IS;
	}

	/*
	 * We do at least one level of search if
	 *	- there is no dot and RES_DEFNAME is set, or
	 *	- there is at least one dot, there is no trailing dot,
	 *	  and RES_DNSRCH is set.
	 */
	if ((!dots && (_res_options & RES_DEFNAMES))
	 || (dots && !(state & __TRAILING_DOT) && (_res_options & RES_DNSRCH))
	) {
		bool done = 0;

		for (domain = _res_dnsrch; *domain && !done; domain++) {

			ret = res_querydomain(name, *domain, class, type,
								  answer, anslen);
			if (ret > 0)
				return ret;

			/*
			 * If no server present, give up.
			 * If name isn't found in this domain,
			 * keep trying higher domains in the search list
			 * (if that's enabled).
			 * On a NO_DATA error, keep trying, otherwise
			 * a wildcard entry of another type could keep us
			 * from finding this entry higher in the domain.
			 * If we get some other error (negative answer or
			 * server failure), then stop searching up,
			 * but try the input name below in case it's
			 * fully-qualified.
			 */
			if (errno == ECONNREFUSED) {
				h_errno = TRY_AGAIN;
				return -1;
			}

			switch (h_errno) {
				case NO_DATA:
					state |= __GOT_NODATA;
					/* FALLTHROUGH */
				case HOST_NOT_FOUND:
					/* keep trying */
					break;
				case TRY_AGAIN:
					if (hp->rcode == SERVFAIL) {
						/* try next search element, if any */
						state |= __GOT_SERVFAIL;
						break;
					}
					/* FALLTHROUGH */
				default:
					/* anything else implies that we're done */
					done = 1;
			}
			/*
			 * if we got here for some reason other than DNSRCH,
			 * we only wanted one iteration of the loop, so stop.
			 */
			if (!(_res_options & RES_DNSRCH))
				done = 1;
		}
	}

	/*
	 * if we have not already tried the name "as is", do that now.
	 * note that we do this regardless of how many dots were in the
	 * name or whether it ends with a dot.
	 */
	if (!(state & __TRIED_AS_IS)) {
		ret = res_querydomain(name, NULL, class, type, answer, anslen);
		if (ret > 0)
			return ret;
	}

	/*
	 * if we got here, we didn't satisfy the search.
	 * if we did an initial full query, return that query's h_errno
	 * (note that we wouldn't be here if that query had succeeded).
	 * else if we ever got a nodata, send that back as the reason.
	 * else send back meaningless h_errno, that being the one from
	 * the last DNSRCH we did.
	 */
	if (saved_herrno != -1)
		h_errno = saved_herrno;
	else if (state & __GOT_NODATA)
		h_errno = NO_DATA;
	else if (state & __GOT_SERVFAIL)
		h_errno = TRY_AGAIN;
	return -1;
}
#undef __TRAILING_DOT
#undef __GOT_NODATA
#undef __GOT_SERVFAIL
#undef __TRIED_AS_IS
/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int res_querydomain(const char *name, const char *domain, int class, int type,
			u_char *answer, int anslen)
{
	char nbuf[MAXDNAME];
	const char *longname = nbuf;
	size_t n, d;
#ifdef DEBUG
	uint32_t _res_options;
#endif

	if (!name || !answer) {
		h_errno = NETDB_INTERNAL;
		return -1;
	}

#ifdef DEBUG
 again:
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	_res_options = _res.options;
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
	if (!(_res_options & RES_INIT)) {
		res_init(); /* our res_init never fails */
		goto again;
	}
	if (_res_options & RES_DEBUG)
		printf(";; res_querydomain(%s, %s, %d, %d)\n",
			   name, (domain ? domain : "<Nil>"), class, type);
#endif
	if (domain == NULL) {
		/*
		 * Check for trailing '.';
		 * copy without '.' if present.
		 */
		n = strlen(name);
		if (n + 1 > sizeof(nbuf)) {
			h_errno = NO_RECOVERY;
			return -1;
		}
		if (n > 0 && name[--n] == '.') {
			strncpy(nbuf, name, n);
			nbuf[n] = '\0';
		} else
			longname = name;
	} else {
		n = strlen(name);
		d = strlen(domain);
		if (n + 1 + d + 1 > sizeof(nbuf)) {
			h_errno = NO_RECOVERY;
			return -1;
		}
		snprintf(nbuf, sizeof(nbuf), "%s.%s", name, domain);
	}
	return res_query(longname, class, type, answer, anslen);
}
libc_hidden_def(res_querydomain)
#endif /* L_res_query */

#ifdef L_ns_netint
unsigned int ns_get16(const unsigned char *src)
{
	unsigned int dst;
	NS_GET16(dst, src);
	return dst;
}

unsigned long ns_get32(const unsigned char *src)
{
	unsigned long dst;
	NS_GET32(dst, src);
	return dst;
}

void ns_put16(unsigned int src, unsigned char *dst)
{
	NS_PUT16(src, dst);
}

void ns_put32(unsigned long src, unsigned char *dst)
{
	NS_PUT32(src, dst);
}
#endif /* L_ns_netint */

#ifdef L_ns_parse
/* These need to be in the same order as the nres.h:ns_flag enum. */
struct _ns_flagdata { unsigned short mask, shift; };
static const struct _ns_flagdata _ns_flagdata[16] = {
	{ 0x8000, 15 },         /*%< qr. */
	{ 0x7800, 11 },         /*%< opcode. */
	{ 0x0400, 10 },         /*%< aa. */
	{ 0x0200, 9 },          /*%< tc. */
	{ 0x0100, 8 },          /*%< rd. */
	{ 0x0080, 7 },          /*%< ra. */
	{ 0x0040, 6 },          /*%< z. */
	{ 0x0020, 5 },          /*%< ad. */
	{ 0x0010, 4 },          /*%< cd. */
	{ 0x000f, 0 },          /*%< rcode. */
	{ 0x0000, 0 },          /*%< expansion (1/6). */
	{ 0x0000, 0 },          /*%< expansion (2/6). */
	{ 0x0000, 0 },          /*%< expansion (3/6). */
	{ 0x0000, 0 },          /*%< expansion (4/6). */
	{ 0x0000, 0 },          /*%< expansion (5/6). */
	{ 0x0000, 0 },          /*%< expansion (6/6). */
};

static void setsection(ns_msg *msg, ns_sect sect)
{
	msg->_sect = sect;
	if (sect == ns_s_max) {
		msg->_rrnum = -1;
		msg->_ptr = NULL;
	} else {
		msg->_rrnum = 0;
		msg->_ptr = msg->_sections[(int)sect];
	}
}

int ns_skiprr(const unsigned char *ptr,
			  const unsigned char *eom,
			  ns_sect section, int count)
{
	const u_char *optr = ptr;

	for (; count > 0; count--) {
		int b, rdlength;

		b = dn_skipname(ptr, eom);
		if (b < 0) {
			errno = EMSGSIZE;
			return -1;
		}

		ptr += b/*Name*/ + NS_INT16SZ/*Type*/ + NS_INT16SZ/*Class*/;
		if (section != ns_s_qd) {
			if (ptr + NS_INT32SZ + NS_INT16SZ > eom) {
				errno = EMSGSIZE;
				return -1;
			}

			ptr += NS_INT32SZ/*TTL*/;
			NS_GET16(rdlength, ptr);
			ptr += rdlength/*RData*/;
		}
	}

	if (ptr > eom) {
		errno = EMSGSIZE;
		return -1;
	}

	return ptr - optr;
}
libc_hidden_def(ns_skiprr)

int
ns_initparse(const unsigned char *msg, int msglen, ns_msg *handle)
{
	const u_char *eom = msg + msglen;
	int i;

	handle->_msg = msg;
	handle->_eom = eom;
	if (msg + NS_INT16SZ > eom) {
		errno = EMSGSIZE;
		return -1;
	}

	NS_GET16(handle->_id, msg);
	if (msg + NS_INT16SZ > eom) {
		errno = EMSGSIZE;
		return -1;
	}

	NS_GET16(handle->_flags, msg);
	for (i = 0; i < ns_s_max; i++) {
		if (msg + NS_INT16SZ > eom) {
			errno = EMSGSIZE;
			return -1;
		}

		NS_GET16(handle->_counts[i], msg);
	}
	for (i = 0; i < ns_s_max; i++)
		if (handle->_counts[i] == 0)
			handle->_sections[i] = NULL;
		else {
			int b = ns_skiprr(msg, eom, (ns_sect)i,
					  handle->_counts[i]);

			if (b < 0)
				return -1;
			handle->_sections[i] = msg;
			msg += b;
		}

	if (msg != eom) {
		errno = EMSGSIZE;
		return -1;
	}

	setsection(handle, ns_s_max);
	return 0;
}

int
ns_parserr(ns_msg *handle, ns_sect section, int rrnum, ns_rr *rr)
{
	int b;
	int tmp;

	/* Make section right. */
	tmp = section;
	if (tmp < 0 || section >= ns_s_max) {
		errno = ENODEV;
		return -1;
	}

	if (section != handle->_sect)
		setsection(handle, section);

	/* Make rrnum right. */
	if (rrnum == -1)
		rrnum = handle->_rrnum;
	if (rrnum < 0 || rrnum >= handle->_counts[(int)section]) {
		errno = ENODEV;
		return -1;
	}
	if (rrnum < handle->_rrnum)
		setsection(handle, section);
	if (rrnum > handle->_rrnum) {
		b = ns_skiprr(handle->_ptr, handle->_eom, section,
			      rrnum - handle->_rrnum);

		if (b < 0)
			return -1;
		handle->_ptr += b;
		handle->_rrnum = rrnum;
	}

	/* Do the parse. */
	b = dn_expand(handle->_msg, handle->_eom,
		      handle->_ptr, rr->name, NS_MAXDNAME);
	if (b < 0)
		return -1;
	handle->_ptr += b;
	if (handle->_ptr + NS_INT16SZ + NS_INT16SZ > handle->_eom) {
		errno = EMSGSIZE;
		return -1;
	}
	NS_GET16(rr->type, handle->_ptr);
	NS_GET16(rr->rr_class, handle->_ptr);
	if (section == ns_s_qd) {
		rr->ttl = 0;
		rr->rdlength = 0;
		rr->rdata = NULL;
	} else {
		if (handle->_ptr + NS_INT32SZ + NS_INT16SZ > handle->_eom) {
			errno = EMSGSIZE;
			return -1;
		}
		NS_GET32(rr->ttl, handle->_ptr);
		NS_GET16(rr->rdlength, handle->_ptr);
		if (handle->_ptr + rr->rdlength > handle->_eom) {
			errno = EMSGSIZE;
			return -1;
		}
		rr->rdata = handle->_ptr;
		handle->_ptr += rr->rdlength;
	}
	if (++handle->_rrnum > handle->_counts[(int)section])
		setsection(handle, (ns_sect)((int)section + 1));

	return 0;
}

int ns_msg_getflag(ns_msg handle, int flag)
{
	return ((handle)._flags & _ns_flagdata[flag].mask) >> _ns_flagdata[flag].shift;
}
#endif /* L_ns_parse */

#ifdef L_res_data
int res_mkquery(int op, const char *dname, int class, int type,
				const unsigned char *data, int datalen,
				const unsigned char *newrr_in,
				unsigned char *buf, int buflen)
{
	HEADER *hp;
	unsigned char *cp, *ep;
	unsigned char *dnptrs[20], **dpp, **lastdnptr;
	uint32_t _res_options;
	int n;

	if (!buf || buflen < HFIXEDSZ) {
		h_errno = NETDB_INTERNAL;
		return -1;
	}

 again:
	__UCLIBC_MUTEX_LOCK(__resolv_lock);
	_res_options = _res.options;
	__UCLIBC_MUTEX_UNLOCK(__resolv_lock);
	if (!(_res_options & RES_INIT)) {
		res_init(); /* our res_init never fails */
		goto again;
	}

#ifdef DEBUG
	if (_res_options & RES_DEBUG)
		printf(";; res_mkquery(%d, %s, %d, %d)\n",
			   op, dname && *dname ? dname : "<null>", class, type);
#endif

	memset(buf, 0, HFIXEDSZ);
	hp = (HEADER *) buf;
	hp->id = getpid() & 0xffff;
	hp->opcode = op;
	hp->rd = (_res_options & RES_RECURSE) != 0U;
	hp->rcode = NOERROR;

	cp = buf + HFIXEDSZ;
	ep = buf + buflen;
	dpp = dnptrs;
	*dpp++ = buf;
	*dpp++ = NULL;
	lastdnptr = dnptrs + sizeof dnptrs / sizeof dnptrs[0];

	/*
	 * perform opcode specific processing
	 */
	switch (op) {
	case QUERY:
	case NS_NOTIFY_OP:
		if (ep - cp < QFIXEDSZ)
			return -1;

		n = dn_comp(dname, cp, ep - cp - QFIXEDSZ, dnptrs, lastdnptr);
		if (n < 0)
			return -1;

		cp += n;
		NS_PUT16(type, cp);
		NS_PUT16(class, cp);
		hp->qdcount = htons(1);

		if (op == QUERY || data == NULL)
			break;

		/*
		 * Make an additional record for completion domain.
		 */
		if ((ep - cp) < RRFIXEDSZ)
			return -1;

		n = dn_comp((const char *)data, cp, ep - cp - RRFIXEDSZ,
					 dnptrs, lastdnptr);
		if (n < 0)
			return -1;

		cp += n;
		NS_PUT16(T_NULL, cp);
		NS_PUT16(class, cp);
		NS_PUT32(0, cp);
		NS_PUT16(0, cp);
		hp->arcount = htons(1);

		break;

	case IQUERY:
		/*
		 * Initialize answer section
		 */
		if (ep - cp < 1 + RRFIXEDSZ + datalen)
			return -1;

		*cp++ = '\0';   /*%< no domain name */
		NS_PUT16(type, cp);
		NS_PUT16(class, cp);
		NS_PUT32(0, cp);
		NS_PUT16(datalen, cp);

		if (datalen) {
			memcpy(cp, data, (size_t)datalen);
			cp += datalen;
		}

		hp->ancount = htons(1);
		break;

	default:
		return -1;
	}

	return cp - buf;
}
#endif /* L_res_data */

/* Unimplemented: */
/* res_send */
