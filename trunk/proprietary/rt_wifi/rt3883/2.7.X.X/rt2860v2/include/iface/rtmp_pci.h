/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rtmp_pci.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RTMP_PCI_H__
#define __RTMP_PCI_H__

#define RT28XX_HANDLE_DEV_ASSIGN(handle, dev_p)				\
	((POS_COOKIE)handle)->pci_dev = dev_p;


#ifdef LINUX
/* set driver data */
#define RT28XX_DRVDATA_SET(_a)			pci_set_drvdata(_a, net_dev);

#define RT28XX_PUT_DEVICE(dev_p)



#define RTMP_PCI_DEV_UNMAP()										\
{	if (net_dev->base_addr)	{								\
		iounmap((void *)(net_dev->base_addr));				\
		release_mem_region(pci_resource_start(dev_p, 0),	\
							pci_resource_len(dev_p, 0)); }	\
	if (net_dev->irq) pci_release_regions(dev_p); }


#define PCI_REG_READ_WORD(pci_dev, offset, Configuration)   \
    if (pci_read_config_word(pci_dev, offset, &reg16) == 0)     \
        Configuration = le2cpu16(reg16);                        \
    else                                                        \
        Configuration = 0;

#define PCI_REG_WIRTE_WORD(pci_dev, offset, Configuration)  \
    reg16 = cpu2le16(Configuration);                        \
    pci_write_config_word(pci_dev, offset, reg16);

#endif /* LINUX */



// TODO: shiang, for RT3290, make sure following definition is correct to put as here

#define ASIC_VERSION	0x0000

#define CMB_CTRL		0x20
#ifdef RT_BIG_ENDIAN
typedef union _CMB_CTRL_STRUC{
	struct{
		UINT32       	LDO0_EN:1;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	PLL_LD:1;
		UINT32       	XTAL_RDY:1;
		UINT32       	Rsv:2;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_LEVEL:2;
		UINT32		AUX_OPT_Bit15_Two_AntennaMode:1;
		UINT32		AUX_OPT_Bit14_TRSW1_as_GPIO:1;
		UINT32		AUX_OPT_Bit13_GPIO7_as_GPIO:1;
		UINT32		AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;
		UINT32		AUX_OPT_Bit11_Rsv:1;
		UINT32		AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1;
		UINT32		AUX_OPT_Bit9_GPIO3_as_GPIO:1;
		UINT32		AUX_OPT_Bit8_AuxPower_Exists:1;
		UINT32		AUX_OPT_Bit7_KeepInterfaceClk:1;
		UINT32		AUX_OPT_Bit6_KeepXtal_On:1;
		UINT32		AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;
		UINT32		AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;
		UINT32		AUX_OPT_Bit3_PLLOn_L1:1;
		UINT32		AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
		UINT32		AUX_OPT_Bit1_PCIePhyClkOn_L1:1;	
		UINT32		AUX_OPT_Bit0_InterfaceClk_40Mhz:1;
	}field;
	UINT32 word;
}CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#else
typedef union _CMB_CTRL_STRUC{
	struct{
		UINT32		AUX_OPT_Bit0_InterfaceClk_40Mhz:1;
		UINT32		AUX_OPT_Bit1_PCIePhyClkOn_L1:1;	
		UINT32		AUX_OPT_Bit2_PCIeCoreClkOn_L1:1;
		UINT32		AUX_OPT_Bit3_PLLOn_L1:1;
		UINT32		AUX_OPT_Bit4_RemovePCIePhyClk_WLANOff:1;
		UINT32		AUX_OPT_Bit5_RemovePCIePhyClk_BTOff:1;
		UINT32		AUX_OPT_Bit6_KeepXtal_On:1;
		UINT32		AUX_OPT_Bit7_KeepInterfaceClk:1;
		UINT32		AUX_OPT_Bit8_AuxPower_Exists:1;
		UINT32		AUX_OPT_Bit9_GPIO3_as_GPIO:1;
		UINT32		AUX_OPT_Bit10_NotSwap_WL_LED_ACT_RDY:1;	
		UINT32		AUX_OPT_Bit11_Rsv:1;
		UINT32		AUX_OPT_Bit12_TRSW0_as_WLAN_ANT_SEL:1;
		UINT32		AUX_OPT_Bit13_GPIO7_as_GPIO:1;
		UINT32		AUX_OPT_Bit14_TRSW1_as_GPIO:1;
		UINT32		AUX_OPT_Bit15_Two_AntennaMode:1;
		UINT32		LDO25_LEVEL:2;
		UINT32		LDO25_LARGEA:1;
		UINT32		LDO25_FRC_ON:1;//4      
		UINT32       	Rsv:2;
		UINT32       	XTAL_RDY:1;
		UINT32       	PLL_LD:1;
		UINT32       	LDO_CORE_LEVEL:4;
		UINT32       	LDO_BGSEL:2;
		UINT32       	LDO3_EN:1;
		UINT32       	LDO0_EN:1;
	}field;
	UINT32 word;
}CMB_CTRL_STRUC, *PCMB_CTRL_STRUC;
#endif


#define EFUSE_CTRL_3290		0x24
#define EFUSE_DATA0_3290		0x28
#define EFUSE_DATA1_3290		0x2c
#define EFUSE_DATA2_3290		0x30
#define EFUSE_DATA3_3290		0x34


#define OSCCTL				0x38
#ifdef RT_BIG_ENDIAN
typedef	union _OSCCTL_STRUC{
	struct{
		UINT32       	ROSC_EN:1;
		UINT32       	CAL_REQ:1;
		UINT32       	CLK_32K_VLD:1;
		UINT32		CAL_ACK:1;
		UINT32		CAL_CNT:12;
		UINT32		Rsv:3;
		UINT32		REF_CYCLE:13;
	}field;
	UINT32 word;
}OSCCTL_STRUC, *POSCCTL_STRUC;
#else
typedef union _OSCCTL_STRUC{
	struct{
		UINT32		REF_CYCLE:13;
		UINT32		Rsv:3;
		UINT32		CAL_CNT:12;
		UINT32		CAL_ACK:1;
		UINT32       	CLK_32K_VLD:1;
		UINT32       	CAL_REQ:1;
		UINT32       	ROSC_EN:1;
	}field;
	UINT32 word;
}OSCCTL_STRUC, *POSCCTL_STRUC;
#endif


#define COEXCFG0			0x40
#ifdef RT_BIG_ENDIAN
typedef union _COEXCFG0_STRUC{
	struct{
		UINT32       	COEX_CFG1:8;
		UINT32       	COEX_CFG0:8;		
		UINT32       	FIX_WL_RF_LNA:2;
		UINT32		FIX_BT_H_PA:3;
		UINT32		FIX_BT_L_PA:3;
		UINT32		FIX_WL_TX_PWR:2;
		UINT32		Rsv:3;
		UINT32		FIX_WL_ANT_EN:1;
		UINT32		FIX_WL_DI_ANT:1;
		UINT32		COEX_ENT:1;
	}field;
	UINT32 word;
}COEXCFG0_STRUC, *PCOEXCFG0_STRUC;
#else
typedef union _COEXCFG0_STRUC{
	struct{
		UINT32		COEX_ENT:1;
		UINT32		FIX_WL_DI_ANT:1;
		UINT32		FIX_WL_ANT_EN:1;
		UINT32		Rsv:3;
		UINT32		FIX_WL_TX_PWR:2;
		UINT32		FIX_BT_L_PA:3;
		UINT32		FIX_BT_H_PA:3;
		UINT32       	FIX_WL_RF_LNA:2;

		UINT32       	COEX_CFG0:8;
		UINT32       	COEX_CFG1:8;
	}field;
	UINT32 word;
}COEXCFG0_STRUC, *PCOEXCFG0_STRUC;
#endif


#define COEXCFG1			0x44
#ifdef RT_BIG_ENDIAN
typedef union _COEXCFG1_STRUC{
	struct{
		UINT32       	Rsv:8;
		UINT32		DIS_WL_RF_DELY:8;
		UINT32		DIS_WL_PA_DELY:8;
		UINT32		DIS_WL_TR_DELY:8;
	}field;
	UINT32 word;
}COEXCFG1_STRUC, *PCOEXCFG1_STRUC;
#else
typedef union _COEXCFG1_STRUC{
	struct{
		UINT32		DIS_WL_TR_DELY:8;
		UINT32		DIS_WL_PA_DELY:8;
		UINT32		DIS_WL_RF_DELY:8;
		UINT32       	Rsv:8;
	}field;
	UINT32 word;
}COEXCFG1_STRUC, *PCOEXCFG1_STRUC;
#endif


#define COEXCFG2			0x48
#ifdef RT_BIG_ENDIAN
typedef union _COEXCFG2_STRUC{
	struct{
		UINT32		BT_COEX_CFG1_Bit31_Rsv:1;
		UINT32		BT_COEX_CFG1_Bit30_Rsv:1;	
		UINT32		BT_COEX_CFG1_Bit29_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit28_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit27_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG1_Bit26_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit25_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit24_HaltLowPriorityTx_wl_busy:1;		
		
		UINT32		BT_COEX_CFG0_Bit23_Rsv:1;
		UINT32		BT_COEX_CFG0_Bit22_Rsv:1;	
		UINT32		BT_COEX_CFG0_Bit21_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit20_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit19_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG0_Bit18_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit17_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit16_HaltLowPriorityTx_wl_busy:1;
		
		UINT32		WL_COEX_CFG1_Bit15_LowerTxPwr_bt_high_priority:1;
		UINT32		WL_COEX_CFG1_Bit14_Enable_Tx_free_timer:1;
		UINT32		WL_COEX_CFG1_Bit13_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG1_Bit12_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit11_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG1_Bit10_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit9_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG1_Bit8_HaltTx_bt_rx_busy:1;		
		
		UINT32		WL_COEX_CFG0_Bit7_LowerTxPwr_bt_high_priority:1;
		UINT32		WL_COEX_CFG0_Bit6_Enable_Tx_free_timer:1;
		UINT32		WL_COEX_CFG0_Bit5_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG0_Bit4_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit3_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG0_Bit2_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit1_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG0_Bit0_HaltTx_bt_rx_busy:1;			
	}field;
	UINT32 word;
}COEXCFG2_STRUC, *PCOEXCFG2_STRUC;
#else
typedef union _COEXCFG2_STRUC{
	struct{
		UINT32		WL_COEX_CFG0_Bit0_HaltTx_bt_rx_busy:1;	
		UINT32		WL_COEX_CFG0_Bit1_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG0_Bit2_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit3_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG0_Bit4_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG0_Bit5_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG0_Bit6_Enable_Tx_free_timer:1;		
		UINT32		WL_COEX_CFG0_Bit7_LowerTxPwr_bt_high_priority:1;

		UINT32		WL_COEX_CFG1_Bit8_HaltTx_bt_rx_busy:1;
		UINT32		WL_COEX_CFG1_Bit9_HaltTx_bt_rx_req_h:1;	
		UINT32		WL_COEX_CFG1_Bit10_HaltTx_bt_tx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit11_HaltTx_bt_tx_req_l:1;
		UINT32		WL_COEX_CFG1_Bit12_Disable_bt_rx_req_h:1;
		UINT32		WL_COEX_CFG1_Bit13_Disable_TxAgg_bi_high_priority:1;
		UINT32		WL_COEX_CFG1_Bit14_Enable_Tx_free_timer:1;		
		UINT32		WL_COEX_CFG1_Bit15_LowerTxPwr_bt_high_priority:1;
		
		UINT32		BT_COEX_CFG0_Bit16_HaltLowPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG0_Bit17_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit18_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit19_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG0_Bit20_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG0_Bit21_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG0_Bit22_Rsv:1;	
		UINT32		BT_COEX_CFG0_Bit23_Rsv:1;
		
		UINT32		BT_COEX_CFG1_Bit24_HaltLowPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG1_Bit25_HaltLowPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit26_HaltLowPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit27_HaltHighPriorityTx_wl_busy:1;
		UINT32		BT_COEX_CFG1_Bit28_HaltHighPriorityTx_wl_rx_busy:1;
		UINT32		BT_COEX_CFG1_Bit29_HaltHighPriorityTx_wl_bcn_busy:1;
		UINT32		BT_COEX_CFG1_Bit30_Rsv:1;			
		UINT32		BT_COEX_CFG1_Bit31_Rsv:1;
	}field;
	UINT32 word;
}COEXCFG2_STRUC, *PCOEXCFG2_STRUC;
#endif


#define PLL_CTRL		0x50
#ifdef RT_BIG_ENDIAN
typedef	union	_PLL_CTRL_STRUC	{
	struct	{
		ULONG		VBGBK_EN:1;
		ULONG      	LOCK_DETECT_WINDOW_CTRL:3;
		ULONG		PFD_DELAY_CTRL:2;
		ULONG		CP_CURRENT_CTRL:2;
		ULONG		LPF_C2_CTRL:2;
		ULONG		LPF_C1_CTRL:2;
		ULONG		LPF_R1:1;
		ULONG		VCO_FIXED_CURRENT_CONTROL:3;
		ULONG		RESERVED_INPUT2:8;
		ULONG		RESERVED_INPUT1:8;
	}	field;
	ULONG			word;
}	PLL_CTRL_STRUC, *PPLL_CTRL_STRUC;
#else
typedef	union	_PLL_CTRL_STRUC	{
	struct	{
		ULONG		RESERVED_INPUT1:8;
		ULONG		RESERVED_INPUT2:8;
		ULONG		VCO_FIXED_CURRENT_CONTROL:3;
		ULONG		LPF_R1:1;
		ULONG		LPF_C1_CTRL:2;
		ULONG		LPF_C2_CTRL:2;
		ULONG		CP_CURRENT_CTRL:2;
		ULONG		PFD_DELAY_CTRL:2;
		ULONG      	LOCK_DETECT_WINDOW_CTRL:3;
		ULONG		VBGBK_EN:1;
	}	field;
	ULONG			word;
} PLL_CTRL_STRUC, *PPLL_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */


#define WLAN_FUN_CTRL		0x80
#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		UINT32		GPIO0_OUT_OE_N:8;
		UINT32		GPIO0_OUT:8;
		UINT32		GPIO0_IN:8;
		UINT32		WLAN_ACC_BT:1;
		UINT32		INV_TR_SW0:1;
		UINT32		FRC_WL_ANT_SET:1;
		UINT32		PCIE_APP0_CLK_REQ:1;
		UINT32		WLAN_RESET:1;
		UINT32		Rsv1:1;
		UINT32		WLAN_CLK_EN:1;
		UINT32		WLAN_EN:1;
	}field;
	UINT32 word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#else
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		UINT32		WLAN_EN:1;
		UINT32		WLAN_CLK_EN:1;
		UINT32		Rsv1:1;
		UINT32		WLAN_RESET:1;
		UINT32		PCIE_APP0_CLK_REQ:1;
		UINT32		FRC_WL_ANT_SET:1;
		UINT32		INV_TR_SW0:1;
		UINT32		WLAN_ACC_BT:1;
		UINT32		GPIO0_IN:8;
		UINT32		GPIO0_OUT:8;
		UINT32		GPIO0_OUT_OE_N:8;
	}field;
	UINT32 word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#endif


#define WLAN_FUN_INFO		0x84
#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		UINT32		BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
		UINT32		Rsv1:26;		
		UINT32		COEX_MODE:5; /* WLAN function enable */
	}field;
	UINT32 word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#else
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		UINT32		COEX_MODE:5; /* WLAN function enable */
		UINT32		Rsv1:26;
		UINT32		BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
	}field;
	UINT32 word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#endif


#define BT_FUN_CTRL		0xC0
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_CTRL_STRUC{
	struct{
		UINT32		GPIO1_OUT_OE_N:8;
		UINT32		GPIO1_OUT:8;
		UINT32		GPIO1_IN:8;
		UINT32		BT_ACC_WLAN:1;
		UINT32		INV_TR_SW1:1;
		UINT32		URXD_GPIO_MODE:1;
		UINT32		PCIE_APP1_CLK_REQ:1;
		UINT32		BT_RESET:1;
		UINT32		BT_RF_EN:1;
		UINT32		BT_CLK_EN:1;
		UINT32		BT_EN:1;
	}field;
	UINT32 word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#else
typedef union _BT_FUN_CTRL_STRUC	{
	struct{
		UINT32		BT_EN:1;
		UINT32		BT_CLK_EN:1;
		UINT32		BT_RF_EN:1;
		UINT32		BT_RESET:1;
		UINT32		PCIE_APP1_CLK_REQ:1;
		UINT32		URXD_GPIO_MODE:1;
		UINT32		INV_TR_SW1:1;
		UINT32		BT_ACC_WLAN:1;
		UINT32		GPIO1_IN:8;
		UINT32		GPIO1_OUT:8;
		UINT32		GPIO1_OUT_OE_N:8;
	}field;
	UINT32 word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#endif


#define BT_FUN_INFO		0xC4
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_INFO_STRUC{
	struct{
		UINT32		WLAN_EEP_BUSY:1;
		UINT32		BTPower1:7;	/* Peer */
		UINT32		BTPower0:8; /* Self */
		UINT32		AFH_END_CH:8;		
		UINT32		AFH_START_CH:8;
	}field;
	UINT32 word;
} BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#else
typedef	union _BT_FUN_INFO_STRUC	{
	struct{
		UINT32		AFH_START_CH:8;
		UINT32		AFH_END_CH:8;
		UINT32		BTPower0:8;	/* Self */
		UINT32		BTPower1:7;	/* Peer */
		UINT32		WLAN_EEP_BUSY:1;			
	}field;
	UINT32 word;
}BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#endif

// TODO: shiang, this data structure is not defined for register. may can move to other place
typedef struct _WLAN_BT_COEX_SETTING
{
	BOOLEAN					ampduOff;
	BOOLEAN					coexSettingRunning;
	BOOLEAN					RateSelectionForceToUseRSSI;
	UCHAR					TxQualityFlag;
	ULONG					alc;
	ULONG					slna;
}WLAN_BT_COEX_SETTING, *PWLAN_BT_COEX_SETTING;

#endif /* __RTMP_PCI_H__ */
