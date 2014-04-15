#ifndef _RTL8370_ASICDRV_SVLAN_H_
#define _RTL8370_ASICDRV_SVLAN_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_C2SIDXNO                     128
#define RTL8370_C2SIDXMAX                   (RTL8370_C2SIDXNO-1)
#define RTL8370_MC2SIDXNO                  32
#define RTL8370_MC2SIDXMAX                 (RTL8370_MC2SIDXNO-1)
#define RTL8370_SP2CIDXNO                       128
#define RTL8370_SP2CMAX                      (RTL8370_SP2CIDXNO-1)


enum RTL8370_SPRISEL
{
    SPRISEL_INTERNALPRI =  0,
    SPRISEL_CTAGPRI,
    SPRISEL_VSPRI,
    SPRISEL_MAX
};

typedef struct  rtl8370_svlan_memconf_s{

    uint16 vs_relaysvid:12;
    uint16 vs_msti:4;
    
    uint16 vs_member;

    uint16 vs_fid:12;
    uint16 vs_priority:3;
    
    uint16 vs_svid:12;  
    uint16 vs_efiden:1; 
    uint16 vs_efid:3;   

}rtl8370_svlan_memconf_t;

typedef struct  rtl8370_svlan_memconf_smi_s{
#ifdef _LITTLE_ENDIAN
    uint16 vs_relaysvid:12;
    uint16 vs_msti:4;
    
    uint16 vs_member;

    uint16 vs_fid:12;
    uint16 vs_priority:3;
    uint16 reserved:1;
    
    uint16 vs_svid:12;  
    uint16 vs_efiden:1; 
    uint16 vs_efid:3;   

#else
    uint16 vs_msti:4;
    uint16 vs_relaysvid:12;
    
    uint16 vs_member;

    uint16 reserved:1;
    uint16 vs_priority:3;
    uint16 vs_fid:12;
    
    uint16 vs_efid:3;   
    uint16 vs_efiden:1; 
    uint16 vs_svid:12;  

#endif
}rtl8370_svlan_memconf_smi_t;


typedef struct  rtl8370_svlan_c2s_smi_s{

#ifdef _LITTLE_ENDIAN

    uint16 svidx:6;
    uint16 reserved:10;
    
    uint16 c2senPmsk;
    
    uint16 evid:13;
    uint16 reserved2:3;
    
#else

    uint16 reserved:10;
    uint16 svidx:6;
    
    uint16 c2senPmsk;
    
    uint16 reserved2:3;
    uint16 evid:13;

#endif
}rtl8370_svlan_c2s_smi_t;


typedef struct  rtl8370_svlan_mc2s_s{

    uint16 valid:1; 
    uint16 format:1;
    uint16 svidx:6;	
    uint32 value;
    uint32 mask;
}rtl8370_svlan_mc2s_t;

typedef struct  rtl8370_svlan_mc2s_smi_s{

#ifdef _LITTLE_ENDIAN

    uint16 svidx:6;
    uint16 format:1;
    uint16 valid:1; 
    uint16 reserved:8;

    uint16 mask0:8;
    uint16 mask1:8;
	
    uint16 mask2:8;
    uint16 mask3:8;

    uint16 data0:8;
    uint16 data1:8;
	
    uint16 data2:8;
    uint16 data3:8;	

#else
    uint16 reserved:8;
    uint16 valid:1; 
    uint16 format:1;
    uint16 svidx:6;

    uint16 mask1:8;
    uint16 mask0:8;
	
    uint16 mask3:8;
    uint16 mask2:8;

    uint16 data1:8;
    uint16 data0:8;
	
    uint16 data3:8;
    uint16 data2:8;
    
#endif
}rtl8370_svlan_mc2s_smi_t;

typedef struct  rtl8370_svlan_s2c_s{

    uint16 svid:12; 
    uint16 dstport:4;
    uint32 evid:13;
}rtl8370_svlan_s2c_t;

typedef struct  rtl8370_svlan_s2c_smi_s{

#ifdef _LITTLE_ENDIAN

    uint16 dstport:4;
    uint16 svid:12;
    uint16 evid:13; 
    uint16 reserved:3;
#else
    uint16 svid:12;
    uint16 dstport:4; 
    uint16 reserved:3;	
    uint16 evid:13;    
#endif
}rtl8370_svlan_s2c_smi_t;


extern ret_t rtl8370_setAsicSvlanIngressUntag(uint32 enabled);
extern ret_t rtl8370_getAsicSvlanIngressUntag(uint32* enabled);
extern ret_t rtl8370_setAsicSvlanIngressUnmatch(uint32 enabled);
extern ret_t rtl8370_getAsicSvlanIngressUnmatch(uint32* enabled);
extern ret_t rtl8370_setAsicSvlanTrapPriority(uint32 priority);
extern ret_t rtl8370_getAsicSvlanTrapPriority(uint32* priority);
extern ret_t rtl8370_setAsicSvlanDefaultVlan(uint32 index);
extern ret_t rtl8370_getAsicSvlanDefaultVlan(uint32* index);

extern ret_t rtl8370_setAsicSvlanMemberConfiguration(uint32 index,rtl8370_svlan_memconf_t* svlanMemConf);
extern ret_t rtl8370_getAsicSvlanMemberConfiguration(uint32 index,rtl8370_svlan_memconf_t* svlanMemConf);

extern ret_t rtl8370_setAsicSvlanPrioritySel(uint32 prisel);
extern ret_t rtl8370_getAsicSvlanPrioritySel(uint32* prisel);
extern ret_t rtl8370_setAsicSvlanTpid(uint32 protocolType);
extern ret_t rtl8370_getAsicSvlanTpid(uint32* protocolType);
extern ret_t rtl8370_setAsicSvlanUplinkPortMask(uint32 portMask);
extern ret_t rtl8370_getAsicSvlanUplinkPortMask(uint32* portMask);
extern ret_t rtl8370_setAsicSvlanEgressUnassign(uint32 enabled);
extern ret_t rtl8370_getAsicSvlanEgressUnassign(uint32* enabled);
extern ret_t rtl8370_setAsicSvlanC2SConf(uint32 index,uint32 evid, uint32 pmsk, uint32 svidx);
extern ret_t rtl8370_getAsicSvlanC2SConf(uint32 index,uint32* evid, uint32* pmsk, uint32* svidx);
extern ret_t rtl8370_setAsicSvlanMC2SConf(uint32 index,rtl8370_svlan_mc2s_t* svlanMC2SConf);
extern ret_t rtl8370_getAsicSvlanMC2SConf(uint32 index,rtl8370_svlan_mc2s_t* svlanMC2SConf);
extern ret_t rtl8370_setAsicSvlanSP2CConf(uint32 index,rtl8370_svlan_s2c_t* svlanSP2CConf);
extern ret_t rtl8370_getAsicSvlanSP2CConf(uint32 index,rtl8370_svlan_s2c_t* svlanSP2CConf);	
#endif /*#ifndef _RTL8370_ASICDRV_SVLAN_H_*/

