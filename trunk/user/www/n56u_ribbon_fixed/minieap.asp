<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_19#></title>
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
<% minieap_status(); %>

$j(document).ready(function(){
	init_itoggle('minieap_enable');
	fill_status(minieap_status());
});

function initial(){
	show_banner(2);
	show_menu(5,15,1);
	show_footer();
	var o1 = document.form.minieap_module;
	var o2 = document.form.minieap_daemonize;
	var o3 = document.form.minieap_if_impl;
	var o4 = document.form.minieap_eap_bcast_addr;
	var o5 = document.form.minieap_dhcp_type;
	o1.value = '<% nvram_get_x("","minieap_module"); %>';
	o2.value = '<% nvram_get_x("","minieap_daemonize"); %>';
	o3.value = '<% nvram_get_x("","minieap_if_impl"); %>';
	o4.value = '<% nvram_get_x("","minieap_eap_bcast_addr"); %>';
	o5.value = '<% nvram_get_x("","minieap_dhcp_type"); %>';
}

function applyRule(){
	if(validForm()){
		showLoading();
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "minieap.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	return true;
}

function submitInternet(v){
	showLoading();
	document.minieap_action.action = "minieap_action.asp";
	document.minieap_action.connect_action.value = v;
	document.minieap_action.submit();
}

function fill_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("minieap_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
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
	
    <input type="hidden" name="current_page" value="minieap.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="minieapConf;">
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
                            <h2 class="box_head round_top"><#menu5_19#></h2>
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

                                        <tr> <th><#running_status#></th>
                                            <td id="minieap_status" colspan="3"></td>
                                        </tr>

                                        <tr> <th><#menu5_19_1#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="minieap_enable_on_of">
                                                        <input type="checkbox" id="minieap_enable_fake" <% nvram_match_x("", "minieap_enable", "1", "value=1 checked"); %><% nvram_match_x("", "minieap_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="minieap_enable" id="minieap_enable_1" <% nvram_match_x("", "minieap_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="minieap_enable" id="minieap_enable_0" <% nvram_match_x("", "minieap_enable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_2#></th>
                                            <td>
                                                <input type="text" maxlength="48" class="input" size="48" name="minieap_username" style="width: 180px" value="<% nvram_get_x("","minieap_username"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_3#></th>
                                            <td>
                                                <input type="password" maxlength="32" class="input" size="32" name="minieap_password" id="minieap_password" style="width: 145px" value="<% nvram_get_x("","minieap_password"); %>" />
                                                <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('minieap_password')"><i class="icon-eye-close"></i></button>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_4#></th>
                                            <td>
                                                <select name="minieap_module" class="input" style="width: 145px;">
                                                    <option value="printer" ><#menu5_19_4_0#></option>
                                                    <option value="rjv3" ><#menu5_19_4_1#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_5#></th>
                                            <td>
                                                <select name="minieap_daemonize" class="input" style="width: 145px;">
                                                    <option value="0" ><#menu5_19_5_0#></option>
                                                    <option value="1" ><#menu5_19_5_1#></option>
                                                    <option value="2" ><#menu5_19_5_2#></option>
                                                    <option value="3" ><#menu5_19_5_3#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_6#></th>
                                            <td>
                                                <select name="minieap_if_impl" class="input" style="width: 145px;">
                                                    <option value="sockraw" ><#menu5_19_6_0#></option>
                                                    <option value="libpcap" ><#menu5_19_6_1#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_7#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="minieap_max_fail" style="width: 145px" value="<% nvram_get_x("","minieap_max_fail"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_19_8_0#></th> </tr>

                                        <tr> <th width="50%"><#menu5_19_8#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="minieap_max_retries" style="width: 145px" value="<% nvram_get_x("","minieap_max_retries"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_9#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="minieap_no_auto_reauth" style="width: 145px" value="<% nvram_get_x("","minieap_no_auto_reauth"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_10#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="minieap_stage_timeout" style="width: 145px" value="<% nvram_get_x("","minieap_stage_timeout"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_11#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="minieap_auth_round" style="width: 145px" value="<% nvram_get_x("","minieap_auth_round"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_12#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="minieap_heartbeat" style="width: 145px" value="<% nvram_get_x("","minieap_heartbeat"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_13#></th>
                                            <td>
                                                <select name="minieap_eap_bcast_addr" class="input" style="width: 145px;">
                                                    <option value="0" ><#menu5_19_13_0#></option>
                                                    <option value="1" ><#menu5_19_13_1#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_14#></th>
                                            <td>
                                                <select name="minieap_dhcp_type" class="input" style="width: 145px;">
                                                    <option value="0" ><#menu5_19_14_0#></option>
                                                    <option value="1" ><#menu5_19_14_1#></option>
                                                    <option value="2" ><#menu5_19_14_2#></option>
                                                    <option value="3" ><#menu5_19_14_3#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_15#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="minieap_service" style="width: 145px;" value="<% nvram_get_x("","minieap_service"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_16#></th>
                                            <td>
					    	                    <input type="text" maxlength="32" class="input" size="15" name="minieap_version_str" style="width: 145px" value="<% nvram_get_x("","minieap_version_str"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_17#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="32" name="minieap_dhcp_script" style="width: 145px" value="<% nvram_get_x("","minieap_dhcp_script"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_18#></th>
                                            <td>
                                                <input type="text" maxlength="24" class="input" size="24" name="minieap_fake_serial" value="<% nvram_get_x("","minieap_fake_serial"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_19_19#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="minieap_max_dhcp_count" value="<% nvram_get_x("","minieap_max_dhcp_count"); %>" />
                                            </td>
                                        </tr>

                                        <tr>
                                            <th colspan="2" style="color:dimgray;"> 
                                                <center>Supported by 洛 & Powered by MiniEAP-Taiga</center>
                                            </th>    
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

<form method="post" name="minieap_action" action="">
<input type="hidden" name="connect_action" value="">
</form>


</body>
</html>
