<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - 2.4G <#menu5_1_1#></title>
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
<script type="text/javascript" src="/wireless_2g.js"></script>
<script type="text/javascript" src="/help_wl.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('rt_radio_x');
	init_itoggle('rt_closed');
});

</script>
<script>

function initial(){
	show_banner(1);
	show_menu(5,1,1);
	show_footer();

	document.form.rt_radio_date_x_Sun.checked = getDateCheck(document.form.rt_radio_date_x.value, 0);
	document.form.rt_radio_date_x_Mon.checked = getDateCheck(document.form.rt_radio_date_x.value, 1);
	document.form.rt_radio_date_x_Tue.checked = getDateCheck(document.form.rt_radio_date_x.value, 2);
	document.form.rt_radio_date_x_Wed.checked = getDateCheck(document.form.rt_radio_date_x.value, 3);
	document.form.rt_radio_date_x_Thu.checked = getDateCheck(document.form.rt_radio_date_x.value, 4);
	document.form.rt_radio_date_x_Fri.checked = getDateCheck(document.form.rt_radio_date_x.value, 5);
	document.form.rt_radio_date_x_Sat.checked = getDateCheck(document.form.rt_radio_date_x.value, 6);
	document.form.rt_radio_time_x_starthour.value = getTimeRange(document.form.rt_radio_time_x.value, 0);
	document.form.rt_radio_time_x_startmin.value = getTimeRange(document.form.rt_radio_time_x.value, 1);
	document.form.rt_radio_time_x_endhour.value = getTimeRange(document.form.rt_radio_time_x.value, 2);
	document.form.rt_radio_time_x_endmin.value = getTimeRange(document.form.rt_radio_time_x.value, 3);
	document.form.rt_radio_time2_x_starthour.value = getTimeRange(document.form.rt_radio_time2_x.value, 0);
	document.form.rt_radio_time2_x_startmin.value = getTimeRange(document.form.rt_radio_time2_x.value, 1);
	document.form.rt_radio_time2_x_endhour.value = getTimeRange(document.form.rt_radio_time2_x.value, 2);
	document.form.rt_radio_time2_x_endmin.value = getTimeRange(document.form.rt_radio_time2_x.value, 3);

	document.form.rt_ssid.value = decodeURIComponent(document.form.rt_ssid2.value);
	document.form.rt_wpa_psk.value = decodeURIComponent(document.form.rt_wpa_psk_org.value);
	document.form.rt_key1.value = decodeURIComponent(document.form.rt_key1_org.value);
	document.form.rt_key2.value = decodeURIComponent(document.form.rt_key2_org.value);
	document.form.rt_key3.value = decodeURIComponent(document.form.rt_key3_org.value);
	document.form.rt_key4.value = decodeURIComponent(document.form.rt_key4_org.value);
	document.form.rt_phrase_x.value = decodeURIComponent(document.form.rt_phrase_x_org.value);

	if(document.form.rt_wpa_psk.value.length < 1)
		document.form.rt_wpa_psk.value = "Please type Password";

	insertChannelOption();
	rt_auth_mode_change(1);

	document.form.rt_channel.value = document.form.rt_channel_orig.value;

	if (!support_5g_radio()) {
		document.form.goto5.style.display = "none";
		$("col_goto5").width = "33%";
	}

	load_body();

	automode_hint();
	enableExtChRows(document.form.rt_gmode);
	insertExtChannelOption();
}

function applyRule(){
	var auth_mode = document.form.rt_auth_mode.value;
	
	if(document.form.rt_wpa_psk.value == "Please type Password")
		document.form.rt_wpa_psk.value = "";

	if(validForm()){
		document.form.rt_radio_date_x.value = setDateCheck(
		    document.form.rt_radio_date_x_Sun,
		    document.form.rt_radio_date_x_Mon,
		    document.form.rt_radio_date_x_Tue,
		    document.form.rt_radio_date_x_Wed,
		    document.form.rt_radio_date_x_Thu,
		    document.form.rt_radio_date_x_Fri,
		    document.form.rt_radio_date_x_Sat);
		document.form.rt_radio_time_x.value = setTimeRange(
		    document.form.rt_radio_time_x_starthour,
		    document.form.rt_radio_time_x_startmin,
		    document.form.rt_radio_time_x_endhour,
		    document.form.rt_radio_time_x_endmin);
		document.form.rt_radio_time2_x.value = setTimeRange(
		    document.form.rt_radio_time2_x_starthour,
		    document.form.rt_radio_time2_x_startmin,
		    document.form.rt_radio_time2_x_endhour,
		    document.form.rt_radio_time2_x_endmin);
		
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Wireless2g_Content.asp";
		document.form.next_page.value = "";
		
		if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "radius")
			document.form.next_page.value = "/Advanced_WSecurity2g_Content.asp";
		
		inputCtrl(document.form.rt_crypto, 1);
		inputCtrl(document.form.rt_wpa_psk, 1);
		inputCtrl(document.form.rt_wep_x, 1);
		inputCtrl(document.form.rt_key, 1);
		inputCtrl(document.form.rt_key1, 1);
		inputCtrl(document.form.rt_key2, 1);
		inputCtrl(document.form.rt_key3, 1);
		inputCtrl(document.form.rt_key4, 1);
		inputCtrl(document.form.rt_phrase_x, 1);
		inputCtrl(document.form.rt_wpa_gtk_rekey, 1);
		
		document.form.submit();
	}
}

function validForm(){
	var auth_mode = document.form.rt_auth_mode.value;

    if(document.form.rt_radio_x[0].checked){
        if(!validate_timerange(document.form.rt_radio_time_x_starthour, 0)
                || !validate_timerange(document.form.rt_radio_time_x_startmin, 1)
                || !validate_timerange(document.form.rt_radio_time_x_endhour, 2)
                || !validate_timerange(document.form.rt_radio_time_x_endmin, 3)
                )
            return false;

        var starttime = eval(document.form.rt_radio_time_x_starthour.value + document.form.rt_radio_time_x_startmin.value);
        var endtime = eval(document.form.rt_radio_time_x_endhour.value + document.form.rt_radio_time_x_endmin.value);

        if(starttime == endtime){
            alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
            document.form.rt_radio_time_x_starthour.focus();
            document.form.rt_radio_time_x_starthour.select;
            return false;
        }

        if(!validate_timerange(document.form.rt_radio_time2_x_starthour, 0)
                || !validate_timerange(document.form.rt_radio_time2_x_startmin, 1)
                || !validate_timerange(document.form.rt_radio_time2_x_endhour, 2)
                || !validate_timerange(document.form.rt_radio_time2_x_endmin, 3)
                )
            return false;

        var starttime2 = eval(document.form.rt_radio_time2_x_starthour.value + document.form.rt_radio_time2_x_startmin.value);
        var endtime2 = eval(document.form.rt_radio_time2_x_endhour.value + document.form.rt_radio_time2_x_endmin.value);

        if(starttime2 == endtime2){
            alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
            document.form.rt_radio_time2_x_starthour.focus();
            document.form.rt_radio_time2_x_starthour.select;
            return false;
        }

        if((document.form.rt_radio_date_x_Sun.checked ==false)
                && (document.form.rt_radio_date_x_Mon.checked ==false)
                && (document.form.rt_radio_date_x_Tue.checked ==false)
                && (document.form.rt_radio_date_x_Wed.checked ==false)
                && (document.form.rt_radio_date_x_Thu.checked ==false)
                && (document.form.rt_radio_date_x_Fri.checked ==false)
                && (document.form.rt_radio_date_x_Sat.checked ==false)){
            alert("<#WLANConfig11b_x_RadioEnableDate_itemname#> - <#JS_fieldblank#>");
            document.form.rt_radio_x[0].checked=false;
            document.form.rt_radio_x[1].checked=true;
            return false;
        }
    }

	if(!validate_string_ssid(document.form.rt_ssid))
		return false;

	if(document.form.rt_ssid.value == "")
		document.form.rt_ssid.value = "ASUS";

	if(document.form.rt_wep_x.value != "0")
		if(!validate_wlphrase('WLANConfig11b', 'rt_phrase_x', document.form.rt_phrase_x))
			return false;

	if(auth_mode == "psk"){
		if(!validate_psk(document.form.rt_wpa_psk))
			return false;
		
		if(!validate_range(document.form.rt_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else if(auth_mode == "wpa" || auth_mode == "wpa2"){
		if(!validate_range(document.form.rt_wpa_gtk_rekey, 0, 2592000))
			return false;
	}
	else{
		var cur_wep_key = eval('document.form.rt_key'+document.form.rt_key.value);
		if(auth_mode != "radius" && !validate_wlkey(cur_wep_key))
			return false;
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

function change_key_des(){
	var objs = getElementsByName_iefix("span", "key_des");
	var wep_type = document.form.rt_wep_x.value;
	var str = "";
	
	if(wep_type == "1")
		str = "(<#WLANConfig11b_WEPKey_itemtype1#>)";
	else if(wep_type == "2")
		str = "(<#WLANConfig11b_WEPKey_itemtype2#>)";
	
	for(var i = 0; i < objs.length; ++i)
		showtext(objs[i], str);
}

function validate_wlphrase(s, v, obj){
	if(!validate_string(obj)){
		is_wlphrase(s, v, obj);
		return(false);
	}
	
	return true;
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

    <input type="hidden" name="current_page" value="Advanced_Wireless2g_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="WLANConfig11b;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

    <input type="hidden" name="rt_radio_date_x" value="<% nvram_get_x("","rt_radio_date_x"); %>">
    <input type="hidden" name="rt_radio_time_x" value="<% nvram_get_x("","rt_radio_time_x"); %>">
    <input type="hidden" name="rt_radio_time2_x" value="<% nvram_get_x("","rt_radio_time2_x"); %>">
    <input type="hidden" name="rt_ssid2" value="<% nvram_char_to_ascii("",  "rt_ssid"); %>">
    <input type="hidden" name="rt_wpa_mode" value="<% nvram_get_x("","rt_wpa_mode"); %>">
    <input type="hidden" name="rt_wpa_psk_org" value="<% nvram_char_to_ascii("", "rt_wpa_psk"); %>">
    <input type="hidden" name="rt_key1_org" value="<% nvram_char_to_ascii("", "rt_key1"); %>">
    <input type="hidden" name="rt_key2_org" value="<% nvram_char_to_ascii("", "rt_key2"); %>">
    <input type="hidden" name="rt_key3_org" value="<% nvram_char_to_ascii("", "rt_key3"); %>">
    <input type="hidden" name="rt_key4_org" value="<% nvram_char_to_ascii("", "rt_key4"); %>">
    <input type="hidden" name="rt_phrase_x_org" value="<% nvram_char_to_ascii("", "rt_phrase_x"); %>">
    <input type="hidden" name="rt_gmode_protection" value="<% nvram_get_x("", "rt_gmode_protection"); %>">
    <input type="hidden" name="rt_mode_x" value="<% nvram_get_x("","rt_mode_x"); %>">
    <input type="hidden" name="rt_nmode" value="<% nvram_get_x("","rt_nmode"); %>">
    <input type="hidden" name="rt_HT_EXTCHA_old" value="<% nvram_get_x("","rt_HT_EXTCHA"); %>">
    <input type="hidden" name="rt_key_type" value='<% nvram_get_x("","rt_key_type"); %>'>
    <input type="hidden" name="rt_channel_orig" value='<% nvram_get_x("","rt_channel"); %>'>

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
                            <h2 class="box_head round_top"><#menu5_1#> - <#menu5_1_1#> (2.4GHz)</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 22);"><#WLANConfig11b_x_RadioEnable_itemname#></a></th>
                                            <td style="border-top: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="rt_radio_x_on_of">
                                                        <input type="checkbox" id="rt_radio_x_fake" <% nvram_match_x("","rt_radio_x", "1", "value=1 checked"); %><% nvram_match_x("","rt_radio_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="rt_radio_x_1" name="rt_radio_x" class="input" <% nvram_match_x("","rt_radio_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="rt_radio_x_0" name="rt_radio_x" class="input" <% nvram_match_x("","rt_radio_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 23);"><#WLANConfig11b_x_RadioEnableDate_itemname#></a></th>
                                            <td>
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="rt_radio_date_x_Mon" onChange="return changeDate();"/><#DAY_Mon#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="rt_radio_date_x_Tue" onChange="return changeDate();"/><#DAY_Tue#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="rt_radio_date_x_Wed" onChange="return changeDate();"/><#DAY_Wed#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="rt_radio_date_x_Thu" onChange="return changeDate();"/><#DAY_Thu#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="rt_radio_date_x_Fri" onChange="return changeDate();"/><#DAY_Fri#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th style="border-top: 0 none;"><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 0, 24);"><#WLANConfig11b_x_RadioEnableTime_itemname#></a></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time_x_starthour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time_x_startmin" onKeyPress="return is_number(this,event);"/>&nbsp;-&nbsp;
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time_x_endhour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time_x_endmin" onKeyPress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WLANConfig11b_x_RadioEnableDate_itemname2#></th>
                                            <td>
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="rt_radio_date_x_Sat" onChange="return changeDate();"/><#DAY_Sat#></label>
                                                    <label class="checkbox inline"><input type="checkbox" class="input" name="rt_radio_date_x_Sun" onChange="return changeDate();"/><#DAY_Sun#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th style="border-top: 0 none;"><#WLANConfig11b_x_RadioEnableTime_itemname2#></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time2_x_starthour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time2_x_startmin" onKeyPress="return is_number(this,event);"/>&nbsp;-&nbsp;
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time2_x_endhour" onKeyPress="return is_number(this,event);"/>:
                                                <input type="text" maxlength="2" style="width: 20px;" size="2" name="rt_radio_time2_x_endmin" onKeyPress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 0, 1);"><#WLANConfig11b_SSID_itemname#></a></th>
                                            <td><input type="text" maxlength="32" class="input" size="32" name="rt_ssid" value="" onkeypress="return is_string(this,event);"></td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 2);"><#WLANConfig11b_x_BlockBCSSID_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="rt_closed_on_of">
                                                        <input type="checkbox" id="rt_closed_fake" <% nvram_match_x("", "rt_closed", "1", "value=1 checked"); %><% nvram_match_x("", "rt_closed", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" id="rt_closed_1" name="rt_closed" <% nvram_match_x("", "rt_closed", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" id="rt_closed_0" name="rt_closed" <% nvram_match_x("", "rt_closed", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 4);"><#WLANConfig11b_x_Mode11g_itemname#></a></th>
                                            <td>
                                                <select name="rt_gmode" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_gmode')">
                                                    <option value="2" <% nvram_match_x("","rt_gmode", "2","selected"); %>>b/g/n Mixed</option>
                                                    <option value="1" <% nvram_match_x("","rt_gmode", "1","selected"); %>>b/g Mixed</option>
                                                    <option value="5" <% nvram_match_x("","rt_gmode", "5","selected"); %>>g/n Mixed (*)</option>
                                                    <option value="3" <% nvram_match_x("","rt_gmode", "3","selected"); %>>n Only</option>
                                                    <option value="4" <% nvram_match_x("","rt_gmode", "4","selected"); %>>g Only</option>
                                                    <option value="0" <% nvram_match_x("","rt_gmode", "0","selected"); %>>b Only</option>
                                                </select>
                                                <span id="rt_gmode_hint" style="display:none;color:#F75"><#WLANConfig11n_automode_limition_hint#></span>
                                            </td>
                                        </tr>
                                        <tr id="row_protect" style="display:none;">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 16);"><#WLANConfig11b_BGProt11g_itemname#></a></th>
                                            <td>
                                                <select name="rt_gmode_protection" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_gmode_protection')">
                                                    <option value="auto" <% nvram_match_x("","rt_gmode_protection", "auto","selected"); %>>Auto</option>
                                                    <option value="on" <% nvram_match_x("","rt_gmode_protection", "on","selected"); %>>Always On</option>
                                                    <option value="off" <% nvram_match_x("","rt_gmode_protection", "off","selected"); %>>Always Off</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_HT_BW">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 14);"><#WLANConfig11b_ChannelBW_itemname#></a></th>
                                            <td>
                                                <select name="rt_HT_BW" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_HT_BW')">
                                                    <option value="0" <% nvram_match_x("","rt_HT_BW", "0","selected"); %>>20 MHz</option>
                                                    <option value="1" <% nvram_match_x("","rt_HT_BW", "1","selected"); %>>20/40 MHz</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 3);"><#WLANConfig11b_Channel_itemname#></a></th>
                                            <td>
                                                <select name="rt_channel" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_channel')">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_HT_EXTCHA">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 15);"><#WLANConfig11b_EChannel_itemname#></a></th>
                                            <td>
                                                <select name="rt_HT_EXTCHA" class="input">
                                                    <option value="0" selected>Below</option>
                                                    <option value="1">Above</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIGuestMCS#></th>
                                            <td>
                                                <select name="rt_mcs_mode" class="input">
                                                    <option value="0" <% nvram_match_x("", "rt_mcs_mode", "0", "selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "rt_mcs_mode", "1", "selected"); %>>HTMIX (1S) 45 Mbps</option>
                                                    <option value="2" <% nvram_match_x("", "rt_mcs_mode", "2", "selected"); %>>HTMIX (1S) 30 Mbps</option>
                                                    <option value="3" <% nvram_match_x("", "rt_mcs_mode", "3", "selected"); %>>HTMIX (1S) 15 Mbps</option>
                                                    <option value="4" <% nvram_match_x("", "rt_mcs_mode", "4", "selected"); %>>OFDM 12 Mbps</option>
                                                    <option value="5" <% nvram_match_x("", "rt_mcs_mode", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                    <option value="6" <% nvram_match_x("", "rt_mcs_mode", "6", "selected"); %>>OFDM 6 Mbps</option>
                                                    <option value="7" <% nvram_match_x("", "rt_mcs_mode", "7", "selected"); %>>CCK 5.5 Mbps</option>
                                                    <option value="8" <% nvram_match_x("", "rt_mcs_mode", "8", "selected"); %>>CCK 2 Mbps</option>
                                                    <option value="9" <% nvram_match_x("", "rt_mcs_mode", "9", "selected"); %>>CCK 1 Mbps</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 5);"><#WLANConfig11b_AuthenticationMethod_itemname#></a></th>
                                            <td>
                                                <select name="rt_auth_mode" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_auth_mode');">
                                                    <option value="open" <% nvram_match_x("", "rt_auth_mode", "open", "selected"); %>>Open System</option>
                                                    <option value="shared" <% nvram_match_x("", "rt_auth_mode", "shared", "selected"); %>>Shared Key</option>
                                                    <option value="psk" <% nvram_double_match_x("", "rt_auth_mode", "psk", "", "rt_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
                                                    <option value="psk" <% nvram_double_match_x("", "rt_auth_mode", "psk", "", "rt_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
                                                    <option value="psk" <% nvram_double_match_x("", "rt_auth_mode", "psk", "", "rt_wpa_mode", "0", "selected"); %>>WPA-Auto-Personal</option>
                                                    <option value="wpa" <% nvram_double_match_x("", "rt_auth_mode", "wpa", "", "rt_wpa_mode", "3", "selected"); %>>WPA-Enterprise (Radius)</option>
                                                    <option value="wpa2" <% nvram_match_x("", "rt_auth_mode", "wpa2", "selected"); %>>WPA2-Enterprise (Radius)</option>
                                                    <option value="wpa" <% nvram_double_match_x("", "rt_auth_mode", "wpa", "", "rt_wpa_mode", "4", "selected"); %>>WPA-Auto-Enterprise (Radius)</option>
                                                    <option value="radius" <% nvram_match_x("", "rt_auth_mode", "radius", "selected"); %>>Radius with 802.1x</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_wpa1">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 6);"><#WLANConfig11b_WPAType_itemname#></a></th>
                                            <td>
                                                <select name="rt_crypto" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_crypto')">
                                                    <option value="aes" <% nvram_match_x("", "rt_crypto", "aes", "selected"); %>>AES</option>
                                                    <option value="tkip+aes" <% nvram_match_x("", "rt_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_wpa2">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 7);"><#WLANConfig11b_x_PSKKey_itemname#></a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" name="rt_wpa_psk" id="rt_wpa_psk" maxlength="64" size="32" value="" style="width: 175px;">
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('rt_wpa_psk')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_wep1">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 9);"><#WLANConfig11b_WEPType_itemname#></a></th>
                                            <td>
                                                <select name="rt_wep_x" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_wep_x');">
                                                    <option value="0" <% nvram_match_x("", "rt_wep_x", "0", "selected"); %>>None</option>
                                                    <option value="1" <% nvram_match_x("", "rt_wep_x", "1", "selected"); %>>WEP-64bits</option>
                                                    <option value="2" <% nvram_match_x("", "rt_wep_x", "2", "selected"); %>>WEP-128bits</option>
                                                </select>
                                                <br>
                                                <span name="key_des" style="color:#888"></span>
                                            </td>
                                        </tr>
                                        <tr id="row_wep2">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 10);"><#WLANConfig11b_WEPDefaultKey_itemname#></a></th>
                                            <td>
                                                <select name="rt_key" class="input"  onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_key');">
                                                    <option value="1" <% nvram_match_x("","rt_key", "1","selected"); %>>1</option>
                                                    <option value="2" <% nvram_match_x("","rt_key", "2","selected"); %>>2</option>
                                                    <option value="3" <% nvram_match_x("","rt_key", "3","selected"); %>>3</option>
                                                    <option value="4" <% nvram_match_x("","rt_key", "4","selected"); %>>4</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_wep3">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 18);"><#WLANConfig11b_WEPKey1_itemname#></a></th>
                                            <td><input type="text" name="rt_key1" id="rt_key1" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
                                        </tr>
                                        <tr id="row_wep4">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 18);"><#WLANConfig11b_WEPKey2_itemname#></a></th>
                                            <td><input type="text" name="rt_key2" id="rt_key2" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
                                        </tr>
                                        <tr id="row_wep5">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 18);"><#WLANConfig11b_WEPKey3_itemname#></a></th>
                                            <td><input type="text" name="rt_key3" id="rt_key3" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
                                        </tr>
                                        <tr id="row_wep6">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 18);"><#WLANConfig11b_WEPKey4_itemname#></a></th>
                                            <td><input type="text" name="rt_key4" id="rt_key4" maxlength="32" class="input" size="34" value="" onKeyUp="return change_wlkey(this, 'WLANConfig11b');"></td>
                                        </tr>
                                        <tr id="row_wep7">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 8);"><#WLANConfig11b_x_Phrase_itemname#></a></th>
                                            <td>
                                                <input type="text" name="rt_phrase_x" maxlength="64" class="input" size="32" value="" onKeyUp="return is_wlphrase('WLANConfig11b', 'rt_phrase_x', this);">
                                            </td>
                                        </tr>
                                        <tr id="row_wpa3">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 11);"><#WLANConfig11b_x_Rekey_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="7" size="7" name="rt_wpa_gtk_rekey" class="input" value="<% nvram_get_x("", "rt_wpa_gtk_rekey"); %>" onKeyPress="return is_number(this,event);" />
                                                &nbsp;<span style="color:#888;">[0..2592000]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 17);"><#WLANConfig11b_TxPower_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="3" size="3" name="rt_TxPower" class="input" value="<% nvram_get_x("", "rt_TxPower"); %>" onKeyPress="return is_number(this,event);" onblur="return validate_range(this, 0, 100);" />
                                                &nbsp;<span style="color:#888;">[0..100]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 25);"><#WLANConfig11b_KickStaRssiLow_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="4" name="rt_KickStaRssiLow" class="input" value="<% nvram_get_x("", "rt_KickStaRssiLow"); %>" />
                                                &nbsp;<span style="color:#888;">[-100..0]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 0, 26);"><#WLANConfig11b_AssocReqRssiThres_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="4" name="rt_AssocReqRssiThres" class="input" value="<% nvram_get_x("", "rt_AssocReqRssiThres"); %>" />
                                                &nbsp;<span style="color:#888;">[-100..0]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WIFIRegionCode#></th>
                                            <td>
                                                <select name="rt_country_code" class="input" onChange="return change_common_rt(this, 'WLANConfig11b', 'rt_country_code')">
                                                    <option value="US" <% nvram_match_x("", "rt_country_code", "US","selected"); %>>USA (channels 1-11)</option>
                                                    <option value="TW" <% nvram_match_x("", "rt_country_code", "TW","selected"); %>>Taiwan (channels 1-11)</option>
                                                    <option value="CN" <% nvram_match_x("", "rt_country_code", "CN","selected"); %>>China (channels 1-13)</option>
                                                    <option value="JP" <% nvram_match_x("", "rt_country_code", "JP","selected"); %>>Japan (channels 1-13)</option>
                                                    <option value="AU" <% nvram_match_x("", "rt_country_code", "AU","selected"); %>>Australia (channels 1-13)</option>
                                                    <option value="GB" <% nvram_match_x("", "rt_country_code", "GB","selected"); %>>Europe (channels 1-13)</option>
                                                    <option value="RU" <% nvram_match_x("", "rt_country_code", "RU","selected"); %>>Russia (channels 1-13)</option>
                                                    <option value="DB" <% nvram_match_x("", "rt_country_code", "DB","selected"); %>>Debug (all channels)</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td id="col_goto5" width="50%" style="margin-top: 10px; border-top: 0 none;">
                                                <input type="button" class="btn btn-info" name="goto5" value="<#GO_5G#>" onclick="location.href='Advanced_Wireless_Content.asp';">
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
