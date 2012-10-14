#ifndef _RTL8370_ASICDRV_VLAN_H_
#define _RTL8370_ASICDRV_VLAN_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include "rtl8370_asicdrv.h"

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define RTL8370_METERIDXMAX        0x3F
#define RTL8370_PROTOVLAN_GIDX_MAX 3
#define RTL8370_PROTOVLAN_GROUPNO  4


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef struct  VLANCONFIGSMI
{
#ifdef _LITTLE_ENDIAN
	uint16	mbr:16;

	uint16	fid:12;
	uint16	msti:4;
	
	uint16	lurep:1;
	uint16	vbpen:1;
	uint16	vbpri:3;
	uint16	envlanpol:1;
	uint16	meteridx:8;
	uint16	reserved1:2;

	uint16	evid:13;
	uint16  reserved2:3;
#else
	uint16	mbr:16;

	uint16	msti:4;
	uint16	fid:12;
	
	uint16	reserved1:2;
	uint16	meteridx:8;
	uint16	envlanpol:1;
	uint16	vbpri:3;
	uint16	vbpen:1;
	uint16	lurep:1;

	uint16  reserved2:3;
	uint16	evid:13;
#endif
	
}rtl8370_vlanconfigsmi;

typedef struct  VLANCONFIGUSER
{
    uint16 	evid;
    uint16  lurep;
	uint16 	mbr;
    uint16  fid;
    uint16  msti;
    uint16  envlanpol;
    uint16  meteridx;
    uint16  vbpen;
    uint16  vbpri;
}rtl8370_vlanconfiguser;

typedef struct  VLANTABLE
{
#ifdef _LITTLE_ENDIAN
	uint16 	mbr;

 	uint16 	fid:12;
	uint16 	msti:4;
	
 	uint16 	lurep:1;
 	uint16 	vbpen:1;
	uint16	vbpri:3;
	uint16	envlanpol:1;
	uint16	meteridx:8;
	uint16	untag1:2;
	
	uint16	untag2:14;
	uint16	reserved1:2;
#else
	uint16 	mbr;

	uint16 	msti:4;
 	uint16 	fid:12;

	uint16	untag1:2;	
	uint16	meteridx:8;
	uint16	envlanpol:1;	
	uint16	vbpri:3;
 	uint16 	vbpen:1;
 	uint16 	lurep:1;
	
	uint16	reserved1:2;
	uint16	untag2:14;
#endif
}rtl8370_vlan4kentrysmi;

typedef struct  USER_VLANTABLE{

	uint16 	vid;
    uint16  lurep;
	uint16 	mbr;
 	uint16 	untag;
    uint16  fid;
    uint16  msti;
    uint16  envlanpol;
    uint16  meteridx;
    uint16  vbpen;
    uint16  vbpri;

}rtl8370_user_vlan4kentry;

typedef enum
{
    FRAME_TYPE_BOTH = 0,
    FRAME_TYPE_TAGGED_ONLY,
    FRAME_TYPE_UNTAGGED_ONLY,
    FRAME_TYPE_MAX_BOUND
} rtl8370_accframetype;

typedef enum
{
    EG_TAG_MODE_ORI = 0,
    EG_TAG_MODE_KEEP,
    EG_TAG_MODE_PRI_TAG,
    EG_TAG_MODE_REAL_KEEP,    
    EG_TAG_MODE_MAX_BOUND
} rtl8370_egtagmode;

typedef enum
{
    PPVLAN_FRAME_TYPE_ETHERNET = 0,
    PPVLAN_FRAME_TYPE_LLC,
    PPVLAN_FRAME_TYPE_RFC1042,
    PPVLAN_FRAME_TYPE_MAX_BOUND
} rtl8370_provlan_frametype;

enum RTL8370_STPST
{
	STPST_DISABLED = 0,
	STPST_BLOCKING,
	STPST_LEARNING,
	STPST_FORWARDING
};

typedef struct
{
    rtl8370_provlan_frametype  frame_type;
    uint32                      ether_type;
} rtl8370_protocolgdatacfg;

typedef struct
{
    uint32 valid;
    uint32 vlan_idx;
    uint32 priority;
} rtl8370_protocolvlancfg;


/****************************************************************/
/* Driver Proto Type Definition                                 */
/****************************************************************/
extern ret_t rtl8370_setAsicVlanMemberConfig(uint32 index, rtl8370_vlanconfiguser *ptr_vlancfg);
extern ret_t rtl8370_getAsicVlanMemberConfig(uint32 index, rtl8370_vlanconfiguser *ptr_vlancfg);
extern ret_t rtl8370_setAsicVlan4kEntry(rtl8370_user_vlan4kentry *ptr_vlan4kEntry );
extern ret_t rtl8370_getAsicVlan4kEntry(rtl8370_user_vlan4kentry *ptr_vlan4kEntry );
extern ret_t rtl8370_setAsicVlanAccpetFrameType(uint32 port, rtl8370_accframetype frame_type);
extern ret_t rtl8370_getAsicVlanAccpetFrameType(uint32 port, rtl8370_accframetype *ptr_frame_type);
extern ret_t rtl8370_setAsicVlanIngressFilter(uint32 port, uint32 enabled);
extern ret_t rtl8370_getAsicVlanIngressFilter(uint32 port, uint32 *ptr_enabled);
extern ret_t rtl8370_setAsicVlanEgressTagMode(uint32 port, rtl8370_egtagmode tag_mode);
extern ret_t rtl8370_getAsicVlanEgressTagMode(uint32 port, rtl8370_egtagmode *ptr_tag_mode);
extern ret_t rtl8370_setAsicVlanPortBasedVID(uint32 port, uint32 index, uint32 pri);
extern ret_t rtl8370_getAsicVlanPortBasedVID(uint32 port, uint32 *ptr_index, uint32 *ptr_pri);
extern ret_t rtl8370_setAsicVlanProtocolBasedGroupData(uint32 index, rtl8370_protocolgdatacfg *ptr_pbcfg);
extern ret_t rtl8370_getAsicVlanProtocolBasedGroupData(uint32 index, rtl8370_protocolgdatacfg *ptr_pbcfg);
extern ret_t rtl8370_setAsicVlanPortAndProtocolBased(uint32 port, uint32 index, rtl8370_protocolvlancfg *ptr_ppbcfg);
extern ret_t rtl8370_getAsicVlanPortAndProtocolBased(uint32 port, uint32 index, rtl8370_protocolvlancfg *ptr_ppbcfg);
extern ret_t rtl8370_setAsicVlanFilter(uint32 enabled);
extern ret_t rtl8370_getAsicVlanFilter(uint32* enabled);

extern ret_t rtl8370_setAsicPortBasedFid(uint32 port, uint32 fid);
extern ret_t rtl8370_getAsicPortBasedFid(uint32 port, uint32* fid);
extern ret_t rtl8370_setAsicPortBasedFidEn(uint32 port, uint32 enabled);
extern ret_t rtl8370_getAsicPortBasedFidEn(uint32 port, uint32* enabled);
extern ret_t rtl8370_setAsicSpanningTreeStatus(uint32 port, uint32 msti, uint32 state);
extern ret_t rtl8370_getAsicSpanningTreeStatus(uint32 port, uint32 msti, uint32* state);


#endif /*#ifndef _RTL8370_ASICDRV_VLAN_H_*/

