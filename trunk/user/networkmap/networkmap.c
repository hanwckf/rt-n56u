#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <shutils.h>    // for eval()
#include <bcmnvram.h>
#include "networkmap.h"
#include "endianness.h"
#include <semaphore_mfp.h>
#include <sys/sysinfo.h>

NET_CLIENT net_clients[256];
unsigned char my_hwaddr[6];
unsigned char my_ipaddr[4];

int daemon_exit = 0;
int networkmap_fullscan = 1;
int scan_count = 0;
int arp_sockfd = -1;

/******** Build ARP Socket Function *********/
struct sockaddr_ll src_sockll, dst_sockll;

static int
iface_get_id(int fd, const char *device)
{
        struct ifreq    ifr;
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
        int                     err, flags;
        socklen_t               errlen = sizeof(err);
        memset(&src_sockll, 0, sizeof(src_sockll));
        src_sockll.sll_family          = AF_PACKET;
        src_sockll.sll_ifindex         = ifindex;
        src_sockll.sll_protocol        = htons(ETH_P_ARP);

        flags = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flags, sizeof(flags));

        if (bind(fd, (struct sockaddr *) &src_sockll, sizeof(src_sockll)) == -1) {
                perror("bind device ERR:\n");
                return -1;
        }

        /* Any pending errors, e.g., network is down? */
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
                return -2;
        }
        if (err > 0) {
                return -2;
        }

        int alen = sizeof(src_sockll);
        if (getsockname(fd, (struct sockaddr*)&src_sockll, &alen) == -1) {
                perror("getsockname");
                exit(2);
        }

        if (src_sockll.sll_halen == 0) {
                printf("Interface is not ARPable (no ll address)\n");
                exit(2);
        }

	dst_sockll = src_sockll;

         return 0;
}

int create_socket(char *device)
{
        /* create socket */
        int sock_fd, device_id;
        sock_fd = socket(PF_PACKET, SOCK_DGRAM, 0); //2008.06.27 Yau change to UDP Socket
        if(sock_fd < 0)
                perror("create socket ERR:");
        device_id = iface_get_id(sock_fd, device);
        if (device_id == -1)
               printf("iface_get_id REEOR\n");
        if ( iface_bind(sock_fd, device_id) < 0)
                printf("iface_bind ERROR\n");
        return sock_fd;
}

int  sent_arppacket(int raw_sockfd, unsigned char * dst_ipaddr)
{
        ARP_HEADER* arp;
        char raw_buffer[46];

        memset(dst_sockll.sll_addr, 0xFF, sizeof(dst_sockll.sll_addr));  // set dmac addr FF:FF:FF:FF:FF:FF
        bzero(raw_buffer, sizeof(raw_buffer));

        // Allow 14 bytes for the ethernet header
        arp = (ARP_HEADER *)(raw_buffer);// + 14);
        arp->hardware_type = htons(DIX_ETHERNET);
        arp->protocol_type = htons(IP_PACKET);
        arp->hwaddr_len = 6;
        arp->ipaddr_len = 4;
        arp->message_type = htons(ARP_REQUEST);

        // My hardware address and IP addresses
        memcpy(arp->source_hwaddr, my_hwaddr, 6);
        memcpy(arp->source_ipaddr, my_ipaddr, 4);

        // Destination hwaddr and dest IP addr
        memset(arp->dest_hwaddr, 0xFF, 6);
        memcpy(arp->dest_ipaddr, dst_ipaddr, 4);

        if( (sendto(raw_sockfd, raw_buffer, sizeof(raw_buffer), 0, (struct sockaddr *)&dst_sockll, sizeof(dst_sockll))) < 0 )
        {
                 perror("sendto");
                 return 1;
        }

        return 0;
}

/******* End of Build ARP Socket Function ********/

long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);
	return info.uptime;
}


static int
is_invalid_char_for_hostname(char c)
{
	int ret = 0;

	if (c < 0x20)
		ret = 1;
	else if (c >= 0x21 && c <= 0x2c)
		ret = 1;
	else if (c >= 0x2e && c <= 0x2f)
		ret = 1;
	else if (c >= 0x3a && c <= 0x40)
		ret = 1;
#if 0
	else if (c >= 0x5b && c <= 0x60)
		ret = 1;
#else
	else if (c >= 0x5b && c <= 0x5e)
		ret = 1;
	else if (c == 0x60)
		ret = 1;
#endif
	else if (c >= 0x7b)
		ret = 1;

	return ret;
}

static int
is_valid_hostname(const char *name)
{
	int ret = 1, len, i;

	if (!name)
		return 0;

	len = strlen(name);
	if (len == 0)
		return 0;

	for (i = 0; i < len ; i++)
		if (is_invalid_char_for_hostname(name[i]))
		{
			ret = 0;
			break;
		}

	return ret;
}

/* remove space in the end of string */
void trim_r(char buf[18])
{
	int i;
	char *p = (char *) buf;
	
	i = strlen(buf);
	
	while (i >= 1)
	{
		if (*(p+i-1) == ' ' || *(p+i-1) == 0x0a || *(p+i-1) == 0x0d)
			*(p+i-1)=0x0;
		else
			break;
		i--;
	}
}

void fixstr(char buf[18])
{
	int i;
	char *p = (char *) buf;
	buf[17] = '\0';
	
	for (i = 0; i < 17; i++)
	{
		if (*p < 0x20)
			*p = 0x0;
		p++;
	}
	
	if (is_valid_hostname(buf))
		trim_r(buf);
	else
		buf[0] = '\0';
}

void clear_resources()
{
	if (arp_sockfd > 0) {
		close(arp_sockfd);
		arp_sockfd =-1;
	}
	
	nvram_set("networkmap_fullscan", "0");
	remove("/var/run/networkmap.pid");
}

/*********** Signal functions **************/

static void sig_refresh(int sig)
{
	networkmap_fullscan = 1;
	scan_count = 0;
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
	char timestampstr[32];
	
	// reset exist ip table
	memset(net_clients, 0x00, sizeof(net_clients));
	
	sprintf(timestampstr, "%lu", uptime());
	
	spinlock_lock(SPINLOCK_Networkmap);
	
	// clear file;
	fp = fopen("/tmp/static_ip.inf", "w");
	if (fp)
		close(fp);
	
	nvram_set("fullscan_timestamp", timestampstr);
	nvram_set("networkmap_fullscan", "1");
	
	spinlock_unlock(SPINLOCK_Networkmap);
}

void net_clients_update()
{
	int i;
	FILE *fp;
	struct in_addr in;
	char *hostname;
	
	spinlock_lock(SPINLOCK_Networkmap);
	
	// clear file and fill IP/MAC info
	fp = fopen("/tmp/static_ip.inf", "w");
	if (fp)
	{
		for (i=1; i<255; i++)
		{
			if (net_clients[i].ip_addr)
			{
				in.s_addr = net_clients[i].ip_addr;
				hostname = inet_ntoa(in);
				
				fprintf(fp, "%s,%02X:%02X:%02X:%02X:%02X:%02X,%s,%d,%d,%d,%d\n",
					hostname,
					net_clients[i].mac_addr[0], net_clients[i].mac_addr[1],
					net_clients[i].mac_addr[2], net_clients[i].mac_addr[3],
					net_clients[i].mac_addr[4], net_clients[i].mac_addr[5],
					net_clients[i].device_name,
					net_clients[i].type, net_clients[i].http, net_clients[i].printer, net_clients[i].itune);
			}
		}
		
		fclose(fp);
	}
	
	spinlock_unlock(SPINLOCK_Networkmap);
}


/******************************************/

int main(int argc, char *argv[])
{
	int arp_getlen;
	int ip_index, need_update_file;
	struct sockaddr_in router_addr;
	char router_ipaddr[17], router_mac[17], buffer[512];
	unsigned char scan_ipaddr[4]; // scan ip
	ARP_HEADER * arp_ptr;
	struct timeval arp_timeout;
	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGUSR1, sig_refresh); //catch UI refresh signal
	signal(SIGTERM, sig_exit);
	
	// Get Router's IP/Mac
	strcpy(router_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	strcpy(router_mac, nvram_safe_get("lan_hwaddr"));
	inet_aton(router_ipaddr, &router_addr.sin_addr);
	memcpy(my_ipaddr,  &router_addr.sin_addr, 4);
	
	// Prepare scan 
	memset(scan_ipaddr, 0x00, 4);
	memcpy(scan_ipaddr, &router_addr.sin_addr, 3);
	
	if (strlen(router_mac)!=0) ether_atoe(router_mac, my_hwaddr);
	
	// create UDP socket and bind to "br0" to get ARP packet//
	arp_sockfd = create_socket(INTERFACE);
	if(arp_sockfd < 0)
	{
		perror("create socket ERROR");
		clear_resources();
		exit(errno);
	}
	
	if (daemon(0, 0) < 0) {
		perror("daemon");
		clear_resources();
		exit(errno);
	}
	
	FILE *fp = fopen("/var/run/networkmap.pid", "w");
	if(fp){
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	
	dst_sockll = src_sockll;
	memset(dst_sockll.sll_addr, 0xFF, sizeof(dst_sockll.sll_addr));
	
	scan_count = 0;
	networkmap_fullscan = 1;
	
	while(!daemon_exit)
	{
		if (networkmap_fullscan == 1)
		{
			if (scan_count == 0)
			{
				// 50 ms timeout
				arp_timeout.tv_sec = 0;
				arp_timeout.tv_usec = 50000;
				setsockopt(arp_sockfd, SOL_SOCKET, SO_RCVTIMEO, &arp_timeout, sizeof(arp_timeout));
				
				scan_ipaddr[3] = 0;
				
				net_clients_reset();
			}
			
			scan_count++;
			scan_ipaddr[3]++;
			
			// Skip our address
			if (!memcmp(scan_ipaddr, my_ipaddr, 4))
			{
				scan_count++;
				scan_ipaddr[3]++;
			}
			
			if( scan_ipaddr[3] < 255 )
			{
				sent_arppacket(arp_sockfd, scan_ipaddr);
				
				NMP_DEBUG("sent_arppacket to: %d.%d.%d.%d\n", scan_ipaddr[0], scan_ipaddr[1], scan_ipaddr[2], scan_ipaddr[3]);
			}
			else
			{
				arp_timeout.tv_sec = 2;
				arp_timeout.tv_usec = 0;
				setsockopt(arp_sockfd, SOL_SOCKET, SO_RCVTIMEO, &arp_timeout, sizeof(arp_timeout));
				networkmap_fullscan = 0;
				
				nvram_set("networkmap_fullscan", "0");
			}
		}
		
		while(1)
		{
			need_update_file = 0;
			
			arp_getlen = recvfrom(arp_sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
			if (arp_getlen < (int)sizeof(ARP_HEADER))
			{
				break;
			}
			
			arp_ptr = (ARP_HEADER*)(buffer);
			
			NMP_DEBUG("Receive ARP! HLEN: %d, PLEN: %d, SPA: %d.%d.%d.%d, SHA: %02X:%02X:%02X:%02X:%02X:%02X, TPA: %d.%d.%d.%d, THA: %02X:%02X:%02X:%02X:%02X:%02X\n",
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
			
			// Check ARP packet if source ip and router ip at the same network
			if( memcmp(my_ipaddr, arp_ptr->source_ipaddr, 3) )
			{
				continue;
			}
			
			// Check valid source IP
			ip_index = (int)arp_ptr->source_ipaddr[3];
			if (ip_index < 1 || ip_index > 254)
			{
				continue;
			}
			
			swapbytes16(arp_ptr->message_type);
			
			// ARP Response packet to router
			if( arp_ptr->message_type == 0x02 &&   		       	// ARP response
				!memcmp(arp_ptr->dest_ipaddr, my_ipaddr, 4) && 	// dest IP
				!memcmp(arp_ptr->dest_hwaddr, my_hwaddr, 6) ) 	// dest MAC
			{
				NMP_DEBUG("   It's ARP Response Packet!\n");
				
				if ( memcmp(arp_ptr->source_hwaddr, net_clients[ip_index].mac_addr, 6) )
				{
					memcpy(net_clients[ip_index].mac_addr, arp_ptr->source_hwaddr, 6);
					need_update_file = 1;
				}
				
				if ( !net_clients[ip_index].ip_addr )
				{
					memcpy(&net_clients[ip_index].ip_addr, arp_ptr->source_ipaddr, 4);
					need_update_file = 1;
				}
			}
			else
			{
				if ( !net_clients[ip_index].ip_addr )
				{
					// Find a new IP! Send an ARP request to it
					
					NMP_DEBUG("New IP\n");
					
					if( memcmp(my_ipaddr, arp_ptr->source_ipaddr, 4) )
					{
						sent_arppacket(arp_sockfd, arp_ptr->source_ipaddr);
					}
					else
					{
						NMP_DEBUG("New IP is the same as Router IP! Ignore it!\n");
					}
				}
			}
			
			if (need_update_file)
			{
				// Set unknown type
				net_clients[ip_index].type = 6;
				net_clients[ip_index].http = 0;
				
				// Find all application
				FindAllApp(my_ipaddr, arp_ptr->source_ipaddr, &net_clients[ip_index]);
				
				net_clients_update();
			}
		}
	}
	
	clear_resources();
	
	return 0;
}
