/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
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
#include <asm/mipsregs.h>
#include <rt_mmap.h>


#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD)
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ra_outl(RALINK_TIMER_BASE + 0x2c, 0x1);		//timer3 load value
	ra_or(RALINK_TIMER_BASE, (1 << 5) | (1 << 25));	//timer3 enabled as watchdog
	return 1;
}
#else

#define SOFTRES_REG (RALINK_SYSCTL_BASE + 0x0034)
#define GORESET		(0x01)

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	*(volatile unsigned int*)(SOFTRES_REG) = GORESET;
	return 1;
}
#endif
void flush_cache (ulong start_addr, ulong size)
{

}
#ifdef RT2880_U_BOOT_CMD_OPEN

void write_one_tlb( int index, u32 pagemask, u32 hi, u32 low0, u32 low1 ){
	write_32bit_cp0_register(CP0_ENTRYLO0, low0);
	write_32bit_cp0_register(CP0_PAGEMASK, pagemask);
	write_32bit_cp0_register(CP0_ENTRYLO1, low1);
	write_32bit_cp0_register(CP0_ENTRYHI, hi);
	write_32bit_cp0_register(CP0_INDEX, index);
	tlb_write_indexed();
}
#endif
