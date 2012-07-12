<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title>Wireless Router <#Web_Title#> - <#menu5_4_4#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
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
var $j = jQuery.noConflict();

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding", "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>]; // [[MAC, associated, authorized], ...]

var modem = '<% nvram_get_x("General", "Dev3G"); %>';
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
	show_menu(5, 5, 4);
	show_footer();

	switch_modem_mode(document.form.modem_enable.value);
	gen_list(document.form.modem_enable.value);
	show_ISP_list();
	show_APN_list();
	
	enable_auto_hint(21, 7);
}

function show_modem_list(mode){
	if(mode == "4")
		show_4G_modem_list();
	else
		show_3G_modem_list();
}

function show_3G_modem_list(){
	modemlist = new Array(
			"AUTO"
			, "ASUS-T500"
			, "BandLuxe-C120"
			, "BandLuxe-C170"
			, "BandLuxe-C339"
			, "Huawei-E1550"
			, "Huawei-E160G"
			, "Huawei-E161"
			, "Huawei-E169"
			, "Huawei-E176"
			, "Huawei-E220"
			, "Huawei-K3520"
			, "Huawei-ET128"
			, "Huawei-E1800"
			, "Huawei-K4505"
			, "Huawei-E172"
			, "Huawei-E372"
			, "Huawei-E122"
			, "Huawei-EC306"
			, "Huawei-E160E"
			, "Huawei-E1552"
			, "Huawei-E173"
			, "Huawei-E1823"
			, "Huawei-E1762"
			, "Huawei-E4505"
			, "Huawei-E1750C"
			, "Huawei-E1752Cu"
			//, "MU-Q101"
			, "Sierra-U598"
			, "Alcatel-X200"
			, "Alcatel-Oune-touch-X220S"
			, "AnyData-ADU-510A"
			, "AnyData-ADU-500A"
			, "Onda-MT833UP"
			, "Onda-MW833UP"
			, "ZTE-AC5710"
			, "ZTE-MU351"
			, "ZTE-MF100"
			, "ZTE-MF636"
			, "ZTE-MF622"
			, "ZTE-MF626"
			, "ZTE-MF632"
			, "ZTE-MF112"
			, "ZTE-MF180"
			, "ZTE-MFK3570-Z"
			, "CS15"
			, "CS17"
			, "ICON401"
			);

	free_options($("shown_modems"));
	for(var i = 0; i < modemlist.length; i++){
		$("shown_modems").options[i] = new Option(modemlist[i], modemlist[i]);
		if(modemlist[i] == modem)
			$("shown_modems").options[i].selected = "1";
	}
}

function show_4G_modem_list(){
	modemlist = new Array(
			"AUTO"
			, "Yota One LU150"
			);

	free_options($("shown_modems"));
	for(var i = 0; i < modemlist.length; i++){
		$("shown_modems").options[i] = new Option(modemlist[i], modemlist[i]);
		if(modemlist[i] == modem)
			$("shown_modems").options[i].selected = "1";
	}
}

function switch_modem_mode(mode){

	show_modem_list(mode);
	
	if (mode == "4")
	{
		$("ras_mode_row1").style.display = "none";
		$("ras_mode_row2").style.display = "none";
		$("ras_mode_row3").style.display = "none";
		$("ras_mode_row4").style.display = "none";
		$("ras_mode_row5").style.display = "none";
		$("ras_mode_row6").style.display = "none";
		$("ras_mode_row7").style.display = "none";
	}
	else
	{
		$("ras_mode_row1").style.display = "";
		$("ras_mode_row2").style.display = "";
		$("ras_mode_row3").style.display = "";
		$("ras_mode_row4").style.display = "";
		$("ras_mode_row5").style.display = "";
		$("ras_mode_row6").style.display = "";
		$("ras_mode_row7").style.display = "";
	}
	
	if (mode == "1" || mode == "2" || mode == "3")
	{
		document.form.Dev3G.disabled = false;
		document.form.modem_country.disabled = false;
		document.form.modem_isp.disabled = false;
		document.form.modem_apn.disabled = false;
		document.form.wan_3g_pin.disabled = false;
		document.form.modem_dialnum.disabled = false;
		document.form.modem_user.disabled = false;
		document.form.modem_pass.disabled = false;
		document.form.modem_node.disabled = false;
	}
	else if (mode == "4")
	{
		document.form.Dev3G.disabled = false;
		document.form.modem_country.disabled = false;
		document.form.modem_isp.disabled = false;
		document.form.modem_apn.disabled = true;
		document.form.wan_3g_pin.disabled = true;
		document.form.modem_dialnum.disabled = true;
		document.form.modem_user.disabled = true;
		document.form.modem_pass.disabled = true;
		document.form.modem_node.disabled = true;
	}
	else
	{
		document.form.Dev3G.disabled = true;
		document.form.modem_country.disabled = true;
		document.form.modem_isp.disabled = true;
		document.form.modem_apn.disabled = true;
		document.form.wan_3g_pin.disabled = true;
		document.form.modem_dialnum.disabled = true;
		document.form.modem_user.disabled = true;
		document.form.modem_pass.disabled = true;
		document.form.modem_node.disabled = true;
	}

	gen_country_list(mode);
}

function gen_country_list(mode){
	if(mode == "1"){
		show_wcdma_country_list();
	}
	else if(mode == "2"){
		show_cdma2000_country_list();
	}
	else if(mode == "3"){
		show_tdscdma_country_list();
	}
	else if(mode == "4"){
		show_4G_country_list();
	}
}

function gen_list(mode){
	if(mode == "1"){
		gen_wcdma_list();
	}
	else if(mode == "2"){
		gen_cdma2000_list();
	}
	else if(mode == "3"){
		gen_tdscdma_list();
	}
	else if(mode == "4"){
		gen_4G_list();
	}
}

function show_ISP_list(){
	free_options($("modem_isp"));
	$("modem_isp").options.length = isplist.length;

	for(var i = 0; i < isplist.length; i++){
		$("modem_isp").options[i] = new Option(isplist[i], isplist[i]);
		if(isplist[i] == isp)
			$("modem_isp").options[i].selected = "1";
	}
}

function show_APN_list(){
	var ISPlist = $("modem_isp").value;

	if(document.form.modem_enable.value == "1"
			|| document.form.modem_enable.value == "2"
			|| document.form.modem_enable.value == "3"
			){
		if(ISPlist == isp
				&& (apn != "" || dialnum != "" || user != "" || pass != "")){
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
	else if(document.form.modem_enable.value == "4"){
		$("modem_apn").value = "";
		$("modem_dialnum").value = "";

		if(ISPlist == isp
				&& (user != "" || pass != "")){
			$("modem_user").value = user;
			$("modem_pass").value = pass;
		}
		else{
			for(var i = 0; i < isplist.length; i++){
				if(isplist[i] == ISPlist){
					$("modem_user").value = userlist[i];
					$("modem_pass").value = passlist[i];
				}
			}
		}
	}
}

function applyRule(){
	var mode = document.form.modem_enable.value;

	if(document.form.modem_enable.value != "0"){
			showLoading(); 

			document.form.action_mode.value = " Apply ";
			document.form.current_page.value = "/index.asp";
			document.form.next_page.value = "";

			document.form.submit();
	}
	else{
		showLoading(); 

		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Modem_others.asp";
		document.form.next_page.value = "";

		document.form.submit();
	}
}

function done_validating(action){
	refreshpage();
}
</script>
<style>
    .nav-tabs > li > a {
          padding-right: 6px;
          padding-left: 6px;
    }
</style>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(21, 7);return unload_body();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

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
    <input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("", "firmver"); %>">

    <div class="container-fluid">
        <div class="row-fluid">
            <div class="span3">
                <!--Sidebar content-->
                <!--=====Beginning of Main Menu=====-->
                <div class="well sidebar-nav side_nav" style="padding: 0px;">
                    <ul id="mainMenu" class="clearfix"></ul>
                    <ul class="clearfix">
                        <li>
                            <div id="subMenu" class="accordion"></div>
                        </li>
                    </ul>
                </div>
            </div>

            <div class="span9">
                <!--Body content-->
                <div class="row-fluid">
                    <div class="span12">
                        <div class="box well grad_colour_dark_blue">
                            <h2 class="box_head round_top"><#menu5_4#> - <#menu5_4_4#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#HSDPAConfig_hsdpa_mode_itemdesc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%">
                                                <a class="help_tooltip" onmouseover="openTooltip(this,21,1);"><#HSDPAConfig_hsdpa_enable_itemname#></a>
                                            </th>
                                            <td>
                                                <select name="modem_enable" class="input" onchange="switch_modem_mode(this.value);gen_list(this.value);show_ISP_list();show_APN_list();">
                                                    <option value="0" <% nvram_match_x("General", "modem_enable", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                                    <option value="1" <% nvram_match_x("General", "modem_enable", "1", "selected"); %>>WCDMA (UMTS)</option>
                                                    <option value="2" <% nvram_match_x("General", "modem_enable", "2", "selected"); %>>CDMA2000 (EVDO)</option>
                                                    <option value="3" <% nvram_match_x("General", "modem_enable", "3", "selected"); %>>TD-SCDMA</option>
                                                    <option value="4" <% nvram_match_x("General", "modem_enable", "4", "selected"); %>>RNDIS Modems (LTE and other)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,9);"><#HSDPAConfig_Country_itemname#></a></th>
                                            <td>
                                                <select name="modem_country" id="isp_countrys" class="input" onfocus="parent.showHelpofDrSurf(21,9);" onchange="gen_list(document.form.modem_enable.value);show_ISP_list();show_APN_list();"></select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,8);"><#HSDPAConfig_ISP_itemname#></a></th>
                                            <td>
                                                <select name="modem_isp" id="modem_isp" class="input" onchange="show_APN_list()"></select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,13);"><#HSDPAConfig_USBAdapter_itemname#></a></th>
                                            <td>
                                                <select name="Dev3G" id="shown_modems" class="input"  disabled="disabled"></select>
                                            </td>
                                        </tr>
                                        <tr id="ras_mode_row1">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,3);"><#HSDPAConfig_private_apn_itemname#></a></th>
                                            <td>
                                                <input id="modem_apn" name="modem_apn" class="input" type="text" value=""/>
                                            </td>
                                        </tr>
                                        <tr id="ras_mode_row2">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,2);"><#HSDPAConfig_PIN_itemname#></a></th>
                                            <td>
                                                <input id="wan_3g_pin" name="wan_3g_pin" class="input" type="password" maxLength="8" value="<% nvram_get_x("", "wan_3g_pin"); %>"/>
                                            </td>
                                        </tr>
                                        <tr id="ras_mode_row3">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,10);"><#HSDPAConfig_DialNum_itemname#></a></th>
                                            <td>
                                                <input id="modem_dialnum" name="modem_dialnum" class="input" type="text" value=""/>
                                            </td>
                                        </tr>
                                        <tr id="ras_mode_row4">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,11);"><#HSDPAConfig_Username_itemname#></a></th>
                                            <td>
                                                <input id="modem_user" name="modem_user" class="input" type="text" value="<% nvram_get_x("", "modem_user"); %>"/>
                                            </td>
                                        </tr>
                                        <tr id="ras_mode_row5">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,12);"><#AiDisk_Password#></a></th>
                                            <td>
                                                <input id="modem_pass" name="modem_pass" class="input" type="text" value="<% nvram_get_x("", "modem_pass"); %>"/>
                                            </td>
                                        </tr>
                                        <tr id="ras_mode_row6">
                                            <th><#COM_User_AT#></th>
                                            <td>
                                                <input name="modem_cmd" class="input" type="text" maxLength="40" value="<% nvram_get_x("", "modem_cmd"); %>"/>
                                            </td>
                                        </tr>
                                        <tr id="ras_mode_row7">
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
                                        <tr>
                                            <th><#ModemARun#></th>
                                            <td>
                                                <select name="modem_arun" class="input">
                                                    <option value="0" <% nvram_match_x("General", "modem_arun", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("General", "modem_arun", "1", "selected"); %>><#checkbox_Yes#></option>
                                                    <option value="2" <% nvram_match_x("General", "modem_arun", "2", "selected"); %>><#ModemARunItem2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2">
                                                <center><input type="button" class="btn btn-primary" style="width: 219px" value="<#CTL_apply#>" onclick="applyRule();"></center>
                                            </td>
                                        </tr>
                                    </table>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    </form>

    <!--==============Beginning of hint content=============-->
    <div id="help_td" style="position: absolute; margin-left: -10000px" valign="top">
        <form name="hint_form"></form>
        <div id="helpicon" onClick="openHint(0,0);"><img src="images/help.gif" /></div>

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
    </div>
    <!--==============Ending of hint content=============-->

    <div id="footer"></div>
</div>
</body>
</html>
