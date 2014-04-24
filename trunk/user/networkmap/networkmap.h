
// Hardware type field in ARP message
#define DIX_ETHERNET            1
// Type number field in Ethernet frame
#define IP_PACKET               0x0800
#define ARP_PACKET              0x0806
#define RARP_PACKET             0x8035
// Message type field in ARP messages
#define ARP_REQUEST             1
#define ARP_RESPONSE            2
#define RARP_REQUEST            3
#define RARP_RESPONSE           4
#define RCV_TIMEOUT             2 //sec
#define MAXDATASIZE             512
#define LPR                     0x02
#define LPR_RESPONSE            0x00

// Service Port
#define HTTP_PORT               80
#define NBNS_PORT               137
#define NBSS_PORT		139
#define LPD_PORT                515
#define MDNS_PORT               5353
#define RAW_PORT                9100

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

typedef struct {
	unsigned long ip_addr;
	unsigned char mac_addr[6];
	char          device_name[18];
	unsigned char type;
	unsigned char http;
	unsigned char skip_ping;
	unsigned char staled:1;
	unsigned char probed:1;
	unsigned char macval:1;
	unsigned char pending:5;


} NET_CLIENT, *PNET_CLIENT;

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
