#ifndef CHINADNS_NG_NETUTILS_H
#define CHINADNS_NG_NETUTILS_H

#define _GNU_SOURCE
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#undef _GNU_SOURCE

/* ipset setname max len (including '\0') */
#define IPSET_MAXNAMELEN 32

/* ipv4 binary addr typedef */
typedef struct __attribute__((packed)) {
    uint32_t addr;
} inet4_ipaddr_t;

/* ipv6 binary addr typedef */
typedef struct __attribute__((packed)) {
    uint8_t addr[16];
} inet6_ipaddr_t;

/* uniform struct sockaddr_* name */
typedef struct sockaddr_in  inet4_skaddr_t;
typedef struct sockaddr_in6 inet6_skaddr_t;

/* socket port number typedef */
typedef uint16_t sock_port_t;

/* create a udp socket (AF_INET) */
int new_udp4_socket(void);

/* create a udp socket (AF_INET6) */
int new_udp6_socket(void);

/* setsockopt(IPV6_V6ONLY) */
void set_ipv6_only(int sockfd);

/* setsockopt(SO_REUSEADDR) */
void set_reuse_addr(int sockfd);

/* setsockopt(SO_REUSEPORT) */
void set_reuse_port(int sockfd);

/* create a timer fd (in seconds) */
int new_once_timerfd(time_t second);

/* AF_INET or AF_INET6 or -1(invalid) */
int get_addrstr_family(const char *addrstr);

/* build ipv4/ipv6 address structure */
void build_ipv4_addr(inet4_skaddr_t *addr, const char *host, sock_port_t port);
void build_ipv6_addr(inet6_skaddr_t *addr, const char *host, sock_port_t port);

/* parse ipv4/ipv6 address structure */
void parse_ipv4_addr(const inet4_skaddr_t *addr, char *host, sock_port_t *port);
void parse_ipv6_addr(const inet6_skaddr_t *addr, char *host, sock_port_t *port);

/* init netlink socket for ipset query */
void ipset_init_nlsocket(void);

/* check given ipaddr is exists in ipset */
bool ipset_addr4_is_exists(const inet4_ipaddr_t *addr_ptr);
bool ipset_addr6_is_exists(const inet6_ipaddr_t *addr_ptr);

#endif
