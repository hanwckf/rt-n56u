<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_16#></title>
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
<% shadowsocks_status(); %>
<% rules_count(); %>

var $j = jQuery.noConflict();

$j(document).ready(function(){
	init_itoggle('ss_enable');
	init_itoggle('ss_router_proxy',change_ss_watchcat_display);
	init_itoggle('ss_udp');
	init_itoggle('ss_watchcat');
	init_itoggle('ss_update_chnroute');
	init_itoggle('ss_update_gfwlist');
	init_itoggle('ss-tunnel_enable');
});

function initial(){
	show_banner(2);
	show_menu(5,13,1);
	show_footer();
	var o1 = document.form.ss_method;
	var o2 = document.form.ss_mode;
	var o3 = document.form.ss_protocol;
	var o4 = document.form.ss_obfs;
	var o5 = document.form.ss_lower_port_only;
	var o6 = document.form.ss_type;
	o1.value = '<% nvram_get_x("","ss_method"); %>';
	o2.value = '<% nvram_get_x("","ss_mode"); %>';
	o3.value = '<% nvram_get_x("","ss_protocol"); %>';
	o4.value = '<% nvram_get_x("","ss_obfs"); %>';
	o5.value = '<% nvram_get_x("","ss_lower_port_only"); %>';
	o6.value = '<% nvram_get_x("","ss_type"); %>';
	change_ss_watchcat_display();
	fill_ss_status(shadowsocks_status());
	fill_ss_tunnel_status(shadowsocks_tunnel_status());
	$("chnroute_count").innerHTML = '<#menu5_17_3#>' + chnroute_count() ;
	$("gfwlist_count").innerHTML = '<#menu5_17_3#>' + gfwlist_count() ;
	switch_ss_type();
}

function switch_ss_type(){
	var v = document.form.ss_type.value; //0:ss-orig;1:ssr
	showhide_div('row_ss_protocol', v);
	showhide_div('row_ss_protocol_para', v);
	showhide_div('row_ss_obfs', v);
	showhide_div('row_ss_obfs_para', v);
}

function applyRule(){
	showLoading();
	document.form.action_mode.value = " Restart ";
	document.form.current_page.value = "Shadowsocks.asp";
	document.form.next_page.value = "";
	document.form.submit();
}

function submitInternet(v){
	showLoading();
	document.Shadowsocks_action.action = "Shadowsocks_action.asp";
	document.Shadowsocks_action.connect_action.value = v;
	document.Shadowsocks_action.submit();
}

function change_ss_watchcat_display(){
	var v = document.form.ss_router_proxy[0].checked;
	showhide_div('ss_wathcat_option', v);
}

function fill_ss_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("ss_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
}

function fill_ss_tunnel_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("ss_tunnel_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
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
	
    <input type="hidden" name="current_page" value="Shadowsocks.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="ShadowsocksConf;">
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
                            <h2 class="box_head round_top"><#menu5_16#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_16_1#></th> </tr>

                                        <tr> <th width="50%"><#InetControl#></th>
                                            <td style="border-top: 0 none;" colspan="2">
                                                <input type="button" id="btn_connect_1" class="btn btn-info" value=<#Connect#> onclick="submitInternet('Reconnect');">
                                            </td>
                                        </tr>

                                        <tr> <th><#running_status#></th>
                                            <td id="ss_status" colspan="3"></td>
                                        </tr>

                                        <tr> <th><#menu5_16_2#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ss_enable_on_of">
                                                        <input type="checkbox" id="ss_enable_fake" <% nvram_match_x("", "ss_enable", "1", "value=1 checked"); %><% nvram_match_x("", "ss_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_enable" id="ss_enable_1" <% nvram_match_x("", "ss_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_enable" id="ss_enable_0" <% nvram_match_x("", "ss_enable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_16_30#></th>
                                            <td>
                                                <select name="ss_type" class="input" style="width: 200px;" onchange="switch_ss_type()">
                                                    <option value="0" >SS</option>
                                                    <option value="1" >SSR</option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_16_3#></th> </tr>
										
                                        <tr> <th width="50%"><#menu5_16_4#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="64" name="ss_server" value="<% nvram_get_x("","ss_server"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_16_5#></th>
                                            <td>
                                                <input type="password" maxlength="32" class="input" size="32" name="ss_key" id="ss_key" value="<% nvram_get_x("","ss_key"); %>" />
                                                <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('ss_key')"><i class="icon-eye-close"></i></button>
                                            </td>
                                        </tr>	
										
                                        <tr> <th width="50%"><#menu5_16_6#></th>
                                            <td>
                                                <input type="text" maxlength="6" class="input" size="15" name="ss_server_port" style="width: 145px" value="<% nvram_get_x("","ss_server_port"); %>" />
                                            </td>
                                        </tr>
										
                                        <tr> <th width="50%"><#menu5_16_7#></th>
                                            <td>
                                                <select name="ss_method" class="input" style="width: 250px;">
                                                    <option value="none" >none (ssr only)</option>
                                                    <option value="rc4" >rc4</option>
                                                    <option value="rc4-md5" >rc4-md5</option>
                                                    <option value="aes-128-cfb" >aes-128-cfb</option>
                                                    <option value="aes-192-cfb" >aes-192-cfb</option>
                                                    <option value="aes-256-cfb" >aes-256-cfb</option>
                                                    <option value="aes-128-ctr" >aes-128-ctr</option>
                                                    <option value="aes-192-ctr" >aes-192-ctr</option>
                                                    <option value="aes-256-ctr" >aes-256-ctr</option>
                                                    <option value="camellia-128-cfb" >camellia-128-cfb</option>
                                                    <option value="camellia-192-cfb" >camellia-192-cfb</option>
                                                    <option value="camellia-256-cfb" >camellia-256-cfb</option>
                                                    <option value="bf-cfb" >bf-cfb</option>
                                                    <option value="salsa20" >salsa20</option>
                                                    <option value="chacha20" >chacha20</option>
                                                    <option value="chacha20-ietf" >chacha20-ietf</option>
                                                    <option value="aes-128-gcm" >aes-128-gcm (ss only)</option>
                                                    <option value="aes-192-gcm" >aes-192-gcm (ss only)</option>
                                                    <option value="aes-256-gcm" >aes-256-gcm (ss only)</option>
                                                    <option value="chacha20-ietf-poly1305" >chacha20-ietf-poly1305 (ss only)</option>
                                                    <option value="xchacha20-ietf-poly1305" >xchacha20-ietf-poly1305 (ss only)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        
                                        <tr> <th width="50%"><#menu5_16_21#></th>
                                            <td>
                                                <input type="text" maxlength="6" class="input" size="15" name="ss_timeout" style="width: 145px" value="<% nvram_get_x("","ss_timeout"); %>" />
                                            </td>
                                        </tr>
                                        
                                        <tr id="row_ss_protocol" style="display:none;"> <th width="50%"><#menu5_16_22#></th>
                                            <td>
                                                <select name="ss_protocol" class="input" style="width: 200px;">   
                                                    <option value="origin" >origin</option>
                                                    <option value="auth_sha1" >auth_sha1</option>
                                                    <option value="auth_sha1_v2" >auth_sha1_v2</option>
                                                    <option value="auth_sha1_v4" >auth_sha1_v4</option>
                                                    <option value="auth_aes128_md5" >auth_aes128_md5</option>
                                                    <option value="auth_aes128_sha1" >auth_aes128_sha1</option>
                                                    <option value="auth_chain_a" >auth_chain_a</option>
                                                    <option value="auth_chain_b" >auth_chain_b</option>
                                                </select>
                                            </td>
                                        </tr>
                                        
                                        <tr id="row_ss_protocol_para" style="display:none;"> <th width="50%"><#menu5_16_23#></th>
                                            <td>
                                                <input type="text" maxlength="72" class="input" size="64" name="ss_proto_param" value="<% nvram_get_x("","ss_proto_param"); %>" />
                                            </td>
                                        </tr>
                                        
                                        <tr id="row_ss_obfs" style="display:none;"> <th width="50%"><#menu5_16_24#></th>
                                            <td>
                                                <select name="ss_obfs" class="input" style="width: 200px;">   
                                                    <option value="plain" >plain</option>
                                                    <option value="http_simple" >http_simple</option>
                                                    <option value="http_post" >http_post</option>
                                                    <option value="tls1.2_ticket_auth" >tls1.2_ticket_auth</option>
                                                </select>
                                            </td>
                                        </tr>
                                        
                                        <tr id="row_ss_obfs_para" style="display:none;"> <th width="50%"><#menu5_16_25#></th>
                                            <td>
                                                <input type="text" maxlength="72" class="input" size="64" name="ss_obfs_param" value="<% nvram_get_x("","ss_obfs_param"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_16_8#></th> </tr>

                                        <tr> <th width="50%"><#menu5_16_9#></th>
                                            <td>
                                                <input type="text" maxlength="6" class="input" size="15" name="ss_local_port" style="width: 145px" value="<% nvram_get_x("", "ss_local_port"); %>">
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_16_10#></th>
                                            <td>
                                                <select name="ss_mode" class="input" style="width: 145px;">   
                                                    <option value="0" ><#menu5_16_11#></option>
                                                    <option value="1" ><#ChnRoute#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_16_12#></th> </tr>

                                        <tr> <th width="50%"><#InetControl#></th>
                                            <td style="border-top: 0 none;" colspan="2">
                                                <input type="button" id="btn_connect_2" class="btn btn-info" value=<#Connect#> onclick="submitInternet('Reconnect_ss_tunnel');">
                                            </td>
                                        </tr>

                                        <tr> <th><#menu5_16_13#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ss-tunnel_enable_on_of">
                                                        <input type="checkbox" id="ss-tunnel_enable_fake" <% nvram_match_x("", "ss-tunnel_enable", "1", "value=1 checked"); %><% nvram_match_x("", "ss-tunnel_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss-tunnel_enable" id="ss-tunnel_enable_1" <% nvram_match_x("", "ss-tunnel_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss-tunnel_enable" id="ss-tunnel_enable_0" <% nvram_match_x("", "ss-tunnel_enable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th><#running_status#></th>
                                            <td id="ss_tunnel_status" colspan="3"></td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_16_14#></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="64" name="ss-tunnel_remote" value="<% nvram_get_x("","ss-tunnel_remote"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_16_15#></th>
                                            <td>
                                                <input type="text" maxlength="6" class="input" size="15" name="ss-tunnel_local_port" style="width: 145px" value="<% nvram_get_x("", "ss-tunnel_local_port"); %>">
                                            </td>
                                        </tr>

                                        <tr> <th width="50%">MTU:</th>
                                            <td>
                                                <input type="text" maxlength="6" class="input" size="15" name="ss-tunnel_mtu" style="width: 145px" value="<% nvram_get_x("", "ss-tunnel_mtu"); %>">
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#menu5_1_6#></th> </tr>										

                                        <tr> <th><#menu5_16_16#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ss_router_proxy_on_of">
                                                        <input type="checkbox" id="ss_router_proxy_fake" <% nvram_match_x("", "ss_router_proxy", "1", "value=1 checked"); %><% nvram_match_x("", "ss_router_proxy", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_router_proxy" id="ss_router_proxy_1" <% nvram_match_x("", "ss_router_proxy", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_router_proxy" id="ss_router_proxy_0" <% nvram_match_x("", "ss_router_proxy", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr id="ss_wathcat_option"> <th><#menu5_13_watchcat#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ss_watchcat_on_of">
                                                        <input type="checkbox" id="ss_watchcat_fake" <% nvram_match_x("", "ss_watchcat", "1", "value=1 checked"); %><% nvram_match_x("", "ss_watchcat", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_watchcat" id="ss_watchcat_1" <% nvram_match_x("", "ss_watchcat", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_watchcat" id="ss_watchcat_0" <% nvram_match_x("", "ss_watchcat", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th><#menu5_16_17#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ss_udp_on_of">
                                                        <input type="checkbox" id="ss_udp_fake" <% nvram_match_x("", "ss_udp", "1", "value=1 checked"); %><% nvram_match_x("", "ss_udp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_udp" id="ss_udp_1" <% nvram_match_x("", "ss_udp", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_udp" id="ss_udp_0" <% nvram_match_x("", "ss_udp", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%"><#menu5_16_18#></th>
                                            <td>
                                                <select name="ss_lower_port_only" class="input" style="width: 200px;">
                                                    <option value="0" ><#menu5_16_18_0#></option>
                                                    <option value="1" ><#menu5_16_18_1#></option>
                                                    <option value="2" ><#menu5_16_18_2#></option>
                                                </select>
                                            </td>
                                        </tr>

                                        <tr> <th width="50%">MTU:</th>
                                            <td>
                                                <input type="text" maxlength="6" class="input" size="15" name="ss_mtu" style="width: 145px" value="<% nvram_get_x("", "ss_mtu"); %>">
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#ChnRoute#></th> </tr>

                                        <tr>
                                            <th width="50%"><#menu5_17_1#>&nbsp;&nbsp;&nbsp;&nbsp;<span class="label label-info" style="padding: 5px 5px 5px 5px;" id="chnroute_count"></span></th>
                                            <td style="border-top: 0 none;" colspan="2">
                                                <input type="button" id="btn_connect_3" class="btn btn-info" value=<#menu5_17_2#> onclick="submitInternet('Update_chnroute');">
                                            </td>
                                        </tr>

                                        <tr> <th><#menu5_16_19#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ss_update_chnroute_on_of">
                                                        <input type="checkbox" id="ss_update_chnroute_fake" <% nvram_match_x("", "ss_update_chnroute", "1", "value=1 checked"); %><% nvram_match_x("", "ss_update_chnroute", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_update_chnroute" id="ss_update_chnroute_1" <% nvram_match_x("", "ss_update_chnroute", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_update_chnroute" id="ss_update_chnroute_0" <% nvram_match_x("", "ss_update_chnroute", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th colspan="2" style="background-color: #E3E3E3;"><#GfwList#></th> </tr>

                                        <tr>
                                            <th width="50%"><#menu5_17_1#>&nbsp;&nbsp;&nbsp;&nbsp;<span class="label label-info" style="padding: 5px 5px 5px 5px;" id="gfwlist_count"></span></th>
                                            <td style="border-top: 0 none;" colspan="2">
                                                <input type="button" id="btn_connect_4" class="btn btn-info" value=<#menu5_17_2#> onclick="submitInternet('Update_gfwlist');">
                                            </td>
                                        </tr>

                                        <tr> <th><#menu5_16_19#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ss_update_gfwlist_on_of">
                                                        <input type="checkbox" id="ss_update_gfwlist_fake" <% nvram_match_x("", "ss_update_gfwlist", "1", "value=1 checked"); %><% nvram_match_x("", "ss_update_gfwlist", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_update_gfwlist" id="ss_update_gfwlist_1" <% nvram_match_x("", "ss_update_gfwlist", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_update_gfwlist" id="ss_update_gfwlist_0" <% nvram_match_x("", "ss_update_gfwlist", "0", "checked"); %>><#checkbox_No#>
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

<form method="post" name="Shadowsocks_action" action="">
    <input type="hidden" name="connect_action" value="">
</form>


</body>
</html>
