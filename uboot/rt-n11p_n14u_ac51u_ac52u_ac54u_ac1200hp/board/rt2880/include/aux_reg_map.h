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
*  File Name: aux_reg_map.h
*     Author: Linda Yang 
*
*    Purpose: Contains the chip register definitions for auxilliary blocks of
*             the PalmPak system.
*
*  Sp. Notes: The contents of this file are automatically modified by the
*             script release_setup.
*
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    03/25/02  LYT   Created
*
*
*******************************************************************************/

#ifndef AUX_REG_MAP_H
#define AUX_REG_MAP_H

/*=====================*
 *  Include Files      *
 *=====================*/


/*=====================*
 *  Defines            *
 *=====================*/

#define DESHW		AUX0
#define PCMCIA		AUX1
#define IDE3710		AUX2
#define ENET		AUX3
#define ENET1		AUX4
#define IDE37101		AUX5
#define ATA		AUX6


#define DESHW_BASE		AUX0_BASE
#define PCMCIA_BASE		AUX1_BASE
#define IDE3710_BASE		AUX2_BASE
#define ENET_BASE		AUX3_BASE
#define ENET1_BASE		AUX4_BASE
#define IDE37101_BASE		AUX5_BASE
#define ATA_BASE		AUX6_BASE



/* The following are automatically defined for all aux blocks by running the
** software release_setup script.  The presence of the define does not 
** necessarily mean that the block is capable of using DMA.
*/
#define DMA_DESHW_BASE		DMA_AUX0_BASE
#define DMA_PCMCIA_BASE		DMA_AUX1_BASE



/* The following are hard coded and will not be updated by the software
** release_setup script.
*/
#define DMA_ATA_BASE                    (DMA_AUX1_BASE)




#endif /* AUX_REG_MAP_H */

