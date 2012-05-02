
#ifndef	__IP_DNAT_H
#define	__IP_DNAT_H

	#define int8_t		char
	#define int16_t		short
	#define int32_t		long
	#define int64_t		long long
	#define uint8_t		u_char
	#define uint16_t	u_short
	#define uint32_t	u_long
	#define uint64_t	unsigned long long
	
	#define	IPT_DNAT_TO_DEST	0x40000000
	#define	IPT_DNAT_TO_SHIFT	0x80000000

	/*enum	dnat_type {
		__TO_DEST=1, 
		__TO_SHIFT
	};
	
	struct dnat_range {
		enum dnat_type	flag;
		uint32_t		min_ip, max_ip;
		uint16_t		offset;
		union ip_conntrack_manip_proto	min, max;
	};*/

#endif
