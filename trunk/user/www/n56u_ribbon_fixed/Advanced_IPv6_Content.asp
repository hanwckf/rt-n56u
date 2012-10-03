<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Wireless Router <#Web_Title#> - IPv6</title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>

<script>
    var $j = jQuery.noConflict();

    $j(document).ready(function() {
        $j('#ip6_wan_dhcp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ip6_wan_dhcp_fake").attr("checked", "checked").attr("value", 1);
                $j("#ip6_wan_dhcp_1").attr("checked", "checked");
                $j("#ip6_wan_dhcp_0").removeAttr("checked");
                change_ip6_wan_dhcp(1);
            },
            onClickOff: function(){
                $j("#ip6_wan_dhcp_fake").removeAttr("checked").attr("value", 0);
                $j("#ip6_wan_dhcp_0").attr("checked", "checked");
                $j("#ip6_wan_dhcp_1").removeAttr("checked");
                change_ip6_wan_dhcp(1);
            }
        });
        $j("#ip6_wan_dhcp_on_of label.itoggle").css("background-position", $j("input#ip6_wan_dhcp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#ip6_dns_auto_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ip6_dns_auto_fake").attr("checked", "checked").attr("value", 1);
                $j("#ip6_dns_auto_1").attr("checked", "checked");
                $j("#ip6_dns_auto_0").removeAttr("checked");
                change_ip6_dns_auto(1);
            },
            onClickOff: function(){
                $j("#ip6_dns_auto_fake").removeAttr("checked").attr("value", 0);
                $j("#ip6_dns_auto_0").attr("checked", "checked");
                $j("#ip6_dns_auto_1").removeAttr("checked");
                change_ip6_dns_auto(1);
            }
        });
        $j("#ip6_dns_auto_on_of label.itoggle").css("background-position", $j("input#ip6_dns_auto_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#ip6_lan_auto_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ip6_lan_auto_fake").attr("checked", "checked").attr("value", 1);
                $j("#ip6_lan_auto_1").attr("checked", "checked");
                $j("#ip6_lan_auto_0").removeAttr("checked");
                change_ip6_lan_auto(1);
            },
            onClickOff: function(){
                $j("#ip6_lan_auto_fake").removeAttr("checked").attr("value", 0);
                $j("#ip6_lan_auto_0").attr("checked", "checked");
                $j("#ip6_lan_auto_1").removeAttr("checked");
                change_ip6_lan_auto(1);
            }
        });
        $j("#ip6_lan_auto_on_of label.itoggle").css("background-position", $j("input#ip6_lan_auto_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#ip6_lan_radv_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ip6_lan_radv_fake").attr("checked", "checked").attr("value", 1);
                $j("#ip6_lan_radv_1").attr("checked", "checked");
                $j("#ip6_lan_radv_0").removeAttr("checked");
                change_ip6_lan_radv();
            },
            onClickOff: function(){
                $j("#ip6_lan_radv_fake").removeAttr("checked").attr("value", 0);
                $j("#ip6_lan_radv_0").attr("checked", "checked");
                $j("#ip6_lan_radv_1").removeAttr("checked");
                change_ip6_lan_radv();
            }
        });
        $j("#ip6_lan_radv_on_of label.itoggle").css("background-position", $j("input#ip6_lan_radv_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#ip6_lan_dhcp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#ip6_lan_dhcp_fake").attr("checked", "checked").attr("value", 1);
                $j("#ip6_lan_dhcp_1").attr("checked", "checked");
                $j("#ip6_lan_dhcp_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#ip6_lan_dhcp_fake").removeAttr("checked").attr("value", 0);
                $j("#ip6_lan_dhcp_0").attr("checked", "checked");
                $j("#ip6_lan_dhcp_1").removeAttr("checked");
            }
        });
        $j("#ip6_lan_dhcp_on_of label.itoggle").css("background-position", $j("input#ip6_lan_dhcp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });

</script>

<script>

<% login_state_hook(); %>

<% kernel_caps_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5, 5, 0);
	show_footer();

	if(!support_ipv6()){
		$('hint_no_ipv6').style.display="";
		$('tbl_ip6_con').style.display="none";
		$('tbl_apply').style.display="none";
	}
	else {
		change_ip6_service();
		change_ip6_lan_radv();
	}
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.current_page.value = "/Advanced_IPv6_Content.asp"
		document.form.next_page.value = "";
		document.form.action_mode.value = " Apply ";
		document.form.submit();
	}
}

function validate_not_empty(o) {
	if (o.value.length == 0) {
		alert("<#JS_fieldblank#>");
		o.focus();
		o.select();
		return false;
	}
	return true;
}


function validForm(){
	var ip6_con = document.form.ip6_service.value;
	if (ip6_con=="static" || ip6_con=="6in4") {
		if (!validate_not_empty(document.form.ip6_wan_addr))
			return false;
		if (!validate_not_empty(document.form.ip6_wan_gate))
			return false;
		if (!validate_not_empty(document.form.ip6_lan_addr))
			return false;
	}

	if (ip6_con=="dhcp6") {
		if (!document.form.ip6_lan_auto[0].checked && !validate_not_empty(document.form.ip6_lan_addr))
			return false;
	}

	if (ip6_con=="6in4") {
		if (!validate_not_empty(document.form.ip6_6in4_remote))
			return false;
	}

	if (ip6_con=="6to4") {
		if (!validate_not_empty(document.form.ip6_6to4_relay))
			return false;
	}

	if (ip6_con=="6rd" && !document.form.ip6_wan_dhcp[0].checked) {
		if (!validate_not_empty(document.form.ip6_6rd_relay))
			return false;
	}
	
	return true;
}

function done_validating(action){
	refreshpage();
}

function validate_ip6addr(o){
	// thanks http://www.intermapper.com
	var regex = /^((([0-9a-f]{1,4}:){7}([0-9a-f]{1,4}|:))|(([0-9a-f]{1,4}:){6}(:[0-9a-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9a-f]{1,4}:){5}(((:[0-9a-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9a-f]{1,4}:){4}(((:[0-9a-f]{1,4}){1,3})|((:[0-9a-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9a-f]{1,4}:){3}(((:[0-9a-f]{1,4}){1,4})|((:[0-9a-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9a-f]{1,4}:){2}(((:[0-9a-f]{1,4}){1,5})|((:[0-9a-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9a-f]{1,4}:){1}(((:[0-9a-f]{1,4}){1,6})|((:[0-9a-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9a-f]{1,4}){1,7})|((:[0-9a-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?$/i;

	if (o.value.length===0)
	{
		return true;
	}

	if (!regex.test(o.value))
	{
	
		alert(o.value + " - " + "<#IP6_hint_addr#>");
		o.focus();
		return false;
	}

	return true;
}

function change_ip6_service(){
	var ip6_con = document.form.ip6_service.value;
	var wan_proto = document.form.wan_proto.value;
	var wif = 'IPoE: <#BOP_ctype_title5#>';
	var ppp = (wan_proto == "pppoe" || wan_proto == "pptp" || wan_proto == "l2tp") ? true : false;
	var warn = false;
	var pppif = false;
	var ip6on = true;
	
	if (ip6_con=="static") {
		$('tbl_ip6_sit').style.display="none";
		$('row_ip6_wan_dhcp').style.display="none";
		$('row_ip6_dns_auto').style.display="none";
		$('row_ip6_lan_auto').style.display="none";
		inputRCtrl2(document.form.ip6_wan_dhcp, 1);
		inputRCtrl2(document.form.ip6_dns_auto, 1);
		inputRCtrl2(document.form.ip6_lan_auto, 1);
		$j('#ip6_wan_dhcp_on_of').iState(0);
		$j('#ip6_dns_auto_on_of').iState(0);
		$j('#ip6_lan_auto_on_of').iState(0);
		if (ppp) pppif = true;
	}
	else if (ip6_con=="dhcp6") {
		$('lbl_ip6_wan_dhcp').innerHTML="<#IP6_WAN_DHCP#>";
		$('tbl_ip6_sit').style.display="none";
		$('row_ip6_wan_dhcp').style.display="";
		$('row_ip6_dns_auto').style.display="";
		$('row_ip6_lan_auto').style.display="";
		if (ppp) pppif = true;
	}
	else if (ip6_con=="6in4") {
		$('tbl_ip6_sit').style.display="";
		$('row_ip6_wan_dhcp').style.display="none";
		$('row_ip6_dns_auto').style.display="none";
		$('row_ip6_lan_auto').style.display="none";
		$('row_ip6_6in4_remote').style.display="";
		$('row_ip6_6to4_relay').style.display="none";
		$('row_ip6_6rd_relay').style.display="none";
		$('row_ip6_6rd_size').style.display="none";
		inputRCtrl2(document.form.ip6_wan_dhcp, 1);
		inputRCtrl2(document.form.ip6_dns_auto, 1);
		inputRCtrl2(document.form.ip6_lan_auto, 1);
		$j('#ip6_wan_dhcp_on_of').iState(0);
		$j('#ip6_dns_auto_on_of').iState(0);
		$j('#ip6_lan_auto_on_of').iState(0);
	}
	else if (ip6_con=="6to4") {
		$('tbl_ip6_sit').style.display="";
		$('row_ip6_wan_dhcp').style.display="none";
		$('row_ip6_dns_auto').style.display="none";
		$('row_ip6_lan_auto').style.display="none";
		$('row_ip6_6in4_remote').style.display="none";
		$('row_ip6_6to4_relay').style.display="";
		$('row_ip6_6rd_relay').style.display="none";
		$('row_ip6_6rd_size').style.display="none";
		inputRCtrl2(document.form.ip6_wan_dhcp, 0);
		inputRCtrl2(document.form.ip6_dns_auto, 1);
		inputRCtrl2(document.form.ip6_lan_auto, 0);
		$j('#ip6_wan_dhcp_on_of').iState(1);
		$j('#ip6_dns_auto_on_of').iState(0);
		$j('#ip6_lan_auto_on_of').iState(1);
	}
	else if (ip6_con=="6rd") {
		$('lbl_ip6_wan_dhcp').innerHTML="<#IP6_WAN_Auto#>";
		$('tbl_ip6_sit').style.display="";
		$('row_ip6_wan_dhcp').style.display="";
		$('row_ip6_dns_auto').style.display="none";
		$('row_ip6_lan_auto').style.display="none";
		$('row_ip6_6in4_remote').style.display="none";
		$('row_ip6_6to4_relay').style.display="none";
		$('row_ip6_6rd_relay').style.display="";
		$('row_ip6_6rd_size').style.display="";
		inputRCtrl2(document.form.ip6_dns_auto, 1);
		inputRCtrl2(document.form.ip6_lan_auto, 0);
		$j('#ip6_dns_auto_on_of').iState(0);
		$j('#ip6_lan_auto_on_of').iState(1);
		
		if (document.form.ip6_wan_dhcp[0].checked) {
			if (wan_proto != "dhcp")
				warn = true;
		}
		
		if (ppp) warn = true;
	}
	else {
		ip6on = false;
	}

	if (!ip6on) {
		$('row_wan_type').style.display="none";
		$('tbl_ip6_sit').style.display="none";
		$('tbl_ip6_wan').style.display="none";
		$('tbl_ip6_dns').style.display="none";
		$('tbl_ip6_lan').style.display="none";
	}
	else {
		$('row_wan_type').style.display="";
		$('tbl_ip6_wan').style.display="";
		$('tbl_ip6_dns').style.display="";
		$('tbl_ip6_lan').style.display="";
	}

	if (pppif && ip6on) {
		$('row_wan_if').style.display="";
	}
	else {
		$('row_wan_if').style.display="none";
	}

	if (wan_proto == "dhcp")
		wif = 'IPoE: <#BOP_ctype_title1#>';
	else if (wan_proto == "pppoe")
		wif = 'PPPoE';
	else if (wan_proto == "pptp")
		wif = 'PPTP';
	else if (wan_proto == "l2tp")
		wif = 'L2TP';

	$('wan_type').innerHTML = '<span class="label ' + (warn == false ? 'label-success' : 'label-warning') + '">' + wif + '</span>';

	change_ip6_wan_dhcp(1);
	change_ip6_dns_auto(1);
	change_ip6_lan_auto(1);
}

function change_ip6_wan_dhcp(enable){
	var ip6_con = document.form.ip6_service.value;
	var val = (!enable || document.form.ip6_wan_dhcp[0].checked) ? 0 : 1;
	var val_addr = val;
	var val_gate = val;
	if (ip6_con == "6rd") {
		val_gate = 0;
		inputCtrl(document.form.ip6_6rd_relay, val);
		inputCtrl(document.form.ip6_6rd_size, val);
	} else if (ip6_con == "dhcp6") {
		val_addr = 0;
		val_gate = 0;
	}

	inputCtrl(document.form.ip6_wan_addr, val_addr);
	inputCtrl(document.form.ip6_wan_size, val_addr);
	inputCtrl(document.form.ip6_wan_gate, val_gate);
}

function change_ip6_dns_auto(enable){
	var val = (!enable || document.form.ip6_dns_auto[0].checked) ? 0 : 1;

	inputCtrl(document.form.ip6_dns1, val);
	inputCtrl(document.form.ip6_dns2, val);
	inputCtrl(document.form.ip6_dns3, val);
}

function change_ip6_lan_auto(enable){
	var val = (!enable || document.form.ip6_lan_auto[0].checked) ? 0 : 1;

	inputCtrl(document.form.ip6_lan_addr, val);
}

function change_ip6_lan_radv(){
	if (document.form.ip6_lan_radv[0].checked) {
		$('row_ip6_lan_dhcp').style.display="";
	} else {
		$('row_ip6_lan_dhcp').style.display="none";
	}
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
    <input type="hidden" name="current_page" value="Advanced_IPv6_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="IP6Connection;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log", "productid"); %>">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("", "firmver"); %>">
    <input type="hidden" name="wan_proto" value="<% nvram_get_x("", "wan_proto"); %>" readonly="1">

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
                            <h2 class="box_head round_top"><#IP6_proto#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#IP6_desc#></div>
                                    <div id="hint_no_ipv6" class="alert alert-danger" style="display:none; margin: 10px;"><b><#IP6_hint#></b></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_ip6_con">
                                        <tr>
                                            <th width="50%"><#IP6_SVC#></th>
                                            <td align="left">
                                                <select class="input" name="ip6_service" style="width: 246px;" onchange="change_ip6_service()" >
                                                    <option value="" <% nvram_match_x("", "ip6_service", "", "selected"); %>>Disabled</option>
                                                    <option value="static" <% nvram_match_x("", "ip6_service", "static", "selected"); %>>Native Static</option>
                                                    <option value="dhcp6" <% nvram_match_x("", "ip6_service", "dhcp6", "selected"); %>>Native DHCPv6 (Stateful/Stateless)</option>
                                                    <option value="6in4" <% nvram_match_x("", "ip6_service", "6in4", "selected"); %>>Tunnel 6in4</option>
                                                    <option value="6to4" <% nvram_match_x("", "ip6_service", "6to4", "selected"); %>>Tunnel 6to4</option>
                                                    <option value="6rd" <% nvram_match_x("", "ip6_service", "6rd", "selected"); %>>Tunnel 6rd</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_if">
                                            <th><#IP6_WAN_IF#></th>
                                            <td align="left">
                                                <select class="input" name="ip6_wan_if" style="width: 123px;" >
                                                    <option value="0" <% nvram_match_x("", "ip6_wan_if", "0", "selected"); %>>WAN (ppp0)</option>
                                                    <option value="1" <% nvram_match_x("", "ip6_wan_if", "1", "selected"); %>>MAN (eth3)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_type">
                                            <th><#Layer3Forwarding_x_ConnectionType_itemname#></th>
                                            <td align="left" id="wan_type"></td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_ip6_sit" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IP6_SIT_desc#></th>
                                        </tr>
                                        <tr id="row_ip6_6in4_remote">
                                            <th width="50%"><#IP6_SIT_6in4R#></th>
                                            <td>
                                                <input type="text" size="12" maxlength="15" style="width: 103px;" name="ip6_6in4_remote" value="<% nvram_get_x("", "ip6_6in4_remote"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_ip6_6to4_relay">
                                            <th width="50%"><#IP6_SIT_6to4R#></th>
                                            <td>
                                                <input type="text" size="12" maxlength="15" style="width: 103px;" name="ip6_6to4_relay" value="<% nvram_get_x("", "ip6_6to4_relay"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_ip6_6rd_relay">
                                            <th width="50%"><#IP6_SIT_6rdR#></th>
                                            <td>
                                                <input type="text" size="12" maxlength="15" style="width: 103px;" name="ip6_6rd_relay" value="<% nvram_get_x("", "ip6_6rd_relay"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_ip6_6rd_size">
                                            <th><#IP6_SIT_6rdM#></th>
                                            <td>
                                                <input type="text" maxlength="2" style="width: 40px;" class="input" size="4" name="ip6_6rd_size" value="<% nvram_get_x("", "ip6_6rd_size"); %>" onkeypress="return is_number(this)" onblur="return validate_range(this, 0, 32)">
                                            </td>
                                        </tr>
                                        <tr id="row_ip6_sit_mtu">
                                            <th><#IP6_SIT_MTU#></th>
                                            <td>
                                                <input type="text" maxlength="4" style="width: 40px;" class="input" size="4" name="ip6_sit_mtu" value="<% nvram_get_x("", "ip6_sit_mtu"); %>" onkeypress="return is_number(this)" onblur="return validate_range(this, 1280, 1480)">
                                            </td>
                                        </tr>
                                        <tr id="row_ip6_sit_ttl">
                                            <th><#IP6_SIT_TTL#></th>
                                            <td>
                                                <input type="text" maxlength="3" style="width: 40px;" class="input" size="4" name="ip6_sit_ttl" value="<% nvram_get_x("", "ip6_sit_ttl"); %>" onkeypress="return is_number(this)" onblur="return validate_range(this, 1, 255)">
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_ip6_wan" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IP6_WAN_desc#></th>
                                        </tr>
                                        <tr id="row_ip6_wan_dhcp">
                                            <th id="lbl_ip6_wan_dhcp"><#IP6_WAN_DHCP#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ip6_wan_dhcp_on_of">
                                                        <input type="checkbox" id="ip6_wan_dhcp_fake" <% nvram_match_x("", "ip6_wan_dhcp", "1", "value=1 checked"); %><% nvram_match_x("", "ip6_wan_dhcp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="ip6_wan_dhcp" id="ip6_wan_dhcp_1" class="input" value="1" onclick="change_ip6_wan_dhcp(1);" <% nvram_match_x("", "ip6_wan_dhcp", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="ip6_wan_dhcp" id="ip6_wan_dhcp_0" class="input" value="0" onclick="change_ip6_wan_dhcp(1);" <% nvram_match_x("", "ip6_wan_dhcp", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_WAN_Addr#></th>
                                            <td>
                                                <input type="text" maxlength="40" style="width: 286px;" class="input" size="30" name="ip6_wan_addr" value="<% nvram_get_x("", "ip6_wan_addr"); %>" onkeypress="return is_string(this)" onblur="validate_ip6addr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_WAN_Pref#></th>
                                            <td>
                                                <input type="text" maxlength="3" style="width: 30px;" class="input" size="4" name="ip6_wan_size" value="<% nvram_get_x("", "ip6_wan_size"); %>" onkeypress="return is_number(this)" onblur="return validate_range(this, 1, 128)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_WAN_Gate#></th>
                                            <td>
                                                <input type="text" maxlength="40" style="width: 286px;" class="input" size="30" name="ip6_wan_gate" value="<% nvram_get_x("", "ip6_wan_gate"); %>" onkeypress="return is_string(this)" onblur="validate_ip6addr(this)">
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_ip6_dns" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IP6_DNS_desc#></th>
                                        </tr>
                                        <tr id="row_ip6_dns_auto">
                                            <th><#IP6_DNS_Auto#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ip6_dns_auto_on_of">
                                                        <input type="checkbox" id="ip6_dns_auto_fake" <% nvram_match_x("", "ip6_dns_auto", "1", "value=1 checked"); %><% nvram_match_x("", "ip6_dns_auto", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="ip6_dns_auto" id="ip6_dns_auto_1" class="input" value="1" onclick="change_ip6_dns_auto(1);" <% nvram_match_x("", "ip6_dns_auto", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="ip6_dns_auto" id="ip6_dns_auto_0" class="input" value="0" onclick="change_ip6_dns_auto(1);" <% nvram_match_x("", "ip6_dns_auto", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_DNS_Addr#>&nbsp;1:</th>
                                            <td>
                                                <input type="text" maxlength="40" style="width: 286px;" class="input" size="30" name="ip6_dns1" value="<% nvram_get_x("", "ip6_dns1"); %>" onkeypress="return is_string(this)" onblur="validate_ip6addr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_DNS_Addr#>&nbsp;2:</th>
                                            <td>
                                                <input type="text" maxlength="40" style="width: 286px;" class="input" size="30" name="ip6_dns2" value="<% nvram_get_x("", "ip6_dns2"); %>" onkeypress="return is_string(this)" onblur="validate_ip6addr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_DNS_Addr#>&nbsp;3:</th>
                                            <td>
                                                <input type="text" maxlength="40" style="width: 286px;" class="input" size="30" name="ip6_dns3" value="<% nvram_get_x("", "ip6_dns3"); %>" onkeypress="return is_string(this)" onblur="validate_ip6addr(this)">
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_ip6_lan" style="display:none;">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IP6_LAN_desc#></th>
                                        </tr>
                                        <tr id="row_ip6_lan_auto">
                                            <th><#IP6_LAN_Auto#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ip6_lan_auto_on_of">
                                                        <input type="checkbox" id="ip6_lan_auto_fake" <% nvram_match_x("", "ip6_lan_auto", "1", "value=1 checked"); %><% nvram_match_x("", "ip6_lan_auto", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="ip6_lan_auto" id="ip6_lan_auto_1" class="input" value="1" onclick="change_ip6_lan_auto(1);" <% nvram_match_x("", "ip6_lan_auto", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="ip6_lan_auto" id="ip6_lan_auto_0" class="input" value="0" onclick="change_ip6_lan_auto(1);" <% nvram_match_x("", "ip6_lan_auto", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_LAN_Addr#></th>
                                            <td>
                                                <input type="text" maxlength="40" style="width: 286px;" class="input" size="30" name="ip6_lan_addr" value="<% nvram_get_x("", "ip6_lan_addr"); %>" onkeypress="return is_string(this)" onblur="validate_ip6addr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IP6_LAN_Pref#></th>
                                            <td>
                                                <input type="text" maxlength="3" style="width: 30px;" class="input" size="4" name="ip6_lan_size" value="<% nvram_get_x("", "ip6_lan_size"); %>" onkeypress="return is_number(this)" onblur="return validate_range(this, 48, 80)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#IP6_LAN_RAdv#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ip6_lan_radv_on_of">
                                                        <input type="checkbox" id="ip6_lan_radv_fake" <% nvram_match_x("", "ip6_lan_radv", "1", "value=1 checked"); %><% nvram_match_x("", "ip6_lan_radv", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="ip6_lan_radv" id="ip6_lan_radv_1" class="input" value="1" onclick="change_ip6_lan_radv();" <% nvram_match_x("", "ip6_lan_radv", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="ip6_lan_radv" id="ip6_lan_radv_0" class="input" value="0" onclick="change_ip6_lan_radv();" <% nvram_match_x("", "ip6_lan_radv", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_ip6_lan_dhcp">
                                            <th><#IP6_LAN_DHCP#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="ip6_lan_dhcp_on_of">
                                                        <input type="checkbox" id="ip6_lan_dhcp_fake" <% nvram_match_x("", "ip6_lan_dhcp", "1", "value=1 checked"); %><% nvram_match_x("", "ip6_lan_dhcp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="ip6_lan_dhcp" id="ip6_lan_dhcp_1" class="input" value="1" onclick="" <% nvram_match_x("", "ip6_lan_dhcp", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="ip6_lan_dhcp" id="ip6_lan_dhcp_0" class="input" value="0" onclick="" <% nvram_match_x("", "ip6_lan_dhcp", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table" id="tbl_apply">
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
</div>
</body>
</html>
