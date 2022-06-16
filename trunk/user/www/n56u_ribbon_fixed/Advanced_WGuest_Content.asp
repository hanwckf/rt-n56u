<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - 5G <#menu5_1_2#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<style>
.table th, .table td{vertical-align: middle;}
.table input, .table select {margin-bottom: 0px;}
</style>

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/itoggle.js"></script>
<script type="text/javascript" src="/wireless.js"></script>
<script type="text/javascript" src="/help_wl.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('wl_guest_enable', change_guest_enabled); // change_guest_enabled(1) change_guest_enabled(1)
	init_itoggle('wl_guest_closed');
	init_itoggle('wl_guest_ap_isolate');
	init_itoggle('wl_guest_lan_isolate');
	init_itoggle('wl_guest_macrule');
});

</script>
<script>

function initial(){
	show_banner(1);
	show_menu(5,2,2);
	show_footer();

	if (!support_5g_11ac()){
		o1 = document.form.wl_guest_mcs_mode;
		o1.remove(1);
		o1.remove(1);
		o1.remove(1);
	}

	document.form.wl_guest_date_x_Sun.checked = getDateCheck(document.form.wl_guest_date_x.value, 0);
	document.form.wl_guest_date_x_Mon.checked = getDateCheck(document.form.wl_guest_date_x.value, 1);
	document.form.wl_guest_date_x_Tue.checked = getDateCheck(document.form.wl_guest_date_x.value, 2);
	document.form.wl_guest_date_x_Wed.checked = getDateCheck(document.form.wl_guest_date_x.value, 3);
	document.form.wl_guest_date_x_Thu.checked = getDateCheck(document.form.wl_guest_date_x.value, 4);
	document.form.wl_guest_date_x_Fri.checked = getDateCheck(document.form.wl_guest_date_x.value, 5);
	document.form.wl_guest_date_x_Sat.checked = getDateCheck(document.form.wl_guest_date_x.value, 6);
	document.form.wl_guest_time_x_starthour.value = getTimeRange(document.form.wl_guest_time_x.value, 0);
	document.form.wl_guest_time_x_startmin.value = getTimeRange(document.form.wl_guest_time_x.value, 1);
	document.form.wl_guest_time_x_endhour.value = getTimeRange(document.form.wl_guest_time_x.value, 2);
	document.form.wl_guest_time_x_endmin.value = getTimeRange(document.form.wl_guest_time_x.value, 3);
	document.form.wl_guest_time2_x_starthour.value = getTimeRange(document.form.wl_guest_time2_x.value, 0);
	document.form.wl_guest_time2_x_startmin.value = getTimeRange(document.form.wl_guest_time2_x.value, 1);
	document.form.wl_guest_time2_x_endhour.value = getTimeRange(document.form.wl_guest_time2_x.value, 2);
	document.form.wl_guest_time2_x_endmin.value = getTimeRange(document.form.wl_guest_time2_x.value, 3);

	document.form.wl_guest_ssid.value = decodeURIComponent(document.form.wl_guest_ssid_org.value);
	document.form.wl_guest_wpa_psk.value = decodeURIComponent(document.form.wl_guest_wpa_psk_org.value);

	if (get_ap_mode())
		$("col_isolate").innerHTML = "<#WIFIGuestIsolate#>";

	load_body();

	change_guest_enabled(0);
	change_guest_auth_mode(0);
}

function applyRule(){
	if(validForm()){
		document.form.wl_guest_date_x.value = setDateCheck(
		    document.form.wl_guest_date_x_Sun,
		    document.form.wl_guest_date_x_Mon,
		    document.form.wl_guest_date_x_Tue,
		    document.form.wl_guest_date_x_Wed,
		    document.form.wl_guest_date_x_Thu,
		    document.form.wl_guest_date_x_Fri,
		    document.form.wl_guest_date_x_Sat);
		document.form.wl_guest_time_x.value = setTimeRange(
		    document.form.wl_guest_time_x_starthour,
		    document.form.wl_guest_time_x_startmin,
		    document.form.wl_guest_time_x_endhour,
		    document.form.wl_guest_time_x_endmin);
		document.form.wl_guest_time2_x.value = setTimeRange(
		    document.form.wl_guest_time2_x_starthour,
		    document.form.wl_guest_time2_x_startmin,
		    document.form.wl_guest_time2_x_endhour,
		    document.form.wl_guest_time2_x_endmin);
		
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WGuest_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	if (!document.form.wl_guest_enable[0].checked)
		return true;

	var mode = document.form.wl_guest_auth_mode.value;

	if(!validate_string_ssid(document.form.wl_guest_ssid))
		return false;

	if(!validate_timerange(document.form.wl_guest_time_x_starthour, 0)
			|| !validate_timerange(document.form.wl_guest_time_x_startmin, 1)
			|| !validate_timerange(document.form.wl_guest_time_x_endhour, 2)
			|| !validate_timerange(document.form.wl_guest_time_x_endmin, 3)
			)
		return false;

	var starttime = eval(document.form.wl_guest_time_x_starthour.value + document.form.wl_guest_time_x_startmin.value);
	var endtime = eval(document.form.wl_guest_time_x_endhour.value + document.form.wl_guest_time_x_endmin.value);
	if(starttime == endtime){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
			document.form.wl_guest_time_x_starthour.focus();
			document.form.wl_guest_time_x_starthour.select;
		return false;
	}

	if(!validate_timerange(document.form.wl_guest_time2_x_starthour, 0)
			|| !validate_timerange(document.form.wl_guest_time2_x_startmin, 1)
			|| !validate_timerange(document.form.wl_guest_time2_x_endhour, 2)
			|| !validate_timerange(document.form.wl_guest_time2_x_endmin, 3)
			)
		return false;

	var starttime2 = eval(document.form.wl_guest_time2_x_starthour.value + document.form.wl_guest_time2_x_startmin.value);
	var endtime2 = eval(document.form.wl_guest_time2_x_endhour.value + document.form.wl_guest_time2_x_endmin.value);
	if(starttime2 == endtime2){
		alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
			document.form.wl_guest_time2_x_starthour.focus();
			document.form.wl_guest_time2_x_starthour.select;
		return false;
	}

	if(document.form.wl_guest_ssid.value == "") {
		document.form.wl_guest_ssid.focus();
		return false;
	}

	if(mode == "psk"){
		if(!validate_psk(document.form.wl_guest_wpa_psk))
			return false;
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

function change_guest_enabled(mflag) {
	var v = document.form.wl_guest_enable[0].checked;
	showhide_div('row_guest_1', v);
	showhide_div('row_guest_2', v);
	showhide_div('row_guest_3', v);
	showhide_div('row_guest_4', v);
	showhide_div('row_guest_5', v);
	showhide_div('row_guest_6', v);
	if (support_lan_ap_isolate()) {
		showhide_div('row_guest_7', v);
	}
	showhide_div('row_guest_8', v);
	showhide_div('row_guest_9', v);
	showhide_div('row_guest_10', v);
	showhide_div('row_guest_11', v);
	showhide_div('row_guest_12', v);
	showhide_div('row_guest_13', v);
}

function change_guest_auth_mode(mflag) {
	var mode = document.form.wl_guest_auth_mode.value;
	var opts = document.form.wl_guest_auth_mode.options;
	
	if (mode == "psk")
	{
		inputCtrl(document.form.wl_guest_crypto, 1);
		inputCtrl(document.form.wl_guest_wpa_psk, 1);
		
		if(opts[opts.selectedIndex].text == "WPA2-Personal")
		{
			if (mflag == 1) {
				document.form.wl_guest_crypto.options[2].selected = 0;
				document.form.wl_guest_crypto.options[0].selected = 0;
				document.form.wl_guest_crypto.options[1].selected = 1;
				document.form.wl_guest_wpa_mode.value = "2";
			}
		}
		else if(opts[opts.selectedIndex].text == "WPA-Personal")
		{
			if (mflag == 1) {
				document.form.wl_guest_crypto.options[2].selected = 0;
				document.form.wl_guest_crypto.options[1].selected = 0;
				document.form.wl_guest_crypto.options[0].selected = 1;
				document.form.wl_guest_wpa_mode.value = "1";
			}
		}
		else
		{
			if (mflag == 1) {
				document.form.wl_guest_crypto.options[1].selected = 0;
				document.form.wl_guest_crypto.options[0].selected = 0;
				document.form.wl_guest_crypto.options[2].selected = 1;
				document.form.wl_guest_wpa_mode.value = "0";
			}
		}
	}
	else
	{
		inputCtrl(document.form.wl_guest_crypto, 0);
		inputCtrl(document.form.wl_guest_wpa_psk, 0);
	}
}
</script>
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
    <iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
    <form method="post" name="form" action="/start_apply.htm" target="hidden_frame">

    <input type="hidden" name="current_page" value="Advanced_WGuest_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="WLANConfig11a;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

    <input type="hidden" name="wl_gmode" value="<% nvram_get_x("","wl_gmode"); %>" readonly="1">
    <input type="hidden" name="wl_country_code" value="<% nvram_get_x("","wl_country_code"); %>">
    <input type="hidden" name="wl_guest_ssid_org" value="<% nvram_char_to_ascii("", "wl_guest_ssid"); %>">
    <input type="hidden" name="wl_guest_wpa_mode" value="<% nvram_get_x("","wl_guest_wpa_mode"); %>">
    <input type="hidden" name="wl_guest_wpa_psk_org" value="<% nvram_char_to_ascii("", "wl_guest_wpa_psk"); %>">
    <input type="hidden" name="wl_guest_date_x" value="<% nvram_get_x("","wl_guest_date_x"); %>">
    <input type="hidden" name="wl_guest_time_x" value="<% nvram_get_x("","wl_guest_time_x"); %>">
    <input type="hidden" name="wl_guest_time2_x" value="<% nvram_get_x("","wl_guest_time2_x"); %>">

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
                            <h2 class="box_head round_top"><#menu5_1#> - <#menu5_1_2#> (5GHz)</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;"><#WIFIGuestEnable#></th>
                                            <td style="border-top: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="wl_guest_enable_on_of">
                                                        <input type="checkbox" id="wl_guest_enable_fake" <% nvram_match_x("", "wl_guest_enable", "1", "value=1 checked"); %><% nvram_match_x("", "wl_guest_enable", "0", "value=0"); %> />
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="wl_guest_enable_1" name="wl_guest_enable" class="input" onClick="change_guest_enabled(1);" <% nvram_match_x("","wl_guest_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="wl_guest_enable_0" name="wl_guest_enable" class="input" onClick="change_guest_enabled(1);" <% nvram_match_x("","wl_guest_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_1" style="display:none;">
                                            <th><#WIFIGuestDate#></th>
                                            <td>
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="wl_guest_date_x_Mon" onChange="return changeDate();"/><#DAY_Mon#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="wl_guest_date_x_Tue" onChange="return changeDate();"/><#DAY_Tue#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="wl_guest_date_x_Wed" onChange="return changeDate();"/><#DAY_Wed#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="wl_guest_date_x_Thu" onChange="return changeDate();"/><#DAY_Thu#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="wl_guest_date_x_Fri" onChange="return changeDate();"/><#DAY_Fri#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_2" style="display:none;">
                                            <th style="border-top: 0 none;"><#WIFIGuestTime#></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time_x_starthour" onKeyPress="return is_number(this,event);">:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time_x_startmin" onKeyPress="return is_number(this,event);">&nbsp;-&nbsp;
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time_x_endhour" onKeyPress="return is_number(this,event);">:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time_x_endmin" onKeyPress="return is_number(this,event);">
                                            </td>
                                        </tr>
                                        <tr id="row_guest_3" style="display:none;">
                                            <th><#WIFIGuestDate2#></th>
                                            <td>
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="wl_guest_date_x_Sat" onChange="return changeDate();"/><#DAY_Sat#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="wl_guest_date_x_Sun" onChange="return changeDate();"/><#DAY_Sun#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_4" style="display:none;">
                                            <th style="border-top: 0 none;"><#WIFIGuestTime2#></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time2_x_starthour" onKeyPress="return is_number(this,event);">:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time2_x_startmin" onKeyPress="return is_number(this,event);">&nbsp;-&nbsp;
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time2_x_endhour" onKeyPress="return is_number(this,event);">:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="wl_guest_time2_x_endmin" onKeyPress="return is_number(this,event);">
                                            </td>
                                        </tr>
                                        <tr id="row_guest_5" style="display:none;">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 1);"><#WIFIGuestSSID#></a></th>
                                            <td><input type="text" maxlength="32" class="input" size="32" name="wl_guest_ssid" value="" onkeypress="return is_string(this,event);"/></td>
                                        </tr>
                                        <tr id="row_guest_6" style="display:none;">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_guest_closed_on_of">
                                                        <input type="checkbox" id="wl_guest_closed_fake" <% nvram_match_x("", "wl_guest_closed", "1", "value=1 checked"); %><% nvram_match_x("", "wl_guest_closed", "0", "value=0"); %> />
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="wl_guest_closed_1" name="wl_guest_closed" class="input" <% nvram_match_x("","wl_guest_closed", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="wl_guest_closed_0" name="wl_guest_closed" class="input" <% nvram_match_x("","wl_guest_closed", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_7" style="display:none;">
                                            <th id="col_isolate"><#WIFIGuestIsoLAN#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_guest_lan_isolate_on_of">
                                                        <input type="checkbox" id="wl_guest_lan_isolate_fake" <% nvram_match_x("", "wl_guest_lan_isolate", "1", "value=1 checked"); %><% nvram_match_x("", "wl_guest_lan_isolate", "0", "value=0"); %> />
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="wl_guest_lan_isolate_1" name="wl_guest_lan_isolate" class="input" <% nvram_match_x("","wl_guest_lan_isolate", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="wl_guest_lan_isolate_0" name="wl_guest_lan_isolate" class="input" <% nvram_match_x("","wl_guest_lan_isolate", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_8" style="display:none;">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 5);"><#WLANConfig11b_x_IsolateAP_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_guest_ap_isolate_on_of">
                                                        <input type="checkbox" id="wl_guest_ap_isolate_fake" <% nvram_match_x("", "wl_guest_ap_isolate", "1", "value=1 checked"); %><% nvram_match_x("", "wl_guest_ap_isolate", "0", "value=0"); %> />
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="wl_guest_ap_isolate_1" name="wl_guest_ap_isolate" class="input" <% nvram_match_x("","wl_guest_ap_isolate", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="wl_guest_ap_isolate_0" name="wl_guest_ap_isolate" class="input" <% nvram_match_x("","wl_guest_ap_isolate", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_9" style="display:none;">
                                            <th><#WIFIGuestMCS#></th>
                                            <td>
                                                <select name="wl_guest_mcs_mode" class="input">
                                                    <option value="0" <% nvram_match_x("", "wl_guest_mcs_mode", "0", "selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="7" <% nvram_match_x("", "wl_guest_mcs_mode", "7", "selected"); %>>VHT (1S) 98 Mbps</option>
                                                    <option value="8" <% nvram_match_x("", "wl_guest_mcs_mode", "8", "selected"); %>>VHT (1S) 65 Mbps</option>
                                                    <option value="9" <% nvram_match_x("", "wl_guest_mcs_mode", "9", "selected"); %>>VHT (1S) 33 Mbps</option>
                                                    <option value="1" <% nvram_match_x("", "wl_guest_mcs_mode", "1", "selected"); %>>HTMIX (1S) 45 Mbps</option>
                                                    <option value="2" <% nvram_match_x("", "wl_guest_mcs_mode", "2", "selected"); %>>HTMIX (1S) 30 Mbps</option>
                                                    <option value="3" <% nvram_match_x("", "wl_guest_mcs_mode", "3", "selected"); %>>HTMIX (1S) 15 Mbps</option>
                                                    <option value="4" <% nvram_match_x("", "wl_guest_mcs_mode", "4", "selected"); %>>OFDM 12 Mbps</option>
                                                    <option value="5" <% nvram_match_x("", "wl_guest_mcs_mode", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                    <option value="6" <% nvram_match_x("", "wl_guest_mcs_mode", "6", "selected"); %>>OFDM 6 Mbps</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_10" style="display:none;">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
                                            <td>
                                                <select name="wl_guest_auth_mode" class="input" onChange="change_guest_auth_mode(1);">
                                                    <option value="open" <% nvram_match_x("", "wl_guest_auth_mode", "open", "selected"); %>>Open System</option>
                                                    <option value="psk" <% nvram_double_match_x("", "wl_guest_auth_mode", "psk", "", "wl_guest_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
                                                    <option value="psk" <% nvram_double_match_x("", "wl_guest_auth_mode", "psk", "", "wl_guest_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
                                                    <option value="psk" <% nvram_double_match_x("", "wl_guest_auth_mode", "psk", "", "wl_guest_wpa_mode", "0", "selected"); %>>WPA-Auto-Personal</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_11" style="display:none;">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
                                            <td>
                                                <select name="wl_guest_crypto" class="input">
                                                    <option value="tkip" <% nvram_match_x("", "wl_guest_crypto", "tkip", "selected"); %>>TKIP</option>
                                                    <option value="aes" <% nvram_match_x("", "wl_guest_crypto", "aes", "selected"); %>>AES</option>
                                                    <option value="tkip+aes" <% nvram_match_x("", "wl_guest_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_12" style="display:none;">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" name="wl_guest_wpa_psk" id="wl_guest_wpa_psk" maxlength="64" size="32" value="" style="width: 175px;">
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('wl_guest_wpa_psk');"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_guest_13" style="display:none;">
                                            <th><#WIFIGuestMAC#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_guest_macrule_on_of">
                                                        <input type="checkbox" id="wl_guest_macrule_fake" <% nvram_match_x("", "wl_guest_macrule", "1", "value=1 checked"); %><% nvram_match_x("", "wl_guest_macrule", "0", "value=0"); %> />
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="wl_guest_macrule_1" name="wl_guest_macrule" class="input" <% nvram_match_x("","wl_guest_macrule", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="wl_guest_macrule_0" name="wl_guest_macrule" class="input" <% nvram_match_x("","wl_guest_macrule", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td width="50%" style="margin-top: 10px; border-top: 0 none;">
                                                <input type="button" class="btn btn-info" value="<#GO_2G#>" onclick="location.href='Advanced_WGuest2g_Content.asp';">
                                            </td>
                                            <td style="border-top: 0 none;">
                                                <input type="button" id="applyButton" class="btn btn-primary" style="width: 219px" value="<#CTL_apply#>" onclick="applyRule();">
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
