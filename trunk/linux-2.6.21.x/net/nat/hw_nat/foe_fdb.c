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
    foe_fdb.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include "frame_engine.h"
#include "foe_fdb.h"
#include "hwnat_ioctl.h"
#include "util.h"

extern struct FoeEntry *PpeFoeBase; 
extern dma_addr_t phy_foe_base;

/* 
 * Mac address is not continuous in foe table
 *
 * 3      2	  0
 * +------+-------+
 * |VLAN  |DMac_hi|
 * +------+-------+
 * |  Dmac_lo     |
 * +--------------+
 *
 * 3      2	  0
 * +------+-------+
 * |PPPoE |SMac_hi|
 * +------+-------+
 * |  Smac_lo     |
 * +--------------+
 *
 * Ex: 
 *
 * Mac=01:80:C2:01:23:45
 *
 * 3      2	  0
 * +------+-------+
 * |PPPoE | 01:80 |
 * +------+-------+
 * | C2:01:23:45  |
 * +--------------+
 *
 */
void FoeSetMacInfo(uint8_t *Dst, uint8_t *Src)
{
    Dst[1] = Src[0];
    Dst[0] = Src[1];
    Dst[7] = Src[2];
    Dst[6] = Src[3];
    Dst[5] = Src[4];
    Dst[4] = Src[5];
}

void FoeGetMacInfo(uint8_t *Dst, uint8_t *Src)
{
    Dst[0]=Src[1];
    Dst[1]=Src[0];
    Dst[2]=Src[7];
    Dst[3]=Src[6];
    Dst[4]=Src[5];
    Dst[5]=Src[4];
}

void FoeDumpEntry(uint32_t Index)
{
    struct FoeEntry *entry=&PpeFoeBase[Index];
    int *p;

    NAT_PRINT("==========<Flow Table Entry=%d>===============\n", Index);
    p=(int *)entry;


    NAT_PRINT("-----------------<Flow Info>------------------\n");
    NAT_PRINT("Information Block 1: %08X\n",*(p));
    NAT_PRINT("SIP: %08X\n",*(p+1));
    NAT_PRINT("DIP: %08X\n",*(p+2));
    if(entry->bfib1.fmt == IPV4_NAPT) {
	NAT_PRINT("SP|DP: %08X\n",*(p+3));
    }
    NAT_PRINT("Information Block 2: %08X\n",*(p+4));
    NAT_PRINT("NewSIP: %08X\n",*(p+5));
    NAT_PRINT("NewDIP: %08X\n",*(p+6));
    if(entry->bfib1.fmt == IPV4_NAPT) {
	NAT_PRINT("NewSP|NewDP: %08X\n",*(p+7));
    }
    NAT_PRINT("VL1|DMAC_Hi: %08X\n",*(p+8));
    NAT_PRINT("DMAC_Lo: %08X\n",*(p+9));
    NAT_PRINT("PPPoE|SMAC_Hi: %08X\n",*(p+10));
    NAT_PRINT("SMAC_Lo: %08X\n",*(p+11));
    NAT_PRINT("Resv|SNAP: %08X\n",*(p+12));
    NAT_PRINT("Resv|VL2: %08X\n",*(p+13));
    NAT_PRINT("Resv: %08X\n",*(p+14));
    NAT_PRINT("Resv: %08X\n",*(p+15));

    if(entry->bfib1.fmt == IPV4_NAPT) {
	NAT_PRINT("IPv4 Org IP/Port: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n",
		IP_FORMAT(entry->sip),entry->sport, 
		IP_FORMAT(entry->dip),entry->dport);
	NAT_PRINT("IPv4 New IP/Port: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d\n",
		IP_FORMAT(entry->new_sip),entry->new_sport, 
		IP_FORMAT(entry->new_dip),entry->new_dport);
    } else if (entry->bfib1.fmt == IPV4_NAT) {
	NAT_PRINT("IPv4 Org IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
		IP_FORMAT(entry->sip),
		IP_FORMAT(entry->dip));
	NAT_PRINT("IPv4 New IP: %u.%u.%u.%u->%u.%u.%u.%u\n",
		IP_FORMAT(entry->new_sip), 
		IP_FORMAT(entry->new_dip));
    } else if (entry->bfib1.fmt == IPV6_ROUTING) {
	NAT_PRINT("IP: %04X:%04X:%04X:%04X",
		entry->ipv6_dip0, entry->ipv6_dip1, 
		entry->ipv6_dip2, entry->ipv6_dip3); 
    } else {
	NAT_PRINT("Wrong MFT value\n");
    }

    NAT_PRINT("DMAC=%02X:%02X:%02X:%02X:%02X:%02X \
	       SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n",
	    entry->dmac_hi[1], entry->dmac_hi[0], entry->dmac_lo[3],
	    entry->dmac_lo[2], entry->dmac_lo[1],entry->dmac_lo[0], 
	    entry->smac_hi[1],entry->smac_hi[0], entry->smac_lo[3],
	    entry->smac_lo[2],entry->smac_lo[1],entry->smac_lo[0]);
    NAT_PRINT("=========================================\n\n");
}

int FoeAddEntry(struct hwnat_tuple *opt)
{
	struct FoePriKey key;
	struct FoeEntry *entry;
	int32_t hash_index;

	key.ipv4.sip=opt->sip;
	key.ipv4.dip=opt->dip;
	key.ipv4.sport=opt->sport;
	key.ipv4.dport=opt->dport;
	key.ipv4.is_udp=opt->is_udp;
	key.fmt=IPV4_NAPT; /* FIXME */
   
	hash_index = FoeHashFun(&key,INVALID);
	if(hash_index != -1) {

		opt->hash_index=hash_index;
		entry=&PpeFoeBase[hash_index];
		entry->bfib1.sta=0; /* static entry=>dynamic to fix TCP last ACK issue */
		entry->bfib1.t_u=opt->is_udp; /* tcp/udp */
		entry->bfib1.state=BIND; 
		entry->bfib1.ka=1; /* keepalive */
		entry->bfib1.ttl=0; /* TTL-1 */
		entry->bfib1.pppoe=opt->pppoe_act; /* insert / remove */
		entry->bfib1.snap=0;
		entry->bfib1.v1=opt->vlan1_act; /* insert / remove */
		entry->bfib1.v2=opt->vlan2_act;
		entry->bfib1.time_stamp=RegRead(FOE_TS)&0xFFFF;

		entry->sip=opt->sip; 
		entry->dip=opt->dip;
		entry->dport=opt->dport;
		entry->sport=opt->sport;

		entry->iblk2.fd=1;
		entry->iblk2.dp=opt->dst_port; /* 0:cpu, 1:GE1 */

		entry->new_sip=opt->new_sip;
		entry->new_dip=opt->new_dip;
		entry->new_dport=opt->new_dport;
		entry->new_sport=opt->new_sport;
		entry->vlan1=opt->vlan1;
		entry->vlan2=opt->vlan2;
		entry->pppoe_id=opt->pppoe_id;

		FoeSetMacInfo(entry->dmac_hi,opt->dmac);
		FoeSetMacInfo(entry->smac_hi,opt->smac);

		return HWNAT_SUCCESS;


	}

	return HWNAT_FAIL;

}

int FoeGetAllEntries(struct hwnat_args *opt)
{
	struct FoeEntry *entry;
	int hash_index=0;
	int count=0; /* valid entry count */

	for(hash_index=0; hash_index<FOE_4TB_SIZ; hash_index++) {
		entry=&PpeFoeBase[hash_index];

		if(entry->bfib1.state == opt->entry_state) {
			opt->entries[count].hash_index = hash_index;
			opt->entries[count].sip = entry->sip;
			opt->entries[count].dip = entry->dip;
			opt->entries[count].sport = entry->sport;
			opt->entries[count].dport = entry->dport;
			opt->entries[count].new_sip = entry->new_sip;
			opt->entries[count].new_dip = entry->new_dip;
			opt->entries[count].new_sport = entry->new_sport;
			opt->entries[count].new_dport = entry->new_dport;
			opt->entries[count].fmt = entry->bfib1.fmt;
			opt->entries[count].is_udp = entry->bfib1.t_u;
			count++;
		}

	}

	opt->num_of_entries = count;

	if(opt->num_of_entries > 0) {
		return HWNAT_SUCCESS;
	}else {
		return HWNAT_ENTRY_NOT_FOUND;
	}
}


int FoeDelEntry(struct hwnat_tuple *opt)
{
	struct FoePriKey key;
	int32_t hash_index;

	key.ipv4.sip=opt->sip;
	key.ipv4.dip=opt->dip;
	key.ipv4.sport=opt->sport;
	key.ipv4.dport=opt->dport;
	key.ipv4.is_udp=opt->is_udp;
	key.fmt=IPV4_NAPT; /* FIXME */

	// find bind entry                 
	hash_index = FoeHashFun(&key,BIND);
	if(hash_index != -1) {
		opt->hash_index=hash_index;
		FoeDelEntryByNum(hash_index);
		return HWNAT_SUCCESS;
	}

	return HWNAT_ENTRY_NOT_FOUND;
}

int FoeBindEntry(struct hwnat_args *opt)
{
    struct FoeEntry *entry;

    entry = &PpeFoeBase[opt->entry_num];

    //restore right information block1
    entry->tmp_buf.time_stamp=RegRead(FOE_TS)&0xFFFF;
    entry->tmp_buf.state = BIND;
    memcpy(&entry->bfib1, &entry->tmp_buf, sizeof(entry->tmp_buf));


    return HWNAT_SUCCESS;
}


int FoeUnBindEntry(struct hwnat_args *opt)
{

    struct FoeEntry *entry;

    entry = &PpeFoeBase[opt->entry_num];

    entry->udib1.state = UNBIND;
    entry->udib1.time_stamp=RegRead(FOE_TS)&0xFF;
    
    return HWNAT_SUCCESS;
}

int FoeDelEntryByNum(uint32_t entry_num)
{
    struct FoeEntry *entry;

    entry = &PpeFoeBase[entry_num];
    memset(entry, 0, sizeof(struct FoeEntry));

    return HWNAT_SUCCESS;
}

#ifdef CONFIG_RA_HW_NAT_HASH1
static uint32_t FoeHashMask[] = {
	0x0, /* 0 */
	0x1, /* 1 */
	0x3, /* 2 */
	0x7, /* 3 */
	0xF, /* 4 */
	0x1F, /* 5 */
	0x3F, /* 6 */
	0x7F, /* 7 */
	0xFF, /* 8 */
	0x1FF, /* 9 */
	0x3FF, /* 10 */
	0x7FF, /* 11 */
	0xFFF, /* 12 */
	0x1FFF, /* 13 */
	0x3FFF, /* 14 */
	0x7FFF, /* 15 */
	0xFFFF  /* 16 */
};

static uint32_t inline ExtractBits(uint32_t *Buf, uint8_t StartBit, uint8_t EndBit)
{
	uint8_t *Data=(uint8_t *)Buf;
	uint32_t Result=0;
	uint8_t  BaseIndex=StartBit/8;

	Result = Data[BaseIndex+2]<<16 | Data[BaseIndex+1]<<8 | Data[BaseIndex];
	Result = Result >> (StartBit%8);

	Result &= FoeHashMask[EndBit-StartBit+1];
	return Result;

}
#endif

int32_t FoeHashFun(struct FoePriKey *key,enum FoeEntryState TargetState)
{

	uint32_t index=0;
	uint32_t buf[3];
	struct FoeEntry *entry;

#if defined(CONFIG_RA_HW_NAT_HASH0)
	if(key->fmt == IPV4_NAPT) {
            buf[0] = (key->ipv4.sip & FOE_HASH_MASK);
            buf[0] += (key->ipv4.dip & FOE_HASH_MASK);
            buf[0] += (key->ipv4.sport & FOE_HASH_MASK);
            buf[0] += (key->ipv4.dport & FOE_HASH_MASK);
        }else if(key->fmt == IPV4_NAT) {
            buf[0] = (key->ipv4.sip & FOE_HASH_MASK);
            buf[0] += (key->ipv4.dip & FOE_HASH_MASK);
        }else if(key->fmt == IPV6_ROUTING) {
            buf[0] = (key->ipv6.dip0 & FOE_HASH_MASK);
            buf[0] += (key->ipv6.dip1 & FOE_HASH_MASK);
            buf[0] += (key->ipv6.dip2 & FOE_HASH_MASK);
            buf[0] += (key->ipv6.dip3 & FOE_HASH_MASK);
        }else {
	    NAT_PRINT("Wrong MFT value\n");
	}

	switch(FOE_4TB_SIZ)
	{
	case 1024:
		index = (buf[0] & FOE_1K_SIZ_MASK) * FOE_HASH_WAY;
		break;
	case 2048:
		index = (buf[0] & FOE_2K_SIZ_MASK) * FOE_HASH_WAY;
		break;
	case 4096:
		index = (buf[0] & FOE_4K_SIZ_MASK) * FOE_HASH_WAY;
		break;
	case 8192:
		index = (buf[0] & FOE_8K_SIZ_MASK) * FOE_HASH_WAY;
		break;
	case 16384:
		index = (buf[0] & FOE_16K_SIZ_MASK) * FOE_HASH_WAY;
		break;
	default:
		return -1;
	}

#elif defined(CONFIG_RA_HW_NAT_HASH1)
	if(key->fmt == IPV4_NAPT) {
	    buf[0] = key->ipv4.sport<<16 | key->ipv4.dport;
	    buf[1] = key->ipv4.dip;
	    buf[2] = key->ipv4.sip;
	}else if (key->fmt == IPV4_NAT) {
	    buf[0] = (key->ipv4.dip<<16) | (key->ipv4.sip & 0xFFFF);
	    buf[1] = key->ipv4.dip;
	    buf[2] = key->ipv4.sip;
	}else if (key->fmt == IPV6_ROUTING) {
	    buf[0] = key->ipv6.dip0 ^ key->ipv6.dip3;
	    buf[1] = key->ipv6.dip1 ^ 0x0;
	    buf[2] = key->ipv6.dip2 ^ 0x0;
	}else {
	    NAT_PRINT("Wrong MFT value\n");
	}

	switch(FOE_4TB_SIZ)
	{
	case 1024:
		index =  (ExtractBits(buf, 0, 8) ^ ExtractBits(buf, 9, 17) 
				^ ExtractBits(buf, 18, 26) ^ ExtractBits(buf, 27, 35) 
				^ ExtractBits(buf, 36, 44) ^ ExtractBits(buf, 45, 53)
				^ ExtractBits(buf, 54, 62) ^ ExtractBits(buf,63, 71) 
				^ ExtractBits(buf, 72, 80) ^ ExtractBits(buf, 81, 89) 
				^ ExtractBits(buf, 90,95)) * FOE_HASH_WAY;
		break;
	case 2048:
		index =  (ExtractBits(buf, 0, 9) ^ ExtractBits(buf, 10, 19) 
				^ ExtractBits(buf, 20, 29) ^ ExtractBits(buf, 30, 39) 
				^ ExtractBits(buf, 40, 49) ^ ExtractBits(buf, 50, 59)
				^ ExtractBits(buf, 60, 69) ^ ExtractBits(buf,70, 79) 
				^ ExtractBits(buf, 80, 89) ^ ExtractBits(buf, 90, 95) 
			 ) * FOE_HASH_WAY;
		break;
	case 4096:
		index =  (ExtractBits(buf, 0, 10) ^ ExtractBits(buf, 11, 21) 
				^ ExtractBits(buf, 22, 32) ^ ExtractBits(buf, 33, 43) 
				^ ExtractBits(buf, 44, 54) ^ ExtractBits(buf, 55, 65)
				^ ExtractBits(buf, 66, 76) ^ ExtractBits(buf, 77, 87) 
				^ ExtractBits(buf, 88, 95)) * FOE_HASH_WAY;
		break;
	case 8192:
		index =  (ExtractBits(buf, 0, 11) ^ ExtractBits(buf, 12, 23) 
				^ ExtractBits(buf, 24, 35) ^ ExtractBits(buf, 36, 47) 
				^ ExtractBits(buf, 48, 59) ^ ExtractBits(buf, 60, 71)
				^ ExtractBits(buf, 72, 83) ^ ExtractBits(buf, 84, 95) 
			 ) * FOE_HASH_WAY;
		break;
	case 16384:
		index =  (ExtractBits(buf, 0, 12) ^ ExtractBits(buf, 13, 25) 
				^ ExtractBits(buf, 26, 38) ^ ExtractBits(buf, 39, 51) 
				^ ExtractBits(buf, 52, 64) ^ ExtractBits(buf, 65, 77)
				^ ExtractBits(buf, 78, 90) ^ ExtractBits(buf, 91, 95) 
			 ) * FOE_HASH_WAY;
		break;
	default:
		return -1;
	}

#endif

	do {
		entry = &PpeFoeBase[index];

		//case1.want to search binding entry
		//case2.want to insert duplicate entry
		if(TargetState==BIND || 
		  (TargetState==INVALID && entry->bfib1.state == BIND)) {
		    if(key->fmt == IPV4_NAPT) {
			if(entry->bfib1.state == BIND && 
		  	   entry->sip==key->ipv4.sip && 
			   entry->dip==key->ipv4.dip && 
			   entry->sport== key->ipv4.sport &&
			   entry->dport==key->ipv4.dport && 
			   entry->bfib1.t_u==key->ipv4.is_udp){
			    return index;
			}
		    }else if(key->fmt == IPV4_NAT){
			if(entry->bfib1.state == BIND && 
		  	   entry->sip==key->ipv4.sip && 
			   entry->dip==key->ipv4.dip) {
			    return index;
			}
		    }else if (key->fmt == IPV6_ROUTING ) {
			if(entry->bfib1.state == BIND &&
			   entry->ipv6_dip0==key->ipv6.dip0 && 
			   entry->ipv6_dip1==key->ipv6.dip1 && 
			   entry->ipv6_dip2==key->ipv6.dip2 &&
			   entry->ipv6_dip3==key->ipv6.dip3){
			    return index;
			}
		    } else {
			NAT_PRINT("Wrong MFT value\n");
		    }
		}else if(TargetState==INVALID) 
		{
		    // empty entry found
		    if(entry->bfib1.state == INVALID) 
		    {
			return index;
		    }
		} 
	} while( ((index++)%FOE_HASH_WAY) ==0 );

	//entry not found
	return -1;

}


void FoeTblClean(void)
{
	uint32_t FoeTblSize;

	FoeTblSize = FOE_4TB_SIZ * sizeof(struct FoeEntry);
	memset(PpeFoeBase, 0, FoeTblSize);

}


