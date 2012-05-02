/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    hwnat_ioctl.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#ifndef	__HW_NAT_IOCTL_H__
#define	__HW_NAT_IOCTL_H__

#define HW_NAT_ADD_ENTRY    		(0x01)
#define HW_NAT_DEL_ENTRY    		(0x02)
#define HW_NAT_DUMP_ENTRY    		(0x03)
#define HW_NAT_GET_ALL_ENTRIES 		(0x04)
#define HW_NAT_BIND_ENTRY		(0x05)
#define HW_NAT_UNBIND_ENTRY		(0x06)
#define HW_NAT_INVALID_ENTRY		(0x07)
#define HW_NAT_DEBUG	   		(0x08)

/*HNAT QOS*/
#define HW_NAT_DSCP_REMARK		(0x09)
#define HW_NAT_VPRI_REMARK		(0x0a)
#define HW_NAT_FOE_WEIGHT		(0x0b)
#define HW_NAT_ACL_WEIGHT		(0x0c)
#define HW_NAT_DSCP_WEIGHT		(0x0d)
#define HW_NAT_VPRI_WEIGHT		(0x0e)
#define HW_NAT_DSCP_UP			(0x0f)
#define HW_NAT_UP_IDSCP			(0x10)
#define HW_NAT_UP_ODSCP			(0x11)
#define HW_NAT_UP_VPRI			(0x12)
#define HW_NAT_UP_AC			(0x13)
#define HW_NAT_SCH_MODE			(0x14)
#define HW_NAT_SCH_WEIGHT		(0x15)
#define HW_NAT_BIND_THRESHOLD		(0x16)
#define HW_NAT_MAX_ENTRY_LMT		(0x17)
#define HW_NAT_RULE_SIZE		(0x18)
#define HW_NAT_KA_INTERVAL		(0x19)
#define HW_NAT_UB_LIFETIME		(0x1a)
#define HW_NAT_BIND_LIFETIME		(0x1b)
#define HW_NAT_BIND_DIRECTION		(0x1c)

#define HW_NAT_DEVNAME			"hwnat0"
#define HW_NAT_MAJOR			(215)

enum hwnat_status {
	HWNAT_SUCCESS=0,
	HWNAT_FAIL=1,
	HWNAT_ENTRY_NOT_FOUND=2
};

struct hwnat_tuple {
	unsigned short  hash_index;
	unsigned char   is_udp;
	unsigned char   fmt;
	union {
	    unsigned int    sip;
	    unsigned int    ipv6_dip0;
	};
	union {
	    unsigned int    dip;
	    unsigned int    ipv6_dip1;
	};
	unsigned short  sport;
	unsigned short  dport;
	union {
	    unsigned int    new_sip;
	    unsigned int    ipv6_dip2;
	};
	union {
	    unsigned int    new_dip;
	    unsigned int    ipv6_dip3;
	};
	unsigned short  new_sport;
	unsigned short  new_dport;
	unsigned short  vlan1;
	unsigned short  vlan2;
	unsigned short  pppoe_id;
	unsigned char	dmac[6];
	unsigned char	smac[6];
	unsigned char   vlan1_act:2;
	unsigned char   vlan2_act:2;
	unsigned char   snap_act:2;
	unsigned char   pppoe_act:2;
	unsigned char	dst_port;/*dst interface 0:CPU 1:GE1*/
    	enum hwnat_status	result;
};

struct hwnat_args {
    unsigned int    	debug:3;
    unsigned int    	bind_dir:2; /* 0=upstream, 1=downstream, 2=bi-direction */
    unsigned int    	entry_state:2; /* invalid=0, unbind=1, bind=2, fin=3 */
    enum hwnat_status	result;
    unsigned int    	entry_num:16;
    unsigned int    	num_of_entries:16;
    struct hwnat_tuple  entries[0];
};

/*hnat qos*/
struct hwnat_qos_args {
    unsigned int    	enable:1;
    unsigned int    	up:3; 
    unsigned int    	weight:3; /*UP resolution*/
    unsigned int    	dscp:6;
    unsigned int    	dscp_set:3;
    unsigned int    	vpri:3;
    unsigned int        ac:2;
    unsigned int        mode:2;
    unsigned int        weight0:4;/*WRR 4 queue weight*/
    unsigned int        weight1:4;
    unsigned int        weight2:4;
    unsigned int        weight3:4;
    enum hwnat_status	result;

};


/*hnat config*/
struct hwnat_config_args {
    unsigned int    	bind_threshold:16;
    unsigned int    	foe_full_lmt:14; 
    unsigned int    	foe_half_lmt:14; 
    unsigned int    	foe_qut_lmt:14; 
    unsigned int    	pre_acl:9;
    unsigned int    	pre_meter:9;
    unsigned int    	pre_ac:9;
    unsigned int        post_meter:9;
    unsigned int        post_ac:9;
    unsigned int        foe_tcp_ka:8;/*unit 4 sec*/
    unsigned int        foe_udp_ka:8;/*unit 4 sec*/
    unsigned int        foe_unb_dlta:8;/*unit 1 sec*/
    unsigned int        foe_tcp_dlta:16;/*unit 1 sec*/
    unsigned int        foe_udp_dlta:16;/*unit 1 sec*/
    unsigned int        foe_fin_dlta:16;/*unit 1 sec*/
    enum hwnat_status	result;

};
int PpeRegIoctlHandler(void);
void PpeUnRegIoctlHandler(void);

#endif
