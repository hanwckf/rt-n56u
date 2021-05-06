<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_6_5#></title>
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
	init_itoggle('telnetd');
	init_itoggle('wins_enable', change_wins_enabled);
	init_itoggle('lltd_enable');
	init_itoggle('adsc_enable');
	init_itoggle('crond_enable', change_crond_enabled);
	init_itoggle('ttyd_enable', change_ttyd_enabled);
	init_itoggle('vlmcsd_enable');
	init_itoggle('napt66_enable');
	init_itoggle('watchdog_cpu');
});

</script>
<script>

<% login_state_hook(); %>
<% openssl_util_hook(); %>
var lan_ipaddr = '<% nvram_get_x("", "lan_ipaddr_t"); %>';
var http_proto = '<% nvram_get_x("", "http_proto"); %>';
var http_port = '<% nvram_get_x("", "http_lanport"); %>';
var https_port = '<% nvram_get_x("", "https_lport"); %>';

function initial(){
	show_banner(1);
	show_menu(5,7,2);
	show_footer();
	load_body();

	if(!found_app_sshd()){
		showhide_div('row_sshd', 0);
		textarea_sshd_enabled(0);
	}else
		sshd_auth_change();

	if(found_app_nmbd()){
		showhide_div('tbl_wins', 1);
		change_wins_enabled();
	}

	if(!support_http_ssl()) {
		document.form.http_proto.value = "0";
		showhide_div('row_http_proto', 0);
		showhide_div('row_https_lport', 0);
		showhide_div('row_https_clist', 0);
		textarea_https_enabled(0);
	}else{
		if (openssl_util_found() && login_safe()) {
			if(!support_openssl_ec()) {
				var o = document.form.https_gen_rb;
				o.remove(3);
				o.remove(3);
				o.remove(3);
			}
			showhide_div('row_https_gen', 1);
		}
		http_proto_change();
	}
	change_crond_enabled();
	
	if(found_app_ttyd()){	
		$("tbl_ttyd").style.display = "";
		change_ttyd_enabled();
	}
	
	if(!found_app_vlmcsd()){
		showhide_div('div_vlmcsd', 0);
	}
	
	if(!found_app_napt66()){
		showhide_div('div_napt66', 0);
	}
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Services_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(!validate_range(document.form.http_lanport, 80, 65535))
		return false;

	if (support_http_ssl()){
		var mode = document.form.http_proto.value;
		if (mode == "0" || mode == "2"){
			if(!validate_range(document.form.http_lanport, 80, 65535))
				return false;
		}
		if (mode == "1" || mode == "2"){
			if(!validate_range(document.form.https_lport, 81, 65535))
				return false;
		}
		if (mode == "2"){
			if (document.form.http_lanport.value == document.form.https_lport.value){
				alert("HTTP and HTTPS ports is equal!");
				document.form.https_lport.focus();
				document.form.https_lport.select();
				return false;
			}
		}
	}else{
		if(!validate_range(document.form.http_lanport, 80, 65535))
			return false;
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

function textarea_https_enabled(v){
	inputCtrl(document.form['httpssl.ca.crt'], v);
	inputCtrl(document.form['httpssl.dh1024.pem'], v);
	inputCtrl(document.form['httpssl.server.crt'], v);
	inputCtrl(document.form['httpssl.server.key'], v);
}

function textarea_sshd_enabled(v){
	inputCtrl(document.form['scripts.authorized_keys'], v);
}

function textarea_crond_enabled(v){
	inputCtrl(document.form['crontab.login'], v);
}

function http_proto_change(){
	var proto = document.form.http_proto.value;
	var v1 = (proto == "0" || proto == "2") ? 1 : 0;
	var v2 = (proto == "1" || proto == "2") ? 1 : 0;

	showhide_div('row_http_lport', v1);
	showhide_div('row_https_lport', v2);

	if (!login_safe())
		v2 = 0;

	showhide_div('row_https_clist', v2);
	showhide_div('tbl_https_certs', v2);
	textarea_https_enabled(v2);
}

var id_timeout_btn_gen;
function flashing_btn_gen(is_shown){
	var $btn=$j('#https_gen_bn');
	if (is_shown)
		$btn.val('Please wait...');
	else
		$btn.val('');
	id_timeout_btn_gen = setTimeout("flashing_btn_gen("+!is_shown+")", 250);
}

function reset_btn_gen(is_refresh){
	var $btn=$j('#https_gen_bn');
	$btn.removeClass('alert-error').removeClass('alert-success');
	$btn.val('<#VPNS_GenNew#>');
	if (is_refresh)
		location.href = location.href;
}

function create_server_cert() {
	if(!confirm('<#Adm_System_https_query#>'))
		return false;
	var $btn=$j('#https_gen_bn');
	flashing_btn_gen(1);
	$btn.addClass('alert-error');
	$j.ajax({
		type: "post",
		url: "/apply.cgi",
		data: {
			action_mode: " CreateCertHTTPS ",
			common_name: $('https_gen_cn').value,
			rsa_bits: $('https_gen_rb').value,
			days_valid: $('https_gen_dv').value
		},
		dataType: "json",
		error: function(xhr) {
			clearTimeout(id_timeout_btn_gen);
			$btn.val('Failed!');
			setTimeout("reset_btn_gen(0)", 1500);
		},
		success: function(response) {
			var sys_result = (response != null && typeof response === 'object' && "sys_result" in response)
				? response.sys_result : -1;
			clearTimeout(id_timeout_btn_gen);
			if(sys_result == 0){
				$btn.removeClass('alert-error').addClass('alert-success');
				$btn.val('Success!');
				setTimeout("reset_btn_gen(1)", 1000);
			}else{
				$btn.val('Failed!');
				setTimeout("reset_btn_gen(0)", 1500);
			}
		}
	});
}

function sshd_auth_change(){
	var auth = document.form.sshd_enable.value;
	var v = (auth != "0") ? 1 : 0;
	showhide_div('row_ssh_keys', v);
	if (!login_safe())
		v = 0;
	textarea_sshd_enabled(v);
}

function change_wins_enabled(){
	var v = document.form.wins_enable[0].checked;
	showhide_div('row_smb_wgrp', v);
	showhide_div('row_smb_lmb', v);
}

function change_crond_enabled(){
	var v = document.form.crond_enable[0].checked;
	showhide_div('row_crontabs', v);
	if (!login_safe())
		v = 0;
	textarea_crond_enabled(v);
}

function change_ttyd_enabled(){
	var v = document.form.ttyd_enable[0].checked;
	showhide_div('ttyd_webui', v);
	showhide_div('ttyd_port', v);
}

function on_ttyd_link(){
	var ttyd_url="http";
	var http_url=lan_ipaddr;
	ttyd_url+="://"+http_url+":"+"<% nvram_get_x("","ttyd_port"); %>";
	window_ttyd = window.open(ttyd_url, "ttyd");
	window_ttyd.focus();
}
</script>
<style>
    .caption-bold {
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
    <input type="hidden" name="current_page" value="Advanced_Services_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;General;Storage;">
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
                            <h2 class="box_head round_top"><#menu5_6#> - <#menu5_6_5#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#Adm_Svc_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_System_webs#></th>
                                        </tr>
                                        <tr id="row_http_proto">
                                            <th><#Adm_System_http_proto#></th>
                                            <td>
                                                <select name="http_proto" class="input" onchange="http_proto_change();">
                                                    <option value="0" <% nvram_match_x("", "http_proto", "0","selected"); %>>HTTP</option>
                                                    <option value="1" <% nvram_match_x("", "http_proto", "1","selected"); %>>HTTPS</option>
                                                    <option value="2" <% nvram_match_x("", "http_proto", "2","selected"); %>>HTTP & HTTPS</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_http_lport">
                                            <th><#Adm_System_http_lport#></th>
                                            <td>
                                                <input type="text" maxlength="5" size="15" name="http_lanport" class="input" value="<% nvram_get_x("", "http_lanport"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[80..65535]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_https_lport" style="display:none">
                                            <th><#Adm_System_https_lport#></th>
                                            <td>
                                                <input type="text" maxlength="5" size="15" name="https_lport" class="input" value="<% nvram_get_x("", "https_lport"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[81..65535]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_https_clist" style="display:none">
                                            <th><#Adm_System_https_clist#></th>
                                            <td>
                                                <input type="text" maxlength="256" size="15" name="https_clist" class="input" style="width: 286px;" value="<% nvram_get_x("", "https_clist"); %>" onkeypress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#Adm_System_http_access#></th>
                                            <td>
                                                <select name="http_access" class="input">
                                                    <option value="0" <% nvram_match_x("", "http_access", "0","selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "http_access", "1","selected"); %>>Wired clients only</option>
                                                    <option value="2" <% nvram_match_x("", "http_access", "2","selected"); %>>Wired and MainAP clients</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_https_certs" style="display:none">
                                        <tr>
                                            <th colspan="4" style="background-color: #E3E3E3;"><#Adm_System_https_certs#></th>
                                        </tr>
                                        <tr id="row_https_gen" style="display:none">
                                            <td align="right" style="text-align:right;">
                                                <span class="caption-bold">Server CN:</span>
                                                <input id="https_gen_cn" type="text" maxlength="32" size="10" style="width: 105px;" placeholder="my.domain" onKeyPress="return is_string(this,event);"/>
                                            </td>
                                            <td align="left">
                                                <span class="caption-bold">Bits:</span>
                                                <select id="https_gen_rb" class="input" style="width: 108px;">
                                                    <option value="1024">RSA 1024 (*)</option>
                                                    <option value="2048">RSA 2048</option>
                                                    <option value="4096">RSA 4096</option>
                                                    <option value="prime256v1">EC P-256</option>
                                                    <option value="secp384r1">EC P-384</option>
                                                    <option value="secp521r1">EC P-521</option>
                                                </select>
                                            </td>
                                            <td align="left">
                                                <span class="caption-bold">Days valid:</span>
                                                <input id="https_gen_dv" type="text" maxlength="5" size="10" style="width: 35px;" value="365" onKeyPress="return is_number(this,event);"/>
                                            </td>
                                            <td align="left">
                                                <input id="https_gen_bn" type="button" class="btn" style="width: 145px; outline:0" onclick="create_server_cert();" value="<#VPNS_GenNew#>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="4" style="padding-bottom: 0px;">
                                                <a href="javascript:spoiler_toggle('ca.crt')"><span>Root CA Certificate (optional)</span></a>
                                                <div id="ca.crt" style="display:none;">
                                                    <textarea rows="8" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="httpssl.ca.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("httpssl.ca.crt",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="4" style="padding-bottom: 0px; border-top: 0 none;">
                                                <a href="javascript:spoiler_toggle('dh1024.pem')"><span>Diffie-Hellman PEM (optional)</span></a>
                                                <div id="dh1024.pem" style="display:none;">
                                                    <textarea rows="8" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="httpssl.dh1024.pem" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("httpssl.dh1024.pem",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="4" style="padding-bottom: 0px; border-top: 0 none;">
                                                <a href="javascript:spoiler_toggle('server.crt')"><span>Server Certificate (required)</span></a>
                                                <div id="server.crt" style="display:none;">
                                                    <textarea rows="8" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="httpssl.server.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("httpssl.server.crt",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="4" style="padding-bottom: 0px; border-top: 0 none;">
                                                <a href="javascript:spoiler_toggle('server.key')"><span>Server Private Key (required)</span></a>
                                                <div id="server.key" style="display:none;">
                                                    <textarea rows="8" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="httpssl.server.key" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("httpssl.server.key",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_System_term#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#Adm_System_telnetd#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="telnetd_on_of">
                                                        <input type="checkbox" id="telnetd_fake" <% nvram_match_x("", "telnetd", "1", "value=1 checked"); %><% nvram_match_x("", "telnetd", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="telnetd" id="telnetd_1" class="input" value="1" <% nvram_match_x("", "telnetd", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="telnetd" id="telnetd_0" class="input" value="0" <% nvram_match_x("", "telnetd", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_sshd">
                                            <th><#Adm_System_sshd#></th>
                                            <td>
                                                <select name="sshd_enable" class="input" onchange="sshd_auth_change();">
                                                    <option value="0" <% nvram_match_x("", "sshd_enable", "0","selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "sshd_enable", "1","selected"); %>><#checkbox_Yes#></option>
                                                    <option value="2" <% nvram_match_x("", "sshd_enable", "2","selected"); %>><#checkbox_Yes#> (authorized_keys only)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ssh_keys" style="display:none">
                                            <td colspan="2" style="padding-bottom: 0px;">
                                                <a href="javascript:spoiler_toggle('authorized_keys')"><span><#Adm_System_sshd_keys#> (authorized_keys)</span></a>
                                                <div id="authorized_keys" style="display:none;">
                                                    <textarea rows="8" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="scripts.authorized_keys" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.authorized_keys",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_wins" style="display:none">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">Windows Internet Name Service (WINS)</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#Adm_Svc_wins#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wins_enable_on_of">
                                                        <input type="checkbox" id="wins_enable_fake" <% nvram_match_x("", "wins_enable", "1", "value=1 checked"); %><% nvram_match_x("", "wins_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="wins_enable" id="wins_enable_1" class="input" value="1" onclick="change_wins_enabled();" <% nvram_match_x("", "wins_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="wins_enable" id="wins_enable_0" class="input" value="0" onclick="change_wins_enabled();" <% nvram_match_x("", "wins_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_smb_wgrp" style="display:none;">
                                            <th>
                                                <a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,17, 3);"><#ShareNode_WorkGroup_itemname#></a>
                                            </th>
                                            <td>
                                                <input type="text" name="st_samba_workgroup" class="input" maxlength="32" size="32" value="<% nvram_get_x("", "st_samba_workgroup"); %>"/>
                                            </td>
                                        </tr>
                                        <tr id="row_smb_lmb" style="display:none;">
                                            <th>
                                                <#StorageLMB#>
                                            </th>
                                            <td>
                                                <select name="st_samba_lmb" class="input">
                                                    <option value="0" <% nvram_match_x("", "st_samba_lmb", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "st_samba_lmb", "1", "selected"); %>>Local Master Browser (*)</option>
                                                    <option value="2" <% nvram_match_x("", "st_samba_lmb", "2", "selected"); %>>Local & Domain Master Browser</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" id="tbl_ttyd" cellpadding="4" cellspacing="0" class="table" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_Svc_ttyd_setup#></th>
                                        </tr>
                                        <tr id="div_ttyd">
                                            <th width="50%"><#Adm_Svc_ttyd_enable#></th>
                                            <td colspan="2">
                                                <div class="main_itoggle">
                                                    <div id="ttyd_enable_on_of">
                                                        <input type="checkbox" id="ttyd_enable_fake" <% nvram_match_x("", "ttyd_enable", "1", "value=1 checked"); %><% nvram_match_x("", "ttyd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="ttyd_enable" id="ttyd_enable_1" class="input" value="1" onclick="change_ttyd_enabled();" <% nvram_match_x("", "ttyd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="ttyd_enable" id="ttyd_enable_0" class="input" value="0" onclick="change_ttyd_enabled();" <% nvram_match_x("", "ttyd_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="ttyd_port"> <th width="50%"><#Adm_Svc_ttyd_port#></th>
                                            <td>
                                                <input type="text" maxlength="6" class="input" size="15" name="ttyd_port" style="width: 145px" value="<% nvram_get_x("","ttyd_port"); %>" />
                                            </td>
                                        </tr>
                                        <tr id="ttyd_webui">
                                            <td>
                                                <a href="javascript:on_ttyd_link();" id="web_ttyd_link">ttyd Web Shell</a>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_System_misc#></th>
                                        </tr>
										
                                        <tr id="div_vlmcsd">
                                            <th><#Adm_Svc_vlmcsd#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="vlmcsd_enable_on_of">
                                                        <input type="checkbox" id="vlmcsd_enable_fake" <% nvram_match_x("", "vlmcsd_enable", "1", "value=1 checked"); %><% nvram_match_x("", "vlmcsd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="vlmcsd_enable" id="vlmcsd_enable_1" class="input" value="1" <% nvram_match_x("", "vlmcsd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="vlmcsd_enable" id="vlmcsd_enable_0" class="input" value="0" <% nvram_match_x("", "vlmcsd_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr id="div_napt66">
                                            <th><#Adm_Svc_napt66#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="napt66_enable_on_of">
                                                        <input type="checkbox" id="napt66_enable_fake" <% nvram_match_x("", "napt66_enable", "1", "value=1 checked"); %><% nvram_match_x("", "napt66_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="napt66_enable" id="napt66_enable_1" class="input" value="1" <% nvram_match_x("", "napt66_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="napt66_enable" id="napt66_enable_0" class="input" value="0" <% nvram_match_x("", "napt66_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr>
                                            <th><#Adm_Svc_lltd#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="lltd_enable_on_of">
                                                        <input type="checkbox" id="lltd_enable_fake" <% nvram_match_x("", "lltd_enable", "1", "value=1 checked"); %><% nvram_match_x("", "lltd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="lltd_enable" id="lltd_enable_1" class="input" value="1" <% nvram_match_x("", "lltd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="lltd_enable" id="lltd_enable_0" class="input" value="0" <% nvram_match_x("", "lltd_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_Svc_adsc#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="adsc_enable_on_of">
                                                        <input type="checkbox" id="adsc_enable_fake" <% nvram_match_x("", "adsc_enable", "1", "value=1 checked"); %><% nvram_match_x("", "adsc_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="adsc_enable" id="adsc_enable_1" class="input" value="1" <% nvram_match_x("", "adsc_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="adsc_enable" id="adsc_enable_0" class="input" value="0" <% nvram_match_x("", "adsc_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_Svc_crond#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="crond_enable_on_of">
                                                        <input type="checkbox" id="crond_enable_fake" <% nvram_match_x("", "crond_enable", "1", "value=1 checked"); %><% nvram_match_x("", "crond_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="crond_enable" id="crond_enable_1" class="input" value="1" <% nvram_match_x("", "crond_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="crond_enable" id="crond_enable_0" class="input" value="0" <% nvram_match_x("", "crond_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_crontabs" style="display:none">
                                            <td colspan="2">
                                                <a href="javascript:spoiler_toggle('crond_crontabs')"><span><#Adm_Svc_crontabs#></span></a>
                                                <div id="crond_crontabs" style="display:none;">
                                                    <textarea rows="8" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="crontab.login" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("crontab.login",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,23,1);"><#TweaksWdg#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="watchdog_cpu_on_of">
                                                        <input type="checkbox" id="watchdog_cpu_fake" <% nvram_match_x("", "watchdog_cpu", "1", "value=1 checked"); %><% nvram_match_x("", "watchdog_cpu", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="watchdog_cpu" id="watchdog_cpu_1" class="input" value="1" <% nvram_match_x("", "watchdog_cpu", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="watchdog_cpu" id="watchdog_cpu_0" class="input" value="0" <% nvram_match_x("", "watchdog_cpu", "0", "checked"); %>/><#checkbox_No#>
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
