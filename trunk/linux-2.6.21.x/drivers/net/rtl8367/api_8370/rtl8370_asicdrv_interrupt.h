#ifndef _RTL8370_ASICDRV_INTERRUPT_H_
#define _RTL8370_ASICDRV_INTERRUPT_H_

#include "rtl8370_asicdrv.h"

enum RTL8370_INTR_IMRS
{
    IMRS_LINK_CHANGE,
    IMRS_METER_EXCEED,
    IMRS_L2_LEARN,
    IMRS_SPEED_CHANGE,
    IMRS_SPECIAL_CONGESTION,
    IMRS_GREEN_FEATURE,
    IMRS_LOOP_DETECTION,
    IMRS_8051,
    IMRS_RESERVED,
    IMRS_MAX,    
};

enum RTL8370_INTR_INDICATOR
{
	INTRST_L2_LEARN = 0,
	INTRST_SPEED_CHANGE,
	INTRST_SPECIAL_CONGESTION,
	INTRST_PORT_LINKDOWN,
	INTRST_PORT_LINKUP,
	INTRST_METER0_15,
	INTRST_METER16_31,
	INTRST_METER32_47,
	INTRST_METER48_63,
	INTRST_METER64_79,
	INTRST_METER80_95,
	INTRST_METER96_111,
	INTRST_METER112_127,
	INTRST_METER128_143,
	INTRST_METER144_159,
	INTRST_METER160_175,
	INTRST_METER176_191,
	INTRST_METER192_207,
	INTRST_METER208_223,
	INTRST_METER224_239,
	INTRST_METER240_255,
	INTRST_MAX,
};

extern ret_t rtl8370_setAsicInterruptPolarity(uint32 polarity);
extern ret_t rtl8370_getAsicInterruptPolarity(uint32* polarity);
extern ret_t rtl8370_setAsicInterruptMask(uint32 imr);
extern ret_t rtl8370_getAsicInterruptMask(uint32* imr);
extern ret_t rtl8370_setAsicInterruptStatus(uint32 ims);
extern ret_t rtl8370_getAsicInterruptStatus(uint32* ims);
extern ret_t rtl8370_setAsicInterruptRelatedStatus(uint32 type,uint32 status);
extern ret_t rtl8370_getAsicInterruptRelatedStatus(uint32 type,uint32* status);


#endif /*#ifndef _RTL8370_ASICDRV_INTERRUPT_H_*/

