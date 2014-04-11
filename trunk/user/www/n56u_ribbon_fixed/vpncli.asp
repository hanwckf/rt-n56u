<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu6#></title>

<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>

<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#vpnc_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#vpnc_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#vpnc_enable_1").attr("checked", "checked");
                $j("#vpnc_enable_0").removeAttr("checked");
                change_vpnc_enabled();
            },
            onClickOff: function(){
                $j("#vpnc_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#vpnc_enable_0").attr("checked", "checked");
                $j("#vpnc_enable_1").removeAttr("checked");
                change_vpnc_enabled();
            }
        });
        $j("#vpnc_enable_on_of label.itoggle").css("background-position", $j("input#vpnc_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#tab_vpn_config').click(function(){
            $j(this).parents('ul').find('li').removeClass('active');
            $j(this).parent().addClass('active');

            $j('#wnd_vpn_config').show();
            $j('#wnd_ssl_certs').hide();

            return false;
        });

        $j('#tab_ssl_certs').click(function(){
            $j(this).parents('ul').find('li').removeClass('active');
            $j(this).parent().addClass('active');

            $j('#wnd_ssl_certs').show();
            $j('#wnd_vpn_config').hide();

            return false;
        });
    });
</script>
<script>

lan_ipaddr_x = '<% nvram_get_x("", "lan_ipaddr"); %>';
lan_netmask_x = '<% nvram_get_x("", "lan_netmask"); %>';
fw_enable_x = '<% nvram_get_x("", "fw_enable_x"); %>';

<% login_state_hook(); %>

<% vpnc_state_hook(); %>

<% openvpn_cli_cert_hook(); %>

function initial(){
	show_banner(0);
	show_menu(4, -1, 0);
	show_footer();
	
	if (!found_app_ovpn()){
		document.form.vpnc_type.remove(2);
	}
	
	if (fw_enable_x == "0")
		$("row_vpnc_sfw").style.display = "none";
	
	change_vpnc_enabled();
	
	load_body();
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

function applyRule2(){
	$j('#tab_vpn_config').click();
	applyRule();
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
		alert(om.value + " <#JS_validip#>");
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
	var a = rcheck(document.form.vpnc_enable);
	if (a == "0")
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

function change_vpnc_enabled() {
	var a = rcheck(document.form.vpnc_enable);
	if (a == "0"){
		$("tab_ssl_certs").style.display = "none";
		$("tbl_vpnc_config").style.display = "none";
		$("tbl_vpnc_server").style.display = "none";
		$("tbl_vpnc_route").style.display = "none";
	} else {
		$("tbl_vpnc_config").style.display = "";
		$("tbl_vpnc_server").style.display = "";
		change_vpnc_type();
	}
}

function change_vpnc_type() {
	var mode = document.form.vpnc_type.value;
	if (mode == "2")
		$("row_vpnc_mppe").style.display = "none";
	else
		$("row_vpnc_mppe").style.display = "";

	if (mode == "2" && !openvpn_cli_cert_found())
		$("certs_hint").style.display = "";
	else
		$("certs_hint").style.display = "none";

	if (mode == "2") {
		$("row_vpnc_auth").style.display = "none";
		$("row_vpnc_pppd").style.display = "none";
		$("row_vpnc_mtu").style.display = "none";
		$("row_vpnc_mru").style.display = "none";
		$("tbl_vpnc_route").style.display = "none";
		
		$("row_vpnc_ov_port").style.display = "";
		$("row_vpnc_ov_prot").style.display = "";
		$("row_vpnc_ov_auth").style.display = "";
		$("row_vpnc_ov_atls").style.display = "";
		$("row_vpnc_ov_mode").style.display = "";
		$("row_vpnc_ov_conf").style.display = "";
		$("tab_ssl_certs").style.display = "";
		
		change_vpnc_ov_auth();
		change_vpnc_ov_atls();
		change_vpnc_ov_mode();
	}
	else {
		$("row_vpnc_ov_port").style.display = "none";
		$("row_vpnc_ov_prot").style.display = "none";
		$("row_vpnc_ov_auth").style.display = "none";
		$("row_vpnc_ov_atls").style.display = "none";
		$("row_vpnc_ov_mode").style.display = "none";
		$("row_vpnc_ov_cnat").style.display = "none";
		$("row_vpnc_ov_conf").style.display = "none";
		$("tab_ssl_certs").style.display = "none";
		
		$("row_vpnc_user").style.display = "";
		$("row_vpnc_pass").style.display = "";
		$("row_vpnc_auth").style.display = "";
		$("row_vpnc_pppd").style.display = "";
		$("row_vpnc_mtu").style.display = "";
		$("row_vpnc_mru").style.display = "";
		$("tbl_vpnc_route").style.display = "";
	}

	if (vpnc_state() != 0)
		$("col_vpnc_state").style.display = "";
	else
		$("col_vpnc_state").style.display = "none";
}

function change_vpnc_ov_auth() {
	var ov_auth = document.form.vpnc_ov_auth.value;
	if (ov_auth == "1") {
		$("row_vpnc_user").style.display = "";
		$("row_vpnc_pass").style.display = "";
		$("row_client_key").style.display = "none";
		$("row_client_crt").style.display = "none";
	} else {
		$("row_vpnc_user").style.display = "none";
		$("row_vpnc_pass").style.display = "none";
		$("row_client_key").style.display = "";
		$("row_client_crt").style.display = "";
	}
}

function change_vpnc_ov_atls() {
	var ov_atls = document.form.vpnc_ov_atls.value;
	if (ov_atls == "1") {
		$("row_ta_key").style.display = "";
	} else {
		$("row_ta_key").style.display = "none";
	}
}

function change_vpnc_ov_mode() {
	var ov_mode = document.form.vpnc_ov_mode.value;
	if (ov_mode == "1") {
		$("row_vpnc_ov_cnat").style.display = "none";
	} else {
		$("row_vpnc_ov_cnat").style.display = "";
	}
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
    if(sw_mode == 3){
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
                                    <a id="tab_vpn_config" href="#"><#Settings#></a>
                                </li>
                                <li>
                                    <a id="tab_ssl_certs" href="#" style="display:none"><#OVPN_Cert#></a>
                                </li>
                            </ul>
                        </div>

                        <div id="wnd_vpn_config">
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
                                        <input type="text" name="vpnc_peer" class="input" maxlength="256" size="32" value="<% nvram_get_x("", "vpnc_peer"); %>" onKeyPress="return is_string(this)"/>
                                        &nbsp;<span id="col_vpnc_state" style="display:none" class="label label-success"><#Connected#></span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_port" style="display:none">
                                    <th><#OVPN_Port#></th>
                                    <td>
                                        <input type="text" maxlength="5" size="5" name="vpnc_ov_port" class="input" value="<% nvram_get_x("", "vpnc_ov_port"); %>" onkeypress="return is_number(this)">
                                        &nbsp;<span style="color:#888;">[ 1194 ]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_prot" style="display:none">
                                    <th><#OVPN_Prot#></th>
                                    <td>
                                        <select name="vpnc_ov_prot" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_prot", "0","selected"); %>>UDP (*)</option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_prot", "1","selected"); %>>TCP</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_auth" style="display:none">
                                    <th><#VPNS_Auth#></th>
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
                                       <input type="text" maxlength="64" class="input" size="32" name="vpnc_user" value="<% nvram_get_x("", "vpnc_user"); %>" onkeypress="return is_string(this)"/>
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
                                    <th><#VPNS_MPPE#></th>
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
                                        <input type="text" maxlength="4" size="5" name="vpnc_mtu" class="input" value="<% nvram_get_x("", "vpnc_mtu"); %>" onkeypress="return is_number(this)">
                                        &nbsp;<span style="color:#888;">[1000..1460]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_mru">
                                    <th>MRU:</th>
                                    <td>
                                        <input type="text" maxlength="4" size="5" name="vpnc_mru" class="input" value="<% nvram_get_x("", "vpnc_mru"); %>" onkeypress="return is_number(this)">
                                        &nbsp;<span style="color:#888;">[1000..1460]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_pppd">
                                    <th style="padding-bottom: 0px;"><#PPPConnection_x_AdditionalOptions_itemname#></th>
                                    <td style="padding-bottom: 0px;">
                                        <input type="text" name="vpnc_pppd" value="<% nvram_get_x("", "vpnc_pppd"); %>" class="input" maxlength="255" size="32" onKeyPress="return is_string(this)" onBlur="validate_string(this)"/>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_ov_atls" style="display:none">
                                    <th><#OVPN_HMAC#></th>
                                    <td>
                                        <select name="vpnc_ov_atls" class="input" onchange="change_vpnc_ov_atls();">
                                            <option value="0" <% nvram_match_x("", "vpnc_ov_atls", "0","selected"); %>><#checkbox_No#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_ov_atls", "1","selected"); %>><#checkbox_Yes#></option>
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
                                    <th width="50%"><#VPNC_PDNS#></th>
                                    <td>
                                        <select name="vpnc_pdns" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_pdns", "0","selected"); %>><#checkbox_No#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_pdns", "1","selected"); %>><#VPNC_PDNS_Item1#></option>
                                            <option value="2" <% nvram_match_x("", "vpnc_pdns", "2","selected"); %>><#VPNC_PDNS_Item2#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpnc_sfw">
                                    <th style="padding-bottom: 0px;"><#VPNC_SFW#></th>
                                    <td style="padding-bottom: 0px;">
                                        <select name="vpnc_sfw" class="input">
                                            <option value="0" <% nvram_match_x("", "vpnc_sfw", "0","selected"); %>><#VPNC_SFW_Item0#></option>
                                            <option value="1" <% nvram_match_x("", "vpnc_sfw", "1","selected"); %>><#VPNC_SFW_Item1#></option>
                                        </select>
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
                                        <input type="text" maxlength="15" size="14" name="vpnc_rnet" style="width: 94px;" value="<% nvram_get_x("", "vpnc_rnet"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);" />&nbsp;/
                                        <input type="text" maxlength="15" size="14" name="vpnc_rmsk" style="width: 94px;" value="<% nvram_get_x("", "vpnc_rmsk"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);" />
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
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                </tr>
                            </table>
                        </div>

                        <div id="wnd_ssl_certs" style="display:none">
                            <table class="table">
                                <tr>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">Root CA Certificate:</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.ca.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.ca.crt",""); %></textarea>
                                    </td>
                                </tr>
                                <tr id="row_client_crt">
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">Client Certificate:</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.client.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.client.crt",""); %></textarea>
                                    </td>
                                </tr>
                                <tr id="row_client_key">
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">Client Private Key (secret):</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.client.key" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.client.key",""); %></textarea>
                                    </td>
                                </tr>
                                <tr id="row_ta_key">
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">TLS Auth Key (secret):</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpncli.ta.key" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpncli.ta.key",""); %></textarea>
                                    </td>
                                </tr>
                            </table>
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none;"><center><input name="button2" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule2();" value="<#CTL_apply#>"/></center></td>
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
