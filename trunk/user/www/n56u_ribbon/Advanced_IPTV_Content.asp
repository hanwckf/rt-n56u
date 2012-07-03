<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_4#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#mr_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                change_common_radio(this, 'WLANConfig11b', 'mr_enable_x', '1');
                $j("#mr_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#mr_enable_x_1").attr("checked", "checked");
                $j("#mr_enable_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                change_common_radio(this, 'WLANConfig11b', 'mr_enable_x', '0');
                $j("#mr_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#mr_enable_x_0").attr("checked", "checked");
                $j("#mr_enable_x_1").removeAttr("checked");
            }
        });
        $j("#mr_enable_x_on_of label.itoggle").css("background-position", $j("input#mr_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#rt_IgmpSnEnable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#rt_IgmpSnEnable_fake").attr("checked", "checked").attr("value", 1);
                $j("#rt_IgmpSnEnable_1").attr("checked", "checked");
                $j("#rt_IgmpSnEnable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#rt_IgmpSnEnable_fake").removeAttr("checked").attr("value", 0);
                $j("#rt_IgmpSnEnable_0").attr("checked", "checked");
                $j("#rt_IgmpSnEnable_1").removeAttr("checked");
            }
        });
        $j("#rt_IgmpSnEnable_on_of label.itoggle").css("background-position", $j("input#rt_IgmpSnEnable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#wl_IgmpSnEnable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wl_IgmpSnEnable_fake").attr("checked", "checked").attr("value", 1);
                $j("#wl_IgmpSnEnable_1").attr("checked", "checked");
                $j("#wl_IgmpSnEnable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#wl_IgmpSnEnable_fake").removeAttr("checked").attr("value", 0);
                $j("#wl_IgmpSnEnable_0").attr("checked", "checked");
                $j("#wl_IgmpSnEnable_1").removeAttr("checked");
            }
        });
        $j("#wl_IgmpSnEnable_on_of label.itoggle").css("background-position", $j("input#wl_IgmpSnEnable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    })
</script>

<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,2,4);
	show_footer();
	
	enable_auto_hint(6, 5);
}

function applyRule(){
	if(document.form.udpxy_enable_x.value != 0){
		if(!validate_range(document.form.udpxy_enable_x, 1024, 65535)){
			return;
		}
	}
	
	showLoading();
	
	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "/Advanced_IPTV_Content.asp";
	document.form.next_page.value = "";
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function valid_udpxy(){
	if(document.form.udpxy_enable_x.value != 0)
		validate_range(document.form.udpxy_enable_x, 1024, 65535);
}

function valid_muliticast(){
	if(document.form.controlrate_unknown_unicast.value != 0)
		validate_range(document.form.controlrate_unknown_unicast, 0, 1024);
	if(document.form.controlrate_unknown_multicast.value != 0)
		validate_range(document.form.controlrate_unknown_multicast, 0, 1024);
	if(document.form.controlrate_multicast.value != 0)
		validate_range(document.form.controlrate_multicast, 0, 1024);
	if(document.form.controlrate_broadcast.value != 0)
		validate_range(document.form.controlrate_broadcast, 0, 1024);
}

</script>

<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(6, 5);return unload_body();">
<div class="container-fluid" style="padding-right: 0px">
    <div class="row-fluid">
        <div class="span2"><center><div id="logo"></div></center></div>
        <div class="span10" >
            <div id="TopBanner"></div>
        </div>
    </div>
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_IPTV_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="RouterConfig;LANHostConfig;WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<div class="container-fluid">
    <div class="row-fluid">
        <div class="span2">
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

        <div class="span10">
            <!--Body content-->
            <div class="row-fluid">
                <div class="span12">
                    <div class="box well grad_colour_dark_blue">
                        <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_4#></h2>
                        <div class="round_bottom">
                            <div class="row-fluid">
                                <div id="tabMenu" class="submenuBlock"></div>
                                <div class="alert alert-info" style="margin: 10px;">Multicast and IPTV</div>

                                <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th colspan="2" style="background-color: #E3E3E3;">IPTV Control</th>
                                    </tr>

                                    <tr>
                                        <th width="50%"><#RouterConfig_GWMulticastEnable_itemname#></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="mr_enable_x_on_of">
                                                    <input type="checkbox" id="mr_enable_x_fake" <% nvram_match_x("WLANConfig11b", "mr_enable_x", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b", "mr_enable_x", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" value="1" name="mr_enable_x" id="mr_enable_x_1" class="input" <% nvram_match_x("RouterConfig", "mr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                <input type="radio" value="0" name="mr_enable_x" id="mr_enable_x_0" class="input" <% nvram_match_x("RouterConfig", "mr_enable_x", "0", "checked"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6, 6);"><#RouterConfig_IPTV_itemname#>:</a></th>
                                        <td>
                                            <input id="udpxy_enable_x" type="text" maxlength="5" class="input" size="15" name="udpxy_enable_x" value="<% nvram_get_x("LANHostConfig", "udpxy_enable_x"); %>" onkeypress="return is_number(this);" onblur="valid_udpxy();"/>
                                        </td>
                                    </tr>
                                </table>

                                <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th colspan="2" style="background-color: #E3E3E3;">Storm Control for Ethernet</th>
                                    </tr>
                                    <tr>
                                        <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 7);"><#RouterConfig_GWMulticast_unknownUni_itemname#></a></th>
                                        <td>
                                            <input id="controlrate_unknown_unicast" type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_unicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_unicast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 8);"><#RouterConfig_GWMulticast_unknownMul_itemname#></a></th>
                                        <td>
                                            <input id="controlrate_unknown_multicast" type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_multicast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 9);"><#RouterConfig_GWMulticast_Multicast_itemname#></a></th>
                                        <td>
                                            <input id="controlrate_multicast" type="text" maxlength="4" class="input" size="15" name="controlrate_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_multicast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 10);"><#RouterConfig_GWMulticast_Broadcast_itemname#></a></th>
                                        <td>
                                            <input id="controlrate_broadcast" type="text" maxlength="4" class="input" size="15" name="controlrate_broadcast" value="<% nvram_get_x("LANHostConfig", "controlrate_broadcast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                        </td>
                                    </tr>
                                </table>

                                <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th colspan="2" style="background-color: #E3E3E3;">Multicast - WiFi 2.4GHz</th>
                                    </tr>
                                    <tr>
                                        <th width="50%">IGMP Snooping</th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="rt_IgmpSnEnable_on_of">
                                                    <input type="checkbox" id="rt_IgmpSnEnable_fake" <% nvram_match_x("WLANConfig11b", "rt_IgmpSnEnable", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b", "rt_IgmpSnEnable", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" value="1" name="rt_IgmpSnEnable" id="rt_IgmpSnEnable_1" class="input" <% nvram_match_x("WLANConfig11b", "rt_IgmpSnEnable", "1", "checked"); %>><#checkbox_Yes#>
                                                <input type="radio" value="0" name="rt_IgmpSnEnable" id="rt_IgmpSnEnable_0" class="input" <% nvram_match_x("WLANConfig11b", "rt_IgmpSnEnable", "0", "checked"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
                                        <td>
                                            <select name="rt_mcastrate" class="input">
                                                <option value="0" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "0", "selected"); %>>HTMIX (1S) 6.5-15 Mbps</option>
                                                <option value="1" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "1", "selected"); %>>HTMIX (1S) 13-30 Mbps</option>
                                                <option value="2" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "2", "selected"); %>>HTMIX (1S) 19.5-45 Mbps</option>
                                                <option value="3" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "3", "selected"); %>>HTMIX (2S) 13-30 Mbps</option>
                                                <option value="4" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "4", "selected"); %>>HTMIX (2S) 26-60 Mbps</option>
                                                <option value="5" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                <option value="6" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
                                                <option value="7" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
                                                <option value="8" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
                                                <option value="9" <% nvram_match_x("WLANConfig11b", "rt_mcastrate", "9", "selected"); %>>CCK 11 Mbps</option>
                                            </select>
                                        </td>
                                    </tr>
                                </table>

                                <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <th colspan="2" style="background-color: #E3E3E3;">Multicast - WiFi 5GHz</th>
                                    </tr>
                                    <tr>
                                        <th width="50%">IGMP Snooping</th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="wl_IgmpSnEnable_on_of">
                                                    <input type="checkbox" id="wl_IgmpSnEnable_fake" <% nvram_match_x("WLANConfig11b", "wl_IgmpSnEnable", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b", "wl_IgmpSnEnable", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" value="1" name="wl_IgmpSnEnable" id="wl_IgmpSnEnable_1" class="input" <% nvram_match_x("WLANConfig11b", "wl_IgmpSnEnable", "1", "checked"); %>><#checkbox_Yes#>
                                                <input type="radio" value="0" name="wl_IgmpSnEnable" id="wl_IgmpSnEnable_0" class="input" <% nvram_match_x("WLANConfig11b", "wl_IgmpSnEnable", "0", "checked"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 3, 7);"><#WLANConfig11b_MultiRateAll_itemname#></a></th>
                                        <td>
                                            <select name="wl_mcastrate" class="input">
                                                <option value="0" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "0", "selected"); %>>HTMIX (1S) 6.5-15 Mbps</option>
                                                <option value="1" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "1", "selected"); %>>HTMIX (1S) 13-30 Mbps</option>
                                                <option value="2" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "2", "selected"); %>>HTMIX (1S) 19.5-45 Mbps</option>
                                                <option value="3" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "3", "selected"); %>>HTMIX (2S) 13-30 Mbps</option>
                                                <option value="4" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "4", "selected"); %>>HTMIX (2S) 26-60 Mbps</option>
                                                <option value="5" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                <option value="6" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
                                                <option value="7" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
                                                <option value="8" <% nvram_match_x("WLANConfig11b", "wl_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td colspan="2" style="border-top: 0 none;">
                                            <br/>
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

<!--==============Beginning of hint content=============-->
<div id="help_td" style="position: absolute; margin-left: -10000px" valign="top">
    <form name="hint_form"></form>
    <div id="helpicon" onClick="openHint(0,0);"><img src="images/help.gif" /></div>

    <div id="hintofPM" style="display:none;">
        <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
        <thead>
            <tr>
                <td>
                    <div id="helpname" class="AiHintTitle"></div>
                    <a href="javascript:;" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
                </td>
            </tr>
        </thead>

            <tr>
                <td valign="top" >
                    <div class="hint_body2" id="hint_body"></div>
                    <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
                </td>
            </tr>
        </table>
    </div>
</div>
<!--==============Ending of hint content=============-->

<div id="footer"></div>

</body>
</html>
