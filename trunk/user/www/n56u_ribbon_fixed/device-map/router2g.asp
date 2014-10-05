<!DOCTYPE html>
<html>
<head>
<title></title>
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
<script type="text/javascript" src="formcontrol.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('rt_radio_x');
	init_itoggle('rt_closed');
});

</script>
<script>

var had_wrong_wep_key = false;

<% wl_bssid_2g(); %>

function initial(){
	flash_button();
	loadXML();

	if (!support_5g_radio()) {
		$("tab_radio5g").style.display = "none";
	}

	if(parent.document.rt_form.rt_ssid.value == ""){
		document.form.rt_ssid.value = decodeURIComponent(document.form.rt_ssid2.value);
		document.form.rt_wpa_psk.value = decodeURIComponent(document.form.rt_wpa_psk_org.value);
		document.form.rt_key1.value = decodeURIComponent(document.form.rt_key1_org.value);
		document.form.rt_key2.value = decodeURIComponent(document.form.rt_key2_org.value);
		document.form.rt_key3.value = decodeURIComponent(document.form.rt_key3_org.value);
		document.form.rt_key4.value = decodeURIComponent(document.form.rt_key4_org.value);
	}
	else{
		document.form.rt_ssid.value = parent.document.rt_form.rt_ssid.value;
		document.form.rt_ssid2.value = parent.document.rt_form.rt_ssid2.value;
		document.form.rt_key_type.value = parent.document.rt_form.rt_key_type.value;
		document.form.rt_auth_mode.value = parent.document.rt_form.rt_auth_mode.value;
		document.form.rt_wpa_mode.value = parent.document.rt_form.rt_wpa_mode.value;
		document.form.rt_wep_x.value = parent.document.rt_form.rt_wep_x.value;
		document.form.rt_key.value = parent.document.rt_form.rt_key.value;
		document.form.rt_key1.value = parent.document.rt_form.rt_key1.value;
		document.form.rt_key2.value = parent.document.rt_form.rt_key2.value;
		document.form.rt_key3.value = parent.document.rt_form.rt_key3.value;
		document.form.rt_key4.value = parent.document.rt_form.rt_key4.value;
		document.form.rt_crypto.value = parent.document.rt_form.rt_crypto.value;
		document.form.rt_wpa_psk.value = parent.document.rt_form.rt_wpa_psk.value;
	}

	if(document.form.rt_wpa_psk.value.length <= 0)
		document.form.rt_wpa_psk.value = "Please type Password";

	if(document.form.rt_auth_mode.value == "psk"){
		if(document.form.rt_wpa_mode.value == "0")
			document.form.rt_auth_mode[4].selected = true;
		else if(document.form.rt_wpa_mode.value == "1")
			document.form.rt_auth_mode[2].selected = true;
		else
			document.form.rt_auth_mode[3].selected = true;
	}

	rt_auth_mode_change(1);

	if(sw_mode == "3"){
		stopFlag = 1;
	}
	show_middle_status_router();
	show_LAN_info();
	domore_create();
}

function show_middle_status_router(){
	var auth_mode = document.form.rt_auth_mode_orig.value;
	var wpa_mode = document.form.rt_wpa_mode_orig.value;
	var wep_x = parseInt(document.form.rt_wep_x_orig.value);
	var security_mode;

	if(auth_mode == "open")
		security_mode = "Open System";
	else if(auth_mode == "shared")
		security_mode = "Shared Key";
	else if(auth_mode == "psk"){
		if(wpa_mode == "1")
			security_mode = "WPA-Personal";
		else if(wpa_mode == "2")
			security_mode = "WPA2-Personal";
		else if(wpa_mode == "0")
			security_mode = "WPA-Auto-Personal";
	}
	else if(auth_mode == "wpa"){
		if(wpa_mode == "3")
			security_mode = "WPA-Enterprise";
		else if(wpa_mode == "4")
			security_mode = "WPA-Auto-Enterprise";
	}
	else if(auth_mode == "wpa2")
		security_mode = "WPA2-Enterprise";
	else if(auth_mode == "radius")
		security_mode = "Radius with 802.1x";

	//parent.$("wl_securitylevel_span").innerHTML = security_mode;

	if(auth_mode == "open" && wep_x == 0){
		parent.$j("#wl_securitylevel_span").removeClass("badge badge-success");
		parent.$j("#wl_securitylevel_span").addClass("badge badge-important");
		parent.$j("#wl_securitylevel_span").html('<i class="icon-exclamation-sign icon-white"></i>');
	}else{
		parent.$j("#wl_securitylevel_span").removeClass("badge badge-important");
		parent.$j("#wl_securitylevel_span").addClass("badge badge-success");
		parent.$j("#wl_securitylevel_span").html('<i class="icon-lock icon-white"></i>');
	}
}

function domore_create(){
	if(get_ap_mode()){
		$("Router_domore").remove(6);
		$("Router_domore").remove(5);
		$("Router_domore").options[4].value="../Advanced_APLAN_Content.asp";
	}
}

function rt_auth_mode_change(isload){
	var mode = document.form.rt_auth_mode.value;
	var opts = document.form.rt_auth_mode.options;
	var new_array;
	var cur_crypto;
	var cur_key_index, cur_key_obj;

	if(mode == "open" || mode == "shared"){
		$("all_related_wep").style.display = "";
		$("all_wep_key").style.display = "";
		$("asus_wep_key").style.display = "";
		change_wep_type(mode);
	}
	else{
		$("all_related_wep").style.display = "none";
		$("all_wep_key").style.display = "none";
		$("asus_wep_key").style.display = "none";
	}

	if(mode == "wpa" || mode == "wpa2" || mode == "psk")
		$("rt_crypto").style.display = "";
	else
		$("rt_crypto").style.display = "none";

	if(mode == "psk")
		$("rt_wpa_psk").style.display = "";
	else
		$("rt_wpa_psk").style.display = "none";

	for(var i = 0; i < document.form.rt_crypto.length; ++i)
		if(document.form.rt_crypto[i].selected){
			cur_crypto = document.form.rt_crypto[i].value;
			break;
		}

	if(mode == "psk"){
		if(opts[opts.selectedIndex].text == "WPA-Personal")
			new_array = new Array("TKIP");
		else if(opts[opts.selectedIndex].text == "WPA2-Personal")
			new_array = new Array("AES");
		else
			new_array = new Array("AES", "TKIP+AES");
		
		free_options(document.form.rt_crypto);
		for(var i in new_array){
			document.form.rt_crypto[i] = new Option(new_array[i], new_array[i].toLowerCase());
			document.form.rt_crypto[i].value = new_array[i].toLowerCase();
			if(new_array[i].toLowerCase() == cur_crypto)
				document.form.rt_crypto[i].selected = true;
		}
	}
	else if(mode == "wpa"){
		if(opts[opts.selectedIndex].text == "WPA-Enterprise (Radius)")
			new_array = new Array("TKIP");
		else
			new_array = new Array("AES", "TKIP+AES");
		
		free_options(document.form.rt_crypto);
		for(var i in new_array){
			document.form.rt_crypto[i] = new Option(new_array[i], new_array[i].toLowerCase());
			document.form.rt_crypto[i].value = new_array[i].toLowerCase();
			if(new_array[i].toLowerCase() == cur_crypto)
				document.form.rt_crypto[i].selected = true;
		}
	}
	else if(mode == "wpa2"){
		new_array = new Array("AES");
		
		free_options(document.form.rt_crypto);
		for(var i in new_array){
			document.form.rt_crypto[i] = new Option(new_array[i], new_array[i].toLowerCase());
			document.form.rt_crypto[i].value = new_array[i].toLowerCase();
			if(new_array[i].toLowerCase() == cur_crypto)
				document.form.rt_crypto[i].selected = true;
		}
	}

	for(var i = 0; i < document.form.rt_key.length; ++i)
		if(document.form.rt_key[i].selected){
			cur_key_index = document.form.rt_key[i].value;
			break;
		}

	if(mode == "psk" || mode == "wpa" || mode == "wpa2")
		new_array = new Array("2", "3");
	else{
		new_array = new Array("1", "2", "3", "4");
		
		if(!isload)
			cur_key_index = "1";
	}

	free_options(document.form.rt_key);
	for(var i in new_array){
		document.form.rt_key[i] = new Option(new_array[i], new_array[i]);
		document.form.rt_key[i].value = new_array[i];
		if(new_array[i] == cur_key_index)
			document.form.rt_key[i].selected = true;
	}

	rt_wep_change();
}

function change_wep_type(mode){

	var cur_wep = document.form.rt_wep_x.value;
	var wep_type_array;
	var value_array;

	free_options(document.form.rt_wep_x);

	if(mode == "shared"){
		wep_type_array = new Array("WEP-64bits", "WEP-128bits");
		value_array = new Array("1", "2");
	}
	else{
		wep_type_array = new Array("None", "WEP-64bits", "WEP-128bits");
		value_array = new Array("0", "1", "2");
	}

	add_options_x2(document.form.rt_wep_x, wep_type_array, value_array, cur_wep);

	if(mode == "open"){
		if(document.form.rt_wep_x.value == 0){
			document.form.rt_wep_x.selectedIndex = 0;
		}
	}

	if(mode == "psk" || mode == "wpa" || mode == "wpa2")
		document.form.rt_wep_x.value = "0";

	change_wlweptype(document.form.rt_wep_x);
}

function change_wlweptype(wep_type_obj){
	var mode = document.form.rt_auth_mode.value;

	if(wep_type_obj.value == "0"){
		$("all_wep_key").style.display = "none";
		$("asus_wep_key").style.display = "none";
	}
	else{
		if(document.form.rt_gmode.value == "3" && document.form.rt_wep_x.value != 0){
			nmode_limitation2();
		}
		$("all_wep_key").style.display = "";
		$("asus_wep_key").style.display = "";
	}

	rt_wep_change();
}

function rt_wep_change(){
	var mode = document.form.rt_auth_mode.value;
	var wep = document.form.rt_wep_x.value;

	if(mode == "psk" || mode == "wpa" || mode == "wpa2"){
		if(mode == "psk"){
			$("rt_crypto").style.display = "";
			$("rt_wpa_psk").style.display = "";
		}
		
		$("all_wep_key").style.display = "none";
		$("asus_wep_key").style.display = "none";
	}
	else{
		$("rt_crypto").style.display = "none";
		$("rt_wpa_psk").style.display = "none";
		
		if(wep == "0" || mode == "radius"){
			$("all_wep_key").style.display = "none";
			$("asus_wep_key").style.display = "none";
		}
		else{
			$("all_wep_key").style.display = "";
			$("asus_wep_key").style.display = "";
			show_key();
		}
	}
	change_key_des();
}

function change_key_des(){
	var objs = getElementsByName_iefix("span", "key_des");
	var wep_type = document.form.rt_wep_x.value;
	var str = "";

	if(wep_type == "1")
		str = " (<#WLANConfig11b_WEPKey_itemtype1#>)";
	else if(wep_type == "2")
		str = " (<#WLANConfig11b_WEPKey_itemtype2#>)";

	str += ":";

	for(var i = 0; i < objs.length; ++i)
		showtext(objs[i], str);
}

function change_auth_mode(auth_mode_obj){
	rt_auth_mode_change(0);
	if(auth_mode_obj.value == "psk" || auth_mode_obj.value == "wpa"){
		var opts = document.form.rt_auth_mode.options;
		
		if(opts[opts.selectedIndex].text == "WPA-Personal")
			document.form.rt_wpa_mode.value = "1";
		else if(opts[opts.selectedIndex].text == "WPA2-Personal")
			document.form.rt_wpa_mode.value="2";
		else if(opts[opts.selectedIndex].text == "WPA-Auto-Personal")
			document.form.rt_wpa_mode.value="0";
		else if(opts[opts.selectedIndex].text == "WPA-Enterprise (Radius)")
			document.form.rt_wpa_mode.value="3";
		else if(opts[opts.selectedIndex].text == "WPA-Auto-Enterprise (Radius)")
			document.form.rt_wpa_mode.value = "4";
		
		if(auth_mode_obj.value == "psk"){
			document.form.rt_wpa_psk.focus();
			document.form.rt_wpa_psk.select();
		}
	}
	else if(auth_mode_obj.value == "shared"){
		show_key();
	}
	else{
		document.form.rt_wep_x.selectedIndex = 0;
		show_key();
		rt_wep_change();
	}
	nmode_limitation2();
}

function show_key(){
	var wep_type = document.form.rt_wep_x.value;
	var keyindex = document.form.rt_key.value;
	var cur_key_obj = eval("document.form.rt_key"+keyindex);
	var cur_key_length = cur_key_obj.value.length;

	if(wep_type == 1){
		if(cur_key_length == 5 || cur_key_length == 10)
			document.form.rt_asuskey1.value = cur_key_obj.value;
		else if(parent.document.rt_form.rt_asuskey1.value != "")
			document.form.rt_asuskey1.value = parent.document.rt_form.rt_asuskey1.value;
		else
			document.form.rt_asuskey1.value = "0000000000";
	}
	else if(wep_type == 2){
		if(cur_key_length == 13 || cur_key_length == 26)
			document.form.rt_asuskey1.value = cur_key_obj.value;
		else if(parent.document.rt_form.rt_asuskey1.value != "")
			document.form.rt_asuskey1.value = parent.document.rt_form.rt_asuskey1.value;
		else
			document.form.rt_asuskey1.value = "00000000000000000000000000";
	}
	else
		document.form.rt_asuskey1.value = "";

	document.form.rt_asuskey1.focus();
	document.form.rt_asuskey1.select();
}

function show_LAN_info(){
	var lan_ipaddr_t = '<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>';
	if(lan_ipaddr_t != '')
		showtext($("LANIP"), '<% nvram_get_x("LANHostConfig", "lan_ipaddr_t"); %>');
	else	
		showtext($("LANIP"), '<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>');
	showtext($("MAC"), get_bssid_rai0());
}

function show_wepkey_help(){
}

var secs;
var timerID = null;
var timerRunning = false;
var timeout = 1000;
var delay = 500;
var stopFlag=0;

function resetTimer()
{
	if (stopFlag==1){
		stopFlag=0;
		InitializeTimer();
	}
}

function InitializeTimer()
{
	if(document.form.rt_auth_mode.value == "shared"
		|| document.form.rt_auth_mode.value == "wpa"
		|| document.form.rt_auth_mode.value == "wpa2"
		|| document.form.rt_auth_mode.value == "radius")
		return;

	msecs = timeout;
	StopTheClock();
	StartTheTimer();
}

function StopTheClock()
{
	if(timerRunning)
		clearTimeout(timerID);
	timerRunning = false;
}

function StartTheTimer(){
	if(msecs == 0){
		StopTheClock();
		
		if(stopFlag == 1)
			return;
		
		msecs = timeout;
		StartTheTimer();
	}
	else{
		msecs = msecs-500;
		timerRunning = true;
		timerID = setTimeout("StartTheTimer();", delay);
	}
}

function loadXML()
{
	InitializeTimer();
}

function submitForm(){
	var auth_mode = document.form.rt_auth_mode.value;

	if(document.form.rt_wpa_psk.value == "Please type Password")
		document.form.rt_wpa_psk.value = "";

	if(!validate_string_ssid(document.form.rt_ssid))
		return false;

	if(auth_mode == "psk"){
		if(!validate_psk(document.form.rt_wpa_psk))
			return false;
	}
	else{
		if(!validate_wlkey(document.form.rt_asuskey1))
			return false;
	}

	stopFlag = 1;
	document.form.current_page.value = "/";
	document.form.next_page.value = "";
	document.form.action_mode.value = " Apply ";

	var wep11 = eval('document.form.rt_key'+document.form.rt_key.value);
	wep11.value = document.form.rt_asuskey1.value;

	if(auth_mode == "wpa" || auth_mode == "wpa2" || auth_mode == "radius"){
		document.form.target = "";
		document.form.next_page.value = "/Advanced_WSecurity2g_Content.asp";
	}

	parent.showLoading();
	document.form.submit();

	return true;
}

function nmode_limitation2(){
	if(document.form.rt_gmode.value == "3"){
		if(document.form.rt_auth_mode.selectedIndex == 0 && (document.form.rt_wep_x.selectedIndex == "1" || document.form.rt_wep_x.selectedIndex == "2")){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.rt_auth_mode.selectedIndex = 0;
			document.form.rt_wep_x.selectedIndex = 0;
		}
		else if(document.form.rt_auth_mode.selectedIndex == 1){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.rt_auth_mode.selectedIndex = 3;
			document.form.rt_wpa_mode.value = 2;
		}
		else if(document.form.rt_auth_mode.selectedIndex == 2){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.rt_auth_mode.selectedIndex = 3;
			document.form.rt_wpa_mode.value = 2;
		}
		else if(document.form.rt_auth_mode.selectedIndex == 5){
			alert("<#WLANConfig11n_nmode_limition_hint#>");
			document.form.rt_auth_mode.selectedIndex = 6;
		}
		rt_auth_mode_change(1);
	}
	document.form.rt_wpa_psk.focus();
	document.form.rt_wpa_psk.select();
}

window.onunload  = function(){
	var auth_mode = document.form.rt_auth_mode.value;

	if(document.form.rt_wpa_psk.value == "Please type Password")
		document.form.rt_wpa_psk.value = "";

	validate_string_ssid(document.form.rt_ssid);

	if(auth_mode == "psk")
		validate_psk(document.form.rt_wpa_psk);

	var keyindex = document.form.rt_key.value;
	var cur_key_obj = eval("parent.document.rt_form.rt_key"+keyindex);

	cur_key_obj.value = document.form.rt_asuskey1.value;
	parent.document.rt_form.rt_ssid.value = document.form.rt_ssid.value; 
	parent.document.rt_form.rt_wpa_mode.value = document.form.rt_wpa_mode.value;
	parent.document.rt_form.rt_ssid2.value = document.form.rt_ssid2.value;
	parent.document.rt_form.rt_auth_mode.value = document.form.rt_auth_mode.value;
	parent.document.rt_form.rt_key_type.value = document.form.rt_key_type.value;
	parent.document.rt_form.rt_key.value = document.form.rt_key.value;
	parent.document.rt_form.rt_key1.value = document.form.rt_key1.value;
	parent.document.rt_form.rt_key2.value = document.form.rt_key2.value;
	parent.document.rt_form.rt_key3.value = document.form.rt_key3.value;
	parent.document.rt_form.rt_key4.value = document.form.rt_key4.value;
	parent.document.rt_form.rt_wep_x.value = document.form.rt_wep_x.value;
	parent.document.rt_form.rt_crypto.value = document.form.rt_crypto.value;
	parent.document.rt_form.rt_wpa_psk.value = document.form.rt_wpa_psk.value;
	parent.document.rt_form.rt_asuskey1.value = document.form.rt_asuskey1.value;
}
</script>

<style>
    .table th{vertical-align: middle;}
    .table input, .table select{margin-bottom: 0px;}
</style>
</head>

<body class="body_iframe" onload="initial();">
<iframe name="hidden_frame" style="position: absolute;" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply.htm">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="sid_list" value="WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">

<input type="hidden" name="rt_wpa_mode" value="<% nvram_get_x("", "rt_wpa_mode"); %>">
<input type="hidden" name="rt_key1" value="">
<input type="hidden" name="rt_key2" value="">
<input type="hidden" name="rt_key3" value="">
<input type="hidden" name="rt_key4" value="">
<input type="hidden" name="rt_ssid2" value="<% nvram_char_to_ascii("", "rt_ssid"); %>">
<input type="hidden" name="rt_wpa_psk_org" value="<% nvram_char_to_ascii("", "rt_wpa_psk"); %>">
<input type="hidden" name="rt_auth_mode_orig" value="<% nvram_get_x("","rt_auth_mode"); %>">
<input type="hidden" name="rt_wpa_mode_orig" value="<% nvram_get_x("", "rt_wpa_mode"); %>">
<input type="hidden" name="rt_wep_x_orig" value="<% nvram_get_x("", "rt_wep_x"); %>">
<input type="hidden" name="rt_key_type" value="<% nvram_get_x("","rt_key_type"); %>">
<input type="hidden" name="rt_key1_org" value="<% nvram_char_to_ascii("", "rt_key1"); %>">
<input type="hidden" name="rt_key2_org" value="<% nvram_char_to_ascii("", "rt_key2"); %>">
<input type="hidden" name="rt_key3_org" value="<% nvram_char_to_ascii("", "rt_key3"); %>">
<input type="hidden" name="rt_key4_org" value="<% nvram_char_to_ascii("", "rt_key4"); %>">
<input type="hidden" name="rt_gmode" value="<% nvram_get_x("","rt_gmode"); %>">

<ul class="nav nav-tabs">
    <li class="active"><a href="javascript:;">2.4GHz</a></li>
    <li id="tab_radio5g"><a href="router.asp">5GHz</a></li>
</ul>

<table class="table">
  <tr>
    <th width="50%" style="border-top: 0 none; padding-top: 0px;"><#WLANConfig11b_x_RadioEnable_itemname#></th>
    <td style="border-top: 0 none; padding-top: 0px;">
        <div class="main_itoggle">
            <div id="rt_radio_x_on_of">
                <input type="checkbox" id="rt_radio_x_fake" <% nvram_match_x("", "rt_radio_x", "1", "value=1 checked"); %><% nvram_match_x("", "rt_radio_x", "0", "value=0"); %>>
            </div>
            <div style="position: absolute; margin-left: -10000px;">
                <input type="radio" name="rt_radio_x" id="rt_radio_x_1" value="1" <% nvram_match_x("", "rt_radio_x", "1", "checked"); %>>On
                <input type="radio" name="rt_radio_x" id="rt_radio_x_0" value="0" <% nvram_match_x("", "rt_radio_x", "0", "checked"); %>>Off
            </div>
        </div>
    </td>
  </tr>
  <tr>
    <th><#Wireless_name#> (SSID)</th>
    <td>
      <input id="sta_ssid" type="text" name="rt_ssid" value="<% nvram_get_x("", "rt_ssid"); %>" maxlength="32" size="22" />
    </td>
  </tr>
  <tr>
      <th><#WLANConfig11b_x_BlockBCSSID_itemname#></th>
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
    <th><#WLANConfig11b_AuthenticationMethod_itemname#></th>
    <td>
    <select name="rt_auth_mode" class="input" onchange="change_auth_mode(this);">
		<option value="open" <% nvram_match_x("","rt_auth_mode", "open","selected"); %>>Open System</option>
		<option value="shared" <% nvram_match_x("","rt_auth_mode", "shared","selected"); %>>Shared Key</option>
		<option value="psk" <% nvram_double_match_x("", "rt_auth_mode", "psk", "", "rt_wpa_mode", "1", "selected"); %>>WPA-Personal</option>
		<option value="psk" <% nvram_double_match_x("", "rt_auth_mode", "psk", "", "rt_wpa_mode", "2", "selected"); %>>WPA2-Personal</option>
		<option value="psk" <% nvram_double_match_x("", "rt_auth_mode", "psk", "", "rt_wpa_mode", "0", "selected"); %>>WPA-Auto-Personal</option>
		<option value="wpa" <% nvram_double_match_x("", "rt_auth_mode", "wpa", "", "rt_wpa_mode", "3", "selected"); %>>WPA-Enterprise (Radius)</option>
		<option value="wpa2" <% nvram_match_x("", "rt_auth_mode", "wpa2", "selected"); %>>WPA2-Enterprise (Radius)</option>
		<option value="wpa" <% nvram_double_match_x("", "rt_auth_mode", "wpa", "", "rt_wpa_mode", "4", "selected"); %>>WPA-Auto-Enterprise (Radius)</option>
		<option value="radius" <% nvram_match_x("","rt_auth_mode", "radius","selected"); %>>Radius with 802.1x</option>
	  </select>
    </td>
  </tr>

  <tr id='all_related_wep' style='display:none;'>
	<th width="110"><#WLANConfig11b_WEPType_itemname#></th>
	<td>
	  <select name="rt_wep_x" id="rt_wep_x" class="input" onchange="change_wlweptype(this);">
		<option value="0" <% nvram_match_x("", "rt_wep_x", "0", "selected"); %>>None</option>
		<option value="1" <% nvram_match_x("", "rt_wep_x", "1", "selected"); %>>WEP-64bits</option>
		<option value="2" <% nvram_match_x("", "rt_wep_x", "2", "selected"); %>>WEP-128bits</option>
	  </select>
	</td>
  </tr>

  <tr id='all_wep_key' style='display:none;'>
    <th width="110"><#WLANConfig11b_WEPDefaultKey_itemname#></th>
    <td>
      <select name="rt_key" class="input" onchange="show_key();">
        <option value="1" <% nvram_match_x("", "rt_key", "1", "selected"); %>>Key1</option>
        <option value="2" <% nvram_match_x("", "rt_key", "2", "selected"); %>>Key2</option>
        <option value="3" <% nvram_match_x("", "rt_key", "3", "selected"); %>>Key3</option>
        <option value="4" <% nvram_match_x("", "rt_key", "4", "selected"); %>>Key4</option>
      </select>
    </td>
  </tr>
  <tr id='asus_wep_key'>
    <th width="110"><#WLANConfig11b_WEPKey_itemname#></th>
    <td>
      <input type="text" id="sta_asuskey1" name="rt_asuskey1" onfocus="show_wepkey_help();" onKeyUp="return change_wlkey(this, 'WLANConfig11b');" value="" size="22" class="input"/>
    </td>
  </tr>

  <tr id='rt_crypto' style='display:none;'>
	<th width="110"><#WLANConfig11b_WPAType_itemname#></th>
	<td>
	  <select name="rt_crypto" class="input" onchange="rt_auth_mode_change(0);">
		<option value="aes" <% nvram_match_x("", "rt_crypto", "aes", "selected"); %>>AES</option>
		<option value="tkip+aes" <% nvram_match_x("", "rt_crypto", "tkip+aes", "selected"); %>>TKIP+AES</option>
	  </select>
	</td>
  </tr>

  <tr id='rt_wpa_psk' style='display:none'>
    <th width="110"><#WPA-PSKKey#></th>
    <td>
      <div class="input-append">
          <input type="password" name="rt_wpa_psk" id="sta_wpa_psk" maxlength="63" size="22" value="" style="width: 175px;">
          <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('sta_wpa_psk')"><i class="icon-eye-close"></i></button>
      </div>
    </td>
  </tr>
  <tr>
      <th>&nbsp;</th>
      <td>
          <input id="bn_apply" type="button" class="btn btn-primary" value="<#CTL_apply#>" onclick="submitForm();" style="width: 219px;">
      </td>
  </tr>
 </table>
 <table class="table">
  <tr>
    <th width="50%"><#LAN_IP#></th>
    <td id="LANIP"></td>
  </tr>
  <tr>
    <th><#MAC_Address#></th>
    <td id="MAC"></td>
  </tr>
  <tr>
    <th>&nbsp;</th>
    <td>
        <select id="Router_domore" class="domore" onchange="domore_link(this);">
            <option><#MoreConfig#>...</option>
            <option value="../Advanced_Wireless2g_Content.asp"><#menu5_1#> - <#menu5_1_1#></option>
            <option value="../Advanced_WGuest2g_Content.asp"><#menu5_1#> - <#menu5_1_2#></option>
            <option value="../Advanced_WAdvanced2g_Content.asp"><#menu5_1#> - <#menu5_1_6#></option>
            <option value="../Advanced_LAN_Content.asp"><#menu5_2_1#></option>
            <option value="../Advanced_DHCP_Content.asp"><#menu5_2_2#></option>
            <option value="../Advanced_GWStaticRoute_Content.asp"><#menu5_2_3#></option>
            <option value="../Main_WStatus2g_Content.asp"><#menu5_9#></option>
            <option value="../Main_LogStatus_Content.asp"><#menu5_7_2#></option>
        </select>
    </td>
  </tr>
</table>
</form>

</body>
</html>
