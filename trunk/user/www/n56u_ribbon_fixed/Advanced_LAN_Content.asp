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
	init_itoggle('lan_stp');
});

</script>
<script>

var old_lan_addr = "<% nvram_get_x("","lan_ipaddr"); %>";
var old_lan_mask = "<% nvram_get_x("","lan_netmask"); %>";

function initial(){
	show_banner(1);
	show_menu(5,3,1);
	show_footer();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "Advanced_LAN_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function valid_LAN_IP(ip_obj){
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

	if(ip_num > A_class_min && ip_num < A_class_max)
		return true;
	else if(ip_num > B_class_min && ip_num < B_class_max)
		return false;
	else if(ip_num > C_class_min && ip_num < C_class_max)
		return true;
	return false;
}

function validForm(){
	var addr_obj = document.form.lan_ipaddr;
	var mask_obj = document.form.lan_netmask;
	var addr_num = inet_network(addr_obj.value);

	if(!validate_ipaddr_final(addr_obj, 'lan_ipaddr') ||
			!validate_ipaddr_final(mask_obj, 'lan_netmask'))
		return false;

	if(!valid_LAN_IP(addr_obj)) {
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

	var wan_addr = document.form.wan_ipaddr.value;
	var wan_mask = document.form.wan_netmask.value;

	if(wan_addr != "0.0.0.0" && wan_addr != "" && wan_mask != "0.0.0.0" && wan_mask != ""){
		if(matchSubnet2(wan_addr, wan_mask, addr_obj.value, mask_obj.value)){
			alert("<#JS_validsubnet#>");
			mask_obj.focus();
			mask_obj.select();
			return false;
		}
	}

	if(addr_obj.value != old_lan_addr || mask_obj.value != old_lan_mask){
		var o_min = document.form.dhcp_start;
		var o_max = document.form.dhcp_end;
		if(!matchSubnet(o_min.value, addr_obj.value, mask_obj.value) ||
				!matchSubnet(o_max.value, addr_obj.value, mask_obj.value) ||
				inet_network(o_min.value) <= snet_min ||
				inet_network(o_max.value) >= snet_max) {
			if(confirm("<#JS_DHCP1#>")){
				var snet_pool = snet_max-snet_min;
				o_min.value = num2ip4(snet_min+2);
				if (snet_pool > 30)
					o_max.value=num2ip4(snet_max-11);
				else
					o_max.value=num2ip4(snet_max-1);
			}else{
				mask_obj.focus();
				mask_obj.select();
				return false;
			}
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

    <input type="hidden" name="current_page" value="Advanced_LAN_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="wan_ipaddr" value="<% nvram_get_x("", "wan0_ipaddr"); %>" readonly="1">
    <input type="hidden" name="wan_netmask" value="<% nvram_get_x("", "wan0_netmask"); %>" readonly="1">
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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_1#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#LANHostConfig_display1_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 4, 1);"><#LANHostConfig_IPRouters_itemname#></a></th>
                                            <td style="border-top: 0 none;">
                                                <input type="text" maxlength="15" class="input" size="15" id="lan_ipaddr" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>" onKeyPress="return is_ipaddr(this,event);" />
                                                &nbsp;<span style="color:#888;">192.168.1.1</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 4,2);"><#LANHostConfig_SubnetMask_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="lan_netmask" value="<% nvram_get_x("LANHostConfig","lan_netmask"); %>" onkeypress="return is_ipaddr(this,event);" />
                                                &nbsp;<span style="color:#888;">255.255.255.0</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#LAN_STP#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="lan_stp_on_of">
                                                        <input type="checkbox" id="lan_stp_fake" <% nvram_match_x("", "lan_stp", "1", "value=1 checked"); %><% nvram_match_x("", "lan_stp", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="lan_stp" id="lan_stp_1" <% nvram_match_x("", "lan_stp", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="lan_stp" id="lan_stp_0" <% nvram_match_x("", "lan_stp", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2" style="border-top: 0 none;">
                                                <br />
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
