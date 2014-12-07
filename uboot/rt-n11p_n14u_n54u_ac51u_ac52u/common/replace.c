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
#include <command.h>
#include <asm/errno.h>
#include <malloc.h>
#include <linux/ctype.h>
#include "replace.h"

int replace(unsigned long addr, uchar *value, int len)
{
	if (addr >= CFG_FACTORY_SIZE || !value || len <= 0 || (addr + len) >= CFG_FACTORY_SIZE)
		return -1;

	return ra_factory_erase_write(value, addr, len, 0);
}


#ifdef DEBUG_FACTORY_RW
int do_replace(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i, r, parse_method = 1;		/* 0: hex; otherwise: string (default) */
	size_t len, len2;
	unsigned long addr;
	unsigned char buf[256], *p = &buf[0], h[3], *d = NULL;

	if (argc < 3)
		return EINVAL;

	addr = simple_strtoul(argv[1], NULL, 16);
	switch (argc) {
		case 4:
			d = argv[3];
			if (!strnicmp(argv[2], "hex", 3))
				parse_method = 0;
			break;
		case 3:
			d = argv[2];
			break;
	}
	len = strlen(d);
	if (!parse_method && (len & 1)) {
		/* in hexadecimal mode, only even digits is accepts. */
		printf("Length %d is not even\n", len);
		return EINVAL;
	}

	if (len >= sizeof(buf)) {
		p = malloc(len + 1);
		if (!p)
			return ENOMEM;
	}

	if (parse_method)	{
		/* data format is string */
		strcpy(p, d);
	} else {
		/* data format is hexadecimal digit */
		len2 = len;
		i = 0;
		while (len2 > 0) {
			if (!isxdigit(*d) || !isxdigit(*(d+1))) {
				printf("Invalid hexadecimal digit found.\n");
				goto out_replace;
			}

			h[0] = *d++;
			h[1] = *d++;
			h[2] = '\0';
			len2 -= 2;
			p[i++] = (unsigned char) simple_strtoul(&h[0], NULL, 16);
		}
		len = i;
	}

	r = ra_factory_erase_write(p, addr, len, 0);
	if (r)
		printf("%s: buf %p len %x fail. (r = %d)\n", __func__, p, len, r);

out_replace:
	if (p != &buf[0])
		free(p);

	return 0;
}

U_BOOT_CMD(
	replace,	4,	1,	do_replace,
	"replace	- modify factory area\n",
	"factory_offset [hex|[string]] data	- modify factory area\n"
);
#endif	/* DEBUG_FACTORY_RW */


extern const char *model, *blver, *bl_stage;

int chkVer(void)
{
	uchar rfbuf[4];
	ulong addr = 0x18a;

	memset(rfbuf, 0x0, 4);
	ra_factory_read(rfbuf, addr, 4);
        printf("\n%s bootloader%s version: %c.%c.%c.%c\n", model, bl_stage, blver[0], blver[1], blver[2], blver[3]);

	if((rfbuf[0] == blver[0]) && (rfbuf[1] == blver[1]) && (rfbuf[2] == blver[2]) && (rfbuf[3] == blver[3]))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int chkMAC(void)
{	
	uchar rfbuf[0x06];
#if defined(ASUS_RTN14U)// LAN & 2.4G MAC
	ulong addr = 0x4; // Change to first block
#else
	ulong addr = 0x4;
#endif

	memset(rfbuf, 0, 0x06);
	ra_factory_read(rfbuf, addr, 6);

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
