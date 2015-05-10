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
*  File Name: mem_map.h
*     Author: Ian Thompson 
*
*    Purpose: 
*       This file contains the appropriate memory map as defined in 
*       the makefile.
*
*  Sp. Notes:
*
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    01/15/01  IST   Created.
*
*
*******************************************************************************/
#include <rt_mmap.h>

#ifndef MEM_MAP_H
#define MEM_MAP_H

/*=====================*
 *  Include Files      *
 *=====================*/

#include "mem_map_1fc0.h"
#include "chip_reg_map.h"


/*=====================*
 *  Defines            *
 *=====================*/


#ifdef USE_CACHE
  // Overwrite some defines to specify virtual addresses instead of physical.
  // Also define uncached locations (*_NC).

  // Re-define MAC_ROM_BASE to KSEG0(MAC_ROM_BASE)
  #undef	MAC_ROM_BASE
  #define	MAC_ROM_BASE		(0x9fc00000)
  #define	MAC_ROM_BASE_NC		(0xbfc00000)

  // Re-define ROM_REMAPPED_BASE to KSEG0(ROM_REMAPPED_BASE)
  #undef	ROM_REMAPPED_BASE
  #define	ROM_REMAPPED_BASE	(0x9fc02000)
  #define	ROM_REMAPPED_BASE_NC	(0xbfc02000)

  // Re-define ISRAM_BOOT_BASE to KSEG1(ISRAM_BOOT_BASE)
  #undef	ISRAM_BOOT_BASE
  #define	ISRAM_BOOT_BASE		(0x80200000)
  #define	ISRAM_BOOT_BASE_NC	(0xa0200000)

  // Re-define ISRAM_REMAPPED_BASE to KSEG1(ISRAM_REMAPPED_BASE)
  #undef	ISRAM_REMAPPED_BASE
  #define	ISRAM_REMAPPED_BASE	(0x9fc00000)
  #define	ISRAM_REMAPPED_BASE_NC	(0xbfc00000)

  // Re-define PALMPAK_BASE to KSEG1(PALMPAK_BASE)
  #undef	PALMPAK_BASE
  #define	PALMPAK_BASE		(RALINK_SYSCTL_BASE)

  // Re-define MAC_SRAM_BASE to KSEG0(MAC_SRAM_BASE)
  #undef	MAC_SRAM_BASE
  #define	MAC_SRAM_BASE		(0x80000000)
  #define	MAC_SRAM_BASE_NC	(0xa0000000)

  // Re-define MAC_SDRAM_BASE to KSEG0(MAC_SDRAM_BASE)
  #undef	MAC_SDRAM_BASE
  #define	MAC_SDRAM_BASE		(0x88000000)
  #define	MAC_SDRAM_BASE_NC	(0xa8000000)

#endif /* USE_CACHE */


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/



#endif	// MEM_MAP_H
