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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,2,4);
	show_footer();
	
	enable_auto_hint(6, 5);
}

function applyRule(){
	if(document.form.udpxy_enable_x.value != 0){
		if(!validate_range(document.form.udpxy_enable_x, 1024, 65535)){
			return;
		}
	}
	
	showLoading();
	
	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "/Advanced_IPTV_Content.asp";
	document.form.next_page.value = "";
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function valid_udpxy(){
	if(document.form.udpxy_enable_x.value != 0)
		validate_range(document.form.udpxy_enable_x, 1024, 65535);
}

function valid_muliticast(){
	if(document.form.controlrate_unknown_unicast.value != 0)
		validate_range(document.form.controlrate_unknown_unicast, 0, 1024);
	if(document.form.controlrate_unknown_multicast.value != 0)
		validate_range(document.form.controlrate_unknown_multicast, 0, 1024);
	if(document.form.controlrate_multicast.value != 0)
		validate_range(document.form.controlrate_multicast, 0, 1024);
	if(document.form.controlrate_broadcast.value != 0)
		validate_range(document.form.controlrate_broadcast, 0, 1024);
}

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(6, 5);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_IPTV_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="RouterConfig;LANHostConfig;WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div><br />
<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td valign="top" >
	  <table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
		<thead>
		<tr>
		  <td><#menu5_2#> - <#menu5_2_4#></td>
		</tr>
		</thead>
		<tr>
		  <td bgcolor="#FFFFFF">Multicast and IPTV</td>
		</tr>
		<tbody>
		<tr>
		  <td bgcolor="#FFFFFF"></td>
		</tr>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
				<tr>
					<td colspan="2">IPTV Control</td>
				</tr>
				</thead>
				<tr>
					<th width="50%"><#RouterConfig_GWMulticastEnable_itemname#></th>
					<td>
						<input type="radio" value="1" name="mr_enable_x" class="input" <% nvram_match_x("RouterConfig", "mr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="mr_enable_x" class="input" <% nvram_match_x("RouterConfig", "mr_enable_x", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 6);"><#RouterConfig_IPTV_itemname#>:</a></th>
					<td>
						<input id="udpxy_enable_x" type="text" maxlength="5" class="input" size="15" name="udpxy_enable_x" value="<% nvram_get_x("LANHostConfig", "udpxy_enable_x"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 6);" onblur="valid_udpxy();"/>
					</td>
				</tr>
			</table>
		  </td>
		</tr>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
				<tr>
					<td colspan="2">Storm Control for Ethernet</td>
				</tr>
				</thead>
				<tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 7);"><#RouterConfig_GWMulticast_unknownUni_itemname#></a></th>
					<td>
						<input id="controlrate_unknown_unicast" type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_unicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_unicast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 7);" onblur="valid_muliticast();"/>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 8);"><#RouterConfig_GWMulticast_unknownMul_itemname#></a></th>
					<td>
						<input id="controlrate_unknown_multicast" type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_multicast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 8);" onblur="valid_muliticast();"/>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 9);"><#RouterConfig_GWMulticast_Multicast_itemname#></a></th>
					<td>
						<input id="controlrate_multicast" type="text" maxlength="4" class="input" size="15" name="controlrate_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_multicast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 9);" onblur="valid_muliticast();"/>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 10);"><#RouterConfig_GWMulticast_Broadcast_itemname#></a></th>
					<td>
						<input id="controlrate_broadcast" type="text" maxlength="4" class="input" size="15" name="controlrate_broadcast" value="<% nvram_get_x("LANHostConfig", "controlrate_broadcast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 10);" onblur="valid_muliticast();"/>
					</td>
				</tr>
			</table>
		  </td>
		</tr>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
				<tr>
					<td colspan="2">Multicast - WiFi 2.4GHz</td>
				</tr>
				</thead>
				<tr>
					<th width="50%">IGMP Snooping</th>
					<td>
						<input type="radio" value="1" name="rt_IgmpSnEnable" class="input" <% nvram_match_x("WLANConfig11b", "rt_IgmpSnEnable", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="rt_IgmpSnEnable" class="input" <% nvram_match_x("WLANConfig11b", "rt_IgmpSnEnable", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
					<td>
						<select name="rt_mcastrate" class="input" onClick="openHint(3, 7);">
							<option value="0" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "0", "selected"); %>>HTMIX (1S) 6.5-15 Mbps</option>
							<option value="1" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "1", "selected"); %>>HTMIX (1S) 13-30 Mbps</option>
							<option value="2" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "2", "selected"); %>>HTMIX (1S) 19.5-45 Mbps</option>
							<option value="3" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "3", "selected"); %>>HTMIX (2S) 13-30 Mbps</option>
							<option value="4" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "4", "selected"); %>>HTMIX (2S) 26-60 Mbps</option>
							<option value="5" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
							<option value="6" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
							<option value="7" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
							<option value="8" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
							<option value="9" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "9", "selected"); %>>CCK 11 Mbps</option>
						</select>
					</td>
				</tr>
			</table>
		  </td>
		</tr>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
				<tr>
					<td colspan="2">Multicast - WiFi 5GHz</td>
				</tr>
				</thead>
				<tr>
					<th width="50%">IGMP Snooping</th>
					<td>
						<input type="radio" value="1" name="wl_IgmpSnEnable" class="input" <% nvram_match_x("WLANConfig11b", "wl_IgmpSnEnable", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl_IgmpSnEnable" class="input" <% nvram_match_x("WLANConfig11b", "wl_IgmpSnEnable", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
					<td>
						<select name="wl_mcastrate" class="input" onClick="openHint(3, 7);">
							<option value="0" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "0", "selected"); %>>HTMIX (1S) 6.5-15 Mbps</option>
							<option value="1" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "1", "selected"); %>>HTMIX (1S) 13-30 Mbps</option>
							<option value="2" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "2", "selected"); %>>HTMIX (1S) 19.5-45 Mbps</option>
							<option value="3" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "3", "selected"); %>>HTMIX (2S) 13-30 Mbps</option>
							<option value="4" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "4", "selected"); %>>HTMIX (2S) 26-60 Mbps</option>
							<option value="5" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
							<option value="6" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
							<option value="7" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
							<option value="8" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
						</select>
					</td>
				</tr>
			</table>
		  </td>
		</tr>
		</tbody>
		<tr>
			<td bgcolor="#FFFFFF" align="right"><input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/></td>
		</tr>
	  </table>
	</td>
</form>

	<!--==============Beginning of hint content=============-->
	<td id="help_td" style="width:15px;"  valign="top">
	  <div id="helpicon" onClick="openHint(0, 0);" title="<#Help_button_default_hint#>">
		<img src="images/help.gif">
	  </div>
	  
	  <div id="hintofPM" style="display:none;">
<form name="hint_form"></form>
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
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

</body>
</html>
