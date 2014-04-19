EXTRA_CFLAGS = -Idrivers/net/wireless/rt3593/include
ifeq ($(CONFIG_RT3593_AP_ATE),y)
EXTRA_CFLAGS += -Idrivers/net/wireless/rt3593/ate/include
endif

obj-$(CONFIG_RT3593_AP) += rt3593_ap.o
rt3593_ap-objs += ../rt3593/common/crypt_md5.o
rt3593_ap-objs += ../rt3593/common/crypt_sha2.o
rt3593_ap-objs += ../rt3593/common/crypt_hmac.o
rt3593_ap-objs += ../rt3593/common/crypt_aes.o
rt3593_ap-objs += ../rt3593/common/crypt_arc4.o
rt3593_ap-objs += ../rt3593/common/mlme.o
rt3593_ap-objs += ../rt3593/common/cmm_wep.o
rt3593_ap-objs += ../rt3593/common/action.o
rt3593_ap-objs += ../rt3593/common/ba_action.o
rt3593_ap-objs += ../rt3593/common/cmm_data.o
rt3593_ap-objs += ../rt3593/common/rtmp_init.o
rt3593_ap-objs += ../rt3593/common/rtmp_init_inf.o
rt3593_ap-objs += ../rt3593/common/cmm_tkip.o
rt3593_ap-objs += ../rt3593/common/cmm_aes.o
rt3593_ap-objs += ../rt3593/common/cmm_sync.o
rt3593_ap-objs += ../rt3593/common/cmm_sanity.o
rt3593_ap-objs += ../rt3593/common/cmm_info.o
rt3593_ap-objs += ../rt3593/common/cmm_cfg.o
rt3593_ap-objs += ../rt3593/common/cmm_wpa.o
rt3593_ap-objs += ../rt3593/common/cmm_radar.o
rt3593_ap-objs += ../rt3593/common/spectrum.o
rt3593_ap-objs += ../rt3593/common/rtmp_timer.o
rt3593_ap-objs += ../rt3593/common/rt_channel.o
rt3593_ap-objs += ../rt3593/common/cmm_profile.o
rt3593_ap-objs += ../rt3593/common/cmm_asic.o
rt3593_ap-objs += ../rt3593/common/cmm_cmd.o
rt3593_ap-objs += ../rt3593/common/rtmp_mcu.o
rt3593_ap-objs += ../rt3593/common/cmm_mac_pci.o
rt3593_ap-objs += ../rt3593/common/cmm_data_pci.o
rt3593_ap-objs += ../rt3593/common/rt_os_util.o
rt3593_ap-objs += ../rt3593/common/ps.o
rt3593_ap-objs += ../rt3593/common/uapsd.o
rt3593_ap-objs += ../rt3593/common/ee_prom.o
rt3593_ap-objs += ../rt3593/common/ee_efuse.o
rt3593_ap-objs += ../rt3593/common/ee_flash.o
rt3593_ap-objs += ../rt3593/common/eeprom.o
rt3593_ap-objs += ../rt3593/common/rt_rf.o

rt3593_ap-objs += ../rt3593/rate_ctrl/ra_ctrl.o
rt3593_ap-objs += ../rt3593/rate_ctrl/alg_legacy.o

rt3593_ap-objs += ../rt3593/ap/ap.o
rt3593_ap-objs += ../rt3593/ap/ap_assoc.o
rt3593_ap-objs += ../rt3593/ap/ap_auth.o
rt3593_ap-objs += ../rt3593/ap/ap_connect.o
rt3593_ap-objs += ../rt3593/ap/ap_mlme.o
rt3593_ap-objs += ../rt3593/ap/ap_sanity.o
rt3593_ap-objs += ../rt3593/ap/ap_sync.o
rt3593_ap-objs += ../rt3593/ap/ap_wpa.o
rt3593_ap-objs += ../rt3593/ap/ap_data.o
rt3593_ap-objs += ../rt3593/ap/ap_autoChSel.o
rt3593_ap-objs += ../rt3593/ap/ap_qload.o
rt3593_ap-objs += ../rt3593/ap/ap_cfg.o

rt3593_ap-objs += ../rt3593/os/linux/rt_proc.o
rt3593_ap-objs += ../rt3593/os/linux/rt_main_dev.o
rt3593_ap-objs += ../rt3593/os/linux/rt_pci_rbus.o
rt3593_ap-objs += ../rt3593/os/linux/rt_profile.o
rt3593_ap-objs += ../rt3593/os/linux/rt_linux.o
rt3593_ap-objs += ../rt3593/os/linux/rt_rbus_pci_util.o
rt3593_ap-objs += ../rt3593/os/linux/rt_rbus_pci_drv.o
rt3593_ap-objs += ../rt3593/os/linux/ap_ioctl.o
rt3593_ap-objs += ../rt3593/os/linux/pci_main_dev.o

ifeq ($(CONFIG_RT3593_AP_ATE),y)
rt3593_ap-objs += ../rt3593/ate/common/rt_ate.o
rt3593_ap-objs += ../rt3593/ate/common/ate_pci.o
ifeq ($(CONFIG_RT3593_AP_QA),y)
rt3593_ap-objs += ../rt3593/ate/common/rt_qa.o
endif
endif

### RT3593
rt3593_ap-objs += ../rt3593/chips/rtmp_chip.o
rt3593_ap-objs += ../rt3593/chips/rt28xx.o
rt3593_ap-objs += ../rt3593/chips/rt30xx.o
rt3593_ap-objs += ../rt3593/chips/rt35xx.o
rt3593_ap-objs += ../rt3593/chips/rt3593.o
rt3593_ap-objs += ../rt3593/rate_ctrl/alg_grp.o
ifeq ($(CONFIG_RT3593_AP_TXBF_SUPPORT),y)
rt3593_ap-objs += ../rt3593/common/cmm_txbf.o
rt3593_ap-objs += ../rt3593/common/cmm_txbf_cal.o
endif
ifeq ($(CONFIG_RT3593_AP_ATE),y)
rt3593_ap-objs += ../rt3593/ate/chips/rt30xx_ate.o
rt3593_ap-objs += ../rt3593/ate/chips/rt35xx_ate.o
rt3593_ap-objs += ../rt3593/ate/chips/rt3593_ate.o
endif

ifeq ($(CONFIG_RT3593_AP_VIDEO_TURBINE),y)
rt3593_ap-objs += ../rt3593/common/cmm_video.o
endif

ifeq ($(CONFIG_RT3593_AP_BRIDGE_FASTPATH),y)
rt3593_ap-objs += ../rt3593/os/linux/br_ftph.o
endif

ifeq ($(CONFIG_RT3593_AP_LED),y)
rt3593_ap-objs += ../rt3593/common/rt_led.o
endif

ifeq ($(CONFIG_RT3593_AP_WSC),y)
rt3593_ap-objs += ../rt3593/common/wsc.o
rt3593_ap-objs += ../rt3593/common/wsc_tlv.o
rt3593_ap-objs += ../rt3593/common/crypt_dh.o
rt3593_ap-objs += ../rt3593/common/wsc_ufd.o
rt3593_ap-objs += ../rt3593/common/crypt_biginteger.o
ifeq ($(CONFIG_RT3593_AP_WSC_V2),y)
rt3593_ap-objs += ../rt3593/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_RT3593_AP_WDS),y)
rt3593_ap-objs += ../rt3593/ap/ap_wds.o
rt3593_ap-objs += ../rt3593/ap/ap_wds_inf.o
rt3593_ap-objs += ../rt3593/common/client_wds.o
endif

ifeq ($(CONFIG_RT3593_AP_MBSS),y)
rt3593_ap-objs += ../rt3593/ap/ap_mbss.o
rt3593_ap-objs += ../rt3593/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_RT3593_AP_APCLI),y)
rt3593_ap-objs += ../rt3593/ap/ap_apcli.o
rt3593_ap-objs += ../rt3593/ap/ap_apcli_inf.o
rt3593_ap-objs += ../rt3593/ap/apcli_assoc.o
rt3593_ap-objs += ../rt3593/ap/apcli_auth.o
rt3593_ap-objs += ../rt3593/ap/apcli_ctrl.o
rt3593_ap-objs += ../rt3593/ap/apcli_sync.o
rt3593_ap-objs += ../rt3593/common/cmm_mat.o
rt3593_ap-objs += ../rt3593/common/cmm_mat_iparp.o
rt3593_ap-objs += ../rt3593/common/cmm_mat_pppoe.o
rt3593_ap-objs += ../rt3593/common/cmm_mat_ipv6.o
endif

ifeq ($(CONFIG_RT3593_AP_IGMP_SNOOP),y)
rt3593_ap-objs += ../rt3593/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT3593_AP_NETIF_BLOCK),y)
rt3593_ap-objs += ../rt3593/common/netif_block.o
endif

ifeq ($(CONFIG_RT3593_AP_DLS),y)
rt3593_ap-objs += ../rt3593/ap/ap_dls.o
endif

ifeq ($(CONFIG_RT3593_AP_IDS),y)
rt3593_ap-objs += ../rt3593/ap/ap_ids.o
endif

ifeq ($(CONFIG_RT3593_AP_WAPI),y)
rt3593_ap-objs += ../rt3593/common/wapi.o
endif

ifeq ($(CONFIG_RT3593_AP_DFS),y)
rt3593_ap-objs += ../rt3593/common/cmm_dfs.o
endif

ifeq ($(CONFIG_RT3593_AP_CARRIER),y)
rt3593_ap-objs += ../rt3593/common/cmm_cs.o
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

ifeq ($(CONFIG_RT3593_AP_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

ifeq ($(CONFIG_RT3593_AP_DBG),y)
EXTRA_CFLAGS += -DDBG
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DCONFIG_RT2880_ATE_CMD_NEW
ifeq ($(CONFIG_RT3593_AP_QA),y)
EXTRA_CFLAGS += -DRALINK_QA
endif
endif

ifeq ($(CONFIG_RT3593_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT -DEXPORT_SYMTAB
endif

ifeq ($(CONFIG_RT3593_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_AUTO_CH_SELECT_ENHANCE),y)
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_RT3593_AP_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_RT3593_AP_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_RT3593_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
ifeq ($(CONFIG_RT3593_AP_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
ifeq ($(CONFIG_RT3593_AP_LED),y)
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
EXTRA_CFLAGS += -DAPCLI_SUPPORT
EXTRA_CFLAGS += -DMAT_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_RT3593_AP_HIGH_POWER),y)
EXTRA_CFLAGS += -DHIGH_POWER_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_RT3593_AP_MC_SUPPORT),y)
EXTRA_CFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

ifeq ($(CONFIG_RT3593_AP_BRIDGE_FASTPATH),y)
EXTRA_CFLAGS += -DBG_FT_SUPPORT
endif


clean:
	@rm -f `find ./ -name "*.o.cmd"`
	@rm -f `find ./ -name "*.ko"`
	@rm -f `find ./ -name "*.o"`
