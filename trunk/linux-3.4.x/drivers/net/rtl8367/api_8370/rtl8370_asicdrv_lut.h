#ifndef _RTL8370_ASICDRV_LUT_H_
#define _RTL8370_ASICDRV_LUT_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_LUT_AGETIMERMAX     7
#define RTL8370_LUT_AGESPEEDMAX     3
#define RTL8370_LUT_LEARNLIMITMAX   0x2040
#define RTL8370_LUT_ADDRMAX         0x203F
#define RTL8370_LUT_TABLE_SIZE      0x2000

enum RTL8370_LRNOVERACT{

	LRNOVERACT_FORWARD=0, 		
	LRNOVERACT_DROP, 		
	LRNOVERACT_TRAP,
	LRNOVERACT_MAX,		
};

enum RTL8370_LUTREADMETHOD{

	LUTREADMETHOD_MAC =0, 		
	LUTREADMETHOD_ADDRESS, 		
};


typedef struct LUTTABLE{

	ipaddr_t sip;
	ipaddr_t dip;
	ether_addr_t mac;
	
	uint16 	static_bit:1;
	uint16 	block:1;
	uint16 	auth:1;
	uint16 	macpri:3;
	uint16 	sa_en:1;
	uint16 	da_en:1;
	uint16 	portmask;
	uint16 	spa:4;
	uint16 	age:3;
	uint16 	fid:12;
	uint16 	efid:3;
	uint16 	ipmul:1;
	uint16 	valid:1;
	uint16 	lookup_hit:1;
	uint16 	address:14;
	
}rtl8370_luttb;

struct fdb_maclearn_st{

#ifdef _LITTLE_ENDIAN
	uint16 	static_bit:1;
	uint16 	block:1;
	uint16 	auth:1;
	uint16 	macpri:3;
	uint16 	da_en:1;
	uint16 	sa_en:1;
	uint16 	spa:4;
	uint16 	age:3;
    uint16 	reserved1:1;

    uint16 	reserved2:8;
	uint16	mac5:8;

	uint16	mac4:8;
	uint16	mac3:8;

	uint16	mac2:8;
	uint16	mac1:8;

	uint16	mac0:8;
	uint16 	fid1:8;

	uint16 	fid2:4;
	uint16 	efid:3;
  	uint16 	ipmul:1;
    uint16 	reserved3:8;

	uint16 	valid:1;
    uint16 	reserved4:15;
#else
    uint16 	reserved1:1;
	uint16 	age:3;
	uint16 	spa:4;
	uint16 	sa_en:1;
	uint16 	da_en:1;
	uint16 	macpri:3;
	uint16 	auth:1;
	uint16 	block:1;
	uint16 	static_bit:1;

    uint16	mac5:8;
    uint16 	reserved2:8;

	uint16	mac3:8;
    uint16	mac4:8;

	uint16	mac1:8;
    uint16	mac2:8;

	uint16 	fid1:8;
    uint16	mac0:8;

    uint16 	reserved3:8;
  	uint16 	ipmul:1;
	uint16 	efid:3;
	uint16 	fid2:4;

    uint16 	reserved4:15;
	uint16 	valid:1;
#endif	
};

struct fdb_l2multicast_st{

#ifdef _LITTLE_ENDIAN
	uint16 	static_bit:1;
	uint16 	block:1;
	uint16 	auth:1;
	uint16 	macpri:3;
	uint16 	da_en:1;
	uint16 	sa_en:1;
	uint16 	portmask1:8;
    
	uint16 	portmask2:8;
    uint16	mac5:8;

    uint16	mac4:8;
	uint16	mac3:8;

    uint16	mac2:8;
	uint16	mac1:8;

    uint16	mac0:8;
	uint16 	fid1:8;
    
	uint16 	fid2:4;
	uint16 	efid:3;
	uint16 	ipmul:1;
    uint16 	reserved1:8;

	uint16 	valid:1;
    uint16 	reserved2:15;
#else
	uint16 	portmask1:8;
	uint16 	sa_en:1;
	uint16 	da_en:1;
	uint16 	macpri:3;
	uint16 	auth:1;
	uint16 	block:1;
	uint16 	static_bit:1;

    uint16	mac5:8;
	uint16 	portmask2:8;

	uint16	mac3:8;
    uint16	mac4:8;

	uint16	mac1:8;
    uint16	mac2:8;

	uint16 	fid1:8;
    uint16	mac0:8;
    
    uint16 	reserved1:8;
	uint16 	ipmul:1;
	uint16 	efid:3;
	uint16 	fid2:4;

    uint16 	reserved2:15;
	uint16 	valid:1;
#endif	
};

struct fdb_ipmulticast_st{

#ifdef _LITTLE_ENDIAN
	uint16 	static_bit:1;
    uint16  reserved1:2;
	uint16 	macpri:3;
	uint16 	da_en:1;
	uint16 	sa_en:1;
	uint16 	portmask1:8;
    
	uint16 	portmask2:8;
    uint16 	sip0:8;

	uint16 	sip1:8;
	uint16 	sip2:8;

	uint16 	sip3:8;
	uint16 	dip0:8;

	uint16 	dip1:8;
	uint16 	dip2:8;

	uint16 	dip3:4;
    uint16 	reserved2:3;
    uint16 	ipmul:1;
    uint16 	reserved3:8;

	uint16 	valid:1;
    uint16 	reserved4:15;
#else
	uint16 	portmask1:8;
	uint16 	sa_en:1;
	uint16 	da_en:1;
	uint16 	macpri:3;
    uint16  reserved1:2;
	uint16 	static_bit:1;

	uint16 	sip3:8;
	uint16 	portmask2:8;

	uint16 	sip1:8;
	uint16 	sip2:8;

	uint16 	dip3:8;
	uint16 	sip0:8;

	uint16 	dip1:8;
	uint16 	dip2:8;

    uint16 	reserved3:8;
 	uint16 	ipmul:1;
    uint16 	reserved2:3;
	uint16 	dip0:4;

    uint16 	reserved4:15;
	uint16 	valid:1;
#endif	
};

typedef union FDBSMITABLE{

	struct fdb_ipmulticast_st	smi_ipmul;
	struct fdb_l2multicast_st   smi_l2mul;
	struct fdb_maclearn_st		smi_auto;
		
}rtl8370_fdbtb;

extern ret_t  rtl8370_setAsicLutIpMulticastLookup(uint32 enabled);
extern ret_t  rtl8370_getAsicLutIpMulticastLookup(uint32* enabled);
extern ret_t  rtl8370_setAsicLutAgeTimerSpeed( uint32 timer, uint32 speed);
extern ret_t  rtl8370_getAsicLutAgeTimerSpeed( uint32* timer, uint32* speed);
extern ret_t  rtl8370_setAsicLutCamTbUsage(uint32 disabled);
extern ret_t  rtl8370_getAsicLutCamTbUsage(uint32* disabled);
extern ret_t  rtl8370_setAsicLutCamType(uint32 type);
extern ret_t  rtl8370_getAsicLutCamType(uint32* type);
extern ret_t  rtl8370_setAsicLutLearnLimitNo(uint32 port,uint32 number);
extern ret_t  rtl8370_getAsicLutLearnLimitNo(uint32 port,uint32* number);
extern ret_t  rtl8370_setAsicLutLearnOverAct(uint32 action);
extern ret_t  rtl8370_getAsicLutLearnOverAct(uint32* action);
extern ret_t  rtl8370_setAsicL2LookupTb(rtl8370_luttb *l2Table);
extern ret_t  rtl8370_getAsicL2LookupTb(enum RTL8370_LUTREADMETHOD method, rtl8370_luttb *l2Table);
extern ret_t  rtl8370_getAsicLutLearnNo(uint32 port,uint32* number);
extern ret_t  rtl8370_setAsicLutLinkDownForceAging(uint32 enable);
extern ret_t  rtl8370_getAsicLutLinkDownForceAging(uint32* enable);
#endif /*_RTL8370_ASICDRV_LUT_H_*/

