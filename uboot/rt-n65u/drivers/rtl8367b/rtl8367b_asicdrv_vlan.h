#ifndef _RTL8367B_ASICDRV_VLAN_H_
#define _RTL8367B_ASICDRV_VLAN_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include "rtl8367b_asicdrv.h"

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define RTL8367B_PROTOVLAN_GIDX_MAX 3
#define RTL8367B_PROTOVLAN_GROUPNO  4


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef struct  VLANCONFIGSMI
{
#ifdef _LITTLE_ENDIAN
	rtk_uint16	mbr:8;
	rtk_uint16  reserved:8;

	rtk_uint16	fid_msti:4;
	rtk_uint16  reserved2:12;
	
	rtk_uint16	vbpen:1;
	rtk_uint16	vbpri:3;
	rtk_uint16	envlanpol:1;
	rtk_uint16	meteridx:5;
	rtk_uint16	reserved3:6;

	rtk_uint16	evid:13;
	rtk_uint16  reserved4:3;
#else
	rtk_uint16  reserved:8;
	rtk_uint16	mbr:8;

	rtk_uint16  reserved2:12;
	rtk_uint16	fid_msti:4;
	
	rtk_uint16	reserved3:6;
	rtk_uint16	meteridx:5;
	rtk_uint16	envlanpol:1;
	rtk_uint16	vbpri:3;
	rtk_uint16	vbpen:1;

	rtk_uint16  reserved4:3;
	rtk_uint16	evid:13;
#endif
	
}rtl8367b_vlanconfigsmi;

typedef struct  VLANCONFIGUSER
{
    rtk_uint16 	evid;
	rtk_uint16 	mbr;
    rtk_uint16  fid_msti;
    rtk_uint16  envlanpol;
    rtk_uint16  meteridx;
    rtk_uint16  vbpen;
    rtk_uint16  vbpri;
}rtl8367b_vlanconfiguser;

typedef struct  VLANTABLE
{
#ifdef _LITTLE_ENDIAN
	rtk_uint16 	mbr:8;
 	rtk_uint16 	untag:8;

 	rtk_uint16 	fid_msti:4;
 	rtk_uint16 	vbpen:1;
	rtk_uint16	vbpri:3;
	rtk_uint16	envlanpol:1;
	rtk_uint16	meteridx:5;
	rtk_uint16	ivl_svl:1;	
	rtk_uint16	reserved:1;	
#else
 	rtk_uint16 	untag:8;
	rtk_uint16 	mbr:8;

	rtk_uint16	reserved:1;
	rtk_uint16	ivl_svl:1;	
	rtk_uint16	meteridx:5;
	rtk_uint16	envlanpol:1;
	rtk_uint16	vbpri:3;
 	rtk_uint16 	vbpen:1;
 	rtk_uint16 	fid_msti:4;

#endif
}rtl8367b_vlan4kentrysmi;

typedef struct  USER_VLANTABLE{

	rtk_uint16 	vid;
	rtk_uint16 	mbr;
 	rtk_uint16 	untag;
    rtk_uint16  fid_msti;
    rtk_uint16  envlanpol;
    rtk_uint16  meteridx;
    rtk_uint16  vbpen;
    rtk_uint16  vbpri;
	rtk_uint16 	ivl_svl;

}rtl8367b_user_vlan4kentry;

typedef enum
{
    FRAME_TYPE_BOTH = 0,
    FRAME_TYPE_TAGGED_ONLY,
    FRAME_TYPE_UNTAGGED_ONLY,
    FRAME_TYPE_MAX_BOUND
} rtl8367b_accframetype;

typedef enum
{
    EG_TAG_MODE_ORI = 0,
    EG_TAG_MODE_KEEP,
    EG_TAG_MODE_PRI_TAG,
    EG_TAG_MODE_REAL_KEEP,    
    EG_TAG_MODE_END
} rtl8367b_egtagmode;

typedef enum
{
    PPVLAN_FRAME_TYPE_ETHERNET = 0,
    PPVLAN_FRAME_TYPE_LLC,
    PPVLAN_FRAME_TYPE_RFC1042,
    PPVLAN_FRAME_TYPE_END
} rtl8367b_provlan_frametype;

enum RTL8367B_STPST
{
	STPST_DISABLED = 0,
	STPST_BLOCKING,
	STPST_LEARNING,
	STPST_FORWARDING
};


typedef struct
{
    rtl8367b_provlan_frametype  frameType;
    rtk_uint32                      etherType;
} rtl8367b_protocolgdatacfg;

typedef struct
{
    rtk_uint32 valid;
    rtk_uint32 vlan_idx;
    rtk_uint32 priority;
} rtl8367b_protocolvlancfg;


void _rtl8367b_VlanMCStUser2Smi(rtl8367b_vlanconfiguser *pVlanCg, rtl8367b_vlanconfigsmi *pSmiVlanCfg);
void _rtl8367b_VlanMCStSmi2User(rtl8367b_vlanconfigsmi *pSmiVlanCfg, rtl8367b_vlanconfiguser *pVlanCg);
void _rtl8367b_Vlan4kStUser2Smi(rtl8367b_user_vlan4kentry *pUserVlan4kEntry, rtl8367b_vlan4kentrysmi *pSmiVlan4kEntry);
void _rtl8367b_Vlan4kStSmi2User(rtl8367b_vlan4kentrysmi *pSmiVlan4kEntry, rtl8367b_user_vlan4kentry *pUserVlan4kEntry);

extern ret_t rtl8367b_setAsicVlanMemberConfig(rtk_uint32 index, rtl8367b_vlanconfiguser *pVlanCg);
extern ret_t rtl8367b_getAsicVlanMemberConfig(rtk_uint32 index, rtl8367b_vlanconfiguser *pVlanCg);
extern ret_t rtl8367b_setAsicVlan4kEntry(rtl8367b_user_vlan4kentry *pVlan4kEntry );
extern ret_t rtl8367b_getAsicVlan4kEntry(rtl8367b_user_vlan4kentry *pVlan4kEntry );
extern ret_t rtl8367b_setAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367b_accframetype frameType);
extern ret_t rtl8367b_getAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367b_accframetype *pFrameType);
extern ret_t rtl8367b_setAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 *pEnable);
extern ret_t rtl8367b_setAsicVlanEgressTagMode(rtk_uint32 port, rtl8367b_egtagmode tagMode);
extern ret_t rtl8367b_getAsicVlanEgressTagMode(rtk_uint32 port, rtl8367b_egtagmode *pTagMode);
extern ret_t rtl8367b_setAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 index, rtk_uint32 pri);
extern ret_t rtl8367b_getAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 *pIndex, rtk_uint32 *pPri);
extern ret_t rtl8367b_setAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367b_protocolgdatacfg *pPbCfg);
extern ret_t rtl8367b_getAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367b_protocolgdatacfg *pPbCfg);
extern ret_t rtl8367b_setAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367b_protocolvlancfg *pPpbCfg);
extern ret_t rtl8367b_getAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367b_protocolvlancfg *pPpbCfg);
extern ret_t rtl8367b_setAsicVlanFilter(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicVlanFilter(rtk_uint32* pEnabled);

extern ret_t rtl8367b_setAsicPortBasedFid(rtk_uint32 port, rtk_uint32 fid);
extern ret_t rtl8367b_getAsicPortBasedFid(rtk_uint32 port, rtk_uint32* pFid);
extern ret_t rtl8367b_setAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32 state);
extern ret_t rtl8367b_getAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32* pState);
extern ret_t rtl8367b_setAsicVlanUntagDscpPriorityEn(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicVlanUntagDscpPriorityEn(rtk_uint32* enabled);
extern ret_t rtl8367b_setAsicVlanTransparent(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicVlanTransparent(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32* pPortmask);

#endif /*#ifndef _RTL8367B_ASICDRV_VLAN_H_*/

