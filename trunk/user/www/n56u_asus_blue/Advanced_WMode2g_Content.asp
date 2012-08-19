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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_1_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<style>
#pull_arrow{
	margin-left: -2px;
	margin-top: 2px;
 	float:left;
 	cursor:pointer;
 	border:2px outset #EFEFEF;
 	background-color:#CCC;
 	padding:3px 2px 4px 0px;
}
#WDSAPList{
	margin-left: 2px;
	margin-left: -150px \9;
	margin-top: 22px;
	border:2px outset #999;
	background-color:#EFEFEF;
	position:absolute;
	width:auto;
	text-align:left;
	height:auto;
	overflow-y:auto;
	padding: 1px;
	display:none;
}
#WDSAPList div{
	height:20px;
	line-height:20px;
	text-decoration:none;
	padding-left:2px;
}

#WDSAPList a{
	background-color:#EFEFEF;
	color:#000;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
#WDSAPList div:hover, #ClientList_Block a:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}
</style>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help_2g.js"></script>
<script language="JavaScript" type="text/javascript" src="/general_2g.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script language="JavaScript" type="text/JavaScript" src="/jquery.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var wds_aplist = [["", "", ""]];
var $j = jQuery.noConflict();

function initial(){
	show_banner(1);
	show_menu(5,1,3);
	
	show_footer();
	enable_auto_hint(1, 3);
	load_body();
	showLANIPList();
	
	change_wireless_bridge(0);
	change_sta_auth_mode(0);
	
	document.form.rt_channel.value = document.form.rt_channel_org.value;
	document.form.rt_sta_ssid.value = decodeURIComponent(document.form.rt_sta_ssid_org.value);
	document.form.rt_sta_wpa_psk.value = decodeURIComponent(document.form.rt_sta_wpa_psk_org.value);
}

function applyRule(){
	var m = document.form.rt_mode_x.value;
	
	if(validForm()){
		showLoading();
		if (m == "1" || m == "2")
			document.form.action_mode.value = " Restart ";
		else
			document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WMode2g_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	var m = document.form.rt_mode_x.value;
	
	if (m == "3" || m == "4") {
		if(!validate_string_ssid(document.form.rt_sta_ssid))
			return false;
		
		if(document.form.rt_sta_ssid.value == "") {
			document.form.rt_sta_ssid.focus();
			return false;
		}
		
		if(document.form.rt_sta_auth_mode.value == "psk"){
			if(!validate_psk(document.form.rt_sta_wpa_psk))
				return false;
		}
	}
	return true;
}


function done_validating(action){
	refreshpage();
}

function wds_scan(){
	$j.ajax({
		url: '/wds_aplist_2g.asp',
		dataType: 'script',
		
		error: function(xhr){
			setTimeout("wds_scan();", 1000);
		},
		success: function(response){
			showLANIPList();
		}
	});
}

function change_wireless_bridge(mflag) {
	var m = document.form.rt_mode_x.value;
	
	if (m != "0")
	{
		if (mflag == 1 && document.form.rt_channel.value == "0")
		{
			alert("<#JS_fixchannel#>");
			document.form.rt_channel.options[0].selected = 0;
			document.form.rt_channel.options[1].selected = 1;
		}
	}
	
	if (m == "0") // AP only
	{
		inputRCtrl2(document.form.rt_wdsapply_x, 1);
		inputRCtrl1(document.form.rt_wdsapply_x, 0);
		
		$("row_wds_1").style.display = "none";
		$("row_wds_2").style.display = "none";
		$("row_wds_apc").style.display = "none";
		
		$("row_apc_1").style.display = "none";
		$("row_apc_2").style.display = "none";
		$("row_apc_3").style.display = "none";
	}
	else if (m == "1") // WDS only
	{
		inputRCtrl2(document.form.rt_wdsapply_x, 0);
		inputRCtrl1(document.form.rt_wdsapply_x, 0);
		
		$("row_wds_1").style.display = "";
		$("row_wds_2").style.display = "";
		$("row_wds_apc").style.display = "";
		
		$("ctl_wds_1").style.display = "";
		$("ctl_wds_2").style.display = "";
		$("ctl_wds_3").style.display = "";
		$("ctl_apc_1").style.display = "none";
		$("ctl_apc_2").style.display = "none";
		
		$("row_apc_1").style.display = "none";
		$("row_apc_2").style.display = "none";
		$("row_apc_3").style.display = "none";
	}
	else if (m == "2") // AP & WDS
	{
		$("row_wds_1").style.display = "";
		
		if (document.form.rt_wdsapply_x_org.value == "0")
		{
			inputRCtrl2(document.form.rt_wdsapply_x, 1);
			$("row_wds_2").style.display = "none";
			$("row_wds_apc").style.display = "none";
		}
		else
		{
			inputRCtrl2(document.form.rt_wdsapply_x, 0);
			$("row_wds_2").style.display = "";
			$("row_wds_apc").style.display = "";
		}
		
		inputRCtrl1(document.form.rt_wdsapply_x, 1);
		
		$("ctl_wds_1").style.display = "";
		$("ctl_wds_2").style.display = "";
		$("ctl_wds_3").style.display = "";
		$("ctl_apc_1").style.display = "none";
		$("ctl_apc_2").style.display = "none";
		
		$("row_apc_1").style.display = "none";
		$("row_apc_2").style.display = "none";
		$("row_apc_3").style.display = "none";
	}
	else if (m == "3") // AP-Client only
	{
		$("row_wds_1").style.display = "none";
		$("row_wds_2").style.display = "none";
		$("row_wds_apc").style.display = "";
		
		$("ctl_wds_1").style.display = "none";
		$("ctl_wds_2").style.display = "none";
		$("ctl_wds_3").style.display = "none";
		$("ctl_apc_1").style.display = "";
		$("ctl_apc_2").style.display = "";
		
		$("row_apc_1").style.display = "";
		$("row_apc_2").style.display = "";
		$("row_apc_3").style.display = "";
	}
	else if (m == "4") // AP & AP-Client
	{
		$("row_wds_1").style.display = "none";
		$("row_wds_2").style.display = "none";
		$("row_wds_apc").style.display = "";
		
		$("ctl_wds_1").style.display = "none";
		$("ctl_wds_2").style.display = "none";
		$("ctl_wds_3").style.display = "none";
		$("ctl_apc_1").style.display = "";
		$("ctl_apc_2").style.display = "";
		
		$("row_apc_1").style.display = "";
		$("row_apc_2").style.display = "";
		$("row_apc_3").style.display = "";
	}
}

function change_wdsapply(){
	var m = document.form.rt_mode_x.value;
	var a = rcheck(document.form.rt_wdsapply_x);
	
	if (m == "2") // AP & WDS
	{
		if (a == "0")
		{
			$("row_wds_2").style.display = "none";
			$("row_wds_apc").style.display = "none";
		}
		else
		{
			$("row_wds_2").style.display = "";
			$("row_wds_apc").style.display = "";
		}
	}
}


function change_sta_auth_mode(mflag){
	var mode = document.form.rt_sta_auth_mode.value;
	var opts = document.form.rt_sta_auth_mode.options;
	
	if(mode == "psk")
	{
		inputCtrl(document.form.rt_sta_crypto, 1);
		inputCtrl(document.form.rt_sta_wpa_psk, 1);
		
		if(opts[opts.selectedIndex].text == "WPA2-Personal")
		{
			if (mflag == 1) {
				document.form.rt_sta_crypto.options[0].selected = 0;
				document.form.rt_sta_crypto.options[1].selected = 1;
				document.form.rt_sta_wpa_mode.value = "2";
			}
		}
		else
		{
			if (mflag == 1) {
				document.form.rt_sta_crypto.options[1].selected = 0;
				document.form.rt_sta_crypto.options[0].selected = 1;
				document.form.rt_sta_wpa_mode.value = "1";
			}
		}
	}
	else
	{
		inputCtrl(document.form.rt_sta_crypto, 0);
		inputCtrl(document.form.rt_sta_wpa_psk, 0);
	}
}

/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setClientIP(num){
	var smac = wds_aplist[num][1].split(":");
	var mode = document.form.rt_mode_x.value;
	if (mode == "1" || mode == "2")
		document.form.rt_wdslist_x_0.value = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];
	else if (mode == "3" || mode == "4")
		document.form.rt_sta_ssid.value = wds_aplist[num][0];
	hideClients_Block();
	over_var = 0;
}

function rescan(){
	wds_aplist = "";
	showLANIPList();
	wds_scan();
}

function showLANIPList(){
	var code = "";
	var show_name = "";

	if(wds_aplist != ""){
		for(var i = 0; i < wds_aplist.length ; i++){
			wds_aplist[i][0] = decodeURIComponent(wds_aplist[i][0]);
			if(wds_aplist[i][0] && wds_aplist[i][0].length > 12)
				show_name = wds_aplist[i][0].substring(0, 10) + "..";
			else
				show_name = wds_aplist[i][0];
			
			if(wds_aplist[i][1] && wds_aplist[i][1].length > 0){
				code += '<a href="#"><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP('+i+');"><strong>'+show_name+'</strong>';
				code += ' ['+wds_aplist[i][1]+']';
				code += ', '+wds_aplist[i][2]+'%';
				code += ' </div></a>';
			}
		}
		code += '<div style="text-decoration:underline;font-weight:bolder;cursor:pointer;" onclick="rescan();"><#AP_survey#>&nbsp;</div>';
	}
	else{
		code += '<div style="width:98px"><img style="margin-left:40px;margin-top:2px;" src="/images/load.gif"></div>';
	}

	code +='<!--[if lte IE 6.5]><iframe class="hackiframe_wdssurvey"></iframe><![endif]-->';	
	document.getElementById("WDSAPList").innerHTML = code;
}

function pullLANIPList(obj){
	
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		document.getElementById("WDSAPList").style.display = 'block';		
		document.form.rt_wdslist_x_0.focus();		
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}
var over_var = 0;
var isMenuopen = 0;

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('WDSAPList').style.display='none';
	isMenuopen = 0;
}
/*----------} Mouse event of fake LAN IP select menu-----------------*/
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(1, 3);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" value="<% nvram_get_f("general.log","productid"); %>" name="productid" >
<input type="hidden" value="<% nvram_get_x("IPConnection","wan_route_x"); %>" name="wan_route_x" >
<input type="hidden" value="<% nvram_get_x("IPConnection","wan_nat_x"); %>" name="wan_nat_x" >
<input type="hidden" name="current_page" value="Advanced_WMode2g_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANConfig11b;">
<input type="hidden" name="group_id" value="rt_RBRList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="rt_country_code" value="<% nvram_get_x("","rt_country_code"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="rt_HT_BW" value="<% nvram_get_x("",  "rt_HT_BW"); %>">
<input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get_x("Regulatory","x_RegulatoryDomain"); %>" readonly="1">
<input type="hidden" name="rt_wdsnum_x_0" value="<% nvram_get_x("WLANConfig11b", "rt_wdsnum_x"); %>" readonly="1">
<input type="hidden" name="rt_channel_org" value="<% nvram_get_x("WLANConfig11b","rt_channel"); %>">
<input type="hidden" name="rt_wdsapply_x_org" value="<% nvram_get_x("WLANConfig11b","rt_wdsapply_x"); %>">
<input type="hidden" name="rt_sta_ssid_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_sta_ssid"); %>">
<input type="hidden" name="rt_sta_wpa_mode" value="<% nvram_get_x("WLANConfig11b","rt_sta_wpa_mode"); %>">
<input type="hidden" name="rt_sta_wpa_psk_org" value="<% nvram_char_to_ascii("WLANConfig11b", "rt_sta_wpa_psk"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>
		
		<td valign="top" width="202">				
		<div id="mainMenu"></div>	
		<div id="subMenu"></div>		
		</td>				
		
	<td valign="top">
		<div id="tabMenu" class="submenuBlock"></div>
		<br />
		<!--===================================Beginning of Main Content===========================================-->
	<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >
		
	<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
	<thead>
	<tr>
		<td><#menu5_1#> - <#menu5_1_3#> (2.4GHz)</td>
	</tr>
	</thead>	
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#WLANConfig11b_display3_sectiondesc#>
			<ul>
				<li><#WLANConfig11b_display3_sectiondesc2#></li>
				<li><#WLANConfig11b_display3_sectiondesc3#></li>
			</ul>
		</td>
	</tr>
	</tbody>
	<tr>
	  <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		<tr>
			<th align="right"><a class="hintstyle" href="javascript:void(0);"  onClick="openHint(1,1);"><#WLANConfig11b_x_APMode_itemname#></a></th>
			<td width="60%">
				<select name="rt_mode_x" class="input" onChange="change_wireless_bridge(1);">
					<option value="0" <% nvram_match_x("WLANConfig11b","rt_mode_x", "0","selected"); %>>AP Only</option>
					<option value="1" <% nvram_match_x("WLANConfig11b","rt_mode_x", "1","selected"); %>>WDS Only</option>
					<option value="2" <% nvram_match_x("WLANConfig11b","rt_mode_x", "2","selected"); %>>AP & WDS</option>
					<option value="3" <% nvram_match_x("WLANConfig11b","rt_mode_x", "3","selected"); %>>AP-Client Only</option>
					<option value="4" <% nvram_match_x("WLANConfig11b","rt_mode_x", "4","selected"); %>>AP & AP-Client</option>
				</select>
			</td>
		</tr>
		<tr>
			<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(1,2);"><#WLANConfig11b_Channel_itemname#></a></th>
			<td>
				<select name="rt_channel" class="input" onChange="return change_common(this, 'WLANConfig11b', 'rt_channel')">
					<% select_channel("WLANConfig11b"); %>
				</select>
			</td>
		</tr>
		<tr id="row_wds_1" style="display:none;">
			<th align="right"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(1,3);"><#WLANConfig11b_x_BRApply_itemname#></a></th>
			<td>
				<input type="radio" value="1" name="rt_wdsapply_x" class="input" onClick="change_wdsapply();" <% nvram_match_x("WLANConfig11b","rt_wdsapply_x", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="rt_wdsapply_x" class="input" onClick="change_wdsapply();" <% nvram_match_x("WLANConfig11b","rt_wdsapply_x", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr id="row_wds_apc" style="display:none;">
			<th align="right"><a id="ctl_apc_1" class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 1);">STA SSID:</a></th>
			<td>
				<input type="text" id="ctl_wds_1" name="rt_wdslist_x_0" maxlength="12" class="input" size="20" onKeyPress="return is_hwaddr();" style="float:left;"/>
				<input type="text" id="ctl_apc_2" name="rt_sta_ssid" value="" maxlength="32" class="input" size="20" onKeyPress="return is_string(this);" style="float:left;"/>
				<div id="WDSAPList" class="WDSAPList"></div>
				<img id="pull_arrow" src="/images/arrow-down.gif" onclick="pullLANIPList(this);" title="Select the Access Point" onmouseover="over_var=1;" onmouseout="over_var=0;">
				<input class="button" id="ctl_wds_2" style="margin-top:3px;" type="submit" onClick="return markGroup(this, 'rt_RBRList', 2, ' Add ');" name="rt_RBRList" value="<#CTL_add#>" size="12"/>
				<br/><span id="ctl_wds_3" style="float:left;">* <#JS_validmac#></span>
			</td>
		</tr>
		<tr id="row_wds_2" style="display:none;">
			<th align="right"><#WLANConfig11b_RBRList_groupitemdesc#></th>
			<td>
				<select size="4" name="rt_RBRList_s" multiple="true" class="input" style="vertical-align:middle; width:150px;">
					<% nvram_get_table_x("WLANConfig11b","rt_RBRList"); %>
				</select>
				<input class="button" type="submit" onClick="return markGroup(this, 'rt_RBRList', 2, ' Del ');" name="rt_RBRList2" value="<#CTL_del#>" size="12">
			</td>
		</tr>
		<tr id="row_apc_1" style="display:none;">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
			<td>
				<select name="rt_sta_auth_mode" class="input" onChange="change_sta_auth_mode(1);">
					<option value="open" <% nvram_match_x("WLANConfig11b", "rt_sta_auth_mode", "open", "selected"); %>>Open System</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_sta_auth_mode", "psk", "WLANConfig11b", "rt_sta_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
					<option value="psk" <% nvram_double_match_x("WLANConfig11b", "rt_sta_auth_mode", "psk", "WLANConfig11b", "rt_sta_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
				</select>
			</td>
		</tr>
		<tr id="row_apc_2" style="display:none;">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
			<td>
				<select name="rt_sta_crypto" class="input">
					<option value="tkip" <% nvram_match_x("WLANConfig11b", "rt_sta_crypto", "tkip", "selected"); %>>TKIP</option>
					<option value="aes" <% nvram_match_x("WLANConfig11b", "rt_sta_crypto", "aes", "selected"); %>>AES</option>
				</select>
			</td>
		</tr>
		<tr id="row_apc_3" style="display:none;">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
			<td>
				<input type="text" name="rt_sta_wpa_psk" maxlength="64" class="input" size="34" value=""/>
			</td>
		</tr>
		<tr align="right">
			<td colspan="2">
				<input type="button" class="button5" value="<#GO_5G#>" onclick="location.href='Advanced_WMode_Content.asp';"/>
				<input type="button" class="button" onclick="applyRule()" value="<#CTL_apply#>" />
			</td>
		</tr>
		</table>
	</td>
	</tr>
</table>
</td>
</form>
		
	<!--==============Beginning of hint content=============-->
	<td id="help_td" style="width:15px;" valign="top">
<form name="hint_form"></form>
	  <div id="helpicon" onClick="openHint(0,0);">
	  	<img src="images/help.gif" />
	  </div>
	  
	  <div id="hintofPM" style="display:none;">
	    <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
		  <thead>
		  <tr>
			<td>
			  <div id="helpname" class="AiHintTitle"></div>
			  <a href="javascript:;" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
			</td>
		  </tr>
		  </thead>
		  
		  <tr>				
			<td valign="top" >
			  <div class="hint_body2" id="hint_body"></div>
			  <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
			</td>
		  </tr>
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
