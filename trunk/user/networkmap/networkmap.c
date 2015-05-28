#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>

#include <shutils.h>
#include <netutils.h>
#include <nvram_linux.h>
#include <bin_sem_asus.h>

#include "networkmap.h"

#define MAX_SUBNET_SCAN		(24) /* 23..24 */
#define NUM_CLIENTS_SCAN	(1U << (32U - MAX_SUBNET_SCAN))
#define MAX_CLIENT_ITEMS	4096

static NET_CLIENT_LIST net_clients;
static unsigned char my_hwaddr[8];
static struct in_addr my_ipaddr;
static struct in_addr my_ipmask;

volatile unsigned int scan_now = 0;
static unsigned int scan_max = NUM_CLIENTS_SCAN - 1;
static unsigned int scan_net = 0xC0A80A00;
static volatile int scan_block = 0;
static volatile int networkmap_fullscan = 1;
static volatile int daemon_exit = 0;

/******** Build ARP Socket Function *********/
static int arp_sockfd = -1;
static struct sockaddr_ll src_sockll, dst_sockll;

static int
iface_get_id(int fd, const char *device)
{
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
		perror("iface_get_id ERR:\n");
		return -1;
	}

	return ifr.ifr_ifindex;
}

/*
 *  Bind the socket associated with FD to the given device.
 */
static int
iface_bind(int fd, int ifindex)
{
	int err, flags, alen;
	socklen_t errlen = sizeof(err);

	memset(&src_sockll, 0, sizeof(src_sockll));
	src_sockll.sll_family = AF_PACKET;
	src_sockll.sll_ifindex = ifindex;
	src_sockll.sll_protocol = htons(ETH_P_ARP);

	flags = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flags, sizeof(flags));

	if (bind(fd, (struct sockaddr *) &src_sockll, sizeof(src_sockll)) == -1) {
		perror("bind device ERR:");
		return -1;
	}

	/* Any pending errors, e.g., network is down? */
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
		perror("getsockopt");
		return -2;
	}

	if (err > 0)
		return -2;

	alen = sizeof(src_sockll);
	if (getsockname(fd, (struct sockaddr*)&src_sockll, &alen) == -1) {
		perror("getsockname");
		return -2;
	}

	if (src_sockll.sll_halen == 0) {
		printf("Interface is not ARPable (no ll address)\n");
		return -2;
	}

	dst_sockll = src_sockll;

	memset(dst_sockll.sll_addr, 0xFF, sizeof(dst_sockll.sll_addr));

	return 0;
}

static int
create_socket(const char *device)
{
	/* create socket */
	int sock_fd, device_id;

	sock_fd = socket(PF_PACKET, SOCK_DGRAM, 0);
	if(sock_fd < 0) {
		perror("create socket ERR:");
		return -1;
	}

	device_id = iface_get_id(sock_fd, device);
	if (iface_bind(sock_fd, device_id) < 0) {
		close(sock_fd);
		sock_fd = -1;
		printf("iface_bind ERROR\n");
	}

	return sock_fd;
}

static int
sent_arppacket(int raw_sockfd, struct in_addr *dst_ip)
{
	char raw_buffer[46];
	ARP_HEADER* arp;

	memset(dst_sockll.sll_addr, 0xFF, sizeof(dst_sockll.sll_addr));  // set dmac addr FF:FF:FF:FF:FF:FF
	bzero(raw_buffer, sizeof(raw_buffer));

	// Allow 14 bytes for the ethernet header
	arp = (ARP_HEADER *)raw_buffer;
	arp->hardware_type = htons(ARPHRD_ETHER);
	arp->protocol_type = htons(ETH_P_IP);
	arp->hwaddr_len = 6;
	arp->ipaddr_len = 4;
	arp->message_type = htons(ARPOP_REQUEST);

	// My hardware address and IP addresses
	memcpy(arp->source_hwaddr, my_hwaddr, 6);
	memcpy(arp->source_ipaddr, &my_ipaddr, 4);

	// Destination hwaddr and dest IP addr
	memset(arp->dest_hwaddr, 0xFF, 6);
	memcpy(arp->dest_ipaddr, dst_ip, 4);

	if ((sendto(raw_sockfd, raw_buffer, sizeof(raw_buffer), 0, (struct sockaddr *)&dst_sockll, sizeof(dst_sockll))) < 0) {
		perror("sendto");
		return 1;
	}

	NMP_DEBUG("sent_arppacket --> %s\n", inet_ntoa(*dst_ip));

	return 0;
}

/******* End of Build ARP Socket Function ********/

static void
net_clients_release(void)
{
	NET_CLIENT *item, *next;

	SLIST_FOREACH_SAFE(item, &net_clients.head, entry, next) {
		free(item);
	}

	SLIST_INIT(&net_clients.head);
	net_clients.count = 0;
}

static void
clear_resources(void)
{
	if (arp_sockfd > 0) {
		close(arp_sockfd);
		arp_sockfd = -1;
	}

	net_clients_release();

	nvram_set_int_temp("networkmap_fullscan", 0);
	remove("/var/run/networkmap.pid");
}

/*********** Signal functions **************/

static void
sig_refresh(int sig)
{
	networkmap_fullscan = 1;
	scan_now = 0;
}

static void
sig_exit(int sig)
{
	daemon_exit = 1;
	clear_resources();

	exit(0);
}

static void
net_clients_reset(void)
{
	FILE *fp;
	int lock;

	// reset exist ip table
	lock = file_lock("networkmap");

	net_clients_release();

	fp = fopen("/tmp/static_ip.inf", "w");
	if (fp)
		fclose(fp);

	fp = fopen("/tmp/static_ip.num", "w");
	if (fp) {
		fprintf(fp, "%u", 0);
		fclose(fp);
	}

	file_unlock(lock);

	nvram_set_int_temp("networkmap_fullscan", 1);
}

static void
net_clients_update(void)
{
	FILE *fp;
	NET_CLIENT *item;
	int lock;
	struct in_addr in;
	unsigned int vcount;

	lock = file_lock("networkmap");

	vcount = 0;
	fp = fopen("/tmp/static_ip.inf", "w");
	if (fp) {
		SLIST_FOREACH(item, &net_clients.head, entry) {
			if (item->macval) {
				in.s_addr = item->ip_addr;
				
				if (!item->staled)
					vcount++;
				
				fprintf(fp, "%s,%02X:%02X:%02X:%02X:%02X:%02X,%s,%d,%d,%d\n",
					inet_ntoa(in),
					item->mac_addr[0], item->mac_addr[1], item->mac_addr[2],
					item->mac_addr[3], item->mac_addr[4], item->mac_addr[5],
					item->device_name,
					item->type,
					item->http,
					item->staled);
			}
		}
		
		fclose(fp);
	}

	fp = fopen("/tmp/static_ip.num", "w");
	if (fp) {
		fprintf(fp, "%u", vcount);
		fclose(fp);
	}

	file_unlock(lock);
}

static int
is_same_subnet(struct in_addr *ip1, struct in_addr *ip2, struct in_addr *msk)
{
	unsigned int mask = ntohl(msk->s_addr);

	return ((ntohl(ip1->s_addr) & mask) == (ntohl(ip2->s_addr) & mask)) ? 1 : 0;
}

static NET_CLIENT *
lookup_client(unsigned int ip_addr)
{
	NET_CLIENT *item;

	SLIST_FOREACH(item, &net_clients.head, entry) {
		if (item->ip_addr == ip_addr)
			return item;
	}

	item = NULL;

	/* item not found, try create */
	if (net_clients.count < MAX_CLIENT_ITEMS) {
		item = malloc(sizeof(*item));
		if (item) {
			memset(item, 0, sizeof(*item));
			item->ip_addr = ip_addr;
			SLIST_INSERT_HEAD(&net_clients.head, item, entry);
			net_clients.count++;
		}
	}

	return item;
}

static void
lookup_static_dhcp_list(struct in_addr *dst_ip, NET_CLIENT* pnet_client)
{
	int i, i_max;
	struct in_addr src_ip;
	char nvram_ip[32], nvram_name[32], *sname;

	if (nvram_get_int("dhcp_static_x") != 1)
		return;

	i_max  = nvram_get_int("dhcp_staticnum_x");
	if (i_max > 64) i_max = 64;

	for (i = 0; i < i_max; i++) {
		sprintf(nvram_ip, "dhcp_staticip_x%d", i);
		if (!inet_aton(nvram_safe_get(nvram_ip), &src_ip))
			continue;
		
		sprintf(nvram_name, "dhcp_staticname_x%d", i);
		sname = nvram_safe_get(nvram_name);
		if (src_ip.s_addr == dst_ip->s_addr && is_valid_hostname(sname)) {
			strncpy(pnet_client->device_name, sname, 18);
			pnet_client->device_name[18] = 0;
			break;
		}
	}
}

static void
fixup_hostname(NET_CLIENT* pnet_client)
{
	int i;
	char *hname = (char *)pnet_client->device_name;
	char *p = hname;

	for (i = 0; i < 18; i++) {
		if (*p < 0x20)
			*p = 0x0;
		p++;
	}

	hname[18] = '\0';
	trim_r(hname);
}

static int
resolve_hostname(struct in_addr *dst_ip, NET_CLIENT *pnet_client)
{
	struct sockaddr_in dst_saddr;
	char hname[NI_MAXHOST];

	memset(&dst_saddr, 0, sizeof(dst_saddr));
	memcpy(&dst_saddr.sin_addr, dst_ip, sizeof(struct in_addr));
	dst_saddr.sin_family = AF_INET;

	bzero(hname, sizeof(hname));
	if (getnameinfo((struct sockaddr *)&dst_saddr, sizeof(dst_saddr),
			 hname, sizeof(hname), NULL, 0, NI_NAMEREQD|NI_NOFQDN) != 0)
		return -1;

	strncpy(pnet_client->device_name, hname, 18);
	fixup_hostname(pnet_client);

	if (!pnet_client->device_name[0])
		return -1;

	return 0;
}

static void
arp_proc_fetch(void)
{
	FILE *fp;
	NET_CLIENT *item;
	char buffer[256], arp_ip[16], arp_if[16];
	struct in_addr src_addr;
	unsigned int arp_flags;

	fp = fopen("/proc/net/arp", "r");
	if (fp) {
		// skip first line
		fgets(buffer, sizeof(buffer), fp);
		
		while (fgets(buffer, sizeof(buffer), fp)) {
			arp_flags = 0;
			if (sscanf(buffer, "%15s %*s 0x%x %*s %*s %15s", arp_ip, &arp_flags, arp_if) == 3) {
				if ((arp_flags & 0x02) && strcmp(arp_if, IFNAME_BR) == 0 && inet_aton(arp_ip, &src_addr)) {
					if (src_addr.s_addr != my_ipaddr.s_addr && is_same_subnet(&src_addr, &my_ipaddr, &my_ipmask)) {
						item = lookup_client(src_addr.s_addr);
						if (item) {
							item->macval = 0;
							item->staled = 0;
							item->probed = 1;
							item->pending = 0;
							item->skip_ping = 255; // next ping after ~2s
						}
					}
				}
			}
		}
		
		fclose(fp);
	}
}

static void
nmap_init(void)
{
	unsigned int lan_pool;
	struct timeval arp_timeout = {2, 0};

	scan_block = 1;

	if (!ether_atoe(nvram_safe_get("lan_hwaddr"), my_hwaddr))
		goto error_exit;

	if (!inet_aton(nvram_safe_get("lan_ipaddr_t"), &my_ipaddr))
		goto error_exit;

	if (!inet_aton(nvram_safe_get("lan_netmask_t"), &my_ipmask))
		goto error_exit;

	scan_net = ntohl(my_ipaddr.s_addr) & ntohl(my_ipmask.s_addr);
	lan_pool = ~(ntohl(my_ipmask.s_addr));

	if (lan_pool < NUM_CLIENTS_SCAN) {
		scan_max = lan_pool;
		scan_block = 0;
		arp_timeout.tv_sec = 0;
		arp_timeout.tv_usec = 60000;
	} else {
		arp_proc_fetch();
	}

error_exit:

	setsockopt(arp_sockfd, SOL_SOCKET, SO_RCVTIMEO, &arp_timeout, sizeof(arp_timeout));
}

static int
nmap_ping_hosts_again(void)
{
	NET_CLIENT *item;
	int need_update_file = 0;
	struct in_addr in;

	SLIST_FOREACH(item, &net_clients.head, entry) {
		if (item->staled)
			continue;
		
		in.s_addr = item->ip_addr;
		if (!is_same_subnet(&in, &my_ipaddr, &my_ipmask))
			continue;
		
		if (item->pending > 2) {
			NMP_DEBUG("address: %s is expired!!!\n", inet_ntoa(in));
			
			item->staled = 1;
			item->probed = 0;
			item->pending = 0;
			item->skip_ping = 0;
			need_update_file = 1;
			continue;
		}
		
		item->skip_ping++;
		
		if (!item->skip_ping) {
			item->pending++;
			item->skip_ping = 224; // next ping after ~60s
			
			sent_arppacket(arp_sockfd, &in);
		}
	}

	return need_update_file;
}

static void
nmap_receive_arp(void)
{
	char buffer[64] = {0};
	struct in_addr src_addr;
	int arp_count, nmap_changed, need_update_file;
	NET_CLIENT *item;
	ARP_HEADER *arp_ptr;

	arp_count = 0;
	need_update_file = 0;

	while (!daemon_exit)
	{
		int recvsize;
		nmap_changed = 0;
		
		recvsize = recvfrom(arp_sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
		if (recvsize < (int)(sizeof(ARP_HEADER)))
			break;
		
		/* prevent arp storm deadlock */
		arp_count++;
		if (arp_count > 1024) {
			usleep(10000);
			break;
		}
		
		arp_ptr = (ARP_HEADER*)buffer;
		
		NMP_DEBUG("Receive ARP (%d)! Type: %X, HLEN: %d, PLEN: %d, SPA: %d.%d.%d.%d, SHA: %02X:%02X:%02X:%02X:%02X:%02X, TPA: %d.%d.%d.%d, THA: %02X:%02X:%02X:%02X:%02X:%02X\n",
			recvsize,
			ntohs(arp_ptr->message_type),
			arp_ptr->hwaddr_len,
			arp_ptr->ipaddr_len,
			arp_ptr->source_ipaddr[0],arp_ptr->source_ipaddr[1],
			arp_ptr->source_ipaddr[2],arp_ptr->source_ipaddr[3],
			arp_ptr->source_hwaddr[0],arp_ptr->source_hwaddr[1],
			arp_ptr->source_hwaddr[2],arp_ptr->source_hwaddr[3],
			arp_ptr->source_hwaddr[4],arp_ptr->source_hwaddr[5],
			arp_ptr->dest_ipaddr[0],arp_ptr->dest_ipaddr[1],
			arp_ptr->dest_ipaddr[2],arp_ptr->dest_ipaddr[3],
			arp_ptr->dest_hwaddr[0],arp_ptr->dest_hwaddr[1],
			arp_ptr->dest_hwaddr[2],arp_ptr->dest_hwaddr[3],
			arp_ptr->dest_hwaddr[4],arp_ptr->dest_hwaddr[5]
			);
		
		memcpy(&src_addr, arp_ptr->source_ipaddr, 4);
		
		// Check valid source IP
		if (src_addr.s_addr == INADDR_ANY || src_addr.s_addr == INADDR_NONE)
			continue;
		
		// Check own source IP
		if (src_addr.s_addr == my_ipaddr.s_addr)
			continue;
		
		// Check ARP packet if source ip and router ip at the same network
		if (!is_same_subnet(&src_addr, &my_ipaddr, &my_ipmask))
			continue;
		
		// ARP Response packet to router
		if( ntohs(arp_ptr->message_type) == ARPOP_REPLY &&
			!memcmp(arp_ptr->dest_ipaddr, &my_ipaddr, 4) &&	// dest IP
			!memcmp(arp_ptr->dest_hwaddr, my_hwaddr, 6) )	// dest MAC
		{
			NMP_DEBUG("   It's ARP Response Packet!\n");
			
			item = lookup_client(src_addr.s_addr);
			if (!item)
				continue;
			
			item->pending = 0;
			item->skip_ping = (unsigned char)(random() % 31);
			
			if (memcmp(arp_ptr->source_hwaddr, item->mac_addr, 6)) {
				memcpy(item->mac_addr, arp_ptr->source_hwaddr, 6);
				item->macval = 1;
				nmap_changed = 1;
			}
			
			if (item->probed) {
				item->probed = 0;
				nmap_changed = 1;
			}
			
			if (item->staled) {
				item->staled = 0;
				nmap_changed = 1;
			}
			
			if (nmap_changed || !item->device_name[0]) {
				if (resolve_hostname(&src_addr, item) == 0)
					need_update_file |= 1;
			}
			
			if (nmap_changed) {
				// Set unknown type
				item->type = 6;
				item->http = 0;
				
				// Find all application
				find_all_app(&my_ipaddr, &src_addr, item);
				fixup_hostname(item);
				if (!item->device_name[0])
					lookup_static_dhcp_list(&src_addr, item);
				
				need_update_file |= 1;
			}
			
		} else if (!networkmap_fullscan) {
			// Find a new IP! Send an ARP request to it
			
			item = lookup_client(src_addr.s_addr);
			if (!item)
				continue;
			
			if (!item->macval || item->staled) {
				NMP_DEBUG("   New IP: %s\n", inet_ntoa(src_addr));
				item->staled = 0;
				item->probed = 1;
				item->pending = 0;
				item->skip_ping = 254; // next ping after ~4s
			}
		}
	}

	if (!daemon_exit && !networkmap_fullscan)
		need_update_file |= nmap_ping_hosts_again();

	if (!daemon_exit && need_update_file)
		net_clients_update();
}

static void
nmap_iterate(void)
{
	if (networkmap_fullscan) {
		unsigned int scan_addr;
		
		if (scan_now == 0) {
			net_clients_reset();
			nmap_init();
		}
		
		scan_now++;
		scan_addr = scan_net | scan_now;
		
		// Skip our address
		if (scan_addr == ntohl(my_ipaddr.s_addr)) {
			scan_now++;
			scan_addr = scan_net | scan_now;
		}
		
		if (scan_now < scan_max && !scan_block) {
			struct in_addr in;
			
			in.s_addr = htonl(scan_addr);
			sent_arppacket(arp_sockfd, &in);
		} else {
			struct timeval arp_timeout = {2, 0};
			setsockopt(arp_sockfd, SOL_SOCKET, SO_RCVTIMEO, &arp_timeout, sizeof(arp_timeout));
			
			networkmap_fullscan = 0;
			
			nvram_set_int_temp("networkmap_fullscan", 0);
			
			if (!scan_block) {
				NMP_DEBUG("fullscan complete!\n");
			}
		}
	}

	nmap_receive_arp();
}

/******************************************/

int main(int argc, char *argv[])
{
	FILE *fp;
	int c, no_daemon = 0, do_wait = 0;

	SLIST_INIT(&net_clients.head);
	net_clients.count = 0;

	// usage: networkmap [-w] [-b]
	if (argc) {
		while ((c = getopt(argc, argv, "bw")) != -1) {
			switch (c) {
			case 'b':
				no_daemon = 1;
				break;
			case 'w':
				do_wait = 1;
				break;
			}
		}
	}

	// create UDP socket and bind to "br0" to get ARP packet//
	arp_sockfd = create_socket(IFNAME_BR);
	if (arp_sockfd < 0) {
		clear_resources();
		exit(errno);
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGUSR1, sig_refresh); //catch UI refresh signal
	signal(SIGTERM, sig_exit);

	if (!no_daemon) {
		if (daemon(0, 0) < 0) {
			perror("daemon");
			clear_resources();
			exit(errno);
		}
	}

	fp = fopen("/var/run/networkmap.pid", "w");
	if(fp){
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	scan_now = 0;
	networkmap_fullscan = 1;

	if (do_wait)
		sleep(5);

	nmap_init();

	while (!daemon_exit)
		nmap_iterate();

	clear_resources();

	return 0;
}

