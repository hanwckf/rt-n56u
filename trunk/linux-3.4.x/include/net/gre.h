#ifndef __LINUX_GRE_H
#define __LINUX_GRE_H

#include <linux/skbuff.h>

struct gre_base_hdr {
	__be16 flags;
	__be16 protocol;
} __packed;

struct gre_full_hdr {
	struct gre_base_hdr fixed_header;
	__be16 csum;
	__be16 reserved1;
	__be32 key;
	__be32 seq;
} __packed;

#define GREPROTO_CISCO		0
#define GREPROTO_PPTP		1
#define GREPROTO_MAX		2

struct gre_protocol {
	int  (*handler)(struct sk_buff *skb);
	void (*err_handler)(struct sk_buff *skb, u32 info);
};

int gre_add_protocol(const struct gre_protocol *proto, u8 version);
int gre_del_protocol(const struct gre_protocol *proto, u8 version);

#endif
