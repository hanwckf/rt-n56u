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
*  File Name: aux_intc.h
*     Author: Linda Yang
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    03/25/02  LYT   Created.
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************/
//
// Purpose:
//    The file contains all the interrupt controller definitions for auxilliary
//    blocks.
//
// Sp. Notes:
//    The contents of this file are automatically modified by the script
//    release_setup.
//

#ifndef AUX_INTC_H
#define AUX_INTC_H

/*=====================*
 *  Include Files      *
 *=====================*/


/*=====================*
 *  Defines            *
 *=====================*/
 
#define INTC_DESHW_DMA_INT			INTC_DMA_AUX0_INT              
#define deshwDmaInt			dmaAux0Int

#define INTC_PCMCIA_DMA_INT			INTC_DMA_AUX1_INT              
#define pcmciaDmaInt			dmaAux1Int



#define INTC_DESHW_INT			INTC_AUX0_INT              
#define deshwInt				aux0Int

#define INTC_PCMCIA_INT			INTC_AUX1_INT              
#define pcmciaInt				aux1Int

#define INTC_IDE3710_INT			INTC_AUX2_INT              
#define ide3710Int				aux2Int

#define INTC_ENET_INT			INTC_AUX3_INT              
#define enetInt				aux3Int

#define INTC_ENET1_INT			INTC_AUX4_INT              
#define enet1Int				aux4Int

#define INTC_IDE37101_INT			INTC_AUX5_INT              
#define ide37101Int				aux5Int

#define INTC_ATA_INT			INTC_AUX6_INT              
#define ataInt				aux6Int




#endif /* AUX_INTC_H */
