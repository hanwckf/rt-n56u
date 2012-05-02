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

<% login_state_hook(); %>

function initial(){
	show_banner(0);
	show_menu(3, -1, 0);
	show_footer();
	
	calc_lan();
	
//	enable_auto_hint(5, 7);
	
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
	
	if (dhcp_enable_x != "1")
		$("use_lan_dhcp").style.display = "none";
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/poptop.asp";
		
		document.form.submit();
	}
}

function validForm(){
	var pptpd_clib_ip = parseInt(document.form.pptpd_clib.value);
	var pptpd_clie_ip = parseInt(document.form.pptpd_clie.value);
	
	var pptpd_mtu = parseInt(document.form.pptpd_mtu.value);
	var pptpd_mru = parseInt(document.form.pptpd_mru.value);
	
	if(pptpd_mtu < 512 || pptpd_mtu > 1460){
		alert("MTU value should be between 512 and 1460!");
		document.form.pptpd_mtu.focus();
		return false;
	}
	
	if(pptpd_mru < 512 || pptpd_mru > 1460){
		alert("MRU value should be between 512 and 1460!");
		document.form.pptpd_mru.focus();
		return false;
	}
	
	if(pptpd_clib_ip < 2 || pptpd_clib_ip > 254){
		alert("Start IP value should be between 2 and 254!");
		document.form.pptpd_clib.focus();
		return false;
	}
	
	if(pptpd_clie_ip < 2 || pptpd_clie_ip > 254){
		alert("End IP value should be between 2 and 254!");
		document.form.pptpd_clie.focus();
		return false;
	}
	
	if(pptpd_clib_ip > pptpd_clie_ip){
		alert("End IP value should higher or equal than Start IP!");
		document.form.pptpd_clie.focus();
		return false;
	}
	
	if((pptpd_clie_ip - pptpd_clib_ip) > 9){
		alert("PPTPD server only allows max 10 clients!");
		document.form.pptpd_clie.focus();
		return false;
	}
	
	return true;
}

</script>
</head>

<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

<input type="hidden" name="current_page" value="poptop.asp">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="flag" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

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
				<input type="radio" name="pptpd_enable" class="input" value="1" <% nvram_match_x("LANHostConfig", "pptpd_enable", "1", "checked"); %>><#checkbox_Yes#>
				<input type="radio" name="pptpd_enable" class="input" value="0" <% nvram_match_x("LANHostConfig", "pptpd_enable", "0", "checked"); %>><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th><#PopTopAuth#></th>
			<td align="left">
				<select name="pptpd_auth" class="input">
					<option value="0" <% nvram_match_x("LANHostConfig", "pptpd_auth", "0","selected"); %>>MS-CHAP v2 only</option>
					<option value="1" <% nvram_match_x("LANHostConfig", "pptpd_auth", "1","selected"); %>>MS-CHAP v2 and v1</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><#PopTopMPPE#></th>
			<td align="left">
				<select name="pptpd_mppe" class="input">
					<option value="0" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "0","selected"); %>>Auto</option>
					<option value="1" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "1","selected"); %>>MPPE-128</option>
					<option value="2" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "2","selected"); %>>MPPE-56</option>
					<option value="3" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "3","selected"); %>>MPPE-40</option>
					<option value="4" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "4","selected"); %>>No Encryption (unsafe)</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><#PopTopCast#></th>
			<td align="left">
				<select name="pptpd_cast" class="input">
					<option value="0" <% nvram_match_x("LANHostConfig", "pptpd_cast", "0","selected"); %>>Disable</option>
					<option value="1" <% nvram_match_x("LANHostConfig", "pptpd_cast", "1","selected"); %>>LAN to VPN</option>
					<option value="2" <% nvram_match_x("LANHostConfig", "pptpd_cast", "2","selected"); %>>VPN to LAN</option>
					<option value="3" <% nvram_match_x("LANHostConfig", "pptpd_cast", "3","selected"); %>>Both directions</option>
				</select>
			</td>
		</tr>
		<tr>
			<th>MTU:</th>
			<td>
				<input type="text" maxlength="4" size="5" name="pptpd_mtu" class="input" value="<% nvram_get_x("LANHostConfig", "pptpd_mtu"); %>" onkeypress="return is_number(this)">
			</td>
		</tr>
		<tr>
			<th>MRU:</th>
			<td>
				<input type="text" maxlength="4" size="5" name="pptpd_mru" class="input" value="<% nvram_get_x("LANHostConfig", "pptpd_mru"); %>" onkeypress="return is_number(this)">
			</td>
		</tr>
		</table>
		</td>
	</tr>
	<tr>
		<td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
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
				<span style="font-family: Lucida Console;color:#555;">&nbsp;~&nbsp;</span>
				<span style="font-family: Lucida Console;color: #555;"><% nvram_get_x("LANHostConfig","dhcp_end"); %></span>
			</td>
		</tr>
		<tr>
			<th><#PopTopCliPool#></th>
			<td>
				<span id="lanip1" style="font-family: Lucida Console;color:#555;"></span>
				<input type="text" maxlength="3" size="1" class="input" name="pptpd_clib" value="<% nvram_get_x("LANHostConfig","pptpd_clib"); %>" onKeyPress="return is_number(this)"/>
				<span style="font-family: Lucida Console;color:#555;">&nbsp;~&nbsp;</span>
				<span id="lanip2" style="font-family: Lucida Console;color:#555;"></span>
				<input type="text" maxlength="3" size="1" class="input" name="pptpd_clie" value="<% nvram_get_x("LANHostConfig","pptpd_clie"); %>" onKeyPress="return is_number(this)"/>
			</td>
		</tr>
		<tr>
			<th><#PopTopAcc#></th>
			<td>
				<span style="color:#555;"><#PopTopSec#>&nbsp;</span>
				<span style="color:#A55;">/etc/storage/chap-secrets</span>
			</td>
		</tr>
		</table>
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
		<td id="help_td" style="width:15px;" valign="top">
			<form name="hint_form"></form>
			<div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
			<div id="hintofPM" style="display:none;">
			<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
				<thead>
				<tr>
					<td><div id="helpname" class="AiHintTitle"></div><a href="javascript:void(0);" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a></td>
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
