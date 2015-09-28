#include <common.h>
#include <rt_mmap.h>

#define DRAMC_BASE 0xBE005000

#define MPLL_IN_LBK	1
//#define MEMPLL_CLK_600	1
//#define MEMPLL_CLK_400	1
#define MEMPLL_CLK_200	1

#define udelay_a(count) \
        do {    \
           register unsigned int delay;        \
           asm volatile (	\	
           				 "move %0, %1\n\t"      \
                         "1:\n\t"              \
                         "subu %0, %0, 1\n\t" \
                         "bgtz %0, 1b\n\t"          \
                         "nop\n\t"	\
                         : "+r" (delay)        \
                         : "r" (count)         \
                         : "cc"); \
        } while (0) 

int ddr_initialize(void)
{
	int oneusec = 25;	/* 1/((1/50)*2) */

#if defined (FPGA_BOARD)
	RALINK_REG(0xbe005110)=0x00051100;
	RALINK_REG(0xbe00507c)=0xb18731b5;
	RALINK_REG(0xbe005048)=0x0000110d;
	RALINK_REG(0xbe0050d8)=0x00100900;
	RALINK_REG(0xbe0050e4)=0x000000a3;
	RALINK_REG(0xbe00508c)=0x00000001;
	RALINK_REG(0xbe005090)=0x00000000;
	RALINK_REG(0xbe005094)=0x80000000;
	RALINK_REG(0xbe0050dc)=0x83040040;
	RALINK_REG(0xbe0050e0)=0x10040040;
	RALINK_REG(0xbe0050f0)=0x00000000;
	RALINK_REG(0xbe0050f4)=0x01000000;
	RALINK_REG(0xbe005168)=0x00000080;
	RALINK_REG(0xbe005130)=0x30000000;
	RALINK_REG(0xbe0050d8)=0x00300900;
	RALINK_REG(0xbe005004)=0xf0748663;
	RALINK_REG(0xbe005124)=0x80000011;
	RALINK_REG(0xbe005094)=0x40404040;
	RALINK_REG(0xbe0051c0)=0x8000c8b8;
	RALINK_REG(0xbe00507c)=0xb18711b5;
	RALINK_REG(0xbe005028)=0xf1200f01;
	RALINK_REG(0xbe0051e0)=0xa8000000;
	RALINK_REG(0xbe005158)=0x00000000;
	RALINK_REG(0xbe005004)=0xf07402e2;
	RALINK_REG(0xbe0050e4)=0x000000a7;

	udelay_a(oneusec);
	
	RALINK_REG(0xbe005088)=0x00004008;
	RALINK_REG(0xbe0051e4)=0x00000001;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe005088)=0x00006000;
	RALINK_REG(0xbe0051e4)=0x00000001;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe005088)=0x00002000;
	RALINK_REG(0xbe0051e4)=0x00000001;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe005088)=0x00000121;
	RALINK_REG(0xbe0051e4)=0x00000001;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe005088)=0x00000400;
	RALINK_REG(0xbe0051e4)=0x00000010;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe0051e4)=0x00001100;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000004;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe0051e4)=0x00000008;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe005088)=0x00002001;
	RALINK_REG(0xbe0051e4)=0x00000001;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe0051e4)=0x00000004;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe0051e4)=0x00000008;

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe0050e4)=0x000007a3;   
	RALINK_REG(0xbe0051e0)=0x88000000;
	RALINK_REG(0xbe005088)=0x0000ffff;
	RALINK_REG(0xbe0051e4)=0x00000020; 

	udelay_a(oneusec);

	RALINK_REG(0xbe0051e4)=0x00000000;
	RALINK_REG(0xbe0051dc)=0x106b2842;
	RALINK_REG(0xbe005004)=0xf0748653;     

	RALINK_REG(0xbe00500c)=0x00000000;
	RALINK_REG(0xbe005000)=0x00054411;
	RALINK_REG(0xbe005044)=0xa8800401; 
	RALINK_REG(0xbe0051e8)=0x00000600;
	RALINK_REG(0xbe005008)=0x00047905;
	RALINK_REG(0xbe005010)=0x00000000;
	RALINK_REG(0xbe0050f8)=0xedcb000f;
	RALINK_REG(0xbe0050fc)=0x27010000;
	RALINK_REG(0xbe0051d8)=0x00c80008;
#else
	
	if ((RALINK_REG(0xBE000010)>>4)&0x1)
	{
#include "ddr2.h"
	}
	else
	{
#include "ddr3.h"		
	}		
#endif	
}

int mempll_init()
{
	int oneusec = 25;
#include "mpll40Mhz.h"
	return 0;
}	
