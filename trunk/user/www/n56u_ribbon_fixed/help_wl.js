var helpcontent = new Array(5);
var help_enable = '<% nvram_get_x("", "help_enable"); %>';

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
				"<#WLANConfig11b_BGProt11g_itemdesc#>",
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
				"<#WLANConfig11b_x_NOACK_itemdesc#>",
				"<#WLANConfig11b_x_PktAggregate_itemdesc#>",
				"<#WLANConfig11b_x_APSD_itemdesc#>",
				"<#WLANConfig11b_x_DLS_itemdesc#>",
				"[n Only]: <#WLANConfig11b_x_HT_OpMode_itemdesc#>");
// MAC filter
helpcontent[4] = new Array("",
				"<#FirewallConfig_MFMethod_itemdesc#>");

function openTooltip(obj, hint_array_id, hint_show_id)
{
	if (help_enable == "0" && hint_show_id > 0)
		return;

	if(hint_array_id >= helpcontent.length)
		return;

	if(hint_show_id >= helpcontent[hint_array_id].length)
		return;

	$j(obj).attr('data-original-title', obj.innerHTML).attr('data-content', helpcontent[hint_array_id][hint_show_id]);
	$j(obj).popover('show');
}
