EXTRA_CFLAGS = -Idrivers/net/wireless/ralink/rt2860v2/include
ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -Idrivers/net/wireless/ralink/rt2860v2/ate/include
endif

obj-$(CONFIG_RT2860V2_AP) += rt2860v2_ap.o

rt2860v2_ap-objs += ../rt2860v2/common/crypt_md5.o
rt2860v2_ap-objs += ../rt2860v2/common/crypt_sha2.o
rt2860v2_ap-objs += ../rt2860v2/common/crypt_hmac.o
rt2860v2_ap-objs += ../rt2860v2/common/crypt_aes.o
rt2860v2_ap-objs += ../rt2860v2/common/crypt_arc4.o
rt2860v2_ap-objs += ../rt2860v2/common/mlme.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_wep.o
rt2860v2_ap-objs += ../rt2860v2/common/action.o
rt2860v2_ap-objs += ../rt2860v2/common/ba_action.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_data.o
rt2860v2_ap-objs += ../rt2860v2/common/rtmp_init.o
rt2860v2_ap-objs += ../rt2860v2/common/rtmp_init_inf.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_tkip.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_aes.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_sync.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_sanity.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_info.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_cfg.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_wpa.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_radar.o
rt2860v2_ap-objs += ../rt2860v2/common/spectrum.o
rt2860v2_ap-objs += ../rt2860v2/common/uapsd.o
rt2860v2_ap-objs += ../rt2860v2/common/ps.o
rt2860v2_ap-objs += ../rt2860v2/common/rtmp_timer.o
rt2860v2_ap-objs += ../rt2860v2/common/rt_channel.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_profile.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_asic.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_cmd.o
rt2860v2_ap-objs += ../rt2860v2/common/rtmp_swmcu.o
rt2860v2_ap-objs += ../rt2860v2/common/rt_os_util.o
rt2860v2_ap-objs += ../rt2860v2/common/eeprom.o
rt2860v2_ap-objs += ../rt2860v2/common/ee_flash.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_mac_pci.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_data_pci.o

rt2860v2_ap-objs += ../rt2860v2/rate_ctrl/ra_ctrl.o
rt2860v2_ap-objs += ../rt2860v2/rate_ctrl/alg_legacy.o

rt2860v2_ap-objs += ../rt2860v2/ap/ap.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_assoc.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_auth.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_connect.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_mlme.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_sanity.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_sync.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_wpa.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_data.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_autoChSel.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_cfg.o
ifeq ($(CONFIG_RT2860V2_AP_QLOAD),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_qload.o
endif

rt2860v2_ap-objs += ../rt2860v2/os/linux/rt_proc.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/rt_linux.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/rt_profile.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/rt_main_dev.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/rt_pci_rbus.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/rt_rbus_pci_drv.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/rt_rbus_pci_util.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/ap_ioctl.o
rt2860v2_ap-objs += ../rt2860v2/os/linux/rbus_main_dev.o

ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_ap-objs += ../rt2860v2/ate/common/rt_ate.o
rt2860v2_ap-objs += ../rt2860v2/ate/common/ate_pci.o
ifeq ($(CONFIG_RT_QA),y)
rt2860v2_ap-objs += ../rt2860v2/ate/common/rt_qa.o
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_LED),y)
rt2860v2_ap-objs += ../rt2860v2/common/rt_led.o
endif

ifeq ($(CONFIG_RT2860V2_AP_WDS),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_wds.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_wds_inf.o
rt2860v2_ap-objs += ../rt2860v2/common/client_wds.o
endif

ifeq ($(CONFIG_RT2860V2_AP_MBSS),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_mbss.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_RT2860V2_AP_APCLI),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_apcli.o
rt2860v2_ap-objs += ../rt2860v2/ap/ap_apcli_inf.o
rt2860v2_ap-objs += ../rt2860v2/ap/apcli_assoc.o
rt2860v2_ap-objs += ../rt2860v2/ap/apcli_auth.o
rt2860v2_ap-objs += ../rt2860v2/ap/apcli_ctrl.o
rt2860v2_ap-objs += ../rt2860v2/ap/apcli_sync.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_mat.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_mat_iparp.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_mat_pppoe.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_mat_ipv6.o
ifeq ($(CONFIG_RT2860V2_AP_MAC_REPEATER),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_repeater.o
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_DLS),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_dls.o
endif

ifeq ($(CONFIG_RT2860V2_AP_IDS),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_ids.o
endif

ifeq ($(CONFIG_RT2860V2_AP_DFS),y)
rt2860v2_ap-objs += ../rt2860v2/common/cmm_dfs.o
endif

ifeq ($(CONFIG_RT2860V2_AP_CARRIER),y)
rt2860v2_ap-objs += ../rt2860v2/common/cmm_cs.o
endif

ifeq ($(CONFIG_RT_WSC),y)
rt2860v2_ap-objs += ../rt2860v2/common/wsc.o
rt2860v2_ap-objs += ../rt2860v2/common/wsc_tlv.o
rt2860v2_ap-objs += ../rt2860v2/common/crypt_dh.o
rt2860v2_ap-objs += ../rt2860v2/common/crypt_biginteger.o
rt2860v2_ap-objs += ../rt2860v2/common/wsc_ufd.o
ifeq ($(CONFIG_RT_WSC_V2),y)
rt2860v2_ap-objs += ../rt2860v2/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_RT_WAPI),y)
rt2860v2_ap-objs += ../rt2860v2/common/wapi.o
rt2860v2_ap-objs += ../rt2860v2/common/wapi_crypt.obj
rt2860v2_ap-objs += ../rt2860v2/common/wapi_sms4.obj
endif

ifeq ($(CONFIG_RT_IGMP_SNOOP),y)
rt2860v2_ap-objs += ../rt2860v2/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT_NETIF_BLOCK),y)
rt2860v2_ap-objs += ../rt2860v2/common/netif_block.o
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
rt2860v2_ap-objs += ../rt2860v2/common/cmm_video.o
endif

ifeq ($(CONFIG_RT_BAND_STEERING),y)
rt2860v2_ap-objs += ../rt2860v2/ap/ap_band_steering.o
endif

rt2860v2_ap-objs += ../rt2860v2/chips/rtmp_chip.o
rt2860v2_ap-objs += ../rt2860v2/common/rt_rf.o

ifeq ($(CONFIG_RALINK_RT3883),y)
rt2860v2_ap-objs += ../rt2860v2/chips/rt3883.o
rt2860v2_ap-objs += ../rt2860v2/rate_ctrl/alg_grp.o
ifeq ($(CONFIG_RT2860V2_AP_TXBF),y)
rt2860v2_ap-objs += ../rt2860v2/common/cmm_txbf.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_txbf_cal.o
endif
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_ap-objs += ../rt2860v2/ate/chips/rt3883_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
rt2860v2_ap-objs += ../rt2860v2/chips/rt305x.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_ap-objs += ../rt2860v2/ate/chips/rt305x_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
rt2860v2_ap-objs += ../rt2860v2/chips/rt305x.o
rt2860v2_ap-objs += ../rt2860v2/chips/rt3352.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_ap-objs += ../rt2860v2/ate/chips/rt305x_ate.o
rt2860v2_ap-objs += ../rt2860v2/ate/chips/rt3352_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
rt2860v2_ap-objs += ../rt2860v2/chips/rt305x.o
rt2860v2_ap-objs += ../rt2860v2/chips/rt5350.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_ap-objs += ../rt2860v2/ate/chips/rt305x_ate.o
rt2860v2_ap-objs += ../rt2860v2/ate/chips/rt5350_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7620),y)
rt2860v2_ap-objs += ../rt2860v2/chips/rt6352.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_rf_cal.o
rt2860v2_ap-objs += ../rt2860v2/common/cmm_loft_cal.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_ap-objs += ../rt2860v2/ate/chips/rt6352_ate.o
endif
endif

###################
#  CFLAGS
##################
EXTRA_CFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DAP_SCAN_SUPPORT -DUAPSD_SUPPORT -DDOT11_N_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_RBUS_SUPPORT -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT -DCONFIG_SWMCU_SUPPORT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT -DENHANCED_STAT_DISPLAY -DRSSI_FEEDBACK
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC

# do not lock CountryRegion from EEPROM
EXTRA_CFLAGS += -DEEPROM_COUNTRY_UNLOCK

# provide roaming support
#EXTRA_CFLAGS += -DIAPP_SUPPORT

# provide information on the current STA population and traffic levels in the QBSS
ifeq ($(CONFIG_RT2860V2_AP_QLOAD),y)
EXTRA_CFLAGS += -DAP_QLOAD_SUPPORT -DQLOAD_FUNC_BUSY_TIME_STATS
endif

ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_RALINK_RT3883),y)
EXTRA_CFLAGS += -DRT3883 -DDOT11N_SS3_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DSTREAM_MODE_SUPPORT
EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
EXTRA_CFLAGS += -DRANGE_EXTEND
EXTRA_CFLAGS += -DDROP_MASK_SUPPORT -DMULTI_CLIENT_SUPPORT
#EXTRA_CFLAGS += -DPRE_ANT_SWITCH
#EXTRA_CFLAGS += -DCFO_TRACK
#EXTRA_CFLAGS += -DDBG_CTRL_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
#EXTRA_CFLAGS += -DBOARD_EXT_SWITCH_LNA_2DOT4_5
ifeq ($(CONFIG_RT2860V2_AP_TXBF),y)
EXTRA_CFLAGS += -DTXBF_SUPPORT
endif
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
ifeq ($(CONFIG_RALINK_RT3350),y)
EXTRA_CFLAGS += -DRT3350 -DRT305x
else
EXTRA_CFLAGS += -DRT3052 -DRT305x
endif
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
EXTRA_CFLAGS += -DRT3352 -DRT305x
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
EXTRA_CFLAGS += -DRT5350 -DRT305x
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
endif

ifeq ($(CONFIG_RALINK_MT7620),y)
EXTRA_CFLAGS += -DRT6352 -DRF_BANK
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DLOFT_IQ_CAL_SUPPORT
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_CALIBRATION
EXTRA_CFLAGS += -DDYNAMIC_VGA_SUPPORT
EXTRA_CFLAGS += -DTX_STA_FIFO_EXT_SUPPORT
#EXTRA_CFLAGS += -DMICROWAVE_OVEN_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT
EXTRA_CFLAGS += -DDROP_MASK_SUPPORT -DMULTI_CLIENT_SUPPORT
#EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
ifeq ($(CONFIG_INTERNAL_PA_EXTERNAL_LNA),y)
EXTRA_CFLAGS += -DRT6352_EL_SUPPORT
endif
ifeq ($(CONFIG_EXTERNAL_PA_EXTERNAL_LNA),y)
EXTRA_CFLAGS += -DRT6352_EP_SUPPORT -DRT6352_EL_SUPPORT
endif
endif

ifeq ($(CONFIG_RT_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

ifeq ($(CONFIG_RT_DEBUG),y)
EXTRA_CFLAGS += -DDBG -DRELEASE_EXCLUDE
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DCONFIG_RT2880_ATE_CMD_NEW
ifeq ($(CONFIG_RT_QA),y)
EXTRA_CFLAGS += -DRALINK_QA
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_DFS),y)
EXTRA_CFLAGS += -DDFS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_CARRIER),y)
EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_LED_SOFT),y)
EXTRA_CFLAGS += -DLED_SOFT_SUPPORT
EXTRA_CFLAGS += -DLED_SOFT_BLINK_GPIO=$(CONFIG_RT2860V2_AP_LED_SOFT_GPIO)
endif

ifeq ($(CONFIG_RT2860V2_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
ifeq ($(CONFIG_RT2860V2_AP_MBSS_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
ifeq ($(CONFIG_RALINK_MT7620),y)
EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
endif
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT
EXTRA_CFLAGS += -DAPCLI_AUTO_CONNECT_SUPPORT
ifeq ($(CONFIG_RT2860V2_AP_MAC_REPEATER),y)
ifeq ($(CONFIG_RALINK_MT7620),y)
EXTRA_CFLAGS += -DMAC_APCLI_SUPPORT
endif
EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT
endif
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

ifeq ($(CONFIG_RT_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RT_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_RT_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_RT2860V2_AP_ANTENNA_DIVERSITY),y)
EXTRA_CFLAGS += -DHW_ANTENNA_DIVERSITY_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_RT_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT
ifeq ($(CONFIG_RALINK_RT3052),y)
EXTRA_CFLAGS += -DSOFT_ENCRYPT
endif
endif

ifeq ($(CONFIG_RT_BAND_STEERING),y)
EXTRA_CFLAGS += -DBAND_STEERING
endif

ifeq ($(CONFIG_RT2860V2_AP_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif

ifeq ($(CONFIG_RT2860V2_AP_RTMP_TEMPERATURE_COMPENSATION),y)
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_COMPENSATION
endif

ifeq ($(CONFIG_RT2860V2_AP_ADJ_PWR_CONSUMPTION),y)
EXTRA_CFLAGS += -DADJUST_POWER_CONSUMPTION_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_32B_DESC),y)
EXTRA_CFLAGS += -DDESC_32B_SUPPORT
endif

ifeq ($(CONFIG_RT_SINGLE_SKU),y)
ifeq ($(CONFIG_RALINK_MT7620),y)
EXTRA_CFLAGS += -DSINGLE_SKU_V2
else
EXTRA_CFLAGS += -DSINGLE_SKU
endif
endif

ifeq ($(CONFIG_RT_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_RT_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

