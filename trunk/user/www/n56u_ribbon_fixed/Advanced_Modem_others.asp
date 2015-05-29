<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_4_4#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/itoggle.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/modem_isp.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('modem_rule', switch_modem_rule);
	init_itoggle('modem_dnsa', change_modem_dns_auto);
});

</script>
<script>

var country = '<% nvram_get_x("", "modem_country"); %>';
var isp = '<% nvram_get_x("", "modem_isp"); %>';
var apn = '<% nvram_get_x("", "modem_apn"); %>';
var dialnum = '<% nvram_get_x("", "modem_dialnum"); %>';
var user = '<% nvram_get_x("", "modem_user"); %>';
var pass = '<% nvram_get_x("", "modem_pass"); %>';

var countrylist = new Array();
var protolist = new Array();
var isplist = new Array();
var apnlist = new Array();
var diallist = new Array();
var userlist = new Array();
var passlist = new Array();

function initial(){
	var id_menu = 4;

	if(!found_app_smbd() && !found_app_ftpd())
		id_menu = 2;
	else if(!found_app_smbd() || !found_app_ftpd())
		id_menu = 3;

	show_banner(1);
	show_menu(5,6,id_menu);
	show_footer();

	gen_country_list();
	switch_modem_rule();
}

function switch_modem_rule(){
	var v = document.form.modem_rule[0].checked;

	showhide_div('tbl_modem_base', v);
	showhide_div('tbl_modem_dns', v);
	showhide_div('tbl_modem_adv', v);

	if (v){
		change_modem_dns_auto();
	}

	switch_modem_type();
}

function switch_modem_type(){
	var mtype = document.form.modem_type.value;
	if (mtype == "3") {
		$("row_modem_dial").style.display = "none";
		$("row_modem_apn").style.display = "";
		$("row_modem_nets").style.display = "";
		
		$("hint_user").innerHTML = "* QMI only";
		$("hint_pass").innerHTML = "* QMI only";
		$("hint_node").innerHTML = "* NCM only";
		$("hint_nets").innerHTML = "* QMI only";
		$("hint_pin").innerHTML  = "* QMI only";
		$("hint_cmd").innerHTML  = "* NCM only";
	}
	else {
		$("row_modem_dial").style.display = "";
		$("row_modem_nets").style.display = "none";
		
		if (mtype == "1")
			$("row_modem_apn").style.display = "none";
		else
			$("row_modem_apn").style.display = "";
		
		$("hint_user").innerHTML = "";
		$("hint_pass").innerHTML = "";
		$("hint_node").innerHTML = "";
		$("hint_nets").innerHTML = "";
		$("hint_pin").innerHTML  = "";
		$("hint_cmd").innerHTML  = "";
	}
	gen_list();
	show_APN_list();
}

function change_modem_dns_auto(){
	var use_auto = document.form.modem_dnsa[0].checked;
	inputCtrl(document.form.wan_dns1_x, !use_auto);
	inputCtrl(document.form.wan_dns2_x, !use_auto);
	inputCtrl(document.form.wan_dns3_x, !use_auto);

	if (use_auto){
		$("row_wan_dns1").style.display = "none";
		$("row_wan_dns2").style.display = "none";
		$("row_wan_dns3").style.display = "none";
	} else {
		$("row_wan_dns1").style.display = "";
		$("row_wan_dns2").style.display = "";
		$("row_wan_dns3").style.display = "";
	}
}

function gen_list(){
	var i;
	var mtype = document.form.modem_type.value;

	gen_isp_list();

	if (mtype != "3"){
		var sp_idx = 0;
		var sp_len = 0;
		var ar_len = protolist.length;
		for(i = 0; i < ar_len; i++){
			if(protolist[i] == mtype){
				if (sp_len == 0)
					sp_idx = i;
				sp_len++;
			}
			else if (sp_len > 0){
				break;
			}
		}
		
		if (sp_len > 0){
			var x, n;
			if ((sp_idx+sp_len) < ar_len){
				x = sp_idx+sp_len;
				n = ar_len-(sp_idx+sp_len);
				protolist.splice(x, n);
				isplist.splice(x, n);
				apnlist.splice(x, n);
				diallist.splice(x, n);
				userlist.splice(x, n);
				passlist.splice(x, n);
			}
			
			if (sp_idx > 0) {
				protolist.splice(0, sp_idx);
				isplist.splice(0, sp_idx);
				apnlist.splice(0, sp_idx);
				diallist.splice(0, sp_idx);
				userlist.splice(0, sp_idx);
				passlist.splice(0, sp_idx);
			}
		}
		else {
			gen_isp_list_empty();
		}
	}

	append_isp_list_empty();

	free_options($("modem_isp"));
	$("modem_isp").options.length = isplist.length;

	for(i = 0; i < isplist.length; i++){
		var caption = isplist[i];
		if (mtype == "3"){
			if (protolist[i] == "1")
				caption = caption + " (EVDO)";
			else if (protolist[i] == "2")
				caption = caption + " (TD-SCDMA)";
			else if (protolist[i] == "3")
				caption = caption + " (LTE)";
		}
		$("modem_isp").options[i] = new Option(caption, isplist[i]);
		if(isplist[i] == isp)
			$("modem_isp").options[i].selected = "1";
	}
}

function show_APN_list(){
	var ISPlist = $("modem_isp").value;

	if((ISPlist == isp) && (apn != "" || user != "" || pass != "")){
		$("modem_apn").value = apn;
		if (dialnum != "")
			$("modem_dialnum").value = dialnum;
		$("modem_user").value = user;
		$("modem_pass").value = pass;
	}
	else{
		for(var i = 0; i < isplist.length; i++){
			if(isplist[i] == ISPlist){
				$("modem_apn").value = apnlist[i];
				$("modem_dialnum").value = diallist[i];
				$("modem_user").value = userlist[i];
				$("modem_pass").value = passlist[i];
				break;
			}
		}
	}
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Modem_others.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	if (!document.form.modem_rule[0].checked)
		return true;

	if(!validate_range(document.form.modem_mtu, 1000, 1500))
		return false;

	if(!document.form.modem_dnsa[0].checked){
		if(!validate_ipaddr_final(document.form.wan_dns1_x, 'wan_dns_x'))
			return false;
		if(!validate_ipaddr_final(document.form.wan_dns2_x, 'wan_dns_x'))
			return false;
		if(!validate_ipaddr_final(document.form.wan_dns3_x, 'wan_dns_x'))
			return false;
	}

	return true;
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

<body onload="initial();" onunLoad="return unload_body();">

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
    <input type="hidden" name="current_page" value="Advanced_Modem_others.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="General;Layer3Forwarding;IPConnection;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

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
                                            <th width="50%" style="padding-bottom: 0px; border-top: 0 none;"><#HSDPAConfig_hsdpa_enable_itemname#></th>
                                            <td style="padding-bottom: 0px; border-top: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="modem_rule_on_of">
                                                        <input type="checkbox" id="modem_rule_fake" <% nvram_match_x("", "modem_rule", "1", "value=1 checked"); %><% nvram_match_x("", "modem_rule", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="modem_rule" id="modem_rule_1" class="input" <% nvram_match_x("", "modem_rule", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="modem_rule" id="modem_rule_0" class="input" <% nvram_match_x("", "modem_rule", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table" id="tbl_modem_base" style="display:none">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#ModemBase#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#ModemType#></th>
                                            <td>
                                                <select name="modem_type" class="input" onchange="switch_modem_type();">
                                                    <option value="0" <% nvram_match_x("", "modem_type", "0", "selected"); %>>RAS: WCDMA (UMTS)</option>
                                                    <option value="1" <% nvram_match_x("", "modem_type", "1", "selected"); %>>RAS: CDMA2000 (EVDO)</option>
                                                    <option value="2" <% nvram_match_x("", "modem_type", "2", "selected"); %>>RAS: TD-SCDMA</option>
                                                    <option value="3" <% nvram_match_x("", "modem_type", "3", "selected"); %>>NDIS: LTE and other</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,10);"><#HSDPAConfig_Country_itemname#>:</a></th>
                                            <td>
                                                <select name="modem_country" id="isp_countrys" class="input" onchange="gen_list();show_APN_list();"></select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,9);"><#HSDPAConfig_ISP_itemname#>:</a></th>
                                            <td>
                                                <select name="modem_isp" id="modem_isp" class="input" onchange="show_APN_list()"></select>
                                            </td>
                                        </tr>
                                        <tr id="row_modem_apn">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,3);"><#HSDPAConfig_private_apn_itemname#>:</a></th>
                                            <td>
                                                <input id="modem_apn" name="modem_apn" maxlength="32" class="input" type="text" value=""/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,2);"><#HSDPAConfig_pin_code_itemname#>:</a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input id="modem_pin" name="modem_pin" class="input" type="password" maxlength="8" size="32" style="width: 175px;" value="<% nvram_get_x("", "modem_pin"); %>" onkeypress="return is_number(this,event);"/>
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('modem_pin')"><i class="icon-eye-close"></i></button>
                                                    &nbsp;<span id="hint_pin" style="color:#888;"></span>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_modem_dial">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,11);"><#HSDPAConfig_DialNum_itemname#>:</a></th>
                                            <td>
                                                <input id="modem_dialnum" name="modem_dialnum" class="input" type="text" value=""/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,12);"><#HSDPAConfig_Username_itemname#>:</a></th>
                                            <td>
                                                <input id="modem_user" name="modem_user" class="input" type="text" value="<% nvram_get_x("", "modem_user"); %>"/>
                                                &nbsp;<span id="hint_user" style="color:#888;"></span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,21,13);"><#AiDisk_Password#>:</a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" name="modem_pass" id="modem_pass" maxlength="32" size="32" style="width: 175px;" value="<% nvram_get_x("", "modem_pass"); %>">
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('modem_pass')"><i class="icon-eye-close"></i></button>
                                                    &nbsp;<span id="hint_pass" style="color:#888;"></span>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_modem_nets">
                                            <th><#ModemNets#></th>
                                            <td>
                                                <select name="modem_nets" class="input">
                                                    <option value="0" <% nvram_match_x("", "modem_nets", "0", "selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("", "modem_nets", "1", "selected"); %>>LTE</option>
                                                    <option value="2" <% nvram_match_x("", "modem_nets", "2", "selected"); %>>LTE->UMTS</option>
                                                    <option value="3" <% nvram_match_x("", "modem_nets", "3", "selected"); %>>LTE->UMTS->GSM</option>
                                                    <option value="4" <% nvram_match_x("", "modem_nets", "4", "selected"); %>>UMTS</option>
                                                    <option value="5" <% nvram_match_x("", "modem_nets", "5", "selected"); %>>UMTS->GSM</option>
                                                    <option value="6" <% nvram_match_x("", "modem_nets", "6", "selected"); %>>GSM->UMTS</option>
                                                    <option value="7" <% nvram_match_x("", "modem_nets", "7", "selected"); %>>GSM</option>
                                                    <option value="8" <% nvram_match_x("", "modem_nets", "8", "selected"); %>>CDMA</option>
                                                    <option value="9" <% nvram_match_x("", "modem_nets", "9", "selected"); %>>TD-SCDMA</option>
                                                </select>
                                                &nbsp;<span id="hint_nets" style="color:#888;"></span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th style="padding-bottom: 0px;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,4);">MTU:</a></th>
                                            <td style="padding-bottom: 0px;">
                                                <input name="modem_mtu" class="input" type="text" maxlength="4" value="<% nvram_get_x("", "modem_mtu"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1000..1500]</span>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table" id="tbl_modem_dns" style="display:none">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IPConnection_x_DNSServerEnable_sectionname#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,5);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="modem_dnsa_on_of">
                                                        <input type="checkbox" id="modem_dnsa_fake" <% nvram_match_x("", "modem_dnsa", "1", "value=1 checked"); %><% nvram_match_x("", "modem_dnsa", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="modem_dnsa" id="modem_dnsa_1" value="1" onclick="change_modem_dns_auto();" <% nvram_match_x("", "modem_dnsa", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="modem_dnsa" id="modem_dnsa_0" value="0" onclick="change_modem_dns_auto();" <% nvram_match_x("", "modem_dnsa", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns1">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,6);"><#IPConnection_x_DNSServer1_itemname#> 1:</a></th>
                                            <td>
                                              <input type="text" maxlength="15" class="input" size="15" name="wan_dns1_x" value="<% nvram_get_x("", "wan_dns1_x"); %>" onkeypress="return is_ipaddr(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns2">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,7);"><#IPConnection_x_DNSServer1_itemname#> 2:</a></th>
                                            <td>
                                               <input type="text" maxlength="15" class="input" size="15" name="wan_dns2_x" value="<% nvram_get_x("", "wan_dns2_x"); %>" onkeypress="return is_ipaddr(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns3">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,21,8);"><#IPConnection_x_DNSServer1_itemname#> 3:</a></th>
                                            <td>
                                               <input type="text" maxlength="15" class="input" size="15" name="wan_dns3_x" value="<% nvram_get_x("", "wan_dns3_x"); %>" onkeypress="return is_ipaddr(this,event);" />
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table" id="tbl_modem_adv" style="display:none">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#ModemAdv#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#COM_Port_Node#></th>
                                            <td>
                                                <select name="modem_node" class="input">
                                                    <option value="0" <% nvram_match_x("", "modem_node", "0", "selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("", "modem_node", "1", "selected"); %>>ttyUSB0/ttyACM0</option>
                                                    <option value="2" <% nvram_match_x("", "modem_node", "2", "selected"); %>>ttyUSB1/ttyACM1</option>
                                                    <option value="3" <% nvram_match_x("", "modem_node", "3", "selected"); %>>ttyUSB2</option>
                                                    <option value="4" <% nvram_match_x("", "modem_node", "4", "selected"); %>>ttyUSB3</option>
                                                    <option value="5" <% nvram_match_x("", "modem_node", "5", "selected"); %>>ttyUSB4</option>
                                                    <option value="6" <% nvram_match_x("", "modem_node", "6", "selected"); %>>ttyUSB5</option>
                                                    <option value="7" <% nvram_match_x("", "modem_node", "7", "selected"); %>>ttyUSB6</option>
                                                    <option value="8" <% nvram_match_x("", "modem_node", "8", "selected"); %>>ttyUSB7</option>
                                                </select>
                                                &nbsp;<span id="hint_node" style="color:#888;"></span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#COM_User_AT#></th>
                                            <td>
                                                <input name="modem_cmd" class="input" type="text" maxlength="40" value="<% nvram_get_x("", "modem_cmd"); %>"/>
                                                &nbsp;<span id="hint_cmd" style="color:#888;"></span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#ModemZCD#></th>
                                            <td>
                                                <select name="modem_zcd" class="input">
                                                    <option value="0" <% nvram_match_x("", "modem_zcd", "0", "selected"); %>>usb-modeswitch</option>
                                                    <option value="1" <% nvram_match_x("", "modem_zcd", "1", "selected"); %>>legacy eject</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td style="border: 0 none;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
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

    <div id="footer"></div>
</div>
</body>
</html>
