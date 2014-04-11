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
                change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '1');
                $j("#ddns_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#ddns_enable_x_1").attr("checked", "checked");
                $j("#ddns_enable_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '0');
                $j("#ddns_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#ddns_enable_x_0").attr("checked", "checked");
                $j("#ddns_enable_x_1").removeAttr("checked");
            }
        });
        $j("#ddns_enable_x_on_of label.itoggle").css("background-position", $j("input#ddns_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });
</script>

<script>

<% wanlink(); %>
<% login_state_hook(); %>

function init(){
	show_banner(1);
	show_footer();
	
	if(sw_mode == "4")
		show_menu(5,4,2);
	else
		show_menu(5,4,4);
	
	hideLoading();
	
	ddns_load_body();
}

function valid_wan_ip() {
	// test if WAN IP is a private IP.
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

function ddns_show_domains(is_visible) {
	if (!is_visible) {
		showhide("ddnsname_input", 0);
		showhide("asusddnsname_input", 1);
		$("ddnsname2_row").style.display = "none";
		$("ddnsname3_row").style.display = "none";
	}else{
		showhide("ddnsname_input", 1);
		showhide("asusddnsname_input", 0);
		$("ddnsname2_row").style.display = "";
		$("ddnsname3_row").style.display = "";
	}
}

function ddns_load_body(){

	valid_wan_ip();

	var ddns_hostname_x = '<% nvram_get_x("LANHostConfig","ddns_hostname_x"); %>';
	var ddns_return_code = '<% nvram_get_ddns("","ddns_return_code"); %>';
	var ddns_server_x = '<% nvram_get_x("","ddns_server_x"); %>';
	var wan_ipaddr_t='<% nvram_get_x("","wan_ipaddr_t"); %>';

	if (ddns_server_x == "")
		ddns_server_x = "WWW.ASUS.COM";

	if(document.form.ddns_server_x.selectedIndex == 0 && ddns_hostname_x != ''){
		ddns_hostname_title = ddns_hostname_x.substring(0, ddns_hostname_x.indexOf('.'));
		ddns_show_domains(0);
		$("DDNSName").value = ddns_hostname_title;
	}
	else if(document.form.ddns_server_x.selectedIndex == 0 && ddns_hostname_x == ''){
		ddns_show_domains(0);
		$("DDNSName").value = "<#asusddns_inputhint#>";
	}else if(document.form.ddns_server_x.selectedIndex != 0 && ddns_hostname_x == ''){
		ddns_show_domains(1);
		$("ddns_hostname_x").value = "<#asusddns_inputhint#>";
	}else{
		ddns_show_domains(1);
		$("ddns_hostname_x").value = ddns_hostname_x;
	}
	
	if(ddns_server_x == "WWW.ASUS.COM"){
		inputCtrl(document.form.ddns_username_x, 0);
		inputCtrl(document.form.ddns_passwd_x, 0);
	}else{
		inputCtrl(document.form.ddns_username_x, 1);
		inputCtrl(document.form.ddns_passwd_x, 1);
	}
	
	if(ddns_server_x == "WWW.ASUS.COM"){
		document.form.ddns_wildcard_x[0].disabled= 1;
		document.form.ddns_wildcard_x[1].disabled= 1;
		document.form.LANHostConfig_x_DDNSHostnameCheck_button.disabled = 0;
		showhide("link", 0);
	}
	else{
		document.form.ddns_wildcard_x[0].disabled = 0;
		document.form.ddns_wildcard_x[1].disabled = 0;
		document.form.LANHostConfig_x_DDNSHostnameCheck_button.disabled = 1;
	}
	
	if(ddns_server_x == "WWW.ASUS.COM") {
		if(ddns_return_code == 'register,-1')
			alert("<#LANHostConfig_x_DDNS_alarm_2#>");
		else if(ddns_return_code == 'register,200') {
			showhide("wan_ip_hide3", 1);
			alert("<#LANHostConfig_x_DDNS_alarm_3#>");
		}
		else if(ddns_return_code == 'register,203')
			alert("<#LANHostConfig_x_DDNS_alarm_hostname#> '"+ddns_hostname_x+"' <#LANHostConfig_x_DDNS_alarm_registered#>");
		else if(ddns_return_code == 'register,220') {
			showhide("wan_ip_hide3", 1);
			alert("<#LANHostConfig_x_DDNS_alarm_4#>");
		}
		else if(ddns_return_code == 'register,230') {
			showhide("wan_ip_hide3", 1);
			alert("<#LANHostConfig_x_DDNS_alarm_5#>");
		}
		else if(ddns_return_code == 'register,233')
			alert("<#LANHostConfig_x_DDNS_alarm_hostname#> '"+ddns_hostname_x+"' <#LANHostConfig_x_DDNS_alarm_registered_2#>");
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

function validForm(){
	if(document.form.ddns_enable_x[0].checked){
		if(document.form.ddns_server_x.selectedIndex == 0){
			if(document.form.DDNSName.value == "" || !validate_ddns_hostname(document.form.DDNSName)){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				document.form.DDNSName.focus();
				document.form.DDNSName.select();
				return false;
			}else{
				document.form.ddns_hostname_x.value = document.form.DDNSName.value+".asuscomm.com";
				return true;
			}
		}else{
			if(document.form.ddns_hostname_x.value == ""){
				alert("<#LANHostConfig_x_DDNS_alarm_14#>");
				document.form.ddns_hostname_x.focus();
				document.form.ddns_hostname_x.select();
				return false;
			}else
				return true;
		}
	}
	else
		return true;
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
                                                    <input type="radio" value="1" name="ddns_enable_x" id="ddns_enable_x_1" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '1')" <% nvram_match_x("LANHostConfig","ddns_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ddns_enable_x" id="ddns_enable_x_0" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '0')" <% nvram_match_x("LANHostConfig","ddns_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_DDNSServer_itemname#></th>
                                            <td>
                                                <select name="ddns_server_x"class="input"onchange="return change_common(this, 'LANHostConfig', 'ddns_server_x')">
                                                    <option value="WWW.ASUS.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.ASUS.COM","selected"); %>>WWW.ASUS.COM</option>
                                                    <option value="WWW.DYNDNS.ORG" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.DYNDNS.ORG","selected"); %>>WWW.DYNDNS.ORG</option>
                                                    <option value="WWW.TZO.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.TZO.COM","selected"); %>>WWW.TZO.COM</option>
                                                    <option value="WWW.ZONEEDIT.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.ZONEEDIT.COM","selected"); %>>WWW.ZONEEDIT.COM</option>
                                                    <option value="WWW.EASYDNS.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.EASYDNS.COM","selected"); %>>WWW.EASYDNS.COM</option>
                                                    <option value="WWW.NO-IP.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.NO-IP.COM","selected"); %>>WWW.NO-IP.COM</option>
                                                    <option value="WWW.DNSOMATIC.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.DNSOMATIC.COM","selected"); %>>WWW.DNSOMATIC.COM</option>
                                                    <option value="WWW.DNSEXIT.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.DNSEXIT.COM","selected"); %>>WWW.DNSEXIT.COM</option>
                                                    <option value="WWW.CHANGEIP.COM" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.CHANGEIP.COM","selected"); %>>WWW.CHANGEIP.COM</option>
                                                    <option value="WWW.TUNNELBROKER.NET" <% nvram_match_x("LANHostConfig","ddns_server_x", "WWW.TUNNELBROKER.NET","selected"); %>>WWW.TUNNELBROKER.NET</option>
                                                    <option value="DNS.HE.NET" <% nvram_match_x("LANHostConfig","ddns_server_x", "DNS.HE.NET","selected"); %>>DNS.HE.NET</option>
                                                    <option value="FREEDNS.AFRAID.ORG" <% nvram_match_x("LANHostConfig","ddns_server_x", "FREEDNS.AFRAID.ORG","selected"); %>>FREEDNS.AFRAID.ORG</option>
                                                </select>
                                                <div id="link">
                                                    <a href="javascript:openLink('x_DDNSServer')" class="content_input_link" name="x_DDNSServer_link">
                                                        <#LANHostConfig_x_DDNSServer_linkname#>
                                                    </a>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_DDNSHostNames_itemname#></th>
                                            <td>
                                                <div id="ddnsname_input" class="aidiskdesc" style="display:none;">
                                                    <input type="text" maxlength="64" class="input" size="48" id="ddns_hostname_x" name="ddns_hostname_x" value="<% nvram_get_x("LANHostConfig","ddns_hostname_x"); %>" onKeyPress="return is_string(this)">
                                                </div>
                                                <div id="asusddnsname_input" class="aidiskdesc" style="display:none;">
                                                    <input type="text" name="DDNSName" id="DDNSName" style="width: 110px;">.asuscomm.com&nbsp;
                                                    <input type="submit" maxlength="15" class="btn btn-info" onClick="return onSubmitApply('hostname_check');" size="12" name="LANHostConfig_x_DDNSHostnameCheck_button" value="<#LANHostConfig_x_DDNSHostnameCheck_buttonname#>"><br />
                                                    <div id="alert_block" class="alert alert-danger" style="display:none;"><span id="alert_str"></span></div>
                                                </div>
                                                <!--div id="Hostname_Note"><span><#LANHostConfig_x_DDNSHostNames_Note#></span></div-->
                                            </td>
                                        </tr>
                                        <tr id="ddnsname2_row">
                                            <th><#LANHostConfig_x_DDNSHostNames_itemname#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="48" name="ddns_hostname2_x" value="<% nvram_get_x("LANHostConfig","ddns_hostname2_x"); %>" onKeyPress="return is_string(this)">
                                            </td>
                                        </tr>
                                        <tr id="ddnsname3_row">
                                            <th><#LANHostConfig_x_DDNSHostNames_itemname#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="48" name="ddns_hostname3_x" value="<% nvram_get_x("LANHostConfig","ddns_hostname3_x"); %>" onKeyPress="return is_string(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_DDNSUserName_itemname#></th>
                                            <td><input type="text" maxlength="32" class="input" size="32" name="ddns_username_x" value="<% nvram_get_x("LANHostConfig","ddns_username_x"); %>" onKeyPress="return is_string(this)"></td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_DDNSPassword_itemname#></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" maxlength="64" class="input" size="32" name="ddns_passwd_x" id="ddns_passwd_x" style="width: 175px;" value="<% nvram_get_x("LANHostConfig","ddns_passwd_x"); %>">
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('ddns_passwd_x')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_DDNSWildcard_itemname#></th>
                                            <td>
                                                <div class="controls">
                                                    <label class="radio inline"><input type="radio" value="1" name="ddns_wildcard_x" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '1')" <% nvram_match_x("LANHostConfig","ddns_wildcard_x", "1", "checked"); %>><#checkbox_Yes#></label>
                                                    <label class="radio inline"><input type="radio" value="0" name="ddns_wildcard_x" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '0')" <% nvram_match_x("LANHostConfig","ddns_wildcard_x", "0", "checked"); %>><#checkbox_No#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#DDNS_Source#></th>
                                            <td>
                                                <select name="ddns_source" class="input">
                                                    <option value="0" <% nvram_match_x("", "ddns_source", "0","selected"); %>><#DDNS_AddrExt#></option>
                                                    <option value="1" <% nvram_match_x("", "ddns_source", "1","selected"); %>><#DDNS_AddrWAN#></option>
                                                    <option value="2" <% nvram_match_x("", "ddns_source", "2","selected"); %>><#DDNS_AddrMAN#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#DDNS_Period#></th>
                                            <td>
                                                <select name="ddns_period" class="input">
                                                    <option value="0" <% nvram_match_x("", "ddns_period", "0","selected"); %>>10 mins</option>
                                                    <option value="1" <% nvram_match_x("", "ddns_period", "1","selected"); %>>1 hour</option>
                                                    <option value="2" <% nvram_match_x("", "ddns_period", "2","selected"); %>>2 hours</option>
                                                    <option value="3" <% nvram_match_x("", "ddns_period", "3","selected"); %>>3 hours</option>
                                                    <option value="6" <% nvram_match_x("", "ddns_period", "6","selected"); %>>6 hours</option>
                                                    <option value="12" <% nvram_match_x("", "ddns_period", "12","selected"); %>>12 hours</option>
                                                    <option value="24" <% nvram_match_x("", "ddns_period", "24","selected"); %>>1 day</option>
                                                    <option value="48" <% nvram_match_x("", "ddns_period", "48","selected"); %>>2 days</option>
                                                    <option value="72" <% nvram_match_x("", "ddns_period", "72","selected"); %>>3 days</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#DDNS_Verbose#></th>
                                            <td>
                                                <select name="ddns_verbose" class="input">
                                                    <option value="0" <% nvram_match_x("", "ddns_verbose", "0","selected"); %>>0 (Quiet)</option>
                                                    <option value="1" <% nvram_match_x("", "ddns_verbose", "1","selected"); %>>1 (Default)</option>
                                                    <option value="2" <% nvram_match_x("", "ddns_verbose", "2","selected"); %>>2 (Verbose)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_DDNSStatus_itemname#></th>
                                            <td>
                                                <input type="hidden" maxlength="15" class="button" size="12" name="" value="<% nvram_get_x("", "DDNSStatus"); %>">
                                                <input type="submit" maxlength="15" class="btn btn-info span12" style="max-width: 220px;" onclick="showLoading();return onSubmitApply('ddnsclient');" size="12" name="LANHostConfig_x_DDNSStatus_button" value="<#LANHostConfig_x_DDNSStatus_buttonname#>" />
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2">
                                                <br />
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
