<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_5_4#></title>
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
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('fw_lw_enable', change_lw_enable);
});

</script>
<script>

var LWFilterList = [<% get_nvram_list("FirewallConfig", "LWFilterList"); %>];

var wItem = new Array(
		new Array("HTTP", "80", "TCP"),
		new Array("HTTPS", "443", "TCP"),
		new Array("FTP", "20:21", "TCP"),
		new Array("SSH", "22", "TCP"),
		new Array("TELNET", "23", "TCP"),
		new Array("L2TP Tunnel", "1701", "UDP"),
		new Array("PPTP Control", "1723", "TCP"),
		new Array("GRE", "47", "OTHER"),
		new Array("IPSEC-ESP", "50", "OTHER"),
		new Array("IPSEC-AH", "51", "OTHER"),
		new Array("IPSEC IKE", "500", "UDP"),
		new Array("IPSEC NAT-T", "4500", "UDP"),
		new Array("IPv6 Tunnel", "41", "OTHER"));

function initial(){
	show_banner(1);
	show_menu(5,5,5);
	show_footer();

	change_lw_enable();

	document.form.filter_lw_date_x_Sun.checked = getDateCheck(document.form.filter_lw_date_x.value, 0);
	document.form.filter_lw_date_x_Mon.checked = getDateCheck(document.form.filter_lw_date_x.value, 1);
	document.form.filter_lw_date_x_Tue.checked = getDateCheck(document.form.filter_lw_date_x.value, 2);
	document.form.filter_lw_date_x_Wed.checked = getDateCheck(document.form.filter_lw_date_x.value, 3);
	document.form.filter_lw_date_x_Thu.checked = getDateCheck(document.form.filter_lw_date_x.value, 4);
	document.form.filter_lw_date_x_Fri.checked = getDateCheck(document.form.filter_lw_date_x.value, 5);
	document.form.filter_lw_date_x_Sat.checked = getDateCheck(document.form.filter_lw_date_x.value, 6);

	document.form.filter_lw_time_x_starthour.value = getTimeRange(document.form.filter_lw_time_x.value, 0);
	document.form.filter_lw_time_x_startmin.value = getTimeRange(document.form.filter_lw_time_x.value, 1);
	document.form.filter_lw_time_x_endhour.value = getTimeRange(document.form.filter_lw_time_x.value, 2);
	document.form.filter_lw_time_x_endmin.value = getTimeRange(document.form.filter_lw_time_x.value, 3);

	showLWFilterList();

	load_wizard();
	change_proto();

	load_body();
}

function applyRule(){
	if(validForm()){
		document.form.filter_lw_date_x.value = setDateCheck(
			document.form.filter_lw_date_x_Sun,
			document.form.filter_lw_date_x_Mon,
			document.form.filter_lw_date_x_Tue,
			document.form.filter_lw_date_x_Wed,
			document.form.filter_lw_date_x_Thu,
			document.form.filter_lw_date_x_Fri,
			document.form.filter_lw_date_x_Sat);
		document.form.filter_lw_time_x.value = setTimeRange(
			document.form.filter_lw_time_x_starthour,
			document.form.filter_lw_time_x_startmin,
			document.form.filter_lw_time_x_endhour,
			document.form.filter_lw_time_x_endmin);
		
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_Firewall_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if((document.form.fw_lw_enable_x[0].checked ==true)
		&& (document.form.filter_lw_date_x_Sun.checked ==false)
		&& (document.form.filter_lw_date_x_Mon.checked ==false)
		&& (document.form.filter_lw_date_x_Tue.checked ==false)
		&& (document.form.filter_lw_date_x_Wed.checked ==false)
		&& (document.form.filter_lw_date_x_Thu.checked ==false)
		&& (document.form.filter_lw_date_x_Fri.checked ==false)
		&& (document.form.filter_lw_date_x_Sat.checked ==false)){
			alert("<#FirewallConfig_LanWanActiveDate_itemname#> <#JS_fieldblank#>");
			return false;
	}

	if(document.form.fw_lw_enable_x[0].checked ==true){
		if(!validate_timerange(document.form.filter_lw_time_x_starthour, 0)
			|| !validate_timerange(document.form.filter_lw_time_x_startmin, 1)
			|| !validate_timerange(document.form.filter_lw_time_x_endhour, 2)
			|| !validate_timerange(document.form.filter_lw_time_x_endmin, 3)
			)
		return false;
		
		var starttime = eval(document.form.filter_lw_time_x_starthour.value + document.form.filter_lw_time_x_startmin.value);
		var endtime = eval(document.form.filter_lw_time_x_endhour.value + document.form.filter_lw_time_x_endmin.value);
		if(starttime == endtime){
			alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
			document.form.filter_lw_time_x_starthour.focus();
			document.form.filter_lw_time_x_starthour.select;
			return false;
		}
	}

	if(!validate_portlist(document.form.filter_lw_icmp_x, 'filter_lw_icmp_x'))
		return false;

	return true;
}

function done_validating(action){
	refreshpage();
}

function change_proto(){
	var v = (document.form.filter_lw_proto_x_0.options.selectedIndex == 9) ? 1 : 0;
	inputCtrl(document.form.filter_lw_protono_x_0, v);
	inputCtrl(document.form.filter_lw_srcport_x_0, !v);
	inputCtrl(document.form.filter_lw_dstport_x_0, !v);
	if(v){
		document.form.filter_lw_dstport_x_0.style.display = "none";
		document.form.filter_lw_protono_x_0.style.display = "";
		$("col_port_proto").innerHTML = "<#IPConnection_VServerPNo_itemname#>";
	}else{
		document.form.filter_lw_protono_x_0.style.display = "none";
		document.form.filter_lw_dstport_x_0.style.display = "";
		$("col_port_proto").innerHTML = '<a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,2);"><#FirewallConfig_LanWanDstPort_itemname#></a>';
	}
}

function load_wizard(){
	free_options(document.form.LWKnownApps);
	add_option(document.form.LWKnownApps, "<#Select_menu_default#>", "User Defined", 1);
	for (var i = 0; i < wItem.length; i++)
		add_option(document.form.LWKnownApps, wItem[i][0], wItem[i][0], 0);
}

function change_wizard(o, id){
	var i;
	var obj = document.form.filter_lw_proto_x_0;
	for(i = 0; i < wItem.length; i++){
		if(wItem[i][0] != null){
			if(o.value == wItem[i][0]){
				if(wItem[i][2] == "TCP")
					obj.options[0].selected = 1;
				else if(wItem[i][2] == "UDP")
					obj.options[8].selected = 1;
				else if(wItem[i][2] == "OTHER")
					obj.options[9].selected = 1;
				
				if (obj.options.selectedIndex == 9)
					document.form.filter_lw_protono_x_0.value = wItem[i][1];
				else
					document.form.filter_lw_dstport_x_0.value = wItem[i][1];
				
				break;
			}
		}
	}
	change_proto();
}

function change_lw_enable(){
	var v = document.form.fw_lw_enable_x[0].checked;
	showhide_div('tbl_lwf_main', v);
	showhide_div('LWFilterList_Block', v);
}

function valid_IP_subnet(obj){
	var ipPattern1 = new RegExp("(^([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.(\\*)$)", "gi");
	var ipPattern2 = new RegExp("(^([0-9]{1,3})\\.([0-9]{1,3})\\.(\\*)\\.(\\*)$)", "gi");
	var ipPattern3 = new RegExp("(^([0-9]{1,3})\\.(\\*)\\.(\\*)\\.(\\*)$)", "gi");
	var ipPattern4 = new RegExp("(^(\\*)\\.(\\*)\\.(\\*)\\.(\\*)$)", "gi");
	var parts = obj.value.split(".");
	if(!ipPattern1.test(obj.value) && !ipPattern2.test(obj.value) && !ipPattern3.test(obj.value) && !ipPattern4.test(obj.value)){
		alert(obj.value + " <#JS_validip#>");
		obj.focus();
		obj.select();
		return false;
	}else if(parts[0] == 0 || parts[0] > 255 || parts[1] > 255 || parts[2] > 255){
		alert(obj.value + " <#JS_validip#>");
		obj.focus();
		obj.select();
		return false;
	}else
		return true;
}

function valid_IP_form(obj){
	if(obj.value == "")
		return true;
	if(!validate_ipaddr_final(obj, ""))
		return false;
	return true;
}

function markGroupLWF(o, c, b) {
	var i, obj, proto_other;
	document.form.group_id.value = "LWFilterList";
	if(b == " Add "){
		proto_other = (document.form.filter_lw_proto_x_0.options.selectedIndex == 9)?true:false;
		if (document.form.filter_lw_num_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}
		obj = document.form.filter_lw_srcip_x_0;
		if(obj.value.split("*").length >= 2){
			if(!valid_IP_subnet(obj))
				return false;
		}else if(!valid_IP_form(obj))
			return false;
		if (!validate_iprange(obj, ""))
			return false;
		obj = document.form.filter_lw_dstip_x_0;
		if(obj.value.split("*").length >= 2){
			if(!valid_IP_subnet(obj))
				return false;
		}else if(!valid_IP_form(obj))
			return false;
		if (!validate_iprange(obj, ""))
			return false;
		if (proto_other){
			obj = document.form.filter_lw_protono_x_0;
			if (obj.value==""){
				alert("<#JS_fieldblank#>");
				obj.focus();
				return false;
			}else if (!validate_range(obj, 0, 255))
				return false;
			
			for (i = 0; i < LWFilterList.length; i++) {
				if (document.form.filter_lw_srcip_x_0.value == LWFilterList[i][0] &&
						document.form.filter_lw_dstip_x_0.value == LWFilterList[i][2] &&
						document.form.filter_lw_protono_x_0.value == LWFilterList[i][5] &&
						document.form.filter_lw_proto_x_0.value == LWFilterList[i][4]) {
					alert("<#JS_duplicate#>");
					return false;
				}
			}
			document.form.filter_lw_srcport_x_0.value = "";
			document.form.filter_lw_dstport_x_0.value = "";
		}else{
			if (!validate_portrange(document.form.filter_lw_srcport_x_0, ""))
				return false;
			if (!validate_portrange(document.form.filter_lw_dstport_x_0, ""))
				return false;
			
			if (document.form.filter_lw_srcip_x_0.value == "" &&
					document.form.filter_lw_dstip_x_0.value == "" &&
					document.form.filter_lw_srcport_x_0.value == "" &&
					document.form.filter_lw_dstport_x_0.value == ""){
				alert("<#JS_fieldblank#>");
				return false;
			}
			for (i = 0; i < LWFilterList.length; i++) {
				if (document.form.filter_lw_srcip_x_0.value == LWFilterList[i][0] &&
						document.form.filter_lw_srcport_x_0.value == LWFilterList[i][1] &&
						document.form.filter_lw_dstip_x_0.value == LWFilterList[i][2] &&
						document.form.filter_lw_dstport_x_0.value == LWFilterList[i][3] &&
						document.form.filter_lw_proto_x_0.value == LWFilterList[i][4]) {
					alert("<#JS_duplicate#>");
					return false;
				}
			}
			document.form.filter_lw_protono_x_0.value = "";
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
}

function showLWFilterList(){
	var i;
	var code = '';
	var srcaddr, srcport, dstaddr, dstport, protono;
	if(LWFilterList.length == 0)
		code +='<tr><td colspan="6" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
	    for(i = 0; i < LWFilterList.length; i++){
		srcaddr = "*";
		dstaddr = "*";
		protono = LWFilterList[i][4];
		if (LWFilterList[i][0] != null && LWFilterList[i][0] != "")
			srcaddr = LWFilterList[i][0];
		if (LWFilterList[i][2] != null && LWFilterList[i][2] != "")
			dstaddr = LWFilterList[i][2];
		if(protono == "OTHER"){
			srcport = "-";
			dstport = "-";
			protono = LWFilterList[i][5];
		}else{
			srcport = "*";
			dstport = "*";
			if (LWFilterList[i][1] != null && LWFilterList[i][1] != "")
				srcport = LWFilterList[i][1];
			if (LWFilterList[i][3] != null && LWFilterList[i][3] != "")
				dstport = LWFilterList[i][3];
		}
		code +='<tr id="row' + i + '">';
		code +='<td>&nbsp;'             + srcaddr + '</td>';
		code +='<td width="15%">&nbsp;' + srcport + '</td>';
		code +='<td width="25%">&nbsp;' + dstaddr + '</td>';
		code +='<td width="15%">&nbsp;' + dstport + '</td>';
		code +='<td width="15%">&nbsp;' + protono + '</td>';
		code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="LWFilterList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="5">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupLWF(this,64,\' Del \');" name="LWFilterList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	$j('#LWFilterList_Block').append(code);
}

function changeBgColor(obj, num){
	$("row" + num).style.background=(obj.checked)?'#D9EDF7':'whiteSmoke';
}

</script>
<style>
.nav-tabs > li > a {
    padding-right: 6px;
    padding-left: 6px;
}

.radio.inline + .radio.inline,
.checkbox.inline + .checkbox.inline {
    margin-left: 3px;
}
.table-list td {
    padding: 6px 4px;
}
.table-list input,
.table-list select {
    margin-top: 0px;
    margin-bottom: 0px;
}
.table-list tr:nth-child(2) {
    font-size: 75%;
    font-weight: bold;
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

    <input type="hidden" name="current_page" value="Advanced_Firewall_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="FirewallConfig;">
    <input type="hidden" name="group_id" value="LWFilterList">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

    <input type="hidden" name="filter_lw_date_x" value="<% nvram_get_x("","filter_lw_date_x"); %>">
    <input type="hidden" name="filter_lw_time_x" value="<% nvram_get_x("","filter_lw_time_x"); %>">
    <input type="hidden" name="filter_lw_num_x_0" value="<% nvram_get_x("", "filter_lw_num_x"); %>" readonly="1">

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
                            <h2 class="box_head round_top"><#menu5_5#> - <#menu5_5_4#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#FirewallConfig_display1_sectiondesc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="padding-bottom: 0px; border-top: 0 none;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,10,5);"><#FirewallConfig_LanWanFirewallEnable_itemname#></a></th>
                                            <td style="padding-bottom: 0px; border-top: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="fw_lw_enable_on_of">
                                                        <input type="checkbox" id="fw_lw_enable_fake" <% nvram_match_x("", "fw_lw_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "fw_lw_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="fw_lw_enable_x" id="fw_lw_enable_1" onClick="change_lw_enable();" <% nvram_match_x("","fw_lw_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="fw_lw_enable_x" id="fw_lw_enable_0" onClick="change_lw_enable();" <% nvram_match_x("","fw_lw_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_lwf_main" style="display:none">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#menu5_5_4#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,10,3);"><#FirewallConfig_LanWanDefaultAct_itemname#></a></th>
                                            <td>
                                                <select name="filter_lw_default_x" class="input">
                                                    <option value="DROP" <% nvram_match_x("","filter_lw_default_x", "DROP","selected"); %>><#WhiteList#></option>
                                                    <option value="ACCEPT" <% nvram_match_x("","filter_lw_default_x", "ACCEPT","selected"); %>><#BlackList#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,10,1);"><#FirewallConfig_LanWanActiveDate_itemname#></a></th>
                                            <td>
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" name="filter_lw_date_x_Mon" class="input" onChange="return changeDate();"><#DAY_Mon#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="filter_lw_date_x_Tue" class="input" onChange="return changeDate();"><#DAY_Tue#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="filter_lw_date_x_Wed" class="input" onChange="return changeDate();"><#DAY_Wed#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="filter_lw_date_x_Thu" class="input" onChange="return changeDate();"><#DAY_Thu#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="filter_lw_date_x_Fri" class="input" onChange="return changeDate();"><#DAY_Fri#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="filter_lw_date_x_Sat" class="input" onChange="return changeDate();"><#DAY_Sat#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="filter_lw_date_x_Sun" class="input" onChange="return changeDate();"><#DAY_Sun#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,10,2);"><#FirewallConfig_LanWanActiveTime_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="filter_lw_time_x_starthour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="filter_lw_time_x_startmin" onKeyPress="return is_number(this,event);"/>-
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="filter_lw_time_x_endhour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="filter_lw_time_x_endmin" onKeyPress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,10,4);"><#FirewallConfig_LanWanICMP_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="36" class="input" size="32" name="filter_lw_icmp_x" value="<% nvram_get_x("","filter_lw_icmp_x"); %>" onKeyPress="return is_portlist(this,event)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#FirewallConfig_LWFilterList_widzarddesc#></th>
                                            <td>
                                                <select name="LWKnownApps"  class="input" onChange="change_wizard(this, 'LWKnownApps');">
                                                    <option value="User Defined"><#Select_menu_default#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table table-list" id="LWFilterList_Block" style="display:none">
                                        <tr>
                                            <th colspan="6" style="background-color: #E3E3E3;"><#FirewallConfig_LWFilterList_groupitemdesc#></th>
                                        </tr>
                                        <tr>
                                            <td><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,3);"><#FirewallConfig_LanWanSrcIP_itemname#></a></td>
                                            <td width="15%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,2);"><#FirewallConfig_LanWanSrcPort_itemname#></a></td>
                                            <td width="25%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,3);"><#FirewallConfig_LanWanDstIP_itemname#></a></td>
                                            <td width="15%" id="col_port_proto"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,2);"><#FirewallConfig_LanWanDstPort_itemname#></a></td>
                                            <td width="15%"><#FirewallConfig_LanWanProFlag_itemname#></td>
                                            <td width="5%">&nbsp;</td>
                                        </tr>
                                        <tr>
                                            <td>
                                                <input type="text" maxlength="15" class="span12" size="14" name="filter_lw_srcip_x_0" value="<% nvram_get_x("", "filter_lw_srcip_x_0"); %>" onKeyPress="return is_iprange(this,event);"/>
                                            </td>
                                            <td>
                                                <input type="text" maxlength="11" class="span12" size="10" name="filter_lw_srcport_x_0" value="<% nvram_get_x("", "filter_lw_srcport_x_0"); %>" onKeyPress="return is_portrange(this,event);"/>
                                            </td>
                                            <td>
                                                <input type="text" maxlength="15" class="span12" size="14" name="filter_lw_dstip_x_0" value="<% nvram_get_x("", "filter_lw_dstip_x_0"); %>" onKeyPress="return is_iprange(this,event);"/>
                                            </td>
                                            <td>
                                                <input type="text" maxlength="11" class="span12" size="10" name="filter_lw_dstport_x_0" value="<% nvram_get_x("", "filter_lw_dstport_x_0"); %>" onKeyPress="return is_portrange(this,event);"/>
                                                <input style="display:none" type="text" class="span12" maxlength="3" size="3" name="filter_lw_protono_x_0" value="<% nvram_get_x("", "filter_lw_protono_x_0"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                            <td>
                                                <select name="filter_lw_proto_x_0" class="span12" onchange="change_proto()">
                                                    <option value="TCP"     <% nvram_match_x("","filter_lw_proto_x_0","TCP","selected"); %>>TCP</option>
                                                    <option value="TCP ALL" <% nvram_match_x("","filter_lw_proto_x_0","TCP ALL","selected"); %>>TCP ALL</option>
                                                    <option value="TCP SYN" <% nvram_match_x("","filter_lw_proto_x_0","TCP SYN","selected"); %>>TCP SYN</option>
                                                    <option value="TCP ACK" <% nvram_match_x("","filter_lw_proto_x_0","TCP ACK","selected"); %>>TCP ACK</option>
                                                    <option value="TCP FIN" <% nvram_match_x("","filter_lw_proto_x_0","TCP FIN","selected"); %>>TCP FIN</option>
                                                    <option value="TCP RST" <% nvram_match_x("","filter_lw_proto_x_0","TCP RST","selected"); %>>TCP RST</option>
                                                    <option value="TCP URG" <% nvram_match_x("","filter_lw_proto_x_0","TCP URG","selected"); %>>TCP URG</option>
                                                    <option value="TCP PSH" <% nvram_match_x("","filter_lw_proto_x_0","TCP PSH","selected"); %>>TCP PSH</option>
                                                    <option value="UDP"     <% nvram_match_x("","filter_lw_proto_x_0","UDP","selected"); %>>UDP</option>
                                                    <option value="OTHER"   <% nvram_match_x("","filter_lw_proto_x_0","OTHER","selected"); %>>Other</option>
                                                </select>
                                            </td>
                                            <td>
                                                <button class="btn" type="submit" onclick="return markGroupLWF(this, 64, ' Add ');" name="LWFilterList2"><i class="icon icon-plus"></i></button>
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
