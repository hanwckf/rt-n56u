<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
lan_ipaddr_x = '<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>';
dhcp_enable_x = '<% nvram_get_x("LANHostConfig", "dhcp_enable_x"); %>';
var ACLList = [<% get_nvram_list("LANHostConfig", "VPNSACLList", "vpns_pass_x"); %>];

<% login_state_hook(); %>


function initial(){
	show_banner(0);
	show_menu(3, -1, 0);
	show_footer();
	
	calc_lan();
	
	change_vpn_srv_proto(0);
	showACLList();
	
	load_body();
}

function calc_lan(){
	var lan_part = lan_ipaddr_x;
	var lastdot = lan_ipaddr_x.lastIndexOf(".");
	if (lastdot > 3)
		lan_part = lan_part.slice(0, lastdot+1);
	
	$("lanip0").innerHTML = lan_ipaddr_x;
	$("lanip1").innerHTML = lan_part;
	$("lanip2").innerHTML = lan_part;
	$("lanip3").innerHTML = lan_part;
	
	if (dhcp_enable_x != "1")
		$("use_lan_dhcp").style.display = "none";
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/vpnsrv.asp";
		
		document.form.submit();
	}
}

function validForm(){
	var vpns_cli0_ip = parseInt(document.form.vpns_cli0.value);
	var vpns_cli1_ip = parseInt(document.form.vpns_cli1.value);
	
	var vpns_mtu = parseInt(document.form.vpns_mtu.value);
	var vpns_mru = parseInt(document.form.vpns_mru.value);
	
	if(vpns_mtu < 512 || vpns_mtu > 1460){
		alert("MTU value should be between 512 and 1460!");
		document.form.vpns_mtu.focus();
		return false;
	}
	
	if(vpns_mru < 512 || vpns_mru > 1460){
		alert("MRU value should be between 512 and 1460!");
		document.form.vpns_mru.focus();
		return false;
	}
	
	if(vpns_cli0_ip < 2 || vpns_cli0_ip > 254){
		alert("Start IP value should be between 2 and 254!");
		document.form.vpns_cli0.focus();
		return false;
	}
	
	if(vpns_cli1_ip < 2 || vpns_cli1_ip > 254){
		alert("End IP value should be between 2 and 254!");
		document.form.vpns_cli1.focus();
		return false;
	}
	
	if(vpns_cli0_ip > vpns_cli1_ip){
		alert("End IP value should higher or equal than Start IP!");
		document.form.vpns_cli1.focus();
		return false;
	}
	
	if((vpns_cli1_ip - vpns_cli0_ip) > 9){
		alert("PPTPD server only allows max 10 clients!");
		document.form.vpns_cli1.focus();
		return false;
	}
	
	return true;
}

function done_validating(action){
	if (action == " Del ")
		refreshpage();
}

function change_vpn_srv_proto(mflag) {
	var mode = document.form.vpns_type.value;
	if (mode == "1") {
		$("mppe_row").style.display = "none";
	}
	else {
		$("mppe_row").style.display = "";
	}
}

function markGroupACL(o, c, b) {
	var acl_addr;
	document.form.group_id.value = "VPNSACLList";
	if(b == " Add "){
		acl_addr = parseInt(document.form.vpns_addr_x_0.value);
		if (document.form.vpns_num_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.vpns_user_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.vpns_user_x_0.focus();
			document.form.vpns_user_x_0.select();
			return false;
		}else if(document.form.vpns_pass_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.vpns_pass_x_0.focus();
			document.form.vpns_pass_x_0.select();
			return false;
		}else if((document.form.vpns_addr_x_0.value!="") && (acl_addr<2 || acl_addr>254)){
			alert("IP octet value should be between 2 and 254!");
			document.form.vpns_addr_x_0.focus();
			document.form.vpns_addr_x_0.select();
			return false;
		}else{
			for(i=0; i< ACLList.length; i++){
				if(document.form.vpns_user_x_0.value==ACLList[i][0]) {
					alert('<#JS_duplicate#>' + ' (' + ACLList[i][0] + ')' );
					document.form.vpns_user_x_0.focus();
					document.form.vpns_user_x_0.select();
					return false;
				}
				if((document.form.vpns_addr_x_0.value!="") &&
				   (document.form.vpns_addr_x_0.value==ACLList[i][2])) {
					alert('<#JS_duplicate#>' + ' (' + ACLList[i][0] + ')' );
					document.form.vpns_addr_x_0.focus();
					document.form.vpns_addr_x_0.select();
					return false;
				}
			}
		}
	}
	pageChanged = 0;
	pageChangedCount = 0;
	document.form.action_mode.value = b;
	return true;
}

function showACLList(){
	var code = "";
	var acl_addr;
	var lan_part = lan_ipaddr_x;
	var lastdot = lan_ipaddr_x.lastIndexOf(".");
	if (lastdot > 3)
		lan_part = lan_part.slice(0, lastdot+1);

	code +='<table width="100%" border="1" cellspacing="0" cellpadding="3" align="center" class="list_table">';
	if(ACLList.length == 0)
		code +='<tr><td colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < ACLList.length; i++){
		if (ACLList[i][2] == "")
			acl_addr = "*";
		else
			acl_addr = lan_part + ACLList[i][2];
		code +='<tr id="row' + i + '">';
		code +='<td width="30%">' + ACLList[i][0] + '</td>';
		code +='<td width="30%">*****</td>';
		code +='<td width="20%">' + acl_addr + '</td>';
		code +='<td width="20%"><input type="checkbox" name="VPNSACLList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		code +='</tr>';
		}

		if(ACLList.length > 0)
		{
		    code += '<tfoot><tr>';
		    code += '<td colspan="3">&nbsp;</td>'
		    code += '<td><input class="button" type="submit" onclick="markGroupACL(this, 10, \' Del \');" name="VPNSACLList" value="<#CTL_del#>"/></td>';
		    code += '</tr></tfoot>'
		}
	}

	code +='</table>';

	$("ACLList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
		$("row" + num).style.background='#FF9';
	else
		$("row" + num).style.background='#FFF';
}

</script>
</head>

<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
<input type="hidden" name="current_page" value="vpnsrv.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;PPPConnection;">
<input type="hidden" name="group_id" value="VPNSACLList">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="flag" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="vpns_num_x_0" value="<% nvram_get_x("LANHostConfig", "vpns_num_x"); %>" readonly="1">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
		
    <td valign="top">
	<div id="tabMenu"></div>
		<!--===================================Beginning of Main Content===========================================-->
	<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
	<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu2#></td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"></td>
	</tr>
	</tbody>
	<tr>
		<td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
		<thead>
		<tr>
			<td colspan="2"><#PopTopBase#></td>
		</tr>
		</thead>
		<tr>
			<th width="40%"><#PopTopEnable#></th>
			<td>
				<input type="radio" name="vpns_enable" class="input" value="1" <% nvram_match_x("LANHostConfig", "vpns_enable", "1", "checked"); %>><#checkbox_Yes#>
				<input type="radio" name="vpns_enable" class="input" value="0" <% nvram_match_x("LANHostConfig", "vpns_enable", "0", "checked"); %>><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th><#PopTopType#></th>
			<td align="left">
				<select name="vpns_type" class="input" onchange="change_vpn_srv_proto(1);">
					<option value="0" <% nvram_match_x("LANHostConfig", "vpns_type", "0","selected"); %>>PPTP</option>
					<option value="1" <% nvram_match_x("LANHostConfig", "vpns_type", "1","selected"); %>>L2TP (w/o IPSec)</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><#PopTopAuth#></th>
			<td align="left">
				<select name="vpns_auth" class="input">
					<option value="0" <% nvram_match_x("LANHostConfig", "vpns_auth", "0","selected"); %>>MS-CHAP v2 only</option>
					<option value="1" <% nvram_match_x("LANHostConfig", "vpns_auth", "1","selected"); %>>MS-CHAP v2/v1</option>
					<option value="2" <% nvram_match_x("LANHostConfig", "vpns_auth", "2","selected"); %>>MS-CHAP v2/v1 and CHAP</option>
				</select>
			</td>
		</tr>
		<tr id="mppe_row">
			<th><#PopTopMPPE#></th>
			<td align="left">
				<select name="vpns_mppe" class="input">
					<option value="0" <% nvram_match_x("LANHostConfig", "vpns_mppe", "0","selected"); %>>Auto</option>
					<option value="1" <% nvram_match_x("LANHostConfig", "vpns_mppe", "1","selected"); %>>MPPE-128</option>
					<option value="2" <% nvram_match_x("LANHostConfig", "vpns_mppe", "2","selected"); %>>MPPE-40</option>
					<option value="3" <% nvram_match_x("LANHostConfig", "vpns_mppe", "3","selected"); %>>No Encryption</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><#PopTopCast#></th>
			<td align="left">
				<select name="vpns_cast" class="input">
					<option value="0" <% nvram_match_x("LANHostConfig", "vpns_cast", "0","selected"); %>>Disable</option>
					<option value="1" <% nvram_match_x("LANHostConfig", "vpns_cast", "1","selected"); %>>LAN to VPN</option>
					<option value="2" <% nvram_match_x("LANHostConfig", "vpns_cast", "2","selected"); %>>VPN to LAN</option>
					<option value="3" <% nvram_match_x("LANHostConfig", "vpns_cast", "3","selected"); %>>Both directions</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><#PPP_LimitCPU#></th>
			<td>
				<select name="wan_pppoe_cpul" class="input">
					<option value="0" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "0","selected"); %>><#checkbox_No#></option>
					<option value="2500" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "2500","selected"); %>>2500 cycles</option>
					<option value="3000" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "3000","selected"); %>>3000 cycles</option>
					<option value="3500" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "3500","selected"); %>>3500 cycles</option>
					<option value="4000" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "4000","selected"); %>>4000 cycles</option>
					<option value="4500" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "4500","selected"); %>>4500 cycles</option>
					<option value="5000" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "5000","selected"); %>>5000 cycles</option>
				</select>
			</td>
		</tr>
		<tr>
			<th>MTU:</th>
			<td>
				<input type="text" maxlength="4" size="5" name="vpns_mtu" class="input" value="<% nvram_get_x("LANHostConfig", "vpns_mtu"); %>" onkeypress="return is_number(this)">
			</td>
		</tr>
		<tr>
			<th>MRU:</th>
			<td>
				<input type="text" maxlength="4" size="5" name="vpns_mru" class="input" value="<% nvram_get_x("LANHostConfig", "vpns_mru"); %>" onkeypress="return is_number(this)">
			</td>
		</tr>
		</table>
		</td>
	</tr>
	<tr>
		<td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
		<thead>
		<tr>
			<td colspan="2"><#PopTopCli#></td>
		</tr>
		</thead>
		<tr>
			<th width="40%"><#PopTopSrvIP#></th>
			<td>
				<span id="lanip0" style="font-family: Lucida Console;color: #555;"></span>
			</td>
		</tr>
		<tr id="use_lan_dhcp">
			<th><#PopTopCliDHCP#></th>
			<td>
				<span style="font-family: Lucida Console;color: #555;"><% nvram_get_x("LANHostConfig","dhcp_start"); %></span>
				<span style="font-family: Lucida Console;color: #555;">&nbsp;~&nbsp;</span>
				<span style="font-family: Lucida Console;color: #555;"><% nvram_get_x("LANHostConfig","dhcp_end"); %></span>
			</td>
		</tr>
		<tr>
			<th><#PopTopCliPool#></th>
			<td>
				<span id="lanip1" style="font-family: Lucida Console;color:#555;"></span>
				<input type="text" maxlength="3" size="2" class="input" style="width: 25px;" name="vpns_cli0" value="<% nvram_get_x("LANHostConfig","vpns_cli0"); %>" onKeyPress="return is_number(this)"/>
				<span style="font-family: Lucida Console;color:#555;">&nbsp;~&nbsp;</span>
				<span id="lanip2" style="font-family: Lucida Console;color:#555;"></span>
				<input type="text" maxlength="3" size="2" class="input" style="width: 25px;" name="vpns_cli1" value="<% nvram_get_x("LANHostConfig","vpns_cli1"); %>" onKeyPress="return is_number(this)"/>
			</td>
		</tr>
		</table>
		</td>
	</tr>
	<tr>
		<td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
		<thead>
                <tr>
                    <td colspan="4"><#PopTopAcc#></td>
                </tr>
		</thead>
                <tr>
                    <th width="30%" style="text-align:center;"><#ISP_Authentication_user#></th>
                    <th width="30%" style="text-align:center;"><#ISP_Authentication_pass#></th>
                    <th width="20%" style="text-align:center;"><#PopTopResIP#></th>
                    <th width="20%">&nbsp;</th>
                </tr>
                <tr>
                    <td align="center">
                        <input type="text" class="input" size="22" autocomplete="off" maxlength="32" name="vpns_user_x_0" onkeypress="return is_string(this)" />
                    </td>
                    <td align="center">
                        <input type="text" class="input" size="22" autocomplete="off" maxlength="32" name="vpns_pass_x_0" onkeypress="return is_string(this)" />
                    </td>
                    <td align="center">
                        <span id="lanip3" style="font-family: Lucida Console;color: #555;"></span>
                        <input type="text" class="input" size="2" maxlength="3" autocomplete="off" style="width: 25px;" name="vpns_addr_x_0" onkeypress="return is_number(this)" />
                    </td>
                    <td align="center">
                        <input class="button" type="submit" onclick="return markGroupACL(this, 10, ' Add ');" name="VPNSACLList2" value="<#CTL_add#>"/>
                    </td>
                </tr>
                </table>
                <div id="ACLList_Block"></div>
		</td>
	</tr>
	<tr>
		<td bgcolor="#FFFFFF" colspan="2" align="right">
			<input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/>
		</td>
	</tr>
	</table>
	</td>
</form>
	<!--==============Beginning of hint content=============-->
		<td style="width:15px;" valign="top">
		</td>
	</tr>
	</table>
		<!--===================================Ending of Main Content===========================================-->		
	</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>
