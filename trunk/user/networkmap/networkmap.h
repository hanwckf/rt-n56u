
#include <include/bsd_queue.h>

// Service Ports
#define HTTP_PORT	80
#define NBNS_PORT	137
#define NBSS_PORT	139
#define LPD_PORT	515
#define MDNS_PORT	5353
#define RAW_PORT	9100

#ifdef DEBUG
 #define NMP_DEBUG(fmt, args...) printf(fmt, ## args)
#else
 #define NMP_DEBUG(fmt, args...)
#endif

#ifdef DEBUG_MORE
 #define NMP_DEBUG_M(fmt, args...) printf(fmt, ## args)
#else
 #define NMP_DEBUG_M(fmt, args...)
#endif

typedef struct net_client {
	SLIST_ENTRY(net_client) entry;
	unsigned long ip_addr;
	unsigned char mac_addr[6];
	unsigned char skip_ping;
	unsigned char type;
	unsigned char http:1;
	unsigned char staled:1;
	unsigned char probed:1;
	unsigned char macval:1;
	unsigned char pending:4;
	char          device_name[19];
} NET_CLIENT, *PNET_CLIENT;

typedef struct net_client_list {
	SLIST_HEAD(, net_client) head;
	unsigned int count;
} NET_CLIENT_LIST, *PNET_CLIENT_LIST;

// walf test
typedef struct
{
	unsigned short  hardware_type;
	unsigned short  protocol_type;
	unsigned char hwaddr_len;
	unsigned char ipaddr_len;
	unsigned short  message_type;
	unsigned char source_hwaddr[6];
	unsigned char source_ipaddr[4];
	unsigned char dest_hwaddr[6];
	unsigned char dest_ipaddr[4];
} ARP_HEADER;

typedef struct
{
	unsigned short transaction_id;
	unsigned char flags[2];
	unsigned short questions;
	unsigned short answer_rrs;
	unsigned short authority_rrs;
	unsigned short additional_rrs;
	unsigned char name[34];
	unsigned short type;
	unsigned short name_class;
	unsigned char ttl[4];
	unsigned short data_len;
	unsigned char number_of_names;
	unsigned char device_name1[16];
	unsigned char name_flags1[2];
	unsigned char device_name2[16];
	unsigned char name_flags2[2];
	unsigned char device_name3[16];
	unsigned char name_flags3[2];
	unsigned char device_name4[16];
	unsigned char name_flags4[2];
	unsigned char device_name5[16];
	unsigned char name_flags5[2];
	unsigned char device_name6[16];
	unsigned char name_flags6[2];
	unsigned char mac_addr[6];
	unsigned char name_info[58];
} NBNS_RESPONSE;


void find_all_app(struct in_addr *src_ip, struct in_addr *dst_ip, NET_CLIENT *pnet_client);
