#define _GNU_SOURCE
#include "netutils.h"
#include "logutils.h"
#include "chinadns.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <linux/netlink.h>
#undef _GNU_SOURCE

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15
#endif

/* netfilter and ipset constants definition */
#define NFNETLINK_V0 0
#define NFNL_SUBSYS_IPSET 6
#define IPSET_CMD_TEST 11
#define IPSET_PROTOCOL 6
#define IPSET_ATTR_PROTOCOL 1
#define IPSET_ATTR_SETNAME 2
#define IPSET_ATTR_DATA 7
#define IPSET_ATTR_IP 1
#define IPSET_ATTR_IPADDR_IPV4 1
#define IPSET_ATTR_IPADDR_IPV6 2

/* ipset error code constants definition */
#define IPSET_ERR_PROTOCOL -4097
#define IPSET_ERR_FIND_TYPE -4098
#define IPSET_ERR_MAX_SETS -4099
#define IPSET_ERR_BUSY -4100
#define IPSET_ERR_EXIST_SETNAME2 -4101
#define IPSET_ERR_TYPE_MISMATCH -4102
#define IPSET_ERR_EXIST -4103
#define IPSET_ERR_INVALID_CIDR -4104
#define IPSET_ERR_INVALID_NETMASK -4105
#define IPSET_ERR_INVALID_FAMILY -4106
#define IPSET_ERR_TIMEOUT -4107
#define IPSET_ERR_REFERENCED -4108
#define IPSET_ERR_IPADDR_IPV4 -4109
#define IPSET_ERR_IPADDR_IPV6 -4110
#define IPSET_ERR_COUNTER -4111
#define IPSET_ERR_COMMENT -4112
#define IPSET_ERR_INVALID_MARKMASK -4113
#define IPSET_ERR_SKBINFO -4114
#define IPSET_ERR_HASH_FULL -4352
#define IPSET_ERR_HASH_ELEM -4353
#define IPSET_ERR_INVALID_PROTO -4354
#define IPSET_ERR_MISSING_PROTO -4355
#define IPSET_ERR_HASH_RANGE_UNSUPPORTED -4356
#define IPSET_ERR_HASH_RANGE -4357

/* netfilter's general netlink message structure */
struct nfgenmsg {
    __u8    nfgen_family;   /* AF_xxx */
    __u8    version;        /* nfnetlink version */
    __be16  res_id;         /* resource id */
};

/* ipset netlink msg buffer maxlen */
#define MSGBUFFER_MAXLEN 256

/* static global variable declaration */
static int    g_ipset_nlsocket                      = -1;
static __u32  g_ipset_nlmsg_seq                     = 1;
static char   g_ipset_sendbuffer4[MSGBUFFER_MAXLEN] = {0};
static char   g_ipset_sendbuffer6[MSGBUFFER_MAXLEN] = {0};
static char   g_ipset_recvbuffer[MSGBUFFER_MAXLEN]  = {0};
static void  *g_ipset_ipv4addr_ptr                  = NULL;
static void  *g_ipset_ipv6addr_ptr                  = NULL;
static __u32 *g_ipset_nlmsg4_seq_ptr                = NULL;
static __u32 *g_ipset_nlmsg6_seq_ptr                = NULL;

/* create a udp socket (AF_INET) */
int new_udp4_socket(void) {
    int sockfd = socket(AF_INET, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        LOGERR("[new_udp4_socket] failed to create udp socket: (%d) %s", errno, strerror(errno));
        exit(errno);
    }
    return sockfd;
}

/* create a udp socket (AF_INET6) */
int new_udp6_socket(void) {
    int sockfd = socket(AF_INET6, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        LOGERR("[new_udp6_socket] failed to create udp socket: (%d) %s", errno, strerror(errno));
        exit(errno);
    }
    return sockfd;
}

/* setsockopt(IPV6_V6ONLY) */
void set_ipv6_only(int sockfd) {
    const int optval = 1;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval))) {
        LOGERR("[set_ipv6_only] setsockopt(%d, IPV6_V6ONLY): (%d) %s", sockfd, errno, strerror(errno));
        exit(errno);
    }
}

/* setsockopt(SO_REUSEADDR) */
void set_reuse_addr(int sockfd) {
    const int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
        LOGERR("[set_reuse_addr] setsockopt(%d, SO_REUSEADDR): (%d) %s", sockfd, errno, strerror(errno));
        exit(errno);
    }
}

/* setsockopt(SO_REUSEPORT) */
void set_reuse_port(int sockfd) {
    const int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) {
        LOGERR("[set_reuse_port] setsockopt(%d, SO_REUSEPORT): (%d) %s", sockfd, errno, strerror(errno));
        exit(errno);
    }
}

/* create a timer fd (in seconds) */
int new_once_timerfd(time_t second) {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd < 0) {
        LOGERR("[new_once_timerfd] failed to create timer fd: (%d) %s", errno, strerror(errno));
        exit(errno);
    }
    struct itimerspec time_value;
    time_value.it_value.tv_sec = second;
    time_value.it_value.tv_nsec = 0;
    time_value.it_interval.tv_sec = 0;
    time_value.it_interval.tv_nsec = 0;
    if (timerfd_settime(timerfd, 0, &time_value, NULL)) {
        LOGERR("[new_once_timerfd] failed to settime for timer fd: (%d) %s", errno, strerror(errno));
        exit(errno);
    }
    return timerfd;
}

/* AF_INET or AF_INET6 or -1(invalid) */
int get_addrstr_family(const char *addrstr) {
    if (!addrstr) return -1;
    inet6_ipaddr_t addrbin; /* save ipv4 and ipv6 addr */
    if (inet_pton(AF_INET, addrstr, &addrbin) == 1) {
        return AF_INET;
    } else if (inet_pton(AF_INET6, addrstr, &addrbin) == 1) {
        return AF_INET6;
    } else {
        return -1;
    }
}

/* build ipv4 address structure */
void build_ipv4_addr(inet4_skaddr_t *addr, const char *host, sock_port_t port) {
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, host, &addr->sin_addr);
    addr->sin_port = htons(port);
}

/* build ipv6 address structure */
void build_ipv6_addr(inet6_skaddr_t *addr, const char *host, sock_port_t port) {
    addr->sin6_family = AF_INET6;
    inet_pton(AF_INET6, host, &addr->sin6_addr);
    addr->sin6_port = htons(port);
}

/* parse ipv4 address structure */
void parse_ipv4_addr(const inet4_skaddr_t *addr, char *host, sock_port_t *port) {
    inet_ntop(AF_INET, &addr->sin_addr, host, INET_ADDRSTRLEN);
    *port = ntohs(addr->sin_port);
}

/* parse ipv6 address structure */
void parse_ipv6_addr(const inet6_skaddr_t *addr, char *host, sock_port_t *port) {
    inet_ntop(AF_INET6, &addr->sin6_addr, host, INET6_ADDRSTRLEN);
    *port = ntohs(addr->sin6_port);
}

/* create ipset netlink socket */
static void ipset_create_nlsocket(void) {
    /* create netlink socket */
    g_ipset_nlsocket = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_NETFILTER);
    if (g_ipset_nlsocket < 0) {
        LOGERR("[ipset_create_nlsocket] failed to create netlink socket: (%d) %s", errno, strerror(errno));
        exit(errno);
    }

    /* bind netlink address */
    struct sockaddr_nl self_addr = {.nl_family = AF_NETLINK, .nl_pid = getpid(), .nl_groups = 0};
    if (bind(g_ipset_nlsocket, (void *)&self_addr, sizeof(self_addr))) {
        LOGERR("[ipset_create_nlsocket] failed to bind address to socket: (%d) %s", errno, strerror(errno));
        exit(errno);
    }

    /* connect to kernel */
    struct sockaddr_nl kernel_addr = {.nl_family = AF_NETLINK, .nl_pid = 0, .nl_groups = 0};
    if (connect(g_ipset_nlsocket, (void *)&kernel_addr, sizeof(kernel_addr))) {
        LOGERR("[ipset_create_nlsocket] failed to connect to kernel: (%d) %s", errno, strerror(errno));
        exit(errno);
    }
}

/* prebuild nlmsg for ipset query */
static void ipset_prebuild_nlmsg(bool is_ipv4) {
    void *buffer = is_ipv4 ? g_ipset_sendbuffer4 : g_ipset_sendbuffer6;
    const char *setname = is_ipv4 ? g_ipset_setname4 : g_ipset_setname6;
    size_t setnamelen = strlen(setname) + 1;

    /* netlink msg */
    struct nlmsghdr *netlink_msg = buffer;
    netlink_msg->nlmsg_len = NLMSG_ALIGN(sizeof(struct nlmsghdr));
    netlink_msg->nlmsg_type = (NFNL_SUBSYS_IPSET << 8) | IPSET_CMD_TEST;
    netlink_msg->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    netlink_msg->nlmsg_pid = getpid();
    netlink_msg->nlmsg_seq = g_ipset_nlmsg_seq; // should be incremented
    if (is_ipv4) g_ipset_nlmsg4_seq_ptr = &netlink_msg->nlmsg_seq; // ptr for set ipv4 nlmsg seq
    if (!is_ipv4) g_ipset_nlmsg6_seq_ptr = &netlink_msg->nlmsg_seq; // ptr for set ipv6 nlmsg seq

    /* netfilter msg */
    struct nfgenmsg *netfilter_msg = buffer + netlink_msg->nlmsg_len;
    netfilter_msg->nfgen_family = is_ipv4 ? AF_INET : AF_INET6;
    netfilter_msg->version = NFNETLINK_V0;
    netfilter_msg->res_id = htons(0);
    netlink_msg->nlmsg_len += NLMSG_ALIGN(sizeof(struct nfgenmsg)); // update netlink msglen

    /* ipset_protocol attr */
    struct nlattr *ipset_protocol_attr = buffer + netlink_msg->nlmsg_len;
    ipset_protocol_attr->nla_len = NLMSG_ALIGN(sizeof(struct nlattr)) + sizeof(uint8_t);
    ipset_protocol_attr->nla_type = IPSET_ATTR_PROTOCOL;
    *(uint8_t *)((void *)ipset_protocol_attr + NLMSG_ALIGN(sizeof(struct nlattr))) = IPSET_PROTOCOL;
    netlink_msg->nlmsg_len += NLMSG_ALIGN(ipset_protocol_attr->nla_len); // update netlink msglen

    /* ipset_setname attr */
    struct nlattr *ipset_setname_attr = buffer + netlink_msg->nlmsg_len;
    ipset_setname_attr->nla_len = NLMSG_ALIGN(sizeof(struct nlattr)) + setnamelen;
    ipset_setname_attr->nla_type = IPSET_ATTR_SETNAME;
    memcpy((void *)ipset_setname_attr + NLMSG_ALIGN(sizeof(struct nlattr)), setname, setnamelen);
    netlink_msg->nlmsg_len += NLMSG_ALIGN(ipset_setname_attr->nla_len); // update netlink msglen

    /* ipset_data attr (nested) */
    struct nlattr *ipset_data_nestedattr = buffer + netlink_msg->nlmsg_len;
    ipset_data_nestedattr->nla_len = NLMSG_ALIGN(sizeof(struct nlattr));
    ipset_data_nestedattr->nla_type = IPSET_ATTR_DATA | NLA_F_NESTED;
    netlink_msg->nlmsg_len += ipset_data_nestedattr->nla_len; // update netlink msglen

    /* ipset_ip addr (nested) */
    struct nlattr *ipset_ip_nestedattr = buffer + netlink_msg->nlmsg_len;
    ipset_ip_nestedattr->nla_len = NLMSG_ALIGN(sizeof(struct nlattr));
    ipset_ip_nestedattr->nla_type = IPSET_ATTR_IP | NLA_F_NESTED;
    ipset_data_nestedattr->nla_len += ipset_ip_nestedattr->nla_len; // update ipset_data attrlen
    netlink_msg->nlmsg_len += ipset_ip_nestedattr->nla_len; // update netlink msglen

    /* ipset_ip attr */
    struct nlattr *ipset_ip_attr = buffer + netlink_msg->nlmsg_len;
    ipset_ip_attr->nla_len = NLMSG_ALIGN(sizeof(struct nlattr)) + (is_ipv4 ? sizeof(inet4_ipaddr_t) : sizeof(inet6_ipaddr_t));
    ipset_ip_attr->nla_type = (is_ipv4 ? IPSET_ATTR_IPADDR_IPV4 : IPSET_ATTR_IPADDR_IPV6) | NLA_F_NET_BYTEORDER;
    if (is_ipv4) g_ipset_ipv4addr_ptr = (void *)&ipset_ip_attr->nla_type + sizeof(ipset_ip_attr->nla_type); // ptr for set ipv4 addr
    if (!is_ipv4) g_ipset_ipv6addr_ptr = (void *)&ipset_ip_attr->nla_type + sizeof(ipset_ip_attr->nla_type); // ptr for set ipv6 addr
    ipset_ip_nestedattr->nla_len += NLMSG_ALIGN(ipset_ip_attr->nla_len); // update ipset_ip attrlen
    ipset_data_nestedattr->nla_len += NLMSG_ALIGN(ipset_ip_attr->nla_len); // update ipset_data attrlen
    netlink_msg->nlmsg_len += NLMSG_ALIGN(ipset_ip_attr->nla_len); // update netlink msglen
}

/* init netlink socket for ipset query */
void ipset_init_nlsocket(void) {
    ipset_create_nlsocket(); /* create netlink socket */
    ipset_prebuild_nlmsg(true); /* prebuild ipv4 nlmsg */
    ipset_prebuild_nlmsg(false); /* prebuild ipv6 nlmsg */
}

/* get a string description of the ipset error code */
static inline const char* ipset_error_tostr(int errcode) {
    switch (errcode) {
        case IPSET_ERR_PROTOCOL: return "IPSET_ERR_PROTOCOL";
        case IPSET_ERR_FIND_TYPE: return "IPSET_ERR_FIND_TYPE";
        case IPSET_ERR_MAX_SETS: return "IPSET_ERR_MAX_SETS";
        case IPSET_ERR_BUSY: return "IPSET_ERR_BUSY";
        case IPSET_ERR_EXIST_SETNAME2: return "IPSET_ERR_EXIST_SETNAME2";
        case IPSET_ERR_TYPE_MISMATCH: return "IPSET_ERR_TYPE_MISMATCH";
        case IPSET_ERR_EXIST: return "IPSET_ERR_EXIST";
        case IPSET_ERR_INVALID_CIDR: return "IPSET_ERR_INVALID_CIDR";
        case IPSET_ERR_INVALID_NETMASK: return "IPSET_ERR_INVALID_NETMASK";
        case IPSET_ERR_INVALID_FAMILY: return "IPSET_ERR_INVALID_FAMILY";
        case IPSET_ERR_INVALID_MARKMASK: return "IPSET_ERR_INVALID_MARKMASK";
        case IPSET_ERR_TIMEOUT: return "IPSET_ERR_TIMEOUT";
        case IPSET_ERR_REFERENCED: return "IPSET_ERR_REFERENCED";
        case IPSET_ERR_IPADDR_IPV4: return "IPSET_ERR_IPADDR_IPV4";
        case IPSET_ERR_IPADDR_IPV6: return "IPSET_ERR_IPADDR_IPV6";
        case IPSET_ERR_COUNTER: return "IPSET_ERR_COUNTER";
        case IPSET_ERR_COMMENT: return "IPSET_ERR_COMMENT";
        case IPSET_ERR_SKBINFO: return "IPSET_ERR_SKBINFO";
        case IPSET_ERR_HASH_FULL: return "IPSET_ERR_HASH_FULL";
        case IPSET_ERR_HASH_ELEM: return "IPSET_ERR_HASH_ELEM";
        case IPSET_ERR_INVALID_PROTO: return "IPSET_ERR_INVALID_PROTO";
        case IPSET_ERR_MISSING_PROTO: return "IPSET_ERR_MISSING_PROTO";
        case IPSET_ERR_HASH_RANGE_UNSUPPORTED: return "IPSET_ERR_HASH_RANGE_UNSUPPORTED";
        case IPSET_ERR_HASH_RANGE: return "IPSET_ERR_HASH_RANGE";
        default: return strerror(-errcode);
    }
}

/* check given ipaddr is exists in ipset */
bool ipset_addr4_is_exists(const inet4_ipaddr_t *addr_ptr) {
    memcpy(g_ipset_ipv4addr_ptr, addr_ptr, sizeof(inet4_ipaddr_t));
    *g_ipset_nlmsg4_seq_ptr = g_ipset_nlmsg_seq++;
    const struct nlmsghdr *netlink_msg = (struct nlmsghdr *)g_ipset_sendbuffer4;
    if (send(g_ipset_nlsocket, g_ipset_sendbuffer4, netlink_msg->nlmsg_len, 0) < 0) {
        LOGERR("[ipset_addr4_is_exists] failed to send netlink msg to kernel: (%d) %s", errno, strerror(errno));
        return false;
    }
    if (recv(g_ipset_nlsocket, g_ipset_recvbuffer, MSGBUFFER_MAXLEN, 0) < 0) {
        LOGERR("[ipset_addr4_is_exists] failed to recv netlink msg from kernel: (%d) %s", errno, strerror(errno));
        return false;
    }
    const struct nlmsgerr *netlink_errmsg = NLMSG_DATA(g_ipset_recvbuffer);
    switch (netlink_errmsg->error) {
        case 0:
            return true; // exist
        case IPSET_ERR_EXIST:
            return false; // not exist
        default:
            LOGERR("[ipset_addr4_is_exists] received an error code from kernel: (%d) %s", netlink_errmsg->error, ipset_error_tostr(netlink_errmsg->error));
            return false; // error occurred
    }
    return false; // unachievable
}

/* check given ipaddr is exists in ipset */
bool ipset_addr6_is_exists(const inet6_ipaddr_t *addr_ptr) {
    memcpy(g_ipset_ipv6addr_ptr, addr_ptr, sizeof(inet6_ipaddr_t));
    *g_ipset_nlmsg6_seq_ptr = g_ipset_nlmsg_seq++;
    const struct nlmsghdr *netlink_msg = (struct nlmsghdr *)g_ipset_sendbuffer6;
    if (send(g_ipset_nlsocket, g_ipset_sendbuffer6, netlink_msg->nlmsg_len, 0) < 0) {
        LOGERR("[ipset_addr6_is_exists] failed to send netlink msg to kernel: (%d) %s", errno, strerror(errno));
        return false;
    }
    if (recv(g_ipset_nlsocket, g_ipset_recvbuffer, MSGBUFFER_MAXLEN, 0) < 0) {
        LOGERR("[ipset_addr6_is_exists] failed to recv netlink msg from kernel: (%d) %s", errno, strerror(errno));
        return false;
    }
    const struct nlmsgerr *netlink_errmsg = NLMSG_DATA(g_ipset_recvbuffer);
    switch (netlink_errmsg->error) {
        case 0:
            return true; // exist
        case IPSET_ERR_EXIST:
            return false; // not exist
        default:
            LOGERR("[ipset_addr6_is_exists] received an error code from kernel: (%d) %s", netlink_errmsg->error, ipset_error_tostr(netlink_errmsg->error));
            return false; // error occurred
    }
    return false; // unachievable
}
