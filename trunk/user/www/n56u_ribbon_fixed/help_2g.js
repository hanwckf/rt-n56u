var helptitle = new Array(24);
// Wireless
helptitle[0] = [["", ""],
				["<#WLANConfig11b_SSID_itemname#>", "rt_ssid"],
				["<#WLANConfig11b_x_BlockBCSSID_itemname#>", "rt_closed"],
				["<#WLANConfig11b_Channel_itemname#>", "rt_channel"],
				["<#WLANConfig11b_x_Mode11g_itemname#>", "rt_gmode"],
				["<#WLANConfig11b_AuthenticationMethod_itemname#>", "rt_auth_mode"],
				["<#WLANConfig11b_WPAType_itemname#>", "rt_crypto"],
				["<#WLANConfig11b_x_PSKKey_itemname#>", "rt_wpa_psk"],
				["<#WLANConfig11b_x_Phrase_itemname#>", "rt_phrase_x"],
				["<#WLANConfig11b_WEPType_itemname#>", "rt_wep_x"],
				["<#WLANConfig11b_WEPDefaultKey_itemname#>", "rt_key"],
				["<#WLANConfig11b_x_Rekey_itemname#>", "rt_wpa_gtk_rekey"],
				["<#WLANConfig11b_WEPKey_itemname#>", "rt_asuskey1"],
				["<#WLANConfig11b_WEPKey_itemname#>", "rt_asuskey1"],
				["<#WLANConfig11b_ChannelBW_itemname#>", "rt_HT_BW"],
				["<#WLANConfig11b_EChannel_itemname#>", "rt_HT_EXTCHA"],
				["", ""],
				["<#WLANConfig11b_TxPower_itemname#>", "rt_TxPower"],
				["<#WLANConfig11b_WEPKey_itemname#>", "rt_key1"],
				["<#WLANConfig11b_WEPKey_itemname#>", "rt_key2"],
				["<#WLANConfig11b_WEPKey_itemname#>", "rt_key3"],
				["<#WLANConfig11b_WEPKey_itemname#>", "rt_key4"],
				["<#WLANConfig11b_x_RadioEnable_itemname#>", "rt_radio_x"],
				["<#WLANConfig11b_x_RadioEnableDate_itemname#>", "rt_radio_date_x_"],
				["<#WLANConfig11b_x_RadioEnableTime_itemname#>", "rt_radio_time_x_"]];
helptitle[1] = [["", ""],
				["<#WLANConfig11b_x_APMode_itemname#>", "rt_mode_x"],
				["<#WLANConfig11b_Channel_itemname#>", "rt_channel"],
				["<#WLANConfig11b_x_BRApply_itemname#>", "rt_wdsapply_x"]];
helptitle[2] = [["", ""],
				["<#WLANAuthentication11a_ExAuthDBIPAddr_itemname#>", "rt_radius_ipaddr"],
				["<#WLANAuthentication11a_ExAuthDBPortNumber_itemname#>", "rt_radius_port"],
				["<#WLANAuthentication11a_ExAuthDBPassword_itemname#>", "rt_radius_key"]];
helptitle[3] = [["", ""],
				["", ""],
				["", ""],
				["", ""],
				["<#WLANConfig11n_PremblesType_itemname#>", "rt_preamble"],
				["<#WLANConfig11b_x_IsolateAP_itemname#>", "rt_ap_isolate"],
				["<#WLANConfig11b_DataRateAll_itemname#>", "rt_rate"],
				["<#WLANConfig11b_MultiRateAll_itemname#>", "rt_mcastrate"],
				["<#WLANConfig11b_DataRate_itemname#>", "rt_rateset"],
				["<#WLANConfig11b_x_Frag_itemname#>", "rt_frag"],
				["<#WLANConfig11b_x_RTS_itemname#>", "rt_rts"],
				["<#WLANConfig11b_x_DTIM_itemname#>", "rt_dtim"],
				["<#WLANConfig11b_x_Beacon_itemname#>", "rt_bcn"],
				["<#WLANConfig11b_x_TxBurst_itemname#>", "rt_TxBurst"],
				["<#WLANConfig11b_x_WMM_itemname#>", "rt_wme"],
				["<#WLANConfig11b_x_NOACK_itemname#>", "rt_wme_no_ack"],
				["<#WLANConfig11b_x_PktAggregate_itemname#>", "rt_PktAggregate"],
				["<#WLANConfig11b_x_APSD_itemname#>", "rt_APSDCapable"],
				["<#WLANConfig11b_x_DLS_itemname#>", "rt_DLSCapable"],
				["<#WLANConfig11b_x_HT_OpMode_itemname#>", "rt_HT_OpMode"],
				["<#WLANConfig11n_PremblesType_itemname#>", "rt_plcphdr"] // 20
				];
// LAN
helptitle[4] = [["", ""],
				["<#LANHostConfig_IPRouters_itemname#>", "lan_ipaddr"],
				["<#LANHostConfig_SubnetMask_itemname#>", "lan_netmask"],
				["<#LANHostConfig_x_Gateway_itemname#>", "lan_gateway"]];
helptitle[5] = [["", ""],
			 	["<#LANHostConfig_DHCPServerConfigurable_itemname#>", "dhcp_enable_x"],
				["<#LANHostConfig_DomainName_itemname#>", "lan_domain"],
				["<#LANHostConfig_MinAddress_itemname#>", "dhcp_start"],
				["<#LANHostConfig_MaxAddress_itemname#>", "dhcp_end"],
				["<#LANHostConfig_LeaseTime_itemname#>", "dhcp_lease"],
				["<#LANHostConfig_x_LGateway_itemname#>", "dhcp_gateway_x"],
				["<#LANHostConfig_x_LDNSServer1_itemname#>", "dhcp_dns1_x"],
				["<#LANHostConfig_x_LDNSServer2_itemname#>", "dhcp_dns2_x"],
				["<#LANHostConfig_x_LDNSServer3_itemname#>", "dhcp_dns3_x"],
				["<#LANHostConfig_x_WINSServer_itemname#>", "dhcp_wins_x"],
				["<#LANHostConfig_ManualDHCPEnable_itemname#>", "dhcp_static_x"]];
helptitle[6] = [["", ""],
				["<#RouterConfig_GWStaticIP_itemname#>", "sr_ipaddr_x_0"],
				["<#RouterConfig_GWStaticMask_itemname#>", "sr_netmask_x_0"],
				["<#RouterConfig_GWStaticGW_itemname#>", "sr_gateway_x_0"],
				["<#RouterConfig_GWStaticMT_itemname#>", "sr_matric_x_0"],
				["<#RouterConfig_GWStaticIF_itemname#>", "sr_if_x_0"]];
// WAN
helptitle[7] = [["", ""],
				["<#IPConnection_ExternalIPAddress_itemname#>", "wan_ipaddr"],
				["<#IPConnection_x_ExternalSubnetMask_itemname#>", "wan_netmask"],
				["<#IPConnection_x_ExternalGateway_itemname#>", "wan_gateway"],
				["<#PPPConnection_UserName_itemname#>", "wan_pppoe_username"],
				["<#PPPConnection_Password_itemname#>", "wan_pppoe_passwd"],
				["<#PPPConnection_IdleDisconnectTime_itemname#>", "wan_pppoe_idletime"],
				["<#PPPConnection_x_PPPoEMTU_itemname#>", "wan_pppoe_mtu"],
				["<#PPPConnection_x_PPPoEMRU_itemname#>", "wan_pppoe_mru"],
				["<#PPPConnection_x_ServiceName_itemname#>", "wan_pppoe_service"],
				["<#PPPConnection_x_AccessConcentrator_itemname#>", "wan_pppoe_ac"],
				["<#PPPConnection_x_PPPoERelay_itemname#>", "fw_pt_pppoe"],
				["<#IPConnection_x_DNSServerEnable_itemname#>", "wan_dnsenable_x"],
				["<#IPConnection_x_DNSServer1_itemname#>", "wan_dns1_x"],
				["<#IPConnection_x_DNSServer2_itemname#>", "wan_dns2_x"],
				["<#PPPConnection_x_HostNameForISP_itemname#>", "wan_hostname"],
				["<#PPPConnection_x_MacAddressForISP_itemname#>", "wan_hwaddr_x"],
				["<#PPPConnection_x_PPTPOptions_itemname#>", "wan_pptp_options_x"],
				["<#PPPConnection_x_AdditionalOptions_itemname#>", "wan_ppp_user"],
				["<#PPPConnection_x_HeartBeat_itemname#>", "wan_ppp_peer"],
				["<#IPConnection_BattleNet_itemname#>", "sp_battle_ips"],
				["<#Layer3Forwarding_x_STB_itemname#>", "wan_stb_x"]];
//Firewall
helptitle[8] = [["", ""],
				["<#FirewallConfig_WanLanLog_itemname#>", "fw_log_x"],
				["<#FirewallConfig_x_WanWebEnable_itemname#>", "misc_http_x"],
				["<#FirewallConfig_x_WanWebPort_itemname#>", "misc_httpport_x"],
				["<#FirewallConfig_x_WanLPREnable_itemname#>", "misc_lpr_x"],
				["<#FirewallConfig_x_WanPingEnable_itemname#>", "misc_ping_x"],
				["<#FirewallConfig_FirewallEnable_itemname#>", "fw_enable_x"],
				["<#FirewallConfig_DoSEnable_itemname#>", "fw_dos_x"]];
helptitle[9] = [["", ""],
				["<#FirewallConfig_URLActiveDate_itemname#>", "url_date_x_"],
				["<#FirewallConfig_URLActiveTime_itemname#>", "url_time_x_"]];
helptitle[10] = [["", ""],
				["<#FirewallConfig_LanWanActiveDate_itemname#>", "filter_lw_date_x_"],
				["<#FirewallConfig_LanWanActiveTime_itemname#>", "filter_lw_time_x_"],
				["<#FirewallConfig_LanWanDefaultAct_itemname#>", "filter_lw_default_x"],
				["<#FirewallConfig_LanWanICMP_itemname#>", "filter_lw_icmp_x"],
				["<#FirewallConfig_LanWanFirewallEnable_itemname#>", "fw_lw_enable_x"]];
//Administration
helptitle[11] = [["", ""],
				["<#LANHostConfig_x_ServerLogEnable_itemname#>", "log_ipaddr"],
				["<#LANHostConfig_x_TimeZone_itemname#>", "time_zone"],
				["<#LANHostConfig_x_NTPServer_itemname#>", "ntp_server0"]];
//Log
helptitle[12] = [["", ""],
				["<#General_x_SystemUpTime_itemname#>", "system_now_time"],
				["<#PrinterStatus_x_PrinterModel_itemname#>", ""],
				["<#PrinterStatus_x_PrinterStatus_itemname#>", ""],
				["<#PrinterStatus_x_PrinterUser_itemname#>", ""]];
//WPS
helptitle[13] = [["", ""],
				["<#WLANConfig11b_x_WPS_itemname#>", ""],
				["<#WLANConfig11b_x_WPSMode_itemname#>", ""],
				["<#WLANConfig11b_x_WPSPIN_itemname#>", ""],
				["<#WLANConfig11b_x_DevicePIN_itemname#>", ""]];
//UPnP
helptitle[14] = [["", ""],
				["<#menu2#>", ""]];
//AiDisk Wizard
helptitle[15] = [["", ""],
				["<a href='../Advanced_AiDisk_ftp.asp' target='_parent' hidefocus='true'><#t1USB#></a>", ""],
				["<#AiDisk_Step1_helptitle#>", ""],
				["<#AiDisk_Step2_helptitle#>", ""],
				["<#AiDisk_Step3_helptitle#>", ""]];

helptitle[16] = [["", ""],
				["<#EzQoS_helptitle#>", ""]];
//Others in the USB application
helptitle[17] = [["", ""],
				["<#ShareNode_MaximumLoginUser_itemname#>", "st_max_user"],
				["<#ShareNode_DeviceName_itemname#>", "computer_name"],
				["<#ShareNode_WorkGroup_itemname#>", "st_samba_workgroup"],
				["<#BasicConfig_EnableDownloadMachine_itemname#>", "apps_dl"],
				["<#BasicConfig_EnableDownloadShare_itemname#>", "apps_dl_share"],
				["<#BasicConfig_EnableMediaServer_itemname#>", "upnp_enable"],
				["<#ShareNode_Seeding_itemname#>", "apps_seed"],
				["<#ShareNode_MaxUpload_itemname#>", "apps_upload_max"],
				["<#StorageEnableTRMD#>", "trmd_enable"],
				["<#StorageEnableAria#>", "aria_enable"]];
// MAC filter
helptitle[18] = [["", ""],
				["<#FirewallConfig_MFMethod_itemname#>", "macfilter_enable_x"],
				["<#FirewallConfig_LanWanSrcPort_itemname#>", ""],
				["<#FirewallConfig_LanWanSrcIP_itemname#>/<#FirewallConfig_LanWanDstIP_itemname#>", ""],
				["<#BM_UserList3#>", ""]];
// Setting
helptitle[19] = [["", ""],
				["<#Setting_factorydefault_itemname#>", ""],
				["<#Setting_save_itemname#>", ""],
				["<#Setting_upload_itemname#>", ""]];
// QoS
helptitle[20] = [["", ""],
				["<#BM_measured_uplink_speed#>", ""],
				["<#BM_manual_uplink_speed#>", "qos_manual_ubw"]];
// HSDPA
helptitle[21] = [["", ""],
				["<#HSDPAConfig_hsdpa_mode_itemname#>", "hsdpa_mode"],
				["<#HSDPAConfig_pin_code_itemname#>", "pin_code"],
				["<#HSDPAConfig_private_apn_itemname#>", "private_apn"],
				["<#PPPConnection_x_PPPoEMTU_itemname#>", "modem_mtu"],
				["<#IPConnection_x_DNSServerEnable_itemname#>", "modem_dnsa"],
				["<#IPConnection_x_DNSServer1_itemname#>", "wan_dns1_x"],
				["<#IPConnection_x_DNSServer2_itemname#>", "wan_dns2_x"],
				["<#HSDPAConfig_ISP_itemname#>", "private_isp"],
				["<#HSDPAConfig_Country_itemname#>", "private_country"],
				["<#HSDPAConfig_DialNum_itemname#>", "private_dialnum"],
				["<#HSDPAConfig_Username_itemname#>", "private_username"],
				["<#HSDPAConfig_Password_itemname#>", "private_passowrd"]];

helptitle[22] = [["", ""],
				["Router(<#OP_GW_item#>)", ""],
				["AP(<#OP_AP_item#>)", ""]];

// title ssid
helptitle[23] = [["", ""],
				["<#TweaksWdg#>", "watchdog_cpu"]];

var helpcontent = new Array(24);
helpcontent[0] = new Array("",
				"<#WLANConfig11b_SSID_itemdesc#>",
				"<#WLANConfig11b_x_BlockBCSSID_itemdesc#>",
				"<#WLANConfig11b_Channel_itemdesc#>",
				"<#WLANConfig11b_x_Mode11g_itemdesc#>",
				"<#WLANConfig11b_AuthenticationMethod_itemdesc#>",
				"<#WLANConfig11b_WPAType_itemdesc#>",
				"<#WLANConfig11b_x_PSKKey_itemdesc#>",
				"<#WLANConfig11b_x_Phrase_itemdesc#>",
				"<#WLANConfig11b_WEPType_itemdesc#>",
				"<#WLANConfig11b_WEPDefaultKey_itemdesc#>",
				"<#WLANConfig11b_x_Rekey_itemdesc#><#JS_field_wanip_rule3#>",
				"<#WLANConfig11b_WEPKey_itemtype1#>",
				"<#WLANConfig11b_WEPKey_itemtype2#>",
				"<#WLANConfig11b_ChannelBW_itemdesc#><br/><#WLANConfig11b_Wireless_Speed_itemname_3#>",
				"<#WLANConfig11b_EChannel_itemdesc#>",
				"",
				"<#WLANConfig11b_TxPower_itemdesc#>(<#JS_validrange#> 0 <#JS_validrange_to#> 100)",
				"<#WLANConfig11b_WEPKey_itemtype1#><br/><#WLANConfig11b_WEPKey_itemtype2#>",
				"<#WLANConfig11b_WEPKey_itemtype1#><br/><#WLANConfig11b_WEPKey_itemtype2#>",
				"<#WLANConfig11b_WEPKey_itemtype1#><br/><#WLANConfig11b_WEPKey_itemtype2#>",
				"<#WLANConfig11b_WEPKey_itemtype1#><br/><#WLANConfig11b_WEPKey_itemtype2#>",
				"<#WLANConfig11b_x_RadioEnable_itemdesc#>",
				"<#WLANConfig11b_x_RadioEnableDate_itemdesc#>",
				"<#WLANConfig11b_x_RadioEnableTime_itemdesc#>");
helpcontent[1] = new Array("",
				"<#WLANConfig11b_x_APMode_itemdesc#>",
				"<#WLANConfig11b_Channel_itemdesc#>",
				"<#WLANConfig11b_x_BRApply_itemdesc#>");
helpcontent[2] = new Array("",
				"<#WLANAuthentication11a_ExAuthDBIPAddr_itemdesc#>",
				"<#WLANAuthentication11a_ExAuthDBPortNumber_itemdesc#>",
				"<#WLANAuthentication11a_ExAuthDBPassword_itemdesc#>");
helpcontent[3] = new Array("",
				"",
				"",
				"",
				"<#WLANConfig11n_PremblesType_itemdesc#>",
				"<#WLANConfig11b_x_IsolateAP_itemdesc#>",
				"<#WLANConfig11b_DataRateAll_itemdesc#>",
				"<#WLANConfig11b_MultiRateAll_itemdesc#>",
				"<#WLANConfig11b_DataRate_itemdesc#>",
				"<#WLANConfig11b_x_Frag_itemdesc#>",
				"<#WLANConfig11b_x_RTS_itemdesc#>",
				"<#WLANConfig11b_x_DTIM_itemdesc#>",
				"<#WLANConfig11b_x_Beacon_itemdesc#>",
				"<#WLANConfig11b_x_TxBurst_itemdesc#>",
				"<#WLANConfig11b_x_WMM_itemdesc#>",
				"[b Only, g Only, b/g Mixed]: <#WLANConfig11b_x_NOACK_itemdesc#>",
				"<#WLANConfig11b_x_PktAggregate_itemdesc#>",
				"<#WLANConfig11b_x_APSD_itemdesc#>",
				"<#WLANConfig11b_x_DLS_itemdesc#>",
				"[n Only]: <#WLANConfig11b_x_HT_OpMode_itemdesc#>",
				"<#WLANConfig11n_PremblesType_itemdesc#>");
helpcontent[4] = new Array("",
				"<#LANHostConfig_IPRouters_itemdesc#>",
				"<#LANHostConfig_SubnetMask_itemdesc#>",
				"<#LANHostConfig_x_Gateway_itemdesc#>");
helpcontent[5] = new Array("",
				"<#LANHostConfig_DHCPServerConfigurable_itemdesc#>",
				"<#LANHostConfig_DomainName_itemdesc#>",
				"<#LANHostConfig_MinAddress_itemdesc#>",
				"<#LANHostConfig_MaxAddress_itemdesc#>",
				"<#LANHostConfig_LeaseTime_itemdesc#>",
				"<#LANHostConfig_x_LGateway_itemdesc#>",
				"<#LANHostConfig_x_LDNSServer1_itemdesc#>",
				"<#LANHostConfig_x_LDNSServer1_itemdesc#>",
				"<#LANHostConfig_x_LDNSServer1_itemdesc#>",
				"<#LANHostConfig_x_WINSServer_itemdesc#>",
				"<#LANHostConfig_ManualDHCPEnable_itemdesc#>");
helpcontent[6] = new Array("",
				"<#RHELP_desc4#>",
				"<#RHELP_desc5#>",
				"<#RHELP_desc6#>",
				"<#RHELP_desc7#>",
				"<#RHELP_desc8#>");
//WAN
helpcontent[7] = new Array("",
				"<#IPConnection_ExternalIPAddress_itemdesc#>",
				"<#IPConnection_x_ExternalSubnetMask_itemdesc#>",
				"<#IPConnection_x_ExternalGateway_itemdesc#>",
				"<#PPPConnection_UserName_itemdesc#>",
				"<#PPPConnection_Password_itemdesc#>",
				"<#PPPConnection_IdleDisconnectTime_itemdesc#>",
				"<#PPPConnection_x_PPPoEMTU_itemdesc#>",
				"<#PPPConnection_x_PPPoEMRU_itemdesc#>",
				"<#PPPConnection_x_ServiceName_itemdesc#>",
				"<#PPPConnection_x_AccessConcentrator_itemdesc#>",
				"<#PPPConnection_x_PPPoERelay_itemdesc#>",
				"<#IPConnection_x_DNSServerEnable_itemdesc#>",
				"<#IPConnection_x_DNSServer1_itemdesc#>",
				"<#IPConnection_x_DNSServer1_itemdesc#>",
				"<#BOP_isp_host_desc#>",
				"<#PPPConnection_x_MacAddressForISP_itemdesc#>",
				"<#PPPConnection_x_PPTPOptions_itemdesc#>",
				"<#PPPConnection_x_AdditionalOptions_itemdesc#>",
				"<#BOP_isp_heart_desc#>",
				"<#IPConnection_BattleNet_itemdesc#>",
				"<#Layer3Forwarding_x_STB_itemdesc#>");
//Firewall
helpcontent[8] = new Array("",
				"<#FirewallConfig_WanLanLog_itemdesc#>",
				"<#FirewallConfig_x_WanWebEnable_itemdesc#>",
				"<#FirewallConfig_x_WanWebPort_itemdesc#>",
				"<#FirewallConfig_x_WanLPREnable_itemdesc#>",
				"<#FirewallConfig_x_WanPingEnable_itemdesc#>",
				"<#FirewallConfig_FirewallEnable_itemdesc#>",
				"<#FirewallConfig_DoSEnable_itemdesc#>");
helpcontent[9] = new Array("",
				"<#FirewallConfig_URLActiveDate_itemdesc#>",
				"<#FirewallConfig_URLActiveTime_itemdesc#>");
helpcontent[10] = new Array("",
				"<#FirewallConfig_LanWanActiveDate_itemdesc#>",
				"<#FirewallConfig_LanWanActiveTime_itemdesc#>",
				"<#FirewallConfig_LanWanDefaultAct_itemdesc#>",
				"<#FirewallConfig_LanWanICMP_itemdesc#>",
				"<#FirewallConfig_LanWanFirewallEnable_itemdesc#>");
//Administration
helpcontent[11] = new Array("",
				"<#LANHostConfig_x_ServerLogEnable_itemdesc#>",
				"<#LANHostConfig_x_TimeZone_itemdesc#>",
				"<#LANHostConfig_x_NTPServer_itemdesc#>");
//Log
helpcontent[12] = new Array("",
				"<#General_x_SystemUpTime_itemdesc#>",
				"<#PrinterStatus_x_PrinterModel_itemdesc#>",
				"<#PrinterStatus_x_PrinterStatus_itemdesc#>",
				"<#PrinterStatus_x_PrinterUser_itemdesc#>");
//WPS
helpcontent[13] = new Array("",
				"<#WLANConfig11b_x_WPS_itemdesc#>",
				"<#WLANConfig11b_x_WPSMode_itemdesc#>",
				"<#WLANConfig11b_x_WPSPIN_itemdesc#>",
				"<#WLANConfig11b_x_DevicePIN_itemdesc#>");
//UPnP
helpcontent[14] = new Array("",
				"<#UPnPMediaServer_Help#>");
//AiDisk Wizard
helpcontent[15] = new Array("",
				"<#AiDisk_moreconfig#>",
				"<#AiDisk_Step1_help#><p><a href='../Advanced_AiDisk_ftp.asp' target='_parent' hidefocus='true'><#MoreConfig#></a></p><!--span style='color:#CC0000'><#AiDisk_Step1_help2#></span-->",
				"<#AiDisk_Step2_help#>",
				"<#AiDisk_Step3_help#>");
//EzQoS
helpcontent[16] = new Array("",
				"<#EZQoSDesc1#><p><#EZQoSDesc2#> <a href='/Advanced_QOSUserSpec_Content.asp'><#BM_title_User#></a></p>");
//Others in the USB application
helpcontent[17] = new Array("",
				"<#JS_storageMLU#>",
				"<#JS_storageright#>",
				"<#Help_of_Workgroup#>",
				"<#JS_basiconfig1#>",
				"<#JS_basiconfig3#>",
				"<#JS_basiconfig8#>",
				"<#ShareNode_Seeding_itemdesc#>",
				"<#ShareNode_MaxUpload_itemdesc#>",
				"<#StorageTorrent_itemdesc#>",
				"<#StorageAria_itemdesc#>");
// MAC filter
helpcontent[18] = new Array("",
				"<#FirewallConfig_MFMethod_itemdesc#>",
				"<#Port_format#>",
				"<#IP_format#>",
				"<#Port_format#>");
// Setting
helpcontent[19] = new Array("",
				"<#Setting_factorydefault_itemdesc#>",
				"<#Setting_save_itemdesc#>",
				"<#Setting_upload_itemdesc#>");
// QoS
helpcontent[20] = new Array("",
				"<#BM_measured_uplink_speed_desc#>",
				"<#BM_manual_uplink_speed_desc#>");
// HSDPA
helpcontent[21] = new Array("",
				"<#HSDPAConfig_hsdpa_mode_itemdesc#>",
				"<#HSDPAConfig_pin_code_itemdesc#>",
				"<#HSDPAConfig_private_apn_itemdesc#>",
				"<#HSDPAConfig_MTU_itemdesc#>",
				"<#IPConnection_x_DNSServerEnable_itemdesc#>",
				"<#IPConnection_x_DNSServer1_itemdesc#>",
				"<#IPConnection_x_DNSServer1_itemdesc#>",
				"<#HSDPAConfig_isp_itemdesc#>",
				"<#HSDPAConfig_country_itemdesc#>",
				"<#HSDPAConfig_dialnum_itemdesc#>",
				"<#HSDPAConfig_username_itemdesc#>",
				"<#HSDPAConfig_password_itemdesc#>");

helpcontent[22] = new Array("",
				"<#OP_GW_desc1#>",
				"<#OP_GW_desc1#>",
				"<#OP_AP_desc1#>");

helpcontent[23] = new Array("",
				"<#TweaksWdg_desc#>");

var help_enable = '<% nvram_get_x("General", "help_enable"); %>';

function openTooltip(obj, hint_array_id, hint_show_id)
{
	if (help_enable == "0" && hint_show_id > 0)
		return;

	if(hint_array_id == 14
	    || hint_array_id == 15
	    || hint_array_id == 16
	    || hint_array_id == 20)
		return;

	$j(obj).attr('data-original-title', helptitle[hint_array_id][hint_show_id][0]).attr('data-content', helpcontent[hint_array_id][hint_show_id]);
	$j(obj).popover('show');
}

