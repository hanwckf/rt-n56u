#ifndef _RTL8367B_ASICDRV_SVLAN_H_
#define _RTL8367B_ASICDRV_SVLAN_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_C2SIDXNO               128
#define RTL8367B_C2SIDXMAX              (RTL8367B_C2SIDXNO-1)
#define RTL8367B_MC2SIDXNO              32
#define RTL8367B_MC2SIDXMAX             (RTL8367B_MC2SIDXNO-1)
#define RTL8367B_SP2CIDXNO              128
#define RTL8367B_SP2CMAX        		(RTL8367B_SP2CIDXNO-1)


enum RTL8367B_SPRISEL
{
    SPRISEL_INTERNALPRI =  0,
    SPRISEL_CTAGPRI,
    SPRISEL_VSPRI,
    SPRISEL_PBPRI,
    SPRISEL_END
};

enum RTL8367B_SUNACCEPT
{
    SUNACCEPT_DROP =  0,
    SUNACCEPT_TRAP,
    SUNACCEPT_SVLAN,
    SUNACCEPT_END
};

enum RTL8367B_SVLAN_MC2S_MODE
{
    SVLAN_MC2S_MODE_MAC =  0,
    SVLAN_MC2S_MODE_IP,
    SVLAN_MC2S_MODE_END
};


typedef struct  rtl8367b_svlan_memconf_s{

    rtk_uint16 vs_member:8;
    rtk_uint16 vs_untag:8;

    rtk_uint16 vs_fid_msti:4;
    rtk_uint16 vs_priority:3;
    rtk_uint16 vs_force_fid:1;
    rtk_uint16 reserved:8;

    rtk_uint16 vs_svid:12;
    rtk_uint16 vs_efiden:1;
    rtk_uint16 vs_efid:3;


}rtl8367b_svlan_memconf_t;

typedef struct  rtl8367b_svlan_memconf_smi_s{
#ifdef _LITTLE_ENDIAN

    rtk_uint16 vs_member:8;
	rtk_uint16 vs_untag:8;

    rtk_uint16 vs_fid_msti:4;
    rtk_uint16 vs_priority:3;
    rtk_uint16 vs_force_fid:1;
    rtk_uint16 reserved:8;

    rtk_uint16 vs_svid:12;
    rtk_uint16 vs_efiden:1;
    rtk_uint16 vs_efid:3;

#else
	rtk_uint16 vs_untag:8;
    rtk_uint16 vs_member:8;

    rtk_uint16 reserved:8;
    rtk_uint16 vs_force_fid:1;
    rtk_uint16 vs_priority:3;
    rtk_uint16 vs_fid_msti:4;

    rtk_uint16 vs_efid:3;
    rtk_uint16 vs_efiden:1;
    rtk_uint16 vs_svid:12;

#endif
}rtl8367b_svlan_memconf_smi_t;


typedef struct  rtl8367b_svlan_c2s_smi_s{

#ifdef _LITTLE_ENDIAN

    rtk_uint16 svidx:6;
    rtk_uint16 reserved:10;

    rtk_uint16 c2senPmsk:8;
    rtk_uint16 reserved2:8;

    rtk_uint16 evid:13;
    rtk_uint16 reserved3:3;

#else

    rtk_uint16 reserved:10;
    rtk_uint16 svidx:6;

    rtk_uint16 reserved2:8;
    rtk_uint16 c2senPmsk:8;

    rtk_uint16 reserved3:3;
    rtk_uint16 evid:13;

#endif
}rtl8367b_svlan_c2s_smi_t;


typedef struct  rtl8367b_svlan_mc2s_s{

    rtk_uint16 valid:1;
    rtk_uint16 format:1;
    rtk_uint16 svidx:6;
    rtk_uint32 sdata;
    rtk_uint32 smask;
}rtl8367b_svlan_mc2s_t;

typedef struct  rtl8367b_svlan_mc2s_smi_s{

#ifdef _LITTLE_ENDIAN

    rtk_uint16 svidx:6;
    rtk_uint16 format:1;
    rtk_uint16 valid:1;
    rtk_uint16 reserved:8;

    rtk_uint16 mask0:8;
    rtk_uint16 mask1:8;

    rtk_uint16 mask2:8;
    rtk_uint16 mask3:8;

    rtk_uint16 data0:8;
    rtk_uint16 data1:8;

    rtk_uint16 data2:8;
    rtk_uint16 data3:8;

#else
    rtk_uint16 reserved:8;
    rtk_uint16 valid:1;
    rtk_uint16 format:1;
    rtk_uint16 svidx:6;

    rtk_uint16 mask1:8;
    rtk_uint16 mask0:8;

    rtk_uint16 mask3:8;
    rtk_uint16 mask2:8;

    rtk_uint16 data1:8;
    rtk_uint16 data0:8;

    rtk_uint16 data3:8;
    rtk_uint16 data2:8;

#endif
}rtl8367b_svlan_mc2s_smi_t;

typedef struct  rtl8367b_svlan_s2c_s{

	rtk_uint16 valid:1;
    rtk_uint16 svidx:6;
    rtk_uint16 dstport:3;
    rtk_uint32 vid:12;
}rtl8367b_svlan_s2c_t;

typedef struct  rtl8367b_svlan_s2c_smi_s{

#ifdef _LITTLE_ENDIAN

    rtk_uint16 dstport:3;
    rtk_uint16 svidx:6;
	rtk_uint16 reserved_1:7;

    rtk_uint16 vid:12;
	rtk_uint16 valid:1;
    rtk_uint16 reserved_2:3;
#else
	rtk_uint16 reserved_1:7;
    rtk_uint16 svidx:6;
    rtk_uint16 dstport:3;

    rtk_uint16 reserved_2:3;
	rtk_uint16 valid:1;
    rtk_uint16 vid:12;
#endif
}rtl8367b_svlan_s2c_smi_t;

extern ret_t rtl8367b_setAsicSvlanIngressUntag(rtk_uint32 mode);
extern ret_t rtl8367b_getAsicSvlanIngressUntag(rtk_uint32* pMode);
extern ret_t rtl8367b_setAsicSvlanIngressUnmatch(rtk_uint32 mode);
extern ret_t rtl8367b_getAsicSvlanIngressUnmatch(rtk_uint32* pMode);
extern ret_t rtl8367b_setAsicSvlanTrapPriority(rtk_uint32 priority);
extern ret_t rtl8367b_getAsicSvlanTrapPriority(rtk_uint32* pPriority);
extern ret_t rtl8367b_setAsicSvlanDefaultVlan(rtk_uint32 port, rtk_uint32 index);
extern ret_t rtl8367b_getAsicSvlanDefaultVlan(rtk_uint32 port, rtk_uint32* pIndex);

extern ret_t rtl8367b_setAsicSvlanMemberConfiguration(rtk_uint32 index,rtl8367b_svlan_memconf_t* pSvlanMemCfg);
extern ret_t rtl8367b_getAsicSvlanMemberConfiguration(rtk_uint32 index,rtl8367b_svlan_memconf_t* pSvlanMemCfg);

extern ret_t rtl8367b_setAsicSvlanPrioritySel(rtk_uint32 priSel);
extern ret_t rtl8367b_getAsicSvlanPrioritySel(rtk_uint32* pPriSel);
extern ret_t rtl8367b_setAsicSvlanTpid(rtk_uint32 protocolType);
extern ret_t rtl8367b_getAsicSvlanTpid(rtk_uint32* pProtocolType);
extern ret_t rtl8367b_setAsicSvlanUplinkPortMask(rtk_uint32 portMask);
extern ret_t rtl8367b_getAsicSvlanUplinkPortMask(rtk_uint32* pPortmask);
extern ret_t rtl8367b_setAsicSvlanEgressUnassign(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicSvlanEgressUnassign(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicSvlanC2SConf(rtk_uint32 index, rtk_uint32 evid, rtk_uint32 portmask, rtk_uint32 svidx);
extern ret_t rtl8367b_getAsicSvlanC2SConf(rtk_uint32 index, rtk_uint32* pEvid, rtk_uint32* pPortmask, rtk_uint32* pSvidx);
extern ret_t rtl8367b_setAsicSvlanMC2SConf(rtk_uint32 index,rtl8367b_svlan_mc2s_t* pSvlanMc2sCfg);
extern ret_t rtl8367b_getAsicSvlanMC2SConf(rtk_uint32 index,rtl8367b_svlan_mc2s_t* pSvlanMc2sCfg);
extern ret_t rtl8367b_setAsicSvlanSP2CConf(rtk_uint32 index,rtl8367b_svlan_s2c_t* pSvlanSp2cCfg);
extern ret_t rtl8367b_getAsicSvlanSP2CConf(rtk_uint32 index,rtl8367b_svlan_s2c_t* pSvlanSp2cCfg);
extern ret_t rtl8367b_setAsicSvlanDmacCvidSel(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicSvlanDmacCvidSel(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicSvlanUntagVlan(rtk_uint32 index);
extern ret_t rtl8367b_getAsicSvlanUntagVlan(rtk_uint32* pIndex);
extern ret_t rtl8367b_setAsicSvlanUnmatchVlan(rtk_uint32 index);
extern ret_t rtl8367b_getAsicSvlanUnmatchVlan(rtk_uint32* pIndex);

#endif /*#ifndef _RTL8367B_ASICDRV_SVLAN_H_*/

