/* ipset.c is Copyright (c) 2013 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

#if defined(HAVE_IPSET) && defined(HAVE_LINUX_NETWORK)

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/version.h>
#include <linux/netlink.h>

/* We want to be able to compile against old header files
   Kernel version is handled at run-time. */

#define NFNL_SUBSYS_IPSET 6

#define IPSET_ATTR_DATA 7
#define IPSET_ATTR_IP 1
#define IPSET_ATTR_IPADDR_IPV4 1
#define IPSET_ATTR_IPADDR_IPV6 2
#define IPSET_ATTR_PROTOCOL 1
#define IPSET_ATTR_SETNAME 2
#define IPSET_CMD_ADD 9
#define IPSET_CMD_DEL 10
#define IPSET_MAXNAMELEN 32
#define IPSET_PROTOCOL 6

#ifndef NFNETLINK_V0
#define NFNETLINK_V0    0
#endif

#ifndef NLA_F_NESTED
#define NLA_F_NESTED		(1 << 15)
#endif

#ifndef NLA_F_NET_BYTEORDER
#define NLA_F_NET_BYTEORDER	(1 << 14)
#endif

struct my_nlattr {
        __u16           nla_len;
        __u16           nla_type;
};

struct my_nfgenmsg {
        __u8  nfgen_family;             /* AF_xxx */
        __u8  version;          /* nfnetlink version */
        __be16    res_id;               /* resource id */
};


/* data structure size in here is fixed */
#define BUFF_SZ 256

#define NL_ALIGN(len) (((len)+3) & ~(3))
static const struct sockaddr_nl snl = { .nl_family = AF_NETLINK };
static int ipset_sock;
static char *buffer;

static inline void add_attr(struct nlmsghdr *nlh, uint16_t type, size_t len, const void *data)
{
  struct my_nlattr *attr = (void *)nlh + NL_ALIGN(nlh->nlmsg_len);
  uint16_t payload_len = NL_ALIGN(sizeof(struct my_nlattr)) + len;
  attr->nla_type = type;
  attr->nla_len = payload_len;
  memcpy((void *)attr + NL_ALIGN(sizeof(struct my_nlattr)), data, len);
  nlh->nlmsg_len += NL_ALIGN(payload_len);
}

void ipset_init(void)
{
  if ( 
      (buffer = safe_malloc(BUFF_SZ)) &&
      (ipset_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_NETFILTER)) != -1 &&
      (bind(ipset_sock, (struct sockaddr *)&snl, sizeof(snl)) != -1))
    return;
  
  die (_("failed to create IPset control socket: %s"), NULL, EC_MISC);
}

static int new_add_to_ipset(const char *setname, const struct all_addr *ipaddr, int af, int remove)
{
  struct nlmsghdr *nlh;
  struct my_nfgenmsg *nfg;
  struct my_nlattr *nested[2];
  uint8_t proto;
  int addrsz = INADDRSZ;

#ifdef HAVE_IPV6
  if (af == AF_INET6)
    addrsz = IN6ADDRSZ;
#endif
    
  if (strlen(setname) >= IPSET_MAXNAMELEN) 
    {
      errno = ENAMETOOLONG;
      return -1;
    }
  
  memset(buffer, 0, BUFF_SZ);

  nlh = (struct nlmsghdr *)buffer;
  nlh->nlmsg_len = NL_ALIGN(sizeof(struct nlmsghdr));
  nlh->nlmsg_type = (remove ? IPSET_CMD_DEL : IPSET_CMD_ADD) | (NFNL_SUBSYS_IPSET << 8);
  nlh->nlmsg_flags = NLM_F_REQUEST;
  
  nfg = (struct my_nfgenmsg *)(buffer + nlh->nlmsg_len);
  nlh->nlmsg_len += NL_ALIGN(sizeof(struct my_nfgenmsg));
  nfg->nfgen_family = af;
  nfg->version = NFNETLINK_V0;
  nfg->res_id = htons(0);
  
  proto = IPSET_PROTOCOL;
  add_attr(nlh, IPSET_ATTR_PROTOCOL, sizeof(proto), &proto);
  add_attr(nlh, IPSET_ATTR_SETNAME, strlen(setname) + 1, setname);
  nested[0] = (struct my_nlattr *)(buffer + NL_ALIGN(nlh->nlmsg_len));
  nlh->nlmsg_len += NL_ALIGN(sizeof(struct my_nlattr));
  nested[0]->nla_type = NLA_F_NESTED | IPSET_ATTR_DATA;
  nested[1] = (struct my_nlattr *)(buffer + NL_ALIGN(nlh->nlmsg_len));
  nlh->nlmsg_len += NL_ALIGN(sizeof(struct my_nlattr));
  nested[1]->nla_type = NLA_F_NESTED | IPSET_ATTR_IP;
  add_attr(nlh, 
	   (af == AF_INET ? IPSET_ATTR_IPADDR_IPV4 : IPSET_ATTR_IPADDR_IPV6) | NLA_F_NET_BYTEORDER,
	   addrsz, &ipaddr->addr);
  nested[1]->nla_len = (void *)buffer + NL_ALIGN(nlh->nlmsg_len) - (void *)nested[1];
  nested[0]->nla_len = (void *)buffer + NL_ALIGN(nlh->nlmsg_len) - (void *)nested[0];
	
  while (retry_send(sendto(ipset_sock, buffer, nlh->nlmsg_len, 0,
			   (struct sockaddr *)&snl, sizeof(snl))));
								    
  return errno == 0 ? 0 : -1;
}


int add_to_ipset(const char *setname, const struct all_addr *ipaddr, int flags, int remove)
{
  int ret, af = AF_INET;

#ifdef HAVE_IPV6
  if (flags & F_IPV6)
      af = AF_INET6;
#endif

  ret = new_add_to_ipset(setname, ipaddr, af, remove);
  if (ret == -1)
     my_syslog(LOG_ERR, _("failed to update ipset %s: %s"), setname, strerror(errno));

  return ret;
}

#endif
