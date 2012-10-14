#ifndef _RTL8370_ASICDRV_CPUTAG_H_
#define _RTL8370_ASICDRV_CPUTAG_H_

#include "rtl8370_asicdrv.h"

enum CPUTAG_INSERT_MODE
{
    CPUTAG_INSERT_TO_ALL = 0,
    CPUTAG_INSERT_TO_TRAPPING,
    CPUTAG_INSERT_TO_NO,
    CPUTAG_INSERT_MAX
};

extern ret_t rtl8370_setAsicCputagEnable(uint32 enable);
extern ret_t rtl8370_getAsicCputagEnable(uint32 *enable);
extern ret_t rtl8370_setAsicCputagTrapPort(uint32 port);
extern ret_t rtl8370_getAsicCputagTrapPort(uint32 *port);
extern ret_t rtl8370_setAsicCputagPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicCputagPortmask(uint32 *pmsk);
extern ret_t rtl8370_setAsicCputagInsertMode(uint32 mode);
extern ret_t rtl8370_getAsicCputagInsertMode(uint32 *mode);
extern ret_t rtl8370_setAsicCputagPriorityRemapping(uint32 srcPri, uint32 newPri);
extern ret_t rtl8370_getAsicCputagPriorityRemapping(uint32 srcPri, uint32 *newPri);

#endif /*#ifndef _RTL8370_ASICDRV_CPUTAG_H_*/

