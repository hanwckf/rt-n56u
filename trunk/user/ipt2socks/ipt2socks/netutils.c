#define _GNU_SOURCE
#include "netutils.h"
#include "logutils.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <linux/limits.h>
#include <pwd.h>
#include <grp.h>
#undef _GNU_SOURCE

#ifndef SO_ORIGINAL_DST
#define SO_ORIGINAL_DST 80
#endif

#ifndef IP6T_SO_ORIGINAL_DST
#define IP6T_SO_ORIGINAL_DST 80
#endif

#ifndef IP_RECVORIGDSTADDR
#define IP_RECVORIGDSTADDR 20
#endif

#ifndef IPV6_RECVORIGDSTADDR
#define IPV6_RECVORIGDSTADDR 74
#endif

#define KEEPALIVE_CONN_IDLE_SEC 15
#define KEEPALIVE_RETRY_MAX_COUNT 5
#define KEEPALIVE_RETRY_INTERVAL_SEC 1

/* suppress the warning of openwrt */
int initgroups(const char *user, gid_t group);

/* setsockopt(SO_KEEPALIVE) */
void set_keepalive(int sockfd) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int))) {
        LOGERR("[set_keepalive] setsockopt(%d, SO_KEEPALIVE): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &(int){KEEPALIVE_CONN_IDLE_SEC}, sizeof(int))) {
        LOGERR("[set_keepalive] setsockopt(%d, TCP_KEEPIDLE): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &(int){KEEPALIVE_RETRY_MAX_COUNT}, sizeof(int))) {
        LOGERR("[set_keepalive] setsockopt(%d, TCP_KEEPCNT): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &(int){KEEPALIVE_RETRY_INTERVAL_SEC}, sizeof(int))) {
        LOGERR("[set_keepalive] setsockopt(%d, TCP_KEEPINTVL): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
}

/* setsockopt(IPV6_V6ONLY) */
void set_ipv6_only(int sockfd) {
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &(int){1}, sizeof(int))) {
        LOGERR("[set_ipv6_only] setsockopt(%d, IPV6_V6ONLY): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
}

/* setsockopt(SO_REUSEADDR) */
void set_reuse_addr(int sockfd) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) {
        LOGERR("[set_reuse_addr] setsockopt(%d, SO_REUSEADDR): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
}

/* setsockopt(SO_REUSEPORT) */
void set_reuse_port(int sockfd) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int))) {
        LOGERR("[set_reuse_port] setsockopt(%d, SO_REUSEPORT): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
}

/* setsockopt(IP_TRANSPARENT) */
void set_transparent(int sockfd) {
    if (setsockopt(sockfd, SOL_IP, IP_TRANSPARENT, &(int){1}, sizeof(int))) {
        LOGERR("[set_transparent] setsockopt(%d, IP_TRANSPARENT): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
}

/* setsockopt(IP_RECVORIGDSTADDR) */
void set_recv_origdstaddr4(int sockfd) {
    if (setsockopt(sockfd, SOL_IP, IP_RECVORIGDSTADDR, &(int){1}, sizeof(int))) {
        LOGERR("[set_recv_origdstaddr4] setsockopt(%d, IP_RECVORIGDSTADDR): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
}

/* setsockopt(IPV6_RECVORIGDSTADDR) */
void set_recv_origdstaddr6(int sockfd) {
    if (setsockopt(sockfd, SOL_IPV6, IPV6_RECVORIGDSTADDR, &(int){1}, sizeof(int))) {
        LOGERR("[set_recv_origdstaddr6] setsockopt(%d, IPV6_RECVORIGDSTADDR): (%d) %s", sockfd, errno, errstring(errno));
        exit(errno);
    }
}

/* getsockopt(SO_ORIGINAL_DST) REDIRECT ipv4 */
bool get_tcp_origdstaddr4(int sockfd, skaddr4_t *dstaddr) {
    if (getsockopt(sockfd, SOL_IP, SO_ORIGINAL_DST, dstaddr, &(socklen_t){sizeof(skaddr4_t)})) {
        LOGERR("[get_tcp_origdstaddr4] getsockopt(%d, SO_ORIGINAL_DST): (%d) %s", sockfd, errno, errstring(errno));
        return false;
    }
    return true;
}

/* getsockopt(IP6T_SO_ORIGINAL_DST) REDIRECT ipv6 */
bool get_tcp_origdstaddr6(int sockfd, skaddr6_t *dstaddr) {
    if (getsockopt(sockfd, SOL_IPV6, IP6T_SO_ORIGINAL_DST, dstaddr, &(socklen_t){sizeof(skaddr6_t)})) {
        LOGERR("[get_tcp_origdstaddr6] getsockopt(%d, IP6T_SO_ORIGINAL_DST): (%d) %s", sockfd, errno, errstring(errno));
        return false;
    }
    return true;
}

/* get the original dest addr of the ipv4 udp packet */
bool get_udp_origdstaddr4(struct msghdr *msg, skaddr4_t *dstaddr) {
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(msg); cmsg; cmsg = CMSG_NXTHDR(msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVORIGDSTADDR) {
            memcpy(dstaddr, CMSG_DATA(cmsg), sizeof(skaddr4_t));
            dstaddr->sin_family = AF_INET;
            return true;
        }
    }
    return false;
}

/* get the original dest addr of the ipv6 udp packet */
bool get_udp_origdstaddr6(struct msghdr *msg, skaddr6_t *dstaddr) {
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(msg); cmsg; cmsg = CMSG_NXTHDR(msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_IPV6 && cmsg->cmsg_type == IPV6_RECVORIGDSTADDR) {
            memcpy(dstaddr, CMSG_DATA(cmsg), sizeof(skaddr6_t));
            dstaddr->sin6_family = AF_INET6;
            return true;
        }
    }
    return false;
}

/* create non-blocking tcp socket (ipv4) */
int new_tcp4_socket(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        LOGERR("[new_tcp4_socket] socket(AF_INET, SOCK_STREAM): (%d) %s", errno, errstring(errno));
        exit(errno);
    }
    return sockfd;
}

/* create non-blocking tcp socket (ipv6) */
int new_tcp6_socket(void) {
    int sockfd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        LOGERR("[new_tcp6_socket] socket(AF_INET6, SOCK_STREAM): (%d) %s", errno, errstring(errno));
        exit(errno);
    }
    return sockfd;
}

/* create non-blocking udp socket (ipv4) */
int new_udp4_socket(void) {
    int sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        LOGERR("[new_udp4_socket] socket(AF_INET, SOCK_DGRAM): (%d) %s", errno, errstring(errno));
        exit(errno);
    }
    return sockfd;
}

/* create non-blocking udp socket (ipv6) */
int new_udp6_socket(void) {
    int sockfd = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        LOGERR("[new_udp6_socket] socket(AF_INET6, SOCK_DGRAM): (%d) %s", errno, errstring(errno));
        exit(errno);
    }
    return sockfd;
}

/* create tcp socket use to listen (ipv4) */
int new_tcp4_bindsock(void) {
    int sockfd = new_tcp4_socket();
    set_reuse_addr(sockfd);
    set_reuse_port(sockfd);
    return sockfd;
}

/* create tcp socket use to listen (ipv6) */
int new_tcp6_bindsock(void) {
    int sockfd = new_tcp6_socket();
    set_ipv6_only(sockfd);
    set_reuse_addr(sockfd);
    set_reuse_port(sockfd);
    return sockfd;
}

/* create tcp socket use to tproxy-listen (ipv4) */
int new_tcp4_bindsock_tproxy(void) {
    int sockfd = new_tcp4_bindsock();
    set_transparent(sockfd);
    return sockfd;
}

/* create tcp socket use to tproxy-listen (ipv6) */
int new_tcp6_bindsock_tproxy(void) {
    int sockfd = new_tcp6_bindsock();
    set_transparent(sockfd);
    return sockfd;
}

/* create udp socket use to tproxy-reply (ipv4) */
int new_udp4_respsock_tproxy(void) {
    int sockfd = new_udp4_socket();
    set_reuse_addr(sockfd);
    set_transparent(sockfd);
    return sockfd;
}

/* create udp socket use to tproxy-reply (ipv6) */
int new_udp6_respsock_tproxy(void) {
    int sockfd = new_udp6_socket();
    set_ipv6_only(sockfd);
    set_reuse_addr(sockfd);
    set_transparent(sockfd);
    return sockfd;
}

/* create udp socket use to tproxy-listen (ipv4) */
int new_udp4_bindsock_tproxy(void) {
    int sockfd = new_udp4_respsock_tproxy();
    set_recv_origdstaddr4(sockfd);
    return sockfd;
}

/* create udp socket use to tproxy-listen (ipv6) */
int new_udp6_bindsock_tproxy(void) {
    int sockfd = new_udp6_respsock_tproxy();
    set_recv_origdstaddr6(sockfd);
    return sockfd;
}

/* build ipv4 socket address from ipstr and portno */
void build_ipv4_addr(skaddr4_t *addr, const char *ipstr, portno_t portno) {
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, ipstr, &addr->sin_addr);
    addr->sin_port = htons(portno);
}

/* build ipv6 socket address from ipstr and portno */
void build_ipv6_addr(skaddr6_t *addr, const char *ipstr, portno_t portno) {
    addr->sin6_family = AF_INET6;
    inet_pton(AF_INET6, ipstr, &addr->sin6_addr);
    addr->sin6_port = htons(portno);
}

/* parse ipstr and portno from ipv4 socket address */
void parse_ipv4_addr(const skaddr4_t *addr, char *ipstr, portno_t *portno) {
    inet_ntop(AF_INET, &addr->sin_addr, ipstr, IP4STRLEN);
    *portno = ntohs(addr->sin_port);
}

/* parse ipstr and portno from ipv6 socket address */
void parse_ipv6_addr(const skaddr6_t *addr, char *ipstr, portno_t *portno) {
    inet_ntop(AF_INET6, &addr->sin6_addr, ipstr, IP6STRLEN);
    *portno = ntohs(addr->sin6_port);
}

/* AF_INET or AF_INET6 or -1(invalid ip string) */
int get_ipstr_family(const char *ipstr) {
    if (!ipstr) return -1;
    ipaddr_t ipaddr; /* save ipv4/ipv6 addr */
    if (inet_pton(AF_INET, ipstr, &ipaddr) == 1) {
        return AF_INET;
    } else if (inet_pton(AF_INET6, ipstr, &ipaddr) == 1) {
        return AF_INET6;
    } else {
        return -1;
    }
}

/* set nofile limit (may require root privileges) */
void set_nofile_limit(rlim_t nofile) {
    if (setrlimit(RLIMIT_NOFILE, &(struct rlimit){nofile, nofile}) < 0) {
        LOGERR("[set_nofile_limit] setrlimit(nofile, %lu): (%d) %s", (long unsigned)nofile, errno, errstring(errno));
        exit(errno);
    }
}

/* run the current process with a given user */
void run_as_user(const char *username, char *const argv[]) {
    if (geteuid() != 0) return; /* ignore if current user is not root */

    struct passwd *userinfo = getpwnam(username);
    if (!userinfo) {
        LOGERR("[run_as_user] the given user does not exist: %s", username);
        exit(1);
    }

    if (userinfo->pw_uid == 0) return; /* ignore if target user is root */

    if (setgid(userinfo->pw_gid) < 0) {
        LOGERR("[run_as_user] failed to change group_id of user '%s': (%d) %s", userinfo->pw_name, errno, errstring(errno));
        exit(errno);
    }

    if (initgroups(userinfo->pw_name, userinfo->pw_gid) < 0) {
        LOGERR("[run_as_user] failed to call initgroups() of user '%s': (%d) %s", userinfo->pw_name, errno, errstring(errno));
        exit(errno);
    }

    if (setuid(userinfo->pw_uid) < 0) {
        LOGERR("[run_as_user] failed to change user_id of user '%s': (%d) %s", userinfo->pw_name, errno, errstring(errno));
        exit(errno);
    }

    static char exec_file_abspath[PATH_MAX] = {0};
    if (readlink("/proc/self/exe", exec_file_abspath, PATH_MAX - 1) < 0) {
        LOGERR("[run_as_user] failed to get the abspath of execfile: (%d) %s", errno, errstring(errno));
        exit(errno);
    }

    if (argv && execv(exec_file_abspath, argv) < 0) {
        LOGERR("[run_as_user] failed to call execv(%s, args): (%d) %s", exec_file_abspath, errno, errstring(errno));
        exit(errno);
    }
}
