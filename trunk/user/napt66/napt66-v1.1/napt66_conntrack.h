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
#include <linux/socket.h>
#include <linux/version.h>


#include "napt66_global.h"

int get_entry(struct sk_buff *skb,struct conn_entry** pp_entry,int direc);
int in_cksum(u_int16_t *addr, int len);
