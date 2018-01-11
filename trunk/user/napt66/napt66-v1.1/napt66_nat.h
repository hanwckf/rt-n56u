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
#include <linux/byteorder/generic.h>
#include <linux/string.h>
#include "napt66_global.h"

#define CKSUM_CARRY(x) \
    (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))

