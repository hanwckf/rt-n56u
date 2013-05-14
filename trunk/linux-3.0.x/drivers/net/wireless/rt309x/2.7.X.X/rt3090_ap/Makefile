EXTRA_CFLAGS = -Idrivers/net/wireless/rt3090/include
ifeq ($(CONFIG_RT3090_AP_ATE),y)
EXTRA_CFLAGS += -Idrivers/net/wireless/rt3090/ate/include
endif

obj-$(CONFIG_RT3090_AP) += rt3090_ap.o
rt3090_ap-objs += ../rt3090/common/crypt_md5.o
rt3090_ap-objs += ../rt3090/common/crypt_sha2.o
rt3090_ap-objs += ../rt3090/common/crypt_hmac.o
rt3090_ap-objs += ../rt3090/common/crypt_aes.o
rt3090_ap-objs += ../rt3090/common/crypt_arc4.o
rt3090_ap-objs += ../rt3090/common/mlme.o
rt3090_ap-objs += ../rt3090/common/cmm_wep.o
rt3090_ap-objs += ../rt3090/common/action.o
rt3090_ap-objs += ../rt3090/common/ba_action.o
rt3090_ap-objs += ../rt3090/common/cmm_data.o
rt3090_ap-objs += ../rt3090/common/rtmp_init.o
rt3090_ap-objs += ../rt3090/common/rtmp_init_inf.o
rt3090_ap-objs += ../rt3090/common/cmm_tkip.o
rt3090_ap-objs += ../rt3090/common/cmm_aes.o
rt3090_ap-objs += ../rt3090/common/cmm_sync.o
rt3090_ap-objs += ../rt3090/common/cmm_sanity.o
rt3090_ap-objs += ../rt3090/common/cmm_info.o
rt3090_ap-objs += ../rt3090/common/cmm_cfg.o
rt3090_ap-objs += ../rt3090/common/cmm_wpa.o
rt3090_ap-objs += ../rt3090/common/spectrum.o
rt3090_ap-objs += ../rt3090/common/rtmp_timer.o
rt3090_ap-objs += ../rt3090/common/rt_channel.o
rt3090_ap-objs += ../rt3090/common/cmm_profile.o
rt3090_ap-objs += ../rt3090/common/cmm_asic.o
rt3090_ap-objs += ../rt3090/common/cmm_cmd.o
rt3090_ap-objs += ../rt3090/common/rtmp_mcu.o
rt3090_ap-objs += ../rt3090/common/cmm_mac_pci.o
rt3090_ap-objs += ../rt3090/common/cmm_data_pci.o
rt3090_ap-objs += ../rt3090/common/rt_os_util.o
rt3090_ap-objs += ../rt3090/common/ps.o
rt3090_ap-objs += ../rt3090/common/uapsd.o
rt3090_ap-objs += ../rt3090/common/ee_prom.o
rt3090_ap-objs += ../rt3090/common/ee_efuse.o
rt3090_ap-objs += ../rt3090/common/ee_flash.o
rt3090_ap-objs += ../rt3090/common/eeprom.o
rt3090_ap-objs += ../rt3090/common/rt_rf.o

rt3090_ap-objs += ../rt3090/rate_ctrl/ra_ctrl.o
rt3090_ap-objs += ../rt3090/rate_ctrl/alg_legacy.o

rt3090_ap-objs += ../rt3090/ap/ap.o
rt3090_ap-objs += ../rt3090/ap/ap_assoc.o
rt3090_ap-objs += ../rt3090/ap/ap_auth.o
rt3090_ap-objs += ../rt3090/ap/ap_connect.o
rt3090_ap-objs += ../rt3090/ap/ap_mlme.o
rt3090_ap-objs += ../rt3090/ap/ap_sanity.o
rt3090_ap-objs += ../rt3090/ap/ap_sync.o
rt3090_ap-objs += ../rt3090/ap/ap_wpa.o
rt3090_ap-objs += ../rt3090/ap/ap_data.o
rt3090_ap-objs += ../rt3090/ap/ap_autoChSel.o
rt3090_ap-objs += ../rt3090/ap/ap_qload.o
rt3090_ap-objs += ../rt3090/ap/ap_cfg.o

ifeq ($(CONFIG_RT3090_AP_ATE),y)
rt3090_ap-objs += ../rt3090/ate/common/rt_ate.o
rt3090_ap-objs += ../rt3090/ate/common/ate_pci.o
ifeq ($(CONFIG_RT3090_AP_QA),y)
rt3090_ap-objs += ../rt3090/ate/common/rt_qa.o
endif
endif

rt3090_ap-objs += ../rt3090/chips/rtmp_chip.o

### RT3090 (default)
rt3090_ap-objs += ../rt3090/chips/rt3090.o
rt3090_ap-objs += ../rt3090/chips/rt30xx.o
ifeq ($(CONFIG_RT3090_AP_INCLUDE_RT3390),y)
rt3390_ap-objs += ../rt3090/chips/rt3390.o
rt3390_ap-objs += ../rt3090/chips/rt33xx.o
endif
ifeq ($(CONFIG_RT3090_AP_ATE),y)
rt3090_ap-objs += ../rt3090/ate/chips/rt30xx_ate.o
ifeq ($(CONFIG_RT3090_AP_INCLUDE_RT3390),y)
rt3390_ap-objs += ../rt3090/ate/chips/rt33xx_ate.o
endif
endif

### RT5392
ifeq ($(CONFIG_RT5392_AP),m)
rt5392_ap-objs += ../rt5392/chips/rt3090.o
rt5392_ap-objs += ../rt5392/chips/rt30xx.o
rt5392_ap-objs += ../rt5392/chips/rt3390.o
rt5392_ap-objs += ../rt5392/chips/rt33xx.o
rt5392_ap-objs += ../rt5392/chips/rt5390.o
ifeq ($(CONFIG_RT5392_AP_ATE),y)
rt5392_ap-objs += ../rt5392/ate/chips/rt30xx_ate.o
rt5392_ap-objs += ../rt5392/ate/chips/rt33xx_ate.o
rt5392_ap-objs += ../rt5392/ate/chips/rt5390_ate.o
endif
endif

### RT5592
ifeq ($(CONFIG_RT5592_AP),m)
rt5592_ap-objs += ../rt5592/chips/rt30xx.o
rt5592_ap-objs += ../rt5592/chips/rt5592.o
ifeq ($(CONFIG_RT5592_AP_ATE),y)
rt5592_ap-objs += ../rt5592/ate/chips/rt30xx_ate.o
rt5592_ap-objs += ../rt5592/ate/chips/rt5592_ate.o
endif
endif

### RT3593
ifeq ($(CONFIG_RT3593_AP),m)
rt3593_ap-objs += ../rt3593/chips/rt28xx.o
rt3593_ap-objs += ../rt3593/chips/rt30xx.o
rt3593_ap-objs += ../rt3593/chips/rt35xx.o
rt3593_ap-objs += ../rt3593/chips/rt3593.o
rt3593_ap-objs += ../rt3593/rate_ctrl/alg_grp.o
ifeq ($(CONFIG_RT3593_AP_TXBF_SUPPORT),y)
rt3090_ap-objs += ../rt3593/common/cmm_txbf.o
rt3090_ap-objs += ../rt3593/common/cmm_txbf_cal.o
endif
ifeq ($(CONFIG_RT3593_AP_ATE),y)
rt3593_ap-objs += ../rt3593/ate/chips/rt30xx_ate.o
rt3593_ap-objs += ../rt3593/ate/chips/rt35xx_ate.o
rt3593_ap-objs += ../rt3593/ate/chips/rt3593_ate.o
endif
endif

ifeq ($(CONFIG_RT3090_AP_VIDEO_TURBINE),y)
rt3090_ap-objs += ../rt3090/common/cmm_video.o
endif

rt3090_ap-objs += ../rt3090/os/linux/rt_proc.o
rt3090_ap-objs += ../rt3090/os/linux/ap_ioctl.o
rt3090_ap-objs += ../rt3090/os/linux/rt_main_dev.o
rt3090_ap-objs += ../rt3090/os/linux/pci_main_dev.o
rt3090_ap-objs += ../rt3090/os/linux/rt_pci_rbus.o
rt3090_ap-objs += ../rt3090/os/linux/rt_profile.o
rt3090_ap-objs += ../rt3090/os/linux/rt_linux.o
rt3090_ap-objs += ../rt3090/os/linux/rt_rbus_pci_util.o
rt3090_ap-objs += ../rt3090/os/linux/rt_rbus_pci_drv.o

ifeq ($(CONFIG_RT3090_AP_BRIDGE_FASTPATH),y)
rt3090_ap-objs += ../rt3090/os/linux/br_ftph.o
endif

ifeq ($(CONFIG_RT3090_AP_LED),y)
rt3090_ap-objs += ../rt3090/common/rt_led.o
endif

ifeq ($(CONFIG_RT3090_AP_WSC),y)
rt3090_ap-objs += ../rt3090/common/wsc.o
rt3090_ap-objs += ../rt3090/common/wsc_tlv.o
rt3090_ap-objs += ../rt3090/common/crypt_dh.o
rt3090_ap-objs += ../rt3090/common/wsc_ufd.o
rt3090_ap-objs += ../rt3090/common/crypt_biginteger.o
ifeq ($(CONFIG_RT3090_AP_WSC_V2),y)
rt3090_ap-objs += ../rt3090/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_RT3090_AP_WDS),y)
rt3090_ap-objs += ../rt3090/ap/ap_wds.o
rt3090_ap-objs += ../rt3090/ap/ap_wds_inf.o
endif

ifeq ($(CONFIG_RT3090_AP_MBSS),y)
rt3090_ap-objs += ../rt3090/ap/ap_mbss.o
rt3090_ap-objs += ../rt3090/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_RT3090_AP_APCLI),y)
rt3090_ap-objs += ../rt3090/ap/ap_apcli.o
rt3090_ap-objs += ../rt3090/ap/ap_apcli_inf.o
rt3090_ap-objs += ../rt3090/ap/apcli_assoc.o
rt3090_ap-objs += ../rt3090/ap/apcli_auth.o
rt3090_ap-objs += ../rt3090/ap/apcli_ctrl.o
rt3090_ap-objs += ../rt3090/ap/apcli_sync.o
rt3090_ap-objs += ../rt3090/common/cmm_mat.o
rt3090_ap-objs += ../rt3090/common/cmm_mat_iparp.o
rt3090_ap-objs += ../rt3090/common/cmm_mat_pppoe.o
rt3090_ap-objs += ../rt3090/common/cmm_mat_ipv6.o
endif

ifeq ($(CONFIG_RT3090_AP_IGMP_SNOOP),y)
rt3090_ap-objs += ../rt3090/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT3090_AP_NETIF_BLOCK),y)
rt3090_ap-objs += ../rt3090/common/netif_block.o
endif

ifeq ($(CONFIG_RT3090_AP_DLS),y)
rt3090_ap-objs += ../rt3090/ap/ap_dls.o
endif

ifeq ($(CONFIG_RT3090_AP_IDS),y)
rt3090_ap-objs += ../rt3090/ap/ap_ids.o
endif

ifeq ($(CONFIG_RT3090_AP_WAPI),y)
rt3090_ap-objs += ../rt3090/common/wapi.o
endif

ifeq ($(CONFIG_RT3090_AP_DFS),y)
rt3090_ap-objs += ../rt3090/common/cmm_dfs.o
rt3090_ap-objs += ../rt3090/common/cmm_radar.o
endif

ifeq ($(CONFIG_RT3090_AP_CARRIER),y)
rt3090_ap-objs += ../rt3090/common/cmm_cs.o
endif


###################
#  CFLAGS
###################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DAP_SCAN_SUPPORT -DUAPSD_SUPPORT -DDOT11_N_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DRT_SOC_SUPPORT
EXTRA_CFLAGS += -DRSSI_FEEDBACK

# do not lock CountryRegion from EEPROM
EXTRA_CFLAGS += -DEEPROM_COUNTRY_UNLOCK

# provide external Flash memory support
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT

# provide internal eFuse memory support
#EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT

# provide roaming support
#EXTRA_CFLAGS += -DIAPP_SUPPORT

### RT3090 (default)
EXTRA_CFLAGS += -DRT30xx -DRT3090
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
ifeq ($(CONFIG_RT3090_AP_INCLUDE_RT3390),y)
EXTRA_CFLAGS += -DRT33xx -DRT3390
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif

### RT5392
ifeq ($(CONFIG_RT5392_AP),m)
EXTRA_CFLAGS += -DRT30xx -DRT3090
EXTRA_CFLAGS += -DRT33xx -DRT3390
EXTRA_CFLAGS += -DRT53xx -DRT5390
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
EXTRA_CFLAGS += -DRTMP_FREQ_CALIBRATION_SUPPORT
EXTRA_CFLAGS += -DIQ_CAL_SUPPORT
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_COMPENSATION
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
endif

### RT5592
ifeq ($(CONFIG_RT5592_AP),m)
EXTRA_CFLAGS += -DRT30xx
EXTRA_CFLAGS += -DRT5592
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
endif

### RT3593
ifeq ($(CONFIG_RT3593_AP),m)
EXTRA_CFLAGS += -DRT28xx
EXTRA_CFLAGS += -DRT30xx
EXTRA_CFLAGS += -DRT35xx
EXTRA_CFLAGS += -DRT3593
EXTRA_CFLAGS += -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DDOT11N_SS3_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DSTREAM_MODE_SUPPORT
EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
EXTRA_CFLAGS += -DRANGE_EXTEND
EXTRA_CFLAGS += -DDBG_CTRL_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
ifeq ($(CONFIG_RT3090_AP_TXBF_SUPPORT),y)
EXTRA_CFLAGS += -DTXBF_SUPPORT
EXTRA_CFLAGS += -DPRE_ANT_SWITCH
EXTRA_CFLAGS += -DCFO_TRACK
endif
endif

ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

ifeq ($(CONFIG_RT3090_AP_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

ifeq ($(CONFIG_RT3090_AP_DBG),y)
EXTRA_CFLAGS += -DDBG
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DCONFIG_RT2880_ATE_CMD_NEW
ifeq ($(CONFIG_RT3090_AP_QA),y)
EXTRA_CFLAGS += -DRALINK_QA
endif
endif

ifeq ($(CONFIG_RT3090_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT -DEXPORT_SYMTAB
endif

ifeq ($(CONFIG_RT3090_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_AUTO_CH_SELECT_ENHANCE),y)
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_RT3090_AP_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_RT3090_AP_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_RT3090_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
ifeq ($(CONFIG_RT3090_AP_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
ifeq ($(CONFIG_RT3090_AP_LED),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_RT3090_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_MBSS_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
endif

ifeq ($(CONFIG_RT3090_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT
EXTRA_CFLAGS += -DMAT_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_RT3090_AP_HIGH_POWER),y)
EXTRA_CFLAGS += -DHIGH_POWER_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_RT3090_AP_MC_SUPPORT),y)
EXTRA_CFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_RT3090_AP_BRIDGE_FASTPATH),y)
EXTRA_CFLAGS += -DBG_FT_SUPPORT
endif


clean:
	@rm -f `find ./ -name "*.o.cmd"`
	@rm -f `find ./ -name "*.ko"`
	@rm -f `find ./ -name "*.o"`
