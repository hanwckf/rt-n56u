<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_6_2#></title>
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
<script type="text/javascript" src="/validator.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
    init_itoggle('reboot_schedule_enable',change_on);
	init_itoggle('help_enable');
});

</script>
<script>

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,7,1);
	show_footer();
	
	if(reboot_schedule_support){
		document.form.reboot_date_x_Sun.checked = getDateCheck(document.form.reboot_schedule.value, 0);
		document.form.reboot_date_x_Mon.checked = getDateCheck(document.form.reboot_schedule.value, 1);
		document.form.reboot_date_x_Tue.checked = getDateCheck(document.form.reboot_schedule.value, 2);
		document.form.reboot_date_x_Wed.checked = getDateCheck(document.form.reboot_schedule.value, 3);
		document.form.reboot_date_x_Thu.checked = getDateCheck(document.form.reboot_schedule.value, 4);
		document.form.reboot_date_x_Fri.checked = getDateCheck(document.form.reboot_schedule.value, 5);
		document.form.reboot_date_x_Sat.checked = getDateCheck(document.form.reboot_schedule.value, 6);
		document.form.reboot_time_x_hour.value = getrebootTimeRange(document.form.reboot_schedule.value, 0);
		document.form.reboot_time_x_min.value = getrebootTimeRange(document.form.reboot_schedule.value, 1);
		document.getElementById('reboot_schedule_enable_tr').style.display = "";
		change_on();
		
	}
	else{
		document.getElementById('reboot_schedule_enable_tr').style.display = "none";
		document.getElementById('reboot_schedule_date_tr').style.display = "none";
		document.getElementById('reboot_schedule_time_tr').style.display = "none";
	}

	if(document.form.computer_name2.value != "")
		document.form.computer_name.value = decodeURIComponent(document.form.computer_name2.value);
	else
		document.form.computer_name.value = "";

	document.form.http_passwd2.value = "";

	if (login_safe()){
		showhide_div('row_user', 1);
		showhide_div('row_pass1', 1);
		showhide_div('row_pass2', 1);
	}

	load_body();
}

function applyRule(){
	if(validForm()){
	if(reboot_schedule_support){
			updateDateTime();
		}
		showLoading();
		
		if(document.form.http_passwd2.value.length > 0)
			document.form.http_passwd.value = document.form.http_passwd2.value;
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_System_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
		
		
	}
	

}
function change_on(){
var v = document.form.reboot_schedule_enable_x.value;
        showhide_div('reboot_schedule_date_tr', v);
		showhide_div('reboot_schedule_time_tr', v);
		if ( v == 1 )
		check_Timefield_checkbox();

}
function validForm(){
	if(!validate_string(document.form.http_username))
		return false;

	if(!validate_string(document.form.http_passwd2) || !validate_string(document.form.v_password2))
		return false;

	if(document.form.http_passwd2.value != document.form.v_password2.value){
		showtext($("alert_msg"),"*<#File_Pop_content_alert_desc7#>");
		
		document.form.http_passwd2.focus();
		document.form.http_passwd2.select();
		
		return false;
	}

	if(!blanktest(document.form.computer_name, "computer_name")){
		document.form.computer_name.focus();
		document.form.computer_name.select();
		return false;
	}

	var re = new RegExp("[^a-zA-Z0-9 _-]+", "gi");
	if(re.test(document.form.computer_name.value)){
		alert("<#JS_validchar#>");
		document.form.computer_name.focus();
		document.form.computer_name.select();
		return false;
	}

	if(!validate_string(document.form.ntp_server0))
		return false;

	if(!validate_ipaddr_final(document.form.log_ipaddr, 'log_ipaddr'))
		return false;

	if(!validate_range(document.form.log_port, 1, 65535))
		return false;

	if(document.form.http_passwd2.value.length > 0)
		alert("<#File_Pop_content_alert_desc10#>");
	
	if(reboot_schedule_support){
		if(!document.form.reboot_date_x_Sun.checked && !document.form.reboot_date_x_Mon.checked &&
		!document.form.reboot_date_x_Tue.checked && !document.form.reboot_date_x_Wed.checked &&
		!document.form.reboot_date_x_Thu.checked && !document.form.reboot_date_x_Fri.checked &&
		!document.form.reboot_date_x_Sat.checked && document.form.reboot_schedule_enable_x[0].checked)
		{
			alert(Untranslated.filter_lw_date_valid);
			document.form.reboot_date_x_Sun.focus();
			return false;
		}
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

function blanktest(obj, flag){
	var value2 = eval("document.form."+flag+"2.value");

	if(obj.value == ""){
		if(value2 != "")
			obj.value = decodeURIComponent(value2);
		else
			obj.value = "";
		
		alert("<#JS_Shareblanktest#>");
		
		return false;
	}

	return true;
}

function openLink(s) {
	var link_params = "toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=480";
	var tourl = "http://support.ntp.org/bin/view/Servers/WebHome";
	link = window.open(tourl, "NTPLink", link_params);
	if (!link.opener) link.opener = self;
}

function check_Timefield_checkbox(){	// To check Date checkbox checked or not and control Time field disabled or not
	if( document.form.reboot_date_x_Sun.checked == true 
		|| document.form.reboot_date_x_Mon.checked == true 
		|| document.form.reboot_date_x_Tue.checked == true
		|| document.form.reboot_date_x_Wed.checked == true
		|| document.form.reboot_date_x_Thu.checked == true
		|| document.form.reboot_date_x_Fri.checked == true
		|| document.form.reboot_date_x_Sat.checked == true	){
			inputCtrl(document.form.reboot_time_x_hour,1);
			inputCtrl(document.form.reboot_time_x_min,1);
			document.form.reboot_schedule.disabled = false;
	}
	else{
			inputCtrl(document.form.reboot_time_x_hour,0);
			inputCtrl(document.form.reboot_time_x_min,0);
			document.form.reboot_schedule.disabled = true;
			document.getElementById('reboot_schedule_time_tr').style.display ="";
	}
}


function getrebootTimeRange(str, pos)
{
	if (pos == 0)
		return str.substring(7,9);
	else if (pos == 1)
		return str.substring(9,11);
}

function setrebootTimeRange(rd, rh, rm)
{
	return(rd.value+rh.value+rm.value);
}

function updateDateTime()
{
	if(document.form.reboot_schedule_enable_x[0].checked){
		document.form.reboot_schedule_enable.value = "1";
		document.form.reboot_schedule.disabled = false;
		document.form.reboot_schedule.value = setDateCheck(
		document.form.reboot_date_x_Sun,
		document.form.reboot_date_x_Mon,
		document.form.reboot_date_x_Tue,
		document.form.reboot_date_x_Wed,
		document.form.reboot_date_x_Thu,
		document.form.reboot_date_x_Fri,
		document.form.reboot_date_x_Sat);
		document.form.reboot_schedule.value = setrebootTimeRange(
		document.form.reboot_schedule,
		document.form.reboot_time_x_hour,
		document.form.reboot_time_x_min);
	}
	else
		document.form.reboot_schedule_enable.value = "0";
}

</script>
<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
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
    <input type="hidden" name="current_page" value="Advanced_System_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;General;Storage;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("", "preferred_lang"); %>">
    <input type="hidden" name="http_passwd" value="">
    <input type="hidden" name="computer_name2" value="<% nvram_get_x("", "computer_name"); %>">
	<input type="hidden" name="reboot_schedule" value="<% nvram_get_x("", "reboot_schedule"); %>" disabled>
    <input type="hidden" name="reboot_schedule_enable" value="<% nvram_get_x("", "reboot_schedule_enable"); %>">

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
                            <h2 class="box_head round_top"><#menu5_6#> - <#menu5_6_2#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#Adm_System_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_System_ident#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%">
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17,2);"><#ShareNode_DeviceName_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="computer_name" id="computer_name" class="input" maxlength="15" size="32" value=""/>
                                            </td>
                                        </tr>
                                        <tr id="row_user" style="display:none">
                                            <th><#Adm_System_admin#></th>
                                            <td>
                                                <input type="text" name="http_username" class="input" autocomplete="off" maxlength="32" size="25" value="<% nvram_get_x("","http_username"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_pass1" style="display:none">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,11,4)"><#PASS_new#></a></th>
                                            <td>
                                                <input type="password" name="http_passwd2" class="input" autocomplete="off" maxlength="32" size="25" onKeyPress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_pass2" style="display:none">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,11,4)"><#PASS_retype#></a></th>
                                            <td>
                                                <input type="password" name="v_password2" class="input" autocomplete="off" maxlength="32" size="25" onKeyPress="return is_string(this,event);"/><br/><span id="alert_msg"></span>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#General_x_SystemTime_itemname#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,11,2)"><#LANHostConfig_x_TimeZone_itemname#></a></th>
                                            <td>
                                                <select name="time_zone" class="input" onchange="return change_common(this, 'LANHostConfig', 'time_zone')">
                                                    <option value="UCT12" <% nvram_match_x("","time_zone", "UCT12","selected"); %>			>(GMT-12:00) <#TZ01#></option>
                                                    <option value="UCT11" <% nvram_match_x("","time_zone", "UCT11","selected"); %>			>(GMT-11:00) <#TZ02#></option>
                                                    <option value="UCT10" <% nvram_match_x("","time_zone", "UCT10","selected"); %>			>(GMT-10:00) <#TZ03#></option>
                                                    <option value="NAST9NADT" <% nvram_match_x("","time_zone", "NAST9NADT","selected"); %>		>(GMT-09:00) <#TZ04#></option>
                                                    <option value="PST8PDT" <% nvram_match_x("","time_zone", "PST8PDT","selected"); %>			>(GMT-08:00) <#TZ05#></option>
                                                    <option value="MST7_1" <% nvram_match_x("","time_zone", "MST7_1","selected"); %>			>(GMT-07:00) <#TZ06#></option>
                                                    <option value="MST7" <% nvram_match_x("","time_zone", "MST7","selected"); %>			>(GMT-07:00) <#TZ07#></option>
                                                    <option value="MST7MDT" <% nvram_match_x("","time_zone", "MST7MDT","selected"); %>			>(GMT-07:00) <#TZ08#></option>
                                                    <option value="CST6CDT_1" <% nvram_match_x("","time_zone", "CST6CDT_1","selected"); %>		>(GMT-06:00) <#TZ09#></option>
                                                    <option value="CST6" <% nvram_match_x("","time_zone", "CST6","selected"); %>			>(GMT-06:00) <#TZ10#></option>
                                                    <option value="CST6CDT_3" <% nvram_match_x("","time_zone", "CST6CDT_3","selected"); %>		>(GMT-06:00) <#TZ11#></option>
                                                    <option value="CST6CDT_3_1" <% nvram_match_x("","time_zone", "CST6CDT_3_1","selected"); %>		>(GMT-06:00) <#TZ12#></option>
                                                    <option value="UCT6" <% nvram_match_x("","time_zone", "UCT6","selected"); %>			>(GMT-06:00) <#TZ13#></option>
                                                    <option value="EST5EDT" <% nvram_match_x("","time_zone", "EST5EDT","selected"); %>			>(GMT-05:00) <#TZ14#></option>
                                                    <option value="UCT5_1" <% nvram_match_x("","time_zone", "UCT5_1","selected"); %>			>(GMT-05:00) <#TZ15#></option>
                                                    <option value="UCT5_2" <% nvram_match_x("","time_zone", "UCT5_2","selected"); %>			>(GMT-05:00) <#TZ16#></option>
                                                    <option value="AST4ADT" <% nvram_match_x("","time_zone", "AST4ADT","selected"); %>			>(GMT-04:00) <#TZ17#></option>
                                                    <option value="UCT4_1" <% nvram_match_x("","time_zone", "UCT4_1","selected"); %>			>(GMT-04:00) <#TZ18#></option>
                                                    <option value="UCT4_2" <% nvram_match_x("","time_zone", "UCT4_2","selected"); %>			>(GMT-04:00) <#TZ19#></option>
                                                    <option value="NST3.30NDT" <% nvram_match_x("","time_zone", "NST3.30NDT","selected"); %>		>(GMT-03:30) <#TZ20#></option>
                                                    <option value="BRT3BRST" <% nvram_match_x("","time_zone", "BRT3BRST","selected"); %>		>(GMT-03:00) <#TZ21#></option>
                                                    <option value="UCT3" <% nvram_match_x("","time_zone", "UCT3","selected"); %>			>(GMT-03:00) <#TZ22#></option>
                                                    <option value="EBST3EBDT_2" <% nvram_match_x("","time_zone", "EBST3EBDT_2","selected"); %>		>(GMT-03:00) <#TZ23#></option>
                                                    <option value="UCT2" <% nvram_match_x("","time_zone", "UCT2","selected"); %>			>(GMT-02:00) <#TZ24#></option>
                                                    <option value="EUT1EUTDST" <% nvram_match_x("","time_zone", "EUT1EUTDST","selected"); %>		>(GMT-01:00) <#TZ25#></option>
                                                    <option value="UCT1" <% nvram_match_x("","time_zone", "UCT1","selected"); %>			>(GMT-01:00) <#TZ26#></option>
                                                    <option value="GMT0BST_1" <% nvram_match_x("","time_zone", "GMT0BST_1","selected"); %>		>(GMT) <#TZ27#></option>
                                                    <option value="UCT" <% nvram_match_x("","time_zone", "UCT","selected"); %>				>(GMT) <#TZ28#></option>
                                                    <option value="CET-1CEST" <% nvram_match_x("","time_zone", "CET-1CEST","selected"); %>		>(GMT+01:00) <#TZ29#></option>
                                                    <option value="CET-1CEST_1" <% nvram_match_x("","time_zone", "CET-1CEST_1","selected"); %>		>(GMT+01:00) <#TZ30#></option>
                                                    <option value="CET-1CEST_2" <% nvram_match_x("","time_zone", "CET-1CEST_2","selected"); %>		>(GMT+01:00) <#TZ31#></option>
                                                    <option value="CET-1CEST_3" <% nvram_match_x("","time_zone", "CET-1CEST_3","selected"); %>		>(GMT+01:00) <#TZ32#></option>
                                                    <option value="MET-1METDST" <% nvram_match_x("","time_zone", "MET-1METDST","selected"); %>		>(GMT+01:00) <#TZ33#></option>
                                                    <option value="MET-1METDST_1" <% nvram_match_x("","time_zone", "MET-1METDST_1","selected"); %>	>(GMT+01:00) <#TZ34#></option>
                                                    <option value="MEZ-1MESZ" <% nvram_match_x("","time_zone", "MEZ-1MESZ","selected"); %>		>(GMT+01:00) <#TZ35#></option>
                                                    <option value="MEZ-1MESZ_1" <% nvram_match_x("","time_zone", "MEZ-1MESZ_1","selected"); %>		>(GMT+01:00) <#TZ36#></option>
                                                    <option value="UCT-1_3" <% nvram_match_x("","time_zone", "UCT-1_3","selected"); %>			>(GMT+01:00) <#TZ37#></option>
                                                    <option value="EET-2EETDST_2" <% nvram_match_x("","time_zone", "EET-2EETDST_2","selected"); %>	>(GMT+02:00) <#TZ38#></option>
                                                    <option value="EST-2EDT" <% nvram_match_x("","time_zone", "EST-2EDT","selected"); %>		>(GMT+02:00) <#TZ39#></option>
                                                    <option value="EET-2EETDST_1" <% nvram_match_x("","time_zone", "EET-2EETDST_1","selected"); %>	>(GMT+02:00) <#TZ40#></option>
                                                    <option value="EET-2EETDST_3" <% nvram_match_x("","time_zone", "EET-2EETDST_3","selected"); %>	>(GMT+02:00) <#TZ41#></option>
                                                    <option value="IST-2IDT" <% nvram_match_x("","time_zone", "IST-2IDT","selected"); %>		>(GMT+02:00) <#TZ42#></option>
                                                    <option value="SAST-2" <% nvram_match_x("","time_zone", "SAST-2","selected"); %>			>(GMT+02:00) <#TZ43#></option>
                                                    <option value="EET-2EETDST" <% nvram_match_x("","time_zone", "EET-2EETDST","selected"); %>		>(GMT+02:00) <#TZ43_2#></option>
                                                    <option value="UTC-2" <% nvram_match_x("","time_zone", "UTC-2","selected"); %>			>(GMT+02:00) <#TZ43_3#></option>
                                                    <option value="UTC-3" <% nvram_match_x("","time_zone", "UTC-3","selected"); %>			>(GMT+03:00) <#TZ44#></option>
                                                    <option value="UCT-3_1" <% nvram_match_x("","time_zone", "UCT-3_1","selected"); %>			>(GMT+03:00) <#TZ46#></option>
                                                    <option value="UCT-3_2" <% nvram_match_x("","time_zone", "UCT-3_2","selected"); %>			>(GMT+03:00) <#TZ47#></option>
                                                    <option value="IST-3IDT" <% nvram_match_x("","time_zone", "IST-3IDT","selected"); %>		>(GMT+03:00) <#TZ48#></option>
                                                    <option value="UCT-3.30" <% nvram_match_x("","time_zone", "UCT-3.30","selected"); %>		>(GMT+03:30) <#TZ49#></option>
                                                    <option value="UTC-4" <% nvram_match_x("","time_zone", "UTC-4","selected"); %>			>(GMT+04:00) <#TZ45#></option>
                                                    <option value="UCT-4_1" <% nvram_match_x("","time_zone", "UCT-4_1","selected"); %>			>(GMT+04:00) <#TZ50#></option>
                                                    <option value="UCT-4_2" <% nvram_match_x("","time_zone", "UCT-4_2","selected"); %>			>(GMT+04:00) <#TZ51#></option>
                                                    <option value="UCT-4.30" <% nvram_match_x("","time_zone", "UCT-4.30","selected"); %>		>(GMT+04:30) <#TZ52#></option>
                                                    <option value="UTC-5" <% nvram_match_x("","time_zone", "UTC-5","selected"); %>			>(GMT+05:00) <#TZ53#></option>
                                                    <option value="UCT-5" <% nvram_match_x("","time_zone", "UCT-5","selected"); %>			>(GMT+05:00) <#TZ54#></option>
                                                    <option value="UCT-5.30" <% nvram_match_x("","time_zone", "UCT-5.30","selected"); %>		>(GMT+05:30) <#TZ59#></option>
                                                    <option value="UCT-5.30_2" <% nvram_match_x("","time_zone", "UCT-5.30_2","selected"); %>		>(GMT+05:30) <#TZ55#></option>
                                                    <option value="UCT-5.30_1" <% nvram_match_x("","time_zone", "UCT-5.30_1","selected"); %>		>(GMT+05:30) <#TZ56#></option>
                                                    <option value="UCT-5.45" <% nvram_match_x("","time_zone", "UCT-5.45","selected"); %>		>(GMT+05:45) <#TZ57#></option>
                                                    <option value="UTC-6_1" <% nvram_match_x("","time_zone", "UTC-6_1","selected"); %>			>(GMT+06:00) <#TZ60_2#></option>
                                                    <option value="UCT-6" <% nvram_match_x("","time_zone", "UCT-6","selected"); %>			>(GMT+06:00) <#TZ58#></option>
                                                    <option value="UCT-6.30" <% nvram_match_x("","time_zone", "UCT-6.30","selected"); %>		>(GMT+06:30) <#TZ61#></option>
                                                    <option value="UTC-7_1" <% nvram_match_x("","time_zone", "UTC-7_1","selected"); %>			>(GMT+07:00) <#TZ60#></option>
                                                    <option value="UTC-7" <% nvram_match_x("","time_zone", "UTC-7","selected"); %>			>(GMT+07:00) <#TZ63#></option>
                                                    <option value="UCT-7" <% nvram_match_x("","time_zone", "UCT-7","selected"); %>			>(GMT+07:00) <#TZ62#></option>
                                                    <option value="UTC-8" <% nvram_match_x("","time_zone", "UTC-8","selected"); %>			>(GMT+08:00) <#TZ69#></option>
                                                    <option value="UTC-8_1" <% nvram_match_x("","time_zone", "UTC-8_1","selected"); %>			>(GMT+08:00) <#TZ69_2#></option>
                                                    <option value="CST-8" <% nvram_match_x("","time_zone", "CST-8","selected"); %>			>(GMT+08:00) <#TZ64#></option>
                                                    <option value="CST-8_1" <% nvram_match_x("","time_zone", "CST-8_1","selected"); %>			>(GMT+08:00) <#TZ65#></option>
                                                    <option value="SST-8" <% nvram_match_x("","time_zone", "SST-8","selected"); %>			>(GMT+08:00) <#TZ66#></option>
                                                    <option value="CCT-8" <% nvram_match_x("","time_zone", "CCT-8","selected"); %>			>(GMT+08:00) <#TZ67#></option>
                                                    <option value="WAS-8WAD" <% nvram_match_x("","time_zone", "WAS-8WAD","selected"); %>		>(GMT+08:00) <#TZ68#></option>
                                                    <option value="UTC-9" <% nvram_match_x("","time_zone", "UTC-9","selected"); %>			>(GMT+09:00) <#TZ72#></option>
                                                    <option value="UCT-9_1" <% nvram_match_x("","time_zone", "UCT-9_1","selected"); %>			>(GMT+09:00) <#TZ70#></option>
                                                    <option value="JST" <% nvram_match_x("","time_zone", "JST","selected"); %>				>(GMT+09:00) <#TZ71#></option>
                                                    <option value="CST-9.30CDT" <% nvram_match_x("","time_zone", "CST-9.30CDT","selected"); %>		>(GMT+09:30) <#TZ73#></option>
                                                    <option value="UCT-9.30" <% nvram_match_x("","time_zone", "UCT-9.30","selected"); %>		>(GMT+09:30) <#TZ74#></option>
                                                    <option value="UTC-10" <% nvram_match_x("","time_zone", "UTC-10","selected"); %>			>(GMT+10:00) <#TZ78#></option>
                                                    <option value="EST-10EDT" <% nvram_match_x("","time_zone", "EST-10EDT","selected"); %>		>(GMT+10:00) <#TZ75#></option>
                                                    <option value="UCT-10_2" <% nvram_match_x("","time_zone", "UCT-10_2","selected"); %>		>(GMT+10:00) <#TZ76#></option>
                                                    <option value="TST-10TDT" <% nvram_match_x("","time_zone", "TST-10TDT","selected"); %>		>(GMT+10:00) <#TZ77#></option>
                                                    <option value="UCT-10_5" <% nvram_match_x("","time_zone", "UCT-10_5","selected"); %>		>(GMT+10:00) <#TZ79#></option>
                                                    <option value="UTC-11" <% nvram_match_x("","time_zone", "UTC-11","selected"); %>			>(GMT+11:00) <#TZ80_2#></option>
                                                    <option value="UCT-11" <% nvram_match_x("","time_zone", "UCT-11","selected"); %>			>(GMT+11:00) <#TZ80#></option>
                                                    <option value="UCT-11_1" <% nvram_match_x("","time_zone", "UCT-11_1","selected"); %>		>(GMT+11:00) <#TZ81#></option>
                                                    <option value="UTC-12" <% nvram_match_x("","time_zone", "UTC-12","selected"); %>			>(GMT+12:00) <#TZ81_2#></option>
                                                    <option value="UCT-12" <% nvram_match_x("","time_zone", "UCT-12","selected"); %>			>(GMT+12:00) <#TZ82#></option>
                                                    <option value="NZST-12NZDT" <% nvram_match_x("","time_zone", "NZST-12NZDT","selected"); %>		>(GMT+12:00) <#TZ83#></option>
                                                    <option value="UCT-13" <% nvram_match_x("","time_zone", "UCT-13","selected"); %>			>(GMT+13:00) <#TZ84#></option>
                                                </select>
                                                <span id="timezone_hint" style="display:none;"></span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_System_ntpp#></th>
                                            <td>
                                                <select name="ntp_period" class="input">
                                                    <option value="6"   <% nvram_match_x("","ntp_period",  "6","selected"); %>>6 hours</option>
                                                    <option value="12"  <% nvram_match_x("","ntp_period", "12","selected"); %>>12 hours</option>
                                                    <option value="24"  <% nvram_match_x("","ntp_period", "24","selected"); %>>1 day (*)</option>
                                                    <option value="48"  <% nvram_match_x("","ntp_period", "48","selected"); %>>2 days</option>
                                                    <option value="72"  <% nvram_match_x("","ntp_period", "72","selected"); %>>3 days</option>
                                                    <option value="168" <% nvram_match_x("","ntp_period","168","selected"); %>>1 week</option>
                                                    <option value="336" <% nvram_match_x("","ntp_period","336","selected"); %>>2 weeks</option>
                                                    <option value="0"   <% nvram_match_x("","ntp_period",  "0","selected"); %>><#btn_Disable#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,11,3)"><#LANHostConfig_x_NTPServer1_itemname#> 1:</a></th>
                                            <td>
                                                <input type="text" maxlength="128" class="input" size="32" name="ntp_server0" value="<% nvram_get_x("","ntp_server0"); %>" onKeyPress="return is_string(this,event);"/>
                                                <a href="javascript:openLink('x_NTPServer1')" class="label label-info" name="x_NTPServer1_link"><#LANHostConfig_x_NTPServer1_linkname#></a>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,11,3)"><#LANHostConfig_x_NTPServer1_itemname#> 2:</a></th>
                                            <td>
                                                <input type="text" maxlength="128" class="input" size="32" name="ntp_server1" value="<% nvram_get_x("","ntp_server1"); %>" onKeyPress="return is_string(this,event);"/>
                                            </td>
                                        </tr>

                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#t2Misc#></th>
                                        </tr>
										<tr id="reboot_schedule_enable_tr">
				                        <tr>
                                            <th><#reboot_schedule_enable#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="reboot_schedule_enable_on_of">
                                                        <input type="checkbox" id="reboot_schedule_enable_fake" <% nvram_match_x("", "reboot_schedule_enable", "1", "value=1 checked"); %><% nvram_match_x("", "reboot_schedule_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="reboot_schedule_enable_x" id="reboot_schedule_enable_1" class="input" value="1" <% nvram_match_x("", "reboot_schedule_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="reboot_schedule_enable_x" id="reboot_schedule_enable_0" class="input" value="0" <% nvram_match_x("", "reboot_schedule_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
				<tr id="reboot_schedule_date_tr">
					<th><#reboot_schedule_date#></th>
					<td>
						<input type="checkbox" name="reboot_date_x_Sun" class="input" onclick="check_Timefield_checkbox();"><#DAY_Sun#>
						<input type="checkbox" name="reboot_date_x_Mon" class="input" onclick="check_Timefield_checkbox();"><#DAY_Mon#>
						<input type="checkbox" name="reboot_date_x_Tue" class="input" onclick="check_Timefield_checkbox();"><#DAY_Tue#>
						<input type="checkbox" name="reboot_date_x_Wed" class="input" onclick="check_Timefield_checkbox();"><#DAY_Wed#>
						<input type="checkbox" name="reboot_date_x_Thu" class="input" onclick="check_Timefield_checkbox();"><#DAY_Thu#>
						<input type="checkbox" name="reboot_date_x_Fri" class="input" onclick="check_Timefield_checkbox();"><#DAY_Fri#>
						<input type="checkbox" name="reboot_date_x_Sat" class="input" onclick="check_Timefield_checkbox();"><#DAY_Sat#>
					</td>
				</tr>
				<tr id="reboot_schedule_time_tr">
					<th><#reboot_schedule_time#></th>
					<td>
						<input type="text" maxlength="2" class="input_3_table" style="width: 30px" name="reboot_time_x_hour" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 0);" autocorrect="off" autocapitalize="off"><#Hour#>
						<input type="text" maxlength="2" class="input_3_table" style="width: 30px" name="reboot_time_x_min" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 1);" autocorrect="off" autocapitalize="off"><#Minute#>
					</td>
				</tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,11,1)"><#LANHostConfig_x_ServerLogEnable_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="log_ipaddr" style="width: 145px" value="<% nvram_get_x("", "log_ipaddr"); %>" onKeyPress="return is_ipaddr(this,event);" />&nbsp;:
                                                <input type="text" maxlength="5" class="input" size="10" name="log_port" style="width: 44px;"  value="<% nvram_get_x("","log_port"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_System_logf#></th>
                                            <td>
                                                <select name="log_float_ui" class="input">
                                                    <option value="0" <% nvram_match_x("", "log_float_ui", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "log_float_ui", "1","selected"); %>><#checkbox_Yes#> (*)</option>
                                                    <option value="2" <% nvram_match_x("", "log_float_ui", "2","selected"); %>><#Adm_System_logf_item2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#PASS_LANG#></th>
                                            <td>
                                                <select name="select_lang" id="select_lang" onchange="submit_language();">
                                                    <% shown_language_option(); %>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_System_help#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="help_enable_on_of">
                                                        <input type="checkbox" id="help_enable_fake" <% nvram_match_x("General", "help_enable", "1", "value=1 checked"); %><% nvram_match_x("General", "help_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="help_enable" id="help_enable_1" class="input" value="1" <% nvram_match_x("General", "help_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="help_enable" id="help_enable_0" class="input" value="0" <% nvram_match_x("General", "help_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td style="border: 0 none;">
                                                <center><input class="btn btn-primary" style="width: 219px" onclick="applyRule();" type="button" value="<#CTL_apply#>" /></center>
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

    <div id="footer"></div>
</div>
</body>
</html>
