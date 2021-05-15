<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_3_6#></title>
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
	init_itoggle('ddns_enable_x', change_ddns_enabled);
});

</script>
<script>

<% wanlink(); %>

var ddns_prov1 = '<% nvram_get_x("","ddns_server_x"); %>';
var ddns_prov2 = '<% nvram_get_x("","ddns2_server"); %>';
var ddns_hname = '<% nvram_get_x("","ddns_hostname_x"); %>';
var ddns_list = [
	[ 0x0f, "WWW.DYNDNS.ORG",       "", "https://account.dyn.com/entrance/" ],
	[ 0x0f, "WWW.ZONEEDIT.COM",     "", "http://www.zoneedit.com/signUp.html" ],
	[ 0x01, "WWW.EASYDNS.COM",      "", "https://web.easydns.com/Open_Account/" ],
	[ 0x0f, "WWW.NO-IP.COM",        "", "http://www.noip.com/newUser.php" ],
	[ 0x0f, "WWW.DNSOMATIC.COM",    "", "https://www.dnsomatic.com/create/" ],
	[ 0x0f, "WWW.DNSEXIT.COM",      "", "https://www.dnsexit.com/Direct.sv?cmd=signup" ],
	[ 0x0f, "WWW.CHANGEIP.COM",     "", "https://www.changeip.com/accounts/register.php" ],
	[ 0x0f, "WWW.SITELUTIONS.COM",  "", "https://sitelutions.com/signup" ],
	[ 0x0f, "WWW.DHIS.ORG",         "", "http://dhis.org/WebEngine.ipo?context=dhis.website.register" ],
	[ 0x0f, "WWW.DUCKDNS.ORG",      "", "https://www.duckdns.org/" ],
	[ 0x0f, "WWW.OVH.COM",          "", "https://www.ovh.com/" ],
	[ 0x0f, "WWW.LOOPIA.COM",       "", "https://www.loopia.com/loopiadns/" ],
	[ 0x0f, "WWW.DUIADNS.NET",      "", "https://www.duiadns.net/services" ],
	[ 0x0f, "WWW.TUNNELBROKER.NET", "(HE)", "http://www.tunnelbroker.net/register.php" ],
	[ 0x0f, "DNS.HE.NET",           "(HE)", "http://ipv6.he.net/certification/register.php" ],
	[ 0x0f, "DDNSS.DE",             "", "https://www.ddnss.de/user_new.php" ],
	[ 0x0f, "HOMESERVER.GIRA.DE",   "", "https://homeserver.gira.de/en/registrierung/index.html" ],
	[ 0x0f, "DOMAINS.GOOGLE.COM",   "", "https://domains.google.com/registrar" ],
	[ 0x0f, "IPV4.DYNV6.COM",       "", "https://ipv4.dynv6.com/users/sign_up" ],
	[ 0x0f, "DYNV6.COM",            "", "https://dynv6.com/users/sign_up" ],
	[ 0x0f, "TB.NETASSIST.UA",      "", "http://tb.netassist.ua/reg.php" ],
	[ 0x0f, "IPV4.NSUPDATE.INFO",   "", "https://nsupdate.info/account/register/" ],
	[ 0x0f, "FREEDNS.AFRAID.ORG",   "", "http://freedns.afraid.org/signup/" ],
	[ 0x0f, "FREEMYIP.COM",         "", "https://freemyip.com/" ],
	[ 0x0f, "SPDYN.DE",             "", "https://spdyn.de/" ],
	[ 0x0f, "WWW.STRATO.DE",        "", "https://www.strato.de/" ],
	[ 0x0f, "CLOUDXNS.NET",         "", "https://www.cloudxns.net/" ],
	[ 0x0f, "3322.ORG",             "", "http://www.pubyun.com/" ],
	[ 0x0f, "DNSPOD.CN",            "", "https://www.dnspod.cn/" ],
	[ 0x0f, "DYNU.COM",             "", "https://www.dynu.com/" ],
	[ 0x0f, "SELFHOST.DE",          "", "https://selfhost.de/cgi-bin/selfhost" ],
	[ 0x0f, "PDD.YANDEX.RU",        "", "https://connect.yandex.ru/admintools" ],
	[ 0x0f, "CLOUDFLARE.COM",       "", "https://www.cloudflare.com/products/registrar" ],
	[ 0x0f, "ORAY.COM",             "", "https://hsk.oray.com/" ],
	[ 0x01, "CUSTOM",               "(http basic auth)", "" ]
];

function init()
{
	var id_menu = 5;
	if(sw_mode == '4')
		id_menu = 3;
	if(!support_ipv6())
		id_menu--;

	show_banner(1);
	show_footer();
	show_menu(5,4,id_menu);

	ddns_init();

	hideLoading();
}

function test_wan_ip()
{
	var A_class_start = inet_network("10.0.0.0");
	var A_class_end = inet_network("10.255.255.255");
	var B_class_start = inet_network("172.16.0.0");
	var B_class_end = inet_network("172.31.255.255");
	var C_class_start = inet_network("192.168.0.0");
	var C_class_end = inet_network("192.168.255.255");

	var ip_obj = wanlink_ip4_wan();
	if (ip_obj == '---')
		ip_obj = "";
	var ip_num = inet_network(ip_obj);
	var ip_class = "";

	if(ip_num > A_class_start && ip_num < A_class_end)
		ip_class = 'A';
	else if(ip_num > B_class_start && ip_num < B_class_end)
		ip_class = 'B';
	else if(ip_num > C_class_start && ip_num < C_class_end)
		ip_class = 'C';
	else if(ip_num != 0){
		showhide("wan_ip_hide2", 0);
		showhide("wan_ip_hide3", 0);
		return;
	}
	showhide("wan_ip_hide2", 1);
	showhide("wan_ip_hide3", 0);
}

function get_url_link(ddns)
{
	var url = "";
	if (ddns == "")
		return url;
	for(var i = 0; i < ddns_list.length; i++){
		if (ddns == ddns_list[i][1]){
			url = ddns_list[i][3];
			break;
		}
	}
	return url;
}

function disable_update()
{
	document.form.x_DDNSStatus.disabled = 1;
}

function change_ddns_server(man)
{
	var v = document.form.ddns_server_x.value;
	var o = document.form.x_DDNSHostCheck;
	var e = (v == "WWW.ASUS.COM") ? 0 : 1;
	var tourl = get_url_link(v);

	showhide("ddnsname_input", e);
	showhide("asusddnsname_input", !e);
	showhide_div("ddns_link", (tourl != ""));
	showhide_div("row_ddns_hname2", e);
	showhide_div("row_ddns_hname3", e);
	showhide_div("row_ddns_user", e);
	showhide_div("row_ddns_pass", e);
	showhide_div("row_ddns_ssl", (e && support_ddns_ssl()));
	o.disabled = e;

	showhide_div("row_ddns_wcard", 1);

	e = (v == "CUSTOM") ? 1 : 0;
	showhide_div("row_ddns_cst_svr", e);
	showhide_div("row_ddns_cst_url", e);
	if (man)
		disable_update();
}

function change_ddns2_server(man)
{
	var v = document.form.ddns2_server.value;
	var e = (v == "") ? 0 : 1;
	var tourl = get_url_link(v);

	showhide_div("ddns2_link", (tourl != ""));
	showhide_div("row_ddns2_hname", e);
	showhide_div("row_ddns2_user", e);
	showhide_div("row_ddns2_pass", e);
	showhide_div("row_ddns2_wcard", e);
	showhide_div("row_ddns2_ssl", (e && support_ddns_ssl()));
	if (man)
		disable_update();
}

function change_ddns_source(man)
{
	var e = (document.form.ddns_source.value == "0") ? 1 : 0;
	showhide_div("row_ddns_checkip", e);
	showhide_div("row_ddns2_checkip", e);
	if (man)
		disable_update();
}

function ddns_enable(man)
{
	test_wan_ip();
	change_ddns_server(man);
	showhide_div("row_ddns_server", 1);
	showhide_div("row_ddns_hname1", 1);
	change_ddns2_server(man);
	change_ddns_source(man);
	showhide_div("tbl_ddns2", 1);
	showhide_div("tbl_common", 1);
}

function ddns_disable()
{
	showhide("wan_ip_hide2", 0);
	showhide("wan_ip_hide3", 0);
	showhide_div("row_ddns_server", 0);
	showhide_div("row_ddns_cst_svr", 0);
	showhide_div("row_ddns_cst_url", 0);
	showhide_div("row_ddns_hname1", 0);
	showhide_div("row_ddns_hname2", 0);
	showhide_div("row_ddns_hname3", 0);
	showhide_div("row_ddns_user", 0);
	showhide_div("row_ddns_pass", 0);
	showhide_div("row_ddns_ssl", 0);
	showhide_div("row_ddns_wcard", 0);
	showhide_div("tbl_ddns2", 0);
	showhide_div("tbl_common", 0);
}

function change_ddns_enabled()
{
	if (document.form.ddns_enable_x[0].checked)
		ddns_enable(1);
	else
		ddns_disable();
}

function ddns_init()
{
	if (ddns_prov1 == '')
		ddns_prov1 = "WWW.DYNDNS.ORG";

	fill_provider_list("ddns_server_x", ddns_prov1, 0x01);
	fill_provider_list("ddns2_server",  ddns_prov2, 0x02);

	if(document.form.ddns_server_x.selectedIndex == 0){
		if(ddns_hname != '')
			$("DDNSName").value = ddns_hname.substring(0, ddns_hname.indexOf('.'));
	}else
		$("ddns_hostname_x").value = ddns_hname;

	if (document.form.ddns_enable_x[0].checked) {
		ddns_enable(0);
		if (ddns_prov1 == "WWW.ASUS.COM")
			show_asus_alert(ddns_hname);
	}else
		ddns_disable();
}

function fill_provider_list(oname,sel,mask){
	var o = document.form[oname];
	if(o === undefined)
		return;
	if(!(mask&0x01))
		add_option(o, "<#btn_Disable#>", "", (sel=="") ? 1 : 0);
	for(var i = 0; i < ddns_list.length; i++){
		if (!(ddns_list[i][0]&mask))
			continue;
		var caption = ddns_list[i][1].toLowerCase();
		if(ddns_list[i][2] != "")
			caption += " " + ddns_list[i][2];
		add_option(o, caption, ddns_list[i][1], (sel==ddns_list[i][1]) ? 1 : 0);
	}
}

function show_asus_alert(hname)
{
	var ddns_return_code = '<% nvram_get_ddns("","ddns_return_code"); %>';

	if(ddns_return_code == 'register,-1')
		alert("<#LANHostConfig_x_DDNS_alarm_2#>");
	else if(ddns_return_code == 'register,200') {
		showhide("wan_ip_hide3", 1);
		alert("<#LANHostConfig_x_DDNS_alarm_3#>");
	}
	else if(ddns_return_code == 'register,203')
		alert("<#LANHostConfig_x_DDNS_alarm_hostname#> '"+hname+"' <#LANHostConfig_x_DDNS_alarm_registered#>");
	else if(ddns_return_code == 'register,220') {
		showhide("wan_ip_hide3", 1);
		alert("<#LANHostConfig_x_DDNS_alarm_4#>");
	}
	else if(ddns_return_code == 'register,230') {
		showhide("wan_ip_hide3", 1);
		alert("<#LANHostConfig_x_DDNS_alarm_5#>");
	}
	else if(ddns_return_code == 'register,233')
		alert("<#LANHostConfig_x_DDNS_alarm_hostname#> '"+hname+"' <#LANHostConfig_x_DDNS_alarm_registered_2#>");
	else if(ddns_return_code == 'register,296')
		alert("<#LANHostConfig_x_DDNS_alarm_6#>");
	else if(ddns_return_code == 'register,297'){
		document.form.ddns_hostname_x.value = ""; 
		alert("<#LANHostConfig_x_DDNS_alarm_7#>");
	}
	else if(ddns_return_code == 'register,298'){
		document.form.ddns_hostname_x.value = ""; 
		alert("<#LANHostConfig_x_DDNS_alarm_8#>");
	}
	else if(ddns_return_code == 'register,299')
		alert("<#LANHostConfig_x_DDNS_alarm_9#>");
	else if(ddns_return_code == 'register,401')
		alert("<#LANHostConfig_x_DDNS_alarm_10#>");
	else if(ddns_return_code == 'register,407')
		alert("<#LANHostConfig_x_DDNS_alarm_11#>");
	else if(ddns_return_code == 'time_out')
		alert("<#LANHostConfig_x_DDNS_alarm_1#>");
	else if(ddns_return_code =='unknown_error')
		alert("<#LANHostConfig_x_DDNS_alarm_2#>");
	else if(ddns_return_code =='connect_fail')
		alert("<#LANHostConfig_x_DDNS_alarm_12#>");
	else if(ddns_return_code == 'inadyn_unsupport')
		alert("inadyn does not support register to asuscomm.com");
}

function openLink(s) {
	var link_params = "toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=480";
	var o = (s == 'x_DDNSServer2') ? document.form.ddns2_server : document.form.ddns_server_x;
	var tourl = get_url_link(o.value);
	if (tourl == "")
		return;
	link = window.open(tourl, "DDNSLink", link_params);
	if (!link.opener) link.opener = self;
}

function validForm(){
	if(document.form.ddns_enable_x[0].checked){
		var o1 = document.form.ddns_server_x;
		var o2 = document.form.ddns2_server;
		var o3 = document.form.ddns_hostname_x;
		if(o1.value == "WWW.ASUS.COM"){
			var o4 = document.form.DDNSName;
			if(o4.value == "" || !validate_ddns_hostname(o4)){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				o4.focus();
				o4.select();
				return false;
			}else{
				o3.value = o4.value+".asuscomm.com";
			}
		}else{
			if(o3.value == ""){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				o3.focus();
				o3.select();
				return false;
			}
		}
		if(o1.value == "CUSTOM"){
			var o5 = document.form.ddns_cst_svr;
			if(o5.value == ""){
				alert("<#JS_fieldblank#>");
				o5.focus();
				o5.select();
				return false;
			}
			var o6 = document.form.ddns_cst_url;
			if(o6.value == ""){
				alert("<#JS_fieldblank#>");
				o6.focus();
				o6.select();
				return false;
			}
		}
		if(document.form.ddns2_server.value != ""){
			var o8 = document.form.ddns2_hname;
			if(o8.value == ""){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				o8.focus();
				o8.select();
				return false;
			}
		}
	}

	return true;
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_DDNS_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function checkDDNSReturnCode(){
	$j.ajax({
		url: '/ajax_ddnscode.asp',
		dataType: 'script',
		error: function(xhr){
			checkDDNSReturnCode();
		},
		success: function(response){
			if(ddns_return_code == 'ddns_query')
				setTimeout("checkDDNSReturnCode();", 500);
			else
				refreshpage();
		}
	});
}
</script>
</head>

<body onload="init();" onunLoad="return unload_body();">

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
    <input type="hidden" name="current_page" value="Advanced_DDNS_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
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
                            <h2 class="box_head round_top"><#menu5_3#> - <#menu5_3_6#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#LANHostConfig_x_DDNSEnable_sectiondesc#></div>
                                    <div id="wan_ip_hide2" class="alert alert-danger" style="display:none; margin: 10px;"><#LANHostConfig_x_DDNSEnable_sectiondesc2#></div>
                                    <div id="wan_ip_hide3" class="alert alert-danger" style="display:none; margin: 10px;"><#LANHostConfig_x_DDNSEnable_sectiondesc3#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#LANHostConfig_x_DDNSEnable_itemname#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ddns_enable_x_on_of">
                                                        <input type="checkbox" id="ddns_enable_x_fake" <% nvram_match_x("", "ddns_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "ddns_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ddns_enable_x" id="ddns_enable_x_1" onClick="change_ddns_enabled();" <% nvram_match_x("","ddns_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ddns_enable_x" id="ddns_enable_x_0" onClick="change_ddns_enabled();" <% nvram_match_x("","ddns_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_server">
                                            <th><#LANHostConfig_x_DDNSServer_itemname#></th>
                                            <td>
                                                <select name="ddns_server_x" class="input" onchange="change_ddns_server(1)">
                                                </select>&nbsp;
                                                <a id="ddns_link" href="javascript:openLink('x_DDNSServer')" class="label label-info" name="x_DDNS_link"><#LANHostConfig_x_DDNSServer_linkname#></a>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_cst_svr" style="display:none;">
                                            <th><#DDNS_SVR#></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" name="ddns_cst_svr" value="<% nvram_get_x("","ddns_cst_svr"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_cst_url" style="display:none;">
                                            <th><#DDNS_URL#></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns_cst_url" value="<% nvram_get_x("","ddns_cst_url"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_hname1">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <div id="ddnsname_input" style="display:none;">
                                                    <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" id="ddns_hostname_x" name="ddns_hostname_x" value="<% nvram_get_x("","ddns_hostname_x"); %>" onKeyPress="return is_string(this,event);" />
                                                </div>
                                                <div id="asusddnsname_input" style="display:none;">
                                                    <input type="text" name="DDNSName" id="DDNSName" style="width: 110px;" placeholder="<#asusddns_inputhint#>">.asuscomm.com&nbsp;&nbsp;
                                                    <input type="submit" maxlength="15" size="15" class="btn btn-info" style="max-width: 98px;" onClick="return onSubmitApply('hostname_check');" name="x_DDNSHostCheck" value="<#LANHostConfig_x_DDNSHostnameCheck_buttonname#>"/>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_hname2">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns_hostname2_x" value="<% nvram_get_x("","ddns_hostname2_x"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_hname3">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns_hostname3_x" value="<% nvram_get_x("","ddns_hostname3_x"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_user">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,1);"><#LANHostConfig_x_DDNSUserName_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns_username_x" value="<% nvram_get_x("","ddns_username_x"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_pass">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,2);"><#LANHostConfig_x_DDNSPassword_itemname#></a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" maxlength="64" class="input" size="32" name="ddns_passwd_x" id="ddns_passwd_x" style="width: 175px;" value="<% nvram_get_x("","ddns_passwd_x"); %>" />
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('ddns_passwd_x')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_ssl" style="display:none;">
                                            <th><#DDNS_SSL#></th>
                                            <td>
                                                <select name="ddns_ssl" class="input">
                                                    <option value="0" <% nvram_match_x("", "ddns_ssl", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns_ssl", "1","selected"); %>><#checkbox_Yes#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_wcard">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,4);"><#LANHostConfig_x_DDNSWildcard_itemname#></a></th>
                                            <td>
                                                <select name="ddns_wildcard_x" class="input">
                                                    <option value="0" <% nvram_match_x("", "ddns_wildcard_x", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns_wildcard_x", "1","selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_ddns2" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#DDNS_Second#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#LANHostConfig_x_DDNSServer_itemname#></th>
                                            <td>
                                                <select name="ddns2_server" class="input" onchange="change_ddns2_server(1)">
                                                </select>&nbsp;
                                                <a id="ddns2_link" href="javascript:openLink('x_DDNSServer2')" class="label label-info" name="x_DDNS2_link"><#LANHostConfig_x_DDNSServer_linkname#></a>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_hname">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns2_hname" value="<% nvram_get_x("","ddns2_hname"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_user">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,1);"><#LANHostConfig_x_DDNSUserName_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns2_user" value="<% nvram_get_x("","ddns2_user"); %>" onKeyPress="return is_string(this,event);" />
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_pass">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,2);"><#LANHostConfig_x_DDNSPassword_itemname#></a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" maxlength="64" class="input" size="32" name="ddns2_pass" id="ddns2_pass" style="width: 175px;" value="<% nvram_get_x("","ddns2_pass"); %>">
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('ddns2_pass')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_ssl" style="display:none;">
                                            <th><#DDNS_SSL#></th>
                                            <td>
                                                <select name="ddns2_ssl" class="input" onchange="disable_update();">
                                                    <option value="0" <% nvram_match_x("", "ddns2_ssl", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns2_ssl", "1","selected"); %>><#checkbox_Yes#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_wcard">
                                            <th><#LANHostConfig_x_DDNSWildcard_itemname#></a></th>
                                            <td>
                                                <select name="ddns2_wildcard_x" class="input">
                                                    <option value="0" <% nvram_match_x("", "ddns2_wildcard_x", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns2_wildcard_x", "1","selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_common" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#DDNS_Common#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#DDNS_Source#></th>
                                            <td>
                                                <select name="ddns_source" class="input" onchange="change_ddns_source(1)">
                                                    <option value="0" <% nvram_match_x("", "ddns_source", "0","selected"); %>><#DDNS_AddrExt#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns_source", "1","selected"); %>><#DDNS_AddrWAN#></option>
                                                    <option value="2" <% nvram_match_x("", "ddns_source", "2","selected"); %>><#DDNS_AddrMAN#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_checkip" style="display:none;">
                                            <th><#DDNS_CheckIP#></th>
                                            <td>
                                                <select name="ddns_checkip" class="input" onchange="disable_update();">
                                                    <option value="0" <% nvram_match_x("", "ddns_checkip", "0","selected"); %>><#DDNS_CheckIP_item0#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns_checkip", "1","selected"); %>>checkip.dyndns.org</option>
                                                    <option value="2" <% nvram_match_x("", "ddns_checkip", "2","selected"); %>>checkip.dyndns.org:8245</option>
                                                    <option value="3" <% nvram_match_x("", "ddns_checkip", "3","selected"); %>>ip.dnsexit.com</option>
                                                    <option value="4" <% nvram_match_x("", "ddns_checkip", "4","selected"); %>>ip.changeip.com</option>
                                                    <option value="5" <% nvram_match_x("", "ddns_checkip", "5","selected"); %>>myip.dnsomatic.com</option>
                                                    <option value="6" <% nvram_match_x("", "ddns_checkip", "6","selected"); %>>ip1.dynupdate.no-ip.com</option>
                                                    <option value="7" <% nvram_match_x("", "ddns_checkip", "7","selected"); %>>checkip.dns.he.net (dual stack)</option>
                                                    <option value="8" <% nvram_match_x("", "ddns_checkip", "8","selected"); %>>checkip.two-dns.de</option>
                                                    <option value="9" <% nvram_match_x("", "ddns_checkip", "9","selected"); %>>ip.3322.net</option>
                                                    <option value="10" <% nvram_match_x("", "ddns_checkip", "10","selected"); %>>myip.ipip.net/s</option>
                                                    <option value="11" <% nvram_match_x("", "ddns_checkip", "11","selected"); %>>api.myip.la (dual stack)</option>
                                                    <option value="12" <% nvram_match_x("", "ddns_checkip", "12","selected"); %>>ipv4.wtfismyip.com/text</option>
                                                    <option value="13" <% nvram_match_x("", "ddns_checkip", "13","selected"); %>>ipv6.wtfismyip.com/text</option>
                                                    <option value="14" <% nvram_match_x("", "ddns_checkip", "14","selected"); %>>ipv4.nsupdate.info/myip</option>
                                                    <option value="15" <% nvram_match_x("", "ddns_checkip", "15","selected"); %>>ipv6.nsupdate.info/myip</option>
                                                    <option value="16" <% nvram_match_x("", "ddns_checkip", "16","selected"); %>>api-ipv4.ip.sb/ip</option>
                                                    <option value="17" <% nvram_match_x("", "ddns_checkip", "17","selected"); %>>api-ipv6.ip.sb/ip</option>
                                                    <option value="18" <% nvram_match_x("", "ddns_checkip", "18","selected"); %>>v4.ident.me</option>
                                                    <option value="19" <% nvram_match_x("", "ddns_checkip", "19","selected"); %>>v6.ident.me</option>
                                                    <option value="20" <% nvram_match_x("", "ddns_checkip", "20","selected"); %>>api4.my-ip.io/ip</option>
                                                    <option value="21" <% nvram_match_x("", "ddns_checkip", "21","selected"); %>>api6.my-ip.io/ip</option>
                                                    <option value="22" <% nvram_match_x("", "ddns_checkip", "22","selected"); %>>api.ipify.org</option>
                                                    <option value="23" <% nvram_match_x("", "ddns_checkip", "23","selected"); %>>api6.ipify.org</option>
                                                    <option value="24" <% nvram_match_x("", "ddns_checkip", "24","selected"); %>>ipv4.duiadns.net</option>
                                                    <option value="25" <% nvram_match_x("", "ddns_checkip", "25","selected"); %>>ipv6.duiadns.net</option>
                                                    <option value="26" <% nvram_match_x("", "ddns_checkip", "26","selected"); %>>checkip4.spdyn.de</option>
                                                    <option value="27" <% nvram_match_x("", "ddns_checkip", "27","selected"); %>>checkip6.spdyn.de</option>
                                                    <option value="28" <% nvram_match_x("", "ddns_checkip", "28","selected"); %>>v4.ipv6-test.com/api/myip.php</option>
                                                    <option value="29" <% nvram_match_x("", "ddns_checkip", "29","selected"); %>>v6.ipv6-test.com/api/myip.php</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_checkip" style="display:none;">
                                            <th><#DDNS2_CheckIP#></th>
                                            <td>
                                                <select name="ddns2_checkip" class="input" onchange="disable_update();">
                                                    <option value="0" <% nvram_match_x("", "ddns2_checkip", "0","selected"); %>><#DDNS_CheckIP_item0#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns2_checkip", "1","selected"); %>>checkip.dyndns.org</option>
                                                    <option value="2" <% nvram_match_x("", "ddns2_checkip", "2","selected"); %>>checkip.dyndns.org:8245</option>
                                                    <option value="3" <% nvram_match_x("", "ddns2_checkip", "3","selected"); %>>ip.dnsexit.com</option>
                                                    <option value="4" <% nvram_match_x("", "ddns2_checkip", "4","selected"); %>>ip.changeip.com</option>
                                                    <option value="5" <% nvram_match_x("", "ddns2_checkip", "5","selected"); %>>myip.dnsomatic.com</option>
                                                    <option value="6" <% nvram_match_x("", "ddns2_checkip", "6","selected"); %>>ip1.dynupdate.no-ip.com</option>
                                                    <option value="7" <% nvram_match_x("", "ddns2_checkip", "7","selected"); %>>checkip.dns.he.net (dual stack)</option>
                                                    <option value="8" <% nvram_match_x("", "ddns2_checkip", "8","selected"); %>>checkip.two-dns.de</option>
                                                    <option value="9" <% nvram_match_x("", "ddns2_checkip", "9","selected"); %>>ip.3322.net</option>
                                                    <option value="10" <% nvram_match_x("", "ddns2_checkip", "10","selected"); %>>myip.ipip.net/s</option>
                                                    <option value="11" <% nvram_match_x("", "ddns2_checkip", "11","selected"); %>>api.myip.la (dual stack)</option>
                                                    <option value="12" <% nvram_match_x("", "ddns2_checkip", "12","selected"); %>>ipv4.wtfismyip.com/text</option>
                                                    <option value="13" <% nvram_match_x("", "ddns2_checkip", "13","selected"); %>>ipv6.wtfismyip.com/text</option>
                                                    <option value="14" <% nvram_match_x("", "ddns2_checkip", "14","selected"); %>>ipv4.nsupdate.info/myip</option>
                                                    <option value="15" <% nvram_match_x("", "ddns2_checkip", "15","selected"); %>>ipv6.nsupdate.info/myip</option>
                                                    <option value="16" <% nvram_match_x("", "ddns2_checkip", "16","selected"); %>>api-ipv4.ip.sb/ip</option>
                                                    <option value="17" <% nvram_match_x("", "ddns2_checkip", "17","selected"); %>>api-ipv6.ip.sb/ip</option>
                                                    <option value="18" <% nvram_match_x("", "ddns2_checkip", "18","selected"); %>>v4.ident.me</option>
                                                    <option value="19" <% nvram_match_x("", "ddns2_checkip", "19","selected"); %>>v6.ident.me</option>
                                                    <option value="20" <% nvram_match_x("", "ddns2_checkip", "20","selected"); %>>api4.my-ip.io/ip</option>
                                                    <option value="21" <% nvram_match_x("", "ddns2_checkip", "21","selected"); %>>api6.my-ip.io/ip</option>
                                                    <option value="22" <% nvram_match_x("", "ddns2_checkip", "22","selected"); %>>api.ipify.org</option>
                                                    <option value="23" <% nvram_match_x("", "ddns2_checkip", "23","selected"); %>>api6.ipify.org</option>
                                                    <option value="24" <% nvram_match_x("", "ddns2_checkip", "24","selected"); %>>ipv4.duiadns.net</option>
                                                    <option value="25" <% nvram_match_x("", "ddns2_checkip", "25","selected"); %>>ipv6.duiadns.net</option>
                                                    <option value="26" <% nvram_match_x("", "ddns2_checkip", "26","selected"); %>>checkip4.spdyn.de</option>
                                                    <option value="27" <% nvram_match_x("", "ddns2_checkip", "27","selected"); %>>checkip6.spdyn.de</option>
                                                    <option value="28" <% nvram_match_x("", "ddns2_checkip", "28","selected"); %>>v4.ipv6-test.com/api/myip.php</option>
                                                    <option value="29" <% nvram_match_x("", "ddns2_checkip", "29","selected"); %>>v6.ipv6-test.com/api/myip.php</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#DDNS_Period#></th>
                                            <td>
                                                <select name="ddns_period" class="input" onchange="disable_update();">
                                                    <option value="0" <% nvram_match_x("", "ddns_period", "0","selected"); %>>10 mins</option>
                                                    <option value="1" <% nvram_match_x("", "ddns_period", "1","selected"); %>>1 hour</option>
                                                    <option value="2" <% nvram_match_x("", "ddns_period", "2","selected"); %>>2 hours</option>
                                                    <option value="3" <% nvram_match_x("", "ddns_period", "3","selected"); %>>3 hours</option>
                                                    <option value="6" <% nvram_match_x("", "ddns_period", "6","selected"); %>>6 hours</option>
                                                    <option value="12" <% nvram_match_x("", "ddns_period", "12","selected"); %>>12 hours</option>
                                                    <option value="24" <% nvram_match_x("", "ddns_period", "24","selected"); %>>1 day (*)</option>
                                                    <option value="48" <% nvram_match_x("", "ddns_period", "48","selected"); %>>2 days</option>
                                                    <option value="72" <% nvram_match_x("", "ddns_period", "72","selected"); %>>3 days</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#DDNS_Forced#></th>
                                            <td>
                                                <select name="ddns_forced" class="input" onchange="disable_update();">
                                                    <option value="7" <% nvram_match_x("", "ddns_forced", "7","selected"); %>>7 days</option>
                                                    <option value="10" <% nvram_match_x("", "ddns_forced", "10","selected"); %>>10 days (*)</option>
                                                    <option value="20" <% nvram_match_x("", "ddns_forced", "20","selected"); %>>20 days</option>
                                                    <option value="30" <% nvram_match_x("", "ddns_forced", "30","selected"); %>>30 days</option>
                                                </select>&nbsp;
                                                <input type="submit" maxlength="15" size="15" class="btn btn-info" style="max-width: 94px;" onclick="return onSubmitApply('ddnsclient');" name="x_DDNSStatus" value="<#LANHostConfig_x_DDNSStatus_buttonname#>"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#DDNS_IPv6#></th>
                                            <td>
                                                <select name="ddns_ipv6" class="input" onchange="disable_update();">
                                                    <option value="0" <% nvram_match_x("", "ddns_ipv6", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns_ipv6", "1","selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#DDNS_Verbose#></th>
                                            <td>
                                                <select name="ddns_verbose" class="input" onchange="disable_update();">
                                                    <option value="0" <% nvram_match_x("", "ddns_verbose", "0","selected"); %>>0 (Quiet)</option>
                                                    <option value="1" <% nvram_match_x("", "ddns_verbose", "1","selected"); %>>1 (Default)</option>
                                                    <option value="2" <% nvram_match_x("", "ddns_verbose", "2","selected"); %>>2 (Verbose)</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td style="border-top: 0 none;">
                                                <center><input type="button" class="btn btn-primary" style="width: 219px" value="<#CTL_apply#>" onclick="applyRule();"/></center>
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
