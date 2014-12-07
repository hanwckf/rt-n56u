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
*  File Name: ethernet.h
*     Author: Lee-John Fernandes
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    01/06/01  LRF   Created
*
*
*
*******************************************************************************/

/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains all the definitions for the Ethernet MAC Hardware 
//    Abstraction Layer (HAL). 
//
//
// Sp. Notes:
//    The following must be defined in chip_reg_map.h and intc.h according to 
//    which AUX port the Ethernet DMA and Ethernet MAC blocks are connected:
//
//        ENET                Test Class code (ex. AUX1)
//
//        ENET_BASE           Address base for Ethernet Block (ex. AUX1)    
//
//        enetInt             Ethernet MAC's Interrupt enumerated bit 
//                            position (see intc.h)
//                            (ex. aux1Int)   
//
//        INTC_ENET_INT       Ethernet MAC's Interrupt Bit position in
//                            Interrupt Controller's registers (see intc.h)
//                            (ex. INTC_AUX1_INT)
//
//
//    The Ethernet Controller hardware requires that Descriptors are Double 
//    Word Aligned in memory.  The Ethernet Controller hardware also requires 
//    that the Tx Buffer Address Pointers (TDES_2, TDES_3) are Double Word 
//    Aligned.  The Ethernet Controller hardware has no limitations on Rx 
//    Buffer Address Pointers (RDES_2, RDES_3).  However, since the Ethernet 
//    HAL uses the same functions and macros for both Transmit and Receive 
//    Descriptors, the Tx Buffer Address Pointers (TDES_2, TDES_3) and Rx 
//    Buffer Address Pointers (RDES_2, RDES_3) must all be Double Word Aligned.
//
/******************************************************************************/

#ifndef ETHERNET_H
#define ETHERNET_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "product.h"
#include "pubdefs.h"


/*=====================*
 *  Defines            *
 *=====================*/
/* Ethernet Block Defines */
#define EnetMacBase(etBase)                 (etBase)
#define EnetDmaBase(etBase)                 ((etBase) + 0x100)


/* Ethernet DMA Block Specific Defines */
/* Ethernet DMA Bus Mode Register bit definitions */
#define ENET_DMA_RESET                       (0x00000001)
#define ENET_DMA_BUS_PRIORITY                (0x00000002)
#define ENET_DMA_DESC_SKIP_DWORD_SHIFT       (2)
#define ENET_DMA_DESC_SKIP_DWORD_MASK        (0x001F)
#define ENET_DMA_BIG_ENDIAN                  (0x00000080)
#define ENET_DMA_BURST_LENGTH_SHIFT          (8)
#define ENET_DMA_BURST_LENGTH_MASK           (0x003F)
#define ENET_DMA_TX_AUTO_POLL_SHIFT          (17)
#define ENET_DMA_TX_AUTO_POLL_MASK           (0x0007)
#define ENET_DMA_DESC_BIG_ENDIAN             (0x00100000)
#define ENET_DMA_TX_ADDR_INCR                (0x00200000)


/* Ethernet DMA Descriptor List Address Register bit definitions */
#define ENET_DMA_DESC_LIST_ADDR_SHIFT        (0)
#define ENET_DMA_DESC_LIST_ADDR_MASK         (0xFFFFFFFC)


/* Ethernet DMA Status, Interrupt, Interrupt Enable 
   Register bit definitions */
#define ENET_DMA_TX_DONE_INT                 (0x00000001)
#define ENET_DMA_TX_STOPPED_INT              (0x00000002)
#define ENET_DMA_TX_BUF_UNAVAIL_INT          (0x00000004)
#define ENET_DMA_TX_JAB_TIMEOUT_INT          (0x00000008)
#define ENET_DMA_TX_UFLOW_INT                (0x00000020)
#define ENET_DMA_RX_DONE_INT                 (0x00000040)
#define ENET_DMA_RX_BUF_UNAVAIL_INT          (0x00000080)
#define ENET_DMA_RX_STOPPED_INT              (0x00000100)
#define ENET_DMA_GP_RX_WDOG_TMR_INT          (0x00000200)
#define ENET_DMA_TX_EARLY_INT                (0x00000400)
#define ENET_DMA_GP_TMR_INT                  (0x00000800)
#define ENET_DMA_BUS_ERROR_INT               (0x00002000)
#define ENET_DMA_RX_EARLY_INT                (0x00004000)
#define ENET_DMA_ABNORM_INT                  (0x00008000)
#define ENET_DMA_NORM_INT                    (0x00010000)
#define ENET_DMA_RX_STATE_SHIFT              (17)
#define ENET_DMA_RX_STATE_MASK               (0x0007)
#define ENET_DMA_TX_STATE_SHIFT              (20)
#define ENET_DMA_TX_STATE_MASK               (0x0007)
#define ENET_DMA_ERROR_SHIFT                 (23)
#define ENET_DMA_ERROR_MASK                  (0x0007)
#define ENET_DMA_ALL_INTS                    (0x0001FFFF)


/* Ethernet DMA Control Register bit definitions */
#define ENET_DMA_RX                          (0x00000002)
#define ENET_DMA_TX_SECOND_FRM               (0x00000004)
#define ENET_DMA_TX                          (0x00002000)
#define ENET_DMA_TX_THOLD_SHIFT              (14)
#define ENET_DMA_TX_THOLD_MASK               (0x0003)
#define ENET_DMA_TX_STORE_FWD                (0x00200000)
#define ENET_DMA_TX_THOLD_MODE               (0x00400000)


/* Ethernet DMA Transaction Register bit definitions */
#define ENET_DMA_TRANS_SHIFT                 (0)
#define ENET_DMA_TRANS_MASK                  (0x0003)
#define ENET_DMA_FRM_ADDR_SHIFT              (2)
#define ENET_DMA_FRM_ADDR_MASK               (0x3FFFFFFF)


/* Ethernet DMA Missed Frames Register bit definitions */
#define ENET_DMA_FRM_MISSED_HOST_SHIFT       (0)       /* Application */
#define ENET_DMA_FRM_MISSED_HOST_MASK        (0xFFFF)  /* Application */
#define ENET_DMA_FRM_MISSED_CTRL_SHIFT       (16)      /* Controller */
#define ENET_DMA_FRM_MISSED_CTRL_MASK        (0xFFFF)  /* Controller */


/* Ethernet MAC Control Register bit defintions */
#define ENET_MAC_RX_ENABLE                   (0x00000004)
#define ENET_MAC_TX_ENABLE                   (0x00000008)
#define ENET_MAC_DEFERRAL_CHECK              (0x00000020)
#define ENET_MAC_BOLMT_SHIFT                 (6)
#define ENET_MAC_BOLMT_MASK                  (0x0003)
#define ENET_MAC_PAD_STRIPPING               (0x00000100)
#define ENET_MAC_RETRY_DISABLE               (0x00000400)
#define ENET_MAC_BROADCAST_DISABLE           (0x00000800)
#define ENET_MAC_LATE_COLLISION              (0x00001000)
#define ENET_MAC_HASH_PERFECT_FILTER         (0x00002000)
#define ENET_MAC_HASH_ONLY                   (0x00008000)
#define ENET_MAC_PASS_BAD_FRM                (0x00010000)
#define ENET_MAC_INVERSE_FILTER              (0x00020000)
#define ENET_MAC_PASS_ALL_ADDR               (0x00040000)
#define ENET_MAC_MULTICAST                   (0x00080000)
#define ENET_MAC_FULL_DUPLEX                 (0x00100000)
#define ENET_MAC_LOOPBACK_MODE_SHIFT         (21)
#define ENET_MAC_LOOPBACK_MODE_MASK          (0x003)
#define ENET_MAC_RX_OWN_DISABLE              (0x00800000)
#define ENET_MAC_PORT_SELECT                 (0x08000000)
#define ENET_MAC_SQE_DISABLE                 (0x10000000)
#define ENET_MAC_BIG_ENDIAN                  (0x40000000)
#define ENET_MAC_RX_ALL_ADDR                 (0x80000000)


/* Ethernet MAC Address Register bit defintions */
#define ENET_MAC_HI_ADDR_SHIFT               (0)
#define ENET_MAC_HI_ADDR_MASK                (0xFFFF)
#define ENET_MAC_LO_ADDR_SHIFT               (0)
#define ENET_MAC_LO_ADDR_MASK                (0xFFFFFFFF)


/* Ethernet MAC Media Independent Interface Address Register bit defintions */
#define ENET_MAC_MII_BUSY                    (0x00000001)
#define ENET_MAC_MII_WR                      (0x00000002)
#define ENET_MAC_MII_REG_SEL_SHIFT           (6)
#define ENET_MAC_MII_REG_SEL_MASK            (0x001F)
#define ENET_MAC_MII_PHY_ADDR_SHIFT          (11)
#define ENET_MAC_MII_PHY_ADDR_MASK           (0x001F)
 

/* Ethernet MAC Media Independent Interface Data Register bit defintions */
#define ENET_MAC_MII_DATA_SHIFT              (0)
#define ENET_MAC_MII_DATA_MASK               (0xFFFF)


/* Ethernet MAC Flow Control Register bit defintions */
#define ENET_MAC_FLOW_CTRL_BUSY              (0x00000001)
#define ENET_MAC_FLOW_CTRL_ENABLE            (0x00000002)
#define ENET_MAC_FLOW_CTRL_FRM_ENABLE        (0x00000004)
#define ENET_MAC_FLOW_PAUSE_TIME_SHIFT       (16)
#define ENET_MAC_FLOW_PAUSE_TIME_MASK        (0xFFFF)


/* Ethernet MAC VLAN1 Tag Register bit defintions */
#define ENET_MAC_VLAN1_TAG_SHIFT             (0)
#define ENET_MAC_VLAN1_TAG_MASK              (0xFFFF)


/* Ethernet MAC VLAN2 Tag Register bit defintions */
#define ENET_MAC_VLAN2_TAG_SHIFT             (0)
#define ENET_MAC_VLAN2_TAG_MASK              (0xFFFF)




/* Ethernet DMA Rx Descriptor Frame Status bit definitions */
#define ENET_DESC_RX_STAT_USR0               (0x00000002)
#define ENET_DESC_RX_STAT_USR1_SHIFT         (2)
#define ENET_DESC_RX_STAT_USR1_MASK          (0x1F)
#define ENET_DESC_RX_STAT_USR2               (0x000080)
#define ENET_DESC_RX_STAT_LAST_DESC          (0x00000100)
#define ENET_DESC_RX_STAT_FIRST_DESC         (0x00000200)
#define ENET_DESC_RX_STAT_USR3_SHIFT         (10)
#define ENET_DESC_RX_STAT_USR3_MASK          (0x0F)
#define ENET_DESC_RX_STAT_DESC_ERROR         (0x00004000)
#define ENET_DESC_RX_STAT_ERROR              (0x00008000)
#define ENET_DESC_RX_STAT_LENGTH_SHIFT       (16)
#define ENET_DESC_RX_STAT_LENGTH_MASK        (0x00003FFF)        


/* Ethernet DMA Tx Descriptor Frame Status bit definitions */
#define ENET_DESC_TX_STAT_UFLOW              (0x00000002)
#define ENET_DESC_TX_STAT_USR1_SHIFT         (2)
#define ENET_DESC_TX_STAT_USR1_MASK          (0x1FFF)
#define ENET_DESC_TX_STAT_ERROR              (0x00008000)


/* Ethernet DMA Tx Descriptor Buffer Info bit definitions */
#define ENET_DESC_TX_PAD_DISABLE             (0x00800000)
#define ENET_DESC_TX_CRC_DISABLE             (0x04000000)
#define ENET_DESC_TX_FIRST_SEGMENT           (0x20000000)
#define ENET_DESC_TX_LAST_SEGMENT            (0x40000000)
#define ENET_DESC_TX_DONE_INT_ENABLE         (0x80000000)


/* Ethernet DMA Tx/Rx Descriptor bit definitions */
#define ENET_DESC_OWN                        (0x80000000)
#define ENET_DESC_STAT_ALL_BITS              (0x7FFFFFFF)
#define ENET_DESC_FRM_LENGTH_SHIFT           (16)
#define ENET_DESC_FRM_LENGTH_MASK            (0x3FFF)   
     
#define ENET_DESC_SECOND_ADDR_CHAINED        (0x01000000)
#define ENET_DESC_RING_END                   (0x02000000)
#define ENET_DESC_BUF_PTR_SHIFT              (0)
#define ENET_DESC_BUF_PTR_MASK               (0xFFFFFFFC)
#define ENET_DESC_BUF1_SIZE_SHIFT            (0)
#define ENET_DESC_BUF1_SIZE_MASK             (0x07FF)
#define ENET_DESC_BUF2_SIZE_SHIFT            (11)
#define ENET_DESC_BUF2_SIZE_MASK             (0x07FF)




/*=====================*
 *  Type Defines       *
 *=====================*/
/* Ethernet DMA Channel */
typedef struct enetDmaRegs_t 
{
    volatile uint32 dmaBusMode;           /* 0x00 */
    volatile uint32 dmaTxDescPoll;        /* 0x04 */
    volatile uint32 dmaRxDescPoll;        /* 0x08 */
    volatile uint32 dmaRxDescListAddr;    /* 0x0C */
    volatile uint32 dmaTxDescListAddr;    /* 0x10 */
    volatile uint32 dmaStat;              /* 0x14 */
    volatile uint32 dmaCtrl;              /* 0x18 */
    volatile uint32 dmaIntEnable;         /* 0x1C */
    volatile uint32 dmaMissedFrm;         /* 0x20 */
    volatile uint32 reserved0[10];        /* 0x24 */
    volatile uint32 dmaTxDestAddr;        /* 0x4C */
    volatile uint32 dmaTxBufAddr;         /* 0x50 */
    volatile uint32 dmaRxBufAddr;         /* 0x54 */
} enetDmaRegs;


/* Ethernet MAC (Media Access Controller) block */
typedef struct enetMacRegs_t 
{
    volatile uint32 macCtrl;              /* 0x00 */
    volatile uint32 macAddrHi;            /* 0x04 */ 
    volatile uint32 macAddrLo;            /* 0x08 */
    volatile uint32 macMulticastHi;       /* 0x0C */
    volatile uint32 macMulticastLo;       /* 0x10 */
    volatile uint32 macMiiAddr;           /* 0x14 */
    volatile uint32 macMiiData;           /* 0x18 */
    volatile uint32 macFlowCtrl;          /* 0x1C */
    volatile uint32 macVlan1Tag;          /* 0x20 */
    volatile uint32 macVlan2Tag;          /* 0x24 */ 
} enetMacRegs;


/* Ethernet DMA Transmit Process States */
typedef enum enetDmaTxState_t
{
    txStopped,
    txDescFetch,
    txWait,
    txBufFill,
    txReserved0,
    txReserved1,
    txSuspended,
    txDescClose

} enetDmaTxState;


/* Ethernet DMA Receive Process States */
typedef enum enetDmaRxState_t
{
    rxStopped,
    rxDescFetch,
    rxWaitPacketEnd,
    rxWaitNextPacket,
    rxSuspended,
    rxDescClose,
    rxBufFlush,
    rxBufEmpty

} enetDmaRxState;


/* Ethernet DMA Transaction  */
typedef enum enetDmaTransaction_t
{
    start,
    inProgress,
    lastDword,
    stat

} enetDmaTransaction;


/* Ethernet MAC BackOff Limit bits */
typedef enum enetMacBolmtBits_t
{
    bolmtBits10,                  /* Wait 1024 slot times maximum */
    bolmtBits8,                   /* Wait 256 slot times maximum */
    bolmtBits4,                   /* Wait 16 slot times maximum */
    bolmtBits1                    /* Wait 2 slot times maximum */

} enetMacBolmtBits;


/* Ethernet MAC Loopback Modes */
typedef enum enetMacLoopbackModes_t
{
    noLoopback,                   /* No feedback */
    intLoopback,                  /* MII */
    extLoopback,                  /* PHY */
    reservedLoopback

} enetMacLoopbackModes;


/* Ethernet MAC Media Independent Interface (MII) Registers */
typedef enum enetMacMiiReg_t
{
    miiReg0,
    miiReg1,
    miiReg2,
    miiReg3,
    miiReg4,
    miiReg5,
    miiReg6,
    miiReg7,
    miiReg8,
    miiReg9,
    miiReg10,
    miiReg11,
    miiReg12,
    miiReg13,
    miiReg14,
    miiReg15,
    miiReg16,
    miiReg17,
    miiReg18
} enetMacMiiReg;


/* Ethernet DMA Descriptor Format Structure */
typedef struct enetDesc_t
{
    volatile uint32 descFrmStat;        /* rdes0 */
    volatile uint32 descCtrl;           /* rdes1 */
    volatile uint32 descBuf1Ptr;        /* rdes2 */
    volatile uint32 descBuf2Ptr;        /* rdes3 */

} enetDesc;


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/
PUBLIC void EnetDescInit ( enetDesc *enetDescPtr, uint32 enetDescCtrlBits, 
                           uint32 enetDescBuf1Addr, uint32 enetDescBuf1Size, 
                           uint32 enetDescBuf2Addr, uint32 enetDescBuf2Size );
PUBLIC void EnetMacLoadPhyMiiReg ( enetMacRegs *etMacPtr, uint32 etPhyAddr,
                                   enetMacMiiReg etMiiReg, uint32 value );
PUBLIC uint32 EnetMacGetPhyMiiReg ( enetMacRegs *etMacPtr, uint32 etPhyAddr,
                                    enetMacMiiReg etMiiReg );


/*=====================*
 *  Macro Functions    *
 *=====================*/




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetBusMode()
//
// SYNOPSIS       uint32 EnetDmaGetBusMode ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Bus Mode Register value
//
// DESCRIPTION    Returns DMA Bus Mode Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetBusMode(enetPtr)                                          \
    ((enetPtr)->dmaBusMode)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaLoadBusMode()
//
// SYNOPSIS       void EnetDmaLoadBusMode ( enetDmaRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into Dma Bus Mode Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaLoadBusMode(enetPtr,val)                                     \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
                                                                            \
    (enetPtr)->dmaBusMode = (uint32)(val);                                  \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaSetBusMode()
//
// SYNOPSIS       void EnetDmaSetBusMode ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets DMA Bus Mode Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaSetBusMode(enetPtr,bits)                                     \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaBusMode |= (bits);                                        \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaClrBusMode()
//
// SYNOPSIS       void EnetDmaClrBusMode ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Clears DMA Bus Mode Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaClrBusMode(enetPtr,bits)                                     \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaBusMode &= ~(bits);                                       \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaIsBusMode()
//
// SYNOPSIS       void EnetDmaIsBusMode ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set status
//
// DESCRIPTION    Returns TRUE if requested DMA Bus Mode Register bits are set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaIsBusMode(enetPtr,bits)                                      \
    (((enetPtr)->dmaBusMode & (bits)) ? TRUE : FALSE)






/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaLoadTxDescPoll()
//
// SYNOPSIS       void EnetDmaLoadTxDescPoll ( enetDmaRegs *enetPtr, 
//                                             uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into DMA Transmit Poll Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaLoadTxDescPoll(enetPtr,val)                                  \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaTxDescPoll = (uint32)(val);                               \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaLoadRxDescPoll()
//
// SYNOPSIS       void EnetDmaLoadRxDescPoll ( enetDmaRegs *enetPtr,
//                                             uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into DMA Receive Poll Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaLoadRxDescPoll(enetPtr,val)                                  \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaRxDescPoll = (uint32)(val);                               \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetRxDescListAddr()
//
// SYNOPSIS       uint32 EnetDmaGetRxDescListAddr ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Receive Base Address Register value
//
// DESCRIPTION    Returns DMA Receive Base Address Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetRxDescListAddr(enetPtr)                                   \
    ((enetPtr)->dmaRxDescListAddr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaLoadRxDescListAddr()
//
// SYNOPSIS       void EnetDmaLoadRxDescListAddr ( enetDmaRegs *enetPtr, 
//                                                 uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into DMA Receive Base Address Register
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaLoadRxDescListAddr(enetPtr,val)                              \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaRxDescListAddr =                                          \
        (((uint32)(val) & ENET_DMA_DESC_LIST_ADDR_MASK) <<                  \
         ENET_DMA_DESC_LIST_ADDR_SHIFT);                                    \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxDescListAddr()
//
// SYNOPSIS       uint32 EnetDmaGetTxDescListAddr ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Transmit Base Address Register value
//
// DESCRIPTION    Returns DMA Transmit Base Address Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxDescListAddr(enetPtr)                                   \
    ((enetPtr)->dmaTxDescListAddr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaLoadTxDescListAddr()
//
// SYNOPSIS       void EnetDmaLoadTxDescListAddr ( enetDmaRegs *enetPtr, 
//                                                 uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into DMA Transmit Base Address Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaLoadTxDescListAddr(enetPtr,val)                              \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaTxDescListAddr =                                          \
        (((uint32)(val) & ENET_DMA_DESC_LIST_ADDR_MASK) <<                  \
         ENET_DMA_DESC_LIST_ADDR_SHIFT);                                    \
} while (0)





/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetStat()
//
// SYNOPSIS       uint32 EnetDmaGetStat ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Status Register value
//
// DESCRIPTION    Returns DMA Status Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetStat(enetPtr)                                             \
    ((enetPtr)->dmaStat)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaClrStat()
//
// SYNOPSIS       void EnetDmaClrStat ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be Clear
//                 
// OUTPUT         None
//
// DESCRIPTION    Clears DMA Status Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaClrStat(enetPtr,bits)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaStat |= (bits);                                           \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaIsStat()
//
// SYNOPSIS       void EnetDmaIsStat ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set status
//
// DESCRIPTION    Returns TRUE if requested DMA Status Register bits are set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaIsStat(enetPtr,bits)                                         \
    (((enetPtr)->dmaStat & (bits)) ? TRUE : FALSE)






/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetCtrl()
//
// SYNOPSIS       uint32 EnetDmaGetCtrl ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Control Register value
//
// DESCRIPTION    Returns DMA Control Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetCtrl(enetPtr)                                             \
    ((enetPtr)->dmaCtrl)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaLoadCtrl()
//
// SYNOPSIS       void EnetDmaLoadCtrl ( enetDmaRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into DMA Control Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaLoadCtrl(enetPtr,val)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaCtrl = (uint32)(val);                                     \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaSetCtrl()
//
// SYNOPSIS       void EnetDmaSetCtrl ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets DMA Control Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaSetCtrl(enetPtr,bits)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaCtrl |= (bits);                                           \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaClrCtrl()
//
// SYNOPSIS       void EnetDmaClrCtrl ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Clears DMA Control Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaClrCtrl(enetPtr,bits)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaCtrl &= ~(bits);                                          \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaIsCtrl()
//
// SYNOPSIS       void EnetDmaIsCtrl ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set Control
//
// DESCRIPTION    Returns TRUE if requested DMA Control Register bits are set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaIsCtrl(enetPtr,bits)                                         \
    (((enetPtr)->dmaCtrl & (bits)) ? TRUE : FALSE)






/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetIntEnable()
//
// SYNOPSIS       uint32 EnetDmaGetIntEnable ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Interrupt Enable Register value
//
// DESCRIPTION    Returns DMA Interrupt Enable Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetIntEnable(enetPtr)                                        \
    ((enetPtr)->dmaIntEnable)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaEnableInt()
//
// SYNOPSIS       void EnetDmaEnableInt ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Interrupt bits to be enabled
//                 
// OUTPUT         None
//
// DESCRIPTION    Enables DMA Interrupts.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaEnableInt(enetPtr,bits)                                      \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaIntEnable |= (bits);                                      \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaDisableInt()
//
// SYNOPSIS       void EnetDmaDisableInt ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Interrupt bits to be disabled
//                 
// OUTPUT         None
//
// DESCRIPTION    Disables DMA Interrupts.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaDisableInt(enetPtr,bits)                                     \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaIntEnable &= ~(bits);                                     \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaIsIntEnable()
//
// SYNOPSIS       void EnetDmaIsIntEnable ( enetDmaRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set IntEnable
//
// DESCRIPTION    Returns TRUE if requested DMA IntEnable Register bits are 
//                set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaIsIntEnable(enetPtr,bits)                                    \
    (((enetPtr)->dmaIntEnable & (bits)) ? TRUE : FALSE)






/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxDestAddr()
//
// SYNOPSIS       uint32 EnetDmaGetTxDestAddr ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Transmit Destination Address Register value
//
// DESCRIPTION    Returns DMA Transmit Destination Address Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxDestAddr(enetPtr)                                       \
    ((enetPtr)->dmaTxDestAddr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxBufAddr()
//
// SYNOPSIS       uint32 EnetDmaGetTxBufAddr ( enetDmaRegs 
//                                             *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Transmit Buffer Address Register value
//
// DESCRIPTION    Returns DMA Transmit Buffer Address Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxBufAddr(enetPtr)                                        \
    ((enetPtr)->dmaTxBufAddr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetRxBufAddr()
//
// SYNOPSIS       uint32 EnetDmaGetRxBufAddr ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Receive Buffer Address Register value
//
// DESCRIPTION    Returns DMA Receive Buffer Address Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetRxBufAddr(enetPtr)                                        \
    ((enetPtr)->dmaRxBufAddr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaLoadRxBufAddr()
//
// SYNOPSIS       void EnetDmaLoadRxBufAddr ( enetDmaRegs *enetPtr, 
//                                            uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into DMA Receive Buffer Address Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaLoadRxBufAddr(enetPtr,val)                                   \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->dmaRxBufAddr = (uint32)(val);                                \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetMissedFrm()
//
// SYNOPSIS       uint32 EnetDmaGetMissedFrm ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: DMA Missed Frame Register value
//
// DESCRIPTION    Returns DMA Missed Frame Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetMissedFrm(enetPtr)                                        \
    ((enetPtr)->dmaMissedFrm)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetCtrl()
//
// SYNOPSIS       uint32 EnetMacGetCtrl ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC Control Register value
//
// DESCRIPTION    Returns MAC Control Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetCtrl(enetPtr)                                             \
    ((enetPtr)->macCtrl)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadCtrl()
//
// SYNOPSIS       void EnetMacLoadCtrl ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC Control Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadCtrl(enetPtr,val)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macCtrl = (uint32)(val);                                     \
} while (0)  




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacSetCtrl()
//
// SYNOPSIS       void EnetMacSetCtrl ( enetMacRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets MAC Control Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacSetCtrl(enetPtr,bits)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macCtrl |= (bits);                                           \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacClrCtrl()
//
// SYNOPSIS       void EnetMacClrCtrl ( enetMacRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Clears MAC Control Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacClrCtrl(enetPtr,bits)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macCtrl &= ~(bits);                                          \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacIsCtrl()
//
// SYNOPSIS       void EnetMacIsCtrl ( enetMacRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set Control
//
// DESCRIPTION    Returns TRUE if requested MAC Control Register bits are set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacIsCtrl(enetPtr,bits)                                         \
    (((enetPtr)->macCtrl & (bits)) ? TRUE : FALSE)






/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetAddrHi()
//
// SYNOPSIS       uint32 EnetMacGetAddrHi ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC Address Hi Register value
//
// DESCRIPTION    Returns MAC Address Hi Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetAddrHi(enetPtr)                                           \
    ((enetPtr)->macAddrHi)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadAddrHi()
//
// SYNOPSIS       void EnetMacLoadAddrHi ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC Address Hi Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadAddrHi(enetPtr,val)                                      \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macAddrHi = (((uint32)(val) & ENET_MAC_HI_ADDR_MASK) <<      \
                            ENET_MAC_HI_ADDR_SHIFT);                        \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetAddrLo()
//
// SYNOPSIS       uint32 EnetMacGetAddrLo ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC Address Lo Register value
//
// DESCRIPTION    Returns MAC Address Lo Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetAddrLo(enetPtr)                                           \
    ((enetPtr)->macAddrLo)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadAddrLo()
//
// SYNOPSIS       void EnetMacLoadAddrLo ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC Address Lo Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadAddrLo(enetPtr,val)                                      \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macAddrLo = (((uint32)(val) & ENET_MAC_LO_ADDR_MASK) <<      \
                            ENET_MAC_LO_ADDR_SHIFT);                        \
} while (0)  




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetMulticastLo()
//
// SYNOPSIS       uint32 EnetMacGetMulticastLo ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC Multicast Lo Register value
//
// DESCRIPTION    Returns MAC Multicast Lo Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetMulticastLo(enetPtr)                                      \
    ((enetPtr)->macMulticastLo)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadMulticastLo()
//
// SYNOPSIS       void EnetMacLoadMulticastLo ( enetMacRegs *enetPtr, 
//                                              uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC Multicast Lo Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadMulticastLo(enetPtr,val)                                 \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macMulticastLo = (uint32)(val);                              \
} while (0)





/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetMulticastHi()
//
// SYNOPSIS       uint32 EnetMacGetMulticastHi ( enetMacRegs enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC Multicast Hi Register value
//
// DESCRIPTION    Returns MAC Multicast Hi Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetMulticastHi(enetPtr)                                      \
    ((enetPtr)->macMulticastHi)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadMulticastHi()
//
// SYNOPSIS       void EnetMacLoadMulticastHi ( enetMacRegs *enetPtr, 
//                                              uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Hiaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC Multicast Hi Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadMulticastHi(enetPtr,val)                                 \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macMulticastHi = (uint32)(val);                              \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetMiiAddr()
//
// SYNOPSIS       uint32 EnetMacGetMiiAddr ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC MII Address Register value
//
// DESCRIPTION    Returns MAC MII Address Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetMiiAddr(enetPtr)                                          \
    ((enetPtr)->macMiiAddr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadMiiAddr()
//
// SYNOPSIS       void EnetMacLoadMiiAddr ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC MII Address Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadMiiAddr(enetPtr,val)                                     \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macMiiAddr = (uint32)(val);                                  \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetMiiData()
//
// SYNOPSIS       uint32 EnetMacGetMiiData ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC MII Data Register value
//
// DESCRIPTION    Returns MAC MII Data Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetMiiData(enetPtr)                                          \
    ((enetPtr)->macMiiData)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadMiiData()
//
// SYNOPSIS       void EnetMacLoadMiiData ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC MII Data Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadMiiData(enetPtr,val)                                     \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macMiiData = (((uint32)(val) & ENET_MAC_MII_DATA_MASK) <<    \
                             ENET_MAC_MII_DATA_SHIFT);                      \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetFlowCtrl()
//
// SYNOPSIS       uint32 EnetMacGetFlowCtrl ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC Flow Control Register value
//
// DESCRIPTION    Returns MAC Flow Control Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetFlowCtrl(enetPtr)                                         \
    ((enetPtr)->macFlowCtrl)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadFlowCtrl()
//
// SYNOPSIS       void EnetMacLoadFlowCtrl ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC Flow Control Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadFlowCtrl(enetPtr,val)                                    \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macFlowCtrl = (uint32)(val);                                 \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacSetFlowCtrl()
//
// SYNOPSIS       void EnetMacSetFlowCtrl ( enetMacRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets MAC FlowControl Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacSetFlowCtrl(enetPtr,bits)                                    \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macFlowCtrl |= (bits);                                       \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacClrFlowCtrl()
//
// SYNOPSIS       void EnetMacClrFlowCtrl ( enetMacRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Clears MAC FlowControl Register bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacClrFlowCtrl(enetPtr,bits)                                    \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macFlowCtrl &= ~(bits);                                      \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacIsFlowCtrl()
//
// SYNOPSIS       void EnetMacIsFlowCtrl ( enetMacRegs *enetPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set FlowControl
//
// DESCRIPTION    Returns TRUE if requested MAC FlowControl Register bits are 
//                set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacIsFlowCtrl(enetPtr,bits)                                     \
    (((enetPtr)->macFlowCtrl & (bits)) ? TRUE : FALSE)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetVlan1Tag()
//
// SYNOPSIS       uint32 EnetMacGetVlan1Tag ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC VLAN1 Tag Register value
//
// DESCRIPTION    Returns MAC VLAN1 Tag Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetVlan1Tag(enetPtr)                                         \
    ((enetPtr)->macVlan1Tag)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadVlan1Tag()
//
// SYNOPSIS       void EnetMacLoadVlan1Tag ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC VLAN1 Tag Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadVlan1Tag(enetPtr,val)                                    \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macVlan1Tag = (((uint32)(val) & ENET_MAC_VLAN1_TAG_MASK) <<  \
                              ENET_MAC_VLAN1_TAG_SHIFT);                    \
} while (0)





/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetVlan2Tag()
//
// SYNOPSIS       uint32 EnetMacGetVlan2Tag ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                
// OUTPUT         uint32: MAC VLAN2 Tag Register value
//
// DESCRIPTION    Returns MAC VLAN2 Tag Register value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetVlan2Tag(enetPtr)                                         \
    ((enetPtr)->macVlan2Tag)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacLoadVlan2Tag()
//
// SYNOPSIS       void EnetMacLoadVlan2Tag ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into MAC VLAN2 Tag Register.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacLoadVlan2Tag(enetPtr,val)                                    \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetPtr)->macVlan2Tag = (((uint32)(val) & ENET_MAC_VLAN2_TAG_MASK) <<  \
                              ENET_MAC_VLAN2_TAG_SHIFT);                    \
} while (0)






/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetDescSkipDword()
//
// SYNOPSIS       void EnetDmaGetDescSkipDword ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Bus Mode Descriptor Skip Dwords value
//
// DESCRIPTION    Gets value of DMA Descriptor Skip Dwords (DMA Bus Mode).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetDescSkipDword(enetPtr)                                    \
    ( (EnetDmaGetBusMode(enetPtr) >>  ENET_DMA_DESC_SKIP_DWORD_SHIFT) &     \
      ENET_DMA_DESC_SKIP_DWORD_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaSetDescSkipDword()
//
// SYNOPSIS       void EnetDmaSetDescSkipDword ( enetDmaRegs *enetPtr, 
//                                               uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of DMA Descriptor Skip Dwords (DMA Bus Mode).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaSetDescSkipDword(enetPtr,val)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetDmaLoadBusMode ( enetPtr,                                           \
        ( (EnetDmaGetBusMode(enetPtr) &                                     \
           (~(ENET_DMA_DESC_SKIP_DWORD_MASK <<                              \
              ENET_DMA_DESC_SKIP_DWORD_SHIFT))) |                           \
          (((uint32)(val) & ENET_DMA_DESC_SKIP_DWORD_MASK) <<               \
           ENET_DMA_DESC_SKIP_DWORD_SHIFT)) );                              \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetBurstLength()
//
// SYNOPSIS       void EnetDmaGetBurstLength ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Bus Mode Burst Length value
//
// DESCRIPTION    Gets value of DMA Burst Length (DMA Bus Mode).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetBurstLength(enetPtr)                                      \
    ( (EnetDmaGetBusMode(enetPtr) >> ENET_DMA_BURST_LENGTH_SHIFT) &         \
      ENET_DMA_BURST_LENGTH_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaSetBurstLength()
//
// SYNOPSIS       void EnetDmaSetBurstLength ( enetDmaRegs *enetPtr, 
//                                             uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of DMA Burst Length (DMA Bus Mode).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaSetBurstLength(enetPtr,val)                                  \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetDmaLoadBusMode ( enetPtr,                                           \
        ( (EnetDmaGetBusMode(enetPtr) &                                     \
           (~(ENET_DMA_BURST_LENGTH_MASK <<                                 \
              ENET_DMA_BURST_LENGTH_SHIFT))) |                              \
          (((uint32)(val) & ENET_DMA_BURST_LENGTH_MASK) <<                  \
           ENET_DMA_BURST_LENGTH_SHIFT)) );                                 \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxAutoPoll()
//
// SYNOPSIS       void EnetDmaGetTxAutoPoll ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Bus Mode Descriptor Tx Auto Poll value
//
// DESCRIPTION    Gets value of DMA Tx Auto Polling (DMA Bus Mode).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxAutoPoll(enetPtr)                                       \
    ( (EnetDmaGetBusMode(enetPtr) >> ENET_DMA_TX_AUTO_POLL_SHIFT) &         \
      ENET_DMA_TX_AUTO_POLL_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaSetTxAutoPoll()
//
// SYNOPSIS       void EnetDmaSetTxAutoPoll ( enetDmaRegs *enetPtr,
//                                            uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of DMA Tx Auto Polling (DMA Bus Mode).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaSetTxAutoPoll(enetPtr,val)                                   \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetDmaLoadBusMode ( enetPtr,                                           \
        ( (EnetDmaGetBusMode(enetPtr) &                                     \
           (~(ENET_DMA_TX_AUTO_POLL_MASK <<                                 \
              ENET_DMA_TX_AUTO_POLL_SHIFT))) |                              \
          (((uint32)(val) & ENET_DMA_TX_AUTO_POLL_MASK) <<                  \
           ENET_DMA_TX_AUTO_POLL_SHIFT)) );                                 \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetRxState()
//
// SYNOPSIS       void EnetDmaGetRxState ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Status Rx State value
//
// DESCRIPTION    Gets value of DMA Rx State (DMA Status).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetRxState(enetPtr)                                          \
    ( (EnetDmaGetStat(enetPtr) >> ENET_DMA_RX_STATE_SHIFT) &                \
      ENET_DMA_RX_STATE_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxState()
//
// SYNOPSIS       void EnetDmaGetTxState ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Status Tx State value
//
// DESCRIPTION    Gets value of DMA Tx State (DMA Status).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxState(enetPtr)                                          \
    ( (EnetDmaGetStat(enetPtr) >> ENET_DMA_TX_STATE_SHIFT) &                \
      ENET_DMA_TX_STATE_MASK )                                  




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetError()
//
// SYNOPSIS       void EnetDmaGetError ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Status Error value
//
// DESCRIPTION    Gets value of DMA Errors (DMA Status).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetError(enetPtr)                                            \
    ( (EnetDmaGetStat(enetPtr) >> ENET_DMA_ERROR_SHIFT) &                   \
      ENET_DMA_ERROR_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxThold()
//
// SYNOPSIS       void EnetDmaGetTxThold ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Control Threshold value
//
// DESCRIPTION    Gets value of DMA Tx Threshold (DMA Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxThold(enetPtr)                                          \
    ( (EnetDmaGetCtrl(enetPtr) >> ENET_DMA_TX_THOLD_SHIFT) &                \
      ENET_DMA_TX_THOLD_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaSetTxThold()
//
// SYNOPSIS       void EnetDmaSetTxThold ( enetDmaRegs *enetPtr, 
//                                             uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of DMA Tx Threshold (DMA Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaSetTxThold(enetPtr,val)                                      \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetDmaLoadCtrl ( enetPtr ,                                             \
        ( (EnetDmaGetCtrl(enetPtr) &                                        \
           (~(ENET_DMA_TX_THOLD_MASK << ENET_DMA_TX_THOLD_SHIFT))) |        \
          (((uint32)(val) & ENET_DMA_TX_THOLD_MASK) <<                      \
           ENET_DMA_TX_THOLD_SHIFT)) );                                     \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxDestTrans()
//
// SYNOPSIS       void EnetDmaGetTxDestTrans ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Tx Dest Transaction value
//
// DESCRIPTION    Gets value of DMA Tx Dest Transaction (DMA Tx Dest Addr).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxDestTrans(enetPtr)                                      \
    ( (EnetDmaGetTxDestAddr(enetPtr) >> ENET_DMA_TRANS_SHIFT) &             \
      ENET_DMA_TRANS_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetTxDestFrmAddr()
//
// SYNOPSIS       void EnetDmaGetTxDestFrmAddr ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Tx Dest Address value
//
// DESCRIPTION    Gets value of DMA Tx Destination Address (DMA Tx Dest Addr).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetTxDestFrmAddr(enetPtr)                                    \
    ( (EnetDmaGetTxDestAddr(enetPtr) >> ENET_DMA_FRM_ADDR_SHIFT) &          \
      ENET_DMA_FRM_ADDR_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetFrmMissedHost()
//
// SYNOPSIS       void EnetDmaGetFrmMissedHost ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Missed Frames By Host value
//
// DESCRIPTION    Gets value of DMA Frames Missed By Host (DMA Missed Frames).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetFrmMissedHost(enetPtr)                                    \
    ( (EnetDmaGetMissedFrm(enetPtr) >> ENET_DMA_FRM_MISSED_HOST_SHIFT) &    \
      ENET_DMA_FRM_MISSED_HOST_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDmaGetFrmMissedCtrl()
//
// SYNOPSIS       void EnetDmaGetFrmMissedCtrl ( enetDmaRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetDmaRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: DMA Missed Frames By Ctrl value
//
// DESCRIPTION    Gets value of DMA Frames Missed By Ctrl (DMA Missed Frames). 
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDmaGetFrmMissedCtrl(enetPtr)                                    \
    ( (EnetDmaGetMissedFrm(enetPtr) >> ENET_DMA_FRM_MISSED_CTRL_SHIFT) &    \
      ENET_DMA_FRM_MISSED_CTRL_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetBolmt()
//
// SYNOPSIS       void EnetMacGetBolmt ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: MAC Control BOLMT value
//
// DESCRIPTION    Gets value of MAC Get Back Off Limit (MAC Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetBolmt(enetPtr)                                            \
    ( (EnetMacGetCtrl(enetPtr) >> ENET_MAC_BOLMT_SHIFT) &                   \
      ENET_MAC_BOLMT_MASK )                                        




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacSetBolmt()
//
// SYNOPSIS       void EnetMacSetBolmt ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of MAC Get Back Off Limit (MAC Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacSetBolmt(enetPtr,val)                                        \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetMacLoadCtrl ( enetPtr,                                              \
        ( (EnetMacGetCtrl(enetPtr) &                                        \
           (~(ENET_MAC_BOLMT_MASK << ENET_MAC_BOLMT_SHIFT))) |              \
          (((uint32)(val) & ENET_MAC_BOLMT_MASK) <<                         \
           ENET_MAC_BOLMT_SHIFT)) );                                        \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetLoopbackMode()
//
// SYNOPSIS       void EnetMacGetLoopbackMode ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: MAC Control loopback mode value
//
// DESCRIPTION    Gets value of MAC Loopback Mode (MAC Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetLoopbackMode(enetPtr)                                     \
    ( (EnetMacGetCtrl(enetPtr) >> ENET_MAC_LOOPBACK_MODE_SHIFT) &           \
      ENET_MAC_LOOPBACK_MODE_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacSetLoopbackMode()
//
// SYNOPSIS       void EnetMacSetLoopbackMode ( enetMacRegs *enetPtr, 
//                                              uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of MAC Loopback Mode (MAC Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacSetLoopbackMode(enetPtr,val)                                 \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetMacLoadCtrl ( enetPtr,                                              \
        ( (EnetMacGetCtrl(enetPtr) &                                        \
           (~(ENET_MAC_LOOPBACK_MODE_MASK <<                                \
              ENET_MAC_LOOPBACK_MODE_SHIFT))) |                             \
          (((uint32)(val) & ENET_MAC_LOOPBACK_MODE_MASK) <<                 \
           ENET_MAC_LOOPBACK_MODE_SHIFT)) );                                \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetMiiRegSel()
//
// SYNOPSIS       void EnetMacGetMiiRegSel ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: MAC MII Register selected value
//
// DESCRIPTION    Gets value of MAC MII Register Selected (MAC MII Addr).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetMiiRegSel(enetPtr)                                        \
    ( (EnetMacGetMiiAddr(enetPtr) >> ENET_MAC_MII_REG_SEL_SHIFT) &          \
      ENET_MAC_MII_REG_SEL_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacSetMiiRegSel()
//
// SYNOPSIS       void EnetMacSetMiiRegSel ( enetMacRegs *enetPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of MAC MII Register Selected (MAC MII Addr).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacSetMiiRegSel(enetPtr,val)                                    \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetMacLoadMiiAddr ( enetPtr,                                           \
        ( (EnetMacGetMiiAddr(enetPtr) &                                     \
           (~(ENET_MAC_MII_REG_SEL_MASK << ENET_MAC_MII_REG_SEL_SHIFT))) |  \
          (((uint32)(val) & ENET_MAC_MII_REG_SEL_MASK) <<                   \
           ENET_MAC_MII_REG_SEL_SHIFT)) );                                  \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetMiiPhyAddr()
//
// SYNOPSIS       void EnetMacGetMiiPhyAddr ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: MAC MII Phy Address selected value
//
// DESCRIPTION    Gets value of MAC MII Phy Address (MAC MII Addr).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetMiiPhyAddr(enetPtr)                                       \
    ( (EnetMacGetMiiAddr(enetPtr) >> ENET_MAC_MII_PHY_ADDR_SHIFT) &         \
      ENET_MAC_MII_PHY_ADDR_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacSetMiiPhyAddr()
//
// SYNOPSIS       void EnetMacSetMiiPhyAddr ( enetMacRegs *enetPtr, 
//                                            uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of MAC MII Phy Address (MAC MII Addr).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacSetMiiPhyAddr(enetPtr,val)                                   \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetMacLoadMiiAddr ( enetPtr,                                           \
        ( (EnetMacGetMiiAddr(enetPtr) &                                     \
           (~(ENET_MAC_MII_PHY_ADDR_MASK << ENET_MAC_MII_PHY_ADDR_SHIFT))) |\
          (((uint32)(val) & ENET_MAC_MII_PHY_ADDR_MASK) <<                  \
           ENET_MAC_MII_PHY_ADDR_SHIFT)) );                                 \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacGetFlowPauseTime()
//
// SYNOPSIS       void EnetMacGetFlowPauseTime ( enetMacRegs *enetPtr )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                 
// OUTPUT         uint32: MAC Flow Control Pause Time value
//
// DESCRIPTION    Gets value of MAC Flow Control Pause Time (MAC Flow Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacGetFlowPauseTime(enetPtr)                                    \
    ( (EnetMacGetFlowCtrl(enetPtr) >> ENET_MAC_FLOW_PAUSE_TIME_SHIFT) &     \
      ENET_MAC_FLOW_PAUSE_TIME_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetMacSetFlowPauseTime()
//
// SYNOPSIS       void EnetMacGetFlowPauseTime ( enetMacRegs *enetPtr, 
//                                               uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetMacRegs *enetPtr: Base Pointer of the Ethernet device
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of MAC Flow Control Pause Time (MAC Flow Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetMacSetFlowPauseTime(enetPtr,val)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetMacLoadFlowCtrl ( enetPtr ,                                         \
        ( (EnetMacGetFlowCtrl(enetPtr) &                                    \
          (~((uint32)ENET_MAC_FLOW_PAUSE_TIME_MASK <<                       \
             ENET_MAC_FLOW_PAUSE_TIME_SHIFT))) |                            \
          (((uint32)(val) & ENET_MAC_FLOW_PAUSE_TIME_MASK) <<               \
           ENET_MAC_FLOW_PAUSE_TIME_SHIFT)) );                              \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescGetFrmStat()
//
// SYNOPSIS       uint32 EnetDescGetFrmStat ( enetDesc *enetDescPtr )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                
// OUTPUT         uint32: Ethernet Descriptor Frame Status value
//
// DESCRIPTION    Returns Ethernet Descriptor Frame Status value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescGetFrmStat(enetDescPtr)                                     \
    ((enetDescPtr)->descFrmStat)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescLoadFrmStat()
//
// SYNOPSIS       void EnetDescLoadFrmStat ( enetDesc *enetDescPtr, 
//                                           uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into Ethernet Descriptor Frame Status.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescLoadFrmStat(enetDescPtr,val)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descFrmStat = (uint32)(val);                             \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescSetFrmStat()
//
// SYNOPSIS       void EnetDescSetFrmStat ( enetDesc *enetDescPtr,
//                                          uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets Ethernet Descriptor Frame Status bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescSetFrmStat(enetDescPtr,bits)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descFrmStat |= (bits);                                   \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescClrFrmStat()
//
// SYNOPSIS       void EnetDescClrFrmStat ( enetDesc *enetDescPtr, 
//                                          uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Clears Ethernet Descriptor Frame Status bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescClrFrmStat(enetDescPtr,bits)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descFrmStat &= ~(bits);                                  \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescIsFrmStat()
//
// SYNOPSIS       void EnetDescIsFrmStat ( enetDesc *enetDescPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set status
//
// DESCRIPTION    Returns TRUE if requested Ethernet Descriptor Frame Status
//                bits are set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescIsFrmStat(enetDescPtr,bits)                                 \
    (((enetDescPtr)->descFrmStat & (bits)) ? TRUE : FALSE)





/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescGetCtrl()
//
// SYNOPSIS       uint32 EnetDescGetCtrl ( enetDesc *enetDescPtr )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                
// OUTPUT         uint32: Ethernet Descriptor Control value
//
// DESCRIPTION    Returns Ethernet Descriptor Control value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescGetCtrl(enetDescPtr)                                        \
    ((enetDescPtr)->descCtrl)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescLoadCtrl()
//
// SYNOPSIS       void EnetDescLoadCtrl ( enetDesc *enetDescPtr, uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into Ethernet Descriptor Control.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescLoadCtrl(enetDescPtr,val)                                   \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descCtrl = (uint32)(val);                                \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescSetCtrl()
//
// SYNOPSIS       void EnetDescSetCtrl ( enetDesc *enetDescPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets Ethernet Descriptor Control bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescSetCtrl(enetDescPtr,bits)                                   \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descCtrl |= (bits);                                      \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescClrCtrl()
//
// SYNOPSIS       void EnetDescClrCtrl ( enetDesc *enetDescPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 bits: Bits to be set
//                 
// OUTPUT         None
//
// DESCRIPTION    Clears Ethernet Descriptor Control bits.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescClrCtrl(enetDescPtr,bits)                                   \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descCtrl &= ~(bits);                                     \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescIsCtrl()
//
// SYNOPSIS       void EnetDescIsCtrl ( enetDesc *enetDescPtr, uint32 bits )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 bits: Bits to be checked
//                 
// OUTPUT         bool: Bits set status
//
// DESCRIPTION    Returns TRUE if requested Ethernet Descriptor Control bits.
//                are set.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescIsCtrl(enetDescPtr,bits)                                    \
    (((enetDescPtr)->descCtrl & (bits)) ? TRUE : FALSE)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescGetBuf1Ptr()
//
// SYNOPSIS       uint32 EnetDescGetBuf1Ptr ( enetDesc *enetDescPtr )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                
// OUTPUT         uint32: Ethernet Descriptor Buffer 1 Address Pointer value
//
// DESCRIPTION    Returns Ethernet Descriptor Buffer 1 Address Pointer value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescGetBuf1Ptr(enetDescPtr)                                     \
    ((enetDescPtr)->descBuf1Ptr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescLoadBuf1Ptr()
//
// SYNOPSIS       void EnetDescLoadBuf1Ptr ( enetDesc *enetDescPtr,
 //                                          uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into Ethernet Descriptor Buffer 1 Address 
//                Pointer.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescLoadBuf1Ptr(enetDescPtr,val)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descBuf1Ptr = (((uint32)(val) &                          \
                                   ENET_DESC_BUF_PTR_MASK) <<               \
                                  ENET_DESC_BUF_PTR_SHIFT);                 \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescGetBuf2Ptr()
//
// SYNOPSIS       uint32 EnetDescGetBuf2Ptr ( enetDesc *enetDescPtr )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                
// OUTPUT         uint32: Ethernet Descriptor Buffer 2 Address Pointer value
//
// DESCRIPTION    Returns Ethernet Descriptor Buffer 2 Address Pointer value.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescGetBuf2Ptr(enetDescPtr)                                     \
    ((enetDescPtr)->descBuf2Ptr)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescLoadBuf2Ptr()
//
// SYNOPSIS       void EnetDescLoadBuf2Ptr ( enetDesc *enetDescPtr,
 //                                          uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDesc *enetDescPtr: Base Pointer of the Ethernet 
//                                       descriptor
//                uint32 val: Value to be loaded
//                 
// OUTPUT         None
//
// DESCRIPTION    Loads value into Ethernet Descriptor Buffer 2 Address 
//                Pointer.
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescLoadBuf2Ptr(enetDescPtr,val)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    (enetDescPtr)->descBuf2Ptr = (((uint32)(val) &                          \
                                   ENET_DESC_BUF_PTR_MASK) <<               \
                                  ENET_DESC_BUF_PTR_SHIFT);                 \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescGetFrmLength()
//
// SYNOPSIS       void EnetDescGetFrmLength ( enetDescRegs *enetDescPtr )
//
// TYPE           Macro 
//
// INPUT          enetDescRegs *enetDescPtr: Base Pointer of the Ethernet
//                                           descriptor
//                 
// OUTPUT         uint32: Ethernet Descriptor Frame Length value
//
// DESCRIPTION    Gets value of Ethernet Descriptor Frame Length (Desc Frame 
//                Status).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescGetFrmLength(enetDescPtr)                                   \
    ( (EnetDescGetFrmStat(enetDescPtr) >> ENET_DESC_FRM_LENGTH_SHIFT) &     \
      ENET_DESC_FRM_LENGTH_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescGetBuf1Size()
//
// SYNOPSIS       void EnetDescGetBuf1Size ( enetDescRegs *enetDescPtr )
//
// TYPE           Macro 
//
// INPUT          enetDescRegs *enetDescPtr: Base Pointer of the Ethernet
//                                           descriptor
//                 
// OUTPUT         uint32: Ethernet Descriptor Buffer 1 Size
//
// DESCRIPTION    Gets value of Ethernet Descriptor Buffer 1 Size (Desc 
//                Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescGetBuf1Size(enetDescPtr)                                    \
    ( (EnetDescGetCtrl(enetDescPtr) >> ENET_DESC_BUF1_SIZE_SHIFT) &         \
      ENET_DESC_BUF1_SIZE_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescSetBuf1Size()
//
// SYNOPSIS       void EnetDesc SetBuf1Size ( enetDescRegs *enetDescPtr, 
//                                            uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDescRegs *enetDescPtr: Base Pointer of the 
//                                           Ethernet descriptor
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of Ethernet Descriptor Buffer 1 Size (Desc 
//                Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescSetBuf1Size(enetDescPtr,val)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetDescLoadCtrl ( enetDescPtr ,                                        \
        ( (EnetDescGetCtrl(enetDescPtr) &                                   \
           (~((uint32)ENET_DESC_BUF1_SIZE_MASK <<                           \
              ENET_DESC_BUF1_SIZE_SHIFT))) |                                \
          (((uint32)(val) & ENET_DESC_BUF1_SIZE_MASK) <<                    \
           ENET_DESC_BUF1_SIZE_SHIFT)) );                                   \
} while (0)




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescGetBuf2Size()
//
// SYNOPSIS       void EnetDescGetBuf2Size ( enetDescRegs *enetDescPtr )
//
// TYPE           Macro 
//
// INPUT          enetDescRegs *enetDescPtr: Base Pointer of the 
//                                           Ethernet descriptor
//                 
// OUTPUT         uint32: Ethernet Descriptor Buffer 2 Size
//
// DESCRIPTION    Gets value of Ethernet Descriptor Buffer 2 Size (Desc 
//                Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescGetBuf2Size(enetDescPtr)                                    \
    ( (EnetDescGetCtrl(enetDescPtr) >> ENET_DESC_BUF2_SIZE_SHIFT) &         \
       ENET_DESC_BUF2_SIZE_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           EnetDescSetBuf2Size()
//
// SYNOPSIS       void EnetDescSetBuf2Size ( enetDescRegs *enetDescPtr, 
//                                           uint32 val )
//
// TYPE           Macro 
//
// INPUT          enetDescRegs *enetDescPtr: Base Pointer of the 
//                                           Ethernet descriptor
//                uint32 val: Value to be Set
//                 
// OUTPUT         None
//
// DESCRIPTION    Sets value of Ethernet Descriptor Buffer 2 Size (Desc 
//                Control).
//
// NOTE           None.
//
/******************************************************************************/
#define EnetDescSetBuf2Size(enetDescPtr,val)                                \
/* do-while added for macro safety */                                       \
do                                                                          \
{                                                                           \
    EnetDescLoadCtrl ( enetDescPtr ,                                        \
        ( (EnetDescGetCtrl(enetDescPtr) &                                   \
           (~((uint32)ENET_DESC_BUF2_SIZE_MASK <<                           \
              ENET_DESC_BUF2_SIZE_SHIFT))) |                                \
          (((uint32)(val) & ENET_DESC_BUF2_SIZE_MASK) <<                    \
           ENET_DESC_BUF2_SIZE_SHIFT)) );                                   \
} while (0)


#endif /* ETHERNET_H */









