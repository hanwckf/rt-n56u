<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_18#></title>
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
<% mentohust_status(); %>

$j(document).ready(function(){
	init_itoggle('mentohust_enable');
	fill_status(mentohust_status());
});

function initial(){
	show_banner(2);
	show_menu(5,14,1);
	show_footer();
	var o1 = document.form.mentohust_startmode;
	var o2 = document.form.mentohust_dhcp;
	var o3 = document.form.mentohust_daemon;
	var o4 = document.form.mentohust_service;
	o1.value = '<% nvram_get_x("","mentohust_startmode"); %>';
	o2.value = '<% nvram_get_x("","mentohust_dhcp"); %>';
	o3.value = '<% nvram_get_x("","mentohust_daemon"); %>';
	o4.value = '<% nvram_get_x("","mentohust_service"); %>';
}

function applyRule(){
	if(validForm()){
		showLoading();
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "mentohust.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	return true;
}

function submitInternet(v){
	showLoading();
	document.mentohust_action.action = "mentohust_action.asp";
	document.mentohust_action.connect_action.value = v;
	document.mentohust_action.submit();
}

function fill_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("mentohust_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
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
	
    <input type="hidden" name="current_page" value="mentohust.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="mentohustConf;">
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
                            <h2 class="box_head round_top"><#menu5_18#></h2>
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
                                            <td id="mentohust_status" colspan="3"></td>
                                        </tr>

                                        <tr> <th><#menu5_18_1#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="mentohust_enable_on_of">
                                                        <input type="checkbox" id="mentohust_enable_fake" <% nvram_match_x("", "mentohust_enable", "1", "value=1 checked"); %><% nvram_match_x("", "mentohust_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="mentohust_enable" id="mentohust_enable_1" <% nvram_match_x("", "mentohust_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="mentohust_enable" id="mentohust_enable_0" <% nvram_match_x("", "mentohust_enable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_2#></th>
                                            <td>
                                                <input type="text" maxlength="48" class="input" size="48" name="mentohust_username" style="width: 180px" value="<% nvram_get_x("","mentohust_username"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_3#></th>
                                            <td>
                                                <input type="password" maxlength="32" class="input" size="32" name="mentohust_password" id="mentohust_password" style="width: 145px" value="<% nvram_get_x("","mentohust_password"); %>" />
                                                <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('mentohust_password')"><i class="icon-eye-close"></i></button>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_4#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="mentohust_ip" style="width: 145px" value="<% nvram_get_x("","mentohust_ip"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_5#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="mentohust_mask" style="width: 145px" value="<% nvram_get_x("","mentohust_mask"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_6#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="mentohust_gw" style="width: 145px" value="<% nvram_get_x("","mentohust_gw"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_7#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="mentohust_dns" style="width: 145px" value="<% nvram_get_x("","mentohust_dns"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_18_8_0#></th> </tr>

                                        <tr> <th width="50%"><#menu5_18_8#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="mentohust_pinghost" style="width: 145px" value="<% nvram_get_x("","mentohust_pinghost"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_9#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="mentohust_timeout" style="width: 145px" value="<% nvram_get_x("","mentohust_timeout"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_10#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="mentohust_interval" style="width: 145px" value="<% nvram_get_x("","mentohust_interval"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_11#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="mentohust_restart_wait" style="width: 145px" value="<% nvram_get_x("","mentohust_restart_wait"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_12#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="mentohust_maxfail" style="width: 145px" value="<% nvram_get_x("","mentohust_maxfail"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_13#></th>
                                            <td>
                                                <select name="mentohust_startmode" class="input" style="width: 145px;">
                                                    <option value="0" ><#menu5_18_13_0#></option>
                                                    <option value="1" ><#menu5_18_13_1#></option>
                                                    <option value="2" ><#menu5_18_13_2#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_14#></th>
                                            <td>
                                                <select name="mentohust_dhcp" class="input" style="width: 145px;">
                                                    <option value="0" ><#menu5_18_14_0#></option>
                                                    <option value="1" ><#menu5_18_14_1#></option>
                                                    <option value="2" ><#menu5_18_14_2#></option>
                                                    <option value="3" ><#menu5_18_14_3#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_15#></th>
                                            <td>
                                                <select name="mentohust_daemon" class="input" style="width: 145px;">
                                                    <option value="1" ><#menu5_18_15_1#></option>
                                                    <option value="3" ><#menu5_18_15_3#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_16#></th>
                                            <td>
                                                <select name="mentohust_service" class="input" style="width: 145px;">
                                                    <option value="0" ><#menu5_18_16_0#></option>
                                                    <option value="1" ><#menu5_18_16_1#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_18_17#></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="mentohust_ver" style="width: 145px" value="<% nvram_get_x("","mentohust_ver"); %>" />
                                            </td>
                                        </tr>


                                        <tr> <th width="50%"><#menu5_18_18#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="32" name="mentohust_datafile" value="<% nvram_get_x("","mentohust_datafile"); %>" />
                                            </td>
                                        </tr>


                                        <tr> <th width="50%"><#menu5_18_19#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="32" name="mentohust_dhcpscript" value="<% nvram_get_x("","mentohust_dhcpscript"); %>" />
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

<form method="post" name="mentohust_action" action="">
<input type="hidden" name="connect_action" value="">
</form>


</body>
</html>
