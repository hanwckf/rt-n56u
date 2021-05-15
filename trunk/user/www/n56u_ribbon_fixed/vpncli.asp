<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu6#></title>
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
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('vpnc_enable', change_vpnc_enabled);

	$j("#tab_vpnc_cfg, #tab_vpnc_ssl").click(function(){
		var newHash = $j(this).attr('href').toLowerCase();
		showTab(newHash);
		return false;
	});
});

</script>
<script>

lan_ipaddr_x = '<% nvram_get_x("", "lan_ipaddr"); %>';
lan_netmask_x = '<% nvram_get_x("", "lan_netmask"); %>';
fw_enable_x = '<% nvram_get_x("", "fw_enable_x"); %>';
vpnc_state_last = '<% nvram_get_x("", "vpnc_state_t"); %>';
ip6_service = '<% nvram_get_x("", "ip6_service"); %>';

<% login_state_hook(); %>
<% openvpn_cli_cert_hook(); %>

function initial(){
	show_banner(0);
	show_menu(4, -1, 0);
	show_footer();

	if (!found_app_ovpn())
		document.form.vpnc_type.remove(2);
	else
	if (!support_ipv6() || ip6_service == ''){
		var o = document.form.vpnc_ov_prot;
		for (var i = 0; i < 4; i++) {
			o.remove(2);
		}
	}

	if (fw_enable_x == "0"){
		var o1 = document.form.vpnc_sfw;
		o1.remove(0);
		o1.remove(0);
	}

	change_vpnc_enabled();

	showTab(getHash());

	load_body();
}

function update_vpnc_status(vpnc_state){
	this.vpnc_state_last = vpnc_state;
	showhide_div('col_vpnc_state', (vpnc_state == '1' && document.form.vpnc_enable[0].checked) ? 1 : 0);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/vpncli.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function valid_rlan_subnet(oa, om){
	var ip4ra = parse_ipv4_addr(oa.value);
	var ip4rm = parse_ipv4_addr(om.value);
	if (ip4ra == null){
		alert(oa.value + " <#JS_validip#>");
		oa.focus();
		oa.select();
		return false;
	}
	if (ip4rm == null || isMask(om.value) <= 0){
		alert(om.value + " <#JS_validmask#>");
		om.focus();
		om.select();
		return false;
	}

	for (i=0;i<4;i++)
		ip4ra[i] = ip4ra[i] & ip4rm[i];
	var r_str = ip4ra[0] + '.' + ip4ra[1] + '.' + ip4ra[2] + '.' + ip4ra[3];

	if (matchSubnet2(oa.value, om.value, lan_ipaddr_x, lan_netmask_x)) {
		alert("Please set remote subnet not equal LAN subnet (" + r_str + ")!");
		oa.focus();
		oa.select();
		return false;
	}

	oa.value = r_str;

	return true;
}

function validForm(){
	if (!document.form.vpnc_enable[0].checked)
		return true;

	if(document.form.vpnc_peer.value.length < 4){
		alert("Remote host is invalid!");
		document.form.vpnc_peer.focus();
		return false;
	}

	if(!validate_string(document.form.vpnc_peer))
		return false;

	var mode = document.form.vpnc_type.value;
	if (mode == "2") {
		if(!validate_range(document.form.vpnc_ov_port, 1, 65535))
			return false;
	}
	else {
		if(!validate_range(document.form.vpnc_mtu, 1000, 1460))
			return false;
		if(!validate_range(document.form.vpnc_mru, 1000, 1460))
			return false;
		
		if (document.form.vpnc_rnet.value.length > 0)
			return valid_rlan_subnet(document.form.vpnc_rnet, document.form.vpnc_rmsk);
	}

	return true;
}

function done_validating(action){
}

function textarea_ovpn_enabled(v){
	inputCtrl(document.form['ovpncli.client.conf'], v);
	inputCtrl(document.form['ovpncli.ca.crt'], v);
	inputCtrl(document.form['ovpncli.client.crt'], v);
	inputCtrl(document.form['ovpncli.client.key'], v);
	inputCtrl(document.form['ovpncli.ta.key'], v);
}

function change_vpnc_enabled() {
	var v = document.form.vpnc_enable[0].checked;

	showhide_div('tbl_vpnc_config', v);
	showhide_div('tbl_vpnc_server', v);

	if (!v){
		showhide_div('tab_vpnc_ssl', 0);
		showhide_div('tbl_vpnc_route', 0);
		textarea_ovpn_enabled(0);
	}else{
		change_vpnc_type();
	}
}

function change_vpnc_type() {
	var mode = document.form.vpnc_type.value;
	var is_ov = (mode == "2") ? 1 : 0;

	showhide_div('row_vpnc_auth', !is_ov);
	showhide_div('row_vpnc_mppe', !is_ov);
	showhide_div('row_vpnc_pppd', !is_ov);
	showhide_div('row_vpnc_mtu', !is_ov);
	showhide_div('row_vpnc_mru', !is_ov);
	showhide_div('tbl_vpnc_route', !is_ov);

	showhide_div('row_vpnc_ov_port', is_ov);
	showhide_div('row_vpnc_ov_prot', is_ov);
	showhide_div('row_vpnc_ov_auth', is_ov);
	showhide_div('row_vpnc_ov_mdig', is_ov);
	showhide_div('row_vpnc_ov_ciph', is_ov);
	showhide_div('row_vpnc_ov_ncp_clist', is_ov);
	showhide_div('row_vpnc_ov_compress', is_ov);
	showhide_div('row_vpnc_ov_atls', is_ov);
	showhide_div('row_vpnc_ov_mode', is_ov);
	showhide_div('row_vpnc_ov_conf', is_ov);
	showhide_div('tab_vpnc_ssl', is_ov);
	showhide_div('certs_hint', (is_ov && !openvpn_cli_cert_found()) ? 1 : 0);

	textarea_ovpn_enabled(is_ov);

	if (is_ov) {
		change_vpnc_ov_auth();
		change_vpnc_ov_atls();
		change_vpnc_ov_mode();
	}
	else {
		showhide_div('row_vpnc_ov_cnat', 0);
		
		showhide_div('row_vpnc_user', 1);
		showhide_div('row_vpnc_pass', 1);
	}

	showhide_div('col_vpnc_state', (vpnc_state_last == '1') ? 1 : 0);
}

function change_vpnc_ov_auth() {
	var v = (document.form.vpnc_ov_auth.value == "1") ? 1 : 0;

	showhide_div('row_vpnc_user', v);
	showhide_div('row_vpnc_pass', v);
	showhide_div('row_client_key', !v);
	showhide_div('row_client_crt', !v);
}

function change_vpnc_ov_atls() {
	var v = (document.form.vpnc_ov_atls.value != "0") ? 1 : 0;

	showhide_div('row_ta_key', v);
	inputCtrl(document.form['ovpncli.ta.key'], v);
}

function change_vpnc_ov_mode() {
	showhide_div('row_vpnc_ov_cnat', (document.form.vpnc_ov_mode.value == "1") ? 0 : 1);
}

var arrHashes = ["cfg", "ssl"];

function showTab(curHash){
	var obj = $('tab_vpnc_'+curHash.slice(1));
	if (obj == null || obj.style.display == 'none')
		curHash = '#cfg';
	for(var i = 0; i < arrHashes.length; i++){
		if(curHash == ('#'+arrHashes[i])){
			$j('#tab_vpnc_'+arrHashes[i]).parents('li').addClass('active');
			$j('#wnd_vpnc_'+arrHashes[i]).show();
		}else{
			$j('#wnd_vpnc_'+arrHashes[i]).hide();
			$j('#tab_vpnc_'+arrHashes[i]).parents('li').removeClass('active');
		}
	}
	window.location.hash = curHash;
}

function getHash(){
	var curHash = window.location.hash.toLowerCase();
	for(var i = 0; i < arrHashes.length; i++){
		if(curHash == ('#'+arrHashes[i]))
			return curHash;
	}
	return ('#'+arrHashes[0]);
}

</script>

<style>
    .caption-bold {
        font-weight: bold;
    }
</style>

</head>

<body onload="initial();" onunload="unload_body();">
<script>
    if(get_ap_mode()){
        alert("<#page_not_support_mode_hint#>");
        location.href = "/as.asp";
    }
</script>

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <br>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0" style="position: absolute;"></iframe>

    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
    <input type="hidden" name="current_page" value="vpncli.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="flag" value="">

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
                <div class="box well grad_colour_dark_blue">
                    <div id="tabMenu"></div>
                    <h2 class="box_head round_top"><#menu6#></h2>

                    <div class="round_bottom">

                        <div>
                            <ul class="nav nav-tabs" style="margin-bottom: 10px;">
                                <li class="active">
                                    <a id="tab_vpnc_cfg" href="#cfg"><#Settings#></a>
                                </li>
                                <li>
                                    <a id="tab_vpnc_ssl" href="#ssl" style="display:none"><#OVPN_Cert#></a>
                                </li>
                            </ul>
                        </div>

                        <div id="wnd_vpnc_cfg">
                            <div class="alert alert-info" style="margin: 10px;"><#VPNC_Info#></div>
                            <table class="table">
                                <tr>
                                    <th width="50%" style="padding-bottom: 0px; border-top: 0 none;"><#VPNC_Enable#></th>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <div class="main_itoggle">
                                            <div id="vpnc_enable_on_of">
                                                <input type="checkbox" id="vpnc_enable_fake" <% nvram_match_x("", "vpnc_enable", "1", "value=1 checked"); %><% nvram_match_x("", "vpnc_enable", "0", "value=0"); %>>
                                            </div>
                                        </div>
                                            <div style="position: absolute; margin-left: -10000px;">
                                            <input type="radio" name="vpnc_enable" id="vpnc_enable_1" class="input" value="1" onclick="change_vpnc_enabled();" <% nvram_match_x("", "vpnc_enable", "1", "checked"); %>><#checkbox_Yes#>
                                            <input type="radio" name="vpnc_enable" id="vpnc_enable_0" class="input" value="0" onclick="change_vpnc_enabled();" <% nvram_match_x("", "vpnc_enable", "0", "checked"); %>><#checkbox_No#>
                                        </div>
                                    </td>
                                </tr>
                            </table>
                            <table class="table" id="tbl_vpnc_config" style="display:none">
                                <tr>
                                    <th colspan="2" style="background-color: #E3E3E3;"><#VPNC_Base#></th>
                                </tr>
                                <tr>
                                    <th width="50%"><#VPNC_Type#></th>
                                    <td>
                                        <select name="vpnc_type" class="input" onchange="change_vpnc_type();">
                                            <option value="0" <% nvram_match_x("", "vpnc_type", "0","selected"); %>>PPTP</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_type", "1","selected"); %>>L2TP (w/o IPSec)</option>
                                            <option value="2" <% nvram_match_x("", "vpnc_type", "2","selected"); %>>OpenVPN</option>
                                        </select>
                                        <span id="certs_hint" style="display:none" class="label label-warning"><#OVPN_Hint#></span>
                                    </td>
                                </tr>
                                <tr>
                                    <th><#VPNC_Peer#></th>
                                    <td>
                                        <input type="text" name="vpnc_peer" class="input" maxlength="256" size="32" value="<% nvram_get_x("", "vpnc_peer"); %>" onKeyPress="return is_string(this,event);"/>
                                        &nbsp;<span id="col_vpnc_state" style="display:none" class="label label-success"><#Connected#></span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_port" style="display:none">
                                    <th><#OVPN_Port#></th>
                                    <td>
                                        <input type="text" maxlength="5" size="5" name="vpnc_ov_port" class="input" value="<% nvram_get_x("", "vpnc_ov_port"); %>" onkeypress="return is_number(this,event);">
                                        &nbsp;<span style="color:#888;">[ 1194 ]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_prot" style="display:none">
                                    <th><#OVPN_Prot#></th>
                                    <td>
                                        <select name="vpnc_ov_prot" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_prot", "0","selected"); %>>UDP over IPv4 (*)</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_prot", "1","selected"); %>>TCP over IPv4</option>
                                            <option value="2" <% nvram_match_x("", "vpnc_ov_prot", "2","selected"); %>>UDP over IPv6</option>
                                            <option value="3" <% nvram_match_x("", "vpnc_ov_prot", "3","selected"); %>>TCP over IPv6</option>
                                            <option value="4" <% nvram_match_x("", "vpnc_ov_prot", "4","selected"); %>>UDP both</option>
                                            <option value="5" <% nvram_match_x("", "vpnc_ov_prot", "5","selected"); %>>TCP both</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_mode" style="display:none">
                                    <th><#OVPN_Mode#></th>
                                    <td>
                                        <select name="vpnc_ov_mode" class="input" onchange="change_vpnc_ov_mode();">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_mode", "0","selected"); %>>L2 - TAP (Ethernet)</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_mode", "1","selected"); %>>L3 - TUN (IP)</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_auth" style="display:none">
                                    <th><#OVPN_Auth#></th>
                                    <td>
                                        <select name="vpnc_ov_auth" class="input" onchange="change_vpnc_ov_auth();">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_auth", "0","selected"); %>>TLS: client.crt/client.key</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_auth", "1","selected"); %>>TLS: username/password</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_user">
                                    <th><#ISP_Authentication_user#></th>
                                    <td>
                                       <input type="text" maxlength="64" class="input" size="32" name="vpnc_user" value="<% nvram_get_x("", "vpnc_user"); %>" onkeypress="return is_string(this,event);"/>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_pass">
                                    <th><#ISP_Authentication_pass#></th>
                                    <td>
                                        <div class="input-append">
                                            <input type="password" maxlength="64" class="input" size="32" name="vpnc_pass" id="vpnc_pass" style="width: 175px;" value="<% nvram_get_x("", "vpnc_pass"); %>"/>
                                            <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('vpnc_pass')"><i class="icon-eye-close"></i></button>
                                        </div>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_auth">
                                    <th><#VPNS_Auth#></th>
                                    <td>
                                        <select name="vpnc_auth" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_auth", "0","selected"); %>>Auto</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_auth", "1","selected"); %>>MS-CHAPv2</option>
                                            <option value="2" <% nvram_match_x("", "vpnc_auth", "2","selected"); %>>CHAP</option>
                                            <option value="3" <% nvram_match_x("", "vpnc_auth", "3","selected"); %>>PAP</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_mppe">
                                    <th><#VPNS_Ciph#></th>
                                    <td>
                                        <select name="vpnc_mppe" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_mppe", "0","selected"); %>>Auto</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_mppe", "1","selected"); %>>MPPE-128</option>
                                            <option value="2" <% nvram_match_x("", "vpnc_mppe", "2","selected"); %>>MPPE-40</option>
                                            <option value="3" <% nvram_match_x("", "vpnc_mppe", "3","selected"); %>>No encryption</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_mtu">
                                    <th>MTU:</th>
                                    <td>
                                        <input type="text" maxlength="4" size="5" name="vpnc_mtu" class="input" value="<% nvram_get_x("", "vpnc_mtu"); %>" onkeypress="return is_number(this,event);"/>
                                        &nbsp;<span style="color:#888;">[1000..1460]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_mru">
                                    <th>MRU:</th>
                                    <td>
                                        <input type="text" maxlength="4" size="5" name="vpnc_mru" class="input" value="<% nvram_get_x("", "vpnc_mru"); %>" onkeypress="return is_number(this,event);"/>
                                        &nbsp;<span style="color:#888;">[1000..1460]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_pppd">
                                    <th style="padding-bottom: 0px;"><#PPPConnection_x_AdditionalOptions_itemname#></th>
                                    <td style="padding-bottom: 0px;">
                                        <input type="text" name="vpnc_pppd" value="<% nvram_get_x("", "vpnc_pppd"); %>" class="input" maxlength="255" size="32" onKeyPress="return is_string(this,event);" />
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_mdig" style="display:none">
                                    <th><#VPNS_Auth#></th>
                                    <td>
                                        <select name="vpnc_ov_mdig" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_mdig", "0","selected"); %>>[MD5] MD-5, 128 bit</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_mdig", "1","selected"); %>>[SHA1] SHA-1, 160 bit (*)</option>
                                            <option value="2" <% nvram_match_x("", "vpnc_ov_mdig", "2","selected"); %>>[SHA224] SHA-224, 224 bit</option>
                                            <option value="3" <% nvram_match_x("", "vpnc_ov_mdig", "3","selected"); %>>[SHA256] SHA-256, 256 bit</option>
                                            <option value="4" <% nvram_match_x("", "vpnc_ov_mdig", "4","selected"); %>>[SHA384] SHA-384, 384 bit</option>
                                            <option value="5" <% nvram_match_x("", "vpnc_ov_mdig", "5","selected"); %>>[SHA512] SHA-512, 512 bit</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_ciph" style="display:none">
                                    <th><#VPNS_Ciph#></th>
                                    <td>
                                        <select name="vpnc_ov_ciph" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_ciph", "0","selected"); %>>[none]</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_ciph", "1","selected"); %>>[DES-CBC] DES, 64 bit</option>
                                            <option value="2" <% nvram_match_x("", "vpnc_ov_ciph", "2","selected"); %>>[DES-EDE-CBC] 3DES, 128 bit</option>
                                            <option value="3" <% nvram_match_x("", "vpnc_ov_ciph", "3","selected"); %>>[BF-CBC] Blowfish, 128 bit (*)</option>
                                            <option value="4" <% nvram_match_x("", "vpnc_ov_ciph", "4","selected"); %>>[AES-128-CBC] AES, 128 bit</option>
                                            <option value="5" <% nvram_match_x("", "vpnc_ov_ciph", "5","selected"); %>>[AES-192-CBC] AES, 192 bit</option>
                                            <option value="6" <% nvram_match_x("", "vpnc_ov_ciph", "6","selected"); %>>[DES-EDE3-CBC] 3DES, 192 bit</option>
                                            <option value="7" <% nvram_match_x("", "vpnc_ov_ciph", "7","selected"); %>>[DESX-CBC] DES-X, 192 bit</option>
                                            <option value="8" <% nvram_match_x("", "vpnc_ov_ciph", "8","selected"); %>>[AES-256-CBC] AES, 256 bit</option>
                                            <option value="9" <% nvram_match_x("", "vpnc_ov_ciph", "9","selected"); %>>[CAMELLIA-128-CBC] CAM, 128 bit</option>
                                            <option value="10" <% nvram_match_x("", "vpnc_ov_ciph", "10","selected"); %>>[CAMELLIA-192-CBC] CAM, 192 bit</option>
                                            <option value="11" <% nvram_match_x("", "vpnc_ov_ciph", "11","selected"); %>>[CAMELLIA-256-CBC] CAM, 256 bit</option>
                                            <option value="12" <% nvram_match_x("", "vpnc_ov_ciph", "12","selected"); %>>[AES-128-GCM] AES-GCM, 128 bit</option>
                                            <option value="13" <% nvram_match_x("", "vpnc_ov_ciph", "13","selected"); %>>[AES-192-GCM] AES-GCM, 192 bit</option>
                                            <option value="14" <% nvram_match_x("", "vpnc_ov_ciph", "14","selected"); %>>[AES-256-GCM] AES-GCM, 256 bit</option>
                                            <option value="15" <% nvram_match_x("", "vpnc_ov_ciph", "15","selected"); %>>[CHACHA20-POLY1305], 256 bit</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_ncp_clist" style="display:none">
                                    <th><#OVPN_NCP_clist#></th>
                                    <td>
                                        <input type="text" maxlength="256" size="15" name="vpnc_ov_ncp_clist" class="input" style="width: 286px;" value="<% nvram_get_x("", "vpnc_ov_ncp_clist"); %>" onkeypress="return is_string(this,event);"/>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_compress" style="display:none">
                                    <th><#OVPN_COMPRESS#></th>
                                    <td>
                                        <select name="vpnc_ov_compress" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_compress", "0","selected"); %>><#btn_Disable#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_compress", "1","selected"); %>><#OVPN_COMPRESS_Item1#></option>
                                            <option value="2" <% nvram_match_x("", "vpnc_ov_compress", "2","selected"); %>><#OVPN_COMPRESS_Item2#> (*)</option>
                                            <option value="3" <% nvram_match_x("", "vpnc_ov_compress", "3","selected"); %>><#OVPN_COMPRESS_Item3#></option>
                                            <option value="4" <% nvram_match_x("", "vpnc_ov_compress", "4","selected"); %>><#OVPN_COMPRESS_Item4#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_atls" style="display:none">
                                    <th><#OVPN_HMAC#></th>
                                    <td>
                                        <select name="vpnc_ov_atls" class="input" onchange="change_vpnc_ov_atls();">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_atls", "0","selected"); %>><#checkbox_No#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_atls", "1","selected"); %>><#OVPN_HMAC_Item1#></option>
                                            <option value="2" <% nvram_match_x("", "vpnc_ov_atls", "2","selected"); %>><#OVPN_HMAC_Item2#></option>
                                            <option value="3" <% nvram_match_x("", "vpnc_ov_atls", "3","selected"); %>><#OVPN_USE_TCV2_ItemC#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_cnat" style="display:none">
                                    <th><#OVPN_Topo#></th>
                                    <td>
                                        <select name="vpnc_ov_cnat" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_cnat", "0","selected"); %>><#OVPN_Topo1#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_cnat", "1","selected"); %>><#OVPN_Topo2#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_conf" style="display:none">
                                    <td colspan="2" style="padding-bottom: 0px;">
                                        <a href="javascript:spoiler_toggle('spoiler_vpnc_ov_conf')"><span><#OVPN_User#></span></a>
                                        <div id="spoiler_vpnc_ov_conf" style="display:none;">
                                            <textarea rows="16" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.client.conf" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.client.conf",""); %></textarea>
                                        </div>
                                    </td>
                                </tr>
                            </table>
                            <table class="table" id="tbl_vpnc_server">
                                <tr>
                                    <th colspan="2" style="background-color: #E3E3E3;"><#VPNC_VPNS#></th>
                                </tr>
                                <tr>
                                    <th width="50%"><#VPNC_SFW#></th>
                                    <td>
                                        <select name="vpnc_sfw" class="input" style="width: 320px;">
                                            <option value="1" <% nvram_match_x("", "vpnc_sfw", "1","selected"); %>><#VPNC_SFW_Item1#></option>
                                            <option value="3" <% nvram_match_x("", "vpnc_sfw", "3","selected"); %>><#VPNC_SFW_Item3#></option>
                                            <option value="0" <% nvram_match_x("", "vpnc_sfw", "0","selected"); %>><#VPNC_SFW_Item0#></option>
                                            <option value="2" <% nvram_match_x("", "vpnc_sfw", "2","selected"); %>><#VPNC_SFW_Item2#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr>
                                    <th><#VPNC_PDNS#></th>
                                    <td>
                                        <select name="vpnc_pdns" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_pdns", "0","selected"); %>><#checkbox_No#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_pdns", "1","selected"); %>><#VPNC_PDNS_Item1#></option>
                                            <option value="2" <% nvram_match_x("", "vpnc_pdns", "2","selected"); %>><#VPNC_PDNS_Item2#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr>
                                    <th><#VPNC_DGW#></th>
                                    <td>
                                        <select name="vpnc_dgw" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_dgw", "0","selected"); %>><#checkbox_No#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_dgw", "1","selected"); %>><#checkbox_Yes#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr>
                                    <td colspan="2" style="padding-bottom: 0px;">
                                        <a href="javascript:spoiler_toggle('spoiler_script')"><span><#RunPostVPNC#></span></a>
                                        <div id="spoiler_script" style="display:none;">
                                            <textarea rows="16" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="scripts.vpnc_server_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.vpnc_server_script.sh",""); %></textarea>
                                        </div>
                                    </td>
                                </tr>
                            </table>
                            <table class="table" id="tbl_vpnc_route" style="display:none">
                                <tr>
                                    <th colspan="2" style="background-color: #E3E3E3;"><#VPNC_Route#></th>
                                </tr>
                                <tr>
                                    <th width="50%"><#VPNC_RNet#></th>
                                    <td>
                                        <input type="text" maxlength="15" size="14" name="vpnc_rnet" style="width: 94px;" value="<% nvram_get_x("", "vpnc_rnet"); %>" onKeyPress="return is_ipaddr(this,event);" />&nbsp;/
                                        <input type="text" maxlength="15" size="14" name="vpnc_rmsk" style="width: 94px;" value="<% nvram_get_x("", "vpnc_rmsk"); %>" onKeyPress="return is_ipaddr(this,event);" />
                                    </td>
                                </tr>
                            </table>
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                </tr>
                            </table>
                        </div>

                        <div id="wnd_vpnc_ssl" style="display:none">
                            <table class="table">
                                <tr>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">ca.crt (Root CA Certificate):</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.ca.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.ca.crt",""); %></textarea>
                                    </td>
                                </tr>
                                <tr id="row_client_crt">
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">client.crt (Client Certificate):</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.client.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.client.crt",""); %></textarea>
                                    </td>
                                </tr>
                                <tr id="row_client_key">
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">client.key (Client Private Key) - secret:</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.client.key" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.client.key",""); %></textarea>
                                    </td>
                                </tr>
                                <tr id="row_ta_key">
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">ta.key/tc.key(ctc2.key) (TLS Auth/Crypt(Crypt-v2) Key) - secret:</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.ta.key" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.ta.key",""); %></textarea>
                                    </td>
                                </tr>
                            </table>
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none;"><center><input name="button2" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                </tr>
                            </table>
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
