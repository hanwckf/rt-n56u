/**********************************************************
*	File:replace.c
*	This file includes a function that is used to replace the 
*	values in RF buffer.
*
*	return rc 	=0: replace successful
*						 !=0:	fail	
							
*
**********************************************************/

#include <common.h>
#include "replace.h"

int replace(unsigned long addr, uchar *value, int len)
{
#if defined (CFG_ENV_IS_IN_NAND)
	if(addr + len < CFG_FACTORY_SIZE)
	{
		addr += (CFG_FACTORY_ADDR - CFG_FLASH_BASE);
		return ranand_erase_write((u8 *)value, addr, len);
	}
	return -1;
#elif defined (CFG_ENV_IS_IN_SPI)
	if(addr + len < CFG_FACTORY_SIZE)
	{
		addr += (CFG_FACTORY_ADDR - CFG_FLASH_BASE);
		return raspi_erase_write((u8 *)value, addr, len);
	}
	return -1;
#else
	int idx = 0;
	uchar RFbuf[CFG_FACTORY_SIZE];
	int rc=0;
	memset(RFbuf,0, sizeof(RFbuf));
	if (0==len)
	{
		printf("\nThe value to replace is vacant!");
		return -1;
	}

	for (idx =0 ;idx < sizeof(RFbuf);idx ++)
		RFbuf[idx]=(*(uint8_t *) (CFG_FACTORY_ADDR + idx));

	for (idx =0 ;idx <len; idx++)
		RFbuf[0x00000 + addr + idx] = value[idx];
	
	flash_sect_protect(0, CFG_FACTORY_ADDR, CFG_FACTORY_ADDR + CFG_FACTORY_SIZE - 1);
	
	printf("\n Erase File System block !!\n From %x To %x\n", CFG_FACTORY_ADDR, CFG_FACTORY_ADDR + CFG_FACTORY_SIZE - 1);
	flash_sect_erase(CFG_FACTORY_ADDR, CFG_FACTORY_ADDR + CFG_FACTORY_SIZE - 1);
	rc = flash_write((uchar *)RFbuf, (ulong)(CFG_FACTORY_ADDR), CFG_FACTORY_SIZE);

	flash_sect_protect(1, CFG_FACTORY_ADDR, CFG_FACTORY_ADDR + CFG_FACTORY_SIZE - 1);

	printf("\nrc=%d\n",rc);
 	return rc;
#endif
}

uchar blver[] = "1008";

int chkVer(void)
{
	int idx;
	uchar rfbuf[4];
	ulong addr = CFG_FACTORY_ADDR + 0x18a;

	memset(rfbuf, 0x0, 4);
#if defined (CFG_ENV_IS_IN_NAND)
	ranand_read((char *)&rfbuf, addr - CFG_FLASH_BASE, 4);
#elif defined (CFG_ENV_IS_IN_SPI)
	raspi_read((char *)&rfbuf, addr - CFG_FLASH_BASE, 4);
#else //CFG_ENV_IS_IN_FLASH
	for(idx = 0; idx < 4; ++idx)
		rfbuf[idx] = (*(uint8_t *) (addr + idx));
#endif
	printf("\nBootloader version: %c.%c.%c.%c\n", blver[0], blver[1], blver[2], blver[3]);

	if((rfbuf[0] == blver[0]) && (rfbuf[1] == blver[1]) && (rfbuf[2] == blver[2]) && (rfbuf[3] == blver[3]))
		return 1;

	return 0;
}

int chkMAC(void)
{
	int idx;
	uchar rfbuf[0x06];
	ulong addr = CFG_FACTORY_ADDR + 0x4;

	memset(rfbuf, 0, 0x06);
#if defined (CFG_ENV_IS_IN_NAND)
	ranand_read((char *)&rfbuf, addr - CFG_FLASH_BASE, 6);
#elif defined (CFG_ENV_IS_IN_SPI)
	raspi_read((char *)&rfbuf, addr - CFG_FLASH_BASE, 6);
#else //CFG_ENV_IS_IN_FLASH
	for(idx = 0; idx < 0x06; ++idx)
		rfbuf[idx] = (*(uint8_t *) (addr + idx));
#endif

	printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
		(unsigned char)rfbuf[0],(unsigned char)rfbuf[1],(unsigned char)rfbuf[2],
		(unsigned char)rfbuf[3],(unsigned char)rfbuf[4],(unsigned char)rfbuf[5]);

	if((rfbuf[0] == 0xff) && (rfbuf[1] == 0xff) && (rfbuf[2] == 0xff) && (rfbuf[3] == 0xff) && (rfbuf[4] == 0xff) && (rfbuf[5] == 0xff))
	{
		printf("\ninvalid mac ff:ff:ff:ff:ff:ff\n");
		return -2;
	}

	if(rfbuf[0] & 0x01)
	{
		printf("\nerr mac with head 01\n");
		return -1;
	}

	return 0;
}
