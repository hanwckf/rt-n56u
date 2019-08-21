var helpcontent = new Array(25);
var help_enable = '<% nvram_get_x("", "help_enable"); %>';

helpcontent[0] = new Array("");
helpcontent[1] = new Array("");
helpcontent[2] = new Array("");
helpcontent[3] = new Array("");

helpcontent[4] = new Array("",
				"<#LANHostConfig_IPRouters_itemdesc#>",
				"<#LANHostConfig_SubnetMask_itemdesc#>",
				"<#LANHostConfig_x_Gateway_itemdesc#>");
helpcontent[5] = new Array("",
				"<#LANHostConfig_DHCPServerConfigurable_itemdesc#>",
				"<#LANHostConfig_DomainName_itemdesc#> <#LANHostConfig_x_DDNS_alarm_hostname#> <#LANHostConfig_DomainName_itemdesc2#>",
				"<#LANHostConfig_MinAddress_itemdesc#>",
				"<#LANHostConfig_MaxAddress_itemdesc#>",
				"<#LANHostConfig_LeaseTime_itemdesc#>",
				"<#LANHostConfig_x_LGateway_itemdesc#>",
				"<#LANHostConfig_x_LDNSServer1_itemdesc#>",
				"<#LANHostConfig_x_LDNSServer1_itemdesc#>",
				"<#LANHostConfig_x_LDNSServer1_itemdesc#>",
				"<#LANHostConfig_x_WINSServer_itemdesc#>",
				"<#LANHostConfig_ManualDHCPEnable_itemdesc#>",
				"<#LANHostConfig_x_LDNSServer6_itemdesc#>");
helpcontent[6] = new Array("",
				"<#RHELP_desc4#>",
				"<#RHELP_desc5#>",
				"<#RHELP_desc6#>",
				"<#RHELP_desc7#>",
				"<#RHELP_desc8#>",
				"<#RHELP_desc9#>",
				"<#RouterConfig_GWMulticast_Multicast_all_itemdesc#>",
				"<#RouterConfig_GWMulticast_Multicast_all_itemdesc#>",
				"<#RouterConfig_GWMulticast_Multicast_all_itemdesc#>",
				"<#RouterConfig_GWMulticast_Multicast_all_itemdesc#>");
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
				"<#IPConnection_x_DNSServer1_itemdesc#>",
				"<#BOP_isp_host_desc#>",
				"<#PPPConnection_x_MacAddressForISP_itemdesc#>",
				"<#PPPConnection_x_PPTPOptions_itemdesc#>",
				"<#PPPConnection_x_AdditionalOptions_itemdesc#>",
				"<#BOP_isp_heart_desc#>",
				"<#IPConnection_BattleNet_itemdesc#>",
				"<#Layer3Forwarding_x_STB_itemdesc#>",
				"<#hwnat_desc#>",
				"<#hwnat_desc#>",
				"<#vpn_passthrough_desc#>",
				"<#vpn_passthrough_desc#>",
				"<#vpn_passthrough_desc#>");
//Firewall
helpcontent[8] = new Array("",
				"<#FirewallConfig_WanLanLog_itemdesc#>",
				"<#FirewallConfig_x_WanWebEnable_itemdesc#>",
				"<#FirewallConfig_x_WanWebPort_itemdesc#>",
				"<#FirewallConfig_x_WanLPREnable_itemdesc#>",
				"<#FirewallConfig_x_WanPingEnable_itemdesc#>",
				"<#FirewallConfig_FirewallEnable_itemdesc#>",
				"<#FirewallConfig_DoSEnable_itemdesc#>",
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
				"<#LANHostConfig_x_NTPServer_itemdesc#>",
				"<#LANHostConfig_x_Password_itemdesc#>");
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
				"<#WLANConfig11b_x_DevicePIN_itemdesc#>",
				"<#WLANConfig11b_x_WPSband_itemdesc#>");
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
				"<#EZQoSDesc1#><br><#EZQoSDesc2#> <a href='/Advanced_QOSUserSpec_Content.asp'><#BM_title_User#></a>");
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
				"<#BasicConfig_USBStorageWhiteist_itemdesc#>",
				"<#ShareNode_FTPLANG_itemdesc#>",
				"<#StorageTorrent_itemdesc#>",
				"<#StorageAria_itemdesc#>");

// MAC filter
helpcontent[18] = new Array("",
				"<#FirewallConfig_MFMethod_itemdesc#>",
				"<#Port_format#>",
				"<#IP_format#>");
// Setting
helpcontent[19] = new Array("",
				"<#Setting_factorydefault_itemdesc#>",
				"<#Setting_save_itemdesc#>",
				"<#Setting_upload_itemdesc#>",
				"<#Storage_upload_itemdesc#>");
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
// Tweaks
helpcontent[23] = new Array("",
				"<#TweaksWdg_desc#>");

// DDNS
helpcontent[24] = new Array("",
				"<#LANHostConfig_x_DDNSUserName_itemdesc#>",
				"<#LANHostConfig_x_DDNSPassword_itemdesc#>",
				"<#LANHostConfig_x_DDNSHostNames_itemdesc#>",
				"<#LANHostConfig_x_DDNSWildcard_itemdesc#>",
				"<#LANHostConfig_x_DDNSStatus_itemdesc#>");


function openTooltip(obj, hint_array_id, hint_show_id)
{
	if (help_enable == "0" && hint_show_id > 0)
		return;

	if(hint_array_id >= helpcontent.length)
		return;

	if(hint_array_id == 14
	    || hint_array_id == 15
	    || hint_array_id == 16
	    || hint_array_id == 20)
		return;

	if(hint_show_id >= helpcontent[hint_array_id].length)
		return;

	$j(obj).attr('data-original-title', obj.innerHTML).attr('data-content', helpcontent[hint_array_id][hint_show_id]);
	$j(obj).popover('show');
}

function openHint(hint_array_id, hint_show_id){
	if (help_enable == "0" && hint_show_id > 0)
		return;
	
	$('hintofPM').style.display = "";
	
	showtext($('helpname'), "<#CTL_help#>");
	
	if($("statusframe")){
		$("statusframe").src = "";
		$("statusframe").style.display = "none";
	}
	
	$('hint_body').style.display = "";
	$("statusframe").style.display = "none";
	
	showtext($('helpname'), "<#CTL_help#>");
	showtext($('hint_body'), helpcontent[hint_array_id][hint_show_id]);
}

function closeHint(){
	$('hintofPM').style.display = "none";
}

