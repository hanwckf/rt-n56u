<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_1#></title>
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
        $j('#fw_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#fw_enable_x_1").attr("checked", "checked");
                $j("#fw_enable_x_0").removeAttr("checked");
                change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '1');
            },
            onClickOff: function(){
                $j("#fw_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#fw_enable_x_0").attr("checked", "checked");
                $j("#fw_enable_x_1").removeAttr("checked");
                change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '0');
            }
        });
        $j("#fw_enable_x_on_of label.itoggle").css("background-position", $j("input#fw_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#fw_dos_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_dos_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#fw_dos_x_1").attr("checked", "checked");
                $j("#fw_dos_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#fw_dos_x_fake").removeAttr("checked").attr("value", 0);
                $j("#fw_dos_x_0").attr("checked", "checked");
                $j("#fw_dos_x_1").removeAttr("checked");
            }
        });
        $j("#fw_dos_x_on_of label.itoggle").css("background-position", $j("input#fw_dos_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#fw_syn_cook_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#fw_syn_cook_fake").attr("checked", "checked").attr("value", 1);
                $j("#fw_syn_cook_1").attr("checked", "checked");
                $j("#fw_syn_cook_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#fw_syn_cook_fake").removeAttr("checked").attr("value", 0);
                $j("#fw_syn_cook_0").attr("checked", "checked");
                $j("#fw_syn_cook_1").removeAttr("checked");
            }
        });
        $j("#fw_syn_cook_on_of label.itoggle").css("background-position", $j("input#fw_syn_cook_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#misc_ping_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#misc_ping_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#misc_ping_x_1").attr("checked", "checked");
                $j("#misc_ping_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#misc_ping_x_fake").removeAttr("checked").attr("value", 0);
                $j("#misc_ping_x_0").attr("checked", "checked");
                $j("#misc_ping_x_1").removeAttr("checked");
            }
        });
        $j("#misc_ping_x_on_of label.itoggle").css("background-position", $j("input#misc_ping_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#misc_http_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#misc_http_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#misc_http_x_1").attr("checked", "checked");
                $j("#misc_http_x_0").removeAttr("checked");
                http_wopen_changed();
            },
            onClickOff: function(){
                $j("#misc_http_x_fake").removeAttr("checked").attr("value", 0);
                $j("#misc_http_x_0").attr("checked", "checked");
                $j("#misc_http_x_1").removeAttr("checked");
                http_wopen_changed();
            }
        });
        $j("#misc_http_x_on_of label.itoggle").css("background-position", $j("input#misc_http_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#https_wopen_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#https_wopen_fake").attr("checked", "checked").attr("value", 1);
                $j("#https_wopen_1").attr("checked", "checked");
                $j("#https_wopen_0").removeAttr("checked");
                https_wopen_changed();
            },
            onClickOff: function(){
                $j("#https_wopen_fake").removeAttr("checked").attr("value", 0);
                $j("#https_wopen_0").attr("checked", "checked");
                $j("#https_wopen_1").removeAttr("checked");
                https_wopen_changed();
            }
        });
        $j("#https_wopen_on_of label.itoggle").css("background-position", $j("input#https_wopen_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#sshd_wopen_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#sshd_wopen_fake").attr("checked", "checked").attr("value", 1);
                $j("#sshd_wopen_1").attr("checked", "checked");
                $j("#sshd_wopen_0").removeAttr("checked");
                sshd_wopen_changed();
            },
            onClickOff: function(){
                $j("#sshd_wopen_fake").removeAttr("checked").attr("value", 0);
                $j("#sshd_wopen_0").attr("checked", "checked");
                $j("#sshd_wopen_1").removeAttr("checked");
                sshd_wopen_changed();
            }
        });
        $j("#sshd_wopen_on_of label.itoggle").css("background-position", $j("input#sshd_wopen_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#ftpd_wopen_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ftpd_wopen_fake").attr("checked", "checked").attr("value", 1);
                $j("#ftpd_wopen_1").attr("checked", "checked");
                $j("#ftpd_wopen_0").removeAttr("checked");
                ftpd_wopen_changed();
            },
            onClickOff: function(){
                $j("#ftpd_wopen_fake").removeAttr("checked").attr("value", 0);
                $j("#ftpd_wopen_0").attr("checked", "checked");
                $j("#ftpd_wopen_1").removeAttr("checked");
                ftpd_wopen_changed();
            }
        });
        $j("#ftpd_wopen_on_of label.itoggle").css("background-position", $j("input#ftpd_wopen_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#udpxy_wopen_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#udpxy_wopen_fake").attr("checked", "checked").attr("value", 1);
                $j("#udpxy_wopen_1").attr("checked", "checked");
                $j("#udpxy_wopen_0").removeAttr("checked");
                udpxy_wopen_changed();
            },
            onClickOff: function(){
                $j("#udpxy_wopen_fake").removeAttr("checked").attr("value", 0);
                $j("#udpxy_wopen_0").attr("checked", "checked");
                $j("#udpxy_wopen_1").removeAttr("checked");
                udpxy_wopen_changed();
            }
        });
        $j("#udpxy_wopen_on_of label.itoggle").css("background-position", $j("input#udpxy_wopen_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#trmd_ropen_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#trmd_ropen_fake").attr("checked", "checked").attr("value", 1);
                $j("#trmd_ropen_1").attr("checked", "checked");
                $j("#trmd_ropen_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#trmd_ropen_fake").removeAttr("checked").attr("value", 0);
                $j("#trmd_ropen_0").attr("checked", "checked");
                $j("#trmd_ropen_1").removeAttr("checked");
            }
        });
        $j("#trmd_ropen_on_of label.itoggle").css("background-position", $j("input#trmd_ropen_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#aria_ropen_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#aria_ropen_fake").attr("checked", "checked").attr("value", 1);
                $j("#aria_ropen_1").attr("checked", "checked");
                $j("#aria_ropen_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#aria_ropen_fake").removeAttr("checked").attr("value", 0);
                $j("#aria_ropen_0").attr("checked", "checked");
                $j("#aria_ropen_1").removeAttr("checked");
            }
        });
        $j("#aria_ropen_on_of label.itoggle").css("background-position", $j("input#aria_ropen_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });
</script>

<script>

var http_proto = '<% nvram_get_x("", "http_proto"); %>';

function initial(){
	show_banner(1);
	show_menu(5,5,1);
	show_footer();

	load_body();

	if(found_app_sshd()){
		$("row_sshd").style.display = "";
		sshd_wopen_changed();
	}

	if(found_app_ftpd()){
		$("row_ftpd_wopen").style.display = "";
		if (sw_mode != "4")
			ftpd_wopen_changed();
	}

	if (sw_mode != "4")
		udpxy_wopen_changed();

	if(found_app_torr())
		$("row_torrent").style.display = "";

	if(found_app_aria())
		$("row_aria").style.display = "";

	if (support_http_ssl()){
		if (http_proto == "0" || http_proto == "2"){
			if (sw_mode != "4")
				http_wopen_changed();
		}else{
			$("row_http_wopen").style.display = "none";
		}
		if (http_proto == "1" || http_proto == "2"){
			$("row_https_wopen").style.display = "";
			if (sw_mode != "4")
				https_wopen_changed();
		}
	}else{
		if (sw_mode != "4")
			http_wopen_changed();
	}
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_BasicFirewall_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if (sw_mode == '4')
		return true;

	if (support_http_ssl()){
		if (http_proto == "0" || http_proto == "2"){
			if(!validate_range(document.form.misc_httpport_x, 80, 65535))
				return false;
		}
		if (http_proto == "1" || http_proto == "2"){
			if(!validate_range(document.form.https_wport, 81, 65535))
				return false;
		}
		if (http_proto == "2"){
			if (document.form.misc_httpport_x.value == document.form.https_wport.value){
				alert("HTTP and HTTPS ports is equal!");
				document.form.https_wport.focus();
				document.form.https_wport.select();
				return false;
			}
		}
	}else{
		if(!validate_range(document.form.misc_httpport_x, 80, 65535))
			return false;
	}

	if(!validate_range(document.form.udpxy_wport, 1024, 65535))
		return false;

	if(found_app_sshd()){
		if(!validate_range(document.form.sshd_wport, 22, 65535))
			return false;
	}

	if(found_app_ftpd()){
		if(!validate_range(document.form.ftpd_wport, 21, 65535))
			return false;
	}

	return true;
}

function http_wopen_changed(){
	if (sw_mode == '4')
		return;
	var a = rcheck(document.form.misc_http_x);
	if (a == "0")
		$("row_http_wport").style.display = "none";
	else
		$("row_http_wport").style.display = "";
}

function https_wopen_changed(){
	if (sw_mode == '4')
		return;
	var a = rcheck(document.form.https_wopen);
	if (a == "0")
		$("row_https_wport").style.display = "none";
	else
		$("row_https_wport").style.display = "";
}

function sshd_wopen_changed(){
	var a = rcheck(document.form.sshd_wopen);
	if (a == "0") {
		$("row_sshd_wport").style.display = "none";
		$("row_sshd_wbfp").style.display = "none";
	}else{
		if (sw_mode != "4")
			$("row_sshd_wport").style.display = "";
		$("row_sshd_wbfp").style.display = "";
	}
}

function ftpd_wopen_changed(){
	if (sw_mode == '4')
		return;
	var a = rcheck(document.form.ftpd_wopen);
	if (a == "0")
		$("row_ftpd_wport").style.display = "none";
	else
		$("row_ftpd_wport").style.display = "";
}

function udpxy_wopen_changed(){
	if (sw_mode == '4')
		return;
	var a = rcheck(document.form.udpxy_wopen);
	if (a == "0")
		$("row_udpxy_wport").style.display = "none";
	else
		$("row_udpxy_wport").style.display = "";
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

    <input type="hidden" name="current_page" value="Advanced_BasicFirewall_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="FirewallConfig;">
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
                            <h2 class="box_head round_top"><#menu5_5#> - <#menu5_5_1#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#FirewallConfig_display2_sectiondesc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#menu5_5#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,8,6);"><#FirewallConfig_FirewallEnable_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_enable_x_on_of">
                                                        <input type="checkbox" id="fw_enable_x_fake" <% nvram_match_x("", "fw_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "fw_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="fw_enable_x" id="fw_enable_x_1" onClick="return change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '1')" <% nvram_match_x("","fw_enable_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="fw_enable_x" id="fw_enable_x_0" onClick="return change_common_radio(this, 'FirewallConfig', 'fw_enable_x', '0')" <% nvram_match_x("","fw_enable_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,8,7);"><#FirewallConfig_DoSEnable_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_dos_x_on_of">
                                                        <input type="checkbox" id="fw_dos_x_fake" <% nvram_match_x("", "fw_dos_x", "1", "value=1 checked"); %><% nvram_match_x("", "fw_dos_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="fw_dos_x" id="fw_dos_x_1" class="input" <% nvram_match_x("", "fw_dos_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="fw_dos_x" id="fw_dos_x_0" class="input" <% nvram_match_x("", "fw_dos_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,8,8);"><#FirewallConfigSynFlood#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="fw_syn_cook_on_of">
                                                        <input type="checkbox" id="fw_syn_cook_fake" <% nvram_match_x("", "fw_syn_cook", "1", "value=1 checked"); %><% nvram_match_x("", "fw_syn_cook", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="fw_syn_cook" id="fw_syn_cook_1" class="input" <% nvram_match_x("", "fw_syn_cook", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="fw_syn_cook" id="fw_syn_cook_0" class="input" <% nvram_match_x("", "fw_syn_cook", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,8,1);"><#FirewallConfig_WanLanLog_itemname#></a></th>
                                            <td>
                                                <select name="fw_log_x" class="input" onchange="return change_common(this, 'FirewallConfig', 'fw_log_x')">
                                                    <option value="none" <% nvram_match_x("","fw_log_x", "none","selected"); %>><#checkbox_No#></option>
                                                    <option value="drop" <% nvram_match_x("","fw_log_x", "drop","selected"); %>>Dropped</option>
                                                    <option value="accept" <% nvram_match_x("","fw_log_x", "accept","selected"); %>>Accepted</option>
                                                    <option value="both" <% nvram_match_x("","fw_log_x", "both","selected"); %>>Both</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_misc_ping">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,8,5);"><#FirewallConfig_x_WanPingEnable_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="misc_ping_x_on_of">
                                                        <input type="checkbox" id="misc_ping_x_fake" <% nvram_match_x("", "misc_ping_x", "1", "value=1 checked"); %><% nvram_match_x("", "misc_ping_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="misc_ping_x" id="misc_ping_x_1" class="input" <% nvram_match_x("","misc_ping_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="misc_ping_x" id="misc_ping_x_0" class="input" <% nvram_match_x("","misc_ping_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="access_section">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#Adm_Access_WAN#></th>
                                        </tr>
                                        <tr id="row_http_wopen">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,8,2);"><#FirewallConfig_x_WanWebEnable_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="misc_http_x_on_of">
                                                        <input type="checkbox" id="misc_http_x_fake" <% nvram_match_x("", "misc_http_x", "1", "value=1 checked"); %><% nvram_match_x("", "misc_http_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="misc_http_x" id="misc_http_x_1" class="input" onclick="http_wopen_changed();" <% nvram_match_x("","misc_http_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="misc_http_x" id="misc_http_x_0" class="input" onclick="http_wopen_changed();" <% nvram_match_x("","misc_http_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_http_wport" style="display:none;">
                                            <th style="border-top: 0 none;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,8,3);"><#FirewallConfig_x_WanWebPort_itemname#></a></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="5" size="5" name="misc_httpport_x" class="input" value="<% nvram_get_x("", "misc_httpport_x"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[80..65535]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_https_wopen" style="display:none;">
                                            <th width="50%"><#Adm_System_https_wopen#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="https_wopen_on_of">
                                                        <input type="checkbox" id="https_wopen_fake" <% nvram_match_x("", "https_wopen", "1", "value=1 checked"); %><% nvram_match_x("", "https_wopen", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="https_wopen" id="https_wopen_1" class="input" onclick="https_wopen_changed();" <% nvram_match_x("","https_wopen", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="https_wopen" id="https_wopen_0" class="input" onclick="https_wopen_changed();" <% nvram_match_x("","https_wopen", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_https_wport" style="display:none;">
                                            <th style="border-top: 0 none;"><#Adm_System_https_wport#></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="5" size="5" name="https_wport" class="input" value="<% nvram_get_x("", "https_wport"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[81..65535]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_sshd" style="display:none;">
                                            <th><#Adm_System_sshd_wopen#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="sshd_wopen_on_of">
                                                        <input type="checkbox" id="sshd_wopen_fake" <% nvram_match_x("", "sshd_wopen", "1", "value=1 checked"); %><% nvram_match_x("", "sshd_wopen", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="sshd_wopen" id="sshd_wopen_1" class="input" value="1" onclick="sshd_wopen_changed();" <% nvram_match_x("", "sshd_wopen", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="sshd_wopen" id="sshd_wopen_0" class="input" value="0" onclick="sshd_wopen_changed();" <% nvram_match_x("", "sshd_wopen", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_sshd_wport" style="display:none;">
                                            <th style="border-top: 0 none;"><#Adm_System_sshd_wport#></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="5" size="5" name="sshd_wport" class="input" value="<% nvram_get_x("","sshd_wport"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[22..65535]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_sshd_wbfp" style="display:none;">
                                            <th style="border-top: 0 none;"><#Adm_System_sshd_wbfp#></th>
                                            <td style="border-top: 0 none;">
                                                <select name="sshd_wbfp" class="input">
                                                    <option value="0" <% nvram_match_x("","sshd_wbfp", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("","sshd_wbfp", "1","selected"); %>>Max 3 tries / 1 min</option>
                                                    <option value="2" <% nvram_match_x("","sshd_wbfp", "2","selected"); %>>Max 3 tries / 5 min (*)</option>
                                                    <option value="3" <% nvram_match_x("","sshd_wbfp", "3","selected"); %>>Max 3 tries / 10 min</option>
                                                    <option value="4" <% nvram_match_x("","sshd_wbfp", "4","selected"); %>>Max 3 tries / 30 min</option>
                                                    <option value="5" <% nvram_match_x("","sshd_wbfp", "5","selected"); %>>Max 3 tries / 60 min</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ftpd_wopen" style="display:none;">
                                            <th><#Adm_System_ftpd_wopen#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ftpd_wopen_on_of">
                                                        <input type="checkbox" id="ftpd_wopen_fake" <% nvram_match_x("", "ftpd_wopen", "1", "value=1 checked"); %><% nvram_match_x("", "ftpd_wopen", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="ftpd_wopen" id="ftpd_wopen_1" class="input" value="1" onclick="ftpd_wopen_changed();" <% nvram_match_x("", "ftpd_wopen", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="ftpd_wopen" id="ftpd_wopen_0" class="input" value="0" onclick="ftpd_wopen_changed();" <% nvram_match_x("", "ftpd_wopen", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ftpd_wport" style="display:none;">
                                            <th style="border-top: 0 none;"><#Adm_System_ftpd_wport#></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="5" size="5" name="ftpd_wport" class="input" value="<% nvram_get_x("","ftpd_wport"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[21..65535]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#Adm_System_udpxy_wopen#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="udpxy_wopen_on_of">
                                                        <input type="checkbox" id="udpxy_wopen_fake" <% nvram_match_x("", "udpxy_wopen", "1", "value=1 checked"); %><% nvram_match_x("", "udpxy_wopen", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="udpxy_wopen" id="udpxy_wopen_1" class="input" value="1" onclick="udpxy_wopen_changed();" <% nvram_match_x("", "udpxy_wopen", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="udpxy_wopen" id="udpxy_wopen_0" class="input" value="0" onclick="udpxy_wopen_changed();" <% nvram_match_x("", "udpxy_wopen", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_udpxy_wport" style="display:none;">
                                            <th style="border-top: 0 none;"><#Adm_System_udpxy_wport#></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="5" size="5" name="udpxy_wport" class="input" value="<% nvram_get_x("","udpxy_wport"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[1024..65535]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_torrent" style="display:none;">
                                            <th><#Adm_System_trmd_ropen#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="trmd_ropen_on_of">
                                                        <input type="checkbox" id="trmd_ropen_fake" <% nvram_match_x("", "trmd_ropen", "1", "value=1 checked"); %><% nvram_match_x("", "trmd_ropen", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="trmd_ropen" id="trmd_ropen_1" class="input" value="1" <% nvram_match_x("", "trmd_ropen", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="trmd_ropen" id="trmd_ropen_0" class="input" value="0" <% nvram_match_x("", "trmd_ropen", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_aria" style="display:none;">
                                            <th><#Adm_System_aria_ropen#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="aria_ropen_on_of">
                                                        <input type="checkbox" id="aria_ropen_fake" <% nvram_match_x("", "aria_ropen", "1", "value=1 checked"); %><% nvram_match_x("", "aria_ropen", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="aria_ropen" id="aria_ropen_1" class="input" value="1" <% nvram_match_x("", "aria_ropen", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="aria_ropen" id="aria_ropen_0" class="input" value="0" <% nvram_match_x("", "aria_ropen", "0", "checked"); %>/><#checkbox_No#>
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
