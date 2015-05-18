/* Kernel module to match a string into a packet.
 *
 * Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 * 
 * ChangeLog
 *	19.02.2002: Gianni Tedesco <gianni@ecsc.co.uk>
 *		Fixed SMP re-entrancy problem using per-cpu data areas
 *		for the skip/shift tables.
 *	02.05.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 *		Fixed kernel panic, due to overrunning boyer moore string
 *		tables. Also slightly tweaked heuristic for deciding what
 * 		search algo to use.
 * 	27.01.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 * 		Implemented Boyer Moore Sublinear search algorithm
 * 		alongside the existing linear search based on memcmp().
 * 		Also a quick check to decide which method to use on a per
 * 		packet basis.
 */

/* Kernel module to match a http header string into a packet.
 *
 * Copyright (C) 2003, CyberTAN Corporation
 * All Rights Reserved.
 *
 * Description:
 *   This is kernel module for web content inspection. It was derived from 
 *   'string' match module, declared as above.
 *
 *   The module follows the Netfilter framework, called extended packet 
 *   matching modules. 
 */

/* Linux Kernel 2.6 Port ( 2.4 ipt-> 2.6 xt)
 * Copyright (C) 2008, Ralink Technology Corporation. 
 * All Rights Reserved.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <net/ipv6.h>

extern unsigned int web_str_loaded;

#define BM_MAX_NLEN 256

#define BLK_JAVA		0x01
#define BLK_ACTIVE		0x02
#define BLK_COOKIE		0x04
#define BLK_PROXY		0x08

struct xt_webstr_info {
    char string[BM_MAX_NLEN];
    u_int16_t invert;
    u_int16_t len;
    u_int8_t type;
};

enum xt_webstr_type
{
    XT_WEBSTR_HOST,
    XT_WEBSTR_URL,
    XT_WEBSTR_CONTENT
};

typedef char *(*proc_xt_search_t) (char *, char *, int, int);

static proc_xt_search_t search;


#define split(word, wordlist, next, delim) \
    for (next = wordlist, \
	strncpy(word, next, sizeof(word)), \
	word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	next = next ? next + sizeof(delim) - 1 : NULL ; \
	strlen(word); \
	next = next ? : "", \
	strncpy(word, next, sizeof(word)), \
	word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	next = next ? next + sizeof(delim) - 1 : NULL)

/* Flags for get_http_info() */
#define HTTP_HOST	0x01
#define HTTP_URL	0x02
/* Flags for mangle_http_header() */
#define HTTP_COOKIE	0x04

//#define DEBUG

#ifdef DEBUG
#define SPARQ_LOG       printk
#else
#define SPARQ_LOG(format, args...)
#endif

#define BM_MAX_HLEN	(BM_MAX_NLEN*2)

typedef struct httpinfo {
    char url[BM_MAX_HLEN + 1];
    int hostlen;
    int urllen;
} httpinfo_t;

/* Return 1 for match, 0 for accept, -1 for partial. */
static int find_pattern2(const char *data, size_t dlen,
	const char *pattern, size_t plen,
	char term,
	unsigned int *numoff,
	unsigned int *numlen)
{
    size_t i, j, k;
    int state = 0;
    *numoff = *numlen = 0;

    SPARQ_LOG("%s: pattern = '%s', dlen = %u\n",__FUNCTION__, pattern, dlen);
    if (dlen == 0)
	return 0;

    if (dlen <= plen) {	/* Short packet: try for partial? */
	if (strnicmp(data, pattern, dlen) == 0)
	    return -1;
	else 
	    return 0;
    }
    for (i = 0; i <= (dlen - plen); i++) {
	/* DFA : \r\n\r\n :: 1234 */
	if (*(data + i) == '\r') {
	    if (!(state % 2)) state++;	/* forwarding move */
	    else state = 0;		/* reset */
	}
	else if (*(data + i) == '\n') {
	    if (state % 2) state++;
	    else state = 0;
	}
	else state = 0;

	if (state >= 4)
	    break;

	/* pattern compare */
	if (memcmp(data + i, pattern, plen ) != 0)
	    continue;

	/* Here, it means patten match!! */
	*numoff=i + plen;
	for (j = *numoff, k = 0; data[j] != term; j++, k++)
	    if (j > dlen) return -1 ;	/* no terminal char */

	*numlen = k;
	return 1;
    }
    return 0;
}

#if 0
static int mangle_http_header(unsigned char *data, unsigned int datalen, int flags)
{
    int found, offset, len;
    int ret = 0;

    /* Basic checking, is it HTTP packet? */
    if (datalen < 10)
	return ret;	/* Not enough length, ignore it */
    if (memcmp(data, "GET ", sizeof("GET ") - 1) != 0 &&
        memcmp(data, "POST ", sizeof("POST ") - 1) != 0 &&
        memcmp(data, "HEAD ", sizeof("HEAD ") - 1) != 0) //zg add 2006.09.28 for cdrouter3.3 item 186(cdrouter_urlfilter_15)
	return ret;	/* Pass it */

    /* COOKIE modification */
    if (flags & HTTP_COOKIE) {
	found = find_pattern2(data, datalen, "Cookie: ", 
		sizeof("Cookie: ")-1, '\r', &offset, &len);
	if (found) {
	    char c;
	    offset -= (sizeof("Cookie: ") - 1);
	    /* Swap the 2rd and 4th bit */
	    c = *(data + offset + 2) ;
	    *(data + offset + 2) = *(data + offset + 4) ;
	    *(data + offset + 4) = c ;
	    ret++;
	}
    }

    return ret;
}
#endif

static int get_http_info(unsigned char *data, unsigned int datalen,
			int flags, httpinfo_t *info)
{
    int found, offset;
    int hostlen, pathlen;
    int ret = 0;

    /* Basic checking, is it HTTP packet? */
    if (datalen < 10) {
	SPARQ_LOG("%s: Not enough length, ignore it!\n", __FUNCTION__);
	return 0;	/* Not enough length, ignore it */
	}
    if (memcmp(data, "GET ", sizeof("GET ") - 1) != 0 &&
        memcmp(data, "POST ", sizeof("POST ") - 1) != 0 &&
        memcmp(data, "HEAD ", sizeof("HEAD ") - 1) != 0) { //zg add 2006.09.28 for cdrouter3.3 item 186(cdrouter_urlfilter_15)
	SPARQ_LOG("%s: Pass it\n", __FUNCTION__);
	return 0;	/* Pass it */
	}
    if (!(flags & (HTTP_HOST | HTTP_URL))) {
	SPARQ_LOG("%s: Non-flag\n", __FUNCTION__);
	return 0;
	}

    /* find the 'Host: ' value */
    found = find_pattern2(data, datalen, "Host: ", 
	    sizeof("Host: ") - 1, '\r', &offset, &hostlen);
    SPARQ_LOG("Host found=%d\n", found);

    if (!found || !hostlen)
	return 0;

    ret++;	/* Host found, increase the return value */
    hostlen = (hostlen < BM_MAX_HLEN) ? hostlen : BM_MAX_HLEN;
    strncpy(info->url, data + offset, hostlen);
    info->url[hostlen] = '\0';		/* null-terminated */
    info->hostlen = hostlen;
    SPARQ_LOG("HOST=%s, hostlen=%d\n", info->url, info->hostlen);

    if (!(flags & HTTP_URL))
	return ret;

    /* we already check for GET/POST/HEAD, so find url boundaries only */
    found = find_pattern2(data + 3, datalen - 3, " ", 1, '\r', &offset, &pathlen);
    offset += 3;
    pathlen -= (sizeof("HTTP/x.x") - 1);
    SPARQ_LOG("GET/POST found=%d off=%d\n", found, offset);

    if (pathlen <= 0) /* ignore this field */
	return ret;

    ret++;	/* "GET/POST/HEAD url HTTP/x.x" found, increase the return value */
    pathlen = ((pathlen + hostlen) < BM_MAX_HLEN) ? pathlen : BM_MAX_HLEN - hostlen;
    strncpy(info->url + hostlen, data + offset, pathlen);
    info->url[hostlen + pathlen] = '\0';	/* null-terminated */
    info->urllen = hostlen + pathlen;
    SPARQ_LOG("URL=%s, urllen=%d\n", info->url, info->urllen);

    return ret;
}

/* Linear string search based on memcmp() */
static char *search_linear(char *needle, char *haystack, int needle_len, int haystack_len) 
{
	char *k = haystack + (haystack_len-needle_len);
	char *t = haystack;

	SPARQ_LOG("%s: haystack=%s, needle=%s\n", __FUNCTION__, t, needle);
	for(; t <= k; t++) {
		//SPARQ_LOG("%s: haystack=%s, needle=%s\n", __FUNCTION__, t, needle);
		if (strnicmp(t, needle, needle_len) == 0) return t;
		//if ( memcmp(t, needle, needle_len) == 0 ) return t;
	}

	return NULL;
}

static bool
webstr_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_webstr_info *info = par->matchinfo;
	const struct tcphdr *tcph;
	unsigned char *data;
	unsigned int datalen;

	char *wordlist = (char *)&info->string;
	httpinfo_t htinfo;
	int flags = 0;
	int found = 0;
	u_int32_t opt = 0;

	if (par->fragoff != 0) return info->invert;

	if (skb_is_nonlinear(skb)) {
		if (unlikely(skb_linearize((struct sk_buff *)skb))) {
			if (net_ratelimit())
				printk(KERN_WARNING "webstr: nonlinear skb->data_len=%d\n", skb->data_len);
			// failed to linearize packet, bailing
			return info->invert;
		}
	}

	/* assumption: we accept IPPROTO_TCP only */
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	if (par->family == AF_INET6) {
		const struct ipv6hdr *iph = ipv6_hdr(skb);
		u8 nexthdr;
		nexthdr = iph->nexthdr;
		tcph = (void *)iph + ipv6_skip_exthdr(skb, sizeof(*iph), &nexthdr);
		datalen = ntohs(iph->payload_len);
	}
	else
#endif
	/* IPv4 */ {
		const struct iphdr *iph = ip_hdr(skb);
		tcph = (void *)iph + (iph->ihl * 4);
		datalen = ntohs(iph->tot_len);
	}

	data = (void *)tcph + (tcph->doff * 4);
	SPARQ_LOG("%s: type=%s seq=%u family=%d datalen=%d\n ", __FUNCTION__,
		(info->type == XT_WEBSTR_URL) ? "XT_WEBSTR_URL"
		: (info->type == XT_WEBSTR_HOST) ? "XT_WEBSTR_HOST"
		: "XT_WEBSTR_CONTENT",
		ntohl(tcph->seq), par->family, datalen);

	/* Determine the flags value for get_http_info(), and mangle packet 
	 * if needed. */
	switch (info->type)
	{
	    case XT_WEBSTR_URL:	/* fall through */
		flags |= HTTP_URL;

	    case XT_WEBSTR_HOST:
		flags |= HTTP_HOST;
		break;

	    case XT_WEBSTR_CONTENT:
		opt = simple_strtol(wordlist, NULL, 10);
		SPARQ_LOG("%s: string=%s, opt=%#x\n", __FUNCTION__, wordlist, opt);

		if (opt & (BLK_JAVA | BLK_ACTIVE | BLK_PROXY))
		    flags |= HTTP_URL;
		if (opt & BLK_PROXY)
		    flags |= HTTP_HOST;
#if 0
		// Could we modify the packet payload in a "match" module?  --YY@Ralink
		if (opt & BLK_COOKIE)
		    mangle_http_header(data, datalen, HTTP_COOKIE);
#endif
		break;
	}

	/* Get the http header info */
	if (get_http_info(data, datalen, flags, &htinfo) < 1)
	    return 0;

	/* Check if the http header content contains the forbidden keyword */
	if (info->type == XT_WEBSTR_HOST || info->type == XT_WEBSTR_URL) {
	    int nlen, hlen;
	    char needle[BM_MAX_NLEN + 1];
	    char *next;

	    if (info->type == XT_WEBSTR_HOST) {
		hlen = htinfo.hostlen;
	    }
	    else {
		hlen = htinfo.urllen;
	    }
	    if (hlen == 0)
		    goto match_ret;

	    split(needle, wordlist, next, "<&nbsp;>") {
		nlen = strlen(needle);
		SPARQ_LOG("keyword=%s, nlen=%d, hlen=%d\n", needle, nlen, hlen);
		if (nlen == 0 || nlen > hlen) continue;
		if (search(needle, htinfo.url, nlen, hlen) != NULL) {
		    found = 1;
		    break;
		}
	    }
	}
	else {		/* XT_WEBSTR_CONTENT */
	    int vicelen;

	    if (opt & BLK_JAVA) {
		vicelen = sizeof(".js") - 1;
		if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".js", vicelen) == 0) {
		    SPARQ_LOG("%s: MATCH....java\n", __FUNCTION__);
		    found = 1;
		    goto match_ret;
		}
		vicelen = sizeof(".class") - 1;
		if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".class", vicelen) == 0) {
		    SPARQ_LOG("%s: MATCH....java\n", __FUNCTION__);
		    found = 1;
		    goto match_ret;
		}
	    }
	    if (opt & BLK_ACTIVE){
		vicelen = sizeof(".ocx") - 1;
		if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".ocx", vicelen) == 0) {
		    SPARQ_LOG("%s: MATCH....activex\n", __FUNCTION__);
		    found = 1;
		    goto match_ret;
		}
		vicelen = sizeof(".cab") - 1;
		if (strnicmp(htinfo.url + htinfo.urllen - vicelen, ".cab", vicelen) == 0) {
		    SPARQ_LOG("%s: MATCH....activex\n", __FUNCTION__);
		    found = 1;
		    goto match_ret;
		}
	    }
	    if (opt & BLK_PROXY){
		if (strnicmp(htinfo.url + htinfo.hostlen, "http://", sizeof("http://") - 1) == 0) {
		    SPARQ_LOG("%s: MATCH....proxy\n", __FUNCTION__);
		    found = 1;
		    goto match_ret;
		}
	    }
	}

match_ret:
	SPARQ_LOG("%s: Verdict =======> %s \n",__FUNCTION__
		, found ? "DROP" : "ACCEPT");

	return (found ^ info->invert);
}


static int webstr_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_webstr_info *info = par->matchinfo;

	/* allowed types */
	switch (info->type) {
		case XT_WEBSTR_URL:
		case XT_WEBSTR_HOST:
		case XT_WEBSTR_CONTENT:
			break;
		default:
			return -EINVAL;
	}
	/* pattern length */
	if (info->len < 1)
		return -EINVAL;

	return 0;
}

static struct xt_match xt_webstr_match __read_mostly = {
	.name		= "webstr",
	.family		= NFPROTO_UNSPEC,
	.match		= webstr_mt,
	.checkentry	= webstr_mt_check,
	.matchsize	= sizeof(struct xt_webstr_info),
	.proto		= IPPROTO_TCP,
	.me		= THIS_MODULE
};

static int __init webstr_init(void)
{
	web_str_loaded = 1;
	search = search_linear;
	return xt_register_match(&xt_webstr_match);
}

static void __exit webstr_fini(void)
{
	web_str_loaded = 0;
	xt_unregister_match(&xt_webstr_match);
}

module_init(webstr_init);
module_exit(webstr_fini);

MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_webstr");
MODULE_ALIAS("ip6t_webstr");
