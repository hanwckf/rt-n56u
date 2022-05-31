<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_2_1#></title>
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
	init_itoggle('lan_proto_x', on_change_lan_dhcpc);
	init_itoggle('lan_dns_x', on_change_lan_dns);
});

</script>
<script>

var old_lan_addr = "<% nvram_get_x("","lan_ipaddr"); %>";
var old_lan_mask = "<% nvram_get_x("","lan_netmask"); %>";

function initial(){
	show_banner(1);
	show_menu(5,3,1);
	show_footer();

	on_change_lan_dhcpc();
}

function on_change_lan_dhcpc(){
	var en_ip = 1;
	if(document.form.lan_proto_x[0].checked){
		en_ip = 0;
		$j('input[name="lan_dns_x"]').removeAttr('disabled');
		$("row_lan_dns_x").style.display = "";
	}else{
		$("row_lan_dns_x").style.display = "none";
		$j('input[name="lan_dns_x"]').attr('disabled','disabled');
	}
	inputCtrl(document.form.lan_ipaddr, en_ip);
	inputCtrl(document.form.lan_netmask, en_ip);
	inputCtrl(document.form.lan_gateway, en_ip);
	inputCtrl(document.form.lan_domain, en_ip);
	on_change_lan_dns();
}

function on_change_lan_dns(){
	var en_dns = 1;
	if(!document.form.lan_dns_x[0].disabled &&
		document.form.lan_dns_x[0].checked &&
		document.form.lan_proto_x[0].checked) {
		en_dns = 0;
	}
	inputCtrl(document.form.lan_dns1, en_dns);
	inputCtrl(document.form.lan_dns2, en_dns);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "Advanced_APLAN_Content.asp";
		document.form.next_page.value = "/as.asp";
		
		document.form.submit();
	}
}

function valid_LAN_IP(ip_obj,flag){
	// A : 1.0.0.0~126.255.255.255
	// B : 127.0.0.0~127.255.255.255 (forbidden)
	// C : 128.0.0.0~255.255.255.254
	var A_class_min = inet_network("1.0.0.0");
	var A_class_max = inet_network("126.255.255.255");
	var B_class_min = inet_network("127.0.0.0");
	var B_class_max = inet_network("127.255.255.255");
	var C_class_min = inet_network("128.0.0.0");
	var C_class_max = inet_network("255.255.255.255");

	var ip_num = inet_network(ip_obj.value);

	if(ip_num < 0 && (flag == "GW" || flag == "DNS"))
		return true;

	if(ip_num > A_class_min && ip_num < A_class_max)
		return true;
	else if(ip_num > B_class_min && ip_num < B_class_max)
		return false;
	else if(ip_num > C_class_min && ip_num < C_class_max)
		return true;
	return false;
}

function validForm(){
	if(document.form.lan_proto_x[0].checked){
		if (!document.form.lan_dns_x[0].checked){
			if(!validate_ipaddr_final(document.form.lan_dns1, 'lan_dns'))
				return false;
			if(!validate_ipaddr_final(document.form.lan_dns2, 'lan_dns'))
				return false;
		}
		return true;
	}

	var addr_obj = document.form.lan_ipaddr;
	var mask_obj = document.form.lan_netmask;
	var gate_obj = document.form.lan_gateway;
	var addr_num = inet_network(addr_obj.value);

	if(!validate_ipaddr_final(addr_obj, 'lan_ipaddr') ||
			!validate_ipaddr_final(mask_obj, 'lan_netmask') ||
			!validate_ipaddr_final(gate_obj, 'lan_gateway'))
		return false;

	if(!valid_LAN_IP(addr_obj, '')) {
		alert(addr_obj.value+" <#JS_validip#>");
		addr_obj.focus();
		addr_obj.select();
		return false;
	}

	var snet_min = get_subnet_num(addr_obj.value, mask_obj.value, 0);
	var snet_max = get_subnet_num(addr_obj.value, mask_obj.value, 1);

	if(addr_num == snet_min || addr_num == snet_max){
		alert(addr_obj.value+"/"+mask_obj.value+" <#JS_validip#>");
		addr_obj.focus();
		addr_obj.select();
		return false;
	}

	if(!valid_LAN_IP(gate_obj, 'GW')) {
		alert(gate_obj.value+" <#JS_validip#>");
		gate_obj.focus();
		gate_obj.select();
		return false;
	}

	if(gate_obj.value == addr_obj.value){
		alert(gate_obj.value+" <#JS_validip#>");
		gate_obj.select();
		gate_obj.focus();
		return false;
	}

	var re = new RegExp('^(?=[a-z0-9])[a-z0-9\-\.]*[a-z0-9]$','gi');
	var o_dom = document.form.lan_domain;
	if((o_dom.value != "") && (!re.test(o_dom.value))){
		alert("<#JS_validchar#>");
		o_dom.focus();
		o_dom.select();
		return false;
	}

	if(!validate_ipaddr_final(document.form.lan_dns1, 'lan_dns'))
		return false;
	if(!validate_ipaddr_final(document.form.lan_dns2, 'lan_dns'))
		return false;

	if(addr_obj.value != old_lan_addr || mask_obj.value != old_lan_mask){
		var o_min = document.form.dhcp_start;
		var o_max = document.form.dhcp_end;
		if(!matchSubnet(o_min.value, addr_obj.value, mask_obj.value) ||
				!matchSubnet(o_max.value, addr_obj.value, mask_obj.value) ||
				inet_network(o_min.value) <= snet_min ||
				inet_network(o_max.value) >= snet_max) {
			var snet_pool = snet_max-snet_min;
			o_min.value = num2ip4(snet_min+2);
			if (snet_pool > 30)
				o_max.value=num2ip4(snet_max-11);
			else
				o_max.value=num2ip4(snet_max-1);
		}
	}

	if(addr_obj.value != old_lan_addr)
		alert("<#LANHostConfig_lanipaddr_changed_hint#>");

	return true;
}

function done_validating(action){
	refreshpage();
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

    <input type="hidden" name="current_page" value="Advanced_APLAN_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="dhcp_start" value="<% nvram_get_x("", "dhcp_start"); %>">
    <input type="hidden" name="dhcp_end" value="<% nvram_get_x("", "dhcp_end"); %>">

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
                            <h2 class="box_head round_top"><#LANHostConfig_display1_sectionname#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#LANHostConfig_display2_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#LANHostConfig_x_LANDHCPClient_itemname#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="lan_proto_x_on_of">
                                                        <input type="checkbox" id="lan_proto_x_fake" <% nvram_match_x("", "lan_proto_x", "1", "value=1 checked"); %><% nvram_match_x("", "lan_proto_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" id="lan_proto_x_1" name="lan_proto_x" value="1" class="input" onClick="on_change_lan_dhcp();" <% nvram_match_x("","lan_proto_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" id="lan_proto_x_0" name="lan_proto_x" value="0" class="input" onClick="on_change_lan_dhcp();" <% nvram_match_x("","lan_proto_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,4,1);"><#LANHostConfig_IPRouters_itemname#></a></th>
                                            <td>
                                                <input type="text" id="lan_ipaddr" name="lan_ipaddr" value="<% nvram_get_x("", "lan_ipaddr"); %>" maxlength="15" class="input" size="15" onKeyPress="return is_ipaddr(this,event);" />
                                                &nbsp;<span style="color:#888;">192.168.1.1</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,4,2);"><#LANHostConfig_SubnetMask_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="lan_netmask" value="<% nvram_get_x("", "lan_netmask"); %>" onkeypress="return is_ipaddr(this,event);" />
                                                &nbsp;<span style="color:#888;">255.255.255.0</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,4,3);"><#LANHostConfig_x_Gateway_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="lan_gateway" value="<% nvram_get_x("", "lan_gateway"); %>" onkeypress="return is_ipaddr(this,event);" />
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_DomainName_itemname#></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="lan_domain" value="<% nvram_get_x("", "lan_domain"); %>">
                                            </td>
                                        </tr>
                                        <tr id="row_lan_dns_x">
                                            <th><#IPConnection_x_DNSServerEnable_itemname#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="lan_dns_x_on_of">
                                                        <input type="checkbox" id="lan_dns_x_fake" <% nvram_match_x("", "lan_dns_x", "1", "value=1 checked"); %><% nvram_match_x("", "lan_dns_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" id="lan_dns_x_1" name="lan_dns_x" value="1" class="input" onClick="on_change_lan_dns();" <% nvram_match_x("","lan_dns_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" id="lan_dns_x_0" name="lan_dns_x" value="0" class="input" onClick="on_change_lan_dns();" <% nvram_match_x("","lan_dns_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_LDNSServer1_itemname#> 1:</th>
                                            <td>
                                                <input type="text" name="lan_dns1" value="<% nvram_get_x("", "lan_dns1"); %>" maxlength="15" class="input" size="15" onkeypress="return is_ipaddr(this,event);" />
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LANHostConfig_x_LDNSServer1_itemname#> 2:</th>
                                            <td>
                                                <input type="text" name="lan_dns2" value="<% nvram_get_x("", "lan_dns2"); %>" maxlength="15" class="input" size="15" onkeypress="return is_ipaddr(this,event);" />
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

    <div id="footer"></div>
</div>
</body>
</html>
