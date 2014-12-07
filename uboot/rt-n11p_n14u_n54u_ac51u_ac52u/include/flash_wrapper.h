// vim:cin
/* 
 * Copyright 2013, ASUSTeK Inc.
 * All Rights Reserved.
 */

#ifndef _FLASH_WRAPPER_H_
#define _FLASH_WRAPPER_H_

extern int ra_flash_init_layout(void);
extern char *ra_flash_id(void);

#if defined(UBI_SUPPORT)
extern int choose_active_eeprom_set(void);
extern int __SolveUBI(unsigned char *ptr, unsigned int offset, unsigned int copysize);
#else
static inline int choose_active_eeprom_set(void) { return 0; }
#endif

/* Below function use absolute address, include CFG_FLASH_BASE. */
extern int ra_flash_read(uchar * buf, ulong addr, ulong len);
extern int ra_flash_erase_write(uchar * buf, ulong addr, ulong len, int prot);
extern int ra_flash_erase(ulong addr, ulong len);

/* Below function use relative address, respect to start address of factory area. */
extern int ra_factory_read(uchar *buf, ulong off, ulong len);
extern int ra_factory_erase_write(uchar *buf, ulong off, ulong len, int prot);
#endif	/* _FLASH_WRAPPER_H_ */
