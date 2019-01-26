#include "napt66_ftp_alg.h"


#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif

#define NS_INT16SZ      2       /* #/bytes of data in a u_int16_t */

extern long time(void* ptr);
extern struct conn_entry* hash_search_ct(int direc,struct conn_entry* p_entry);
extern struct conn_entry* get_free_ct(struct conn_entry* pkt_entry);


static int getbits(const char *src, int *bitsp) 
{
	static const char digits[] = "0123456789";
	int n;
	int val;
	char ch;

	val = 0;
	n = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		pch = strchr(digits, ch);
		if (pch != NULL) {
			if (n++ != 0 && val == 0)       /* no leading zeros */
				return 0;
			val *= 10;
			val += (pch - digits);
			if (val > 128)                  /* range */
				return 0;
			continue;
		}
		return 0;
	}
	if (n == 0)
		    return 0;
	*bitsp = val;
	return 1;
}


/*将字符串类型IPv6地址更改为in6_addr结构体类型*/
static int inet_net_pton_ipv6(const char *src,struct in6_addr *dst, size_t size) 
{
	static const char xdigits_l[] = "0123456789abcdef",
		              xdigits_u[] = "0123456789ABCDEF";
	u_char tmp[INET6_ADDRSTRLEN], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, saw_xdigit;
	u_int val;
	int digits;
	int bits;
	size_t bytes;
	int words;
	int ipv4;

	memset((tp = tmp), '\0', INET6_ADDRSTRLEN);
	endp = tp + INET6_ADDRSTRLEN;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			goto enoent;
	curtok = src;
	saw_xdigit = 0;
	val = 0;
	digits = 0;
	bits = -1;
	ipv4 = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
			if (pch != NULL) {
			   val <<= 4;
			   val |= (pch - xdigits);
			   if (++digits > 4)
			           goto enoent;
			   saw_xdigit = 1;
			   continue;
			}
			if (ch == ':') {
				curtok = src;
				if (!saw_xdigit) {
					if (colonp)
						goto enoent;
					colonp = tp;
					continue;
				} else if (*src == '\0')
						goto enoent;
				if (tp + NS_INT16SZ > endp)
						return (0);
				*tp++ = (u_char) (val >> 8) & 0xff;
				*tp++ = (u_char) val & 0xff;
				saw_xdigit = 0;
				digits = 0;
				val = 0;
				continue;
			}
			if (ch == '.' && ((tp + INET6_ADDRSTRLEN) <= endp)) {
				tp += INET6_ADDRSTRLEN;
				saw_xdigit = 0;
				ipv4 = 1;
				break;  /* '\0' was seen by inet_pton4(). */
			}
			if (ch == '/' && getbits(src, &bits) > 0)
				break;
			goto enoent;
	}
	if (saw_xdigit) {
		if (tp + NS_INT16SZ > endp)
			goto enoent;
		*tp++ = (u_char) (val >> 8) & 0xff;
		*tp++ = (u_char) val & 0xff;
	}
	if (bits == -1)
		bits = 128;

	words = (bits + 15) / 16;
	if (words < 2)
		words = 2;
	if (ipv4)
		words = 8;
	endp =  tmp + 2 * words;

	if (colonp != NULL) {
		const int n = tp - colonp;
		int i;

		if (tp == endp)
			goto enoent;
		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		goto enoent;

	bytes = (bits + 7) / 8;
	if (bytes > size)
		goto emsgsize;
	memcpy(dst, tmp, bytes);
	return (bits);

	enoent:
	return (-1);

	emsgsize:
	return (-1);
}

/*将in6_addr结构体类型IPv6地址更改为字符串类型*/
static char *inet_net_ntop_ipv6(const u_char *src, int bits, char *dst, size_t size) 
{
	u_int   m;
	int     b;
	int     p;
	int     zero_s, zero_l, tmp_zero_s, tmp_zero_l;
	int     i;
	int     is_ipv4 = 0;
	unsigned char inbuf[16];
	char outbuf[sizeof("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:255.255.255.255/128")];
	char    *cp;
	int     words;
	u_char  *s;

	cp = outbuf;

	if(bits == 0){
		*cp++ = ':';
		*cp++ = ':';
		*cp = '\0';
	} 
	else{
		/* Copy src to private buffer.  Zero host part. */
		p = (bits + 7) / 8;
		memcpy(inbuf, src, p);
		memset(inbuf + p, 0, 16 - p);
		b = bits % 8;
		if (b != 0) {
			m = ~0 << (8 - b);
			inbuf[p-1] &= m;
		}

		s = inbuf;

		/* how many words need to be displayed in output */
		words = (bits + 15) / 16;
		if(words == 1)
			words = 2;

		/* Find the longest substring of zero's */
		zero_s = zero_l = tmp_zero_s = tmp_zero_l = 0;
		for (i = 0; i < (words * 2); i += 2) {
			if ((s[i] | s[i+1]) == 0) {
				if (tmp_zero_l == 0)
					tmp_zero_s = i / 2;
				tmp_zero_l++;
			} 
			else{
				if (tmp_zero_l && zero_l < tmp_zero_l) {
					zero_s = tmp_zero_s;
					zero_l = tmp_zero_l;
					tmp_zero_l = 0;
				}
			}
		}

		if (tmp_zero_l && zero_l < tmp_zero_l) {
			zero_s = tmp_zero_s;
			zero_l = tmp_zero_l;
		}

		if (zero_l != words && zero_s == 0 && ((zero_l == 6) ||
		  ((zero_l == 5 && s[10] == 0xff && s[11] == 0xff) ||
		  ((zero_l == 7 && s[14] != 0 && s[15] != 1)))))
				is_ipv4 = 1;

		/* Format whole words. */
		for (p = 0; p < words; p++) {
			if (zero_l != 0 && p >= zero_s && p < zero_s + zero_l) {
				/* Time to skip some zeros */
				if (p == zero_s)
					*cp++ = ':';
				if (p == words - 1)
					*cp++ = ':';
				s++;
				s++;
				continue;
			}

			if (is_ipv4 && p > 5 ) {
				*cp++ = (p == 6) ? ':' : '.';
				cp += SPRINTF((cp, "%u", *s++));
				/* we can potentially drop the last octet */
				if (p != 7 || bits > 120) {
					*cp++ = '.';
					cp += SPRINTF((cp, "%u", *s++));
				}
			} 
			else{
				if(cp != outbuf)
					*cp++ = ':';
				cp += SPRINTF((cp, "%x", *s * 256 + s[1]));
				s += 2;
			}
		}
	}

	if (strlen(outbuf) + 1 > size)
		    goto emsgsize;
	strcpy(dst, outbuf);

	return (dst);

	emsgsize:
	return (NULL);
}

/*对EPRT命令的提取分析以及修改*/
int analysis_eprt(struct sk_buff *skb,struct conn_entry *entry)
{
	struct ipv6hdr* ipv6_header;
	struct tcphdr* tcp_header;
	u_int16_t eprt_old_port = 0;
	struct in6_addr eprt_old_addr;
	char ftp_ptr[200];

	
	struct conn_entry new_entry;
	struct conn_entry* n_entry;
	
	char eprt_new_command[100];
	char eprt_new_addr[INET6_ADDRSTRLEN];
	char eprt_new_port[10];
	
	u_int16_t eprt_old_len;
	u_int16_t eprt_new_len;
	u_int16_t temp_port;
	
	int i = 0,j = 0,k = 0;
	char port[10];
	char ip6_address[INET6_ADDRSTRLEN] = "0";

	ipv6_header = ipv6_hdr(skb);
	tcp_header = (struct tcphdr*)((char*)ipv6_header + entry->proto_offset);
	
	eprt_old_len = ntohs(ipv6_header->payload_len) - (int)(tcp_header->doff * 4);
	
	/*是否存在FTP命令*/
	if(eprt_old_len > 0){	
		memcpy(ftp_ptr,(unsigned char *)tcp_header + (tcp_header->doff * 4),eprt_old_len);
			
		/*如果不是EPRT命令，则返回0*/
		if(strncmp(ftp_ptr,"EPRT",4) != 0) 
			return 0;
		/*提取EPRT命令*/
		else {	
		//printk("eprt cmd before nat is:%s\n",ftp_ptr);
			i = 8;
			while(ftp_ptr[i] != '|'){
				ip6_address[j] = ftp_ptr[i];
				j++;
				i++;
			}
			i++;
			while(ftp_ptr[i] != '|'){
				port[k] = ftp_ptr[i];
				k++;
				i++;
			}

			/*将字符串地址改为主机字节序,并且保存端口号 即pton*/
			inet_net_pton_ipv6(ip6_address,&eprt_old_addr,sizeof(struct in6_addr));
			/*将字符型端口号转换为长整数*/
			eprt_old_port = simple_strtol(port,NULL,eprt_old_port);	
		}
	}
	else {
		/*不存在FTP命令*/
		return 2;
	}
	
	/*通过分析的EPRT命令建立新的连接表项，处理之后由服务器发起的数据连接*/
	new_entry.proto = IPPROTO_TCP;
	new_entry.lan_ipv6 = eprt_old_addr;
	new_entry.lan_port = htons(eprt_old_port);	

	n_entry = hash_search_ct(SNAT,&new_entry);
	if(n_entry == NULL){
		n_entry = get_free_ct(&new_entry);
	}
		
	n_entry->proto_offset = 40;				
	n_entry->time = time(NULL);		

	/*
		将端口号由整数类型转换为字符串形式
		在转换为字符串之前需要将n_entry表项中的网络字节序转换为主机字节序
	*/
	temp_port = ntohs(n_entry->wan_port);
	//printk("entry wan port is %d\n",n_entry->wan_port);	
		
	/*ntop函数结果为相应的主机序字符串，所以之前并不需要额外的字节序转换*/
	inet_net_ntop_ipv6((char *)(&(n_entry->wan_ipv6)),128,eprt_new_addr,128);
	snprintf(eprt_new_port,sizeof(eprt_new_port),"%d",temp_port);					

	eprt_new_len = sprintf(eprt_new_command,"EPRT |2|%s|%s|\r\n",eprt_new_addr,eprt_new_port);

	/*保存每次EPRT长度变化的累计值*/
	entry->sum_change += entry->eprt_len_change;
	entry->eprt_len_change = eprt_new_len - eprt_old_len;
	
	//printk("eprt len change is %d\n",entry->eprt_len_change);

	if(entry->eprt_len_change > 0){
	/*skb_put和skb_trim函数对skb长度进行调整*/
		skb_put(skb,entry->eprt_len_change);
	}
	else if(entry->eprt_len_change < 0){
		skb_trim(skb,skb->len + entry->eprt_len_change);
	}

	/*构造新的EPRT命令报文*/
	memcpy((unsigned char *)tcp_header + (tcp_header->doff * 4),eprt_new_command,eprt_new_len);
	
//	printk("new eprt cmd is:%s\n",eprt_new_command);
	return 1;				
	
}

