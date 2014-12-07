/******************************************************************************
 *  This program is free software; you can redistribute  it and/or modify it                                                                                           *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your                                                                                           *  option) any later version.
 *                                                                                                                                                                     *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF                                                                                           *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,                                                                                            *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF                                                                                           *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT                                                                                           *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                                                                  *
 *  You should have received a copy of the  GNU General Public License along                                                                                           *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.                                                                                                                            *
 */

/*******************************************************************************
*
*  File Name: mem_map_1fc0.h
*     Author: Ian Thompson 
*
*    Purpose: Contains the entire memory map of the PalmPak chip.
*
*  Sp. Notes: This memory map assumes the processor boots to code at location
*             0x1fc0.0000
*
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    01/15/01  IST   Created from original PalmPak 2.0 memory map
*    02/09/01  IST   Pulled out PalmPak chip regs into chip_reg_map.h
*
*
*******************************************************************************/

#ifndef MEM_MAP_1FC0_H
#define MEM_MAP_1FC0_H

/*=====================*
 *  Include Files      *
 *=====================*/

/*=====================*
 *  Defines            *
 *=====================*/


/*
**-------------------------------------------------------------------------- 
** High-level memory map definitions.
** NOTE: If you change the memory addresses, then change softpak.mak too.
**-------------------------------------------------------------------------- 
*/

/* ROM */
#define MAC_ROM_BASE	(0x1fc00000)
#define MAC_ROM_SIZE	(0x00200000)		/* 2MB reserved */
#define MAC_ROM_END	(MAC_ROM_BASE + MAC_ROM_SIZE)

#define ROM_BASE	(MAC_ROM_BASE)
#define ROM_SIZE	(0x00200000)		/* 2MB actual */
#define ROM_END		(ROM_BASE + ROM_SIZE)

/* When the REMAP is set in the system control block, 
** 1st ISRAM-sized section of ROM is remapped to boot location of ISRAM.
** The following defines refer to the section of ROM which remains un-remapped.
*/
#define ROM_REMAPPED_BASE	(ISRAM_REMAPPED_END)
#define ROM_REMAPPED_SIZE       (ROM_SIZE - ISRAM_SIZE)
#define ROM_REMAPPED_END	(ROM_END)

/* May need a better way to (not) set this for testing hardware! */
#define ROM_BANK_IS_FLASH


/* ISRAM */
#define ISRAM_BOOT_BASE	(0x00200000)	        /* Internal SRAM at boot */
#define ISRAM_SIZE	(0x00002000)		/* 8KB actual */
						/* 16KB contiguous if ISRAM and 
						 * VECTMEM are both not
						 * remapped.
						 */ 
#define ISRAM_BOOT_END	(ISRAM_BOOT_BASE + ISRAM_SIZE)

/* When the REMAP is set in the system control block, 
** Int SRAM is remapped to the boot location of ROM
*/
#define ISRAM_REMAPPED_BASE	(ROM_BASE)
#define ISRAM_REMAPPED_END	(ISRAM_REMAPPED_BASE + ISRAM_SIZE)


/* VECTOR SRAM */
#define VECTMEM_BOOT_BASE	(0x00202000)	/* Vector SRAM at boot */
#define VECTMEM_SIZE		(0x00002000)	/* 8KB */
#define VECTMEM_BOOT_END	(VECTMEM_BOOT_BASE + VECTMEM_SIZE)

/* When the VECTOR MEMORY REMAP is set in the system control block, 
** Vector SRAM is remapped to the location of BEV=0 vectors
*/
#define VECTMEM_REMAPPED_BASE	(0x00000000)
#define VECTMEM_REMAPPED_END	(VECTMEM_REMAPPED_BASE + VECTMEM_SIZE)



/* PALMPAK */
#define PALMPAK_BASE	(0x00300000)		/* PalmPak ASIC regs */
#define PALMPAK_SIZE    (0x00010000)		/* 64KB */
#define PALMPAK_END     (PALMPAK_BASE + PALMPAK_SIZE)


/* Main SRAM */
/* Note: The palmpak memory map reserves 2M of the address space for SRAM,
   and this is reflected in the memory controller configuration, but the
   development board contains 512K. */
#define MAC_SRAM_BASE   (0x00000000)
#define MAC_SRAM_SIZE	(0x00200000)		/* 2MB reserved */
#define MAC_SRAM_END	(MAC_SRAM_BASE + MAC_SRAM_SIZE)

#define SRAM_BASE	MAC_SRAM_BASE
#ifdef DEV_BOARD_SRAM_SIZE
/* WARNING: Must be careful not to use SRAM_SIZE in HAL code,
   as that would cause the HAL to be configuration-specific
   due to the configuration-specific use of this #ifdef! */
#define SRAM_SIZE	DEV_BOARD_SRAM_SIZE
#else
#define SRAM_SIZE	(0x00200000)		/* 2MB actual */
#endif
#define SRAM_END	(SRAM_BASE + SRAM_SIZE)


/* SDRAM */
#ifdef USE_SDRAM
#define MAC_SDRAM_BASE  (0x08000000)
#define MAC_SDRAM_SIZE  (0x08000000)            /* 16MB reserved */
#define MAC_SDRAM_END   (MAC_SDRAM_BASE + MAC_SDRAM_SIZE)

#define SDRAM_BASE      (MAC_SDRAM_BASE)
#define SDRAM_SIZE      (MAC_SDRAM_SIZE)        /* 16MB actual */
#define SDRAM_END       (SDRAM_BASE + SDRAM_SIZE)
#endif


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* MEM_MAP_1FC0_H */


/****************************************************************
 *
 * PalmPak 2.0 Chip Address Map
 * ----------------------------
 *
 * Normal mapping:
 * ---------------
 * 0000_0000 - 001F_FFFF : 2M SRAM
 * 0020_0000 - 0020_1FFF : 8K Embedded SRAM (ISRAM)
 * 0020_2000 - 002F_FFFF : Reserved
 * 0030_0000 - 0030_FFFF : 64K Chip Registers**
 * 0031_0000 - 00FF_FFFF : Reserved
 * 0800_0000 - 09FF_FFFF : 32M SDRAM
 * 0A00_0000 - 0BFF_FFFF : Reserved
 * 0C00_0000 - 0DFF_FFFF : 32M SDRAM
 * 0E00_0000 - 1FBF_FFFF : Reserved
 * 1FC0_0000 - 1FDF_FFFF : 2M ROM/Flash
 * 1FE0_0000 - FFFF_FFFF : Reserved
 *
 * With remap set
 * --------------
 * 0000_0000 - 001F_FFFF : 2M SRAM
 * 0020_0000 - 0020_1FFF : First 8K ROM/Flash
 * 0020_2000 - 002F_FFFF : Reserved
 * 0030_0000 - 0030_FFFF : 64K Chip Registers**
 * 0031_0000 - 00FF_FFFF : Reserved
 * 0800_0000 - 09FF_FFFF : 32M SDRAM
 * 0A00_0000 - 0BFF_FFFF : Reserved
 * 0C00_0000 - 0DFF_FFFF : 32M SDRAM
 * 0E00_0000 - 1FBF_FFFF : Reserved
 * 1FC0_0000 - 1FC0_1FFF : 8K Embedded SRAM (ISRAM)
 * 1FC0_2000 - 1FDF_FFFF : Remaining (2M minus 8K) ROM/Flash
 * 1FE0_0000 - FFFF_FFFF : Reserved
 *
 *
 * ** The Internal ASIC register space is broken up as follows:
 *    256 blocks x 256 regs/block x 4 bytes/reg = 64KB
 *
 ****************************************************************/
