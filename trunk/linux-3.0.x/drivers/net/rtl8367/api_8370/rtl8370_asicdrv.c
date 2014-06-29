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
 * $Revision: 1.1.1.1 $
 * $Date: 2010/12/02 04:34:26 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv.h"

#ifdef EMBEDDED_SUPPORT

extern void setReg(uint16, uint16);
extern uint16 getReg(uint16);

ret_t rtl8370_setAsicRegBit(uint32 reg, uint32 bitIdx, uint32 value)
{ 
    uint16 tmp;

    if(reg > RTL8370_REGDATAMAX || value > 1)
	    return RT_ERR_INPUT;
	
	tmp = getReg(reg);
	tmp &= (1 <<bitIdx);
	tmp |= (value << bitIdx);
	setReg(reg, tmp);
    
    return RT_ERR_OK;
}

ret_t rtl8370_getAsicRegBit(uint32 reg, uint32 bitIdx, uint32 *value)
{
    uint16 tmp;

    if(reg > RTL8370_REGDATAMAX )
	    return RT_ERR_INPUT;

	tmp = getReg(reg);
	tmp = tmp >> bitIdx;
	tmp &= 1;
	*value = tmp;

    return RT_ERR_OK;
}

ret_t rtl8370_setAsicRegBits(uint32 reg, uint32 bits, uint32 value)
{
    uint32 regData;    
    uint32 bitsShift;    
    uint32 valueShifted;        

    if(reg > RTL8370_REGDATAMAX )
	    return RT_ERR_INPUT;

    if(bits >= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;    

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8370_REGDATAMAX)
        return RT_ERR_INPUT;

    regData = getReg(reg);
    regData = regData & (~bits);
    regData = regData | (valueShifted & bits);	    

	setReg(reg, regData);

    return RT_ERR_OK;
}

ret_t rtl8370_getAsicRegBits(uint32 reg, uint32 bits, uint32 *value)
{
    uint32 regData;    
    uint32 bitsShift;    

    if(reg > RTL8370_REGDATAMAX )
	    return RT_ERR_INPUT;

    if(bits>= (1UL<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;    

    bitsShift = 0;
    while(!(bits & (1UL << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    regData = getReg(reg);
    *value = (regData & bits) >> bitsShift;	    

    return RT_ERR_OK;
}

ret_t rtl8370_setAsicReg(uint32 reg, uint32 value)
{
    if(reg > RTL8370_REGDATAMAX || value > RTL8370_REGDATAMAX )
	    return RT_ERR_INPUT;

    setReg(reg, value);

    return RT_ERR_OK;
}

ret_t rtl8370_getAsicReg(uint32 reg, uint32 *value)
{
    if(reg > RTL8370_REGDATAMAX  )
	    return RT_ERR_INPUT;

	*value = getReg(reg);

    return RT_ERR_OK;
}

#else

extern int  smi_read(uint32 addr, uint32 *data);
extern int  smi_write(uint32 addr, uint32 data);

/*for driver verify testing only*/
#ifdef CONFIG_RTL8370_ASICDRV_TEST
#define RTL8370_VIRTUAL_REG_SIZE        0x10000
uint16 Rtl8370sVirtualReg[RTL8370_VIRTUAL_REG_SIZE];
#endif

#if defined(CONFIG_RTL865X_CLE) || defined (RTK_X86_CLE)
uint32 cleDebuggingDisplay;
#endif
/*=======================================================================
 * 1. Asic read/write driver through SMI
 *========================================================================*/
/*
@func ret_t | rtl8370_setAsicRegBit | Set a bit value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bit | Bit location. For 16-bits register only. Maximun value is 15 for MSB location.
@parm uint32 | value | Value to set. It can be value 0 or 1.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter. 
@comm
    Set a bit of a specified register to 1 or 0. It is 16-bits system of RTL8366s chip.
    
*/
ret_t rtl8370_setAsicRegBit(uint32 reg, uint32 bit, uint32 value)
{

#if defined(RTK_X86_ASICDRV)
    uint32 regData;
    ret_t retVal;
    
    if(bit >= RTL8370_REGBITLENGTH)
        return RT_ERR_INPUT;

    retVal = Access_Read(reg, 2, &regData);
    if (retVal != TRUE) 
		return RT_ERR_SMI;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);

    if (value) 
        regData = regData | (1<<bit);
    else
        regData = regData & ~(1<<bit);

    retVal = Access_Write(reg, 2, regData);
    if (retVal != TRUE) 
		return RT_ERR_SMI;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,regData);

    
#elif defined(CONFIG_RTL8370_ASICDRV_TEST)

    if(bit>=RTL8370_REGBITLENGTH)
        return RT_ERR_INPUT;
    else if(reg >= RTL8370_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    if (value) 
    {
        Rtl8370sVirtualReg[reg] =  Rtl8370sVirtualReg[reg] | (1<<bit);

    }
    else
    {
        Rtl8370sVirtualReg[reg] =  Rtl8370sVirtualReg[reg] & (~(1<<bit));
    }
    
    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,Rtl8370sVirtualReg[reg]);

    
#else
    uint32 regData;
    ret_t retVal;
    
    if(bit>=RTL8370_REGBITLENGTH)
        return RT_ERR_INPUT;

    retVal = smi_read(reg, &regData);
    if(retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);
  #endif
    if (value) 
        regData = regData | (1<<bit);
    else
        regData = regData & (~(1<<bit));
    
    retVal = smi_write(reg, regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
  
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,regData);
  #endif

#endif
    return RT_ERR_OK;
}

ret_t rtl8370_getAsicRegBit(uint32 reg, uint32 bit, uint32 *value)
{
    
#if defined(RTK_X86_ASICDRV)/*RTK-CNSD2-NickWu-20061222: for x86 compile*/

    uint32 regData;
    ret_t retVal;

    if(bit>=RTL8370_REGBITLENGTH)
        return RT_ERR_INPUT;    
    
    retVal = Access_Read(reg, 2, &regData);    
    if (retVal != TRUE) 
		return RT_ERR_SMI;    
    
    *value = (regData & (0x1 << bit)) >> bit;    

    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);

#elif defined(CONFIG_RTL8370_ASICDRV_TEST)

    if(bit>=RTL8370_REGBITLENGTH)
        return RT_ERR_INPUT;    

    if(reg >= RTL8370_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    *value = (Rtl8370sVirtualReg[reg] & (0x1 << bit)) >> bit;    

    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,Rtl8370sVirtualReg[reg]);
    
#else
    uint32 regData;
    ret_t retVal;

    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);
  #endif
    
    *value = (regData & (0x1 << bit)) >> bit;    

#endif
    return RT_ERR_OK;
}


/*
@func ret_t | rtl8370_setAsicRegBits | Set bits value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bits | Bits mask for setting. 
@parm uint32 | value | Bits value for setting. Value of bits will be set with mapping mask bit is 1.   
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter. 
@comm
    Set bits of a specified register to value. Both bits and value are be treated as bit-mask.
    
*/
ret_t rtl8370_setAsicRegBits(uint32 reg, uint32 bits, uint32 value)
{
    
#if defined(RTK_X86_ASICDRV)/*RTK-CNSD2-NickWu-20061222: for x86 compile*/

    uint32 regData;    
    ret_t retVal;    
    uint32 bitsShift;    
    uint32 valueShifted;        

    if(bits >= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;    

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8370_REGDATAMAX)
        return RT_ERR_INPUT;

    retVal = Access_Read(reg, 2, &regData);
    if (retVal != TRUE) 
		return RT_ERR_SMI;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);

    regData = regData & (~bits);
    regData = regData | (valueShifted & bits);

    retVal = Access_Write(reg, 2, regData);
    if (retVal != TRUE) 
		return RT_ERR_SMI;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,regData);
    
#elif defined(CONFIG_RTL8370_ASICDRV_TEST)
    uint32 regData;    
    uint32 bitsShift;    
    uint32 valueShifted;        

    if(bits>= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;    

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    valueShifted = value << bitsShift;

    if(valueShifted > RTL8370_REGDATAMAX)
        return RT_ERR_INPUT;

    if(reg >= RTL8370_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    regData = Rtl8370sVirtualReg[reg] & (~bits);
    regData = regData | (valueShifted & bits);
    
    Rtl8370sVirtualReg[reg] = regData;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,regData);
    
#else
    uint32 regData;    
    ret_t retVal;    
    uint32 bitsShift;    
    uint32 valueShifted;        

    if(bits>= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    valueShifted = value << bitsShift;

    if(valueShifted > RTL8370_REGDATAMAX)
        return RT_ERR_INPUT;

    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);
  #endif

    regData = regData & (~bits);
    regData = regData | (valueShifted & bits);

    retVal = smi_write(reg, regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,regData);
  #endif
#endif
    return RT_ERR_OK;
}

ret_t rtl8370_getAsicRegBits(uint32 reg, uint32 bits, uint32 *value)
{
    
#if defined(RTK_X86_ASICDRV)/*RTK-CNSD2-NickWu-20061222: for x86 compile*/

    uint32 regData;    
    ret_t retVal;    
    uint32 bitsShift;    

    if(bits>= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;    

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    
    retVal = Access_Read(reg, 2, &regData);
    if (retVal != TRUE) 
		return RT_ERR_SMI;

    *value = (regData & bits) >> bitsShift;
    
    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);

#elif defined(CONFIG_RTL8370_ASICDRV_TEST)
   
    uint32 bitsShift;    

    if(bits>= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;
    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    
    if(reg >= RTL8370_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

     *value = (Rtl8370sVirtualReg[reg] & bits) >> bitsShift;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,Rtl8370sVirtualReg[reg]);
    
#else
    uint32 regData;    
    ret_t retVal;    
    uint32 bitsShift;    

    if(bits>= (1<<RTL8370_REGBITLENGTH) )
        return RT_ERR_INPUT;    

    bitsShift = 0;
    while(!(bits & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8370_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    
    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;

    *value = (regData & bits) >> bitsShift;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);
  #endif

#endif
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicReg | Set content of asic register.
@parm uint32 | reg | Register's address.
@parm uint32 | value | Value setting to register.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The value will be set to ASIC mapping address only and it is always return RT_ERR_OK while setting un-mapping address registers.
    
*/
ret_t rtl8370_setAsicReg(uint32 reg, uint32 value)
{

#if defined(RTK_X86_ASICDRV)/*RTK-CNSD2-NickWu-20061222: for x86 compile*/

    ret_t retVal;

    retVal = Access_Write(reg, 2, value);
    if (retVal != TRUE) 
		return RT_ERR_SMI;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,value);

#elif defined(CONFIG_RTL8370_ASICDRV_TEST)

    /*MIBs emulating*/
    if(reg == RTL8370_REG_MIB_ADDRESS)
    {
        Rtl8370sVirtualReg[RTL8370_MIB_COUNTER_BASE_REG] = 0x1;
        Rtl8370sVirtualReg[RTL8370_MIB_COUNTER_BASE_REG+1] = 0x2;
        Rtl8370sVirtualReg[RTL8370_MIB_COUNTER_BASE_REG+2] = 0x3;
        Rtl8370sVirtualReg[RTL8370_MIB_COUNTER_BASE_REG+3] = 0x4;
    }
    
    if(reg >= RTL8370_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    Rtl8370sVirtualReg[reg] = value;

    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,Rtl8370sVirtualReg[reg]);

#else
    ret_t retVal;

    retVal = smi_write(reg, value);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,value);
  #endif

#endif
    return RT_ERR_OK;
}


/*
@func ret_t | rtl8370_getAsicReg | Get content of register.
@parm uint32 | reg | Register's address.
@parm uint32* | value | Value of register.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
     Value 0x0000 will be returned for ASIC un-mapping address.
    
*/
ret_t rtl8370_getAsicReg(uint32 reg, uint32 *value)
{
    
#if defined(RTK_X86_ASICDRV)/*RTK-CNSD2-NickWu-20061222: for x86 compile*/

    uint32 regData;
    ret_t retVal;

    retVal = Access_Read(reg, 2, &regData);
    if (retVal != TRUE) 
		return RT_ERR_SMI;

    *value = regData;
    
    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);

#elif defined(CONFIG_RTL8370_ASICDRV_TEST)

    if(reg >= RTL8370_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    *value = Rtl8370sVirtualReg[reg];

    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,Rtl8370sVirtualReg[reg]);
    
#else
    uint32 regData;
    ret_t retVal;

    retVal = smi_read(reg, &regData);
    if (retVal != RT_ERR_OK) 
		return RT_ERR_SMI;

    *value = regData;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8370 == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg,regData);
  #endif

#endif
    return RT_ERR_OK;
}

#endif
