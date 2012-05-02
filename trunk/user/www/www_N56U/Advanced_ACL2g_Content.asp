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
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var client_mac = login_mac_str();
var rt_macnum_x = '<% nvram_get_x("FirewallConfig", "rt_macnum_x"); %>';
var smac = client_mac.split(":");
var simply_client_mac = smac[0] + smac[1] + smac[2] + smac[3] + smac[4] + smac[5];

function initial(){
	show_banner(1);
	
	if(sw_mode == "1" || sw_mode == "4")
		show_menu(5,1,4);
	else if(sw_mode == "2")
		show_menu(5,1,2);
	else
		show_menu(5,1,3);
	
	show_footer();	
}

function applyRule(){
	if(prevent_lock()){
		showLoading();	
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/as.asp";
		document.form.next_page.value = "";
		document.form.submit();
	}
	else
		return false;
}

function done_validating(action){
	refreshpage();
}

function prevent_lock(){

	if(document.form.rt_macmode.value == "allow"){
		if(rt_macnum_x == 0){
			if(confirm("<#FirewallConfig_MFList_accept_hint1#>")){
				document.form.rt_maclist_x_0.value = simply_client_mac;
				markGroup(document.form.rt_ACLList2, 'rt_ACLList', 32, ' Add ');
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
<input type="hidden" name="next_page" value="Advanced_WSecurity_Content2g.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="DeviceSecurity11a;">
<input type="hidden" name="group_id" value="rt_ACLList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
		
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
<table width="500" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
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
				<th width="30%" >
					<a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,1);"><#FirewallConfig_MFMethod_itemname#></a>
				</th>
				<td>
					<select name="rt_macmode" class="input"  onChange="return change_common(this, 'DeviceSecurity11a', 'rt_macmode')">
					<option class="content_input_fd" value="disabled" <% nvram_match_x("DeviceSecurity11a","rt_macmode", "disable","selected"); %>><#CTL_Disabled#></option>
					<option class="content_input_fd" value="allow" <% nvram_match_x("DeviceSecurity11a","rt_macmode", "allow","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
					<option class="content_input_fd" value="deny" <% nvram_match_x("DeviceSecurity11a","rt_macmode", "deny","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
					</select>
				</td>
			</tr>
		</table>
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">

          <tr>
            <th width="30%"><#FirewallConfig_MFhwaddr_itemname#>
                <input type="hidden" name="rt_macnum_x_0" value="<% nvram_get_x("DeviceSecurity11a", "rt_macnum_x"); %>" readonly="1" /></th>
			  <td>
                <input type="text" maxlength="12" class="input" size="14" name="rt_maclist_x_0" onkeypress="return is_hwaddr()" />
                
                <input class="button" type="submit" onclick="return markGroup(this, 'rt_ACLList', 32, ' Add ');" name="rt_ACLList2" value="<#CTL_add#>" size="12"/>
                <br/><span>*<#JS_validmac#></span>
              </td>
          </tr>
          <tr>
            <th align="right"><#FirewallConfig_MFList_groupitemname#></th>
            <td>
			<select size="8" name="rt_ACLList_s" multiple="multiple" class="input" style="font-size:12px; font-weight:bold; vertical-align:middle;">
              <% nvram_get_table_x("DeviceSecurity11a","rt_ACLList"); %>
            </select>
              <input class="button" type="submit" onClick="return markGroup(this, 'rt_ACLList', 32, ' Del ');" name="rt_ACLList" value="<#CTL_del#>" size="12"></td>
          </tr>
          <tr align="right">
			<td colspan="2">
				<input type="button" class="button5" value="<#GO_5G#>" onclick="location.href='Advanced_ACL_Content.asp';">    
				<input class="button" onclick="applyRule()" type="button" value="<#CTL_apply#>"/></td>
		  </tr>
		</table></td>
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
