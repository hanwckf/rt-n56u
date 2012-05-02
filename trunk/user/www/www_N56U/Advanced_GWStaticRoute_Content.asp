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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var GWStaticList = [<% get_nvram_list("RouterConfig", "GWStatic"); %>];

function initial(){
	show_banner(1);
	show_menu(5,2,3);
	show_footer();
	
	enable_auto_hint(6, 5);
	showGWStaticList();
}

function applyRule(){
	showLoading();
	
	document.form.action_mode.value = " Restart ";
	document.form.current_page.value = "/Advanced_GWStaticRoute_Content.asp";
	document.form.next_page.value = "";
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function GWStatic_markGroup(o, s, c, b) {	
	document.form.group_id.value = s;	
	
	if(b == " Add "){
		if (document.form.sr_num_x_0.value > c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}
		else if (!validate_ipaddr(document.form.sr_ipaddr_x_0, "") ||
				 !validate_ipaddr(document.form.sr_netmask_x_0, "") ||
				 !validate_ipaddr(document.form.sr_gateway_x_0, "")){
				 return false;
		}
		else if (document.form.sr_ipaddr_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_ipaddr_x_0.focus();
				 return false;				 
		}
		else if (document.form.sr_netmask_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_netmask_x_0.focus();
				 return false;				 
		}
		else if (document.form.sr_gateway_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_gateway_x_0.focus();
				 return false;				 
		}				
		else if (GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_ipaddr_x_0.value, 16, 0) &&
				 GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_netmask_x_0.value, 16, 1) &&
				 GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_gateway_x_0.value, 16, 2) &&
				 GWStatic_validate_duplicate(GWStaticList, document.form.sr_if_x_0.value, 2, 4)
				) return false;  //Check the IP, Submask, gateway and Interface is duplicate or not.
	}
	
	pageChanged = 0;
	pageChangedCount = 0;
	
	document.form.action_mode.value = b;
	return true;		
}

function GWStatic_validate_duplicate_noalert(o, v, l, off){
	for (var i=0; i < o.length; i++)
	{
		if (entry_cmp(o[i][off], v, l)==0){ 
			return true;
		}
	}
	return false;
}

function GWStatic_validate_duplicate(o, v, l, off){
	for(var i = 0; i < o.length; i++){
		if(entry_cmp(o[i][off].toLowerCase(), v.toLowerCase(), l) == 0){
			alert('<#JS_duplicate#>');
			return true;
		}
	}
	return false;
}

function showGWStaticList(){
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="3" align="center" class="list_table">';
	if(GWStaticList.length == 0)
		code +='<tr><td style="color:#CC0000;"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < GWStaticList.length; i++){
		code +='<tr id="row' + i + '">';
		code +='<td width="110">' + GWStaticList[i][0] + '</td>';	//IP
		code +='<td width="110">' + GWStaticList[i][1] + '</td>';	//Mask
		code +='<td width="110">' + GWStaticList[i][2] + '</td>';	//Gateway
		code +='<td width="48">' + GWStaticList[i][3] + '</td>';	//Metric
		code +='<td width="65">' + GWStaticList[i][4] + '</td>';	//Interface
		code +='<td width="20"><input type="checkbox" name="GWStatic_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		if(i == 0)
			code +='<td style="background:#C0DAE4;" rowspan="' + GWStaticList.length + '"><input class="button" type="submit" onclick="markGroup(this, \'GWStatic\', 32, \' Del \');" value="<#CTL_del#>"/></td>';
		
		code +='</tr>';
		}
	}
	code +='</table>';
	
	$("GWStaticList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
 		$("row" + num).style.background='#FF9';
	else
 		$("row" + num).style.background='#FFF';
}

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(6, 5);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_GWStaticRoute_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="RouterConfig;">
<input type="hidden" name="group_id" value="GWStatic">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="sr_num_x_0" value="<% nvram_get_x("RouterConfig", "sr_num_x"); %>" readonly="1">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div><br />
<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td valign="top" >
	  <table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
		<thead>
		<tr>
		  <td><#menu5_2#> - <#menu5_2_3#></td>
		</tr>
		</thead>
		<tr>
		  <td bgcolor="#FFFFFF"><#RouterConfig_GWStaticEnable_sectiondesc#></td>
		</tr>
		<tbody>
		<tr>
		  <td bgcolor="#FFFFFF"></td>
		</tr>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
				<tr>
					<td colspan="2"><#menu5_2_3#></td>
				</tr>
				</thead>
				<tr>
					<th width="40%"><#RouterConfig_GWDHCPEnable_itemname#></th>
					<td>
						<input type="radio" value="1" name="dr_enable_x" class="input" onClick="return change_common_radio(this, 'RouterConfig', 'dr_enable_x', '1')" <% nvram_match_x("RouterConfig", "dr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="dr_enable_x" class="input" onClick="return change_common_radio(this, 'RouterConfig', 'dr_enable_x', '0')" <% nvram_match_x("RouterConfig", "dr_enable_x", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				<tr>
					<th width="40%"><#RouterConfig_GWStaticEnable_itemname#></th>
					<td>
						<input type="radio" value="1" name="sr_enable_x" class="input" onclick="return change_common_radio(this, 'RouterConfig', 'sr_enable_x', '1')" <% nvram_match_x("RouterConfig", "sr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="sr_enable_x" class="input" onclick="return change_common_radio(this, 'RouterConfig', 'sr_enable_x', '0')" <% nvram_match_x("RouterConfig", "sr_enable_x", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
			</table>
		    </td>
		</tr>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="3" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
				<tr>
					<td colspan="6"><#RouterConfig_GWStatic_groupitemdesc#></td>
				</tr>
				</thead>
				<tr>
					<th width="110" style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,1);"><#RouterConfig_GWStaticIP_itemname#></a></th>
					<th width="110" style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,2);"><#RouterConfig_GWStaticMask_itemname#></a></th>
					<th width="110" style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,3);"><#RouterConfig_GWStaticGW_itemname#></a></th>
					<th width="48" style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,4);"><#RouterConfig_GWStaticMT_itemname#></a></th>
					<th width="65" style="text-align:center;"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,5);"><#RouterConfig_GWStaticIF_itemname#></a></th>
					<th>&nbsp;</th>
				</tr>
				<tr>
					<td align="center"><input type="text" maxlength="15" class="input" size="12" name="sr_ipaddr_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"/></td>
					<td align="center"><input type="text" maxlength="15" class="input" size="12" name="sr_netmask_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"/></td>
					<td align="center"><input type="text" maxlength="15" class="input" size="12" name="sr_gateway_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"/></td>
					<td align="center"><input type="text" maxlength="3"  class="input" size="1" name="sr_matric_x_0"  onkeypress="return is_number(this)"></td>
					<td align="center">
						<select name="sr_if_x_0" class="input">
							<option value="LAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "LAN","selected", 0); %>>LAN</option>
							<option value="MAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "MAN","selected", 0); %>>MAN</option>
							<option value="WAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "WAN","selected", 0); %>>WAN</option>
						</select>
					</td>
					<td align="center">
						<input class="button" type="submit" onClick="return GWStatic_markGroup(this, 'GWStatic', 32, ' Add ');" name="GWStatic" value="<#CTL_add#>">
					</td>
				</tr>
			</table>
			<div id="GWStaticList_Block"></div>
		  </td>
		</tr>
		</tbody>
		<tr>
			<td align="right" bgcolor="#FFFFFF"><input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/></td>
		</tr>
	  </table>
	</td>
</form>

	<!--==============Beginning of hint content=============-->
	<td id="help_td" style="width:15px;"  valign="top">
	  <div id="helpicon" onClick="openHint(0, 0);" title="<#Help_button_default_hint#>">
		<img src="images/help.gif">
	  </div>
	  
	  <div id="hintofPM" style="display:none;">
<form name="hint_form"></form>
		<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
		  <thead>
		  <tr>
			<td>
			  <div id="helpname" class="AiHintTitle"></div>
			  <a href="javascript:closeHint();"><img src="images/button-close.gif" class="closebutton" /></a>
			</td>
		  </tr>
		  </thead>
		  
		  <tbody>
		  <tr>
			<td valign="top">
			  <div id="hint_body" class="hint_body2"></div>
			  <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
			</td>
		  </tr>
		  </tbody>
		</table>
	  </div>
	</td>
	<!--==============Ending of hint content=============-->
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
