EXTRA_CFLAGS = -Idrivers/net/wireless/rt5592/include
ifeq ($(CONFIG_RT5592_AP_ATE),y)
EXTRA_CFLAGS += -Idrivers/net/wireless/rt5592/ate/include
endif

obj-$(CONFIG_RT5592_AP) += rt5592_ap.o
rt5592_ap-objs += ../rt5592/common/crypt_md5.o
rt5592_ap-objs += ../rt5592/common/crypt_sha2.o
rt5592_ap-objs += ../rt5592/common/crypt_hmac.o
rt5592_ap-objs += ../rt5592/common/crypt_aes.o
rt5592_ap-objs += ../rt5592/common/crypt_arc4.o
rt5592_ap-objs += ../rt5592/common/mlme.o
rt5592_ap-objs += ../rt5592/common/cmm_wep.o
rt5592_ap-objs += ../rt5592/common/action.o
rt5592_ap-objs += ../rt5592/common/ba_action.o
rt5592_ap-objs += ../rt5592/common/cmm_data.o
rt5592_ap-objs += ../rt5592/common/rtmp_init.o
rt5592_ap-objs += ../rt5592/common/rtmp_init_inf.o
rt5592_ap-objs += ../rt5592/common/cmm_tkip.o
rt5592_ap-objs += ../rt5592/common/cmm_aes.o
rt5592_ap-objs += ../rt5592/common/cmm_sync.o
rt5592_ap-objs += ../rt5592/common/cmm_sanity.o
rt5592_ap-objs += ../rt5592/common/cmm_info.o
rt5592_ap-objs += ../rt5592/common/cmm_cfg.o
rt5592_ap-objs += ../rt5592/common/cmm_wpa.o
rt5592_ap-objs += ../rt5592/common/spectrum.o
rt5592_ap-objs += ../rt5592/common/rtmp_timer.o
rt5592_ap-objs += ../rt5592/common/rt_channel.o
rt5592_ap-objs += ../rt5592/common/cmm_profile.o
rt5592_ap-objs += ../rt5592/common/cmm_asic.o
rt5592_ap-objs += ../rt5592/common/cmm_cmd.o
rt5592_ap-objs += ../rt5592/common/rtmp_mcu.o
rt5592_ap-objs += ../rt5592/common/cmm_mac_pci.o
rt5592_ap-objs += ../rt5592/common/cmm_data_pci.o
rt5592_ap-objs += ../rt5592/common/rt_os_util.o
rt5592_ap-objs += ../rt5592/common/ps.o
rt5592_ap-objs += ../rt5592/common/uapsd.o
rt5592_ap-objs += ../rt5592/common/ee_prom.o
rt5592_ap-objs += ../rt5592/common/ee_efuse.o
rt5592_ap-objs += ../rt5592/common/ee_flash.o
rt5592_ap-objs += ../rt5592/common/eeprom.o
rt5592_ap-objs += ../rt5592/common/rt_rf.o

rt5592_ap-objs += ../rt5592/rate_ctrl/ra_ctrl.o
rt5592_ap-objs += ../rt5592/rate_ctrl/alg_legacy.o

rt5592_ap-objs += ../rt5592/ap/ap.o
rt5592_ap-objs += ../rt5592/ap/ap_assoc.o
rt5592_ap-objs += ../rt5592/ap/ap_auth.o
rt5592_ap-objs += ../rt5592/ap/ap_connect.o
rt5592_ap-objs += ../rt5592/ap/ap_mlme.o
rt5592_ap-objs += ../rt5592/ap/ap_sanity.o
rt5592_ap-objs += ../rt5592/ap/ap_sync.o
rt5592_ap-objs += ../rt5592/ap/ap_wpa.o
rt5592_ap-objs += ../rt5592/ap/ap_data.o
rt5592_ap-objs += ../rt5592/ap/ap_autoChSel.o
rt5592_ap-objs += ../rt5592/ap/ap_qload.o
rt5592_ap-objs += ../rt5592/ap/ap_cfg.o

rt5592_ap-objs += ../rt5592/os/linux/rt_proc.o
rt5592_ap-objs += ../rt5592/os/linux/rt_main_dev.o
rt5592_ap-objs += ../rt5592/os/linux/rt_pci_rbus.o
rt5592_ap-objs += ../rt5592/os/linux/rt_profile.o
rt5592_ap-objs += ../rt5592/os/linux/rt_linux.o
rt5592_ap-objs += ../rt5592/os/linux/rt_rbus_pci_util.o
rt5592_ap-objs += ../rt5592/os/linux/rt_rbus_pci_drv.o
rt5592_ap-objs += ../rt5592/os/linux/ap_ioctl.o
rt5592_ap-objs += ../rt5592/os/linux/pci_main_dev.o

ifeq ($(CONFIG_RT5592_AP_ATE),y)
rt5592_ap-objs += ../rt5592/ate/common/rt_ate.o
rt5592_ap-objs += ../rt5592/ate/common/ate_pci.o
rt5592_ap-objs += ../rt5592/ate/common/rt_qa.o
endif

### RT5592
rt5592_ap-objs += ../rt5592/chips/rtmp_chip.o
rt5592_ap-objs += ../rt5592/chips/rt30xx.o
rt5592_ap-objs += ../rt5592/chips/rt5592.o
ifeq ($(CONFIG_RT5592_AP_ATE),y)
rt5592_ap-objs += ../rt5592/ate/chips/rt30xx_ate.o
rt5592_ap-objs += ../rt5592/ate/chips/rt5592_ate.o
endif

ifeq ($(CONFIG_RT5592_AP_VIDEO_TURBINE),y)
rt5592_ap-objs += ../rt5592/common/cmm_video.o
endif

ifeq ($(CONFIG_RT5592_AP_BRIDGE_FASTPATH),y)
rt5592_ap-objs += ../rt5592/os/linux/br_ftph.o
endif

ifeq ($(CONFIG_RT5592_AP_LED),y)
rt5592_ap-objs += ../rt5592/common/rt_led.o
endif

ifeq ($(CONFIG_RT5592_AP_WSC),y)
rt5592_ap-objs += ../rt5592/common/wsc.o
rt5592_ap-objs += ../rt5592/common/wsc_tlv.o
rt5592_ap-objs += ../rt5592/common/crypt_dh.o
rt5592_ap-objs += ../rt5592/common/wsc_ufd.o
rt5592_ap-objs += ../rt5592/common/crypt_biginteger.o
ifeq ($(CONFIG_RT5592_AP_WSC_V2),y)
rt5592_ap-objs += ../rt5592/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_RT5592_AP_WDS),y)
rt5592_ap-objs += ../rt5592/ap/ap_wds.o
rt5592_ap-objs += ../rt5592/ap/ap_wds_inf.o
rt5592_ap-objs += ../rt5592/common/client_wds.o
endif

ifeq ($(CONFIG_RT5592_AP_MBSS),y)
rt5592_ap-objs += ../rt5592/ap/ap_mbss.o
rt5592_ap-objs += ../rt5592/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_RT5592_AP_APCLI),y)
rt5592_ap-objs += ../rt5592/ap/ap_apcli.o
rt5592_ap-objs += ../rt5592/ap/ap_apcli_inf.o
rt5592_ap-objs += ../rt5592/ap/apcli_assoc.o
rt5592_ap-objs += ../rt5592/ap/apcli_auth.o
rt5592_ap-objs += ../rt5592/ap/apcli_ctrl.o
rt5592_ap-objs += ../rt5592/ap/apcli_sync.o
rt5592_ap-objs += ../rt5592/common/cmm_mat.o
rt5592_ap-objs += ../rt5592/common/cmm_mat_iparp.o
rt5592_ap-objs += ../rt5592/common/cmm_mat_pppoe.o
rt5592_ap-objs += ../rt5592/common/cmm_mat_ipv6.o
endif

ifeq ($(CONFIG_RT5592_AP_IGMP_SNOOP),y)
rt5592_ap-objs += ../rt5592/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT5592_AP_NETIF_BLOCK),y)
rt5592_ap-objs += ../rt5592/common/netif_block.o
endif

ifeq ($(CONFIG_RT5592_AP_DLS),y)
rt5592_ap-objs += ../rt5592/ap/ap_dls.o
endif

ifeq ($(CONFIG_RT5592_AP_IDS),y)
rt5592_ap-objs += ../rt5592/ap/ap_ids.o
endif

ifeq ($(CONFIG_RT5592_AP_WAPI),y)
rt5592_ap-objs += ../rt5592/common/wapi.o
endif

ifeq ($(CONFIG_RT5592_AP_DFS),y)
rt5592_ap-objs += ../rt5592/common/cmm_dfs.o
rt5592_ap-objs += ../rt5592/common/cmm_radar.o
endif

ifeq ($(CONFIG_RT5592_AP_CARRIER),y)
rt5592_ap-objs += ../rt5592/common/cmm_cs.o
endif


###################
#  CFLAGS
###################
EXTRA_CFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DAP_SCAN_SUPPORT -DUAPSD_SUPPORT -DDOT11_N_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DRSSI_FEEDBACK
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DRT_SOC_SUPPORT

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

### RT5592
EXTRA_CFLAGS += -DRT30xx -DRT5592
EXTRA_CFLAGS += -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRX_DMA_SCATTER
EXTRA_CFLAGS += -DCONFIG_TSO_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
EXTRA_CFLAGS += -DIQ_CAL_SUPPORT
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_COMPENSATION
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
ifeq ($(CONFIG_RT5592_EP_SUPPORT),y)
EXTRA_CFLAGS += -DRT5592EP_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

ifeq ($(CONFIG_RT5592_AP_DBG),y)
EXTRA_CFLAGS += -DDBG
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DRALINK_QA -DCONFIG_RT2880_ATE_CMD_NEW
endif

ifeq ($(CONFIG_RT5592_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT -DEXPORT_SYMTAB
endif

ifeq ($(CONFIG_RT5592_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_AUTO_CH_SELECT_ENHANCE),y)
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_RT5592_AP_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_RT5592_AP_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_RT5592_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
ifeq ($(CONFIG_RT5592_AP_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
ifeq ($(CONFIG_RT5592_AP_LED),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_RT5592_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_MBSS_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
endif

ifeq ($(CONFIG_RT5592_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT
EXTRA_CFLAGS += -DMAT_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_RT5592_AP_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_RT5592_AP_MC_SUPPORT),y)
EXTRA_CFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

ifeq ($(CONFIG_RT5592_AP_BRIDGE_FASTPATH),y)
EXTRA_CFLAGS += -DBG_FT_SUPPORT
endif


clean:
	@rm -f `find ./ -name "*.o.cmd"`
	@rm -f `find ./ -name "*.ko"`
	@rm -f `find ./ -name "*.o"`
