<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_1#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>

var $j = jQuery.noConflict();

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

function initial(){
	final_flag = 1;	// for the function in general.js
	
	show_banner(1);
	show_menu(5,3,1);
	show_footer();
	
	enable_auto_hint(4, 2);
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
	// Viz 2012.01 {	
	if(matchSubnet(document.form.lan_ipaddr.value, document.form.wan_ipaddr.value, 3)){
			alert("<#JS_validip#>");
			document.form.lan_ipaddr.focus();
			document.form.lan_ipaddr.select();
			return false;
	}		
	
	var ip_obj = document.form.lan_ipaddr;
	var ip_num = inet_network(ip_obj.value);
	var ip_class = "";		
	if(!valid_IP(ip_obj, "")) return false;  //LAN IP 

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
  //Viz modify 2011.10 for DHCP pool issue {
	if(sw_mode == "1"){
				var pool_change = changed_DHCP_IP_pool();
	if(!pool_change)
				return false;
	}				
	//}Viz modify 2011.10 for DHCP pool issue 
	
// Viz 2012.01 }	

	changed_hint();
	//checkSubnet();	//change client ipaddr pool
	
	return true;
}

// step1. check IP changed. // step2. check Subnet is the same 
var old_lan_ipaddr = "<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>";
function changed_DHCP_IP_pool(){
	if(document.form.lan_ipaddr.value != old_lan_ipaddr){ // IP changed
		if(!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3) ||
				!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3)){ // Different Subnet or same
				document.form.dhcp_start.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3);
				document.form.dhcp_end.value = subnetPrefix(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3);				
		}
	}
	
	var post_lan_netmask='';
	var pool_start='';
	var pool_end='';
	var nm = new Array("0", "128", "192", "224", "240", "248", "252");
	// --- get lan_ipaddr post set .xxx  By Viz 2011.10
	z=0;
	tmp_ip=0;
	for(i=0;i<document.form.lan_ipaddr.value.length;i++){
			if (document.form.lan_ipaddr.value.charAt(i) == '.')	z++;
			if (z==3){ tmp_ip=i+1; break;}
	}		
	post_lan_ipaddr = document.form.lan_ipaddr.value.substr(tmp_ip,3);
	// --- get lan_netmask post set .xxx	By Viz 2011.10
	c=0;
	tmp_nm=0;
	for(i=0;i<document.form.lan_netmask.value.length;i++){
			if (document.form.lan_netmask.value.charAt(i) == '.')	c++;
			if (c==3){ tmp_nm=i+1; break;}
	}		
	post_lan_netmask = document.form.lan_netmask.value.substr(tmp_nm,3);

// Viz add 2011.10 default DHCP pool range{
	for(i=0;i<nm.length;i++){			 		
				 if(post_lan_netmask==nm[i]){
							gap=256-Number(nm[i]);							
							subnet_set = 256/gap;
							for(j=1;j<=subnet_set;j++){
									if(post_lan_ipaddr < 1*gap && post_lan_ipaddr==1){		//Viz add to avoid default (1st) LAN ip in DHCP pool (start)2011.11
												pool_start=2;
												pool_end=1*gap-2;
												break;										//Viz add to avoid default (1st) LAN ip in DHCP pool (end)2011.11
									}else	if(post_lan_ipaddr < j*gap){
												pool_start=(j-1)*gap+1;
												pool_end=j*gap-2;
												break;						
									}
							}																	
							break;
				 }
	}
	
		var update_pool_start = subnetPostfix(document.form.dhcp_start.value, pool_start, 3);
		var update_pool_end = subnetPostfix(document.form.dhcp_end.value, pool_end, 3);							
		if((document.form.dhcp_start.value != update_pool_start) || (document.form.dhcp_end.value != update_pool_end)){
				if(confirm("<#JS_DHCP1#>")){
						document.form.dhcp_start.value = update_pool_start;
						document.form.dhcp_end.value = update_pool_end;
				}else{
						return false;	
				}
		}	
			
	return true;	
	alert(document.form.dhcp_start.value+" , "+document.form.dhcp_end.value);//Viz
	// } Viz add 2011.10 default DHCP pool range
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

<body onload="initial();" onunLoad="disable_auto_hint(4, 2);return unload_body();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="hiddenMask" class="popup_bg" style="position: absolute; margin-left: -10000px;">
        <table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
            <tr>
            <td>
                <div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
                    <br/>
                    <br/>
                </div>
              <div class="drImg"><img src="images/DrsurfImg.gif"></div>
                <div style="height:70px; "></div>
            </td>
            </tr>
        </table>
    <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

    <input type="hidden" name="current_page" value="Advanced_LAN_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
    <input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
    <input type="hidden" name="wan_ipaddr" value="<% nvram_get_x("IPConnection", "wan0_ipaddr"); %>">
    <input type="hidden" name="wan_netmask" value="<% nvram_get_x("IPConnection", "wan_netmask"); %>" >
    <input type="hidden" name="wan_gateway" value="<% nvram_get_x("IPConnection", "wan0_gateway"); %>">
    <input type="hidden" name="wan_proto" value="<% nvram_get_x("IPConnection", "wan_proto"); %>">

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
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this, 4,2);"><#LANHostConfig_SubnetMask_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="lan_netmask" value="<% nvram_get_x("LANHostConfig","lan_netmask"); %>" onkeypress="return is_ipaddr(this);" onkeyup="change_ipaddr(this);" />
                                                <input type="hidden" name="dhcp_start" value="<% nvram_get_x("LANHostConfig", "dhcp_start"); %>">
                                                <input type="hidden" name="dhcp_end" value="<% nvram_get_x("LANHostConfig", "dhcp_end"); %>">
                                            </td>
                                        </tr>
                                        <tr>
                                            <td style="margin-top: 10px;">&nbsp;</td>
                                            <td>
                                                <br />
                                                <input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" />
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
</div>
</body>
</html>
