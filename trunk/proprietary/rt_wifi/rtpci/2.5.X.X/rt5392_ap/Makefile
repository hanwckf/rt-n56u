EXTRA_CFLAGS = -Idrivers/net/wireless/ralink/rt5392/include

obj-$(CONFIG_RT5392_AP) += rt5392_ap.o
rt5392_ap-objs += ../rt5392/common/crypt_md5.o
rt5392_ap-objs += ../rt5392/common/crypt_sha2.o
rt5392_ap-objs += ../rt5392/common/crypt_hmac.o
rt5392_ap-objs += ../rt5392/common/crypt_aes.o
rt5392_ap-objs += ../rt5392/common/crypt_arc4.o
rt5392_ap-objs += ../rt5392/common/mlme.o
rt5392_ap-objs += ../rt5392/common/cmm_wep.o
rt5392_ap-objs += ../rt5392/common/action.o
rt5392_ap-objs += ../rt5392/common/ba_action.o
rt5392_ap-objs += ../rt5392/common/cmm_data.o
rt5392_ap-objs += ../rt5392/common/rtmp_init.o
rt5392_ap-objs += ../rt5392/common/rtmp_init_inf.o
rt5392_ap-objs += ../rt5392/common/cmm_tkip.o
rt5392_ap-objs += ../rt5392/common/cmm_aes.o
rt5392_ap-objs += ../rt5392/common/cmm_sync.o
rt5392_ap-objs += ../rt5392/common/cmm_sanity.o
rt5392_ap-objs += ../rt5392/common/cmm_info.o
rt5392_ap-objs += ../rt5392/common/cmm_cfg.o
rt5392_ap-objs += ../rt5392/common/cmm_wpa.o
rt5392_ap-objs += ../rt5392/common/spectrum.o
rt5392_ap-objs += ../rt5392/common/rtmp_timer.o
rt5392_ap-objs += ../rt5392/common/rt_channel.o
rt5392_ap-objs += ../rt5392/common/cmm_profile.o
rt5392_ap-objs += ../rt5392/common/cmm_asic.o
rt5392_ap-objs += ../rt5392/common/cmm_cmd.o
rt5392_ap-objs += ../rt5392/common/rtmp_mcu.o
rt5392_ap-objs += ../rt5392/common/cmm_mac_pci.o
rt5392_ap-objs += ../rt5392/common/cmm_data_pci.o
rt5392_ap-objs += ../rt5392/common/ee_prom.o
rt5392_ap-objs += ../rt5392/common/ee_efuse.o
rt5392_ap-objs += ../rt5392/common/ee_flash.o
rt5392_ap-objs += ../rt5392/common/eeprom.o
rt5392_ap-objs += ../rt5392/common/rt_rf.o

rt5392_ap-objs += ../rt5392/ap/ap.o
rt5392_ap-objs += ../rt5392/ap/ap_assoc.o
rt5392_ap-objs += ../rt5392/ap/ap_auth.o
rt5392_ap-objs += ../rt5392/ap/ap_connect.o
rt5392_ap-objs += ../rt5392/ap/ap_mlme.o
rt5392_ap-objs += ../rt5392/ap/ap_sanity.o
rt5392_ap-objs += ../rt5392/ap/ap_sync.o
rt5392_ap-objs += ../rt5392/ap/ap_wpa.o
rt5392_ap-objs += ../rt5392/ap/ap_data.o
rt5392_ap-objs += ../rt5392/ap/ap_uapsd.o
rt5392_ap-objs += ../rt5392/ap/ap_autoChSel.o
rt5392_ap-objs += ../rt5392/ap/ap_qload.o
rt5392_ap-objs += ../rt5392/ap/ap_cfg.o

### RT5392
rt5392_ap-objs += ../rt5392/chips/rt3090.o
rt5392_ap-objs += ../rt5392/chips/rt30xx.o
rt5392_ap-objs += ../rt5392/chips/rt3390.o
rt5392_ap-objs += ../rt5392/chips/rt33xx.o
rt5392_ap-objs += ../rt5392/chips/rt5390.o

rt5392_ap-objs += ../rt5392/os/linux/ap_ioctl.o
rt5392_ap-objs += ../rt5392/os/linux/rt_main_dev.o
rt5392_ap-objs += ../rt5392/os/linux/pci_main_dev.o
rt5392_ap-objs += ../rt5392/os/linux/rt_pci_rbus.o
rt5392_ap-objs += ../rt5392/os/linux/rt_profile.o
rt5392_ap-objs += ../rt5392/os/linux/rt_linux.o
rt5392_ap-objs += ../rt5392/os/linux/rt_rbus_pci_util.o

ifeq ($(CONFIG_RT5392_AP_WMM_ACM),y)
rt5392_ap-objs += ../rt5392/common/acm_edca.o
rt5392_ap-objs += ../rt5392/common/acm_comm.o
rt5392_ap-objs += ../rt5392/common/acm_iocl.o
endif

ifeq ($(CONFIG_RT5392_AP_WDS),y)
rt5392_ap-objs += ../rt5392/ap/ap_wds.o
rt5392_ap-objs += ../rt5392/ap/ap_wds_inf.o
endif

ifeq ($(CONFIG_RT5392_AP_MBSS),y)
rt5392_ap-objs += ../rt5392/ap/ap_mbss.o
endif

ifeq ($(CONFIG_RT5392_AP_APCLI),y)
rt5392_ap-objs += ../rt5392/ap/ap_apcli.o
rt5392_ap-objs += ../rt5392/ap/ap_apcli_inf.o
rt5392_ap-objs += ../rt5392/ap/apcli_assoc.o
rt5392_ap-objs += ../rt5392/ap/apcli_auth.o
rt5392_ap-objs += ../rt5392/ap/apcli_ctrl.o
rt5392_ap-objs += ../rt5392/ap/apcli_sync.o
rt5392_ap-objs += ../rt5392/common/cmm_mat.o
rt5392_ap-objs += ../rt5392/common/cmm_mat_iparp.o
rt5392_ap-objs += ../rt5392/common/cmm_mat_pppoe.o
rt5392_ap-objs += ../rt5392/common/cmm_mat_ipv6.o
endif

ifeq ($(CONFIG_RT5392_AP_DLS),y)
rt5392_ap-objs += ../rt5392/ap/ap_dls.o
endif

ifeq ($(CONFIG_RT5392_AP_IDS),y)
rt5392_ap-objs += ../rt5392/ap/ap_ids.o
endif

ifeq ($(CONFIG_RT_WSC),y)
rt5392_ap-objs += ../rt5392/common/wsc.o
rt5392_ap-objs += ../rt5392/common/wsc_tlv.o
rt5392_ap-objs += ../rt5392/common/crypt_dh.o
rt5392_ap-objs += ../rt5392/common/wsc_ufd.o
rt5392_ap-objs += ../rt5392/common/crypt_biginteger.o
endif

ifeq ($(CONFIG_RT_WAPI),y)
rt5392_ap-objs += ../rt5392/common/wapi.o
endif

ifeq ($(CONFIG_RT_IGMP_SNOOP),y)
rt5392_ap-objs += ../rt5392/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT_NETIF_BLOCK),y)
rt5392_ap-objs += ../rt5392/common/netif_block.o
endif

ifeq ($(CONFIG_RT_ATE),y)
rt5392_ap-objs += ../rt5392/common/rt_ate.o
endif

###################
#  CFLAGS
###################
EXTRA_CFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DAP_SCAN_SUPPORT -DUAPSD_AP_SUPPORT -DDOT11_N_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT
EXTRA_CFLAGS += -DRSSI_FEEDBACK

ifeq ($(CONFIG_RT_FIRST_CARD),5392)
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

# do not lock CountryRegion from EEPROM
EXTRA_CFLAGS += -DEEPROM_COUNTRY_UNLOCK

ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

### RT5392
EXTRA_CFLAGS += -DRT30xx -DRT3090
EXTRA_CFLAGS += -DRT33xx -DRT3390
EXTRA_CFLAGS += -DRT53xx -DRT5390
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
EXTRA_CFLAGS += -DRTMP_FREQ_CALIBRATION_SUPPORT
EXTRA_CFLAGS += -DIQ_CAL_SUPPORT
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_COMPENSATION
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT

ifeq ($(CONFIG_RT_READ_MAC_FROM_MTD),y)
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
endif

ifeq ($(CONFIG_RT_DEBUG),y)
EXTRA_CFLAGS += -DDBG
EXTRA_CFLAGS += -DRELEASE_EXCLUDE
EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT
endif

ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DRALINK_QA -DCONFIG_RT2880_ATE_CMD_NEW
endif

ifeq ($(CONFIG_RT5392_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_RT5392_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_RT5392_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_RT5392_AP_WMM_ACM),y)
EXTRA_CFLAGS += -DWMM_ACM_SUPPORT
endif

ifeq ($(CONFIG_RT5392_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_RT5392_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_RT5392_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RT5392_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
ifeq ($(CONFIG_RT5392_AP_MBSS_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
endif
endif

ifeq ($(CONFIG_RT5392_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT
EXTRA_CFLAGS += -DMAT_SUPPORT
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

ifeq ($(CONFIG_RT_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_RT_MC_SUPPORT),y)
EXTRA_CFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

