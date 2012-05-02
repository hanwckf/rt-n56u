<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">

<title>ASUS Wireless Router <#Web_Title#> - 2.4G <#menu5_1_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link href="other.css"  rel="stylesheet" type="text/css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help_2g.js"></script>
<script type="text/javascript" src="/general_2g.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,1,1);
	show_footer();
	
	enable_auto_hint(0, 21);
	
	load_body();
	
	document.form.rt_ssid.value = decodeURIComponent(document.form.rt_ssid2.value);
	document.form.rt_wpa_psk.value = decodeURIComponent(document.form.rt_wpa_psk_org.value);
	document.form.rt_key1.value = decodeURIComponent(document.form.rt_key1_org.value);
	document.form.rt_key2.value = decodeURIComponent(document.form.rt_key2_org.value);
	document.form.rt_key3.value = decodeURIComponent(document.form.rt_key3_org.value);
	document.form.rt_key4.value = decodeURIComponent(document.form.rt_key4_org.value);
	document.form.rt_phrase_x.value = decodeURIComponent(document.form.rt_phrase_x_org.value);
	
	if(document.form.rt_wpa_psk.value.length <= 0)
		document.form.rt_wpa_psk.value = "Please type Password";

	if(sw_mode == 2)
		disableAdvFn();
		
	//rt_nband_select(2);
	rt_auth_mode_change(1);
	
	document.form.rt_channel.value = document.form.rt_channel_orig.value;
	
	if(document.form.rt_gmode.value=='0'){
			$("bg_protect_tr").style.display = "none";
	}
	else{	
			$("bg_protect_tr").style.display = "";
	}
		
	automode_hint();
		
	insertExtChannelOption();		
}

function applyRule(){
	var auth_mode = document.form.rt_auth_mode.value;
	
	if(document.form.rt_wpa_psk.value == "Please type Password")
		document.form.rt_wpa_psk.value = "";

	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "";
		document.form.next_page.value = "/as.asp";
		document.form.wps_config_state.value = "1";
		
		if((auth_mode == "shared" || auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "radius" ||
				((auth_mode == "open") && !(document.form.rt_wep_x.value == "0")))
				&& document.form.wps_mode.value == "enabled")
			document.form.wps_mode.value = "disabled";
		
		if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "radius")
			document.form.next_page.value = "/Advanced_WSecurity2g_Content.asp";
		
		inputCtrl(document.form.rt_crypto, 1);
		inputCtrl(document.form.rt_wpa_psk, 1);
		inputCtrl(document.form.rt_wep_x, 1);
		inputCtrl(document.form.rt_key, 1);
		inputCtrl(document.form.rt_key1, 1);
		inputCtrl(document.form.rt_key2, 1);
		inputCtrl(document.form.rt_key3, 1);
		inputCtrl(document.form.rt_key4, 1);
		inputCtrl(document.form.rt_phrase_x, 1);
		inputCtrl(document.form.rt_wpa_gtk_rekey, 1);
		
		document.form.submit();
	}
}

function validForm(){
	var auth_mode = document.form.rt_auth_mode.value;
	
	if(!validate_string_ssid(document.form.rt_ssid))
		return false;
	//document.form.ssid_acsii.value = encodeURIComponent(document.form.rt_ssid.value);
	
	if(document.form.rt_wep_x.value != "0")
		if(!validate_wlphrase('WLANConfig11b', 'rt_phrase_x', document.form.rt_phrase_x))
			return false;	
	if(auth_mode == "psk"){ //2008.08.04 lock modified
		if(!validate_psk(document.form.rt_wpa_psk))
			return false;
		
		if(!validate_range(document.form.rt_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else if(auth_mode == "wpa" || auth_mode == "wpa2"){
		if(!validate_range(document.form.rt_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else{
		var cur_wep_key = eval('document.form.rt_key'+document.form.rt_key.value);		
		if(auth_mode != "radius" && !validate_wlkey(cur_wep_key))
			return false;
	}	
	return true;
}

function done_validating(action){
	refreshpage();
}

function change_key_des(){
	var objs = getElementsByName_iefix("span", "key_des");
	var wep_type = document.form.rt_wep_x.value;
	var str = "";
	
	if(wep_type == "1")
		str = "(<#WLANConfig11b_WEPKey_itemtype1#>)";
	else if(wep_type == "2")
		str = "(<#WLANConfig11b_WEPKey_itemtype2#>)";
	
	for(var i = 0; i < objs.length; ++i)
		showtext(objs[i], str);
}

function validate_wlphrase(s, v, obj){
	if(!validate_string(obj)){
		is_wlphrase(s, v, obj);
		return(false);
	}
	
	return true;
}

function disableAdvFn(){
	for(var i=16; i>=1; i--)
		$("WLgeneral").deleteRow(i);
}

/*function rt_nband_select(ch){
	if(ch == "1"){
//		showtext($("rt_channel_select"), "5 GHz <#WLANConfig11b_Channel_itemname#>");
		document.form.rt_nband.value = 1;
		insertExtChannelOption();
		return change_common_radio(this, 'WLANConfig11b', 'rt_nband', '1');
	}
	else{
//		showtext($("rt_channel_select"), "2.4 GHz <#WLANConfig11b_Channel_itemname#>");
		document.form.rt_nband.value = 2;
		insertExtChannelOption();
		return change_common_radio(this, 'WLANConfig11b', 'rt_nband', '2');
	}
}*/
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(0, 11);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
		    </div>
		  <div class="drImg"><img src="images/DrsurfImg.gif"></div>
			<div style="height:70px; "></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="wan_route_x" value="<% nvram_get_x("IPConnection","wan_route_x"); %>">
<input type="hidden" name="wan_nat_x" value="<% nvram_get_x("IPConnection","wan_nat_x"); %>">

<input type="hidden" name="current_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="next_page" value="Advanced_Wireless_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="rt_country_code" value="<% nvram_get_x("","rt_country_code"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="wps_mode" value="<% nvram_get_x("WLANConfig11b", "wps_mode"); %>">
<input type="hidden" name="wps_config_state" value="<% nvram_get_x("WLANConfig11b", "wps_config_state"); %>">

<input type="hidden" name="rt_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b",  "rt_ssid"); %>">
<input type="hidden" name="rt_wpa_mode" value="<% nvram_get_x("WLANConfig11b","rt_wpa_mode"); %>">
<input type="hidden" name="rt_wpa_psk_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_wpa_psk"); %>">
<input type="hidden" name="rt_key1_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_key1"); %>">
<input type="hidden" name="rt_key2_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_key2"); %>">
<input type="hidden" name="rt_key3_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_key3"); %>">
<input type="hidden" name="rt_key4_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_key4"); %>">
<input type="hidden" name="rt_phrase_x_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_phrase_x"); %>">

<input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get_x("Regulatory","x_RegulatoryDomain"); %>" readonly="1">
<input type="hidden" name="rt_gmode_protection" value="<% nvram_get_x("WLANConfig11b", "rt_gmode_protection"); %>">

<input type="hidden" name="rt_wme" value="<% nvram_get_x("WLANConfig11b","rt_wme"); %>">
<input type="hidden" name="rt_mode_x" value="<% nvram_get_x("WLANConfig11b","rt_mode_x"); %>">
<input type="hidden" name="rt_nmode" value="<% nvram_get_x("WLANConfig11b","rt_nmode"); %>">
<input type="hidden" name="rt_HT_EXTCHA_old" value="<% nvram_get_x("WLANConfig11b","rt_HT_EXTCHA"); %>">

<input type="hidden" name="rt_nband" value="2">
<input type="hidden" name="rt_key_type" value='<% nvram_get_x("WLANConfig11b","rt_key_type"); %>'> <!--Lock Add 2009.03.10 for ralink platform-->
<input type="hidden" name="rt_channel_orig" value='<% nvram_get_x("WLANConfig11b","rt_channel"); %>'>

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
	<td height="430" valign="top">
	  <div id="tabMenu" class="submenuBlock"></div><br>

<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td align="left" valign="top" >
	  <table width="98%" border="0" cellpadding="5" cellspacing="0" class="FormTitle">
		<thead>
		<tr>
		  <td><#menu5_1#> - <#menu5_1_1#> (2.4GHz)</td>
		</tr>
		</thead>	
		
		<tbody>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" id="WLgeneral">
				<!--tr>
					<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(0, 16);">Radio Band</a></th>
					<td>
						<input type="radio" value="2" name="rt_nband" class="input" onClick="rt_nband_select(2)" <% nvram_match_x("WLANConfig11b", "rt_nband", "2", "checked"); %>>2.4G
						<input type="radio" value="1" name="rt_nband" class="input" onClick="rt_nband_select(1)" <% nvram_match_x("WLANConfig11b", "rt_nband", "1", "checked"); %>>5G
					</td>
				</tr-->

			  <tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#WLANConfig11b_SSID_itemname#></a></th>
					<td><input type="text" maxlength="32" class="input"  size="32" name="rt_ssid" value="" onkeypress="return is_string(this)"></td>
			  </tr>
			  
				<tr>
					<th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
					<td>
						<input type="radio" value="1" name="rt_closed" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'rt_closed', '1')" <% nvram_match_x("WLANConfig11b", "rt_closed", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="rt_closed" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'rt_closed', '0')" <% nvram_match_x("WLANConfig11b", "rt_closed", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
			  	  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 4);"><#WLANConfig11b_x_Mode11g_itemname#></a></th>
				<td>
					<select name="rt_gmode" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_gmode')">
						<option value="2" <% nvram_match_x("WLANConfig11b","rt_gmode", "2","selected"); %>>b/g/n Mixed</option>
						<option value="1" <% nvram_match_x("WLANConfig11b","rt_gmode", "1","selected"); %>>b/g Mixed</option>
						<option value="5" <% nvram_match_x("WLANConfig11b","rt_gmode", "5","selected"); %>>g/n Mixed</option>
						<option value="3" <% nvram_match_x("WLANConfig11b","rt_gmode", "3","selected"); %>>n Only</option>
						<option value="4" <% nvram_match_x("WLANConfig11b","rt_gmode", "4","selected"); %>>g Only</option>
						<option value="0" <% nvram_match_x("WLANConfig11b","rt_gmode", "0","selected"); %>>b Only</option>
					</select>
					<span id="rt_gmode_hint" style="display:none"><#WLANConfig11n_automode_limition_hint#></span>
				</td>
			  </tr>
			  <tr id="bg_protect_tr" style="display:none;">
			    <th><a class="hintstyle" href="javascript:void(0);" onClick="">b/g Protection</a></th>
			    <td>
				<select name="rt_gmode_protection" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_gmode_protection')">
					<option class="content_input_fd" value="auto" <% nvram_match_x("WLANConfig11b","rt_gmode_protection", "auto","selected"); %>>Auto</option>
					<option class="content_input_fd" value="on" <% nvram_match_x("WLANConfig11b","rt_gmode_protection", "on","selected"); %>>Always On</option>
					<option class="content_input_fd" value="off" <% nvram_match_x("WLANConfig11b","rt_gmode_protection", "off","selected"); %>>Always Off</option>
				</select>				
			    </td>
			  </tr>
			  <tr>
			    <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 14);"><#WLANConfig11b_ChannelBW_itemname#></a></th>
			    <td>
				<select name="rt_HT_BW" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_HT_BW')">				
					<option class="content_input_fd" value="0" <% nvram_match_x("WLANConfig11b","rt_HT_BW", "0","selected"); %>>20 MHz</option>
					<option class="content_input_fd" value="1" <% nvram_match_x("WLANConfig11b","rt_HT_BW", "1","selected"); %>>20/40 MHz</option>
					<option class="content_input_fd" value="2" <% nvram_match_x("WLANConfig11b","rt_HT_BW", "2","selected"); %>>40 MHz</option>
				</select>				
			    </td>
			  </tr>

				<tr>
				<th><a id="rt_channel_select" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 3);"><#WLANConfig11b_Channel_itemname#></a></th>
				<td>
				  <select name="rt_channel" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_channel')">
						<% select_channel("WLANConfig11b"); %>
				  </select>
				</td>
			  </tr>
			  
			  <tr>
			    <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 15);"><#WLANConfig11b_EChannel_itemname#></a></th>
			    <td>
						<select name="rt_HT_EXTCHA" class="input">
							<option value="1" selected>Auto</option>
						</select>
					</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
				<td>
				  <select name="rt_auth_mode" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_auth_mode');">
					<option value="open" <% nvram_match_x("WLANConfig11b", "rt_auth_mode", "open", "selected"); %>>Open System</option>
					<option value="shared" <% nvram_match_x("WLANConfig11b", "rt_auth_mode", "shared", "selected"); %>>Shared Key</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_auth_mode", "psk", "WLANConfig11b", "rt_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_auth_mode", "psk", "WLANConfig11b", "rt_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_auth_mode", "psk", "WLANConfig11b", "rt_wpa_mode", "0", "selected"); %>>WPA-Auto-Personal</option>
					<option value="wpa" <% nvram_double_match_x("WLANConfig11b", "rt_auth_mode", "wpa", "WLANConfig11b", "rt_wpa_mode", "3", "selected"); %>>WPA-Enterprise</option>
					<option value="wpa2" <% nvram_match_x("WLANConfig11b", "rt_auth_mode", "wpa2", "selected"); %>>WPA2-Enterprise</option>
					<option value="wpa" <% nvram_double_match_x("WLANConfig11b", "rt_auth_mode", "wpa", "WLANConfig11b", "rt_wpa_mode", "4", "selected"); %>>WPA-Auto-Enterprise</option>
					<option value="radius" <% nvram_match_x("WLANConfig11b", "rt_auth_mode", "radius", "selected"); %>>Radius with 802.1x</option>
				  </select>
				</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
				<td>
				  <select name="rt_crypto" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_crypto')">
						<!-- the options was defined in general2g.js, plz grep "TKIP" -->
						<option value="aes" <% nvram_match_x("WLANConfig11b", "rt_crypto", "aes", "selected"); %>>AES</option>
						<option value="tkip+aes" <% nvram_match_x("WLANConfig11b", "rt_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				  </select>
				</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
				<td>
				  <input type="text" name="rt_wpa_psk" maxlength="64" class="input" size="32" value="">
				</td>
			  </tr>
			  		  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 9);"><#WLANConfig11b_WEPType_itemname#></a></th>
				<td>
				  <select name="rt_wep_x" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_wep_x');">
					<option value="0" <% nvram_match_x("WLANConfig11b", "rt_wep_x", "0", "selected"); %>>None</option>
					<option value="1" <% nvram_match_x("WLANConfig11b", "rt_wep_x", "1", "selected"); %>>WEP-64bits</option>
					<option value="2" <% nvram_match_x("WLANConfig11b", "rt_wep_x", "2", "selected"); %>>WEP-128bits</option>
				  </select>
				  <br>
				  <span name="key_des"></span>
				</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 10);"><#WLANConfig11b_WEPDefaultKey_itemname#></a></th>
				<td>
				  <select name="rt_key" class="input"  onChange="return change_common(this, 'WLANConfig11b', 'rt_key');">
					<option value="1" <% nvram_match_x("WLANConfig11b","rt_key", "1","selected"); %>>1</option>
					<option value="2" <% nvram_match_x("WLANConfig11b","rt_key", "2","selected"); %>>2</option>
					<option value="3" <% nvram_match_x("WLANConfig11b","rt_key", "3","selected"); %>>3</option>
					<option value="4" <% nvram_match_x("WLANConfig11b","rt_key", "4","selected"); %>>4</option>
				  </select>
				</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey1_itemname#></th>
				<td><input type="text" name="rt_key1" id="rt_key1" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey2_itemname#></th>
				<td><input type="text" name="rt_key2" id="rt_key2" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey3_itemname#></th>
				<td><input type="text" name="rt_key3" id="rt_key3" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 18);"><#WLANConfig11b_WEPKey4_itemname#></th>
				<td><input type="text" name="rt_key4" id="rt_key4" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
		  </tr>

			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 8);"><#WLANConfig11b_x_Phrase_itemname#></a></th>
				<td>
				  <input type="text" name="rt_phrase_x" maxlength="64" class="input" size="32" value="" onKeyUp="return is_wlphrase('WLANConfig11b', 'rt_phrase_x', this);">
				</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
				<td><input type="text" maxlength="7" size="7" name="rt_wpa_gtk_rekey" class="input"  value="<% nvram_get_x("WLANConfig11b", "rt_wpa_gtk_rekey"); %>"></td>
			  </tr>

			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 17);"><#WLANConfig11b_TxPower_itemname#></a></th>
				<td><input type="text" maxlength="3" size="3" name="rt_TxPower" onblur="return validate_range(this, 0, 100)" class="input" onClick="openHint(0, 17);" value="<% nvram_get_x("WLANConfig11b", "rt_TxPower"); %>"></td>
			  </tr>
			
			  <tr>
				<th><#WIFIRegionCode#></th>
				<td>
					<select name="rt_country_code" class="input">
						<option value="US" <% nvram_match_x("WLANConfig11b", "rt_country_code", "US","selected"); %>>USA (channels 1-11)</option>
						<option value="GB" <% nvram_match_x("WLANConfig11b", "rt_country_code", "GB","selected"); %>>Europe (channels 1-13)</option>
						<option value="DB" <% nvram_match_x("WLANConfig11b", "rt_country_code", "DB","selected"); %>>Debug (all channels)</option>
					</select>
				</td>
			  </tr>
			
			  <tr align="right">
				<td colspan="2">
					<input type="button" class="button5" value="<#GO_5G#>" onclick="location.href='Advanced_Wireless_Content.asp';">    
					<input type="button" id="applyButton" class="button" value="<#CTL_apply#>" onclick="applyRule();"></td>
			  </tr>
			</table>
		  </td>
		</tr>
		</tbody>
	  </table>
	</td>
</form>

	<!--==============Beginning of hint content=============-->
	<td id="help_td" style="width:15px;" valign="top">
<form name="hint_form"></form>
	  <div id="helpicon" onClick="openHint(0, 0);" title="<#Help_button_default_hint#>">
		<img src="images/help.gif">
	  </div>
	  
	  <div id="hintofPM" style="display:none;">
		<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
		  <thead>
		  <tr>
			<td>
			  <div id="helpname" class="AiHintTitle"></div>
			  <a href="javascript:closeHint();"><img src="images/button-close.gif" class="closebutton" /></a>
			</td>
		  </tr>
		  </thead>
		  
		  <tbody>
		  <tr>
			<td valign="top">
			  <div id="hint_body" class="hint_body2"></div>
			  <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
			</td>
		  </tr>
		  </tbody>
		</table>
	  </div>
	</td>
	<!--==============Ending of hint content=============-->
  </tr>
</table>
<!--===================================Ending of Main Content===========================================-->

	</td>
	
	<td width="10" align="center" valign="top"></td>
  </tr>
</table>

<div id="footer"></div>
</body>
</html>
