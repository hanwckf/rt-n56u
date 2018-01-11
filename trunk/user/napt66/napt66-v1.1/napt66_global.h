#define SNAT 1
#define RSNAT 0

#define HASHRANGE 3540
#define HASHNUMBER 3533
#define CLEANUP_TIME 20

struct hash_entry;

struct conn_entry{
	u_int8_t proto;
	int proto_offset;
	struct hash_entry* source;//引用该连接项在source_table的hash链节
	struct hash_entry* ipproto;//引用该连接项在ipproto_table的hash链节
	struct in6_addr lan_ipv6;
	union{
		u_int16_t port;
		u_int16_t id;
	}lan_l4_info;
	struct in6_addr wan_ipv6;
	union{
		u_int16_t port;
		u_int16_t id;
	}wan_l4_info;
	int time;
	u_int16_t sub_sum;
	
	u_int16_t dport;
	//eprt_len_change如果不为0，表示为eprt命令之后的控制连接
	int eprt_len_change;
	//sum_change用于累计每次修改EPRT之后的变化值
	int sum_change;
	
};
#define lan_port lan_l4_info.port
#define lan_id lan_l4_info.id
#define wan_port wan_l4_info.port
#define wan_id wan_l4_info.id

struct hash_entry{
	struct hash_entry* next;
	struct hash_entry* prev;
	struct conn_entry* conntrack_entry;
};
