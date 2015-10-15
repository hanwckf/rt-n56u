<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_5_3#></title>
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
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('fw_mac_drop');
});

</script>
<script>

<% login_state_hook(); %>

var client_mac = login_mac_str();
var smac = client_mac.split(":");

var ipmonitor = [<% get_static_client(); %>];
var wireless = [<% wl_auth_list(); %>];

var clients_info = getclients(1,0);

var MACList = [<% get_nvram_list("FirewallConfig", "MFList"); %>];

var isMenuopen = 0;

function initial(){
	show_banner(1);
	show_menu(5,5,4);
	show_footer();

	load_body();

	change_macfilter();

	showMFList();
	showLANIPList();
}

function applyRule(){
	if(prevent_lock()){
		showLoading();
		
		if (document.form.macfilter_enable_x.value == "0")
			document.form.action_mode.value = " Apply ";
		else
			document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_MACFilter_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
	else
		return false;
}

function change_macfilter() {
	if(document.form.macfilter_enable_x.value == "0"){
		$("mac_drop_row").style.display = "none";
		$("MFList_Block").style.display = "none";
	}
	else{
		$("mac_drop_row").style.display = "";
		$("MFList_Block").style.display = "";
	}
}

function prevent_lock(){
	if(document.form.macfilter_enable_x.value == "1"){
		if(document.form.macfilter_num_x_0.value < 1){
			if(confirm("<#FirewallConfig_MFList_accept_hint1#>")){
				document.form.macfilter_list_x_0.value = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];
				document.form.macfilter_time_x_0.value = "00002359";
				document.form.macfilter_date_x_0.value = "1111111";
				markGroupMAC(document.form.MFList2, 64, ' Add ');
				document.form.submit();
			}
			else
				return false;
		}
		else
			return true;
	}
	else
		return true;
}


function setClientMAC(num){
	document.form.macfilter_list_x_0.value = clients_info[num][2];
	hideClients_Block();
}

function showLANIPList(){
	var code = "";
	var show_name = "";
	for(var i = 0; i < clients_info.length ; i++){
		if(clients_info[i][0] && clients_info[i][0].length > 20)
			show_name = clients_info[i][0].substring(0, 18) + "..";
		else
			show_name = clients_info[i][0];
		
		if(clients_info[i][2]){
			code += '<a href="javascript:void(0)"><div onclick="setClientMAC('+i+');"><strong>'+clients_info[i][1]+'</strong>';
			code += ' ['+clients_info[i][2]+']';
			if(show_name && show_name.length > 0)
				code += ' ('+show_name+')';
			code += ' </div></a>';
		}
	}
	if (code == "")
		code = '<div style="text-align: center;" onclick="hideClients_Block();"><#Nodata#></div>';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	$("ClientList_Block").innerHTML = code;
}

function hideClients_Block(){
	$j("#chevron").children('i').removeClass('icon-chevron-up').addClass('icon-chevron-down');
	$('ClientList_Block').style.display='none';
	isMenuopen = 0;
}

function pullLANIPList(obj){
	if(isMenuopen == 0){
		$j(obj).children('i').removeClass('icon-chevron-down').addClass('icon-chevron-up');
		$("ClientList_Block").style.display = 'block';
		document.form.macfilter_list_x_0.focus();
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

function validNewRow(max_rows) {
	if (document.form.macfilter_num_x_0.value >= max_rows){
		alert("<#JS_itemlimit1#> " + max_rows + " <#JS_itemlimit2#>");
		return false;
	}

	if (document.form.macfilter_list_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.macfilter_list_x_0.focus();
		document.form.macfilter_list_x_0.select();
		return false;
	}

	if (!validate_hwaddr(document.form.macfilter_list_x_0)){
		return false;
	}

	if ((document.form.macfilter_date_x_Sun.checked == false) &&
		(document.form.macfilter_date_x_Mon.checked == false) &&
		(document.form.macfilter_date_x_Tue.checked == false) &&
		(document.form.macfilter_date_x_Wed.checked == false) &&
		(document.form.macfilter_date_x_Thu.checked == false) &&
		(document.form.macfilter_date_x_Fri.checked == false) &&
		(document.form.macfilter_date_x_Sat.checked == false)){
		alert("<#MAC_Days#> - <#JS_fieldblank#>");
		return false;
	}

	if (!validate_timerange(document.form.macfilter_time_x_starthour, 0) ||
		!validate_timerange(document.form.macfilter_time_x_startmin, 1) ||
		!validate_timerange(document.form.macfilter_time_x_endhour, 2) ||
		!validate_timerange(document.form.macfilter_time_x_endmin, 3)){
		return false;
	}

	var starttime = eval(document.form.macfilter_time_x_starthour.value + document.form.macfilter_time_x_startmin.value);
	var endtime = eval(document.form.macfilter_time_x_endhour.value + document.form.macfilter_time_x_endmin.value);
	if(starttime == endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
		document.form.macfilter_time_x_starthour.focus();
		document.form.macfilter_time_x_starthour.select();
		return false;
	}

	return true;
}

function updateDT() {
	document.form.macfilter_date_x_0.value = setDateCheck(
		document.form.macfilter_date_x_Sun,
		document.form.macfilter_date_x_Mon,
		document.form.macfilter_date_x_Tue,
		document.form.macfilter_date_x_Wed,
		document.form.macfilter_date_x_Thu,
		document.form.macfilter_date_x_Fri,
		document.form.macfilter_date_x_Sat);
	document.form.macfilter_time_x_0.value = setTimeRange(
		document.form.macfilter_time_x_starthour,
		document.form.macfilter_time_x_startmin,
		document.form.macfilter_time_x_endhour,
		document.form.macfilter_time_x_endmin);
}

function markGroupMAC(o, c, b) {
	document.form.group_id.value = "MFList";
	if(b == " Add "){
		if (validNewRow(c) == false)
			return false;
		
		updateDT();
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
}

function format_time(nvtime) {
	if (nvtime == "")
		nvtime = "00002359";
	return nvtime.substring(0, 2) + ":" + nvtime.substring(2, 4) + " - " + nvtime.substring(4, 6) + ":" + nvtime.substring(6, 8);
}

function format_date(nvdate) {
	var caption = "";
	if (getDateCheck(nvdate, 1) == true)
		caption = caption + ", <#DAY_Mon#>";
	if (getDateCheck(nvdate, 2) == true)
		caption = caption + ", <#DAY_Tue#>";
	if (getDateCheck(nvdate, 3) == true)
		caption = caption + ", <#DAY_Wed#>";
	if (getDateCheck(nvdate, 4) == true)
		caption = caption + ", <#DAY_Thu#>";
	if (getDateCheck(nvdate, 5) == true)
		caption = caption + ", <#DAY_Fri#>";
	if (getDateCheck(nvdate, 6) == true)
		caption = caption + ", <#DAY_Sat#>";
	if (getDateCheck(nvdate, 0) == true)
		caption = caption + ", <#DAY_Sun#>";
	
	if (caption == "")
		caption = "-";
	else
		caption = caption.substring(2);
	
	return caption;
}

function showMFList(){
	var code = '';
	var temp_time = '<% nvram_get_x("", "macfilter_time_x_0"); %>';
	var temp_date = '<% nvram_get_x("", "macfilter_date_x_0"); %>';

	if(MACList.length == 0) {
		code +='<tr><td colspan="4" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
		
		document.form.macfilter_time_x_0.value = (temp_time == '') ? "00002359" : temp_time;
		document.form.macfilter_date_x_0.value = (temp_date == '') ? "1111111" : temp_date;
	}
	else{
	    for(var i = 0; i < MACList.length; i++){
		code +='<tr id="row' + i + '">';
		code +='<td width="25%">&nbsp;' + MACList[i][0] + '</td>';
		code +='<td width="25%" style="text-align: center;">' + format_time(MACList[i][1]) + '</td>';
		code +='<td width="45%">' + format_date(MACList[i][2]) + '</td>';
		code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="MFList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="3">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="return markGroupMAC(this, 64, \' Del \');" name="MFList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
		
		var last_row = MACList.length - 1;
		document.form.macfilter_time_x_0.value = (temp_time == '') ? MACList[last_row][1] : temp_time;
		document.form.macfilter_date_x_0.value = (temp_date == '') ? MACList[last_row][2] : temp_date;
	}

	document.form.macfilter_date_x_Sun.checked = getDateCheck(document.form.macfilter_date_x_0.value, 0);
	document.form.macfilter_date_x_Mon.checked = getDateCheck(document.form.macfilter_date_x_0.value, 1);
	document.form.macfilter_date_x_Tue.checked = getDateCheck(document.form.macfilter_date_x_0.value, 2);
	document.form.macfilter_date_x_Wed.checked = getDateCheck(document.form.macfilter_date_x_0.value, 3);
	document.form.macfilter_date_x_Thu.checked = getDateCheck(document.form.macfilter_date_x_0.value, 4);
	document.form.macfilter_date_x_Fri.checked = getDateCheck(document.form.macfilter_date_x_0.value, 5);
	document.form.macfilter_date_x_Sat.checked = getDateCheck(document.form.macfilter_date_x_0.value, 6);

	document.form.macfilter_time_x_starthour.value = getTimeRange(document.form.macfilter_time_x_0.value, 0);
	document.form.macfilter_time_x_startmin.value = getTimeRange(document.form.macfilter_time_x_0.value, 1);
	document.form.macfilter_time_x_endhour.value = getTimeRange(document.form.macfilter_time_x_0.value, 2);
	document.form.macfilter_time_x_endmin.value = getTimeRange(document.form.macfilter_time_x_0.value, 3);

	$j('#MFList_Block').append(code);
}

function changeBgColor(obj, num){
	$("row" + num).style.background=(obj.checked)?'#D9EDF7':'whiteSmoke';
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
.radio.inline + .radio.inline,
.checkbox.inline + .checkbox.inline {
    margin-left: 3px;
}
.table-list td {
    padding: 6px 8px;
}
.table-list input,
.table-list select {
    margin-top: 0px;
    margin-bottom: 0px;
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
    <input type="hidden" name="current_page" value="Advanced_MACFilter_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="FirewallConfig;">
    <input type="hidden" name="group_id" value="MFList">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

    <input type="hidden" name="macfilter_time_x_0" value="00002359">
    <input type="hidden" name="macfilter_date_x_0" value="1111111">
    <input type="hidden" name="macfilter_num_x_0" value="<% nvram_get_x("", "macfilter_num_x"); %>" readonly="1" />

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
                            <h2 class="box_head round_top"><#menu5_5#> - <#menu5_5_3#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#FirewallConfig_display5_sectiondesc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,18,1);"><#FirewallConfig_MFMethod_itemname#></a></th>
                                            <td style="border-top: 0 none;">
                                                <select name="macfilter_enable_x" class="input" onchange="change_macfilter()">
                                                    <option value="0" <% nvram_match_x("","macfilter_enable_x", "0","selected"); %>><#CTL_Disabled#></option>
                                                    <option value="1" <% nvram_match_x("","macfilter_enable_x", "1","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
                                                    <option value="2" <% nvram_match_x("","macfilter_enable_x", "2","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr id="mac_drop_row" style="display:none;">
                                            <th><#MAC_BlockHost#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_mac_drop_on_of">
                                                        <input type="checkbox" id="fw_mac_drop_fake" <% nvram_match_x("", "fw_mac_drop", "1", "value=1 checked"); %><% nvram_match_x("", "fw_mac_drop", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="fw_mac_drop" id="fw_mac_drop_1" <% nvram_match_x("","fw_mac_drop", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="fw_mac_drop" id="fw_mac_drop_0" <% nvram_match_x("","fw_mac_drop", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table table-list" id="MFList_Block">
                                        <tr>
                                            <th colspan="4" style="background-color: #E3E3E3;"><#FirewallConfig_MFList_groupitemname#></th>
                                        </tr>
                                        <tr>
                                            <th width="25%"><#FirewallConfig_MFhwaddr_itemname#></th>
                                            <th width="25%"><#MAC_Time#></th>
                                            <th width="45%"><#MAC_Days#></th>
                                            <th width="5%">&nbsp;</th>
                                        </tr>
                                        <tr>
                                            <td width="25%">
                                                <div id="ClientList_Block" class="alert alert-info ddown-list" style="width: 400px;"></div>
                                                <div class="input-append">
                                                    <input type="text" maxlength="12" class="span12" size="12" name="macfilter_list_x_0" value="<% nvram_get_x("", "macfilter_list_x_0"); %>" onKeyPress="return is_hwaddr(event);" style="float:left; width: 110px">
                                                    <button class="btn btn-chevron" id="chevron" type="button" onclick="pullLANIPList(this);" title="Select the MAC of LAN clients."><i class="icon icon-chevron-down"></i></button>
                                                </div>
                                            </td>
                                            <td width="25%">
                                                <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_starthour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_startmin" onKeyPress="return is_number(this,event);"/>&nbsp;-
                                                <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_endhour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" class="input" style="width: 18px;" size="2" name="macfilter_time_x_endmin" onKeyPress="return is_number(this,event);"/>
                                            </td>
                                            <td width="45%">
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" name="macfilter_date_x_Mon" class="input"><#DAY_Mon#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="macfilter_date_x_Tue" class="input"><#DAY_Tue#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="macfilter_date_x_Wed" class="input"><#DAY_Wed#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="macfilter_date_x_Thu" class="input"><#DAY_Thu#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="macfilter_date_x_Fri" class="input"><#DAY_Fri#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="macfilter_date_x_Sat" class="input"><#DAY_Sat#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="macfilter_date_x_Sun" class="input"><#DAY_Sun#></label>
                                                </div>
                                            </td>
                                            <td width="5%">
                                                <button class="btn" style="max-width: 219px" type="submit" onclick="return markGroupMAC(this, 64, ' Add ');" name="MFList2" value="<#CTL_add#>" size="12"><i class="icon icon-plus"></i></button>
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
