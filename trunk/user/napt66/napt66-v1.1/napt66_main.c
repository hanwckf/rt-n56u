#define __NO_VERSION__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>                  
#include <linux/ipv6.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv6.h>
#include <linux/in6.h>
#include <linux/types.h>
#include <net/ipv6.h>
#include "napt66_global.h"
#include "napt66_main.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NSRC NAPT66 Team");
MODULE_DESCRIPTION("IPv6-to-IPv6 Network Address PORT Translation (NAPT66)");
MODULE_VERSION("V0.1");

/* hooks for out filter*/
//静态全局变量，作用域是本源文件。
static struct nf_hook_ops nfho_in;    /* Netfilter钩子，RSNAT */
static struct nf_hook_ops nfho_out;   /* Netfilter钩子， SNAT */

struct hash_entry source_table[HASHRANGE];
struct hash_entry ipproto_table[HASHRANGE];

/* module parameters */
char *wan_if="eth0";  //参数，WAN接口名称
//module_param(wan_if,charp,S_IRUGO|S_IWUSR); //字符指针，允许所有人读取，允许root写入
//默认为eth0，如要更改，在挂载模块时输入
//sudo insmod napt66.ko wan_if="eth1"
module_param(wan_if,charp,0000); 
MODULE_PARM_DESC(wan_if, "A character string" );

//WAN接口ipv6地址，新建连接记录时要引用。
//不知道接口，先手动填入本机eth0的IPv6地址。
struct in6_addr wan_ipv6;
bool inet6_addr_flag;

extern int get_entry(struct sk_buff *skb,struct conn_entry** pp_entry,int direc);
extern int nat(struct sk_buff *skb,struct conn_entry* entry,int direc);
extern void hash_table_init(struct hash_entry* table);
extern int napt66_enabled;

long time(void* ptr)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	return tv.tv_sec;	
}


/* the implementation of the hook function at PRE_ROUTING hook point to deal with input data packets */
unsigned int hook_func_in(unsigned int hooknum,struct sk_buff *skb,const struct net_device *in,
			const struct net_device *out,int (*okfn)(struct sk_buff *))
{
	int status;
	struct conn_entry* conntrack_entry = NULL;

	//printk(KERN_INFO "device: %s\n",skb->dev->name);	//in设备名称，只处理eth0	
	if(strncmp(in->name,wan_if,IFNAMSIZ)){
		return NF_ACCEPT;
	}
	if(skb->pkt_type != PACKET_HOST){//通过mac地址过滤不是发往本机的包，
		//printk(KERN_INFO "host packet.\n");
		return NF_ACCEPT;
	}

	status = get_entry(skb,&conntrack_entry,RSNAT);

	if(status > 0){
		nat(skb,conntrack_entry,RSNAT);
	}

	return NF_ACCEPT;/* the follow data process as V6 before */
}

/*  the implementation of the hook function at POST_ROUTING hook point to deal with output data packets */
unsigned int hook_func_out(unsigned int hooknum,struct sk_buff *skb,const struct net_device *in,
			const struct net_device *out,int (*okfn)(struct sk_buff *))
{
	int status;
	struct conn_entry* conntrack_entry = NULL;

	
	//根据设备和IP地址过滤数据包
	//printk(KERN_INFO "out device: %s\n",((struct dst_entry *)skb->_skb_dst)->dev->name);//out设备名称，只处理eth0
	if(strncmp(out->name,wan_if,IFNAMSIZ)){
		return NF_ACCEPT;
	}

	//判断TCP,UDP,ICMPv6协议。struct sk_buff提供的transport_header只是指明了IPv6报头之后的地址。可能是其他扩展报头。

	status = get_entry(skb,&conntrack_entry,SNAT);

	if(status > 0){
		nat(skb,conntrack_entry,SNAT);
	}

	return NF_ACCEPT; /* the follow data process as V6 before*/
}

/* module initialization function */
 int init_module()
{
	printk(KERN_INFO "Netfilter module for IPv6 NAPT66. <NSRC_NAPT66_TEAM@BUPT,2010>\n");
	printk(KERN_INFO "TIME 2011.4.10 final\n");

	inet6_addr_flag = false;
	napt66_enabled = 1;

	hash_table_init(source_table);
	hash_table_init(ipproto_table);

	/* Register the hook data */
	/* RSNAT */
	nfho_in.hook = hook_func_in;   /* hook function deal with input data to modify the dstif necessary*/
	nfho_in.hooknum = NF_INET_PRE_ROUTING;/* register the hook at PRE_ROUTING 
	Note:in linux 2.6.25 is NF_INET_PRE_ROUTING,the earlier kernel is NF_IP6_PRE_ROUTING */
	nfho_in.pf = PF_INET6; /* just deal with IPv6 packets */
	nfho_in.priority = NF_IP6_PRI_NAT_DST;   /* dst */
	nf_register_hook(&nfho_in);

	/* SNAT */
	nfho_out.hook = hook_func_out;  /* hook function deal with output data to modify the src if necessary*/
	nfho_out.hooknum = NF_INET_POST_ROUTING;/* register the hook at PRE_ROUTING */
	nfho_out.pf = PF_INET6;
	nfho_out.priority = NF_IP6_PRI_NAT_SRC;   /* src */
	nf_register_hook(&nfho_out);

	return 0;
}

void cleanup_module()
{
	printk(KERN_INFO "Remove netfilter module for IPv6 NAPT66 . <NSRC_NAPT66_TEAM@BUPT,2010>\n");	
	nf_unregister_hook(&nfho_in);
	nf_unregister_hook(&nfho_out);
	napt66_enabled = 0;
}
