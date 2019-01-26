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

#define SOURCE 1
#define IPPROTO 2
//外部接口

void hash_table_init(struct hash_entry* table);

//搜索到则返回指针，否则返回NULL
struct conn_entry* hash_search_ct(int hook,struct conn_entry* p_entry);
//在两张hash表中插入ct项
int hash_add_entry(struct conn_entry* p_entry);
