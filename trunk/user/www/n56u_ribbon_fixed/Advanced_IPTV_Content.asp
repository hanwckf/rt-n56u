<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_4#></title>
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
        $j('#mr_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#mr_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#mr_enable_x_1").attr("checked", "checked");
                $j("#mr_enable_x_0").removeAttr("checked");
                on_click_mroute();
            },
            onClickOff: function(){
                $j("#mr_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#mr_enable_x_0").attr("checked", "checked");
                $j("#mr_enable_x_1").removeAttr("checked");
                on_click_mroute();
            }
        });
        $j("#mr_enable_x_on_of label.itoggle").css("background-position", $j("input#mr_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#ether_igmp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ether_igmp_fake").attr("checked", "checked").attr("value", 1);
                $j("#ether_igmp_1").attr("checked", "checked");
                $j("#ether_igmp_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#ether_igmp_fake").removeAttr("checked").attr("value", 0);
                $j("#ether_igmp_0").attr("checked", "checked");
                $j("#ether_igmp_1").removeAttr("checked");
            }
        });
        $j("#ether_igmp_on_of label.itoggle").css("background-position", $j("input#ether_igmp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

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

<% login_state_hook(); %>

var lan_ipaddr = '<% nvram_get_x("", "lan_ipaddr_t"); %>';

function initial(){
	show_banner(1);
	show_menu(5,3,4);
	show_footer();

	on_click_mroute();

	var switch_type = support_switch_type();
	if (switch_type == 1) {
		$("row_storm_ucast").style.display = "none";
		$("row_storm_mcast").style.display = "none";
		$("row_storm_bcast").style.display = "none";
	}

	if(document.form.udpxy_enable_x.value == 0)
		$("web_udpxy_link").style.display = "none";

	if(found_app_xupnpd()){
		$("row_xupnpd").style.display = "";
		if(document.form.xupnpd_enable_x.value == 0)
			$("web_xupnpd_link").style.display = "none";
		if(document.form.udpxy_enable_x.value == 0 || document.form.xupnpd_enable_x.value == 0)
			$("row_xupnpd_udpxy").style.display = "none";
		else
			$("row_xupnpd_udpxy").style.display = "";
	}
	else{
		$("row_xupnpd").style.display = "none";
		$("row_xupnpd_udpxy").style.display = "none";
	}

	if (!support_5g_radio())
		$("tbl_mcast_5ghz").style.display = "none";
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_IPTV_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
}

function validForm(){
	if(document.form.udpxy_enable_x.value != 0){
		if(!validate_range(document.form.udpxy_enable_x, 1024, 65535))
			return false;
	}
	if(found_app_xupnpd()){
		if(document.form.xupnpd_enable_x.value != 0){
			if(!validate_range(document.form.xupnpd_enable_x, 1024, 65535))
				return false;
			if (document.form.xupnpd_enable_x.value == document.form.udpxy_enable_x.value){
				document.form.xupnpd_enable_x.focus();
				alert("<#JS_duplicate#>");
				return false;
			}
		}
	}
	return true;
}

function done_validating(action){
	refreshpage();
}

function valid_udpxy(){
	if(document.form.udpxy_enable_x.value != 0)
		validate_range(document.form.udpxy_enable_x, 1024, 65535);
}

function valid_xupnpd(){
	if(found_app_xupnpd() && document.form.xupnpd_enable_x.value != 0)
		validate_range(document.form.xupnpd_enable_x, 1024, 65535);
}

function valid_muliticast(){
	if(document.form.controlrate_unknown_unicast.value != 0)
		validate_range(document.form.controlrate_unknown_unicast, 0, 1000);
	if(document.form.controlrate_unknown_multicast.value != 0)
		validate_range(document.form.controlrate_unknown_multicast, 0, 1000);
	if(document.form.controlrate_multicast.value != 0)
		validate_range(document.form.controlrate_multicast, 0, 1000);
	if(document.form.controlrate_broadcast.value != 0)
		validate_range(document.form.controlrate_broadcast, 0, 1000);
}

function on_click_mroute() {
	if (document.form.mr_enable_x[0].checked)
		$("row_ttl_fix").style.display = "";
	else
		$("row_ttl_fix").style.display = "none";
}


var window_udpxy;
var window_xupnpd;
var window_params="toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=800,height=600";

function on_udpxy_link(){
    var svc_url="http://" + lan_ipaddr + ":" + document.form.udpxy_enable_x.value + "/status";
    window_udpxy = window.open(svc_url, "udpxy", window_params);
    window_udpxy.focus();
}

function on_xupnpd_link(){
    var svc_url="http://" + lan_ipaddr + ":" + document.form.xupnpd_enable_x.value;
    window_xupnpd = window.open(svc_url, "xupnpd", window_params);
    window_xupnpd.focus();
}

</script>
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
    <input type="hidden" name="current_page" value="Advanced_IPTV_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="RouterConfig;LANHostConfig;WLANConfig11a;WLANConfig11b;">
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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_4#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#IPTV_desc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="3" style="background-color: #E3E3E3;"><#IPTVBase#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#RouterConfig_GWMulticastEnable_itemname#></th>
                                            <td colspan="2">
                                                <div class="main_itoggle">
                                                    <div id="mr_enable_x_on_of">
                                                        <input type="checkbox" id="mr_enable_x_fake" <% nvram_match_x("", "mr_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "mr_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="mr_enable_x" id="mr_enable_x_1" class="input" onclick="on_click_mroute();" <% nvram_match_x("RouterConfig", "mr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="mr_enable_x" id="mr_enable_x_0" class="input" onclick="on_click_mroute();" <% nvram_match_x("RouterConfig", "mr_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ttl_fix">
                                            <th><#IPTVTTL#></th>
                                            <td colspan="2">
                                                <select name="mr_ttl_fix" class="input">
                                                    <option value="0" <% nvram_match_x("", "mr_ttl_fix", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "mr_ttl_fix", "1", "selected"); %>><#checkbox_Yes#> (TTL+1)</option>
                                                    <option value="2" <% nvram_match_x("", "mr_ttl_fix", "2", "selected"); %>><#checkbox_Yes#> (TTL=64)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6, 6);"><#RouterConfig_IPTV_itemname#>:</a></th>
                                            <td>
                                                <input type="text" maxlength="5" class="input" size="15" name="udpxy_enable_x" value="<% nvram_get_x("LANHostConfig", "udpxy_enable_x"); %>" onkeypress="return is_number(this);" onblur="valid_udpxy();"/>
                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_udpxy_link();" id="web_udpxy_link">Web status</a>
                                            </td>
                                        </tr>
                                        <tr id="row_xupnpd">
                                            <th><#IPTVXUA#></th>
                                            <td>
                                                <input type="text" maxlength="5" class="input" size="15" name="xupnpd_enable_x" value="<% nvram_get_x("LANHostConfig", "xupnpd_enable_x"); %>" onkeypress="return is_number(this);" onblur="valid_xupnpd();"/>
                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_xupnpd_link();" id="web_xupnpd_link">Web status</a>
                                            </td>
                                        </tr>
                                        <tr id="row_xupnpd_udpxy">
                                            <th><#IPTVXExt#></th>
                                            <td colspan="2">
                                                <select name="xupnpd_udpxy" class="input">
                                                    <option value="0" <% nvram_match_x("", "xupnpd_udpxy", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "xupnpd_udpxy", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#IPTVIGMP#></th>
                                            <td colspan="2">
                                                <select name="force_igmp" class="input">
                                                    <option value="0" <% nvram_match_x("", "force_igmp", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "force_igmp", "1", "selected"); %>>IGMPv1</option>
                                                    <option value="2" <% nvram_match_x("", "force_igmp", "2", "selected"); %>>IGMPv2 (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#SwitchStorm#></th>
                                        </tr>
                                        <tr id="row_storm_ucast">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 7);"><#RouterConfig_GWMulticast_unknownUni_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_unicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_unicast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                                &nbsp;<span style="color:#888;">[0..1000]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 8);"><#RouterConfig_GWMulticast_unknownMul_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_multicast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                                &nbsp;<span style="color:#888;">[0..1000]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_storm_mcast">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 9);"><#RouterConfig_GWMulticast_Multicast_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_multicast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                                &nbsp;<span style="color:#888;">[0..1000]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_storm_bcast">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 10);"><#RouterConfig_GWMulticast_Broadcast_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_broadcast" value="<% nvram_get_x("LANHostConfig", "controlrate_broadcast"); %>" onkeypress="return is_number(this);" onblur="valid_muliticast();"/>
                                                &nbsp;<span style="color:#888;">[0..1000]</span>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IPTVMulticast#> - <#menu5_2_5#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchIgmp#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ether_igmp_on_of">
                                                        <input type="checkbox" id="ether_igmp_fake" <% nvram_match_x("", "ether_igmp", "1", "value=1 checked"); %><% nvram_match_x("", "ether_igmp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ether_igmp" id="ether_igmp_1" class="input" <% nvram_match_x("LANHostConfig", "ether_igmp", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ether_igmp" id="ether_igmp_0" class="input" <% nvram_match_x("LANHostConfig", "ether_igmp", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IPTVMulticast#> - WiFi 2.4GHz</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchIgmp#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="rt_IgmpSnEnable_on_of">
                                                        <input type="checkbox" id="rt_IgmpSnEnable_fake" <% nvram_match_x("", "rt_IgmpSnEnable", "1", "value=1 checked"); %><% nvram_match_x("", "rt_IgmpSnEnable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="rt_IgmpSnEnable" id="rt_IgmpSnEnable_1" class="input" <% nvram_match_x("", "rt_IgmpSnEnable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="rt_IgmpSnEnable" id="rt_IgmpSnEnable_0" class="input" <% nvram_match_x("", "rt_IgmpSnEnable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WLANConfig11b_MultiRateAll_itemname#></th>
                                            <td>
                                                <select name="rt_mcastrate" class="input">
                                                    <option value="0" <% nvram_match_x("", "rt_mcastrate", "0", "selected"); %>>HTMIX (1S) 15 Mbps</option>
                                                    <option value="1" <% nvram_match_x("", "rt_mcastrate", "1", "selected"); %>>HTMIX (1S) 30 Mbps</option>
                                                    <option value="2" <% nvram_match_x("", "rt_mcastrate", "2", "selected"); %>>HTMIX (1S) 45 Mbps</option>
                                                    <option value="3" <% nvram_match_x("", "rt_mcastrate", "3", "selected"); %>>HTMIX (2S) 30 Mbps</option>
                                                    <option value="4" <% nvram_match_x("", "rt_mcastrate", "4", "selected"); %>>HTMIX (2S) 60 Mbps</option>
                                                    <option value="5" <% nvram_match_x("", "rt_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                    <option value="6" <% nvram_match_x("", "rt_mcastrate", "6", "selected"); %>>OFDM 12 Mbps (*)</option>
                                                    <option value="7" <% nvram_match_x("", "rt_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
                                                    <option value="8" <% nvram_match_x("", "rt_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
                                                    <option value="9" <% nvram_match_x("", "rt_mcastrate", "9", "selected"); %>>CCK 11 Mbps</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_mcast_5ghz">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IPTVMulticast#> - WiFi 5GHz</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchIgmp#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wl_IgmpSnEnable_on_of">
                                                        <input type="checkbox" id="wl_IgmpSnEnable_fake" <% nvram_match_x("", "wl_IgmpSnEnable", "1", "value=1 checked"); %><% nvram_match_x("", "wl_IgmpSnEnable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="wl_IgmpSnEnable" id="wl_IgmpSnEnable_1" class="input" <% nvram_match_x("", "wl_IgmpSnEnable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="wl_IgmpSnEnable" id="wl_IgmpSnEnable_0" class="input" <% nvram_match_x("", "wl_IgmpSnEnable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WLANConfig11b_MultiRateAll_itemname#></th>
                                            <td>
                                                <select name="wl_mcastrate" class="input">
                                                    <option value="0" <% nvram_match_x("", "wl_mcastrate", "0", "selected"); %>>HTMIX (1S) 15 Mbps</option>
                                                    <option value="1" <% nvram_match_x("", "wl_mcastrate", "1", "selected"); %>>HTMIX (1S) 30 Mbps (*)</option>
                                                    <option value="2" <% nvram_match_x("", "wl_mcastrate", "2", "selected"); %>>HTMIX (1S) 45 Mbps</option>
                                                    <option value="3" <% nvram_match_x("", "wl_mcastrate", "3", "selected"); %>>HTMIX (2S) 30 Mbps</option>
                                                    <option value="4" <% nvram_match_x("", "wl_mcastrate", "4", "selected"); %>>HTMIX (2S) 60 Mbps</option>
                                                    <option value="5" <% nvram_match_x("", "wl_mcastrate", "5", "selected"); %>>OFDM 9 Mbps</option>
                                                    <option value="6" <% nvram_match_x("", "wl_mcastrate", "6", "selected"); %>>OFDM 12 Mbps</option>
                                                    <option value="7" <% nvram_match_x("", "wl_mcastrate", "7", "selected"); %>>OFDM 18 Mbps</option>
                                                    <option value="8" <% nvram_match_x("", "wl_mcastrate", "8", "selected"); %>>OFDM 24 Mbps</option>
                                                </select>
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
