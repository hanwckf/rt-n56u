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

<title>ASUS Wireless Router <#Web_Title#> - 2.4G <#menu5_1_2#></title>
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

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,1,2);
	show_footer();
	
	enable_auto_hint(0, 21);
	
	load_body();
	
	document.form.rt_guest_ssid.value = decodeURIComponent(document.form.rt_guest_ssid_org.value);
	document.form.rt_guest_wpa_psk.value = decodeURIComponent(document.form.rt_guest_wpa_psk_org.value);
	
	change_guest_enabled(0);
	change_guest_auth_mode(0);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WGuest2g_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	var a = rcheck(document.form.rt_guest_enable);
	var mode = document.form.rt_guest_auth_mode.value;
	
	if (a != "0")
	{
		if(!validate_string_ssid(document.form.rt_guest_ssid))
			return false;
		
		if(document.form.rt_guest_ssid.value == "") {
			document.form.rt_guest_ssid.focus();
			return false;
		}
		
		if(mode == "psk"){
			if(!validate_psk(document.form.rt_guest_wpa_psk))
				return false;
		}
	}
	return true;
}

function done_validating(action){
	refreshpage();
}

function change_guest_enabled(mflag) {
	var a = rcheck(document.form.rt_guest_enable);
	
	if (a == "0")
	{
		$("row_guest_1").style.display = "none";
		$("row_guest_2").style.display = "none";
		$("row_guest_3").style.display = "none";
		$("row_guest_4").style.display = "none";
		$("row_guest_5").style.display = "none";
		$("row_guest_6").style.display = "none";
		$("row_guest_7").style.display = "none";
	}
	else
	{
		$("row_guest_1").style.display = "";
		$("row_guest_2").style.display = "";
		$("row_guest_3").style.display = "";
		$("row_guest_4").style.display = "";
		$("row_guest_5").style.display = "";
		$("row_guest_6").style.display = "";
		$("row_guest_7").style.display = "";
	}
}

function change_guest_auth_mode(mflag) {
	var mode = document.form.rt_guest_auth_mode.value;
	var opts = document.form.rt_guest_auth_mode.options;
	
	if (mode == "psk")
	{
		inputCtrl(document.form.rt_guest_crypto, 1);
		inputCtrl(document.form.rt_guest_wpa_psk, 1);
		
		if(opts[opts.selectedIndex].text == "WPA2-Personal")
		{
			if (mflag == 1) {
				document.form.rt_guest_crypto.options[2].selected = 0;
				document.form.rt_guest_crypto.options[0].selected = 0;
				document.form.rt_guest_crypto.options[1].selected = 1;
				document.form.rt_guest_wpa_mode.value = "2";
			}
		}
		else if(opts[opts.selectedIndex].text == "WPA-Personal")
		{
			if (mflag == 1) {
				document.form.rt_guest_crypto.options[2].selected = 0;
				document.form.rt_guest_crypto.options[1].selected = 0;
				document.form.rt_guest_crypto.options[0].selected = 1;
				document.form.rt_guest_wpa_mode.value = "1";
			}
		}
		else
		{
			if (mflag == 1) {
				document.form.rt_guest_crypto.options[1].selected = 0;
				document.form.rt_guest_crypto.options[0].selected = 0;
				document.form.rt_guest_crypto.options[2].selected = 1;
				document.form.rt_guest_wpa_mode.value = "0";
			}
		}
	}
	else
	{
		inputCtrl(document.form.rt_guest_crypto, 0);
		inputCtrl(document.form.rt_guest_wpa_psk, 0);
	}
}


</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(0, 11);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="wan_route_x" value="<% nvram_get_x("IPConnection","wan_route_x"); %>">
<input type="hidden" name="wan_nat_x" value="<% nvram_get_x("IPConnection","wan_nat_x"); %>">

<input type="hidden" name="current_page" value="Advanced_WGuest2g_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="rt_country_code" value="<% nvram_get_x("","rt_country_code"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="rt_guest_ssid_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_guest_ssid"); %>">
<input type="hidden" name="rt_guest_wpa_mode" value="<% nvram_get_x("WLANConfig11b","rt_guest_wpa_mode"); %>">
<input type="hidden" name="rt_guest_wpa_psk_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_guest_wpa_psk"); %>">

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
		  <td><#menu5_1#> - <#menu5_1_2#> (2.4GHz)</td>
		</tr>
		</thead>
		<tbody>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" id="WLgeneral">
			<tr>
				<th width="50%"><#WIFIGuestEnable#></th>
				<td>
					<input type="radio" value="1" name="rt_guest_enable" class="input" onClick="change_guest_enabled(1);" <% nvram_match_x("WLANConfig11b","rt_guest_enable", "1", "checked"); %>/><#checkbox_Yes#>
					<input type="radio" value="0" name="rt_guest_enable" class="input" onClick="change_guest_enabled(1);" <% nvram_match_x("WLANConfig11b","rt_guest_enable", "0", "checked"); %>/><#checkbox_No#>
				</td>
			</tr>
			<tr id="row_guest_1" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);"><#WIFIGuestSSID#></a></th>
				<td><input type="text" maxlength="32" class="input" size="32" name="rt_guest_ssid" value="" onkeypress="return is_string(this)"/></td>
			</tr>
			<tr id="row_guest_2" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
				<td>
					<input type="radio" value="1" name="rt_guest_closed" class="input" <% nvram_match_x("WLANConfig11b","rt_guest_closed", "1", "checked"); %>/><#checkbox_Yes#>
					<input type="radio" value="0" name="rt_guest_closed" class="input" <% nvram_match_x("WLANConfig11b","rt_guest_closed", "0", "checked"); %>/><#checkbox_No#>
				</td>
			</tr>
			<tr id="row_guest_3" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 5);"><#WLANConfig11b_x_IsolateAP_itemname#></a></th>
				<td>
					<input type="radio" value="1" name="rt_guest_ap_isolate" class="input" <% nvram_match_x("WLANConfig11b","rt_guest_ap_isolate", "1", "checked"); %>/><#checkbox_Yes#>
					<input type="radio" value="0" name="rt_guest_ap_isolate" class="input" <% nvram_match_x("WLANConfig11b","rt_guest_ap_isolate", "0", "checked"); %>/><#checkbox_No#>
				</td>
			</tr>
			<tr id="row_guest_4" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
				<td>
				  <select name="rt_guest_auth_mode" class="input" onChange="change_guest_auth_mode(1);">
					<option value="open" <% nvram_match_x("WLANConfig11b", "rt_guest_auth_mode", "open", "selected"); %>>Open System</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_guest_auth_mode", "psk", "WLANConfig11b", "rt_guest_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_guest_auth_mode", "psk", "WLANConfig11b", "rt_guest_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_guest_auth_mode", "psk", "WLANConfig11b", "rt_guest_wpa_mode", "0", "selected"); %>>WPA-Auto-Personal</option>
				  </select>
				</td>
			</tr>
			<tr id="row_guest_5" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
				<td>
				  <select name="rt_guest_crypto" class="input">
					<option value="tkip" <% nvram_match_x("WLANConfig11b", "rt_guest_crypto", "tkip", "selected"); %>>TKIP</option>
					<option value="aes" <% nvram_match_x("WLANConfig11b", "rt_guest_crypto", "aes", "selected"); %>>AES</option>
					<option value="tkip+aes" <% nvram_match_x("WLANConfig11b", "rt_guest_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
				  </select>
				</td>
			</tr>
			<tr id="row_guest_6" style="display:none;">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
				<td>
				  <input type="text" name="rt_guest_wpa_psk" maxlength="64" class="input" size="32" value=""/>
				</td>
			</tr>
			<tr id="row_guest_7" style="display:none;">
				<th><#WIFIGuestMAC#></th>
				<td>
					<input type="radio" value="1" name="rt_guest_macrule" class="input" <% nvram_match_x("WLANConfig11b","rt_guest_macrule", "1", "checked"); %>/><#checkbox_Yes#>
					<input type="radio" value="0" name="rt_guest_macrule" class="input" <% nvram_match_x("WLANConfig11b","rt_guest_macrule", "0", "checked"); %>/><#checkbox_No#>
				</td>
			</tr>
			<tr align="right">
				<td colspan="2">
					<input type="button" class="button5" value="<#GO_5G#>" onclick="location.href='Advanced_WGuest_Content.asp';">
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
