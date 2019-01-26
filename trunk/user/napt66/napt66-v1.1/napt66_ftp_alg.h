#include <linux/types.h>
#include <linux/inet.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/in6.h>
#include <net/ipv6.h>
#include "napt66_global.h"

int analysis_eprt(struct sk_buff *skb,struct conn_entry *entry);
