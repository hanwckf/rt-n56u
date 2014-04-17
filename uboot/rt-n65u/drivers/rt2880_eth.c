/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_RT2880_ETH)

#include <malloc.h>
#include <net.h>
#include <asm/addrspace.h>
#include <rt_mmap.h>

#undef DEBUG
#define BIT(x)              ((1 << x))

/* ====================================== */
#define GDM_DISPAD       BIT(18)
#define GDM_DISCRC       BIT(17)
#define GDM_STRPCRC      BIT(16)
//GDMA1 uni-cast frames destination port
#define GDM_UFRC_P_CPU     ((u32)(~(0x7 << 12)))
#define GDM_UFRC_P_GDMA1   (1 << 12)
#define GDM_UFRC_P_GDMA2   (2 << 12)
#define GDM_UFRC_P_ppe     (6 << 12)
#define GDM_UFRC_P_DROP    (7 << 12)
//GDMA1 broad-cast MAC address frames
#define GDM_BFRC_P_CPU     ((u32)(~(0x7 << 8)))
#define GDM_BFRC_P_GDMA1   (1 << 8)
#define GDM_BFRC_P_GDMA2   (2 << 8)
#define GDM_BFRC_P_PPE     (6 << 8)
#define GDM_BFRC_P_DROP    (7 << 8)
//GDMA1 multi-cast MAC address frames
#define GDM_MFRC_P_CPU     ((u32)(~(0x7 << 4)))
#define GDM_MFRC_P_GDMA1   (1 << 4)
#define GDM_MFRC_P_GDMA2   (2 << 4)
#define GDM_MFRC_P_PPE     (6 << 4)
#define GDM_MFRC_P_DROP    (7 << 4)
//GDMA1 other MAC address frames destination port
#define GDM_OFRC_P_CPU     ((u32)(~(0x7)))
#define GDM_OFRC_P_GDMA1   1
#define GDM_OFRC_P_GDMA2   2
#define GDM_OFRC_P_PPE     6
#define GDM_OFRC_P_DROP    7

#define PSE_RESET       BIT(0)

#define RST_DRX_IDX0      BIT(16)
#define RST_DTX_IDX0      BIT(0)

#define TX_WB_DDONE       BIT(6)
#define RX_DMA_BUSY       BIT(3)
#define TX_DMA_BUSY       BIT(1)
#define RX_DMA_EN         BIT(2)
#define TX_DMA_EN         BIT(0)

#define GP1_FRC_EN        BIT(15)
#define GP1_FC_TX         BIT(11)
#define GP1_FC_RX         BIT(10)
#define GP1_LNK_DWN       BIT(9)
#define GP1_AN_OK       BIT(8)

/*
 * FE_INT_STATUS
 */
#define CNT_PPE_AF       BIT(31)
#define CNT_GDM1_AF      BIT(29)
#define PSE_P1_FC        BIT(22)
#define PSE_P0_FC        BIT(21)
#define PSE_FQ_EMPTY     BIT(20)
#define GE1_STA_CHG      BIT(18)
#define TX_COHERENT      BIT(17)
#define RX_COHERENT      BIT(16)

#define TX_DONE_INT1     BIT(9)
#define TX_DONE_INT0     BIT(8)
#define RX_DONE_INT0     BIT(2)
#define TX_DLY_INT       BIT(1)
#define RX_DLY_INT       BIT(0)

/*
 * Ethernet chip registers.RT2880
 */
#if defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD) 

#define PDMA_RELATED		0x0800
/* 1. PDMA */
#define TX_BASE_PTR0            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x000)
#define TX_MAX_CNT0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x004)
#define TX_CTX_IDX0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x008)
#define TX_DTX_IDX0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x00C)

#define TX_BASE_PTR1            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x010)
#define TX_MAX_CNT1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x014)
#define TX_CTX_IDX1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x018)
#define TX_DTX_IDX1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x01C)

#define TX_BASE_PTR2            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x020)
#define TX_MAX_CNT2             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x024)
#define TX_CTX_IDX2             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x028)
#define TX_DTX_IDX2             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x02C)

#define TX_BASE_PTR3            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x030)
#define TX_MAX_CNT3             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x034)
#define TX_CTX_IDX3             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x038)
#define TX_DTX_IDX3             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x03C)

#define RX_BASE_PTR0            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x100)
#define RX_MAX_CNT0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x104)
#define RX_CALC_IDX0            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x108)
#define RX_DRX_IDX0             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x10C)

#define RX_BASE_PTR1            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x110)
#define RX_MAX_CNT1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x114)
#define RX_CALC_IDX1            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x118)
#define RX_DRX_IDX1             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x11C)

#define PDMA_INFO               (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x200)
#define PDMA_GLO_CFG            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x204)
#define PDMA_RST_IDX            (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x208)
#define PDMA_RST_CFG            (RALINK_FRAME_ENGINE_BASE + PDMA_RST_IDX)
#define DLY_INT_CFG             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x20C)
#define FREEQ_THRES             (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x210)
#define INT_STATUS              (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x220)
#define FE_INT_STATUS           (INT_STATUS)
#define INT_MASK                (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x228)
#define FE_INT_ENABLE           (INT_MASK)
#define PDMA_WRR                (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED+0x280)
#define PDMA_SCH_CFG            (PDMA_WRR)

#define SDM_RELATED		0x0C00
#define SDM_CON                 (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x00)  //Switch DMA configuration
#define SDM_RRING               (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x04)  //Switch DMA Rx Ring
#define SDM_TRING               (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x08)  //Switch DMA Tx Ring
#define SDM_MAC_ADRL            (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x0C)  //Switch MAC address LSB
#define SDM_MAC_ADRH            (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x10)  //Switch MAC Address MSB
#define SDM_TPCNT               (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x100) //Switch DMA Tx packet count
#define SDM_TBCNT               (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x104) //Switch DMA Tx byte count
#define SDM_RPCNT               (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x108) //Switch DMA rx packet count
#define SDM_RBCNT               (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x10C) //Switch DMA rx byte count
#define SDM_CS_ERR              (RALINK_FRAME_ENGINE_BASE + SDM_RELATED+0x110) //Switch DMA rx checksum error count

#else

#define MDIO_ACCESS         RALINK_FRAME_ENGINE_BASE + 0x00
#ifdef RT3883_USE_GE2
#define MDIO_CFG            RALINK_FRAME_ENGINE_BASE + 0x18
#else
#define MDIO_CFG            RALINK_FRAME_ENGINE_BASE + 0x04
#endif // RT3883_USE_GE2 //
#define FE_DMA_GLO_CFG      RALINK_FRAME_ENGINE_BASE + 0x08
#define FE_RST_GLO          RALINK_FRAME_ENGINE_BASE + 0x0C
#define FE_INT_STATUS       RALINK_FRAME_ENGINE_BASE + 0x10
#define FE_INT_ENABLE       RALINK_FRAME_ENGINE_BASE + 0x14
#define FC_DROP_STA         RALINK_FRAME_ENGINE_BASE + 0x18
#define FOE_TS_T            RALINK_FRAME_ENGINE_BASE + 0x1C

#define GDMA1_RELATED       0x0020
#define GDMA1_FWD_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x00)
#define GDMA1_SCH_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x04)
#define GDMA1_SHRP_CFG      (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x08)
#define GDMA1_MAC_ADRL      (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x0C)
#define GDMA1_MAC_ADRH      (RALINK_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x10)

#define GDMA2_RELATED       0x0060
#define GDMA2_FWD_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x00)
#define GDMA2_SCH_CFG       (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x04)
#define GDMA2_SHRP_CFG      (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x08)
#define GDMA2_MAC_ADRL      (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x0C)
#define GDMA2_MAC_ADRH      (RALINK_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x10)

#define PSE_RELATED         0x0040
#define PSE_FQFC_CFG        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x00)
#define CDMA_FC_CFG         (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x04)
#define GDMA1_FC_CFG        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x08)
#define GDMA2_FC_CFG        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x0C)
#define CDMA_OQ_STA         (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x10)
#define GDMA1_OQ_STA        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x14)
#define GDMA2_OQ_STA        (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x18)
#define PSE_IQ_STA          (RALINK_FRAME_ENGINE_BASE + PSE_RELATED + 0x1C)

#define CDMA_RELATED        0x0080
#define CDMA_CSG_CFG        (RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00)
#define CDMA_SCH_CFG        (RALINK_FRAME_ENGINE_BASE + CDMA_RELATED + 0x04)

#define PDMA_RELATED        0x0100
#define PDMA_GLO_CFG        (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x00)
#define PDMA_RST_IDX        (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x04)
#define PDMA_SCH_CFG        (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x08)
#define DELAY_INT_CFG       (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x0C)
#define TX_BASE_PTR0        (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x10)
#define TX_MAX_CNT0         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x14)
#define TX_CTX_IDX0         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x18)
#define TX_DTX_IDX0         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x1C)
#define TX_BASE_PTR1        (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x20)
#define TX_MAX_CNT1         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x24)
#define TX_CTX_IDX1         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x28)
#define TX_DTX_IDX1         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x2C)
#define RX_BASE_PTR0        (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x30)
#define RX_MAX_CNT0         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x34)
#define RX_CALC_IDX0        (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x38)
#define RX_DRX_IDX0         (RALINK_FRAME_ENGINE_BASE + PDMA_RELATED + 0x3C)


#endif


#define INTERNAL_LOOPBACK_ENABLE 1
#define INTERNAL_LOOPBACK_DISABLE 0

#define ESRAM_ON  1
#define ESRAM_OFF 0


/*****************************************/
//PCI define
#ifdef RALINK_PCI_HOST_TEST_FUN

//RTL8139.h..................................................
#define DelayCount				0X8000
#define RTL8139_DEVICE_VENDOR_ID		0x813910EC
#define PCI_BRIDGE_DEVICE_VENDOR_ID		0x0000eeee
#define RTL8139_VENDOR_ID			0x10EC
#define RTL8139_DEVICE_ID			0x8139
#define RTL8139_RX_BUFFER_LENGTH		(32 * 1024)
#define RX_MAX_PACKET_LENGTH			1518
#define RX_MIN_PACKET_LENGTH			60
#define PACKET_CRC_LENGTH			4
#define RTL8139_TX_BUF_NUM			0x100
#define PACKET_HDR_LENGTH			14

//#define PCI_BRIDGE_DMA_SIZE                         0x0       //power of 2 =>16(MB)
//*** PCI Bridge Definition Configuration Space & Value
#define PCI_BRIDGE_CONFIGURATION_SPACE_CONTROL      0x4C
#define PCI_BRIDGE_CONFIGURATION_INTABCD_STATUS     0x4F
#define PCI_BRIDGE_CONFIGURATION_INTMASK            0x4E
#define PCI_BRIDGE_CONFIGURATION_SPACE_MEM1_BA      0x80000050
#define PCI_BRIDGE_CONFIGURATION_SPACE_MEM2_BA      0x80000054
#define PCI_BRIDGE_CONFIGURATION_SPACE_MEM3_BA      0x80000058
#define PCI_ENABLE_INTA_INTB_INTC_INTD              0x03C00000
#define PCI_BRIDGE_MAX_INT_NUMBER                   0x04
#define PCI_DMA_MEM_REQUEST_FAIL                    0xFFFFFFFF
#define PCI_DMA_DEFAULT_SIZE                        0x00
#define PCI_MAX_SLOT_NUM                            0x04
#define PCI_BRIDGE_MAX_INT_NUMBER                   0x04
// PCI dev ID define
#define PCI_BRIDGE_ID                 0x4321159B
#define PCI_RTL8139_ID                0x813910EC
/*3.PCI configuration register */
#define PCI_CSH_VENDOR_ID_REG				0x00
#define PCI_CSH_DEVICE_ID_REG				0x02
#define PCI_CSH_COMMAND_REG				0x04
#define PCI_CSH_STATUS_REG				0x06
#define PCI_CSH_REVISION_CLASS_REG			0x08
#define PCI_CSH_CACHE_LINE_SIZE_REG			0x0C
#define PCI_CSH_LATENCY_TIMER_REG			0x0D
#define PCI_CSH_HEADER_TYPE_REG				0x0E
#define PCI_CSH_BIST_REG				0x0F
#define	PCI_CSH_BASE_ADDR_REG				0x10

/*7.Alignment Constants */
#define PCI_MEM_SPACE_ALIGNMENT				0x10
#define PCI_IO_SPACE_ALIGNMENT				0x4
#define SYS_DDR_SDRAM_BASE_ADDR				0x00000000
#define PCI_ALLOCATE_SPACE				0x20000000

/*5.PCI command status register bit mapping */
#define PCI_CMD_IO_ENABLE				0x00000001
#define PCI_CMD_MEM_ENABLE				0x00000002
#define PCI_CMD_BUS_MASTER_ENABLE			0x00000004
#define PCI_CMD_MEM_WRITE_INVALIDATE			0X00000010
#define PCI_CMD_PARITY_ERR				0x00000040
#define PCI_CMD_STEPPING_CONTROL			0x00000080
#define PCI_CMD_SERR_ENABLE				0x00000100
#define PCI_CMD_FBB_ENABLE				0x00000200

typedef struct
{
	u32	RegNum:8;
	u32	FunNum:3;
	u32	DevNum:5;
	u32	BusNum:8;
	u32	Always0:7;
	u32	Enable:1;
} PCIDeviceIDStruct;

struct _PCI_DEV
{
	u32       dev_ven;
	PCIDeviceIDStruct  PCIDeviceID;
};

//kaiker
#define RALINK_PCI_BAR0SETUP_ADDR *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x1010)
#define RALINK_PCI_BAR1SETUP_ADDR *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x1014)
#define RALINK_PCI_IMBASEBAR0_ADDR *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x1018)
#define RALINK_PCI_IMBASEBAR1_ADDR *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x101C)

struct _PCI_DEV Finded_PCIDeviceID[PCI_MAX_SLOT_NUM];

#endif // RT2880_ON_UBOOT_TEST_FUNC //



#define TOUT_LOOP   1000
#define ENABLE 1
#define DISABLE 0


VALID_BUFFER_STRUCT  rt2880_free_buf_list;
VALID_BUFFER_STRUCT  rt2880_busing_buf_list;
static BUFFER_ELEM   rt2880_free_buf[PKTBUFSRX];

/*=======================================*/

struct palmeth_desc {
	volatile s32 status;
	u32 des1;
	u32 buf;
	u32 next;
};

#if 1
/*=========================================
      PDMA RX Descriptor Format define
=========================================*/

//-------------------------------------------------
typedef struct _PDMA_RXD_INFO1_  PDMA_RXD_INFO1_T;

struct _PDMA_RXD_INFO1_
{
    unsigned int    PDP0;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO2_    PDMA_RXD_INFO2_T;

struct _PDMA_RXD_INFO2_
{
	unsigned int    PLEN1                   : 14;
	unsigned int    LS1                     : 1;
	unsigned int    UN_USED                 : 1;
	unsigned int    PLEN0                   : 14;
	unsigned int    LS0                     : 1;
	unsigned int    DDONE_bit               : 1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO3_  PDMA_RXD_INFO3_T;

struct _PDMA_RXD_INFO3_
{
	unsigned int    PDP1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO4_    PDMA_RXD_INFO4_T;

struct _PDMA_RXD_INFO4_
{
	unsigned int    FOE_Entry               : 14;
	unsigned int    FVLD                    : 1;
	unsigned int    UN_USE1                 : 1;
	unsigned int    AI                      : 8;
	unsigned int    SP                      : 3;
	unsigned int    AIS                     : 1;
	unsigned int    L4F                     : 1;
	unsigned int    IPF                     : 1;
	unsigned int    L4FVLD_bit              : 1;
	unsigned int    IPFVLD_bit              : 1;
};

struct PDMA_rxdesc {
	PDMA_RXD_INFO1_T rxd_info1;
	PDMA_RXD_INFO2_T rxd_info2;
	PDMA_RXD_INFO3_T rxd_info3;
	PDMA_RXD_INFO4_T rxd_info4;
};
#endif
/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO1_  PDMA_TXD_INFO1_T;

struct _PDMA_TXD_INFO1_
{
	unsigned int    SDP0;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO2_    PDMA_TXD_INFO2_T;

struct _PDMA_TXD_INFO2_
{
	unsigned int    SDL1                  : 14;
	unsigned int    LS1_bit               : 1;
	unsigned int    BURST_bit             : 1;
	unsigned int    SDL0                  : 14;
	unsigned int    LS0_bit               : 1;
	unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO3_  PDMA_TXD_INFO3_T;

struct _PDMA_TXD_INFO3_
{
	unsigned int    SDP1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO4_    PDMA_TXD_INFO4_T;

struct _PDMA_TXD_INFO4_
{
	unsigned int    VIDX                : 4;
	unsigned int    VPRI                : 3;
	unsigned int    INSV                : 1;
	unsigned int    SIDX                : 4;
	unsigned int    INSP                : 1;
	unsigned int    UN_USE3             : 3;
	unsigned int    QN                  : 3;
	unsigned int    UN_USE2             : 5;
	unsigned int    PN                  : 3;
	unsigned int    UN_USE1             : 2;
	unsigned int    TC0                 : 1;
	unsigned int    UC0_bit             : 1;
	unsigned int    IC0_bit             : 1;
};

struct PDMA_txdesc {
	PDMA_TXD_INFO1_T txd_info1;
	PDMA_TXD_INFO2_T txd_info2;
	PDMA_TXD_INFO3_T txd_info3;
	PDMA_TXD_INFO4_T txd_info4;
};



static int is_internal_loopback_test;


static  struct PDMA_txdesc tx_ring0_cache[NUM_TX_DESC] __attribute__ ((aligned(32))); /* TX descriptor ring         */
static  struct PDMA_rxdesc rx_ring_cache[NUM_RX_DESC] __attribute__ ((aligned(32))); /* RX descriptor ring         */

static int rx_dma_owner_idx0;                             /* Point to the next RXD DMA wants to use in RXD Ring#0.  */
static int rx_wants_alloc_idx0;                           /* Point to the next RXD CPU wants to allocate to RXD Ring #0. */
static int tx_cpu_owner_idx0;                             /* Point to the next TXD in TXD_Ring0 CPU wants to use */
static int tx_cpu_owner_idx1;
static volatile struct PDMA_rxdesc *rx_ring;
static volatile struct PDMA_txdesc *tx_ring0;
#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
static volatile struct PDMA_txdesc *tx_ring1;
static  struct PDMA_txdesc tx_ring1_cache[NUM_TX_DESC] __attribute__ ((aligned(32))); /* TX descriptor ring         */
#endif

static char rxRingSize;
static char txRingSize;

static int   rt2880_eth_init(struct eth_device* dev, bd_t* bis);
static int   rt2880_eth_send(struct eth_device* dev, volatile void *packet, int length);
static int   rt2880_eth_recv(struct eth_device* dev);
void  rt2880_eth_halt(struct eth_device* dev);
int   mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
int   mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);


static int   rt2880_eth_setup(struct eth_device* dev);
static int   rt2880_eth_initd;

static int eth_loopback_mode,loopback_protect;
static int force_queue_n;

static int sdp0_alig_16n_x;
static int sdp1_alig_16n_x;

/* RT3052 PHY TEST */
#ifdef RT3052_PHY_TEST

#define PHY_TEST_ENABLE 	1
#define PHY_TEST_DISABLE 	0
int rt3052_phy_test = PHY_TEST_DISABLE;
int rt3052_phy_test_debug = 0;
unsigned char rt3052_phy_test_buf[1520];
int rt3052_phy_test_ret_code;
int phy_init_setup = 0;
#define ETH_P_8021Q  0x8100

int rt3052_port_test_status = 0;	// rt3052 phy production test, intermediate result
void phy_link_detect();
void test_nop();
void packet_dump(unsigned char* packet, unsigned int length);
int phy_mdio_link_check(u32 phy_addr);
#endif
/* END OF RT3052 PHY TEST */


static int internal_loopback_test;
static int rt2880_esram_gear;
static int rt2880_size_of_mem;

static int header_payload_scatter_en;
static u32 rt2880_hdrlen;
static int rt2880_buf_in_esram_en;
static int rt2880_desc_in_esram;
static int rt2880_sdp0_buf_in_esram_en;
static int rt2880_debug_en;

extern char        console_buffer[CFG_CBSIZE];		/* console I/O buffer	*/


#define phys_to_bus(a) (a & 0x1FFFFFFF)

#define PCI_WAIT_INPUT_CHAR(ch) while((ch = getc())== 0)

struct eth_device* 	rt2880_pdev;

volatile uchar	*PKT_HEADER_Buf;// = (uchar *)CFG_EMBEDED_SRAM_SDP0_BUF_START;
static volatile uchar	PKT_HEADER_Buf_Pool[(PKTBUFSRX * PKTSIZE_ALIGN) + PKTALIGN];
#ifdef RALINK_GDMA_SCATTER_TEST_FUN
static volatile uchar	*pkthdrbuf[PKTBUFSRX];
#endif
extern volatile uchar	*NetTxPacket;	/* THE transmit packet			*/
extern volatile uchar	*PktBuf;
extern volatile uchar	Pkt_Buf_Pool[];

extern int rtl8367_switch_init_post(void);

#define PIODIR_R  (RALINK_PIO_BASE + 0X24)
#define PIODATA_R (RALINK_PIO_BASE + 0X20)
#define PIODIR3924_R  (RALINK_PIO_BASE + 0x4c)
#define PIODATA3924_R (RALINK_PIO_BASE + 0x48)


#define FREEBUF_OFFSET(CURR)  ((int)(((0x0FFFFFFF & (u32)CURR) - (u32) (0x0FFFFFFF & (u32) rt2880_free_buf[0].pbuf)) / 1536))



void START_ETH(struct eth_device *dev ) {
	s32 omr;
	omr=RALINK_REG(PDMA_GLO_CFG);
	udelay(100);
	if(is_internal_loopback_test)
	{
	omr |= TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN ;
		omr &= ~RX_DMA_EN;
		printf("\n Interloopback test! So RxDMA is Stop !  \n");
	}
	else
	{
		omr |= TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN ;
	}
		
	RALINK_REG(PDMA_GLO_CFG)=omr;
	udelay(500);
}


void STOP_ETH(struct eth_device *dev)
{
	s32 omr;
	omr=RALINK_REG(PDMA_GLO_CFG);
	udelay(100);
	omr &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN) ;
	RALINK_REG(PDMA_GLO_CFG)=omr;
	udelay(500);
}


BUFFER_ELEM *rt2880_free_buf_entry_dequeue(VALID_BUFFER_STRUCT *hdr)
{
	int     zero = 0;           /* causes most compilers to place this */
	/* value in a register only once */
	BUFFER_ELEM  *node;

	/* Make sure we were not passed a null pointer. */
	if (!hdr) {
		return (NULL);
	}

	/* If there is a node in the list we want to remove it. */
	if (hdr->head) {
		/* Get the node to be removed */
		node = hdr->head;

		/* Make the hdr point the second node in the list */
		hdr->head = node->next;

		/* If this is the last node the headers tail pointer needs to be nulled
		   We do not need to clear the node's next since it is already null */
		if (!(hdr->head)) {
			hdr->tail = (BUFFER_ELEM *)zero;
		}

		node->next = (BUFFER_ELEM *)zero;




	}
	else {
		node = NULL;
		return (node);
	}

	/*  Restore the previous interrupt lockout level.  */

	/* Return a pointer to the removed node */

	//shnat_validation_flow_table_entry[node->index].state = SHNAT_FLOW_TABLE_NODE_USED;
	return (node);
}

static BUFFER_ELEM *rt2880_free_buf_entry_enqueue(VALID_BUFFER_STRUCT *hdr, BUFFER_ELEM *item)
{
	int zero =0;

	if (!hdr) {
		return (NULL);
	}

	if (item != NULL)
	{
		/* Temporarily lockout interrupts to protect global buffer variables. */
		// Sys_Interrupt_Disable_Save_Flags(&cpsr_flags);

		/* Set node's next to point at NULL */
		item->next = (BUFFER_ELEM *)zero;

		/*  If there is currently a node in the linked list, we want to add the
		    new node to the end. */
		if (hdr->head) {
			/* Make the last node's next point to the new node. */
			hdr->tail->next = item;

			/* Make the roots tail point to the new node */
			hdr->tail = item;
		}
		else {
			/* If the linked list was empty, we want both the root's head and
			   tial to point to the new node. */
			hdr->head = item;
			hdr->tail = item;
		}

		/*  Restore the previous interrupt lockout level.  */

	}
	else
	{
		printf("\n shnat_flow_table_free_entry_enqueue is called,item== NULL \n");
	}

	return(item);

} /* MEM_Buffer_Enqueue */


int rt2880_eth_initialize(bd_t *bis)
{
	struct	eth_device* 	dev;
	int	i;
	u32	regValue;

	if (!(dev = (struct eth_device *) malloc (sizeof *dev))) {
		printf("Failed to allocate memory\n");
		return 0;
	}

	memset(dev, 0, sizeof(*dev));

	sprintf(dev->name, "Eth0 (10/100-M)");

	dev->iobase = RALINK_FRAME_ENGINE_BASE;
	dev->init   = rt2880_eth_init;
	dev->halt   = rt2880_eth_halt;
	dev->send   = rt2880_eth_send;
	dev->recv   = rt2880_eth_recv;

	eth_register(dev);
	eth_loopback_mode = 0;
	rt2880_pdev = dev;
	loopback_protect = 0;

	force_queue_n = 3;
	sdp0_alig_16n_x = 0;
	sdp1_alig_16n_x = 0;
	rt2880_eth_initd =0;
	rt2880_size_of_mem = 0;
	rt2880_esram_gear = ESRAM_OFF;
	internal_loopback_test = INTERNAL_LOOPBACK_DISABLE;
	header_payload_scatter_en = DISABLE;
	rt2880_buf_in_esram_en = DISABLE;
	rt2880_desc_in_esram = DISABLE;
	rt2880_sdp0_buf_in_esram_en = DISABLE;
	PktBuf = Pkt_Buf_Pool;
	PKT_HEADER_Buf = PKT_HEADER_Buf_Pool;
	is_internal_loopback_test = 0;
	rt2880_hdrlen = 20;
	NetTxPacket = NULL;
	rt2880_debug_en = DISABLE;
	rx_ring = (struct PDMA_rxdesc *)KSEG1ADDR((ulong)&rx_ring_cache[0]);
	tx_ring0 = (struct PDMA_txdesc *)KSEG1ADDR((ulong)&tx_ring0_cache[0]);

	rt2880_free_buf_list.head = NULL;
	rt2880_free_buf_list.tail = NULL;

	rt2880_busing_buf_list.head = NULL;
	rt2880_busing_buf_list.tail = NULL;

	//2880_free_buf

	/*
	 *	Setup packet buffers, aligned correctly.
	 */
	rt2880_free_buf[0].pbuf = (unsigned char *)(&PktBuf[0] + (PKTALIGN - 1));
	rt2880_free_buf[0].pbuf -= (ulong)rt2880_free_buf[0].pbuf % PKTALIGN;
	rt2880_free_buf[0].next = NULL;

	rt2880_free_buf_entry_enqueue(&rt2880_free_buf_list,&rt2880_free_buf[0]);

#ifdef DEBUG
	printf("\n rt2880_free_buf[0].pbuf = 0x%08X \n",rt2880_free_buf[0].pbuf);
#endif
	for (i = 1; i < PKTBUFSRX; i++) {
		rt2880_free_buf[i].pbuf = rt2880_free_buf[0].pbuf + (i)*PKTSIZE_ALIGN;
		rt2880_free_buf[i].next = NULL;
#ifdef DEBUG
		printf("\n rt2880_free_buf[%d].pbuf = 0x%08X\n",i,rt2880_free_buf[i].pbuf);
#endif
		rt2880_free_buf_entry_enqueue(&rt2880_free_buf_list,&rt2880_free_buf[i]);
	}

	for (i = 0; i < PKTBUFSRX; i++)
	{
		rt2880_free_buf[i].tx_idx = NUM_TX_DESC;
#ifdef DEBUG
		printf("\n rt2880_free_buf[%d] = 0x%08X,rt2880_free_buf[%d].next=0x%08X \n",i,&rt2880_free_buf[i],i,rt2880_free_buf[i].next);
#endif
	}
		
	
	//set clock resolution
	extern unsigned long mips_bus_feq;
	regValue = le32_to_cpu(*(volatile u_long *)(RALINK_FRAME_ENGINE_BASE + 0x0008));
	regValue |=  ((mips_bus_feq/1000000) << 8);
	*((volatile u_long *)(RALINK_FRAME_ENGINE_BASE + 0x0008)) = cpu_to_le32(regValue);
	
	return 1;
}


static int rt2880_eth_init(struct eth_device* dev, bd_t* bis)
{
	if(rt2880_eth_initd == 0)
	{
		rt2880_eth_setup(dev);
	}
	else
	{
		START_ETH(dev);
	}

	rt2880_eth_initd = 1;
	return (1);
}

void LANWANPartition(void)
{
#ifdef MAC_TO_100SW_MODE
	int sw_id = 0;
	mii_mgr_read(29, 31, &sw_id);
#ifdef RALINK_DEMO_BOARD_PVLAN
	if (sw_id == 0x175c) {
		//disable tagged VLAN
		mii_mgr_write(29, 23, 0);

		//WLLLL, wan at P0, demo board
		mii_mgr_write(29, 19, 0x809c);
		mii_mgr_write(29, 20, 0x9a96);
		mii_mgr_write(29, 21, 0x8e00);
		mii_mgr_write(29, 22, 0x8420);
	}
	else {
		mii_mgr_write(20, 13, 0x21);
		mii_mgr_write(22, 14, 0x2002);
		mii_mgr_write(22, 15, 0x1001);
		mii_mgr_write(22, 16, 0x1001);
		mii_mgr_write(22, 17, 0x1001);
		mii_mgr_write(22, 18, 0x1001);
		mii_mgr_write(22, 19, 0x1001);
		mii_mgr_write(23, 0, 0x3e21);
		mii_mgr_write(23, 1, 0x3e3e);
		mii_mgr_write(23, 2, 0x3e3e);
		mii_mgr_write(23, 16, 0x3f3f);
		mii_mgr_write(23, 17, 0x3f3f);
		mii_mgr_write(23, 18, 0x3f3f);
	}
#endif
#ifdef RALINK_EV_BOARD_PVLAN
	if (sw_id == 0x175c) {
		//disable tagged VLAN
		mii_mgr_write(29, 23, 0);

		//LLLLW, wan at P4, ev board
		mii_mgr_write(29, 19, 0x8e8d);
		mii_mgr_write(29, 20, 0x8b87);
		mii_mgr_write(29, 21, 0x8000);
		mii_mgr_write(29, 22, 0x8420);
	}
	else {
		mii_mgr_write(20, 13, 0x21);
		mii_mgr_write(22, 14, 0x1001);
		mii_mgr_write(22, 15, 0x1001);
		mii_mgr_write(22, 16, 0x1001);
		mii_mgr_write(22, 17, 0x1001);
		mii_mgr_write(22, 18, 0x2002);
		mii_mgr_write(22, 19, 0x1001);
		mii_mgr_write(23, 0, 0x2f2f);
		mii_mgr_write(23, 1, 0x2f2f);
		mii_mgr_write(23, 2, 0x2f30);
		mii_mgr_write(23, 16, 0x3f3f);
		mii_mgr_write(23, 17, 0x3f3f);
		mii_mgr_write(23, 18, 0x3f3f);
	}
#endif
#endif // MAC_TO_100SW_MODE //

#if defined (RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD) || \
    defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) || \
    defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)
//	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x14)) = 0x405555; //enable VLANa
//	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x50)) = 0x2001; //VLAN id
//	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x98)) = 0x7f7f; //remove VLAN tag
#ifdef RALINK_DEMO_BOARD_PVLAN
	//WLLLL, wan at P0, demo board
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x40)) = 0x1002; //PVID
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x44)) = 0x1001; //PVID
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x48)) = 0x1001; //PVID
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x70)) = 0xffff417e; //VLAN member
#endif
#ifdef RALINK_EV_BOARD_PVLAN
	//LLLLW, wan at P4, ev board
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x40)) = 0x1001; //PVID
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x44)) = 0x1001; //PVID
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x48)) = 0x1002; //PVID
	*((volatile u32 *)(RALINK_ETH_SW_BASE + 0x70)) = 0xffff506f; //VLAN member
#endif
#endif // (RT3052_ASIC_BOARD || RT3052_FPGA_BOARD || RT3352_ASIC_BOARD || RT3352_FPGA_BOARD)
}

#if defined (P5_RGMII_TO_MAC_MODE) || defined (MAC_TO_VITESSE_MODE)
static void ResetSWusingGPIOx(void)
{
#ifdef GPIOx_RESET_MODE
	u32 value;

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)

	printf("\n GPIO pin 10 reset to switch\n");

	//set spi/gpio share pin to gpio mode
	value = le32_to_cpu(*(volatile u_long *)RT2880_GPIOMODE_REG);
	value |= (1 << 1);
	*(volatile u_long *)(RT2880_GPIOMODE_REG) = cpu_to_le32(value);

	//Set Gpio pin 10 to output
	value = le32_to_cpu(*(volatile u_long *)PIODIR_R);
	value |= (1 << 10);
	*(volatile u_long *)(PIODIR_R) = cpu_to_le32(value);

	//Set Gpio pin 10 to low
	value = le32_to_cpu(*(volatile u_long *)PIODATA_R);
	value &= ~(1 << 10);
	*(volatile u_long *)(PIODATA_R) = cpu_to_le32(value);
	
	udelay(50000);
	//Set Gpio pin 10 to high
	value = le32_to_cpu(*(volatile u_long *)PIODATA_R);
	value |= (1 << 10);
	*(volatile u_long *)(PIODATA_R) = cpu_to_le32(value);

#elif defined (RT2883_FPGA_BOARD) || defined (RT2883_ASIC_BOARD)
	printf("\n GPIO pin 12 reset to switch\n");

	//Set UARTF_SHARED_MODE to 3'b111 bcs we need gpio 12, and SPI to normal mode
	value = le32_to_cpu(*(volatile u_long *)RT2880_GPIOMODE_REG);
	value |= (7 << 2);
	value &= ~(1 << 1);
	*(volatile u_long *)(RT2880_GPIOMODE_REG) = cpu_to_le32(value);

	//Set Gpio pin 12 to output, and pin 7(RTS) to input
	value = le32_to_cpu(*(volatile u_long *)PIODIR_R);
	value |= (1 << 12);
	value &= ~(1 << 7);
	*(volatile u_long *)(PIODIR_R) = cpu_to_le32(value);

	//Set Gpio pin 12 to low
	value = le32_to_cpu(*(volatile u_long *)PIODATA_R);
	value &= ~(1 << 12);
	*(volatile u_long *)(PIODATA_R) = cpu_to_le32(value);
	
	udelay(50000);
	//Set Gpio pin 12 to high
	value = le32_to_cpu(*(volatile u_long *)PIODATA_R);
	value |= (1 << 12);
	*(volatile u_long *)(PIODATA_R) = cpu_to_le32(value);

#elif defined (RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD) 
	printf("\n GPIO pin 36 reset to switch\n");

	//Set UARTF_SHARED_MODE to 3'b111 bcs we need gpio 36, and SPI to normal mode
	value = le32_to_cpu(*(volatile u_long *)RT2880_GPIOMODE_REG);
	value |= (7 << 2);
	value &= ~(1 << 1);
	*(volatile u_long *)(RT2880_GPIOMODE_REG) = cpu_to_le32(value);

	//Set Gpio pin 36 to output
	value = le32_to_cpu(*(volatile u_long *)0xb000064c);
	value |= (1 << 12);
	*(volatile u_long *)(0xb000064c) = cpu_to_le32(value);

	//Set Gpio pin 36 to low
	value = le32_to_cpu(*(volatile u_long *)0xb0000648);
	value &= ~(1 << 12);
	*(volatile u_long *)(0xb0000648) = cpu_to_le32(value);
	
	udelay(50000);
	//Set Gpio pin 36 to high
	value = le32_to_cpu(*(volatile u_long *)0xb0000648);
	value |= (1 << 12);
	*(volatile u_long *)(0xb0000648) = cpu_to_le32(value);

#elif defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) 
	printf("\n Please FIXME... \n");

#elif defined (RT3883_ASIC_BOARD)
	printf("\n GPIO pin 24 reset to switch\n");

	//Set Gpio pin 24 to output
	value = le32_to_cpu(*(volatile u_long *)PIODIR3924_R);
	value |= 1;
	*(volatile u_long *)(PIODIR3924_R) = cpu_to_le32(value);

	//Set Gpio pin 24 to low
	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	value &= ~1;
	*(volatile u_long *)(PIODATA3924_R) = cpu_to_le32(value);

	udelay(50000);
	//Set Gpio pin 24 to high
	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	value |= 1;
	*(volatile u_long *)(PIODATA3924_R) = cpu_to_le32(value);
#else
#error "Unknown Chip"
#endif
#endif // GPIOx_RESET_MODE //
}
#endif

#if defined (MAC_TO_GIGAPHY_MODE) || defined (P5_MAC_TO_PHY_MODE) 
#define EV_MARVELL_PHY_ID0 0x0141
#define EV_MARVELL_PHY_ID1 0x0CC2
static int isMarvellGigaPHY(void)
{
	u32 phy_id0,phy_id1;

	if( ! mii_mgr_read(MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)){
		printf("\n Read PhyID 0 is Fail!!\n");
		phy_id0 =0;
	}

	if( ! mii_mgr_read(MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)){
		printf("\n Read PhyID 1 is Fail!!\n");
		phy_id1 = 0;
	}

	if((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1))
		return 1;

	return 0;
}

#define EV_VTSS_PHY_ID0 0x0007
#define EV_VTSS_PHY_ID1 0x0421
static int isVtssGigaPHY(void)
{
	u32 phy_id0,phy_id1;

	if( ! mii_mgr_read(MAC_TO_GIGAPHY_MODE_ADDR, 2, &phy_id0)){
		printf("\n Read PhyID 0 is Fail!!\n");
		phy_id0 =0;
	}

	if( ! mii_mgr_read(MAC_TO_GIGAPHY_MODE_ADDR, 3, &phy_id1)){
		printf("\n Read PhyID 1 is Fail!!\n");
		phy_id1 = 0;
	}

	if((phy_id0 == EV_VTSS_PHY_ID0) && (phy_id1 == EV_VTSS_PHY_ID1))
		return 1;

	return 0;
}

#endif // MAC_TO_GIGAPHY_MODE || P5_MAC_TO_PHY_MODE //

#if defined (MAC_TO_GIGAPHY_MODE) || defined (P5_MAC_TO_PHY_MODE) || defined (MAC_TO_100PHY_MODE)

void enable_auto_negotiate(void)
{
	u32 regValue;
	u32 addr = MAC_TO_GIGAPHY_MODE_ADDR;	// define in config.mk

#if defined (RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD) || \
    defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD)
	regValue = le32_to_cpu(*(volatile u_long *)(0xb01100C8));
#else
	regValue = RALINK_REG(MDIO_CFG);
#endif

	regValue &= 0xe0ff7fff;				// clear auto polling related field:
							// (MD_PHY1ADDR & GP1_FRC_EN).
	regValue |= 0x20000000;				// force to enable MDC/MDIO auto polling.
	regValue |= (addr << 24);			// setup PHY address for auto polling.

#if defined (RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD) || \
    defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD)
	*(volatile u_long *)(0xb01100C8) = cpu_to_le32(regValue);
#else
	RALINK_REG(MDIO_CFG) = cpu_to_le32(regValue);
#endif

}

#endif // defined (MAC_TO_GIGAPHY_MODE) || defined (P5_MAC_TO_PHY_MODE) || defined (MAC_TO_100PHY_MODE) //

int isDMABusy(struct eth_device* dev)
{
	u32 kk;

	kk = RALINK_REG(PDMA_GLO_CFG);

	if((kk & RX_DMA_BUSY)){
		return 1;
	}

	if((kk & TX_DMA_BUSY)){
		printf("\n  TX_DMA_BUSY !!! ");
		return 1;
	}
	return 0;
}

#if defined (RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD) || \
    defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) || \
    defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)
void rt305x_esw_init(void)
{
	u32	i;
	u32	phy_val=0, phy_val2=0;

	/*                                                                               
	 * FC_RLS_TH=200, FC_SET_TH=160
	 * DROP_RLS=120, DROP_SET_TH=80
	 */
	RALINK_REG(0xb0110008) = 0xC8A07850;       
	RALINK_REG(0xb01100E4) = 0x00000000;
	RALINK_REG(0xb0110014) = 0x00405555;
	RALINK_REG(0xb0110090) = 0x00007f7f;
	RALINK_REG(0xb0110098) = 0x00007f7f; //disable VLAN
	RALINK_REG(0xb01100CC) = 0x0002500c;
	RALINK_REG(0xb011009C) = 0x0008a301; //hashing algorithm=XOR48, aging interval=300sec
	RALINK_REG(0xb011008C) = 0x02404040; 

#if defined (RT3052_ASIC_BOARD) || defined (RT3352_ASIC_BOARD) || defined (RT5350_ASIC_BOARD)
	RALINK_REG(0xb01100C8) = 0x3f502b28; //Ext PHY Addr=0x1F 
	RALINK_REG(0xb0110084) = 0x00000000;
	RALINK_REG(0xb0110110) = 0x7d000000; //1us cycle number=125 (FE's clock=125Mhz)
#elif defined (RT3052_FPGA_BOARD) || defined (RT3352_FPGA_BOARD) || defined (RT5350_FPGA_BOARD)
	RALINK_REG(0xb01100C8) = 0x20f02b28; //Ext PHY Addr=0x0 
	RALINK_REG(0xb0110084) = 0xffdf1f00;
	RALINK_REG(0xb0110110) = 0x0d000000; //1us cycle number=13 (FE's clock=12.5Mhz)

	/* In order to use 10M/Full on FPGA board. We configure phy capable to 
	 * 10M Full/Half duplex, so we can use auto-negotiation on PC side */
	for(i=0;i<5;i++){
	    mii_mgr_write(i, 4, 0x0461);   //Capable of 10M Full/Half Duplex, flow control on/off
	    mii_mgr_write(i, 0, 0xB100);   //reset all digital logic, except phy_reg
	}
#endif

#if defined (P5_RGMII_TO_MAC_MODE)
	RALINK_REG(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
	RALINK_REG(0xb01100C8) &= ~(1<<29); //disable port 5 auto-polling
	RALINK_REG(0xb01100C8) |= 0x3fff; //force 1000M full duplex
	RALINK_REG(0xb01100C8) &= ~(0xf<<20); //rxclk_skew, txclk_skew = 0
#elif defined (P5_MII_TO_MAC_MODE)
	RALINK_REG(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
	RALINK_REG(0xb01100C8) &= ~(1<<29); //disable port 5 auto-polling
	RALINK_REG(0xb01100C8) &= ~(0x3fff);
	RALINK_REG(0xb01100C8) |= 0x3ffd; //force 100M full duplex
#elif defined (P5_MAC_TO_PHY_MODE)
	RALINK_REG(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
	RALINK_REG(0xb0000060) &= ~(1 << 7); //set MDIO to Normal mode
	enable_auto_negotiate();
	if (isMarvellGigaPHY()) {
		printf("\n MARVELL Phy\n");
		mii_mgr_write(MAC_TO_GIGAPHY_MODE_ADDR, 20, 0x0ce0);
#if defined (RT3052_FPGA_BOARD) || defined(RT3352_FPGA_BOARD)
		mii_mgr_read(MAC_TO_GIGAPHY_MODE_ADDR, 9, &phy_val);
		phy_val &= ~(3<<8); //turn off 1000Base-T Advertisement
		mii_mgr_write(MAC_TO_GIGAPHY_MODE_ADDR, 9, phy_val);
#endif
		mii_mgr_write(MAC_TO_GIGAPHY_MODE_ADDR, 0, 0x9140);
	}
	if (isVtssGigaPHY()) {
		mii_mgr_write(MAC_TO_GIGAPHY_MODE_ADDR, 31, 0x0001); //extended page
		mii_mgr_read(MAC_TO_GIGAPHY_MODE_ADDR, 28, &phy_val);
		printf("GE1 Vitesse Phy reg28 %x --> ",phy_val);
		phy_val |= (0x3<<12); // RGMII RX skew compensation= 2.0 ns
		phy_val &= ~(0x3<<14); // RGMII TX skew compensation= 0 ns
		printf("%x (without reset PHY)\n", phy_val);
		mii_mgr_write(MAC_TO_GIGAPHY_MODE_ADDR, 28, phy_val);
		mii_mgr_write(MAC_TO_GIGAPHY_MODE_ADDR, 31, 0); //main registers
	}
#elif defined (P5_RMII_TO_MAC_MODE)
	/* Reserved */
	RALINK_REG(0xb0000060) &= ~(1 << 9); //set RGMII to Normal mode
	RALINK_REG(0xb01100C8) &= ~(1<<29); //disable port 5 auto-polling
	RALINK_REG(0xb01100C8) &= ~(0x3fff);
	RALINK_REG(0xb01100C8) |= 0x3ffd; //force 100M full duplex

#else /* Port 5 disabled */
	RALINK_REG(0xb01100C8) &= ~(1 << 29); //port5 auto polling disable
	RALINK_REG(0xb0000060) |= (1 << 9); //set RGMII to GPIO mode (GPIO41-GPIO50)
	RALINK_REG(0xb0000674) = 0xFDF; //GPIO41-GPIO50 output mode, for RT3352 GPIO40-GPIO45 will set GPIO45 as input mode 
	RALINK_REG(0xb0000670) = 0x0; //GPIO41-GPIO50 output low
#endif // P5_RGMII_TO_MAC_MODE //

#define RSTCTRL_EPHY_RST	(1<<24)
	/* We shall prevent modifying PHY registers if it is FPGA mode */
#if defined (RT3052_ASIC_BOARD) || defined (RT3352_ASIC_BOARD) || defined (RT5350_ASIC_BOARD)
#if defined (RT3052_ASIC_BOARD)

	rw_rf_reg(0, 0, &phy_val);
	phy_val = phy_val >> 4;

	if(phy_val > 0x5) {
	    
	    rw_rf_reg(0, 26, &phy_val);
	    phy_val2 = (phy_val | (0x3 << 5));
	    rw_rf_reg(1, 26, &phy_val2);

	    // reset phy
	    i = RALINK_REG(RT2880_RSTCTRL_REG);
	    i = i | RSTCTRL_EPHY_RST;
	    RALINK_REG(RT2880_RSTCTRL_REG)= i;
	    i = i & ~(RSTCTRL_EPHY_RST);
	    RALINK_REG(RT2880_RSTCTRL_REG)= i;

	    rw_rf_reg(1, 26, &phy_val);

	    //select local register
	    mii_mgr_write(0, 31, 0x8000);
	    for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
		mii_mgr_write(i, 29, 0x7015);   //TX100/TX10 AD/DA current bias
		mii_mgr_write(i, 30, 0x0038);   //TX100 slew rate control
	    }

	    //select global register
	    mii_mgr_write(0, 31, 0x0);
	    mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	    mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	    mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	    mii_mgr_write(0, 12, 0x7eaa);
	    mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	    mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	    mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	    mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	    mii_mgr_write(0, 22, 0x252f); //tune TP_IDL tail and head waveform, enable power down slew rate control
	    mii_mgr_write(0, 27, 0x2fda); //set PLL/Receive bias current are calibrated
	    mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	    mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	    mii_mgr_write(0, 31, 0x8000); //select local register


	    for(i=0;i<5;i++){
		//LSB=1 enable PHY
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	    }

	} else {
	    //select local register
	    mii_mgr_write(0, 31, 0x8000);
	    for(i=0;i<5;i++){
		mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
		mii_mgr_write(i, 29, 0x7058);   //TX100/TX10 AD/DA current bias
		mii_mgr_write(i, 30, 0x0018);   //TX100 slew rate control
	    }
	    
	    //select global register
	    mii_mgr_write(0, 31, 0x0);
	    mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	    mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	    mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	    mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	    mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	    mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	    mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	    mii_mgr_write(0, 22, 0x052f); //tune TP_IDL tail and head waveform
	    mii_mgr_write(0, 27, 0x2fce); //set PLL/Receive bias current are calibrated
	    mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	    mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	    mii_mgr_write(0, 31, 0x8000); //select local register

	    for(i=0;i<5;i++){
		//LSB=1 enable PHY
		mii_mgr_read(i, 26, &phy_val);
		phy_val |= 0x0001;
		mii_mgr_write(i, 26, phy_val);
	    }
	}

#elif defined (RT3352_ASIC_BOARD)
        //PHY IOT
    // reset phy
    i = RALINK_REG(RT2880_RSTCTRL_REG);
    i = i | RSTCTRL_EPHY_RST;
    RALINK_REG(RT2880_RSTCTRL_REG) = i;
    i = i & ~(RSTCTRL_EPHY_RST);
    RALINK_REG(RT2880_RSTCTRL_REG) = i;

	//select local register
	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
	    mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
	    mii_mgr_write(i, 29, 0x7016);   //TX100/TX10 AD/DA current bias
	    mii_mgr_write(i, 30, 0x0038);   //TX100 slew rate control
	}

	//select global register
	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	mii_mgr_write(0, 22, 0x253f); //tune TP_IDL tail and head waveform, enable power down slew rate control
	mii_mgr_write(0, 27, 0x2fda); //set PLL/Receive bias current are calibrated
	mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	mii_mgr_write(0, 31, 0x8000); //select local register

	for(i=0;i<5;i++){
	    //LSB=1 enable PHY
	    mii_mgr_read(i, 26, &phy_val);
	    phy_val |= 0x0001;
	    mii_mgr_write(i, 26, phy_val);
	}
#elif defined (RT5350_ASIC_BOARD)
        //PHY IOT
    // reset phy
    i = RALINK_REG(RT2880_RSTCTRL_REG);
    i = i | RSTCTRL_EPHY_RST;
    RALINK_REG(RT2880_RSTCTRL_REG) = i;
    i = i & ~(RSTCTRL_EPHY_RST);
    RALINK_REG(RT2880_RSTCTRL_REG) = i;

	//select local register
	mii_mgr_write(0, 31, 0x8000);
	for(i=0;i<5;i++){
	    mii_mgr_write(i, 26, 0x1600);   //TX10 waveform coefficient //LSB=0 disable PHY
	    mii_mgr_write(i, 29, 0x7015);   //TX100/TX10 AD/DA current bias
	    mii_mgr_write(i, 30, 0x0038);   //TX100 slew rate control
	}

	//select global register
	mii_mgr_write(0, 31, 0x0);
	mii_mgr_write(0,  1, 0x4a40); //enlarge agcsel threshold 3 and threshold 2
	mii_mgr_write(0,  2, 0x6254); //enlarge agcsel threshold 5 and threshold 4
	mii_mgr_write(0,  3, 0xa17f); //enlarge agcsel threshold 6
	mii_mgr_write(0, 12, 0x7eaa);
	mii_mgr_write(0, 14, 0x65);   //longer TP_IDL tail length
	mii_mgr_write(0, 16, 0x0684); //increased squelch pulse count threshold.
	mii_mgr_write(0, 17, 0x0fe0); //set TX10 signal amplitude threshold to minimum
	mii_mgr_write(0, 18, 0x40ba); //set squelch amplitude to higher threshold
	mii_mgr_write(0, 22, 0x253f); //tune TP_IDL tail and head waveform, enable power down slew rate control
	mii_mgr_write(0, 27, 0x2fda); //set PLL/Receive bias current are calibrated
	mii_mgr_write(0, 28, 0xc410); //change PLL/Receive bias current to internal(RT3350)
	mii_mgr_write(0, 29, 0x598b); //change PLL bias current to internal(RT3052_MP3)
	mii_mgr_write(0, 31, 0x8000); //select local register

	for(i=0;i<5;i++){
	    //LSB=1 enable PHY
	    mii_mgr_read(i, 26, &phy_val);
	    phy_val |= 0x0001;
	    mii_mgr_write(i, 26, phy_val);
	}
#else
#error "Chip is not supported"
#endif // RT3052_ASIC_BOARD //
#endif // RT3052_ASIC_BOARD || RT3352_ASIC_BOARD //

}
#endif

static int rt2880_eth_setup(struct eth_device* dev)
{
	u32	i;
	u32	regValue;
	u16	wTmp;
	uchar	*temp;

	printf("\n Waitting for RX_DMA_BUSY status Start... ");
	while(1)
		if(!isDMABusy(dev))
			break;
	printf("done\n\n");

	// set GE1 and GE2 to RGMII
	regValue = RALINK_REG(RT2880_SYSCFG1_REG);
	regValue &= ~(0xF << 12);
	RALINK_REG(RT2880_SYSCFG1_REG)=regValue;

	// set MDIO mode to match switch IC
	printf("set MDIO_CFG as MAC_FORCE, SPD 1000M, FULL_DUPLEX\n");

	// flow control enabled
	RALINK_REG(MDIO_CFG)=cpu_to_le32((u32)(0x1F01DC01));

#if defined (CHIP_RTL8367RB)
	rtl8367_switch_init_post();
#endif

#ifdef RT3883_USE_GE2
	wTmp = (u16)dev->enetaddr[0];
	regValue = (wTmp << 8) | dev->enetaddr[1];
	RALINK_REG(GDMA2_MAC_ADRH)=regValue;

	wTmp = (u16)dev->enetaddr[2];
	regValue = (wTmp << 8) | dev->enetaddr[3];
	regValue = regValue << 16;
	wTmp = (u16)dev->enetaddr[4];
	regValue |= (wTmp<<8) | dev->enetaddr[5];
	RALINK_REG(GDMA2_MAC_ADRL)=regValue;

	regValue = RALINK_REG(GDMA2_FWD_CFG);

    if(is_internal_loopback_test)
    {
    	regValue = regValue & GDM_UFRC_P_CPU;
		//Broad-cast MAC address frames forward to CPU
		regValue = regValue & GDM_BFRC_P_CPU;
		//Multi-cast MAC address frames forward to CPU
		regValue = regValue & GDM_MFRC_P_CPU;
    	//Other MAC address frames forward to CPU
    	regValue = regValue & GDM_OFRC_P_CPU;


		//All Drop

		regValue = regValue | GDM_UFRC_P_DROP;
		
		regValue = regValue | GDM_BFRC_P_DROP;
		
		regValue = regValue | GDM_MFRC_P_DROP;
    	
    	regValue = regValue | GDM_OFRC_P_DROP;

		printf("\n At interloopback mode, so all drop !\n");
    }
	else
	{
	regValue = regValue & GDM_UFRC_P_CPU;
	//Broad-cast MAC address frames forward to CPU
	regValue = regValue & GDM_BFRC_P_CPU;
	//Multi-cast MAC address frames forward to CPU
	regValue = regValue & GDM_MFRC_P_CPU;
	//Other MAC address frames forward to CPU
	regValue = regValue & GDM_OFRC_P_CPU;
	}

	RALINK_REG(GDMA2_FWD_CFG)=regValue;
	udelay(500);
	regValue = RALINK_REG(GDMA2_FWD_CFG);
#else // non RT3883_USE_GE2 //
	/* Set MAC address. */
	wTmp = (u16)dev->enetaddr[0];
	regValue = (wTmp << 8) | dev->enetaddr[1];

#if  defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)
	RALINK_REG(SDM_MAC_ADRH)=regValue;
	// printf("\n dev->iobase=%08X,SDM_MAC_ADRH=%08X\n",dev->iobase,regValue);
#else
	RALINK_REG(GDMA1_MAC_ADRH)=regValue;
	// printf("\n dev->iobase=%08X,GDMA1_MAC_ADRH=%08X\n ",dev->iobase, regValue);
#endif

	wTmp = (u16)dev->enetaddr[2];
	regValue = (wTmp << 8) | dev->enetaddr[3];
	regValue = regValue << 16;
	wTmp = (u16)dev->enetaddr[4];
	regValue |= (wTmp<<8) | dev->enetaddr[5];
#if  defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)
	RALINK_REG(SDM_MAC_ADRL)=regValue;
	// printf("\n dev->iobase=%08X,SDM_MAC_ADRL=%08X\n",dev->iobase,regValue);
#else
	RALINK_REG(GDMA1_MAC_ADRL)=regValue;
	// printf("\n dev->iobase=%08X,GDMA1_MAC_ADRL=%08X\n ",dev->iobase, regValue);
#endif

	//printf("\n rt2880_eth_setup, set MAC reg to [%02X:%02X:%02X:%02X:%02X:%02X]\n",
	//	dev->enetaddr[0],dev->enetaddr[1],dev->enetaddr[2],
	//	dev->enetaddr[3],dev->enetaddr[4],dev->enetaddr[5]);

#if ! defined (RT5350_ASIC_BOARD) && ! defined (RT5350_FPGA_BOARD)
	regValue = RALINK_REG(GDMA1_FWD_CFG);
	//printf("\n old,GDMA1_FWD_CFG = %08X \n",regValue);

	//Uni-cast frames forward to CPU
	regValue = regValue & GDM_UFRC_P_CPU;
	//Broad-cast MAC address frames forward to CPU
	regValue = regValue & GDM_BFRC_P_CPU;
	//Multi-cast MAC address frames forward to CPU
	regValue = regValue & GDM_MFRC_P_CPU;
	//Other MAC address frames forward to CPU
	regValue = regValue & GDM_OFRC_P_CPU;

	RALINK_REG(GDMA1_FWD_CFG)=regValue;
	udelay(500);
	regValue = RALINK_REG(GDMA1_FWD_CFG);
	//printf("\n new,GDMA1_FWD_CFG = %08X \n",regValue);
	
	regValue = 0x80504000;
	RALINK_REG(PSE_FQFC_CFG)=regValue;
#endif

#endif // RT3883_USE_GE2 //

#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	tx_ring1 = KSEG1ADDR((ulong)&tx_ring1_cache[0]);
#endif

#ifdef RALINK_GDMA_SCATTER_TEST_FUN
	u32	kk;
	if(header_payload_scatter_en == ENABLE)
	{
		temp = &PKT_HEADER_Buf[0] + (PKTALIGN - 1);
		temp -= (ulong)temp % PKTALIGN;

		for (i = 0; i < PKTBUFSRX; i++) {
			pkthdrbuf[i] = temp + (i*PKTSIZE_ALIGN) + sdp0_alig_16n_x;

			kk = (u32)pkthdrbuf[i];
			printf("\n pkthdrbuf[%d]=0x%08X,16N alignment= %d \n",i,kk, (kk % FLANK_TEST_SPX_ALIGNMENT));
		}
	}
#endif // RALINK_GDMA_SCATTER_TEST_FUN //


	for (i = 0; i < NUM_RX_DESC; i++) {
		temp = memset((void *)&rx_ring[i],0,16);
		rx_ring[i].rxd_info2.DDONE_bit = 0;

#ifdef RALINK_GDMA_SCATTER_TEST_FUN
		if(header_payload_scatter_en == ENABLE)
		{
			NetRxPackets[i]+= sdp1_alig_16n_x;
			rx_ring[i].rxd_info1.PDP0 = cpu_to_le32(phys_to_bus((u32) pkthdrbuf[i]));
			rx_ring[i].rxd_info3.PDP1 = cpu_to_le32(phys_to_bus((u32) (NetRxPackets[i])));
			rx_ring[i].rxd_info2.LS0= 0;
			rx_ring[i].rxd_info2.LS1= 1;
			printf("\n rx_ring[%d].rxd_info3.PDP1 = 0x%08X",i,rx_ring[i].rxd_info3.PDP1);
		}
		else
#endif // RALINK_GDMA_SCATTER_TEST_FUN //
		{
			BUFFER_ELEM *buf;
			buf = rt2880_free_buf_entry_dequeue(&rt2880_free_buf_list);
			NetRxPackets[i] = buf->pbuf;
			rx_ring[i].rxd_info2.LS0= 1;
			rx_ring[i].rxd_info1.PDP0 = cpu_to_le32(phys_to_bus((u32) NetRxPackets[i]));
		}
	}

	for (i=0; i < NUM_TX_DESC; i++) {
		temp = memset((void *)&tx_ring0[i],0,16);
#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
		//tx_ring0[i].txd_info2.LS1_bit = 1;
#else
		tx_ring0[i].txd_info2.LS0_bit = 1;
#endif
		tx_ring0[i].txd_info2.DDONE_bit = 1;
		/* PN:
		 *  0:CPU
		 *  1:GE1
		 *  2:GE2 (for RT2883)
		 *  6:PPE
		 *  7:Discard
		 */
		if (internal_loopback_test == INTERNAL_LOOPBACK_ENABLE) {
			tx_ring0[i].txd_info4.PN = 0;
			printf("\n Ring0,Set TX DMA loop back to CPU !! \n");
		}
		else {
#ifdef RT3883_USE_GE2
			tx_ring0[i].txd_info4.PN = 2;
#else
			tx_ring0[i].txd_info4.PN = 1;
#endif
		}

		tx_ring0[i].txd_info4.QN = 0;
	}

#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	for (i=0; i < NUM_TX_DESC; i++) {
		temp = memset(&tx_ring1[i],0,16);
#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
		//tx_ring1[i].txd_info2.LS1_bit = 1;
#else
		tx_ring1[i].txd_info2.LS0_bit = 1;
#endif
		tx_ring1[i].txd_info2.DDONE_bit = 1;
		/* PN:
		 *  0:CPU
		 *  1:GE1
		 *  2:GE2 (for RT2883)
		 *  6:PPE
		 *  7:Discard
		 */
		if (internal_loopback_test == INTERNAL_LOOPBACK_ENABLE) {
			tx_ring1[i].txd_info4.PN = 0;
			printf("\n Ring1,Set TX DMA loop back to CPU ! \n");
		}
		else {
#ifdef RT3883_USE_GE2
			tx_ring1[i].txd_info4.PN = 2;
#else
			tx_ring1[i].txd_info4.PN = 1;
#endif
		}

		tx_ring1[i].txd_info4.QN = 0;
	}
#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //

	rxRingSize = NUM_RX_DESC;
	txRingSize = NUM_TX_DESC;

	rx_dma_owner_idx0 = 0;
	rx_wants_alloc_idx0 = (NUM_RX_DESC - 1);
	tx_cpu_owner_idx0 = 0;
	tx_cpu_owner_idx1 = 0;

	regValue=RALINK_REG(PDMA_GLO_CFG);
	udelay(100);

#ifdef RALINK_GDMA_SCATTER_TEST_FUN
	if(header_payload_scatter_en == ENABLE)
	{
		regValue &= 0x0000FFFF;
		regValue |= (rt2880_hdrlen << 16);
		RALINK_REG(PDMA_GLO_CFG)=regValue;
		udelay(500);
		regValue=RALINK_REG(PDMA_GLO_CFG);
		printf("\n  Default of Header Length = 20 \n");
		printf("\n PDMA_GLO_CFG=%08X \n",regValue);
	}
	else
#endif // RALINK_GDMA_SCATTER_TEST_FUN //
	{
		regValue &= 0x0000FFFF;

		RALINK_REG(PDMA_GLO_CFG)=regValue;
		udelay(500);
		regValue=RALINK_REG(PDMA_GLO_CFG);
#ifndef RT3052_PHY_TEST
		printf("\n Header Payload scatter function is Disable !! \n");
#endif
	}

#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	RALINK_REG(TX_BASE_PTR1)=phys_to_bus((u32) &tx_ring1[0]);
	RALINK_REG(TX_MAX_CNT1)=cpu_to_le32((u32) NUM_TX_DESC);
	RALINK_REG(TX_CTX_IDX1)=cpu_to_le32((u32) tx_cpu_owner_idx1);
#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //

	/* Tell the adapter where the TX/RX rings are located. */
	RALINK_REG(RX_BASE_PTR0)=phys_to_bus((u32) &rx_ring[0]);

	//printf("\n rx_ring=%08X ,RX_BASE_PTR0 = %08X \n",&rx_ring[0],RALINK_REG(RX_BASE_PTR0));
	RALINK_REG(TX_BASE_PTR0)=phys_to_bus((u32) &tx_ring0[0]);

	//printf("\n tx_ring0=%08X, TX_BASE_PTR0 = %08X \n",&tx_ring0[0],RALINK_REG(TX_BASE_PTR0));

	RALINK_REG(RX_MAX_CNT0)=cpu_to_le32((u32) NUM_RX_DESC);
	RALINK_REG(TX_MAX_CNT0)=cpu_to_le32((u32) NUM_TX_DESC);

	RALINK_REG(TX_CTX_IDX0)=cpu_to_le32((u32) tx_cpu_owner_idx0);
	RALINK_REG(PDMA_RST_IDX)=cpu_to_le32((u32)RST_DTX_IDX0);

	RALINK_REG(RX_CALC_IDX0)=cpu_to_le32((u32) (NUM_RX_DESC - 1));
	RALINK_REG(PDMA_RST_IDX)=cpu_to_le32((u32)RST_DRX_IDX0);
	
	udelay(500);
	START_ETH(dev);
	
	return 1;
}


static int rt2880_eth_send(struct eth_device* dev, volatile void *packet, int length)
{
	int		status = -1;
	int		i;
	int		retry_count = 0, temp;
#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
	int		bk_len = 0;
	u8		*sdp1_seg_p;
	u32		*txd_info;
#endif
#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	static int	tingx_is_free = 0;
#endif
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) || \
    defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)
	char *p=(char *)packet;
#endif

Retry:
	if (retry_count > 10) {
		return (status);
	}

	if (length <= 0) {
		printf("%s: bad packet size: %d\n", dev->name, length);
		return (status);
	}

#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) || \
    defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)
	/* padding to 60 bytes for 3052 */
#define PADDING_LENGTH 60
	if (length < PADDING_LENGTH) {
		//	print_packet(packet,length);
		for(i=0;i<PADDING_LENGTH-length;i++) {
			p[length+i]=0;
		}
		length = PADDING_LENGTH;
	}
#endif //RT3052

#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN

#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
	for(i = 0; (tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0 && tx_ring0[tx_cpu_owner_idx0 +1 ].txd_info2.DDONE_bit == 0) || (tx_ring1[tx_cpu_owner_idx1].txd_info2.DDONE_bit == 0 && tx_ring1[tx_cpu_owner_idx1 + 1].txd_info2.DDONE_bit == 0); i++)

#else // Non RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //
		for(i = 0; tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0 || tx_ring1[tx_cpu_owner_idx1].txd_info2.DDONE_bit == 0; i++)
#endif // RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //
#else  // Non RALINK_GDMA_DUP_TX_RING_TEST_FUN //
#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN

			for(i = 0; tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0 && tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.DDONE_bit == 0; i++)
#else // Non RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //

				for(i = 0; tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0 ; i++)

#endif // RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //
#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //
				{
					if (i >= TOUT_LOOP) {
						//printf("%s: TX DMA is Busy !! TX desc is Empty!\n", dev->name);
						goto Done;
					}
				}
	//dump_reg();

	temp = RALINK_REG(TX_DTX_IDX0);

	if(temp == (tx_cpu_owner_idx0+1) % NUM_TX_DESC) {
		puts(" @ ");
		goto Done;
	}

#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	if(tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 1 && tx_ring1[tx_cpu_owner_idx1].txd_info2.DDONE_bit == 1)
	{
		if(tingx_is_free == 0 )
		{
			printf("\n Sent packet with ring1\n");
			tingx_is_free = 1;
		}
		else
		{
			printf("\n Sent packet with ring0\n");
			tingx_is_free = 0;
		}
	}
	else if(tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 1)
	{
		//printf("\n Sent packet with ring0\n");
		tingx_is_free = 0;
	}
	else
	{
		//printf("\n Sent packet with ring1\n");
		tingx_is_free = 1;
	}

	if(force_queue_n == 0)
	{
		//printf("\n Force Sent packet with ring0\n");
		tingx_is_free = 0;
	}
	else if (force_queue_n == 1)
	{
		//printf("\n Force Sent packet with ring1\n");
		tingx_is_free = 1;
	}
#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //

#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
	//printf(" Send's Packet addr= 0x%08X,Total length=%d \n",packet,length);
#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	if(tingx_is_free == 0)
	{
		txd_info = (u32 *)&tx_ring0[tx_cpu_owner_idx0].txd_info2;
		*txd_info = 0;

		bk_len = (length >> 2);
		length = length - (bk_len * 3);

		//Segment 0
		tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) packet));
		tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = bk_len;
		//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0=%08X",tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0);
		//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0=%d",tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0);

		sdp1_seg_p = packet;
		sdp1_seg_p += bk_len;
		//Segment 1
		tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
		tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1 = bk_len;

		//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1=%08X",tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1);
		//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1=%d",tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1);

		sdp1_seg_p += bk_len;
		//Segment 2
		tx_ring0[tx_cpu_owner_idx0 + 1].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
		tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL0 = bk_len;
		//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info1.SDP0=%08X",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info1.SDP0);
		//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL0=%d",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL0);

		sdp1_seg_p += bk_len;

		//Segment 3
		tx_ring0[tx_cpu_owner_idx0 + 1].txd_info3.SDP1 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
		tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL1 = length;

		//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info3.SDP1=%08X",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info3.SDP1);
		//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL1=%d",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL1);
	}
	else
	{
		txd_info = (u32 *)&tx_ring1[tx_cpu_owner_idx1].txd_info2;
		*txd_info = 0;

		bk_len = (length >> 2);
		length = length - (bk_len * 3);

		//Segment 0
		tx_ring1[tx_cpu_owner_idx1].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) packet));
		tx_ring1[tx_cpu_owner_idx1].txd_info2.SDL0 = bk_len;
		//printf("\n tx_ring1[tx_cpu_owner_idx0].txd_info1.SDP0=%08X",tx_ring1[tx_cpu_owner_idx0].txd_info1.SDP0);
		//printf("\n tx_ring1[tx_cpu_owner_idx0].txd_info2.SDL0=%d",tx_ring1[tx_cpu_owner_idx0].txd_info2.SDL0);

		sdp1_seg_p = packet;
		sdp1_seg_p += bk_len;
		//Segment 1
		tx_ring1[tx_cpu_owner_idx1].txd_info3.SDP1 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
		tx_ring1[tx_cpu_owner_idx1].txd_info2.SDL1 = bk_len;

		//printf("\n tx_ring1[tx_cpu_owner_idx0].txd_info3.SDP1=%08X",tx_ring1[tx_cpu_owner_idx0].txd_info3.SDP1);
		//printf("\n tx_ring1[tx_cpu_owner_idx0].txd_info2.SDL1=%d",tx_ring1[tx_cpu_owner_idx0].txd_info2.SDL1);

		sdp1_seg_p += bk_len;
		//Segment 2
		tx_ring1[tx_cpu_owner_idx1 + 1].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
		tx_ring1[tx_cpu_owner_idx1 + 1].txd_info2.SDL0 = bk_len;
		//printf("\n tx_ring1[tx_cpu_owner_idx0 + 1].txd_info1.SDP0=%08X",tx_ring1[tx_cpu_owner_idx0 + 1].txd_info1.SDP0);
		//printf("\n tx_ring1[tx_cpu_owner_idx0 + 1].txd_info2.SDL0=%d",tx_ring1[tx_cpu_owner_idx0 + 1].txd_info2.SDL0);

		sdp1_seg_p += bk_len;

		//Segment 3
		tx_ring1[tx_cpu_owner_idx1 + 1].txd_info3.SDP1 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
		tx_ring1[tx_cpu_owner_idx1 + 1].txd_info2.SDL1 = length;

		//printf("\n tx_ring1[tx_cpu_owner_idx0 + 1].txd_info3.SDP1=%08X",tx_ring1[tx_cpu_owner_idx0 + 1].txd_info3.SDP1);
		//printf("\n tx_ring1[tx_cpu_owner_idx0 + 1].txd_info2.SDL1=%d",tx_ring1[tx_cpu_owner_idx0 + 1].txd_info2.SDL1);
	}
#else // Non RALINK_GDMA_DUP_TX_RING_TEST_FUN //

	txd_info = (u32 *)&tx_ring0[tx_cpu_owner_idx0].txd_info2;
	*txd_info = 0;

	bk_len = (length >> 2);
	length = length - (bk_len * 3);

	//Segment 0
	tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) packet));
	tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = bk_len;
	//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0=%08X",tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0);
	//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0=%d",tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0);

	sdp1_seg_p = packet;
	sdp1_seg_p += bk_len;
	//Segment 1
	tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
	tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1 = bk_len;

	//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1=%08X",tx_ring0[tx_cpu_owner_idx0].txd_info3.SDP1);
	//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1=%d",tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL1);

	sdp1_seg_p += bk_len;
	//Segment 2
	tx_ring0[tx_cpu_owner_idx0 + 1].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
	tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL0 = bk_len;
	//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info1.SDP0=%08X",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info1.SDP0);
	//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL0=%d",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL0);

	sdp1_seg_p += bk_len;

	//Segment 3
	tx_ring0[tx_cpu_owner_idx0 + 1].txd_info3.SDP1 = cpu_to_le32(phys_to_bus((u32) sdp1_seg_p));
	tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL1 = length;

	//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info3.SDP1=%08X",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info3.SDP1);
	//printf("\n tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL1=%d",tx_ring0[tx_cpu_owner_idx0 + 1].txd_info2.SDL1);

#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //

#else // Non RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //
#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	if(tingx_is_free == 0)
	{
		tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) packet));
		tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = length;
	}
	else
	{
		tx_ring1[tx_cpu_owner_idx1].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) packet));
		tx_ring1[tx_cpu_owner_idx1].txd_info2.SDL0 = length;
	}
#else // Non RALINK_GDMA_DUP_TX_RING_TEST_FUN //
	tx_ring0[tx_cpu_owner_idx0].txd_info1.SDP0 = cpu_to_le32(phys_to_bus((u32) packet));
	tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 = length;

	//printf("\n tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0 =%d \n",tx_ring0[tx_cpu_owner_idx0].txd_info2.SDL0);
#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //
#endif // RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //

#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
	if(tingx_is_free == 0)
	{
#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN

		tx_ring0[tx_cpu_owner_idx0 +1 ].txd_info2.LS1_bit = 1;
		tx_ring0[tx_cpu_owner_idx0 +1 ].txd_info2.DDONE_bit = 0;
		tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;
		status = length;

		tx_cpu_owner_idx0 = (tx_cpu_owner_idx0+2) % NUM_TX_DESC;

		RALINK_REG(TX_CTX_IDX0)=cpu_to_le32((u32) tx_cpu_owner_idx0);
#else
		tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;

		status = length;
		tx_cpu_owner_idx0 = (tx_cpu_owner_idx0+1) % NUM_TX_DESC;
		RALINK_REG(TX_CTX_IDX0)=cpu_to_le32((u32) tx_cpu_owner_idx0);
		//printf("\TX_CTX_IDX0 = %08X \n",RALINK_REG(TX_CTX_IDX0));
		//printf("\TX_DTX_IDX0 = %08X \n",RALINK_REG(TX_DTX_IDX0));
#endif // RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //

	}
	else
	{

#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
		tx_ring1[tx_cpu_owner_idx1 +1 ].txd_info2.LS1_bit = 1;
		tx_ring1[tx_cpu_owner_idx1 +1 ].txd_info2.DDONE_bit = 0;
		tx_ring1[tx_cpu_owner_idx1].txd_info2.DDONE_bit = 0;
		status = length;

		tx_cpu_owner_idx1 = (tx_cpu_owner_idx1+2) % NUM_TX_DESC;

		RALINK_ERG(TX_CTX_IDX1)=cpu_to_le32((u32) tx_cpu_owner_idx1);
#else
		tx_ring1[tx_cpu_owner_idx1].txd_info2.DDONE_bit = 0;
		status = length;
		tx_cpu_owner_idx1 = (tx_cpu_owner_idx1+1) % NUM_TX_DESC;
		RALINK_REG(TX_CTX_IDX1)=cpu_to_le32((u32) tx_cpu_owner_idx1);

		//printf("\TX_CTX_IDX1 = %08X \n",RALINK_REG(TX_CTX_IDX1));
		//printf("\TX_DTX_IDX1 = %08X \n",RALINK_REG(TX_DTX_IDX1));

#endif // RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //
	}
#else // Non RALINK_GDMA_DUP_TX_RING_TEST_FUN //

#ifdef RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
	tx_ring0[tx_cpu_owner_idx0 +1 ].txd_info2.LS1_bit = 1;
	tx_ring0[tx_cpu_owner_idx0 +1 ].txd_info2.DDONE_bit = 0;
	tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;
	status = length;

	tx_cpu_owner_idx0 = (tx_cpu_owner_idx0+2) % NUM_TX_DESC;

	RALINK_REG(TX_CTX_IDX0)=cpu_to_le32((u32) tx_cpu_owner_idx0);
#else // Non RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN
	tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit = 0;
	status = length;

#ifdef RT3052_PHY_TEST
	if ( rt3052_phy_test == PHY_TEST_DISABLE ) {
#endif
	if(loopback_protect == 1)
	{
		BUFFER_ELEM *buf;

#if defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD)
		PDMA_RXD_INFO4_T *rxd4 = (PDMA_RXD_INFO4_T *)&rx_ring[rx_dma_owner_idx0].rxd_info4;
		if (rxd4->SP == 1)
			tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = 2;
		else if (rxd4->SP == 2)
			tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = 1;
		else
			tx_ring0[tx_cpu_owner_idx0].txd_info4.PN = 1;
#endif
		buf = rt2880_free_buf_entry_dequeue(&rt2880_busing_buf_list);
		//while
		while(buf != NULL)
		{
			//printf("\n check to Bufnum[%d] \n",FREEBUF_OFFSET(buf->pbuf));

			if(tx_ring0[buf->tx_idx].txd_info2.DDONE_bit == 1)
			{
				//printf("\n Precedent of Packet was  send  \n");
				rt2880_free_buf_entry_enqueue(&rt2880_free_buf_list,buf);
			}
			else
			{
				//printf("\n Precedent of Packet was pending !!  \n");
				rt2880_free_buf_entry_enqueue(&rt2880_busing_buf_list,buf);
			}
			buf = rt2880_free_buf_entry_dequeue(&rt2880_busing_buf_list);
		}

		i = FREEBUF_OFFSET(packet);

		rt2880_free_buf[i].tx_idx = tx_cpu_owner_idx0;
		rt2880_free_buf_entry_enqueue(&rt2880_busing_buf_list,&rt2880_free_buf[i]);
		//printf("\n Loopback Send Bufnum = %d\n",i);
	}
#ifdef RT3052_PHY_TEST
           }  // if not rt3052_phy_test
#endif

	tx_cpu_owner_idx0 = (tx_cpu_owner_idx0+1) % NUM_TX_DESC;
	RALINK_REG(TX_CTX_IDX0)=cpu_to_le32((u32) tx_cpu_owner_idx0);

#endif // RALINK_MUTI_TX_DESCRIPTOR_TEST_FUN //

#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //

	//kaiker_led_tx_ring();
	return status;
Done:
	udelay(500);
	retry_count++;
	goto Retry;
}


static int rt2880_eth_recv(struct eth_device* dev)
{
#ifdef RT3052_PHY_TEST
	int recv_cnt, i;
#endif
	int length = 0,hdr_len=0,bb=0;
	int inter_loopback_cnt =0;
	u32 *rxd_info;
#ifdef RALINK_RUN_COMMAD_AT_ETH_RCV_FUN
	char lastcommand[30];
#endif
#if !defined (RT3883_FPGA_BOARD) && !defined (RT3883_ASIC_BOARD)
	u8 temp_mac[6];
#endif
#ifdef RALINK_GDMA_SCATTER_TEST_FUN
	uchar *scatter_src,*scatter_dst;
#endif

#ifdef RALINK_SWITCH_LOOPBACK_DEBUG_FUN
	static u8 mac_1[]={0x00,0xAA,0xBB,0xCC,0xDD,0x01};
	static u8 mac_2[]={0x00,0xAA,0xBB,0xCC,0xDD,0x02};
	static u8 mac_3[]={0x00,0xAA,0xBB,0xCC,0xDD,0x03};
	static u8 mac_4[]={0x00,0xAA,0xBB,0xCC,0xDD,0x04};
	static u8 mac_5[]={0x00,0xAA,0xBB,0xCC,0xDD,0x05};
	static u8 mac_6[]={0x00,0xAA,0xBB,0xCC,0xDD,0x06};
#endif // RALINK_SWITCH_LOOPBACK_DEBUG_FUN //

	for (; ; ) {
#ifdef RALINK_RUN_COMMAD_AT_ETH_RCV_FUN
		bb = kaiker_button_p();
		if(bb == 3 )
		{
			//kaiker_debug_show(dev);
			input_value(lastcommand);
			kaiker_run_command(lastcommand,0);
		}
#endif // RALINK_RUN_COMMAD_AT_ETH_RCV_FUN //
		rxd_info = (u32 *)KSEG1ADDR(&rx_ring[rx_dma_owner_idx0].rxd_info2);

		if ( (*rxd_info & BIT(31)) == 0 )
		{
			hdr_len =0;
			if (eth_loopback_mode == 1) {
				if (bb == 1) {
					rt2880_eth_halt(rt2880_pdev);
					puts ("\nAbort Loopback Mode\n");
					eth_loopback_mode = 0;
					//rt2880_eth_setup(rt2880_pdev);
					return (0);
				}
				continue;
			}
			else {
				break;
			}
		}

		udelay(1);
#ifdef RALINK_GDMA_SCATTER_TEST_FUN
		if(header_payload_scatter_en == ENABLE)
		{
			length = rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN1;
			hdr_len = rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN0;
		}
		else
#endif // RALINK_GDMA_SCATTER_TEST_FUN //
			length = rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN0;

		if(header_payload_scatter_en == DISABLE && length == 0)
		{
			printf("\n Warring!! Packet Length has error !!,In normal mode !\n");
		}
#ifdef RALINK_GDMA_SCATTER_TEST_FUN
		if(header_payload_scatter_en == ENABLE && hdr_len == 0)
		{
			printf("\n Warring!! Packet Length has error !!,In Scatter mode !\n");
		}
#endif // RALINK_GDMA_SCATTER_TEST_FUN //

		if(eth_loopback_mode == 1)
		{
#ifdef RALINK_GDMA_SCATTER_TEST_FUN
			if(header_payload_scatter_en == ENABLE)
			{
				printf("\n ===== Header len [%d] === \n",hdr_len);
				scatter_dst = (uchar *)KSEG1ADDR(pkthdrbuf[rx_dma_owner_idx0]);
				//scatter_dst[0]=0x00;
				//scatter_dst[1]=0x11;
				//scatter_dst[2]=0x22;
				//scatter_dst[3]=0x33;
				//scatter_dst[4]=0x44;
				//scatter_dst[5]=0x55;
				//print_packet(scatter_dst,hdr_len);
				printf("\n ===== Pay load len [%d] === \n",length);
				scatter_src = (uchar *)KSEG1ADDR(NetRxPackets[rx_dma_owner_idx0]);
				//print_packet(scatter_src,length);

				scatter_dst += hdr_len;
				memcpy(scatter_dst,scatter_src,length);

				length += hdr_len;
				rt2880_eth_send(dev, (void *)KSEG1ADDR(pkthdrbuf[rx_dma_owner_idx0]),length);
			}
			else
#endif // RALINK_GDMA_SCATTER_TEST_FUN //
			{
				BUFFER_ELEM *buf;
#if !defined (RT3883_FPGA_BOARD) && !defined (RT3883_ASIC_BOARD)
				u8 *p = (u8 *)KSEG1ADDR(NetRxPackets[rx_dma_owner_idx0]);
#endif

				buf = rt2880_free_buf_entry_dequeue(&rt2880_free_buf_list);

				if(buf == NULL)
				{
					printf("\n Warrng Packet Buffer is Empty!!\n");
					return (0);
				}

				//RT3883: we don't have to swap DA/SA because it is GE1<-->GE2
#if !defined (RT3883_FPGA_BOARD) && !defined (RT3883_ASIC_BOARD)
				//printf("\n Ready loopback,Current Buff num = %d \n",FREEBUF_OFFSET(NetRxPackets[rx_dma_owner_idx0]));
				if(p[0]!= 0xFF)
				{
					// save da
					memcpy(temp_mac,p,6);

#ifdef RALINK_SWITCH_LOOPBACK_DEBUG_FUN
					if(!memcmp(p+6,mac_1,6))
					{
						memcpy(p+6,mac_2,6);
					}
					else if(!memcmp(p+6,mac_2,6))
					{
						memcpy(p+6,mac_1,6);
					}
					else if(!memcmp(p+6,mac_3,6))
					{
						memcpy(p+6,mac_4,6);
					}
					else if(!memcmp(p+6,mac_4,6))
					{
						memcpy(p+6,mac_3,6);
					}
					else if(!memcmp(p+6,mac_5,6))
					{
						memcpy(p+6,mac_6,6);
					}
					else if(!memcmp(p+6,mac_6,6))
					{
						memcpy(p+6,mac_5,6);
					}
#endif // RALINK_SWITCH_LOOPBACK_DEBUG_FUN //

					//copy sa to da
					memcpy(p,p+6,6);
					memcpy(p+6,temp_mac,6);
				}
#endif

				loopback_protect = 1;
				rt2880_eth_send(dev, (void *)KSEG1ADDR(NetRxPackets[rx_dma_owner_idx0]),length);
				loopback_protect = 0;
				NetRxPackets[rx_dma_owner_idx0] = buf->pbuf;

				rx_ring[rx_dma_owner_idx0].rxd_info2.LS0= 1;
				rx_ring[rx_dma_owner_idx0].rxd_info1.PDP0 = cpu_to_le32(phys_to_bus((u32) NetRxPackets[rx_dma_owner_idx0]));
			}
		}
#ifdef RT3052_PHY_TEST
		else if (rt3052_phy_test == PHY_TEST_ENABLE) {
			// int ret;
			uchar* rx_buf = rx_ring[rx_dma_owner_idx0].rxd_info1.PDP0;

			rt3052_phy_test_ret_code = 0;
			rt3052_phy_test_ret_code = memcmp(&rt3052_phy_test_buf[0], rx_buf, 12);//received packet without vtag
			rt3052_phy_test_ret_code |= memcmp(&rt3052_phy_test_buf[16], rx_buf+12, length-12);

			udelay(50000);//delay to avoid receive fail
			
			if(rt3052_phy_test_debug == 1)
			{
				printf("\nRx Path len - %d, ret = %d\n", length, rt3052_phy_test_ret_code);
				printf("RX Packet Dump -- \n");
				packet_dump(rx_buf, length);
			}
		    	NetReceive(NetRxPackets[rx_dma_owner_idx0], length);
			// printf("--- END of RX Packet Dump ---\n");
		}
#endif
		else
		{
#ifdef RALINK_GDMA_SCATTER_TEST_FUN
			if(header_payload_scatter_en == ENABLE)
			{
				scatter_dst = (uchar *)KSEG1ADDR(pkthdrbuf[rx_dma_owner_idx0]);
				scatter_src = (uchar *)KSEG1ADDR(NetRxPackets[rx_dma_owner_idx0]);

				scatter_dst += hdr_len;
				printf("\n scatter enbale ,rcv hdr_len=%d \n",hdr_len);
				memcpy(scatter_dst,scatter_src,length);
				length += hdr_len;
				if(rx_ring[rx_dma_owner_idx0].rxd_info4.SP == 0)
				{// Packet received from CPU port
					printf("\n HEADER_PAYLOAD_SCATTER mode,Packet received from CPU port,plen=%d \n",length);
					//print_packet((void *)KSEG1ADDR(pkthdrbuf[rx_dma_owner_idx0]),length);
					inter_loopback_cnt++;
					length = inter_loopback_cnt;//for return
				}
				else
		    		NetReceive((void *)KSEG1ADDR(pkthdrbuf[rx_dma_owner_idx0]), length );
			//kaiker_led_scatter_packet();
			}
			else
#endif // RALINK_GDMA_SCATTER_TEST_FUN //
			{
				if(rx_ring[rx_dma_owner_idx0].rxd_info4.SP == 0)
				{// Packet received from CPU port
					printf("\n Normal Mode,Packet received from CPU port,plen=%d \n",length);
					//print_packet((void *)KSEG1ADDR(NetRxPackets[rx_dma_owner_idx0]),length);
					inter_loopback_cnt++;
					length = inter_loopback_cnt;//for return
				}
				else
					NetReceive((void *)KSEG1ADDR(NetRxPackets[rx_dma_owner_idx0]), length );
			}
		}

		#if 0
		rx_ring[rx_dma_owner_idx0].rxd_info2.DDONE_bit = 0;
		rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN = 0;
		#else

		rxd_info = (u32 *)&rx_ring[rx_dma_owner_idx0].rxd_info4;
		*rxd_info = 0;

		rxd_info = (u32 *)&rx_ring[rx_dma_owner_idx0].rxd_info2;
		*rxd_info = 0;
		rx_ring[rx_dma_owner_idx0].rxd_info2.LS0= 1;
		#endif

		/* Tell the adapter where the TX/RX rings are located. */
		RALINK_REG(RX_BASE_PTR0)=phys_to_bus((u32) &rx_ring[0]);

		//udelay(10000);
		/*  Move point to next RXD which wants to alloc*/
		RALINK_REG(RX_CALC_IDX0)=cpu_to_le32((u32) rx_dma_owner_idx0);

		/* Update to Next packet point that was received.
		 */
		rx_dma_owner_idx0 = (rx_dma_owner_idx0 + 1) % NUM_RX_DESC;

		//printf("\n ************************************************* \n");
		//printf("\n RX_CALC_IDX0=%d \n", RALINK_REG(RX_CALC_IDX0));
		//printf("\n RX_DRX_IDX0 = %d \n",RALINK_REG(RX_DRX_IDX0));
		//printf("\n ************************************************* \n");
#ifdef RT3052_PHY_TEST
		if ( rt3052_phy_test == PHY_TEST_ENABLE)
		{
			unsigned int rx_dtx = 0;
			    
			rx_dtx = RALINK_REG(RX_DRX_IDX0);
			if ( rx_dma_owner_idx0  ==  rx_dtx ) 
			{
				return length;
			}
		}
#endif
	}
	return length;
}

void rt2880_eth_halt(struct eth_device* dev)
{
	 STOP_ETH(dev);
	//gmac_phy_switch_gear(DISABLE);
	//printf(" STOP_ETH \n");
	//dump_reg();
}

#ifdef RALINK_GDMA_STATUS_DISPLAY_FUN
void kaiker_debug_show(struct eth_device* dev)
{
	int		kk;

#if 1
	printf("\n# rx_ring[rx_dma_owner_idx0].rxd_info2.DDONE_bit=%d ,length=%d \n",rx_ring[rx_dma_owner_idx0].rxd_info2.DDONE_bit,rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN0);
	printf("#rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.IPFVLD_bit=%d \n",rx_ring[rx_dma_owner_idx0].rxd_info4.IPFVLD_bit);
	rx_ring[rx_dma_owner_idx0].rxd_info4.IPFVLD_bit=0;
	printf("\n #rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.L4FVLD_bit=%d \n",rx_ring[rx_dma_owner_idx0].rxd_info4.L4FVLD_bit);
	rx_ring[rx_dma_owner_idx0].rxd_info4.L4FVLD_bit=0;
	printf("\n #rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.IPF=%d \n",rx_ring[rx_dma_owner_idx0].rxd_info4.IPF);
	rx_ring[rx_dma_owner_idx0].rxd_info4.IPF = 0;
	printf("\n #rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.L4F=%d \n",rx_ring[rx_dma_owner_idx0].rxd_info4.L4F);
	rx_ring[rx_dma_owner_idx0].rxd_info4.L4F =0;
	printf("\n #rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.AIS=%d \n",rx_ring[rx_dma_owner_idx0].rxd_info4.AIS);
	printf("\n #rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.AI=%02X \n",rx_ring[rx_dma_owner_idx0].rxd_info4.AI);
	printf("\n #rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.FVLD=%d \n",rx_ring[rx_dma_owner_idx0].rxd_info4.FVLD);
	printf("\n #rx_ring[rx_dma_owner_idx0].rxd_info2.rxd_info4.FOE_Entry=%04X \n",rx_ring[rx_dma_owner_idx0].rxd_info4.FOE_Entry);
#endif

#if 1
	for(kk=0;kk<NUM_RX_DESC;kk++)
	{
		printf("\n rx_ring[%d].rxd_info2.DDONE_bit=%d \n",kk,rx_ring[kk].rxd_info2.DDONE_bit);
		printf("\n rx_ring[%d].rxd_info2.PLEN=%d \n",kk,rx_ring[kk].rxd_info2.PLEN0);
	}
	for(kk=0;kk<NUM_RX_DESC;kk++)
	{
		printf("\n tx_ring0[%d].txd_info2.DDONE_bit=%d \n",kk,tx_ring0[kk].txd_info2.DDONE_bit);
	}
#endif
	printf("\n ############################ \n");

	kk = RALINK_REG(PDMA_GLO_CFG);
	if((kk & RX_DMA_BUSY))
		printf("\n  RX_DMA_BUSY !!! ");
	if((kk & TX_DMA_BUSY))
		printf("\n  TX_DMA_BUSY !!! ");

	kk = RALINK_REG(FE_INT_STATUS);
	RALINK_REG(FE_INT_STATUS)=cpu_to_le32((u32) kk);
	printf("\n print FE_INT_STATUS content=[0x%08X]\n",kk);

	if((kk & CNT_PPE_AF))
		printf("\n PPE Counter Table Almost Full ");

	if((kk & CNT_GDM1_AF))
		printf("\n GDMA1 Counter Table Almost Full ");

	if((kk & PSE_P1_FC))
		printf("\n PSE port1 (GDMA1) flow control asserted. ");

	if((kk & PSE_P0_FC))
		printf("\n PSE port0 (CDMA) flow control asserted. ");

	if((kk & TX_COHERENT))
		printf("\n TX_COHERENT ");

	if((kk & RX_COHERENT))
		printf("\n RX_COHERENT ");

	if((kk & PSE_FQ_EMPTY))
		printf("\n PSE free Q empty threshold reached & forced  ");

	if((kk & GE1_STA_CHG))
		printf("\n GE port #1 link status changes (link, speed, flow control)  ");

	if((kk & TX_DONE_INT1))
		printf("\n High priority packet transmit interrupt  ");

	if((kk & TX_DONE_INT0))
		printf("\n Low priority packet transmit interrupt  ");

	if((kk & RX_DONE_INT0))
		printf("\n Packet receive interrupt.  ");

	if((kk & RX_DLY_INT))
		printf("\n Delayed version of RX_DONE_INT0.  ");

	if((kk & TX_DLY_INT))
		printf("\n Delayed version of TX_DONE_INT0 and TX_DONE_INT1.  ");

	printf("\n\n");

	printf("\n RX_CALC_IDX0=%d \n", RALINK_REG(RX_CALC_IDX0));
	printf("\n RX_DRX_IDX0 = %d \n",RALINK_REG(RX_DRX_IDX0));

	printf("\n rx_ring[%d].rxd_info2.PLEN=%d \n",rx_dma_owner_idx0,rx_ring[rx_dma_owner_idx0].rxd_info2.PLEN0 );

	printf("\n TX_CTX_IDX0 = %08X \n",RALINK_REG(TX_CTX_IDX0));
	printf("\n TX_DTX_IDX0 = %08X \n",RALINK_REG(TX_DTX_IDX0));

	printf("\n TX_CTX_IDX1 = %08X \n",RALINK_REG(TX_CTX_IDX1));
	printf("\n TX_DTX_IDX1 = %08X \n",RALINK_REG(TX_DTX_IDX1));

	printf("\n ############################ \n");
}
#endif // RALINK_GDMA_STATUS_DISPLAY_FUN //

#if 0
static void print_packet( u8 * buf, int length )
{

	int i;
	int remainder;
	int lines;


	printf("Packet of length %d \n", length );


	lines = length / 16;
	remainder = length % 16;

	for ( i = 0; i < lines ; i ++ ) {
		int cur;

		for ( cur = 0; cur < 8; cur ++ ) {
			u8 a, b;

			a = *(buf ++ );
			b = *(buf ++ );
			printf("%02X %02X ", a, b );
		}
		printf("\n");
	}
	for ( i = 0; i < remainder/2 ; i++ ) {
		u8 a, b;

		a = *(buf ++ );
		b = *(buf ++ );
		printf("%02X %02X ", a, b );
	}
	printf("\n");

}
#endif


#ifdef RALINK_MEMORY_TEST_FUN
#define PSRC_ADDR      0x8a1312ff
#define PDEST_ADDR      0x8b131247
#define PDEST_ADDR1     0x8a731247
#define MEM_SIZE        65535
#define RUN		1
#define MEMTEST 1

/* bob added ++*/
// #define SDRAM_BASE_MEMTEST 0x8a800000
// #define SDRAM_BASE1_MEMTEST 0xaa900000
#define SDRAM_BASE_MEMTEST  0x8b000000
#define SDRAM_BASE1_MEMTEST 0xab00000
#define SRAM_BASE_MEMTEST   0xa0900000
#define SIZE_OF_SRAM (32*1024)

#define TEST_SIZE SIZE_OF_SRAM

#define SRAM_TEST_SIZE (10240)

#define TEST_SIZE_OF_SDRAM (6*1024*1024)

#define MEMTEST_BLOCK_SIZE		(4*1024)

/* bob ++ */
#define    ESRAM    1
void test_random(u32 k) // unused part, legacy code
{
    int i;
    unsigned char *src,*dest;
    unsigned int mem_test_size, sram_mem_offset, sdram_mem_offset;
 unsigned int random_i;


    //srandom(time(0) | getpid());

#if 0

  for ( i = 0; i < k; i++)
  {
  //printf("\n Total Test num [%d]\n",i);
 if(kaiker_button_p())
 {
    printf("\n Abort memtest!!\n");
   return;

 }
 random_i = (unsigned int)get_timer(0);

  /* step 1: decide memory size */
 mem_test_size = (random_i % SIZE_OF_SRAM);

 /* step 2: decide sram offset */
 sram_mem_offset = (random_i % (SIZE_OF_SRAM-mem_test_size));

 /* step 3: decide sdram offset */
 sdram_mem_offset = (random_i % (SIZE_OF_SDRAM-mem_test_size));

 src = (SDRAM_BASE_MEMTEST + sdram_mem_offset);
 dest = (SRAM_BASE_MEMTEST + sram_mem_offset);

 if(random_memcpy(dest,src,mem_test_size))
 {
    udelay(mem_test_size);
    continue;
 }

 random_memcpy(src,dest,mem_test_size);
 udelay(mem_test_size);
 printf("\nTest Count[%d]\n",i+1);
  } /* for */
#endif
}

void sram_test_random(u32 test_times)
{
    int i;
    unsigned char *src,*dest;
    unsigned int mem_test_size, src_mem_offset, dest_mem_offset;
    unsigned int random_i, i_diff;


  for ( i = 0; i < test_times; i++)
  {
  //printf("\n Total Test num [%d]\n",i);
   if(kaiker_button_p())
   {
    printf("\n Abort memtest!!\n");
   return;

   }

memtest_sram_reset:
   random_i = (unsigned int)get_timer(0);


  /* step 1: decide memory size */
  mem_test_size = (random_i % SRAM_TEST_SIZE);

  /* step 2: decide source sram offset */
  src_mem_offset = (random_i % (SIZE_OF_SRAM-mem_test_size));

  /* step 3: decide dest sram offset */
  random_i = (unsigned int)get_timer(random_i);
  dest_mem_offset = (random_i % (SIZE_OF_SRAM-mem_test_size));

  if (src_mem_offset == dest_mem_offset)
	goto memtest_sram_reset;


  if ( src_mem_offset > dest_mem_offset)
   i_diff = src_mem_offset - dest_mem_offset;
  else
   i_diff = dest_mem_offset - src_mem_offset;

  if (i_diff < mem_test_size)
	goto memtest_sram_reset;



 src = src_mem_offset + SRAM_BASE_MEMTEST;
 dest = dest_mem_offset + SRAM_BASE_MEMTEST;


 random_memcpy(src,dest,mem_test_size);

 udelay(mem_test_size);

 printf("\nTest Count[%d]\n",i);
  } /* for */
}

int address_check(unsigned int src, unsigned int dest, unsigned int size)
{
	unsigned int offset;

	if ( src == dest )
		return 0;

	if ( src > dest )
		offset = src - dest;
	else
		offset = dest - src;

	if ( offset < size )
		return 0;

	return 1;
}

int ram_test_random(u32 test_times, u32 mem_base)
{
    int i, result, fail_cnt;
    unsigned char *src,*dest;
    unsigned int mem_test_size, src_mem_offset, dest_mem_offset;
    unsigned int random_i, i_diff;

  fail_cnt = 0;
  for ( i = 0; i < test_times; i++)
  {
  //printf("\n Total Test num [%d]\n",i);
   if(kaiker_button_p())
   {
    	printf("\n--> Run %d test counts ...User Cancel memtest!!\n", i);
   	return;
   }

	do {

		if (kaiker_button_p())
		{
    			printf("\n--> Run %d test counts ...User Cancel memtest!!\n", i);
   			return;
		}

   	  	  random_i = (unsigned int)get_timer(test_times);
		  /* step 1: decide memory size */
		  mem_test_size = (random_i % MEMTEST_BLOCK_SIZE);

		  /* step 2: decide source sdram offset */
		  random_i = (unsigned int)get_timer(random_i);
		  src_mem_offset = (random_i % (TEST_SIZE_OF_SDRAM-mem_test_size));

		  /* step 3: decide dest sdram offset */
		  random_i = (unsigned int)get_timer(random_i);
		  dest_mem_offset = (random_i % (TEST_SIZE_OF_SDRAM-mem_test_size));
	} while (address_check(src_mem_offset, dest_mem_offset, mem_test_size) == 0);


	if ( (i%2) == 0 )  {
    		src = dest_mem_offset + mem_base;
    		dest = src_mem_offset + SDRAM_BASE1_MEMTEST;
	} else {
		src  = src_mem_offset + SDRAM_BASE1_MEMTEST;
    		dest = dest_mem_offset + mem_base;
	}

    	result =  random_memcpy(src,dest,mem_test_size);


    	if(result != 0) {
     		printf("Random Mem Test Count[%d], %d failed\n-------\n\n",i+1, result);
		fail_cnt++;
    	}
    	// udelay(mem_test_size);
  } /* for */
  printf("\nRandom Mem Test Count - %d, %d failed\n",i, fail_cnt);
}

int random_memcpy(unsigned char* p_dest, unsigned char* p_src, unsigned int length)
{
	int i, j, k;
	unsigned char *pSrc = p_src;
	unsigned char *pDest = p_dest;

  if (length > TEST_SIZE_OF_SDRAM)
	return -1;

// printf(" ... memcpy() start...");
 memcpy(pDest, pSrc, length);
// printf("Done\n");

 j = 0;

 for ( i = 0; i < length; i++) {
  if (pDest[i] != pSrc[i]) {
//    printf("data error in pDest[%d], 0x%x[%d] ... pSrc[%d], 0x%x[%d]\n", i, (pDest + i), pDest[i], i, (pSrc + i), pSrc[i]);
  	j++;
  }
    } /* for */

// printf("dest_addr src_addr size : 0x%x 0x%x %d\n\n", pDest, pSrc, length);
//  if ( j == 0)
//  printf("memory test ok!\n\n");
//    else
    	if (j != 0)
    	{
  		printf("src_addr: 0x%x dest_addr: 0x%x size: %d.\n", pSrc, pDest, length);
  // printf("memory test failed!\n\n");
  // while(1);
	}


	return j;
}

int rt2880_memory_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	 unsigned char *src,*dest;
	 u32 count = 0;
	 int choice;

	 switch (argc) {
   		case 1:
 		    printf("memtest usage :\n%s\n", cmdtp->usage);
		    break;

		case 2:
		     count = simple_strtoul(argv[1], NULL, 10);
		     printf("\n Random Test [%d] times \n",count);
		     ram_test_random(count, SDRAM_BASE_MEMTEST);
		     break;
		case 3:
		    choice = simple_strtoul(argv[1], NULL, 16);
		    count = simple_strtoul(argv[2], NULL, 10);
		    if (choice == 0) {
			  printf("\n error \n");
		    } else if (choice == 1) {
			  /* test sdram */
			  ram_test_random(count, SDRAM_BASE_MEMTEST);
		    }
		    else if (choice == 2)
		    {
			  /* test sram */
			  ram_test_random(count, SRAM_BASE_MEMTEST);
 		    }
		    break;
		case 4:
			 src = simple_strtoul(argv[1], NULL, 16);
			 dest = simple_strtoul(argv[2], NULL, 16);
			 count = simple_strtoul(argv[3], NULL, 10);
			 random_memcpy(dest,src,count);
			 break;
		default:
			 printf("memtest input cmd error!");
			 break;

	};

	return 0;
}

U_BOOT_CMD(
 	memtest,	4,	1,	rt2880_memory_test,
 	"memtest   - Ralink memory test !!\n",
 	"memtest [Run Count]  -  Memory test with random's addr and random's size !!\n"
	"memtest [src address] [dest address] [test size]-  Memory test with assign's addr and assign's size !!\n"


);

#endif // RALINK_MEMORY_TEST_FUN //


#ifdef RT2880_U_BOOT_CMD_OPEN 
#if defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD)
void rt3883_init_gdma(int mode)
{
	u32 reg;
	u16 tmp;
	//mode 0: all pkts to cpu,
	if (mode == 0) {
		reg = RALINK_REG(GDMA1_FWD_CFG);
		reg &= (GDM_UFRC_P_CPU & GDM_BFRC_P_CPU & GDM_MFRC_P_CPU & GDM_OFRC_P_CPU);
		RALINK_REG(GDMA1_FWD_CFG)=cpu_to_le32((u32)reg);

		reg = RALINK_REG(GDMA2_FWD_CFG);
		reg &= (GDM_UFRC_P_CPU & GDM_BFRC_P_CPU & GDM_MFRC_P_CPU & GDM_OFRC_P_CPU);
		RALINK_REG(GDMA2_FWD_CFG)=cpu_to_le32((u32)reg);
	}
	//mode 1: ge1->ge2, ge2->ge1
	else if (mode == 1) {
		reg = RALINK_REG(GDMA1_FWD_CFG);
		reg &= (GDM_UFRC_P_CPU & GDM_BFRC_P_CPU & GDM_MFRC_P_CPU & GDM_OFRC_P_CPU);
		reg |= (GDM_UFRC_P_GDMA2 | GDM_BFRC_P_GDMA2 | GDM_MFRC_P_GDMA2 | GDM_OFRC_P_GDMA2);
		RALINK_REG(GDMA1_FWD_CFG)=cpu_to_le32((u32)reg);

		reg = RALINK_REG(GDMA2_FWD_CFG);
		reg &= (GDM_UFRC_P_CPU & GDM_BFRC_P_CPU & GDM_MFRC_P_CPU & GDM_OFRC_P_CPU);
		reg |= (GDM_UFRC_P_GDMA1 | GDM_BFRC_P_GDMA1 | GDM_MFRC_P_GDMA1 | GDM_OFRC_P_GDMA1);
		RALINK_REG(GDMA2_FWD_CFG)=cpu_to_le32((u32)reg);
	}

	printf("rt3883_init_gdma rt2880_pdev->enetaddr: [%02X:%02X:%02X:%02X:%02X:%02X]\n",
		rt2880_pdev->enetaddr[0],rt2880_pdev->enetaddr[1],rt2880_pdev->enetaddr[2],
		rt2880_pdev->enetaddr[3],rt2880_pdev->enetaddr[4],rt2880_pdev->enetaddr[5]);

	//also set GDMA my MAC
	tmp = (u16)rt2880_pdev->enetaddr[0];
	reg = (tmp << 8) | rt2880_pdev->enetaddr[1];
	RALINK_REG(GDMA1_MAC_ADRH)=reg;

	tmp = (u16)rt2880_pdev->enetaddr[2];
	reg = (tmp << 8) | rt2880_pdev->enetaddr[3];
	reg = reg << 16;
	tmp = (u16)rt2880_pdev->enetaddr[4];
	//reg |= (tmp<<8) | rt2880_pdev->enetaddr[5];
	reg |= (tmp<<8) | 1;
	RALINK_REG(GDMA1_MAC_ADRL)=reg;

	tmp = (u16)rt2880_pdev->enetaddr[0];
	reg = (tmp << 8) | rt2880_pdev->enetaddr[1];
	RALINK_REG(GDMA2_MAC_ADRH)=reg;

	tmp = (u16)rt2880_pdev->enetaddr[2];
	reg = (tmp << 8) | rt2880_pdev->enetaddr[3];
	reg = reg << 16;
	tmp = (u16)rt2880_pdev->enetaddr[4];
	//reg |= (tmp<<8) | rt2880_pdev->enetaddr[5];
	reg |= (tmp<<8) | 2;
	RALINK_REG(GDMA2_MAC_ADRL)=reg;

	//enable auto polling for both GE1 and GE2
	reg = RALINK_REG(MDIO_CFG); 
	reg |= 0x20000000;
	RALINK_REG(MDIO_CFG)=reg;

#define MDIO_CFG2           RALINK_FRAME_ENGINE_BASE + 0x18
	reg = RALINK_REG(MDIO_CFG2);
	reg |= 0x20000000;
	RALINK_REG(MDIO_CFG2)=reg;
}

void rt3883_reset_phy(void)
{
	//Marvell phy: adj skew and reset both phy connected to ge1 and ge2
	mii_mgr_write(31, 20, 0x0ce0);
#ifdef RT3883_FPGA_BOARD
	mii_mgr_write(31, 9, 0);
#endif
	mii_mgr_write(31, 0, 0x9140);
	mii_mgr_write(30, 20, 0x0ce0);
#ifdef RT3883_FPGA_BOARD
	mii_mgr_write(30, 9, 0);
#endif
	mii_mgr_write(30, 0, 0x9140);
}

int do_rt3883_cpuloopback(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("RT3883 CPU loopback mode!\n");
	eth_init(NULL);
	eth_loopback_mode = 1;
	rt3883_init_gdma(0);
	rt3883_reset_phy();
	rt2880_eth_recv(rt2880_pdev);
	return 0;
}

U_BOOT_CMD(
	cpuloop,	1,	1,	do_rt3883_cpuloopback,
	"cpuloop   - RT3883 CPU loopback test\n",
	"cpuloop   - RT3883 CPU loopback test\n"
);

int do_rt3883_pseloopback(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("RT3883 PSE loopback mode!\n");
	rt3883_init_gdma(1);
	rt3883_reset_phy();
	return 0;
}

U_BOOT_CMD(
	pseloop,	1,	1,	do_rt3883_pseloopback,
	"pseloop   - RT3883 PSE loopback test\n",
	"pseloop   - RT3883 PSE loopback test\n"
);
#endif

int do_eth_loopback(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (eth_loopback_mode) {
		eth_loopback_mode=0;
		printf("\n Set to Normal mode ! \n");
	}
	else {
		eth_loopback_mode=1;
		printf("\n Set to LoopBack mode ! \n");
	}
	return 0;
}

U_BOOT_CMD(
	loopback,	1,	1,	do_eth_loopback,
	"loopback   - Ralink eth loopback test !!\n",
	"kaiker,loopback   - Ralink eth loopback test !!\n"
);
#endif


#ifdef RALINK_GDMA_STATUS_DISPLAY_FUN
int rt2880_debug_show(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	kaiker_debug_show(rt2880_pdev);
	STOP_ETH(rt2880_pdev);
	puts(" STOP_ETH \n");
	return 0;
}

U_BOOT_CMD(
 	sdd,	1,	1,	rt2880_debug_show,
 	"sdd   - Display to all DMA status  !!\n",
 	"kaiker,sdd   - !!\n"
);
#endif // RALINK_GDMA_STATUS_DISPLAY_FUN //


#ifdef RALINK_PCI_HOST_TEST_FUN
/*
	FUNCTION: ConvertCharToHex
	PURPOSE:   Convert string to hex
*/

void ConvertCharToHex(char *s,u32 *d1)
{
	int i;
	char chr;

	*d1=0;
	for (i=0; i < strlen(s); i++)
	{
        *d1 = *d1 * 16;
        chr = s[i];
        if (chr >= '0' && chr <= '9')
            *d1 += chr - '0';
        else if (chr >= 'a' && chr <= 'f')
            *d1 += chr - 'a' + 10;
        else if (chr >= 'A' && chr <= 'F')
            *d1 += chr - 'A' + 10;

	}


}

static inline u32 byte_transpose(int n,u32 v)
{
	switch(n)
	{
		case 0:
		v= v & 0x000000FF;
		return v;
		case 1:
		v= v & 0x000000FF;
		v = v << 8;
		return v;
		case 2:
		v= v & 0x000000FF;
		v = v << 16;
		return v;
		case 3:
		v= v & 0x000000FF;
		v = v << 24;
		return v;
	}
}

static inline u8 byte_select(int n,u32 v)
{
	switch(n)
	{
		case 0:
		v= v & 0x000000FF;
		return (u8)v;
		case 1:
		v = v & 0x0000FF00;
		v = v >> 8;
		return (u8)v;
		case 2:
		v = v & 0x00FF0000;
		v = v >> 16;
		return (u8)v;
		case 3:
		v = v & 0xFF000000;
		v = v >> 24;
		return (u8)v;
	}
}
static inline u32 halfword_transpose(int n,u32 v)
{
	switch(n)
	{
		case 0:
		v= v & 0x0000FFFF;
		return v;

		case 2:
		v= v & 0x0000FFFF;
		v = v << 16;
		return v;

	}
}
static inline u16 halfword_select(int n,u32 v)
{
	switch(n)
	{
		case 0:
		v= v & 0x0000FFFF;
		return (u16)v;
		case 2:
		v = v & 0xFFFF0000;
		v = v >> 16;
		return (u16)v;

	}
}
static inline u32 halfword_mask(int n,u32 v)
{
	switch(n)
	{
		case 0:
		v= v & 0xFFFF0000;
		break;
		case 2:
		v = v & 0x0000FFFF;

		break;

	}
	return v;
}

static inline u32 byte_mask(int n,u32 v)
{
	switch(n)
	{
		case 0:
		v= v & ~0x000000FF;
		break;
		case 1:
		v = v & ~0x0000FF00;

		break;
		case 2:
		v = v & ~0x00FF0000;

		break;
		case 3:
		v = v & ~0xFF000000;

		break;
	}
	return v;
}


//---------------------------------------------------------------------------------------------
// Function Name: MemBaseTest_8139_Byte
// Description: Byte read
//---------------------------------------------------------------------------------------------
#ifdef RT2880_PCI_0310
int MemBaseTest_8139_Byte(u32 iobase,u32 offset,u8 RW,u8 BW)
{
    u8  RTL8139_value_b,ii,fail_num=0;
    u32  WriteValue,ReadValue;
    u16 halfwordvalue;
    u32 WriteValue_dw;
    u32 TestValue;
    u32 old_value,new_value,reg_ar;
    u16 RTL8139_value_w;
    u32 RTL8139_value_Dw;
	u32 memwinbase = PCI_ALLOCATE_SPACE;
    char  writestr[10];
    int byte_offset,halfword_offset;
    int ReturnValue;
	u8 byte;

    TestValue=0x5A5A00BC;

    //Test Register TSAD0~3


	printf("\n iobase = 0x%08X \n",iobase);

	printf("\n old,offset = 0x%X\n",offset);

	memwinbase = memwinbase + ((offset >> 5) << 5);

	printf("\n memwinbase = 0x%08X \n",memwinbase);



	PCI_MEMBASE = memwinbase;
	//PCI_IOBASE = memwinbase;

	udelay(50000);

	offset = offset % 32;
	byte_offset = offset & 0x3;
	halfword_offset = offset & 0x2;
	offset = offset & 0xFFFFFFFC;

	printf("\n new,offset = 0x%X\n",offset);

	switch(BW)
	{
	case 1: /*byte size read/write*/
		/*-----------------------*/

		switch(RW)
		{
		case 1:/*Byte Read/Write*/

	       ReadValue = PCI_MEMWIN(offset);
			byte = byte_select(byte_offset,ReadValue);
		   	printf("\n kaiker,phy addr=%08X, Read value=%02X\n",memwinbase+offset ,byte);
		      	break;
		case 2:/*Byte write*/

			printf("\n input write value:");

			input_value(writestr);
			ConvertCharToHex(writestr,&WriteValue);

			WriteValue = byte_transpose(byte_offset,WriteValue);
			printf("\n WriteValue = %08X ,after byte_transpose\n",WriteValue);
            ReadValue = PCI_MEMWIN(offset);
			printf("\n ReadValue = %08X\n",ReadValue);
			ReadValue = byte_mask(byte_offset,ReadValue);

			printf("\n ReadValue= %08X ,after byte_mask\n",ReadValue);
			WriteValue = WriteValue | ReadValue;
			printf("\n willing WriteValue= %08X ,after byte_mask\n",WriteValue);

			PCI_MEMWIN(offset) = WriteValue;
			udelay(5000);
			ReadValue = PCI_MEMWIN(offset);
			byte = byte_select(byte_offset,ReadValue);

	      	printf("\n,phy addr=%08X,Write  value=%02X\n",memwinbase+offset,byte);
		break;

		default:
			return 0;
		}
		/*-----------------------*/
	break;/* case 1  Byte Read/Write*/

	case 2: /*word size read/write*/
		/*-----------------------*/
		switch(RW)
		{
		case 1:/*Word Read*/
			   ReadValue = PCI_MEMWIN(offset);
	      		ReadValue = halfword_select(halfword_offset,ReadValue);
		      	printf("\nReg addr %8x,Read HalfWord value=%04x\n",reg_ar,ReadValue);
		      	break;
		case 2:/*Word write*/
			printf("\n input write value:");

			input_value(writestr);
			ConvertCharToHex(writestr,&WriteValue);

			WriteValue = halfword_transpose(halfword_offset,WriteValue);
			printf("\n WriteValue = %08X ,after halfword_transpose\n",WriteValue);
            ReadValue = PCI_MEMWIN(offset);
			printf("\n ReadValue = %08X\n",ReadValue);
			ReadValue = halfword_mask(halfword_offset,ReadValue);

			printf("\n ReadValue= %08X ,after halfword_mask\n",ReadValue);
			WriteValue = WriteValue | ReadValue;
			printf("\n willing WriteValue= %08X ,after byte_mask\n",WriteValue);

			PCI_MEMWIN(offset) = WriteValue;
			udelay(5000);
			ReadValue = PCI_MEMWIN(offset);

			halfwordvalue = halfword_select(halfword_offset,ReadValue);


	      	printf("\n,phy addr=%08X,Write  value=%04X\n",memwinbase+offset,halfwordvalue);
		break;

		default:
			return 0;
		}
	/*-----------------------*/
	break;/*case 2 Dword size read/write*/

	case 3:/*Double word size read/write*/
		/*-----------------------*/
		switch(RW)
		{
		case 1:/*DWord Read*/
			ReadValue = PCI_MEMWIN(offset);

		   	printf("\n kaiker,phy addr=%08X, Read value=%08X\n",memwinbase+offset ,ReadValue);
		   	break;
		case 2:/*DWord write*/
			printf("\n input write value:");

			input_value(writestr);
			ConvertCharToHex(writestr,&WriteValue);
			printf("\n writestr = %08X \n",WriteValue);



			PCI_MEMWIN(offset) = WriteValue;
			udelay(5000);
			ReadValue = PCI_MEMWIN(offset);

	      	printf("\n,phy addr=%08X,Write  value=%08X\n",memwinbase+offset,ReadValue);
		break;

		default:
			return 0;
		}
	/*-----------------------*/
	break;/*Double word size read/write */

	}

}
#else // non RT2880_PCI_0310 //
int MemBaseTest_8139_Byte(u32 iobase,u32 offset,u8 RW,u8 BW)
{
    u8  RTL8139_value_b,ii,fail_num=0;
    u32  WriteValue,ReadValue;
    u16 halfwordvalue;
    u32 WriteValue_dw;
    u32 TestValue;
    u32 old_value,new_value,reg_ar;
    u16 RTL8139_value_w;
    u32 RTL8139_value_Dw;

    char  writestr[10];
    int byte_offset,halfword_offset;
    int ReturnValue;
	u8 byte;

    TestValue=0x5A5A00BC;







	udelay(50000);


	byte_offset = offset & 0x3;
	halfword_offset = offset & 0x2;
	offset = offset & 0xFFFFFFFC;

	printf("\n new,offset = 0x%X\n",offset);

	switch(BW)
	{
	case 1: /*byte size read/write*/
		/*-----------------------*/

		switch(RW)
		{
		case 1:/*Byte Read/Write*/

	       ReadValue = PCI_MEMWIN(offset);
			byte = byte_select(byte_offset,ReadValue);
		   	printf("\n kaiker,Read value=%02X\n",byte);
		      	break;
		case 2:/*Byte write*/

			printf("\n input write value:");

			input_value(writestr);
			ConvertCharToHex(writestr,&WriteValue);

			WriteValue = byte_transpose(byte_offset,WriteValue);
			printf("\n WriteValue = %08X ,after byte_transpose\n",WriteValue);
            ReadValue = PCI_MEMWIN(offset);
			printf("\n ReadValue = %08X\n",ReadValue);
			ReadValue = byte_mask(byte_offset,ReadValue);

			printf("\n ReadValue= %08X ,after byte_mask\n",ReadValue);
			WriteValue = WriteValue | ReadValue;
			printf("\n willing WriteValue= %08X ,after byte_mask\n",WriteValue);

			PCI_MEMWIN(offset) = WriteValue;
			udelay(5000);
			ReadValue = PCI_MEMWIN(offset);
			byte = byte_select(byte_offset,ReadValue);

	      	printf("\n,Write value=%02X\n",byte);
		break;

		default:
			return 0;
		}
		/*-----------------------*/
	break;/* case 1  Byte Read/Write*/

	case 2: /*word size read/write*/
		/*-----------------------*/
		switch(RW)
		{
		case 1:/*Word Read*/
			   ReadValue = PCI_MEMWIN(offset);
	      		ReadValue = halfword_select(halfword_offset,ReadValue);
		      	printf("\nRead HalfWord value=%04x\n",ReadValue);
		      	break;
		case 2:/*Word write*/
			printf("\n input write value:");

			input_value(writestr);
			ConvertCharToHex(writestr,&WriteValue);

			WriteValue = halfword_transpose(halfword_offset,WriteValue);
			printf("\n WriteValue = %08X ,after halfword_transpose\n",WriteValue);
            ReadValue = PCI_MEMWIN(offset);
			printf("\n ReadValue = %08X\n",ReadValue);
			ReadValue = halfword_mask(halfword_offset,ReadValue);

			printf("\n ReadValue= %08X ,after halfword_mask\n",ReadValue);
			WriteValue = WriteValue | ReadValue;
			printf("\n willing WriteValue= %08X ,after byte_mask\n",WriteValue);

			PCI_MEMWIN(offset) = WriteValue;
			udelay(5000);
			ReadValue = PCI_MEMWIN(offset);

			halfwordvalue = halfword_select(halfword_offset,ReadValue);


	      	printf("\nWrite  value=%04X\n",halfwordvalue);
		break;

		default:
			return 0;
		}
	/*-----------------------*/
	break;/*case 2 Dword size read/write*/

	case 3:/*Double word size read/write*/
		/*-----------------------*/
		switch(RW)
		{
		case 1:/*DWord Read*/
			ReadValue = PCI_MEMWIN(offset);

		   	printf("\n Read value=%08X\n",ReadValue);
		   	break;
		case 2:/*DWord write*/
			printf("\n input write value:");

			input_value(writestr);
			ConvertCharToHex(writestr,&WriteValue);
			printf("\n writestr = %08X \n",WriteValue);



			PCI_MEMWIN(offset) = WriteValue;
			udelay(5000);
			ReadValue = PCI_MEMWIN(offset);

	      	printf("\n,Write  value=%08X\n",ReadValue);
		break;

		default:
			return 0;
		}
	/*-----------------------*/
	break;/*Double word size read/write */

	}

}
#endif // RT2880_PCI_0310 //

#endif // RALINK_PCI_HOST_TEST_FUN //

void input_value(u8 *str)
{
	if (str)
		strcpy(console_buffer, str);
	else
		console_buffer[0] = '\0';
	while(1)
	{
		if (readline ("==:", 1) > 0)
		{
			strcpy (str, console_buffer);
			break;
		}
		else
			break;
	}
}

#ifdef RALINK_PCI_HOST_TEST_FUN
#define PCI_BRIDGE_CONFIG_ADDR le32_to_cpu(*(volatile u_long *)(RALINK_PCI_BASE + 0x0020))
#define PCI_BRIDGE_CONFIG_DATA le32_to_cpu(*(volatile u_long *)(RALINK_PCI_BASE + 0x0024))
#define PCI_BRIDGE_CLOCK le32_to_cpu(*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0030))
#define PCI_MEMBASE le32_to_cpu(*(volatile u_long *)(RALINK_PCI_BASE + 0x0028))
#define PCI_IOBASE le32_to_cpu(*(volatile u_long *)(RALINK_PCI_BASE + 0x002C))
#define PCI_MEMWIN(offset) le32_to_cpu(*(volatile u_long *)(RALINK_PCI_BASE + 0x10000 + offset))
#define PCI_IOWIN(offset) le32_to_cpu(*(volatile u_long *)(RALINK_PCI_BASE + 0x20000+ offset))
#define PCI_PCICFG_REG le32_to_cpu(*(volatile u_long *)RALINK_PCI_BASE)
#define PCI_ARBCTL_REG le32_to_cpu(*(volatile u_long *)(RALINK_PCI_BASE + 0x0080))

int rt2880_pci_scan(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32       ii,jj,aa,kk,kk1,mem_ar,io_ar,offset,reg_offset,setvalue,content;
     u32       cpsr_flags,slot_num,Item;
    char soffset[3],RWselect,BWDselect,str_numeral[3],str_setvalue[10];
	static char lastcommand[CFG_CBSIZE] = { 0, };
    u16 status;
	int len;


	PCI_PCICFG_REG = 0;
	PCI_ARBCTL_REG = 0x79;
	PCI_MEMBASE = PCI_ALLOCATE_SPACE;
	//PCI_IOBASE = memwinbase;

	kaiker_PCI_Scan_Bus_Test();

	//printf("\n PCIRAW = 0x%08X \n",*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004));
	//*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004) = 0xFFFFFFFF;
	 do
    {
    RETURN_TO_MAIN_MENU:
        printf("------ Test AHB-PCI Bridge ------\n");

        printf("<1>.Two RTL8139D NIC cards Ping Pong Test \n");
     	printf("<2>.Read/Write  REG test \n");
        printf("<0>.Exit...\n");
        printf("*** Please Input the Item <0>~<4>:");

       while((Item = getc())== 0);
        switch(Item)
        {
         /*
               case 1:
                kaiker_PCI_Bridge_Test();
     	        break;

                case 2:
                kaiker_PCI_Scan_Bus_Test();
     		break;

            case 2:
                kaiker_PCI_InitPCIDevice();
     		break;
     	*/



	case '2':

		/*
	 	Ulw.lw = sys_read_pci_config_word(Finded_PCIDeviceID[0].PCIDeviceID, 0x3C);         // read back value
	        printf("\n\n******8139 num1= 0x%02x:0x%02x:0x%02x:0x%02x: ***********\n",Ulw.lw_u8[3],Ulw.lw_u8[2],Ulw.lw_u8[1],Ulw.lw_u8[0]);

	     	Ulw.lw = sys_read_pci_config_word(Finded_PCIDeviceID[1].PCIDeviceID, 0x3C);         // read back value
	        printf("\n\n******8139 num2= 0x%02x:0x%02x:0x%02x:0x%02x: ***********\n",Ulw.lw_u8[3],Ulw.lw_u8[2],Ulw.lw_u8[1],Ulw.lw_u8[0]);

	     	Ulw.lw = sys_read_pci_config_word(Finded_PCIDeviceID[2].PCIDeviceID, 0x3C);         // read back value
	        printf("\n\n******PCI bridge= 0x%02x:0x%02x:0x%02x:0x%02x: ***********\n",Ulw.lw_u8[3],Ulw.lw_u8[2],Ulw.lw_u8[1],Ulw.lw_u8[0]);
     		break;
     		*/
     		 /***************************************/

		GO_BACK:
		printf("\nSelect Device 1.%08x 2.%08x 3.%08x:",Finded_PCIDeviceID[0].dev_ven,Finded_PCIDeviceID[1].dev_ven,Finded_PCIDeviceID[2].dev_ven);
		  while((ii = getc())== 0);
		if(ii> '3')
		{
			printf("\nDevice select error !!\n");
			break;

		}
		ii = ii - 0x30;

                mem_ar = sys_read_pci_config_word(Finded_PCIDeviceID[ii-1].PCIDeviceID, PCI_CSH_BASE_ADDR_REG+0 ); //read back base addr

                io_ar=sys_read_pci_config_word(Finded_PCIDeviceID[ii-1].PCIDeviceID, PCI_CSH_BASE_ADDR_REG+4 ); //read back base addr
		printf("\nSelect  Space 1.MEMORY 2.IO 3.Configuration :");
		 while((jj = getc())== 0);

		if(jj=='1')
		{/*Select memory space*/
			if(mem_ar & 0x00000001)/*BAR0 is IO space*/
				mem_ar=io_ar;
		}
		else if(jj=='2')
		{/*Selece IO space*/
			if(io_ar & 0x00000001)/*BAR1 is IO space*/
				mem_ar=io_ar;

		}
		else if(jj=='3')
		{
		while(1)
		{
		printf("\nSelect Register number and input read /write value\n");
		printf("\nEx: The command register that write 03H: 4w3\n");
		printf("\nEx:  Read Status register :  6r \n");
		printf("\n [ q ] go back  \n");
		printf("\n [ exit ] return to main menu  \n");
		printf(" \n :");
		//scanf("%s",&lastcommand);

		while(1)
		{
			len = readline (":", 0);

			if (len > 0)
			{
				strcpy (lastcommand, console_buffer);
				break;
			}

		}

		if(lastcommand[0]=='q')
		goto GO_BACK;
		if(strcmp(lastcommand,"exit")==0)
		goto RETURN_TO_MAIN_MENU;
		for(kk=0,kk1=0;lastcommand[kk]!=0;kk++)
		{

			if(kk1>=3)
				break;
			if((lastcommand[kk]>=0x30 && lastcommand[kk]<=0x39)||(lastcommand[kk]>='a'  && lastcommand[kk]<='f')||(lastcommand[kk]>='A'  && lastcommand[kk]<='F'))
			{
				str_numeral[kk1]=lastcommand[kk];
				str_numeral[kk1+1]=0;
				kk1++;
			}


			if(lastcommand[kk]=='r' || lastcommand[kk]=='R')
			{
				ConvertCharToHex(str_numeral,&reg_offset);
				content=sys_read_pci_config_word(Finded_PCIDeviceID[ii-1].PCIDeviceID, reg_offset); //read back base addr
				printf("\n DEV:%08X ,reg 0x%02x,value:0x%08x\n",Finded_PCIDeviceID[ii-1].PCIDeviceID,reg_offset,content);

				break;
			}
			else if(lastcommand[kk]=='w' || lastcommand[kk]=='W')
			{
				kk++;
				for(kk1=0;lastcommand[kk]!=0;kk++)
				{
					if(kk1>=8)
					break;
					if((lastcommand[kk]>=0x30 && lastcommand[kk]<=0x39)||(lastcommand[kk]>='a'  && lastcommand[kk]<='f')||(lastcommand[kk]>='A'  && lastcommand[kk]<='F'))
					{
						str_setvalue[kk1]=lastcommand[kk];
						str_setvalue[kk1+1]=0;
						kk1++;
					}
					else
					{
						printf("\n syntax error !!\n");
						break;
					}

				}
				ConvertCharToHex(str_numeral,&reg_offset);
				ConvertCharToHex(str_setvalue,&setvalue);

				if(reg_offset==4||reg_offset==6)
				sys_write_pci_config_halfword(Finded_PCIDeviceID[ii-1].PCIDeviceID,reg_offset,setvalue);
				else
				sys_write_pci_config_word(Finded_PCIDeviceID[ii-1].PCIDeviceID,reg_offset,setvalue);


				content=sys_read_pci_config_word(Finded_PCIDeviceID[ii-1].PCIDeviceID, reg_offset); //read back base addr
				printf("\n DEV:%08X ,reg 0x%02x,value:0x%08x\n",Finded_PCIDeviceID[ii-1].PCIDeviceID,reg_offset,content);
				break;
			}



		}


			}
		}
		else
		break;



		mem_ar = (mem_ar&0xfffffffe);//bit0:IO/MEM indicator
			while(1)
				{
				printf("\nInput Register offset (input 6. exit):");

				/*Convert HEX char to  decimal*/
				//scanf("%s",&soffset);

				input_value(soffset);


				ConvertCharToHex(soffset,&offset);
				if(offset==6)
				break;

				printf("\n Input 1.Read or 2.Write:");
				//scanf("%d",&RWselect);
				//input_value(&RWselect);
			    //while((RWselect = getc())== 0);

				PCI_WAIT_INPUT_CHAR(RWselect);
				RWselect -= 0x30;



				printf("\n Input  1.Byte or 2.Word or 3.DWord:");
				//scanf("%d",&BWDselect);
				PCI_WAIT_INPUT_CHAR(BWDselect);
				BWDselect -= 0x30;

				//BWDselect =1;
				MemBaseTest_8139_Byte(mem_ar,offset,RWselect,BWDselect);
#if 0
				for(aa=0;Finded_PCIDeviceID[aa].PCIDeviceID.Enable==1;aa++)
				{
					if(Finded_PCIDeviceID[aa].dev_ven==PCI_BRIDGE_DEVICE_VENDOR_ID)
					{
						status=sys_read_pci_config_halfword(Finded_PCIDeviceID[aa].PCIDeviceID,PCI_CSH_STATUS_REG);

						if((status & PCI_STATUS_PARITY_ERROR))
						printf("\nThe Bridge[0x%08x] is Parity error !!\n",Finded_PCIDeviceID[aa].dev_ven);
						if((status & PCI_STATUS_SERR_ERROR))
						printf("\nThe Bridge[0x%08x] of SERR# is Enable !!\n",Finded_PCIDeviceID[aa].dev_ven);
						if((status & PCI_STATUS_R_MASTER_ABORT))
						printf("\n The Bridge[0x%08x] is Received master abort as master !!\n",Finded_PCIDeviceID[aa].dev_ven);
						if((status & PCI_STATUS_R_TARGET_ABORT))
						printf("\n The Bridge[0x%08x] is Received Target abort as master !!\n",Finded_PCIDeviceID[aa].dev_ven);
						if((status & PCI_STATUS_S_TARGET_ABORT))
						printf("\n The Bridge[0x%08x] is Signaled Target abort as target !!\n",Finded_PCIDeviceID[aa].dev_ven);
						if((status & PCI_STATUS_MASTER_PARITY_ERROR))
						printf("\nThe Bridge[0x%08x] is  Master data parity error !!\n",Finded_PCIDeviceID[aa].dev_ven);

					}

				}

				status=sys_read_pci_config_halfword(Finded_PCIDeviceID[ii].PCIDeviceID,PCI_CSH_STATUS_REG);

				if((status & PCI_STATUS_PARITY_ERROR))
					printf("\n The target[0x%08x] is Parity error !!\n",Finded_PCIDeviceID[ii-1].dev_ven);
				if((status & PCI_STATUS_SERR_ERROR))
					printf("\n The target[0x%08x] of SERR# is Enable !!\n",Finded_PCIDeviceID[ii-1].dev_ven);
				if((status & PCI_STATUS_R_MASTER_ABORT))
					printf("\nThe target[0x%08x] is Received master abort as master !!\n",Finded_PCIDeviceID[ii-1].dev_ven);
				if((status & PCI_STATUS_R_TARGET_ABORT))
					printf("\nThe target[0x%08x] is Received Target abort as master !!\n",Finded_PCIDeviceID[ii-1].dev_ven);
				if((status & PCI_STATUS_S_TARGET_ABORT))
					printf("\nThe target[0x%08x] is Signaled Target abort as target !!\n",Finded_PCIDeviceID[ii-1].dev_ven);
				if((status & PCI_STATUS_MASTER_PARITY_ERROR))
					printf("\nThe target[0x%08x] is Master data parity error !!\n",Finded_PCIDeviceID[ii-1].dev_ven);
#endif
				}
			break;
                     /***************************************/
            default:
                printf("Exit Aridge Test ..\n");
                return;
        }
    }while (Item!=0);
	 return 0;
}

U_BOOT_CMD(
 	pci,	1,	1,	rt2880_pci_scan,
 	"pci   -  Scan PCI bus slot and read/write Configuration space  !!\n",
 	"Ralink function create by kaiker   - !!\n"
);
#endif // RALINK_PCI_HOST_TEST_FUN //


#ifdef RALINK_GDMA_SCATTER_TEST_FUN
int set_scatter_len(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 regValue;

	rt2880_hdrlen = simple_strtoul(argv[1], NULL, 10);

	if(rt2880_hdrlen == 999)
	{
		force_queue_n = 1;
		printf("\n Force tx DMA to Queue1 \n");
		return 0;
	}
	else if(rt2880_hdrlen == 888)
	{
		force_queue_n = 0;
		printf("\n Force tx DMA to Queue0 \n");
		return 0;
	}
	else if(rt2880_hdrlen == 777)
	{
		force_queue_n = 3;
		printf("\n Force tx DMA is Disable !! \n");
		return 0;
	}

   while(1)
	{
		regValue = RALINK_REG(PDMA_GLO_CFG);
		if((regValue & RX_DMA_BUSY))
		{
			printf("\n  RX_DMA_BUSY !!! ");
			continue;
		}
		if((regValue & TX_DMA_BUSY))
		{
			printf("\n  TX_DMA_BUSY !!! ");
			continue;
		}
		break;
	}

	 regValue=RALINK_REG(PDMA_GLO_CFG);
    udelay(100);
	regValue &= 0x0000FFFF;
	regValue |= (rt2880_hdrlen << 16);
	RALINK_REG(PDMA_GLO_CFG)=regValue;
	udelay(500);
	regValue=RALINK_REG(PDMA_GLO_CFG);
	printf("\n Set Scatter Length = %d \n",rt2880_hdrlen);
	printf("\n PDMA_GLO_CFG=%08X \n",regValue);
	return 0;
}

U_BOOT_CMD(
 	hdrlen,	2,	2,	set_scatter_len,
 	"hdrlen   - Specify the header segment size in byte !!\n",
 	"hdrlen [size]  - Specify the header segment size in byte !!\n"
);
#endif // RALINK_GDMA_SCATTER_TEST_FUN //


#ifdef RALINK_CACHE_STATE_DETECT_FUNC
static inline void mips_cache_set(u32 v)
{
	asm volatile ("mtc0 %0, $16" : : "r" (v));
}

int set_cache_state(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 kk;
	kk = simple_strtoul(argv[1], NULL, 10);

	mips_cache_set(kk);

	switch(kk)
	{
		case 0:
			printf("\n Set to CONF_CM_CACHABLE_NO_WA \n");
		break;
		case 1:
			printf("\n Set to CONF_CM_CACHABLE_WA \n");
		break;
		case 2:
			printf("\n Set to CONF_CM_UNCACHED \n");
		break;
		case 3:
			printf("\n Set to CONF_CM_CACHABLE_NONCOHERENT \n");
		break;
		case 4:
			printf("\n Set to CONF_CM_CACHABLE_CE \n");
		break;
	};
	return 0;
}

U_BOOT_CMD(
 	cache_set,	2,	2,	set_cache_state,
 	"cache_set   - Specify the header segment size in byte !!\n",
 	"cache_set [value]  - CONF_CM_CACHABLE_NO_WA 0 \n CONF_CM_CACHABLE_WA 1 \nCONF_CM_UNCACHED	2\nCONF_CM_CACHABLE_NONCOHERENT	3\nCONF_CM_CACHABLE_CE 4 \n"
);
#endif // RALINK_CACHE_STATE_DETECT_FUNC //


#ifdef RALINK_SEGMENT_SIZE_ALIGN_TEST_FUNC
int rt2880_sdpx_16n_plus_x(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc != 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return ;
	}

	if(!memcmp(argv[0],"sdp.0",sizeof("sdp.0")))
	{
		sdp0_alig_16n_x = simple_strtoul(argv[1], NULL, 10);
		printf("\n Set SDP0 alignment to 16N+%d\n",sdp0_alig_16n_x);
	}
	else if(!memcmp(argv[0],"sdp.1",sizeof("sdp.1")))
	{
		sdp1_alig_16n_x = simple_strtoul(argv[1], NULL, 10);
		printf("\n Set SDP1 alignment to 16N+%d\n",sdp1_alig_16n_x);
	}
	return 0;
}

U_BOOT_CMD(
 	sdp,	2,	2,	rt2880_sdpx_16n_plus_x,
 	"sdp   - \n",
 	"sdp.0  x  x:[0,4,8,12]  - set SDP0 alignment to 16N+x  \n"
 	"sdp.1  x  x:[0,4,8,12]  - set SDP1 alignment to 16N+x  \n"
);
#endif // RALINK_SEGMENT_SIZE_ALIGN_TEST_FUNC //


#ifdef RALINK_INTERNAL_LOOPBACK_TEST_FUNC

static int ETH_EN=0;
static volatile uchar *NetTxPacket1,*NetTxPacket2,*NetTxPacket3;	/* THE transmit packet			*/




/******************************************************************************
 *
 * FUNCTION:  Gsw_Setup_Transmit_Packet
 * PURPOSE:
 *
 ******************************************************************************/
int Gsw_Setup_AllOne_Transmit_Packet(struct eth_device* dev,int len,u8 patten, u8  *ptr)
{
    u32    ii;
	u8 *temp_ptr;
   


  
	temp_ptr = ptr;
    // setup da
    ptr[0] = patten;
	ptr[1] = patten;
	ptr[2] = patten;
	ptr[3] = patten;
	ptr[4] = patten;
    ptr[5] = patten;
    
    
    ptr += 6;
    
    // setup sa
   // setup da
    ptr[0] = patten;
	ptr[1] = patten;
	ptr[2] = patten;
	ptr[3] = patten;
	ptr[4] = patten;
    ptr[5] = patten;
	
		
    ptr += 6;
    
    // setup type
    *ptr++ = patten;
    
    *ptr++ = patten;
    
    // setup payload
    for (ii = 0; ii < len; ii++)
    {
        *ptr++ = patten;//(ii + 1);
    }
	return ((int)(ptr - temp_ptr));
}




/******************************************************************************
 *
 * FUNCTION:  Gsw_Setup_Transmit_Packet
 * PURPOSE:
 *
 ******************************************************************************/
int Gsw_Setup_Transmit_Packet(struct eth_device* dev,int len,u8 patten, u8  *ptr)
{
    u32    ii;
	u8 *temp_ptr;
   


  
	temp_ptr = ptr;
    // setup da
    ptr[0] = 0x00;
	ptr[1] = 0x01;
	ptr[2] = 0x02;
	ptr[3] = 0x03;
	ptr[4] = 0x99;
    ptr[5] = patten;
    
    
    ptr += 6;
    
    // setup sa
    memset(ptr, 0, 6);
	
	memcpy(ptr, dev->enetaddr, 6);
	
		
    ptr += 6;

// inside vlan
#ifdef INSIDE_VLAN
    *ptr++ = 0x81;
    
    *ptr++ = 0x00;

	*ptr++ = 0x00;
    
    *ptr++ = 0x02;
#endif
	
    // setup type
    *ptr++ = 0x08;
    
    *ptr++ = 0x00;
    
    // setup payload
    for (ii = 0; ii < len; ii++)
    {
        *ptr++ = patten;//(ii + 1);
    }
	return ((int)(ptr - temp_ptr));
}

void STOP_ETHRX(struct eth_device *dev) {
    s32 omr; 
    omr=RALINK_REG(PDMA_GLO_CFG);
    udelay(100);
	omr &= ~(RX_DMA_EN) ;
    RALINK_REG(PDMA_GLO_CFG)=omr;
	udelay(500);
}

void rt2880_internal_loopback_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rx_cnt,tx_cnt,length,valid_sent_cnt,rand_payload;
	ulong t_start;
	uint i=0;

	printf("\n rt2880_internal_loopback_test \n");

	if (argc != 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return ;
	}

	 if(ETH_EN == 0)
    {
		is_internal_loopback_test = 1;    
		rt2880_eth_setup(rt2880_pdev);
		is_internal_loopback_test = 0;

		 t_start = get_timer(0);
	 

		printf("\n Please watting 2 Sec !!,t_start=%08X \n",t_start);
		
		while(1)
		{
			if(get_timer(t_start)>= (2 * CFG_HZ))
			{
				
				 break;
			}
    	}

		ETH_EN = 1;
	//	return;
	}

	NetTxPacket = NULL;

	if (!NetTxPacket) {
		int	i;
		/*
		 *	Setup packet buffers, aligned correctly.
		 */
		NetTxPacket = &PktBuf[0] + (PKTALIGN - 1);
		NetTxPacket -= (ulong)NetTxPacket % PKTALIGN;

		printf("\n NetTxPacket = 0x%08X \n",NetTxPacket);
		for (i = 0; i < PKTBUFSRX; i++) {
			NetRxPackets[i] = KSEG1ADDR(NetTxPacket + (i+1)*PKTSIZE_ALIGN);
			
		}
		}
	STOP_ETHRX(rt2880_pdev);
	NetTxPacket = KSEG1ADDR(NetTxPacket) ;
	NetTxPacket1 = NetRxPackets[0];
	NetTxPacket2 = NetRxPackets[1];
	NetTxPacket3 = NetRxPackets[2];
	
	length = simple_strtoul(argv[1], NULL, 10);

	rand_payload = simple_strtoul(argv[2], NULL, 10);

	if(rand_payload == 1)
	{
		printf("\n Random Payload\n");
	}
	else
	{
		printf("\n All %02X  patten\n",rand_payload);


	}

	//length = simple_strtoul(argv[3], NULL, 10);


	printf("\n length = %d \n",length);

	internal_loopback_test = INTERNAL_LOOPBACK_ENABLE;

	

#if 1  //Jerry test
while(!kaiker_button_p())
{
	if(rand_payload == 1)
	{
			
		
			length = Gsw_Setup_Transmit_Packet(rt2880_pdev,length - 14,(u8)i/*((0x30 + tx_queue_num) % 0x30)*/,NetTxPacket);

		
	 		if(rt2880_eth_send(rt2880_pdev,NetTxPacket,length) == -1)
	 		{
	 			printf("\n Internal loopback, Send packet is fail !! \n");
				break;
	 		}
			i++;
	}
	else
	{
	
		length = Gsw_Setup_AllOne_Transmit_Packet(rt2880_pdev,length - 14,rand_payload,NetTxPacket);

		
		
			if(rt2880_eth_send(rt2880_pdev,NetTxPacket,length) == -1)
		 	{
		 		printf("\n Internal loopback, Send packet is fail !! \n");
				break;
		 	}


		
	}	
}	
	
#else
	if(random_queue_act == ENABLE)
	{
		Gsw_Setup_Transmit_Packet(rt2880_pdev,length - 14,((0x30 + 0) % 0x30),NetTxPacket);	
		 Gsw_Setup_Transmit_Packet(rt2880_pdev,length - 14,((0x30 + 1) % 0x30),NetTxPacket1);	
		 Gsw_Setup_Transmit_Packet(rt2880_pdev,length - 14,((0x30 + 2) % 0x30),NetTxPacket2);	
		Gsw_Setup_Transmit_Packet(rt2880_pdev,length - 14,((0x30 + 3) % 0x30),NetTxPacket3);	
		printf("\n Created four packets for sequence queue change!  \n");
	//print_packet(NetTxPacket,length);
		//print_packet(NetTxPacket1,length);
		//print_packet(NetTxPacket2,length);
		//print_packet(NetTxPacket3,length);
	}	
	else
	{
		length = Gsw_Setup_Transmit_Packet(rt2880_pdev,length - 14,((0x30 + tx_queue_num) % 0x30),NetTxPacket);

	}

	 //
	 
	

	for(i=0;i < tx_cnt ; i++)
	{
		//printf("\n Tx Count[%d]\n",i);
	 	if(random_queue_act == ENABLE)
	 	{
	 		tx_queue_num = i % 4;

			if(tx_queue_num == 0)
			{
		if(rt2880_eth_send(rt2880_pdev,NetTxPacket,length) == -1)
		{
			printf("\n Internal loopback, Send packet is fail !! \n");
			break;
		}
			}
			else if(tx_queue_num == 1)
			{
				if(rt2880_eth_send(rt2880_pdev,NetTxPacket1,length) == -1)
	 			{
	 				printf("\n Internal loopback, Send packet is fail !! \n");
					break;
	 			}
			}
			else if(tx_queue_num == 2)
			{
				if(rt2880_eth_send(rt2880_pdev,NetTxPacket2,length) == -1)
	 			{
	 				printf("\n Internal loopback, Send packet is fail !! \n");
					break;
	 			}
			}
			else if(tx_queue_num == 3)
			{
				if(rt2880_eth_send(rt2880_pdev,NetTxPacket3,length) == -1)
	 			{
	 				printf("\n Internal loopback, Send packet is fail !! \n");
					break;
	 			}
			}
	 	}
		else
		{

			length = Gsw_Setup_Transmit_Packet(rt2880_pdev,length - 14,i/*((0x30 + tx_queue_num) % 0x30)*/,NetTxPacket);


	 		if(rt2880_eth_send(rt2880_pdev,NetTxPacket,length) == -1)
	{
	 			printf("\n Internal loopback, Send packet is fail !! \n");
				break;
	}
		}
	 }
	
#endif	 	

	//rt2880_eth_halt(rt2880_pdev);
	//STOP_ETH(rt2880_pdev);
	internal_loopback_test = INTERNAL_LOOPBACK_DISABLE;
}


U_BOOT_CMD(
 	interloopback,	3,	2,	rt2880_internal_loopback_test,
 	"interloopback   - \n",
 	"interloopback  Ralink designed test !! \n"
 	"interloopback  [created packet length] [fix patten](0 or 0x2~0xFF,1:random patten)\n"
 	
);
#endif // RALINK_INTERNAL_LOOPBACK_TEST_FUNC //


#ifdef RALINK_SDRAM_CONTROLLER_REFRESH_CYCLE_TEST_FUNC

#define RALINK_SDRAM_CFG1 *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x0304)
int rt2880_auto_refresh_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 len = 0x00100000;
	u32 refresh_period,tmp,t_start;
	int ii,kk = 2;
	volatile uchar *uncache_addr;

	if (argc != 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	//uncache_addr = (uchar *)(((u32)&Pkt_Buf_Pool[0] & 0x0FFFFFFF) | 0xA0000000);
	uncache_addr = (uchar *)(((u32)0x8a100000 & 0x0FFFFFFF) | 0xA0000000);

	printf("\n Test Memory Size:%d ,Address:[0x%08X] \n",len,uncache_addr);

	refresh_period = simple_strtoul(argv[1], NULL, 16);
	refresh_period &= 0x0000FFFF;
	kk = simple_strtoul(argv[2], NULL, 10);

	printf("\n Set 0x%04X clock cycles with auto refresh testing !! \n",refresh_period);

	for(ii=0 ; ii < len ; ii++)
	uncache_addr[ii] = ii % 0xFF;

	tmp  = RALINK_SDRAM_CFG1;
	tmp &= 0xFFFF0000;
	tmp |= refresh_period;
	RALINK_SDRAM_CFG1 = tmp;

	tmp  = RALINK_SDRAM_CFG1;
	printf("\n Set 0x%08X to RALINK_SDRAM_CFG1 \n", tmp);
	printf("\n Wait %d sec \n", kk);

	t_start = get_timer(0);

	while(get_timer(t_start)<= (kk * CFG_HZ));

	printf("\n Start testing !!\n");
	for(ii=0 ; ii < len ; ii++)
	{
		if(uncache_addr[ii] != ii % 0xFF)
		{
			printf("\n Fail !! addr[0x%08X],The fail's content:[%02X].The right's content:[%02X]\n",
					&uncache_addr[ii],uncache_addr[ii],ii % 0xFF);
		}
	}
	printf("\n Test finish  !!\n");
	printf("\n\n\n\n");
	return 0;
}

U_BOOT_CMD(
 	refresh,	3,	2,	rt2880_auto_refresh_test,
 	"refresh   - \n",
 	"refresh  Ralink designed test !! \n"
 	"refresh  [tx counter] [created packet length]\n"
);
#endif // RALINK_SDRAM_CONTROLLER_REFRESH_CYCLE_TEST_FUNC //


#if defined (RALINK_GDMA_SCATTER_TEST_FUN) || defined (RALINK_GDMA_DUP_TX_RING_TEST_FUN)
int rt2880_gmac_dma_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if(!memcmp(argv[1],"scatter",sizeof("scatter")))
	{
#ifdef RALINK_GDMA_SCATTER_TEST_FUN
		if(!memcmp(argv[2],"off",sizeof("off")))
		{
			printf("\n Header and payload scatter is disable !!\n");
			header_payload_scatter_en = DISABLE;
		}
		else
		{
			printf("\n Header and payload scatter is Enable !!\n");
			header_payload_scatter_en = ENABLE;
		}
#endif // RALINK_GDMA_SCATTER_TEST_FUN //
	}
	else if(!memcmp(argv[1],"dup_tx_ring",sizeof("dup_tx_ring")))
	{
#ifdef RALINK_GDMA_DUP_TX_RING_TEST_FUN
		if(!memcmp(argv[2],"off",sizeof("off")))
		{
			printf("\n dup_tx_ring is disable !!\n");
			header_payload_scatter_en = DISABLE;
		}
		else
		{
			printf("\n dup_tx_ring is Enable !!\n");
			header_payload_scatter_en = ENABLE;
		}
#endif // RALINK_GDMA_DUP_TX_RING_TEST_FUN //
	}
	// *PKT_HEADER_Buf;// = (uchar *)CFG_EMBEDED_SRAM_SDP0_BUF_START;
	return 0;
}

U_BOOT_CMD(
 	dmatest,	3,	2,	rt2880_gmac_dma_test,
 	"dmatest   - \n",
 	"dmatest  scatter on/off,Header and payload scatter enable/disable \n"
 	"dmatest  [tx counter] [created packet length]\n"
);
#endif // RALINK_GDMA_SCATTER_TEST_FUN


#ifdef RALINK_SWITCH_DEBUG_FUN
#define RALINK_VLAN_ID_BASE	(RALINK_ETH_SW_BASE + 0x50)
#define RALINK_VLAN_MEMB_BASE	(RALINK_ETH_SW_BASE + 0x70)

#define RALINK_TABLE_SEARCH	(RALINK_ETH_SW_BASE + 0x24)
#define RALINK_TABLE_STATUS0	(RALINK_ETH_SW_BASE + 0x28)
#define RALINK_TABLE_STATUS1	(RALINK_ETH_SW_BASE + 0x2c)
#define RALINK_TABLE_STATUS2	(RALINK_ETH_SW_BASE + 0x30)
#define RALINK_WT_MAC_AD0	(RALINK_ETH_SW_BASE + 0x34)
#define RALINK_WT_MAC_AD1	(RALINK_ETH_SW_BASE + 0x38)
#define RALINK_WT_MAC_AD2	(RALINK_ETH_SW_BASE + 0x3C)
#define RALINK_WT_MAC_AD2	(RALINK_ETH_SW_BASE + 0x3C)

void table_dump(void)
{
	int i, j, value, mac;
	int vid[16];

	for (i = 0; i < 8; i++) {
		value = RALINK_REG(RALINK_VLAN_ID_BASE + 4*i);
		vid[2 * i] = value & 0xfff;
		vid[2 * i + 1] = (value & 0xfff000) >> 12;
	}

	RALINK_REG(RALINK_TABLE_SEARCH) = 0x1;
	printf("hash  port(0:6)  vidx  vid  age   mac-address  filt\n");
	for (i = 0; i < 0x400; i++) {
		while(1) {
			value = RALINK_REG(RALINK_TABLE_STATUS0);
			if (value & 0x1) { //search_rdy
				if ((value & 0x70) == 0) {
					printf("found an unused entry (age = 3'b000), please check!\n");
					return;
				}
				printf("%03x:   ", (value >> 22) & 0x3ff); //hash_addr_lu
				j = (value >> 12) & 0x7f; //r_port_map
				printf("%c", (j & 0x01)? '1':'-');
				printf("%c", (j & 0x02)? '1':'-');
				printf("%c", (j & 0x04)? '1':'-');
				printf("%c", (j & 0x08)? '1':'-');
				printf("%c ", (j & 0x10)? '1':'-');
				printf("%c", (j & 0x20)? '1':'-');
				printf("%c", (j & 0x40)? '1':'-');
				printf("   %2d", (value >> 7) & 0xf); //r_vid
				printf("  %4d", vid[(value >> 7) & 0xf]);
				printf("    %1d", (value >> 4) & 0x7); //r_age_field
				mac = RALINK_REG(RALINK_TABLE_STATUS2);
				printf("  %08x", mac);
				mac = RALINK_REG(RALINK_TABLE_STATUS1);
				printf("%04x", (mac & 0xffff));
				printf("     %c\n", (value & 0x8)? 'y':'-');
				if (value & 0x2) {
					printf("end of table %d\n", i);
					return;
				}
				break;
			}
			else if (value & 0x2) { //at_table_end
				printf("found the last entry %d (not ready)\n", i);
				return;
			}
			udelay(5000);
		}
		RALINK_REG(RALINK_TABLE_SEARCH) = 0x2; //search for next address
	}
}

void table_add(int argc, char *argv[])
{
	int i, j, value, is_filter;
	char tmpstr[9];

	is_filter = (argv[1][0] == 'f')? 1 : 0;
	if (!argv[2] || strlen(argv[2]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[2], 8);
	tmpstr[8] = '\0';
	value = simple_strtoul(tmpstr, NULL, 16);
	RALINK_REG(RALINK_WT_MAC_AD2) = value;

	strncpy(tmpstr, argv[2]+8, 4);
	tmpstr[4] = '\0';
	value = simple_strtoul(tmpstr, NULL, 16);
	RALINK_REG(RALINK_WT_MAC_AD1) = value;

	if (!argv[3] || strlen(argv[3]) != 7) {
		if (is_filter)
			argv[3] = "1111111";
		else {
			printf("portmap format error, should be of length 7\n");
			return;
		}
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[3][i] != '0' && argv[3][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[3][i] - '0') * (1 << i);
	}
	value = j << 12; //w_port_map

	if (argc > 4) {
		j = simple_strtoul(argv[4], NULL, 0);
		if (j < 0 || 15 < j) {
			printf("wrong member index range, should be within 0~15\n");
			return;
		}
		value += (j << 7); //w_index
	}

	if (argc > 5) {
		j = simple_strtoul(argv[5], NULL, 0);
		if (j < 1 || 7 < j) {
			printf("wrong age range, should be within 1~7\n");
			return;
		}
		value += (j << 4); //w_age_field
	}
	else
		value += (7 << 4); //w_age_field

	if (is_filter)
		value |= (1 << 3); //sa_filter

	value += 1; //w_mac_cmd
	RALINK_REG(RALINK_WT_MAC_AD0) = value;

	for (i = 0; i < 20; i++) {
		value = RALINK_REG(RALINK_WT_MAC_AD0);
		if (value & 0x2) { //w_mac_done
			printf("done.\n");
			return;
		}
		udelay(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void table_del(int argc, char *argv[])
{
	int i, j, value;
	char tmpstr[9];

	if (!argv[2] || strlen(argv[2]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[2], 8);
	tmpstr[8] = '\0';
	value = simple_strtoul(tmpstr, NULL, 16);
	RALINK_REG(RALINK_WT_MAC_AD2) = value;

	strncpy(tmpstr, argv[2]+8, 4);
	tmpstr[4] = '\0';
	value = simple_strtoul(tmpstr, NULL, 16);
	RALINK_REG(RALINK_WT_MAC_AD1) = value;

	value = 0;
	if (argc > 3) {
		j = simple_strtoul(argv[3], NULL, 0);
		if (j < 0 || 15 < j) {
			printf("wrong member index range, should be within 0~15\n");
			return;
		}
		value += (j << 7); //w_index
	}

	value += 1; //w_mac_cmd
	RALINK_REG(RALINK_WT_MAC_AD0) = value;

	for (i = 0; i < 20; i++) {
		value = RALINK_REG(RALINK_WT_MAC_AD0);
		if (value & 0x2) { //w_mac_done
			if (argv[1] != NULL)
				printf("done.\n");
			return;
		}
		udelay(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void table_clear(void)
{
	int i, value, mac;
	char v[2][13];
	char *argv[4];

	memset(argv, 0, sizeof(v));
	memset(argv, 0, sizeof(argv));

	RALINK_REG(RALINK_TABLE_SEARCH) = 0x1;
	for (i = 0; i < 0x400; i++) {
		while(1) {
			value = RALINK_REG(RALINK_TABLE_STATUS0);
			if (value & 0x1) { //search_rdy
				if ((value & 0x70) == 0) {
					return;
				}
				sprintf(v[1], "%d", (value >> 7) & 0xf);
				mac = RALINK_REG(RALINK_TABLE_STATUS2);
				sprintf(v[0], "%08x", mac);
				mac = RALINK_REG(RALINK_TABLE_STATUS1);
				sprintf(v[0]+8, "%04x", (mac & 0xffff));
				argv[2] = v[0];
				argv[3] = v[1];
				table_del(4, argv);
				if (value & 0x2) {
					return;
				}
				break;
			}
			else if (value & 0x2) { //at_table_end
				return;
			}
			udelay(5000);
		}
		RALINK_REG(RALINK_TABLE_SEARCH) = 0x2; //search for next address
	}
}

void vlan_dump(void)
{
	int i, vid, value;

	printf("idx   vid  portmap\n");
	for (i = 0; i < 8; i++) {
		vid = RALINK_REG(RALINK_VLAN_ID_BASE + 4*i);
		value = RALINK_REG(RALINK_VLAN_MEMB_BASE + 4*(i/2));
		printf(" %2d  %4d  ", 2*i, vid & 0xfff);
		if (i%2 == 0) {
			printf("%c", (value & 0x00000001)? '1':'-');
			printf("%c", (value & 0x00000002)? '1':'-');
			printf("%c", (value & 0x00000004)? '1':'-');
			printf("%c", (value & 0x00000008)? '1':'-');
			printf("%c", (value & 0x00000010)? '1':'-');
			printf("%c", (value & 0x00000020)? '1':'-');
			printf("%c\n", (value & 0x00000040)? '1':'-');
		}
		else {
			printf("%c", (value & 0x00010000)? '1':'-');
			printf("%c", (value & 0x00020000)? '1':'-');
			printf("%c", (value & 0x00040000)? '1':'-');
			printf("%c", (value & 0x00080000)? '1':'-');
			printf("%c", (value & 0x00100000)? '1':'-');
			printf("%c", (value & 0x00200000)? '1':'-');
			printf("%c\n", (value & 0x00400000)? '1':'-');
		}
		printf(" %2d  %4d  ", 2*i+1, ((vid & 0xfff000) >> 12));
		if (i%2 == 0) {
			printf("%c", (value & 0x00000100)? '1':'-');
			printf("%c", (value & 0x00000200)? '1':'-');
			printf("%c", (value & 0x00000400)? '1':'-');
			printf("%c", (value & 0x00000800)? '1':'-');
			printf("%c", (value & 0x00001000)? '1':'-');
			printf("%c", (value & 0x00002000)? '1':'-');
			printf("%c\n", (value & 0x00004000)? '1':'-');
		}
		else {
			printf("%c", (value & 0x01000000)? '1':'-');
			printf("%c", (value & 0x02000000)? '1':'-');
			printf("%c", (value & 0x04000000)? '1':'-');
			printf("%c", (value & 0x08000000)? '1':'-');
			printf("%c", (value & 0x10000000)? '1':'-');
			printf("%c", (value & 0x20000000)? '1':'-');
			printf("%c\n", (value & 0x40000000)? '1':'-');
		}
	}
}

void vlan_set(int argc, char *argv[])
{
	int i, j, value;
	int idx, vid;

	if (argc != 6) {
		printf("insufficient arguments!\n");
		return;
	}
	idx = simple_strtoul(argv[3], NULL, 0);
	if (idx < 0 || 15 < idx) {
		printf("wrong member index range, should be within 0~15\n");
		return;
	}
	vid = simple_strtoul(argv[4], NULL, 0);
	if (vid < 0 || 0xfff < vid) {
		printf("wrong vlan id range, should be within 0~4095\n");
		return;
	}
	if (strlen(argv[5]) != 7) {
		printf("portmap format error, should be of length 7\n");
		return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}

	//set vlan identifier
	value = RALINK_REG(RALINK_VLAN_ID_BASE + 4*(idx/2));
	if (idx % 2 == 0) {
		value &= 0xfff000;
		value |= vid;
	}
	else {
		value &= 0xfff;
		value |= (vid << 12);
	}
	RALINK_REG(RALINK_VLAN_ID_BASE + 4*(idx/2)) = value;

	//set vlan member
	value = RALINK_REG(RALINK_VLAN_MEMB_BASE + 4*(idx/4));
	if (idx % 4 == 0) {
		value &= 0xffffff00;
		value |= j;
	}
	else if (idx % 4 == 1) {
		value &= 0xffff00ff;
		value |= (j << 8);
	}
	else if (idx % 4 == 2) {
		value &= 0xff00ffff;
		value |= (j << 16);
	}
	else {
		value &= 0x00ffffff;
		value |= (j << 24);
	}
	RALINK_REG(RALINK_VLAN_MEMB_BASE + 4*(idx/4)) = value;
}

int rt3052_switch_command(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	if (argc == 2) {
		if (!strncmp(argv[1], "dump", 5))
			table_dump();
		else if (!strncmp(argv[1], "clear", 6)) {
			table_clear();
			printf("done.\n");
		}
		else {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}
	}
	else if (!strncmp(argv[1], "add", 4))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "filt", 5))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "del", 4))
		table_del(argc, argv);
	else if (!strncmp(argv[1], "vlan", 5)) {
		if (argc < 3)
			printf ("Usage:\n%s\n", cmdtp->usage);
		if (!strncmp(argv[2], "dump", 5))
			vlan_dump();
		else if (!strncmp(argv[2], "set", 4))
			vlan_set(argc, argv);
		else
			printf ("Usage:\n%s\n", cmdtp->usage);
	}
	else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	return 0;
}

U_BOOT_CMD(
 	switch,	6,	1,	rt3052_switch_command,
 	"switch  - rt3052 embedded switch command\n",
 	"switch dump - dump switch table\n"
	"switch clear - clear switch table\n"
 	"switch add [mac] [portmap] - add an entry to switch table\n"
 	"switch add [mac] [portmap] [vlan idx] - add an entry to switch table\n"
 	"switch add [mac] [portmap] [vlan idx] [age] - add an entry to switch table\n"
 	"switch filt [mac] - add an SA filtering entry (with portmap 1111111) to switch table\n"
 	"switch filt [mac] [portmap] - add an SA filtering entry to switch table\n"
 	"switch filt [mac] [portmap] [vlan idx] - add an SA filtering entry to switch table\n"
 	"switch filt [mac] [portmap] [vlan idx] [age] - add an SA filtering entry to switch table\n"
 	"switch del [mac] - delete an entry from switch table\n"
 	"switch del [mac] [vlan idx] - delete an entry from switch table\n"
	"switch vlan dump - dump switch table\n"
	"switch vlan set [vlan idx] [vid] [portmap] - set vlan id and associated member\n"
);
#endif // RALINK_SWITCH_DEBUG_FUN //


#ifdef RALINK_PCI_HOST_TEST_FUN

//==========================================================
// PCI test
//==========================================================

// PCI Bus MAX define
#define	PCIB_MAX_DEVICE_TYPE_NUM      0x13
#define PCIB_MAX_BAR_NUM	      0x02
#define	PCIB_MAX_BUS_NUM	      0x01 //bit[23-16],
#define	PCIB_MAX_DEVICE_NUM	      0x13 //bit[15-11]
#define	PCIB_MAX_FUNCTION_NUM	      0x04 //bit[10-8]
#define	PCIB_MAX_REG_NUM	      0x3c //bit[7-2]

// PCIB Config define
#define PCIB_CONFIG_EN                0x80000000
#define PCIB_BUS_NUM_BASE             0x10000
#define PCIB_DEV_NUM_BASE             0x800
#define PCIB_FUN_NUM_BASE             0x100
#define PCIB_REG_NUM_BASE             0x4


u32 sys_read_pci_config_word(PCIDeviceIDStruct PCIDeviceID, u32 Reg) //for Bridge only
{
    u32 uwData,ii;

    PCIDeviceID.Enable = 1;
#if 0
    PCIDeviceID.RegNum = Reg;
    PCI_BRIDGE_CONFIG_ADDR= *(u32*)&PCIDeviceID;
#else
    PCIDeviceID.RegNum = Reg>>2;
    PCI_BRIDGE_CONFIG_ADDR= PCIB_CONFIG_EN +PCIDeviceID.BusNum*PCIB_BUS_NUM_BASE
                            +PCIDeviceID.DevNum*PCIB_DEV_NUM_BASE+PCIDeviceID.FunNum*PCIB_FUN_NUM_BASE
                            +PCIDeviceID.RegNum*PCIB_REG_NUM_BASE;

#endif
    for (ii=0;ii<1000;ii++);
    uwData =  PCI_BRIDGE_CONFIG_DATA;

    return uwData;
}

u16 sys_read_pci_config_halfword(PCIDeviceIDStruct PCIDeviceID, u32 Reg)//for Bridge only
{
    u32 lw;

    lw = sys_read_pci_config_word(PCIDeviceID, (Reg&0xfc));
    switch(Reg % 4)
    {
        case 0:
	case 1:
	    lw &= 0x0000FFFF;
	    break;
	case 2:
	case 3:
	    lw &= 0xFFFF0000;
	    lw = lw >> 16;
	    break;
    }

    return (u16)lw;
}

u8 sys_read_pci_config_byte(PCIDeviceIDStruct PCIDeviceID, u32 Reg)//for Bridge only
{
    u32 lw;

    lw = sys_read_pci_config_word(PCIDeviceID, (Reg&0xfc));
    switch(Reg % 4)
    {
        case 0:
	    lw &= 0x000000FF;
	    break;
	case 1:
	    lw &= 0x0000FF00;
   	    lw = lw >> 8;
	    break;
	case 2:
            lw &= 0x00FF0000;
            lw = lw >> 16;
            break;
 	case 3:
            lw &= 0xFF000000;
            lw = lw >> 24;
 	break;
    }

    return (u8)lw;
}

void sys_write_pci_config_word(PCIDeviceIDStruct PCIDeviceID, u32 Reg, u32 data)//for Bridge only
{
    u32 ii;
    PCIDeviceID.Enable = 1;
#if 0
    PCIDeviceID.RegNum = (Reg&0xfc);
    PCI_BRIDGE_CONFIG_ADDR= *(u32 *)&PCIDeviceID;
#else
    PCIDeviceID.RegNum = (Reg&0xfc)>>2;
    PCI_BRIDGE_CONFIG_ADDR= PCIB_CONFIG_EN +PCIDeviceID.BusNum*PCIB_BUS_NUM_BASE
                             +PCIDeviceID.DevNum*PCIB_DEV_NUM_BASE+PCIDeviceID.FunNum*PCIB_FUN_NUM_BASE
		             +PCIDeviceID.RegNum*PCIB_REG_NUM_BASE;

#endif
    for (ii=0;ii<1000;ii++);
    PCI_BRIDGE_CONFIG_DATA   = data;
}

void sys_write_pci_config_halfword(PCIDeviceIDStruct PCIDeviceID, u32 Reg, u16 data)//for Bridge only
{
    u32 lw;

    lw = sys_read_pci_config_word(PCIDeviceID, (Reg&0xfc));
    switch(Reg % 4)
    {
        case 0:
	case 1:
	    lw &= 0xFFFF0000;
            lw += data;
	    sys_write_pci_config_word(PCIDeviceID, (Reg&0xfc), lw);
	    break;
	case 2:
	case 3:
            lw &= 0x0000FFFF;
            lw += (u32)(((u32)data) << 16);
	    sys_write_pci_config_word(PCIDeviceID, (Reg&0xfc), lw);
            break;
    }

}

void sys_write_pci_config_byte(PCIDeviceIDStruct PCIDeviceID, u32 Reg, u8 data)//for Bridge only
{
    u32 lw;

    lw = sys_read_pci_config_word(PCIDeviceID, (Reg&0xfc));
    switch (Reg % 4)
    {
        case 0:
	    lw &= 0xFFFFFF00;
	    lw += data;
	    sys_write_pci_config_word(PCIDeviceID, (Reg&0xfc), lw);
	    break;
	case 1:
            lw &= 0xFFFF00FF;
	    lw += (u32)(((u32)data) << 8);
	    sys_write_pci_config_word(PCIDeviceID, (Reg&0xfc), lw);
	    break;
	case 2:
            lw &= 0xFF00FFFF;
	    lw += (u32)(((u32)data) << 16);
	    sys_write_pci_config_word(PCIDeviceID, (Reg&0xfc), lw);
	    break;
	case 3:
   	    lw &= 0x00FFFFFF;
	    lw += (u32)(((u32)data) << 24);
	    sys_write_pci_config_word(PCIDeviceID, (Reg&0xfc), lw);
	    break;
    }

}


void kaiker_AssignPCIResource(PCIDeviceIDStruct PCIDeviceID , u32 *PciMemStart, u32 *PciIoStart)
{
    u32  lw,ii,jj, Reg, BaseAddrReg, BaseSize;
    u32  dwAlignmentSize;
    u32  device_id;

    device_id = sys_read_pci_config_word(PCIDeviceID, 0x00);         // read Device ID


    for (ii = 0 ; ii < PCIB_MAX_BAR_NUM ; ii++) //BAR_NUM:0x06
    {
        Reg = PCI_CSH_BASE_ADDR_REG + (ii * 4); //0x10+4xi

		sys_write_pci_config_word(PCIDeviceID, Reg, 0xFFFFFFFF); //write 0xffffffff,then
		lw = sys_read_pci_config_word(PCIDeviceID, Reg);         // read back value
        printf("\n\n******BAR%d= 0x%08x ***********\n",ii,lw);
		if ((lw == 0) || ((lw & 0xffffffff) == 0xffffffff))
		{
			printf("Base Address Register %d is not exist !!\n",ii);
			continue;
		}
		else		/* else-if */
		{
			if ((lw & 0x01) != 0x00)	/* it's IO base */
			{
				if(device_id == PCI_BRIDGE_DEVICE_VENDOR_ID) /*If current device is PCI bridge*/
				{
					//Open all mem space
				#if 1
				RALINK_PCI_BAR0SETUP_ADDR = 0xFFFF0001;//Open 2G mem space
				RALINK_PCI_BAR1SETUP_ADDR = 0xFFFF0001;//Open 2G mem space
				#endif
				RALINK_PCI_IMBASEBAR0_ADDR = 0;
				RALINK_PCI_IMBASEBAR1_ADDR = 0;
					sys_write_pci_config_word(PCIDeviceID, Reg, SYS_DDR_SDRAM_BASE_ADDR);
					printf("\nSet  IO  base of the PCI bridge\n");

					printf("\n\n******write = 0x%08x *read = 0x%08x**********\n",SYS_DDR_SDRAM_BASE_ADDR,sys_read_pci_config_word(PCIDeviceID, Reg));
				}
				else
				{
    	    		lw >>= 2; // 31..2:base addr.
    	    		for(jj=2; jj < 32; jj++)
					{
						if (lw & 0x01 == 0x01) //[b31..b2],only 1 bit will be "1"
						{
							break;  //valid bit found,
						}
						lw >>= 1;
					}
					BaseSize = 1 << jj;// size is power of 2
					if (BaseSize >= PCI_IO_SPACE_ALIGNMENT) //why:0x4,???
					{
						dwAlignmentSize = BaseSize;
					}
					else
					{
						dwAlignmentSize = PCI_IO_SPACE_ALIGNMENT;//4
					}
					printf("This is IO Base Space Register !\n The request of size is %d bytes\n",dwAlignmentSize);
					if ((*PciIoStart % dwAlignmentSize) != 0)
					{
						*PciIoStart = ((*PciIoStart / dwAlignmentSize) + 1) * dwAlignmentSize;
					}
			BaseAddrReg = *PciIoStart;
			*PciIoStart += BaseSize; // CurrentIOStart + Current_BaseSz => NextIOStart
			printf("set I/O base:0x%8x\n",BaseAddrReg);
			sys_write_pci_config_word(PCIDeviceID, Reg, BaseAddrReg);
			printf("\n\n******write = 0x%08x *read = 0x%08x**********\n",BaseAddrReg,sys_read_pci_config_word(PCIDeviceID, Reg));
	    	}
	    }//IO base
	    else if((lw & 0x01) != 0x01)		/* it's Memory base */
	    {
		if(device_id == PCI_BRIDGE_DEVICE_VENDOR_ID)/*If current device is PCI bridge*/
	    	{
	    		printf("\nSet Memory base of the PCI bridge\n");
	    		sys_write_pci_config_word(PCIDeviceID, Reg, SYS_DDR_SDRAM_BASE_ADDR);
	    		printf("\n\n******write = 0x%08x *read = 0x%08x**********\n",SYS_DDR_SDRAM_BASE_ADDR,sys_read_pci_config_word(PCIDeviceID, Reg));
	    	}
		else
		{
		        lw >>= 4;
			for (jj=4; jj < 32; jj++)
			{
			    if (lw & 0x01 == 0x01)
			    {
			        break;
			    }
			    lw >>= 1;
			}
			BaseSize = 1 << jj;
			if (BaseSize>=PCI_MEM_SPACE_ALIGNMENT) //16
			{
			    dwAlignmentSize=BaseSize;
			}
			else
			{
			    dwAlignmentSize=PCI_MEM_SPACE_ALIGNMENT;
			}
			printf("This is MEMORY Base Space Register !\n  The request of size is %d bytes\n",dwAlignmentSize);
			if ((*PciMemStart % dwAlignmentSize) != 0)
			{
			    *PciMemStart = ((*PciMemStart / dwAlignmentSize) + 1) * dwAlignmentSize;
			}
			BaseAddrReg = *PciMemStart;
			*PciMemStart += BaseSize;
			printf("set  MEM base :0x%8x\n",BaseAddrReg);
			sys_write_pci_config_word(PCIDeviceID, Reg, BaseAddrReg);
			printf("\n\n******write = 0x%08x *read = 0x%08x**********\n",BaseAddrReg,sys_read_pci_config_word(PCIDeviceID, Reg));
		}
	    }
        }
    }
}


void Init_LatTimer_CacheSize(PCIDeviceIDStruct PCIDeviceID,u32 isMaster)
{

    u32  CMDType;


    PCIDeviceID.RegNum = PCI_CSH_CACHE_LINE_SIZE_REG;
    CMDType = sys_read_pci_config_word(PCIDeviceID, PCI_CSH_CACHE_LINE_SIZE_REG);

    printf("   Init_LatTimer_CacheSize   is called \n\n");
    printf("    CACHE_LINE_SIZE_REG default value 0x%08x\n",CMDType);

   CMDType=CMDType & 0xFFFF0000;
 //  if(isMaster)
  // {
   	 CMDType=CMDType | 0x00000014;//Cache size
   	 CMDType=CMDType | 0x00001000;//Latency Timer
  // }
  // else
    //     CMDType=CMDType | 0x00001000;//Latency Timer

    sys_write_pci_config_word(PCIDeviceID, PCI_CSH_CACHE_LINE_SIZE_REG, CMDType);

    CMDType = sys_read_pci_config_word(PCIDeviceID, PCI_CSH_CACHE_LINE_SIZE_REG);
    printf("    CACHE_LINE_SIZE_REG Set value 0x%08x\n",CMDType);
}
//====================================================================
// * Function Name: flib_EnablePCIDevice                                 (0708 => ok)
// * Description: 1.Read Command register byte
//                2.Enable BOTH IO/MEM space response
// * Input: PCIDeviceID
//====================================================================
void flib_EnablePCIDevice(PCIDeviceIDStruct PCIDeviceID)
{
    u16 CMDType;
    PCIDeviceID.RegNum = PCI_CSH_COMMAND_REG;
    CMDType = sys_read_pci_config_halfword(PCIDeviceID, PCI_CSH_COMMAND_REG);

    printf("   flib_EnablePCIDevice   is called \n\n");
    printf("    COMMAND_REG default value 0x%04x\n",CMDType);

    printf("    Enable PCI DEV's IO & MEM space response\n");

    sys_write_pci_config_halfword(PCIDeviceID, PCI_CSH_COMMAND_REG, CMDType | PCI_CMD_IO_ENABLE |
    	PCI_CMD_MEM_ENABLE|PCI_CMD_MEM_WRITE_INVALIDATE|PCI_CMD_FBB_ENABLE|PCI_CMD_SERR_ENABLE|
    	PCI_CMD_STEPPING_CONTROL|PCI_CMD_PARITY_ERR);

    CMDType = sys_read_pci_config_halfword(PCIDeviceID, PCI_CSH_COMMAND_REG);
    printf("    COMMAND_REG Set value 0x%04x\n",CMDType);
}


//====================================================================
// * Function Name: flib_SetPCIMaster                                     (0708 => ok)
// * Description: 1.Read Command register byte
//                2.Write Back (PCI_CMD_BUS_MASTER_ENABLE)
// * Input: PCIDeviceID
//====================================================================
void flib_SetPCIMaster(PCIDeviceIDStruct PCIDeviceID)
{
    u32 CMDType;
    PCIDeviceID.RegNum = PCI_CSH_COMMAND_REG;
   //  printf("   flib_SetPCIMaster   is called \n\n");
 //`   printf("    COMMAND_REG default value 0x%2x\n",CMDType);

    CMDType = sys_read_pci_config_byte(PCIDeviceID, PCI_CSH_COMMAND_REG);
    printf("    PCI BUS master mode enable..\n");
    sys_write_pci_config_byte(PCIDeviceID, PCI_CSH_COMMAND_REG, CMDType | PCI_CMD_BUS_MASTER_ENABLE);
}

void kaiker_PCI_Bridge_Test(void)
{
    u32 uwFAIL_CNT=0,uwReadWord,uwResult,jj,Reg;
    u32 M_bass_address=PCI_ALLOCATE_SPACE,IO_bass_address=PCI_ALLOCATE_SPACE;
     //PCIDeviceIDStruct PCIDeviceID;
/*
 *  Using Random Value to test PCI bridge interl regsiter
 */
    printf("=>Test Internal registers...\n");




	for(jj=0;Finded_PCIDeviceID[jj].PCIDeviceID.Enable==1;jj++)
	{
		printf("-  Get IO/MEMORY size and allocate space to the  Dev%d \n",Finded_PCIDeviceID[jj].PCIDeviceID.DevNum);

		kaiker_AssignPCIResource(Finded_PCIDeviceID[jj].PCIDeviceID,&M_bass_address,&IO_bass_address);
	}


 //Enable the all device and set them to master.
  //Add by kaiker

  for(jj=0;Finded_PCIDeviceID[jj].PCIDeviceID.Enable==1;jj++)
  {
   	    printf("EnablePCIDevice,ID=0x%08x\n",Finded_PCIDeviceID[jj].dev_ven);
            flib_EnablePCIDevice(Finded_PCIDeviceID[jj].PCIDeviceID);
		//	printf("\n PCIRAW = 0x%08X \n",*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004));
			//*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004) = 0xFFFFFFFF;
            printf("SetPCIMaster!\n");
            flib_SetPCIMaster(Finded_PCIDeviceID[jj].PCIDeviceID);
			//printf("\n PCIRAW = 0x%08X \n",*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004));
			//*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004) = 0xFFFFFFFF;
             printf("Init_LatTimer_CacheSize!\n");
             if(Finded_PCIDeviceID[jj].dev_ven != PCI_BRIDGE_DEVICE_VENDOR_ID)
             {
             	Init_LatTimer_CacheSize(Finded_PCIDeviceID[jj].PCIDeviceID,0);
             }
             else
             {
             	Init_LatTimer_CacheSize(Finded_PCIDeviceID[jj].PCIDeviceID,1);


             }
         	//printf("\n PCIRAW = 0x%08X \n",*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004));
   			//*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004) = 0xFFFFFFFF;
  }//mask by kaiker

//Add by kaiker
}

void kaiker_PCI_Scan_Bus_Test(void)
{
    u32 ii,jj,kk,ll,mm;
    u32 ID,TempValue;
    int         devnum=0,find_device=0;

	PCI_BRIDGE_CLOCK = 0x4;

	//printf("\n PCIRAW = 0x%08X \n",*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004));

	*(volatile u_long *)(RALINK_PCI_BASE + 0x0004) = 0xFFFFFFFF;
	//le32_to_cpu(*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1080)) = 0;
	udelay(5000);
    //PCI Device Scan (BUS/DEVICE/FUNCTION)
    printf("---------------Bus/Device/Function Scan --------------------------\n");
    for (ii=0;ii < PCIB_MAX_BUS_NUM; ii++)
    {//bus
        printf("  --Bus %d Scan... \n",ii);
	for (jj=0;jj < PCIB_MAX_DEVICE_NUM; jj++)
	{//dev
	    //Read the ID to check the exist of the device(Function-0)
    	    PCI_BRIDGE_CONFIG_ADDR = PCIB_CONFIG_EN + (ii*PCIB_BUS_NUM_BASE)+(jj*PCIB_DEV_NUM_BASE);
			udelay(500);
            printf("  --PCI_BRIDGE_CONFIG_ADDR 0x%x \n",PCI_BRIDGE_CONFIG_ADDR);
            ID = PCI_BRIDGE_CONFIG_DATA;
	    if((ID != 0x00000000) && (ID != 0xFFFFFFFF)) //f0
	    {//DEV found
	    	find_device=1;
	        printf("       --Device %d Found... Bus %d / Dev %d /Fun 0 => ID:0x%08x \n",jj,ii,jj,ID);
            // Dump the Config Space of the found DEV's Function-0
                printf("        DEVICE(0x%08x)'s  Configuration :\n",ID);

                Finded_PCIDeviceID[devnum].PCIDeviceID.BusNum = ii;
                Finded_PCIDeviceID[devnum].PCIDeviceID.DevNum = jj;
                Finded_PCIDeviceID[devnum].PCIDeviceID.FunNum = 0x00;
                Finded_PCIDeviceID[devnum].PCIDeviceID.Enable=1;
                Finded_PCIDeviceID[devnum].dev_ven=ID;

	//	kaiker_PCI_Bridge_Test();

#if 1
#if 0
	        for (ll=0;ll<4 /*24*/;ll++)
	        {
                    //for (mm=0;mm<0x10000;mm++);
                    printf("            Value read at 0x%02x is 0x%08x\n",(ll*4),sys_read_pci_config_word(Finded_PCIDeviceID[devnum].PCIDeviceID,(ll*4)));
	        }
#endif

    		for (ll=0;ll<24;ll++)
		{
		    PCI_BRIDGE_CONFIG_ADDR= PCIB_CONFIG_EN +
		                             Finded_PCIDeviceID[devnum].PCIDeviceID.BusNum*PCIB_BUS_NUM_BASE+
		                             Finded_PCIDeviceID[devnum].PCIDeviceID.DevNum*PCIB_DEV_NUM_BASE+
		                             Finded_PCIDeviceID[devnum].PCIDeviceID.FunNum*PCIB_FUN_NUM_BASE+
		                             ll*4;
                    printf("            Value read at 0x%02x is 0x%8x\n",(ll*4),PCI_BRIDGE_CONFIG_DATA);
                }

#endif

		devnum++;

	    }//DEV found
#if 1
	    else //f0
	    {
	        printf("       --Device %d Not Found... \n",jj);
	    }
#endif
	}//dev
    }//bus
    printf("  --PCIB Bus Scan Finish... \n");

	//printf("\n PCIRAW = 0x%08X \n",*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004));
	//*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x1004) = 0xFFFFFFFF;

	/*clear remnant of devices structl */
	for(;devnum<PCI_MAX_SLOT_NUM;devnum++)
	Finded_PCIDeviceID[devnum].PCIDeviceID.Enable=0;
#if 0
	if(find_device)
	{
		kaiker_PCI_Bridge_Test();
	}
	else
	{
		printf	("\n\n*****DEVICE NOT FIND******\n\n");
	}
#endif
}
#endif // RALINK_PCI_HOST_TEST_FUN //


#ifdef RALINK_MEMORY_TEST_FUN //memory test for 2883

extern gd_t gd_data;

#define SDRAM_BASE_ADDR CFG_SDRAM_BASE
#define TEST_PATTEN_BASE_ADDR PHYS_FLASH_1

static unsigned long
random()
{
	static unsigned long	RandomValue = 1;
	/* cycles pseudo-randomly through all values between 1 and 2^31 - 2 */
	register long	rv = RandomValue;
	register long	lo;
	register long	hi;

	hi = rv / 127773;
	lo = rv % 127773;
	rv = 16807 * lo - 2836 * hi;
	if( rv <= 0 ) rv += 2147483647;
	RandomValue = rv ;

	return( RandomValue);
}

void rt2883_ram_test_random(u32 test_size, u32 mem_base)
{
	int i=0,kk=0,j;
	unsigned char *src,*dest;
	unsigned int mem_test_size, src_mem_offset, dest_mem_offset;
	unsigned int random_i, i_diff;

	while(!kaiker_button_p())
	{
		kk++;
		printf("\n Test count [%d]\n",kk);

		random_i = (unsigned int)random();

		/* step 1: decide memory size */
		mem_test_size = (random_i % test_size);

		printf("\n Max test_size = %d[0x%X] \n",test_size,test_size);
		printf("\n Current Ranom Test Size = %d[0x%08X]\n",mem_test_size,mem_test_size);
		// printf("\n  decide memory size :%X \n",mem_test_size);

		/* step 2: decide source sram offset */
		src_mem_offset = (random_i % (test_size - mem_test_size));
		//src_mem_offset = test_size - src_mem_offset ;

		printf("\n decide source sdram offset = %08X \n",src_mem_offset);

		/* step 3: decide dest sram offset */
		random_i = (unsigned int)random();
		dest_mem_offset = (random_i % (test_size - mem_test_size));

		printf("\n decide dest_mem_offset = %08X \n",dest_mem_offset);

		src = src_mem_offset + TEST_PATTEN_BASE_ADDR;
		dest = dest_mem_offset + mem_base;
		printf("\n decide dest = %08X:0ffset[%X] ,decide src = %08X:offset[%08X]\n",dest,dest_mem_offset,src,src_mem_offset);

		printf(" ... memcpy() start...,Size= %d(0x%X)bytes",mem_test_size,mem_test_size);
		memcpy(dest, src, mem_test_size);
		printf("Done\n");

		j = 0;

		for (i = 0; i < mem_test_size; i++) {
			if (dest[i] != src[i]) {
				printf("data error in dest[%d], 0x%x ... src[%d], 0x%x\n", i, (dest + i), i, (src + i));
				j++;
				while(1);
			}
		} /* for */

		printf("dest_addr src_addr size : 0x%x 0x%x %d\n\n", dest, src, mem_test_size);
		if (j == 0)
			printf("memory test ok!\n\n");
		else {
			printf("memory test failed!\n\n");
			while(1);
		}

		udelay(mem_test_size);
	} /* for */
}

int kaiker_rt2883_mem_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	bd_t *bd;
	u32 ii,kk,test_max_size;

	u32 test_patten;
	u32  *test_patten_source_w;

	u32 seg0_end_addr;

	u16  *test_patten_source_hw;
	u8  *test_patten_source_b;

	u32 *seg0_base_ptr_w;
	u16 *seg0_base_ptr_hw;
	u8 *seg0_base_ptr_b;
	int rand_fix=0;


	if (argc != 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	rand_fix = simple_strtoul(argv[1], NULL, 10);

	if(rand_fix)
		printf("\n random mem test \n");
	else
		printf("\n linear mem test \n");

	bd =  gd_data.bd;

	printf("\n kaiker_rt2883_mem_test ==> bd->bi_reallocate_image_addr = 0x%08X\n",bd->bi_reallocate_image_addr);
	printf("\n kaiker_rt2883_mem_test ==>  bd->bi_total_image_size = 0x%08X\n",bd->bi_total_image_size);

#if	1
	test_patten_source_w = (u32 *)TEST_PATTEN_BASE_ADDR;
	test_patten_source_hw = (u16 *)TEST_PATTEN_BASE_ADDR;
	test_patten_source_b = (u8 *)TEST_PATTEN_BASE_ADDR;

	test_max_size = bd->bi_total_image_size & ~0xF;

	printf("\n Test Max Size=%d[0x%08X] \n",test_max_size,test_max_size);

	test_patten = random();

	seg0_base_ptr_w = (volatile u32 *)SDRAM_BASE_ADDR;
	seg0_base_ptr_hw = (volatile u16 *)SDRAM_BASE_ADDR;
	seg0_base_ptr_b = (volatile u8 *)SDRAM_BASE_ADDR;

	seg0_end_addr = (bd->bi_reallocate_image_addr & ~0xF) - 0x10;

	printf("\n Kaiker Memory test !!! \n\n\n");

	printf("\n memtest base addr = %08X ,end addr = %08X,image size = 0x%X \n",seg0_base_ptr_w,seg0_end_addr,test_max_size);

	kk = 0;

	if (rand_fix) {
		printf("\n ================== random size and random Block test =============== \n");
		printf("\n base addr = %08X ,end addr = %08X,length = 0x%X \n",seg0_base_ptr_w,seg0_end_addr,test_max_size);
		rt2883_ram_test_random(test_max_size,(u32)seg0_base_ptr_w);
		return 0;
	}

	test_max_size = bd->bi_memsize - test_max_size;
	test_max_size = test_max_size & ~(4096 - 1);

while(!kaiker_button_p())
{
#if 1
	test_patten = random();
	kk++;

	printf("\n //----------------------------------------------------");
	printf("\n                 [%d]:test_patten = [%08X]    ",kk,test_patten);
	printf("\n //----------------------------------------------------\n");

	printf("\n Start Test Seg0 Address =%08X,len=0x%08X \n",seg0_base_ptr_w,test_max_size);
	printf("\n End Address =%08X \n",seg0_end_addr);

	printf("\n Write ,32 bit:\n");
	for(ii=0 ; ii< (test_max_size / 4)  ;ii++)
	{
		#ifdef USE_BLOCK_PATTEN
		seg0_base_ptr_w[ii] = test_patten_source_w[(ii % (test_max_size /4) )];
		#else
		seg0_base_ptr_w[ii]= test_patten + ii;
		#endif
	}

	printf("\n Read and compare , 32 bit :\n");
	for(ii=0 ; ii< (test_max_size / 4)  ;ii++)
	{
		#ifdef USE_BLOCK_PATTEN
		if(seg0_base_ptr_w[ii]!= test_patten_source_w[(ii % (test_max_size /4) )])
		#else
		if(seg0_base_ptr_w[ii]!= (test_patten + ii))
		#endif
		{
			printf("\n Fail addr[%08X]=%X  ,true value = %X\n",&seg0_base_ptr_w[ii],seg0_base_ptr_w[ii],(test_patten + ii));
			while(1);
		}

	}
	//-------------------------------------------

	printf("\n Write ,16 bit:\n");
	for(ii=0 ; ii< (test_max_size / 2)  ;ii++)
	{
		#ifdef USE_BLOCK_PATTEN
		seg0_base_ptr_hw[ii] = test_patten_source_hw[(ii % (test_max_size / 2) )];
		#else
		seg0_base_ptr_hw[ii]= (u16)((test_patten + ii) % 0xFFFF);
		#endif
	}
	printf("\n Read and compare , 16 bit :\n");

	for(ii=0 ; ii< (test_max_size / 2)  ;ii++)
	{
		#ifdef USE_BLOCK_PATTEN
		if(seg0_base_ptr_hw[ii]!= test_patten_source_hw[(ii % (test_max_size /2) )])
		#else
		if(seg0_base_ptr_hw[ii]!= (u16)((test_patten + ii) % 0xFFFF))
		#endif
		{
			printf("\n Fail addr[%08X]=%X  ,true value = %04X\n",&seg0_base_ptr_hw[ii],seg0_base_ptr_hw[ii],(u16)((test_patten + ii) % 0xFFFF));
			while(1);
		}
	}

	//------------------------------------------
	printf("\n Write ,8 bit:\n");
	for(ii=0 ; ii< (test_max_size)  ;ii++)
	{
		#ifdef USE_BLOCK_PATTEN
		seg0_base_ptr_b[ii]= test_patten_source_b[(ii % (test_max_size) )];
		#else
		seg0_base_ptr_b[ii]= (u8)((test_patten + ii) % 0xFF);
		#endif
		//printf("\n Write addr[%08X] = %d\n",&seg0_base_ptr_w[ii],ii);
	}

	printf("\n Read and compare , 8 bit :\n");
	for(ii=0 ; ii< (test_max_size )  ;ii++)
	{
		#ifdef USE_BLOCK_PATTEN
		if(seg0_base_ptr_b[ii] !=  test_patten_source_b[(ii % (test_max_size) )])
		#else
		if(seg0_base_ptr_b[ii]!= (u8)((test_patten + ii) % 0xFF))
		#endif
		{
			printf("\n Fail addr[%08X]=%X,true value=%X  \n",&seg0_base_ptr_b[ii],seg0_base_ptr_b[ii],(u8)((test_patten + ii) % 0xFF));
			while(1);
		}
	}
#endif // 1 //
}
#endif // 1 //
	return 0;
}

U_BOOT_CMD(
 	kaiker_memtest,	2,	1,	kaiker_rt2883_mem_test,
 	"kaiker_memtest [0:fix/1:random]   - Ralink memory test !!\n",
 	"kaiker_memtest   -  \n"

);

#endif // RALINK_MEMORY_TEST_FUN //


#ifdef RT3052_PHY_TEST
/*
 * 1. Setup Per Port VID
 * P0 <-> P1
 * P2 <-> P3
 * P4 <-> MII(PHY)
 *
 * 2. Map port to VLAN member
 *
 * 3. Remove tag enable for each recieving port
 *
 */

void rt3052_phy_loopback_routine(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
void packet_dump(unsigned char* packet, unsigned int length)
{
	int i, j, k;
	k = length / 10;

	printf("packet dump -- %d bytes\n", length);
	printf("\n-----------------------------\n");
	for ( i = 0; i < length; i++)
	{	
		printf("0x%02x ", packet[i]);

		if (( i % 10 ) == 9 )
			printf("\n");
	} 
	printf("\n-----------------------------\n");
}

void test_packet_init()
{
	int i;
	BUFFER_ELEM *buf;
        if(!NetTxPacket) {
                // initial tx and rx buffers
                // printf("Inital NetTxPacket...\n");
                buf = rt2880_free_buf_entry_dequeue(&rt2880_free_buf_list);
                NetTxPacket = buf->pbuf;
                // printf("NetTxPacket = 0x%08x\n", NetTxPacket);

                for ( i = 0; i < NUM_RX_DESC; i++) {
                        buf = rt2880_free_buf_entry_dequeue(&rt2880_free_buf_list);
                        if ( buf == NULL) {
                                printf("NetRxPackets[%d] --- Buffer empty\n", i);
                                return -1;
                        }
                        NetRxPackets[i] = buf->pbuf;
                        // printf("NetRxPackets[%d] = 0x%08x\n", i, NetRxPackets[i]);
                        memset(NetRxPackets[i], 0, PKTBUFSRX);
                }
        }

        NetTxPacket = KSEG1ADDR(NetTxPacket);
	// rt2880_eth_setup(rt2880_pdev);

	if (!phy_init_setup) {
		rt2880_eth_setup(rt2880_pdev);
		phy_init_setup = 1;
	}
}

unsigned int rt3052_ether_setup()
{
	int i;
	unsigned int length = 100;
	u32 mdio_value;
	if (!phy_init_setup) {
		mii_mgr_read(0, 1, &mdio_value);
		printf("** Port0 Reg1 mdio data is 0x%08x\n", mdio_value);
		phy_link_detect();
		// PVID initlize for each port
		// port 0 -> VID 1, port 1 -> VID2
		*(unsigned long *)(0xb0110040) = 0x00002001;
		// port 2 -> VID3, port 3 -> VID4
		*(unsigned long *)(0xb0110044) = 0x00004003;
		// port 4-> VID5, port 5(MII Port) -> VID6
		*(unsigned long *)(0xb0110048) = 0x00006005;
		// port 6 -> VID7 (CPU Port)
		*(unsigned long *)(0xb011004c) = 0x00000007;

		// VLAN member set
		*(unsigned long *)(0xb0110070) = 0x48444241;
		*(unsigned long *)(0xb0110074) = 0x00406050;
		*(unsigned long *)(0xb0110078) = 0x0;
		*(unsigned long *)(0xb011007c) = 0x0;

		*(unsigned long *)(0xb0110094) = 0x00007f00;
		*(unsigned long *)(0xb0110098) = 0x00007f7f;
		
		// *(unsigned long *)(0xb0110098) = 0x00007fff;

		// let switch enter force mode
		// -> Let Link Up and 100MB Full
		// *(unsigned long *)(0xb0110084) = 0xffffff00;
		mii_mgr_read(0, 1, &mdio_value);
		printf("++ Port0 Reg1 mdio data is 0x%08x\n", mdio_value);
	}
#if 0
	mii_mgr_read(0, 1, &mdio_value);
	printf("*** mdio data is 0x%08x\n", mdio_value);
#endif
	test_packet_init();

	return 1;
}

void rt3052_ether_loopback_send(int port_no, int send_len)
{
	int length;
	int i, hdr_offset;
	volatile uchar *pkt;

	VLAN_Ethernet_t *vlan;

	if ( send_len < 64)
		send_len = 64;

	pkt = NetTxPacket;
	vlan = (VLAN_Ethernet_t *)pkt;
	pkt += NetSetEther(pkt, NetBcastAddr, PROT_VLAN);

	vlan->vet_vlan_type = htons(ETH_P_8021Q);
	vlan->vet_tag       = htons(port_no);
	hdr_offset = pkt - NetTxPacket + VLAN_ETHER_HDR_SIZE;

	for ( i = hdr_offset; i < send_len; i++)
		pkt[i] = i;

	memcpy(rt3052_phy_test_buf, NetTxPacket, send_len);
	rt2880_eth_send(rt2880_pdev, NetTxPacket, send_len);

#if 0
		printf("------\nsend packet :\n\n");
		packet_dump(NetTxPacket, send_len);
#endif
}

void rt3052_phy_loopback_routine(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int port_no = 0, counter = 1;
	unsigned int test_len;
	int result, i;

	// loopback_dump_reg();
	rt3052_phy_test	= PHY_TEST_ENABLE;
	eth_loopback_mode = 0;

	if (argc < 2 ) {
		printf("%s\n",cmdtp->usage);
		return;
	}

	if (argc == 2)
	{
		// rt3052_ether_setup();
		result = 0;
		port_no = simple_strtoul(argv[1], NULL, 10);
		//test_len = rt2880_eth_recv(rt2880_pdev);
		//printf("Phy Test Start -- %d %d\n", port_no, test_len);
#if 0
		test_len = 64;
		rt3052_ether_loopback_send(port_no, test_len);
		test_len = rt2880_eth_recv(rt2880_pdev);
#endif

                udelay(100000);
		test_len = 64;
		if(port_no == 2)
		{
			udelay(200000);//1-based port_no2 need additional delay
		}
		rt3052_ether_loopback_send(port_no, test_len);
                udelay(50000);

		test_len = rt2880_eth_recv(rt2880_pdev);
		printf("Port#%d recv len for 64bytes - %d\n",port_no, test_len);

		if ( test_len  == 60)//switch remove vlan 
			result++;
                udelay(100000);
		test_len = 1518;
		rt3052_ether_loopback_send(port_no, test_len);
                udelay(50000);

		test_len = rt2880_eth_recv(rt2880_pdev);
		printf("Port#%d recv len for 1518bytes - %d\n",port_no, test_len);
		for ( i = 0; i < 1000; i++)
			test_nop();
		if ((test_len  == 1514) && (rt3052_phy_test_ret_code ==0) )
			result++;
#if 0
		if ( result == 2 )
			printf("\n--> Loopback packet received\n");
			printf("---OK\n\r");
		else
			printf("---Fail\n\r");
			// printf("\n--> error %d\n\n", result);
#endif
		rt3052_port_test_status = result;
		return 0;
	}

	if (argc == 3)
		test_len = 64;

	if (argc == 4)
		test_len = simple_strtoul(argv[3], NULL, 10);

	if (test_len < 64)
		test_len = 64;

	port_no = simple_strtoul(argv[1], NULL, 10);
	counter = simple_strtoul(argv[2], NULL, 10);

	for ( i = 0; i < counter; i++) {
		rt3052_ether_loopback_send(port_no, test_len);
		test_len = rt2880_eth_recv(rt2880_pdev);
		printf("%d len : %d\n", i, test_len);
	}

	rt3052_phy_test	= PHY_TEST_DISABLE;
	eth_loopback_mode = 0;

	// loopback_dump_reg();
}

U_BOOT_CMD(
        phytest, 4, 1,rt3052_phy_loopback_routine,
        "phytest - rt3052 loopback phy test\n",
        "phytest usage:\n"
        "  phytest <port #> : Port # Loopback test.\n"
);

int mac_link_status_check()
{
	u32 port_ability;
	int data;

	port_ability = *(unsigned long *)(0xb0110080);
	data = (port_ability & 0x3e000000);
	return data;
}

int phy_mdio_link_check(u32 phy_addr)
{
	u32 mdio_value;
	int ret;

	mii_mgr_write(0, 31, 0x8000);	//---> select local register
	mii_mgr_read(phy_addr, 1, &mdio_value);
	//printf("mdio_value=%x\n",mdio_value);
	ret = (mdio_value & 0x4);
	mii_mgr_write(0, 31, 0x0);   //select global register
	if (ret!=0)
		return 1;
	else
		return 0; 
}

void phy_link_detect()
{
	int i, data, mdio_poll_count;
	unsigned long port_ability, mem_test_info, phy_mdio_reg;
        int j=0;
	// force_phy_an(1);
	mem_test_info = *(unsigned long*)(0xb01100dc);
#define MEM_TEST_BIT	(1<<6)
	
	while(!(mem_test_info & MEM_TEST_BIT)) {
		mem_test_info = *(unsigned long*)(0xb01100dc);
		printf("#");
	}
	printf("Port ability - 0x%08x\n", *(unsigned long*)(0xb0110080));

#if 0
	while (!(phy_mdio_link_check())) 
		test_nop();
#else
	while (1)
	{
		test_nop();
		i = phy_mdio_link_check(0);
		if ( i )
			break;
		j++;
		if(j>1000)
		{
			printf("j=%d Timeout Port0  phy_mdio_link_check\n\r", j);
		        break;
		}
	}

#endif
}

/*
	mode: 0 - 10Mb, 1 - 100Mb
 */
void force_phy_an(int mode)
{

	int i, port;
	unsigned long mac_port_ability;
#if 0
	if ( mode == 1)
		*(unsigned long*) 0xb0110084 = 0xbf80bf1f;
	else
		*(unsigned long*) 0xb0110084 = 0xbf80bf00;
#endif
	mii_mgr_write(0, 31, 0x8000);	//---> select local register
	for ( port = 0; port < 5; port++) {
		mii_mgr_write(port, 21, 0x6f);
		test_nop();
		if ( mode == 1)
			mii_mgr_write(port,  0, 0x2100);//force 100M
		else
			mii_mgr_write(port,  0, 0x100);//force 10M
		test_nop();
		mii_mgr_write(port, 26, 0x1203);
	}

	if ( mode == 1 )
	{
		while(1) 
		{
			test_nop();
			mac_port_ability = *(unsigned long *)(0xb0110080);
			printf("-- Force 100Mb phy ...POA:0x%08x\n", mac_port_ability);
			if ( mac_port_ability & 0x1f )
				break;
		}
	} 
	else 
	{
		while(1) 
		{
			test_nop();
			mac_port_ability = *(unsigned long *)(0xb0110080);
			i = ~(mac_port_ability & 0x1f);
			printf("-- Force 10Mb phy ...POA:0x%08x\n", mac_port_ability);
			if ( i != 0)
				break;
		}
	}

	for ( i = 0; i < 10; i++)
		test_nop();
}

void test_nop()
{
    int i=0;
    for(i=0; i<60; i++)
    {
	;
    }
}



/*
 * RT3052/RT3352 use different GPIO ouput to idicate results
 */
#if defined (RT3052_ASIC_BOARD) 
void gpio_led_init()
{
	// switch gpio mode from uart full to gpio
	*(unsigned long*)(0xb0000060) |= 0x1f;
	// configure uart/gpio pin to output mode
	*(unsigned long*)(0xb0000624) |= 0x1f80;
}

void gpio_led_set(unsigned long reg_val)
{
	*(unsigned long*)(0xb0000620) = reg_val;
}

void light_led(int led_no)
{
	led_no += 8;
	*(unsigned long*)(0xb0000620) |= (1 << led_no);
}

void rt3052_phy_batch(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int j, i, result, i_success_count = 0;
	char phy_test_cmd[100];
	uchar port_test_10M[5];
	uchar port_test_100M[5];
	unsigned long gpio_led, reg_counter, mdio_value;
#if 1
	// *(unsigned long *)(0xb0110084) = 0xffd01f00;
	gpio_led_init();
	gpio_led_set(0);
	gpio_led_set(0x200);
#endif
	rt3052_ether_setup();
	force_phy_an(0); //force 10M before send packets
        printf("Port ability - 0x%08x\n", *(unsigned long*)(0xb0110080));
	
	gpio_led_set(0x400);

	gpio_led = 0;
	// 10Mb Test
	// *(unsigned long *)(0xb0110084) = 0xffdf1f00;
	for (i = 1; i <=5; i++) {
		sprintf(phy_test_cmd, "phytest %d", i);
		run_command(phy_test_cmd, 0);
		// test_nop();
		if (rt3052_port_test_status == 2)
			port_test_10M[(i-1)] = 1;
		else
			port_test_10M[(i-1)] = 0;
	}

	// display the result to console!
	gpio_led = 0;
	printf("\n");
	for(i = 0; i < 5; i++)
	{
		printf("Port %d PHY Test", i);

		if (port_test_10M[i] == 1) {
			printf("...ok\n");
			i_success_count++;
		}
		else {
			printf("...failed\n");
		}
	}

	printf("\n");
	//udelay(1000);

	if ( i_success_count == 5)
		gpio_led |= 0x200;

                gpio_led_set(gpio_led);


/* force to 100M*/

	force_phy_an(1);
	phy_link_detect();

	// 100Mb Test
	i_success_count = 0;
	for (i = 1; i <=5; i++) {
		sprintf(phy_test_cmd, "phytest %d", i);
		run_command(phy_test_cmd, 0);
		// test_nop();
		if (rt3052_port_test_status == 2)
			port_test_100M[(i-1)] = 1;
		else
			port_test_100M[(i-1)] = 0;
	}
	printf("\n");
	for(i = 0; i < 5; i++)
	{
		printf("Port %d PHY Test", i);

		if (port_test_100M[i] == 1) {
			printf("...ok\n");
			i_success_count++;
		}
		else {
			printf("...failed\n");
		}
	}

	gpio_led_set(0);

        if (i_success_count == 5) 
	{
		gpio_led |= 0x800;
                gpio_led_set(gpio_led);
                printf("\n** test ok**\n");
        }

}


#elif defined (RT3352_ASIC_BOARD)

void gpio_led_init()
{
	// just switch uart full to gpio
	*(unsigned long*)(0xb0000060) |= 0x1c;
	// configure uart/gpio 9~14 pin to output mode
	*(unsigned long*)(0xb0000624) |= 0x7e00;
}

void gpio_led_set(unsigned long reg_val)
{
	*(unsigned long*)(0xb0000620) = reg_val;
}

void rt3052_phy_batch(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int j, i, result, i_success_count = 0;
	char phy_test_cmd[100];
	uchar port_test_10M[5];
	uchar port_test_100M[5];
	unsigned long gpio_led, reg_counter, mdio_value;
#if 1
	// *(unsigned long *)(0xb0110084) = 0xffd01f00;
	gpio_led_init();
	gpio_led_set(0);
#endif
	rt3052_ether_setup();
	force_phy_an(0); //force 10M before send packets
        printf("Port ability - 0x%08x\n", *(unsigned long*)(0xb0110080));
	
	gpio_led = 0x80;//initialize 
	gpio_led_set(gpio_led);//GPIO7 start signal

	// 10Mb Test
	// *(unsigned long *)(0xb0110084) = 0xffdf1f00;
	for (i = 1; i <=5; i++) {
		sprintf(phy_test_cmd, "phytest %d", i);
		run_command(phy_test_cmd, 0);
		// test_nop();
		if (rt3052_port_test_status == 2)
			port_test_10M[(i-1)] = 1;
		else
			port_test_10M[(i-1)] = 0;
	}

	// display the result to console!
	printf("\n");
	for(i = 0; i < 5; i++)
	{
		printf("Port %d PHY Test", i);

		if (port_test_10M[i] == 1) {
			printf("...ok\n");
			i_success_count++;
		}
		else {
			printf("...failed\n");
		}
	}
	printf("\n");

/* force to 100M*/

	force_phy_an(1);
	phy_link_detect();

	// 100Mb Test
	i_success_count = 0;
	for (i = 1; i <=5; i++) {
		sprintf(phy_test_cmd, "phytest %d", i);
		run_command(phy_test_cmd, 0);
		// test_nop();
		if (rt3052_port_test_status == 2)
			port_test_100M[(i-1)] = 1;
		else
			port_test_100M[(i-1)] = 0;
	}
	printf("\n");
	for(i = 0; i < 5; i++)
	{
		printf("Port %d PHY Test", i);

		if (port_test_100M[i] == 1) {
			printf("...ok\n");
			i_success_count++;
		}
		else {
			printf("...failed\n");
		}
	}

/*test results
 *GPIO9 -> port0
 *GPIO10 -> port1
 *GPIO11 -> port2
 *GPIO12 -> port3
 *GPIO13 -> port4
 *
 *GPIO14 -> all ports
 */
	i_success_count = 0;
	for(i = 0; i < 5; i++)
        {
		if ((port_test_100M[i] == 1) && (port_test_10M[i] == 1))
		{
			gpio_led |= (1 << (i+9));
			i_success_count++;
		}	
	}
        if (i_success_count == 5) 
	{
		gpio_led |= (1 << 14);
        }

        //printf("gpio led is  0x%x \n\r", gpio_led);
        
	//set results to gpio
	gpio_led_set(gpio_led);
}


#endif

U_BOOT_CMD(
	rt3052_phy, 3, 1, rt3052_phy_batch,
	"rt3052_phy - RT3052 phy production test\n",
	"rt3052_phy usage:\n"
	" rt3052_phy <option> :\n <option> 1/2 - 10/100-full"
);



// run_command("rt3052_phy", 0);
void over_night_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	for(;;) {
		run_command("rt3052_phy", 0);
		printf("-----\n\n");
	}
		
}

U_BOOT_CMD(
	test_all, 3, 1, over_night_test,
	"test_all - RT3052 phy production test, overnight test\n",
	"test_all usage:\n"
	" test_all"
);
#endif // RT3052_PHY_TEST //

#endif	/* CFG_CMD_NET && CONFIG_TULIP */
