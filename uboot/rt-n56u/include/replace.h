#ifndef REPLACE_H
#define REPLACE_H

#include <common.h>

//extern unsigned char btype;

int replace(unsigned long addr, uchar *value, int len);
int chkMAC(void);
int chkVer(void);
#endif
