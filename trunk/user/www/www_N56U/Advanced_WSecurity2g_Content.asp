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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_1_5#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help_2g.js"></script>
<script type="text/javascript" src="/general_2g.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

function initial(){
	show_banner(1);

	if(sw_mode == "1" || sw_mode == "4")
		show_menu(5,1,5);
	else
		show_menu(5,1,4);
	
	show_footer();
	
	enable_auto_hint(2, 3);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/as.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(!validate_ipaddr(document.form.rt_radius_ipaddr, 'rt_radius_ipaddr'))
		return false;
	
	if(!validate_range(document.form.rt_radius_port, 0, 65535))
		return false;
	
	if(!validate_string(document.form.rt_radius_key))
		return false;
	
	return true;
}

function done_validating(action){
	refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(2, 3);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_WSecurity2g_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WAdvanced_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANAuthentication11a;WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="rt_ssid2" value="<% nvram_get_x("WLANConfig11b",  "rt_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="23">&nbsp;</td>
		
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
		
<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
	<thead>
	<tr>
		<td><#t1Wireless#> - <#t2RADIUS#> (2.4GHz)</td>
	</tr>
	</thead>
	<tbody>
	<tr>
		<td bgcolor="#FFFFFF"><#WLANAuthentication11a_display1_sectiondesc#></td>
	</tr>
	</tbody>	
	<tr>
	  <td bgcolor="#FFFFFF">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<tr>
			  <th>
			  <a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,1);">
			  <#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a>			  </th>
			  <td><input type="text" maxlength="15" class="input" size="15" name="rt_radius_ipaddr" value="<% nvram_get_x("WLANAuthentication11a","rt_radius_ipaddr"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
			  </tr>
			<tr>
			  <th>
			  <a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,2);">
			  <#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a></th>
			  <td>
			    <input type="text" maxlength="5" class="input" size="5" name="rt_radius_port" value="<% nvram_get_x("WLANAuthentication11a","rt_radius_port"); %>" onkeypress="return is_number(this)" onblur="return validate_portrange(this, '')"/></td>
			  </tr>
			<tr>
				<th >
				<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(2,3);">
				<#WLANAuthentication11a_ExAuthDBPassword_itemname#>
				</a>
				</th>
				<td>
				  <input type="password" maxlength="64" class="input" size="32" name="rt_radius_key" value="<% nvram_get_x("WLANAuthentication11a","rt_radius_key"); %>"></td>
			</tr>
			<tr align="right">
				<td colspan="2">
					<input type="button" class="button5" value="<#GO_5G#>" onclick="location.href='Advanced_WSecurity_Content.asp';">    
					<input class="button" onclick="applyRule()" type="button" value="<#CTL_apply#>"/></td>
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
	  <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>">
	  	<img src="images/help.gif" />
	  </div>
	  
	  <div id="hintofPM" style="display:none;">
		<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
		  <thead>
		  <tr>
			<td>
			  <div id="helpname" class="AiHintTitle"></div>
			  <a href="javascript:void(0);" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
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
