RT_DRV_DIR  := rt3593
RT_DRV_PATH := ../$(RT_DRV_DIR)
RT_DRV_NAME := rt3593_ap

obj-$(CONFIG_RT3593_AP) += $(RT_DRV_NAME).o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_md5.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_sha2.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_hmac.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_aes.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_arc4.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/mlme.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_wep.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/action.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ba_action.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_data.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_init.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_init_inf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_tkip.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_aes.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_sync.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_sanity.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_info.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_cfg.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_wpa.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_radar.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/spectrum.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_timer.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rt_channel.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_profile.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_asic.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_cmd.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_mcu.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mac_pci.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_data_pci.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rt_os_util.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ps.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/uapsd.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ee_prom.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ee_efuse.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ee_flash.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/eeprom.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rt_rf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/rate_ctrl/ra_ctrl.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/rate_ctrl/alg_legacy.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_assoc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_auth.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_connect.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_mlme.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_sanity.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_sync.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_wpa.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_data.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_autoChSel.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_qload.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_cfg.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_proc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_main_dev.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_pci_rbus.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_profile.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_linux.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_rbus_pci_util.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_rbus_pci_drv.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/ap_ioctl.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/pci_main_dev.o

### RT3593
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rtmp_chip.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rt28xx.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rt30xx.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rt35xx.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rt3593.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/rate_ctrl/alg_grp.o
ifeq ($(CONFIG_RT3593_AP_TXBF_SUPPORT),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_txbf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_txbf_cal.o
endif
ifeq ($(CONFIG_RT_ATE),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/chips/rt30xx_ate.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/chips/rt35xx_ate.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/chips/rt3593_ate.o
endif

ifeq ($(CONFIG_RT_ATE),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/common/rt_ate.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/common/ate_pci.o
ifeq ($(CONFIG_RT_QA),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/common/rt_qa.o
endif
endif

ifeq ($(CONFIG_RT3593_AP_LED),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rt_led.o
endif

ifeq ($(CONFIG_RT3593_AP_WDS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_wds.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_wds_inf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/client_wds.o
endif

ifeq ($(CONFIG_RT3593_AP_MBSS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_mbss.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_RT3593_AP_APCLI),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_apcli.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_apcli_inf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_assoc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_auth.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_ctrl.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_sync.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat_iparp.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat_pppoe.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat_ipv6.o
endif

ifeq ($(CONFIG_RT3593_AP_DLS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_dls.o
endif

ifeq ($(CONFIG_RT3593_AP_IDS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_ids.o
endif

ifeq ($(CONFIG_RT3593_AP_DFS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_dfs.o
endif

ifeq ($(CONFIG_RT3593_AP_CARRIER),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_cs.o
endif

ifeq ($(CONFIG_RT_WSC),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc_tlv.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_dh.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc_ufd.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_biginteger.o
ifeq ($(CONFIG_RT_WSC_V2),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_RT_WAPI),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wapi.o
endif

ifeq ($(CONFIG_RT_IGMP_SNOOP),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT_NETIF_BLOCK),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/netif_block.o
endif

ifeq ($(CONFIG_RT_BAND_STEERING),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_band_steering.o
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_video.o
endif

###################
#  CFLAGS
###################
EXTRA_CFLAGS  = -Wall -Wstrict-prototypes -Wno-trigraphs
EXTRA_CFLAGS += -Idrivers/net/wireless/ralink/$(RT_DRV_DIR)/include
ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -Idrivers/net/wireless/ralink/$(RT_DRV_DIR)/ate/include
endif
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DAP_SCAN_SUPPORT -DUAPSD_SUPPORT -DDOT11_N_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DRSSI_FEEDBACK
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC

ifeq ($(CONFIG_RT_FIRST_CARD),2860)
EXTRA_CFLAGS += -DRT_SOC_SUPPORT
endif

ifeq ($(CONFIG_RT_FIRST_CARD),3593)
EXTRA_CFLAGS += -DRT_IFNAME_1ST
ifeq ($(CONFIG_RT_FIRST_CARD_EEPROM),"efuse")
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
else
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
endif
else
ifeq ($(CONFIG_RT_SECOND_CARD_EEPROM),"efuse")
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
else
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
endif
endif

ifeq ($(CONFIG_RT_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

# do not lock CountryRegion from EEPROM
EXTRA_CFLAGS += -DEEPROM_COUNTRY_UNLOCK

# provide information on the current STA population and traffic levels in the QBSS
#EXTRA_CFLAGS += -DAP_QLOAD_SUPPORT -DQLOAD_FUNC_BUSY_TIME_STATS

ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

### RT3593
EXTRA_CFLAGS += -DRT28xx -DRT30xx -DRT35xx -DRT3593
EXTRA_CFLAGS += -DA_BAND_SUPPORT -DDOT11N_SS3_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DSTREAM_MODE_SUPPORT
EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
EXTRA_CFLAGS += -DRANGE_EXTEND
#EXTRA_CFLAGS += -DPRE_ANT_SWITCH
#EXTRA_CFLAGS += -DCFO_TRACK
#EXTRA_CFLAGS += -DDBG_CTRL_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
ifeq ($(CONFIG_RT3593_AP_TXBF_SUPPORT),y)
EXTRA_CFLAGS += -DTXBF_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_HIGH_POWER),y)
EXTRA_CFLAGS += -DHIGH_POWER_SUPPORT
endif

#EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST

ifeq ($(CONFIG_RT_DEBUG),y)
EXTRA_CFLAGS += -DDBG
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DCONFIG_RT2880_ATE_CMD_NEW
ifeq ($(CONFIG_RT_QA),y)
EXTRA_CFLAGS += -DRALINK_QA
endif
endif

ifeq ($(CONFIG_RT3593_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_RT3593_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
ifeq ($(CONFIG_RT3593_AP_MBSS_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
endif
endif

ifeq ($(CONFIG_RT3593_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT
EXTRA_CFLAGS += -DAPCLI_AUTO_CONNECT_SUPPORT
endif

ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
ifeq ($(CONFIG_RT_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
endif

ifeq ($(CONFIG_RT_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_RT_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT -DEXPORT_SYMTAB
endif

ifeq ($(CONFIG_RT_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RT_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_RT_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RT_BAND_STEERING),y)
EXTRA_CFLAGS += -DBAND_STEERING
endif

ifeq ($(CONFIG_RT_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_RT_MC_SUPPORT),y)
EXTRA_CFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

