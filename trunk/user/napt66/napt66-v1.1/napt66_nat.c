#include "napt66_nat.h"


#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif

#define FTP_PORT 0x0015

extern long time(void* ptr);
extern struct conn_entry* hash_search_ct(int direc,struct conn_entry* p_entry);
extern int analysis_eprt(struct sk_buff *skb,struct conn_entry *entry);
extern int in_cksum(u_int16_t *addr, int len);
extern struct conn_entry* get_free_ct(struct conn_entry* pkt_entry);

/*
	第二个参数特意设置成32bit整数，以便与函数体内的32bit数协同运算时，正确处理符号位与进位。
	因为16bit整数的符号位与32bit整数的符号位不是对齐的。
*/
u_int16_t adjust_checksum(u_int16_t old_checksum,u_int32_t delta)
{
	u_int32_t new_checksum;

	new_checksum = old_checksum - delta;
	
	if(new_checksum >> 31 != 0){//负数
		new_checksum--;
	}

	while(new_checksum >> 16){
		new_checksum = (new_checksum & 0xffff) + (new_checksum >> 16);
	}

	return new_checksum;
}

u_int16_t calc_checksum(u_int16_t *addr,int length,u_int32_t sum)
{
/*	u_int32_t sum = 0;*/
	
	while(length > 0){
		sum += *addr++;
		length -= 2;
	}
	
	if(length > 0)
      sum += *(unsigned char *)addr;
		
	while(sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return(~sum);
}

struct in6_addr inet6_addr_ntohs(struct in6_addr *net)
{
	struct in6_addr addr6;
	int i;
	memcpy(&addr6,net,sizeof(struct in6_addr));	
	

	for(i=0;i<8;i++){
		addr6.s6_addr16[i] = ntohs(addr6.s6_addr16[i]);
	}
	
	return addr6;
}


struct in6_addr inet6_addr_htons(struct in6_addr *net)
{
	struct in6_addr addr6;
	int i;
	
	memcpy(&addr6,net,sizeof(struct in6_addr));	
	

	for(i=0;i<8;i++){
		addr6.s6_addr16[i] = htons(addr6.s6_addr16[i]);
	}
	
	return addr6;
}

int nat(struct sk_buff *skb,struct conn_entry* entry,int direc)
{
	/*
		POSTROUTING点为SNAT，direc为1
		PREROUTING点为RSNAT，direc为0
	*/
	struct ipv6hdr* ipv6_header;
	u_int8_t proto;
	u_int8_t pl_proto;
	struct tcphdr* tcp_header;
	struct udphdr* udp_header;
	struct icmp6hdr* icmpv6_header;
	struct ipv6hdr* pl_ipv6_header;
	struct tcphdr* pl_tcp_header;
	struct icmp6hdr* pl_icmpv6_header;
	
	u_int32_t sum;
	int len;
	
	/*定位IPv6头部*/	
	ipv6_header = ipv6_hdr(skb);		
	proto = ipv6_header->nexthdr;	
	
	/*SNAT*/
	if(direc == 1){
		memcpy(&(ipv6_header->saddr),&(entry->wan_ipv6),sizeof(struct in6_addr));		
		switch (proto){
			case IPPROTO_TCP:
				tcp_header = (struct tcphdr*)((char*)ipv6_header + entry->proto_offset);
				tcp_header->source = entry->wan_port;
				
				/*非FTP报文，正常处理*/
				if(entry->dport != htons(FTP_PORT)){
					tcp_header->check = adjust_checksum(tcp_header->check,entry->sub_sum);
				}
				
				/*EPRT报文之前的控制报文*/
				else if(entry->eprt_len_change == 0){
					if(analysis_eprt(skb,entry) == 1){

						//printk("old ipv6 len is %x\n",ipv6_header->payload_len);
						ipv6_header->payload_len = htons(ntohs(ipv6_header->payload_len) + entry->eprt_len_change);
										
						//printk("First EPRT\n");						
						len = htons(ipv6_header->payload_len);								
						tcp_header->check = 0;
						sum = in_cksum((u_int16_t *)&ipv6_header->saddr,32);
						sum += ntohs(IPPROTO_TCP + len);
						sum += in_cksum((u_int16_t *)tcp_header, len);
						tcp_header->check = CKSUM_CARRY(sum);	
						
						//printk("checksum done\n");	
						//printk("eprt out seq is %x\n",tcp_header->seq);				
					}			
					
					else {
						//tcp_header->check = adjust_checksum(tcp_header->check,entry->sub_sum);
						/*EPRT之前的控制报文*/						
						len = htons(ipv6_header->payload_len);
						//printk("eprt ck len is %x",len);
									
						tcp_header->check = 0;
						sum = in_cksum((u_int16_t *)&ipv6_header->saddr,32);
						sum += ntohs(IPPROTO_TCP + len);
						sum += in_cksum((u_int16_t *)tcp_header, len);
						tcp_header->check = CKSUM_CARRY(sum);							
					}
				}
				
				/*EPRT之后的控制报文*/
				else {
					/*修改EPRT报文之后，需要对后续数据包的SEQ字段进行调整*/
					tcp_header->seq = htonl(ntohl(tcp_header->seq) + entry->eprt_len_change + entry->sum_change);

					if(analysis_eprt(skb,entry) == 1){				
						ipv6_header->payload_len = htons(ntohs(ipv6_header->payload_len) + entry->eprt_len_change);
						//printk("Houxu EPRT\n");						
						//printk("new ipv6 len is %x\n",ipv6_header->payload_len);
						//printk("old checksum is %x\n",tcp_header->check);
					}
										
					len = htons(ipv6_header->payload_len);	
					tcp_header->check = 0;
					sum = in_cksum((u_int16_t *)&ipv6_header->saddr,32);
					sum += ntohs(IPPROTO_TCP + len);
					sum += in_cksum((u_int16_t *)tcp_header, len);
					tcp_header->check = CKSUM_CARRY(sum);	

				} 						
				break;
			case IPPROTO_UDP:
				udp_header = (struct udphdr*)((char*)ipv6_header + entry->proto_offset);
				udp_header->source = entry->wan_port;
				udp_header->check = adjust_checksum(udp_header->check,entry->sub_sum);	
				break;
			case IPPROTO_ICMPV6:
				icmpv6_header = (struct icmp6hdr*)((char*)ipv6_header + entry->proto_offset);
				icmpv6_header->icmp6_identifier = entry->wan_id;
				icmpv6_header->icmp6_cksum = adjust_checksum(icmpv6_header->icmp6_cksum,entry->sub_sum);
				break;
			default:
				break;
		}
	}
	
	/*RSNAT*/
	else {
		memcpy(&(ipv6_header->daddr),&(entry->lan_ipv6),sizeof(struct in6_addr));		
		switch (proto){
			case IPPROTO_TCP:				
				tcp_header = (struct tcphdr*)((char*)ipv6_header + entry->proto_offset);			
				tcp_header->dest = entry->lan_port;
							
				if(entry->eprt_len_change == 0){
					tcp_header->check = adjust_checksum(tcp_header->check,-entry->sub_sum);
				}
			
				else {
					/*对服务器返回数据包的ack进行调整*/
					//printk("old in ack is %x\n",tcp_header->ack_seq);
					tcp_header->ack_seq = htonl(ntohl(tcp_header->ack_seq) - entry->eprt_len_change - entry->sum_change);
					//printk("new in ack is %x\n",tcp_header->ack_seq);

					//printk("maybe bug start!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					//printk("eprt_len_change is %d,sum_change is %d\n",entry->eprt_len_change,entry->sum_change);					
					//printk("new in ipv6 len is %x\n",ipv6_header->payload_len);
					len = htons(ipv6_header->payload_len);						
												
					tcp_header->check = 0;
					sum = in_cksum((u_int16_t *)&ipv6_header->saddr,32);
					sum += ntohs(IPPROTO_TCP + len);
					sum += in_cksum((u_int16_t *)tcp_header, len);
					tcp_header->check = CKSUM_CARRY(sum);	
				}		
				break;
				
			case IPPROTO_UDP:
				udp_header = (struct udphdr*)((char*)ipv6_header + entry->proto_offset);
				udp_header->dest = entry->lan_port;
				udp_header->check = adjust_checksum(udp_header->check,-entry->sub_sum);
				break;
				
			case IPPROTO_ICMPV6:

				icmpv6_header = (struct icmp6hdr*)((char*)ipv6_header + entry->proto_offset);				
				
				/*ICMPv6消息报文，正常传输*/
				if(icmpv6_header->icmp6_type == ICMPV6_ECHO_REQUEST || icmpv6_header->icmp6_type == ICMPV6_ECHO_REPLY){
					icmpv6_header->icmp6_identifier = entry->lan_id; 
					icmpv6_header->icmp6_cksum = adjust_checksum(icmpv6_header->icmp6_cksum,-entry->sub_sum);
				}
				
				/*
					ICMPv6错误报文
				  	负载如果是ICMPv6协议，则将lan表项内容填入负载中的源地址和identifier字段
				  	负载如果是TCP/UDP协议，则将lan表项内容填入负载中的源地址和端口字段
				  	之后重新计算负载的校验和，填入负载的相应字段
				*/
				else {
					pl_ipv6_header = (struct ipv6hdr *)((char *)icmpv6_header + 8);

					memcpy(&(pl_ipv6_header->saddr),&(entry->lan_ipv6),sizeof(struct in6_addr));
					pl_proto = pl_ipv6_header->nexthdr;
					
					if(pl_proto == IPPROTO_ICMPV6){
						pl_icmpv6_header = (struct icmp6hdr*)((char *)pl_ipv6_header + 40);
						pl_icmpv6_header->icmp6_identifier = entry->lan_id;

						len = htons(pl_ipv6_header->payload_len);	
						pl_icmpv6_header->icmp6_cksum = 0;
				      sum = in_cksum((u_int16_t *)&pl_ipv6_header->saddr, 32);
				      sum += ntohs(IPPROTO_ICMPV6 + len);
				      sum += in_cksum((u_int16_t *)pl_icmpv6_header, len);
				      pl_icmpv6_header->icmp6_cksum = CKSUM_CARRY(sum);												
					}
					else if(pl_proto == IPPROTO_TCP || pl_proto == IPPROTO_UDP){
						pl_tcp_header = (struct tcphdr*)((char *)pl_ipv6_header + 40);
						pl_tcp_header->source = entry->lan_port;
						
						len = htons(pl_ipv6_header->payload_len);	
						pl_tcp_header->check = 0;
				      sum = in_cksum((u_int16_t *)&pl_ipv6_header->saddr, 32);
				      sum += ntohs(pl_proto + len);
				      sum += in_cksum((u_int16_t *)pl_tcp_header, len);
				      pl_tcp_header->check = CKSUM_CARRY(sum);
					}
						
					/*重新计算ICMPv6报文本身校验和*/
					
					len = htons(ipv6_header->payload_len);	
					
					icmpv6_header->icmp6_cksum = 0;
		         sum = in_cksum((u_int16_t *)&ipv6_header->saddr, 32);
		         sum += ntohs(IPPROTO_ICMPV6 + len);
		         sum += in_cksum((u_int16_t *)icmpv6_header, len);
		         icmpv6_header->icmp6_cksum = CKSUM_CARRY(sum);
				}			
				
				break;
			default:
				break;
		}
	}
	
	return 1;
}








