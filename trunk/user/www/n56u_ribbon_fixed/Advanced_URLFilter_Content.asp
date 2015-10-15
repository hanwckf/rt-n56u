<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_5_2#></title>
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
<script type="text/javascript" src="/client_function.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('url_enable', change_url_enable);
});

</script>
<script>

var ipmonitor = [<% get_static_client(); %>];
var wireless = [<% wl_auth_list(); %>];

var clients_info = getclients(1,0);

var isMenuopen = 0;

function initial(){
	show_banner(1);
	show_menu(5,5,3);
	show_footer();

	change_url_enable();

	document.form.url_date_x_Sun.checked = getDateCheck(document.form.url_date_x.value, 0);
	document.form.url_date_x_Mon.checked = getDateCheck(document.form.url_date_x.value, 1);
	document.form.url_date_x_Tue.checked = getDateCheck(document.form.url_date_x.value, 2);
	document.form.url_date_x_Wed.checked = getDateCheck(document.form.url_date_x.value, 3);
	document.form.url_date_x_Thu.checked = getDateCheck(document.form.url_date_x.value, 4);
	document.form.url_date_x_Fri.checked = getDateCheck(document.form.url_date_x.value, 5);
	document.form.url_date_x_Sat.checked = getDateCheck(document.form.url_date_x.value, 6);

	document.form.url_time_x_starthour.value = getTimeRange(document.form.url_time_x.value, 0);
	document.form.url_time_x_startmin.value = getTimeRange(document.form.url_time_x.value, 1);
	document.form.url_time_x_endhour.value = getTimeRange(document.form.url_time_x.value, 2);
	document.form.url_time_x_endmin.value = getTimeRange(document.form.url_time_x.value, 3);

	showLANIPList();

	load_body();
}

function applyRule(){
	if(validForm()){
		document.form.url_date_x.value = setDateCheck(
			document.form.url_date_x_Sun,
			document.form.url_date_x_Mon,
			document.form.url_date_x_Tue,
			document.form.url_date_x_Wed,
			document.form.url_date_x_Thu,
			document.form.url_date_x_Fri,
			document.form.url_date_x_Sat);
		document.form.url_time_x.value = setTimeRange(
			document.form.url_time_x_starthour,
			document.form.url_time_x_startmin,
			document.form.url_time_x_endhour,
			document.form.url_time_x_endmin);
		
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_URLFilter_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if((document.form.url_enable_x[0].checked ==true)
		&& (document.form.url_date_x_Sun.checked ==false)
		&& (document.form.url_date_x_Mon.checked ==false)
		&& (document.form.url_date_x_Tue.checked ==false)
		&& (document.form.url_date_x_Wed.checked ==false)
		&& (document.form.url_date_x_Thu.checked ==false)
		&& (document.form.url_date_x_Fri.checked ==false)
		&& (document.form.url_date_x_Sat.checked ==false)){
			alert("<#FirewallConfig_URLActiveDate_itemname#><#JS_fieldblank#>");
			return false;
	}

	if(document.form.url_enable_x[0].checked ==true){
		if(!validate_timerange(document.form.url_time_x_starthour, 0)
			|| !validate_timerange(document.form.url_time_x_startmin, 1)
			|| !validate_timerange(document.form.url_time_x_endhour, 2)
			|| !validate_timerange(document.form.url_time_x_endmin, 3)
			){
			return false;
		}
		
		var starttime = eval(document.form.url_time_x_starthour.value + document.form.url_time_x_startmin.value);
		var endtime = eval(document.form.url_time_x_endhour.value + document.form.url_time_x_endmin.value);
		if(starttime == endtime){
			alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
			document.form.url_time_x_starthour.focus();
			document.form.url_time_x_starthour.select();
			return false;
		}
	}

	if (!validate_hwaddr(document.form.url_mac_x))
		return false;

	return true;
}

function change_url_enable(){
	var v = document.form.url_enable_x[0].checked;
	showhide_div('tbl_urlf_main', v);
}

function click_mac_inv(o){
	document.form.url_inv_x.value = (o.checked) ? "1" : "0";
}

function setClientMAC(num){
	document.form.url_mac_x.value = clients_info[num][2];
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
		document.form.url_mac_x.focus();
		isMenuopen = 1;
	}
	else
		hideClients_Block();
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
    <input type="hidden" name="current_page" value="Advanced_URLFilter_Content.asp">
    <input type="hidden" name="next_page" value="Advanced_URLFilter_Content.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="FirewallConfig;">
    <input type="hidden" name="group_id" value="UrlList">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

    <input type="hidden" name="url_date_x" value="<% nvram_get_x("","url_date_x"); %>">
    <input type="hidden" name="url_time_x" value="<% nvram_get_x("","url_time_x"); %>">
    <input type="hidden" name="url_inv_x" value="<% nvram_get_x("", "url_inv_x"); %>" />
    <input type="hidden" name="url_num_x_0" value="<% nvram_get_x("","url_num_x"); %>" readonly="1">

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
                            <h2 class="box_head round_top"><#menu5_5#> - <#menu5_5_2#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#FirewallConfig_UrlFilterEnable_sectiondesc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="padding-bottom: 0px; border-top: 0 none;"><#FirewallConfig_UrlFilterEnable_itemname#>?</th>
                                            <td style="padding-bottom: 0px; border-top: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="url_enable_on_of">
                                                        <input type="checkbox" id="url_enable_fake" <% nvram_match_x("", "url_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "url_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="url_enable_x" id="url_enable_1" onClick="change_url_enable();" <% nvram_match_x("","url_enable_x", "1", "checked"); %>><#CTL_Enabled#>
                                                    <input type="radio" value="0" name="url_enable_x" id="url_enable_0" onClick="change_url_enable();" <% nvram_match_x("","url_enable_x", "0", "checked"); %>><#CTL_Disabled#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_urlf_main" style="display:none">
                                        <tr>
                                            <th colspan="3" style="background-color: #E3E3E3;"><#menu5_5_2#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,9,1);"><#FirewallConfig_URLActiveDate_itemname#></a></th>
                                            <td colspan="2">
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Mon" class="input" onChange="return changeDate();"><#DAY_Mon#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Tue" class="input" onChange="return changeDate();"><#DAY_Tue#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Wed" class="input" onChange="return changeDate();"><#DAY_Wed#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Thu" class="input" onChange="return changeDate();"><#DAY_Thu#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Fri" class="input" onChange="return changeDate();"><#DAY_Fri#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Sat" class="input" onChange="return changeDate();"><#DAY_Sat#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Sun" class="input" onChange="return changeDate();"><#DAY_Sun#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,9,2);"><#FirewallConfig_URLActiveTime_itemname#>:</a></th>
                                            <td colspan="2">
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_starthour" onKeyPress="return is_number(this,event);">:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_startmin" onKeyPress="return is_number(this,event);">-
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_endhour" onKeyPress="return is_number(this,event);">:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_endmin" onKeyPress="return is_number(this,event);">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#FirewallConfig_UrlMAC#>:</th>
                                            <td colspan="2" align="left">
                                                <div id="ClientList_Block" class="alert alert-info ddown-list"></div>
                                                <div class="input-append">
                                                    <input type="text" maxlength="12" class="span12" size="15" name="url_mac_x" value="<% nvram_get_x("","url_mac_x"); %>" onKeyPress="return is_hwaddr(event);" style="float:left; width: 162px">
                                                    <button class="btn btn-chevron" id="chevron" type="button" onclick="pullLANIPList(this);" title="Select the MAC of LAN clients."><i class="icon icon-chevron-down"></i></button>&nbsp;
                                                    <label class="checkbox inline"><input type="checkbox" name="url_inv_fake" value="" onclick="click_mac_inv(this);" <% nvram_match_x("", "url_inv_x", "1", "checked"); %>/><#FirewallConfig_UrlInv#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#FirewallConfig_UrlList_groupitemdesc#></th>
                                            <td style="padding-right: 0px;">
                                                <input type="text" class="span12" maxlength="96" size="36" name="url_keyword_x_0" value="<% nvram_get_x("", "url_keyword_x_0"); %>" onKeyPress="return is_string(this,event);">
                                            </td>
                                            <td align="left">
                                                <button class="btn" type="submit" onClick="if(validForm()){return markGroup(this, 'UrlList', 128, ' Add ');}" name="UrlList"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>&nbsp;</th>
                                            <td style="padding-right: 0px;">
                                                <select size="8" class="span12" name="UrlList_s" multiple="true" style="vertical-align:middle;">
                                                    <% nvram_get_table_x("FirewallConfig","UrlList"); %>
                                                </select>
                                            </td>
                                            <td style="vertical-align:top">
                                                <button class="btn btn-danger" type="submit" onClick="return markGroup(this, 'UrlList', 128, ' Del ');" name="UrlList"><i class="icon icon-minus icon-white"></i></button>
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
