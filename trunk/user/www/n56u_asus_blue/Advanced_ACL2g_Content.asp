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

<title>ASUS Wireless Router <#Web_Title#> - <#menu5_1_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help_2g.js"></script>
<script language="JavaScript" type="text/javascript" src="/general_2g.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';

<% login_state_hook(); %>

var client_mac = login_mac_str();
var smac = client_mac.split(":");

var ACLList = [<% get_nvram_list("DeviceSecurity11b", "rt_ACLList"); %>];

function initial(){
	show_banner(1);
	
	show_menu(5,1,4);
	
	show_footer();
	
	showACLList();
}

function applyRule(){
	if(prevent_lock()){
		showLoading();
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_ACL2g_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
	else
		return false;
}

function prevent_lock(){
	if(document.form.rt_macmode.value == "allow"){
		if(document.form.rt_macnum_x_0.value < 1){
			if(confirm("<#FirewallConfig_MFList_accept_hint1#>")){
				document.form.rt_maclist_x_0.value = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];
				document.form.rt_macdesc_x_0.value = "";
				markGroupACL(document.form.rt_ACLList2, 32, ' Add ');
				document.form.submit();
			}
			else
				return false;
		}
		else
			return true;
	}
	else
		return true;
}

function validNewRow(max_rows) {
	if (document.form.rt_maclist_x_0.value >= max_rows){
		alert("<#JS_itemlimit1#> " + max_rows + " <#JS_itemlimit2#>");
		return false;
	}

	if (document.form.rt_maclist_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.rt_maclist_x_0.focus();
		document.form.rt_maclist_x_0.select();
		return false;
	}

	if (!validate_hwaddr(document.form.rt_maclist_x_0)){
		return false;
	}

	return true;
}

function markGroupACL(o, c, b) {
	document.form.group_id.value = "rt_ACLList";
	if(b == " Add "){
		if (validNewRow(c) == false)
			return false;
	}
	pageChanged = 0;
	pageChangedCount = 0;
	document.form.action_mode.value = b;
	return true;
}

function showACLList(){
	var code = "";

	code +='<table width="100%" border="1" cellspacing="0" cellpadding="3" align="center" class="list_table">';
	if(ACLList.length == 0) {
		code +='<tr><td colspan="3"><#IPConnection_VSList_Norule#></td></tr>';
	}
	else{
		for(var i = 0; i < ACLList.length; i++){
		    code +='<tr id="row' + i + '">';
		    code +='<td width="30%">' + ACLList[i][0] + '</td>';
		    code +='<td width="50%">' + ACLList[i][1] + '</td>';
		    code +='<td width="20%"><input type="checkbox" name="rt_ACLList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		    code +='</tr>';
		}
		
		code += '<tfoot><tr>';
		code += '<td colspan="2">&nbsp;</td>'
		code += '<td><input class="button" type="submit" onclick="return markGroupACL(this, 32, \' Del \');" name="rt_ACLList" value="<#CTL_del#>"></td>';
		code += '</tr></tfoot>'
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

function done_validating(action){
	refreshpage();
}

</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>		
		<td valign="top" width="202">				
		<div  id="mainMenu"></div>	
		<div  id="subMenu"></div>		
		</td>				
		
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
	<br />
		<!--===================================Beginning of Main Content===========================================-->
<input type="hidden" name="current_page" value="Advanced_ACL2g_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="DeviceSecurity11b;">
<input type="hidden" name="group_id" value="rt_ACLList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="rt_macnum_x_0" value="<% nvram_get_x("", "rt_macnum_x"); %>" readonly="1" />

	<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
	<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
	<thead>
	<tr>
		<td><#menu5_1#> - <#menu5_1_4#> (2.4GHz)</td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#DeviceSecurity11a_display1_sectiondesc#></td>
	</tr>
	</tbody>
	<tr>
	  <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<tr>
				<th width="40%" >
					<a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,1);"><#FirewallConfig_MFMethod_itemname#></a>
				</th>
				<td>
					<select name="rt_macmode" class="input"  onChange="return change_common(this, 'DeviceSecurity11b', 'rt_macmode')">
					<option value="disabled" <% nvram_match_x("DeviceSecurity11b","rt_macmode", "disable","selected"); %>><#CTL_Disabled#></option>
					<option value="allow" <% nvram_match_x("DeviceSecurity11b","rt_macmode", "allow","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
					<option value="deny" <% nvram_match_x("DeviceSecurity11b","rt_macmode", "deny","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
					</select>
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
                    <td colspan="3"><#FirewallConfig_MFList_groupitemname#></td>
                </tr>
                </thead>
                <tr>
                    <th width="30%" style="text-align:center;"><#FirewallConfig_MFhwaddr_itemname#></th>
                    <th width="50%" style="text-align:center;"><#WIFIMacDesc#></th>
                    <th width="20%">&nbsp;</th>
                </tr>
                <tr>
                    <td align="center">
                        <input type="text" class="input" size="25" maxlength="12" name="rt_maclist_x_0" onkeypress="return is_hwaddr()" />
                    </td>
                    <td align="center">
                        <input type="text" class="input" size="32" maxlength="32" name="rt_macdesc_x_0" onkeypress="return is_string(this)" />
                    </td>
                    <td align="center">
                        <input class="button" type="submit" onclick="return markGroupACL(this, 32, ' Add ');" name="rt_ACLList2" value="<#CTL_add#>"/>
                    </td>
                </tr>
                </table>
                <div id="ACLList_Block"></div>
                <tr align="right">
                    <td colspan="2">
                        <input type="button" class="button5" value="<#GO_5G#>" onclick="location.href='Advanced_ACL_Content.asp';">    
                        <input class="button" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
                    </td>
                </tr>
	    </td>
	</tr>
</table>
</td>
</form>

	<td id="help_td" style="width:15px;" valign="top">
		  
	  <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
	  <div id="hintofPM" style="display:none;">
	  	<form name="hint_form"></form>
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
	  </div><!--End of hintofPM-->
	  
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
