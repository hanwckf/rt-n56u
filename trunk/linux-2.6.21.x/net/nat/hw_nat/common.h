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
    common.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#ifndef _COMMON_WANTED
#define _COMMON_WANTED

#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2], \
                       ((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5]

#define IPV6_ADDR(x) ntohs(x[0]),ntohs(x[1]),ntohs(x[2]),ntohs(x[3]),ntohs(x[4]),\
		     ntohs(x[5]),ntohs(x[6]),ntohs(x[7])

#define IN
#define OUT
#define INOUT

#define NAT_DEBUG

#ifdef NAT_DEBUG
#define NAT_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define NAT_PRINT(fmt, args...) { }
#endif

enum L2RuleDir {
    OTHERS=0,
    DMAC=1,
    SMAC=2,
    SDMAC=3
};

enum L3RuleDir {
    IP_QOS=0,
    DIP=1,
    SIP=2,
    SDIP=3
};

enum L4RuleDir {
    DONT_CARE=0,
    DPORT=1,
    SPORT=2,
    SDPORT=3
};

enum RuleType {
    L2_RULE=0,   
    L3_RULE=1,   
    L4_RULE=2,  
    PT_RULE=3  
};

enum PortNum {
    PN_CPU=0, 
    PN_GE1=1,
    PN_DONT_CARE=7
};

enum OpCode {
    AND=0,
    OR=1
};

enum TcpFlag {
    TCP_FIN=1,
    TCP_SYN=2,
    TCP_RESET=4,
    TCP_PUSH=8,
    TCP_ACK=16,
    TCP_URGENT=32
};

enum TcpFlagOp {
    EQUAL=0,
    NOT_EQUAL=1,
    IN_SET=2,
    NOT_IN_SET=3
};

enum L4Type {
    FLT_IP_PROT=0,
    FLT_UDP=1,
    FLT_TCP=2,
    FLT_TCP_UDP=3
};

enum FpnType {
    FPN_CPU=0,
    FPN_GE1=1,
    FPN_GE2=2,
    FPN_FRC_PRI_ONLY=5,
    FPN_ALLOW=4,
    FPN_DROP=7
};

#endif
