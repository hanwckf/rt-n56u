/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <version.h>
#include <asm/addrspace.h>
//#include "LzmaDecode.h"

#define MAX_SDRAM_SIZE	(256*1024*1024)
#define MIN_SDRAM_SIZE	(8*1024*1024)

#ifdef SDRAM_CFG_USE_16BIT
#define MIN_RT2880_SDRAM_SIZE	(16*1024*1024)
#else
#define MIN_RT2880_SDRAM_SIZE	(32*1024*1024)
#endif

extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
long get_ram_size(volatile long *base, long maxsize)
{
	volatile long *addr;
	long           save[32];
	long           cnt;
	long           val;
	long           size;
	int            i = 0;

	for (cnt = (maxsize / sizeof (long)) >> 1; cnt > 0; cnt >>= 1) {
		addr = base + cnt;	/* pointer arith! */
		save[i++] = *addr;
		
		*addr = ~cnt;
	}

	addr = base;
	save[i] = *addr;

	*addr = 0;

	if ((val = *addr) != 0) {
		/* Restore the original data before leaving the function.
		 */
		*addr = save[i];
		for (cnt = 1; cnt < maxsize / sizeof(long); cnt <<= 1) {
			addr  = base + cnt;
			*addr = save[--i];
		}
		return (0);
	}

	for (cnt = 1; cnt < maxsize / sizeof (long); cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */

	//	printf("\n retrieve addr=%08X \n",addr);
			val = *addr;
		*addr = save[--i];
		if (val != ~cnt) {
			size = cnt * sizeof (long);
			
		//	printf("\n The Addr[%08X],do back ring  \n",addr);
			
			/* Restore the original data before leaving the function.
			 */
			for (cnt <<= 1; cnt < maxsize / sizeof (long); cnt <<= 1) {
				addr  = base + cnt;
				*addr = save[--i];
			}
			return (size);
		}
	}

	return (maxsize);
}



long int initdram(int board_type)
{
	ulong size;
	ulong our_address;
#ifndef CONFIG_MIPS16
	asm volatile ("move %0, $25" : "=r" (our_address) :);
#endif

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
	if (PHYSADDR(our_address) < PHYSADDR(PHYS_FLASH_1))
	{
		//fixed to 32MB
		printf("\n In RAM run \n");
		return MIN_SDRAM_SIZE;
	}
#endif

	size = get_ram_size((ulong *)CFG_SDRAM_BASE, MAX_SDRAM_SIZE);
	if (size <= MIN_SDRAM_SIZE)
	{
		printf("RAM size (0x%08x) too small !!! do_reset\n", size);
		udelay(100 * 1000);
		do_reset (NULL, 0, 0, NULL);
	}

#if defined (ON_BOARD_4096M_DRAM_COMPONENT) 
	/* Do not use highmem in U-Boot! */
	size = DRAM_SIZE * 0x100000;
#endif

	return size;
}

int checkboard (void)
{
	printf("Board: %s APSoC ", RLT_MTK_VENDOR_NAME);
	return 0;
}

