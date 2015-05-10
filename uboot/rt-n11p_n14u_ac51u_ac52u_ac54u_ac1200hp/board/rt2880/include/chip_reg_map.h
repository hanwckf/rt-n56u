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
*  File Name: chip_reg_map.h
*     Author: Ian Thompson 
*
*    Purpose: Contains the chip register map of the PalmPak system.
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
*    02/09/01  IST   Split from mem_map.h
*
*
*******************************************************************************/

#ifndef CHIP_REG_MAP_H
#define CHIP_REG_MAP_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "aux_reg_map.h"


/*=====================*
 *  Defines            *
 *=====================*/


/*
**-------------------------------------------------------------------------- 
** Palmpak-related register definitions
**-------------------------------------------------------------------------- 
*/
#define SYSC		0x0000			/* PALMPAK */
#define TMR		0x0001			/* Timers */
#define INTC		0x0002			/* Interrupt Controller */
#define MAC		0x0003			/* Memory Access Controller */
#define RSV1		0x0004			/* Reserved */
#define UART		0x0005			/* UARTs */
#define PIO		0x0006			/* Programmable IO */
#define DMA		0x0007			/* Direct Memory Access */
#define LCD		0x0008			/* LCD */
#define I2C		0x0009                  /* I2C */
#define RTC		0x000A                  /* Real Time Clock */
#define SPI		0x000B                  /* SPI */
#define UARTLITE	0x000C                  /* UART lite */
#define RSV2		0x000D			/* Unused */
#define RSV3		0x000E			/* Unused */
#define RSV4		0x000F			/* Unused */
#define AUX0		0X0010			/* Auxiliary Block 0 */
#define AUX1		0X0014			/* Auxiliary Block 1 */
#define AUX2		0X0018			/* Auxiliary Block 2 */
#define AUX3		0x001C			/* Auxiliary Block 3 */
#define	AUX4		0x0020			/* Auxiliary Block 4 */
#define	AUX5		0x0024			/* Auxiliary Block 5 */
#define	AUX6		0x0028			/* Auxiliary Block 6 */
#define	AUX7		0x002C			/* Auxiliary Block 7 */
#define	AUX8		0x0030			/* Auxiliary Block 8 */
#define	AUX9		0x0034			/* Auxiliary Block 9 */
#define	AUX10		0x0038			/* Auxiliary Block 10 */

#define IRAM		0x000D			/* For Error Reporting Only */
#define	ERAM		0x000E			/* For Error Reporting Only */
#define EROM		0x000F			/* For Error Reporting Only */
#define	SRAM		ERAM			/* For Error Reporting Only */
#define	FROM		EROM			/* For Error Reporting Only */

#define SYSC_BASE	(PALMPAK_BASE + (SYSC << 8))	  /* PALMPAK */
#define TMR_BASE	(PALMPAK_BASE + (TMR << 8))	  /* Timers */
#define INTC_BASE	(PALMPAK_BASE + (INTC << 8))	  /* Interrupt Controller */
#define MAC_BASE	(PALMPAK_BASE + (MAC << 8))	  /* Memory Access Controller */
#define RSV1_BASE	(PALMPAK_BASE + (RSV1 << 8))	  /* Reserved */
#define UART_BASE	(PALMPAK_BASE + (UART << 8))	  /* UART/s */
#define PIO_BASE	(PALMPAK_BASE + (PIO << 8))	  /* Programmable IO */
#define DMA_BASE	(PALMPAK_BASE + (DMA << 8))	  /* Direct Memory Access */
#define LCD_BASE	(PALMPAK_BASE + (LCD << 8))	  /* LCD */
#define I2C_BASE	(PALMPAK_BASE + (I2C << 8))	  /* I2C */
#define RTC_BASE	(PALMPAK_BASE + (RTC << 8))	  /* Real Time Clock */
#define SPI_BASE	(PALMPAK_BASE + (SPI << 8))	  /* SPI */
#define UARTLITE_BASE	(PALMPAK_BASE + (UARTLITE << 8))  /* UART Lite */
#define RSV2_BASE	(PALMPAK_BASE + (RSV2 << 8))	  /* Reserved */
#define RSV3_BASE	(PALMPAK_BASE + (RSV3 << 8))	  /* Reserved */
#define RSV4_BASE	(PALMPAK_BASE + (RSV4 << 8))	  /* Reserved */
#define AUX0_BASE	(PALMPAK_BASE + (AUX0 << 8)) 	  /* Auxiliary Block 0 */
#define AUX1_BASE	(PALMPAK_BASE + (AUX1 << 8)) 	  /* Auxiliary Block 1 */
#define AUX2_BASE	(PALMPAK_BASE + (AUX2 << 8)) 	  /* Auxiliary Block 2 */
#define AUX3_BASE	(PALMPAK_BASE + (AUX3 << 8)) 	  /* Auxiliary Block 3 */
#define AUX4_BASE	(PALMPAK_BASE + (AUX4 << 8)) 	  /* Auxiliary Block 4 */
#define AUX5_BASE	(PALMPAK_BASE + (AUX5 << 8)) 	  /* Auxiliary Block 5 */
#define AUX6_BASE	(PALMPAK_BASE + (AUX6 << 8)) 	  /* Auxiliary Block 6 */
#define AUX7_BASE	(PALMPAK_BASE + (AUX7 << 8)) 	  /* Auxiliary Block 7 */
#define AUX8_BASE	(PALMPAK_BASE + (AUX8 << 8)) 	  /* Auxiliary Block 8 */
#define AUX9_BASE	(PALMPAK_BASE + (AUX9 << 8)) 	  /* Auxiliary Block 9 */
#define AUX10_BASE	(PALMPAK_BASE + (AUX10 << 8)) 	  /* Auxiliary Block 10 */

#define UART0_BASE	UART_BASE

/* PLL and DLL registers are part of SYSC (system control) registers */
#define PLL		SYSC
#define PLL_BASE	(SYSC_BASE + 0x40)


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* CHIP_REG_MAP_H */

