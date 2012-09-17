#ifndef _RTL8370_ASICDRV_LED_H_
#define _RTL8370_ASICDRV_LED_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_LEDGROUPMAX					2	
#define RTL8370_LEDGROUPMASK               0x7
#define RTL8370_LED_FORCE_MODE_BASE        RTL8370_REG_CPU_FORCE_LED0_CFG0
#define RTL8370_LED_FORCE_CTRL             RTL8370_REG_CPU_FORCE_LED_CFG


enum RTL8370_LEDOP{

    LEDOP_SCAN0=0,         
    LEDOP_SCAN1,        
    LEDOP_PARALLEL,        
    LEDOP_SERIAL, 
    LEDOP_MAX,
};

enum RTL8370_LEDSERACT{

    LEDSERACT_HIGH=0,         
    LEDSERACT_LOW,        
    LEDSERACT_MAX,
};

enum RTL8370_LEDSER{

    LEDSER_16G=0,         
    LEDSER_8G,        
    LEDSER_MAX,
};

enum RTL8370_LEDCONF{

    LEDCONF_LEDOFF=0,         
    LEDCONF_DUPCOL,        
    LEDCONF_LINK_ACT,        
    LEDCONF_SPD1000,        
    LEDCONF_SPD100,        
    LEDCONF_SPD10,            
    LEDCONF_SPD1000ACT,    
    LEDCONF_SPD100ACT,    
    LEDCONF_SPD10ACT,        
    LEDCONF_SPD10010ACT,  
    LEDCONF_LOOPDETECT,            
    LEDCONF_EEE,            
    LEDCONF_LINKRX,        
    LEDCONF_LINKTX,        
    LEDCONF_MASTER,        
    LEDCONF_ACT,    
};

enum RTL8370_LEDBLINKRATE{

	LEDBLINKRATE_32MS=0, 		
	LEDBLINKRATE_64MS,		
	LEDBLINKRATE_128MS,
	LEDBLINKRATE_256MS,
	LEDBLINKRATE_512MS,
	LEDBLINKRATE_1024MS,
	LEDBLINKRATE_48MS,
	LEDBLINKRATE_96MS,
	LEDBLINKRATE_MAX,

};

enum RTL8370_LEDFORCEMODE{

    LEDFORCEMODE_NORMAL=0,
    LEDFORCEMODE_BLINK,
    LEDFORCEMODE_OFF,
    LEDFORCEMODE_ON,
    LEDFORCEMODE_MAX,
};

enum RTL8370_LEDFORCERATE{

    LEDFORCERATE_512MS=0,
    LEDFORCERATE_1024MS,
    LEDFORCERATE_2048MS,
    LEDFORCERATE_NORMAL,
    LEDFORCERATE_MAX,

};

extern ret_t rtl8370_setAsicLedOperationMode(uint32 mode);
extern ret_t rtl8370_getAsicLedOperationMode(uint32 *mode);
extern ret_t rtl8370_setAsicLedIndicateInfoConfig(uint32 ledno, enum RTL8370_LEDCONF config);
extern ret_t rtl8370_getAsicLedIndicateInfoConfig(uint32 ledno, enum RTL8370_LEDCONF* config);
extern ret_t rtl8370_setAsicForceLed(uint32 port, uint32 group, uint32 mode);
extern ret_t rtl8370_getAsicForceLed(uint32 port, uint32 group, uint32* mode);
extern ret_t rtl8370_setAsicForceGroupLed(uint32 groupmask, uint32 mode);
extern ret_t rtl8370_getAsicForceGroupLed(uint32* groupmask, uint32* mode);
extern ret_t rtl8370_setAsicLedBlinkRate(enum RTL8370_LEDBLINKRATE blinkRate);
extern ret_t rtl8370_getAsicLedBlinkRate(enum RTL8370_LEDBLINKRATE* blinkRate);
extern ret_t rtl8370_setAsicLedForceBlinkRate(enum RTL8370_LEDFORCERATE blinkRate);
extern ret_t rtl8370_getAsicLedForceBlinkRate(enum RTL8370_LEDFORCERATE* blinkRate);
extern ret_t rtl8370_setAsicLedGroupMode(uint32 mode);
extern ret_t rtl8370_getAsicLedGroupMode(uint32* mode);
extern ret_t rtl8370_setAsicLedSerialModeConfig(uint32 active, uint32 serimode);
extern ret_t rtl8370_getAsicLedSerialModeConfig(uint32 *active, uint32 *serimode);
extern ret_t rtl8370_setAsicLedSystemEnable(uint32 enable);
extern ret_t rtl8370_getAsicLedSystemEnable(uint32 *enable);
extern ret_t rtl8370_setAsicLedGroupEnable(uint32 group, uint32 portmask);
extern ret_t rtl8370_getAsicLedGroupEnable(uint32 group, uint32 *portmask);
extern ret_t rtl8370_setAsicLedSerialEnable(uint32 enable);
extern ret_t rtl8370_getAsicLedSerialEnable(uint32 *enable);
#endif /*#ifndef _RTL8370_ASICDRV_LED_H_*/

