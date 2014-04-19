EXTRA_CFLAGS = -Idrivers/net/wireless/mt7610/include
ifeq ($(CONFIG_MT7610_AP_ATE),y)
EXTRA_CFLAGS = -Idrivers/net/wireless/mt7610/ate/include
endif

obj-$(CONFIG_MT7610_AP) += mt7610_ap.o
mt7610_ap-objs += ../mt7610/rate_ctrl/ra_ctrl.o
mt7610_ap-objs += ../mt7610/rate_ctrl/alg_grp.o
mt7610_ap-objs += ../mt7610/rate_ctrl/alg_legacy.o
mt7610_ap-objs += ../mt7610/mgmt/mgmt_entrytb.o
mt7610_ap-objs += ../mt7610/mgmt/mgmt_hw.o
mt7610_ap-objs += ../mt7610/mgmt/mgmt_vht.o
mt7610_ap-objs += ../mt7610/mgmt/mgmt_ht.o
mt7610_ap-objs += ../mt7610/os/linux/ap_ioctl.o
mt7610_ap-objs += ../mt7610/os/linux/rt_rbus_pci_drv.o
mt7610_ap-objs += ../mt7610/os/linux/rt_pci_rbus.o
mt7610_ap-objs += ../mt7610/os/linux/rt_rbus_pci_util.o
mt7610_ap-objs += ../mt7610/os/linux/pci_main_dev.o
mt7610_ap-objs += ../mt7610/os/linux/rt_profile.o
mt7610_ap-objs += ../mt7610/os/linux/rt_linux.o
mt7610_ap-objs += ../mt7610/os/linux/rt_proc.o
mt7610_ap-objs += ../mt7610/os/linux/rt_main_dev.o
mt7610_ap-objs += ../mt7610/mcu/mcu.o
mt7610_ap-objs += ../mt7610/mcu/mcu_and.o
mt7610_ap-objs += ../mt7610/common/cmm_data_pci.o
mt7610_ap-objs += ../mt7610/common/eeprom.o
mt7610_ap-objs += ../mt7610/common/cmm_profile.o
mt7610_ap-objs += ../mt7610/common/cmm_sync.o
mt7610_ap-objs += ../mt7610/common/rtmp_init_inf.o
mt7610_ap-objs += ../mt7610/common/cmm_cfg.o
mt7610_ap-objs += ../mt7610/common/rt_rf.o
mt7610_ap-objs += ../mt7610/common/action.o
mt7610_ap-objs += ../mt7610/common/rtmp_init.o
mt7610_ap-objs += ../mt7610/common/txpower.o
mt7610_ap-objs += ../mt7610/common/cmm_cmd.o
mt7610_ap-objs += ../mt7610/common/rtmp_timer.o
mt7610_ap-objs += ../mt7610/common/cmm_info.o
mt7610_ap-objs += ../mt7610/common/ba_action.o
mt7610_ap-objs += ../mt7610/common/cmm_mac_pci.o
mt7610_ap-objs += ../mt7610/common/cmm_tkip.o
mt7610_ap-objs += ../mt7610/common/cmm_asic.o
mt7610_ap-objs += ../mt7610/common/vht.o
mt7610_ap-objs += ../mt7610/common/cmm_aes.o
mt7610_ap-objs += ../mt7610/common/crypt_md5.o
mt7610_ap-objs += ../mt7610/common/cmm_wep.o
mt7610_ap-objs += ../mt7610/common/crypt_arc4.o
mt7610_ap-objs += ../mt7610/common/crypt_hmac.o
mt7610_ap-objs += ../mt7610/common/cmm_wpa.o
mt7610_ap-objs += ../mt7610/common/ps.o
mt7610_ap-objs += ../mt7610/common/cmm_sanity.o
mt7610_ap-objs += ../mt7610/common/cmm_radar.o
mt7610_ap-objs += ../mt7610/common/crypt_aes.o
mt7610_ap-objs += ../mt7610/common/ee_efuse.o
mt7610_ap-objs += ../mt7610/common/cmm_data.o
mt7610_ap-objs += ../mt7610/common/spectrum.o
mt7610_ap-objs += ../mt7610/common/crypt_sha2.o
mt7610_ap-objs += ../mt7610/common/mlme.o
mt7610_ap-objs += ../mt7610/common/ee_prom.o
mt7610_ap-objs += ../mt7610/common/rt_os_util.o
mt7610_ap-objs += ../mt7610/common/rt_channel.o
mt7610_ap-objs += ../mt7610/common/scan.o
mt7610_ap-objs += ../mt7610/common/uapsd.o
mt7610_ap-objs += ../mt7610/common/ee_flash.o
mt7610_ap-objs += ../mt7610/ap/ap_qload.o
mt7610_ap-objs += ../mt7610/ap/ap_auth.o
mt7610_ap-objs += ../mt7610/ap/ap_sync.o
mt7610_ap-objs += ../mt7610/ap/ap_mlme.o
mt7610_ap-objs += ../mt7610/ap/ap_wpa.o
mt7610_ap-objs += ../mt7610/ap/ap_assoc.o
mt7610_ap-objs += ../mt7610/ap/ap_cfg.o
mt7610_ap-objs += ../mt7610/ap/ap_autoChSel.o
mt7610_ap-objs += ../mt7610/ap/ap_sanity.o
mt7610_ap-objs += ../mt7610/ap/ap.o
mt7610_ap-objs += ../mt7610/ap/ap_data.o
mt7610_ap-objs += ../mt7610/ap/ap_connect.o
mt7610_ap-objs += ../mt7610/phy/rtmp_phy.o
mt7610_ap-objs += ../mt7610/phy/rlt_rf.o
mt7610_ap-objs += ../mt7610/phy/rlt_phy.o
mt7610_ap-objs += ../mt7610/mac/ral_nmac.o
mt7610_ap-objs += ../mt7610/mac/rtmp_mac.o

ifeq ($(CONFIG_MT7610_AP_ATE),y)
mt7610_ap-objs += ../mt7610/ate/common/rt_ate.o
mt7610_ap-objs += ../mt7610/ate/common/ate_pci.o
ifeq ($(CONFIG_MT7610_AP_QA),y)
mt7610_ap-objs += ../mt7610/ate/common/rt_qa.o
endif
endif

### MT7610
mt7610_ap-objs += ../mt7610/chips/rtmp_chip.o
mt7610_ap-objs += ../mt7610/chips/rt65xx.o
mt7610_ap-objs += ../mt7610/chips/mt76x0.o
ifeq ($(CONFIG_MT7610_AP_ATE),y)
mt7610_ap-objs += ../mt7610/ate/chips/rt6590_ate.o
endif

ifeq ($(CONFIG_MT7610_AP_VIDEO_TURBINE),y)
mt7610_ap-objs += ../mt7610/common/cmm_video.o
endif

ifeq ($(CONFIG_MT7610_AP_LED),y)
mt7610_ap-objs += ../mt7610/common/rt_led.o
endif

ifeq ($(CONFIG_MT7610_AP_WSC),y)
mt7610_ap-objs += ../mt7610/common/wsc.o
mt7610_ap-objs += ../mt7610/common/wsc_tlv.o
mt7610_ap-objs += ../mt7610/common/crypt_dh.o
mt7610_ap-objs += ../mt7610/common/wsc_ufd.o
mt7610_ap-objs += ../mt7610/common/crypt_biginteger.o
ifeq ($(CONFIG_MT7610_AP_WSC_V2),y)
mt7610_ap-objs += ../mt7610/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_MT7610_AP_WDS),y)
mt7610_ap-objs += ../mt7610/ap/ap_wds.o
mt7610_ap-objs += ../mt7610/ap/ap_wds_inf.o
mt7610_ap-objs += ../mt7610/common/client_wds.o
endif

ifeq ($(CONFIG_MT7610_AP_MBSS),y)
mt7610_ap-objs += ../mt7610/ap/ap_mbss.o
mt7610_ap-objs += ../mt7610/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_MT7610_AP_APCLI),y)
mt7610_ap-objs += ../mt7610/ap/ap_apcli.o
mt7610_ap-objs += ../mt7610/ap/ap_apcli_inf.o
mt7610_ap-objs += ../mt7610/ap/apcli_assoc.o
mt7610_ap-objs += ../mt7610/ap/apcli_auth.o
mt7610_ap-objs += ../mt7610/ap/apcli_ctrl.o
mt7610_ap-objs += ../mt7610/ap/apcli_sync.o
mt7610_ap-objs += ../mt7610/common/cmm_mat.o
mt7610_ap-objs += ../mt7610/common/cmm_mat_iparp.o
mt7610_ap-objs += ../mt7610/common/cmm_mat_pppoe.o
mt7610_ap-objs += ../mt7610/common/cmm_mat_ipv6.o
endif

ifeq ($(CONFIG_MT7610_AP_IGMP_SNOOP),y)
mt7610_ap-objs += ../mt7610/common/igmp_snoop.o
endif

ifeq ($(CONFIG_MT7610_AP_NETIF_BLOCK),y)
mt7610_ap-objs += ../mt7610/common/netif_block.o
endif

ifeq ($(CONFIG_MT7610_AP_DLS),y)
mt7610_ap-objs += ../mt7610/ap/ap_dls.o
endif

ifeq ($(CONFIG_MT7610_AP_IDS),y)
mt7610_ap-objs += ../mt7610/ap/ap_ids.o
endif

ifeq ($(CONFIG_MT7610_AP_WAPI),y)
mt7610_ap-objs += ../mt7610/common/wapi.o
mt7610_ap-objs += ../mt7610/common/wapi_sms4.o
mt7610_ap-objs += ../mt7610/common/wapi_crypt.o
mt7610_ap-objs += ../mt7610/common/libwapi.a
endif

ifeq ($(CONFIG_MT7610_AP_DFS),y)
mt7610_ap-objs += ../mt7610/common/cmm_dfs.o
endif

ifeq ($(CONFIG_MT7610_AP_CARRIER),y)
mt7610_ap-objs += ../mt7610/common/cmm_cs.o
endif

ifeq ($(CONFIG_MT7610_AP_CSO),y)
mt7610_ap-objs += ../mt7610/naf/net_acc.o
endif

###################
#  CFLAGS
##################

EXTRA_CFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DUAPSD_SUPPORT -DDOT11_N_SUPPORT -DDOT1X_SUPPORT -DDOT11_VHT_AC
EXTRA_CFLAGS += -DAP_SCAN_SUPPORT -DSCAN_SUPPORT
EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DRSSI_FEEDBACK
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DRA_NAT_SUPPORT

# do not lock CountryRegion from EEPROM
EXTRA_CFLAGS += -DEEPROM_COUNTRY_UNLOCK

# provide external Flash memory support
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT

# provide internal eFuse memory support
#EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT

# provide roaming support
#EXTRA_CFLAGS += -DIAPP_SUPPORT

ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

### MT7610
EXTRA_CFLAGS += -DMT76x0 -DRT65xx -DMT7610 -DRLT_MAC -DRLT_RF
EXTRA_CFLAGS += -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRX_DMA_SCATTER -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DDYNAMIC_VGA_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT
#EXTRA_CFLAGS += -DFPGA_MODE
#EXTRA_CFLAGS += -DHDR_TRANS_SUPPORT
#EXTRA_CFLAGS += -DCONFIG_CSO_SUPPORT -DCONFIG_TSO_SUPPORT

ifeq ($(CONFIG_MT7610_AP_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

ifeq ($(CONFIG_MT7610_AP_DBG),y)
EXTRA_CFLAGS += -DDBG
EXTRA_CFLAGS += -DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DCONFIG_RT2880_ATE_CMD_NEW
ifeq ($(CONFIG_MT7610_AP_QA),y)
EXTRA_CFLAGS += -DRALINK_QA
endif
endif

ifeq ($(CONFIG_MT7610_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_DFS),y)
EXTRA_CFLAGS += -DDFS_SUPPORT
EXTRA_CFLAGS += -DDFS_DEBUG
endif

ifeq ($(CONFIG_MT7610_AP_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT -DSOFT_ENCRYPT
endif

ifeq ($(CONFIG_MT7610_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_CARRIER),y)
EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_AUTO_CH_SELECT_ENHANCE),y)
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_MT7610_AP_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_MT7610_AP_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_MT7610_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
ifeq ($(CONFIG_MT7610_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_MT7610_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
ifeq ($(CONFIG_MT7610_AP_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
ifeq ($(CONFIG_MT7610_AP_CON_WPS_SUPPORT),y)
EXTRA_CFLAGS += -DCON_WPS
endif
endif

ifeq ($(CONFIG_MT7610_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
ifeq ($(CONFIG_MT7610_AP_MBSS_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
endif
endif

ifeq ($(CONFIG_MT7610_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT
EXTRA_CFLAGS += -DMAT_SUPPORT
ifeq ($(CONFIG_MT7610_AP_MAC_REPEATER),y)
EXTRA_CFLAGS += -DMAC_APCLI_SUPPORT
EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT
EXTRA_CFLAGS += -DAPCLI_CONNECTION_TRIAL
#EXTRA_CFLAGS += -DAPCLI_AUTO_CONNECT_SUPPORT
endif
endif

ifeq ($(CONFIG_MT7610_AP_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_MT7610_AP_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_MT7610_AP_SINGLE_SKU),y)
EXTRA_CFLAGS += -DSINGLE_SKU_V2
endif

ifeq ($(CONFIG_MT7610_AP_TSSI_COMPENSATION),y)
EXTRA_CFLAGS += -DMT76x0_TSSI_CAL_COMPENSATION
endif

ifeq ($(CONFIG_MT7610_AP_TEMPERATURE_COMPENSATION),y)
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_COMPENSATION
endif

ifeq ($(CONFIG_MT7610_AP_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_MT7610_AP_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_BIG_ENDIAN),y)
EXTRA_CFLAGS += -DRT_BIG_ENDIAN
endif
