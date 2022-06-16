<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_2_4#></title>
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
	init_itoggle('mr_enable_x', on_click_mroute);
	init_itoggle('ether_igmp', on_click_snoop);
});

</script>
<script>

var lan_ipaddr = '<% nvram_get_x("", "lan_ipaddr_t"); %>';

function initial(){
	var id_menu = 4;
	if(get_ap_mode()){
		id_menu = 3;
		if (lan_proto == '1')
			id_menu--;
		showhide_div('row_mroute', 0);
	}else{
		on_click_mroute();
	}

	show_banner(1);
	show_menu(5,3,id_menu);
	show_footer();

	var o1 = document.form.ether_uport;
	var num_ephy = support_num_ephy();
	if (!support_2g_inic_mii())
		o1.remove(9);
	if (num_ephy < 8)
		o1.remove(8);
	if (num_ephy < 7)
		o1.remove(7);
	if (num_ephy < 6)
		o1.remove(6);
	if (num_ephy < 5)
		o1.remove(5);
	if (num_ephy < 4)
		o1.remove(4);
	if (num_ephy < 3)
		o1.remove(3);

	var switch_type = support_switch_type();
	if (switch_type > 1) {
		showhide_div('row_storm_ucast', 0);
		showhide_div('row_storm_mcast_unk', 0);
		showhide_div('row_storm_mcast', 0);
	}

	if (switch_type >= 10)
		$("lbl_bcast").innerHTML = "[0..100]";

	if(document.form.udpxy_enable_x.value == 0)
		$("web_udpxy_link").style.display = "none";

	if(found_app_xupnpd()){
		showhide_div('row_xupnpd', 1);
		if(document.form.xupnpd_enable_x.value == 0)
			$("web_xupnpd_link").style.display = "none";
		if(document.form.udpxy_enable_x.value == 0 || document.form.xupnpd_enable_x.value == 0)
			showhide_div('row_xupnpd_udpxy', 0);
		else
			showhide_div('row_xupnpd_udpxy', 1);
	}else{
		showhide_div('row_xupnpd', 0);
	}

	on_click_snoop();
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

	var switch_type = support_switch_type();
	if(document.form.controlrate_unknown_unicast.value != 0 && switch_type < 2){
		if(!validate_range(document.form.controlrate_unknown_unicast, 0, 1000))
			return false;
	}
	if(document.form.controlrate_unknown_multicast.value != 0 && switch_type < 2){
		if(!validate_range(document.form.controlrate_unknown_multicast, 0, 1000))
			return false;
	}
	if(document.form.controlrate_multicast.value != 0 && switch_type < 2){
		if(!validate_range(document.form.controlrate_multicast, 0, 1000))
			return false;
	}
	if(document.form.controlrate_broadcast.value != 0){
		var max_rate = (switch_type >= 10) ? 100 : 1000;
		if(!validate_range(document.form.controlrate_broadcast, 0, max_rate))
			return false;
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

function on_click_mroute(){
	var v = document.form.mr_enable_x[0].checked;
	showhide_div('row_qleave', v);
}

function on_click_snoop(){
	var v = document.form.ether_igmp[0].checked;
	showhide_div('row_igmp_uport', v && allow_uport());
	showhide_div('row_m2u_wire', v);
	showhide_div('row_m2u_2ghz', v);
	showhide_div('row_m2u_5ghz', v && support_5g_radio());
}

function on_change_m2u_wire(){
	var v = document.form.ether_igmp[0].checked;
	showhide_div('row_igmp_uport', v && allow_uport());
}

function allow_uport(){
	return (document.form.ether_m2u.value == "2" && get_ap_mode()) ? 1 : 0;
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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_4#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#IPTV_desc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IPTVBase#></th>
                                        </tr>
                                        <tr id="row_mroute">
                                            <th><#RouterConfig_GWMulticastEnable_itemname#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="mr_enable_x_on_of">
                                                        <input type="checkbox" id="mr_enable_x_fake" <% nvram_match_x("", "mr_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "mr_enable_x", "0", "value=0"); %> />
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="mr_enable_x" id="mr_enable_x_1" class="input" onclick="on_click_mroute();" <% nvram_match_x("", "mr_enable_x", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="mr_enable_x" id="mr_enable_x_0" class="input" onclick="on_click_mroute();" <% nvram_match_x("", "mr_enable_x", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_qleave" style="display:none;">
                                            <th><#IPTVQLeave#></th>
                                            <td>
                                                <select name="mr_qleave_x" class="input">
                                                    <option value="0" <% nvram_match_x("", "mr_qleave_x", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "mr_qleave_x", "1", "selected"); %>><#checkbox_Yes#> (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IPTVIGMP#></th>
                                            <td>
                                                <select name="force_igmp" class="input">
                                                    <option value="0" <% nvram_match_x("", "force_igmp", "0", "selected"); %>><#checkbox_No#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "force_igmp", "1", "selected"); %>>IGMPv1</option>
                                                    <option value="2" <% nvram_match_x("", "force_igmp", "2", "selected"); %>>IGMPv2</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="3" style="background-color: #E3E3E3;"><#IPTVProxy#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6, 6);"><#RouterConfig_IPTV_itemname#>:</a></th>
                                            <td>
                                                <input type="text" maxlength="5" class="input" size="15" name="udpxy_enable_x" value="<% nvram_get_x("", "udpxy_enable_x"); %>" onkeypress="return is_number(this,event);" onblur="valid_udpxy();"/>
                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_udpxy_link();" id="web_udpxy_link">Web status</a>
                                            </td>
                                        </tr>
                                        <tr id="row_xupnpd">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6, 6);"><#IPTVXUA#></a></th>
                                            <td>
                                                <input type="text" maxlength="5" class="input" size="15" name="xupnpd_enable_x" value="<% nvram_get_x("", "xupnpd_enable_x"); %>" onkeypress="return is_number(this,event);" onblur="valid_xupnpd();"/>
                                            </td>
                                            <td width="15%">
                                                <a href="javascript:on_xupnpd_link();" id="web_xupnpd_link">Web status</a>
                                            </td>
                                        </tr>
                                        <tr id="row_xupnpd_udpxy" style="display:none;">
                                            <th><#IPTVXExt#></th>
                                            <td colspan="2">
                                                <select name="xupnpd_udpxy" class="input">
                                                    <option value="0" <% nvram_match_x("", "xupnpd_udpxy", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "xupnpd_udpxy", "1", "selected"); %>><#checkbox_Yes#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IPTVMulticast#> - IGMP/MLD Snooping</th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#SwitchIgmp#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ether_igmp_on_of">
                                                        <input type="checkbox" id="ether_igmp_fake" <% nvram_match_x("", "ether_igmp", "1", "value=1 checked"); %><% nvram_match_x("", "ether_igmp", "0", "value=0"); %> />
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ether_igmp" id="ether_igmp_1" class="input" onclick="on_click_snoop();" <% nvram_match_x("", "ether_igmp", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ether_igmp" id="ether_igmp_0" class="input" onclick="on_click_snoop();" <% nvram_match_x("", "ether_igmp", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_m2u_wire">
                                            <th>M2U - <#menu5_2_5#>:</th>
                                            <td>
                                                <select name="ether_m2u" class="input" onchange="on_change_m2u_wire();">
                                                    <option value="0" <% nvram_match_x("", "ether_m2u", "0", "selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("", "ether_m2u", "1", "selected"); %>>Multicast to Unicast</option>
                                                    <option value="2" <% nvram_match_x("", "ether_m2u", "2", "selected"); %>>HW IGMP/MLD snooping (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_igmp_uport">
                                            <th><#SwitchUport#></th>
                                            <td>
                                                <select name="ether_uport" class="input">
                                                    <option value="-1" <% nvram_match_x("", "ether_uport", "-1", "selected"); %>><#checkbox_No#></option>
                                                    <option value="0" <% nvram_match_x("", "ether_uport", "0", "selected"); %>>WAN (*)</option>
                                                    <option value="1" <% nvram_match_x("", "ether_uport", "1", "selected"); %>>LAN1</option>
                                                    <option value="2" <% nvram_match_x("", "ether_uport", "2", "selected"); %>>LAN2</option>
                                                    <option value="3" <% nvram_match_x("", "ether_uport", "3", "selected"); %>>LAN3</option>
                                                    <option value="4" <% nvram_match_x("", "ether_uport", "4", "selected"); %>>LAN4</option>
                                                    <option value="5" <% nvram_match_x("", "ether_uport", "5", "selected"); %>>LAN5</option>
                                                    <option value="6" <% nvram_match_x("", "ether_uport", "6", "selected"); %>>LAN6</option>
                                                    <option value="7" <% nvram_match_x("", "ether_uport", "7", "selected"); %>>LAN7</option>
                                                    <option value="13" <% nvram_match_x("", "ether_uport", "13", "selected"); %>>iNIC (2.4GHz)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_m2u_2ghz">
                                            <th>M2U - <#menu5_11#>:</th>
                                            <td>
                                                <select name="rt_IgmpSnEnable" class="input">
                                                    <option value="0" <% nvram_match_x("", "rt_IgmpSnEnable", "0", "selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("", "rt_IgmpSnEnable", "1", "selected"); %>>Multicast to Unicast (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_m2u_5ghz">
                                            <th>M2U - <#menu5_12#>:</th>
                                            <td>
                                                <select name="wl_IgmpSnEnable" class="input">
                                                    <option value="0" <% nvram_match_x("", "wl_IgmpSnEnable", "0", "selected"); %>><#btn_Disable#></option>
                                                    <option value="1" <% nvram_match_x("", "wl_IgmpSnEnable", "1", "selected"); %>>Multicast to Unicast (*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#SwitchStorm#></th>
                                        </tr>
                                        <tr id="row_storm_ucast">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 7);"><#RouterConfig_GWMulticast_unknownUni_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_unicast" value="<% nvram_get_x("", "controlrate_unknown_unicast"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[0..1000]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_storm_mcast_unk">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 8);"><#RouterConfig_GWMulticast_unknownMul_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_multicast" value="<% nvram_get_x("", "controlrate_unknown_multicast"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;"></span>
                                            </td>
                                        </tr>
                                        <tr id="row_storm_mcast">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 9);"><#RouterConfig_GWMulticast_Multicast_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_multicast" value="<% nvram_get_x("", "controlrate_multicast"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[0..1000]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 6, 10);"><#RouterConfig_GWMulticast_Broadcast_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" class="input" size="15" name="controlrate_broadcast" value="<% nvram_get_x("", "controlrate_broadcast"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span id="lbl_bcast" style="color:#888;">[0..1000]</span>
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
