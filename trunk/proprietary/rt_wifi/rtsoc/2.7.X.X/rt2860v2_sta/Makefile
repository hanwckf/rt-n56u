EXTRA_CFLAGS = -Idrivers/net/wireless/rt2860v2/include
ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -Idrivers/net/wireless/rt2860v2/ate/include
endif

obj-$(CONFIG_RT2860V2_STA) += rt2860v2_sta.o

rt2860v2_sta-objs += ../rt2860v2/common/crypt_md5.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_sha2.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_hmac.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_aes.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_arc4.o
rt2860v2_sta-objs += ../rt2860v2/common/mlme.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_wep.o
rt2860v2_sta-objs += ../rt2860v2/common/action.o
rt2860v2_sta-objs += ../rt2860v2/common/ba_action.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_data.o
rt2860v2_sta-objs += ../rt2860v2/common/rtmp_init.o
rt2860v2_sta-objs += ../rt2860v2/common/rtmp_init_inf.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_tkip.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_aes.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_sync.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_sanity.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_info.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_wpa.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_radar.o
rt2860v2_sta-objs += ../rt2860v2/common/spectrum.o
rt2860v2_sta-objs += ../rt2860v2/common/rt_os_util.o
rt2860v2_sta-objs += ../rt2860v2/common/rtmp_timer.o
rt2860v2_sta-objs += ../rt2860v2/common/rt_channel.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_profile.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_asic.o
rt2860v2_sta-objs += ../rt2860v2/common/rtmp_swmcu.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_cfg.o
rt2860v2_sta-objs += ../rt2860v2/common/eeprom.o
rt2860v2_sta-objs += ../rt2860v2/common/ee_flash.o
rt2860v2_sta-objs += ../rt2860v2/common/ps.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_mac_pci.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_data_pci.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_cmd.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_wpa_adhoc.o

rt2860v2_sta-objs += ../rt2860v2/rate_ctrl/ra_ctrl.o
rt2860v2_sta-objs += ../rt2860v2/rate_ctrl/alg_legacy.o

rt2860v2_sta-objs += ../rt2860v2/sta/assoc.o
rt2860v2_sta-objs += ../rt2860v2/sta/auth.o
rt2860v2_sta-objs += ../rt2860v2/sta/auth_rsp.o
rt2860v2_sta-objs += ../rt2860v2/sta/sync.o
rt2860v2_sta-objs += ../rt2860v2/sta/sanity.o
rt2860v2_sta-objs += ../rt2860v2/sta/rtmp_data.o
rt2860v2_sta-objs += ../rt2860v2/sta/connect.o
rt2860v2_sta-objs += ../rt2860v2/sta/wpa.o
rt2860v2_sta-objs += ../rt2860v2/sta/sta_cfg.o

rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_proc.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_linux.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_profile.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_main_dev.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/sta_ioctl.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rbus_main_dev.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_pci_rbus.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_rbus_pci_util.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_rbus_pci_drv.o

ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_sta-objs += ../rt2860v2/ate/common/rt_ate.o
rt2860v2_sta-objs += ../rt2860v2/ate/common/rt_qa.o
rt2860v2_sta-objs += ../rt2860v2/ate/common/ate_pci.o
endif

rt2860v2_sta-objs += ../rt2860v2/chips/rtmp_chip.o
rt2860v2_sta-objs += ../rt2860v2/common/rt_rf.o

ifeq ($(CONFIG_RALINK_RT3883),y)
rt2860v2_sta-objs += ../rt2860v2/chips/rt3883.o
rt2860v2_sta-objs += ../rt2860v2/rate_ctrl/alg_grp.o
ifeq ($(CONFIG_RT2860V2_STA_TXBF),y)
rt2860v2_sta-objs += ../rt2860v2/common/cmm_txbf.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_txbf_cal.o
endif
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_sta-objs += ../rt2860v2/ate/chips/rt3883_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_RT3052),y)
rt2860v2_sta-objs += ../rt2860v2/chips/rt305x.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_sta-objs += ../rt2860v2/ate/chips/rt305x_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_RT3352),y)
rt2860v2_sta-objs += ../rt2860v2/chips/rt305x.o
rt2860v2_sta-objs += ../rt2860v2/chips/rt3352.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_sta-objs += ../rt2860v2/ate/chips/rt305x_ate.o
rt2860v2_sta-objs += ../rt2860v2/ate/chips/rt3352_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_RT5350),y)
rt2860v2_sta-objs += ../rt2860v2/chips/rt305x.o
rt2860v2_sta-objs += ../rt2860v2/chips/rt5350.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_sta-objs += ../rt2860v2/ate/chips/rt305x_ate.o
rt2860v2_sta-objs += ../rt2860v2/ate/chips/rt5350_ate.o
endif
endif

ifeq ($(CONFIG_RALINK_MT7620),y)
rt2860v2_sta-objs += ../rt2860v2/chips/rt6352.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_rf_cal.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_loft_cal.o
ifeq ($(CONFIG_RT_ATE),y)
rt2860v2_sta-objs += ../rt2860v2/ate/chips/rt6352_ate.o
endif
endif

ifeq ($(CONFIG_RT2860V2_STA_LED),y)
rt2860v2_sta-objs += ../rt2860v2/common/rt_led.o
endif

ifeq ($(CONFIG_RT_WAPI),y)
rt2860v2_sta-objs += ../rt2860v2/common/wapi.o
rt2860v2_sta-objs += ../rt2860v2/common/wapi_sms4.obj
rt2860v2_sta-objs += ../rt2860v2/common/wapi_crypt.obj
endif

ifeq ($(CONFIG_RT2860V2_STA_DLS),y)
rt2860v2_sta-objs += ../rt2860v2/sta/dls.o
endif

ifeq ($(CONFIG_RT_WSC),y)
rt2860v2_sta-objs += ../rt2860v2/common/wsc.o
rt2860v2_sta-objs += ../rt2860v2/common/wsc_tlv.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_biginteger.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_dh.o
ifeq ($(CONFIG_RT_WSC_V2),y)
rt2860v2_sta-objs += ../rt2860v2/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
rt2860v2_sta-objs += ../rt2860v2/common/cmm_video.o
endif

ifeq ($(CONFIG_RT2860V2_STA_P2P),y)
rt2860v2_sta-objs += ../rt2860v2/common/p2p_inf.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_mbss.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_assoc.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_auth.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_connect.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_mlme.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_sanity.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_sync.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_wpa.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_data.o
rt2860v2_sta-objs += ../rt2860v2/common/uapsd.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_autoChSel.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_qload.o
rt2860v2_sta-objs += ../rt2860v2/sta/p2pcli.o
rt2860v2_sta-objs += ../rt2860v2/sta/p2pcli_ctrl.o
rt2860v2_sta-objs += ../rt2860v2/sta/p2pcli_sync.o
rt2860v2_sta-objs += ../rt2860v2/sta/p2pcli_auth.o
rt2860v2_sta-objs += ../rt2860v2/sta/p2pcli_assoc.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_mat.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_mat_iparp.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_mat_pppoe.o
rt2860v2_sta-objs += ../rt2860v2/common/cmm_mat_ipv6.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_dbg.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_cfg.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_packet.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_action.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_table.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_ctrl.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_disc_mng.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_nego_mng.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_dbg.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_packet.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_action.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_table.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_ctrl.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_disc_mng.o
rt2860v2_sta-objs += ../rt2860v2/common/p2p_nego_mng.o
rt2860v2_sta-objs += ../rt2860v2/common/wsc.o
rt2860v2_sta-objs += ../rt2860v2/common/wsc_tlv.o
rt2860v2_sta-objs += ../rt2860v2/common/wsc_ufd.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_biginteger.o
rt2860v2_sta-objs += ../rt2860v2/common/crypt_dh.o
rt2860v2_sta-objs += ../rt2860v2/ap/ap_mbss_inf.o
rt2860v2_sta-objs += ../rt2860v2/common/rt_os_util.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/ap_ioctl.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/sta_ioctl.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_linux.o
rt2860v2_sta-objs += ../rt2860v2/os/linux/rt_main_dev.o
endif

###################
#  CFLAGS
##################
EXTRA_CFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_STA_SUPPORT -DRTMP_RBUS_SUPPORT -DRTMP_MAC_PCI -DDOT11_N_SUPPORT
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT -DCONFIG_SWMCU_SUPPORT -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC

# do not lock CountryRegion from EEPROM
EXTRA_CFLAGS += -DEEPROM_COUNTRY_UNLOCK

ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_RALINK_RT3883),y)
EXTRA_CFLAGS += -DRT3883 -DDOT11N_SS3_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DSTREAM_MODE_SUPPORT
EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
EXTRA_CFLAGS += -DRANGE_EXTEND
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
ifeq ($(CONFIG_RT2860V2_STA_TXBF),y)
EXTRA_CFLAGS += -DTXBF_SUPPORT -DSTA_ITXBF_SUPPORT
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
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_CALIBRATION
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
ifeq ($(CONFIG_INTERNAL_PA_EXTERNAL_LNA),y)
EXTRA_CFLAGS += -DRT6352_EL_SUPPORT
endif
ifeq ($(CONFIG_EXTERNAL_PA_EXTERNAL_LNA),y)
EXTRA_CFLAGS += -DRT6352_EP_SUPPORT -DRT6352_EL_SUPPORT
endif
endif

ifeq ($(CONFIG_RT_DEBUG),y)
EXTRA_CFLAGS += -DDBG -DRELEASE_EXCLUDE
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DRALINK_QA -DCONFIG_RT2880_ATE_CMD_NEW
endif

ifeq ($(CONFIG_RT_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

ifeq ($(CONFIG_RT2860V2_STA_WPA_SUPPLICANT),y)
EXTRA_CFLAGS += -DWPA_SUPPLICANT_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_RT2860V2_STA_CARRIER),y)
EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_RT2860V2_STA_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT
ifeq ($(CONFIG_RALINK_RT3052),y)
EXTRA_CFLAGS += -DSOFT_ENCRYPT
endif
endif

ifeq ($(CONFIG_RT2860V2_STA_ANTENNA_DIVERSITY),y)
EXTRA_CFLAGS += -DHW_ANTENNA_DIVERSITY_SUPPORT
endif

ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_STA_SUPPORT
ifeq ($(CONFIG_RT_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif

ifeq ($(CONFIG_RT_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_RT_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_STA_P2P),y)
EXTRA_CFLAGS += -DP2P_SUPPORT -DAPCLI_SUPPORT -DMAT_SUPPORT -DAP_SCAN_SUPPORT
EXTRA_CFLAGS += -DP2P_APCLI_SUPPORT -DCONFIG_AP_SUPPORT -DCONFIG_APSTA_MIXED_SUPPORT
EXTRA_CFLAGS += -DUAPSD_SUPPORT -DMBSS_SUPPORT -DIAPP_SUPPORT -DDOT1X_SUPPORT -DAP_SCAN_SUPPORT
EXTRA_CFLAGS += -DWSC_AP_SUPPORT -DWSC_STA_SUPPORT
endif
