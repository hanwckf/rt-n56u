#include "napt66_conntrack.h"

#define MAX_ID 60000
#define MIN_ID 2048

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
#define LINUX2635
#endif

extern char *wan_if;
extern struct in6_addr wan_ipv6;
extern int hash_add_entry(struct conn_entry* p_entry);
extern struct conn_entry* hash_search_ct(int direc,struct conn_entry* p_entry);
extern long time(void* ptr);
extern bool inet6_addr_flag;

int in_cksum(u_int16_t *addr, int len)
{
	int sum;
	sum = 0;

	while (len > 1){
   	sum += *addr++;
      len -= 2;
   }
   if (len == 1){
      sum += htons(*(unsigned char *)addr << 8);
   }
   
   return (sum);
}


int find_conflict(struct conn_entry* pkt_entry)
{
	struct conn_entry* p_entry;
	//查一下wan端口是否已被占用。虽然新建连接是在snat中完成的，但查冲突的参数是rsnat
	p_entry = hash_search_ct(RSNAT,pkt_entry);
	
	if(p_entry == NULL)
		return 0;
	else
		return 1;
}

int get_random_id(void){
	u16 rnd;
	do{
		get_random_bytes(&rnd,sizeof(rnd));
	}while(rnd < MIN_ID || rnd > MAX_ID);

	return rnd;
}

//获得一个可用表项，包含了可用的ＷＡＮ信息
struct conn_entry* get_free_ct(struct conn_entry* pkt_entry)
{
	int status;
	u_int32_t sub_sum = 0;
	struct conn_entry* p_new_entry;

	pkt_entry->wan_ipv6 = wan_ipv6;
	
	//在连接数比较少时，内网id已被占用的机率很小。	
	pkt_entry->wan_id = pkt_entry->lan_id;
	
	while(ntohs(pkt_entry->wan_id) < MIN_ID){
		pkt_entry->wan_id = get_random_id();
	}
	while(find_conflict(pkt_entry)){
		pkt_entry->wan_id = get_random_id();
	}
	//获得了不冲突的L4PROTO ID

	//获取校验和差值，填充表项
	switch(pkt_entry->proto)
	{
		case IPPROTO_TCP:
		case IPPROTO_UDP:
			sub_sum = in_cksum((u_int16_t *)&pkt_entry->wan_ipv6,16) + in_cksum((u_int16_t *)&pkt_entry->wan_port,2)
										-in_cksum((u_int16_t *)&pkt_entry->lan_ipv6,16) - in_cksum((u_int16_t *)&pkt_entry->lan_port,2);
		break;
		case IPPROTO_ICMPV6:				
			sub_sum = in_cksum((u_int16_t *)&pkt_entry->wan_ipv6,16) + in_cksum((u_int16_t *)&pkt_entry->wan_id,2)
										-in_cksum((u_int16_t *)&pkt_entry->lan_ipv6,16) - in_cksum((u_int16_t *)&pkt_entry->lan_id,2);
		break;
		default:
		break;
	}

	if(sub_sum >> 31 != 0){//负数
		sub_sum--;
	}

	while(sub_sum >> 16){
		sub_sum = (sub_sum & 0xffff) + (sub_sum >> 16);
	}
	pkt_entry->sub_sum = sub_sum;
	pkt_entry->eprt_len_change = 0;
	pkt_entry->sum_change = 0;

//kcalloc(1,sizeof(struct in6_addr),GFP_KERNEL);//元素个数，元素大小，内存类型
	p_new_entry = (struct conn_entry*)kcalloc(1,sizeof(struct conn_entry),GFP_KERNEL);	
	memcpy(p_new_entry,pkt_entry,sizeof(struct conn_entry));

	status = hash_add_entry(p_new_entry);
	if(status == 0){
		printk(KERN_INFO "hash_add_entry(p_new_entry) failed\n");
		return NULL;
	}
	
	return p_new_entry;
}

//根据报文方向填充连接信息。成功返回1，不成功返回0
int conn_init(struct sk_buff *skb,struct conn_entry* entry,int direc)
{
	struct ipv6hdr* ip6_h = ipv6_hdr(skb);
	struct ipv6hdr* pl_ipv6_header;
	struct ipv6_opt_hdr* ip6e_h;
	struct udphdr* udp_h;
	struct tcphdr* pl_tcp_header;
	struct icmp6hdr* icmpv6_h;
	struct icmp6hdr* pl_icmpv6_header;
	u_int8_t proto;
	u_int16_t hlen;

	entry->time = 0;
		
	hlen = sizeof(struct ipv6hdr);
	proto = ip6_h->nexthdr;//uint8_t  ip6_un1_nxt;	
	
	/*关键一步：对分片数据包进行重组*/
  	if (0 != skb_linearize(skb)) {
   	return NF_ACCEPT;
   }
	
	
	while(1){
		switch (proto) {
			case IPPROTO_HOPOPTS:
			case IPPROTO_ROUTING:
			case IPPROTO_DSTOPTS:
				ip6e_h = (struct ipv6_opt_hdr*)((char *)ip6_h + hlen);
				
				proto = ip6e_h->nexthdr;
				hlen += ip6e_h->hdrlen * 8;//u_int8_t ip6e_len;
				//printk(KERN_INFO "发现扩展报头\n");
				break;

			case IPPROTO_FRAGMENT:
				//printf("Find fragment.\n");
				return 0;
				break;

			case IPPROTO_UDP://报头端口位置相同
			case IPPROTO_TCP:
				entry->proto = proto;	
				udp_h = (struct udphdr*)((char *)ip6_h + hlen);			
				entry->proto_offset = hlen;//记下四层协议报头的偏移值（基于IPv6报头）	
				
				if(direc == SNAT){//填充LAN信息
					entry->lan_ipv6 = ip6_h->saddr;
#ifdef __FAVOR_BSD
					entry->lan_port = udp_h->uh_sport;
					entry->dport = udp_h->uh_dport;
#else
					entry->lan_port = udp_h->source;
					entry->dport = udp_h->dest;
#endif

				}
				else{//填充WAN信息
					entry->wan_ipv6 = ip6_h->daddr;
#ifdef __FAVOR_BSD
					entry->wan_port = udp_h->uh_dport;
#else
					entry->wan_port = udp_h->dest;
#endif			
				}
				
				return 1;
				break;
			case IPPROTO_ICMPV6:
				entry->proto = proto;
				
				icmpv6_h = (struct icmp6hdr*)((char *)ip6_h + hlen);
				entry->proto_offset = hlen;//记下四层协议报头的偏移值（基于IPv6报头）
				if((icmpv6_h->icmp6_type) != ICMPV6_ECHO_REQUEST
					&& (icmpv6_h->icmp6_type) != ICMPV6_ECHO_REPLY
					&& (icmpv6_h->icmp6_type) != ICMPV6_DEST_UNREACH
					&& (icmpv6_h->icmp6_type) != ICMPV6_PKT_TOOBIG
					&& (icmpv6_h->icmp6_type) != ICMPV6_TIME_EXCEED
					&& (icmpv6_h->icmp6_type) != ICMPV6_PARAMPROB){
					//分别处理四种错误报文
					//printf("ICMPv6 type can't be deal with.\n");
					return 0;
				}
				
				if(direc == SNAT){
					entry->lan_ipv6 = ip6_h->saddr;
					entry->lan_id = icmpv6_h->icmp6_identifier;
				}
				else{
				
				/*针对上面的类型，决定怎么填入id字段*/
				/*echo正常填入，否则填入负载中的id字段*/
				/*问题：如果是tcp等其他协议，对表项设置为tcp*/
					
					/*正确报文，正常传输*/
					if(icmpv6_h->icmp6_type == ICMPV6_ECHO_REQUEST
					||	icmpv6_h->icmp6_type == ICMPV6_ECHO_REPLY){
						entry->wan_ipv6 = ip6_h->daddr;
						entry->wan_id = icmpv6_h->icmp6_identifier;//id	
					}
					/*负载中如果是ICMPv6，则将负载中的源地址和id填入wan表项*/
					/*负载如果是tcp/udp，则将负载中的源地址和端口填入wan表项*/
					else {
						pl_ipv6_header = (struct ipv6hdr *)((char *)icmpv6_h + 8);
						if(pl_ipv6_header->nexthdr == IPPROTO_ICMPV6){
							pl_icmpv6_header = (struct icmp6hdr*)((char *)pl_ipv6_header + 40);
							entry->wan_ipv6 = pl_ipv6_header->saddr;
							entry->wan_id = pl_icmpv6_header->icmp6_identifier;
						}
						else if(pl_ipv6_header->nexthdr == IPPROTO_TCP){
							pl_tcp_header = (struct tcphdr*)((char *)pl_ipv6_header + 40);
							entry->wan_ipv6 = pl_ipv6_header->saddr;
							entry->wan_port = pl_tcp_header->source;
							entry->proto = IPPROTO_TCP;
						}
						else if(pl_ipv6_header->nexthdr == IPPROTO_UDP){
							pl_tcp_header = (struct tcphdr*)((char *)pl_ipv6_header + 40);
							entry->wan_ipv6 = pl_ipv6_header->saddr;
							entry->wan_port = pl_tcp_header->source;
							entry->proto = IPPROTO_UDP;
						}
						else {
							return 0;
						}
					}
				}				
				
				return 1;
				break;
			default:
				//printf("Unknown protocol.\n");
				return 0;
				break;
		}//switch(proto)
		
	}//while(1)
}

/*

*/

#ifdef LINUX2635
/*kernel>=2.6.35*/
/*Modified by woshiyuhao0819@gmail.com to support the kernel 2.6.35 or upper*/
int get_ip6_by_name(char* if_name,struct in6_addr* p_ipv6)
{
	struct net_device *dev;
	struct net *net = NULL;
	struct socket *sock;
	struct inet6_dev *in_dev6;
	struct inet6_ifaddr *ifa6 = NULL;
	struct list_head* list_head_ipv6 = NULL;
	struct list_head* temp;
	sock_create_kern(PF_INET6, SOCK_DGRAM, 0,&sock);
	net = sock_net((const struct sock *)sock->sk);
	dev = dev_get_by_name(net,if_name);
	
	in_dev6 = (struct inet6_dev *)dev->ip6_ptr;
	
	while (in_dev6){
		temp=&(in_dev6->addr_list);
		list_for_each(list_head_ipv6,temp)
		{
			ifa6=list_entry(list_head_ipv6,struct inet6_ifaddr,if_list);
			if(ifa6)
				if(ifa6->scope == IPV6_ADDR_ANY){
					(*p_ipv6) = ifa6->addr;//addr即为eth0接口的v6地址
					dev_put(dev);
					return 1;
				}
		}
	in_dev6 = in_dev6->next;
	}

	dev_put(dev);  
	return 0;
}
#else
/*other kernel*/
int get_ip6_by_name(char* if_name,struct in6_addr* p_ipv6)
{
	struct net_device *dev;
	struct net *net = NULL;
	struct socket *sock;
	struct inet6_dev *in_dev6;
	struct inet6_ifaddr *ifa6 = NULL;

	sock_create_kern(PF_INET6, SOCK_DGRAM, 0,&sock);
	net = sock_net((const struct sock *)sock->sk);
	dev = dev_get_by_name(net,if_name);
	
	in_dev6 = (struct inet6_dev *)dev->ip6_ptr;
	
	while (in_dev6){
    	ifa6 = in_dev6->addr_list;
      while (ifa6){			
			if(ifa6->scope == IPV6_ADDR_ANY){	
				(*p_ipv6) = ifa6->addr;//addr即为eth0接口的v6地址
				dev_put(dev); 
				return 1;
			}
         ifa6 = ifa6->if_next;
		}      
		in_dev6 = in_dev6->next;
	}   
	dev_put(dev);  
	return 0;
}
#endif

//获取连接记录返回1
int get_entry(struct sk_buff *skb,struct conn_entry** pp_entry,int direc)
{
	struct conn_entry pkt_entry;//临时变量
	struct conn_entry* p_entry;
	int status;
	*pp_entry = NULL;	

	if(inet6_addr_flag == false){
		status = get_ip6_by_name(wan_if,&wan_ipv6);
		if(status <= 0){
			printk(KERN_INFO "get_ip6_by_name() failed.\n");
			return 0;
		}
		printk(KERN_INFO "get_ip6_by_name() succeed.\n");
		inet6_addr_flag = true;
	}

	status = conn_init(skb,&pkt_entry,direc);
	if(status == 0)
		return 0;//不能处理的ＩＣＭＰＶ6包类型或其他类型，Ａccept，让协议栈自己处理。

	p_entry = hash_search_ct(direc,&pkt_entry);

	if(direc == SNAT){
		if(p_entry == NULL){//ＳＮＡＴ找不到连接，新建一个
			p_entry = get_free_ct(&pkt_entry);	
			if(p_entry == NULL){//ＳＮＡＴ获取新连接失败
				//printf("ＳＮＡＴ获取新连接失败\n");
				return -1;//连接找不到可用端口外发数据，这种情况很难发生。
			}
		}
	}
	else{//RSNAT
		if(p_entry == NULL){//RＳＮＡＴ找不到连接，丢弃包
			//printf("RＳＮＡＴ找不到连接，丢弃包\n");
			return -1;//数据包连接无记录，不被接受。
		}		
	}
	//time()的内核ＡＰＩ
	p_entry->time = time(NULL);
	*pp_entry = p_entry;
	return 1;
}
