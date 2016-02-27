/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 13305 $
 * $Date: 2010-10-13 11:08:57 +0800 (星期三, 13 十月 2010) $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_lut.h"

#if defined(CONFIG_RTL8370_ASICDRV_TEST)
rtl8370_fdbtb Rtl8370sVirtualFdbTb;
extern uint16 Rtl8370sVirtualReg[];

#endif

void _rtl8370_fdbStUser2Smi( rtl8370_luttb *lut, rtl8370_fdbtb *fdbSmi);
void _rtl8370_fdbStSmi2User( rtl8370_luttb *lut, rtl8370_fdbtb *fdbSmi);
void _rtl8370_mac_hash_algorithm(ether_addr_t *mac_addr, uint16 fid, uint16 efid, uint16 *result);
void _rtl8370_ip_hash_algorithm(ipaddr_t *sip, ipaddr_t *dip, uint16 *result);

/*
@func ret_t | rtl8370_setAsicLutIpMulticastLookup | Set Lut IP multicast lookup function.
@parm uint32 | enabled | Lut IP multicast checking function 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@common
    ASIC will auto learn and write L2 look up entry. Auto learning L2 look up  table contained DMAC and source port information only. System supports L2 entry
    with IP multicast DIP/SIP to forward IP multicasting frame as user desired. If this function is enabled, then system will be looked up L2 IP multicast entry to 
    forward IP multicast frame directly without flooding. The L2 IP multicast forwarding path can be as port mask and not as same as auto learn L2 enrty with source 
    port information only. Both IP_MULT and Static fields of LUT must be wrote by software and these fields of auto learn entries will be 0 by ASIC.    
    
*/
ret_t rtl8370_setAsicLutIpMulticastLookup(uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_IPMCAST_LOOKUP_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsicLutIpMulticastLookup | Get Lut IP multicast lookup function setting.
@parm uint32* | enabled | L2 IP multicast checking function 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get Lut table IP multicast lookup usage.
    
*/
ret_t rtl8370_getAsicLutIpMulticastLookup(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_IPMCAST_LOOKUP_OFFSET, enabled);
}
/*
@func ret_t | rtl8370_setAsicLutAgeTimerSpeed | Set LUT agging out speed
@parm uint32 | timer | Agging out timer 0:Has been aged out.
@parm uint32 | speed | Agging out speed 0-fastest 3-slowest.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_OUT_OF_RANGE | LUT aging parameter out of range.
@comm
     The API can set LUT agging out period for each entry. Differet {timer, speed} parameter will make asic do agging out task
     at different period time. Following description is times period for different timer,speed combination setting.
            Timer        1        2        3        4        5        6        7
    Speed
    0                    14.3s    28.6s    42.9s    57.2s    1.19m    1.43m    1.67m
    1                    28.6s    57.2s    1.43m    1.9m     2.38m    2.86m    3.34m
    2                    57.2s    1.9m     2.86m    3.81m    4.77m    5.72m    6.68m
    3                    1.9m     3.8m     5.72m    7.63m    9.54m    11.45m   13.36m
     (s:Second m:Minute)
 */
ret_t rtl8370_setAsicLutAgeTimerSpeed( uint32 timer, uint32 speed)
{
    if(timer > RTL8370_LUT_AGETIMERMAX)
        return RT_ERR_OUT_OF_RANGE;

    if(speed > RTL8370_LUT_AGESPEEDMAX)
        return RT_ERR_OUT_OF_RANGE;
    
    return rtl8370_setAsicRegBits(RTL8370_REG_LUT_CFG, RTL8370_AGE_TIMER_MASK | RTL8370_AGE_SPEED_MASK, (timer << RTL8370_AGE_TIMER_OFFSET) | (speed << RTL8370_AGE_SPEED_OFFSET));        
}

/*
@func ret_t | rtl8370_getAsicLutAgeTimerSpeed | Set LUT agging out speed
@parm uint32* | timer | Agging out timer 0:Has been aged out.
@parm uint32* | speed | Agging out speed 0-fastest 3-slowest.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
     The API can get LUT agging out period for each entry. 
 */
ret_t rtl8370_getAsicLutAgeTimerSpeed( uint32* timer, uint32* speed)
{
    uint32 regData;
    ret_t retVal;

    retVal = rtl8370_getAsicReg(RTL8370_REG_LUT_CFG,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *timer =  (regData & RTL8370_AGE_TIMER_MASK) >> RTL8370_AGE_TIMER_OFFSET;
    
    *speed =  (regData & RTL8370_AGE_SPEED_MASK) >> RTL8370_AGE_SPEED_OFFSET;

    return RT_ERR_OK;

}
/*
@func ret_t | rtl8370_setAsicLutCamTbUsage | Configure Lut CAM table usage.
@parm uint32 | enabled | L2 CAM table usage 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@common
    System support 64 CAM entries only. Fields of CAM entry are as same as L2 LUT except without IP_MULT field. It means that ASIC will not checking IP multicast
    frame by CAM lookup. ASIC will lookup CAM by entry location sequence (0>1>...>62>63). ASIC looks up L2 LUT and get a hit while receiving frame, then it will 
    abandon look up result from CAM. As same as L2 LUT writing rule by ASIC auto learning, ASIC will not over write CAM entry contained Static field is not 0.
    Only while 4 entries (4 way hash) in L2 LUT are all not free (Static is not 0), then ASIC will write auto learn result to CAM.  
*/
ret_t rtl8370_setAsicLutCamTbUsage(uint32 enabled)
{
    return rtl8370_setAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_BCAM_DISABLE_OFFSET, enabled ? 0 : 1);
}

/*
@func ret_t | rtl8370_getAsicLutCamTbUsage | Configure Lut CAM table usage.
@parm uint32* | enabled | L2 CAM table usage 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@common
    The API can get LUT CAM usage setting
*/
ret_t rtl8370_getAsicLutCamTbUsage(uint32* enabled)
{
    ret_t   retVal;
    uint32  regData;

    if ((retVal = rtl8370_getAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_BCAM_DISABLE_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    *enabled = regData ? 0 : 1;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicLutCamType | Configure Lut CAM type.
@parm uint32 | type | L2 CAM tyep 0: analog BCAM, 1: Digit BCAM. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@common
    The API can set LUT BCAM type. There are two types of BCAM, one is analog BCAM and the other is digit BCAM.
*/
ret_t rtl8370_setAsicLutCamType(uint32 type)
{
    return rtl8370_setAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_BCAM_TYPE_OFFSET, type);
}
/*
@func ret_t | rtl8370_getAsicLutCamType | Configure Lut CAM type.
@parm uint32* | type | L2 CAM tyep 0: analog BCAM, 1: Digit BCAM. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@common
    The API can set LUT BCAM type. There are two types of BCAM, one is analog BCAM and the other is digit BCAM.
*/
ret_t rtl8370_getAsicLutCamType(uint32* type)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_BCAM_TYPE_OFFSET,type);
}

/*
@func ret_t | rtl8370_setAsicLutLearnLimitNo | Set per-Port auto learning limit number
@parm uint32 | port | The port number
@parm uint32 | number | ASIC auto learning entries limit number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_LIMITED_L2ENTRY_NUM | Invalid auto learning limit number
@common
    The API can set per-port ASIC auto learning limit number
*/
ret_t rtl8370_setAsicLutLearnLimitNo(uint32 port,uint32 number)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(number > RTL8370_LUT_LEARNLIMITMAX)
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    return rtl8370_setAsicReg(RTL8370_LUT_PORT_LEARN_LIMITNO_REG(port), number);
}
/*
@func ret_t | rtl8370_getAsicLutLearnLimitNo | Get per-Port auto learning limit number
@parm uint32 | port | The port number
@parm uint32* | number | ASIC auto learning entries limit number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@common
    The API can get per-port ASIC auto learning limit number
*/
ret_t rtl8370_getAsicLutLearnLimitNo(uint32 port,uint32* number)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicReg(RTL8370_LUT_PORT_LEARN_LIMITNO_REG(port), number);
}

/*
@func ret_t | rtl8370_setAsicLutLearnOverAct | Configure auto learn over limit number action.
@parm uint32 | action | Learn over action 0:normal, 1:drop 2:trap. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_NOT_ALLOWED | Invalid learn over action
@common
    The API can set ASIC treat SA unknown packet while auto learn limit number is over 
*/
ret_t rtl8370_setAsicLutLearnOverAct(uint32 action)
{
    if(action >= LRNOVERACT_MAX)
        return RT_ERR_NOT_ALLOWED;
        
    return rtl8370_setAsicRegBits(RTL8370_REG_PORT_SECURITY_CTRL, RTL8370_LUT_LEARN_OVER_ACT_MASK, action);
}
/*
@func ret_t | rtl8370_getAsicLutLearnOverAct | Configure auto learn over limit number action.
@parm uint32* | action | Learn over action 0:normal, 1:drop 2:trap. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@common
    The API can get ASIC treat SA unknown packet while auto learn limit number is over 
*/
ret_t rtl8370_getAsicLutLearnOverAct(uint32* action)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_PORT_SECURITY_CTRL, RTL8370_LUT_LEARN_OVER_ACT_MASK, action);
}

void _rtl8370_fdbStUser2Smi( rtl8370_luttb *lut, rtl8370_fdbtb *fdbSmi)
{
    /*L3 lookup*/
    if(lut->ipmul)
    {
        fdbSmi->smi_ipmul.static_bit    = lut->static_bit;
        fdbSmi->smi_ipmul.reserved1     = 0;
        fdbSmi->smi_ipmul.macpri        = lut->macpri;
        fdbSmi->smi_ipmul.da_en         = lut->da_en;
        fdbSmi->smi_ipmul.sa_en         = lut->sa_en;

        fdbSmi->smi_ipmul.portmask1     = lut->portmask&0x00FF;
        fdbSmi->smi_ipmul.portmask2     = (lut->portmask&0xFF00)>>8;
#ifdef _LITTLE_ENDIAN
        fdbSmi->smi_ipmul.sip0          = (lut->sip & 0xFF000000) >> 24;
        fdbSmi->smi_ipmul.sip1          = (lut->sip & 0x00FF0000) >> 16;
        fdbSmi->smi_ipmul.sip2          = (lut->sip & 0x0000FF00) >> 8;
        fdbSmi->smi_ipmul.sip3          = lut->sip & 0x000000FF;

        fdbSmi->smi_ipmul.dip0          = (lut->dip & 0xFF000000) >> 24;
        fdbSmi->smi_ipmul.dip1          = (lut->dip & 0x00FF0000) >> 16;
        fdbSmi->smi_ipmul.dip2          = (lut->dip & 0x0000FF00) >> 8;
        fdbSmi->smi_ipmul.dip3          = lut->dip & 0x000000FF;
#else
        fdbSmi->smi_ipmul.sip0          = lut->sip & 0x000000FF;
        fdbSmi->smi_ipmul.sip1          = (lut->sip & 0x0000FF00) >> 8;
        fdbSmi->smi_ipmul.sip2          = (lut->sip & 0x00FF0000) >> 16;
        fdbSmi->smi_ipmul.sip3          = (lut->sip & 0xFF000000) >> 24;

        fdbSmi->smi_ipmul.dip0          = lut->dip & 0x000000FF;
        fdbSmi->smi_ipmul.dip1          = (lut->dip & 0x0000FF00) >> 8;
        fdbSmi->smi_ipmul.dip2          = (lut->dip & 0x00FF0000) >> 16;
        fdbSmi->smi_ipmul.dip3          = (lut->dip & 0xFF000000) >> 24;

#endif
        fdbSmi->smi_ipmul.reserved2     = 0;
        fdbSmi->smi_ipmul.ipmul         = 1;
        fdbSmi->smi_ipmul.reserved3     = 0;

        /*clear valid bit while static is reset*/
        //fdbSmi->smi_ipmul.valid         = lut->static_bit;
        fdbSmi->smi_ipmul.valid         = 1;    
        fdbSmi->smi_ipmul.reserved4     = 0;
    }
    /*Multicast L2 Lookup*/
    else if((lut->mac.octet[0])&0x01)
    {
        fdbSmi->smi_l2mul.static_bit    = lut->static_bit;
        fdbSmi->smi_l2mul.block         = lut->block;;
        fdbSmi->smi_l2mul.auth          = lut->auth;;
        fdbSmi->smi_l2mul.macpri        = lut->macpri;
        fdbSmi->smi_l2mul.da_en         = lut->da_en;
        fdbSmi->smi_l2mul.sa_en         = lut->sa_en;

        fdbSmi->smi_l2mul.portmask1     = lut->portmask&0x00FF;
        fdbSmi->smi_l2mul.portmask2     = (lut->portmask&0xFF00)>>8;

         fdbSmi->smi_l2mul.mac0         = lut->mac.octet[0];
         fdbSmi->smi_l2mul.mac1         = lut->mac.octet[1];
         fdbSmi->smi_l2mul.mac2         = lut->mac.octet[2];
         fdbSmi->smi_l2mul.mac3         = lut->mac.octet[3];
         fdbSmi->smi_l2mul.mac4         = lut->mac.octet[4];
         fdbSmi->smi_l2mul.mac5         = lut->mac.octet[5];

        fdbSmi->smi_l2mul.fid1          = lut->fid&0xFF ;
        fdbSmi->smi_l2mul.fid2          = (lut->fid&0xF00)>>8 ;
        fdbSmi->smi_l2mul.efid          = lut->efid;
        fdbSmi->smi_l2mul.ipmul         = 0;
        fdbSmi->smi_l2mul.reserved1     = 0;

        /*clear valid bit while static is reset*/
        //fdbSmi->smi_l2mul.valid         = lut->static_bit;
        fdbSmi->smi_l2mul.valid         = 1;    
        fdbSmi->smi_l2mul.reserved2     = 0;
    }
    /*Asic auto-learning*/
    else 
    {
        fdbSmi->smi_auto.static_bit     = lut->static_bit;
        fdbSmi->smi_auto.block          = lut->block;;
        fdbSmi->smi_auto.auth           = lut->auth;;
        fdbSmi->smi_auto.macpri            = lut->macpri;
        fdbSmi->smi_auto.da_en          = lut->da_en;
        fdbSmi->smi_auto.sa_en          = lut->sa_en;
        fdbSmi->smi_auto.spa            = lut->spa;
        fdbSmi->smi_auto.age            = lut->age;
        fdbSmi->smi_auto.reserved1      = 0;

        fdbSmi->smi_auto.reserved2      = 0;

         fdbSmi->smi_auto.mac0          = lut->mac.octet[0];
         fdbSmi->smi_auto.mac1          = lut->mac.octet[1];
         fdbSmi->smi_auto.mac2          = lut->mac.octet[2];
         fdbSmi->smi_auto.mac3          = lut->mac.octet[3];
         fdbSmi->smi_auto.mac4          = lut->mac.octet[4];
         fdbSmi->smi_auto.mac5          = lut->mac.octet[5];

        fdbSmi->smi_auto.fid1           = lut->fid&0xFF ;
        fdbSmi->smi_auto.fid2           = (lut->fid&0xF00)>>8 ;
        fdbSmi->smi_auto.efid           = lut->efid;
        fdbSmi->smi_auto.ipmul          = 0;
        fdbSmi->smi_auto.reserved3      = 0;

        /*clear valid bit while STATIC_BIT and AGE_BIT are reset*/
        //if(lut->static_bit == 0 && lut->age == 0)
        //{
        //    fdbSmi->smi_auto.valid      = lut->static_bit;
        //}
        fdbSmi->smi_auto.valid          = 1;
        fdbSmi->smi_auto.reserved4      = 0;
    }
}

void _rtl8370_fdbStSmi2User( rtl8370_luttb *lut, rtl8370_fdbtb *fdbSmi)
{
    /*L3 lookup*/
    if(fdbSmi->smi_ipmul.ipmul)
    {
        lut->sip            = fdbSmi->smi_ipmul.sip0;
        lut->sip            = (lut->sip << 8) | fdbSmi->smi_ipmul.sip1;
        lut->sip            = (lut->sip << 8) | fdbSmi->smi_ipmul.sip2;
        lut->sip            = (lut->sip << 8) | fdbSmi->smi_ipmul.sip3;

#ifdef _LITTLE_ENDIAN
        lut->dip            = fdbSmi->smi_ipmul.dip0;
#else
        lut->dip            = fdbSmi->smi_ipmul.dip0 | 0xE0;
#endif
        lut->dip            = (lut->dip << 8) | fdbSmi->smi_ipmul.dip1;
        lut->dip            = (lut->dip << 8) | fdbSmi->smi_ipmul.dip2;
#ifdef _LITTLE_ENDIAN
        lut->dip            = (lut->dip << 8) | fdbSmi->smi_ipmul.dip3 | 0xE0;
#else
        lut->dip            = (lut->dip << 8) | fdbSmi->smi_ipmul.dip3;
#endif
        lut->ipmul          = fdbSmi->smi_ipmul.ipmul;

        lut->static_bit     = fdbSmi->smi_ipmul.static_bit;
        lut->macpri            = fdbSmi->smi_ipmul.macpri;
        lut->da_en          = fdbSmi->smi_ipmul.da_en;
        lut->sa_en          = fdbSmi->smi_ipmul.sa_en;

        lut->portmask       = (fdbSmi->smi_ipmul.portmask2 << 8) | fdbSmi->smi_ipmul.portmask1;
        lut->valid          = fdbSmi->smi_ipmul.valid;        
    }
    /*Multicast L2 Lookup*/
    else if((fdbSmi->smi_l2mul.mac0)&0x01)
    {
         lut->mac.octet[0]  = fdbSmi->smi_l2mul.mac0;
         lut->mac.octet[1]  = fdbSmi->smi_l2mul.mac1;
         lut->mac.octet[2]  = fdbSmi->smi_l2mul.mac2;
         lut->mac.octet[3]  = fdbSmi->smi_l2mul.mac3;
         lut->mac.octet[4]  = fdbSmi->smi_l2mul.mac4;
         lut->mac.octet[5]  = fdbSmi->smi_l2mul.mac5;

        lut->fid            = (fdbSmi->smi_l2mul.fid2<< 8) | fdbSmi->smi_l2mul.fid1;
        lut->efid           = fdbSmi->smi_l2mul.efid;
        lut->ipmul          = 0;

        lut->static_bit     = fdbSmi->smi_l2mul.static_bit;
        lut->block          = fdbSmi->smi_l2mul.block;
        lut->auth           = fdbSmi->smi_l2mul.auth;
        lut->macpri         = fdbSmi->smi_l2mul.macpri;
        lut->da_en          = fdbSmi->smi_l2mul.da_en;
        lut->sa_en          = fdbSmi->smi_l2mul.sa_en;

        lut->portmask       = (fdbSmi->smi_l2mul.portmask2 << 8) | fdbSmi->smi_l2mul.portmask1;
        lut->valid          = fdbSmi->smi_l2mul.valid;
    }
    /*Asic auto-learning*/
    else 
    {
         lut->mac.octet[0]   = fdbSmi->smi_auto.mac0;
         lut->mac.octet[1]   = fdbSmi->smi_auto.mac1;
         lut->mac.octet[2]   = fdbSmi->smi_auto.mac2;
         lut->mac.octet[3]   = fdbSmi->smi_auto.mac3;
         lut->mac.octet[4]   = fdbSmi->smi_auto.mac4;
         lut->mac.octet[5]   = fdbSmi->smi_auto.mac5;

        lut->fid            = (fdbSmi->smi_auto.fid2<< 8) | fdbSmi->smi_auto.fid1;
        lut->efid           = fdbSmi->smi_auto.efid;
        lut->ipmul          = 0;

        lut->static_bit     = fdbSmi->smi_auto.static_bit;
        lut->block          = fdbSmi->smi_auto.block;
        lut->auth           = fdbSmi->smi_auto.auth;
        lut->macpri         = fdbSmi->smi_auto.macpri;
        lut->da_en          = fdbSmi->smi_auto.da_en;
        lut->sa_en          = fdbSmi->smi_auto.sa_en;

        lut->spa            = fdbSmi->smi_auto.spa;
        lut->age            = fdbSmi->smi_auto.age;
        lut->valid          = fdbSmi->smi_auto.valid;        
    }
}
/*
@func ret_t | rtl8370_setAsicL2LookupTb | Configure filtering database.
@parm rtl8370_luttb * | l2Table | L2 table entry writing to 8K+64 filtering database
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_L2_INDEXTBL_FULL |L2 entry if full with the same hash value.
@common
    The API can set LUT table to both L2 and BCAM. Asic will write lut entry based on hashing key
    data to hashed entry. ASIC writes L2 entries using 8k 4-way hash method. if 4-way entries are not 
    free(available), then ASIC will write it to 64 BCAM entries. L2 entries' address are 0~8191(0x0000~01FFF) and BCAM
    address are 8192~8255(0x1000~0x203F).
*/
ret_t rtl8370_setAsicL2LookupTb(rtl8370_luttb *l2Table)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;
    rtl8370_fdbtb smil2Table;


    memset(&smil2Table,0x00,sizeof(rtl8370_fdbtb));
    _rtl8370_fdbStUser2Smi(l2Table, &smil2Table);

    accessPtr =  (uint16*)&smil2Table;
    regData = *accessPtr;
    for(i=0; i<7; i++)
    {
        retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_DATA_BASE + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;

    }

    /* Write Command */
    retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_CTRL_REG, RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_WRITE,TB_TARGET_L2));
    if(retVal != RT_ERR_OK)
        return retVal;

    /*Read access status*/
    retVal = rtl8370_getAsicRegBit(RTL8370_REG_TABLE_LUT_ADDR, RTL8370_HIT_STATUS_OFFSET,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    l2Table->lookup_hit = regData;
    if(!l2Table->lookup_hit)
        return RT_ERR_L2_INDEXTBL_FULL;
    
    /*Read access address*/
    retVal = rtl8370_getAsicRegBits(RTL8370_REG_TABLE_LUT_ADDR, RTL8370_TABLE_LUT_ADDR_ADDRESS_MASK,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    l2Table->address = regData;

#if defined(CONFIG_RTL8370_ASICDRV_TEST)
    memcpy((uint16*)&Rtl8370sVirtualFdbTb,(uint16*)&Rtl8370sVirtualReg[RTL8370_TABLE_ACCESS_DATA_BASE],14);
#endif

    return RT_ERR_OK;
}
/*
@func ret_t | rtl8370_getAsicL2LookupTb | Configure filtering database.
@parm enum RTL8370_LUTREADMETHOD | method | access method 1:specify address 0:specify address.
@parm rtl8370_luttb * | l2Table | L2 table entry writing to 8K+64 filtering database
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_L2_ENTRY_NOTFOUND |L2 entry not found.
@common
    The API can get LUT table to both L2 and BCAM. 
*/
ret_t rtl8370_getAsicL2LookupTb(enum RTL8370_LUTREADMETHOD method, rtl8370_luttb *l2Table)
{
    ret_t retVal;
    uint32 regData;
    uint16* accessPtr;
    uint32 i;
    uint32 cam_parser;
    rtl8370_fdbtb smil2Table;


    if(LUTREADMETHOD_ADDRESS == method)
    {
        retVal = rtl8370_setAsicReg(RTL8370_REG_TABLE_ACCESS_ADDR, l2Table->address);
        if(retVal !=  RT_ERR_OK)
            return retVal;
    }
    else
    {
        memset(&smil2Table,0x00,sizeof(rtl8370_fdbtb));
        _rtl8370_fdbStUser2Smi(l2Table, &smil2Table);

        accessPtr =  (uint16*)&smil2Table;
        regData = *accessPtr;
        for(i = 0; i < 7; i++)
        {
            retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_DATA_BASE + i, regData);
            if(retVal !=  RT_ERR_OK)
                return retVal;

            accessPtr ++;
            regData = *accessPtr;
            
        }
    }

    /* Read Command */
    if(LUTREADMETHOD_ADDRESS == method)
        retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_CTRL_REG, RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_READ,TB_TARGET_L2) | RTL8370_ACCESS_METHOD_MASK);
    else
        retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_CTRL_REG, RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_READ,TB_TARGET_L2));
        
    if(retVal !=  RT_ERR_OK)
        return retVal;

#if defined(CONFIG_RTL8370_ASICDRV_TEST)
    memcpy((uint16*)&Rtl8370sVirtualReg[RTL8370_TABLE_ACCESS_DATA_BASE],(uint16*)&Rtl8370sVirtualFdbTb,14);
#endif

    /*Read access status*/
    retVal = rtl8370_getAsicRegBit(RTL8370_REG_TABLE_LUT_ADDR, RTL8370_HIT_STATUS_OFFSET,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    if(LUTREADMETHOD_MAC == method)
    {
        l2Table->lookup_hit = regData;
        if(!l2Table->lookup_hit)
            return RT_ERR_L2_ENTRY_NOTFOUND;

    /*Read access address*/
    retVal = rtl8370_getAsicRegBits(RTL8370_REG_TABLE_LUT_ADDR, RTL8370_TYPE_MASK | RTL8370_TABLE_LUT_ADDR_ADDRESS_MASK,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    l2Table->address = regData;
    }

    /*read L2 entry */
       memset(&smil2Table,0x00,sizeof(rtl8370_fdbtb));

    cam_parser = 0;
    if ((LUTREADMETHOD_ADDRESS== method)&&(l2Table->address & 0x2000))
    {
        /*Check CAM Table Valid Bit*/
        retVal = rtl8370_getAsicReg(RTL8370_REG_TABLE_ACCESS_DATA6,&regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;
        cam_parser = regData&0x0001;
    }

    if ((LUTREADMETHOD_MAC==method)||(l2Table->address<0x2000)||(cam_parser!=0))
    {
        accessPtr = (uint16*)&smil2Table;
        for(i=0;i<7;i++)
        {
            retVal = rtl8370_getAsicReg(RTL8370_TABLE_ACCESS_DATA_BASE+i,&regData);
            if(retVal !=  RT_ERR_OK)
                return retVal;

            *accessPtr = regData;

            accessPtr ++;
        }
    }

    _rtl8370_fdbStSmi2User(l2Table, &smil2Table);

    return RT_ERR_OK;
}

void _rtl8370_mac_hash_algorithm(ether_addr_t *mac_addr, uint16 fid, uint16 efid, uint16 *result)
{
    uint16 mac_bit[48], fid_bit[12], efid_bit[3];
    uint16 i;
    uint16 index[11];
    uint16 idx;

    memset(index, 0, sizeof(index[11]));
    
    for (i = 0; i < 48; i++)
    {
        mac_bit[47-i] = (mac_addr->octet[i/8]&(0x80>>(i%8)))>>(7-i%8);
    }    

    for (i = 0; i < 12; i++)
    {
        fid_bit[i] = (fid&(0x0001<<i))>>i;
    }

    for (i = 0; i < 3; i++)
    {
        efid_bit[i] = (efid&(0x0001<<i))>>i;
    }
    
    index[0]=  mac_bit[0]^mac_bit[11]^mac_bit[22]^mac_bit[33]^mac_bit[44]^fid_bit[7];
    index[1]=  mac_bit[1]^mac_bit[12]^mac_bit[23]^mac_bit[34]^mac_bit[45]^fid_bit[8];
    index[2]=  mac_bit[2]^mac_bit[13]^mac_bit[24]^mac_bit[35]^mac_bit[46]^fid_bit[9];
    index[3]=  mac_bit[3]^mac_bit[14]^mac_bit[25]^mac_bit[36]^mac_bit[47]^fid_bit[10];    
    index[4]=  mac_bit[4]^mac_bit[15]^mac_bit[26]^mac_bit[37]^fid_bit[0]^fid_bit[11];
    index[5]=  mac_bit[5]^mac_bit[16]^mac_bit[27]^mac_bit[38]^fid_bit[1]^efid_bit[0];
    index[6]=  mac_bit[6]^mac_bit[17]^mac_bit[28]^mac_bit[39]^fid_bit[2]^efid_bit[1];
    index[7]=  mac_bit[7]^mac_bit[18]^mac_bit[29]^mac_bit[40]^fid_bit[3]^efid_bit[2];    
    index[8]=  mac_bit[8]^mac_bit[19]^mac_bit[30]^mac_bit[41]^fid_bit[4];
    index[9]=  mac_bit[9]^mac_bit[20]^mac_bit[31]^mac_bit[42]^fid_bit[5];
    index[10]=mac_bit[10]^mac_bit[21]^mac_bit[32]^mac_bit[43]^fid_bit[6];
    
    idx = 0;

    for (i = 0; i < 11; i++)
    {
        idx = idx |(index[i] << i);
    }

    *result = idx;
    
}

void _rtl8370_ip_hash_algorithm(ipaddr_t *sip, ipaddr_t *dip, uint16 *result)
{
    uint16 sip_bit[32], dip_bit[32];
    uint16 i;
    uint16 index[11];
    uint16 idx;

    memset(index , 0, sizeof(index[11]));
    
    for (i=0;i<32;i++)
    {
        sip_bit[i] = (*sip & (0x1 << i)) >> i;
        dip_bit[i] = (*dip & (0x1 << i)) >> i;
    }    

    
    index[0]=  sip_bit[0]^sip_bit[11]^sip_bit[22]^dip_bit[1]^dip_bit[12]^dip_bit[23];
    index[1]=  sip_bit[1]^sip_bit[12]^sip_bit[23]^dip_bit[2]^dip_bit[13]^dip_bit[24];
    index[2]=  sip_bit[2]^sip_bit[13]^sip_bit[24]^dip_bit[3]^dip_bit[14]^dip_bit[25];
    index[3]=  sip_bit[3]^sip_bit[14]^sip_bit[25]^dip_bit[4]^dip_bit[15]^dip_bit[26];    
    index[4]=  sip_bit[4]^sip_bit[15]^sip_bit[26]^dip_bit[5]^dip_bit[16]^dip_bit[27];
    index[5]=  sip_bit[5]^sip_bit[16]^sip_bit[27]^dip_bit[6]^dip_bit[17]^0;
    index[6]=  sip_bit[6]^sip_bit[17]^sip_bit[28]^dip_bit[7]^dip_bit[18]^0;
    index[7]=  sip_bit[7]^sip_bit[18]^sip_bit[29]^dip_bit[8]^dip_bit[19]^0;    
    index[8]=  sip_bit[8]^sip_bit[19]^sip_bit[30]^dip_bit[9]^dip_bit[20]^0;
    index[9]=  sip_bit[9]^sip_bit[20]^sip_bit[31]^dip_bit[10]^dip_bit[21]^0;
    index[10]=sip_bit[10]^sip_bit[21]^dip_bit[0]^dip_bit[11]^dip_bit[22]^0;
    
    idx = 0;

    for (i = 0; i < 11; i++)
    {
        idx = idx |(index[i] << i);
    }

    *result = idx;
    
}


/*
@func ret_t | rtl8370_getAsicLutLearnNo | Get per-Port auto learning number
@parm uint32 | port | The port number
@parm uint32* | number | ASIC auto learning entries number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@common
    The API can get per-port ASIC auto learning number
*/
ret_t rtl8370_getAsicLutLearnNo(uint32 port,uint32* number)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicReg(RTL8370_REG_L2_LRN_CNT_REG(port), number);
}


/*
@func ret_t | rtl8370_setAsicLutLinkDownForceAging | Set LUT link down aging setting.
@parm uint32 | port | Physical port number.
@parm uint32 | disable | 0: enable aging; 1: disabling aging
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid disable.
@comm
     This API can be used to get L2 LUT link down aging function. 
*/
ret_t rtl8370_setAsicLutLinkDownForceAging(uint32 enable)
{
    if(enable > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_LINKDOWN_AGEOUT_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicLutLinkDownForceAging | Get LUT link down aging setting.
@parm uint32 | port | Physical port number.
@parm uint32* | disable | 0: enable aging; 1: disabling aging
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
     This API can be used to get L2 LUT link down aging function. 
*/
ret_t rtl8370_getAsicLutLinkDownForceAging(uint32* enable)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_LUT_CFG, RTL8370_LINKDOWN_AGEOUT_OFFSET, enable);
}
