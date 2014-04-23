#ifndef __FLASH_MTD_H__
#define __FLASH_MTD_H__

extern int FRead(unsigned char *dst, int src, int count);
extern int FWrite(unsigned char *src, int dst, int count);

#endif
