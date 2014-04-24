#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <netdb.h>

#include "networkmap.h"

#include <shutils.h>
#include <netutils.h>
#include <nvram/bcmnvram.h>
#include <bin_sem_asus.h>

NET_CLIENT net_clients[256];
unsigned char my_hwaddr[8];
struct in_addr my_ipaddr;
struct in_addr my_ipmask;

unsigned int scan_max = 255;
unsigned int scan_now = 0;
int networkmap_fullscan = 1;
int daemon_exit = 0;
int scan_block = 0;
int arp_sockfd = -1;

/******** Build ARP Socket Function *********/
struct sockaddr_ll src_sockll, dst_sockll;

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

int create_socket(char *device)
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

int sent_arppacket(int raw_sockfd, struct in_addr *dst_ip)
{
	char raw_buffer[46];
	ARP_HEADER* arp;

	memset(dst_sockll.sll_addr, 0xFF, sizeof(dst_sockll.sll_addr));  // set dmac addr FF:FF:FF:FF:FF:FF
	bzero(raw_buffer, sizeof(raw_buffer));

	// Allow 14 bytes for the ethernet header
	arp = (ARP_HEADER *)raw_buffer;
	arp->hardware_type = htons(DIX_ETHERNET);
	arp->protocol_type = htons(IP_PACKET);
	arp->hwaddr_len = 6;
	arp->ipaddr_len = 4;
	arp->message_type = htons(ARP_REQUEST);

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

long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);
	return info.uptime;
}

void clear_resources()
{
	if (arp_sockfd > 0) {
		close(arp_sockfd);
		arp_sockfd = -1;
	}

	nvram_set_int_temp("networkmap_fullscan", 0);
	remove("/var/run/networkmap.pid");
}

/*********** Signal functions **************/

static void sig_refresh(int sig)
{
	networkmap_fullscan = 1;
	scan_now = 0;
}

static void sig_exit(int sig)
{
	daemon_exit = 1;

	clear_resources();

	exit(0);
}

void net_clients_reset()
{
	FILE *fp;
	int lock;

	// reset exist ip table
	memset(net_clients, 0, sizeof(net_clients));

	lock = file_lock("networkmap");

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

void net_clients_update()
{
	FILE *fp;
	int i, lock;
	struct in_addr in;
	unsigned int vcount;

	vcount = 0;

	lock = file_lock("networkmap");

	fp = fopen("/tmp/static_ip.inf", "w");
	if (fp) {
		for (i=1; i<255; i++) {
			if (net_clients[i].ip_addr && net_clients[i].macval) {
				in.s_addr = net_clients[i].ip_addr;
				
				if (!net_clients[i].staled)
					vcount++;
				
				fprintf(fp, "%s,%02X:%02X:%02X:%02X:%02X:%02X,%s,%d,%d,%d\n",
					inet_ntoa(in),
					net_clients[i].mac_addr[0], net_clients[i].mac_addr[1],
					net_clients[i].mac_addr[2], net_clients[i].mac_addr[3],
					net_clients[i].mac_addr[4], net_clients[i].mac_addr[5],
					net_clients[i].device_name,
					net_clients[i].type,
					net_clients[i].http,
					net_clients[i].staled);
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

static void
fixup_hostname(NET_CLIENT* pnet_client)
{
	int i;
	char *hname = (char *)pnet_client->device_name;
	char *p = hname;

	for (i = 0; i < 17; i++)
	{
		if (*p < 0x20)
			*p = 0x0;
		p++;
	}

	hname[17] = '\0';
	trim_r(hname);
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
		if (src_ip.s_addr == dst_ip->s_addr && is_valid_hostname(sname))
		{
			strncpy(pnet_client->device_name, sname, 17);
			break;
		}
	}
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

	strncpy(pnet_client->device_name, hname, 17);
	fixup_hostname(pnet_client);

	if (!pnet_client->device_name[0])
		return -1;

	return 0;
}

static void
nmap_init(void)
{
	unsigned int lan_nm;
	struct timeval arp_timeout = {2, 0};

	scan_block = 1;

	if (!ether_atoe(nvram_safe_get("lan_hwaddr"), my_hwaddr))
		goto error_exit;

	if (!inet_aton(nvram_safe_get("lan_ipaddr_t"), &my_ipaddr))
		goto error_exit;

	if (!inet_aton(nvram_safe_get("lan_netmask_t"), &my_ipmask))
		goto error_exit;

	lan_nm = ntohl(my_ipmask.s_addr);

	if ((lan_nm >> 8) == 0x00ffffff) {
		scan_max = ~(lan_nm) & 0xFF;
		scan_block = 0;
		arp_timeout.tv_sec = 0;
		arp_timeout.tv_usec = 60000;
	}

error_exit:

	setsockopt(arp_sockfd, SOL_SOCKET, SO_RCVTIMEO, &arp_timeout, sizeof(arp_timeout));
}

static int
is_same_subnet(struct in_addr *ip1, struct in_addr *ip2, struct in_addr *msk)
{
	unsigned int mask = ntohl(msk->s_addr);

	return ((ntohl(ip1->s_addr) & mask) == (ntohl(ip2->s_addr) & mask)) ? 1 : 0;
}

static int
nmap_ping_hosts_again(void)
{
	int i, need_update_file = 0;
	struct in_addr in;

	for (i=1; i<255; i++) {
		if (!net_clients[i].ip_addr || net_clients[i].staled)
			continue;
		
		in.s_addr = net_clients[i].ip_addr;
		if (!is_same_subnet(&in, &my_ipaddr, &my_ipmask))
			continue;
		
		if (net_clients[i].pending > 2) {
			NMP_DEBUG("address: %s is expired!!!\n", inet_ntoa(in));
			
			net_clients[i].staled = 1;
			net_clients[i].probed = 0;
			net_clients[i].pending = 0;
			net_clients[i].skip_ping = 0;
			need_update_file = 1;
			continue;
		}
		
		net_clients[i].skip_ping++;
		
		if (!net_clients[i].skip_ping) {
			net_clients[i].pending++;
			net_clients[i].skip_ping = 224; // next ping after ~60s
			
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
	int arp_count, ip_index, nmap_changed, need_update_file;
	unsigned int nm_mask;
	ARP_HEADER *arp_ptr;

	arp_count = 0;
	need_update_file = 0;
	nm_mask = ~(ntohl(my_ipmask.s_addr)) & 0xFF;

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
		
		// Check ARP packet if source ip and router ip at the same network
		if (!is_same_subnet(&src_addr, &my_ipaddr, &my_ipmask))
			continue;
		
		// Check valid source IP
		ip_index = (int)(ntohl(src_addr.s_addr) & nm_mask);
		if (ip_index < 1 || ip_index > 254)
			continue;
		
		// ARP Response packet to router
		if( ntohs(arp_ptr->message_type) == ARP_RESPONSE &&
			!memcmp(arp_ptr->dest_ipaddr, &my_ipaddr, 4) &&	// dest IP
			!memcmp(arp_ptr->dest_hwaddr, my_hwaddr, 6) )	// dest MAC
		{
			NMP_DEBUG("   It's ARP Response Packet!\n");
			
			net_clients[ip_index].pending = 0;
			net_clients[ip_index].skip_ping = (unsigned char)(random() % 31);
			
			if (memcmp(arp_ptr->source_hwaddr, net_clients[ip_index].mac_addr, 6)) {
				memcpy(net_clients[ip_index].mac_addr, arp_ptr->source_hwaddr, 6);
				net_clients[ip_index].macval = 1;
				nmap_changed = 1;
			}
			
			if (net_clients[ip_index].ip_addr != src_addr.s_addr) {
				net_clients[ip_index].ip_addr = src_addr.s_addr;
				nmap_changed = 1;
			}
			
			if (net_clients[ip_index].probed) {
				net_clients[ip_index].probed = 0;
				nmap_changed = 1;
			}
			
			if (net_clients[ip_index].staled) {
				net_clients[ip_index].staled = 0;
				nmap_changed = 1;
			}
			
			if (nmap_changed || !net_clients[ip_index].device_name[0]) {
				if (resolve_hostname(&src_addr, &net_clients[ip_index]) == 0)
					need_update_file |= 1;
			}
			
			if (nmap_changed) {
				// Set unknown type
				net_clients[ip_index].type = 6;
				net_clients[ip_index].http = 0;
				
				// Find all application
				find_all_app(&my_ipaddr, &src_addr, &net_clients[ip_index]);
				fixup_hostname(&net_clients[ip_index]);
				if (!net_clients[ip_index].device_name[0])
					lookup_static_dhcp_list(&src_addr, &net_clients[ip_index]);
				
				need_update_file |= 1;
			}
			
		} else if (src_addr.s_addr != my_ipaddr.s_addr && !networkmap_fullscan) {
			// Find a new IP! Send an ARP request to it
			if (!net_clients[ip_index].ip_addr || net_clients[ip_index].staled) {
				NMP_DEBUG("   New IP: %s\n", inet_ntoa(src_addr));
				net_clients[ip_index].ip_addr = src_addr.s_addr;
				net_clients[ip_index].staled = 0;
				net_clients[ip_index].probed = 1;
				net_clients[ip_index].pending = 0;
				net_clients[ip_index].skip_ping = 254; // next ping after ~4s
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
			nmap_init();
			net_clients_reset();
		}
		
		scan_addr = (ntohl(my_ipaddr.s_addr) & ntohl(my_ipmask.s_addr));
		
		scan_now++;
		scan_addr |= scan_now;
		
		// Skip our address
		if (scan_addr == ntohl(my_ipaddr.s_addr)) {
			scan_now++;
			scan_addr |= scan_now;
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
			
			if (!scan_block)
				NMP_DEBUG("fullscan complete!\n");
		}
	}

	if (!scan_block)
		nmap_receive_arp();
	else
		pause();
}


/******************************************/

int main(int argc, char *argv[])
{
	FILE *fp;
	int c, no_daemon = 0, do_wait = 0;

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

