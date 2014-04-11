<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_1#></title>
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
        $j('#lan_stp_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#lan_stp_fake").attr("checked", "checked").attr("value", 1);
                $j("#lan_stp_1").attr("checked", "checked");
                $j("#lan_stp_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#lan_stp_fake").removeAttr("checked").attr("value", 0);
                $j("#lan_stp_0").attr("checked", "checked");
                $j("#lan_stp_1").removeAttr("checked");
            }
        });
        $j("#lan_stp_on_of label.itoggle").css("background-position", $j("input#lan_stp_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    })
</script>

<script>

<% login_state_hook(); %>
var old_lan_ipaddr = "<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>";

function initial(){
	final_flag = 1;	// for the function in general.js
	
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

// test if WAN IP & Gateway & DNS IP is a valid IP
// DNS IP allows to input nothing
function valid_IP(obj_name, obj_flag){
		// A : 1.0.0.0~126.255.255.255
		// B : 127.0.0.0~127.255.255.255 (forbidden)
		// C : 128.0.0.0~255.255.255.254
		var A_class_start = inet_network("1.0.0.0");
		var A_class_end = inet_network("126.255.255.255");
		var B_class_start = inet_network("127.0.0.0");
		var B_class_end = inet_network("127.255.255.255");
		var C_class_start = inet_network("128.0.0.0");
		var C_class_end = inet_network("255.255.255.255");
		
		var ip_obj = obj_name;
		var ip_num = inet_network(ip_obj.value);

		if(obj_flag == "DNS" && ip_num == -1){ //DNS allows to input nothing
			return true;
		}
		
		if(obj_flag == "GW" && ip_num == -1){ //GW allows to input nothing
			return true;
		}
		
		if(ip_num > A_class_start && ip_num < A_class_end)
			return true;
		else if(ip_num > B_class_start && ip_num < B_class_end){
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end)
			return true;
		else{
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
}

function validForm(){
	var ip_obj = document.form.lan_ipaddr;
	var ip_num = inet_network(ip_obj.value);
	var ip_class = "";		
	if(!valid_IP(ip_obj, "")) return false;

	// test if netmask is valid.
	var netmask_obj = document.form.lan_netmask;
	var netmask_num = inet_network(netmask_obj.value);
	var netmask_reverse_num = ~netmask_num;
	var default_netmask = "";
	var wrong_netmask = 0;

	if(netmask_num < 0) wrong_netmask = 1;

	if(ip_class == 'A')
		default_netmask = "255.0.0.0";
	else if(ip_class == 'B')
		default_netmask = "255.255.0.0";
	else
		default_netmask = "255.255.255.0";
	
	var test_num = netmask_reverse_num;
	while(test_num != 0){
		if((test_num+1)%2 == 0)
			test_num = (test_num+1)/2-1;
		else{
			wrong_netmask = 1;
			break;
		}
	}
	if(wrong_netmask == 1){
		alert(netmask_obj.value+" <#JS_validip#>");
		netmask_obj.value = default_netmask;
		netmask_obj.focus();
		netmask_obj.select();
		return false;
	}
	
	var subnet_head = getSubnet(ip_obj.value, netmask_obj.value, "head");
	var subnet_end = getSubnet(ip_obj.value, netmask_obj.value, "end");
	
	if(ip_num == subnet_head || ip_num == subnet_end){
		alert(ip_obj.value+" <#JS_validip#>");
		ip_obj.focus();
		ip_obj.select();
		return false;
	}
	
	// check IP changed or not
	// No matter it changes or not, it will submit the form
	if(sw_mode == "1"){
		var pool_change = changed_DHCP_IP_pool();
		if(!pool_change)
			return false;
	}

	if(document.form.wan_ipaddr.value != "0.0.0.0" && document.form.wan_ipaddr.value != "" && 
	   document.form.wan_netmask.value != "0.0.0.0" && document.form.wan_netmask.value != ""){
			if(matchSubnet2(document.form.wan_ipaddr.value, document.form.wan_netmask.value, document.form.lan_ipaddr.value, document.form.lan_netmask.value)){
					document.form.lan_ipaddr.focus();
					alert("WAN and LAN should have different IP addresses and subnet.");
					return false;
			}
	}

	changed_hint();
	
	return true;
}

// step1. check IP changed. // step2. check Subnet is the same 

function changed_DHCP_IP_pool(){
	if(document.form.lan_ipaddr.value != old_lan_ipaddr){ // IP changed
		if(!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3) ||
				!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3)){ // Different Subnet or same
			if(confirm("<#JS_DHCP1#>")){
				document.form.dhcp_start.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3);
				document.form.dhcp_end.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3);
			}else{
					return false;
			}
		}
	}
	
	return true;
}

function done_validating(action){
	refreshpage();
}

var old_lan_ipaddr = "<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>";
function changed_hint(){

		if(document.form.lan_ipaddr.value != old_lan_ipaddr){
				alert("<#LANHostConfig_lanipaddr_changed_hint#>");
		}
	
		return true;
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
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="wan_ipaddr" value="<% nvram_get_x("", "wan_ipaddr_t"); %>">
    <input type="hidden" name="wan_netmask" value="<% nvram_get_x("", "wan_netmask_t"); %>" >
    <input type="hidden" name="wan_gateway" value="<% nvram_get_x("", "wan_gateway_t"); %>">

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
                                                <input type="text" maxlength="15" class="input" size="15" id="lan_ipaddr" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);">
                                                &nbsp;<span style="color:#888;">192.168.1.1</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 4,2);"><#LANHostConfig_SubnetMask_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="lan_netmask" value="<% nvram_get_x("LANHostConfig","lan_netmask"); %>" onkeypress="return is_ipaddr(this);" onkeyup="change_ipaddr(this);" />
                                                <input type="hidden" name="dhcp_start" value="<% nvram_get_x("LANHostConfig", "dhcp_start"); %>">
                                                <input type="hidden" name="dhcp_end" value="<% nvram_get_x("LANHostConfig", "dhcp_end"); %>">
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
