#ifndef _RTL8367B_ASICDRV_GPIO_H_
#define _RTL8367B_ASICDRV_GPIO_H_

#include "rtl8367b_asicdrv.h"

extern ret_t rtl8367b_getAsicGpioInput(rtk_uint32 gpioPin, rtk_uint32* pGpioInput);
extern ret_t rtl8367b_setAsicGpioOutput(rtk_uint32 gpioPin, rtk_uint32 gpioOutput);
extern ret_t rtl8367b_getAsicGpioOutput(rtk_uint32 gpioPin, rtk_uint32* pGpioOutput);
extern ret_t rtl8367b_setAsicGpioSelect(rtk_uint32 gpioPin, rtk_uint32 gpioSelect);
extern ret_t rtl8367b_getAsicGpioSelect(rtk_uint32 gpioPin, rtk_uint32* pGpioSelect);
extern ret_t rtl8367b_setAsicGpioEn(rtk_uint32 gpioPin, rtk_uint32 gpioEn);
extern ret_t rtl8367b_getAsicGpioEn(rtk_uint32 gpioPin, rtk_uint32* pGpioEn);
extern ret_t rtl8367b_setAsicGpioAclEnClear(rtk_uint32 gpioPin);
extern ret_t rtl8367b_getAsicGpioAclEnClear(rtk_uint32 gpioPin, rtk_uint32* pGpioEn);


#endif /*#ifndef _RTL8367B_ASICDRV_GPIO_H_*/

