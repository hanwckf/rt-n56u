
#include <stdlib.h>             /* malloc, free, etc. */
#include <stdio.h>              /* stdin, stdout, stderr */
#include <string.h>             /* strdup */
#include <sys/ioctl.h>
#include <fcntl.h>

#include "rdm.h"

int ra_reg_write(int offset, int value)
{
	int fd;
	int method = RT_RDM_CMD_WRITE | (offset << 16);

	fd = open("/dev/rdm0", O_RDONLY);
	if (fd < 0)
	{
		printf("Open pseudo device failed\n");
		return -1;
	}

	if(ioctl(fd, method, &value)<0) {
		printf("ioctl error\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int ra_reg_read(int offset)
{
	int fd;

	fd = open("/dev/rdm0", O_RDONLY);
	if (fd < 0)
	{
		printf("Open pseudo device failed\n");
		return -1;
	}

	if(ioctl(fd, RT_RDM_CMD_READ, &offset)<0) {
		printf("ioctl error\n");
		close(fd);
		return -1;
	}

	close(fd);

	return offset;
}

void ra_reg_mod_bits(int offset, int data, int  start_bit, int len)
{
	int Mask=0;
	int Value;
	int i;

	for (i = 0; i < len; i++) {
		Mask |= 1 << (start_bit + i);
	}

	Value = ra_reg_read(offset);
	Value &= ~Mask;
	Value |= (data << start_bit) & Mask;;

	ra_reg_write(offset, Value);
}

//  syntax: reg [r/w] [offset(hex)] [value(hex, w only)] 
//  example reg r 18
//  example reg w 18 12345678
int main(int argc, char *argv[])
{
	int fd, method = 0, offset = 0, value = 0;
	char *p;

	if (argc < 3)
	{
		printf("syntax: reg [method(r/w/s/d/f)] [offset(Hex)] [value(hex, w only)]\n");
		printf("syntax: reg q [interval(Hex in ms)] [count(Hex)]\n");
		printf("syntax: reg o [offset_number] [offset_value]\n");
		printf("read example : reg r 18\n");
		printf("write example : reg w 18 12345678\n");
		printf("dump example : reg d 18 \n");
		printf("dump example [FPGA emulation]: reg f 18 \n");
		printf("modify example : reg m [Offset:Hex] [Data:Hex] [StartBit:Decimal] [DataLen:Decimal] \n");
		printf("To use system register: reg s 0\n");
		printf("To use wireless register: reg s 1\n");
		printf("To use other base address offset: reg s [offset]\n");
		printf("for example: reg s 0xa0500000\n");
		printf("for example: reg m c8 1 31 1\n");
		printf("To show current base address offset: reg s 2\n");
		printf("reg q reads and shows the 16 register values  [count] times in the interval of [interval]ms.\n");
		printf("the addresses of the 16 registers are assigned by reg o\n");
		printf("for example: reg o 5 10. This sets the 5th register offset as 0x10\n");
		printf("[interval]: 1 ~ 1000. [count]: 1 ~ 1000. [offset_number]: 0 ~ 15.\n");
		return 0;
	}

	p = argv[1];
	if (*p == 'r')
	{
		method = RT_RDM_CMD_SHOW;
	}
	else if (*p == 'p')
	{
		method = RT_RDM_CMD_READ;
	}
	else if (*p == 'd')
	{
		method = RT_RDM_CMD_DUMP;
	}
	else if (*p == 'q')
	{
		method = RT_RDM_CMD_DUMP_QUEUE;
	}
	else if (*p == 'f')
	{
		method = RT_RDM_CMD_DUMP_FPGA_EMU;
	}
	else if (*p == 'w')
	{
		method = RT_RDM_CMD_WRITE;
	}
	else if (*p == 's')
	{
		p = argv[2];
		if (*p == '0' && *(p+1) == '\0')
		{
			method = RT_RDM_CMD_SET_BASE_SYS;
		}
		else if (*p == '1')
		{
			method = RT_RDM_CMD_SET_BASE_WLAN;
		}
		else if (*p == '2')
		{
			method = RT_RDM_CMD_SHOW_BASE;
		}
		else
		{
			method = RT_RDM_CMD_SET_BASE;
		}
		
		if (method != RT_RDM_CMD_SET_BASE)
		{
			fd = open("/dev/rdm0", O_RDONLY);
			if (fd < 0)
			{
				printf("Open pseudo device failed\n");
				return 0;
			}
			
			ioctl(fd, method, offset);
			
			close(fd);
			
			return 0;
		}
	}
	else if (*p == 'm')
	{
		int offset=strtoul(argv[2], NULL, 16);
		int data=strtoul(argv[3], NULL, 16);
		int start_bit=strtoul(argv[4], NULL, 10);
		int len=strtoul(argv[5], NULL, 10);
		if (len > 32)
		{
			printf("len must <= 32\n");
			return 0;
		}
		ra_reg_mod_bits(offset,data, start_bit,len);
	}
	else if (*p == 'o')
	{
		method = RT_RDM_CMD_DUMP_QUEUE_OFFSET;
	}
	else
	{
		printf("method must be either r p d f w s m q o\n");
		return 0;
	}

	p = argv[2];
	if (*p == '0' && *(p+1) == 'x')
		p += 2;

	if (strlen(p) > 8)
	{
		printf("invalid offset\n");
	}

	while (*p != '\0')
	{
		if (*p >= '0' && *p <= '9')
			offset = offset * 16 + *p - 48;
		else
		{
			if (*p >= 'A' && *p <= 'F')
				offset = offset * 16 + *p - 55;
			else if (*p >= 'a' && *p <= 'f')
				offset = offset * 16 + *p - 87;
			else
			{
				printf("invalid offset\n");
				return 0;
			}
		}
		p++;
	}

	if (method == RT_RDM_CMD_WRITE || method == RT_RDM_CMD_DUMP_QUEUE  || method == RT_RDM_CMD_DUMP_QUEUE_OFFSET)
	{
		p = argv[3];
		if (*p == '0' && *(p+1) == 'x')
			p += 2;
		if (strlen(p) > 8)
		{
			printf("invalid value\n");
		}
		method = (method | (offset << 16));
		while (*p != '\0')
		{
			if (*p >= '0' && *p <= '9')
				value = value * 16 + *p - 48;
			else 
			{
				if (*p >= 'A' && *p <= 'F')
					value = value * 16 + *p - 55;
				else if (*p >= 'a' && *p <= 'f')
					value = value * 16 + *p - 87;
				else
				{
					printf("invalid value\n");
					return 0;
				}
			}
			p++;
		}
		offset = value;
	}
	if ((method & 0xffff) == RT_RDM_CMD_DUMP_QUEUE && ( ((method >> 16) > 1023 || (method >> 16) == 0) || (offset > 1023 || offset == 0  ) )){
		printf("[interval]: 1 ~ 3ff . [count]: 1 ~ 3ff.\n");
		return 0;
	}
	if (method == 0)
	{
		printf("no method\n");
		return 0;
	}
	fd = open("/dev/rdm0", O_RDONLY);
	if (fd < 0)
	{
		printf("Open pseudo device failed\n");
		return 0;
	}

	ioctl(fd, method, &offset);
	if (method == RT_RDM_CMD_READ) {
		printf("0x%x\n", offset);
	}

	close(fd);

	return 0;
}
