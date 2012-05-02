/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * iplink.c		"ip link".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include "rt_names.h"
#include "utils.h"
#include "ip_common.h"

#define IPLINK_IOCTL_COMPAT	1

static void usage(void) __attribute__((noreturn));

void iplink_usage(void)
{
	fprintf(stderr, "Usage: ip link set DEVICE { up | down |\n");
	fprintf(stderr, "	                     arp { on | off } |\n");
	fprintf(stderr, "	                     dynamic { on | off } |\n");
	fprintf(stderr, "	                     multicast { on | off } |\n");
	fprintf(stderr, "	                     allmulticast { on | off } |\n");
	fprintf(stderr, "	                     promisc { on | off } |\n");
	fprintf(stderr, "	                     trailers { on | off } |\n");
	fprintf(stderr, "	                     txqueuelen PACKETS |\n");
	fprintf(stderr, "	                     name NEWNAME |\n");
	fprintf(stderr, "	                     address LLADDR | broadcast LLADDR |\n");
	fprintf(stderr, "	                     mtu MTU }\n");
	fprintf(stderr, "       ip link show [ DEVICE ]\n");
	exit(-1);
}

static void usage(void)
{
	iplink_usage();
}

static int on_off(char *msg)
{
	fprintf(stderr, "Error: argument of \"%s\" must be \"on\" or \"off\"\n", msg);
	return -1;
}

static void *BODY;		/* cached dlopen(NULL) handle */
static struct link_util *linkutil_list;

struct link_util *get_link_kind(const char *id)
{
	void *dlh;
	char buf[256];
	struct link_util *l;

	for (l = linkutil_list; l; l = l->next)
		if (strcmp(l->id, id) == 0)
			return l;

	snprintf(buf, sizeof(buf), "/usr/lib/ip/link_%s.so", id);
	dlh = dlopen(buf, RTLD_LAZY);
	if (dlh == NULL) {
		/* look in current binary, only open once */
		dlh = BODY;
		if (dlh == NULL) {
			dlh = BODY = dlopen(NULL, RTLD_LAZY);
			if (dlh == NULL)
				return NULL;
		}
	}

	snprintf(buf, sizeof(buf), "%s_link_util", id);
	l = dlsym(dlh, buf);
	if (l == NULL)
		return NULL;

	l->next = linkutil_list;
	linkutil_list = l;
	return l;
}

#if IPLINK_IOCTL_COMPAT
static int have_rtnl_newlink = -1;

static int accept_msg(const struct sockaddr_nl *who,
		      struct nlmsghdr *n, void *arg)
{
	struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(n);

	if (n->nlmsg_type == NLMSG_ERROR &&
	    (err->error == -EOPNOTSUPP || err->error == -EINVAL))
		have_rtnl_newlink = 0;
	else
		have_rtnl_newlink = 1;
	return -1;
}

static int iplink_have_newlink(void)
{
	struct {
		struct nlmsghdr		n;
		struct ifinfomsg	i;
		char			buf[1024];
	} req;

	if (have_rtnl_newlink < 0) {
		memset(&req, 0, sizeof(req));

		req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
		req.n.nlmsg_flags = NLM_F_REQUEST|NLM_F_ACK;
		req.n.nlmsg_type = RTM_NEWLINK;
		req.i.ifi_family = AF_UNSPEC;

		rtnl_send(&rth, (char *)&req.n, req.n.nlmsg_len);
		rtnl_listen(&rth, accept_msg, NULL);
	}
	return have_rtnl_newlink;
}
#else /* IPLINK_IOCTL_COMPAT */
static int iplink_have_newlink(void)
{
	return 1;
}
#endif /* ! IPLINK_IOCTL_COMPAT */

struct iplink_req {
	struct nlmsghdr		n;
	struct ifinfomsg	i;
	char			buf[1024];
};

int iplink_parse(int argc, char **argv, struct iplink_req *req,
		char **name, char **type, char **link, char **dev)
{
	int ret, len;
	char abuf[32];
	int qlen = -1;
	int mtu = -1;

	ret = argc;

	while (argc > 0) {
		if (strcmp(*argv, "up") == 0) {
			req->i.ifi_change |= IFF_UP;
			req->i.ifi_flags |= IFF_UP;
		} else if (strcmp(*argv, "down") == 0) {
			req->i.ifi_change |= IFF_UP;
			req->i.ifi_flags &= ~IFF_UP;
		} else if (strcmp(*argv, "name") == 0) {
			NEXT_ARG();
			*name = *argv;
		} else if (matches(*argv, "link") == 0) {
			NEXT_ARG();
			*link = *argv;
		} else if (matches(*argv, "address") == 0) {
			NEXT_ARG();
			len = ll_addr_a2n(abuf, sizeof(abuf), *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_ADDRESS, abuf, len);
		} else if (matches(*argv, "broadcast") == 0 ||
				strcmp(*argv, "brd") == 0) {
			NEXT_ARG();
			len = ll_addr_a2n(abuf, sizeof(abuf), *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_BROADCAST, abuf, len);
		} else if (matches(*argv, "txqueuelen") == 0 ||
				strcmp(*argv, "qlen") == 0 ||
				matches(*argv, "txqlen") == 0) {
			NEXT_ARG();
			if (qlen != -1)
				duparg("txqueuelen", *argv);
			if (get_integer(&qlen,  *argv, 0))
				invarg("Invalid \"txqueuelen\" value\n", *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_TXQLEN, &qlen, 4);
		} else if (strcmp(*argv, "mtu") == 0) {
			NEXT_ARG();
			if (mtu != -1)
				duparg("mtu", *argv);
			if (get_integer(&mtu, *argv, 0))
				invarg("Invalid \"mtu\" value\n", *argv);
			addattr_l(&req->n, sizeof(*req), IFLA_MTU, &mtu, 4);
		} else if (strcmp(*argv, "multicast") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_MULTICAST;
			if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags |= IFF_MULTICAST;
			} else if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags &= ~IFF_MULTICAST;
			} else
				return on_off("multicast");
		} else if (strcmp(*argv, "allmulticast") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_ALLMULTI;
			if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags |= IFF_ALLMULTI;
			} else if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags &= ~IFF_ALLMULTI;
			} else
				return on_off("allmulticast");
		} else if (strcmp(*argv, "promisc") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_PROMISC;
			if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags |= IFF_PROMISC;
			} else if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags &= ~IFF_PROMISC;
			} else
				return on_off("promisc");
		} else if (strcmp(*argv, "trailers") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_NOTRAILERS;
			if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags |= IFF_NOTRAILERS;
			} else if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags &= ~IFF_NOTRAILERS;
			} else
				return on_off("trailers");
		} else if (strcmp(*argv, "arp") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_NOARP;
			if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags &= ~IFF_NOARP;
			} else if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags |= IFF_NOARP;
			} else
				return on_off("noarp");
#ifdef IFF_DYNAMIC
		} else if (matches(*argv, "dynamic") == 0) {
			NEXT_ARG();
			req->i.ifi_change |= IFF_DYNAMIC;
			if (strcmp(*argv, "on") == 0) {
				req->i.ifi_flags |= IFF_DYNAMIC;
			} else if (strcmp(*argv, "off") == 0) {
				req->i.ifi_flags &= ~IFF_DYNAMIC;
			} else
				return on_off("dynamic");
#endif
		} else if (matches(*argv, "type") == 0) {
			NEXT_ARG();
			*type = *argv;
			argc--; argv++;
			break;
		} else {
			if (strcmp(*argv, "dev") == 0) {
				NEXT_ARG();
			}
			if (*dev)
				duparg2("dev", *argv);
			*dev = *argv;
		}
		argc--; argv++;
	}

	return ret - argc;
}

static int iplink_modify(int cmd, unsigned int flags, int argc, char **argv)
{
	int len;
	char *dev = NULL;
	char *name = NULL;
	char *link = NULL;
	char *type = NULL;
	struct link_util *lu = NULL;
	struct iplink_req req;
	int ret;

	memset(&req, 0, sizeof(req));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.n.nlmsg_flags = NLM_F_REQUEST|flags;
	req.n.nlmsg_type = cmd;
	req.i.ifi_family = preferred_family;

	ret = iplink_parse(argc, argv, &req, &name, &type, &link, &dev);
	if (ret < 0)
		return ret;

	argc -= ret;
	argv += ret;
	ll_init_map(&rth);

	if (type) {
		struct rtattr *linkinfo = NLMSG_TAIL(&req.n);
		addattr_l(&req.n, sizeof(req), IFLA_LINKINFO, NULL, 0);
		addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, type,
			 strlen(type));

		lu = get_link_kind(type);
		if (lu && argc) {
			struct rtattr * data = NLMSG_TAIL(&req.n);
			addattr_l(&req.n, sizeof(req), IFLA_INFO_DATA, NULL, 0);

			if (lu->parse_opt &&
			    lu->parse_opt(lu, argc, argv, &req.n))
				return -1;

			data->rta_len = (void *)NLMSG_TAIL(&req.n) - (void *)data;
		} else if (argc) {
			if (matches(*argv, "help") == 0)
				usage();
			fprintf(stderr, "Garbage instead of arguments \"%s ...\". "
					"Try \"ip link help\".\n", *argv);
			return -1;
		}
		linkinfo->rta_len = (void *)NLMSG_TAIL(&req.n) - (void *)linkinfo;
	}

	if (!(flags & NLM_F_CREATE)) {
		if (!dev) {
			fprintf(stderr, "Not enough information: \"dev\" "
					"argument is required.\n");
			exit(-1);
		}

		req.i.ifi_index = ll_name_to_index(dev);
		if (req.i.ifi_index == 0) {
			fprintf(stderr, "Cannot find device \"%s\"\n", dev);
			return -1;
		}
	} else {
		/* Allow "ip link add dev" and "ip link add name" */
		if (!name)
			name = dev;

		if (link) {
			int ifindex;

			ifindex = ll_name_to_index(link);
			if (ifindex == 0) {
				fprintf(stderr, "Cannot find device \"%s\"\n",
					link);
				return -1;
			}
			addattr_l(&req.n, sizeof(req), IFLA_LINK, &ifindex, 4);
		}
	}

	if (name) {
		len = strlen(name) + 1;
		if (len == 1)
			invarg("\"\" is not a valid device identifier\n", "name");
		if (len > IFNAMSIZ)
			invarg("\"name\" too long\n", name);
		addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, len);
	}

	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		exit(2);

	return 0;
}

#if IPLINK_IOCTL_COMPAT
static int get_ctl_fd(void)
{
	int s_errno;
	int fd;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	s_errno = errno;
	fd = socket(PF_PACKET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	fd = socket(PF_INET6, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	errno = s_errno;
	perror("Cannot create control socket");
	return -1;
}

static int do_chflags(const char *dev, __u32 flags, __u32 mask)
{
	struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	fd = get_ctl_fd();
	if (fd < 0)
		return -1;
	err = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (err) {
		perror("SIOCGIFFLAGS");
		close(fd);
		return -1;
	}
	if ((ifr.ifr_flags^flags)&mask) {
		ifr.ifr_flags &= ~mask;
		ifr.ifr_flags |= mask&flags;
		err = ioctl(fd, SIOCSIFFLAGS, &ifr);
		if (err)
			perror("SIOCSIFFLAGS");
	}
	close(fd);
	return err;
}

static int do_changename(const char *dev, const char *newdev)
{
	struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	strncpy(ifr.ifr_newname, newdev, IFNAMSIZ);
	fd = get_ctl_fd();
	if (fd < 0)
		return -1;
	err = ioctl(fd, SIOCSIFNAME, &ifr);
	if (err) {
		perror("SIOCSIFNAME");
		close(fd);
		return -1;
	}
	close(fd);
	return err;
}

static int set_qlen(const char *dev, int qlen)
{
	struct ifreq ifr;
	int s;

	s = get_ctl_fd();
	if (s < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	ifr.ifr_qlen = qlen;
	if (ioctl(s, SIOCSIFTXQLEN, &ifr) < 0) {
		perror("SIOCSIFXQLEN");
		close(s);
		return -1;
	}
	close(s);

	return 0;
}

static int set_mtu(const char *dev, int mtu)
{
	struct ifreq ifr;
	int s;

	s = get_ctl_fd();
	if (s < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	ifr.ifr_mtu = mtu;
	if (ioctl(s, SIOCSIFMTU, &ifr) < 0) {
		perror("SIOCSIFMTU");
		close(s);
		return -1;
	}
	close(s);

	return 0;
}

static int get_address(const char *dev, int *htype)
{
	struct ifreq ifr;
	struct sockaddr_ll me;
	socklen_t alen;
	int s;

	s = socket(PF_PACKET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket(PF_PACKET)");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		perror("SIOCGIFINDEX");
		close(s);
		return -1;
	}

	memset(&me, 0, sizeof(me));
	me.sll_family = AF_PACKET;
	me.sll_ifindex = ifr.ifr_ifindex;
	me.sll_protocol = htons(ETH_P_LOOP);
	if (bind(s, (struct sockaddr*)&me, sizeof(me)) == -1) {
		perror("bind");
		close(s);
		return -1;
	}

	alen = sizeof(me);
	if (getsockname(s, (struct sockaddr*)&me, &alen) == -1) {
		perror("getsockname");
		close(s);
		return -1;
	}
	close(s);
	*htype = me.sll_hatype;
	return me.sll_halen;
}

static int parse_address(const char *dev, int hatype, int halen,
		char *lla, struct ifreq *ifr)
{
	int alen;

	memset(ifr, 0, sizeof(*ifr));
	strncpy(ifr->ifr_name, dev, IFNAMSIZ);
	ifr->ifr_hwaddr.sa_family = hatype;
	alen = ll_addr_a2n(ifr->ifr_hwaddr.sa_data, 14, lla);
	if (alen < 0)
		return -1;
	if (alen != halen) {
		fprintf(stderr, "Wrong address (%s) length: expected %d bytes\n", lla, halen);
		return -1;
	}
	return 0;
}

static int set_address(struct ifreq *ifr, int brd)
{
	int s;

	s = get_ctl_fd();
	if (s < 0)
		return -1;
	if (ioctl(s, brd?SIOCSIFHWBROADCAST:SIOCSIFHWADDR, ifr) < 0) {
		perror(brd?"SIOCSIFHWBROADCAST":"SIOCSIFHWADDR");
		close(s);
		return -1;
	}
	close(s);
	return 0;
}


static int do_set(int argc, char **argv)
{
	char *dev = NULL;
	__u32 mask = 0;
	__u32 flags = 0;
	int qlen = -1;
	int mtu = -1;
	char *newaddr = NULL;
	char *newbrd = NULL;
	struct ifreq ifr0, ifr1;
	char *newname = NULL;
	int htype, halen;

	while (argc > 0) {
		if (strcmp(*argv, "up") == 0) {
			mask |= IFF_UP;
			flags |= IFF_UP;
		} else if (strcmp(*argv, "down") == 0) {
			mask |= IFF_UP;
			flags &= ~IFF_UP;
		} else if (strcmp(*argv, "name") == 0) {
			NEXT_ARG();
			newname = *argv;
		} else if (matches(*argv, "address") == 0) {
			NEXT_ARG();
			newaddr = *argv;
		} else if (matches(*argv, "broadcast") == 0 ||
			   strcmp(*argv, "brd") == 0) {
			NEXT_ARG();
			newbrd = *argv;
		} else if (matches(*argv, "txqueuelen") == 0 ||
			   strcmp(*argv, "qlen") == 0 ||
			   matches(*argv, "txqlen") == 0) {
			NEXT_ARG();
			if (qlen != -1)
				duparg("txqueuelen", *argv);
			if (get_integer(&qlen,  *argv, 0))
				invarg("Invalid \"txqueuelen\" value\n", *argv);
		} else if (strcmp(*argv, "mtu") == 0) {
			NEXT_ARG();
			if (mtu != -1)
				duparg("mtu", *argv);
			if (get_integer(&mtu, *argv, 0))
				invarg("Invalid \"mtu\" value\n", *argv);
		} else if (strcmp(*argv, "multicast") == 0) {
			NEXT_ARG();
			mask |= IFF_MULTICAST;
			if (strcmp(*argv, "on") == 0) {
				flags |= IFF_MULTICAST;
			} else if (strcmp(*argv, "off") == 0) {
				flags &= ~IFF_MULTICAST;
			} else
				return on_off("multicast");
		} else if (strcmp(*argv, "allmulticast") == 0) {
			NEXT_ARG();
			mask |= IFF_ALLMULTI;
			if (strcmp(*argv, "on") == 0) {
				flags |= IFF_ALLMULTI;
			} else if (strcmp(*argv, "off") == 0) {
				flags &= ~IFF_ALLMULTI;
			} else
				return on_off("allmulticast");
		} else if (strcmp(*argv, "promisc") == 0) {
			NEXT_ARG();
			mask |= IFF_PROMISC;
			if (strcmp(*argv, "on") == 0) {
				flags |= IFF_PROMISC;
			} else if (strcmp(*argv, "off") == 0) {
				flags &= ~IFF_PROMISC;
			} else
				return on_off("promisc");
		} else if (strcmp(*argv, "trailers") == 0) {
			NEXT_ARG();
			mask |= IFF_NOTRAILERS;
			if (strcmp(*argv, "off") == 0) {
				flags |= IFF_NOTRAILERS;
			} else if (strcmp(*argv, "on") == 0) {
				flags &= ~IFF_NOTRAILERS;
			} else
				return on_off("trailers");
		} else if (strcmp(*argv, "arp") == 0) {
			NEXT_ARG();
			mask |= IFF_NOARP;
			if (strcmp(*argv, "on") == 0) {
				flags &= ~IFF_NOARP;
			} else if (strcmp(*argv, "off") == 0) {
				flags |= IFF_NOARP;
			} else
				return on_off("noarp");
#ifdef IFF_DYNAMIC
		} else if (matches(*argv, "dynamic") == 0) {
			NEXT_ARG();
			mask |= IFF_DYNAMIC;
			if (strcmp(*argv, "on") == 0) {
				flags |= IFF_DYNAMIC;
			} else if (strcmp(*argv, "off") == 0) {
				flags &= ~IFF_DYNAMIC;
			} else
				return on_off("dynamic");
#endif
		} else {
                        if (strcmp(*argv, "dev") == 0) {
				NEXT_ARG();
			}
			if (matches(*argv, "help") == 0)
				usage();
			if (dev)
				duparg2("dev", *argv);
			dev = *argv;
		}
		argc--; argv++;
	}

	if (!dev) {
		fprintf(stderr, "Not enough of information: \"dev\" argument is required.\n");
		exit(-1);
	}

	if (newaddr || newbrd) {
		halen = get_address(dev, &htype);
		if (halen < 0)
			return -1;
		if (newaddr) {
			if (parse_address(dev, htype, halen, newaddr, &ifr0) < 0)
				return -1;
		}
		if (newbrd) {
			if (parse_address(dev, htype, halen, newbrd, &ifr1) < 0)
				return -1;
		}
	}

	if (newname && strcmp(dev, newname)) {
		if (strlen(newname) == 0)
			invarg("\"\" is not a valid device identifier\n", "name");
		if (do_changename(dev, newname) < 0)
			return -1;
		dev = newname;
	}
	if (qlen != -1) {
		if (set_qlen(dev, qlen) < 0)
			return -1;
	}
	if (mtu != -1) {
		if (set_mtu(dev, mtu) < 0)
			return -1;
	}
	if (newaddr || newbrd) {
		if (newbrd) {
			if (set_address(&ifr1, 1) < 0)
				return -1;
		}
		if (newaddr) {
			if (set_address(&ifr0, 0) < 0)
				return -1;
		}
	}
	if (mask)
		return do_chflags(dev, flags, mask);
	return 0;
}
#endif /* IPLINK_IOCTL_COMPAT */

int do_iplink(int argc, char **argv)
{
	if (argc > 0) {
		if (iplink_have_newlink()) {
			if (matches(*argv, "add") == 0)
				return iplink_modify(RTM_NEWLINK,
						     NLM_F_CREATE|NLM_F_EXCL,
						     argc-1, argv+1);
			if (matches(*argv, "set") == 0 ||
			    matches(*argv, "change") == 0)
				return iplink_modify(RTM_NEWLINK, 0,
						     argc-1, argv+1);
			if (matches(*argv, "replace") == 0)
				return iplink_modify(RTM_NEWLINK,
						     NLM_F_CREATE|NLM_F_REPLACE,
						     argc-1, argv+1);
			if (matches(*argv, "delete") == 0)
				return iplink_modify(RTM_DELLINK, 0,
						     argc-1, argv+1);
		} else {
#if IPLINK_IOCTL_COMPAT
			if (matches(*argv, "set") == 0)
				return do_set(argc-1, argv+1);
#endif
		}
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return ipaddr_list_link(argc-1, argv+1);
		if (matches(*argv, "help") == 0)
			usage();
	} else
		return ipaddr_list_link(0, NULL);

	fprintf(stderr, "Command \"%s\" is unknown, try \"ip link help\".\n", *argv);
	exit(-1);
}
