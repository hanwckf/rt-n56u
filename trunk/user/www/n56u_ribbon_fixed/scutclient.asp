<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_13#></title>
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
<% scutclient_status(); %>
<% scutclient_version(); %>

$j(document).ready(function(){
	init_itoggle('scutclient_enable');
	init_itoggle('scutclient_debug');
	init_itoggle('scutclient_watchcat');
	init_itoggle('scutclient_wdg_force');
	init_itoggle('scutclient_skip_udp_hb');
	fill_status(scutclient_status());
});

function initial(){
	show_banner(2);
	show_menu(5,11,1);
	show_footer();
	$("scutclient_version").innerHTML = '<#version#>' + scutclient_version() ;
}

function applyRule(){
	if(validForm()){
		showLoading();
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "scutclient.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	var addr_obj = document.form.scutclient_server_auth_ip;

	if(!validate_ipaddr_final(addr_obj, ''))
		return false;

	return true;
}

function submitInternet(v){
	showLoading();
	document.scutclient_action.action = "scutclient_action.asp";
	document.scutclient_action.connect_action.value = v;
	document.scutclient_action.submit();
}

function fill_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("scutclient_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
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
	
    <input type="hidden" name="current_page" value="scutclient.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="ScutclientConf;">
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
                            <h2 class="box_head round_top"><#menu5_13#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_1_1#></th> </tr>

                                        <tr> <th width="50%"><#InetControl#></th>
                                            <td style="border-top: 0 none;" colspan="2">
                                                <input type="button" id="btn_connect_1" class="btn btn-info" value=<#Connect#> onclick="submitInternet('Reconnect');">
                                                <input type="button" id="btn_connect_0" class="btn btn-danger" value=<#Disconnect#> onclick="submitInternet('Disconnect');">
                                            </td>
                                        </tr>

                                        <tr> <th><#running_status#>&nbsp;&nbsp;&nbsp;&nbsp;<span class="label label-info" style="padding: 5px 5px 5px 5px;" id="scutclient_version"></span> </th>
                                            <td id="scutclient_status" colspan="3"></td>
                                        </tr>

                                        <tr> <th><#menu5_13_enable#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="scutclient_enable_on_of">
                                                        <input type="checkbox" id="scutclient_enable_fake" <% nvram_match_x("", "scutclient_enable", "1", "value=1 checked"); %><% nvram_match_x("", "scutclient_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="scutclient_enable" id="scutclient_enable_1" <% nvram_match_x("", "scutclient_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="scutclient_enable" id="scutclient_enable_0" <% nvram_match_x("", "scutclient_enable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_13_username#></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="scutclient_username" style="width: 145px" value="<% nvram_get_x("","scutclient_username"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_13_password#></th>
                                            <td>
                                                <input type="password" maxlength="32" class="input" size="32" name="scutclient_password" id="scutclient_password" style="width: 145px" value="<% nvram_get_x("","scutclient_password"); %>" />
                                                <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('scutclient_password')"><i class="icon-eye-close"></i></button>
                                            </td>
                                        </tr>	

                                        <tr> <th width="50%"><#menu5_13_authip#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="scutclient_server_auth_ip" style="width: 145px" value="<% nvram_get_x("","scutclient_server_auth_ip"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_13_hostname#></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="scutclient_hostname" value="<% nvram_get_x("", "scutclient_hostname"); %>">
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_13_version#></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="scutclient_version" value="<% nvram_get_x("", "scutclient_version"); %>">
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_13_hash#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="64" name="scutclient_hash" value="<% nvram_get_x("", "scutclient_hash"); %>">
                                            </td>
                                        </tr>

                                        <tr> <th><#menu5_13_debug#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="scutclient_debug_on_of">
                                                        <input type="checkbox" id="scutclient_debug_fake" <% nvram_match_x("", "scutclient_debug", "1", "value=1 checked"); %><% nvram_match_x("", "scutclient_debug", "0", "value=0"); %>>
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="scutclient_debug" id="scutclient_debug_1" <% nvram_match_x("", "scutclient_debug", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="scutclient_debug" id="scutclient_debug_0" <% nvram_match_x("", "scutclient_debug", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        
                                        <tr> <th><#menu5_13_watchcat#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="scutclient_watchcat_on_of">
                                                        <input type="checkbox" id="scutclient_watchcat_fake" <% nvram_match_x("", "scutclient_watchcat", "1", "value=1 checked"); %><% nvram_match_x("", "scutclient_watchcat", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="scutclient_watchcat" id="scutclient_watchcat_1" <% nvram_match_x("", "scutclient_watchcat", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="scutclient_watchcat" id="scutclient_watchcat_0" <% nvram_match_x("", "scutclient_watchcat", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th><#menu5_13_udpHB#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="scutclient_skip_udp_hb_on_of">
                                                        <input type="checkbox" id="scutclient_skip_udp_hb_fake" <% nvram_match_x("", "scutclient_skip_udp_hb", "1", "value=1 checked"); %><% nvram_match_x("", "scutclient_skip_udp_hb", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="scutclient_skip_udp_hb" id="scutclient_skip_udp_hb_1" <% nvram_match_x("", "scutclient_skip_udp_hb", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="scutclient_skip_udp_hb" id="scutclient_skip_udp_hb_0" <% nvram_match_x("", "scutclient_skip_udp_hb", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th><#menu5_13_watchcat_force#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="scutclient_wdg_force_on_of">
                                                        <input type="checkbox" id="scutclient_wdg_force_fake" <% nvram_match_x("", "scutclient_wdg_force", "1", "value=1 checked"); %><% nvram_match_x("", "scutclient_wdg_force", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="scutclient_wdg_force" id="scutclient_wdg_force_1" <% nvram_match_x("", "scutclient_wdg_force", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="scutclient_wdg_force" id="scutclient_wdg_force_0" <% nvram_match_x("", "scutclient_wdg_force", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr>
                                            <td colspan="2">
                                                <center><input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" /></center>
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

<form method="post" name="scutclient_action" action="">
<input type="hidden" name="connect_action" value="">
</form>


</body>
</html>
