<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title>Wireless Router <#Web_Title#> - <#menu5_4_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/wcdma_list.js"></script>
<script type="text/javascript" src="/cdma2000_list.js"></script>
<script type="text/javascript" src="/td-scdma_list.js"></script>
<script type="text/javascript" src="/wimax_list.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding", "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>]; // [[MAC, associated, authorized], ...]

var country = '<% nvram_get_x("General", "modem_country"); %>';
var isp = '<% nvram_get_x("General", "modem_isp"); %>';
var apn = '<% nvram_get_x("General", "modem_apn"); %>';
var dialnum = '<% nvram_get_x("General", "modem_dialnum"); %>';
var user = '<% nvram_get_x("General", "modem_user"); %>';
var pass = '<% nvram_get_x("General", "modem_pass"); %>';

var modemlist = new Array();
var countrylist = new Array();
var isplist = new Array();
var apnlist = new Array();
var daillist = new Array();
var userlist = new Array();
var passlist = new Array();

function initial(){
	show_banner(1);
	show_menu(5, 4, 4);
	show_footer();

	switch_modem_type();

	enable_auto_hint(21, 7);
}

function switch_modem_rule(){
	var mrule = document.form.modem_rule[0].checked;
	var mtype = document.form.modem_type.value;

	if (!mrule){
		$("row_modem_type").style.display = "none";
		$("row_modem_country").style.display = "none";
		$("row_modem_isp").style.display = "none";
		$("row_modem_arun").style.display = "none";
	}
	else {
		$("row_modem_type").style.display = "";
		$("row_modem_country").style.display = "";
		$("row_modem_isp").style.display = "";
		$("row_modem_arun").style.display = "";
	}

	if (mtype == "3" || !mrule){
		$("row_modem_ras_1").style.display = "none";
		$("row_modem_ras_2").style.display = "none";
		$("row_modem_ras_3").style.display = "none";
		$("row_modem_ras_4").style.display = "none";
		$("row_modem_ras_5").style.display = "none";
		$("row_modem_ras_6").style.display = "none";
		$("row_modem_ras_7").style.display = "none";
		$("row_modem_ras_8").style.display = "none";
	}
	else {
		if (mtype == "1") {
			$("row_modem_ras_1").style.display = "none";
			$("row_modem_ras_2").style.display = "none";
		}
		else {
			$("row_modem_ras_1").style.display = "";
			$("row_modem_ras_2").style.display = "";
		}
		
		$("row_modem_ras_3").style.display = "";
		$("row_modem_ras_4").style.display = "";
		$("row_modem_ras_5").style.display = "";
		$("row_modem_ras_6").style.display = "";
		$("row_modem_ras_7").style.display = "";
		$("row_modem_ras_8").style.display = "";
	}
}

function switch_modem_type(){
	var mtype = document.form.modem_type.value;

	switch_modem_rule();

	if (mtype == "3"){
		show_4G_country_list();
	}
	else if (mtype == "2"){
		show_tdscdma_country_list();
	}
	else if (mtype == "1"){
		show_cdma2000_country_list();
	}
	else{
		show_wcdma_country_list();
	}

	gen_list(mtype);
	show_APN_list();
}

function gen_list(mtype){
	if (mtype == "3"){
		gen_4G_list();
	}
	else if (mtype == "2"){
		gen_tdscdma_list();
	}
	else if (mtype == "1"){
		gen_cdma2000_list();
	}
	else{
		gen_wcdma_list();
	}

	free_options($("modem_isp"));
	$("modem_isp").options.length = isplist.length;

	for(var i = 0; i < isplist.length; i++){
		$("modem_isp").options[i] = new Option(isplist[i], isplist[i]);
		if(isplist[i] == isp)
			$("modem_isp").options[i].selected = "1";
	}
}

function show_APN_list(){
	if(document.form.modem_type.value != "3"){
		var ISPlist = $("modem_isp").value;
		if((ISPlist == isp) && (apn != "" || dialnum != "" || user != "" || pass != "")){
			$("modem_apn").value = apn;
			$("modem_dialnum").value = dialnum;
			$("modem_user").value = user;
			$("modem_pass").value = pass;
		}
		else{
			for(var i = 0; i < isplist.length; i++){
				if(isplist[i] == ISPlist){
					$("modem_apn").value = apnlist[i];
					$("modem_dialnum").value = daillist[i];
					$("modem_user").value = userlist[i];
					$("modem_pass").value = passlist[i];
				}
			}
		}
	}
}

function applyRule(){
	showLoading(); 

	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "/Advanced_Modem_others.asp";
	document.form.next_page.value = "";

	document.form.submit();
}


function done_validating(action){
	refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(21, 7);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log", "productid"); %>">

<input type="hidden" name="current_page" value="Advanced_Modem_others.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="General;Layer3Forwarding;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("", "firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
        <td width="23">&nbsp;</td>
        
        <!--=====Beginning of Main Menu=====-->
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
                
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
        <thead>
        <tr>
                <td><#menu5_4_4#></td>
        </tr>
        </thead>
        <tbody>
          <tr>
            <td bgcolor="#FFFFFF"><#HSDPAConfig_hsdpa_mode_itemdesc#></td>
          </tr>
        </tbody>
        <tr>
                <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		<tr>
			<th width="50%">
				<#HSDPAConfig_hsdpa_enable_itemname#>
			</th>
			<td>
				<input type="radio" value="1" name="modem_rule" class="input" onClick="switch_modem_rule();" <% nvram_match_x("General", "modem_rule", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" value="0" name="modem_rule" class="input" onClick="switch_modem_rule();" <% nvram_match_x("General", "modem_rule", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr id="row_modem_type">
			<th>
				<#ModemType#>
			</th>
			<td>
				<select name="modem_type" class="input" onchange="switch_modem_type();">
					<option value="0" <% nvram_match_x("General", "modem_type", "0", "selected"); %>>WCDMA (UMTS)</option>
					<option value="1" <% nvram_match_x("General", "modem_type", "1", "selected"); %>>CDMA2000 (EVDO)</option>
					<option value="2" <% nvram_match_x("General", "modem_type", "2", "selected"); %>>TD-SCDMA</option>
					<option value="3" <% nvram_match_x("General", "modem_type", "3", "selected"); %>>RNDIS Modems (LTE and other)</option>
				</select>
			</td>
		</tr>
		<tr id="row_modem_country">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(21,9);"><#HSDPAConfig_Country_itemname#></a></th>
			<td>
				<select name="modem_country" id="isp_countrys" class="input" onfocus="parent.showHelpofDrSurf(21,9);" onchange="gen_list(document.form.modem_type.value);show_APN_list();"></select>
			</td>
		</tr>
		<tr id="row_modem_isp">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(21,8);"><#HSDPAConfig_ISP_itemname#></a></th>
			<td>
				<select name="modem_isp" id="modem_isp" class="input" onClick="openHint(21,8);" onchange="show_APN_list()"></select>
			</td>
		</tr>
		<tr id="row_modem_ras_1">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(21,3);"><#HSDPAConfig_private_apn_itemname#></a></th>
			<td>
				<input id="modem_apn" name="modem_apn" class="input" size="32" maxlength="32" onClick="openHint(21,3);" type="text" value=""/>
			</td>
		</tr>
		<tr id="row_modem_ras_2">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(21,2);"><#HSDPAConfig_PIN_itemname#></a></th>
			<td>
				<input id="wan_3g_pin" name="wan_3g_pin" class="input" size="12" onClick="openHint(21,2);" type="password" maxLength="8" value="<% nvram_get_x("", "wan_3g_pin"); %>"/>
			</td>
		</tr>
		<tr id="row_modem_ras_3">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(21,10);"><#HSDPAConfig_DialNum_itemname#></a></th>
			<td>
				<input id="modem_dialnum" name="modem_dialnum" class="input" size="12" onClick="openHint(21,10);" type="text" value=""/>
			</td>
		</tr>
		<tr id="row_modem_ras_4">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(21,11);"><#HSDPAConfig_Username_itemname#></a></th>
			<td>
				<input id="modem_user" name="modem_user" class="input" size="12" onClick="openHint(21,11);" type="text" value="<% nvram_get_x("", "modem_user"); %>"/>
			</td>
		</tr>
		<tr id="row_modem_ras_5">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(21,12);"><#AiDisk_Password#></a></th>
			<td>
				<input id="modem_pass" name="modem_pass" class="input" size="12" onClick="openHint(21,12);" type="text" value="<% nvram_get_x("", "modem_pass"); %>"/>
			</td>
		</tr>
		<tr id="row_modem_ras_6">
			<th><#COM_User_AT#></th>
			<td>
				<input name="modem_cmd" class="input" type="text" size="32" maxlength="40" value="<% nvram_get_x("", "modem_cmd"); %>"/>
			</td>
		</tr>
		<tr id="row_modem_ras_7">
			<th>
				<#ModemZCD#>
			</th>
			<td>
				<select name="modem_zcd" class="input">
					<option value="0" <% nvram_match_x("General", "modem_zcd", "0", "selected"); %>>usb-modeswitch</option>
					<option value="1" <% nvram_match_x("General", "modem_zcd", "1", "selected"); %>>legacy eject</option>
				</select>
			</td>
		</tr>
		<tr id="row_modem_ras_8">
			<th><#COM_Port_Node#></th>
			<td>
				<select name="modem_node" class="input">
					<option value="0" <% nvram_match_x("General", "modem_node", "0", "selected"); %>>Auto</option>
					<option value="1" <% nvram_match_x("General", "modem_node", "1", "selected"); %>>ttyUSB0</option>
					<option value="2" <% nvram_match_x("General", "modem_node", "2", "selected"); %>>ttyUSB1</option>
					<option value="3" <% nvram_match_x("General", "modem_node", "3", "selected"); %>>ttyUSB2</option>
					<option value="4" <% nvram_match_x("General", "modem_node", "4", "selected"); %>>ttyUSB3</option>
					<option value="5" <% nvram_match_x("General", "modem_node", "5", "selected"); %>>ttyUSB4</option>
					<option value="6" <% nvram_match_x("General", "modem_node", "6", "selected"); %>>ttyUSB5</option>
					<option value="7" <% nvram_match_x("General", "modem_node", "7", "selected"); %>>ttyUSB6</option>
					<option value="8" <% nvram_match_x("General", "modem_node", "8", "selected"); %>>ttyUSB7</option>
				</select>
			</td>
		</tr>
		<tr id="row_modem_arun">
			<th><#ModemARun#></th>
			<td>
				<select name="modem_arun" class="input">
					<option value="0" <% nvram_match_x("General", "modem_arun", "0", "selected"); %>><#checkbox_No#></option>
					<option value="1" <% nvram_match_x("General", "modem_arun", "1", "selected"); %>><#checkbox_Yes#></option>
					<option value="2" <% nvram_match_x("General", "modem_arun", "2", "selected"); %>><#ModemARunItem2#></option>
				</select>
			</td>
		</tr>
		<tr align="right">
			<td colspan="2">
				<input type="button" class="button" value="<#CTL_apply#>" onclick="applyRule();">
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
                                                <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
                                                <div id="hintofPM" style="display:none;">
                                                        <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
                                                                <thead>
                                                                <tr>
                                                                        <td>
                                                                                <div id="helpname" class="AiHintTitle"></div>
                                                                                <a href="javascript:closeHint();">
                                                                                        <img src="images/button-close.gif" class="closebutton">
                                                                                </a>
                                                                        </td>
                                                                </tr>
                                                                </thead>
                                                                <tr>
                                                                        <td valign="top">
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
                </td>
    <td width="10" align="center" valign="top">&nbsp;</td>
        </tr>
</table>

<div id="footer"></div>
</body>
</html>
