#ifndef IPT2SOCKS_NETUTILS_H
#define IPT2SOCKS_NETUTILS_H

#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <uv.h>
#undef _GNU_SOURCE

/* udp related bufsize */
#define UDP_MSGCTL_BUFSIZE 64
#define UDP_PACKET_MAXSIZE 1472

/* ip4/ip6 addr string */
#define IP4STR_LOOPBACK "127.0.0.1"
#define IP4STR_WILDCARD "0.0.0.0"
#define IP6STR_LOOPBACK "::1"
#define IP6STR_WILDCARD "::"

/* ipaddr binary len */
#define IP4BINLEN 4
#define IP6BINLEN 16

/* ipaddr string len */
#define IP4STRLEN INET_ADDRSTRLEN
#define IP6STRLEN INET6_ADDRSTRLEN

/* portno string len */
#define PORTSTRLEN 6

/* ip address typedef */
typedef uint32_t ipaddr4_t;
typedef uint8_t  ipaddr6_t[16];

/* ip4/ip6 union typedef */
typedef union {
    ipaddr4_t ip4;
    ipaddr6_t ip6;
} ipaddr_t;

/* port number typedef */
typedef uint16_t portno_t;

/* ipaddr+portno typedef */
typedef struct {
    ipaddr_t ip;
    portno_t port;
} ip_port_t;

/* sockaddr type alias */
typedef struct sockaddr     skaddr_t;
typedef struct sockaddr_in  skaddr4_t;
typedef struct sockaddr_in6 skaddr6_t;

/* setsockopt(SO_KEEPALIVE) */
void set_keepalive(int sockfd);

/* setsockopt(IPV6_V6ONLY) */
void set_ipv6_only(int sockfd);

/* setsockopt(SO_REUSEADDR) */
void set_reuse_addr(int sockfd);

/* setsockopt(SO_REUSEPORT) */
void set_reuse_port(int sockfd);

/* setsockopt(IP_TRANSPARENT) */
void set_transparent(int sockfd);

/* setsockopt(IP_RECVORIGDSTADDR) */
void set_recv_origdstaddr4(int sockfd);

/* setsockopt(IPV6_RECVORIGDSTADDR) */
void set_recv_origdstaddr6(int sockfd);

/* getsockopt(SO_ORIGINAL_DST) REDIRECT ipv4 */
bool get_tcp_origdstaddr4(int sockfd, skaddr4_t *dstaddr);

/* getsockopt(IP6T_SO_ORIGINAL_DST) REDIRECT ipv6 */
bool get_tcp_origdstaddr6(int sockfd, skaddr6_t *dstaddr);

/* get the original dest addr of the ipv4 udp packet */
bool get_udp_origdstaddr4(struct msghdr *msg, skaddr4_t *dstaddr);

/* get the original dest addr of the ipv6 udp packet */
bool get_udp_origdstaddr6(struct msghdr *msg, skaddr6_t *dstaddr);

/* create non-blocking tcp socket (ipv4) */
int new_tcp4_socket(void);

/* create non-blocking tcp socket (ipv6) */
int new_tcp6_socket(void);

/* create non-blocking udp socket (ipv4) */
int new_udp4_socket(void);

/* create non-blocking udp socket (ipv6) */
int new_udp6_socket(void);

/* create tcp socket use to listen (ipv4) */
int new_tcp4_bindsock(void);

/* create tcp socket use to listen (ipv6) */
int new_tcp6_bindsock(void);

/* create tcp socket use to tproxy-listen (ipv4) */
int new_tcp4_bindsock_tproxy(void);

/* create tcp socket use to tproxy-listen (ipv6) */
int new_tcp6_bindsock_tproxy(void);

/* create udp socket use to tproxy-reply (ipv4) */
int new_udp4_respsock_tproxy(void);

/* create udp socket use to tproxy-reply (ipv6) */
int new_udp6_respsock_tproxy(void);

/* create udp socket use to tproxy-listen (ipv4) */
int new_udp4_bindsock_tproxy(void);

/* create udp socket use to tproxy-listen (ipv6) */
int new_udp6_bindsock_tproxy(void);

/* build ipv4 socket address from ipstr and portno */
void build_ipv4_addr(skaddr4_t *addr, const char *ipstr, portno_t portno);

/* build ipv6 socket address from ipstr and portno */
void build_ipv6_addr(skaddr6_t *addr, const char *ipstr, portno_t portno);

/* parse ipstr and portno from ipv4 socket address */
void parse_ipv4_addr(const skaddr4_t *addr, char *ipstr, portno_t *portno);

/* parse ipstr and portno from ipv6 socket address */
void parse_ipv6_addr(const skaddr6_t *addr, char *ipstr, portno_t *portno);

/* AF_INET or AF_INET6 or -1(invalid ip string) */
int get_ipstr_family(const char *ipstr);

/* set nofile limit (may require root privileges) */
void set_nofile_limit(rlim_t nofile);

/* run the current process with a given user */
void run_as_user(const char *username, char *const argv[]);

/* strerror thread safe version (libuv) */
#define errstring(errnum) uv_strerror(-(errnum))

#endif
