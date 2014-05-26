<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_3_6#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>

<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#ddns_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ddns_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#ddns_enable_x_1").attr("checked", "checked");
                $j("#ddns_enable_x_0").removeAttr("checked");
                enable_ddns(1,1);
            },
            onClickOff: function(){
                $j("#ddns_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#ddns_enable_x_0").attr("checked", "checked");
                $j("#ddns_enable_x_1").removeAttr("checked");
                enable_ddns(0,1);
            }
        });
        $j("#ddns_enable_x_on_of label.itoggle").css("background-position", $j("input#ddns_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });
</script>

<script>

<% wanlink(); %>
<% login_state_hook(); %>

function init()
{
	show_banner(1);
	show_footer();

	if(sw_mode == "4")
		show_menu(5,4,2);
	else
		show_menu(5,4,4);

	hideLoading();

	ddns_load_body();
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

function disable_update()
{
	document.form.x_DDNSStatus.disabled = 1;
}

function change_ddns_server(man)
{
	var v = document.form.ddns_server_x.value;
	var o = document.form.x_DDNSHostCheck;
	var e = (v == "WWW.ASUS.COM") ? 0 : 1;

	showhide("ddnsname_input", e);
	showhide("asusddnsname_input", !e);
	showhide_div("ddns_link", e);
	showhide_div("row_ddns_hname2", e);
	showhide_div("row_ddns_hname3", e);
	showhide_div("row_ddns_user", e);
	showhide_div("row_ddns_pass", e);
	showhide_div("row_ddns_ssl", (e && support_ddns_ssl()));
	o.disabled = e;

	e = (v == "WWW.EASYDNS.COM") ? 1 : 0;
	showhide_div("row_ddns_wcard", e);
	if (man)
		disable_update();
}

function change_ddns2_server(man)
{
	var e = (document.form.ddns2_server.value == "") ? 0 : 1;
	showhide_div("ddns2_link", e);
	showhide_div("row_ddns2_hname", e);
	showhide_div("row_ddns2_user", e);
	showhide_div("row_ddns2_pass", e);
	showhide_div("row_ddns2_ssl", (e && support_ddns_ssl()));
	if (man)
		disable_update();
}

function change_ddns_source(man)
{
	var e = (document.form.ddns_source.value == "0") ? 1 : 0;
	showhide_div("row_ddns_checkip", e);
	if (man)
		disable_update();
}

function enable_ddns(e1,man)
{
	if (e1 == 1){
		test_wan_ip();
		change_ddns_server(man);
		showhide_div("row_ddns_server", 1);
		showhide_div("row_ddns_hname1", 1);
		change_ddns2_server(man);
		change_ddns_source(man);
		showhide_div("tbl_ddns2", 1);
		showhide_div("tbl_common", 1);
	}else{
		showhide("wan_ip_hide2", 0);
		showhide("wan_ip_hide3", 0);
		showhide_div("row_ddns_server", 0);
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
}

function ddns_load_body()
{
	var ddns_server_x = '<% nvram_get_x("","ddns_server_x"); %>';
	var ddns_hostname_x = '<% nvram_get_x("","ddns_hostname_x"); %>';

	if (ddns_server_x == ""){
		ddns_server_x = "WWW.ASUS.COM";
		document.form.ddns_server_x.selectedIndex = 0;
	}

	var d1s = document.form.ddns_server_x;
	if(d1s.selectedIndex == 0 && ddns_hostname_x != '')
		$("DDNSName").value = ddns_hostname_x.substring(0, ddns_hostname_x.indexOf('.'));
	else if(d1s.selectedIndex == 0 && ddns_hostname_x == '')
		$("DDNSName").value = "<#asusddns_inputhint#>";
	else if(d1s.selectedIndex != 0 && ddns_hostname_x == '')
		$("ddns_hostname_x").value = "<#asusddns_inputhint#>";
	else
		$("ddns_hostname_x").value = ddns_hostname_x;

	if (rcheck(document.form.ddns_enable_x) == "1") {
		enable_ddns(1,0);
		if (ddns_server_x == "WWW.ASUS.COM")
			show_asus_alert(ddns_hostname_x);
	}
	else
		enable_ddns(0,0);
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
}

function validForm(){
	if(rcheck(document.form.ddns_enable_x) == "1"){
		if(document.form.ddns_server_x.selectedIndex == 0){
			if(document.form.DDNSName.value == "" || !validate_ddns_hostname(document.form.DDNSName)){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				document.form.DDNSName.focus();
				document.form.DDNSName.select();
				return false;
			}else{
				document.form.ddns_hostname_x.value = document.form.DDNSName.value+".asuscomm.com";
			}
		}else{
			if(document.form.ddns_hostname_x.value == ""){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				document.form.ddns_hostname_x.focus();
				document.form.ddns_hostname_x.select();
				return false;
			}
		}
		if(document.form.ddns2_server.value != ""){
			if(document.form.ddns2_hname.value == ""){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				document.form.ddns2_hname.focus();
				document.form.ddns2_hname.select();
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
		document.form.current_page.value = "/Advanced_ASUSDDNS_Content.asp";
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
    <input type="hidden" name="current_page" value="Advanced_ASUSDDNS_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
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
                                                    <input type="radio" value="1" name="ddns_enable_x" id="ddns_enable_x_1" onClick="enable_ddns(1,1)" <% nvram_match_x("","ddns_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ddns_enable_x" id="ddns_enable_x_0" onClick="enable_ddns(0,1)" <% nvram_match_x("","ddns_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_server">
                                            <th><#LANHostConfig_x_DDNSServer_itemname#></th>
                                            <td>
                                                <select name="ddns_server_x" class="input" onchange="change_ddns_server(1)">
                                                    <option value="WWW.ASUS.COM"         <% nvram_match_x("","ddns_server_x", "WWW.ASUS.COM","selected"); %>>www.asuscomm.com</option>
                                                    <option value="WWW.DYNDNS.ORG"       <% nvram_match_x("","ddns_server_x", "WWW.DYNDNS.ORG","selected"); %>>www.dyndns.org</option>
                                                    <option value="WWW.TZO.COM"          <% nvram_match_x("","ddns_server_x", "WWW.TZO.COM","selected"); %>>www.tzo.com</option>
                                                    <option value="WWW.ZONEEDIT.COM"     <% nvram_match_x("","ddns_server_x", "WWW.ZONEEDIT.COM","selected"); %>>www.zoneedit.com</option>
                                                    <option value="WWW.EASYDNS.COM"      <% nvram_match_x("","ddns_server_x", "WWW.EASYDNS.COM","selected"); %>>www.easydns.com</option>
                                                    <option value="WWW.NO-IP.COM"        <% nvram_match_x("","ddns_server_x", "WWW.NO-IP.COM","selected"); %>>www.no-ip.com</option>
                                                    <option value="WWW.DNSOMATIC.COM"    <% nvram_match_x("","ddns_server_x", "WWW.DNSOMATIC.COM","selected"); %>>www.dnsomatic.com</option>
                                                    <option value="WWW.DNSEXIT.COM"      <% nvram_match_x("","ddns_server_x", "WWW.DNSEXIT.COM","selected"); %>>www.dnsexit.com</option>
                                                    <option value="WWW.CHANGEIP.COM"     <% nvram_match_x("","ddns_server_x", "WWW.CHANGEIP.COM","selected"); %>>www.changeip.com</option>
                                                    <option value="WWW.SITELUTIONS.COM"  <% nvram_match_x("","ddns_server_x", "WWW.SITELUTIONS.COM","selected"); %>>www.sitelutions.com</option>
                                                    <option value="WWW.ZERIGO.COM"       <% nvram_match_x("","ddns_server_x", "WWW.ZERIGO.COM","selected"); %>>www.zerigo.com</option>
                                                    <option value="WWW.DHIS.ORG"         <% nvram_match_x("","ddns_server_x", "WWW.DHIS.ORG","selected"); %>>www.dhis.org</option>
                                                    <option value="WWW.DUCKDNS.ORG"      <% nvram_match_x("","ddns_server_x", "WWW.DUCKDNS.ORG","selected"); %>>www.duckdns.org</option>
                                                    <option value="WWW.TUNNELBROKER.NET" <% nvram_match_x("","ddns_server_x", "WWW.TUNNELBROKER.NET","selected"); %>>www.tunnelbroker.net (HE)</option>
                                                    <option value="DNS.HE.NET"           <% nvram_match_x("","ddns_server_x", "DNS.HE.NET","selected"); %>>dns.he.net (HE)</option>
                                                    <option value="TB.NETASSIST.UA"      <% nvram_match_x("","ddns_server_x", "TB.NETASSIST.UA","selected"); %>>tb.netassist.ua</option>
                                                    <option value="FREEDNS.AFRAID.ORG"   <% nvram_match_x("","ddns_server_x", "FREEDNS.AFRAID.ORG","selected"); %>>freedns.afraid.org</option>
                                                </select>&nbsp;
                                                <a id="ddns_link" href="javascript:openLink('x_DDNSServer')" class="label label-info" name="x_DDNS_link"><#LANHostConfig_x_DDNSServer_linkname#></a>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_hname1">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <div id="ddnsname_input" class="aidiskdesc" style="display:none;">
                                                    <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" id="ddns_hostname_x" name="ddns_hostname_x" value="<% nvram_get_x("","ddns_hostname_x"); %>" onKeyPress="return is_string(this)">
                                                </div>
                                                <div id="asusddnsname_input" class="aidiskdesc" style="display:none;">
                                                    <input type="text" name="DDNSName" id="DDNSName" style="width: 110px;">.asuscomm.com&nbsp;&nbsp;
                                                    <input type="submit" maxlength="15" size="15" class="btn btn-info" style="max-width: 98px;" onClick="return onSubmitApply('hostname_check');" name="x_DDNSHostCheck" value="<#LANHostConfig_x_DDNSHostnameCheck_buttonname#>"/>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_hname2">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns_hostname2_x" value="<% nvram_get_x("","ddns_hostname2_x"); %>" onKeyPress="return is_string(this)">
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_hname3">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns_hostname3_x" value="<% nvram_get_x("","ddns_hostname3_x"); %>" onKeyPress="return is_string(this)">
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_user">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,1);"><#LANHostConfig_x_DDNSUserName_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns_username_x" value="<% nvram_get_x("","ddns_username_x"); %>" onKeyPress="return is_string(this)">
                                            </td>
                                        </tr>
                                        <tr id="row_ddns_pass">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,2);"><#LANHostConfig_x_DDNSPassword_itemname#></a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" maxlength="64" class="input" size="32" name="ddns_passwd_x" id="ddns_passwd_x" style="width: 175px;" value="<% nvram_get_x("","ddns_passwd_x"); %>">
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
                                        <tr id="row_ddns_wcard" style="display:none;">
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
                                                    <option value=""                     <% nvram_match_x("","ddns2_server", "","selected"); %>><#btn_Disable#></option>
                                                    <option value="WWW.DYNDNS.ORG"       <% nvram_match_x("","ddns2_server", "WWW.DYNDNS.ORG","selected"); %>>www.dyndns.org</option>
                                                    <option value="WWW.TZO.COM"          <% nvram_match_x("","ddns2_server", "WWW.TZO.COM","selected"); %>>www.tzo.com</option>
                                                    <option value="WWW.ZONEEDIT.COM"     <% nvram_match_x("","ddns2_server", "WWW.ZONEEDIT.COM","selected"); %>>www.zoneedit.com</option>
                                                    <option value="WWW.NO-IP.COM"        <% nvram_match_x("","ddns2_server", "WWW.NO-IP.COM","selected"); %>>www.no-ip.com</option>
                                                    <option value="WWW.DNSOMATIC.COM"    <% nvram_match_x("","ddns2_server", "WWW.DNSOMATIC.COM","selected"); %>>www.dnsomatic.com</option>
                                                    <option value="WWW.DNSEXIT.COM"      <% nvram_match_x("","ddns2_server", "WWW.DNSEXIT.COM","selected"); %>>www.dnsexit.com</option>
                                                    <option value="WWW.CHANGEIP.COM"     <% nvram_match_x("","ddns2_server", "WWW.CHANGEIP.COM","selected"); %>>www.changeip.com</option>
                                                    <option value="WWW.SITELUTIONS.COM"  <% nvram_match_x("","ddns2_server", "WWW.SITELUTIONS.COM","selected"); %>>www.sitelutions.com</option>
                                                    <option value="WWW.ZERIGO.COM"       <% nvram_match_x("","ddns2_server", "WWW.ZERIGO.COM","selected"); %>>www.zerigo.com</option>
                                                    <option value="WWW.DHIS.ORG"         <% nvram_match_x("","ddns2_server", "WWW.DHIS.ORG","selected"); %>>www.dhis.org</option>
                                                    <option value="WWW.DUCKDNS.ORG"      <% nvram_match_x("","ddns2_server", "WWW.DUCKDNS.ORG","selected"); %>>www.duckdns.org</option>
                                                    <option value="WWW.TUNNELBROKER.NET" <% nvram_match_x("","ddns2_server", "WWW.TUNNELBROKER.NET","selected"); %>>www.tunnelbroker.net (HE)</option>
                                                    <option value="DNS.HE.NET"           <% nvram_match_x("","ddns2_server", "DNS.HE.NET","selected"); %>>dns.he.net (HE)</option>
                                                    <option value="TB.NETASSIST.UA"      <% nvram_match_x("","ddns2_server", "TB.NETASSIST.UA","selected"); %>>tb.netassist.ua</option>
                                                    <option value="FREEDNS.AFRAID.ORG"   <% nvram_match_x("","ddns2_server", "FREEDNS.AFRAID.ORG","selected"); %>>freedns.afraid.org</option>
                                                </select>&nbsp;
                                                <a id="ddns2_link" href="javascript:openLink('x_DDNSServer2')" class="label label-info" name="x_DDNS2_link"><#LANHostConfig_x_DDNSServer_linkname#></a>
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_hname">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,3);"><#LANHostConfig_x_DDNSHostNames_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns2_hname" value="<% nvram_get_x("","ddns2_hname"); %>" onKeyPress="return is_string(this)">
                                            </td>
                                        </tr>
                                        <tr id="row_ddns2_user">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,24,1);"><#LANHostConfig_x_DDNSUserName_itemname#></a></th>
                                            <td>
                                                <input type="text" class="input" maxlength="64" size="48" style="width: 286px;" name="ddns2_user" value="<% nvram_get_x("","ddns2_user"); %>" onKeyPress="return is_string(this)">
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
                                                    <option value="3" <% nvram_match_x("", "ddns_checkip", "3","selected"); %>>echo.tzo.com</option>
                                                    <option value="4" <% nvram_match_x("", "ddns_checkip", "4","selected"); %>>ip.dnsexit.com</option>
                                                    <option value="5" <% nvram_match_x("", "ddns_checkip", "5","selected"); %>>ip.changeip.com</option>
                                                    <option value="6" <% nvram_match_x("", "ddns_checkip", "6","selected"); %>>myip.dnsomatic.com</option>
                                                    <option value="7" <% nvram_match_x("", "ddns_checkip", "7","selected"); %>>ip1.dynupdate.no-ip.com</option>
                                                    <option value="8" <% nvram_match_x("", "ddns_checkip", "8","selected"); %>>checkip.dns.he.net</option>
                                                    <option value="9" <% nvram_match_x("", "ddns_checkip", "9","selected"); %>>checkip.zerigo.com</option>
                                                    <option value="10" <% nvram_match_x("", "ddns_checkip", "10","selected"); %>>checkip.two-dns.de</option>
                                                    <option value="11" <% nvram_match_x("", "ddns_checkip", "11","selected"); %>>ipv4.wtfismyip.com/text</option>
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
