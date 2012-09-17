/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _IP_CONNTRACK_TUPLE_H
#define _IP_CONNTRACK_TUPLE_H

/* A `tuple' is a structure containing the information to uniquely
  identify a connection.  ie. if two packets have the same tuple, they
  are in the same connection; if not, they are not.

  We divide the structure along "manipulatable" and
  "non-manipulatable" lines, for the benefit of the NAT code.
*/

/* The protocol-specific manipulable parts of the tuple: always in
   network order! */
union ip_conntrack_manip_proto
{
	/* Add other protocols here. */
	u_int16_t all;

	struct {
		u_int16_t port;
	} tcp;
	struct {
		u_int16_t port;
	} udp;
	struct {
		u_int16_t id;
	} icmp;
	struct {
		u_int32_t key;
	} gre;
};

/* The manipulable part of the tuple. */
struct ip_conntrack_manip
{
	u_int32_t ip;
	union ip_conntrack_manip_proto u;
};

/* This contains the information to distinguish a connection. */
struct ip_conntrack_tuple
{
	struct ip_conntrack_manip src;

	/* These are the parts of the tuple which are fixed. */
	struct {
		u_int32_t ip;
		union {
			/* Add other protocols here. */
			u_int16_t all;

			struct {
				u_int16_t port;
			} tcp;
			struct {
				u_int16_t port;
			} udp;
			struct {
				u_int8_t type, code;
			} icmp;
			struct {
				u_int16_t protocol;
				u_int8_t version;
				u_int32_t key;
			} gre;
		} u;

		/* The protocol. */
		u_int16_t protonum;
	} dst;
};

/* This is optimized opposed to a memset of the whole structure.  Everything we
 * really care about is the  source/destination unions */
#define IP_CT_TUPLE_U_BLANK(tuple) 				\
	do {							\
		(tuple)->src.u.all = 0;				\
		(tuple)->dst.u.all = 0;				\
	} while (0)

enum ip_conntrack_dir
{
	IP_CT_DIR_ORIGINAL,
	IP_CT_DIR_REPLY,
	IP_CT_DIR_MAX
};

#ifdef __KERNEL__

#define DUMP_TUPLE(tp)						\
DEBUGP("tuple %p: %u %u.%u.%u.%u:%hu -> %u.%u.%u.%u:%hu\n",	\
       (tp), (tp)->dst.protonum,				\
       NIPQUAD((tp)->src.ip), ntohs((tp)->src.u.all),		\
       NIPQUAD((tp)->dst.ip), ntohs((tp)->dst.u.all))
#define DUMP_TUPLE_RAW(x) 						\
	DEBUGP("tuple %p: %u %u.%u.%u.%u:0x%08x -> %u.%u.%u.%u:0x%08x\n",\
	(x), (x)->dst.protonum,						\
	NIPQUAD((x)->src.ip), ntohl((x)->src.u.all), 			\
	NIPQUAD((x)->dst.ip), ntohl((x)->dst.u.all))

#define CTINFO2DIR(ctinfo) ((ctinfo) >= IP_CT_IS_REPLY ? IP_CT_DIR_REPLY : IP_CT_DIR_ORIGINAL)

/* If we're the first tuple, it's the original dir. */
#define DIRECTION(h) ((enum ip_conntrack_dir)(&(h)->ctrack->tuplehash[1] == (h)))

#define IP_TRACK_MAX			10
#define IP_TRACK_COMPARE		40

#define IP_TRACK_FULL		0x01
#define IP_TRACK_PORT		0x02
#define IP_TRACK_DATA		0x04
#define	IP_TRACK_UDP		0x08
#define IP_TRACK_DOWN		0x10
#define IP_TRACK_TIME		0x20

struct ip_track 
{
	int flag;
	u_int8_t number;
	struct timeval tv;
	int length[IP_TRACK_MAX];
};

/* Connections have two entries in the hash table: one for each way */
struct ip_conntrack_tuple_hash
{
	struct list_head list;

	struct ip_conntrack_tuple tuple;

	struct ip_track track;

	/* this == &ctrack->tuplehash[DIRECTION(this)]. */
	struct ip_conntrack *ctrack;
};

#endif /* __KERNEL__ */

static inline int ip_ct_tuple_src_equal(const struct ip_conntrack_tuple *t1,
				        const struct ip_conntrack_tuple *t2)
{
	return t1->src.ip == t2->src.ip
		&& t1->src.u.all == t2->src.u.all;
}

static inline int ip_ct_tuple_dst_equal(const struct ip_conntrack_tuple *t1,
				        const struct ip_conntrack_tuple *t2)
{
	return t1->dst.ip == t2->dst.ip
		&& t1->dst.u.all == t2->dst.u.all
		&& t1->dst.protonum == t2->dst.protonum;
}

static inline int ip_ct_tuple_equal(const struct ip_conntrack_tuple *t1,
				    const struct ip_conntrack_tuple *t2)
{
	return ip_ct_tuple_src_equal(t1, t2) && ip_ct_tuple_dst_equal(t1, t2);
}

/* Added by Steven Liu */
static inline int ip_ct_tuple_equal2(const struct ip_conntrack_tuple *t1,
				    const struct ip_conntrack_tuple *t2)
{
#if CONFIG_NAT_FCONE   /* Full Cone */
	return ip_ct_tuple_dst_equal(t1, t2);
#elif CONFIG_NAT_RCONE   /* Restricted Cone */
	return ip_ct_tuple_dst_equal(t1, t2) && (t1->src.ip == t2->src.ip);
#elif CONFIG_NAT_LINUX  /* Original Linux NAT */
	return ip_ct_tuple_src_equal(t1, t2) && ip_ct_tuple_dst_equal(t1, t2);
#endif
}
/*---*/

static inline int ip_ct_tuple_mask_cmp(const struct ip_conntrack_tuple *t,
				       const struct ip_conntrack_tuple *tuple,
				       const struct ip_conntrack_tuple *mask)
{
	return !(((t->src.ip ^ tuple->src.ip) & mask->src.ip)
		 || ((t->dst.ip ^ tuple->dst.ip) & mask->dst.ip)
		 || ((t->src.u.all ^ tuple->src.u.all) & mask->src.u.all)
		 || ((t->dst.u.all ^ tuple->dst.u.all) & mask->dst.u.all)
		 || ((t->dst.protonum ^ tuple->dst.protonum)
		     & mask->dst.protonum));
}

#endif /* _IP_CONNTRACK_TUPLE_H */
