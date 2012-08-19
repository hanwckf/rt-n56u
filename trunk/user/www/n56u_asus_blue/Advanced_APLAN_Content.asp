<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

function initial(){
	final_flag = 1;	// for the function in general.js
	
	show_banner(1);
	show_menu(5,2,1);
	show_footer();

	if(document.form.lan_proto_x[0].checked == true){
		inputCtrl(document.form.lan_ipaddr, 0);
		inputCtrl(document.form.lan_netmask, 0);
		inputCtrl(document.form.lan_gateway, 0);
	}
	else{
		inputCtrl(document.form.lan_ipaddr, 1);
		inputCtrl(document.form.lan_netmask, 1);
		inputCtrl(document.form.lan_gateway, 1);
	}
	
	enable_auto_hint(4, 3);
}

function checkIP(){
	var strIP = $('lan_ipaddr').value;
	var re=/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/g;
	if(document.form.lan_proto_x[1].checked == 1){
		if(re.test(strIP)){
			if( RegExp.$1 == 192 && RegExp.$2 == 168 && RegExp.$3 < 256 && RegExp.$4 < 256){
				applyRule();
				re.test(strIP);
			}
			else if( RegExp.$1 == 172 && RegExp.$2 > 15 && RegExp.$2 < 32 && RegExp.$3 < 256 && RegExp.$4 < 256){
				applyRule();
				re.test(strIP);
			}
			else if( RegExp.$1 == 10 && RegExp.$2 < 256 && RegExp.$3 < 256 && RegExp.$4 < 256){
				applyRule();
				re.test(strIP);
			}
			else{
				alert('"'+strIP+'"'+" <#BM_alert_IP2#>");
				re.test(strIP);
			}
		}
		else alert('"'+strIP+'"'+" <#JS_validip#>");
	}
	else
		applyRule();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "Advanced_APLAN_Content.asp";
		document.form.next_page.value = "/as.asp";
		get_dhcp_range();
		
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
	if(document.form.lan_proto_x[0].checked == 1)
		return true;
	
//  Viz 2012.01  {
	
	var ip_obj = document.form.lan_ipaddr;
	var ip_num = inet_network(ip_obj.value);
	var ip_class = "";		
	if(!valid_IP(ip_obj, "")) return false;  //LAN IP
	if(!valid_IP(document.form.lan_gateway, "GW"))return false;  //Parent Gateway IP

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
//  Viz 2012.01  }			
	
	return true;
}

function get_dhcp_range(){
	var lan_ip = document.form.lan_ipaddr.value.split(".");
	var netmask = document.form.lan_netmask.value.split(".");
	var dhcp_start = new Array(4);
	var dhcp_end = new Array(4);
	var dhcp_range = new Array(2);
	
	for(var i = 0; i < 4; ++i){
		if(netmask[i] == 255){
			dhcp_start[i] = lan_ip[i];
			dhcp_end[i] = lan_ip[i];
		}
		else if(netmask[i] == 0){
			if(i != 3){
				dhcp_start[i] = 0;
				dhcp_end[i] = 255;
			}
			else{
				dhcp_start[i] = 2;
				dhcp_end[i] = 254;
			}
		}
		else{
			dhcp_start[i] = 0;
			dhcp_end[i] = 255-netmask[i];
		}
	}
	
	dhcp_range[0] = dhcp_start.toString(".");
	dhcp_range[1] = dhcp_end.toString(".");
	for(var i = 0; i < 3; ++i){
		dhcp_range[0] = dhcp_range[0].replace(",", ".");
		dhcp_range[1] = dhcp_range[1].replace(",", ".");
	}
	
	document.form.dhcp_start.value = dhcp_range[0];
	document.form.dhcp_end.value = dhcp_range[1];
}

function done_validating(action){
	refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(4, 2);return unload_body();">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:80px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src=""></div>
			<div style="height:10px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

<input type="hidden" name="current_page" value="Advanced_APLAN_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="dhcp_start" value="<% nvram_get_x("LANHostConfig", "dhcp_start"); %>">
<input type="hidden" name="dhcp_end" value="<% nvram_get_x("LANHostConfig", "dhcp_end"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
		<br />
		
		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#LANHostConfig_display1_sectionname#></td>
	</tr>
	</thead>
	<tbody>
	  <tr>
	    <td bgcolor="#FFFFFF"><#LANHostConfig_display1_sectiondesc#></td>
	  </tr>
	</tbody>
	
	<tr>
		<td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<tr>
					<th width="30%">
						<#LANHostConfig_x_LANDHCPClient_itemname#>
					</th>
					
					<td>
						<input type="radio" id="lan_proto_x" name="lan_proto_x" value="1" class="input" onClick="return change_common_radio(this, 'LANHostConfig', 'lan_proto_x', '1')" <% nvram_match_x("LANHostConfig","lan_proto_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" id="lan_proto_x" name="lan_proto_x" value="0" class="input" onClick="return change_common_radio(this, 'LANHostConfig', 'lan_proto_x', '0')" <% nvram_match_x("LANHostConfig","lan_proto_x", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				
		  	<tr>
					<th width="30%">
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(4,1);"><#LANHostConfig_IPRouters_itemname#></a>
					</th>
					
					<td>
						<input type="text" id="lan_ipaddr" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>" maxlength="15" class="input" size="15" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);">
					</td>
				</tr>
				
				<tr>
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(4,2);"><#LANHostConfig_SubnetMask_itemname#></a>
					</th>
					
					<td>
						<input type="text" name="lan_netmask" value="<% nvram_get_x("LANHostConfig", "lan_netmask"); %>" maxlength="15" class="input" size="15" onkeypress="return is_ipaddr(this);" onkeyup="change_ipaddr(this);" />
					</td>
				</tr>
		  	
				<tr>
					<th>
						<a class="hintstyle" href="javascript:void(0);" onClick="openHint(4,3);"><#LANHostConfig_x_Gateway_itemname#></a>
					</th>
					
					<td>
						<input type="text" name="lan_gateway" value="<% nvram_get_x("LANHostConfig", "lan_gateway"); %>" maxlength="15" class="input" size="15" onkeypress="return is_ipaddr(this);" onkeyup="change_ipaddr(this);" />
					</td>
				</tr>
		  	
				<tr align="right">
					<td colspan="2">
						<input class="button" onclick="checkIP();" type="button" value="<#CTL_apply#>"/>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>		
					
		</td>
</form>

					<!--==============Beginning of hint content=============-->
					<td id="help_td" style="width:15px;" valign="top">
						<form name="hint_form"></form>
						<div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
						<div id="hintofPM" style="display:none;">
							<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
								<thead>
								<tr>
									<td>
										<div id="helpname" class="AiHintTitle"></div>
										<a href="javascript:closeHint();">
											<img src="images/button-close.gif" class="closebutton">
										</a>
									</td>
								</tr>
								</thead>
								
								<tr>
									<td valign="top">
										<div class="hint_body2" id="hint_body"></div>
										<iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
									</td>
								</tr>
							</table>
						</div>
					</td>
					<!--==============Ending of hint content=============-->
					
				</tr>
			</table>				
		</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>
